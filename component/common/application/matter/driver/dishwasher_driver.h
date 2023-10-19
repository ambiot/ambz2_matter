#pragma once

#include <platform_stdlib.h>
#include "pwmout_api.h"

class MatterDishwasher
{
public:
    void Init(PinName pin);
    void deInit(void);
    uint16_t GetMode(void);
    void SetMode(uint16_t newMode);
    int16_t GetMaxTemperature();
    int16_t GetMinTemperature();
    int16_t GetTemperature();
    void SetTemperature(int16_t temp);
    void SetAlarm(void);

private:
    pwmout_t *mPwm_obj = NULL;
    uint16_t mode;
    int16_t localTemperature;
    int16_t maxTemperature = 60;
    int16_t minTemperature = 50;
};
