all: all_clusters

OS := $(shell uname)
LBITS := $(shell getconf LONG_BIT)

BASEDIR			:= $(shell pwd)
SDKROOTDIR		:= $(BASEDIR)/../../../../../..
CHIPDIR			= $(SDKROOTDIR)/third_party/connectedhomeip
MATTER_MAKEDIR		= $(BASEDIR)/make
MATTER_TOOLDIR		= $(SDKROOTDIR)/tools/matter
MATTER_EXAMPLEDIR	= $(SDKROOTDIR)/component/common/application/matter/example

#*****************************************************************************#
#                         Toolchain Extraction                                #
#*****************************************************************************#

.PHONY: toolchain
toolchain:
	@echo Toolchain unzipping...

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

#*****************************************************************************#
#                            MATTER ZAP files                                 #
#*****************************************************************************#
ALL_CLUSTERS_FILE	= $(CHIPDIR)/examples/all-clusters-app/ameba/build/chip/codegen/cluster-file.txt
ALL_CLUSTERS_ZAP	= $(CHIPDIR)/examples/all-clusters-app/all-clusters-common/all-clusters-app.zap
AIR_PURIFIER_FILE	= $(CHIPDIR)/examples/air-purifier-app/ameba/build/chip/codegen/cluster-file.txt
AIR_PURIFIER_ZAP	= $(CHIPDIR)/examples/air-purifier-app/air-purifier-common/air-purifier-app.zap
CHEF_FILE		= $(CHIPDIR)/examples/chef/ameba/build/chip/codegen/cluster-file.txt
CHEF_ZAP		= $(CHIPDIR)/examples/chef/devices/$(SAMPLE_NAME).zap
LIGHTING_FILE		= $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/cluster-file.txt
LIGHTING_ZAP		= $(CHIPDIR)/examples/lighting-app/lighting-common/lighting-app.zap
LIGHT_SWITCH_FILE	= $(CHIPDIR)/examples/light-switch-app/ameba/build/chip/codegen/cluster-file.txt
LIGHT_SWITCH_ZAP	= $(CHIPDIR)/examples/light-switch-app/light-switch-common/light-switch-app.zap
OTAR_FILE		= $(CHIPDIR)/examples/ota-requestor-app/ameba/build/chip/codegen/cluster-file.txt
OTAR_ZAP		= $(CHIPDIR)/examples/ota-requestor-app/ota-requestor-common/ota-requestor-app.zap

AIRCON_FILE		= $(MATTER_MAKEDIR)/aircon/build/chip/codegen/cluster-file.txt
AIRCON_ZAP		= $(MATTER_EXAMPLEDIR)/aircon/aircon-app.zap
BRIDGE_DM_FILE		= $(MATTER_MAKEDIR)/bridge_dm/build/chip/codegen/cluster-file.txt
BRIDGE_DM_ZAP		= $(MATTER_EXAMPLEDIR)/bridge_dm/bridge-app.zap
DISHWASHER_FILE		= $(MATTER_MAKEDIR)/dishwasher/build/chip/codegen/cluster-file.txt
DISHWASHER_ZAP		= $(MATTER_EXAMPLEDIR)/dishwasher/dishwasher-app.zap
FAN_FILE		= $(MATTER_MAKEDIR)/fan/build/chip/codegen/cluster-file.txt
FAN_ZAP			= $(MATTER_EXAMPLEDIR)/fan/fan-app.zap
LAUNDRYWASHER_FILE	= $(MATTER_MAKEDIR)/laundrywasher/build/chip/codegen/cluster-file.txt
LAUNDRYWASHER_ZAP	= $(MATTER_EXAMPLEDIR)/laundrywasher/laundrywasher-app.zap
REFRIGERATOR_FILE	= $(MATTER_MAKEDIR)/refrigerator/build/chip/codegen/cluster-file.txt
REFRIGERATOR_ZAP	= $(MATTER_EXAMPLEDIR)/refrigerator/refrigerator-app.zap
THERMOSTAT_FILE		= $(CHIPDIR)/examples/thermostat/ameba/build/chip/codegen/cluster-file.txt
THERMOSTAT_ZAP		= $(CHIPDIR)/examples/thermostat/thermostat-common/thermostat.zap

#*****************************************************************************#
#                      RULES TO MAKE MATTER DIRS                              #
#*****************************************************************************#
$(ALL_CLUSTERS_FILE): $(ALL_CLUSTERS_ZAP)
	@cp $(MATTER_EXAMPLEDIR)/chiptest/all-clusters-app.zap $(ALL_CLUSTERS_ZAP)
	@mkdir -p $(CHIPDIR)/examples/all-clusters-app/ameba/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json --output-dir examples/all-clusters-app/ameba/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/all-clusters-app/ameba/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/all-clusters-app/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/all-clusters-app/all-clusters-common/all-clusters-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $(CHIPDIR)/examples/all-clusters-app/ameba/build/chip/codegen/cluster-file.txt
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

