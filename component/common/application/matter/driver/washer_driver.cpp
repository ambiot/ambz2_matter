#include <washer_driver.h>
#include <support/logging/CHIPLogging.h>

void MatterWasher::Init(PinName pin)
{
    mPwm_obj                        = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    pwmout_init(mPwm_obj, pin);
}

void MatterWasher::deInit(void)
{
    vPortFree(mPwm_obj);
}

void MatterWasher::setSpinSpeedCurrent(uint8_t current)
{
    if (mCurrent == current)
        return;

    mCurrent = current;
}

void MatterWasher::setNumberOfRinses(uint8_t rinses)
{
    if (mRinses == rinses)
        return;

    mRinses = rinses;
}

void MatterWasher::setCurrentMode(uint8_t mode)
{
    if (mMode == mode)
        return;

    mMode = mode;
}

void MatterWasher::setOnMode(uint8_t mode)
{
    if (mOnMode == mode)
        return;

    mOnMode = mode;
}

void MatterWasher::setStartUpMode(uint8_t mode)
{
    if (mStartUpMode == mode)
        return;

    mStartUpMode = mode;
}

void MatterWasher::setMinMaxTemperature(uint8_t min, uint8_t max)
{
    mMinTemp = min;
    mMaxTemp = max;
}

void MatterWasher::setOpState(uint8_t state)
{
    if (mOpState == state)
        return;

    mOpState = state;
}
