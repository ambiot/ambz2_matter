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

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Platform;
using namespace ::chip::app::Clusters;

#define MATTER_MAX_BRIDGED_DEVICES  20
#define MATTER_MAX_CLUSTERS_PER_ENDPOINT 20

// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGED_NODE 0x0013
// (taken from lo-devices.xml)
#define DEVICE_TYPE_LO_ON_OFF_LIGHT 0x0100
// (taken from chip-devices.xml)
#define DEVICE_TYPE_ROOT_NODE 0x0016
// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

Node& node = Node::getInstance();

// TODO: maybe this dataversion can be a member of Endpoint class
// lifetime of this dataversion should be same as the Endpoint class
DataVersion dataVersions[MATTER_MAX_BRIDGED_DEVICES][MATTER_MAX_CLUSTERS_PER_ENDPOINT];
EmberAfDeviceType deviceTypes[] = {
    // fill up with all the possible device types that you want to bridge
    // TODO: Should this be dynamic as well?
    { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT },
    { DEVICE_TYPE_LO_ON_OFF_LIGHT, DEVICE_VERSION_DEFAULT },
};

const EmberAfDeviceType gBridgedOnOffDeviceTypes[] = { { DEVICE_TYPE_LO_ON_OFF_LIGHT, DEVICE_VERSION_DEFAULT },
                                                       { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };

static void example_matter_light_task(void *pvParameters)
{
    while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
        vTaskDelay(500);
    }

    ChipLogProgress(DeviceLayer, "Lighting example!\n");

    CHIP_ERROR err = CHIP_NO_ERROR;

    initPref();     // init NVS
    //
    err = matter_core_start();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_core_start failed!\n");

    EndpointConfig rootNodeEndpointConfig;
    Presets::Endpoints::matter_root_node_preset(&rootNodeEndpointConfig);

    // AttributeConfig attributeConfig1(1, ZCL_INT8U_ATTRIBUTE_TYPE, std::uint8_t(10), 1 ,0);

    // EventConfig eventConfig1(1);

    // CommandConfig commandConfig1(1, COMMAND_MASK_ACCEPTED);

    // ClusterConfig clusterConfig1;
    // clusterConfig1.clusterId = 1;
    // clusterConfig1.attributeConfigs.push_back(attributeConfig1);
    // clusterConfig1.eventConfigs.push_back(eventConfig1);
    // clusterConfig1.commandConfigs.push_back(commandConfig1);

    // EndpointConfig endpointConfig1;
    // endpointConfig1.clusterConfigs.push_back(clusterConfig1);

    // Initial
    // node.addEndpoint(endpointConfig1);
    node.addEndpoint(rootNodeEndpointConfig);

    node.getEndpoint(0)->enableEndpoint(Span<const EmberAfDeviceType>(deviceTypes));
    node.print();
    node.getEndpoint(0)->disableEndpoint();
    node.print();

    vTaskDelete(NULL);
}

extern "C" void example_matter_light(void)
{
    if(xTaskCreate(example_matter_light_task, ((const char*)"example_matter_task_thread"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        ChipLogProgress(DeviceLayer, "\n\r%s xTaskCreate(example_matter_light) failed", __FUNCTION__);
}
