#include "humidity_temperature_sensor.h"

HumidityTemperatureSensor::HumidityTemperatureSensor(int pin) : dht(pin, DHT22), humidity(0), temperature(0) {}

void HumidityTemperatureSensor::begin()
{
    dht.begin();
}

void HumidityTemperatureSensor::read()
{
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
}

float HumidityTemperatureSensor::getHumidity()
{
    return humidity;
}

float HumidityTemperatureSensor::getTemperature()
{
    return temperature;
}