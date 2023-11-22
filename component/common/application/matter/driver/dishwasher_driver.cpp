#include <dishwasher_driver.h>
#include <support/logging/CHIPLogging.h>

void MatterDishwasher::Init(PinName pin)
{
    mPwm_obj                        = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    pwmout_init(mPwm_obj, pin);
}

void MatterDishwasher::deInit(void)
{
    vPortFree(mPwm_obj);
}

uint16_t MatterDishwasher::GetMode(void)
{
    return mode;
}

void MatterDishwasher::SetMode(uint16_t newMode)
{
    mode = newMode;
}

int16_t MatterDishwasher::GetMaxTemperature()
{
    return maxTemperature;
}

int16_t MatterDishwasher::GetMinTemperature()
{
    return minTemperature;
}

int16_t MatterDishwasher::GetTemperature()
{
    return localTemperature;
}

void MatterDishwasher::SetTemperature(int16_t temp)
{
    if ((temp >= minTemperature) && (temp <= maxTemperature))
        localTemperature = temp;
    else
        ChipLogProgress(DeviceLayer, "Temperature must be set between %i and %i", minTemperature, maxTemperature);
}

void MatterDishwasher::SetAlarm(void)
{
    // depends on customer implementation
}