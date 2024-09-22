#ifndef HUMIDITY_SENSOR_H
#define HUMIDITY_SENSOR_H

#include <DHT.h>

class HumiditySensor
{
public:
    HumiditySensor(int pin = 2);
    void begin();
    void read();
    float getHumidity();

private:
    DHT dht;
    float humidity;
};

#endif