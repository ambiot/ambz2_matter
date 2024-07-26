#include "matter_drivers.h"
#include "matter_interaction.h"
#include "gpio_irq_api.h"
#include "gpio_irq_ex_api.h"
#include "us_ticker_api.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <protocols/interaction_model/StatusCode.h>

#include "matter_switch_cluster/matter_switch_server.h"
#include "matter_switch_cluster/switch_driver.h"

using namespace Ameba::Clusters::GenericSwitch::Event;
using namespace ::chip::app;
using chip::Protocols::InteractionModel::Status;

#define GPIO_IRQ_PIN    PA_19

MatterSwitch swtch;
gpio_irq_t gpio_btn;
volatile char irq_rise;
static unsigned int rise_time;
static unsigned int fall_time;
uint32_t pressStartTime = 0;        // Time when the switch was pressed

bool isShortRelease = false;        // Flag to indicate short release
bool isLongRelease = false;         // Flag to indicate long release

void matter_driver_switch_callback(uint32_t value, gpio_irq_event event)
{
    AppEvent downlink_event;
    uint32_t pressDuration = 0;
    if (irq_rise)
    {
        rise_time = us_ticker_read();
        gpio_irq_set_event(&gpio_btn, IRQ_FALL);
        irq_rise = 0;

        pressStartTime = rise_time;

        isShortRelease = false;
        isLongRelease = false;

        downlink_event.Type = AppEvent::kEventType_Downlink_SwitchInitialPress;
        downlink_event.value._u8 = (uint8_t) 1; //position
        downlink_event.path.mEndpointId = 1;
        downlink_event.mHandler = matter_driver_downlink_update_handler;
        PostDownlinkEvent(&downlink_event);

    }
    else
    {
        fall_time = us_ticker_read();
        gpio_irq_set_event(&gpio_btn, IRQ_RISE);
        irq_rise = 1;

        pressDuration = fall_time - rise_time;
        pressStartTime = 0;

        if (pressDuration < 1000000)
        {
            downlink_event.Type = AppEvent::kEventType_Downlink_SwitchShortRelease;
            downlink_event.value._u8 = 0; // 1: Pressed, 0: Release
            downlink_event.path.mEndpointId = 0;
            downlink_event.mHandler = matter_driver_downlink_update_handler;
            PostDownlinkEvent(&downlink_event);
        }
        else
        {
            downlink_event.Type = AppEvent::kEventType_Downlink_SwitchLongRelease;
            downlink_event.value._u8 = 0; // 1: Pressed, 0: Release
            downlink_event.path.mEndpointId = 1;
            downlink_event.mHandler = matter_driver_downlink_update_handler;
            PostDownlinkEvent(&downlink_event);
        }
    }

    return;
}

static void handleSwitch(void *pvParameters)
{
    AppEvent downlink_event;
    uint32_t pressDuration = 0;

    while (1)
    {
        pressDuration = us_ticker_read() - pressStartTime;
        if (gpio_irq_read(&gpio_btn) && pressDuration > 2000000)
        {
            downlink_event.Type = AppEvent::kEventType_Downlink_SwitchLongPress;
            downlink_event.value._u8 = 1;
            downlink_event.path.mEndpointId = 0;
            downlink_event.mHandler = matter_driver_downlink_update_handler;
            PostDownlinkEvent(&downlink_event);
        }
        vTaskDelay(1000);
    }
}

