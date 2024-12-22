#ifndef AZURE_MANAGER_H
#define AZURE_MANAGER_H

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <az_core.h>
#include <az_iot.h>

class AzureManager
{
public:
    AzureManager(const char *host, int port, const char *deviceId, const char *deviceKey);
    void initialize();
    void sendToAzure(float temperature, float humidity, float pressure, float luminosity);
    IPAddress getIP();
    bool isConnected();
    void establishConnection();

private:
    WiFiClientSecure wifi_client;
    PubSubClient client;
    const char *host;
    int port;
    const char *deviceId;
    const char *deviceKey;

    // Azure IoT
    az_iot_hub_client iot_hub_client;
    char sas_token[200];
};
#endif