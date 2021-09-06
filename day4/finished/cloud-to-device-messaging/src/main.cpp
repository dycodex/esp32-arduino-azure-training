#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <AzureIoTSocket_WiFi.h>
#include <AzureIoTHub.h>

#include "azure_c_shared_utility/tickcounter.h"

#include "iot_config.h"
#include "sample_init.h"

#include "Esp.h"

#include "AzureIoTProtocol_MQTT.h"
#include "iothubtransportmqtt.h"

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_SHTC3.h>

Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();

#include "my_device_twins.h"

static const char ssid[] = IOT_CONFIG_WIFI_SSID;
static const char pass[] = IOT_CONFIG_WIFI_PASSWORD;

/* Define several constants/global variables */
static const char *connectionString = DEVICE_CONNECTION_STRING;
static bool g_continueRunning = true; // defines whether or not the device maintains its IoT Hub connection after sending (think receiving messages from the cloud)
static size_t g_message_count_send_confirmations = 0;
static bool g_run_demo = true;

IOTHUB_MESSAGE_HANDLE message_handle;
size_t messages_sent = 0;
static const int message_count = 20; // determines the number of times the device tries to send a message to the IoT Hub in the cloud.

const char *quit_msg = "quit";
const char *exit_msg = "exit";

IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;

MY_DEVICE_TWIN *twin;

/* -- device_twin_callback --
 * Callback method which handle incoming device twin update from Azure IoT Hub
 */
static void device_twin_callback(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char *payload, size_t size, void *user_context)
{
    Serial.printf("Received device twin:\r\n");
    Serial.printf("<<<%.*s>>>\r\n", (int)size, payload);

    bool has_desired_props_changed = false;

    MY_DEVICE_TWIN *old_device = (MY_DEVICE_TWIN *)user_context;
    MY_DEVICE_TWIN *new_device = my_device_twin_parse_desired_props((const char *)payload, update_state);

    if (new_device == NULL)
    {
        printf("Failed parsing json from device twin\r\n");
        return;
    }

    if (new_device->desired.tx_interval > 0 && new_device->desired.tx_interval != old_device->desired.tx_interval)
    {
        printf("Received new tx_interval\r\n");
        old_device->desired.tx_interval = new_device->desired.tx_interval;

        has_desired_props_changed = true;
    }

    if (has_desired_props_changed)
    {
        my_device_twin_save(old_device);
    }

    free(new_device);
}

/* -- reported_state_callback --
 * This callback function will be called when device has updated it's reported state
 */
static void reported_state_callback(int status_code, void *user_context)
{
    (void)user_context;
    LogInfo("Device twin reported properties updated with result: %d\r\n", status_code);
}

/* -- c2d_message_callback --
 * This callback function will be called when device has updated it's reported state
 */
static IOTHUBMESSAGE_DISPOSITION_RESULT c2d_message_callback(IOTHUB_MESSAGE_HANDLE message, void *user_context)
{
    const char *buffer;
    size_t size;
    MAP_HANDLE mapProperties;
    const char *messageId;
    const char *correlationId;

    // Message properties
    if ((messageId = IoTHubMessage_GetMessageId(message)) == NULL)
    {
        messageId = "<null>";
    }

    if ((correlationId = IoTHubMessage_GetCorrelationId(message)) == NULL)
    {
        correlationId = "<null>";
    }

    if (IoTHubMessage_GetByteArray(message, (const unsigned char **)&buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        LogInfo("Unable to retrieve the message data\r\n");
    }
    else
    {
        LogInfo("Received message:\r\nMessage ID: %s\r\nCorrelation ID: %s\r\nData: <<<%.*s>>> & Size=%d\r\n", messageId, correlationId, (int)size, buffer, (int)size);
    }

    mapProperties = IoTHubMessage_Properties(message);
    if (mapProperties != NULL)
    {
        const char *const *keys;
        const char *const *values;
        size_t propertyCount = 0;
        if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
        {
            if (propertyCount > 0)
            {
                size_t index;

                printf(" Message Properties:\r\n");
                for (index = 0; index < propertyCount; index++)
                {
                    printf("\tKey: %s Value: %s\r\n", keys[index], values[index]);
                }
                printf("\r\n");
            }
        }
    }

    return IOTHUBMESSAGE_ACCEPTED;
}

/* -- send_confirm_callback --
 * Callback method which executes upon confirmation that a message originating from this device has been received by the IoT Hub in the cloud.
 */
static void send_confirm_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    (void)userContextCallback;
    // When a message is sent this callback will get envoked
    g_message_count_send_confirmations++;
    LogInfo("Confirmation callback received for message %lu with result %s\r\n", (unsigned long)g_message_count_send_confirmations, MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}

/* -- connection_status_callback --
 * Callback method which executes on receipt of a connection status message from the IoT Hub in the cloud.
 */
static void connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void *user_context)
{
    (void)reason;
    (void)user_context;
    // This sample DOES NOT take into consideration network outages.
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
    {
        LogInfo("The device client is connected to iothub\r\n");
    }
    else
    {
        LogInfo("The device client has been disconnected\r\n");
    }
}

