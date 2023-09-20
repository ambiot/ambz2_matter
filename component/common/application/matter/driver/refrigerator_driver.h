#pragma once

#include <platform_stdlib.h>
#include "pwmout_api.h"
#include "gpio_api.h"

class MatterRefrigerator
{
public:
    void Init(PinName pwmPin, PinName gpioLight);
    void deInit(void);
    uint8_t GetDoorStatus(void);
    int8_t GetTemperature(void);
    int8_t GetMaxTemperature(void);
    int8_t GetMinTemperature(void);
    void SetDoorStatus(uint8_t temp);
    void SetInnerLight(void);
    void SetTemperature(int8_t temp);
    void SetTemperatureRange(int8_t minTemp, int8_t maxTemp);
    
private:
    pwmout_t *mPwm_obj = NULL;
    gpio_t innerLight;
    uint8_t doorStatus;
    int8_t measuredTemperature;
    int8_t maxTemperature = 4;
    int8_t minTemperature = 0;
    uint8_t pollingFrequency = 10;     // Poll every 10 seconds
};
