#pragma once

#include <platform_stdlib.h>
#include "gpio_api.h"
#include "gpio_irq_api.h"

class MatterRefrigerator
{
public:
    void Init(PinName gpioLight);
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
    gpio_t innerLight;
    uint8_t doorStatus;
    int8_t measuredTemperature;
    int8_t maxTemperature = 4;
    int8_t minTemperature = 0;
    uint8_t pollingFrequency = 10;     // Poll every 10 seconds
};
