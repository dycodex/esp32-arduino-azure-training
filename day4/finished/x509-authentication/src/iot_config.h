#ifndef IOT_CONFIG_H
#define IOT_CONFIG_H

/**
 * WiFi setup
 */
#define IOT_CONFIG_WIFI_SSID            ""
#define IOT_CONFIG_WIFI_PASSWORD        ""

/**
 * IoT Hub Device Connection String setup
 * Find your Device Connection String by going to your Azure portal, creating (or navigating to) an IoT Hub, 
 * navigating to IoT Devices tab on the left, and creating (or selecting an existing) IoT Device. 
 * Then click on the named Device ID, and you will have able to copy the Primary or Secondary Device Connection String to this sample.
 */

// DEVICE_CONNECTION_STRING format -> HostName=hostname.azure-devices.net;DeviceId=common-name;x509=true
#define DEVICE_CONNECTION_STRING        ""


#endif