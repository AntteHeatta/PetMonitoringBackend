#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "humidity_temperature/humidity_temperature_sensor.h"

class SensorManager
{
public:
    SensorManager();
    void initialize();
    void readSensors();
    float getHumidity();
    float getTemperature();

private:
    HumidityTemperatureSensor humidityTemperatureSensor;
};

#endif