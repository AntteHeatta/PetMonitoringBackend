#include "luminosity_sensor.h"

LuminositySensor::LuminositySensor(int pin) : pin(), luminosity() {}

void LuminositySensor::begin()
{
    // No initialization needed for LDR
}

void LuminositySensor::read()
{
    luminosity = analogRead(pin);
}

float LuminositySensor::getLuminosity()
{
    return luminosity = analogRead(pin);
}
