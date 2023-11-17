#include "FreeRTOS.h"
#include "task.h"
#include "platform/platform_stdlib.h"
#include "basic_types.h"
#include "platform_opts.h"
#include "section_config.h"
#include "wifi_constants.h"
#include "wifi/wifi_conf.h"
#include "chip_porting.h"
#include "matter_core.h"
#include "matter_data_model.h"
#include "matter_data_model_presets.h"
#include "matter_drivers.h"
#include "matter_interaction.h"

#include <app/ConcreteAttributePath.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

#if defined(CONFIG_EXAMPLE_MATTER_CUSTOMIZED) && CONFIG_EXAMPLE_MATTER_CUSTOMIZED

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Platform;
using namespace ::chip::app::Clusters;

// (taken from chip-devices.xml)
#define DEVICE_TYPE_ROOT_NODE       0x0016
#define DEVICE_TYPE_ROOM_AIRCON     0x0072
#define DEVICE_TYPE_LAUNDRY_WASHER  0x0073
#define DEVICE_TYPE_REFRIGERATOR    0x0070

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

EmberAfDeviceType rootNodeDeviceTypes[] = {
    { DEVICE_TYPE_ROOT_NODE, DEVICE_VERSION_DEFAULT },
};

EmberAfDeviceType RoomAirConDeviceTypes[] = {
    { DEVICE_TYPE_ROOM_AIRCON, DEVICE_VERSION_DEFAULT },
};

EmberAfDeviceType LaundryWasherDeviceTypes[] = {
    { DEVICE_TYPE_LAUNDRY_WASHER, DEVICE_VERSION_DEFAULT },
};

EmberAfDeviceType RefrigeratorDeviceTypes[] = {
    { DEVICE_TYPE_REFRIGERATOR, DEVICE_VERSION_DEFAULT },
};

Node &node = Node::getInstance();

static void example_matter_aircon_task(void *pvParameters)
{
    while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
        vTaskDelay(500);
    }

    ChipLogProgress(DeviceLayer, "Room Air-Conditioner Dynamic Endpoint example!\n");

    CHIP_ERROR err = CHIP_NO_ERROR;

    initPref();     // init NVS
    //
    err = matter_core_start();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_core_start failed!\n");

    err = matter_driver_fan_init();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_fan_init failed!\n");

    err = matter_driver_temphumsensor_init();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_temphumsensor_init failed!\n");

    EndpointConfig rootNodeEndpointConfig;
    EndpointConfig roomAirconEndpointConfig;
    Presets::Endpoints::matter_root_node_preset(&rootNodeEndpointConfig);
    Presets::Endpoints::matter_room_air_conditioner_preset(&roomAirconEndpointConfig);

    // Initial, root node on ep0, dimmable light on ep1
    node.addEndpoint(rootNodeEndpointConfig, Span<const EmberAfDeviceType>(rootNodeDeviceTypes));
    node.addEndpoint(roomAirconEndpointConfig, Span<const EmberAfDeviceType>(RoomAirConDeviceTypes));

    // Enable endpoints
    node.enableAllEndpoints();

    err = matter_driver_fan_set_startup_value();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_fan_set_startup_value failed!\n");

    err = matter_driver_temphumsensor_start();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_temphumsensor_start failed!\n");

    err = matter_interaction_start_uplink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_uplink failed!\n");

    vTaskDelete(NULL);
}

extern "C" void example_matter_aircon_dm(void)
{
    if(xTaskCreate(example_matter_aircon_task, ((const char*)"example_matter_task_thread"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        ChipLogProgress(DeviceLayer, "\n\r%s xTaskCreate(example_matter_aircon) failed", __FUNCTION__);
}

static void example_matter_laundrywasher_task(void *pvParameters)
{
    while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
        vTaskDelay(500);
    }

    ChipLogProgress(DeviceLayer, "Laundry Washer Dynamic Endpoint example!\n");

    CHIP_ERROR err = CHIP_NO_ERROR;

    initPref();     // init NVS
    //
    err = matter_core_start();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_core_start failed!\n");

    err = matter_driver_laundry_washer_init();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_laundry_washer_init failed!\n");

    EndpointConfig rootNodeEndpointConfig;
    EndpointConfig laundrywasherEndpointConfig;
    Presets::Endpoints::matter_root_node_preset(&rootNodeEndpointConfig);
    Presets::Endpoints::matter_laundrywasher_preset(&laundrywasherEndpointConfig);

    // Initial, root node on ep0, dimmable light on ep1
    node.addEndpoint(rootNodeEndpointConfig, Span<const EmberAfDeviceType>(rootNodeDeviceTypes));
    node.addEndpoint(laundrywasherEndpointConfig, Span<const EmberAfDeviceType>(LaundryWasherDeviceTypes));

    // Enable endpoints
    node.enableAllEndpoints();

    err = matter_driver_laundry_washer_set_startup_value();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_laundry_washer_set_startup_value failed!\n");

    err = matter_interaction_start_downlink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_downlink failed!\n");

    err = matter_interaction_start_uplink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_uplink failed!\n");

    vTaskDelete(NULL);
}

extern "C" void example_matter_laundrywasher_dm(void)
{
    if(xTaskCreate(example_matter_laundrywasher_task, ((const char*)"example_matter_laundrywasher_task"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        ChipLogProgress(DeviceLayer, "\n\r%s xTaskCreate(example_matter_laundrywasher) failed", __FUNCTION__);
}

static void example_matter_refrigerator_task(void *pvParameters)
{
    while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
        vTaskDelay(500);
    }

    ChipLogProgress(DeviceLayer, "Refrigerator Dynamic Endpoint example!\n");

    CHIP_ERROR err = CHIP_NO_ERROR;

    initPref();     // init NVS

    err = matter_core_start();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_core_start failed!\n");

    err = matter_driver_refrigerator_init();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_refrigerator_init failed!\n");

    EndpointConfig rootNodeEndpointConfig;
    EndpointConfig refrigeratorEndpointConfig;
    Presets::Endpoints::matter_root_node_preset(&rootNodeEndpointConfig);
    Presets::Endpoints::matter_refrigerator_preset(&refrigeratorEndpointConfig);

    // Initial, root node on ep0, dimmable light on ep1
    node.addEndpoint(rootNodeEndpointConfig, Span<const EmberAfDeviceType>(rootNodeDeviceTypes));
    node.addEndpoint(refrigeratorEndpointConfig, Span<const EmberAfDeviceType>(RefrigeratorDeviceTypes));

    // Enable endpoints
    node.enableAllEndpoints();

    err = matter_driver_refrigerator_set_startup_value();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_refrigerator_set_startup_value failed!\n");

    err = matter_interaction_start_downlink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_downlink failed!\n");

    err = matter_interaction_start_uplink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_uplink failed!\n");

    vTaskDelete(NULL);
}

extern "C" void example_matter_refrigerator_dm(void)
{
    if(xTaskCreate(example_matter_refrigerator_task, ((const char*)"example_matter_task_thread"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        ChipLogProgress(DeviceLayer, "\n\r%s xTaskCreate(example_matter_refrigerator) failed", __FUNCTION__);
}
#endif /* CONFIG_EXAMPLE_MATTER_CUSTOMIZED */