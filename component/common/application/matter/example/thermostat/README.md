# Thermostat-app Example
This example is an implementation of the *Thermostat* device type. Peripherals consists of a thermostat and the thermostat user interface itself. Note that these driver codes are meant to be just the skeleton, you should replace them and implement your own.

## How it works
We support bidirectional exchanges of attrubute updates (see `light` example) but this example will only make use of uplink updates.
Feel free to create downlink updates by posting events to the downlink queue (again, see `light` example).

### Peripheral Initialization
Both the initializations of the thermostat and the thermostat UI are handled in `matter_drivers.cpp`.

### Matter Attribute Change Callback
Whenever the Matter controller changes the attribute of the Thermostat cluster, 2 types of callbacks will be invoked:
  1. MatterPreAttributeChangeCallback - Do action before updating the On/Off attribute (TBD)
  2. MatterPostAttributeChangeCallback - Do action after updating the On/Off attribute

These callbacks are defined in `core/matter_interaction.cpp`.
These callbacks will post an event to the uplink queue, which will be handled by `matter_driver_uplink_update_handler` in `matter_drivers.cpp`.
The driver codes will be called to carry out your actions depending on the Cluster and Attribute ID received.
You may add clusters and attributes handling in `matter_driver_uplink_update_handler` if they are not present. 

## How to build

### Configurations
Enable `CONFIG_EXAMPLE_MATTER` and `CONFIG_EXAMPLE_MATTER_THERMOSTAT` in `platform_opts.h`.
Ensure that `CONFIG_EXAMPLE_MATTER_CHIPTEST` is disabled.

### Setup the Build Environment
  
    cd connectedhomeip
    source scripts/activate.sh

### Build Matter Libraries

    cd ambz2_matter/component/common/application/matter/example/thermostat
    make thermostat
    
### Build the Final Firmware

    cd ambz2_matter/project/realtek_amebaz2_v0_example/GCC-RELEASE/
    make is_matter
    
### Flash the Image
Refer to this [guide](https://github.com/ambiot/ambz2_matter/blob/main/tools/AmebaZ2/Image_Tool_Linux/README.md) to flash the image with the Linux Image Tool

### Clean Matter Libraries

    cd ambz2_matter/component/common/application/matter/example/thermostat
    make clean
