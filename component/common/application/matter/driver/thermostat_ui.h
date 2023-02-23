#pragma once

#include <platform_stdlib.h>

class MatterThermostatUI
{
public:
    void Init(void);
    void deInit(void);
    void UpdateDisplay(void);
    void SetLocalTemperature(uint16_t temp);
    void SetOccupiedCoolingSetpoint(uint16_t temp);
    void SetOccupiedHeatingSetpoint(uint16_t temp);
    void SetSystemMode(uint8_t mode);
    uint16_t GetLocalTemperature(void);
    uint16_t GetOccupiedCoolingSetpoint(void);
    uint16_t GetOccupiedHeatingSetpoint(void);
    uint8_t GetSystemMode(void);

private:
    uint16_t mLocalTemperature;
    uint16_t mOccupiedCoolingSetpoint;
    uint16_t mOccupiedHeatingSetpoint;
    uint8_t mSystemMode;
};
