#pragma once

#include <platform_stdlib.h>
#include "pwmout_api.h"
// #include "gpio_api.h"
// #include "gpio_irq_api.h"
// #include "gpio_irq_ex_api.h"

class MatterRefrigerator
{
public:
    void Init(PinName pwmPin);
    void deInit(void);
    uint8_t GetDoorStatus(void);
    int8_t GetTemperature(void);
    int8_t GetMaxTemperature(void);
    int8_t GetMinTemperature(void);
    void SetDoorStatus(uint8_t status);
    void SetTemperature(int8_t temp);
    void SetTemperatureRange(int8_t minTemp, int8_t maxTemp);
    
private:
    // void gpio_level_irq_handler (uint32_t id, gpio_irq_event event);
    // void gpio_level_irq_handler_impl (uint32_t id, gpio_irq_event event);
    pwmout_t *mPwm_obj = NULL;
    // gpio_irq_t gpio_level;
    uint8_t doorStatus;
    // int8_t current_level;
    int8_t measuredTemperature;
    int8_t maxTemperature = 4;
    int8_t minTemperature = 0;
    uint8_t pollingFrequency = 10;     // Poll every 10 seconds
};
