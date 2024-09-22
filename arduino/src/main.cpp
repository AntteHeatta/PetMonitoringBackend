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
  Serial.print("Humidity: ");
  Serial.print(humidity);
  delay(2000);
}