
# Initialize tool chain
# -------------------------------------------------------------------

AMEBAZ2_TOOLDIR	= ../../../component/soc/realtek/8710c/misc/iar_utility
AMEBAZ2_GCCTOOLDIR	= ../../../component/soc/realtek/8710c/misc/gcc_utility
AMEBAZ2_BSPDIR = ../../../component/soc/realtek/8710c/misc/bsp
AMEBAZ2_BOOTLOADERDIR = $(AMEBAZ2_BSPDIR)/image
AMEBAZ2_ROMSYMDIR = $(AMEBAZ2_BSPDIR)/ROM

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

LDSCRIPT := ./rtl8710c_img1.ld

# Initialize target name and target object files
# -------------------------------------------------------------------

all: bootloader manipulate_images

TARGET=bootloader

OBJ_DIR=$(TARGET)/Debug/obj
BIN_DIR=$(TARGET)/Debug/bin
INFO_DIR=$(TARGET)/Debug/info

# Include folder list
# -------------------------------------------------------------------

INCLUDES =
INCLUDES += -I../inc

INCLUDES += -I../../../component/common/api
INCLUDES += -I../../../component/common/api/platform
INCLUDES += -I../../../component/common/mbed/api
INCLUDES += -I../../../component/common/mbed/hal
INCLUDES += -I../../../component/common/mbed/hal_ext
INCLUDES += -I../../../component/common/mbed/targets/hal/rtl8710c

INCLUDES += -I../../../component/soc/realtek/8710c/cmsis/rtl8710c/include
INCLUDES += -I../../../component/soc/realtek/8710c/cmsis/rtl8710c/lib/include
INCLUDES += -I../../../component/soc/realtek/8710c/fwlib/include
INCLUDES += -I../../../component/soc/realtek/8710c/fwlib/lib/include
INCLUDES += -I../../../component/soc/realtek/8710c/cmsis/cmsis-core/include
INCLUDES += -I../../../component/soc/realtek/8710c/cmsis/rtl8710c/include
INCLUDES += -I../../../component/soc/realtek/8710c/app/rtl_printf/include
INCLUDES += -I../../../component/soc/realtek/8710c/app/shell
INCLUDES += -I../../../component/soc/realtek/8710c/app/stdio_port
INCLUDES += -I../../../component/soc/realtek/8710c/misc/utilities/include
INCLUDES += -I../../../component/soc/realtek/8710c/misc/platform/lzma

INCLUDES += -I../../../component/os/freertos
INCLUDES += -I../../../component/os/freertos/freertos_v10.0.1/Source/include
INCLUDES += -I../../../component/os/freertos/freertos_v10.0.1/Source/portable/GCC/ARM_RTL8710C
INCLUDES += -I../../../component/os/os_dep/include

# Source file list
# -------------------------------------------------------------------

SRC_C =

#mpu
SRC_C += ../../../component/soc/realtek/8710c/cmsis/rtl8710c/source/ram/mpu_config.c

#libc api wrapper
SRC_C += ../../../component/soc/realtek/8710c/misc/utilities/source/ram/libc_wrap.c

#console
SRC_C += ../../../component/soc/realtek/8710c/app/shell/cmd_shell.c

