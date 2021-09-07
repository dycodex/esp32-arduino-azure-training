// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>

#include "iothub.h"
#include "iothub_message.h"
#include "iothub_client_version.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/http_proxy_io.h"

#include "iothub_device_client_ll.h"
#include "iothub_client_options.h"
#include "azure_prov_client/prov_device_ll_client.h"
#include "azure_prov_client/prov_security_factory.h"
#include "sdkconfig.h"

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_SHTC3.h>

Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();

#include "iot_config.h"

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
#include "certs/certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

#include "iothubtransportmqtt.h"
#include "azure_prov_client/prov_transport_mqtt_client.h"

MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(PROV_DEVICE_RESULT, PROV_DEVICE_RESULT_VALUE);
MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(PROV_DEVICE_REG_STATUS, PROV_DEVICE_REG_STATUS_VALUES);

static const char *global_prov_uri = "global.azure-devices-provisioning.net";
static const char *id_scope = AZURE_DPS_ID_SCOPE;

#define MESSAGES_TO_SEND 20

typedef struct CLIENT_SAMPLE_INFO_TAG
{
    unsigned int sleep_time;
    char *iothub_uri;
    char *access_key_name;
    char *device_key;
    char *device_id;
    int registration_complete;
} CLIENT_SAMPLE_INFO;

typedef struct IOTHUB_CLIENT_SAMPLE_INFO_TAG
{
    int connected;
    int stop_running;
} IOTHUB_CLIENT_SAMPLE_INFO;

static IOTHUBMESSAGE_DISPOSITION_RESULT receive_msg_callback(IOTHUB_MESSAGE_HANDLE message, void *user_context)
{
    (void)message;
    IOTHUB_CLIENT_SAMPLE_INFO *iothub_info = (IOTHUB_CLIENT_SAMPLE_INFO *)user_context;
    (void)printf("Stop message received from IoT Hub\r\n");
    iothub_info->stop_running = 1;

    return IOTHUBMESSAGE_ACCEPTED;
}

static void registration_status_callback(PROV_DEVICE_REG_STATUS reg_status, void *user_context)
{
    (void)user_context;
    (void)printf("Provisioning status: %s\r\n", MU_ENUM_TO_STRING(PROV_DEVICE_REG_STATUS, reg_status));
}

static void iothub_connection_status(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void *user_context)
{
    (void)reason;
    if (user_context == NULL)
    {
        printf("iothub_connection_status user_context is null!\r\n");
    }
    else
    {
        IOTHUB_CLIENT_SAMPLE_INFO *iothub_info = (IOTHUB_CLIENT_SAMPLE_INFO *)user_context;
        if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
        {
            iothub_info->connected = 1;
        }
        else
        {
            iothub_info->connected = 0;
            iothub_info->stop_running = 1;
        }
    }
}

static void register_device_callback(PROV_DEVICE_RESULT register_result, const char *iothub_uri, const char *device_id, void *user_context)
{
    if (user_context == NULL)
    {
        printf("user_context is null!\r\n");
    }
    else
    {
        CLIENT_SAMPLE_INFO *user_ctx = (CLIENT_SAMPLE_INFO *)user_context;
        if (register_result == PROV_DEVICE_RESULT_OK)
        {
            (void)printf("Registration information received from service: %s!\r\n", iothub_uri);
            (void)mallocAndStrcpy_s(&user_ctx->iothub_uri, iothub_uri);
            (void)mallocAndStrcpy_s(&user_ctx->device_id, device_id);
            user_ctx->registration_complete = 1;
        }
        else
        {
            (void)printf("Failure encountered on registration %s\r\n", MU_ENUM_TO_STRING(PROV_DEVICE_RESULT, register_result));
            user_ctx->registration_complete = 2;
        }
    }
}

