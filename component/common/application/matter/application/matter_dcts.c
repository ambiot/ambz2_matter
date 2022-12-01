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
#include "dct.h"
#include "chip_porting.h"
/*
   module size is 4k, we set max module number as 12;
   if backup enabled, the total module number is 12 + 1*12 = 24, the size is 96k;
   if wear leveling enabled, the total module number is 12 + 2*12 + 3*12 = 36, the size is 288k"
*/
#define DCT_BEGIN_ADDR_MATTER   DCT_BEGIN_ADDR    /*!< DCT begin address of flash, ex: 0x100000 = 1M */
#define MODULE_NUM              13                /*!< max number of module */
#define VARIABLE_NAME_SIZE      32                /*!< max size of the variable name */
#define VARIABLE_VALUE_SIZE     64 + 4          /*!< max size of the variable value */
                                                  /*!< max value number in moudle = 4024 / (32 + 1860+4) = 2 */

#define DCT_BEGIN_ADDR_MATTER2  DCT_BEGIN_ADDR2
#define MODULE_NUM2             6 
#define VARIABLE_NAME_SIZE2     32
#define VARIABLE_VALUE_SIZE2    400 + 4

#define ENABLE_BACKUP           0
#define ENABLE_WEAR_LEVELING    0

const char *matter_domain[19] =
{
    "chip-factory",
    "chip-config",
    "chip-counters",
    "chip-fabric-1",
    "chip-fabric-2",
    "chip-fabric-3",
    "chip-fabric-4",
    "chip-fabric-5",
    "chip-acl",
    "chip-groupmsgcounters",
    "chip-attributes",
    "chip-bindingtable",
    "chip-ota",
    "chip-failsafe",
    "chip-sessionresumption",
    "chip-deviceinfoprovider",
    "chip-groupdataprovider",
    "chip-others2",
    "chip-others"
};

/*
 * allocate into predefined domains
 * needs to be more robust, but currently it gets the job done
 * chip-groupdataproviders is put into their respective chip-fabric-x
 */

/*
 * domainAllocator allocates key-value data in domain 1
 */
const char* domainAllocator(const char *domain)
{
    //chip-factory
    if(strcmp(domain, "chip-factory") == 0)
        return matter_domain[0];
    //chip-config
    if(strcmp(domain, "chip-config") == 0)
        return matter_domain[1];
    //chip-counters
    if(strcmp(domain, "chip-counters") == 0)
        return matter_domain[2];
    // chip-acl
    if(strncmp(domain+4, "ac", 2) == 0)
    {
        // acl extension
        if(strncmp(domain+7, "1", 1) == 0)
            return matter_domain[17];

        return matter_domain[8];
    }

    if(domain[0] == 'f')
    {
        // chip-groupdataprovider
        if(strncmp(domain+4, "g", 1) == 0) 
            return matter_domain[16];

        // chip-fabrics
        switch(atoi(&domain[2]))
        {
            case 1:
                return matter_domain[3];
                break;
            case 2:
                return matter_domain[4];
                break;
            case 3:
                return matter_domain[5];
                break;
            case 4:
                return matter_domain[6];
                break;
            case 5:
                return matter_domain[7];
                break;
        }
    }
    // chip-groupmsgcounters
    if((strcmp(domain, "g/gdc") == 0) || (strcmp(domain, "g/gcc") == 0))
        return matter_domain[9];
    // chip-attributes
    if(strncmp(domain, "g/a", 3) == 0)
        return matter_domain[10];
    // chip-bindingtable
    if(strncmp(domain, "g/bt", 4) == 0)
        return matter_domain[11];
    // chip-ota
    if(strncmp(domain, "g/o", 3) == 0)
        return matter_domain[12];
    // chip-failsafe
    if(strncmp(domain, "g/fs", 4) == 0)
        return matter_domain[13];
    // chip-sessionresumption
    if((strncmp(domain, "g/s", 3) == 0) && strcmp(domain, "g/sri") != 0)
        return matter_domain[14];
    // chip-deviceinfoprovider
    if(strncmp(domain, "g/userlbl", 9) == 0)
        return matter_domain[15];
    // chip-others2
    // Store KV pairs that can't fit in chip-others (>64bytes)
    if((strcmp(domain, "wifi-pass") == 0) || (strcmp(domain, "g/sri") == 0))
        return matter_domain[17];
    // chip-others
    // store FabricTable, FailSafeContextKey, GroupFabricList, FabricIndexInfo, IMEventNumber in chip-others
    return matter_domain[18];
}

