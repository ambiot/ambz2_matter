#include "matter_drivers.h"
#include "matter_interaction.h"
#include "bridge_driver.h"
#include <lwip/sockets.h>
#include "wifi_conf.h"

#include <app/ConcreteAttributePath.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

#include <lib/support/ZclString.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Platform;
using namespace ::chip::app::Clusters;

MatterBridgedDeviceOnOff ALight1("Light 1", "Office");

namespace {
void CallReportingCallback(intptr_t closure)
{
    auto path = reinterpret_cast<app::ConcreteAttributePath *>(closure);
    MatterReportingAttributeChangeCallback(*path);
    Platform::Delete(path);
}

void ScheduleReportingCallback(MatterBridgeDevice * dev, ClusterId cluster, AttributeId attribute)
{
    auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), cluster, attribute);
    PlatformMgr().ScheduleWork(CallReportingCallback, reinterpret_cast<intptr_t>(path));
}
} // anonymous namespace

void HandleDeviceStatusChanged(MatterBridgeDevice * dev, MatterBridgeDevice::Changed_t itemChangedMask)
{
    if (itemChangedMask & MatterBridgeDevice::kChanged_Reachable)
    {
        ScheduleReportingCallback(dev, BridgedDeviceBasicInformation::Id, BridgedDeviceBasicInformation::Attributes::Reachable::Id);
    }

    if (itemChangedMask & MatterBridgeDevice::kChanged_Name)
    {
        ScheduleReportingCallback(dev, BridgedDeviceBasicInformation::Id, BridgedDeviceBasicInformation::Attributes::NodeLabel::Id);
    }
}

void HandleDeviceOnOffStatusChanged(MatterBridgedDeviceOnOff * dev, MatterBridgedDeviceOnOff::Changed_t itemChangedMask)
{
    if (itemChangedMask & (MatterBridgedDeviceOnOff::kChanged_Reachable | MatterBridgedDeviceOnOff::kChanged_Name | MatterBridgedDeviceOnOff::kChanged_Location))
    {
        HandleDeviceStatusChanged(static_cast<MatterBridgeDevice *>(dev), (MatterBridgeDevice::Changed_t) itemChangedMask);
    }

    if (itemChangedMask & MatterBridgedDeviceOnOff::kChanged_OnOff)
    {
        ScheduleReportingCallback(dev, OnOff::Id, OnOff::Attributes::OnOff::Id);
    }
}

CHIP_ERROR matter_driver_bridge_light_init(void)
{
    ALight1.SetReachable(true);
    ALight1.SetChangeCallback(&HandleDeviceOnOffStatusChanged);

    return CHIP_NO_ERROR;
}

void matter_driver_uplink_update_handler(AppEvent *aEvent)
{
    chip::app::ConcreteAttributePath path = aEvent->path;

    switch(path.mClusterId)
    {
    case Clusters::OnOff::Id:
        if(path.mAttributeId == Clusters::OnOff::Attributes::OnOff::Id)
        {
            ALight1.Set(aEvent->value._u8, true);
        }
        break;
    case Clusters::Identify::Id:
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
    default:
        break;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

// add more stuff inside this bridgeData if needed
typedef struct {
    uint8_t type;
    uint8_t *payload;
} bridgeData;

void matter_customer_bridge_code(void *param)
{
    bridgeData *dummy;
    // this code will be implemented by customer
    while(1)
    {
        if (0 /* check for incoming bridgeData and put into dummy_buffer (malloc) */)
        {
            // setup downlink_event
            int type = 3; //dummy->type;
            switch(type)
            {
            case 1:
                // add new endpoint
                break;
            case 2:
                // remove endpoint
                break;
            case 3:
                {
                    // attribute change
                    uint8_t state = 1; //dummy->payload;
                    AppEvent downlink_event;
                    downlink_event.Type     = AppEvent::kEventType_Downlink_OnOff;  // assume this is a on/off command
                    downlink_event.value._u8 = (uint8_t) state; // toggle
                    downlink_event.mHandler = matter_driver_downlink_update_handler;
                    PostDownlinkEvent(&downlink_event);
                }
                break;
            case 4:
                // some other stuff, up to you
                break;
            default:
                break;
            }
            vTaskDelay(100);
        }
        vTaskDelay(500);
    }
}
