#ifndef AZURE_MANAGER_H
#define AZURE_MANAGER_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

class AzureManager
{
public:
    AzureManager(const char *mqttServer, int mqttPort, const char *deviceId, const char *sasToken);
    void initialize();
    void sendToAzure(float temperature, float humidity, float luminosity);
    void reconnect();
    IPAddress getIP();
    bool isConnected();

private:
    WiFiClientSecure espClient; // Secure Wi-Fi client
    PubSubClient client;        // MQTT client
    const char *mqttServer;     // Azure IoT Hub hostname
    int mqttPort;               // MQTT port (usually 8883 for secure)
    const char *deviceId;       // Device ID registered in IoT Hub
    const char *sasToken;       // SAS token for authentication
};
#endif