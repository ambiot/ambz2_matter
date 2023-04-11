/************************** 
* Matter Utility Functions 
**************************/
#include "platform_opts.h"
#include "platform/platform_stdlib.h"

#ifdef __cplusplus
 extern "C" {
#endif

#include "chip_porting.h"
#include "flash_api.h"
#include "ameba_factory.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "device_lock.h"

#if CONFIG_ENABLE_FACTORY_DATA_ENCRYPTION
#include "mbedtls/aes.h"
unsigned char test_key[] = {0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
unsigned char test_iv[] = {0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
mbedtls_aes_context aes_ctx;
#endif

bool store_string_spake2_salt(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    FactoryData *fdp = *(FactoryData**) arg;
    uint8_t buffer[1024] = {0};

    /* We could read block-by-block to avoid the large buffer... */
    size_t data_length = stream->bytes_left;
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;

    memcpy(fdp->cdata.spake2_salt.value, buffer, data_length);
    fdp->cdata.spake2_salt.len = data_length;
    return true;
}

bool store_string_spake2_verifier(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    FactoryData *fdp = *(FactoryData**) arg;
    uint8_t buffer[1024] = {0};

    /* We could read block-by-block to avoid the large buffer... */
    size_t data_length = stream->bytes_left;
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;

    memcpy(fdp->cdata.spake2_verifier.value, buffer, data_length);
    fdp->cdata.spake2_verifier.len = data_length;
    return true;
}

bool store_string_dac_cert(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    FactoryData *fdp = *(FactoryData**) arg;
    uint8_t buffer[1024] = {0};

    /* We could read block-by-block to avoid the large buffer... */
    size_t data_length = stream->bytes_left;
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;

    memcpy(fdp->dac.dac_cert.value, buffer, data_length);
    fdp->dac.dac_cert.len = data_length;
    return true;
}

bool store_string_dac_key(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    FactoryData *fdp = *(FactoryData**) arg;
    uint8_t buffer[1024] = {0};

    /* We could read block-by-block to avoid the large buffer... */
    size_t data_length = stream->bytes_left;
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;

    memcpy(fdp->dac.dac_key.value, buffer, data_length);
    fdp->dac.dac_key.len = data_length;
    return true;
}

bool store_string_pai_cert(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    FactoryData *fdp = *(FactoryData**) arg;
    uint8_t buffer[1024] = {0};

    /* We could read block-by-block to avoid the large buffer... */
    size_t data_length = stream->bytes_left;
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;

    memcpy(fdp->dac.pai_cert.value, buffer, data_length);
    fdp->dac.pai_cert.len = data_length;
    return true;
}

bool store_string_cd(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    FactoryData *fdp = *(FactoryData**) arg;
    uint8_t buffer[1024] = {0};

    /* We could read block-by-block to avoid the large buffer... */
    size_t data_length = stream->bytes_left;
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;

    memcpy(fdp->dac.cd.value, buffer, data_length);
    fdp->dac.cd.len = data_length;
    return true;
}

bool store_string_vendor_name(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    FactoryData *fdp = *(FactoryData**) arg;
    uint8_t buffer[1024] = {0};

    /* We could read block-by-block to avoid the large buffer... */
    size_t data_length = stream->bytes_left;
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;

    memcpy(fdp->dii.vendor_name.value, buffer, data_length);
    fdp->dii.vendor_name.len = data_length;
    return true;
}

bool store_string_product_name(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    FactoryData *fdp = *(FactoryData**) arg;
    uint8_t buffer[1024] = {0};

    /* We could read block-by-block to avoid the large buffer... */
    size_t data_length = stream->bytes_left;
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;

    memcpy(fdp->dii.product_name.value, buffer, data_length);
    fdp->dii.product_name.len = data_length;
    return true;
}

bool store_string_hw_ver_string(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    FactoryData *fdp = *(FactoryData**) arg;
    uint8_t buffer[1024] = {0};

    /* We could read block-by-block to avoid the large buffer... */
    size_t data_length = stream->bytes_left;
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;

    memcpy(fdp->dii.hw_ver_string.value, buffer, data_length);
    fdp->dii.hw_ver_string.len = data_length;
    return true;
}

bool store_string_mfg_date(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    FactoryData *fdp = *(FactoryData**) arg;
    uint8_t buffer[1024] = {0};

    /* We could read block-by-block to avoid the large buffer... */
    size_t data_length = stream->bytes_left;
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;

    memcpy(fdp->dii.mfg_date.value, buffer, data_length);
    fdp->dii.mfg_date.len = data_length;
    return true;
}

bool store_string_serial_num(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    FactoryData *fdp = *(FactoryData**) arg;
    uint8_t buffer[1024] = {0};

    /* We could read block-by-block to avoid the large buffer... */
    size_t data_length = stream->bytes_left;
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;

    memcpy(fdp->dii.serial_num.value, buffer, data_length);
    fdp->dii.serial_num.len = data_length;
    return true;
}

bool store_string_rd_id_uid(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    FactoryData *fdp = *(FactoryData**) arg;
    uint8_t buffer[1024] = {0};

    /* We could read block-by-block to avoid the large buffer... */
    size_t data_length = stream->bytes_left;
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;

    memcpy(fdp->dii.rd_id_uid.value, buffer, data_length);
    fdp->dii.rd_id_uid.len = data_length;
    return true;
}

int32_t ReadFactory(uint8_t *buffer, uint16_t *pfactorydata_len)
{
    uint32_t ret;
    flash_t flash;
    uint32_t address = MATTER_FACTORY_DATA;
    uint8_t length_bytes = 2;

    // The first 2 bytes of the binary file is the length of the FactoryData
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    ret = flash_stream_read(&flash, address, length_bytes, pfactorydata_len);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    // Check if factory data length is more than 4096
    // Which indicates that factory data is not flashed
    // Return to prevent reading from non-existent address
    if(*pfactorydata_len > 4096)
    {
        return -1;
    }

    // +2 offset to read the FactoryData
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    ret = flash_stream_read(&flash, address+2, *pfactorydata_len, buffer);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
    
    return ret;
}

int32_t DecodeFactory(uint8_t *buffer, FactoryData *fdp, uint16_t data_len)
{
    int32_t ret = 1;
    pb_istream_t stream;
    FactoryDataProvider FDP = FactoryDataProvider_init_zero;

#if CONFIG_ENABLE_FACTORY_DATA_ENCRYPTION
    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_enc(&aes_ctx, test_key, 256);
    // decrypt the factorydata
    size_t nc_off = 0;
    unsigned char nonce_counter[16] = {0};
    unsigned char stream_block[16] = {0};
    size_t iv_offset = 0;
    unsigned char *decrypted_output = (unsigned char*) pvPortMalloc(data_len);
    if (decrypted_output == NULL)
    {
        ret = -1;
        goto exit;
    }

    memcpy(nonce_counter, test_iv, sizeof(nonce_counter));
    ret = mbedtls_aes_crypt_ctr(&aes_ctx, data_len, &nc_off, nonce_counter, stream_block, buffer, decrypted_output);
    if (ret !=0)
    {
        goto exit; 
    }

    memcpy(buffer, decrypted_output, data_len);
    vPortFree(decrypted_output);
    mbedtls_aes_free(&aes_ctx);
#endif

    stream = pb_istream_from_buffer(buffer, data_len);

    // Set the callbacks for these fields
    // Once decoding is done, the decoded data will be passed to the callback functions and handled there
    // TODO: Combine all the callbacks into a single switch case callback
    FDP.cdata.spake2_salt.value.funcs.decode = &store_string_spake2_salt;
    FDP.cdata.spake2_verifier.value.funcs.decode = &store_string_spake2_verifier;
    FDP.dac.dac_cert.value.funcs.decode = &store_string_dac_cert;
    FDP.dac.dac_key.value.funcs.decode = &store_string_dac_key;
    FDP.dac.pai_cert.value.funcs.decode = &store_string_pai_cert;
    FDP.dac.cd.value.funcs.decode = &store_string_cd;
    FDP.dii.vendor_name.value.funcs.decode = &store_string_vendor_name;
    FDP.dii.product_name.value.funcs.decode = &store_string_product_name;
    FDP.dii.hw_ver_string.value.funcs.decode = &store_string_hw_ver_string;
    FDP.dii.mfg_date.value.funcs.decode = &store_string_mfg_date;
    FDP.dii.serial_num.value.funcs.decode = &store_string_serial_num;
    FDP.dii.rd_id_uid.value.funcs.decode = &store_string_rd_id_uid;

    // Pass in fdp as an argument to the callback
    FDP.cdata.spake2_salt.value.arg = fdp;
    FDP.cdata.spake2_verifier.value.arg = fdp;
    FDP.dac.dac_cert.value.arg = fdp;
    FDP.dac.dac_key.value.arg = fdp;
    FDP.dac.pai_cert.value.arg = fdp;
    FDP.dac.cd.value.arg = fdp;
    FDP.dii.vendor_name.value.arg = fdp;
    FDP.dii.product_name.value.arg = fdp;
    FDP.dii.hw_ver_string.value.arg = fdp;
    FDP.dii.mfg_date.value.arg = fdp;
    FDP.dii.serial_num.value.arg = fdp;
    FDP.dii.rd_id_uid.value.arg = fdp;

    if (!pb_decode(&stream, FactoryDataProvider_fields, &FDP))
    {
        ret = -1;
        goto exit;
    }

    // We handle the integer fields here, don't need for callbacks
    fdp->cdata.passcode = FDP.cdata.passcode;
    fdp->cdata.discriminator = FDP.cdata.discriminator;
    fdp->cdata.spake2_it = FDP.cdata.spake2_it;
    fdp->dii.vendor_id = FDP.dii.vendor_id;
    fdp->dii.product_id = FDP.dii.product_id;
    fdp->dii.hw_ver = FDP.dii.hw_ver;

exit:
    return ret;
}

#ifdef __cplusplus
}
#endif
