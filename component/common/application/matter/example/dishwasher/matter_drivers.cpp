#include "matter_drivers.h"
#include "matter_interaction.h"
#include "opstate_driver.h"
#include "dishwasher_driver.h"
#include "dishwasher_mode.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <clusters/dishwasher-alarm-server/dishwasher-alarm-server.h>

using namespace ::chip::app;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OnOff;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::DishwasherMode;
using namespace chip::app::Clusters::DishwasherAlarm;
using namespace chip::app::Clusters::TemperatureControl;


#define PWM_PIN         PA_23

MatterDishwasher dishwasher;

CHIP_ERROR matter_driver_dishwasher_init()
{
    dishwasher.Init(PWM_PIN);
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_dishwasher_set_startup_value()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EmberAfStatus status;
    ModeBase::Commands::ChangeToModeResponse::Type modeChangedResponse;
    ModeBase::Instance & dishwasherInstance = DishwasherMode::Instance();
    DishwasherAlarmServer & dishwasherAlarmInstance = DishwasherAlarmServer::Instance();

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    status = Clusters::OnOff::Attributes::OnOff::Set(1, false);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "Failed to set OnOff!\n");
        err = CHIP_ERROR_INTERNAL;
    }

    status = Clusters::TemperatureControl::Attributes::MaxTemperature::Set(1, dishwasher.GetMaxTemperature());
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "Failed to set MaxTemperature!\n");
        err = CHIP_ERROR_INTERNAL;
    }

    status = Clusters::TemperatureControl::Attributes::MinTemperature::Set(1, dishwasher.GetMinTemperature());
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "Failed to set MinTemperature!\n");
        err = CHIP_ERROR_INTERNAL;
    }

    dishwasher.SetTemperature(55); // Set dishwasher temperature
    status = Clusters::TemperatureControl::Attributes::TemperatureSetpoint::Set(1, dishwasher.GetTemperature());
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "Failed to set TemperatureSetpoint!\n");
        err = CHIP_ERROR_INTERNAL;
    }

    modeChangedResponse.status = to_underlying(dishwasherInstance.UpdateCurrentMode(to_underlying(ModeTag::kNormal))); // Set dishwasher mode
    if (modeChangedResponse.status != to_underlying(ModeBase::StatusCode::kSuccess))
    {
        ChipLogProgress(DeviceLayer, "Failed to set Dishwasher Mode!\n");
        err = CHIP_ERROR_INTERNAL;
    }

    BitMask<AlarmMap> supported; // Set dishwasher alarm supported value
    supported.SetField(AlarmMap::kInflowError, 1);           // 0x01, 1
    supported.SetField(AlarmMap::kDrainError, 1);            // 0x02, 2
    supported.SetField(AlarmMap::kDoorError, 1);             // 0x04, 4
    supported.SetField(AlarmMap::kTempTooLow, 1);            // 0x08, 8
    supported.SetField(AlarmMap::kTempTooHigh, 1);           // 0x10, 16
    supported.SetField(AlarmMap::kWaterLevelError, 1);       // 0x20, 32 
    dishwasherAlarmInstance.SetSupportedValue(1, supported); // 0x3F, 63
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "Failed to set Dishwasher Alarm Supported Value!\n");
        err = CHIP_ERROR_INTERNAL;
    }

    BitMask<AlarmMap> mask; // Set dishwasher alarm mask value
    mask.SetField(AlarmMap::kInflowError, 1);      // 0x01, 1
    mask.SetField(AlarmMap::kDrainError, 1);       // 0x02, 2
    mask.SetField(AlarmMap::kDoorError, 1);        // 0x04, 4
    mask.SetField(AlarmMap::kTempTooLow, 1);       // 0x08, 8
    mask.SetField(AlarmMap::kTempTooHigh, 1);      // 0x10, 16
    mask.SetField(AlarmMap::kWaterLevelError, 1);  // 0x20, 32 
    dishwasherAlarmInstance.SetMaskValue(1, mask); // 0x3F, 63
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "Failed to set Dishwasher Alarm Mask Value!\n");
        err = CHIP_ERROR_INTERNAL;
    }

    BitMask<AlarmMap> latch; // Set dishwasher alarm latch value
    // SetLatchValue will call DishwasherAlarmServer::HasResetFeature(EndpointId endpoint)
    // This means that the device needs to support reset feature to be able to set the latch value.
    // Hence FeatureMap value of DishwasherAlarm Cluster must be set to 1 in Zap file.
    latch.SetField(AlarmMap::kInflowError, 1);       // 0x01, 1
    latch.SetField(AlarmMap::kDoorError, 1);         // 0x04, 4
    latch.SetField(AlarmMap::kTempTooLow, 1);        // 0x08, 8
    dishwasherAlarmInstance.SetLatchValue(1, latch); // 0x0D, 13
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "Failed to set Dishwasher Alarm Latch Value!\n");
        err = CHIP_ERROR_INTERNAL;
    }

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    return err;
}

