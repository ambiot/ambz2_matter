#include "matter_drivers.h"
#include "matter_interaction.h"
#include "fan_driver.h"
#include "temp_hum_sensor_driver.h"
#include "opstate_driver.h"
#include "washer_driver.h"
#include "refrigerator_driver.h"
#include "tcc_mode.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <clusters/laundry-washer-controls-server/laundry-washer-controls-server.h>
#include <clusters/refrigerator-alarm-server/refrigerator-alarm-server.h>

using namespace ::chip::app;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::LaundryWasherControls;
using namespace chip::app::Clusters::ModeSelect;
using namespace chip::app::Clusters::RefrigeratorAlarm;
using namespace chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode;

#define GPIO_IRQ_LEVEL_PIN    PA_17
#define GPIO_LED_PIN          PA_19
#define FAN_PWM_PIN           PA_23
#define WASHER_PWM_PIN        PA_22

MatterFan fan;
MatterTemperatureHumiditySensor tempHumSensor;
MatterWasher washer;
MatterRefrigerator refrigerator;
gpio_irq_t gpio_level;
uint32_t current_level = IRQ_LOW;

// Set identify cluster and its callback on ep1
static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, matter_driver_on_identify_start, matter_driver_on_identify_stop, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, matter_driver_on_trigger_effect,
};

CHIP_ERROR matter_driver_fan_init()
{
    fan.Init(FAN_PWM_PIN);
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

CHIP_ERROR matter_driver_temphumsensor_init()
{
    tempHumSensor.Init();
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_temphumsensor_start()
{
    tempHumSensor.startPollingTask();
    return CHIP_NO_ERROR;
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

/* LaundryWasher API */
CHIP_ERROR matter_driver_laundry_washer_init()
{
    washer.Init(WASHER_PWM_PIN);
    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_laundry_washer_set_startup_value()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EmberAfStatus status;

    DataModel::Nullable<uint8_t> SpinSpeedCurrent;
    chip::app::Clusters::LaundryWasherControls::NumberOfRinsesEnum NumberOfRinses;
    uint8_t currentMode = 0;
    int16_t minTemperature = 0;
    int16_t maxTemperature = 0;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    
    status = Clusters::LaundryWasherControls::Attributes::SpinSpeedCurrent::Get(1, SpinSpeedCurrent);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, err = CHIP_ERROR_INTERNAL);
    status = Clusters::LaundryWasherControls::Attributes::NumberOfRinses::Get(1, &NumberOfRinses);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, err = CHIP_ERROR_INTERNAL);

    GetOperationalStateInstance()->SetOperationalState(chip::to_underlying(Clusters::OperationalState::OperationalStateEnum::kRunning)); 

    status = Clusters::TemperatureControl::Attributes::MinTemperature::Get(1, &minTemperature);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, err = CHIP_ERROR_INTERNAL);
    status = Clusters::TemperatureControl::Attributes::MaxTemperature::Get(1, &maxTemperature);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, err = CHIP_ERROR_INTERNAL);

    washer.setSpinSpeedCurrent(SpinSpeedCurrent.Value());
    washer.setNumberOfRinses((uint8_t) NumberOfRinses);
    washer.setCurrentMode(0);
    washer.setOpState((uint8_t) Clusters::OperationalState::OperationalStateEnum::kRunning);
    washer.setMinMaxTemperature(minTemperature, maxTemperature);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

exit:
    if (err == CHIP_ERROR_INTERNAL)
    {
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    return err;
}


