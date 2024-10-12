#include "pressure_sensor.h"

PressureSensor::PressureSensor(uint8_t address) : pressure(0.0)
{
}

bool PressureSensor::begin()
{
    if (!bmp.begin(0x76))
    {
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        return false;
    }
    return true;
}

void PressureSensor::read()
{
    pressure = bmp.readPressure() / 100.0F; // Conversion to hPa

    if (isnan(pressure))
    {
        Serial.println("Failed to read from BMP280 sensor!");
    }
}

float PressureSensor::getPressure()
{
    return pressure;
}