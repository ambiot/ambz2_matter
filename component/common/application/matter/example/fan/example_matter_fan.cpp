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

#include "matter_fs.h"
#include "ameba_logging_faultlog.h"
#include "ameba_logging_redirect_handler.h"

#if defined(CONFIG_EXAMPLE_MATTER_FAN) && CONFIG_EXAMPLE_MATTER_FAN
static void example_matter_fan_task(void *pvParameters)
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

    ChipLogProgress(DeviceLayer, "Fan Example!\n");

    CHIP_ERROR err = CHIP_NO_ERROR;

    initPref();     // init NVS
    
    // register log redirection
    auto & instance = AmebaLogRedirectHandler::GetInstance();
    instance.InitAmebaLogSubsystem();
    //
    err = matter_core_start();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_core_start failed!\n");

    err = matter_driver_fan_init();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_fan_init failed!\n");

    err = matter_driver_fan_set_startup_value();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_fan_set_startup_value failed!\n");

    err = matter_interaction_start_uplink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_uplink failed!\n");

    vTaskDelete(NULL);
}

extern "C" void example_matter_fan(void)
{
    if(xTaskCreate(example_matter_fan_task, ((const char*)"example_matter_fan_task"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        ChipLogProgress(DeviceLayer, "\n\r%s xTaskCreate(example_matter_fan) failed", __FUNCTION__);
}
#endif /* CONFIG_EXAMPLE_MATTER_FAN */
