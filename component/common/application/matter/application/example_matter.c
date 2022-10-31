#include "FreeRTOS.h"
#include "task.h"
#include "platform/platform_stdlib.h"
#include "basic_types.h"
#include "platform_opts.h"
#include "section_config.h"
#include "wifi_constants.h"
#include "wifi/wifi_conf.h"

#ifdef CHIP_PROJECT

extern void ChipTest(void);

static void example_matter_task_thread(void *pvParameters)
{
    //vTaskDelay(5000);

    while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
        //waiting for Wifi to be initialized
    }

    ChipTest();

    vTaskDelete(NULL);
    return;
}

void example_matter_task(void)
{
    if(xTaskCreate(example_matter_task_thread, ((const char*)"example_matter_task_thread"), 1024, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        printf("\n\r%s xTaskCreate(example_matter_task_thread) failed", __FUNCTION__);
}

#endif /* CHIP_PROJECT */
