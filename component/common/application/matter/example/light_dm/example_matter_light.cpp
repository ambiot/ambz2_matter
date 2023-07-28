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

extern "C" void addLightEndpoint(int count) {
    EndpointConfig dimmableLightEndpointConfig;
    Presets::Endpoints::matter_dimmable_light_preset(&dimmableLightEndpointConfig);
    node.addEndpoint(dimmableLightEndpointConfig);
    node.getEndpoint(count)->enableEndpoint(Span<const EmberAfDeviceType>(deviceTypes));
}

extern "C" void removeLightEndpoint(int count) {
    node.getEndpoint(count)->disableEndpoint();
    node.removeEndpoint(count);
}

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
    EndpointConfig dimmableLightEndpointConfig;
    Presets::Endpoints::matter_root_node_preset(&rootNodeEndpointConfig);
    Presets::Endpoints::matter_dimmable_light_preset(&dimmableLightEndpointConfig);

    // Initial
    node.addEndpoint(rootNodeEndpointConfig);
    node.addEndpoint(dimmableLightEndpointConfig);


    // Enable endpoints
    // TODO: use enable all endpoints?
    node.getEndpoint(0)->enableEndpoint(Span<const EmberAfDeviceType>(deviceTypes));
    node.getEndpoint(1)->enableEndpoint(Span<const EmberAfDeviceType>(deviceTypes));

    // for (auto & clusterConfig : dimmableLightEndpointConfig.clusterConfigs)
    // {
    //     printf("clusterId %d\r\n", clusterConfig.clusterId);
    // }

    Endpoint *ep0 = node.getEndpoint(0);
    // for (auto & cluster : ep0->clusters)
    // {
    //     printf("parent endpointId: %d\r\n", cluster.getParentEndpointId());
    //     printf("clusterId: %d\r\n", cluster.getClusterId());

    //     for (auto & attribute : cluster.attributes)
    //     {
    //         printf("\t\tparent clusterId: %d\r\n", attribute.getParentClusterId());
    //         printf("\t\tattributeId: %d\r\n", attribute.getAttributeId());
    //     }
    // }
    Cluster *cls = ep0->getCluster(0x30);
    Attribute *att = cls->getAttribute(0);
    uint64_t breadcrumb;
    att->getValue((uint8_t*)&breadcrumb);
    printf("breadcrumb: %d\r\n\r\n", breadcrumb);
    uint8_t *ptr = (uint8_t*) breadcrumb;
    printf("*breadcrumb: %d\r\n\r\n", *ptr);

    // for (auto & clusterConfig : rootNodeEndpointConfig.clusterConfigs)
    // {
    //     if (clusterConfig.clusterId == 0x30)
    //     {
    //         for (auto & attributeConfig : clusterConfig.attributeConfigs)
    //         {
    //             if (attributeConfig.attributeId == 0)
    //             {
    //                 printf("attributeConfig breadcrumb: %d\r\n", attributeConfig.value.defaultValue);
    //                 printf("attributeConfig breadcrumb ptr: %d\r\n", attributeConfig.value.ptrToDefaultValue);
    //                 printf("type: 0x%x\r\n", attributeConfig.dataType);
    //             }
    //         }
    //     }
    // }

    // vTaskDelay(100000);
    // node.addEndpoint(dimmableLightEndpointConfig);
    // node.getEndpoint(2)->enableEndpoint(Span<const EmberAfDeviceType>(deviceTypes));

    // vTaskDelay(30000);
    // node.getEndpoint(2)->disableEndpoint();
    // node.removeEndpoint(2);





    // node.print();

    // Disable root node endpoint
    // node.getEndpoint(0)->disableEndpoint();
    // node.print();

    vTaskDelete(NULL);
}

extern "C" void example_matter_light(void)
{
    if(xTaskCreate(example_matter_light_task, ((const char*)"example_matter_task_thread"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        ChipLogProgress(DeviceLayer, "\n\r%s xTaskCreate(example_matter_light) failed", __FUNCTION__);
}