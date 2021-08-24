# WiFi Provisioning

This project handle the WiFi provisioning using WiFiProv library for Arduino Core of ESP32.
WiFiProv is arguably the best way for you to connect your ESP32 to an access point without hardcoding any WiFi credentials on the firmware.

## What do you need?

You need to have an Android phone or an iPhone and install the ESP32 SoftAP Prov app from either Google Play Store or Apple App Store.

You can download the app from the Play Store [here](https://play.google.com/store/apps/details?id=com.espressif.provsoftap). Or you can also download the app from App Store [here](https://apps.apple.com/us/app/esp-softap-provisioning/id1474040630).

## Reset Provisioning Data

You can reset the stored provisioning data which includes the credentials for connecting to a WiFi access point by pressing the GPIO 0 or Boot button on the ESP-WROVER-KIT.