/*
 * Assigns DCT region to an allocated domain
 * retval:
 *      1 => DCT1 region
 *      2 => DCT2 region
 */
u8 allocateRegion(const char *allocatedDomain)
{
    if (strncmp(allocatedDomain, "chip-fabric", 11) == 0)
        return 2;
    if (strcmp(allocatedDomain, "chip-others2") == 0)
        return 2;
    else
        return 1;
}

s32 initPref(void)
{
    s32 ret;
    ret = dct_init(DCT_BEGIN_ADDR_MATTER, MODULE_NUM, VARIABLE_NAME_SIZE, VARIABLE_VALUE_SIZE, ENABLE_BACKUP, ENABLE_WEAR_LEVELING);
    if (ret != 0)
        printf("dct_init failed with error: %d\n", ret);
    else
        printf("dct_init success\n");

    ret = dct_init2(DCT_BEGIN_ADDR_MATTER2, MODULE_NUM2, VARIABLE_NAME_SIZE2, VARIABLE_VALUE_SIZE2, ENABLE_BACKUP, ENABLE_WEAR_LEVELING);
    if (ret != 0)
        printf("dct_init2 failed with error: %d\n", ret);
    else
        printf("dct_init2 success\n");

    return ret;
}

s32 deinitPref(void)
{
    s32 ret;
    ret = dct_format(DCT_BEGIN_ADDR_MATTER, MODULE_NUM, VARIABLE_NAME_SIZE, VARIABLE_VALUE_SIZE, ENABLE_BACKUP, ENABLE_WEAR_LEVELING);
    if (ret != 0)
        printf("dct_format failed with error: %d\n", ret);
    else
        printf("dct_format success\n");

    ret = dct_format2(DCT_BEGIN_ADDR_MATTER2, MODULE_NUM2, VARIABLE_NAME_SIZE2, VARIABLE_VALUE_SIZE2, ENABLE_BACKUP, ENABLE_WEAR_LEVELING);
    if (ret != 0)
        printf("dct_format2 failed with error: %d\n", ret);
    else
        printf("dct_format2 success\n");

    return ret;
}

s32 registerPref(const char * ns)
{
    s32 ret;
    ret = dct_register_module(ns);
    if (ret != 0)
        printf("dct_register_module %s failed with error: %d\n",ns, ret);
    else
        printf("dct_register_module %s success\n",ns);

    return ret;
}

s32 registerPref2(const char * ns)
{
    s32 ret;
    ret = dct_register_module2(ns);
    if (ret != 0)
        printf("dct_register_module2 %s failed with error: %d\n",ns, ret);
    else
        printf("dct_register_module2 %s success\n",ns);

    return ret;
}


s32 clearPref(const char * ns)
{
    s32 ret;
    ret = dct_unregister_module(ns);
    if (ret != 0)
        printf("dct_unregister_module %s failed with error: %d\n",ns, ret);
    else
        printf("dct_unregister_module %s success\n",ns);

    return ret;
}

