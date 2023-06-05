#pragma once

#include <platform_stdlib.h>
#include "pwmout_api.h"
#include <app/util/af.h>

class MatterFan
{
public:
    void Init(PinName pin);
    void deInit(void);
    void setFanMode(uint8_t mode);
    void setFanSpeedPercent(uint8_t speed);
    chip::app::Clusters::FanControl::FanModeType mapPercentToMode(uint8_t percent);
    uint8_t mapModeToPercent(uint8_t mode);

    pwmout_t *mPwm_obj = NULL;
    uint8_t mMode;
    uint8_t mPercent;
    uint8_t mSpeed;
    uint8_t mMaxSpeed = 10;
};
