/**
 ******************************************************************************
 *This file contains general configurations for ameba platform
 ******************************************************************************
*/

#ifndef __PLATFORM_OPTS_MATTER_H__
#define __PLATFORM_OPTS_MATTER_H__

/* For Matter */
#define CONFIG_EXAMPLE_MATTER                   1
#define CONFIG_EXAMPLE_MATTER_CHIPTEST          1
#define CONFIG_EXAMPLE_MATTER_AIRCON            0
#define CONFIG_EXAMPLE_MATTER_BRIDGE            0
#define CONFIG_EXAMPLE_MATTER_DISHWASHER        0
#define CONFIG_EXAMPLE_MATTER_LIGHT             0
#define CONFIG_EXAMPLE_MATTER_FAN               0
#define CONFIG_EXAMPLE_MATTER_LAUNDRY_WASHER    0
#define CONFIG_EXAMPLE_MATTER_REFRIGERATOR      0
#define CONFIG_EXAMPLE_MATTER_THERMOSTAT        0

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
#define FAST_RECONNECT_DATA        (0x400000 - 0x1000)        // 0x3FF000
#define BT_FTL_PHY_ADDR0           (0x400000 - 0x2000)        // 0x3FE000
#define BT_FTL_PHY_ADDR1           (0x400000 - 0x3000)        // 0x3FD000
#define BT_FTL_BKUP_ADDR           (0x400000 - 0x4000)        // 0x3FC000
#define UART_SETTING_SECTOR        (0x400000 - 0x5000)        // 0x3FB000
#define DCT_BEGIN_ADDR             (0x400000 - 0x13000)    // 0x3ED000 ~ 0x3FB000 : 56K 
#define DCT_BEGIN_ADDR2            (0x400000 - 0x1A000)    // 0x3E6000 ~ 0x3ED000 : 24K
#define MATTER_FACTORY_DATA        (0x3FF000)              // last 4KB of external flash - write protection is supported in this region

#endif /* CONFIG_PLATFORM_87XXX */

#endif /* __PLATFORM_OPTS_MATTER_H__ */
