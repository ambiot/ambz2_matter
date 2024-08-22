#include <matter_drivers.h>
#include <matter_interaction.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace ::chip::app;
using chip::Protocols::InteractionModel::Status;

// Initialize any GPIO/PWM pin if required
#define OUTPUT_PIN PA_19

// Create an instance for the device driver
MatterTemperatureSensor tempSensor;

void matter_driver_set_downlink_callback(int16_t value)
{
    //This is an example to report temperature by the DUT to the Matter Controller via Downlink Task
    AppEvent downlink_event;
    downlink_event.Type = AppEvent::kEventType_Downlink_Temperature;
    downlink_event.value._i16 = (int16_t) value;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

CHIP_ERROR matter_driver_template_init(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Status status;

    chip::EndpointId ep = 1;

    //Create a device with the device driver e.g., MatterTemperatureSensor and do any initialization here

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    //Initialize any Matter attribute, if necessary

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

exit:
    if (err == CHIP_ERROR_INTERNAL)
    {
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    return err;
}

void matter_driver_uplink_update_handler(AppEvent *aEvent)
{
    chip::app::ConcreteAttributePath path = aEvent->path;

    // this example only considers endpoint 1
    VerifyOrExit(aEvent->path.mEndpointId == 1,
                 ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", path.mEndpointId));

    switch (path.mClusterId)
    {
    // Handle any commands from Matter Controller reported from Matter Layer

    // This is an example to handle any changes for TemperatureMeasurement Cluster MeasuredValue Attribute by the Matter Controller
    case Clusters::TemperatureMeasurement::Id:
        if (path.mAttributeId == Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Id)
        {
            tempSensor.setMeasuredTemperature(aEvent->value._u16);
        }
        break;
    }

exit:
    return;
}

void matter_driver_downlink_update_handler(AppEvent *aEvent)
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();

    switch (aEvent->Type)
    {
    // Provides any update required to the Matter Layer to alert Matter Controller of Attribute value changed

    // This is an example of updating MeasuredValue for TemperatureMeasurement Clusters
    case AppEvent::kEventType_Downlink_Temperature:
        {
            chip::EndpointId ep = tempSensor.GetEp();
            Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(ep, aEvent->value._i16);
        }
        break;
    }

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}
