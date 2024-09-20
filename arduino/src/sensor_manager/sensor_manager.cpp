#include "sensor_manager.h"

SensorManager::SensorManager() : humidityTemperatureSensor(13), luminositySensor(A0), pressureSensor(0x76) {}

void SensorManager::initialize()
{
    humidityTemperatureSensor.begin();
    pressureSensor.begin();
}

void SensorManager::readSensors()
{
    humidityTemperatureSensor.read();
    pressureSensor.read();
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

float SensorManager::getPressure()
{
    return pressureSensor.getPressure();
}