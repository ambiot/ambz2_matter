# FAN Example
This example is an implementation of the *FAN* device type. You will need a PWM fan.

| Peripheral | Pin |
| ----------- | ----------- |
| Fan | PA_23 |

## ZAP
Since there is no example ZAP file for the fan device type, we will use `fan-app.zap`.

## How it works
The fan can be controlled in two ways, by the Matter controller, or by external means. In this example, we only demonstrate control via Matter controller. If you wish to add more methods to control (eg. a push button), you will need to implement the `downlink` task and handler. See `lighting-app` for button example.
Thus, we only use 1 Uplink queue to for the fan to be controlled by the Matter controller. 

### Peripheral Initialization
The initializations of the fan are handled in `matter_drivers.cpp`.

### Fan Attribute Change
Whenever the Matter controller changes the Fanmode/Fanspeed attribute of the fan, 2 types of callbacks will be invoked:
  1. MatterPreAttributeChangeCallback - Change the Fanmode/Fanspeed before updating the Fanmode/Fanspeed attribute (TBD)
  2. MatterPostAttributeChangeCallback - Change the Fanmode/Fanspeed after updating the Fanmode/Fanspeed attribute

These callbacks are defined in `core/matter_interaction.cpp`.
These callbacks will post an event to the uplink queue, which will be handled by `matter_driver_uplink_update_handler` in `matter_drivers.cpp`.
The driver codes will be called to carry out your actions depending on the Cluster and Attribute ID received.
You may add clusters and attributes handling in `matter_driver_uplink_update_handler` if they are not present. 

## How to build

### Configurations
Enable `CONFIG_EXAMPLE_MATTER` and `CONFIG_EXAMPLE_MATTER_FAN` in `platform_opts.h`.
Ensure that `CONFIG_EXAMPLE_MATTER_CHIPTEST` is disabled.

### Setup the Build Environment

    cd connectedhomeip
    source scripts/activate.sh

### Build Matter Libraries

    cd ambz2_matter/component/common/application/matter/example/fan
    make fan

### Build the Final Firmware

    cd ambz2_matter/project/realtek_amebaz2_v0_example/GCC-RELEASE/
    make is_matter

### Flash the Image
Refer to this [guide](https://github.com/ambiot/ambz2_matter/blob/main/tools/AmebaZ2/Image_Tool_Linux/README.md) to flash the image with the Linux Image Tool

### Clean Matter Libraries

    cd ambz2_matter/component/common/application/matter/example/fan
    make clean