$(AIR_PURIFIER_FILE): $(AIR_PURIFIER_ZAP)
	@mkdir -p $(CHIPDIR)/examples/air-purifier-app/ameba/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json --output-dir examples/air-purifier-app/ameba/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/air-purifier-app/ameba/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/air-purifier-app/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/air-purifier-app/air-purifier-common/air-purifier-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $(CHIPDIR)/examples/air-purifier-app/ameba/build/chip/codegen/cluster-file.txt
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

$(CHEF_FILE): $(CHEF_ZAP)
	@mkdir -p $(CHIPDIR)/examples/chef-app/ameba/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json --output-dir examples/chef-app/ameba/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/chef-app/ameba/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/chef-app/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/chef-app/chef-common/chef-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $(CHIPDIR)/examples/chef-app/ameba/build/chip/codegen/cluster-file.txt
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

$(AIRCON_FILE): $(AIRCON_ZAP)
	@mkdir -p $(MATTER_MAKEDIR)/aircon/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(MATTER_MAKEDIR)/aircon/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(MATTER_MAKEDIR)/aircon/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(MATTER_MAKEDIR)/aircon/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(MATTER_EXAMPLEDIR)/aircon/aircon-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $@
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

$(BRIDGE_DM_FILE): $(BRIDGE_DM_ZAP)
	@mkdir -p $(MATTER_MAKEDIR)/bridge_dm/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(MATTER_MAKEDIR)/bridge_dm/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(MATTER_MAKEDIR)/bridge_dm/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(MATTER_MAKEDIR)/bridge_dm/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(MATTER_EXAMPLEDIR)/bridge_dm/bridge-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $(MATTER_MAKEDIR)/bridge_dm/build/chip/codegen/cluster-file.txt
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)
	@cp $(MATTER_EXAMPLEDIR)/bridge_dm/endpoint_config.h $(MATTER_MAKEDIR)/bridge_dm/build/chip/codegen/zap-generated
	@cp $(MATTER_EXAMPLEDIR)/bridge_dm/gen_config.h $(MATTER_MAKEDIR)/bridge_dm/build/chip/codegen/zap-generated

$(DISHWASHER_FILE): $(DISHWASHER_ZAP)
	@mkdir -p $(MATTER_MAKEDIR)/dishwasher/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(MATTER_MAKEDIR)/dishwasher/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(MATTER_MAKEDIR)/dishwasher/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(MATTER_MAKEDIR)/dishwasher/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(MATTER_EXAMPLEDIR)/dishwasher/dishwasher-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $@
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

$(FAN_FILE): $(FAN_ZAP)
	@mkdir -p $(MATTER_MAKEDIR)/fan/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(MATTER_MAKEDIR)/fan/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(MATTER_MAKEDIR)/fan/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(MATTER_MAKEDIR)/fan/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(MATTER_EXAMPLEDIR)/fan/fan-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $@
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

$(LAUNDRYWASHER_FILE): $(LAUNDRYWASHER_ZAP)
	@mkdir -p $(MATTER_MAKEDIR)/laundrywasher/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(MATTER_MAKEDIR)/laundrywasher/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(MATTER_MAKEDIR)/laundrywasher/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(MATTER_MAKEDIR)/laundrywasher/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(MATTER_EXAMPLEDIR)/laundrywasher/laundrywasher-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $@
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

$(REFRIGERATOR_FILE): $(REFRIGERATOR_ZAP)
	@mkdir -p $(MATTER_MAKEDIR)/refrigerator/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(MATTER_MAKEDIR)/refrigerator/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(MATTER_MAKEDIR)/refrigerator/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(MATTER_MAKEDIR)/refrigerator/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(MATTER_EXAMPLEDIR)/refrigerator/refrigerator-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $@
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

$(THERMOSTAT_FILE): $(THERMOSTAT_ZAP)
	@mkdir -p $(CHIPDIR)/examples/thermostat/ameba/build/chip/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json --output-dir examples/thermostat/ameba/build/chip/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json --output-dir examples/thermostat/ameba/build/chip/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(CHIPDIR)/examples/thermostat/ameba/build/chip/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(CHIPDIR)/examples/thermostat/thermostat-common/thermostat.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $(CHIPDIR)/examples/thermostat/ameba/build/chip/codegen/cluster-file.txt
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

copy_gen_files_light_dm:
	@cp $(MATTER_EXAMPLEDIR)/light_dm/endpoint_config.h $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/zap-generated
	@cp $(MATTER_EXAMPLEDIR)/light_dm/gen_config.h $(CHIPDIR)/examples/lighting-app/ameba/build/chip/codegen/zap-generated

