all: is

OS := $(shell uname)
LBITS := $(shell getconf LONG_BIT)

BASEDIR = $(shell pwd)
MATTER_BUILDDIR = $(shell pwd)/../../../component/common/application/matter/project/amebaz2

include toolchain.mk

.PHONY: is
is: toolchain
	@$(MAKE) -f application.is.mk

.PHONY: mp
mp: toolchain
	@$(MAKE) -f application.is.mk mp

.PHONY: clean
clean:
	@$(MAKE) -f application.is.mk clean

debug: toolchain
	@$(MAKE) -f application.is.mk debug

flash: toolchain
	@$(MAKE) -f application.is.mk flash

setup:
	@$(MAKE) -f application.is.mk $(MAKECMDGOALS)

#*****************************************************************************#
#                         MATTER MAKE DEFINES                                 #
#*****************************************************************************#
.PHONY: toolchain_matter
toolchain_matter:
	@$(MAKE) -C $(MATTER_BUILDDIR) toolchain_matter

.PHONY: is_matter
is_matter: toolchain_matter
	cp $(MATTER_BUILDDIR)/partition_matter.json partition.json
	@$(MAKE) -f application.is.matter.mk

.PHONY: all_clusters
all_clusters:
	@$(MAKE) -C $(MATTER_BUILDDIR) all_clusters

.PHONY: aircon_port
aircon_port:
	@$(MAKE) -C $(MATTER_BUILDDIR) aircon_port

.PHONY: air_purifier
air_purifier:
	@$(MAKE) -C $(MATTER_BUILDDIR) air_purifier

.PHONY: bridge_dm
bridge_dm:
	$(MAKE) -C $(MATTER_BUILDDIR) bridge_dm

.PHONY: dishwasher_port
dishwasher_port:
	$(MAKE) -C $(MATTER_BUILDDIR) dishwasher_port

.PHONY: fan_port
fan_port:
	$(MAKE) -C $(MATTER_BUILDDIR) fan_port

.PHONY: chef
chef:
	@$(MAKE) -C $(MATTER_BUILDDIR) chef

.PHONY: laundrywasher_port
laundrywasher_port:
	@$(MAKE) -C $(MATTER_BUILDDIR) laundrywasher_port

.PHONY: light
light:
	@$(MAKE) -C $(MATTER_BUILDDIR) light

.PHONY: light_dm
light_dm:
	@$(MAKE) -C $(MATTER_BUILDDIR) light_dm

.PHONY: light_port
light_port:
	@$(MAKE) -C $(MATTER_BUILDDIR) light_port

.PHONY: light_switch
light_switch:
	@$(MAKE) -C $(MATTER_BUILDDIR) light_switch

.PHONY: otar
otar:
	@$(MAKE) -C $(MATTER_BUILDDIR) otar

.PHONY: refrigerator_port
refrigerator_port:
	@$(MAKE) -C $(MATTER_BUILDDIR) refrigerator_port

.PHONY: thermostat_port
thermostat_port:
	@$(MAKE) -C $(MATTER_BUILDDIR) thermostat_port

.PHONY: tempsensor_diagnosticlogs
tempsensor_diagnosticlogs:
	@$(MAKE) -C $(MATTER_BUILDDIR) tempsensor_diagnosticlogs

.PHONY: light_logredirect
light_logredirect:
	@$(MAKE) -C $(MATTER_BUILDDIR) light_logredirect

.PHONY: clean_matter
clean_matter:
	@$(MAKE) -f application.is.matter.mk clean

.PHONY: clean_matter_libs
clean_matter_libs:
	@$(MAKE) -C $(MATTER_BUILDDIR) clean_matter_libs
