#include "humidity_temperature_sensor.h"

HumidityTemperatureSensor::HumidityTemperatureSensor(int pin) : dht(pin, DHT22), humidity(2), temperature(2) {}

void HumidityTemperatureSensor::begin()
{
    dht.begin();
}

void HumidityTemperatureSensor::read()
{
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    if (isnan(humidity) || isnan(temperature))
    {
        Serial.println("Failed to read from DHT sensor!");
    }
}

float HumidityTemperatureSensor::getHumidity()
{
    return humidity;
}

float HumidityTemperatureSensor::getTemperature()
{
    return temperature;
}