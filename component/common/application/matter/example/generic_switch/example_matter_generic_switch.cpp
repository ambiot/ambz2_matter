#include "FreeRTOS.h"
#include "task.h"
#include "platform/platform_stdlib.h"
#include "basic_types.h"
#include "platform_opts.h"
#include "wifi_constants.h"
#include "wifi/wifi_conf.h"

#include "chip_porting.h"
#include "matter_core.h"
#include "matter_drivers.h"
#include "matter_interaction.h"

#if defined(CONFIG_EXAMPLE_MATTER_GENERIC_SWITCH) && CONFIG_EXAMPLE_MATTER_GENERIC_SWITCH

static void example_matter_generic_switch_task(void *pvParameters)
{
    while (!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE)))
    {
        vTaskDelay(500);
    }

    ChipLogProgress(DeviceLayer, "Matter Generic Switch Example!");

    CHIP_ERROR err = CHIP_NO_ERROR;

    initPref();     // init NVS (Non-Volatile Storage)

    err = matter_core_start();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "matter_core_start failed!");
    }

    err = matter_driver_switch_init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "matter_driver_switch_init failed!");
    }

    err = matter_driver_switch_set_startup_value();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "matter_driver_switch_set_startup_value failed!");
    }

    err = matter_interaction_start_downlink();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "matter_interaction_start_downlink failed!");
    }

    err = matter_interaction_start_uplink();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "matter_interaction_start_uplink failed!");
    }

    vTaskDelete(NULL);
}

extern "C" void example_matter_generic_switch(void)
{
    if (xTaskCreate(example_matter_generic_switch_task, ((const char*)"example_matter_generic_switch_task"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        ChipLogProgress(DeviceLayer, "xTaskCreate(example_matter_generic_switch) failed");
    }
}

#endif /* CONFIG_EXAMPLE_MATTER_GENERIC_SWITCH */
