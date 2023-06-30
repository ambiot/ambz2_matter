#pragma once

#include <platform_stdlib.h>

class MatterTemperatureHumiditySensor
{
public:
    void Init();
    void deInit();
    void pollingTask();
    void startPollingTask();

private:
    uint32_t measuredTemperature;
    uint32_t measuredHumidity;
    uint16_t pollingFrequency = 60;     // Poll every minute
};
