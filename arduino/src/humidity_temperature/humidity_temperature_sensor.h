#ifndef HUMIDITY_SENSOR_H
#define HUMIDITY_SENSOR_H

#include <DHT.h>

class HumidityTemperatureSensor
{
public:
    HumidityTemperatureSensor(int pin = 2);
    void begin();
    void read();
    float getHumidity();
    float getTemperature();

private:
    DHT dht;
    float humidity;
    float temperature;
};

#endif