/* -- reset_esp_helper -- 
 * waits for call of exit_msg over Serial line to reset device
 */
static void reset_esp_helper()
{
#ifdef is_esp_board
    // Read from local serial
    if (Serial.available())
    {
        String s1 = Serial.readStringUntil('\n'); // s1 is String type variable.
        Serial.print("Received Data: ");
        Serial.println(s1); //display same received Data back in serial monitor.

        // Restart device upon receipt of 'exit' call.
        int e_start = s1.indexOf('e');
        String ebit = (String)s1.substring(e_start, e_start + 4);
        if (ebit == exit_msg)
        {
            ESP.restart();
        }
    }
#endif // is_esp_board
}

/* -- run_demo --
 * Runs active task of sending telemetry to IoTHub
 * WARNING: only call this function once, as it includes steps to destroy handles and clean up at the end.
 */
static void run_demo()
{
    TICK_COUNTER_HANDLE tick_counter_handle = tickcounter_create();
    tickcounter_ms_t current_tick;
    tickcounter_ms_t last_send_time = 0;

    // action phase of the program, sending messages to the IoT Hub in the cloud.
    do
    {
        if (messages_sent < message_count)
        {
            (void)tickcounter_get_current_ms(tick_counter_handle, &current_tick);
            if ((current_tick - last_send_time) / 1000 > twin->desired.tx_interval)
            {
                sensors_event_t humidity, temp;

                shtc3.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
                char message_content[64];
                sprintf(message_content, "{\"temp\":%f, \"hum\": %f}", temp.temperature, humidity.relative_humidity);
                // Construct the iothub message from a string or a byte array
                message_handle = IoTHubMessage_CreateFromString(message_content);
                //message_handle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText)));

                LogInfo("Sending message %d to IoTHub\r\n", (int)(messages_sent + 1));
                IoTHubDeviceClient_LL_SendEventAsync(device_ll_handle, message_handle, send_confirm_callback, NULL);
                // The message is copied to the sdk so the we can destroy it
                IoTHubMessage_Destroy(message_handle);

                messages_sent++;

                (void)tickcounter_get_current_ms(tick_counter_handle, &last_send_time);
            }
        }
        else if (g_message_count_send_confirmations >= message_count)
        {
            // After all messages are all received stop running
            g_continueRunning = false;
        }

        IoTHubDeviceClient_LL_DoWork(device_ll_handle);
        ThreadAPI_Sleep(10);
        reset_esp_helper();

    } while (g_continueRunning);

    // Clean up the iothub sdk handle
    IoTHubDeviceClient_LL_Destroy(device_ll_handle);
    // Free all the sdk subsystem
    IoTHub_Deinit();

    LogInfo("done with sending");
    return;
}

void setup()
{
    IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol = MQTT_Protocol;

    shtc3.begin();

    sample_init(ssid, pass);

    // Used to initialize IoTHub SDK subsystem
    (void)IoTHub_Init();
    // Create the iothub handle here
    device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString, protocol);
    LogInfo("Creating IoTHub Device handle\r\n");

    if (device_ll_handle == NULL)
    {
        LogInfo("Error AZ002: Failure creating Iothub device. Hint: Check you connection string.\r\n");
    }
    else
    {
        // Set any option that are neccessary.
        // For available options please see the iothub_sdk_options.md documentation in the main C SDK
        // turn off diagnostic sampling
        int diag_off = 0;
        IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_DIAGNOSTIC_SAMPLING_PERCENTAGE, &diag_off);

        // Setting the Trusted Certificate.
        IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_TRUSTED_CERT, certificates);

        //Setting the auto URL Encoder (recommended for MQTT). Please use this option unless
        //you are URL Encoding inputs yourself.
        //ONLY valid for use with MQTT
        bool urlEncodeOn = true;
        IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_AUTO_URL_ENCODE_DECODE, &urlEncodeOn);

        // initialize device twin
        twin = my_device_twin_init();
        my_device_twin_print(twin);

        // Setting connection status callback to get indication of connection to iothub
        (void)IoTHubDeviceClient_LL_SetConnectionStatusCallback(device_ll_handle, connection_status_callback, NULL);

        // Setting cloud-to-device messages callback
        (void)IoTHubDeviceClient_LL_SetMessageCallback(device_ll_handle, c2d_message_callback, NULL);

        // Set device twin status callback
        (void)IoTHubDeviceClient_LL_SetDeviceTwinCallback(device_ll_handle, device_twin_callback, twin);

        if (strcmp(FIRMWARE_VERSION, twin->reported.firmware_version) != 0)
        {
            // Firmware version has changed! Report to IoT Hub.
            strcpy(twin->reported.firmware_version, FIRMWARE_VERSION);
            my_device_twin_save(twin);

            char *reported_props = my_device_twin_serialize_reported_props(twin);
            IoTHubDeviceClient_LL_SendReportedState(device_ll_handle, (const uint8_t *)reported_props, strlen(reported_props), reported_state_callback, NULL);
        }
    }
}

void loop(void)
{
    if (g_run_demo)
    {
        run_demo();
        g_run_demo = false;
    }
    reset_esp_helper();
}
