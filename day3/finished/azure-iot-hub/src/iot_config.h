#ifndef IOT_CONFIG_H
#define IOT_CONFIG_H

/**
 * WiFi setup
 */
#define IOT_CONFIG_WIFI_SSID            "Dyware 3"
#define IOT_CONFIG_WIFI_PASSWORD        "p@ssw0rd"

/**
 * IoT Hub Device Connection String setup
 * Find your Device Connection String by going to your Azure portal, creating (or navigating to) an IoT Hub, 
 * navigating to IoT Devices tab on the left, and creating (or selecting an existing) IoT Device. 
 * Then click on the named Device ID, and you will have able to copy the Primary or Secondary Device Connection String to this sample.
 */
#define DEVICE_CONNECTION_STRING        "HostName=alwinhub.azure-devices.net;DeviceId=wroverkit0;SharedAccessKey=yeDodRMb3qlSF6j/pmsjQopBsNtxBCVi5u/Xvj0bGEQ="

// The protocol you wish to use should be uncommented
//
#define SAMPLE_MQTT
//#define SAMPLE_HTTP


#endif