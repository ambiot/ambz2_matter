#include "FreeRTOS.h"
#include "task.h"
#include "platform/platform_stdlib.h"
#include "basic_types.h"
#include "platform_opts.h"
#include "section_config.h"
#include "wifi_constants.h"
#include "wifi/wifi_conf.h"

#include "matter_fs.h"
#include "ameba_logging_faultlog.h"
#include "ameba_logging_redirect_wrapper.h"

#if defined(CONFIG_EXAMPLE_MATTER_CHIPTEST) && CONFIG_EXAMPLE_MATTER_CHIPTEST
extern void ChipTest(void);

static void example_matter_task_thread(void *pvParameters)
{
    //vTaskDelay(5000);

    while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
        //waiting for Wifi to be initialized
    }

    int res = matter_fs_init();

    /* init flash fs and read existing fault log into fs */
    if(res == 0)
    {
        printf("\n** Matter FlashFS ready! **\n");
        read_last_fault_log();
    }

    matter_timer_init();

    // register log redirection: C wrapper version
    ameba_logging_redirect_wrapper_init();

    ChipTest();

    vTaskDelete(NULL);
    return;
}

void example_matter_task(void)
{
    if(xTaskCreate(example_matter_task_thread, ((const char*)"example_matter_task_thread"), 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        printf("\n\r%s xTaskCreate(example_matter_task_thread) failed", __FUNCTION__);
}
#endif /* CONFIG_EXAMPLE_MATTER_CHIPTEST */
