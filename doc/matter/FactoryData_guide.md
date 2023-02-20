# Factory Data Guide

Follow this guide to generate and use your own factory data instead of using test data

### Prerequisites

Build chip-cert tool

    cd connectedhomeip
    source scripts/activate.sh
    cd src/tools/chip-cert
    gn gen out
    ninja -C out

Build spake2p tool

    cd connectedhomeip
    source scripts/activate.sh
    cd src/tools/spake2p
    gn gen out
    ninja -C out
    
Install python dependency

    pip3 install protobuf

Make sure your firmware is built with `CONFIG_ENABLE_AMEBA_FACTORY_DATA` enabled in the **core** and **main** Matter library makefile

### Generating your own Certificates and Keys

- Note that this is only for testing and development
- For actual products, the Certificates and Keys will be provided by your vendor or by CSA once you have passed the certification test

Edit `gen-certs.sh` under `ambz2_matter/tools/matter`

To use your own VID/PID
- Change the `vid` and `pid` variable to your own `VendorID` and `ProductID` 

To use Matter's test PAA instead of generating a new PAA 
- Uncomment the `paa_key_file` and `paa_cert_file` variables under `Matter's test PAA` section
- Comment out the `paa_key_file` and `paa_cert_file` varialbes under `Self generated PAA` section
- Comment out the line where PAA is generated

To use your existing PAA instead of generating a new PAA
- Comment out he line where PAA is generated
- Provide the `paa_key_file` and `paa_cert_file` variables with the correct filepath to the existing key and cert respectively

To use CD to remap manufacturer's VID/PID to end-product's VID/PID
- Ensure that `vid` and `pid` variables are manufacturer's VID and PID respectively
- Ensure that `endproduct_vid` and `endproduct_pid` are end-product's VID and PID respectively
- Under `Generate Credential Declaration` section, comment out `CD without dac_origin_vid, dac_origin_pid` section
- Under `Generate Credential Declaration` section, uncomment `CD with dac_origin_vid, dac_origin_pid` section

Generate the certs and keys
    
    ./gen-certs.sh <path to connectedhomeip> <path to chip-cert binary> <c-style filename>
    The certs and keys will be outputted in `connectedhomeip/myattestation`
    
### Generate Factory Data Binary File 

Navigate to below directory, if matter's environment is activated, deactivate it

    cd ambz2_matter/tools/matter
    
Run the `ameba_factory.py` python script, passing in neccessary arguments

    python3 ameba_factory.py \
    --spake2p_path <path to spake2p binary> \
    -d <discriminator> \
    -p <passcode> \
    --dac_cert <path to DAC cert> \
    --dac_key <path to DAC key> \
    --pai_cert <path to PAI cert> \
    --cd <path to CD> \
    --vendor-id <vendor id> \
    --vendor-name <vendor name> \
    --product-id <product id> \
    --product-name <product-name> \
    --hw-ver <hardware version> \
    --hw-ver-str <hardware version string> \
    --mfg-date <manufacturing date> \
    --serial-num <serial number> \
    --rd-id-uid <rotating id unique id> \
    --factorydata-key <32-bytes key to encrypt factorydata, hexstring, without "0x" in front> \
    --factorydata-iv <16-bytes iv to encrypt factorydata, hexstring, without "0x" in front>
    
Example command, run from `tools/matter/factorydata`

    python3 ameba_factory.py \
    --spake2p_path ../../../third_party/connectedhomeip/src/tools/spake2p/out/spake2p \
    -d 3840 \
    -p 20202021 \
    --dac_cert ../../../third_party/connectedhomeip/myattestation/Chip-Test-DAC-8888-9999-Cert.der \
    --dac_key ../../../third_party/connectedhomeip/myattestation/Chip-Test-DAC-8888-9999-Key.der \
    --pai_cert ../../../third_party/connectedhomeip/myattestation/Chip-Test-PAI-8888-NoPID-Cert.der \
    --cd ../../../third_party/connectedhomeip/myattestation/Chip-Test-CD-8888-9999.der \
    --vendor-id 0x8888 \
    --vendor-name ameba \
    --product-id 0x9999 \
    --product-name amebaz2 \
    --hw-ver 1 \
    --hw-ver-str "1.0" \
    --mfg-date 2022-12-01 \
    --serial-num 123456 \
    --rd-id-uid 00112233445566778899aabbccddeeff \
    --factorydata-key ff0102030405060708090a0b0c0d0e0fff0102030405060708090a0b0c0d0e0f \
    --factorydata-iv ff0102030405060708090a0b0c0d0e0f
    
After running the script successfully, `ameba_factory.bin` should be generated in the same directory

### Factory Data Encryption

If you want to encrypt the factorydata, pass in `factorydata-key`, if you want to use an IV for encryption, pass in `factorydata-iv` as well

Make sure that you have enabled `CONFIG_ENABLE_FACTORY_DATA_ENCRYPTION` in `application.is.matter.mk` when building the firmware

Make sure that in `DecodeFactory` in `matter_utils.c`, you have implemented a way to retrieve the key and iv for runtime factorydata decryption (By default, it is using a hardcoded key and iv)

If you don't want to encrypt the factorydata, don't pass in the key and iv

### Flashing the Factory Data Binary File

Flash the binary file using Image_Tool_Linux

    cd ambz2_matter/tools/AmebaZ2/Image_Tool_Linux
    ./flash.sh <serial port> <path to factorydata bin> <address>
    
Note: Default address to flash `ameba_factory.bin` is 0x083FF000, you may configure it using `MATTER_FACTORY_DATA` macro in `platform_opts.h`. Make sure to check for partition conflicts.
An example is shown below

    ./flash.sh /dev/ttyUSB0 ./ameba_factory.bin 0x083FF000

### Commissioning

If this is for testing and development, during commissioning, pass the path to the PAA as an argument to chiptool

    ./chip-tool pairing ble-wifi 1 <SSID> <PASSWORD> <passcode> <discriminator> --paa-trust-store-path <path to myattestation>
