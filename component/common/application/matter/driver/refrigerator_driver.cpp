#include <refrigerator_driver.h>
#include <support/logging/CHIPLogging.h>

void MatterRefrigerator::Init(PinName pin)
{
    mPwm_obj                        = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    pwmout_init(mPwm_obj, pin);
}

void MatterRefrigerator::deInit(void)
{
    vPortFree(mPwm_obj);
}

void MatterRefrigerator::Do(void)
{

}
