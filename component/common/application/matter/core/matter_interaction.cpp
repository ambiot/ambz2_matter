#include "matter_drivers.h"
#include "matter_events.h"
#include "matter_interaction.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>

using namespace ::chip;
using namespace ::chip::app;

QueueHandle_t UplinkEventQueue;
QueueHandle_t DownlinkEventQueue;
TaskHandle_t UplinkTaskHandle;
TaskHandle_t DownlinkTaskHandle;

void PostDownlinkEvent(const AppEvent * aEvent)
{
    if (DownlinkEventQueue != NULL)
    {
        BaseType_t status;

        // Event is posted in ISR, use ISR api
        BaseType_t higherPrioTaskWoken = pdFALSE;
        status                         = xQueueSendFromISR(DownlinkEventQueue, aEvent, &higherPrioTaskWoken);

        if (!status)
            ChipLogError(DeviceLayer, "Failed to post downlink event to downlink event queue with");
    }
    else
    {
        ChipLogError(DeviceLayer, "Downlink Event Queue is NULL should never happen");
    }
}

void DispatchDownlinkEvent(AppEvent * aEvent)
{
    if (aEvent->mHandler)
    {
        aEvent->mHandler(aEvent);
    }
    else
    {
        ChipLogError(DeviceLayer, "Downlink event received with no handler. Dropping event.");
    }
}

void DownlinkTask(void * pvParameter)
{
    AppEvent event;

    ChipLogProgress(DeviceLayer, "Downlink Task started");

    // Loop here and keep listening on the queue for Downlink (Firmware application to matter)
    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(DownlinkEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            DispatchDownlinkEvent(&event);
            eventReceived = xQueueReceive(DownlinkEventQueue, &event, 0); // return immediately if the queue is empty
        }
    }
}

CHIP_ERROR matter_interaction_start_downlink()
{
    DownlinkEventQueue = xQueueCreate(10, sizeof(AppEvent));
    if (DownlinkEventQueue == NULL)
    {
        ChipLogError(DeviceLayer, "Failed to allocate downlink event queue");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Start Downlink task.
    BaseType_t xReturned;
    xReturned = xTaskCreate(DownlinkTask, "Downlink", 1024, NULL, 1, &DownlinkTaskHandle);

    return (xReturned == pdPASS) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

void PostUplinkEvent(const AppEvent * aEvent)
{
    if (UplinkEventQueue != NULL)
    {
        BaseType_t status;
        status = xQueueSend(UplinkEventQueue, aEvent, 1);

        if (!status)
            ChipLogError(DeviceLayer, "Failed to post uplink event to uplink event queue");
    }
    else
    {
        ChipLogError(DeviceLayer, "Uplink Event Queue is NULL should never happen");
    }
}

void DispatchUplinkEvent(AppEvent * aEvent)
{
    if (aEvent->mHandler)
    {
        aEvent->mHandler(aEvent);
    }
    else
    {
        ChipLogError(DeviceLayer, "Uplink event received with no handler. Dropping event.");
    }
}

void UplinkTask(void * pvParameter)
{
    AppEvent event;

    ChipLogProgress(DeviceLayer, "Uplink Task started");

    // Loop here and keep listening on the queue for Uplink (matter to Firmware application)
    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(UplinkEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            DispatchUplinkEvent(&event);
            eventReceived = xQueueReceive(UplinkEventQueue, &event, 0); // return immediately if the queue is empty
        }
    }
}

CHIP_ERROR matter_interaction_start_uplink()
{
    UplinkEventQueue = xQueueCreate(10, sizeof(AppEvent));
    if (UplinkEventQueue == NULL)
    {
        ChipLogError(DeviceLayer, "Failed to allocate uplink event queue");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Start Uplink task.
    BaseType_t xReturned;
    xReturned = xTaskCreate(UplinkTask, "Uplink", 1024, NULL, 1, &UplinkTaskHandle);
    return (xReturned == pdPASS) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & path, uint8_t type, uint16_t size, uint8_t * value)
{
    AppEvent uplink_event;
    uplink_event.Type = AppEvent::kEventType_Uplink;
    uplink_event.value = *value;
    uplink_event.path = path;

    switch (path.mClusterId)
    {
    case ZCL_ON_OFF_CLUSTER_ID:
        uplink_event.mHandler = matter_driver_attribute_update;
        PostUplinkEvent(&uplink_event);
        break;

    case ZCL_LEVEL_CONTROL_CLUSTER_ID:
        uplink_event.mHandler = matter_driver_attribute_update;
        PostUplinkEvent(&uplink_event);
        break;

    case ZCL_IDENTIFY_CLUSTER_ID:
        uplink_event.mHandler = matter_driver_attribute_update;
        PostUplinkEvent(&uplink_event);
        break;

    default:
        uplink_event.mHandler = NULL;
        break;
    }
}

void matter_interaction_update_cluster(AppEvent * event)
{
    switch (event->Type)
    {
        case AppEvent::kEventType_Downlink_OnOff:
            ChipLogProgress(DeviceLayer, "Writing to OnOff cluster");
            // write the new on/off value
            // TODO: we only support endpoint1
            EmberAfStatus status = Clusters::OnOff::Attributes::OnOff::Set(1, matter_driver_led_get_onoff());

            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogError(DeviceLayer, "Updating on/off cluster failed: %x", status);
            }

            ChipLogError(DeviceLayer, "Writing to Current Level cluster");
            // write the new currentlevel value
            // TODO: we only support endpoint1
            status = Clusters::LevelControl::Attributes::CurrentLevel::Set(1, matter_driver_led_get_level());

            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                ChipLogError(DeviceLayer, "Updating level cluster failed: %x", status);
            }
            break;

    // TODO: Add more attribute changes
    }
}

void matter_interaction_onoff_handler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Downlink_OnOff)
    {
        ChipLogError(DeviceLayer, "Wrong downlink event handler, should not happen!");
        return;
    }

    matter_driver_led_toggle();
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    matter_interaction_update_cluster(aEvent);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}
