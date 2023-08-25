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

#if defined(CONFIG_EXAMPLE_MATTER_LIGHT) && CONFIG_EXAMPLE_MATTER_LIGHT

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Platform;
using namespace ::chip::app::Clusters;

// (taken from chip-devices.xml)
#define DEVICE_TYPE_ROOT_NODE 0x0016
#define DEVICE_TYPE_LO_ON_OFF_LIGHT 0x0100

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

EmberAfDeviceType rootNodeDeviceTypes[] = {
    { DEVICE_TYPE_ROOT_NODE, DEVICE_VERSION_DEFAULT },
};

EmberAfDeviceType dimmableLightDeviceTypes[] = {
    { DEVICE_TYPE_LO_ON_OFF_LIGHT, DEVICE_VERSION_DEFAULT },
};

Node &node = Node::getInstance();

static void example_matter_light_task(void *pvParameters)
{
    while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
        vTaskDelay(500);
    }

    ChipLogProgress(DeviceLayer, "Lighting example!\n");

    CHIP_ERROR err = CHIP_NO_ERROR;

    initPref();     // init NVS

    err = matter_core_start();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "matter_core_start failed!\n");
    }

    EndpointConfig rootNodeEndpointConfig;
    EndpointConfig dimmableLightEndpointConfig;
    Presets::Endpoints::matter_root_node_preset(&rootNodeEndpointConfig);
    Presets::Endpoints::matter_dimmable_light_preset(&dimmableLightEndpointConfig);

    // Initial, root node on ep0, dimmable light on ep1
    node.addEndpoint(rootNodeEndpointConfig, Span<const EmberAfDeviceType>(rootNodeDeviceTypes));
    node.addEndpoint(dimmableLightEndpointConfig, Span<const EmberAfDeviceType>(dimmableLightDeviceTypes));

    // Enable endpoints
    node.enableAllEndpoints();

    vTaskDelay(20000);

    // Test add another dimmable light on ep2
    chip::EndpointId testEndpointId;
    testEndpointId = node.addEndpoint(dimmableLightEndpointConfig, Span<const EmberAfDeviceType>(dimmableLightDeviceTypes));
    node.enableAllEndpoints();

    vTaskDelay(20000);

    // Test remove ep2
    node.getEndpoint(testEndpointId)->disableEndpoint();
    node.removeEndpoint(testEndpointId);

    vTaskDelete(NULL);
}

extern "C" void example_matter_light(void)
{
    if(xTaskCreate(example_matter_light_task, ((const char*)"example_matter_task_thread"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        ChipLogProgress(DeviceLayer, "\n\r%s xTaskCreate(example_matter_light) failed", __FUNCTION__);
}

#endif /* CONFIG_EXAMPLE_MATTER_LIGHT */
