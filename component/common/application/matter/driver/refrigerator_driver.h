#pragma once

#include <platform_stdlib.h>
#include "gpio_api.h"
#include "gpio_irq_api.h"

class MatterRefrigerator
{
public:
    void Init(PinName outputGpio);
    void deInit(void);
    uint16_t GetMode();
    void SetMode(uint16_t newMode);
    uint8_t GetDoorStatus(void);
    void SetDoorStatus(uint8_t temp);
    void SetAlarm(void);
    
private:
    gpio_t alarmGpio;
    uint16_t mode;
    uint8_t doorStatus;
};
