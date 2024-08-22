#include <FreeRTOS.h>
#include <task.h>
#include <room_aircon_driver.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::app;

void MatterRoomAirCon::SetEp(EndpointId ep)
{
    mEp = ep;
}

EndpointId MatterRoomAirCon::GetEp(void)
{
    return mEp;
}

/* Fan Cluster */
void MatterRoomAirCon::FanControl::Init(PinName pin)
{
    mPwm_obj = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));

    pwmout_init(mPwm_obj, pin);
    pwmout_period_us(mPwm_obj, 20000);
    pwmout_start(mPwm_obj);

    mMode = 0;
    mPercent = 0;
}

void MatterRoomAirCon::FanControl::deInit(void)
{
    vPortFree(mPwm_obj);
}

void MatterRoomAirCon::FanControl::setFanMode(uint8_t mode)
{
    if (mMode == mode)
    {
        return;
    }

    mMode = mode;
}

void MatterRoomAirCon::FanControl::setFanSpeedPercent(uint8_t percent)
{
    if (mPercent == percent)
    {
        return;
    }

    ChipLogProgress(DeviceLayer, "Setting fan speed to %d\%", percent);
    mPercent = percent;
    float duty_cycle = (float) (percent) / 100;
    pwmout_write(mPwm_obj, duty_cycle);
}

chip::app::Clusters::FanControl::FanModeEnum MatterRoomAirCon::FanControl::mapPercentToMode(uint8_t percent)
{
    if (percent >= 80) // high
    {
        return chip::app::Clusters::FanControl::FanModeEnum::kHigh;
    }
    else if (percent >= 40) // medium
    {
        return chip::app::Clusters::FanControl::FanModeEnum::kMedium;
    }
    else if (percent >= 10) // low
    {
        return chip::app::Clusters::FanControl::FanModeEnum::kLow;
    }
    else // off
    {
        return chip::app::Clusters::FanControl::FanModeEnum::kOff;
    }
}

uint8_t MatterRoomAirCon::FanControl::mapModeToPercent(uint8_t mode)
{
    if (mode == 1)
    {
        return 30;
    }
    if (mode == 2)
    {
        return 70;
    }
    if (mode == 3)
    {
        return 100;
    }
}

/* Thermostat Cluster */
void MatterRoomAirCon::Thermostat::Init(PinName pin)
{
    mPwm_obj = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));

    pwmout_init(mPwm_obj, pin);
    pwmout_period_us(mPwm_obj, 20000);
    pwmout_start(mPwm_obj);
}

void MatterRoomAirCon::Thermostat::deInit(void)
{
    vPortFree(mPwm_obj);
}

void MatterRoomAirCon::Thermostat::SetLocalTemperature(uint16_t temp)
{
    mLocalTemperature = temp;
}

uint16_t MatterRoomAirCon::Thermostat::GetLocalTemperature(void)
{
    return mLocalTemperature;
}

void MatterRoomAirCon::Thermostat::SetOccupiedCoolingSetpoint(uint16_t temp)
{
    mOccupiedCoolingSetpoint = temp;
}

uint16_t MatterRoomAirCon::Thermostat::GetOccupiedCoolingSetpoint(void)
{
    return mOccupiedCoolingSetpoint;
}

void MatterRoomAirCon::Thermostat::SetOccupiedHeatingSetpoint(uint16_t temp)
{
    mOccupiedHeatingSetpoint = temp;
}

uint16_t MatterRoomAirCon::Thermostat::GetOccupiedHeatingSetpoint(void)
{
    return mOccupiedHeatingSetpoint;
}

void MatterRoomAirCon::Thermostat::SetSystemMode(uint8_t mode)
{
    mSystemMode = mode;
}

uint8_t MatterRoomAirCon::Thermostat::GetSystemMode()
{
    return mSystemMode;
}
