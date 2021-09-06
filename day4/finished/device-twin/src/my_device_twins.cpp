#include "my_device_twins.h"
#include "parson.h"
#include "stdlib.h"
#include <Preferences.h>
#include "iot_config.h"

static Preferences prefs;

static const char *default_firmware_version = FIRMWARE_VERSION;
static const uint16_t default_tx_interval = 30;

MY_DEVICE_TWIN *my_device_twin_init()
{
    static bool has_initted = false;
    if (!has_initted)
    {
        prefs.begin("twin");
        has_initted = true;
    }

    size_t twin_size = sizeof(MY_DEVICE_TWIN);
    MY_DEVICE_TWIN *twin = (MY_DEVICE_TWIN *)malloc(twin_size);
    memset(twin, 0, twin_size);

    char buffer[twin_size] = {0};
    size_t read_bytes = prefs.getBytes("twin", (void *)buffer, twin_size);

    log_i("read: %d/%d", read_bytes, twin_size);
    if (read_bytes == twin_size)
    {
        memcpy(twin, buffer, twin_size);
    }
    else
    {
        twin->desired.tx_interval = default_tx_interval;
        sprintf(twin->reported.firmware_version, "%s", default_firmware_version);

        my_device_twin_save(twin);
    }

    return twin;
}

void my_device_twin_deinit(MY_DEVICE_TWIN *twin)
{
    if (twin == NULL)
    {
        return;
    }

    free(twin);
}

void my_device_twin_print(MY_DEVICE_TWIN *twin)
{
    if (twin == NULL)
    {
        return;
    }
    Serial.println("-----BEGIN TWIN DUMP-----");
    Serial.printf("desired.tx_interval: %u\r\n", twin->desired.tx_interval);
    Serial.printf("reported.firmware_version: %s\r\n", twin->reported.firmware_version);
    Serial.println("------END TWIN DUMP------");
}

void my_device_twin_save(MY_DEVICE_TWIN *twin)
{
    if (twin == NULL)
    {
        return;
    }

    size_t twin_size = sizeof(MY_DEVICE_TWIN);
    size_t written = prefs.putBytes("twin", (const void *)twin, twin_size);
    log_i("written %d/%d", written, twin_size);
}

char *my_device_twin_serialize_reported_props(MY_DEVICE_TWIN *twin)
{
    char *result;

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    json_object_dotset_string(root_object, "firmware_version", twin->reported.firmware_version);

    result = json_serialize_to_string(root_value);
    json_value_free(root_value);

    return result;
}

MY_DEVICE_TWIN *my_device_twin_parse_desired_props(const char *json, DEVICE_TWIN_UPDATE_STATE update_state)
{
    MY_DEVICE_TWIN *device = (MY_DEVICE_TWIN *)malloc(sizeof(MY_DEVICE_TWIN));
    JSON_Value *root_value = NULL;
    JSON_Object *root_object = NULL;

    if (device == NULL)
    {
        printf("Failed to allocate memory for incoming device twin!\r\n");
        return NULL;
    }

    memset(device, 0, sizeof(MY_DEVICE_TWIN));
    root_value = json_parse_string(json);
    root_object = json_value_get_object(root_value);

    JSON_Value *desiredTxInterval;

    if (update_state == DEVICE_TWIN_UPDATE_COMPLETE)
    {
        desiredTxInterval = json_object_dotget_value(root_object, "desired.tx_interval");
    }
    else
    {
        desiredTxInterval = json_object_get_value(root_object, "tx_interval");
    }

    if (desiredTxInterval != NULL)
    {
        unsigned int data = (unsigned int)json_value_get_number(desiredTxInterval);
        if (data > 0)
        {
            device->desired.tx_interval = data;
        }
    }

    json_value_free(root_value);

    return device;
}