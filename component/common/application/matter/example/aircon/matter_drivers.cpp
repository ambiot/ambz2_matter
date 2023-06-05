#include "matter_drivers.h"
#include "matter_interaction.h"
#include "fan_driver.h"
// #include "gpio_irq_api.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

using namespace ::chip::app;

#define PWM_PIN         PA_23
// #define GPIO_IRQ_PIN    PA_17

MatterFan fan;
// gpio_irq_t gpio_btn;

// Set identify cluster and its callback on ep1
static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, matter_driver_on_identify_start, matter_driver_on_identify_stop, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, matter_driver_on_trigger_effect,
};

// void matter_driver_button_callback(uint32_t id, gpio_irq_event event)
// {
//     AppEvent downlink_event;
//     downlink_event.Type     = AppEvent::kEventType_Downlink_OnOff;
//     downlink_event.value._u8 = (uint8_t) !led.IsTurnedOn(); // toggle
//     downlink_event.mHandler = matter_driver_downlink_update_handler;
//     PostDownlinkEvent(&downlink_event);
// }

// CHIP_ERROR matter_driver_button_init()
// {
//     // TODO: error checking
//     // Initialize downlink interrupt source here
//     // Initial Push Button pin as interrupt source
//     gpio_irq_init(&gpio_btn, GPIO_IRQ_PIN, matter_driver_button_callback, 1);
//     gpio_irq_set(&gpio_btn, IRQ_FALL, 1);   // Falling Edge Trigger
//     gpio_irq_enable(&gpio_btn) ;
//     return CHIP_NO_ERROR;
// }

CHIP_ERROR matter_driver_fan_init()
{
    fan.Init(PWM_PIN);
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_fan_set_startup_value()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // bool LEDOnOffValue = 0;
    DataModel::Nullable<uint8_t> FanPercentSettingValue;
    // uint8_t newMode;
    // EmberAfStatus getpercentsettingstatus;
    // EmberAfStatus getcurrentlevelstatus;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    EmberAfStatus status = Clusters::FanControl::Attributes::PercentSetting::Get(1, FanPercentSettingValue);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, err = CHIP_ERROR_INTERNAL);

    // getcurrentlevelstatus = Clusters::LevelControl::Attributes::CurrentLevel::Get(1, LEDCurrentLevelValue);
    // VerifyOrExit(getcurrentlevelstatus == EMBER_ZCL_STATUS_SUCCESS, err = CHIP_ERROR_INTERNAL);

    // Set fan speed to percent setting value
    fan.setFanSpeedPercent(FanPercentSettingValue.Value());
    // newMode = fan.mapPercentToMode(FanPercentSettingValue.Value());
    status = Clusters::FanControl::Attributes::FanMode::Set(1, fan.mapPercentToMode(FanPercentSettingValue.Value()));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Updating fan mode failed: %x", status);
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    // fan.setFanMode(newMode);

    // Set LED to currentlevel value
    // led.SetBrightness(LEDCurrentLevelValue.Value());

exit:
    if (err == CHIP_ERROR_INTERNAL)
    {
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    return err;
}

void matter_driver_on_identify_start(Identify * identify)
{
    ChipLogProgress(Zcl, "OnIdentifyStart");
}

void matter_driver_on_identify_stop(Identify * identify)
{
    ChipLogProgress(Zcl, "OnIdentifyStop");
}

void matter_driver_on_trigger_effect(Identify * identify)
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

void matter_driver_uplink_update_handler(AppEvent *aEvent)
{
    // uint8_t newMode;
    chip::app::ConcreteAttributePath path = aEvent->path;
    EmberAfStatus status;
    DataModel::Nullable<uint8_t> FanPercentSettingValue;
    chip::app::Clusters::FanControl::FanModeType FanModeValue;

    // this example only considers endpoint1
    VerifyOrExit(aEvent->path.mEndpointId == 1,
                 ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", path.mEndpointId));

    switch(path.mClusterId)
    {
    case Clusters::FanControl::Id:
        if (path.mAttributeId == Clusters::FanControl::Attributes::PercentSetting::Id)
        {
            printf("%s %d percent setting: %d\r\n", __FUNCTION__, __LINE__, aEvent->value._u8);
            fan.setFanMode((uint8_t) fan.mapPercentToMode(aEvent->value._u8));
            fan.setFanSpeedPercent(aEvent->value._u8);

            chip::DeviceLayer::PlatformMgr().LockChipStack();
            status = Clusters::FanControl::Attributes::FanMode::Get(1, &FanModeValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogError(DeviceLayer, "Get fan mode failed: %x", status);
            }
            chip::DeviceLayer::PlatformMgr().UnlockChipStack();
            
            if ((uint8_t) FanModeValue != fan.mMode)
            {
                printf("%s %d \r\n", __FUNCTION__, __LINE__);
                chip::DeviceLayer::PlatformMgr().LockChipStack();
                status = Clusters::FanControl::Attributes::FanMode::Set(1, fan.mapPercentToMode(aEvent->value._u8));
                if (status != EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogError(DeviceLayer, "Updating fan mode failed: %x", status);
                }
                chip::DeviceLayer::PlatformMgr().UnlockChipStack();
            }
        }
        else if (path.mAttributeId == Clusters::FanControl::Attributes::FanMode::Id)
        {
            fan.setFanMode(aEvent->value._u8);
            fan.setFanSpeedPercent(fan.mapModeToPercent(aEvent->value._u8));
            
            chip::DeviceLayer::PlatformMgr().LockChipStack();
            status = Clusters::FanControl::Attributes::PercentSetting::Get(1, FanPercentSettingValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogError(DeviceLayer, "Get fan percent failed: %x", status);
            }
            chip::DeviceLayer::PlatformMgr().UnlockChipStack();

            if (FanPercentSettingValue.Value() != fan.mPercent)
            {
                printf("%s %d \r\n", __FUNCTION__, __LINE__);
                chip::DeviceLayer::PlatformMgr().LockChipStack();
                status = Clusters::FanControl::Attributes::PercentSetting::Set(1, fan.mapModeToPercent(aEvent->value._u8));
                if (status != EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogError(DeviceLayer, "Updating fan percent failed: %x", status);
                }
                chip::DeviceLayer::PlatformMgr().UnlockChipStack();
            }
        }
        break;
    case Clusters::Identify::Id:
        break;
    }

exit:
    return;
}

// void matter_driver_downlink_update_handler(AppEvent * event)
// {
//     chip::DeviceLayer::PlatformMgr().LockChipStack();
//     switch (event->Type)
//     {
//         case AppEvent::kEventType_Downlink_OnOff:
//             led.Toggle();
//             ChipLogProgress(DeviceLayer, "Writing to OnOff cluster");
//             EmberAfStatus status = Clusters::OnOff::Attributes::OnOff::Set(1, led.IsTurnedOn());

//             if (status != EMBER_ZCL_STATUS_SUCCESS)
//             {
//                 ChipLogError(DeviceLayer, "Updating on/off cluster failed: %x", status);
//             }

//             ChipLogProgress(DeviceLayer, "Writing to LevelControl cluster");
//             status = Clusters::LevelControl::Attributes::CurrentLevel::Set(1, led.GetLevel());

//             if (status != EMBER_ZCL_STATUS_SUCCESS)
//             {
//                 ChipLogError(DeviceLayer, "Updating level cluster failed: %x", status);
//             }
//             break;
//     }
//     chip::DeviceLayer::PlatformMgr().UnlockChipStack();
// }
