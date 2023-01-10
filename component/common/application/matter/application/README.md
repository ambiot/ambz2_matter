# Matter application

## Files
| File Name | Description |
| ----------- | ----------- |
| chip_porting.h | Header files with all the Matter API declarations |
| matter_dcts | DCT API for Matter |
| matter_timers | Timer API for Matter |
| matter_utils | Utility API for Matter |
| matter_wifis | WiFi API for Matter |
| example_matter | Main Matter application example |
| example_matter_write_protect | Matter write protection example |

## Run Matter Example
- To run Matter example, enable `CONFIG_MATTER_EXAMPLE` in `platform_opts.h`

## Run Matter Write Protect Example
- To run Matter write protect example, disable `CONFIG_MATTER_EXAMPLE` and enable `CONFIG_MATTER_WRITE_PROTECT_EXAMPLE` in `platform_opts.h`
- This example adds/removes write protection to the last 4KB region of the external flash
- External flash used: Winbond W25Q32JV (4MB)
- Region protected: 0x3FF000 - 0x3FFFFF
- Please check your external flash's datasheet for instructions on how to set the status registers correctly
- To add write protection, enable `LOCK_FACTORY_DATA` in the source file
- To remove write protection, disable `LOCK_FACTORY_DATA` in the source file
- Make sure that `Status Register After Setting` is printed out with the correct value
- In this example, status register after setting should be `0x44` if you are adding write protection and `0x00` if you are removing write protection
- You may use image tool to erase/modify the contents of this flash region after adding write protection, it should fail
