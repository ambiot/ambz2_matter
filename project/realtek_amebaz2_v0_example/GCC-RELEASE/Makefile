all: is

OS := $(shell uname)
LBITS := $(shell getconf LONG_BIT)

CHIPDIR = ../../../third_party/connectedhomeip
MATTER_TOOLDIR = ../../../tools/matter

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

.PHONY: mkdir_codegen
mkdir_codegen:
	if [ ! -d "$(MATTER_TOOLDIR)/codegen_helpers/gen" ]; then mkdir "$(MATTER_TOOLDIR)/codegen_helpers/gen"; fi

.PHONY: check_zap
check_zap:
	cd $(MATTER_TOOLDIR)/codegen_helpers/ && \
	./check_zap.sh

.PHONY: is
is: toolchain
	cp partition_default.json partition.json
	@$(MAKE) -f application.is.mk

.PHONY: mp
mp: toolchain
	@$(MAKE) -f application.is.mk mp

.PHONY: clean
clean:
	@$(MAKE) -f application.is.mk clean

.PHONY: all_clusters
all_clusters: toolchain check_zap
	export ZAP_INSTALL_PATH="$(MATTER_TOOLDIR)/codegen_helpers/zap" && \
	mkdir -p $(CHIPDIR)/examples/all-clusters-app/ameba/build/chip/codegen/zap-generated && \
	python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl.json --output-dir examples/all-clusters-app/ameba/build/chip/codegen/zap-generated examples/all-clusters-app/all-clusters-common/all-clusters-app.zap && \
	python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/all-clusters-app/ameba/build/chip/codegen/zap-generated examples/all-clusters-app/all-clusters-common/all-clusters-app.zap && \
	python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/all-clusters-app/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/all-clusters-app/all-clusters-common/all-clusters-app.matter && \
	python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $(CHIPDIR)/examples/all-clusters-app/all-clusters-common/all-clusters-app.zap > $(CHIPDIR)/examples/all-clusters-app/ameba/build/chip/codegen/cluster-file.txt && \
	python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $(CHIPDIR)/examples/all-clusters-app/ameba/build/chip/codegen/cluster-file.txt --chip_path $(CHIPDIR) && \
	$(MAKE) -f lib_chip.mk all
	$(MAKE) -f lib_chip_main.mk all

.PHONY: light
light: toolchain check_zap
	export ZAP_INSTALL_PATH="$(MATTER_TOOLDIR)/codegen_helpers/zap" && \
	mkdir -p $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/zap-generated && \
	python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl.json --output-dir examples/lighting-app/ameba/build/chip/codegen/zap-generated examples/lighting-app/lighting-common/lighting-app.zap && \
	python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/lighting-app/ameba/build/chip/codegen/zap-generated examples/lighting-app/lighting-common/lighting-app.zap && \
	python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/lighting-app/lighting-common/lighting-app.matter && \
	python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $(CHIPDIR)/examples/lighting-app/lighting-common/lighting-app.zap > $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/cluster-file.txt && \
	python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/cluster-file.txt --chip_path $(CHIPDIR) && \
	$(MAKE) -f lib_chip_light_core.mk all
	$(MAKE) -f lib_chip_light_main.mk all

.PHONY: switch
switch: toolchain check_zap
	export ZAP_INSTALL_PATH="$(MATTER_TOOLDIR)/codegen_helpers/zap" && \
	mkdir -p $(CHIPDIR)/examples/light-switch-app/ameba/build/chip/codegen/zap-generated && \
	python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl.json --output-dir examples/light-switch-app/ameba/build/chip/codegen/zap-generated examples/light-switch-app/light-switch-common/light-switch-app.zap && \
	python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/light-switch-app/ameba/build/chip/codegen/zap-generated examples/light-switch-app/light-switch-common/light-switch-app.zap && \
	python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/light-switch-app/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/light-switch-app/light-switch-common/light-switch-app.matter && \
	python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $(CHIPDIR)/examples/light-switch-app/light-switch-common/light-switch-app.zap > $(CHIPDIR)/examples/light-switch-app/ameba/build/chip/codegen/cluster-file.txt && \
	python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $(CHIPDIR)/examples/light-switch-app/ameba/build/chip/codegen/cluster-file.txt --chip_path $(CHIPDIR) && \
	$(MAKE) -f lib_chip_switch_core.mk all
	$(MAKE) -f lib_chip_switch_main.mk all