void matter_driver_set_onoff_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_OnOff;
    downlink_event.value._u8 = (uint8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_opstate_state_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_Opstate_State;
    downlink_event.value._u8 = (uint8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_opstate_error_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_Opstate_Error_State;
    downlink_event.value._u8 = (uint8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_dishwasher_mode_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_DW_Mode;
    downlink_event.value._u8 = (uint8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_dishwasher_alarm_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_DW_Alarm_Set;
    downlink_event.value._u8 = (uint8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_temperature_callback(int32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_TempControl_SetPoint;
    downlink_event.value._i16 = (int16_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_uplink_update_handler(AppEvent *event)
{
    chip::app::ConcreteAttributePath path = event->path;
    EmberAfStatus status;

    // this example only considers endpoint 1
    VerifyOrExit(event->path.mEndpointId == 1,
                 ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", path.mEndpointId));

    switch(path.mClusterId)
    {
    case Clusters::OnOff::Id:
        {
            ChipLogProgress(DeviceLayer, "OnOff(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
        }
        break;
    case Clusters::OperationalState::Id:
        {
            ChipLogProgress(DeviceLayer, "OperationalState(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
        }
        break;    
    case Clusters::DishwasherMode::Id:
        {
            ChipLogProgress(DeviceLayer, "DishwasherMode(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
            if (path.mAttributeId == Clusters::DishwasherMode::Attributes::CurrentMode::Id)
                dishwasher.SetMode(event->value._u16); // Set mode when current mode is changed
        }
        break;
    case Clusters::DishwasherAlarm::Id:
        {
            ChipLogProgress(DeviceLayer, "DishwasherAlarm(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
            if (path.mAttributeId == Clusters::DishwasherAlarm::Attributes::State::Id)
                dishwasher.SetAlarm(); // Set alarm when alarm state is changed
        }
        break;
    case Clusters::TemperatureControl::Id:
        {
            ChipLogProgress(DeviceLayer, "TemperatureControl(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
            if (path.mAttributeId == Clusters::TemperatureControl::Attributes::TemperatureSetpoint::Id)
                dishwasher.SetTemperature(event->value._i16); // Change physical temperature    
        }
        break;
    default:
        break;
    }
exit:
    return;
}

void matter_driver_downlink_update_handler(AppEvent *event)
{
    EmberAfStatus status;
    CHIP_ERROR error;
    ModeBase::Commands::ChangeToModeResponse::Type modeChangedResponse;
    ModeBase::Instance & dishwasherInstance = DishwasherMode::Instance();
    DishwasherAlarmServer & dishwasherAlarmInstance = DishwasherAlarmServer::Instance();
    
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    switch (event->Type)
    {
    case AppEvent::kEventType_Downlink_OnOff:
        {
            ChipLogProgress(DeviceLayer, "Set OnOff 0x%x", event->value._u8);
            status = Clusters::OnOff::Attributes::OnOff::Set(1, (bool) event->value._u8);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogProgress(DeviceLayer, "Failed to set OnOff!\n");
            }
        }
        break;
    case AppEvent::kEventType_Downlink_Opstate_State:
        {
            ChipLogProgress(DeviceLayer, "Set Operational State 0x%x", event->value._u8);
            error = Clusters::OperationalState::GetOperationalStateInstance()->SetOperationalState(event->value._u8);
            if (error != CHIP_NO_ERROR)
            {
                ChipLogProgress(DeviceLayer, "Failed to set Operational State!\n");
            }
        }
        break;
    case AppEvent::kEventType_Downlink_Opstate_Error_State:
        {
            ChipLogProgress(DeviceLayer, "Set Operational State Error 0x%x", event->value._u8);
            Clusters::detail::Structs::ErrorStateStruct::Type errStateObj = {.errorStateID = event->value._u8};
            Clusters::OperationalState::GetOperationalStateInstance()->OnOperationalErrorDetected(errStateObj);
        }
        break;
    case AppEvent::kEventType_Downlink_DW_Alarm_Set:
        {
            ChipLogProgress(DeviceLayer, "Set Dishwasher Alarm State 0x%u", event->value._u8);
            status = dishwasherAlarmInstance.SetStateValue(1, event->value._u8, false); // We can input the value directly, no need to use BitMask<AlarmMap> and setfield one by one.
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogProgress(DeviceLayer, "Failed to set DishwasherAlarm state!\n");
            }
        }
        break;
    case AppEvent::kEventType_Downlink_DW_Mode:
        {
            ChipLogProgress(DeviceLayer, "Set Dishwasher Mode 0x%x", event->value._u8);
            modeChangedResponse.status = to_underlying(dishwasherInstance.UpdateCurrentMode(event->value._u8));
            if (modeChangedResponse.status != to_underlying(ModeBase::StatusCode::kSuccess))
            {
                ChipLogProgress(DeviceLayer, "Failed to set Dishwasher mode!\n");
            }
        }
        break;
    case AppEvent::kEventType_Downlink_TempControl_SetPoint:
        {
            if ((event->value._i16 >= dishwasher.GetMinTemperature()) && (event->value._i16 <= dishwasher.GetMaxTemperature()))
            {
                ChipLogProgress(DeviceLayer, "Set TemperatureSetpoint %i", event->value._i16);
                status = Clusters::TemperatureControl::Attributes::TemperatureSetpoint::Set(1, event->value._i16);
                if (status != EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogProgress(DeviceLayer, "Failed to set TemperatureSetpoint!\n");
                }
            }
            else
                ChipLogProgress(DeviceLayer, "Temperature must be set between %i and %i", dishwasher.GetMinTemperature(), dishwasher.GetMaxTemperature());
        }
        break;
    default:
        break;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}