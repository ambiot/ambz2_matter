#Prerequisites

1. Ensure that your Cygwin installation includes Python3, version 3.9.16 or above.
2. Install the required packages:
	- "pip install bitarray==2.6.0"
	- "pip install python_stdnum==1.18"

## Python tool to generate Matter onboarding codes

This tool generates Manual Pairing Code and QR Codes for Matter onboarding.

#### Example usage:

```
To display the help message:
./generate_setup_payload.py -h

To generate a setup payload:
./generate_setup_payload.py -d 3840 -p 20202021 -cf 0 -dm 2 -vid 65521 -pid 32768
```

#### Output

```
Manualcode : 34970112332
QRCode     : MT:Y.K9042C00KA0648G00
```

For more details, please refer to the Matter Specification.

---

NOTE: This tool is only capable of generating the payloads and does not support parsing the payloads.
