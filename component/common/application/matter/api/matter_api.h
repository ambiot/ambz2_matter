#pragma once

using namespace ::chip;

/* ======== Utilities and Information ========= */
bool matter_server_is_commissioned(void);
void matter_get_fabric_indexes(uint16_t *pFabricIndexes, size_t bufSize);
CHIP_ERROR matter_get_manual_pairing_code(char *buf, size_t bufSize);
CHIP_ERROR matter_get_qr_code(char *buf, size_t bufSize);
CHIP_ERROR matter_open_basic_commissioning_window(void);

/* ======== Device Attestation Credentials ========= */
CHIP_ERROR matter_get_certificate_declaration(MutableByteSpan & outBuffer);
CHIP_ERROR matter_get_dac_cert(MutableByteSpan & outBuffer);
CHIP_ERROR matter_get_pai_cert(MutableByteSpan & outBuffer);
CHIP_ERROR matter_get_firmware_information(MutableByteSpan & outBuffer);

/* ======== Commissionable Data ========= */
CHIP_ERROR matter_get_setup_discriminator(uint16_t & discriminator);
CHIP_ERROR matter_get_spake2p_iteration_count(uint32_t & iterationCount);
CHIP_ERROR matter_get_spake2p_salt(MutableByteSpan & saltBuf);
CHIP_ERROR matter_get_spak2p_verifier(MutableByteSpan & verifierBuf, size_t & verifierLen);
CHIP_ERROR matter_get_setup_passcode(uint32_t & passcode);

/* ======== Device Instance Information ========= */
CHIP_ERROR matter_get_vendor_name(char *buf, size_t bufSize);
CHIP_ERROR matter_get_vendor_id(uint16_t & vendorId);
CHIP_ERROR matter_get_product_name(char *buf, size_t bufSize);
CHIP_ERROR matter_get_product_id(uint16_t & productId);
CHIP_ERROR matter_get_part_number(char *buf, size_t bufSize);
CHIP_ERROR matter_get_product_url(char *buf, size_t bufSize);
CHIP_ERROR matter_get_product_label(char *buf, size_t bufSize);
CHIP_ERROR matter_get_serial_number(char *buf, size_t bufSize);
CHIP_ERROR matter_get_manufacturing_date(uint16_t & year, uint8_t & month, uint8_t & day);
CHIP_ERROR matter_get_hardware_version(uint16_t & hardwareVersion);
CHIP_ERROR matter_get_hardware_version_string(char *buf, size_t bufSize);
CHIP_ERROR matter_get_software_version(uint32_t & softwareVersion);
CHIP_ERROR matter_get_software_version_string(char *buf, size_t bufSize);
