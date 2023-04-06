# Matter OTA Guide for AmebaZ2

Follow this guide to carry out Matter standard Over the Air Software Update

## Prerequisites

Build Linux ota-provider-app

    cd connectedhomeip
    source scripts/activate.sh
    ./scripts/examples/gn_build_example.sh examples/ota-provider-app/linux/ ota-provider

## Generating the OTA Image

When building the OTA firmware, ensure that `CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION` and `CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING` is higher than that of the current image.
You can check the current image's software version using `chip-tool` commands below.

    ./chip-tool basicinformation read software-version 1 0
    ./chip-tool basicinformation read software-version-string 1 0

In `GCC-Release/ameba_firmware.json`, update the **serial** value to a higher number than that of current image.

After building the firmware, use the `ota_image_tool.py` to generate the OTA image. This tool will add Matter OTA header to the firmware image.

    python3 ota_image_tool.py create -v <VENDORID> -p <PRODUCTID> -vn <VERSION> -vs <VERSIONSTRING> -da <DIGESTALGO> <path to firmware> <output ota image>

For example

    cd ambz2_matter/tools/matter/ota
    python3 ota_image_tool.py create -v 0x8888 -p 0x9999 -vn 2 -vs 2.0 -da sha256 ../../../project/realtek_amebaz2_v0_example/GCC-RELEASE/application_is/Debug/bin/firmware_is.bin ota_image.bin

Ensure that the `VERSION` and `VERSIONSTRING` matches your `CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION` and `CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING` respectively.

To check your OTA image

    python3 ota_image_tool.py show <ota image>

Once you have ensured that the OTA image is correct, copy the output OTA image to the directory of the ota-provider built from the previous step.

    cp ota_image.bin <path to the ota-provider directory>

## Executing the OTA

#### Terminal 1 (Linux ota-provider-app)

    cd connectedhomeip/ota-provider
    ./chip-ota-provider-app -f ota_image.bin

#### Terminal 2 (chip-tool): 

Pair the device on NodeID=1, pair the ota-provider-app on NodeID=2.

    ./chip-tool ble-wifi 1 <SSID> <PASSWORD> 20202021 3840
    ./chip-tool pairing onnetwork 2 20202021

Set the ota-provider to be the default-otaprovider of the device.

    ./chip-tool otasoftwareupdaterequestor write default-otaproviders '[{"fabricIndex": 1, "providerNodeID": 2, "endpoint": 0}]' 1 0

Configure the ACL of the ota-provider-app to allow access for device.

    ./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": [{"cluster": 41, "endpoint": null, "deviceType": null}]}]' 2 0

Announce the ota-provider-app to the device to start the OTA process.

    ./chip-tool otasoftwareupdaterequestor announce-otaprovider 2 0 0 0 1 0

## Expected Outcome

- The ota-provider-app will transfer the ota image by blocks of 1024 bytes.
- After completion, device will send an `ApplyUpdateRequest` to the ota-provider-app, who will send back an `ApplyUpdateResponse`.
- Upon receiving the `ApplyUpdateResponse`, the device will countdown 10 seconds before rebooting.
- If the OTA is successful, the device will reboot into the new image and will send a `NotifyUpdateApplied` to the ota-provider-app. 

## Common Mistakes

- VendorID and ProductID of the new OTA image does not match the ones in the Basic Information cluster, causing OTA to fail. 
Check your VendorID and ProductID using below `chip-tool` commands.

    ./chip-tool basicinformation read vendor-id 1 0
    ./chip-tool basicinformation read product-id 1 0

- Version of the new OTA image is not higher than the current image's version. OTA will only be allowed to execute if new image is newer than the current image.
- OTA process completed, but device still reboots into old image instead of the new image. Check if you have updated the **Serial** field in `ameba_firmware.json`.
