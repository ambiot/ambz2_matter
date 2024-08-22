#pragma once

#include <platform_stdlib.h>
#include <gpio_api.h>
#include <pwmout_api.h>
#include <app/util/attribute-table.h>

using namespace ::chip;
using namespace ::chip::app;

class MatterTemperatureSensor
{
public:
    void SetEp(EndpointId ep);
    EndpointId GetEp(void);

    void Init(PinName pin);
    void deInit(void);
    gpio_t getDevice(void);

    int16_t readTemperature(void);

    // Attribute (0x0000) MeasuredTemperature
    void setMeasuredTemperature(int16_t temp);
    int16_t getMeasuredTemperature(void);

    // Attribute (0x0001) MinMeasuredTemperature
    void setMinMeasuredTemperature(int16_t temp);
    int16_t getMinMeasuredTemperature(void);

    // Attribute (0x0002) MaxMeasuredTemperature
    void setMaxMeasuredTemperature(int16_t temp);
    int16_t getMaxMeasuredTemperature(void);

private:
    gpio_t gpio_device;
    EndpointId mEp;
    int16_t measuredTemperature;
    int16_t minMeasuredTemperature;
    int16_t maxMeasuredTemperature;
};
