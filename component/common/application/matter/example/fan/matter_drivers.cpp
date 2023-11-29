#include "matter_drivers.h"
#include "matter_interaction.h"
#include "fan_driver.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

using namespace ::chip::app;

#define PWM_PIN         PA_23

MatterFan fan;

// Set identify cluster and its callback on ep1
static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, matter_driver_on_identify_start, matter_driver_on_identify_stop, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, matter_driver_on_trigger_effect,
};

CHIP_ERROR matter_driver_fan_init()
{
    fan.Init(PWM_PIN);
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_fan_set_startup_value()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EmberAfStatus status;
    DataModel::Nullable<uint8_t> FanPercentSettingValue;
    chip::app::Clusters::FanControl::FanModeEnum FanModeValue;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    status = Clusters::FanControl::Attributes::PercentSetting::Get(1, FanPercentSettingValue);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, err = CHIP_ERROR_INTERNAL);
    status = Clusters::FanControl::Attributes::FanMode::Get(1, &FanModeValue);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, err = CHIP_ERROR_INTERNAL);

    // Set fan speed to percent setting value
    fan.setFanSpeedPercent(FanPercentSettingValue.Value());
    fan.setFanMode((uint8_t) FanModeValue);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

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
    chip::app::ConcreteAttributePath path = aEvent->path;
    EmberAfStatus status;
    DataModel::Nullable<uint8_t> FanPercentSettingValue;

    // this example only considers endpoint 1
    VerifyOrExit(aEvent->path.mEndpointId == 1,
                 ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", path.mEndpointId));

    switch(path.mClusterId)
    {
    case Clusters::OnOff::Id:
        if (path.mAttributeId == Clusters::OnOff::Attributes::OnOff::Id)
        {
            fan.setFanMode((aEvent->value._u8 == 1) ? 4 /* kOn */ : 0 /* kOff */);
            chip::DeviceLayer::PlatformMgr().LockChipStack();
            Clusters::FanControl::Attributes::FanMode::Set(1, (aEvent->value._u8 == 1) ? Clusters::FanControl::FanModeEnum::kOn : Clusters::FanControl::FanModeEnum::kOff);
            chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        }
    case Clusters::FanControl::Id:
        if (path.mAttributeId == Clusters::FanControl::Attributes::PercentSetting::Id)
        {
            fan.setFanSpeedPercent(aEvent->value._u8);
        }
        else if (path.mAttributeId == Clusters::FanControl::Attributes::FanMode::Id)
        {
            fan.setFanMode(aEvent->value._u8);
        }
        break;
    case Clusters::Identify::Id:
        break;
    }

exit:
    return;
}
