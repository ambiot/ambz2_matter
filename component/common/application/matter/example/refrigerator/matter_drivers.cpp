#include "matter_drivers.h"
#include "matter_interaction.h"
#include "refrigerator_driver.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <clusters/refrigerator-alarm-server/refrigerator-alarm-server.h>

using namespace ::chip::app;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RefrigeratorAlarm;
using namespace chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode;

#define PWM_PIN         PA_23

MatterRefrigerator refrigerator;

CHIP_ERROR matter_driver_refrigerator_init()
{
    refrigerator.Init(PWM_PIN);
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_refrigerator_set_startup_value()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    return err;
}

void matter_driver_set_mask_value_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_Refrigerator_Alarm_SetMaskValue;
    downlink_event.value._u8 = (uint8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_state_value_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_Refrigerator_Alarm_SetStateValue;
    downlink_event.value._u8 = (uint8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_supported_value_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_Refrigerator_Alarm_SetSupportedValue;
    downlink_event.value._u8 = (uint8_t) id;
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
    case Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Id:
        {
            ChipLogProgress(DeviceLayer, "RefrigeratorAndTemperatureControlledCabinetMode(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
        }
        break;
    case Clusters::RefrigeratorAlarm::Id:
        {
            ChipLogProgress(DeviceLayer, "RefrigeratorAlarm(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
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
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    switch (event->Type)
    {
    case AppEvent::kEventType_Downlink_Refrigerator_Alarm_SetMaskValue:
        {
            BitMask<AlarmMap> value;
            value.SetField(AlarmMap::kDoorOpen, event->value._u8);
            ChipLogProgress(DeviceLayer, "Set Mask Value 0x%x", event->value._u8);
            RefrigeratorAlarmServer::Instance().SetMaskValue(1, value);
        }
        break;
   case AppEvent::kEventType_Downlink_Refrigerator_Alarm_SetStateValue:
        {
            BitMask<AlarmMap> value;
            value.SetField(AlarmMap::kDoorOpen, event->value._u8);
            ChipLogProgress(DeviceLayer, "Set State Value 0x%x", event->value._u8);
            RefrigeratorAlarmServer::Instance().SetStateValue(1, value);
        }
        break;
   case AppEvent::kEventType_Downlink_Refrigerator_Alarm_SetSupportedValue:
        {
            BitMask<AlarmMap> value;
            value.SetField(AlarmMap::kDoorOpen, event->value._u8);
            ChipLogProgress(DeviceLayer, "Set Supported Value 0x%x", event->value._u8);
            RefrigeratorAlarmServer::Instance().SetSupportedValue(1, value);
        }
        break;
    default:
        break;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}
