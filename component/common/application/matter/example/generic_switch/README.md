# Generic Switch

## Cluster Requirements (Server)

To implement a Generic Switch, the following clusters are required:

1. **Identify (0x0003)**
2. **Switch (0x003B)**

## ZAP Configuration

There are two types of switches, each with its own ZAP Configuration:

1. **Latching Switch**
2. **Momentary Switch**

Please refer to the [Matter Specification](https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/app_clusters/Switch.adoc) to understand more about Generic Switch and its clusters.

### Latching Switch

For a Latching Switch, make sure the `SwitchLatched` event is enabled and set the `FeatureMap` to `0x1` (Bit 0) to indicate support for the Latching Switch feature.

| Bit  | Feature        | Mandatory Event |
|------|----------------|-----------------|
| 0    | LatchingSwitch | SwitchLatched   |

### Momentary Switch

Momentary Switches support several features, as outlined below:

| Bit  | Feature                   | Mandatory Event         | Additional Event                      |
|------|---------------------------|-------------------------|----------------------------------------|
| 1    | MomentarySwitch          | N/A                     | InitialPress                           |
| 2    | MomentarySwitchRelease   | N/A                     | ShortRelease                           |
| 3    | MomentarySwitchLongPress | N/A                     | LongPress & LongRelease                |
| 4    | MomentarySwitchMultiPress| MultiPressMax           | MultiPressOngoing, MultiPressComplete  |

The `Featuremap` SHALL be set to `0x1E` to indicate support for the Momentary Switch Feature.

# Ameba Generic Switch Example

This example demonstrates the implementation of the Matter Generic Switch device type.
The ZAP Configuration is set as Momentary Switch.

The example has setup a switch(button) to perform the action.

| Peripheral  | Pin   |
| ----------- | ----- |
| Button      | PA_17 |

## How it works

The `Switch` must be 

# Two-Way Control Mechanism for Matter Generic Switch

The Generic Switch device can be controlled in two primary ways:

1. **Matter Controller (e.g., SmartHub)**: The controller sends messages to the Device Under Test (DUT) to control its state.
2. **Device GPIO (e.g., Button)**: The DUT sends messages to the controller to indicate state changes.

These two communication streams utilize FreeRTOS Queues to manage state changes and invoke appropriate callbacks.

## Communication Streams and FreeRTOS Queues

### 1. Uplink: Matter Controller to DUT

In this direction, the Matter Controller changes the attribute state. The following process occurs:

- **Action**: The Matter Controller sends a message to change the switch state.
- **Callback Invocation**: A callback is triggered in the DUT.
- **Driver Action**: The callback invokes the Switch driver to toggle the its state.

**Diagram:**

Matter Controller (SmartHub) → [Message: Initial Press] → DUT → [Callback] → Switch Driver → Toggle Switch > Toggle LED

### 2. Downlink: DUT to Matter Controller

In this direction, the DUT sends messages to the Matter Controller to indicate changes in the button state:

- **Action**: A button press event on the DUT toggles the LED state.
- **Attribute Update**: The DUT updates the Matter On/Off attribute of the LED.
- **Message Sending**: The new attribute state is sent to the Matter Controller.

**Diagram:**

Button Press → DUT → [Message: Initial Press] → Matter Controller (SmartHub) -> [Toggle LED]

### Peripheral Initialization

The initializations of the button are handled in `matter_drivers.cpp`.
The initialization of the button sets up an IRQ that is triggered whenever the button is pressed.

### Button Press
Whenever the button is pressed, the interrupt handler will be invoked.
The interrupt handler will post an event to the downlink queue, which will be handled by `matter_driver_downlink_update_handler`.

To implement this, under `matter_drivers.cpp`, setup your GPIO interrupt callback to create and post events to the downlink queue. See `matter_driver_switch_callback` for reference.
When creating the event to post to downlink queue, create a handler function for the event that will update the attributes on the Matter stack. See `matter_driver_downlink_update_handler` for reference.

### Matter Attribute Change Callback
Whenever the Matter controller changes the attribute state, 2 types of callbacks will be invoked:
  1. MatterPreAttributeChangeCallback - e.g., Toggle the LED before updating the On/Off attribute (TBD)
  2. MatterPostAttributeChangeCallback - e.g., Toggle the LED after updating the On/Off attribute

These callbacks are defined in `core/matter_interaction.cpp`.
These callbacks will post an event to the uplink queue, which will be handled by `matter_driver_uplink_update_handler` in `matter_drivers.cpp`.
The driver codes will be called to carry out your actions depending on the Cluster and Attribute ID received.
You may add clusters and attributes handling in `matter_driver_uplink_update_handler` if they are not present. 

## How to build

### Configurations
Enable `CONFIG_EXAMPLE_MATTER` and `CONFIG_EXAMPLE_MATTER_GENERIC_SWITCH` in `platform_opts.h`.
Ensure that `CONFIG_EXAMPLE_MATTER_CHIPTEST` is disabled.

### Setup the Build Environment
  
    cd connectedhomeip
    source scripts/activate.sh
  
### Build Matter Libraries

    cd ambz2_matter/component/common/application/matter/example/generic_switch
    make generic_switch
    
### Build the Final Firmware

    cd ambz2_matter/project/realtek_amebaz2_v0_example/GCC-RELEASE/
    make is_matter
    
### Flash the Image
Refer to this [guide](https://github.com/ambiot/ambz2_matter/blob/main/tools/AmebaZ2/Image_Tool_Linux/README.md) to flash the image with the Linux Image Tool

### Clean Matter Libraries

    cd ambz2_matter/component/common/application/matter/example/generic_switch
    make clean