int iothub_prov_sample_run(void)
{
    shtc3.begin();

    SECURE_DEVICE_TYPE hsm_type;
    hsm_type = SECURE_DEVICE_TYPE_X509;

    bool trace_on = false;

    (void)IoTHub_Init();
    (void)prov_dev_security_init(hsm_type);

    PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION prov_transport;
    CLIENT_SAMPLE_INFO user_ctx;

    memset(&user_ctx, 0, sizeof(CLIENT_SAMPLE_INFO));

    prov_transport = Prov_Device_MQTT_Protocol;

    user_ctx.registration_complete = 0;
    user_ctx.sleep_time = 50;

    printf("Provisioning API Version: %s\r\n", Prov_Device_LL_GetVersionString());
    printf("Iothub API Version: %s\r\n", IoTHubClient_GetVersionString());

    PROV_DEVICE_LL_HANDLE handle;
    if ((handle = Prov_Device_LL_Create(global_prov_uri, id_scope, prov_transport)) == NULL)
    {
        (void)printf("failed calling Prov_Device_LL_Create\r\n");
    }
    else
    {
        Prov_Device_LL_SetOption(handle, PROV_OPTION_LOG_TRACE, &trace_on);
#ifdef SET_TRUSTED_CERT_IN_SAMPLES
        Prov_Device_LL_SetOption(handle, OPTION_TRUSTED_CERT, certificates);
#endif

        if (Prov_Device_LL_Register_Device(handle, register_device_callback, &user_ctx, registration_status_callback, &user_ctx) != PROV_DEVICE_RESULT_OK)
        {
            (void)printf("failed calling Prov_Device_LL_Register_Device\r\n");
        }
        else
        {
            do
            {
                Prov_Device_LL_DoWork(handle);
                ThreadAPI_Sleep(user_ctx.sleep_time);

            } while (user_ctx.registration_complete == 0);
        }

        Prov_Device_LL_Destroy(handle);
    }

    if (user_ctx.registration_complete != 1)
    {
        (void)printf("registration failed!\r\n");
    }
    else
    {
        IOTHUB_CLIENT_TRANSPORT_PROVIDER iothub_transport;
        iothub_transport = MQTT_Protocol;

        IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;
        (void)printf("Creating IoTHub Device handle\r\n");
        if ((device_ll_handle = IoTHubDeviceClient_LL_CreateFromDeviceAuth(user_ctx.iothub_uri, user_ctx.device_id, iothub_transport)) == NULL)
        {
            (void)printf("failed create IoTHub client from connection string %s!\r\n", user_ctx.iothub_uri);
        }
        else
        {
            IOTHUB_CLIENT_SAMPLE_INFO iothub_info;
            TICK_COUNTER_HANDLE tick_counter_handle = tickcounter_create();
            tickcounter_ms_t current_tick;
            tickcounter_ms_t last_send_time = 0;
            size_t msg_count = 0;
            iothub_info.stop_running = 0;
            iothub_info.connected = 0;

            (void)IoTHubDeviceClient_LL_SetConnectionStatusCallback(device_ll_handle, iothub_connection_status, &iothub_info);

            // Set any option that are neccessary.
            // For available options please see the iothub_sdk_options.md documentation

            IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_LOG_TRACE, &trace_on);

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
            // Setting the Trusted Certificate.  This is only necessary on system with without
            // built in certificate stores.
            IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_TRUSTED_CERT, certificates);
#endif // SET_TRUSTED_CERT_IN_SAMPLES

            (void)IoTHubDeviceClient_LL_SetMessageCallback(device_ll_handle, receive_msg_callback, &iothub_info);
            (void)printf("Sending 1 message to IoT Hub every %d seconds for %d messages (Send any message to stop)\r\n", TX_INTERVAL_SECOND, MESSAGES_TO_SEND);

            do
            {
                if (iothub_info.connected != 0)
                {
                    (void)tickcounter_get_current_ms(tick_counter_handle, &current_tick);
                    if ((current_tick - last_send_time) / 1000 > TX_INTERVAL_SECOND)
                    {
                        sensors_event_t humidity, temp;

                        shtc3.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
                        char message_content[64];
                        sprintf(message_content, "{\"temp\":%f, \"hum\": %f}", temp.temperature, humidity.relative_humidity);

                        IOTHUB_MESSAGE_HANDLE msg_handle = IoTHubMessage_CreateFromByteArray((const unsigned char *)message_content, strlen(message_content));
                        if (msg_handle == NULL)
                        {
                            (void)printf("ERROR: iotHubMessageHandle is NULL!\r\n");
                        }
                        else
                        {

                            if (IoTHubDeviceClient_LL_SendEventAsync(device_ll_handle, msg_handle, NULL, NULL) != IOTHUB_CLIENT_OK)
                            {
                                (void)printf("ERROR: IoTHubClient_LL_SendEventAsync..........FAILED!\r\n");
                            }
                            else
                            {
                                (void)tickcounter_get_current_ms(tick_counter_handle, &last_send_time);
                                (void)printf("IoTHubClient_LL_SendEventAsync accepted message [%zu] for transmission to IoT Hub.\r\n", msg_count);
                            }
                            IoTHubMessage_Destroy(msg_handle);
                        }
                    }
                }

                IoTHubDeviceClient_LL_DoWork(device_ll_handle);
                ThreadAPI_Sleep(50);
            } while (iothub_info.stop_running == 0 && msg_count < MESSAGES_TO_SEND);

            size_t index = 0;
            for (index = 0; index < 10; index++)
            {
                IoTHubClientCore_LL_DoWork(device_ll_handle);
                ThreadAPI_Sleep(1);
            }

            tickcounter_destroy(tick_counter_handle);
            IoTHubDeviceClient_LL_Destroy(device_ll_handle);
        }
    }

    free(user_ctx.iothub_uri);
    free(user_ctx.device_id);
    prov_dev_security_deinit();

    IoTHub_Deinit();

    return 0;
}
