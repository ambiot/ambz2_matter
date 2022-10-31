#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "main.h"
#include <example_entry.h>

extern void console_init(void);

#ifdef CHIP_PROJECT
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
	/* Initialize log uart and at command service */
	console_init();

#ifdef CHIP_PROJECT
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