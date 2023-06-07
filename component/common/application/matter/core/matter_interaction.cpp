#include "matter_drivers.h"
#include "matter_events.h"
#include "matter_interaction.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

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
        BaseType_t eventReceived = xQueueReceive(DownlinkEventQueue, &event, portMAX_DELAY);
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
        BaseType_t eventReceived = xQueueReceive(UplinkEventQueue, &event, portMAX_DELAY);
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
    uplink_event.path = path;

    if (size == 1)
    {
        uplink_event.value._u8 = *value;
    }
    else if (size == 2)
    {
        memcpy(&uplink_event.value._u16, value, size);
    }
    else if (size <= 4)
    {
        memcpy(&uplink_event.value._u32, value, size);
    }
    else if (size <= 8)
    {
        memcpy(&uplink_event.value._u64, value, size);
    }
    else if (size <= 256) // TODO: check max attribute length
    {
        memcpy(&uplink_event.value._str, value, size);
    }
    else
    {
        ChipLogError(DeviceLayer, "Data size is too large to put into the event, please increase the value buffer size");
        return;
    }

    uplink_event.mHandler = matter_driver_uplink_update_handler;
    PostUplinkEvent(&uplink_event);
}
