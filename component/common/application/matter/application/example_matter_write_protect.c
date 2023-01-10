#include "FreeRTOS.h"
#include "task.h"
#include "platform/platform_stdlib.h"
#include "example_matter_write_protect.h"
#include "flash_api.h"

//  This exmaple shows you how to add/remove write protection to the last 4KB region of the external flash
//  Flash used: Winbond W25Q32JV (4MB)
//  Region protected: 0x3FF000 - 0x3FFFFF
//  Note: Please check your flash data sheet for instructions on how to set the status registers
//  Note: Run this example only after you have flashed the matter factory data binary
//  Note: To add write protection, enable LOCK_FACTORY_DATA
//  Note: To remove write protection, disable LOCK_FACTORY_DATA

#define LOCK_FACTORY_DATA 1

static void example_matter_write_protect_task_thread(void *pvParameters)
{
    printf("\r\nMatter Flash Write Protection\r\n");
    flash_t flash;
    uint32_t address = 0x3FF000;
    uint32_t length = 4096;
    
#if LOCK_FACTORY_DATA
    printf("Status Register Before Setting = %x \r\n", flash_get_status(&flash));
    flash_set_status(&flash, 0x44); // Lock factory data
    printf("Status Register After Setting = %x \r\n", flash_get_status(&flash));
#else
    printf("Status Register Before Setting = %x \r\n", flash_get_status(&flash));
    flash_set_status(&flash, flash_get_status(&flash) & (~0x44));    // Unlock factory data
    printf("Status Register After Setting = %x \r\n", flash_get_status(&flash));
#endif //LOCK_FACTORY_DATA

    vTaskDelete(NULL);
    return;
}

void example_matter_write_protect_task(void)
{
    if(xTaskCreate(example_matter_write_protect_task_thread, ((const char*)"example_matter_write_protect_task_thread"), 8092, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
        printf("\n\r%s xTaskCreate(example_matter_write_protect_task_thread) failed", __FUNCTION__);
}