void matter_driver_set_opstate_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_Opstate_State;
    downlink_event.value._u8 = (uint8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_spinspeed_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_LW_SpinSpeed;
    downlink_event.value._u8 = (uint8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_rinses_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_LW_NumberOfRinses;
    downlink_event.value._u8 = (uint8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_laundrywasher_mode_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_LW_Mode;
    downlink_event.value._u8 = (uint8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_gpio_level_irq_handler(uint32_t id, gpio_irq_event event)
{
    uint32_t *level = (uint32_t *) id;
    if (*level == IRQ_LOW) { // Door closed
        refrigerator.SetDoorStatus((uint8_t) 0);
        matter_driver_set_door_callback((uint32_t) 0);

        // Change to listen to high level event
        *level = IRQ_HIGH;
        gpio_irq_set(&gpio_level, (gpio_irq_event)IRQ_HIGH, 1);
    } else if (*level == IRQ_HIGH) { // Door opened
        refrigerator.SetDoorStatus((uint8_t) 1);
        matter_driver_set_door_callback((uint32_t) 1);

        // Change to listen to low level event
        *level = IRQ_LOW;
        gpio_irq_set(&gpio_level, (gpio_irq_event)IRQ_LOW, 1);
    }
}

CHIP_ERROR matter_driver_refrigerator_init(void)
{
    refrigerator.Init(GPIO_LED_PIN);

    gpio_irq_init(&gpio_level, GPIO_IRQ_LEVEL_PIN, matter_driver_gpio_level_irq_handler, (uint32_t)(&current_level));
    gpio_irq_set(&gpio_level, (gpio_irq_event)IRQ_LOW, 1);
    gpio_irq_enable(&gpio_level);

    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_refrigerator_set_startup_value(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Clusters::ModeBase::Commands::ChangeToModeResponse::Type modeChangedResponse;
    EmberAfStatus status;
    Clusters::ModeBase::Instance & refrigeratorObject = Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Instance();
    RefrigeratorAlarmServer & refrigeratorAlarmObject = RefrigeratorAlarmServer::Instance();
    chip::DeviceLayer::PlatformMgr().LockChipStack();

    modeChangedResponse.status = to_underlying(refrigeratorObject.UpdateCurrentMode(to_underlying(ModeTag::kRapidCool))); // Set refrigerator mode
    if (modeChangedResponse.status != to_underlying(Clusters::ModeBase::StatusCode::kSuccess))
    {
        ChipLogProgress(DeviceLayer, "Failed to set Refrigerator Mode!\n");
        err = CHIP_ERROR_INTERNAL;
    }

    BitMask<AlarmMap> supported; // Set refrigerator alarm supported value
    supported.SetField(AlarmMap::kDoorOpen, 1);
    refrigeratorAlarmObject.SetSupportedValue(1, supported);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "Failed to set Refrigerator Alarm Supported Value!\n");
        err = CHIP_ERROR_INTERNAL;
    }

    BitMask<AlarmMap> mask; // Set refrigerator alarm mask value
    mask.SetField(AlarmMap::kDoorOpen, 1);
    refrigeratorAlarmObject.SetMaskValue(1, mask);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "Failed to set Refrigerator Alarm Mask Value!\n");
        err = CHIP_ERROR_INTERNAL;
    }

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    return err;
}

void matter_driver_set_refrigerator_mode_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_Refrigerator_Mode;
    downlink_event.value._u16 = (uint16_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
}

void matter_driver_set_door_callback(uint32_t id)
{
    AppEvent downlink_event;
    downlink_event.Type     = AppEvent::kEventType_Downlink_Refrigerator_Alarm_State;
    downlink_event.value._u8 = (uint8_t) id;
    downlink_event.mHandler = matter_driver_downlink_update_handler;
    PostDownlinkEvent(&downlink_event);
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
    case Clusters::LaundryWasherMode::Id:
        {
            ChipLogProgress(DeviceLayer, "LaundryWasherMode(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
            if (path.mAttributeId == Clusters::LaundryWasherMode::Attributes::OnMode::Id)
            {
                washer.setCurrentMode(aEvent->value._u16);
            }
            if (path.mAttributeId == Clusters::LaundryWasherMode::Attributes::StartUpMode::Id)
            {
                washer.setCurrentMode(aEvent->value._u16);
            }
        }
        break;
    case Clusters::LaundryWasherControls::Id:
        {
            ChipLogProgress(DeviceLayer, "LaundryWasherControl(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
            if (path.mAttributeId == Clusters::LaundryWasherControls::Attributes::SpinSpeedCurrent::Id)
            {
                washer.setSpinSpeedCurrent(aEvent->value._u16);
            }
            else if (path.mAttributeId == Clusters::LaundryWasherControls::Attributes::NumberOfRinses::Id)
            {
                washer.setNumberOfRinses(aEvent->value._u16);
            }
        }
        break;
    case Clusters::TemperatureControl::Id:
        {
            ChipLogProgress(DeviceLayer, "TemperatureControl(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
        }
        break;
    case Clusters::OperationalState::Id:
        {
            ChipLogProgress(DeviceLayer, "OperationalState(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
        }
        break;
    case Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Id:
        {
            ChipLogProgress(DeviceLayer, "RefrigeratorAndTemperatureControlledCabinetMode(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
            if (path.mAttributeId == Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Attributes::CurrentMode::Id)
                refrigerator.SetMode(aEvent->value._u16);
        }
        break;
    case Clusters::RefrigeratorAlarm::Id:
        {
            ChipLogProgress(DeviceLayer, "RefrigeratorAlarm(ClusterId=0x%x) at Endpoint%x: change AttributeId=0x%x\n", path.mEndpointId, path.mClusterId, path.mAttributeId);
            if (path.mAttributeId == Clusters::RefrigeratorAlarm::Attributes::State::Id)
                refrigerator.SetAlarm();
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
    Clusters::ModeBase::Commands::ChangeToModeResponse::Type modeChangedResponse;
    EmberAfStatus alarmChangedStatus;
    Clusters::ModeBase::Instance & refrigeratorObject = Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Instance();
    RefrigeratorAlarmServer & refrigeratorAlarmObject = RefrigeratorAlarmServer::Instance();

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    switch (event->Type)
    {
    case AppEvent::kEventType_Downlink_Opstate_State:
        {
            ChipLogProgress(DeviceLayer, "Set Operational State 0x%x", event->value._u8);
            GetOperationalStateInstance()->SetOperationalState(event->value._u8); 
        }
        break;
    case AppEvent::kEventType_Downlink_LW_SpinSpeed:
        {
            ChipLogProgress(DeviceLayer, "Set Spin Speed0x%x", event->value._u8);
            washer.setSpinSpeedCurrent(event->value._u8);
            Clusters::LaundryWasherControls::Attributes::SpinSpeedCurrent::Set(1, event->value._u8);
        }
        break;
    case AppEvent::kEventType_Downlink_LW_NumberOfRinses:
        {
            ChipLogProgress(DeviceLayer, "Set Number Of Rinses 0x%x", event->value._u8);
            washer.setNumberOfRinses(event->value._u8);
            switch(event->value._u8)
            {
                case 0:
                    Clusters::LaundryWasherControls::Attributes::NumberOfRinses::Set(1, Clusters::LaundryWasherControls::NumberOfRinsesEnum::kNone);
                    break;
                case 1:
                    Clusters::LaundryWasherControls::Attributes::NumberOfRinses::Set(1, Clusters::LaundryWasherControls::NumberOfRinsesEnum::kNormal);
                    break;
                case 2:
                    Clusters::LaundryWasherControls::Attributes::NumberOfRinses::Set(1, Clusters::LaundryWasherControls::NumberOfRinsesEnum::kExtra);
                    break;
                case 3:
                    Clusters::LaundryWasherControls::Attributes::NumberOfRinses::Set(1, Clusters::LaundryWasherControls::NumberOfRinsesEnum::kMax);
                    break;
            }
        }
        break;
    case AppEvent::kEventType_Downlink_LW_Mode:
        {
            ChipLogProgress(DeviceLayer, "Change Mode to 0x%x", event->value._u8);
            washer.setCurrentMode(event->value._u8);
            Clusters::ModeSelect::Attributes::CurrentMode::Set(1, event->value._u8);
        }
        break;
    case AppEvent::kEventType_Downlink_Refrigerator_Mode:
        {
            modeChangedResponse.status = to_underlying(refrigeratorObject.UpdateCurrentMode(event->value._u16));
            if (modeChangedResponse.status != to_underlying(Clusters::ModeBase::StatusCode::kSuccess))
            {
                ChipLogProgress(DeviceLayer, "Failed to set refrigerator mode!\n");
            }
        }
        break;
    case AppEvent::kEventType_Downlink_Refrigerator_Alarm_State:
        {
            BitMask<AlarmMap> value;
            value.SetField(AlarmMap::kDoorOpen, event->value._u8);
            ChipLogProgress(DeviceLayer, "Set Refrigerator Alarm State Value 0x%x", event->value._u8);
            alarmChangedStatus = refrigeratorAlarmObject.SetStateValue(1, value);
            if (alarmChangedStatus != EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogProgress(DeviceLayer, "Failed to set door status!\n");
            }
        }
        break;
    default:
        break;
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}