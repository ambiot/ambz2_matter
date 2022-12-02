SHELL = /bin/bash

# Initialize tool chain
# -------------------------------------------------------------------
BASEDIR := $(shell pwd)
SDKROOTDIR := $(BASEDIR)/../../../../../..
AMEBAZ2_TOOLDIR	= $(SDKROOTDIR)/component/soc/realtek/8710c/misc/iar_utility
CHIPDIR = $(SDKROOTDIR)/third_party/connectedhomeip
OUTPUT_DIR = $(CHIPDIR)/examples/lighting-app/ameba/build/chip

OS := $(shell uname)

CROSS_COMPILE = $(ARM_GCC_TOOLCHAIN)/arm-none-eabi-

# Compilation tools
AR = $(CROSS_COMPILE)ar
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
NM = $(CROSS_COMPILE)nm
LD = $(CROSS_COMPILE)gcc
GDB = $(CROSS_COMPILE)gdb
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

OS := $(shell uname)

# Initialize target name and target object files
# -------------------------------------------------------------------

OBJ_DIR=$(TARGET)/Debug/obj
BIN_DIR=$(TARGET)/Debug/bin
INFO_DIR=$(TARGET)/Debug/info

# Include folder list
# -------------------------------------------------------------------

INCLUDES =
INCLUDES += -I$(SDKROOTDIR)/project/realtek_amebaz2_v0_example/inc

INCLUDES += -I$(SDKROOTDIR)/component/common/api
INCLUDES += -I$(SDKROOTDIR)/component/common/api/at_cmd
INCLUDES += -I$(SDKROOTDIR)/component/common/api/platform
INCLUDES += -I$(SDKROOTDIR)/component/common/api/wifi
INCLUDES += -I$(SDKROOTDIR)/component/common/api/wifi/rtw_wpa_supplicant/src
INCLUDES += -I$(SDKROOTDIR)/component/common/api/wifi/rtw_wpa_supplicant/src/crypto
INCLUDES += -I$(SDKROOTDIR)/component/common/api/network/include
INCLUDES += -I$(SDKROOTDIR)/component/common/application
INCLUDES += -I$(SDKROOTDIR)/component/common/application/mqtt/MQTTClient
INCLUDES += -I$(SDKROOTDIR)/component/common/example
INCLUDES += -I$(SDKROOTDIR)/component/common/file_system
INCLUDES += -I$(SDKROOTDIR)/component/common/file_system/dct
INCLUDES += -I$(SDKROOTDIR)/component/common/file_system/fatfs
INCLUDES += -I$(SDKROOTDIR)/component/common/file_system/fatfs/r0.10c/include
INCLUDES += -I$(SDKROOTDIR)/component/common/file_system/ftl
INCLUDES += -I$(SDKROOTDIR)/component/common/utilities
INCLUDES += -I$(SDKROOTDIR)/component/common/mbed/hal
INCLUDES += -I$(SDKROOTDIR)/component/common/mbed/hal_ext
INCLUDES += -I$(SDKROOTDIR)/component/common/mbed/targets/hal/rtl8710c
INCLUDES += -I$(SDKROOTDIR)/component/common/network
INCLUDES += -I$(SDKROOTDIR)/component/common/network/coap/include
INCLUDES += -I$(SDKROOTDIR)/component/common/network/libcoap/include
INCLUDES += -I$(SDKROOTDIR)/component/common/network/http2/nghttp2-1.31.0/includes
INCLUDES += -I$(SDKROOTDIR)/component/common/network/lwip/lwip_v2.1.2/src/include
INCLUDES += -I$(SDKROOTDIR)/component/common/network/lwip/lwip_v2.1.2/src/include/lwip
INCLUDES += -I$(SDKROOTDIR)/component/common/network/lwip/lwip_v2.1.2/port/realtek
INCLUDES += -I$(SDKROOTDIR)/component/common/network/lwip/lwip_v2.1.2/port/realtek/freertos
INCLUDES += -I$(SDKROOTDIR)/component/common/network/ssl/mbedtls-matter/include
INCLUDES += -I$(SDKROOTDIR)/component/common/network/ssl/mbedtls-matter/include/mbedtls
#INCLUDES += -I$(SDKROOTDIR)/component/common/network/ssl/ssl_ram_map/rom
INCLUDES += -I$(SDKROOTDIR)/component/common/drivers/wlan/realtek/include
INCLUDES += -I$(SDKROOTDIR)/component/common/drivers/wlan/realtek/src/osdep
INCLUDES += -I$(SDKROOTDIR)/component/common/drivers/wlan/realtek/src/core/option
INCLUDES += -I$(SDKROOTDIR)/component/common/test
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/inc
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/inc/app
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/inc/bluetooth/gap
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/inc/bluetooth/profile
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/inc/bluetooth/profile/client
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/inc/bluetooth/profile/server
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/inc/os
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/inc/platform
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/inc/stack
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/board/amebaz2/lib
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/board/amebaz2/src
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/board/amebaz2/src/data_uart
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/board/amebaz2/src/hci
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/board/amebaz2/src/os
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/board/amebaz2/src/vendor_cmd
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/ble_central
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/ble_peripheral
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/ble_scatternet
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_fuzz_test
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_ota_central_client
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_datatrans
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/lib/cmd
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/lib/common
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/lib/gap
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/lib/inc
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/lib/inc/amebaz2
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/lib/model
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/lib/model/realtek
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/lib/platform
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/lib/profile
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/lib/utility
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/provisioner
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/device
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/api/common
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/api/provisioner
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/api/device
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh/api
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh_provisioner_rtk_demo
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh_provisioner_rtk_demo/inc
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh_device_rtk_demo
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh_multiple_profile/device_multiple_profile
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh_multiple_profile/provisioner_multiple_profile
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh_test
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/src/mcu/module/data_uart_cmd
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/src/app/hrp/gap
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/board/common/inc
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_airsync_config
INCLUDES += -I$(SDKROOTDIR)/component/common/media/rtp_codec
INCLUDES += -I$(SDKROOTDIR)/component/common/media/mmfv2
INCLUDES += -I$(SDKROOTDIR)/component/common/application/airsync/1.0.4

