#include <refrigerator_driver.h>
#include <support/logging/CHIPLogging.h>

void MatterRefrigerator::Init(PinName outputGpio)
{
    gpio_init(&alarmGpio, outputGpio);
    gpio_dir(&alarmGpio, PIN_OUTPUT);        // Direction: Output
    gpio_mode(&alarmGpio, PullNone);         // No pull

    doorStatus = 0;
}

void MatterRefrigerator::deInit(void)
{
    return;
}

uint16_t MatterRefrigerator::GetMode(void)
{
    return mode;
}

void MatterRefrigerator::SetMode(uint16_t newMode)
{
    mode = newMode;
}

uint8_t MatterRefrigerator::GetDoorStatus(void)
{
    return doorStatus;
}

void MatterRefrigerator::SetDoorStatus(uint8_t status)
{
    if ((status != 0) && (status != 1))
    {
        ChipLogProgress(DeviceLayer, "Compatible refrigerator door status are only 0 (closed) and 1 (opened)\n");
    }
    else
    {
        doorStatus = status;
        if (doorStatus == 1)
        {
            ChipLogProgress(DeviceLayer, "Refrigerator door is opened\n");
        }
        else
        {
            ChipLogProgress(DeviceLayer, "Refrigerator door is closed\n");
        }
    } 
}

void MatterRefrigerator::SetAlarm(void)
{
    // depends on the customer implementation, in this examples it turns on/off the alarm gpio based on door status
    gpio_write(&alarmGpio, doorStatus);
}