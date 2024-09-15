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
#include "bridge_driver.h"

#include <app/ConcreteAttributePath.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

#include "matter_fs.h"
#include "ameba_logging_faultlog.h"
#include "ameba_logging_redirect_handler.h"

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Platform;
using namespace ::chip::app::Clusters;

MatterBridge bridge;
Node& node = Node::getInstance();

EmberAfDeviceType gBridgedOnOffDeviceTypes[] = {
    { DEVICE_TYPE_LO_ON_OFF_LIGHT, DEVICE_VERSION_DEFAULT },
    { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT },
};

static void example_matter_bridge_task(void *pvParameters)
{
    int res = matter_fs_init();

    /* init flash fs and read existing fault log into fs */
    if(res == 0)
    {
        ChipLogProgress(DeviceLayer, "\n** Matter FlashFS ready! **\n");
        read_last_fault_log();
    }

    while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
        vTaskDelay(500);
    }

    ChipLogProgress(DeviceLayer, "\nBridge Dynamic Endpoint example!\n");

    CHIP_ERROR err = CHIP_NO_ERROR;

    initPref();     // init NVS

    // register log redirection
    auto & instance = AmebaLogRedirectHandler::GetInstance();
    instance.InitAmebaLogSubsystem();

    err = matter_driver_bridge_light_init();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_bridge_light_init failed!\n");

    err = matter_core_start();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_core_start failed!\n");

    err = matter_interaction_start_downlink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_downlink failed!\n");

    err = matter_interaction_start_uplink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_uplink failed!\n");

    vTaskDelay(50);

    bridge.Init(node);

    EndpointConfig bridgedonoffEndpointConfig;
    Presets::Endpoints::matter_dimmable_light_preset(&bridgedonoffEndpointConfig);
    bridge.addBridgedEndpoint(bridgedonoffEndpointConfig, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes));

    if(xTaskCreate(matter_customer_bridge_code, ((const char*)"matter_customer_bridge_code"), 1024, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        printf("\n\r%s xTaskCreate(matter_customer_bridge_code) failed\n", __FUNCTION__);

    vTaskDelay(20000);

    bridge.removeBridgedEndpoint(2);

    vTaskDelete(NULL);
}

extern "C" void example_matter_bridge(void)
{
    if(xTaskCreate(example_matter_bridge_task, ((const char*)"example_matter_task_thread"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        ChipLogProgress(DeviceLayer, "\n\r%s xTaskCreate(example_matter_light) failed", __FUNCTION__);
}
