#include "azure/azure_manager.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "credentials.h"

const char *publishTopic = "devices/arduino_device/messages/events/";

AzureManager::AzureManager(const char *mqttServer, int mqttPort, const char *deviceId, const char *sasToken)
    : espClient(), client(espClient), mqttServer(mqttServer), mqttPort(mqttPort), deviceId(deviceId), sasToken(sasToken)
{
}

void AzureManager::initialize()
{
    client.setServer(mqttServer, mqttPort);
}

void AzureManager::sendToAzure(float temperature, float humidity, float luminosity)
{
    if (!client.connected())
    {
        reconnect();
    }

    client.loop();
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["temperature"] = temperature - 3;
    jsonDoc["humidity"] = humidity;
    jsonDoc["luminosity"] = luminosity;

    char jsonBuffer[256];
    serializeJson(jsonDoc, jsonBuffer);

    client.publish(publishTopic, jsonBuffer);
}

bool AzureManager::isConnected()
{
    return client.connected();
}

IPAddress AzureManager::getIP()
{
    return WiFi.localIP();
}

void AzureManager::reconnect()
{
    while (!client.connected())
    {
        if (client.connect(deviceId, iotHubUser, iotHubPass))
        {
            Serial.println("Connected to Azure IoT Hub");
        }
        else
        {
            Serial.print("Failed to connect to Azure IoT Hub, state: ");
            Serial.println(client.state());
            delay(5000);
        }
    }
}