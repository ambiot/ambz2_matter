#include "matter_drivers.h"
#include "matter_interaction.h"
#include "thermostat.h"
#include "thermostat_ui.h"
#include "gpio_irq_api.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

using namespace ::chip::app;

MatterThermostatUI ui;
MatterThermostat thermostat;

CHIP_ERROR matter_driver_thermostat_init()
{
    thermostat.Init();
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_thermostat_ui_init()
{
    ui.Init();
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_thermostat_ui_set_startup_value()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EmberAfStatus getstatus;
    DataModel::Nullable<int16_t> temp;
    int16_t OccupiedCoolingSetpoint;
    int16_t OccupiedHeatingSetpoint;
    uint8_t SystemMode;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    getstatus = Clusters::Thermostat::Attributes::LocalTemperature::Get(1, temp);
    VerifyOrExit(getstatus == EMBER_ZCL_STATUS_SUCCESS, err = CHIP_ERROR_INTERNAL);
    getstatus = Clusters::Thermostat::Attributes::OccupiedCoolingSetpoint::Get(1, &OccupiedCoolingSetpoint);
    VerifyOrExit(getstatus == EMBER_ZCL_STATUS_SUCCESS, err = CHIP_ERROR_INTERNAL);
    getstatus = Clusters::Thermostat::Attributes::OccupiedHeatingSetpoint::Get(1, &OccupiedHeatingSetpoint);
    VerifyOrExit(getstatus == EMBER_ZCL_STATUS_SUCCESS, err = CHIP_ERROR_INTERNAL);
    getstatus = Clusters::Thermostat::Attributes::SystemMode::Get(1, &SystemMode);
    VerifyOrExit(getstatus == EMBER_ZCL_STATUS_SUCCESS, err = CHIP_ERROR_INTERNAL);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (temp.IsNull())
        ui.SetLocalTemperature(0);
    else
        ui.SetLocalTemperature(temp.Value());

    ui.SetOccupiedCoolingSetpoint(OccupiedCoolingSetpoint);
    ui.SetOccupiedHeatingSetpoint(OccupiedHeatingSetpoint);
    ui.SetSystemMode(SystemMode);

    ui.UpdateDisplay();

exit:
    return err;
}

void matter_driver_uplink_update_handler(AppEvent *aEvent)
{
    chip::app::ConcreteAttributePath path = aEvent->path;

    VerifyOrExit(aEvent->path.mEndpointId == 1,
                 ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", path.mEndpointId));

    switch(path.mClusterId)
    {
    case Clusters::Identify::Id:
        break;
    case Clusters::Thermostat::Id:
        if(path.mAttributeId == Clusters::Thermostat::Attributes::LocalTemperature::Id)
        {
            ui.SetLocalTemperature(aEvent->value._u16);
            thermostat.Do();
        }
        if(path.mAttributeId == Clusters::Thermostat::Attributes::OccupiedCoolingSetpoint::Id)
        {
            ui.SetOccupiedCoolingSetpoint(aEvent->value._u16);
            thermostat.Do();
        }
        if(path.mAttributeId == Clusters::Thermostat::Attributes::OccupiedHeatingSetpoint::Id)
        {
            ui.SetOccupiedHeatingSetpoint(aEvent->value._u16);
            thermostat.Do();
        }
        if(path.mAttributeId == Clusters::Thermostat::Attributes::SystemMode::Id)
        {
            ui.SetSystemMode(aEvent->value._u8);
            thermostat.Do();
        }
        ui.UpdateDisplay();
        break;
    case Clusters::Scenes::Id:
        break;
    case Clusters::RelativeHumidityMeasurement::Id:
        break;
    case Clusters::FanControl::Id:
        break;
    }

exit:
    return;
}