.PHONY: otar
otar: toolchain check_zap
	export ZAP_INSTALL_PATH="$(MATTER_TOOLDIR)/codegen_helpers/zap" && \
	mkdir -p $(CHIPDIR)/examples/ota-requestor-app/ameba/build/chip/codegen/zap-generated && \
	python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl.json --output-dir examples/ota-requestor-app/ameba/build/chip/codegen/zap-generated examples/ota-requestor-app/ota-requestor-common/ota-requestor-app.zap && \
	python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/ota-requestor-app/ameba/build/chip/codegen/zap-generated examples/ota-requestor-app/ota-requestor-common/ota-requestor-app.zap && \
	python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/ota-requestor-app/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/ota-requestor-app/ota-requestor-common/ota-requestor-app.matter && \
	python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $(CHIPDIR)/examples/ota-requestor-app/ota-requestor-common/ota-requestor-app.zap > $(CHIPDIR)/examples/ota-requestor-app/ameba/build/chip/codegen/cluster-file.txt && \
	python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $(CHIPDIR)/examples/ota-requestor-app/ameba/build/chip/codegen/cluster-file.txt --chip_path $(CHIPDIR) && \
	$(MAKE) -f lib_chip_otar_core.mk all
	$(MAKE) -f lib_chip_otar_main.mk all

.PHONY: chef
chef: toolchain check_zap
	export ZAP_INSTALL_PATH="$(MATTER_TOOLDIR)/codegen_helpers/zap" && \
	mkdir -p $(CHIPDIR)/examples/chef/ameba/build/chip/codegen/zap-generated && \
	python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl.json --output-dir examples/chef/ameba/build/chip/codegen/zap-generated examples/chef/devices/$(SAMPLE_NAME).zap && \
	python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/chef/ameba/build/chip/codegen/zap-generated examples/chef/devices/$(SAMPLE_NAME).zap && \
	python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/chef/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/chef/devices/$(SAMPLE_NAME).matter && \
	python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $(CHIPDIR)/examples/chef/devices/$(SAMPLE_NAME).zap > $(CHIPDIR)/examples/chef/ameba/build/chip/codegen/cluster-file.txt && \
	python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $(CHIPDIR)/examples/chef/ameba/build/chip/codegen/cluster-file.txt --chip_path $(CHIPDIR) && \
	$(MAKE) -f lib_chip_chef_core.mk all
	$(MAKE) -f lib_chip_chef_main.mk all

.PHONY: is_matter
is_matter: toolchain
	cp partition_matter.json partition.json
	@$(MAKE) -f application.is.matter.mk
	
.PHONY: clean_matter
clean_matter:
	@$(MAKE) -f lib_chip_main.mk clean
	@$(MAKE) -f lib_chip.mk clean
	@$(MAKE) -f lib_chip_light_main.mk clean
	@$(MAKE) -f lib_chip_light_core.mk clean
	@$(MAKE) -f lib_chip_switch_main.mk clean
	@$(MAKE) -f lib_chip_switch_core.mk clean
	@$(MAKE) -f lib_chip_otar_main.mk clean
	@$(MAKE) -f lib_chip_otar_core.mk clean
	@$(MAKE) -f lib_chip_chef_main.mk clean
	@$(MAKE) -f lib_chip_chef_core.mk clean
	@$(MAKE) -f application.is.matter.mk clean

debug: toolchain
	@$(MAKE) -f application.is.mk debug

flash: toolchain
	@$(MAKE) -f application.is.mk flash
	
setup:
	@$(MAKE) -f application.is.mk $(MAKECMDGOALS)	
