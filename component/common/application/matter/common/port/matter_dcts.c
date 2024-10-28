/************************** 
* Matter DCT Related 
**************************/
#include "platform_opts.h"
#include "platform/platform_stdlib.h"

#ifdef __cplusplus
 extern "C" {
#endif

#include "stddef.h"
#include "string.h"
#include "stdbool.h"
#include "dct.h"
#include "chip_porting.h"

uint8_t temp_buf[30];

#if CONFIG_ENABLE_DCT_ENCRYPTION
#include "mbedtls/aes.h"
#endif
/*
   module size is 4k, we set max module number as 12;
   if backup enabled, the total module number is 12 + 1*12 = 24, the size is 96k;
   if wear leveling enabled, the total module number is 12 + 2*12 + 3*12 = 36, the size is 288k"
*/
#define DCT_BEGIN_ADDR_MATTER   DCT_BEGIN_ADDR    /*!< DCT begin address of flash, ex: 0x100000 = 1M */
#define MODULE_NUM              6                /*!< max number of module */
#define VARIABLE_NAME_SIZE      32                /*!< max size of the variable name */
#define VARIABLE_VALUE_SIZE     64 + 4            /*!< max size of the variable value, +4 is required, else the max variable size we can store is 60 */ 
                                                  /*!< max number of variable in module = floor (4024 / (32 + 64)) = 41 */

#define DCT_BEGIN_ADDR_MATTER2  DCT_BEGIN_ADDR2
#define MODULE_NUM2             10
#define VARIABLE_NAME_SIZE2     32
#define VARIABLE_VALUE_SIZE2    400 + 4           /* +4 is required, else the max variable size we can store is 396 */
                                                  /*!< max number of variable in module = floor (4024 / (32 + 400)) = 9 */

#define ENABLE_BACKUP           1
#define ENABLE_WEAR_LEVELING    0

#define DCT_REGION_1 0
#define DCT_REGION_2 1

#if CONFIG_ENABLE_DCT_ENCRYPTION
#if defined(MBEDTLS_CIPHER_MODE_CTR)
mbedtls_aes_context aes;

// key length 32 bytes for 256 bit encrypting, it can be 16 or 24 bytes for 128 and 192 bits encrypting mode
unsigned char key[] = {0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

int32_t dct_encrypt(unsigned char *input_to_encrypt, int input_len, unsigned char *encrypt_output)
{
    size_t nc_off = 0;

    unsigned char nonce_counter[16] = {0};
    unsigned char stream_block[16] = {0};

    return mbedtls_aes_crypt_ctr(&aes, input_len, &nc_off, nonce_counter, stream_block, input_to_encrypt, encrypt_output);
}

int32_t dct_decrypt(unsigned char *input_to_decrypt, int input_len, unsigned char *decrypt_output)
{
    size_t nc_off1 = 0;
    unsigned char nonce_counter1[16] = {0};
    unsigned char stream_block1[16] = {0};

    return mbedtls_aes_crypt_ctr(&aes, input_len, &nc_off1, nonce_counter1, stream_block1, input_to_decrypt, decrypt_output);
}

int32_t dct_set_encrypted_variable(dct_handle_t *dct_handle, char *variable_name, char *variable_value, uint16_t variable_value_length, uint8_t region)
{
    int32_t ret;
    char encrypted_data[VARIABLE_VALUE_SIZE2] = {0};

    // encrypt the variable value
    ret = dct_encrypt(variable_value, variable_value_length, encrypted_data);
    if (ret != 0)
    {
       return DCT_ERROR;
    }

    // store in dct
    if (region == DCT_REGION_1)
        ret = dct_set_variable_new(dct_handle, variable_name, encrypted_data, variable_value_length);
    else if (region == DCT_REGION_2)
        ret = dct_set_variable_new2(dct_handle, variable_name, encrypted_data, variable_value_length);

    return ret;
}

int32_t dct_get_encrypted_variable(dct_handle_t *dct_handle, char *variable_name, char *buffer, uint16_t *buffer_size, uint8_t region)
{
    int32_t ret;
    uint8_t encrypted_data[404] = {0};

    // get the encrypted value from dct
    if (region == DCT_REGION_1)
        ret = dct_get_variable_new(dct_handle, variable_name, encrypted_data, buffer_size);
    else if (region == DCT_REGION_2)
        ret = dct_get_variable_new2(dct_handle, variable_name, encrypted_data, buffer_size);

    if (ret != DCT_SUCCESS)
        return ret;

    // decrypt the encrypted value
    ret = dct_decrypt(encrypted_data, *buffer_size, buffer);
    if (ret != 0)
    {
        return DCT_ERROR;
    }
    
    return ret;
}

#else
#error "MBEDTLS_CIPHER_MODE_CTR must be enabled to perform DCT flash encryption" 
#endif // MBEDTLS_CIPHER_MODE_CTR
#endif

s32 initPref(void)
{
    s32 ret;

#if defined(DCT_UPDATE_ENABLE) && DCT_UPDATE_ENABLE
    extern void matter_dct_update(uint8_t region, uint32_t old_address, uint32_t new_address, uint16_t old_mod_num, uint16_t new_mod_num);
    matter_dct_update(DCT_REGION_1, DCT_BEGIN_ADDR_OLD, DCT_BEGIN_ADDR_MATTER, MODULE_NUM_OLD, MODULE_NUM);
    matter_dct_update(DCT_REGION_2, DCT_BEGIN_ADDR2_OLD, DCT_BEGIN_ADDR_MATTER2, MODULE_NUM2_OLD, MODULE_NUM2);
#endif

    ret = dct_init(DCT_BEGIN_ADDR_MATTER, MODULE_NUM, VARIABLE_NAME_SIZE, VARIABLE_VALUE_SIZE, ENABLE_BACKUP, ENABLE_WEAR_LEVELING);
    if (ret != DCT_SUCCESS)
        printf("dct_init failed with error: %d\n", ret);
    else
        printf("dct_init success\n");

    ret = dct_init2(DCT_BEGIN_ADDR_MATTER2, MODULE_NUM2, VARIABLE_NAME_SIZE2, VARIABLE_VALUE_SIZE2, ENABLE_BACKUP, ENABLE_WEAR_LEVELING);
    if (ret != DCT_SUCCESS)
        printf("dct_init2 failed with error: %d\n", ret);
    else
        printf("dct_init2 success\n");

#if CONFIG_ENABLE_DCT_ENCRYPTION
    // Initialize mbedtls aes context and set encryption key
    mbedtls_aes_init(&aes);
    if (mbedtls_aes_setkey_enc(&aes, key, 256) != 0)
    {
        return DCT_ERROR;
    }
#endif

    return ret;
}

s32 deinitPref(void)
{
    s32 ret;
    ret = dct_format(DCT_BEGIN_ADDR_MATTER, MODULE_NUM, VARIABLE_NAME_SIZE, VARIABLE_VALUE_SIZE, ENABLE_BACKUP, ENABLE_WEAR_LEVELING);
    if (ret != DCT_SUCCESS)
        printf("dct_format failed with error: %d\n", ret);
    else
        printf("dct_format success\n");

    ret = dct_format2(DCT_BEGIN_ADDR_MATTER2, MODULE_NUM2, VARIABLE_NAME_SIZE2, VARIABLE_VALUE_SIZE2, ENABLE_BACKUP, ENABLE_WEAR_LEVELING);
    if (ret != DCT_SUCCESS)
        printf("dct_format2 failed with error: %d\n", ret);
    else
        printf("dct_format2 success\n");

#if CONFIG_ENABLE_DCT_ENCRYPTION
    // free aes context
    mbedtls_aes_free(&aes);
#endif

    return ret;
}

s32 registerPref()
{
    s32 ret;
    char ns[15];

    for (size_t i=0; i<MODULE_NUM; i++)
    {
        snprintf(ns, 15, "matter_kvs1_%d", i+1); 
        ret = dct_register_module(ns);
        if (ret != DCT_SUCCESS)
            goto exit;
        else
            printf("dct_register_module %s success\n", ns);
    }

exit:
    if (ret != DCT_SUCCESS)
        printf("DCT1 modules registration failed");
    return ret;
}

s32 registerPref2()
{
    s32 ret;
    char ns[15];

    for (size_t i=0; i<MODULE_NUM2; i++)
    {
        snprintf(ns, 15, "matter_kvs2_%d", i+1); 
        ret = dct_register_module2(ns);
        if (ret != DCT_SUCCESS)
            goto exit;
        else
            printf("dct_register_module2 %s success\n", ns);
    }

exit:
    if (ret != DCT_SUCCESS)
        printf("DCT2 modules registration failed");
    return ret;
}

s32 clearPref()
{
    s32 ret;
    char ns[15];

    for (size_t i=0; i<MODULE_NUM; i++)
    {
        snprintf(ns, 15, "matter_kvs1_%d", i+1); 
        ret = dct_unregister_module(ns);
        if (ret != DCT_SUCCESS)
            goto exit;
        else
            printf("dct_unregister_module %s success\n", ns);
    }

exit:
    if (ret != DCT_SUCCESS)
        printf("DCT1 modules unregistration failed");
    return ret;
}

s32 clearPref2()
{
    s32 ret;
    char ns[15];

    for (size_t i=0; i<MODULE_NUM2; i++)
    {
        snprintf(ns, 15, "matter_kvs2_%d", i+1); 
        ret = dct_unregister_module2(ns);
        if (ret != DCT_SUCCESS)
            goto exit;
        else
            printf("dct_unregister_module2 %s success\n", ns);
    }

exit:
    if (ret != DCT_SUCCESS)
        printf("DCT2 modules unregistration failed");
    return ret;
}

s32 deleteKey(const char *domain, const char *key)
{
    dct_handle_t handle;
    s32 ret;
    char ns[15];

    // Loop over DCT1 modules
    for (size_t i=0; i<MODULE_NUM; i++)
    {
        snprintf(ns, 15, "matter_kvs1_%d", i+1); 
        ret = dct_open_module(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }
        ret = dct_delete_variable(&handle, key);
        dct_close_module(&handle);
        if (ret == DCT_SUCCESS) // return success once deleted
            return ret;
    }

    // Loop over DCT2 modules
    for (size_t i=0; i<MODULE_NUM2; i++)
    {
        snprintf(ns, 15, "matter_kvs2_%d", i+1); 
        ret = dct_open_module2(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }
        ret = dct_delete_variable2(&handle, key);
        dct_close_module2(&handle);
        if (ret == DCT_SUCCESS) // return success once deleted
            return ret;
    }

exit:
    return ret;
}

bool checkExist(const char *domain, const char *key)
{
    dct_handle_t handle;
    s32 ret;
    uint16_t len = 0;
    u8 *str = malloc(sizeof(u8) * VARIABLE_VALUE_SIZE2); // use the bigger buffer size
    char ns[15];

    // Loop over DCT1 modules
    for (size_t i=0; i<MODULE_NUM; i++)
    {
        snprintf(ns, 15, "matter_kvs1_%d", i+1); 
        ret = dct_open_module(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }

        len = sizeof(u32);
        ret = dct_get_variable_new(&handle, key, (char *)str, &len);
        if(ret == DCT_SUCCESS)
        {
            printf("checkExist key=%s found.\n", key);
            dct_close_module(&handle);
            goto exit;
        }

        len = sizeof(u64);
        ret = dct_get_variable_new(&handle, key, (char *)str, &len);
        if(ret == DCT_SUCCESS)
        {
            printf("checkExist key=%s found.\n", key);
            dct_close_module(&handle);
            goto exit;
        }

        dct_close_module(&handle);
    }

    // Loop over DCT2 modules
    for (size_t i=0; i<MODULE_NUM2; i++)
    {
        snprintf(ns, 15, "matter_kvs2_%d", i+1); 
        ret = dct_open_module2(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module2(%s) failed with error : %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }

        len = VARIABLE_VALUE_SIZE2;
        ret = dct_get_variable_new2(&handle, key, str, &len);
        if(ret == DCT_SUCCESS)
        {
            printf("checkExist key=%s found.\n", key);
            dct_close_module2(&handle);
            goto exit;
        }

        dct_close_module2(&handle);
    }

exit:
    free(str);
    return (ret == DCT_SUCCESS) ? true : false;
}

s32 setPref_new(const char *domain, const char *key, u8 *value, size_t byteCount)
{
    dct_handle_t handle;
    s32 ret;
    char ns[15];

    if (byteCount <= 64)
    {
        // Loop over DCT1 modules
        for (size_t i=0; i<MODULE_NUM; i++)
        {
            snprintf(ns, 15, "matter_kvs1_%d", i+1); 
            ret = dct_open_module(&handle, ns);
            if (ret != DCT_SUCCESS)
            {
                printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
                goto exit;
            }

            if (dct_remain_variable(&handle) > 0)
            {
#if CONFIG_ENABLE_DCT_ENCRYPTION
                ret = dct_set_encrypted_variable(&handle, key, value, byteCount, DCT_REGION_1);
#else
                ret = dct_set_variable_new(&handle, key, (char *)value, (uint16_t)byteCount);
#endif
                if (ret != DCT_SUCCESS)
                {
                    printf("%s : dct_set_variable(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);
                    dct_close_module(&handle);
                    goto exit;
                }
                dct_close_module(&handle);
                break;
            }
            dct_close_module(&handle);
        }
    }
    else
    {
        // Loop over DCT2 modules
        for (size_t i=0; i<MODULE_NUM2; i++)
        {
            snprintf(ns, 15, "matter_kvs2_%d", i+1); 
            ret = dct_open_module2(&handle, ns);
            if (ret != DCT_SUCCESS)
            {
                printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
                goto exit;
            }

            if (dct_remain_variable2(&handle) > 0)
            {
#if CONFIG_ENABLE_DCT_ENCRYPTION
                ret = dct_set_encrypted_variable(&handle, key, value, byteCount, DCT_REGION_2);
#else
                ret = dct_set_variable_new2(&handle, key, (char *)value, (uint16_t)byteCount);
#endif
                if (ret != DCT_SUCCESS)
                {
                    printf("%s : dct_set_variable2(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);
                    dct_close_module2(&handle);
                    goto exit;
                }
                dct_close_module2(&handle);
                break;
            }
            dct_close_module2(&handle);
        }
    }

exit:
    return ret;
}

s32 getPref_bool_new(const char *domain, const char *key, u8 *val)
{
    dct_handle_t handle;
    s32 ret;
    uint16_t len = sizeof(u8);
    char ns[15];

    // Loop over DCT1 modules
    for (size_t i=0; i<MODULE_NUM; i++)
    {
        snprintf(ns, 15, "matter_kvs1_%d", i+1); 
        ret = dct_open_module(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }
#if CONFIG_ENABLE_DCT_ENCRYPTION
        ret = dct_get_encrypted_variable(&handle, key, (char *)val, &len, DCT_REGION_1);
#else
        ret = dct_get_variable_new(&handle, key, (char *)val, &len);
#endif
        dct_close_module(&handle);
        if (ret == DCT_SUCCESS)
        {
            return ret;
        }
    }

    // Loop over DCT2 modules
    for (size_t i=0; i<MODULE_NUM2; i++)
    {
        snprintf(ns, 15, "matter_kvs2_%d", i+1); 
        ret = dct_open_module2(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }
#if CONFIG_ENABLE_DCT_ENCRYPTION
        ret = dct_get_encrypted_variable(&handle, key, (char *)val, &len, DCT_REGION_2);
#else
        ret = dct_get_variable_new2(&handle, key, (char *)val, &len);
#endif
        dct_close_module2(&handle);
        if (ret == DCT_SUCCESS)
        {
            return ret;
        }
    }

exit:
    return ret;
}

s32 getPref_u32_new(const char *domain, const char *key, u32 *val)
{
    dct_handle_t handle;
    s32 ret;
    uint16_t len = sizeof(u32);
    char ns[15];

    // Loop over DCT1 modules
    for (size_t i=0; i<MODULE_NUM; i++)
    {
        snprintf(ns, 15, "matter_kvs1_%d", i+1); 
        ret = dct_open_module(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }
#if CONFIG_ENABLE_DCT_ENCRYPTION
        ret = dct_get_encrypted_variable(&handle, key, (char *)val, &len, DCT_REGION_1);
#else
        ret = dct_get_variable_new(&handle, key, (char *)val, &len);
#endif
        dct_close_module(&handle);
        if (ret == DCT_SUCCESS)
        {
            return ret;
        }
    }

    // Loop over DCT2 modules
    for (size_t i=0; i<MODULE_NUM2; i++)
    {
        snprintf(ns, 15, "matter_kvs2_%d", i+1); 
        ret = dct_open_module2(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }
#if CONFIG_ENABLE_DCT_ENCRYPTION
        ret = dct_get_encrypted_variable(&handle, key, (char *)val, &len, DCT_REGION_2);
#else
        ret = dct_get_variable_new2(&handle, key, (char *)val, &len);
#endif
        dct_close_module2(&handle);
        if (ret == DCT_SUCCESS)
        {
            return ret;
        }
    }

exit:
    return ret;
}

s32 getPref_u64_new(const char *domain, const char *key, u64 *val)
{
    dct_handle_t handle;
    s32 ret;
    uint16_t len = sizeof(u64);
    char ns[15];

    // Loop over DCT1 modules
    for (size_t i=0; i<MODULE_NUM; i++)
    {
        snprintf(ns, 15, "matter_kvs1_%d", i+1); 
        ret = dct_open_module(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }
#if CONFIG_ENABLE_DCT_ENCRYPTION
        ret = dct_get_encrypted_variable(&handle, key, (char *)val, &len, DCT_REGION_1);
#else
        ret = dct_get_variable_new(&handle, key, (char *)val, &len);
#endif
        dct_close_module(&handle);
        if (ret == DCT_SUCCESS)
        {
            return ret;
        }
    }

    // Loop over DCT2 modules
    for (size_t i=0; i<MODULE_NUM2; i++)
    {
        snprintf(ns, 15, "matter_kvs2_%d", i+1); 
        ret = dct_open_module2(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }
#if CONFIG_ENABLE_DCT_ENCRYPTION
        ret = dct_get_encrypted_variable(&handle, key, (char *)val, &len, DCT_REGION_2);
#else
        ret = dct_get_variable_new2(&handle, key, (char *)val, &len);
#endif
        dct_close_module2(&handle);
        if (ret == DCT_SUCCESS)
        {
            return ret;
        }
    }

exit:
    return ret;
}

s32 getPref_str_new(const char *domain, const char *key, char * buf, size_t bufSize, size_t *outLen)
{
    dct_handle_t handle;
    s32 ret;
    char ns[15];

    // Loop over DCT1 modules
    for (size_t i=0; i<MODULE_NUM; i++)
    {
        snprintf(ns, 15, "matter_kvs1_%d", i+1); 
        ret = dct_open_module(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }
#if CONFIG_ENABLE_DCT_ENCRYPTION
        ret = dct_get_encrypted_variable(&handle, key, buf, &bufSize, DCT_REGION_1);
#else
        ret = dct_get_variable_new(&handle, key, buf, &bufSize);
#endif
        dct_close_module(&handle);
        if (ret == DCT_SUCCESS)
        {
            *outLen = bufSize;
            return ret;
        }
    }

    // Loop over DCT2 modules
    for (size_t i=0; i<MODULE_NUM2; i++)
    {
        snprintf(ns, 15, "matter_kvs2_%d", i+1); 
        ret = dct_open_module2(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }
#if CONFIG_ENABLE_DCT_ENCRYPTION
        ret = dct_get_encrypted_variable(&handle, key, buf, &bufSize, DCT_REGION_2);
#else
        ret = dct_get_variable_new2(&handle, key, buf, &bufSize);
#endif
        dct_close_module2(&handle);
        if (ret == DCT_SUCCESS)
        {
            *outLen = bufSize;
            return ret;
        }
    }

exit:
    return ret;
}

s32 getPref_bin_new(const char *domain, const char *key, u8 * buf, size_t bufSize, size_t *outLen)
{
    dct_handle_t handle;
    s32 ret;
    char ns[15];

    // Loop over DCT1 modules
    for (size_t i=0; i<MODULE_NUM; i++)
    {
        snprintf(ns, 15, "matter_kvs1_%d", i+1); 
        ret = dct_open_module(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }
#if CONFIG_ENABLE_DCT_ENCRYPTION
        ret = dct_get_encrypted_variable(&handle, key, (char *)buf, &bufSize, DCT_REGION_1);
#else
        ret = dct_get_variable_new(&handle, key, (char *)buf, &bufSize);
#endif
        dct_close_module(&handle);
        if (ret == DCT_SUCCESS)
        {
            *outLen = bufSize;
            return ret;
        }
    }

    // Loop over DCT2 modules
    for (size_t i=0; i<MODULE_NUM2; i++)
    {
        snprintf(ns, 15, "matter_kvs2_%d", i+1); 
        ret = dct_open_module2(&handle, ns);
        if (ret != DCT_SUCCESS)
        {
            printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, ns, ret);
            goto exit;
        }
#if CONFIG_ENABLE_DCT_ENCRYPTION
        ret = dct_get_encrypted_variable(&handle, key, (char *)buf, &bufSize, DCT_REGION_2);
#else
        ret = dct_get_variable_new2(&handle, key, (char *)buf, &bufSize);
#endif
        dct_close_module2(&handle);
        if (ret == DCT_SUCCESS)
        {
            *outLen = bufSize;
            return ret;
        }
    }

exit:
    return ret;
}

#if defined(DCT_UPDATE_ENABLE) && DCT_UPDATE_ENABLE
#include "utility.h"
#include <flash_api.h>
#include <device_lock.h>

/**
 * Please change the DCT_BEGIN_ADDR_OLD, DCT_BEGIN_ADDR2_OLD, MODULE_NUM_OLD and MODULE_NUM2_OLD before using updating DCT.
 */

#define MODULE_NAME_OFFSET      8
#define MODULE_INFO_SIZE        32
#define MODULE_NUM_OFFSET       44
#define DCT_BACKUP_OFFSET       52
#define BUFFER_SIZE 0x1000

/* ***************************************************
 * @brief DCT layout
 *****************************************************
 * Bit       Unit    Define
 * 0 - 3     4       DCT Signature e.g., DCT1, DCT2
 * 4 - 7     4       Module State
 * 8 - 39    32      Module Name
 * 40 - 43   4       Variable CRC
 * 44 - 45   2       Module Number
 * 46 - 47   2       Variable Name Size
 * 48 - 49   2       Variable Value Size
 * 50 - 51   2       Used Variable Num
 * 52        1       Backup
 * 53        1       Wear Leveling
 * 54 - 71   18      Reserved
 */

static uint8_t dct_value_is_changed(flash_t flash, uint32_t old_address, uint32_t new_address, uint8_t *buf, int *changed)
{
    uint8_t *read_buf_new = NULL;
    uint8_t ret = 0;

    // Allocate memory for reading the new address data
    read_buf_new = rtw_malloc(BUFFER_SIZE);
    if (!read_buf_new)
    {
        printf("[MATTER_DCT] buffer malloc failed\n");
        return (uint8_t)-1;
    }

    // Check if the address has changed
    if (old_address != new_address)
    {
        // Read DCT data from the new address
        device_mutex_lock(RT_DEV_LOCK_FLASH);
        flash_stream_read(&flash, new_address, BUFFER_SIZE, read_buf_new);
        device_mutex_unlock(RT_DEV_LOCK_FLASH);

        // If the new data is not empty (not all 0xFF)
        if (read_buf_new[0] != 0xFF)
        {
            // Compare the new buffer with the original buffer
            if (memcmp(read_buf_new, buf, BUFFER_SIZE) != 0)
            {
                *changed = 1;
            }
            else
            {
                ret = 1; // Indicate that both addresses have the same data
            }
        }
        else
        {
            *changed = 1; // New address is empty, mark it as changed
        }
    }

    // Free allocated memory
    rtw_free(read_buf_new);

    return ret;
}

static void dct_change_module_name(int loop, uint8_t *buf, int *changed, uint32_t new_address)
{
    char new_module_name[15]; // for matter v1.1 name change
    int value = loop + 1;

    // Check if the module name starts with "chip" and update if needed
    if (strncmp((const char *)(buf + MODULE_NAME_OFFSET), "chip", 4) == 0)
    {
        switch (new_address)
        {
            case DCT_BEGIN_ADDR_MATTER:
                snprintf(new_module_name, sizeof(new_module_name), "matter_kvs1_%d", value);
                break;
            case DCT_BEGIN_ADDR_MATTER2:
                snprintf(new_module_name, sizeof(new_module_name), "matter_kvs2_%d", value);
                break;
        }

        // Copy the new module name and clear the rest
        memcpy(buf + MODULE_NAME_OFFSET, new_module_name, sizeof(new_module_name));
        memset(buf + MODULE_NAME_OFFSET + sizeof(new_module_name), 0, MODULE_INFO_SIZE - MODULE_NAME_OFFSET - sizeof(new_module_name));

        *changed = 1;
    }
}

static void dct_module_num_is_changed(uint8_t *buf, int *changed, uint8_t module_num)
{
    if (buf[MODULE_NUM_OFFSET] != module_num)
    {
        buf[MODULE_NUM_OFFSET] = module_num;
        *changed = 1;
    }
}

static void dct_backup_is_changed(uint8_t *buf, int *changed)
{
    if (buf[DCT_BACKUP_OFFSET] != ENABLE_BACKUP)
    {
        buf[DCT_BACKUP_OFFSET] = ENABLE_BACKUP;
        *changed = 1;
    }
}

static void dct_manual_init(uint8_t *buffer, int loop, uint32_t new_address)
{
    uint8_t init_size = 72;
    uint8_t signature[4];
    uint32_t mod_state = 0xFFFFFFFE;
    char module_name[15]; // for matter v1.1 name change
    int value = loop + 1;

    uint32_t variable_crc = 0xFFFFFFFF;
    uint16_t module_num = 0, variable_name_size = 0, variable_value_size = 0, used_variable_num = 0;

    // Initialize the buffer to zeros
    memset(buffer, 0, init_size);

    // Set variables based on address
    if (new_address == DCT_BEGIN_ADDR_MATTER)
    {
        module_num = MODULE_NUM;
        snprintf(module_name, sizeof(module_name), "matter_kvs1_%d", value);
        memcpy(signature, "DCT1", 4);
        variable_name_size = VARIABLE_NAME_SIZE;
        variable_value_size = VARIABLE_VALUE_SIZE;
    }
    else if (new_address == DCT_BEGIN_ADDR_MATTER2)
    {
        module_num = MODULE_NUM2;
        snprintf(module_name, sizeof(module_name), "matter_kvs2_%d", value);
        memcpy(signature, "DCT2", 4);
        variable_name_size = VARIABLE_NAME_SIZE2;
        variable_value_size = VARIABLE_VALUE_SIZE2;
    }

    // Fill buffer
    memcpy(buffer, signature, sizeof(signature));
    memcpy(buffer + 4, &mod_state, sizeof(mod_state));
    memcpy(buffer + 8, module_name, sizeof(module_name));
    memcpy(buffer + 40, &variable_crc, sizeof(variable_crc));
    memcpy(buffer + 44, &module_num, sizeof(module_num));
    memcpy(buffer + 46, &variable_name_size, sizeof(variable_name_size));
    memcpy(buffer + 48, &variable_value_size, sizeof(variable_value_size));
    memcpy(buffer + 50, &used_variable_num, sizeof(used_variable_num));

    // Set flags for backup and wear leveling
    buffer[52] = ENABLE_BACKUP;
    buffer[53] = ENABLE_WEAR_LEVELING;

    return;
}

// Helper function to check the DCT signature and module name at a given address
int matter_dct1_check_signature_and_module_name(flash_t *flash, uint32_t address, uint16_t mod_num, const char *expected_name)
{
    uint8_t *buffer = rtw_malloc(BUFFER_SIZE);
    if (!buffer)
    {
        printf("[MATTER_DCT] malloc failed\n");
        return -1;
    }

    // Read the buffer from flash memory
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_stream_read(flash, address + (mod_num * BUFFER_SIZE), BUFFER_SIZE, buffer);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    // Check DCT signature and module name
    int result = (strncmp((const char *)buffer, "DCT1", 4) == 0 &&
                  strncmp((const char *)(buffer + MODULE_NAME_OFFSET), expected_name, strlen(expected_name)) == 0);

    rtw_free(buffer);
    return result;
}

// Helper function to check the DCT signature and module name at a given address
int matter_dct2_check_signature_and_module_name(flash_t *flash, uint32_t address, uint16_t mod_num, const char *expected_name)
{
    uint8_t *buffer = rtw_malloc(BUFFER_SIZE);
    if (!buffer)
    {
        printf("[MATTER_DCT] malloc failed\n");
        return -1;
    }

    // Read the buffer from flash memory
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_stream_read(flash, address + (mod_num * BUFFER_SIZE), BUFFER_SIZE, buffer);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    // Check DCT signature and module name
    int result = (strncmp((const char *)buffer, "DCT2", 4) == 0 &&
                  strncmp((const char *)(buffer + MODULE_NAME_OFFSET), expected_name, strlen(expected_name)) == 0);

    rtw_free(buffer);
    return result;
}

// Helper function to erase and write data to flash with verification
int matter_dct_flash_write_with_verification(flash_t *flash, uint32_t address, uint8_t *data, uint32_t size)
{
    uint8_t *verify_buf = rtw_malloc(size);
    if (!verify_buf)
    {
        printf("[MATTER_DCT] malloc failed\n");
        return -1;
    }

    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_erase_sector(flash, address);
    flash_stream_write(flash, address, size, data);
    flash_stream_read(flash, address, size, verify_buf);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    int result = (memcmp(data, verify_buf, size) == 0);
    rtw_free(verify_buf);
    return result;
}

// Helper function to read data from flash into a buffer
int matter_dct_flash_read(flash_t *flash, uint32_t address, uint8_t *buffer, uint32_t size)
{
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_stream_read(flash, address, size, buffer);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
    return 0;
}

// Function to handle updating new modules
void matter_dct_handle_new_modules(flash_t *flash, uint32_t old_address, uint32_t new_address, uint16_t
                              old_mod_num, uint16_t new_mod_num, uint8_t *read_buf, uint8_t *read_check)
{
    uint8_t kvs_num = 0, count_val = 0;
    uint8_t check_point = 0, k = 0;
    uint32_t new_record;

    for (int i = new_mod_num - 1; i >= old_mod_num; i--)
    {
        kvs_num = i + 1;

        if (i == new_mod_num - 1 || i == new_mod_num - 2)
        {
            count_val = 0;
        }
        else
        {
            count_val++;
        }

        new_record = new_address + (((new_mod_num * 2) - (new_mod_num - old_mod_num)) * BUFFER_SIZE) + count_val;
        memset(read_check, 0, BUFFER_SIZE);
        matter_dct_flash_read(flash, new_record, read_check, sizeof(uint8_t));
        check_point = read_check[0];

        // Read data from old and new backup addresses
        memset(read_buf, 0, BUFFER_SIZE);
        matter_dct_flash_read(flash, old_address + (i * BUFFER_SIZE), read_buf, BUFFER_SIZE);

        memset(read_check, 0, BUFFER_SIZE);
        matter_dct_flash_read(flash, new_address + (i * BUFFER_SIZE), read_check, sizeof(kvs_num));

        // If data is valid, perform updates
        if (read_buf[0] == 0xFF || read_check[0] == 0xFF)
        {
            if (check_point < kvs_num)
            {
                continue;
            }

            dct_manual_init(read_buf, i, new_address);
            if (!matter_dct_flash_write_with_verification(flash, new_address + (i  * BUFFER_SIZE), read_buf, BUFFER_SIZE))
            {
                printf("[MATTER_DCT] write failed\n");
                return;
            }
            if (!matter_dct_flash_write_with_verification(flash, new_address + ((i + new_mod_num) * BUFFER_SIZE), read_buf, BUFFER_SIZE))
            {
                printf("[MATTER_DCT] write failed\n");
                return;
            }

            if (i != old_mod_num)
            {
                new_record = new_address + (((new_mod_num * 2) - (new_mod_num - old_mod_num)) * BUFFER_SIZE) + k;
                k++;
                flash_stream_write(&flash, new_record, sizeof(kvs_num), &kvs_num);
            }
        }
        else
        {
            return;
        }
    }
}

// Main DCT update function
void matter_dct_update(uint8_t region, uint32_t old_address, uint32_t new_address, uint16_t old_mod_num, uint16_t new_mod_num)
{
    flash_t flash;
    uint8_t *read_buf = NULL, *read_check = NULL, kvs_num = 0;
    uint32_t new_record;
    int write_flash = 0, count_val = 0, k = 0;

    // Check if the DCT1 signature and module name are already correct at the new address
    if (region == DCT_REGION_1 &&
        matter_dct1_check_signature_and_module_name(&flash, new_address, new_mod_num, "matter_kvs1_1"))
    {
        goto cleanup; // Skip if valid
    }

    // Check if the DCT2 signature and module name are already correct at the new address
    if (region == DCT_REGION_2 &&
        matter_dct2_check_signature_and_module_name(&flash, new_address, new_mod_num, "matter_kvs2_1"))
    {
        goto cleanup; // Skip if valid
    }

    read_buf = rtw_malloc(BUFFER_SIZE);
    if (!read_buf)
    {
        printf("[MATTER_DCT] malloc failed\n");
        return;
    }

    read_check = rtw_malloc(BUFFER_SIZE);
    if (!read_check)
    {
        printf("[MATTER_DCT] malloc failed\n");
        goto cleanup;
    }

    // Loop over original and backup DCTs
    for (int j = 0; j <= 1; j++)
    {
        for (int i = old_mod_num - 1; i >= 0; i--)
        {
            kvs_num = i + 1;

            if (i == old_mod_num - 1 || i == old_mod_num - 2)
            {
                count_val = 0;
            }
            else
            {
                count_val++;
            }

            new_record = new_address + (((new_mod_num * 2) - 1) * BUFFER_SIZE) + count_val + (j * new_mod_num);
            memset(read_check, 0, BUFFER_SIZE);
            matter_dct_flash_read(&flash, new_record, read_check, sizeof(uint8_t));

            if ((read_check[0] < kvs_num) && (read_check[0] != 0xFF))
            {
                continue;
            }

            uint32_t old_offset = old_address + (i + (old_mod_num * j)) * BUFFER_SIZE;
            uint32_t new_offset = new_address + (i + (new_mod_num * j)) * BUFFER_SIZE;

            // Read old data
            memset(read_buf, 0, BUFFER_SIZE);
            matter_dct_flash_read(&flash, old_offset, read_buf, BUFFER_SIZE);

            // Skip if the DCT1 signature is not found
            if (region == DCT_REGION_1 && strncmp((const char *)read_buf, "DCT1", 4) != 0)
            {
                continue;
            }

            // Skip if the DCT2 signature is not found
            if (region == DCT_REGION_2 && strncmp((const char *)read_buf, "DCT2", 4) != 0)
            {
                continue;
            }

            // Perform DCT value change checks
            uint8_t ret = dct_value_is_changed(flash, old_offset, new_offset, read_buf, &write_flash);
            if (ret == 1)
            {
                continue; // No changes
            }

            // Apply DCT updates
            dct_change_module_name(i, read_buf, &write_flash, new_address);
            dct_backup_is_changed(read_buf, &write_flash);
            dct_module_num_is_changed(read_buf, &write_flash, new_mod_num);

            // Write updated data if changes were made
            if (write_flash)
            {
                if (!matter_dct_flash_write_with_verification(&flash, new_offset, read_buf, BUFFER_SIZE))
                {
                    printf("[MATTER_DCT] Flash write verification failed\n");
                    goto cleanup;
                }

                // If the new and old addresses are different, erase the old address
                if (new_offset != old_offset)
                {
                    device_mutex_lock(RT_DEV_LOCK_FLASH);
                    flash_erase_sector(&flash, old_offset);
                    device_mutex_unlock(RT_DEV_LOCK_FLASH);
                }

                if (i != 0)
                {
                    // Write kvs_num into the new record
                    new_record = new_address + (((new_mod_num * 2) - 1) * BUFFER_SIZE)  + k + (j * new_mod_num);
                    k++;
                    flash_stream_write(&flash, new_record, sizeof(kvs_num), &kvs_num);
                }

                write_flash = 0; // Reset flag
            }
        }
    }

    // Additional logic for handling new modules, similar to the above loop
    matter_dct_handle_new_modules(&flash, old_address, new_address, old_mod_num, new_mod_num, read_buf, read_check);

cleanup:
    if (read_buf)
    {
        rtw_free(read_buf);
    }
    if (read_check)
    {
        rtw_free(read_check);
    }
}

#endif

#ifdef __cplusplus
}
#endif
