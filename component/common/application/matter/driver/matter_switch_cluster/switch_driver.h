#pragma once

#include <platform_stdlib.h>
#include <pwmout_api.h>

class MatterSwitch
{
public:
    void Init(PinName pin);
    void deInit(void);

    // NumberOfPosition Attribute
    uint8_t GetNumberOfPosition(void);
    void SetNumberOfPosition(uint8_t num);

    // CurrentPosition Attribute
    uint8_t GetCurrentPosition(void);
    uint8_t GetPreviousPosition(void);
    void SetCurrentPosition(uint8_t position);

    // MultiPressMax Attribute
    uint8_t GetMultiPressMax(void);
    void SetMultiPressMax(uint8_t value);

private:
    pwmout_t *mPwm_obj = NULL;
    uint8_t mNumberOfPosition;
    uint8_t mCurrentPosition;
    uint8_t mPreviousPosition;
    uint8_t mMultiPressMaxCount;
};