#peripheral - hal
SRC_C += ../../../component/soc/realtek/8710c/fwlib/source/ram_s/hal_efuse.c
SRC_C += ../../../component/soc/realtek/8710c/fwlib/source/ram_s/hal_efuse_nsc.c
SRC_C += ../../../component/soc/realtek/8710c/fwlib/source/ram_s/hal_pinmux_nsc.c
SRC_C += ../../../component/soc/realtek/8710c/fwlib/source/ram/hal_gdma.c
SRC_C += ../../../component/soc/realtek/8710c/fwlib/source/ram/hal_uart.c
SRC_C += ../../../component/soc/realtek/8710c/fwlib/source/ram/hal_misc.c
SRC_C += ../../../component/soc/realtek/8710c/fwlib/source/ram/hal_gpio.c
SRC_C += ../../../component/soc/realtek/8710c/fwlib/source/ram_ns/hal_flash.c
SRC_C += ../../../component/soc/realtek/8710c/fwlib/source/ram_ns/hal_i2c.c
SRC_C += ../../../component/soc/realtek/8710c/fwlib/source/ram_ns/hal_pwm.c
SRC_C += ../../../component/soc/realtek/8710c/fwlib/source/ram_ns/hal_sdio_dev.c
SRC_C += ../../../component/soc/realtek/8710c/fwlib/source/ram_ns/hal_spic.c
SRC_C += ../../../component/soc/realtek/8710c/fwlib/source/ram_ns/hal_ssi.c

# Generate obj list
# -------------------------------------------------------------------

SRC_O = $(patsubst %.c,%_$(TARGET).o,$(SRC_C))

SRC_C_LIST = $(notdir $(SRC_C))
OBJ_LIST = $(addprefix $(OBJ_DIR)/,$(patsubst %.c,%_$(TARGET).o,$(SRC_C_LIST)))
DEPENDENCY_LIST = $(addprefix $(OBJ_DIR)/,$(patsubst %.c,%_$(TARGET).d,$(SRC_C_LIST)))


# Compile options
# -------------------------------------------------------------------

CFLAGS =
CFLAGS += -march=armv8-m.main+dsp -mthumb -mcmse -mfloat-abi=soft -D__thumb2__ -g -gdwarf-3 -Os
CFLAGS += -D__ARM_ARCH_8M_MAIN__=1 -gdwarf-3 -fstack-usage -fdata-sections -ffunction-sections
CFLAGS += -fdiagnostics-color=always -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Wno-write-strings
CFLAGS += -Wno-maybe-uninitialized --save-temps -c -MMD
CFLAGS += -DCONFIG_PLATFORM_8710C -DCONFIG_BUILD_RAM=1
CFLAGS += -DCONFIG_BUILD_BOOT=1 
LIBCFLAGS = 
LIBCFLAGS += $(CFLAGS)
LIBCFLAGS += -DCONFIG_BUILD_LIB=1

LFLAGS = 
LFLAGS += -Os -march=armv8-m.main+dsp -mthumb -mcmse -mfloat-abi=soft -nostartfiles -nodefaultlibs -nostdlib -specs=nosys.specs 
LFLAGS += -Wl,--gc-sections -Wl,--warn-section-align -Wl,--cref -Wl,--build-id=none -Wl,--use-blx
LFLAGS += -Wl,-Map=$(BIN_DIR)/$(TARGET).map
# libc api wrapper
LFLAGS += -Wl,-wrap,strcat  -Wl,-wrap,strchr   -Wl,-wrap,strcmp
LFLAGS += -Wl,-wrap,strncmp -Wl,-wrap,strnicmp -Wl,-wrap,strcpy
LFLAGS += -Wl,-wrap,strncpy -Wl,-wrap,strlcpy  -Wl,-wrap,strlen
LFLAGS += -Wl,-wrap,strnlen -Wl,-wrap,strncat  -Wl,-wrap,strpbrk
LFLAGS += -Wl,-wrap,strspn  -Wl,-wrap,strstr   -Wl,-wrap,strtok
LFLAGS += -Wl,-wrap,strxfrm -Wl,-wrap,strsep   -Wl,-wrap,strtod
LFLAGS += -Wl,-wrap,strtof  -Wl,-wrap,strtold  -Wl,-wrap,strtoll
LFLAGS += -Wl,-wrap,strtoul -Wl,-wrap,strtoull -Wl,-wrap,atoi
LFLAGS += -Wl,-wrap,atoui   -Wl,-wrap,atol     -Wl,-wrap,atoul
LFLAGS += -Wl,-wrap,atoull  -Wl,-wrap,atof
LFLAGS += -Wl,-wrap,malloc  -Wl,-wrap,realloc  -Wl,-wrap,free
LFLAGS += -Wl,-wrap,memcmp  -Wl,-wrap,memcpy
LFLAGS += -Wl,-wrap,memmove -Wl,-wrap,memset
LFLAGS += -Wl,-wrap,printf  -Wl,-wrap,sprintf
LFLAGS += -Wl,-wrap,snprintf  -Wl,-wrap,vsnprintf

