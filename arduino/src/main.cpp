#include <Arduino.h>
#include <Adafruit_I2CDevice.h>
#include <SPI.h>
#include "sensor_manager/sensor_manager.h"
#include "wifi/wifi_manager.h"
#include "azure/azure_manager.h"
#include "credentials.h"

SensorManager sensorManager;
WiFiManager wifiManager(ssid, pass);
AzureManager azureManager(host, port, deviceId, deviceKey);

void setup()
{
  Serial.begin(9600);
  wifiManager.initialize();
  wifiManager.connect();
  sensorManager.initialize();
  azureManager.initialize();
  delay(2000);
}

void loop()
{
  azureManager.establishConnection();
  sensorManager.readSensors();

  float humidity = sensorManager.getHumidity();
  float temperature = sensorManager.getTemperature();
  float luminosity = sensorManager.getLuminosity();
  float pressure = sensorManager.getPressure();
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  Serial.println();
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");
  Serial.println();
  Serial.print("Luminosity: ");
  Serial.println(luminosity);
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println("hPa");
  Serial.println();

  if (!wifiManager.isConnected())
  {
    Serial.println("WiFi connection lost, reconnecting.");
    wifiManager.connect();
  }
  azureManager.sendToAzure(temperature, humidity, pressure, luminosity);

  delay(3000);
}