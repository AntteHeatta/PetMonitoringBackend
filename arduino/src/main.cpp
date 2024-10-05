#include <Arduino.h>
#include "sensor_manager/sensor_manager.h"
#include "wifi/wifi_manager.h"
#include "wifi_credentials.h"

SensorManager sensorManager;
WiFiManager wifiManager(2, 3, ssid, pass);

void setup()
{
  Serial.begin(9600);
  wifiManager.initialize();
  wifiManager.connect();

  sensorManager.initialize();
  delay(2000);
}

void loop()
{
  sensorManager.readSensors();

  float humidity = sensorManager.getHumidity();
  float temperature = sensorManager.getTemperature();
  float luminosity = sensorManager.getLuminosity();
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Luminosity: ");
  Serial.println(luminosity);

  if (!wifiManager.isConnected())
  {
    Serial.println("WiFi connection lost, reconnecting.");
    wifiManager.connect();
  }

  delay(5000);
}