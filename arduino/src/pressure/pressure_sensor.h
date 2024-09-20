#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include <Adafruit_BMP280.h>
#include <Wire.h>

class PressureSensor
{
public:
    PressureSensor(uint8_t address = 0x76);
    bool begin();
    void read();
    float getPressure();

private:
    Adafruit_BMP280 bmp;
    float pressure;
};

#endif