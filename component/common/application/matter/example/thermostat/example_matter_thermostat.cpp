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

#if defined(CONFIG_EXAMPLE_MATTER_THERMOSTAT) && CONFIG_EXAMPLE_MATTER_THERMOSTAT
static void example_matter_thermostat_task(void *pvParameters)
{
    while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
        vTaskDelay(500);
    }

    ChipLogProgress(DeviceLayer, "Thermostat example!\n");

    CHIP_ERROR err = CHIP_NO_ERROR;

    initPref();     // init NVS
    //
    err = matter_core_start();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_core_start failed!\n");

    err = matter_driver_thermostat_init();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_thermostat_init failed!\n");

    err = matter_driver_thermostat_ui_init();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_thermostat_ui_init failed!\n");

    err = matter_driver_thermostat_ui_set_startup_value();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_thermostat_ui_set_startup_value failed!\n");

    err = matter_interaction_start_downlink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_downlink failed!\n");

    err = matter_interaction_start_uplink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_uplink failed!\n");

    vTaskDelete(NULL);
}

extern "C" void example_matter_thermostat(void)
{
    if(xTaskCreate(example_matter_thermostat_task, ((const char*)"example_matter_task_thread"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        ChipLogProgress(DeviceLayer, "\n\r%s xTaskCreate(example_matter_thermostat) failed", __FUNCTION__);
}
#endif /* CONFIG_EXAMPLE_MATTER_THERMOSTAT */
