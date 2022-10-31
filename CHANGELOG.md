# Changes Notes for Matter Support

SDK Base: sdk-ameba-v7.1d
Critical Patch: 7.1d_critical_patch_full_20220905_988a2b12_v(17)

## 1. Add Matter application
	
	Adding Matter Implementation: component/common/application/
	
	=> component/common/application/matter/application/chip_porting.c
	=> component/common/application/matter/application/chip_porting.h
	=> component/common/application/matter/application/example_matter.c
	=> component/common/application/matter/application/example_matter.h
	=> component/common/application/matter/application/matter_dcts.c
	=> component/common/application/matter/application/matter_dcts.h
	=> component/common/application/matter/application/matter_timers.c
	=> component/common/application/matter/application/matter_timers.h
	=> component/common/application/matter/application/matter_wifis.c
	=> component/common/application/matter/application/matter_wifis.h
	=> component/common/application/matter/common/atcmd_matter.c
	=> component/common/application/matter/common/bluetooth/bt_matter_adapter/bt_matter_adapter_app_flags.h
	=> component/common/application/matter/common/bluetooth/bt_matter_adapter/bt_matter_adapter_app_main.c
	=> component/common/application/matter/common/bluetooth/bt_matter_adapter/bt_matter_adapter_app_main.h
	=> component/common/application/matter/common/bluetooth/bt_matter_adapter/bt_matter_adapter_app_task.c
	=> component/common/application/matter/common/bluetooth/bt_matter_adapter/bt_matter_adapter_app_task.h
	=> component/common/application/matter/common/bluetooth/bt_matter_adapter/bt_matter_adapter_config.h
	=> component/common/application/matter/common/bluetooth/bt_matter_adapter/bt_matter_adapter_peripheral_app.c
	=> component/common/application/matter/common/bluetooth/bt_matter_adapter/bt_matter_adapter_peripheral_app.h
	=> component/common/application/matter/common/bluetooth/bt_matter_adapter/bt_matter_adapter_service.c
	=> component/common/application/matter/common/bluetooth/bt_matter_adapter/bt_matter_adapter_service.h
	=> component/common/application/matter/common/bluetooth/bt_matter_adapter/bt_matter_adapter_wifi.c
	=> component/common/application/matter/common/bluetooth/bt_matter_adapter/bt_matter_adapter_wifi.h
	=> component/common/application/matter/mbedtls/mbedtls_config.h
	=> component/common/application/matter/mbedtls/net_sockets.c

## 2. Add toolchain asdk-10.3.0

	Toolchain path: tools/arm-none-eabi-gcc

	=> asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2.partaf
	=> asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2.partae
	=> asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2.partad
	=> asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2.partac
	=> asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2.partab
	=> asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2.partaa

## 3. Add lwip_v2.1.2 folder

	=> component/common/network/lwip/lwip_v2.1.2

## 4. Add script and makefile for building Matter SDK

	=> project/realtek_amebaz2_v0_example/GCC-RELEASE/ameba_zap_cluster_list.py
	=> project/realtek_amebaz2_v0_example/GCC-RELEASE/lib_chip.mk
	=> project/realtek_amebaz2_v0_example/GCC-RELEASE/lib_chip_chef_core.mk
	=> project/realtek_amebaz2_v0_example/GCC-RELEASE/lib_chip_chef_main.mk
	=> project/realtek_amebaz2_v0_example/GCC-RELEASE/lib_chip_light_core.mk
	=> project/realtek_amebaz2_v0_example/GCC-RELEASE/lib_chip_light_main.mk
	=> project/realtek_amebaz2_v0_example/GCC-RELEASE/lib_chip_main.mk
	=> project/realtek_amebaz2_v0_example/GCC-RELEASE/lib_chip_otar_core.mk
	=> project/realtek_amebaz2_v0_example/GCC-RELEASE/lib_chip_otar_main.mk
	=> project/realtek_amebaz2_v0_example/GCC-RELEASE/lib_chip_switch_core.mk
	=> project/realtek_amebaz2_v0_example/GCC-RELEASE/lib_chip_switch_main.mk
	
