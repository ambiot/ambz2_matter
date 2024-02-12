BASEDIR := $(shell pwd)
SDKROOTDIR := $(BASEDIR)/../../../../../..
AMEBAZ2_TOOLDIR	= $(SDKROOTDIR)/component/soc/realtek/8710c/misc/iar_utility
CHIPDIR = $(SDKROOTDIR)/third_party/connectedhomeip
OUTPUT_DIR = $(CHIPDIR)/examples/lighting-app/ameba/build/chip
MATTER_TOOLDIR = $(SDKROOTDIR)/tools/matter

LIGHTING_FILE = $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/cluster-file.txt
LIGHTING_ZAP = $(CHIPDIR)/examples/lighting-app/lighting-common/lighting-app.zap

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
	if [ ! -f $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk-6.4.1-cygwin-newlib-build-2778-i686.tar.bz2 ] ; then cat $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk-6.4.1-cygwin-newlib-build-2778-i686.tar.bz2* > $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk-6.4.1-cygwin-newlib-build-2778-i686.tar.bz2; fi;\
	if [ ! -d $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk ] ; then mkdir $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk; fi;\
	if [ ! -d $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk/cygwin ] ; then tar -jxf $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk-6.4.1-cygwin-newlib-build-2778-i686.tar.bz2 -C $(SDKROOTDIR)/tools/arm-none-eabi-gcc/ ; mv $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk-6.4.1/cygwin $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk/cygwin ;fi
endif

ifeq ($(findstring MINGW32, $(OS)), MINGW32)
	if [ ! -f /bin/unzip.exe ] ; then mingw-get.exe install msys-unzip; fi;\
	if [ ! -d $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk ] ; then mkdir $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk; fi;\
	if [ ! -d $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk/mingw32 ] ; then unzip -q -u $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk64-6.4.1-mingw32-newlib-build-3026.zip ; mv asdk-6.4.1/mingw32 $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk/mingw32 ; rm -rf asdk-6.4.1 ;fi
endif

ifeq ($(findstring Linux, $(OS)), Linux)
ifneq ("$(LBITS)", "64")
	@echo ONLY 64-BIT LINUX IS SUPPORTED!
	@exit -1
endif
	if [ ! -f $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2 ] ; then cat $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2* > $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2; fi;\
	if [ ! -d $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk ] ; then mkdir $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk; fi;\
	if [ ! -d $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk/linux ] ; then tar -jxf $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2 -C $(SDKROOTDIR)/tools/arm-none-eabi-gcc/ ; mv $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk-10.3.0/linux $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk/linux ; rmdir $(SDKROOTDIR)/tools/arm-none-eabi-gcc/asdk-10.3.0; fi
endif
	@echo Toolchain unzip done!

$(LIGHTING_FILE): $(LIGHTING_ZAP)
	@mkdir -p $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json --output-dir examples/lighting-app/ameba/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/lighting-app/ameba/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/lighting-app/lighting-common/lighting-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/cluster-file.txt
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)
	@cp endpoint_config.h $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/zap-generated
	@cp gen_config.h $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/zap-generated

.PHONY: light_dm
light_dm: toolchain  $(LIGHTING_FILE)
	export 
	$(MAKE) -f lib_chip_light_core.mk all
	$(MAKE) -f lib_chip_light_main.mk all

.PHONY: clean
clean:
	@$(MAKE) -f lib_chip_light_main.mk clean
	@$(MAKE) -f lib_chip_light_core.mk clean
