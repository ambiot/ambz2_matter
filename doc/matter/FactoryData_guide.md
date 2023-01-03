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
    --rd-id-uid <rotating id unique id>
    
After running the script successfully, `ameba_factory.bin` should be generated in the same directory

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
