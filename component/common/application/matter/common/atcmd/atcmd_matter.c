/*
 * AT Command for Matter
*/
#include <platform_stdlib.h>
#include <platform_opts.h>

#ifdef CHIP_PROJECT
#include <sys_api.h>
#include "log_service.h"
extern void ChipTest(void);
extern u32 deinitPref(void);
#if CONFIG_ENABLE_OTA_REQUESTOR
#if CONFIG_EXAMPLE_MATTER_CHIPTEST
extern void amebaQueryImageCmdHandler();
extern void amebaApplyUpdateCmdHandler();
#endif
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
#if defined(CONFIG_EXAMPLE_MATTER_CUSTOMIZED) && CONFIG_EXAMPLE_MATTER_CUSTOMIZED
void fATmatterApp(void *arg)
{
	if(!arg)
	{
		printf("[ATMA]Usage: ATMA=0/1/2 (0: aircon, 1: laundrywasher, 2: refrigerator\n\r");
		return;
	}
	uint8_t select = atoi((const char *)(arg));
	switch(select)
	{
	case 0:
		example_matter_aircon_dm();
		break;
	case 1:
		example_matter_laundrywasher_dm();
		break;
	case 2:
		example_matter_refrigerator_dm();
		break;
	}
}
#endif
log_item_t at_matter_items[] = {
#ifndef CONFIG_INIC_NO_FLASH
#if ATCMD_VER == ATVER_1
    {"ATM$", fATchipapp, {NULL,NULL}},
    {"ATM%", fATchipapp1, {NULL, NULL}},
    {"ATM^", fATchipapp2, {NULL, NULL}},
    {"ATMS", fATmattershell, {NULL, NULL}},
#if defined(CONFIG_EXAMPLE_MATTER_CUSTOMIZED) && CONFIG_EXAMPLE_MATTER_CUSTOMIZED
	{"ATMA", fATmatterApp, {NULL, NULL}},
#endif
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

#endif /* CHIP_PROJECT */
