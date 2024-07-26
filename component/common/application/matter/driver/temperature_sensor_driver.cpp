#include "temperature_sensor_driver.h"
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

void pollingTask(void *pvParameters)
{
    MatterTemperatureSensor *psensor = (MatterTemperatureSensor*) pvParameters;

    while(1)
    {
        //ChipLogProgress(DeviceLayer, "Poll cycle to read temperature...");
        // read temperature and set
        psensor->setMeasuredTemperature(readTemperature());

        // sent downlink event for temperature and humidity attribute update
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(1, psensor->getMeasuredTemperature());
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();

        // Delay till next poll
        vTaskDelay(psensor->getPollingFrequency() * 1000);
    }
}

void  MatterTemperatureSensor::Init()
{
    // init tempearture sensor
}

void MatterTemperatureSensor::deInit()
{
    // init humidity sensor
}

int16_t MatterTemperatureSensor::getMeasuredTemperature()
{
    return measuredTemperature;
}

uint16_t MatterTemperatureSensor::getPollingFrequency()
{
    return pollingFrequency;
}

void MatterTemperatureSensor::setMeasuredTemperature(int16_t newTemp)
{
    measuredTemperature = newTemp;
}

void MatterTemperatureSensor::setPollingFrequency(uint16_t newPollingFrequency)
{
    pollingFrequency = newPollingFrequency;
}

void MatterTemperatureSensor::startPollingTask()
{
    if (xTaskCreate(pollingTask, "temp_polling_task", 1024, this, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        ChipLogError(DeviceLayer, "failed to create temperature polling task");
    }
}
