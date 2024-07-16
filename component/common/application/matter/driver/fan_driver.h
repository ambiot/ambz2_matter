#pragma once

#include <platform_stdlib.h>
#include "pwmout_api.h"
#include <app/util/attribute-table.h>

class MatterFan
{
public:
    void Init(PinName pin);
    void deInit(void);
    void setFanMode(uint8_t mode);
    uint8_t getPrevFanSpeedPercent(void);
    void setFanSpeedPercent(uint8_t speed);
    chip::app::Clusters::FanControl::FanModeEnum mapPercentToMode(uint8_t percent);
    uint8_t mapModeToPercent(uint8_t mode);

private:
    pwmout_t *mPwm_obj = NULL;
    uint8_t mMode;
    uint8_t mPercent;
    uint8_t mPrevPercent;
    uint8_t mSpeed;
    uint8_t mMaxSpeed = 10;
};
