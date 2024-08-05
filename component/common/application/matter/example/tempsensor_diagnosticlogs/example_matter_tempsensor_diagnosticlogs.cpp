/* standard header includes */
#include "FreeRTOS.h"
#include "ameba_diagnosticlogs_provider_delegate_impl.h"
#include "ff.h"
#include "task.h"

/* example specific */
#include "example_matter_tempsensor_diagnosticlogs.h"

/* ameba specific */
#include "platform_opts.h"
#include "wifi_constants.h"
#include "wifi/wifi_conf.h"
#include "chip_porting.h"
#include "matter_core.h"
#include "matter_drivers.h"
#include "matter_interaction.h"
#include "matter_flashfs.h"
#include "ameba_logging_faultlog.h"

#if defined(CONFIG_EXAMPLE_MATTER_TEMPSENSOR_DIAGNOSTICLOGS) && (CONFIG_EXAMPLE_MATTER_TEMPSENSOR_DIAGNOSTICLOGS == 1)

#include <flash_api.h>
#include <device_lock.h>

#if defined(CONFIG_AMEBA_ENABLE_DIAGNOSTIC_LOGS) && (CONFIG_AMEBA_ENABLE_DIAGNOSTIC_LOGS == 1)
/**
 * reads the last crash log stored in the flash partition and copy it to the FATFS file for extraction
 * rollover the flash partition when next crash happens
*/

static void example_matter_tempsensor_diagnosticlogs_task_thread(void *pvParameters) 
{

#if defined(CONFIG_AMEBA_ENABLE_DIAGNOSTIC_LOGS) && (CONFIG_AMEBA_ENABLE_DIAGNOSTIC_LOGS == 1)
    /* init flash fs and read existing fault log into fs */
    int res = matter_flashfs_init();
    if(res == 0)
    {
        ChipLogProgress(DeviceLayer, "\n** Matter FlashFS ready! **\n");
        read_last_fault_log();
    }
#endif

    /* Task Initialization */
    while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
        //waiting for Wifi to be initialized - give cpu time to other tasks
        vTaskDelay(500);
    }


    ChipLogProgress(DeviceLayer, "Temperature Sensor Example with Diagnostic Logs Server\n");

    CHIP_ERROR err = CHIP_NO_ERROR;

    initPref();     // init NVS

    err = matter_core_start();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_core_start failed!\n");

    // other HAL inits
    err = matter_driver_tempsensor_init();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_tempsensor_init failed!\n");
    
    err = matter_driver_tempsensor_set_startup_value();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_tempsensor_set_startup_value failed!\n");

    err = matter_driver_tempsensor_start();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_driver_tempsensor_start failed!\n");

    err = matter_interaction_start_downlink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_downlink failed!\n");

    err = matter_interaction_start_uplink();
    if (err != CHIP_NO_ERROR)
        ChipLogProgress(DeviceLayer, "matter_interaction_start_uplink failed!\n");

    /* Task Deinit */
    vTaskDelete(NULL);
    return;
}

/* called by entry point */
extern "C" void example_matter_tempsensor_diagnostic_logs(void) {
    /* Create rtos task*/
    if(xTaskCreate(example_matter_tempsensor_diagnosticlogs_task_thread, ((const char*)"example_matter_tempsensor_diagnosticlogs_task_thread"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        printf("\n\r%s xTaskCreate(example_matter_tempsensor_diagnosticlogs_task_thread) failed", __FUNCTION__);

    return;
}

#endif
#endif