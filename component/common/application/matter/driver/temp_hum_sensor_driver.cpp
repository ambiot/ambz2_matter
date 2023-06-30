#include "temp_hum_sensor_driver.h"
#include <support/logging/CHIPLogging.h>

void  MatterTemperatureHumiditySensor::Init()
{

}

void MatterTemperatureHumiditySensor::deInit()
{

}

void MatterTemperatureHumiditySensor::pollingTask()
{
    // read temperature
    // read humidity
    // sent downlink event for temperature and humidity attribute update
    vTaskDelay(pollingFrequency * 1000);
}

void MatterTemperatureHumiditySensor::startPollingTask()
{
    xTaskCreate();
}
