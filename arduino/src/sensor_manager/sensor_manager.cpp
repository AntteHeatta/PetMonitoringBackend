#include "sensor_manager.h"

SensorManager::SensorManager() : humiditySensor(2) {}

void SensorManager::initialize()
{
    humiditySensor.begin();
}

void SensorManager::readSensors()
{
    humiditySensor.read();
}

float SensorManager::getHumidity()
{
    return humiditySensor.getHumidity();
}