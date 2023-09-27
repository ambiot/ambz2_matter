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
using namespace chip::app::Clusters::TemperatureControl;
using namespace chip::app::Clusters::TemperatureMeasurement;

#define GPIO_IRQ_LEVEL_PIN    PA_17
#define GPIO_LED_PIN PA_19

MatterRefrigerator refrigerator;
RefrigeratorAlarmServer refrigeratorAlarmObject;
gpio_irq_t gpio_level;
uint32_t current_level = IRQ_LOW;

void matter_driver_gpio_level_irq_handler(uint32_t id, gpio_irq_event event)
{
    uint32_t *level = (uint32_t *) id;

    // Disable level irq because the irq will keep triggered when it keeps in same level.
    gpio_irq_disable(&gpio_level);

    if (*level == IRQ_LOW) { // Door closed
        refrigerator.SetDoorStatus((uint8_t) 0);
        matter_driver_set_door_callback((uint32_t) 0);

        // Change to listen to high level event
        *level = IRQ_HIGH;
        gpio_irq_set(&gpio_level, (gpio_irq_event)IRQ_HIGH, 1);
        gpio_irq_enable(&gpio_level);
    } else if (*level == IRQ_HIGH) { // Door opened
        refrigerator.SetDoorStatus((uint8_t) 1);
        matter_driver_set_door_callback((uint32_t) 1);

        // Change to listen to low level event
        *level = IRQ_LOW;
        gpio_irq_set(&gpio_level, (gpio_irq_event)IRQ_LOW, 1);
        gpio_irq_enable(&gpio_level);
    }
}

CHIP_ERROR matter_driver_refrigerator_init()
{
    refrigerator.Init(GPIO_LED_PIN);

    gpio_irq_init(&gpio_level, GPIO_IRQ_LEVEL_PIN, matter_driver_gpio_level_irq_handler, (uint32_t)(&current_level));
    gpio_irq_set(&gpio_level, (gpio_irq_event)IRQ_LOW, 1);
    gpio_irq_enable(&gpio_level);

    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_refrigerator_set_startup_value(int8_t minTemp, int8_t maxTemp)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    refrigerator.SetTemperatureRange(minTemp, maxTemp); // Set fridge temperature range
    matter_driver_set_temperature_callback(refrigerator.GetTemperature()); // Update the matter layer

    return err;
}

void matter_driver_set_door_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_Refrigerator_Alarm_SetStateValue;
    downlink_event.value._u8 = (uint8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_temperature_callback(int32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_Refrigerator_SetTemperaturePoint;
    downlink_event.value._i8 = (int8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_refrigerator_status(void)
{
    ChipLogProgress(DeviceLayer, "Refrigerator Status: \nTemperature: %i\n Door: %i\n", refrigerator.GetTemperature(), refrigerator.GetDoorStatus());
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
    case Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Id:
        {
            ChipLogProgress(DeviceLayer, "RefrigeratorAndTemperatureControlledCabinetMode(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
        }
        break;
    case Clusters::RefrigeratorAlarm::Id:
        {
            ChipLogProgress(DeviceLayer, "RefrigeratorAlarm(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
            if (path.mAttributeId == Clusters::RefrigeratorAlarm::Attributes::State::Id)
                refrigerator.SetInnerLight(); // Turn on Light when alarm is on
        }
        break;
    case Clusters::TemperatureControl::Id:
        {
            ChipLogProgress(DeviceLayer, "TemperatureControl(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
            if (path.mAttributeId == Clusters::TemperatureControl::Attributes::TemperatureSetpoint::Id)
                refrigerator.SetTemperature(event->value._i8); // Change physical temperature    
        }
        break;
    case Clusters::TemperatureMeasurement::Id:
        {
            ChipLogProgress(DeviceLayer, "TemperatureMeasurement(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
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
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    switch (event->Type)
    {
    case AppEvent::kEventType_Downlink_Refrigerator_Alarm_SetStateValue:
        {
            BitMask<AlarmMap> value;
            value.SetField(AlarmMap::kDoorOpen, event->value._u8);
            ChipLogProgress(DeviceLayer, "Set Refrigerator Alarm State Value 0x%x", event->value._u8);
            status = refrigeratorAlarmObject.SetStateValue(1, value);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogProgress(DeviceLayer, "Failed to set door status!\n");
            }
        }
        break;
    case AppEvent::kEventType_Downlink_Refrigerator_SetTemperaturePoint:
        {
            if ((event->value._i8 >= refrigerator.GetMinTemperature()) && (event->value._i8 <= refrigerator.GetMaxTemperature()))
            {
                ChipLogProgress(DeviceLayer, "Set TemperatureSetpoint %i", event->value._i8);
                status = Clusters::TemperatureControl::Attributes::TemperatureSetpoint::Set(1, event->value._i8);

                if (status != EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogProgress(DeviceLayer, "Failed to set TemperatureSetpoint!\n");
                }
                else
                {
                    ChipLogProgress(DeviceLayer, "Set MeasuredLevel %i", event->value._i8);
                    status = Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(1, event->value._i8);
                    if (status != EMBER_ZCL_STATUS_SUCCESS)
                    {
                        ChipLogProgress(DeviceLayer, "Failed to set MeasuredValue!\n");
                    }
                }
            }
            else
                ChipLogProgress(DeviceLayer, "Temperature must be set between %i and %i", refrigerator.GetMinTemperature(), refrigerator.GetMaxTemperature());
        }
        break;
    default:
        break;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}
