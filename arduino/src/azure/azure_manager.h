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
    bool isConnected();
    void establishConnection();
    void keepConnection();

private:
    WiFiClientSecure wifi_client;
    PubSubClient client;
    const char *host;     // Azure IoT Hub hostname
    int port;             // MQTT port
    const char *deviceId; // Device ID in IoT Hub
    const char *deviceKey;

    az_iot_hub_client iot_hub_client;
    char sas_token[200];

    // void generateSasToken(char *sas_token, size_t token_size, const char *resourceUri, const char *deviceKey, int expirySeconds);
};
#endif