#pragma once

#include <platform_stdlib.h>

class MatterTemperatureSensor
{
public:
    void Init();
    void deInit();
    void startPollingTask();
    int16_t getMeasuredTemperature();
    uint16_t getPollingFrequency();
    void setMeasuredTemperature(int16_t newTemp);
    void setPollingFrequency(uint16_t newPollingFrequency);

private:
    int16_t measuredTemperature;
    uint16_t pollingFrequency = 10;     // Poll every 10 seconds
};