INCLUDES += -I$(SDKROOTDIR)/component/soc/realtek/8710c/cmsis/rtl8710c/include
INCLUDES += -I$(SDKROOTDIR)/component/soc/realtek/8710c/cmsis/rtl8710c/lib/include
INCLUDES += -I$(SDKROOTDIR)/component/soc/realtek/8710c/fwlib/include
INCLUDES += -I$(SDKROOTDIR)/component/soc/realtek/8710c/fwlib/lib/include
INCLUDES += -I$(SDKROOTDIR)/component/soc/realtek/8710c/cmsis/cmsis-core/include
INCLUDES += -I$(SDKROOTDIR)/component/soc/realtek/8710c/app/rtl_printf/include
INCLUDES += -I$(SDKROOTDIR)/component/soc/realtek/8710c/app/shell
INCLUDES += -I$(SDKROOTDIR)/component/soc/realtek/8710c/app/stdio_port
INCLUDES += -I$(SDKROOTDIR)/component/soc/realtek/8710c/misc/utilities/include
INCLUDES += -I$(SDKROOTDIR)/component/soc/realtek/8710c/mbed-drivers/include
INCLUDES += -I$(SDKROOTDIR)/component/soc/realtek/8710c/misc/platform
INCLUDES += -I$(SDKROOTDIR)/component/soc/realtek/8710c/misc/driver
INCLUDES += -I$(SDKROOTDIR)/component/soc/realtek/8710c/misc/os

INCLUDES += -I$(SDKROOTDIR)/component/os/freertos
INCLUDES += -I$(SDKROOTDIR)/component/os/freertos/freertos_v10.0.1/Source/include
INCLUDES += -I$(SDKROOTDIR)/component/os/freertos/freertos_v10.0.1/Source/portable/GCC/ARM_RTL8710C
INCLUDES += -I$(SDKROOTDIR)/component/os/os_dep/include

INCLUDES += -I$(SDKROOTDIR)/component/common/application/matter/common/bluetooth/bt_matter_adapter
INCLUDES += -I$(SDKROOTDIR)/component/common/application/matter/common/mbedtls
INCLUDES += -I$(SDKROOTDIR)/component/common/application/matter/common/port

# CHIP Include folder list
# -------------------------------------------------------------------
INCLUDES += -I$(CHIPDIR)/config/ameba
INCLUDES += -I$(CHIPDIR)/src/include/platform/Ameba
INCLUDES += -I$(CHIPDIR)/src/include
INCLUDES += -I$(CHIPDIR)/src/lib
INCLUDES += -I$(CHIPDIR)/src
INCLUDES += -I$(CHIPDIR)/src/system
INCLUDES += -I$(CHIPDIR)/src/app
INCLUDES += -I$(CHIPDIR)/third_party/nlassert/repo/include
INCLUDES += -I$(CHIPDIR)/third_party/nlio/repo/include
INCLUDES += -I$(CHIPDIR)/third_party/nlunit-test/repo/src

# Compile options
# -------------------------------------------------------------------

CFLAGS =
CFLAGS += -march=armv8-m.main+dsp -mthumb -mcmse -mfloat-abi=soft -D__thumb2__ -g -gdwarf-3 -Os
CFLAGS += -D__ARM_ARCH_8M_MAIN__=1 -gdwarf-3 -fstack-usage -fdata-sections -ffunction-sections 
CFLAGS += -fdiagnostics-color=always -Wall -Wpointer-arith -Wno-write-strings 
CFLAGS += -Wno-maybe-uninitialized -c -MMD
CFLAGS += -DCONFIG_PLATFORM_8710C -DCONFIG_BUILD_RAM=1
CFLAGS += -DV8M_STKOVF
#for time64 
ifdef SYSTEM_TIME64_MAKE_OPTION
CFLAGS += -DCONFIG_SYSTEM_TIME64=1
CFLAGS += -include time64.h
else
CFLAGS += -DCONFIG_SYSTEM_TIME64=0
endif