LIBFLAGS =
LIBFLAGS += -L$(AMEBAZ2_ROMSYMDIR)
LIBFLAGS += -L../../../component/soc/realtek/8710c/misc/bsp/lib/common/GCC
LIBFLAGS += -Wl,--whole-archive ../../../component/soc/realtek/8710c/misc/bsp/lib/common/GCC/lib_boot.a -Wl,--no-whole-archive
all: LIBFLAGS +=

include toolchain.mk

# Compile
# -------------------------------------------------------------------

.PHONY: bootloader
bootloader: prerequirement $(SRC_O)
	$(LD) $(LFLAGS) -o $(BIN_DIR)/$(TARGET).axf  $(OBJ_LIST) $(ROMIMG) $(LIBFLAGS) -T$(LDSCRIPT)
	$(OBJDUMP) -d $(BIN_DIR)/$(TARGET).axf > $(BIN_DIR)/$(TARGET).asm

# Manipulate Image
# -------------------------------------------------------------------

.PHONY: manipulate_images
manipulate_images: | bootloader
	@echo ===========================================================
	@echo Image manipulating
	@echo ===========================================================
ifeq ($(findstring Linux, $(OS)), Linux)
	chmod 0774 $(ELF2BIN)
endif
	$(ELF2BIN) keygen keycfg.json
	$(ELF2BIN) convert amebaz2_bootloader.json BOOTLOADER
	cp $(BIN_DIR)/$(TARGET).axf $(AMEBAZ2_BOOTLOADERDIR)/$(TARGET).axf
	#cp $(BIN_DIR)/$(TARGET).bin $(AMEBAZ2_BOOTLOADERDIR)/$(TARGET).bin

# Generate build info
# -------------------------------------------------------------------

.PHONY: prerequirement
prerequirement:
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

$(SRC_O): %_$(TARGET).o : %.c | prerequirement
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -MM -MT $@ -MF $(OBJ_DIR)/$(notdir $(patsubst %.o,%.d,$@))
	cp $@ $(OBJ_DIR)/$(notdir $@)
	mv $(notdir $*.i) $(INFO_DIR)
	mv $(notdir $*.s) $(INFO_DIR)
	chmod 777 $(OBJ_DIR)/$(notdir $@)

#$(SRC_LIB_O): %_$(TARGET).o : %.c | prerequirement
#	$(CC) $(LIBCFLAGS) $(INCLUDES) -c $< -o $@
#	$(CC) $(LIBCFLAGS) $(INCLUDES) -c $< -MM -MT $@ -MF $(OBJ_DIR)/$(notdir $(patsubst %.o,%.d,$@))
#	cp $@ $(OBJ_DIR)/$(notdir $@)
#	mv $(notdir $*.i) $(INFO_DIR)
#	mv $(notdir $*.s) $(INFO_DIR)
#	chmod 777 $(OBJ_DIR)/$(notdir $@)

-include $(DEPENDENCY_LIST)

# Generate build info
# -------------------------------------------------------------------	
#ifeq (setup,$(firstword $(MAKECMDGOALS)))
#  # use the rest as arguments for "run"
#  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
#  # ...and turn them into do-nothing targets
#  $(eval $(RUN_ARGS):;@:)
#endif

