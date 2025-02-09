#ifndef LUMINOSITY_SENSOR_H
#define LUMINOSITY_SENSOR_H

#include <Arduino.h>
#include <BH1750.h>

class LuminositySensor
{
public:
    LuminositySensor(int pin);
    void begin();
    void read();
    float getLuminosity();

private:
    int pin;
    float luminosity;
};

#endif