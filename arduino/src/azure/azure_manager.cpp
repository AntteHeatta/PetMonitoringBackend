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
    establishConnection();
}

void AzureManager::keepConnection()
{
    if (!mqtt_client.connected())
    {
        Serial.println("Reconnecting MQTT client");
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

    if (!mqtt_client.connected())
    {
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

// From Azure SDK for C documentation examples
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

static int generateSasToken(char *sas_token, size_t size)
{
    az_span signature_span = az_span_create((uint8_t *)signature, sizeofarray(signature));
    az_span out_signature_span;
    az_span encrypted_signature_span = az_span_create((uint8_t *)encrypted_signature, sizeofarray(encrypted_signature));

    uint32_t expiration = (uint32_t)time(NULL) + ONE_HOUR_IN_SECS;

    // Get signature
    if (az_result_failed(az_iot_hub_client_sas_get_signature(
            &iot_hub_client, expiration, signature_span, &out_signature_span)))
    {
        Serial.println("Failed getting SAS signature");
        return 1;
    }

    // Base64-decode device key
    int base64_decoded_device_key_length = base64_decode_chars(deviceKey, strlen(deviceKey), base64_decoded_device_key);

    if (base64_decoded_device_key_length == 0)
    {
        Serial.println("Failed base64 decoding device key");
        return 1;
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
        return 1;
    }

    return 0;
}

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

    mqtt_client.setBufferSize(MQTT_PACKET_SIZE);

    generateSasToken(sas_token, sizeofarray(sas_token));

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
            delay(5000);
        }
    }

    mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC);

    return 0;
}

void AzureManager::establishConnection()
{
    initializeClients();
    connectToAzureIoTHub();
}
