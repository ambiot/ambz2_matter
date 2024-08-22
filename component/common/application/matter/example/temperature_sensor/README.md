# Temperature Sensor Example
This example is an implementation of the *Temperature Sensor* device type. 

| Peripheral         | Pin   |
| ------------------ | ----- |
| Temperature Sensor | PA_19 |

## ZAP
Since a specific ZAP file for the temperature sensor device type is not available, we will utilize temp-sensor-app.zap as a substitute.

##Functionality
The temperature sensor is designed to measure temperature through an external physical sensor. However, in this example, a static value is read and reported to the Matter Controller via the `downlink` task and handler.

In `matter_drivers.cpp`:

- `matter_driver_temperature_sensor_init`: Configures the endpoint based on the ZAP settings, sets initial values such as MinMeasuredValue and MaxMeasuredValue, and creates a task for reading the temperature sensor measurements.
- `matter_driver_take_measurement`: Takes temperature measurements every 10 seconds, with the timing adjustable to meet specific sensor requirements.
- `matter_driver_downlink_update_handler`: Updates the temperature value to the Matter layer (Controller).
- `matter_driver_uplink_update_handler`: Updates from Matter Controller so that DUT can update its status. This is not needed for a temperature sensor device type.

### Peripheral Initialization
Both the initializations of the temperature sensor are handled in `matter_drivers.cpp`.

## How to build

### Configurations
Enable `CONFIG_EXAMPLE_MATTER` and `CONFIG_EXAMPLE_MATTER_TEMP_SENSOR` in `platform_opts.h`.
Ensure that `CONFIG_EXAMPLE_MATTER_CHIPTEST` is disabled.

### Setup the Build Environment
  
    cd connectedhomeip
    source scripts/activate.sh
  
### Build Matter Libraries

    cd ambz2_matter/component/common/application/matter/example/temperature_sensor
    make temp_sensor
    
### Build the Final Firmware

    cd ambz2_matter/project/realtek_amebaz2_v0_example/GCC-RELEASE/
    make is_matter
    
### Flash the Image
Refer to this [guide](https://github.com/ambiot/ambz2_matter/blob/main/tools/AmebaZ2/Image_Tool_Linux/README.md) to flash the image with the Linux Image Tool

### Clean Matter Libraries

    cd ambz2_matter/component/common/application/matter/example/aircon
    make clean
