all: is

OS := $(shell uname)
LBITS := $(shell getconf LONG_BIT)

CHIPDIR = $(shell pwd)/../../../third_party/connectedhomeip
MATTER_TOOLDIR = ../../../tools/matter

ALL_CLUSTERS_FILE = $(CHIPDIR)/examples/all-clusters-app/ameba/build/chip/codegen/cluster-file.txt
ALL_CLUSTERS_ZAP = $(CHIPDIR)/examples/all-clusters-app/all-clusters-common/all-clusters-app.zap
LIGHTING_FILE = $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/cluster-file.txt
LIGHTING_ZAP = $(CHIPDIR)/examples/lighting-app/lighting-common/lighting-app.zap
LIGHT_SWITCH_FILE = $(CHIPDIR)/examples/light-switch-app/ameba/build/chip/codegen/cluster-file.txt
LIGHT_SWITCH_ZAP = $(CHIPDIR)/examples/light-switch-app/light-switch-common/light-switch-app.zap
OTAR_FILE = $(CHIPDIR)/examples/ota-requestor-app/ameba/build/chip/codegen/cluster-file.txt
OTAR_ZAP = $(CHIPDIR)/examples/ota-requestor-app/ota-requestor-common/ota-requestor-app.zap
CHEF_FILE = $(CHIPDIR)/examples/chef/ameba/build/chip/codegen/cluster-file.txt
CHEF_ZAP = $(CHIPDIR)/examples/chef/devices/$(SAMPLE_NAME).zap

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

$(ALL_CLUSTERS_FILE): $(ALL_CLUSTERS_ZAP)
	@mkdir -p $(CHIPDIR)/examples/all-clusters-app/ameba/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json --output-dir examples/all-clusters-app/ameba/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/all-clusters-app/ameba/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/all-clusters-app/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/all-clusters-app/all-clusters-common/all-clusters-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $(CHIPDIR)/examples/all-clusters-app/ameba/build/chip/codegen/cluster-file.txt
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

$(LIGHTING_FILE): $(LIGHTING_ZAP)
	@mkdir -p $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json --output-dir examples/lighting-app/ameba/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/lighting-app/ameba/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/lighting-app/lighting-common/lighting-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/cluster-file.txt
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

$(LIGHT_SWITCH_FILE): $(LIGHT_SWITCH_ZAP)
	@mkdir -p $(CHIPDIR)/examples/light-switch-app/ameba/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json --output-dir examples/light-switch-app/ameba/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/light-switch-app/ameba/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/light-switch-app/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/light-switch-app/light-switch-common/light-switch-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $(CHIPDIR)/examples/light-switch-app/ameba/build/chip/codegen/cluster-file.txt
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

$(OTAR_FILE): $(OTAR_ZAP)
	@mkdir -p $(CHIPDIR)/examples/ota-requestor-app/ameba/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json --output-dir examples/ota-requestor-app/ameba/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/ota-requestor-app/ameba/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/ota-requestor-app/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/ota-requestor-app/ota-requestor-common/ota-requestor-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $(CHIPDIR)/examples/ota-requestor-app/ameba/build/chip/codegen/cluster-file.txt
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

$(CHEF_FILE): $(CHEF_ZAP)
	@mkdir -p $(CHIPDIR)/examples/chef-app/ameba/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json --output-dir examples/chef-app/ameba/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/chef-app/ameba/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/chef-app/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/chef-app/chef-common/chef-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $(CHIPDIR)/examples/chef-app/ameba/build/chip/codegen/cluster-file.txt
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

.PHONY: all_clusters
all_clusters: toolchain $(ALL_CLUSTERS_FILE)
	$(MAKE) -f lib_chip.mk all
	$(MAKE) -f lib_chip_main.mk all

.PHONY: light
light: toolchain  $(LIGHTING_FILE)
	$(MAKE) -f lib_chip_light_core.mk all
	$(MAKE) -f lib_chip_light_main.mk all

.PHONY: switch
switch: toolchain $(LIGHT_SWITCH_FILE)
	$(MAKE) -f lib_chip_switch_core.mk all
	$(MAKE) -f lib_chip_switch_main.mk all

.PHONY: otar
otar: toolchain $(OTAR_FILE)
	$(MAKE) -f lib_chip_otar_core.mk all
	$(MAKE) -f lib_chip_otar_main.mk all

.PHONY: chef
chef: toolchain $(CHEF_FILE)
	$(MAKE) -f lib_chip_chef_core.mk all
	$(MAKE) -f lib_chip_chef_main.mk all

.PHONY: is_matter
is_matter: toolchain
	cp partition_matter.json partition.json
	@$(MAKE) -f application.is.matter.mk
	
.PHONY: clean_matter_libs
clean_matter_libs:
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

.PHONY: clean_matter
clean_matter:
	@$(MAKE) -f application.is.matter.mk clean

debug: toolchain
	@$(MAKE) -f application.is.mk debug

flash: toolchain
	@$(MAKE) -f application.is.mk flash
	
setup:
	@$(MAKE) -f application.is.mk $(MAKECMDGOALS)	