s32 deleteKey(const char *domain, const char *key)
{
    dct_handle_t handle;
    s32 ret = -1;
    const char *allocatedDomain = domainAllocator(domain);
    u8 allocatedRegion = allocateRegion(allocatedDomain);

    if (allocatedRegion == 1)
    {
        ret = dct_open_module(&handle, allocatedDomain);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        ret = dct_delete_variable(&handle, key);
        dct_close_module(&handle);
        if(DCT_SUCCESS != ret)
        {
            printf("%s : dct_delete_variable(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

            if (DCT_ERR_NOT_FIND == ret) // Only enter here if variable is not found in region1
            {
                // Now we search for this variable in chip-others2
                allocatedDomain = matter_domain[17];

                ret = dct_open_module2(&handle, allocatedDomain);
                if (DCT_SUCCESS != ret)
                {
                    printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
                    goto exit;
                }

                ret = dct_delete_variable2(&handle, key);
                if(DCT_SUCCESS != ret)
                    printf("%s : dct_delete_variable(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

                dct_close_module2(&handle);
            }
        }

    }
    else
    {
        ret = dct_open_module2(&handle, allocatedDomain);
        if (DCT_SUCCESS != ret){
            printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        ret = dct_delete_variable2(&handle, key);
        if(DCT_SUCCESS != ret)
            printf("%s : dct_delete_variable(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

        dct_close_module2(&handle);
        //dct_unregister_module2(key);
    }

exit:
    return ret;
}

BOOL checkExist(const char *domain, const char *key)
{
    dct_handle_t handle;
    s32 ret = -1;
    uint16_t len = 0;
    u8 found = 0;
    u8 *str = malloc(sizeof(u8) * VARIABLE_VALUE_SIZE-4);
    const char *allocatedDomain = domainAllocator(domain);
    u8 allocatedRegion = allocateRegion(allocatedDomain);

    if (allocatedRegion == 1)
    {
        ret = dct_open_module(&handle, allocatedDomain);
        if (ret != DCT_SUCCESS){
            printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        if(found == 0)
        {
            len = sizeof(u32);
            ret = dct_get_variable_new(&handle, key, (char *)str, &len);
            if(ret == DCT_SUCCESS)
            {
                printf("checkExist key=%s found.\n", key);
                found = 1;
            }
        }

        if(found == 0)
        {
            len = sizeof(u64);
            ret = dct_get_variable_new(&handle, key, (char *)str, &len);
            if(ret == DCT_SUCCESS)
            {
                printf("checkExist key=%s found.\n", key);
                found = 1;
            }
        }

        dct_close_module(&handle);

        // Find in chip-others2 if not found
        if(found == 0)
        {
            allocatedDomain = matter_domain[17];
            ret = dct_open_module2(&handle, allocatedDomain);
            if (ret != DCT_SUCCESS){
                printf("%s : dct_open_module2(%s) failed with error : %d\n" ,__FUNCTION__, allocatedDomain, ret);
                goto exit;
            }

            len = VARIABLE_VALUE_SIZE-4;
            ret = dct_get_variable_new2(&handle, key, str, &len);
            if(ret == DCT_SUCCESS)
            {
                printf("checkExist key=%s found.\n", key);
                found = 1;
            }

            dct_close_module2(&handle);
        }
    }
    else
    {
        ret = dct_open_module2(&handle, allocatedDomain);
        if (ret != DCT_SUCCESS){
            printf("%s : dct_open_module2(%s) failed with error : %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        len = VARIABLE_VALUE_SIZE-4;
        ret = dct_get_variable_new2(&handle, key, str, &len);
        if(ret == DCT_SUCCESS)
        {
            printf("checkExist key=%s found.\n", key);
            found = 1;
        }

        dct_close_module2(&handle);
    }

    if(found == 0)
        printf("checkExist key=%s not found. ret=%d\n",key ,ret);

exit:
    free(str);
    return found;
}

s32 setPref_new(const char *domain, const char *key, u8 *value, size_t byteCount)
{
    dct_handle_t handle;
    s32 ret = -1;
    const char *allocatedDomain;
    u8 allocatedRegion;
    allocatedDomain = domainAllocator(domain);
    allocatedRegion = allocateRegion(allocatedDomain);

    if (allocatedRegion == 1)
    {
        if (byteCount > 64) // This is a **new unknown key** that should be stored in chip-others2 inside region2
        {
            allocatedDomain = matter_domain[17];

            ret = dct_open_module2(&handle, allocatedDomain);
            if (DCT_SUCCESS != ret)
            {
                printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
                goto exit;
            }

            ret = dct_set_variable_new2(&handle, key, (char *)value, (uint16_t)byteCount);
            if (DCT_SUCCESS != ret)
                printf("%s : dct_set_variable2(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

            dct_close_module2(&handle);
        }
        else
        {
            ret = dct_open_module(&handle, allocatedDomain);
            if (DCT_SUCCESS != ret)
            {
                printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
                goto exit;
            }

            ret = dct_set_variable_new(&handle, key, (char *)value, (uint16_t)byteCount);
            if (DCT_SUCCESS != ret)
                printf("%s : dct_set_variable(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

            dct_close_module(&handle);
        }
    }
    else
    {
        ret = dct_open_module2(&handle, allocatedDomain);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        ret = dct_set_variable_new2(&handle, key, (char *)value, (uint16_t)byteCount);
        if (DCT_SUCCESS != ret)
            printf("%s : dct_set_variable2(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

        dct_close_module2(&handle);
    }

exit:
    return (DCT_SUCCESS == ret ? 1 : 0);
}

s32 getPref_bool_new(const char *domain, const char *key, u8 *val)
{
    dct_handle_t handle;
    s32 ret = -1;
    uint16_t len = 0;
    const char *allocatedDomain = domainAllocator(domain);
    u8 allocatedRegion = allocateRegion(allocatedDomain);

    if (allocatedRegion == 1)
    {
        ret = dct_open_module(&handle, allocatedDomain);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        len = sizeof(u8);
        ret = dct_get_variable_new(&handle, key, (char *)val, &len);
        dct_close_module(&handle);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_get_variable(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

            if (DCT_ERR_NOT_FIND == ret) // Only enter here if variable is not found in region1
            {
                // Now we search for this variable in chip-others2
                allocatedDomain = matter_domain[17];

                ret = dct_open_module2(&handle, allocatedDomain);
                if (DCT_SUCCESS != ret)
                {
                    printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
                    goto exit;
                }

                len = sizeof(u8);
                ret = dct_get_variable_new2(&handle, key, (char *)val, &len);
                if (DCT_SUCCESS != ret)
                    printf("%s : dct_get_variable2(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

                dct_close_module2(&handle);
            }
        }
    }
    else
    {
        ret = dct_open_module2(&handle, allocatedDomain);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        len = sizeof(u8);
        ret = dct_get_variable_new2(&handle, key, (char *)val, &len);
        if (DCT_SUCCESS != ret)
            printf("%s : dct_get_variable2(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

        dct_close_module2(&handle);
    }

exit:
    return ret;
}

s32 getPref_u32_new(const char *domain, const char *key, u32 *val)
{
    dct_handle_t handle;
    s32 ret = -1;
    uint16_t len = 0;
    const char *allocatedDomain = domainAllocator(domain);
    u8 allocatedRegion = allocateRegion(allocatedDomain);

    if (allocatedRegion == 1)
    {
        ret = dct_open_module(&handle, allocatedDomain);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        len = sizeof(u32);
        ret = dct_get_variable_new(&handle, key, (char *)val, &len);
        dct_close_module(&handle);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_get_variable(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

            if (DCT_ERR_NOT_FIND == ret) // Only enter here if variable is not found in region1
            {
                // Now we search for this variable in chip-others2
                allocatedDomain = matter_domain[17];

                ret = dct_open_module2(&handle, allocatedDomain);
                if (DCT_SUCCESS != ret)
                {
                    printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
                    goto exit;
                }

                len = sizeof(u32);
                ret = dct_get_variable_new2(&handle, key, (char *)val, &len);
                if (DCT_SUCCESS != ret)
                    printf("%s : dct_get_variable2(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

                dct_close_module2(&handle);
            }
        }
    }
    else
    {
        ret = dct_open_module2(&handle, allocatedDomain);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        len = sizeof(u32);
        ret = dct_get_variable_new2(&handle, key, (char *)val, &len);
        if (DCT_SUCCESS != ret)
            printf("%s : dct_get_variable2(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

        dct_close_module2(&handle);
    }
exit:
    return ret;
}

s32 getPref_u64_new(const char *domain, const char *key, u64 *val)
{
    dct_handle_t handle;
    s32 ret = -1;
    uint16_t len = 0;
    const char *allocatedDomain = domainAllocator(domain);
    u8 allocatedRegion = allocateRegion(allocatedDomain);

    if (allocatedRegion == 1)
    {
        ret = dct_open_module(&handle, allocatedDomain);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        len = sizeof(u64);
        ret = dct_get_variable_new(&handle, key, (char *)val, &len);
        dct_close_module(&handle);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_get_variable(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

            if (DCT_ERR_NOT_FIND == ret) // Only enter here if variable is not found in region1
            {
                // Now we search for this variable in chip-others2
                allocatedDomain = matter_domain[17];

                ret = dct_open_module2(&handle, allocatedDomain);
                if (DCT_SUCCESS != ret)
                {
                    printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
                    goto exit;
                }

                len = sizeof(u64);
                ret = dct_get_variable_new2(&handle, key, (char *)val, &len);
                if (DCT_SUCCESS != ret)
                    printf("%s : dct_get_variable2(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

                dct_close_module2(&handle);
            }
        }
    }
    else
    {
        ret = dct_open_module2(&handle, allocatedDomain);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        len = sizeof(u64);
        ret = dct_get_variable_new2(&handle, key, (char *)val, &len);
        if (DCT_SUCCESS != ret)
            printf("%s : dct_get_variable2(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

        dct_close_module2(&handle);
    }

exit:
    return ret;
}

s32 getPref_str_new(const char *domain, const char *key, char * buf, size_t bufSize, size_t *outLen)
{
    dct_handle_t handle;
    s32 ret = -1;
    uint16_t _bufSize = bufSize;
    const char *allocatedDomain = domainAllocator(domain);
    u8 allocatedRegion = allocateRegion(allocatedDomain);

    if (allocatedRegion == 1)
    {
        ret = dct_open_module(&handle, allocatedDomain);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_open_module(%s) failed with error: %d\n",__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        ret = dct_get_variable_new(&handle, key, buf, &_bufSize);
        dct_close_module(&handle);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_get_variable(%s) failed with error: %d\n",__FUNCTION__, key, ret);
            *outLen = _bufSize;

            if (DCT_ERR_NOT_FIND == ret) // Only enter here if variable is not found in region1
            {
                // Now we search for this variable in chip-others2
                allocatedDomain = matter_domain[17];

                ret = dct_open_module2(&handle, allocatedDomain);
                if (DCT_SUCCESS != ret)
                {
                    printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
                    goto exit;
                }

                ret = dct_get_variable_new2(&handle, key, buf, &_bufSize);
                if (DCT_SUCCESS != ret)
                    printf("%s : dct_get_variable2(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

                *outLen = bufSize;
                dct_close_module2(&handle);
            }
        }
        else
            *outLen = _bufSize;
    }
    else
    {
        ret = dct_open_module2(&handle, allocatedDomain);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        ret = dct_get_variable_new2(&handle, key, buf, &_bufSize);
        if (DCT_SUCCESS != ret)
            printf("%s : dct_get_variable2(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

        *outLen = _bufSize;

        dct_close_module2(&handle);
    }

exit:
    return ret;
}

s32 getPref_bin_new(const char *domain, const char *key, u8 * buf, size_t bufSize, size_t *outLen)
{
    dct_handle_t handle;
    s32 ret = -1;
    uint16_t _bufSize = bufSize;
    const char *allocatedDomain = domainAllocator(domain);
    u8 allocatedRegion = allocateRegion(allocatedDomain);

    if (allocatedRegion == 1)
    {
        ret = dct_open_module(&handle, allocatedDomain);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_open_module(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        ret = dct_get_variable_new(&handle, key, (char *)buf, &_bufSize);
        dct_close_module(&handle);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_get_variable(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);
            *outLen = _bufSize;

            if (DCT_ERR_NOT_FIND == ret) // Only enter here if variable is not found in region1
            {
                // Now we search for this variable in chip-others2
                allocatedDomain = matter_domain[17];

                ret = dct_open_module2(&handle, allocatedDomain);
                if (DCT_SUCCESS != ret)
                {
                    printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
                    goto exit;
                }

                ret = dct_get_variable_new2(&handle, key, buf, &_bufSize);
                if (DCT_SUCCESS != ret)
                    printf("%s : dct_get_variable2(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

                *outLen = bufSize;
                dct_close_module2(&handle);
            }
        }
        else
            *outLen = _bufSize;
    }
    else
    {
        ret = dct_open_module2(&handle, allocatedDomain);
        if (DCT_SUCCESS != ret)
        {
            printf("%s : dct_open_module2(%s) failed with error: %d\n" ,__FUNCTION__, allocatedDomain, ret);
            goto exit;
        }

        ret = dct_get_variable_new2(&handle, key, (char *)buf, &_bufSize);
        if (DCT_SUCCESS != ret)
            printf("%s : dct_get_variable2(%s) failed with error: %d\n" ,__FUNCTION__, key, ret);

        *outLen = _bufSize;

        dct_close_module2(&handle);
    }

exit:
    return ret;
}

#ifdef __cplusplus
}
#endif
