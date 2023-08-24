#include "matter_drivers.h"
#include "matter_interaction.h"
#include "washer_driver.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <clusters/laundry-washer-controls-server/laundry-washer-controls-server.h>
#include <opstate_driver.h>

using namespace ::chip::app;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::LaundryWasherControls;
using namespace chip::app::Clusters::ModeSelect;

#define PWM_PIN         PA_23

MatterWasher washer;

CHIP_ERROR matter_driver_laundry_washer_init()
{
    washer.Init(PWM_PIN);
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_laundry_washer_set_startup_value()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    return err;
}

void matter_driver_set_opstate_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_Opstate_State;
    downlink_event.value._u8 = (uint8_t) id; // 0: Stop; 1:Running ,2:Paused; 3: Error
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_spinspeed_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_LW_SpinSpeed;
    downlink_event.value._u8 = (uint8_t) id; // 0: Stop; 1:Running ,2:Paused; 3: Error
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_rinses_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_LW_NumberOfRinses;
    downlink_event.value._u8 = (uint8_t) id; // 0: Stop; 1:Running ,2:Paused; 3: Error
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_mode_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_LW_Mode;
    downlink_event.value._u8 = (uint8_t) id; // 0: Stop; 1:Running ,2:Paused; 3: Error
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_uplink_update_handler(AppEvent *aEvent)
{
    chip::app::ConcreteAttributePath path = aEvent->path;
    EmberAfStatus status;

    // this example only considers endpoint 1
    VerifyOrExit(aEvent->path.mEndpointId == 1,
                 ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", path.mEndpointId));

    switch(path.mClusterId)
    {
    case Clusters::LaundryWasherMode::Id:
        {
            ChipLogProgress(DeviceLayer, "LaundryWasherMode(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
        }
        break;
    case Clusters::OnOff::Id:
        {
            ChipLogProgress(DeviceLayer, "OnOff(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
        }
        break;
    case Clusters::LaundryWasherControls::Id:
        {
            ChipLogProgress(DeviceLayer, "LaundryWasherControl(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
        }
        break;
    case Clusters::TemperatureControl::Id:
        {
            ChipLogProgress(DeviceLayer, "TemperatureControl(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
        }
        break;
    case Clusters::OperationalState::Id:
        {
            ChipLogProgress(DeviceLayer, "OperationalState(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
        }
        break;
    default:
        break;
    }
exit:
    return;
}

void matter_driver_downlink_update_handler(AppEvent * event)
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    switch (event->Type)
    {
    case AppEvent::kEventType_Downlink_Opstate_State:
        {
            ChipLogProgress(DeviceLayer, "Set Operational State 0x%x", event->value._u8);
            GetOperationalStateInstance()->SetOperationalState(event->value._u8); 
        }
        break;
   case AppEvent::kEventType_Downlink_LW_SpinSpeed:
        {
            DataModel::Nullable<uint8_t> value;
            value.SetNonNull(event->value._u8);
            ChipLogProgress(DeviceLayer, "Set Spin Speed0x%x", event->value._u8);
            LaundryWasherControlsServer::Instance().SetSpinSpeedCurrent(1, value);
        }
        break;
   case AppEvent::kEventType_Downlink_LW_NumberOfRinses:
        {
            ChipLogProgress(DeviceLayer, "Set Number Of Rinses 0x%x", event->value._u8);
            LaundryWasherControlsServer::Instance().SetNumberOfRinses(1, (NumberOfRinsesEnum) event->value._u8);
        }
        break;
   case AppEvent::kEventType_Downlink_LW_Mode:
        {
            ChipLogProgress(DeviceLayer, "Change Mode to 0x%x", event->value._u8);
            ModeSelect::Attributes::CurrentMode::Set(1, event->value._u8);
        }
        break;
    default:
        break;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}
