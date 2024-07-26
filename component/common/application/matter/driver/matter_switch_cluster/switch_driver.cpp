#include <matter_switch_cluster/switch_driver.h>
#include <support/logging/CHIPLogging.h>
#include <algorithm>

void MatterSwitch::Init(PinName pin)
{
    mPwm_obj = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    pwmout_init(mPwm_obj, pin);
}

void MatterSwitch::deInit(void)
{
    vPortFree(mPwm_obj);
}

uint8_t MatterSwitch::GetNumberOfPosition(void)
{
    return this->mNumberOfPosition;
}

void MatterSwitch::SetNumberOfPosition(uint8_t num)
{
    if (mNumberOfPosition == num)
    {
        return;
    }

    mNumberOfPosition = num;

    ChipLogProgress(DeviceLayer, "Number of Position is %d\%", mNumberOfPosition);
}

uint8_t MatterSwitch::GetCurrentPosition(void)
{
    return this->mCurrentPosition;
}

uint8_t MatterSwitch::GetPreviousPosition(void)
{
    return this->mPreviousPosition;
}

void MatterSwitch::SetCurrentPosition(uint8_t position)
{
    if (mCurrentPosition == position)
    {
        return;
    }

    // Update previousPosition before changing currentPosition
    mPreviousPosition = mCurrentPosition;

    // Set the new currentPosition
    mCurrentPosition = position;

    ChipLogProgress(DeviceLayer, "Change position from %d to %d\%", mPreviousPosition, mCurrentPosition);
}

uint8_t MatterSwitch::GetMultiPressMax(void)
{
    return this->mMultiPressMaxCount;
}

void MatterSwitch::SetMultiPressMax(uint8_t value)
{
    if (mMultiPressMaxCount == value)
    {
        return;
    }

    mMultiPressMaxCount = value;

    ChipLogProgress(DeviceLayer, "Multi-Press Maximum Count is %d\%", mMultiPressMaxCount);
}
