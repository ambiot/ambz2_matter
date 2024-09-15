/*
 * AT Command for Matter
*/
#include <platform_stdlib.h>
#include <platform_opts.h>

#if defined(CONFIG_MATTER) && CONFIG_MATTER
#include <main.h>
#include <sys_api.h>
#include "log_service.h"
extern void ChipTest(void);
extern u32 deinitPref(void);
#if CONFIG_ENABLE_OTA_REQUESTOR
#if CONFIG_EXAMPLE_MATTER_CHIPTEST
extern void amebaQueryImageCmdHandler();
extern void amebaApplyUpdateCmdHandler();
#endif

extern void ChipDiagLogInsertSector(void);
#endif

// Queue for matter shell
QueueHandle_t shell_queue;

void fATchipapp(void *arg)
{
	/* To avoid gcc warnings */
	( void ) arg;
	printf("xPortGetTotalHeapSize = %d \n",xPortGetTotalHeapSize());
	printf("xPortGetFreeHeapSize = %d \n",xPortGetFreeHeapSize());
	printf("xPortGetMinimumEverFreeHeapSize = %d \n",xPortGetMinimumEverFreeHeapSize());

	deinitPref();

#if CONFIG_EXAMPLE_WLAN_FAST_CONNECT
	Erase_Fastconnect_data();
	printf("Erased Fast Connect data\r\n");
#endif
	AT_PRINTK("[ATM$]: _AT_SYSTEM_TEST_");
	wifi_disconnect();
	sys_reset();
}

void fATchipapp1(void *arg)
{
#if CONFIG_ENABLE_OTA_REQUESTOR
#if CONFIG_EXAMPLE_MATTER_CHIPTEST
	printf("Chip Test: amebaQueryImageCmdHandler\r\n");
	amebaQueryImageCmdHandler();
#endif
#endif
}

void fATchipapp2(void *arg)
{
#if CONFIG_ENABLE_OTA_REQUESTOR
#if CONFIG_EXAMPLE_MATTER_CHIPTEST
	(void) arg;
	printf("Chip Test: amebaApplyUpdateCmdHandler\r\n");

	amebaApplyUpdateCmdHandler();
#endif
#endif
}

void fATmattershell(void *arg)
{
    if (arg != NULL)
    {
        xQueueSend(shell_queue, arg, pdMS_TO_TICKS(10));
    }
    else
    {
        printf("No arguments provided for matter shell\r\n");
    } 
}

void fATcrash(void *arg)
{
#if defined(CONFIG_AMEBA_DEBUG_FORCE_CRASH_ATCMD) && (CONFIG_AMEBA_DEBUG_FORCE_CRASH_ATCMD == 1)
printf("!@#$ FORCE CRASHING CORE !@#$\n");
	((void (*)(void))2)();
#endif // CONFIG_AMEBA_DEBUG_FORCE_CRASH_ATCMD
}

void fATinsertlog(void* arg)
{
#if defined(CONFIG_EXAMPLE_MATTER_LIGHT_LOGREDIRECT) && (CONFIG_EXAMPLE_MATTER_LIGHT_LOGREDIRECT == 1)
	(void)arg;
	ChipDiagLogInsertSector();	// debug
#endif
}

log_item_t at_matter_items[] = {
#ifndef CONFIG_INIC_NO_FLASH
#if ATCMD_VER == ATVER_1
    {"ATM$", fATchipapp},
    {"ATM%", fATchipapp1},
    {"ATM^", fATchipapp2},
    {"ATMS", fATmattershell},
	{"@@@@", fATcrash},
	{.log_cmd="####", fATinsertlog},
#endif // end of #if ATCMD_VER == ATVER_1
#endif
};

void at_matter_init(void)
{
    shell_queue = xQueueCreate(3, 256); // backlog 3 commands max
	log_service_add_table(at_matter_items, sizeof(at_matter_items)/sizeof(at_matter_items[0]));
}

#if SUPPORT_LOG_SERVICE
log_module_init(at_matter_init);
#endif

#endif /* CONFIG_MATTER */
