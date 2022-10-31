all: is

OS := $(shell uname)
LBITS := $(shell getconf LONG_BIT)

.PHONY: toolchain
toolchain:
	@echo Toolchain unzipping...
ifeq ($(findstring CYGWIN, $(OS)), CYGWIN)
ifneq ("$(LBITS)", "32")
	@echo ONLY 32-BIT CYGWIN IS SUPPORTED!
	@exit -1
endif
	if [ ! -f ../../../tools/arm-none-eabi-gcc/asdk-6.4.1-cygwin-newlib-build-2778-i686.tar.bz2 ] ; then cat ../../../tools/arm-none-eabi-gcc/asdk-6.4.1-cygwin-newlib-build-2778-i686.tar.bz2* > ../../../tools/arm-none-eabi-gcc/asdk-6.4.1-cygwin-newlib-build-2778-i686.tar.bz2; fi;\
	if [ ! -d ../../../tools/arm-none-eabi-gcc/asdk ] ; then mkdir ../../../tools/arm-none-eabi-gcc/asdk; fi;\
	if [ ! -d ../../../tools/arm-none-eabi-gcc/asdk/cygwin ] ; then tar -jxf ../../../tools/arm-none-eabi-gcc/asdk-6.4.1-cygwin-newlib-build-2778-i686.tar.bz2 -C ../../../tools/arm-none-eabi-gcc/ ; mv ../../../tools/arm-none-eabi-gcc/asdk-6.4.1/cygwin ../../../tools/arm-none-eabi-gcc/asdk/cygwin ;fi
endif

ifeq ($(findstring MINGW32, $(OS)), MINGW32)
	if [ ! -f /bin/unzip.exe ] ; then mingw-get.exe install msys-unzip; fi;\
	if [ ! -d ../../../tools/arm-none-eabi-gcc/asdk ] ; then mkdir ../../../tools/arm-none-eabi-gcc/asdk; fi;\
	if [ ! -d ../../../tools/arm-none-eabi-gcc/asdk/mingw32 ] ; then unzip -q -u ../../../tools/arm-none-eabi-gcc/asdk64-6.4.1-mingw32-newlib-build-3026.zip ; mv asdk-6.4.1/mingw32 ../../../tools/arm-none-eabi-gcc/asdk/mingw32 ; rm -rf asdk-6.4.1 ;fi
endif

ifeq ($(findstring Linux, $(OS)), Linux)
ifneq ("$(LBITS)", "64")
	@echo ONLY 64-BIT LINUX IS SUPPORTED!
	@exit -1
endif
	if [ ! -f ../../../tools/arm-none-eabi-gcc/asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2 ] ; then cat ../../../tools/arm-none-eabi-gcc/asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2* > ../../../tools/arm-none-eabi-gcc/asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2; fi;\
	if [ ! -d ../../../tools/arm-none-eabi-gcc/asdk ] ; then mkdir ../../../tools/arm-none-eabi-gcc/asdk; fi;\
	if [ ! -d ../../../tools/arm-none-eabi-gcc/asdk/linux ] ; then tar -jxf ../../../tools/arm-none-eabi-gcc/asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2 -C ../../../tools/arm-none-eabi-gcc/ ; mv ../../../tools/arm-none-eabi-gcc/asdk-10.3.0/linux ../../../tools/arm-none-eabi-gcc/asdk/linux ; rmdir ../../../tools/arm-none-eabi-gcc/asdk-10.3.0; fi
endif
	@echo Toolchain unzip done!


.PHONY: lib_all
lib_all: toolchain
	python3 ameba_zap_cluster_list.py --zap_file ../../../third_party/connectedhomeip/examples/all-clusters-app/all-clusters-common/all-clusters-app.zap
	@$(MAKE) -f lib_chip.mk all
	@$(MAKE) -f lib_chip_main.mk all

.PHONY: light
light: toolchain
	python3 ameba_zap_cluster_list.py --zap_file ../../../third_party/connectedhomeip/examples/lighting-app/lighting-common/lighting-app.zap
	@$(MAKE) -f lib_chip_light_core.mk all
	@$(MAKE) -f lib_chip_light_main.mk all

.PHONY: switch
switch: toolchain
	python3 ameba_zap_cluster_list.py --zap_file ../../../third_party/connectedhomeip/examples/light-switch-app/light-switch-common/light-switch-app.zap
	@$(MAKE) -f lib_chip_switch_core.mk all
	@$(MAKE) -f lib_chip_switch_main.mk all

.PHONY: otar
otar: toolchain
	python3 ameba_zap_cluster_list.py --zap_file ../../../third_party/connectedhomeip/examples/ota-requestor-app/ota-requestor-common/ota-requestor-app.zap
	@$(MAKE) -f lib_chip_otar_core.mk all
	@$(MAKE) -f lib_chip_otar_main.mk all

.PHONY: chef
chef: toolchain
	python3 ameba_zap_cluster_list.py --zap_file ../../../third_party/connectedhomeip/examples/chef/devices/$(SAMPLE_NAME).zap
	@$(MAKE) -f lib_chip_chef_core.mk all
	@$(MAKE) -f lib_chip_chef_main.mk all

.PHONY: is
is: toolchain
	@$(MAKE) -f application.is.mk

.PHONY: mp
mp: toolchain
	@$(MAKE) -f application.is.mk mp

.PHONY: clean
clean:
	@$(MAKE) -f lib_chip.mk clean
	@$(MAKE) -f lib_chip_main.mk clean
	@$(MAKE) -f lib_chip_light_core.mk clean
	@$(MAKE) -f lib_chip_light_main.mk clean
	@$(MAKE) -f lib_chip_switch_core.mk clean
	@$(MAKE) -f lib_chip_switch_main.mk clean
	@$(MAKE) -f lib_chip_otar_core.mk clean
	@$(MAKE) -f lib_chip_otar_main.mk clean
	@$(MAKE) -f lib_chip_chef_core.mk clean
	@$(MAKE) -f lib_chip_chef_main.mk clean
	@$(MAKE) -f application.is.mk clean

debug: toolchain
	@$(MAKE) -f application.is.mk debug

flash: toolchain
	@$(MAKE) -f application.is.mk flash
	
setup:
	@$(MAKE) -f application.is.mk $(MAKECMDGOALS)	
