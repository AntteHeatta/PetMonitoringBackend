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

void generateSasToken(char *sas_token, size_t size);

AzureManager::AzureManager(const char *host, int port, const char *deviceId, const char *deviceKey)
    : client(wifi_client), host(host), port(port), deviceId(deviceId), deviceKey(deviceKey)
{
}

void AzureManager::initialize()
{
    Serial.println("AzureManager::initialize()");
    establishConnection();
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

    if (!mqtt_client.connected())
    {
        establishConnection();
    }

    DynamicJsonDocument jsonDoc(200);
    jsonDoc["deviceId"] = deviceId;
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
    if (az_result_failed(az_iot_hub_client_get_user_name(
            &iot_hub_client, mqtt_username, sizeof(mqtt_username), NULL)))
    {
        Serial.println("Failed getting MQTT username");
        return 1;
    }

    mqtt_client.setBufferSize(MQTT_PACKET_SIZE);

    Serial.print("Client ID: ");
    Serial.println(mqtt_client_id);

    Serial.print("Username: ");
    Serial.println(mqtt_username);

    generateSasToken(sas_token, sizeofarray(sas_token));

    Serial.print("SAS Token: ");
    Serial.println(sas_token);

    while (!mqtt_client.connected())
    {
        Serial.print("MQTT connecting ... ");

        if (mqtt_client.connect(mqtt_client_id, mqtt_username, sas_token))
        {
            Serial.println("Connected.");
        }
        else
        {
            // Possible values for client.state()
            // #define MQTT_CONNECTION_TIMEOUT     -4
            // #define MQTT_CONNECTION_LOST        -3
            // #define MQTT_CONNECT_FAILED         -2
            // #define MQTT_DISCONNECTED           -1
            // #define MQTT_CONNECTED               0
            // #define MQTT_CONNECT_BAD_PROTOCOL    1
            // #define MQTT_CONNECT_BAD_CLIENT_ID   2
            // #define MQTT_CONNECT_UNAVAILABLE     3
            // #define MQTT_CONNECT_BAD_CREDENTIALS 4
            // #define MQTT_CONNECT_UNAUTHORIZED    5
            Serial.print("Failed, status code = ");
            Serial.println(mqtt_client.state());
            delay(5000); // Retry after 5 seconds
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

static void initializeTime()
{
    Serial.print("Setting time using SNTP");

    configTime(-5 * 3600, 0, NTP_SERVERS);
    time_t now = time(NULL);
    while (now < 1510592825)
    {
        delay(500);
        Serial.print(".");
        now = time(NULL);
    }
    Serial.println("done!");
}

static char *getCurrentLocalTimeString()
{
    time_t now = time(NULL);
    return ctime(&now);
}

static void printCurrentTime()
{
    Serial.print("Current time: ");
    Serial.print(getCurrentLocalTimeString());
}

void generateSasToken(char *sas_token, size_t size)
{
    initializeTime();
    az_span signature_span = az_span_create((uint8_t *)signature, sizeofarray(signature));
    az_span out_signature_span;
    az_span encrypted_signature_span = az_span_create((uint8_t *)encrypted_signature, sizeofarray(encrypted_signature));
    Serial.print("TIME now: ");
    Serial.print(getSecondsSinceEpoch());

    uint32_t expiration = getSecondsSinceEpoch() + ONE_HOUR_IN_SECS;

    // Get signature
    if (az_result_failed(az_iot_hub_client_sas_get_signature(
            &iot_hub_client, expiration, signature_span, &out_signature_span)))
    {
        Serial.println("Failed getting SAS signature");
    }

    // Base64-decode device key
    int base64_decoded_device_key_length = base64_decode_chars(deviceKey, strlen(deviceKey), base64_decoded_device_key);

    if (base64_decoded_device_key_length == 0)
    {
        Serial.println("Failed base64 decoding device key");
    }

    // SHA-256 encrypt
    br_hmac_key_context kc;
    br_hmac_key_init(
        &kc, &br_sha256_vtable, base64_decoded_device_key, base64_decoded_device_key_length);

    br_hmac_context hmac_ctx;
    br_hmac_init(&hmac_ctx, &kc, 32);
    br_hmac_update(&hmac_ctx, az_span_ptr(out_signature_span), az_span_size(out_signature_span));
    br_hmac_out(&hmac_ctx, encrypted_signature);

    // Base64 encode encrypted signature
    String b64enc_hmacsha256_signature = base64::encode(encrypted_signature, br_hmac_size(&hmac_ctx));

    az_span b64enc_hmacsha256_signature_span = az_span_create(
        (uint8_t *)b64enc_hmacsha256_signature.c_str(), b64enc_hmacsha256_signature.length());

    // URl-encode base64 encoded encrypted signature
    if (az_result_failed(az_iot_hub_client_sas_get_password(
            &iot_hub_client,
            expiration,
            b64enc_hmacsha256_signature_span,
            AZ_SPAN_EMPTY,
            sas_token,
            size,
            NULL)))
    {
        Serial.println("Failed getting SAS token");
    }
    Serial.println("Successfully generated SAS token");
}