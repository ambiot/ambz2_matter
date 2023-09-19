#include <refrigerator_driver.h>
#include <support/logging/CHIPLogging.h>

void MatterRefrigerator::Init(PinName pwmPin)
{
    mPwm_obj                        = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    pwmout_init(mPwm_obj, pwmPin);

    // gpio_irq_init(&gpio_level, gpioLevelPin, this->gpio_level_irq_handler, (uint32_t)(&current_level));
    // gpio_irq_set(&gpio_level, (gpio_irq_event)IRQ_LOW, 1);
    // gpio_irq_enable(&gpio_level);

    // gpio_init(&gpio_level, gpioLevelPin);

    doorStatus = 0;
    measuredTemperature = 0;
}

void MatterRefrigerator::deInit(void)
{
    vPortFree(mPwm_obj);
}

uint8_t MatterRefrigerator::GetDoorStatus(void)
{
    if (doorStatus == 0)
        ChipLogProgress(DeviceLayer, "Refrigerator door is closed\n")
    else if (doorStatus == 1)
        ChipLogProgress(DeviceLayer, "Refrigerator door is opened\n");
    return doorStatus;
}

int8_t MatterRefrigerator::GetTemperature(void)
{
    return measuredTemperature;
}

int8_t MatterRefrigerator::GetMaxTemperature(void)
{
    return maxTemperature;
}

int8_t MatterRefrigerator::GetMinTemperature(void)
{
    return minTemperature;
}

void MatterRefrigerator::SetDoorStatus(uint8_t status)
{
    doorStatus = status;
}

void MatterRefrigerator::SetTemperature(int8_t temp)
{
    if ((temp >= minTemperature) && (temp <= maxTemperature))
        measuredTemperature = temp;
    else
        ChipLogProgress(DeviceLayer, "Temperature must be set between %i and %i", minTemperature, maxTemperature);
}

void MatterRefrigerator::SetTemperatureRange(int8_t minTemp, int8_t maxTemp)
{
    minTemperature = minTemp;
    maxTemperature = maxTemp;
}

// void gpio_level_irq_handler(uint32_t id, gpio_irq_event event) 
// {
//     MatterRefrigerator* instance = reinterpret_cast<MatterRefrigerator*>(id);
//     instance->gpio_level_irq_handler_impl(event);
// }

// void gpio_level_irq_handler_impl(uint32_t id, gpio_irq_event event)
// {
//     uint32_t *level = (uint32_t *) id;

//     // Disable level irq because the irq will keep triggered when it keeps in same level.
//     gpio_irq_disable(&gpio_level);

//     // make some software de-bounce here if the signal source is not stable.

//     if (*level == IRQ_LOW) {
//         dbg_printf("low level event \r\n");
//         this->SetDoorStatus((uint8_t) 0);

//         // Change to listen to high level event
//         *level = IRQ_HIGH;
//         gpio_irq_set(&gpio_level, (gpio_irq_event)IRQ_HIGH, 1);
//         gpio_irq_enable(&gpio_level);
//     } else if (*level == IRQ_HIGH) {
//         dbg_printf("high level event \r\n");
//         this->SetDoorStatus((uint8_t) 1);

//         // Change to listen to low level event
//         *level = IRQ_LOW;
//         gpio_irq_set(&gpio_level, (gpio_irq_event)IRQ_LOW, 1);
//         gpio_irq_enable(&gpio_level);
//     }
// }