## 5. Add Matter into GCC

	5.1 Modify project/realtek_amebaz2_v0_example/GCC-RELEASE/Makefile

		=> Change toolchain asdk-6.4.0 to asdk-10.3.0
		=> Add Matter libraries makefile into build

	5.2 Modify project/realtek_amebaz2_v0_example/GCC-RELEASE/application.is.mk

		=> Add Matter Path
		=> Add Matter OTA header information
		=> Add Matter application folder header and source file (component/common/application/matter)
		=> Add or Remove CFLAGS & CPPFLAGS & LIBFLAGS Manually
		=> Add ota_image_tool.py
		=> Change mbedtls-2.4.0 to Matter SDK's mbedtls
		=> Change lwip_v2.0.2 to lwip_v2.1.2
		=> Delete component/common/network/ssl/ssl_func_stubs/ssl_func_stubs.c

	5.3 Modify project/realtek_amebaz2_v0_example/GCC-RELEASE/partition.json

		=> Revise Start Address and Length to support Matter

	5.4 Modify project/realtek_amebaz2_v0_example/GCC-RELEASE/rtl8710c_ram.ld

		=> Revise rtl8710c_ram.ld for supporting Matter and C++

	5.5 Modify project/realtek_amebaz2_v0_example/GCC-RELEASE/toolchain.mk

		=> Change toolchain asdk-6.4.0 to asdk-10.3.0

## 6.	Matter Modification part 1

	6.1	Delete component/soc/realtek/8710c/misc/utilities/include/ctype.h

		=> conflict with C++ Toolchain

	6.2	Modify component/soc/realtek/8710c/cmsis/rtl8710c/include/basic_types.h

		=> Remove typedef unsigned char           bool;
			=> Fix declaration conflict: redeclaration of C++ built-in type "bool"

		=> Remove #define IN 
			=> Fix declaration conflict: MDNS source code consist different declaration

		=> Redefine data type
			=> Fix declaration conflict: conflict data type in Matter/Ameba SDK

	6.3 Fix compile error for "IN"

		=>Define IN only in the file
			=> component/common/api/wifi/wifi_conf.c
			=> component/common/api/wifi/wifi_simple_config.c
			=> component/soc/realtek/8710c/fwlib/include/hal_crypto.h
	
	6.4	Modify files to fix compile error for "Conversion of types may change value"

		=> Ignore the Conversion check when building these headers
			=> component/soc/realtek/8710c/fwlib/include/hal_i2c.h
			=> component/soc/realtek/8710c/fwlib/include/hal_pwm.h
			=> component/soc/realtek/8710c/fwlib/include/hal_timer.h
			=> component/soc/realtek/8710c/fwlib/include/rtl8710c_pwm.h
			=> component/soc/realtek/8710c/fwlib/include/rtl8710c_spic.h
			=> component/soc/realtek/8710c/fwlib/include/rtl8710c_timer.h

	6.5	Modify component/soc/realtek/8710c/misc/utilities/source/ram/libc_wrap.c

		=> Include time-functions for asdk-10.3.0 (__tzinfo)

	6.6 Modify files to support C++

		=> Add _fini and __libc_init_array to support C++ constructors
			=> component/soc/realtek/8710c/cmsis/rtl8710c/source/ram_s/app_start.c

		=> Add extern C to support C++ 
			=> component/common/bluetooth/realtek/sdk/board/amebaz2/src/rtk_coex.h
			=> component/os/os_dep/include/device_lock.h
			=> component/soc/realtek/8710c/misc/platform/ota_8710c.h

	6.7 Modify component/common/network/sntp/sntp.c

		=> Fix issue when running EXAMPLE_SNTP_SHOWTIME

