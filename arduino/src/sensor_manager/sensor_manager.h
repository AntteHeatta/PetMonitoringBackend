#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "humidity_temperature/humidity_temperature_sensor.h"
#include "luminosity/luminosity_sensor.h"
#include "pressure/pressure_sensor.h"

class SensorManager
{
public:
    SensorManager();
    void initialize();
    void readSensors();
    float getHumidity();
    float getTemperature();
    float getLuminosity();
    float getPressure();

private:
    HumidityTemperatureSensor humidityTemperatureSensor;
    LuminositySensor luminositySensor;
    PressureSensor pressureSensor;
};

#endif