#.PHONY: setup
#setup:
#	@echo "----------------"
#	@echo Setup $(GDB_SERVER)
#	@echo "----------------"
#ifeq ($(GDB_SERVER), openocd)
#	cp -p $(FLASH_TOOLDIR)/rtl_gdb_debug_openocd.txt $(FLASH_TOOLDIR)/rtl_gdb_debug.txt
#	cp -p $(FLASH_TOOLDIR)/rtl_gdb_ramdebug_openocd.txt $(FLASH_TOOLDIR)/rtl_gdb_ramdebug.txt
#	cp -p $(FLASH_TOOLDIR)/rtl_gdb_flash_write_openocd.txt $(FLASH_TOOLDIR)/rtl_gdb_flash_write.txt
#else
#	cp -p $(FLASH_TOOLDIR)/rtl_gdb_debug_jlink.txt $(FLASH_TOOLDIR)/rtl_gdb_debug.txt
#	cp -p $(FLASH_TOOLDIR)/rtl_gdb_ramdebug_jlink.txt $(FLASH_TOOLDIR)/rtl_gdb_ramdebug.txt
#	cp -p $(FLASH_TOOLDIR)/rtl_gdb_flash_write_jlink.txt $(FLASH_TOOLDIR)/rtl_gdb_flash_write.txt
#endif

#.PHONY: flashburn
#flashburn:
#	@if [ ! -f $(FLASH_TOOLDIR)/rtl_gdb_flash_write.txt ] ; then echo Please do \"make setup GDB_SERVER=[jlink or openocd]\" first; echo && false ; fi
#ifeq ($(findstring CYGWIN, $(OS)), CYGWIN) 
#	$(FLASH_TOOLDIR)/Check_Jtag.sh
#endif
#	cp	$(FLASH_TOOLDIR)/target_NORMALB.axf $(FLASH_TOOLDIR)/target_NORMAL.axf
#	chmod 777 $(FLASH_TOOLDIR)/target_NORMAL.axf
#	chmod +rx $(FLASH_TOOLDIR)/SetupGDB_NORMAL.sh
#	$(FLASH_TOOLDIR)/SetupGDB_NORMAL.sh
#	$(GDB) -x $(FLASH_TOOLDIR)/rtl_gdb_flash_write.txt

#.PHONY: debug
#debug:
#	@if [ ! -f $(FLASH_TOOLDIR)/rtl_gdb_debug.txt ] ; then echo Please do \"make setup GDB_SERVER=[jlink or openocd]\" first; echo && false ; fi
#ifeq ($(findstring CYGWIN, $(OS)), CYGWIN) 
#	$(FLASH_TOOLDIR)/Check_Jtag.sh
#	cmd /c start $(GDB) -x $(FLASH_TOOLDIR)/rtl_gdb_debug.txt
#else
#	$(GDB) -x $(FLASH_TOOLDIR)/rtl_gdb_debug.txt
#endif

#.PHONY: ramdebug
#ramdebug:
#	@if [ ! -f $(FLASH_TOOLDIR)/rtl_gdb_ramdebug.txt ] ; then echo Please do \"make setup GDB_SERVER=[jlink or openocd]\" first; echo && false ; fi
#ifeq ($(findstring CYGWIN, $(OS)), CYGWIN) 
#	$(FLASH_TOOLDIR)/Check_Jtag.sh
#	cmd /c start $(GDB) -x $(FLASH_TOOLDIR)/rtl_gdb_ramdebug.txt	
#else
#	$(GDB) -x $(FLASH_TOOLDIR)/rtl_gdb_ramdebug.txt	
#endif

.PHONY: clean
clean:
	rm -rf $(TARGET)
	rm -f $(SRC_O) $(SRC_LIB_O)
	rm -f $(patsubst %.o,%.d,$(SRC_O)) $(patsubst %.o,%.d,$(SRC_LIB_O))
	rm -f $(patsubst %.o,%.su,$(SRC_O)) $(patsubst %.o,%.su,$(SRC_LIB_O))
	rm -f *.i
	rm -f *.s
