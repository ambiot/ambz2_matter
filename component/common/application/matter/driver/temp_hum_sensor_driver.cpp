#include <FreeRTOS.h>
#include <task.h>

#include <temp_hum_sensor_driver.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::app;

void MatterTemperatureHumiditySensor::SetTempSensorEp(EndpointId ep)
{
    mTempSensorEp = ep;
}

EndpointId MatterTemperatureHumiditySensor::GetTempSensorEp(void)
{
    return mTempSensorEp;
}

void MatterTemperatureHumiditySensor::Init(PinName pin)
{
    gpio_init(&gpio_device, pin);
    gpio_dir(&gpio_device, PIN_INPUT);
    gpio_mode(&gpio_device, PullUp);

    measuredTemperature = 0;
    minMeasuredTemperature = 0;
    maxMeasuredTemperature = 0;
}

void MatterTemperatureHumiditySensor::deInit(void)
{
    // deinit temperature sensor
}

gpio_t MatterTemperatureHumiditySensor::getDevice(void)
{
    return gpio_device;
}

int16_t MatterTemperatureHumiditySensor::readTemperature(void)
{
    // replace with own temperature sensor reading
    return 5;
}

void MatterTemperatureHumiditySensor::setMeasuredTemperature(int16_t temp)
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

int16_t MatterTemperatureHumiditySensor::getMeasuredTemperature(void)
{
    return measuredTemperature;
}

void MatterTemperatureHumiditySensor::setMinMeasuredTemperature(int16_t temp)
{
    minMeasuredTemperature = temp;
}

int16_t MatterTemperatureHumiditySensor::getMinMeasuredTemperature(void)
{
    return minMeasuredTemperature;
}

void MatterTemperatureHumiditySensor::setMaxMeasuredTemperature(int16_t temp)
{
    maxMeasuredTemperature = temp;
}

int16_t MatterTemperatureHumiditySensor::getMaxMeasuredTemperature(void)
{
    return maxMeasuredTemperature;
}

void MatterTemperatureHumiditySensor::SetHumSensorEp(EndpointId ep)
{
    mHumSensorEp = ep;
}

EndpointId MatterTemperatureHumiditySensor::GetHumSensorEp(void)
{
    return mHumSensorEp;
}

uint16_t MatterTemperatureHumiditySensor::readHumidity(void)
{
    // replace with own humidity sensor reading
    return 10;
}

void MatterTemperatureHumiditySensor::setMeasuredHumidity(uint16_t newHum)
{
    if ((newHum >= minMeasuredHumidity) && (newHum <= maxMeasuredHumidity))
    {
        measuredHumidity = newHum;
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Humidity must be set between %i and %i", minMeasuredHumidity, maxMeasuredHumidity);
    }
}

uint16_t MatterTemperatureHumiditySensor::getMeasuredHumidity(void)
{
    return measuredHumidity;
}

void MatterTemperatureHumiditySensor::setMinMeasuredHumidity(uint16_t newHum)
{
    minMeasuredHumidity = newHum;
}

uint16_t MatterTemperatureHumiditySensor::getMinMeasuredHumidity(void)
{
    return minMeasuredHumidity;
}

void MatterTemperatureHumiditySensor::setMaxMeasuredHumidity(uint16_t newHum)
{
    maxMeasuredHumidity = newHum;
}

uint16_t MatterTemperatureHumiditySensor::getMaxMeasuredHumidity(void)
{
    return maxMeasuredHumidity;
}
