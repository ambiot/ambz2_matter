#pragma once

#include <platform_stdlib.h>
#include <pwmout_api.h>
#include <app/util/attribute-table.h>

using namespace ::chip;
using namespace ::chip::app;

class MatterRoomAirCon
{
public:
    void SetEp(EndpointId ep);
    EndpointId GetEp(void);

    class FanControl {
    public:
        void Init(PinName pin);
        void deInit(void);

        void setFanMode(uint8_t mode);
        void setFanSpeedPercent(uint8_t percent);

        // Mapping Methods
        Clusters::FanControl::FanModeEnum mapPercentToMode(uint8_t percent);
        uint8_t mapModeToPercent(uint8_t mode);

    private:
        pwmout_t* mPwm_obj;
        uint8_t mMode;
        uint8_t mPercent;
    };

    class Thermostat {
    public:
        void Init(PinName pin);
        void deInit(void);

        void SetLocalTemperature(uint16_t temp);
        uint16_t GetLocalTemperature();

        void SetOccupiedCoolingSetpoint(uint16_t temp);
        uint16_t GetOccupiedCoolingSetpoint();

        void SetOccupiedHeatingSetpoint(uint16_t temp);
        uint16_t GetOccupiedHeatingSetpoint();

        void SetSystemMode(uint8_t mode);
        uint8_t GetSystemMode();

    private:
        pwmout_t* mPwm_obj;
        uint16_t mLocalTemperature;
        uint16_t mOccupiedCoolingSetpoint;
        uint16_t mOccupiedHeatingSetpoint;
        uint8_t mSystemMode;
    };

private:
    EndpointId mEp;
};
