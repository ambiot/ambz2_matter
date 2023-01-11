# Matter Application Porting Layer

The goal of this porting layer is to make it easier for users to design their own Matter applications. The porting layer provides core Matter APIs for users and also makes it easier to add their own codes, such as their own peripheral drivers.

## Structure
| Directory | Description |
| ----------- | ----------- |
| common | contains common files and utilities to support Matter, used by all types of Matter applications |
| core | contains files that provides core Matter APIs |
| driver | peripheral drivers to be used by Matter application callbacks |
| example | Matter application examples for reference, according to device types |

## How to design your own custom Matter application with the porting layer
1. You do not need to modify/add files under `common` or `core`
2. Under `driver`, place your peripheral driver code, see existing driver files for reference
3. Under `example`, you may create your new custom example directory, or modify an existing one
  - Your example should at least have the following files:
    - Main task (see `light/example_matter_light.cpp`)
    - Driver interface (see `light/matter_drivers.cpp`)
    - Makefiles to build the Matter libraries
4. More details on how it works will be explained in the examples themselves
