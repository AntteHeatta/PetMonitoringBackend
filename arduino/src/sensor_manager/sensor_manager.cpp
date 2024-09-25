#include "sensor_manager.h"

SensorManager::SensorManager() : humidityTemperatureSensor(0) {}

void SensorManager::initialize()
{
    humidityTemperatureSensor.begin();
}

void SensorManager::readSensors()
{
    humidityTemperatureSensor.read();
}

float SensorManager::getHumidity()
{
    return humidityTemperatureSensor.getHumidity();
}

float SensorManager::getTemperature()
{
    return humidityTemperatureSensor.getTemperature();
}