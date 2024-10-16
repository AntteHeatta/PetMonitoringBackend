#include <cstdlib>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "azure/azure_manager.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <base64.h>
#include <bearssl/bearssl.h>
#include <bearssl/bearssl_hmac.h>
#include <libb64/cdecode.h>
#include <az_core.h>
#include <az_iot.h>
#include <azure_ca.h>

#include "credentials.h"

const char *publishTopic = "devices/esp8266_gradu/messages/events/";
WiFiClientSecure wifi_client;

static X509List cert((const char *)ca_pem);
static PubSubClient mqtt_client(wifi_client);
static az_iot_hub_client iot_hub_client;
static char sas_token[300];
static uint8_t signature[512];
static unsigned char encrypted_signature[32];
static char base64_decoded_device_key[32];

#define AZURE_SDK_CLIENT_USER_AGENT "c%2F" AZ_SDK_VERSION_STRING "(ard;esp8266)"
#define NTP_SERVERS "pool.ntp.org", "time.nist.gov"
#define MQTT_PACKET_SIZE 2048
#define ONE_HOUR_IN_SECS 3600
#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))

AzureManager::AzureManager(const char *host, int port, const char *deviceId, const char *deviceKey)
    : client(wifi_client), host(host), port(port), deviceId(deviceId), deviceKey(deviceKey)
{
}

void AzureManager::initialize()
{
    Serial.println("AzureManager::initialize()");
    establishConnection();
}

void AzureManager::keepConnection()
{
    if (!mqtt_client.connected())
    {
        Serial.println("keepConnection no mqtt connected... reconnecting");
        establishConnection();
    }
    mqtt_client.loop();
}

void AzureManager::sendToAzure(float temperature, float humidity, float pressure, float luminosity)
{
    static unsigned long lastTokenRefresh = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - lastTokenRefresh > 3300000)
    {
        Serial.println("Token refresh");
        establishConnection();
        lastTokenRefresh = currentMillis;
    }

    Serial.println("-----");
    Serial.println("sendToAzure");
    if (!mqtt_client.connected())
    {
        Serial.println("!mqtt_client.connected()");
        establishConnection();
    }

    StaticJsonDocument<200> jsonDoc;
    jsonDoc["temperature"] = temperature;
    jsonDoc["humidity"] = humidity;
    jsonDoc["pressure"] = pressure;
    jsonDoc["luminosity"] = luminosity;

    char jsonBuffer[256];
    serializeJson(jsonDoc, jsonBuffer);
    Serial.print("jsonBuffer");
    Serial.println(jsonBuffer);

    mqtt_client.publish(publishTopic, jsonBuffer, false);
}

bool AzureManager::isConnected()
{
    return mqtt_client.connected();
}

IPAddress AzureManager::getIP()
{
    return WiFi.localIP();
}

// From Azure SDK for C documentation examples
void receivedCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Received [");
    Serial.print(topic);
    Serial.print("]: ");
    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println("");
}

void initializeClients()
{
    az_iot_hub_client_options options = az_iot_hub_client_options_default();
    options.user_agent = AZ_SPAN_FROM_STR(AZURE_SDK_CLIENT_USER_AGENT);

    wifi_client.setTrustAnchors(&cert);
    if (az_result_failed(az_iot_hub_client_init(
            &iot_hub_client,
            az_span_create((uint8_t *)host, strlen(host)),
            az_span_create((uint8_t *)deviceId, strlen(deviceId)),
            &options)))
    {
        Serial.println("Failed initializing Azure IoT Hub client");
        return;
    }

    mqtt_client.setServer(host, port);
    mqtt_client.setCallback(receivedCallback);
}

static uint32_t getSecondsSinceEpoch() { return (uint32_t)time(NULL); }

static int connectToAzureIoTHub()
{
    size_t client_id_length;
    char mqtt_client_id[128];
    if (az_result_failed(az_iot_hub_client_get_client_id(
            &iot_hub_client, mqtt_client_id, sizeof(mqtt_client_id) - 1, &client_id_length)))
    {
        Serial.println("Failed getting client id");
        return 1;
    }

    mqtt_client_id[client_id_length] = '\0';

    char mqtt_username[128];

    Serial.print("Client ID: ");
    Serial.println(mqtt_client_id);

    Serial.print("Username: ");
    Serial.println(mqtt_username);

    mqtt_client.setBufferSize(MQTT_PACKET_SIZE);

    Serial.print("sas_token: ");
    Serial.println(sas_token);
    while (!mqtt_client.connected())
    {
        time_t now = time(NULL);

        Serial.print("MQTT connecting ... ");
        if (mqtt_client.connect(mqtt_client_id, mqtt_username, sas_token))
        {
            Serial.println("connected.");
        }
        else
        {
            Serial.print("failed, status code =");
            Serial.print(mqtt_client.state());
            Serial.println(". Trying again in 5 seconds.");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }

    mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC);

    return 0;
}

void AzureManager::establishConnection()
{
    Serial.println("establishConnection()");
    initializeClients();
    connectToAzureIoTHub();
}
