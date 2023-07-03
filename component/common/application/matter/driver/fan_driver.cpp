#include <fan_driver.h>
#include <support/logging/CHIPLogging.h>


void MatterFan::Init(PinName pin)
{
    mPwm_obj                        = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    pwmout_init(mPwm_obj, pin);
}

void MatterFan::deInit(void)
{
    vPortFree(mPwm_obj);
}

void MatterFan::setFanMode(uint8_t mode)
{
    if (mMode == mode)
        return;

    mMode = mode;
}

void MatterFan::setFanSpeedPercent(uint8_t percent)
{
    if (mPercent == percent)
        return;

    ChipLogProgress(DeviceLayer, "Setting fan speed to %d\%", percent);
    mPercent = percent;
    float duty_cycle = (float) (percent) / 100;
    pwmout_write(mPwm_obj, duty_cycle);
}

chip::app::Clusters::FanControl::FanModeType MatterFan::mapPercentToMode(uint8_t percent)
{
    if (percent >= 80) // high
        return chip::app::Clusters::FanControl::FanModeType::kHigh;
    else if (percent >= 40) // medium
        return chip::app::Clusters::FanControl::FanModeType::kMedium;
    else if (percent >= 10) // low
        return chip::app::Clusters::FanControl::FanModeType::kLow;
    else // off
        return chip::app::Clusters::FanControl::FanModeType::kOff;
}

uint8_t MatterFan::mapModeToPercent(uint8_t mode)
{
    if (mode == 1)
        return 30;
    if (mode == 2)
        return 70;
    if (mode == 3)
        return 100;
    // off mode is handled in server code
}
