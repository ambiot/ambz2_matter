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
#include "matter_drivers.h"
#include "matter_interaction.h"

#if defined(CONFIG_EXAMPLE_MATTER_REFRIGERATOR) && CONFIG_EXAMPLE_MATTER_REFRIGERATOR

static void example_matter_refrigerator_task(void *pvParameters)
{
    while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
        vTaskDelay(500);
    }

    ChipLogProgress(DeviceLayer, "Refrigerator example!\n");

    CHIP_ERROR err = CHIP_NO_ERROR;

    initPref();     // init NVS
    
    err = matter_core_start();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_core_start failed!\n");

    err = matter_driver_refrigerator_init();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_refrigerator_init failed!\n");

    err = matter_driver_refrigerator_set_startup_value();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_refrigerator_set_startup_value failed!\n");

    err = matter_interaction_start_downlink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_downlink failed!\n");

    err = matter_interaction_start_uplink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_uplink failed!\n");

    vTaskDelay(15000);
    ChipLogProgress(DeviceLayer, "Set Refrigerator Mode to 1\n");
    matter_driver_set_mode_callback(1);

    vTaskDelay(15000);
    ChipLogProgress(DeviceLayer, "Set Refrigerator Mode to 0\n");
    matter_driver_set_mode_callback(0);

    vTaskDelete(NULL);
}

extern "C" void example_matter_refrigerator(void)
{
    if(xTaskCreate(example_matter_refrigerator_task, ((const char*)"example_matter_task_thread"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        ChipLogProgress(DeviceLayer, "\n\r%s xTaskCreate(example_matter_refrigerator) failed", __FUNCTION__);
}

#endif /* CONFIG_EXAMPLE_MATTER_REFRIGERATOR */