# CHIP options
# -------------------------------------------------------------------
CFLAGS += -DCHIP_PROJECT=1
CFLAGS += -DCONFIG_ENABLE_AMEBA_FACTORY_DATA=0
CFLAGS += -DCHIP_DEVICE_LAYER_TARGET=Ameba
CFLAGS += -DMBEDTLS_CONFIG_FILE=\"mbedtls_config.h\"

CFLAGS += -DLWIP_IPV6_ND=1
CFLAGS += -DLWIP_IPV6_SCOPES=0
CFLAGS += -DLWIP_PBUF_FROM_CUSTOM_POOLS=0
CFLAGS += -DLWIP_IPV6_ROUTE_TABLE_SUPPORT=1

CFLAGS += -DCHIP_DEVICE_LAYER_NONE=0
CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF=0
CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS=0
CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS=0

CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_LWIP=1
CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_SOCKETS=0
CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK=0

CXXFLAGS += -DFD_SETSIZE=10

CXXFLAGS += -Wno-sign-compare
CXXFLAGS += -Wno-unused-function
CXXFLAGS += -Wno-unused-but-set-variable
CXXFLAGS += -Wno-unused-label
CXXFLAGS += -Wno-unused-variable
CXXFLAGS += -Wno-deprecated-declarations
CXXFLAGS += -Wno-unused-parameter
CXXFLAGS += -Wno-format
CXXFLAGS += -Wno-format-nonliteral
CXXFLAGS += -Wno-format-security

CXXFLAGS += -std=gnu++14
#CXXFLAGS += -std=c++14
CXXFLAGS += -fno-rtti

CHIP_CFLAGS = $(CFLAGS)
CHIP_CFLAGS += $(INCLUDES)

CHIP_CXXFLAGS += $(CFLAGS)
CHIP_CXXFLAGS += $(CXXFLAGS)
CHIP_CXXFLAGS += $(INCLUDES)

#*****************************************************************************#
#                        RULES TO GENERATE TARGETS                            #
#*****************************************************************************#

# Define the Rules to build the core targets
all: GENERATE_NINJA

GENERATE_NINJA:
	echo "INSTALL CHIP..."
	mkdir -p $(OUTPUT_DIR)
	echo                                   > $(OUTPUT_DIR)/args.gn
	echo "import(\"//args.gni\")"          >> $(OUTPUT_DIR)/args.gn
	echo target_cflags_c  = [$(foreach word,$(CHIP_CFLAGS),\"$(word)\",)] | sed -e 's/=\"/=\\"/g;s/\"\"/\\"\"/g;'  >> $(OUTPUT_DIR)/args.gn
	echo target_cflags_cc = [$(foreach word,$(CHIP_CXXFLAGS),\"$(word)\",)] | sed -e 's/=\"/=\\"/g;s/\"\"/\\"\"/g;'   >> $(OUTPUT_DIR)/args.gn
	echo ameba_ar = \"arm-none-eabi-ar\"    >> $(OUTPUT_DIR)/args.gn
	echo ameba_cc = \"arm-none-eabi-gcc\"   >> $(OUTPUT_DIR)/args.gn
	echo ameba_cxx = \"arm-none-eabi-c++\"  >> $(OUTPUT_DIR)/args.gn
	echo ameba_cpu = \"ameba\"               >> $(OUTPUT_DIR)/args.gn
	echo chip_enable_ota_requestor = "true" >> $(OUTPUT_DIR)/args.gn
	echo chip_inet_config_enable_ipv4 = "false" >> $(OUTPUT_DIR)/args.gn
	echo chip_support_enable_storage_api_audit = "false" >> $(OUTPUT_DIR)/args.gn
	echo chip_use_transitional_commissionable_data_provider = "false" >> $(OUTPUT_DIR)/args.gn
	sed -i 's/chip_build_tests\ =\ true/chip_build_tests\ =\ false/g' $(CHIPDIR)/config/ameba/args.gni
	mkdir -p $(CHIPDIR)/config/ameba/components/chip
	cd $(CHIPDIR)/config/ameba/components/chip && gn gen --check --fail-on-unused-args $(CHIPDIR)/examples/lighting-app/ameba/build/chip
	cd $(CHIPDIR)/config/ameba/components/chip ; ninja -C $(CHIPDIR)/examples/lighting-app/ameba/build/chip :ameba
	cp -f $(OUTPUT_DIR)/lib/* $(SDKROOTDIR)/component/soc/realtek/8710c/misc/bsp/lib/common/GCC
#*****************************************************************************#
#              CLEAN GENERATED FILES                                          #
#*****************************************************************************#
.PHONY: clean
clean:
	echo "RM $(OUTPUT_DIR)"
	rm -rf $(OUTPUT_DIR)

