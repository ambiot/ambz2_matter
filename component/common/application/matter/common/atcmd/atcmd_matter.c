/*
 * AT Command for Matter
*/
#include <platform_stdlib.h>
#include <platform_opts.h>

#ifdef CHIP_PROJECT
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

#define CONFIG_ENABLE_AMEBA_DLOG_TEST 1
#if defined(CONFIG_ENABLE_AMEBA_DLOG_TEST) && (CONFIG_ENABLE_AMEBA_DLOG_TEST == 1)
extern int requires_bdx;
void fATcrash(void *arg)
{
    printf("!@#$ FORCE CRASHING CORE !@#$\n");
    requires_bdx = 1;

    ((void (*)(void))2)();

    return;
}

void fATcrashbdx(void *arg)
{
    printf("!@#$ FORCE CRASHING CORE !@#$\n");
    requires_bdx = 0;

    ((void (*)(void))2)();

    return;
}

void fATuserlog(void *arg)
{
    if (!arg) {
        printf("[@@@@]Usage: ####=[size]\n\r");
        printf("      Set more than 1024 to trigger bdx transfer\n\r");
        return;
    }

    size_t dataSize = (size_t)atoi((const char *)arg);

    u8 *data = (u8 *)malloc(dataSize * sizeof(u8));
    if (data == NULL)
    {
        return;
    }

    const char *logMessage = "Hello World";
    strncpy((char *)data, logMessage, 11);
    data[sizeof(data) - 1] = '\0';
    matter_insert_user_log(data, dataSize);

    if (data)
    {
        free(data);
    }

    return;
}

void fATnetworklog(void *arg)
{
    if (!arg) {
        printf("[@@@@]Usage: ####=[size]\n\r");
        printf("      Set more than 1024 to trigger bdx transfer\n\r");
        return;
    }

    size_t dataSize = (size_t)atoi((const char *)arg);

    u8 *data = (u8 *)malloc(dataSize * sizeof(u8));
    if (data == NULL)
    {
        return;
    }

    const char *logMessage = "No Error Found";
    strncpy((char *)data, logMessage, 14);
    data[sizeof(data) - 1] = '\0';
    matter_insert_network_log(data, dataSize);

    if (data)
    {
        free(data);
    }

    return;
}
#endif /* CONFIG_ENABLE_AMEBA_DLOG_TEST */

log_item_t at_matter_items[] = {
#ifndef CONFIG_INIC_NO_FLASH
#if ATCMD_VER == ATVER_1
    {"ATM$", fATchipapp},
    {"ATM%", fATchipapp1},
    {"ATM^", fATchipapp2},
    {"ATMS", fATmattershell},
#if defined(CONFIG_ENABLE_AMEBA_DLOG_TEST) && (CONFIG_ENABLE_AMEBA_DLOG_TEST == 1)
    {"@@@@", fATcrash},
    {"####", fATcrashbdx},
    {"$$$$", fATuserlog},
    {"^^^^", fATnetworklog},
#endif /* CONFIG_ENABLE_AMEBA_DLOG_TEST */
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
