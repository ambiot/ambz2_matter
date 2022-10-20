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


# Initialize target name and target object files
# -------------------------------------------------------------------

all: build_info application_tz manipulate_images

#mp: build_info application manipulate_images

TARGET=application_tz
BOOT_BIN_DIR=bootloader/Debug/bin

# Compile options
# -------------------------------------------------------------------

RAMALL_BIN =
OTA_BIN = 

include toolchain.mk

# Compile
# -------------------------------------------------------------------

.PHONY: application_tz
application_tz: prerequirement
	make -f application.s.mk all
	make -f application.ns.mk all

# Manipulate Image
# -------------------------------------------------------------------
	
.PHONY: manipulate_images
manipulate_images: | application_tz
	@echo ===========================================================
	@echo Image manipulating
	@echo ===========================================================
	cp $(AMEBAZ2_BOOTLOADERDIR)/bootloader.axf $(BOOT_BIN_DIR)/bootloader.axf
ifeq ($(findstring Linux, $(OS)), Linux)
	chmod 0774 $(ELF2BIN) $(CHKSUM)
endif
	$(ELF2BIN) keygen keycfg.json
	$(ELF2BIN) convert amebaz2_bootloader.json BOOTLOADER secure_bit=0
	$(ELF2BIN) convert amebaz2_bootloader.json PARTITIONTABLE secure_bit=0
	$(ELF2BIN) convert amebaz2_firmware_tz.json FIRMWARE secure_bit=0
	$(CHKSUM) $(TARGET)/firmware_tz.bin
	$(ELF2BIN) combine $(TARGET)/flash_tz.bin PTAB=partition.bin,BOOT=$(BOOT_BIN_DIR)/bootloader.bin,FW1=$(TARGET)/firmware_tz.bin

# Generate build info
# -------------------------------------------------------------------	

.PHONY: build_info
build_info:
	@echo \#define RTL_FW_COMPILE_TIME RTL8710CFW_COMPILE_TIME\ > .ver
	@echo \#define RTL_FW_COMPILE_DATE RTL8710CFW_COMPILE_DATE\ >> .ver
	@echo \#define UTS_VERSION \"`date +%Y/%m/%d-%T`\" >> .ver
	@echo \#define RTL8710CFW_COMPILE_TIME \"`date +%Y/%m/%d-%T`\" >> .ver
	@echo \#define RTL8710CFW_COMPILE_DATE \"`date +%Y%m%d`\" >> .ver
	@echo \#define RTL8710CFW_COMPILE_BY \"`id -u -n`\" >> .ver
	@echo \#define RTL8710CFW_COMPILE_HOST \"`$(HOSTNAME_APP)`\" >> .ver
	@if [ -x /bin/dnsdomainname ]; then \
		echo \#define RTL8710CFW_COMPILE_DOMAIN \"`dnsdomainname`\"; \
	elif [ -x /bin/domainname ]; then \
		echo \#define RTL8710CFW_COMPILE_DOMAIN \"`domainname`\"; \
	else \
		echo \#define RTL8710CFW_COMPILE_DOMAIN ; \
	fi >> .ver

	@echo \#define RTL8710CFW_COMPILER \"gcc `$(CC) $(CFLAGS) -dumpversion | tr --delete '\r'`\" >> .ver
	@mv -f .ver ../inc/$@.h

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
	mkdir -p $(TARGET)
	mkdir -p $(BOOT_BIN_DIR)

# Only needed for FPGA phase
.PHONY: romdebug
romdebug:
ifeq ($(findstring CYGWIN, $(OS)), CYGWIN) 
#	$(FLASH_TOOLDIR)/Check_Jtag.sh
	cmd /c start $(GDB) -x ./rtl_gdb_debug_jlink.txt
else
	$(GDB) -x ./rtl_gdb_debug_jlink.txt	
endif

.PHONY: flash
flash:
#	@if [ ! -f $(FLASH_TOOLDIR)/rtl_gdb_flash_write.txt ] ; then echo Please do /"make setup GDB_SERVER=[jlink or openocd]/" first; echo && false ; fi
#ifeq ($(findstring CYGWIN, $(OS)), CYGWIN) 
#	$(FLASH_TOOLDIR)/Check_Jtag.sh
#endif
	@if [ ! -e $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_flashloader.txt ]; then \
		echo ===========================================================; \
		echo gdb script not found, \"make setup_tz GDB_SERVER=[jlink, pyocd or openocd]\" first!; \
		echo ===========================================================; \
		exit -1; \
	fi
	chmod +rx $(AMEBAZ2_GCCTOOLDIR)/flashloader.sh
	$(AMEBAZ2_GCCTOOLDIR)/flashloader.sh $(TARGET)/flash_tz.bin
	$(GDB) -x $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_flashloader.txt

.PHONY: debug
debug:
	@if [ ! -e $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_debug.txt ]; then \
		echo ===========================================================; \
		echo gdb script not found, \"make setup_tz GDB_SERVER=[jlink, pyocd or openocd]\" first!; \
		echo ===========================================================; \
		exit -1; \
	fi
	make -f application.ns.mk debug
	make -f application.s.mk debug
	$(GDB) -x $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_debug.txt


.PHONY: setup setup_tz
setup: setup_tz

setup_tz:
	@echo "----------------"
	@echo Setup $(GDB_SERVER)
	@echo "----------------"
ifeq ($(GDB_SERVER), pyocd)
	cp -p $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_debug_pyocd.txt $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_debug.txt
	cp -p $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_flashloader_pyocd.txt $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_flashloader.txt
else ifeq ($(GDB_SERVER), openocd)
	cp -p $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_debug_openocd.txt $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_debug.txt
	cp -p $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_flashloader_openocd.txt $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_flashloader.txt
else
	cp -p $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_debug_jlink.txt $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_debug.txt
	cp -p $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_flashloader_jlink.txt $(AMEBAZ2_GCCTOOLDIR)/rtl_gdb_flashloader.txt
endif
    
.PHONY: clean
clean:
	make -f application.s.mk clean
	make -f application.ns.mk clean
	rm -rf $(TARGET)
	rm -f $(SRC_O) $(SRAM_O) $(ERAM_O)
	rm -f $(patsubst %.o,%.d,$(SRC_O)) $(patsubst %.o,%.d,$(SRAM_O)) $(patsubst %.o,%.d,$(ERAM_O))
	rm -f $(patsubst %.o,%.su,$(SRC_O)) $(patsubst %.o,%.su,$(SRAM_O)) $(patsubst %.o,%.su,$(ERAM_O))
	rm -f *.i
	rm -f *.s

