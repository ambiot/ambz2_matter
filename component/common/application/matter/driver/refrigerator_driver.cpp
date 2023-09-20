#include <refrigerator_driver.h>
#include <support/logging/CHIPLogging.h>

void MatterRefrigerator::Init(PinName gpioLight)
{
    gpio_init(&innerLight, gpioLight);
    gpio_dir(&innerLight, PIN_OUTPUT);        // Direction: Output
    gpio_mode(&innerLight, PullNone);         // No pull

    doorStatus = 0;
    measuredTemperature = 0;
}

void MatterRefrigerator::deInit(void)
{
    
}

uint8_t MatterRefrigerator::GetDoorStatus(void)
{
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

void MatterRefrigerator::SetInnerLight(void)
{
    gpio_write(&innerLight, doorStatus);
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
