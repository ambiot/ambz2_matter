#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "main.h"
#include <example_entry.h>

#include <flash_api.h>
#include <device_lock.h>

extern void fault_handler_override(void(*fault_log)(char *msg, int len), void(*bt_log)(char *msg, int len));
void fault_log(char *msg, int len)
{
	flash_t	fault_flash;
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	flash_erase_sector(&fault_flash, FAULT_LOG1);
	flash_stream_write(&fault_flash, FAULT_LOG1, len, (uint8_t*)msg);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);
}

void bt_log(char *msg, int len)
{
	flash_t	fault_flash;
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	flash_erase_sector(&fault_flash, FAULT_LOG2);
	flash_stream_write(&fault_flash, FAULT_LOG2, len, (uint8_t*)msg);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);
}

extern void console_init(void);

#if defined(CONFIG_MATTER) && CONFIG_MATTER
static void* app_mbedtls_calloc_func(size_t nelements, size_t elementSize)
{
	size_t size;
	void *ptr = NULL;

	size = nelements * elementSize;
	ptr = pvPortMalloc(size);

	if(ptr)
		memset(ptr, 0, size);

	return ptr;
}
#endif

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
	fault_handler_override(fault_log, bt_log);

	/* Initialize log uart and at command service */
	console_init();

#if defined(CONFIG_MATTER) && CONFIG_MATTER
	mbedtls_platform_set_calloc_free(app_mbedtls_calloc_func, vPortFree);
#endif

	/* pre-processor of application example */
	pre_example_entry();

	/* wlan intialization */
	wlan_network();

	/* Execute application example */
	example_entry();

	/* Enable Schedule, Start Kernel */
	vTaskStartScheduler();

	/* Should NEVER reach here */
	return 0;
}