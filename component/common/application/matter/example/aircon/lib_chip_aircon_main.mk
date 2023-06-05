SHELL = /bin/bash

# Initialize tool chain
# -------------------------------------------------------------------
BASEDIR := $(shell pwd)
SDKROOTDIR := $(BASEDIR)/../../../../../..
AMEBAZ2_TOOLDIR	= $(SDKROOTDIR)/component/soc/realtek/8710c/misc/iar_utility
CHIPDIR = $(SDKROOTDIR)/third_party/connectedhomeip
OUTPUT_DIR = $(BASEDIR)/build/chip
CODEGENDIR = $(OUTPUT_DIR)/codegen

OS := $(shell uname)

#CROSS_COMPILE = $(ARM_GCC_TOOLCHAIN)/arm-none-eabi-
CROSS_COMPILE = arm-none-eabi-

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

all: lib_main

TARGET=lib_main

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
INCLUDES += -I$(SDKROOTDIR)/component/common/application/matter
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
INCLUDES += -I$(SDKROOTDIR)/component/common/bluetooth/realtek/sdk/example/bt_mesh_multiple_profile/bt_mesh_device_matter
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

INCLUDES += -I$(SDKROOTDIR)/component/common/application/matter/api
INCLUDES += -I$(SDKROOTDIR)/component/common/application/matter/common/bluetooth
INCLUDES += -I$(SDKROOTDIR)/component/common/application/matter/common/bluetooth/bt_matter_adapter
INCLUDES += -I$(SDKROOTDIR)/component/common/application/matter/common/mbedtls
INCLUDES += -I$(SDKROOTDIR)/component/common/application/matter/common/port
INCLUDES += -I$(SDKROOTDIR)/component/common/application/matter/core
INCLUDES += -I$(SDKROOTDIR)/component/common/application/matter/driver
INCLUDES += -I$(SDKROOTDIR)/component/common/application/matter/example/light

# CHIP Include folder list
# -------------------------------------------------------------------
INCLUDES += -I$(CHIPDIR)/zzz_generated/app-common
# INCLUDES += -I$(CHIPDIR)/zzz_generated/lighting-app
# INCLUDES += -I$(CHIPDIR)/zzz_generated/lighting-app/zap-generated
# INCLUDES += -I$(CHIPDIR)/examples/lighting-app/lighting-common
# INCLUDES += -I$(CHIPDIR)/examples/lighting-app/ameba/main/include
# INCLUDES += -I$(CHIPDIR)/examples/lighting-app/ameba/build/chip/gen/include
INCLUDES += -I$(OUTPUT_DIR)/gen/include
INCLUDES += -I$(CHIPDIR)/examples/platform/ameba
INCLUDES += -I$(CHIPDIR)/examples/providers
INCLUDES += -I$(CHIPDIR)/src/include
INCLUDES += -I$(CHIPDIR)/src/lib
INCLUDES += -I$(CHIPDIR)/src
INCLUDES += -I$(CHIPDIR)/third_party/nlassert/repo/include
INCLUDES += -I$(CHIPDIR)/src/app
INCLUDES += -I$(CHIPDIR)/src/app/util
INCLUDES += -I$(CHIPDIR)/src/app/server
INCLUDES += -I$(CHIPDIR)/src/app/clusters/bindings
INCLUDES += -I$(CHIPDIR)/third_party/nlio/repo/include
INCLUDES += -I$(CHIPDIR)/third_party/nlunit-test/repo/src
INCLUDES += -I$(CODEGENDIR)

# Source file list
# -------------------------------------------------------------------

SRC_C =
SRC_C += $(CHIPDIR)/examples/platform/ameba/route_hook/ameba_route_hook.c
SRC_C += $(CHIPDIR)/examples/platform/ameba/route_hook/ameba_route_table.c

SRC_CPP = 

SRC_CPP += $(CHIPDIR)/src/app/server/EchoHandler.cpp
SRC_CPP += $(CHIPDIR)/src/app/server/Dnssd.cpp
SRC_CPP += $(CHIPDIR)/src/app/server/OnboardingCodesUtil.cpp
SRC_CPP += $(CHIPDIR)/src/app/server/Server.cpp
SRC_CPP += $(CHIPDIR)/src/app/server/CommissioningWindowManager.cpp

