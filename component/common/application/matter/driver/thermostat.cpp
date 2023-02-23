#include "thermostat.h"
#include <support/logging/CHIPLogging.h>

void MatterThermostat::Init()
{
    // init thermostat driver code
}

void MatterThermostat::deInit()
{
    // deinit thermostat driver code
}

void MatterThermostat::Do()
{
    // implement thermostat action here
    ChipLogProgress(DeviceLayer, "Thermostat action");
}
