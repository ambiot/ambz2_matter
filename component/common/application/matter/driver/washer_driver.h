#pragma once

#include <platform_stdlib.h>
#include "pwmout_api.h"

class MatterWasher
{
public:
    void Init(PinName pin);
    void deInit(void);
    void setSpinSpeedCurrent(uint8_t current);
    void setNumberOfRinses(uint8_t rinses);
    void setCurrentMode(uint8_t mode);
    void setOnMode(uint8_t mode);
    void setStartUpMode(uint8_t mode);
    void setMinMaxTemperature(uint8_t min, uint8_t max);
    void setOpState(uint8_t state);

private:
    pwmout_t *mPwm_obj = NULL;
    uint8_t mCurrent;
    uint8_t mRinses;
    uint8_t mMode;
    uint8_t mOnMode;
    uint8_t mStartUpMode;
    uint8_t mOpState;
    int16_t mMinTemp;
    int16_t mMaxTemp;
};
