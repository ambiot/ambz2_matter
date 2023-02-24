#include "thermostat_ui.h"
#include <support/logging/CHIPLogging.h>

void MatterThermostatUI::Init()
{
    // init UI driver code here
}

void MatterThermostatUI::deInit()
{
    // deinit UI driver code here
}

void MatterThermostatUI::UpdateDisplay()
{
    // implement driver code to display values to the UI
    // printfs here are for example
    
    ChipLogProgress(DeviceLayer, "Mode: %d | LocalTemperature: %d | OccupiedCoolingSetpoint: %d | OccupiedHeatingSetpoint: %d", mSystemMode, mLocalTemperature, mOccupiedCoolingSetpoint, mOccupiedHeatingSetpoint);
}

void MatterThermostatUI::SetLocalTemperature(uint16_t temp)
{
    mLocalTemperature = temp;
}

void MatterThermostatUI::SetOccupiedCoolingSetpoint(uint16_t temp)
{
    mOccupiedCoolingSetpoint = temp;
}

void MatterThermostatUI::SetOccupiedHeatingSetpoint(uint16_t temp)
{
    mOccupiedHeatingSetpoint = temp;
}

void MatterThermostatUI::SetSystemMode(uint8_t mode)
{
    mSystemMode = mode;
}

uint16_t MatterThermostatUI::GetLocalTemperature()
{
    return mLocalTemperature;
}

uint16_t MatterThermostatUI::GetOccupiedCoolingSetpoint()
{
    return mOccupiedCoolingSetpoint;
}

uint16_t MatterThermostatUI::GetOccupiedHeatingSetpoint()
{
    return mOccupiedHeatingSetpoint;
}

uint8_t MatterThermostatUI::GetSystemMode()
{
    return mSystemMode;
}
