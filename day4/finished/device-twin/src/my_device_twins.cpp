#include "my_device_twins.h"
#include "parson.h"
#include "stdlib.h"

char *serializeReportedProps(MY_DEVICE_TWIN *twin)
{
    char *result;

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    json_object_dotset_string(root_object, "firmware_version", twin->reported.firmware_version);

    result = json_serialize_to_string(root_value);
    json_value_free(root_value);

    return result;
}

MY_DEVICE_TWIN* parseDesiredProps(const char* json, DEVICE_TWIN_UPDATE_STATE update_state)
{
    MY_DEVICE_TWIN *device = (MY_DEVICE_TWIN*)malloc(sizeof(MY_DEVICE_TWIN));
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