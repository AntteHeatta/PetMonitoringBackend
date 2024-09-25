#include <Arduino.h>
#include "sensor_manager/sensor_manager.h"

SensorManager sensorManager;

void setup()
{
  Serial.begin(9600);
  sensorManager.initialize();
}

void loop()
{
  sensorManager.readSensors();

  float humidity = sensorManager.getHumidity();
  float temperature = sensorManager.getTemperature();
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Temperature: ");
  Serial.println(temperature);

  delay(4000);
}