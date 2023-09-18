#include <refrigerator_driver.h>
#include <support/logging/CHIPLogging.h>

void MatterRefrigerator::Init(PinName pin)
{
    mPwm_obj                        = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    pwmout_init(mPwm_obj, pin);
    doorStatus = 0;
    measuredTemperature = 0;
}

void MatterRefrigerator::deInit(void)
{
    vPortFree(mPwm_obj);
}

uint8_t MatterRefrigerator::GetDoorStatus(void)
{
    return doorStatus;
}

uint8_t MatterRefrigerator::GetTemperature(void)
{
    return measuredTemperature;
}

void MatterRefrigerator::SetDoorStatus(uint8_t status)
{
    doorStatus = status;
}

void MatterRefrigerator::SetTemperature(uint8_t temp)
{
    if ((temp >= 0) && (temp <= 4))
        measuredTemperature = temp;
    else
        ChipLogProgress(DeviceLayer, "Temperature must be set between 0 and 4");
}