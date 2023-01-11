#include "matter_drivers.h"
#include "matter_interaction.h"
#include "led_driver.h"
#include "gpio_irq_api.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>

using namespace ::chip::app;

#define PWM_LED         PA_23
#define GPIO_IRQ_PIN    PA_17

MatterLED led;
gpio_irq_t gpio_btn;

void matter_driver_button_callback(uint32_t id, gpio_irq_event event)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_OnOff;
    downlink_event.mHandler = matter_interaction_onoff_handler;
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

CHIP_ERROR matter_driver_led_set_onoff(uint8_t value)
{
    led.Set(value);
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_led_set_brightness(uint8_t value)
{
    led.SetBrightness(value);
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_led_toggle()
{
    led.Toggle();
    return CHIP_NO_ERROR;
}

bool matter_driver_led_get_onoff()
{
    return led.IsTurnedOn();
}

uint8_t matter_driver_led_get_level()
{
    return led.GetLevel();
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
    err = matter_driver_led_set_onoff(LEDOnOffValue);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to set startup onoff value");
        return CHIP_ERROR_INTERNAL;
    }
    // Set LED to currentlevel value
    // AppLED.SetBrightness(LEDCurrentLevelValue.Value());
    err = matter_driver_led_set_brightness(LEDCurrentLevelValue.Value());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to set startup level value");
        return CHIP_ERROR_INTERNAL;
    }

    return err;
}

void matter_driver_attribute_update(AppEvent *aEvent)
{
    // get endpoint, cluster, attribute, val
    // according to above, call the LED api
    chip::app::ConcreteAttributePath path = aEvent->path;

    // TODO: this example only considers endpoint1
    VerifyOrExit(aEvent->path.mEndpointId == 1,
                 ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", path.mEndpointId));

    switch(path.mClusterId)
    {
    case ZCL_ON_OFF_CLUSTER_ID:
        if(path.mAttributeId == ZCL_ON_OFF_ATTRIBUTE_ID)
        {
            // led.Set(aEvent->value);
            matter_driver_led_set_onoff(aEvent->value);
        }
        break;
    case ZCL_LEVEL_CONTROL_CLUSTER_ID:
        if(path.mAttributeId == ZCL_CURRENT_LEVEL_ATTRIBUTE_ID)
        {
            // led.SetBrightness(aEvent->value);
            matter_driver_led_set_brightness(aEvent->value);
        }
        break;
    case ZCL_IDENTIFY_CLUSTER_ID:
        break;
    }

exit:
    return;
}
