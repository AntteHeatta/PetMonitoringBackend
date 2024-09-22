#include "humidity_sensor.h"

HumiditySensor::HumiditySensor(int pin) : dht(pin, DHT22), humidity(0) {}

void HumiditySensor::begin()
{
    dht.begin();
}

void HumiditySensor::read()
{
    humidity = dht.readHumidity();
}

float HumiditySensor::getHumidity()
{
    return humidity;
}