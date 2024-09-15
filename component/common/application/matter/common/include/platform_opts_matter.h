/**
 ******************************************************************************
 *This file contains general configurations for ameba platform
 ******************************************************************************
*/

#ifndef __PLATFORM_OPTS_MATTER_H__
#define __PLATFORM_OPTS_MATTER_H__

/* chip wide*/
/* logging functionality */
// See ./sdk-ameba-v7.1d/third_party/connectedhomeip/src/platform/Ameba/CHIPPlatformConfig.h for CHIP_CONFIG_ERROR_SOURCE macro
#define CONFIG_AMEBA_MATTER_SHORT_LOG_FMT       0  // 0 -> file name and line number stored, but will take up more flash!
                                                   // 1 -> file name and line number will NOT be stored, but flash usage is reduced
#if defined(CONFIG_AMEBA_MATTER_SHORT_LOG_FMT) && (CONFIG_AMEBA_MATTER_SHORT_LOG_FMT == 0)
#define CONFIG_AMEBA_MATTER_LOG_FILENAME_MAXSZ  32
#endif
#define CHIP_CONFIG_ERROR_FORMAT_AS_STRING      1
#define CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER     1   // must be enabled to transfer full log to controller
#define CONFIG_AMEBA_DEBUG_FORCE_CRASH_ATCMD    1   // used to generate test crash log via atcmd @@@@
#define CONFIG_AMEBA_LOGS_USE_FATFS             0   // 0 -> direct flash operation, 1 -> use fatfs driver

/* For Matter */
#define CONFIG_EXAMPLE_MATTER                   1
#define CONFIG_EXAMPLE_MATTER_CHIPTEST          0
#define CONFIG_EXAMPLE_MATTER_AIRCON            0
#define CONFIG_EXAMPLE_MATTER_BRIDGE            0
#define CONFIG_EXAMPLE_MATTER_DISHWASHER        0
#define CONFIG_EXAMPLE_MATTER_FAN               0
#define CONFIG_EXAMPLE_MATTER_LIGHT             0
#define CONFIG_EXAMPLE_MATTER_LAUNDRY_WASHER    0
#define CONFIG_EXAMPLE_MATTER_MICROWAVE_OVEN    0
#define CONFIG_EXAMPLE_MATTER_REFRIGERATOR      0
#define CONFIG_EXAMPLE_MATTER_THERMOSTAT        0
#define CONFIG_EXAMPLE_MATTER_LIGHT_LOGREDIRECT 0

#if defined(CONFIG_EXAMPLE_MATTER) && (CONFIG_EXAMPLE_MATTER == 1)

#undef CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#undef CONFIG_FAST_DHCP
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT        0
#if CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#define CONFIG_FAST_DHCP                        1
#else
#define CONFIG_FAST_DHCP                        0
#endif /* CONFIG_EXAMPLE_WLAN_FAST_CONNECT */
#endif /* CONFIG_EXAMPLE_MATTER */

#if defined(CONFIG_PLATFORM_8710C)
#undef CONFIG_USE_AZURE_EMBEDDED_C
#define CONFIG_USE_AZURE_EMBEDDED_C             0

/* Matter layout */
#undef FAST_RECONNECT_DATA
#undef BT_FTL_PHY_ADDR0
#undef BT_FTL_PHY_ADDR1
#undef BT_FTL_BKUP_ADDR
#undef UART_SETTING_SECTOR
#undef DCT_BEGIN_ADDR
#undef DCT_BEGIN_ADDR2
#undef MATTER_FACTORY_DATA
#define FAST_RECONNECT_DATA         (0x400000 - 0x1000)        // 0x3FF000
#define BT_FTL_PHY_ADDR0            (0x400000 - 0x2000)        // 0x3FE000
#define BT_FTL_PHY_ADDR1            (0x400000 - 0x3000)        // 0x3FD000
#define BT_FTL_BKUP_ADDR            (0x400000 - 0x4000)        // 0x3FC000
#define UART_SETTING_SECTOR         (0x400000 - 0x5000)        // 0x3FB000
#define DCT_BEGIN_ADDR              (0x400000 - 0x13000)    // 0x3ED000 ~ 0x3FB000 : 56K 
#define DCT_BEGIN_ADDR2             (0x400000 - 0x1A000)    // 0x3E6000 ~ 0x3ED000 : 24K


#undef SECTOR_SIZE_FLASH
#define SECTOR_SIZE_FLASH	        4096
#define FAULT_FLASH_SECTOR_SIZE	   (SECTOR_SIZE_FLASH)
#define USER_LOG_FILENAME           "user.log"
#define NET_LOG_FILENAME            "net.log"
#define CRASH_LOG_FILENAME          "crash.log"

#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 1)

#define FATFS_DISK_FLASH                        1   // flash for capturing crashlog

/* FatFS related parameters */
#undef FLASH_APP_BASE
#undef FLASH_BLOCK_SIZE
#undef FLASH_SECTOR_COUNT
#define FLASH_APP_BASE              (0x400000 - 0x9A000)    // assume a max volume size of 512K (0x80000)
#define FLASH_BLOCK_SIZE	        512		// not passing any
#define FLASH_SECTOR_COUNT	        128

#define FAULT_LOG1				   (0x400000 - 0x9C000) //Store fault log 8K
#define FAULT_LOG2				   (0x400000 - 0x9E000) //Store fault log 8K


#elif defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 0)
/* littlefs related parameters */
#define LITTLEFS_MAX_SIZE           0x20000
#define LITTLEFS_START_ADDR         DCT_BEGIN_ADDR2 - LITTLEFS_MAX_SIZE
#define LITTLEFS_NUM_BLOCKS         (LITTLEFS_MAX_SIZE / SECTOR_SIZE_FLASH)

#define RETAIN_NLOGS_WHEN_FULL      40  // most recent N logs will be kept, the rest cleared  

#define FAULT_LOG1                  (LITTLEFS_START_ADDR - 0x1000)
#define FAULT_LOG2                  (LITTLEFS_START_ADDR - 0x2000)

#endif


#define MATTER_FACTORY_DATA        (0x3FF000)                 // last 4KB of external flash - write protection is supported in this region

#endif /* CONFIG_PLATFORM_87XXX */

#endif /* __PLATFORM_OPTS_MATTER_H__ */
