/**
 * @brief High resolution sleep.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/nanosleep.html
 *
 * @note rmtp is ignored, as signals are not implemented.
 */
#ifndef MATTER_UTILS_H_
#define MATTER_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CHIP_PROJECT

typedef struct
{
    uint8_t value[602];
    size_t len;
} FactoryDataString;

typedef struct  
{
    int passcode; 
    int discriminator;    
    int spake2_it;
    FactoryDataString spake2_salt;
    FactoryDataString spake2_verifier;
} CommissionableData;

typedef struct 
{
    FactoryDataString dac_cert;
    FactoryDataString dac_key;
    FactoryDataString pai_cert;
    FactoryDataString cd;
} DeviceAttestationCredentials;

typedef struct 
{
    int vendor_id;
    FactoryDataString vendor_name;
    int product_id;
    FactoryDataString product_name;
    int hw_ver;
    FactoryDataString hw_ver_string; 
    FactoryDataString mfg_date;
    FactoryDataString serial_num;
    FactoryDataString rd_id_uid;
} DeviceInstanceInfo;

typedef struct
{
    CommissionableData cdata;
    DeviceAttestationCredentials dac;
    DeviceInstanceInfo dii;
} FactoryData;

// Functions
uint32_t ReadFactory(uint8_t *buffer, uint16_t *pfactorydata_len);
uint32_t DecodeFactory(uint8_t *buffer, FactoryData *fdp, uint16_t data_len);
#endif /* CHIP_PROJECT */

#ifdef __cplusplus
}
#endif

#endif // MATTER_UTILS_H_
