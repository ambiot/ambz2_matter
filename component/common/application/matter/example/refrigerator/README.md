# Refrigerator Example
This example is an implementation of the *Room Air Conditioner* device type. You will need a PWM fan and a temperature/humidity sensor.

| Peripheral | Pin |
| ----------- | ----------- |
| Fan | PA_23 |
| Temp/Hum Sensor | Depends on type of sensor |

## ZAP
Since there is no example ZAP file for the refrigerator device type, we will use `refrigerator-app.zap`.

## How it works
The fan can be controlled in two ways, by the Matter controller, or by external means. In this example, we only demonstrate control via Matter controller. If you wish to add more methods to control (eg. a push button), you will need to implement the `downlink` task and handler. See `lighting-app` for button example.
Thus, we only use 1 Uplink queue to for the fan to be controlled by the Matter controller.

### Peripheral Initialization
Both the initializations of the fan and the temperature/humidity sensor are handled in `matter_drivers.cpp`.

### Fan Attribute Change
Whenever the Matter controller changes the Fanmode/Fanspeed attribute of the fan, 2 types of callbacks will be invoked:
  1. MatterPreAttributeChangeCallback - Change the Fanmode/Fanspeed before updating the Fanmode/Fanspeed attribute (TBD)
  2. MatterPostAttributeChangeCallback - Change the Fanmode/Fanspeed after updating the Fanmode/Fanspeed attribute

These callbacks are defined in `core/matter_interaction.cpp`.
These callbacks will post an event to the uplink queue, which will be handled by `matter_driver_uplink_update_handler` in `matter_drivers.cpp`.
The driver codes will be called to carry out your actions depending on the Cluster and Attribute ID received.
You may add clusters and attributes handling in `matter_driver_uplink_update_handler` if they are not present. 

### Temperature/Humidity Sensor Attribute Change
By calling `matter_driver_temphumsensor_start`, a task will be created to poll the temperature and humidity periodically.
After obtaining the temperature and humidity measurements, the task will update the respective attributes on the Matter data model by invoking the `Set()` function. 

## How to build

### Configurations
Enable `CONFIG_EXAMPLE_MATTER` and `CONFIG_EXAMPLE_MATTER_REFRIGERATOR` in `platform_opts.h`.
Ensure that `CONFIG_EXAMPLE_MATTER_CHIPTEST` is disabled.

### Setup the Build Environment
  
    cd connectedhomeip
    source scripts/activate.sh
  
### Build Matter Libraries

    cd ambz2_matter/component/common/application/matter/example/refrigerator
    make refrigerator
    
### Build the Final Firmware

    cd ambz2_matter/project/realtek_amebaz2_v0_example/GCC-RELEASE/
    make is_matter
    
### Flash the Image
Refer to this [guide](https://github.com/ambiot/ambz2_matter/blob/main/tools/AmebaZ2/Image_Tool_Linux/README.md) to flash the image with the Linux Image Tool

### Clean Matter Libraries

    cd ambz2_matter/component/common/application/matter/example/refrigerator
    make clean
