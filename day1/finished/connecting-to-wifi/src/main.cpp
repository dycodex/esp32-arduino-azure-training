#include <Arduino.h>
#include <WiFi.h>

const char* wifiSSID = "";
const char* wifiPassword = "";

void setup()
{
    Serial.begin(115200);

    WiFi.begin(wifiSSID, wifiPassword);

    Serial.print("Connecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nConnected!");
    Serial.println(WiFi.localIP());
}

void loop()
{

}