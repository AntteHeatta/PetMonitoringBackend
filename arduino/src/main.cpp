#include <Arduino.h>
#include "sensor_manager/sensor_manager.h"
#include "wifi/wifi_manager.h"
#include "azure/azure_manager.h"
#include "credentials.h"

SensorManager sensorManager;
WiFiManager wifiManager(ssid, pass);
AzureManager azureManager(mqttServer, mqttPort, deviceId, sasToken);

void setup()
{
  Serial.print("Starting the Pet Monitoring application backend");
  Serial.begin(9600);
  wifiManager.initialize();
  wifiManager.connect();
  // azureManager.initialize();
  // sensorManager.initialize();
  delay(2000);
}

void loop()
{
  // sensorManager.readSensors();

  // float humidity = sensorManager.getHumidity();
  // float temperature = sensorManager.getTemperature();
  // float luminosity = sensorManager.getLuminosity();
  // Serial.print("Humidity: ");
  // Serial.println(humidity);
  // Serial.print("Temperature: ");
  // Serial.println(temperature);
  // Serial.print("Luminosity: ");
  // Serial.println(luminosity);

  if (!wifiManager.isConnected())
  {
    Serial.println("WiFi connection lost, reconnecting.");
    wifiManager.connect();
  }

  // azureManager.sendToAzure(temperature, humidity, luminosity);

  delay(5000);
}