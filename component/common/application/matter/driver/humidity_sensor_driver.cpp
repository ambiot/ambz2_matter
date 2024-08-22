#include <FreeRTOS.h>
#include <task.h>
#include <humidity_sensor_driver.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::app;

void MatterHumiditySensor::SetEp(EndpointId ep)
{
    mEp = ep;
}

EndpointId MatterHumiditySensor::GetEp(void)
{
    return mEp;
}

void MatterHumiditySensor::Init(PinName pin)
{
    measuredValue = 0;
    minMeasuredValue = 0;
    maxMeasuredValue = 0;
}

void MatterHumiditySensor::deInit(void)
{
    // deinit humidity sensor
}

uint16_t MatterHumiditySensor::readHumidity(void)
{
    // replace with own humidity sensor reading
    return 80;
}

void MatterHumiditySensor::setMeasuredValue(uint16_t newHum)
{
    if ((newHum >= minMeasuredValue) && (newHum <= maxMeasuredValue))
    {
        measuredValue = newHum;
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Humidity must be set between %i and %i", minMeasuredValue, maxMeasuredValue);
    }
}

uint16_t MatterHumiditySensor::getMeasuredValue(void)
{
    return measuredValue;
}

void MatterHumiditySensor::setMinMeasuredValue(uint16_t newHum)
{
    minMeasuredValue = newHum;
}

uint16_t MatterHumiditySensor::getMinMeasuredValue(void)
{
    return minMeasuredValue;
}

void MatterHumiditySensor::setMaxMeasuredValue(uint16_t newHum)
{
    maxMeasuredValue = newHum;
}

uint16_t MatterHumiditySensor::getMaxMeasuredValue(void)
{
    return maxMeasuredValue;
}
