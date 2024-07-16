#include "matter_drivers.h"
#include "matter_interaction.h"
#include "fan_driver.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace ::chip::app;
using namespace chip::app::Clusters::FanControl;
using namespace chip::app::Clusters::FanControl::Attributes;
using chip::Protocols::InteractionModel::Status;

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

extern "C" void UpdateState(int state)
{
    Status status;
    chip::Percent FanPercentCurrentValue;

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    //Clusters::OnOff::Attributes::OnOff::Set(1, state);
    status = chip::app::Clusters::FanControl::Attributes::PercentSetting::Set(1, (uint8_t)state);
    VerifyOrReturn(Status::Success == status,
       ChipLogError(DeviceLayer, "Failed to Set PercentSetting with error: 0x%02x", chip::to_underlying(status)));

    fan.setFanSpeedPercent(state);

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

CHIP_ERROR matter_driver_fan_set_startup_value()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Status status;
    DataModel::Nullable<uint8_t> FanPercentSettingValue;
    chip::app::Clusters::FanControl::FanModeEnum FanModeValue;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    status = Clusters::FanControl::Attributes::PercentSetting::Get(1, FanPercentSettingValue);
    VerifyOrExit(status == Status::Success, err = CHIP_ERROR_INTERNAL);
    status = Clusters::FanControl::Attributes::FanMode::Get(1, &FanModeValue);
    VerifyOrExit(status == Status::Success, err = CHIP_ERROR_INTERNAL);

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
    Status status;
    chip::app::Clusters::FanControl::FanModeEnum FanModeValue;
    DataModel::Nullable<uint8_t> FanPercentSettingValue;

    // this example only considers endpoint 1
    VerifyOrExit(aEvent->path.mEndpointId == 1,
                 ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", path.mEndpointId));

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    ChipLogProgress(DeviceLayer, "Uplink: ClusterId=0x%x AttributeId=0x%x value=%d", path.mClusterId, path.mAttributeId, aEvent->value._u8);

    switch(path.mClusterId)
    {
    case Clusters::OnOff::Id:
        if (path.mAttributeId == Clusters::OnOff::Attributes::OnOff::Id)
        {
            fan.setFanMode((aEvent->value._u8 == 1) ? 4 /* kOn */ : 0 /* kOff */);
            Clusters::FanControl::Attributes::FanMode::Set(1, (aEvent->value._u8 == 1) ? Clusters::FanControl::FanModeEnum::kOn : Clusters::FanControl::FanModeEnum::kOff);
        }
    case Clusters::FanControl::Id:
        if (path.mAttributeId == Clusters::FanControl::Attributes::PercentSetting::Id)
        {
            status = chip::app::Clusters::FanControl::Attributes::FanMode::Set(1, fan.mapPercentToMode(aEvent->value._u8));
            VerifyOrReturn(Status::Success == status,
                   ChipLogError(DeviceLayer, "Failed to Set Fanmode with error: 0x%02x", chip::to_underlying(status)));

            fan.setFanSpeedPercent(aEvent->value._u8);
        }
        else if (path.mAttributeId == Clusters::FanControl::Attributes::FanMode::Id)
        {
            status = chip::app::Clusters::FanControl::Attributes::FanMode::Get(1, &FanModeValue);
            VerifyOrReturn(Status::Success == status,
                   ChipLogError(DeviceLayer, "Failed to Get Fanmode with error: 0x%02x", chip::to_underlying(status)));

            switch (FanModeValue)
            {
                case FanModeEnum::kOff:
                    if (aEvent->value._u8 != 0)
                    {
                        status = chip::app::Clusters::FanControl::Attributes::FanMode::Set(1, FanModeEnum::kOn);
                        VerifyOrReturn(Status::Success == status,
                               ChipLogError(DeviceLayer, "Failed to Set Fanmode with error: 0x%02x", chip::to_underlying(status)));
                    }
                    break;
                case FanModeEnum::kHigh:
                    status = Clusters::FanControl::Attributes::PercentSetting::Get(1, FanPercentSettingValue);
                    VerifyOrReturn(Status::Success == status,
                           ChipLogError(DeviceLayer, "Failed to Get Fanmode with error: 0x%02x", chip::to_underlying(status)));
                    if (FanPercentSettingValue.Value() == 0)
                    {
                        uint8_t value8 = fan.getPrevFanSpeedPercent();
                        if (value8 == 0)
                        {
                            value8 = fan.mapModeToPercent(aEvent->value._u8);
                        }
                        status = chip::app::Clusters::FanControl::Attributes::PercentSetting::Set(1, value8);
                        VerifyOrReturn(Status::Success == status,
                               ChipLogError(DeviceLayer, "Failed to Set PercentSetting with error: 0x%02x", chip::to_underlying(status)));
                    }
                    break;
                default:
                    break;
            }
            fan.setFanMode(aEvent->value._u8);
        }
        break;
    case Clusters::Identify::Id:
        break;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
exit:
    return;
}
