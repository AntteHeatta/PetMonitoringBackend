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
static char sas_token[200];
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
    // mqtt_client.setCallback(receivedCallback);
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

// SAS Token generation
String urlEncode(const String &str)
{
    String encoded = "";
    char c;
    for (unsigned int i = 0; i < str.length(); i++)
    {
        c = str.charAt(i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            encoded += c;
        }
        else
        {
            char buffer[4];
            sprintf(buffer, "%%%02X", c);
            encoded += buffer;
        }
    }
    return encoded;
}
void generateSasToken(char *sas_token, size_t token_size, const char *resourceUri, const char *deviceKey, int expirySeconds)
{
    // Step 1: Get the expiry time
    time_t now = time(NULL);
    uint32_t expiry = now + expirySeconds;

    Serial.print("SAS Token Expiry (Unix Time): ");
    Serial.println(expiry);

    // Step 2: Create the string to sign (resourceUri should NOT be URL-encoded here)
    String stringToSign = String(resourceUri) + "\n" + String(expiry);
    Serial.print("String to Sign: ");
    Serial.println(stringToSign);

    // Step 3: Decode device key from base64
    uint8_t decoded_device_key[64]; // Adjust size as per your key
    int decoded_key_length = base64_decode_chars(deviceKey, strlen(deviceKey), (char *)decoded_device_key);
    if (decoded_key_length <= 0)
    {
        Serial.println("Failed to decode device key");
        sas_token[0] = '\0'; // Return an empty string in case of failure
        return;
    }
    Serial.print("Decoded Device Key Length: ");
    Serial.println(decoded_key_length);

    // Step 4: Compute HMAC-SHA256
    unsigned char hmac_signature[32]; // SHA-256 output is 32 bytes
    br_hmac_key_context kc;
    br_hmac_key_init(&kc, &br_sha256_vtable, decoded_device_key, decoded_key_length);

    br_hmac_context hmac_ctx;
    br_hmac_init(&hmac_ctx, &kc, 32);
    br_hmac_update(&hmac_ctx, (const uint8_t *)stringToSign.c_str(), stringToSign.length());
    br_hmac_out(&hmac_ctx, hmac_signature);

    // Step 5: Base64 encode the signature
    String base64_signature = base64::encode(hmac_signature, sizeof(hmac_signature));
    Serial.print("Base64 Encoded Signature: ");
    Serial.println(base64_signature);

    // Step 6: URL encode the signature
    String url_encoded_signature = urlEncode(base64_signature);
    Serial.print("URL Encoded Signature: ");
    Serial.println(url_encoded_signature);

    // Step 7: URL encode the resource URI for the SAS token string
    String encoded_resourceUri = urlEncode(String(resourceUri));
    Serial.print("URL Encoded Resource URI: ");
    Serial.println(encoded_resourceUri);

    // Step 8: Construct the SAS token with URL-encoded sr
    String sas_token_str = "SharedAccessSignature sr=" + encoded_resourceUri + "&sig=" + url_encoded_signature + "&se=" + String(expiry);
    Serial.print("Constructed SAS Token String: ");
    Serial.println(sas_token_str);

    // Step 9: Copy the SAS token to the char array (C-style string)
    if (sas_token_str.length() >= token_size)
    {
        Serial.println("Token buffer too small");
        sas_token[0] = '\0'; // Clear the buffer
        return;
    }
    strncpy(sas_token, sas_token_str.c_str(), token_size - 1);
    sas_token[token_size - 1] = '\0'; // Ensure null termination

    Serial.print("Final SAS Token: ");
    Serial.println(sas_token);
    // // Step 1: Get the expiry time
    // time_t now = time(NULL);
    // uint32_t expiry = now + expirySeconds;
    // Serial.print("expiry: ");
    // Serial.println(expiry);
    // // Step 2: Create the string to sign
    // String stringToSign = String(resourceUri) + "\n" + String(expiry);

    // // Step 3: Decode device key from base64
    // uint8_t decoded_device_key[64]; // Adjust size as per your key
    // int decoded_key_length = base64_decode_chars(deviceKey, strlen(deviceKey), (char *)decoded_device_key);
    // if (decoded_key_length <= 0)
    // {
    //     Serial.println("Failed to decode device key");
    //     sas_token[0] = '\0'; // Return an empty string in case of failure
    //     return;
    // }

    // // Step 4: Compute HMAC-SHA256
    // unsigned char hmac_signature[32]; // SHA-256 output is 32 bytes
    // br_hmac_key_context kc;
    // br_hmac_key_init(&kc, &br_sha256_vtable, decoded_device_key, decoded_key_length);

    // br_hmac_context hmac_ctx;
    // br_hmac_init(&hmac_ctx, &kc, 32);
    // br_hmac_update(&hmac_ctx, (const uint8_t *)stringToSign.c_str(), stringToSign.length());
    // br_hmac_out(&hmac_ctx, hmac_signature);

    // // Step 5: Base64 encode the signature
    // String base64_signature = base64::encode(hmac_signature, sizeof(hmac_signature));

    // // Step 6: URL encode the signature
    // String url_encoded_signature = urlEncode(base64_signature);

    // // Step 7: Construct the SAS token
    // String sas_token_str = "SharedAccessSignature sr=" + String(resourceUri) + "&sig=" + url_encoded_signature + "&se=" + String(expiry);

    // // Step 8: Copy the SAS token to the char array (C-style string)
    // if (sas_token_str.length() >= token_size)
    // {
    //     Serial.println("Token buffer too small");
    //     sas_token[0] = '\0'; // Clear the buffer
    //     return;
    // }
    // strncpy(sas_token, sas_token_str.c_str(), token_size);
}

// From Azure SDK for C documentation examples
void initializeTime()
{
    // Serial.print("Setting time using SNTP");

    configTime(-5 * 3600, 0, NTP_SERVERS);
    time_t now = time(NULL);
    while (now < 1510592825)
    {
        delay(500);
        // Serial.print(".");
        now = time(NULL);
    }
    // Serial.println("done!");
}

char *getCurrentLocalTimeString()
{
    time_t now = time(NULL);
    return ctime(&now);
}

void printCurrentTime()
{
    // Serial.print("Current time: ");
    // Serial.print(getCurrentLocalTimeString());
}

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
    // Get the MQTT user name used to connect to IoT Hub
    if (az_result_failed(az_iot_hub_client_get_user_name(
            &iot_hub_client, mqtt_username, sizeofarray(mqtt_username), NULL)))
    {
        printf("Failed to get MQTT clientId, return code\n");
        return 1;
    }

    // Serial.print("Client ID: ");
    // Serial.println(mqtt_client_id);

    // Serial.print("Username: ");
    // Serial.println(mqtt_username);

    mqtt_client.setBufferSize(MQTT_PACKET_SIZE);
    generateSasToken(sas_token, sizeofarray(sas_token), resourceUri, deviceKey, ONE_HOUR_IN_SECS);
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
    initializeTime();
    printCurrentTime();
    initializeClients();
    connectToAzureIoTHub();
}