## 7. Matter Modification part 2

	7.1 Modify Device Configuration Table (DCT) for Matter

		=> component/common/file_system/dct/dct.h
		=> component/common/mbed/hal_ext/flash_api.h
		=> component/common/mbed/targets/hal/rtl8710c/flash_api.c
		=> component/soc/realtek/8710c/misc/bsp/lib/common/GCC/lib_dct.a
		=> component/soc/realtek/8710c/misc/bsp/lib/common/GCC/lib_dct_ns.a

	7.2 Modify for Matter OTA

		=> component/soc/realtek/8710c/misc/utilities/include/strproc.h
		=> component/common/mbed/hal_ext/sys_api.h

	7.3 Modify project/realtek_amebaz2_v0_example/src/main.c

		=> Fix mbedTLS error: -0xFFFF5180 when start CHIP task by adding mbedtls_platform_set_calloc_free

	7.4 Add Wi-Fi features for Matter use

		=> Add wext_get_auth_type
			=> component/common/api/wifi/wifi_util.h

		=> Add RTW_SECURITY_WPA_WPA2_MIXED
			=> component/common/drivers/wlan/realtek/include/wifi_constants.h

## 8. Matter Modification part 3

	8.1 Modify component/common/example/example_entry.c

		=> Add example_matter into example_entry
	    
	8.2 Modify  component/common/api/at_cmd/log_service.c

		=> Initialize Matter AT Command
	
	8.3 Modify files to support lwip_2.1.2 and IPv6
	
		=> Add IPv6 Function
			=> component/common/api/lwip_netconf.h

		=> Modify component/common/api/network/include/lwipopts.h

			Modify PBUF and UDP_PCB SIZE
			=> #define PBUF_POOL_SIZ                40
			=> #define PBUF_POOL_BUFSI              1300
			=> #define MEMP_NUM_UDP_PCB             10

			Extra option for lwip
			=> #define LWIP_COMPAT_MUTEX_ALLOWED     1
			=> #define LWIP_IPV6_ND                  1
			=> #define LWIP_IPV6_SCOPES              0
			=> #define LWIP_PBUF_FROM_CUSTOM_POOLS   0
			=> #define ERRNO                         1
			=> #define LWIP_SO_SNDTIMEO              1
			=> #define LWIP_SOCKET_SET_ERRNO         1

			Enable IPV6
			=> #define LWIP_IPV6                     1
			=> #define LWIP_IPV6_MLD                 1
			=> #define LWIP_IPV6_AUTOCONFIG          1
			=> #define LWIP_ICMP6                    1
			=> #define LWIP_IPV6_DHCP6               1

	8.4 Modify project/realtek_amebaz2_v0_example/inc/platform_opts_bt.h

		=> Enable #define CONFIG_BT                1
		=> Enable #define CONFIG_BT_PERIPHERAL     1
		=> Add #define CONFIG_BT_MATTER_ADAPTER    1
	
	8.5 Modify project/realtek_amebaz2_v0_example/inc/platform_opts.h

		=> Disable CONFIG_USE_AZURE_EMBEDDED_C
		=> Disable CONFIG_EXAMPLE_WLAN_FAST_CONNECT
		=> Add settings for Matter
	
	8.6 Add Freertos Features

		=> To check the heap usage
			=> component/os/freertos/freertos_v10.0.1/Source/include/portable.h
			=> component/os/freertos/freertos_v10.0.1/Source/portable/MemMang/heap_5.c

		=> To support Matter Software Diagnostic
			=> component/os/freertos/freertos_v10.0.1/Source/tasks.c
			=> component/os/freertos/freertos_v10.0.1/Source/include/task.h

		=> Modify	project/realtek_amebaz2_v0_example/inc/FreeRTOSConfig.h

			=> #define configUSE_TRACE_FACILITY           1
			=> #define configRECORD_STACK_HIGH_ADDRESS		1
			=> #define INCLUDE_uxTaskGetStackSize         1
			=> #define INCLUDE_uxTaskGetFreeStackSize     1