#*****************************************************************************#
#                         MATTER MAKE DEFINES                                 #
#*****************************************************************************#
.PHONY: all_clusters
all_clusters: toolchain $(ALL_CLUSTERS_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/all_clusters/lib_chip.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/all_clusters/lib_chip_main.mk all

.PHONY: aircon_port
aircon_port: toolchain  $(AIRCON_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/aircon/lib_chip_aircon_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/aircon/lib_chip_aircon_main.mk all

.PHONY: air_purifier
air_purifier: toolchain $(AIR_PURIFIER_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/air_purifier/lib_chip_air_purifier_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/air_purifier/lib_chip_air_purifier_main.mk all

.PHONY: bridge_dm
bridge_dm: toolchain  $(BRIDGE_DM_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/bridge_dm/lib_chip_bridge_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/bridge_dm/lib_chip_bridge_main.mk all

.PHONY: dishwasher_port
dishwasher_port: toolchain  $(DISHWASHER_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/dishwasher/lib_chip_dishwasher_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/dishwasher/lib_chip_dishwasher_main.mk all

.PHONY: fan_port
fan_port: toolchain  $(FAN_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/fan/lib_chip_fan_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/fan/lib_chip_fan_main.mk all

.PHONY: laundrywasher_port
laundrywasher_port: toolchain  $(LAUNDRYWASHER_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/laundrywasher/lib_chip_laundrywasher_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/laundrywasher/lib_chip_laundrywasher_main.mk all

.PHONY: light
light: toolchain  $(LIGHTING_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/light/lib_chip_light_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/light/lib_chip_light_main.mk all

.PHONY: light_dm
light_dm: toolchain  $(LIGHTING_FILE) copy_gen_files_light_dm
	$(MAKE) -f $(MATTER_MAKEDIR)/light_dm/lib_chip_light_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/light_dm/lib_chip_light_main.mk all

.PHONY: light_port
light_port: toolchain  $(LIGHTING_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/light_port/lib_chip_light_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/light_port/lib_chip_light_main.mk all

.PHONY: light_switch
light_switch: toolchain $(LIGHT_SWITCH_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/light_switch/lib_chip_switch_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/light_switch/lib_chip_switch_main.mk all

.PHONY: refrigerator_port
refrigerator_port: toolchain  $(REFRIGERATOR_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/refrigerator/lib_chip_refrigerator_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/refrigerator/lib_chip_refrigerator_main.mk all

.PHONY: thermostat_port
thermostat_port: toolchain $(THERMOSTAT_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/thermostat/lib_chip_thermostat_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/thermostat/lib_chip_thermostat_main.mk all

.PHONY: otar
otar: toolchain $(OTAR_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/otar/lib_chip_otar_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/otar/lib_chip_otar_main.mk all

.PHONY: chef
chef: toolchain $(CHEF_FILE)
	$(MAKE) -f $(MATTER_MAKEDIR)/chef/lib_chip_chef_core.mk all
	$(MAKE) -f $(MATTER_MAKEDIR)/chef/lib_chip_chef_main.mk all

.PHONY: clean_matter_libs
clean_matter_libs:
	@$(MAKE) -f $(MATTER_MAKEDIR)/all_clusters/lib_chip_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/aircon/lib_chip_aircon_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/air_purifier/lib_chip_air_purifier_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/bridge_dm/lib_chip_bridge_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/chef/lib_chip_chef_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/dishwasher/lib_chip_dishwasher_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/fan/lib_chip_fan_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/laundrywasher/lib_chip_laundrywasher_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/light/lib_chip_light_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/light_dm/lib_chip_light_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/light_port/lib_chip_light_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/light_switch/lib_chip_switch_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/otar/lib_chip_otar_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/refrigerator/lib_chip_refrigerator_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/thermostat/lib_chip_thermostat_main.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/all_clusters/lib_chip.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/aircon/lib_chip_aircon_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/air_purifier/lib_chip_air_purifier_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/bridge_dm/lib_chip_bridge_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/chef/lib_chip_chef_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/dishwasher/lib_chip_dishwasher_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/fan/lib_chip_fan_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/laundrywasher/lib_chip_laundrywasher_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/light/lib_chip_light_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/light_dm/lib_chip_light_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/light_port/lib_chip_light_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/light_switch/lib_chip_switch_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/otar/lib_chip_otar_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/refrigerator/lib_chip_refrigerator_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/thermostat/lib_chip_thermostat_core.mk clean
	@$(MAKE) -f $(MATTER_MAKEDIR)/thermostat/lib_chip_thermostat_core.mk clean