SRC_CPP += $(CHIPDIR)/src/app/util/attribute-size-util.cpp
SRC_CPP += $(CHIPDIR)/src/app/util/attribute-storage.cpp
SRC_CPP += $(CHIPDIR)/src/app/util/attribute-table.cpp
SRC_CPP += $(CHIPDIR)/src/app/util/binding-table.cpp
SRC_CPP += $(CHIPDIR)/src/app/util/DataModelHandler.cpp
SRC_CPP += $(CHIPDIR)/src/app/util/ember-compatibility-functions.cpp
SRC_CPP += $(CHIPDIR)/src/app/util/ember-print.cpp
SRC_CPP += $(CHIPDIR)/src/app/util/generic-callback-stubs.cpp
SRC_CPP += $(CHIPDIR)/src/app/util/message.cpp
SRC_CPP += $(CHIPDIR)/src/app/util/util.cpp
SRC_CPP += $(CHIPDIR)/src/app/util/error-mapping.cpp
SRC_CPP += $(CHIPDIR)/src/app/util/privilege-storage.cpp

SRC_CPP += $(CHIPDIR)/src/app/reporting/Engine.cpp

SRC_CPP += $(shell cat $(CODEGENDIR)/cluster-file.txt)

SRC_CPP += $(CODEGENDIR)/app/callback-stub.cpp
SRC_CPP += $(CODEGENDIR)/zap-generated/IMClusterCommandHandler.cpp

SRC_CPP += $(CHIPDIR)/zzz_generated/app-common/app-common/zap-generated/attributes/Accessors.cpp
SRC_CPP += $(CHIPDIR)/zzz_generated/app-common/app-common/zap-generated/cluster-objects.cpp

SRC_CPP += $(CHIPDIR)/examples/providers/DeviceInfoProviderImpl.cpp

# Custom light-app src files with porting layer
SRC_CPP += $(SDKROOTDIR)/component/common/application/matter/api/matter_api.cpp
SRC_CPP += $(SDKROOTDIR)/component/common/application/matter/core/matter_core.cpp
SRC_CPP += $(SDKROOTDIR)/component/common/application/matter/core/matter_interaction.cpp
SRC_CPP += $(SDKROOTDIR)/component/common/application/matter/core/matter_ota.cpp
SRC_CPP += $(SDKROOTDIR)/component/common/application/matter/driver/fan_driver.cpp
SRC_CPP += $(SDKROOTDIR)/component/common/application/matter/example/aircon/example_matter_aircon.cpp
SRC_CPP += $(SDKROOTDIR)/component/common/application/matter/example/aircon/matter_drivers.cpp

#lib_version
VER_C += $(TARGET)_version.c

# Generate obj list
# -------------------------------------------------------------------

SRC_O = $(patsubst %.c,%_$(TARGET).o,$(SRC_C))
VER_O = $(patsubst %.c,%_$(TARGET).o,$(VER_C))

SRC_C_LIST = $(notdir $(SRC_C)) $(notdir $(DRAM_C))
OBJ_LIST = $(addprefix $(OBJ_DIR)/,$(patsubst %.c,%_$(TARGET).o,$(SRC_C_LIST)))
DEPENDENCY_LIST = $(addprefix $(OBJ_DIR)/,$(patsubst %.c,%_$(TARGET).d,$(SRC_C_LIST)))

SRC_OO += $(patsubst %.cpp,%_$(TARGET).oo,$(SRC_CPP))
SRC_CPP_LIST = $(notdir $(SRC_CPP))
OBJ_CPP_LIST = $(addprefix $(OBJ_DIR)/,$(patsubst %.cpp,%_$(TARGET).oo,$(SRC_CPP_LIST)))
DEPENDENCY_LIST += $(addprefix $(OBJ_DIR)/,$(patsubst %.cpp,%_$(TARGET).d,$(SRC_CPP_LIST)))

# Compile options
# -------------------------------------------------------------------

CFLAGS =
CFLAGS += -march=armv8-m.main+dsp -mthumb -mcmse -mfloat-abi=soft -D__thumb2__ -g -gdwarf-3 -Os
CFLAGS += -D__ARM_ARCH_8M_MAIN__=1 -gdwarf-3 -fstack-usage -fdata-sections -ffunction-sections 
CFLAGS += -fdiagnostics-color=always -Wall -Wpointer-arith -Wno-write-strings 
CFLAGS += -Wno-maybe-uninitialized --save-temps -c -MMD
CFLAGS += -DCONFIG_PLATFORM_8710C -DCONFIG_BUILD_RAM=1
CFLAGS += -DV8M_STKOVF

# CHIP options
# -------------------------------------------------------------------
CFLAGS += -DCHIP_PROJECT=1
CFLAGS += -DCONFIG_ENABLE_OTA_REQUESTOR=1
CFLAGS += -DCONFIG_ENABLE_AMEBA_FACTORY_DATA=0
CFLAGS += -DCHIP_DEVICE_LAYER_TARGET=Ameba
CFLAGS += -DMBEDTLS_CONFIG_FILE=\"mbedtls_config.h\"
CFLAGS += -DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=\"lib/address_resolve/AddressResolve_DefaultImpl.h\"

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
CFLAGS += -DCHIP_SYSTEM_CONFIG_POSIX_LOCKING=0
CFLAGS += -DINET_CONFIG_ENABLE_IPV4=0

