#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <AzureIoTSocket_WiFi.h>
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_HTTP.h>
#include <AzureIoTProtocol_AMPQ.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>
#include "iot_config.h"
#include "iothub_prov_sample.h"
#include "esp32/sample_init.h"

void azure_task(void*data)
{
    iothub_prov_sample_run();
    vTaskDelete(NULL);
}

void setup()
{
    esp32_sample_init(WIFI_SSID, WIFI_PASSPHRASE);
    xTaskCreate(azure_task,"azure_task", 1024*10, NULL, 10, NULL);
}

void loop()
{

}