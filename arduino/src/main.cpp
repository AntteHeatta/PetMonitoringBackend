#include <Arduino.h>
#include "sensor_manager/sensor_manager.h"

SensorManager sensorManager;

void setup()
{
  Serial.begin(9600);
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

  delay(5000);
}