#include <matter_drivers.h>
#include <matter_interaction.h>
#include <temp_sensor_driver.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace ::chip::app;
using chip::Protocols::InteractionModel::Status;

#define SENSOR_PIN PA_19

MatterTemperatureSensor tempSensor;

// Set identify cluster and its callback on ep1
static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, matter_driver_on_identify_start, matter_driver_on_identify_stop, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, matter_driver_on_trigger_effect,
};

void matter_driver_set_measured_temp_cb(int16_t value)
{
    AppEvent downlink_event;
    downlink_event.Type = AppEvent::kEventType_Downlink_Temperature;
    downlink_event.value._i16 = (int16_t) value;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_on_identify_start(Identify *identify)
{
    ChipLogProgress(Zcl, "OnIdentifyStart");
}

void matter_driver_on_identify_stop(Identify *identify)
{
    ChipLogProgress(Zcl, "OnIdentifyStop");
}

void matter_driver_on_trigger_effect(Identify *identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBlink");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBreathe");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kOkay");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kChannelChange");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        return;
    }
}

void matter_driver_take_measurement(void *pvParameters)
{
    // User implementation to take measurement
    int16_t temperature = 0;

    while (1)
    {
        temperature = tempSensor.readTemperature();
        //printf("Temperature: %i *C \r\n", temperature);

        matter_driver_set_measured_temp_cb(temperature * 100);
        tempSensor.setMeasuredTemperature(temperature * 100);

        vTaskDelay(10000);
    }
}

CHIP_ERROR matter_driver_temperature_sensor_init(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Status status;
    int16_t minValue = -500;
    int16_t maxValue = 6000;

    chip::EndpointId ep = 1;

    tempSensor.SetEp(ep);

    tempSensor.Init(SENSOR_PIN);

    ChipLogProgress(DeviceLayer, "Temperature Sensor on Endpoint%d", ep);

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    status = Clusters::TemperatureMeasurement::Attributes::MinMeasuredValue::Set(ep, minValue);
    VerifyOrExit(status == Status::Success, err = CHIP_ERROR_INTERNAL);

    status = Clusters::TemperatureMeasurement::Attributes::MaxMeasuredValue::Set(ep, maxValue);
    VerifyOrExit(status == Status::Success, err = CHIP_ERROR_INTERNAL);

    ChipLogProgress(DeviceLayer, "Temperature range: Min = %i, Max = %i", minValue, maxValue);

    tempSensor.setMinMeasuredTemperature(minValue);
    tempSensor.setMaxMeasuredTemperature(maxValue);

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (xTaskCreate(matter_driver_take_measurement, "matter_driver_take_measurement", 1024, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        ChipLogError(DeviceLayer, "failed to create matter_driver_take_measurement");
    }

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
    case AppEvent::kEventType_Downlink_Temperature:
        {
            chip::EndpointId ep = tempSensor.GetEp();
            //ChipLogProgress(DeviceLayer, "Set Temperature %i on Endpoint%d", aEvent->value._i16, ep);
            Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(ep, aEvent->value._i16);
        }
        break;
    default:
        break;
    }

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}
