#ifndef ESP32_AZURE_MY_DEVICE_TWINS_H
#define ESP32_AZURE_MY_DEVICE_TWINS_H

#include "parson.h"
#include "iothub_device_client_ll.h"
#include <stdint.h>

typedef struct DESIRED_PROPERTIES_TAG
{
    uint16_t tx_interval;
} DESIRED_PROPERTIES;

typedef struct REPORTED_PROPERTIES_TAG
{
    char firmware_version[12];
} REPORTED_PROPERTIES;

typedef struct DEVICE_TWIN_TAGS
{
    DESIRED_PROPERTIES desired;
    REPORTED_PROPERTIES reported;
} MY_DEVICE_TWIN;

MY_DEVICE_TWIN *my_device_twin_init();
void my_device_twin_deinit(MY_DEVICE_TWIN *twin);
void my_device_twin_print(MY_DEVICE_TWIN *twin);
void my_device_twin_save(MY_DEVICE_TWIN *twin);

char *my_device_twin_serialize_reported_props(MY_DEVICE_TWIN *twin);
MY_DEVICE_TWIN *my_device_twin_parse_desired_props(const char *json, DEVICE_TWIN_UPDATE_STATE update_state);

#endif