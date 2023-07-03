#include "temp_hum_sensor_driver.h"
#include <FreeRTOS.h>
#include "task.h"
#include <platform/CHIPDeviceLayer.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <support/logging/CHIPLogging.h>

using namespace ::chip::app;

int32_t readTemperature()
{
    // replace with own temperature sensor reading
    return 37;
}

uint16_t readHumidity()
{
    // replace with own humidity sensor reading
    return 80;
}

void pollingTask(void *pvParameters)
{
    MatterTemperatureHumiditySensor *psensor = (MatterTemperatureHumiditySensor*) pvParameters;

    while(1)
    {
        // read temperature and set
        psensor->setMeasuredTemperature(readTemperature());

        // read humidity and set
        psensor->setMeasuredHumidity(readHumidity());

        // sent downlink event for temperature and humidity attribute update
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(1, psensor->getMeasuredTemperature());
        Clusters::RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(1, psensor->getMeasuredHumidity());
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();

        // Delay till next poll
        vTaskDelay(psensor->getPollingFrequency() * 1000);
    }
}

void  MatterTemperatureHumiditySensor::Init()
{
    // init tempearture sensor
}

void MatterTemperatureHumiditySensor::deInit()
{
    // init humidity sensor
}

int16_t MatterTemperatureHumiditySensor::getMeasuredTemperature()
{
    return measuredTemperature;
}

uint16_t MatterTemperatureHumiditySensor::getMeasuredHumidity()
{
    return measuredHumidity;
}

uint16_t MatterTemperatureHumiditySensor::getPollingFrequency()
{
    return pollingFrequency;
}

void MatterTemperatureHumiditySensor::setMeasuredTemperature(int16_t newTemp)
{
    measuredTemperature = newTemp;
}

void MatterTemperatureHumiditySensor::setMeasuredHumidity(uint16_t newHum)
{
    measuredHumidity = newHum;
}

void MatterTemperatureHumiditySensor::setPollingFrequency(uint16_t newPollingFrequency)
{
    pollingFrequency = newPollingFrequency;
}

void MatterTemperatureHumiditySensor::startPollingTask()
{
    if (xTaskCreate(pollingTask, "temphum_polling_task", 1024, this, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        ChipLogError(DeviceLayer, "failed to create temperature & humidity polling task");
    }
}
