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
	AT_PRINTK("[ATS#]: _AT_SYSTEM_TEST_");
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

log_item_t at_matter_items[] = {
#ifndef CONFIG_INIC_NO_FLASH
#if ATCMD_VER == ATVER_1
	{"ATS$", fATchipapp, {NULL,NULL}},
	{"ATS%", fATchipapp1, {NULL, NULL}},
	{"ATS^", fATchipapp2, {NULL, NULL}},
#endif // end of #if ATCMD_VER == ATVER_1
#endif
};

void at_matter_init(void)
{
	log_service_add_table(at_matter_items, sizeof(at_matter_items)/sizeof(at_matter_items[0]));
}

#if SUPPORT_LOG_SERVICE
log_module_init(at_matter_init);
#endif

#endif /* CHIP_PROJECT */