CHIP_ERROR matter_driver_switch_init(void)
{
    gpio_irq_init(&gpio_btn, GPIO_IRQ_PIN, matter_driver_switch_callback, 1);
    gpio_irq_set(&gpio_btn, IRQ_RISE, 1);   // Falling Edge Trigger
    irq_rise = 1;
    gpio_irq_pull_ctrl(&gpio_btn, PullNone);
    gpio_irq_debounce_set(&gpio_btn, 10000, 1);
    gpio_irq_enable(&gpio_btn);

    if (xTaskCreate(handleSwitch, ((const char*)"handleSwitch"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        ChipLogProgress(DeviceLayer, "xTaskCreate(handleSwitch) failed");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR matter_driver_switch_set_startup_value(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::EndpointId ep = 1;
    Status status;
    uint8_t NumberOfPositionsValue, CurrentPositionValue, MultiPressMaxValue;

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    ChipLogProgress(DeviceLayer, "Initialize Switch");

    // Read NumberOfPositions
    status = Clusters::Switch::Attributes::NumberOfPositions::Get(ep, &NumberOfPositionsValue);
    VerifyOrExit(status == Status::Success, err = CHIP_ERROR_INTERNAL);

    // Read CurrentPosition
    status = Clusters::Switch::Attributes::CurrentPosition::Get(ep, &CurrentPositionValue);
    VerifyOrExit(status == Status::Success, err = CHIP_ERROR_INTERNAL);

    // Read MultiPressMax
    status = Clusters::Switch::Attributes::MultiPressMax::Get(ep, &MultiPressMaxValue);
    VerifyOrExit(status == Status::Success, err = CHIP_ERROR_INTERNAL);

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    swtch.SetNumberOfPosition(NumberOfPositionsValue);
    swtch.SetCurrentPosition(CurrentPositionValue);
    swtch.SetMultiPressMax(MultiPressMaxValue);

exit:
    if (err == CHIP_ERROR_INTERNAL)
    {
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    return err;
}

/* Set identify cluster and its callback on ep1 */

static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, matter_driver_on_identify_start, matter_driver_on_identify_stop, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, matter_driver_on_trigger_effect,
};

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

void matter_driver_uplink_update_handler(AppEvent *aEvent)
{
    chip::app::ConcreteAttributePath path = aEvent->path;

    //VerifyOrExit(aEvent->path.mEndpointId == 1, ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", path.mEndpointId));

    ChipLogProgress(DeviceLayer, "Uplink: ClusterId:0x%x, AttributeId:0x%x value:%d", path.mClusterId,
                                    path.mAttributeId, aEvent->value._u8);

    switch (path.mClusterId)
    {
    case Clusters::Identify::Id:
        break;
    case Clusters::Switch::Id:
        {
            if (path.mAttributeId == Clusters::Switch::Attributes::NumberOfPositions::Id)
            {
                swtch.SetNumberOfPosition(aEvent->value._u8);
            }
            if (path.mAttributeId == Clusters::Switch::Attributes::CurrentPosition::Id)
            {
                swtch.SetCurrentPosition(aEvent->value._u8);
            }
            if (path.mAttributeId == Clusters::Switch::Attributes::MultiPressMax::Id)
            {
                swtch.SetMultiPressMax(aEvent->value._u8);
            }
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
    chip::app::ConcreteAttributePath path = event->path;
    Ameba::Clusters::GenericSwitch::Event::GenericSwitchEventHandler handler;

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    switch (event->Type)
    {
    case AppEvent::kEventType_Downlink_SwitchInitialPress:
        {
            GenericSwitchEventHandler::SwitchInitialPress switchInitialPressInstance(&handler);
            switchInitialPressInstance.Set(path.mEndpointId, event->value._u8);
            swtch.SetCurrentPosition(event->value._u8);
        }
        break;
    case AppEvent::kEventType_Downlink_SwitchLongPress:
        {
            GenericSwitchEventHandler::SwitchLongPress switchLongPressInstance(&handler);
            switchLongPressInstance.Set(path.mEndpointId, event->value._u8);
            swtch.SetCurrentPosition(event->value._u8);
        }
        break;
    case AppEvent::kEventType_Downlink_SwitchShortRelease:
        {
            GenericSwitchEventHandler::SwitchShortRelease switchShortPressInstance(&handler);
            switchShortPressInstance.Set(path.mEndpointId, event->value._u8);
            swtch.SetCurrentPosition(event->value._u8);
        }
        break;
    case AppEvent::kEventType_Downlink_SwitchLongRelease:
        {
            GenericSwitchEventHandler::SwitchLongRelease switchLongReleaseInstance(&handler);
            switchLongReleaseInstance.Set(path.mEndpointId, event->value._u8);
            swtch.SetCurrentPosition(event->value._u8);
        }
        break;
    default:
        break;
    }

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}