CFLAGS += -DUSE_ZAP_CONFIG
CFLAGS += -DCHIP_HAVE_CONFIG_H

CPPFLAGS := $(CFLAGS)

CPPFLAGS += -DFD_SETSIZE=10
CPPFLAGS += -Wno-sign-compare
CPPFLAGS += -Wno-unused-function
CPPFLAGS += -Wno-unused-but-set-variable
CPPFLAGS += -Wno-unused-variable
CPPFLAGS += -Wno-deprecated-declarations
CPPFLAGS += -Wno-unused-parameter
CPPFLAGS += -Wno-format

CPPFLAGS += -std=gnu++14
#CPPFLAGS += -std=c++14
CPPFLAGS += -fno-rtti

# Compile
# -------------------------------------------------------------------

.PHONY: lib_main
lib_main: prerequirement $(SRC_O) $(DRAM_O) $(SRC_OO)
	$(AR) crv $(BIN_DIR)/$(TARGET).a $(OBJ_CPP_LIST) $(OBJ_LIST) $(VER_O)
	cp $(BIN_DIR)/$(TARGET).a $(SDKROOTDIR)/component/soc/realtek/8710c/misc/bsp/lib/common/GCC/$(TARGET).a

# Manipulate Image
# -------------------------------------------------------------------

.PHONY: manipulate_images
manipulate_images:
	@echo ===========================================================
	@echo Image manipulating
	@echo ===========================================================

# Generate build info
# -------------------------------------------------------------------

.PHONY: prerequirement
prerequirement:
	@rm -f $(TARGET)_version*.o
	@echo const char $(TARGET)_rev[] = \"$(TARGET)_ver_`git rev-parse HEAD`_`date +%Y/%m/%d-%T`\"\; > $(TARGET)_version.c
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(VER_C) -o $(VER_O)
	@if [ ! -d $(ARM_GCC_TOOLCHAIN) ]; then \
		echo ===========================================================; \
		echo Toolchain not found, \"make toolchain\" first!; \
		echo ===========================================================; \
		exit -1; \
	fi
	@echo ===========================================================
	@echo Build $(TARGET)
	@echo ===========================================================
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)
	mkdir -p $(INFO_DIR)

$(SRC_OO): %_$(TARGET).oo : %.cpp | prerequirement
	$(CC) $(CPPFLAGS) $(INCLUDES) -c $< -o $@
	$(CC) $(CPPFLAGS) $(INCLUDES) -c $< -MM -MT $@ -MF $(OBJ_DIR)/$(notdir $(patsubst %.oo,%.d,$@))
	cp $@ $(OBJ_DIR)/$(notdir $@)
	cp $*_$(TARGET).ii $(INFO_DIR)
	cp $*_$(TARGET).s $(INFO_DIR)
	chmod 777 $(OBJ_DIR)/$(notdir $@)

$(SRC_O): %_$(TARGET).o : %.c | prerequirement
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -MM -MT $@ -MF $(OBJ_DIR)/$(notdir $(patsubst %.o,%.d,$@))
	cp $@ $(OBJ_DIR)/$(notdir $@)
	cp $*_$(TARGET).i $(INFO_DIR)
	cp $*_$(TARGET).s $(INFO_DIR)
	chmod 777 $(OBJ_DIR)/$(notdir $@)

-include $(DEPENDENCY_LIST)

.PHONY: clean
clean:
	rm -rf $(TARGET)
	rm -f $(SRC_O) $(DRAM_O) $(VER_O) $(SRC_OO)
	rm -f $(patsubst %.o,%.d,$(SRC_O)) $(patsubst %.o,%.d,$(DRAM_O)) $(patsubst %.o,%.d,$(VER_O)) $(patsubst %.oo,%.d,$(SRC_OO))
	rm -f $(patsubst %.o,%.su,$(SRC_O)) $(patsubst %.o,%.su,$(DRAM_O)) $(patsubst %.o,%.su,$(VER_O)) $(patsubst %.oo,%.su,$(SRC_OO))
	rm -f $(patsubst %.o,%.i,$(SRC_O)) $(patsubst %.o,%.i,$(DRAM_O)) $(patsubst %.o,%.i,$(VER_O)) $(patsubst %.oo,%.ii,$(SRC_OO))
	rm -f $(patsubst %.o,%.s,$(SRC_O)) $(patsubst %.o,%.s,$(DRAM_O)) $(patsubst %.o,%.s,$(VER_O)) $(patsubst %.oo,%.s,$(SRC_OO))
	rm -f *.i
	rm -f *.s
	rm -f $(VER_C)
