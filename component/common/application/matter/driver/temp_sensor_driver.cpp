#include <FreeRTOS.h>
#include <task.h>

#include <temp_sensor_driver.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::app;

void MatterTemperatureSensor::SetEp(EndpointId ep)
{
    mEp = ep;
}

EndpointId MatterTemperatureSensor::GetEp(void)
{
    return mEp;
}

void MatterTemperatureSensor::Init(PinName pin)
{
    gpio_init(&gpio_device, pin);
    gpio_dir(&gpio_device, PIN_INPUT);
    gpio_mode(&gpio_device, PullUp);

    measuredTemperature = 0;
    minMeasuredTemperature = 0;
    maxMeasuredTemperature = 0;
}

void MatterTemperatureSensor::deInit(void)
{
    // deinit temperature sensor
}

gpio_t MatterTemperatureSensor::getDevice(void)
{
    return gpio_device;
}

int16_t MatterTemperatureSensor::readTemperature(void)
{
    // replace with own temperature sensor reading
    return 37;
}

void MatterTemperatureSensor::setMeasuredTemperature(int16_t temp)
{
    if ((temp >= minMeasuredTemperature) && (temp <= maxMeasuredTemperature))
    {
        measuredTemperature = temp;
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Temperature must be set between %i and %i", minMeasuredTemperature, maxMeasuredTemperature);
    }
}

int16_t MatterTemperatureSensor::getMeasuredTemperature(void)
{
    return measuredTemperature;
}

void MatterTemperatureSensor::setMinMeasuredTemperature(int16_t temp)
{
    minMeasuredTemperature = temp;
}

int16_t MatterTemperatureSensor::getMinMeasuredTemperature(void)
{
    return minMeasuredTemperature;
}

void MatterTemperatureSensor::setMaxMeasuredTemperature(int16_t temp)
{
    maxMeasuredTemperature = temp;
}

int16_t MatterTemperatureSensor::getMaxMeasuredTemperature(void)
{
    return maxMeasuredTemperature;
}
