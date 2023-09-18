#pragma once

#include <platform_stdlib.h>
#include "pwmout_api.h"

class MatterRefrigerator
{
public:
    void Init(PinName pin);
    void deInit(void);
    uint8_t GetDoorStatus(void);
    uint8_t GetTemperature(void);
    void SetDoorStatus(uint8_t status);
    void SetTemperature(uint8_t temp);

private:
    pwmout_t *mPwm_obj = NULL;
    uint8_t doorStatus;
    uint8_t measuredTemperature;
    uint8_t maxTemperature = 4;
    uint8_t minTemperature = 0;
    uint8_t pollingFrequency = 10;     // Poll every 10 seconds
};
