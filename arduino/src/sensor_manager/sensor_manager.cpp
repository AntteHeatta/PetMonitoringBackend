#include "sensor_manager.h"

SensorManager::SensorManager() : humidityTemperatureSensor(5), luminositySensor(A0) {}

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

float SensorManager::getLuminosity()
{
    return luminositySensor.getLuminosity();
}