#include "wifi_manager.h"
#include <WiFiEsp.h>
#include <SoftwareSerial.h>

WiFiManager::WiFiManager(int rxPin, int txPin, const char *ssid, const char *pass)
    : espSerial(rxPin, txPin), ssid(ssid), pass(pass) {}

void WiFiManager::initialize()
{
    espSerial.begin(115200);
    WiFi.init(&espSerial);

    if (WiFi.status() == WL_NO_SHIELD)
    {
        Serial.println("WiFi module not connected");
        while (true)
            ;
    }
}

void WiFiManager::connect()
{
    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(ssid, pass);
        Serial.print("Attempting to connect to: ");
        Serial.println(ssid);
        delay(8000);
    }

    Serial.print("Connected to WiFi: ");
    Serial.println(ssid);
}

bool WiFiManager::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

IPAddress WiFiManager::getIP()
{
    return WiFi.localIP();
}
