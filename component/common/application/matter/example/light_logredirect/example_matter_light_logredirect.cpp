/* standard header includes */
#include "FreeRTOS.h"
#include "ameba_logging_redirect_handler.h"
#include "task.h"

/* example specific */
#include "example_matter_light_logredirect.h"

/* ameba specific */
#include "platform_opts.h"
#include "chip_porting.h"
#include "matter_core.h"
#include "matter_drivers.h"
#include "matter_interaction.h"
#include "ameba_logging_faultlog.h"

#if defined(CONFIG_EXAMPLE_MATTER_LIGHT_LOGREDIRECT) && (CONFIG_EXAMPLE_MATTER_LIGHT_LOGREDIRECT == 1)

static void report_test_chip_error_loop_task(void *pvParameters) {
    CHIP_ERROR err;
    int count = 0;
    while(1)
    {
        vTaskDelay(10);
        //ChipLogError(Ble, "test error WITH err.Format!!!: %" CHIP_ERROR_FORMAT, err.Format());
        err = CHIP_BLE_ERROR(0x03);
        err.Format();
        if(count >= 500) break;
        
        count++;
    }

    ChipLogProgress(Ble, "=== STOP GENERATING FAKE ERRORS ===");

    /* Task Deinit */
    vTaskDelete(NULL);
    return;
}

extern "C" void ChipDiagLogInsertSector(void)
{
    CHIP_ERROR err;
    int count = 0;
    while(1)
    {
        err = CHIP_BLE_ERROR(0x04);
        err.Format();
        if(count >= 100) break;
        count++;
    }
    ChipLogProgress(Ble, ">inserted logs into buffer");

    return;
}

static void example_matter_light_logredirect_task_thread(void *pvParameters) 
{
    /* init flash fs and read existing fault log into fs */
    int res = matter_flashfs_init();
    if(res == 0)
    {
        ChipLogProgress(DeviceLayer, "\n** Matter FlashFS ready! **\n");
        read_last_fault_log();
    }

    /* sample userlog message */
    write_debug_userlog("@@@@@@@@@@@@@@@@@@@@ This is a test diagnostic log USER message! @@@@@@@@@@@@@@@@@@@@\n");

    /* Task Initialization */
    while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
        //waiting for Wifi to be initialized - give cpu time to other tasks
        vTaskDelay(500);
    }

    /* App Start */
    ChipLogProgress(DeviceLayer, "\nLight Cluster example with Log Redirect\n");
#if 0
    for(int i = 0; i < 10; i++) {
        printf("attach gdb within %ds\n", 10 - i);
        vTaskDelay(1000);
    }
#endif

    CHIP_ERROR err = CHIP_NO_ERROR;
    
    initPref();     // init NVS
    
    // register log redirection
    auto & instance = AmebaLogRedirectHandler::GetInstance();
    instance.RegisterAmebaLogRedirect();

    err = matter_core_start();
    if (err != CHIP_NO_ERROR)
        ChipLogError(DeviceLayer, "matter_core_start failed! %" CHIP_ERROR_FORMAT, err.Format());

    err = matter_driver_led_init();
    if (err != CHIP_NO_ERROR)
        ChipLogError(DeviceLayer, "matter_driver_led_init failed! %" CHIP_ERROR_FORMAT, err.Format());

    err = matter_driver_led_set_startup_value();
    if (err != CHIP_NO_ERROR)
        ChipLogError(DeviceLayer, "matter_driver_led_set_startup_value failed! %" CHIP_ERROR_FORMAT, err.Format());

    err = matter_driver_button_init();
    if (err != CHIP_NO_ERROR)
        ChipLogError(DeviceLayer, "matter_driver_button_init failed! %" CHIP_ERROR_FORMAT, err.Format());
        
    err = matter_interaction_start_downlink();
    if (err != CHIP_NO_ERROR)
        ChipLogError(DeviceLayer, "matter_interaction_start_downlink failed! %" CHIP_ERROR_FORMAT, err.Format());
        
    err = matter_interaction_start_uplink();
    if (err != CHIP_NO_ERROR)
        ChipLogError(DeviceLayer, "matter_interaction_start_uplink failed! %" CHIP_ERROR_FORMAT, err.Format());
        
    //if(xTaskCreate(report_test_chip_error_loop_task, ((const char*)"report_test_chip_error_loop_task"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    //    printf("\n\r%s xTaskCreate(report_test_chip_error_loop_task) failed", __FUNCTION__);

    /* Task Deinit */
    vTaskDelete(NULL);
    return;
}

/* called by entry point */
extern "C" void example_matter_light_logredirect(void) {
    /* Create rtos task*/
    if(xTaskCreate(example_matter_light_logredirect_task_thread, ((const char*)"example_matter_light_logredirect_task_thread"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        printf("\n\r%s xTaskCreate(example_matter_light_logredirect_task_thread) failed", __FUNCTION__);

    return;
}

#endif // CONFIG_EXAMPLE_MATTER_LIGHT_LOGREDIRECT
