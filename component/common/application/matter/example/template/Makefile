BASEDIR            := $(shell pwd)
SDKROOTDIR         := $(BASEDIR)/../../../../../..
AMEBAZ2_TOOLDIR     = $(SDKROOTDIR)/component/soc/realtek/8710c/misc/iar_utility
CHIPDIR             = $(SDKROOTDIR)/third_party/connectedhomeip
BUILDDIR            = $(BASEDIR)/build
OUTPUT_DIR          = $(BUILDDIR)/chip
MATTER_TOOLDIR      = $(SDKROOTDIR)/tools/matter

TEMPLATE_FILE       = $(OUTPUT_DIR)/codegen/cluster-file.txt
TEMPLATE_ZAP        = $(BASEDIR)/template.zap

LBITS := $(shell getconf LONG_BIT)
OS := $(shell uname)

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

$(TEMPLATE_FILE): $(TEMPLATE_ZAP)
	@mkdir -p $(OUTPUT_DIR)/codegen/zap-generated
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/matter-idl-server.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(OUTPUT_DIR)/codegen/zap-generated  $^
	@python3 $(CHIPDIR)/scripts/tools/zap/generate.py --no-prettify-output --templates src/app/zap-templates/app-templates.json -z $(CHIPDIR)/src/app/zap-templates/zcl/zcl.json --output-dir $(OUTPUT_DIR)/codegen/zap-generated $^
	@python3 $(CHIPDIR)/scripts/codegen.py --generator cpp-app --output-dir $(OUTPUT_DIR)/codegen --expected-outputs $(MATTER_TOOLDIR)/codegen_helpers/expected.outputs $(BASEDIR)/temp-sensor-app.matter
	@python3 $(CHIPDIR)/src/app/zap_cluster_list.py --zap_file $^ > $@
	@python3 $(MATTER_TOOLDIR)/codegen_helpers/parse_clusters.py --cluster_file $@ --chip_path $(CHIPDIR)

.PHONY: template
template: toolchain  $(TEMPLATE_FILE)
	$(MAKE) -f lib_chip_template_core.mk all
	$(MAKE) -f lib_chip_template_main.mk all

.PHONY: clean
clean:
	@$(MAKE) -f lib_chip_template_main.mk clean
	@$(MAKE) -f lib_chip_template_core.mk clean
	rm -rf $(BUILDDIR)
	rm -rf $(BASEDIR)/template-app.matter
