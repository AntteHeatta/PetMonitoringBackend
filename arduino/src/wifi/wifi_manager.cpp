#include "wifi_manager.h"
#include <ESP8266WiFi.h>

WiFiManager::WiFiManager(const char *ssid, const char *pass)
    : ssid(ssid), pass(pass) {}

void WiFiManager::initialize()
{
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Attempting to connect to WiFi");
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