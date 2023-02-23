#include "matter_drivers.h"
#include "matter_interaction.h"
#include "led_driver.h"
#include "gpio_irq_api.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

using namespace ::chip::app;

#define PWM_LED         PA_23
#define GPIO_IRQ_PIN    PA_17

MatterLED led;
gpio_irq_t gpio_btn;

void matter_driver_button_callback(uint32_t id, gpio_irq_event event)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_OnOff;
    downlink_event.value._u8 = (uint8_t) !led.IsTurnedOn(); // toggle
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

CHIP_ERROR matter_driver_button_init()
{
    // TODO: error checking
    // Initialize downlink interrupt source here
    // Initial Push Button pin as interrupt source
    gpio_irq_init(&gpio_btn, GPIO_IRQ_PIN, matter_driver_button_callback, 1);
    gpio_irq_set(&gpio_btn, IRQ_FALL, 1);   // Falling Edge Trigger
    gpio_irq_enable(&gpio_btn) ;
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_led_init()
{
    led.Init(PWM_LED);
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_led_set_startup_value()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool LEDOnOffValue = 0;
    DataModel::Nullable<uint8_t> LEDCurrentLevelValue;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    // TODO: this example only considers endpoint1
    EmberAfStatus onoffstatus = Clusters::OnOff::Attributes::OnOff::Get(1, &LEDOnOffValue);
    if (onoffstatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to read onoff value: %x", onoffstatus);
        return CHIP_ERROR_INTERNAL;
    }

    // TODO: this example only considers endpoint1
    EmberAfStatus currentlevelstatus = Clusters::LevelControl::Attributes::CurrentLevel::Get(1, LEDCurrentLevelValue);
    if (currentlevelstatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to read currentlevel value: %x", currentlevelstatus);
        return CHIP_ERROR_INTERNAL;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    // Set LED to onoff value
    // AppLED.Set(LEDOnOffValue);
    led.Set(LEDOnOffValue);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to set startup onoff value");
        return CHIP_ERROR_INTERNAL;
    }
    // Set LED to currentlevel value
    led.SetBrightness(LEDCurrentLevelValue.Value());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to set startup level value");
        return CHIP_ERROR_INTERNAL;
    }

    return err;
}

void matter_driver_uplink_update_handler(AppEvent *aEvent)
{
    chip::app::ConcreteAttributePath path = aEvent->path;

    // this example only considers endpoint1
    VerifyOrExit(aEvent->path.mEndpointId == 1,
                 ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", path.mEndpointId));

    switch(path.mClusterId)
    {
    case Clusters::OnOff::Id:
        if(path.mAttributeId == Clusters::OnOff::Attributes::OnOff::Id)
        {
            led.Set(aEvent->value._u8);
        }
        break;
    case Clusters::LevelControl::Id:
        if(path.mAttributeId == Clusters::LevelControl::Attributes::CurrentLevel::Id)
        {
            led.SetBrightness(aEvent->value._u8);
        }
        break;
    case Clusters::Identify::Id:
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
        case AppEvent::kEventType_Downlink_OnOff:
            led.Toggle();
            ChipLogProgress(DeviceLayer, "Writing to OnOff cluster");
            EmberAfStatus status = Clusters::OnOff::Attributes::OnOff::Set(1, led.IsTurnedOn());

            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogError(DeviceLayer, "Updating on/off cluster failed: %x", status);
            }

            ChipLogProgress(DeviceLayer, "Writing to LevelControl cluster");
            status = Clusters::LevelControl::Attributes::CurrentLevel::Set(1, led.GetLevel());

            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogError(DeviceLayer, "Updating level cluster failed: %x", status);
            }
            break;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}
