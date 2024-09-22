#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "humidity/humidity_sensor.h"

class SensorManager
{
public:
    SensorManager();
    void initialize();
    void readSensors();
    float getHumidity();

private:
    HumiditySensor humiditySensor;
};

#endif