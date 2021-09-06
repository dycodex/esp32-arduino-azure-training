#ifndef ESP32_AZURE_MY_DEVICE_TWINS_H
#define ESP32_AZURE_MY_DEVICE_TWINS_H

#include "parson.h"
#include "iothub_device_client_ll.h"

typedef struct DESIRED_PROPERTIES_TAG
{
    unsigned int tx_interval;
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

char* serializeReportedProps(MY_DEVICE_TWIN *twin);
MY_DEVICE_TWIN* parseDesiredProps(const char* json, DEVICE_TWIN_UPDATE_STATE update_state);

#endif