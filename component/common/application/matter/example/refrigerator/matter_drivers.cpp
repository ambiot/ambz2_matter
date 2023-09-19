#include "matter_drivers.h"
#include "matter_interaction.h"
#include "refrigerator_driver.h"
// #include "gpio_api.h"
// #include "gpio_irq_api.h"
// #include "gpio_irq_ex_api.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
// #include <clusters/refrigerator-alarm-server/refrigerator-alarm-server.h>

using namespace ::chip::app;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RefrigeratorAlarm;
using namespace chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode;
using namespace chip::app::Clusters::TemperatureControl;
using namespace chip::app::Clusters::TemperatureMeasurement;

#define PWM_PIN         PA_23
#define GPIO_IRQ_LEVEL_PIN    PA_17

MatterRefrigerator refrigerator;
gpio_irq_t gpio_level;
uint32_t current_level = IRQ_LOW;

// void matter_driver_button_trigger_fall()
// {
//     gpio_irq_free(&gpio_btn);
//     gpio_irq_init(&gpio_btn, GPIO_IRQ_PIN, matter_driver_button_fall_callback, 0);
//     gpio_irq_set(&gpio_btn, IRQ_FALL, 1);   // Falling Edge Trigger
//     gpio_irq_enable(&gpio_btn);
//     ChipLogProgress(DeviceLayer, "Set Fall Trigger to GPIO\n");
// }

// void matter_driver_button_trigger_rise()
// {
//     gpio_irq_free(&gpio_btn);
//     gpio_irq_init(&gpio_btn, GPIO_IRQ_PIN, matter_driver_button_rise_callback, 1);
//     gpio_irq_set(&gpio_btn, IRQ_RISE, 1);   // Rising Edge Trigger
//     gpio_irq_enable(&gpio_btn);
//     ChipLogProgress(DeviceLayer, "Set Rise Trigger to GPIO\n");
// }

// void matter_driver_button_fall_callback(uint32_t id, gpio_irq_event event)
// {
//     AppEvent downlink_event;
//     downlink_event.Type     = AppEvent::kEventType_Downlink_Refrigerator_Alarm_SetStateValue;
//     downlink_event.value._u8 = (uint8_t) id;
//     downlink_event.mHandler = matter_driver_downlink_update_handler;
//     PostDownlinkEvent(&downlink_event);
//     matter_driver_button_trigger_rise();
// }

// void matter_driver_button_rise_callback(uint32_t id, gpio_irq_event event)
// {
//     AppEvent downlink_event;
//     downlink_event.Type     = AppEvent::kEventType_Downlink_Refrigerator_Alarm_SetStateValue;
//     downlink_event.value._u8 = (uint8_t) id;
//     downlink_event.mHandler = matter_driver_downlink_update_handler;
//     PostDownlinkEvent(&downlink_event);
//     matter_driver_button_trigger_fall();
// }

void matter_driver_gpio_level_irq_handler(uint32_t id, gpio_irq_event event)
{
    uint32_t *level = (uint32_t *) id;

    // Disable level irq because the irq will keep triggered when it keeps in same level.
    gpio_irq_disable(&gpio_level);

    // make some software de-bounce here if the signal source is not stable.
    if (*level == IRQ_LOW) {
        refrigerator.SetDoorStatus((uint8_t) 0);
        matter_driver_set_door_callback((uint32_t) 0);

        // Change to listen to high level event
        *level = IRQ_HIGH;
        gpio_irq_set(&gpio_level, (gpio_irq_event)IRQ_HIGH, 1);
        gpio_irq_enable(&gpio_level);
    } else if (*level == IRQ_HIGH) {
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
    refrigerator.Init(PWM_PIN);

    // matter_driver_button_trigger_rise();

    gpio_irq_init(&gpio_level, GPIO_IRQ_LEVEL_PIN, matter_driver_gpio_level_irq_handler, (uint32_t)(&current_level));
    gpio_irq_set(&gpio_level, (gpio_irq_event)IRQ_LOW, 1);
    gpio_irq_enable(&gpio_level);

    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_refrigerator_set_startup_value()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    refrigerator.SetTemperatureRange((int8_t) 0, (int8_t) 4);

    // BitMask<AlarmMap> value;

    // value.SetField(AlarmMap::kDoorOpen, 0);
    // RefrigeratorAlarmServer::Instance().SetSupportedValue(1, value);
    // ChipLogProgress(DeviceLayer, "SetSupportedValue to 0\n");

    // value.SetField(AlarmMap::kDoorOpen, 1);
    // RefrigeratorAlarmServer::Instance().SetSupportedValue(1, value);
    // ChipLogProgress(DeviceLayer, "SetSupportedValue to 1\n");

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
    downlink_event.Type     = AppEvent::kEventType_Downlink_Refrigerator_Set_Temperature;
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
        }
        break;
    case Clusters::TemperatureControl::Id:
        {
            ChipLogProgress(DeviceLayer, "TemperatureControl(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
            refrigerator.SetTemperature(event->value._i8);
            // status = Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(1, event->value._i8);
            // if (status != EMBER_ZCL_STATUS_SUCCESS)
            // {
            //     ChipLogProgress(DeviceLayer, "Failed to set temperature status!\n");
            // }
            // else
            // {
            //     refrigerator.SetTemperature(event->value._i8);
            // }        
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
            // RefrigeratorAlarmServer::Instance().SetStateValue(1, value);
            status = Clusters::RefrigeratorAlarm::Attributes::State::Set(1,value);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogProgress(DeviceLayer, "Failed to set door status!\n");
            }
            // else
            // {
            //     refrigerator.SetDoorStatus(event->value._u8);
            // }
        }
        break;
    case AppEvent::kEventType_Downlink_Refrigerator_Set_Temperature:
        {
            if ((event->value._u8 >= refrigerator.GetMinTemperature()) && (event->value._u8 <= refrigerator.GetMaxTemperature()))
            {
                ChipLogProgress(DeviceLayer, "Set SelectedTemperatureLevel %i", event->value._i8);
                status = Clusters::TemperatureControl::Attributes::SelectedTemperatureLevel::Set(1, event->value._i8);
                if (status != EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogProgress(DeviceLayer, "Failed to set SelectedTemperatureLevel!\n");
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
