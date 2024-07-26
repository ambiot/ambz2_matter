/**
    Implementation of Flash Filesystem with FAT driver
*/

#include "ff.h"
#include "platform_opts.h"
#include <platform/platform_stdlib.h>
#include "chip_porting.h"

#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 1)

#ifdef __cplusplus
extern "C" {
#endif

#include "fatfs_flash_api.h"
#include "matter_flashfs_fat.h"

static int bInited = 0;

int impl_flashfs_init(void)
{
    int err = fatfs_flash_init();
    if(err == FALSE)
    {
        bInited = 1;
    }

    return err;
}

int impl_flashfs_close(void)
{
    bInited = 0;
    return fatfs_flash_close();
}

int impl_flashfs_get_inited(void)
{
    return bInited;
}

int impl_flashfs_file_open(char* filename, void* fp, uint8_t mode)
{
    if(fp == NULL) 
        return FR_INVALID_PARAMETER;

    if(impl_flashfs_get_inited() == 0)
    {
        return FR_NO_FILESYSTEM;
    }

    fatfs_flash_params_t* fs_params = malloc(sizeof(fatfs_flash_params_t));
    if(fs_params == NULL)
    {
        return FR_NO_FILESYSTEM;
    }

    int err = fatfs_flash_get_param(fs_params);
    if(err != 0) 
    {
        return FR_NOT_READY;
    }

    if((filename == NULL) || (filename != NULL && strlen(filename) == 0)) 
    {
        return FR_INVALID_PARAMETER;
    }

    int res;
    char path[64];
    strcpy(path, fs_params->drv);
    sprintf(&path[strlen(path)],"%s",filename);
    free(fs_params);
    res = f_open((FIL*)fp, path, mode);
    if(res) 
    {
        printf("fserr: open file (%s) fail. res = %d\n\r", filename, res);
        return res;
    }

    return FR_OK;
}

int impl_flashfs_file_close(void* fp)
{
    if(fp == NULL)
        return FR_INVALID_PARAMETER;
    int res = f_close((FIL*)fp);
    if(res) {
        printf("fserr: close file fail res: %d\n", res);
        return res;
    }
}

int impl_flashfs_file_lseek(void* fp, uint32_t to)
{
    if(fp == NULL)
        return FR_INVALID_PARAMETER;                                        // check if fp is valid
    return f_lseek((FIL*)fp, to);   
}

int impl_flashfs_file_tell(void* fp)
{
    if(fp == NULL)
        return -1;                                                          // check if fp is valid
    return f_tell((FIL*)fp);
}

int impl_flashfs_file_size(void* fp)
{
    if(fp == NULL)
        return -1;                                                          // check if fp is valid
    return f_size((FIL*)fp);
}

int impl_flashfs_file_read(void* fp, uint32_t in_position, uint32_t in_count, void* out_buf, uint32_t out_buflen, uint* out_num)
{
    int res = 0;
    int max, to_read;
    *out_num = 0;
    
    if(fp == NULL)          return FR_INVALID_PARAMETER;                    // check if fp is valid
    if(in_count == 0)       return FR_INVALID_PARAMETER;                    // cannot read zero bytes
    if(out_buf == NULL)     return FR_INVALID_PARAMETER;                    // output buffer cannot be null

    max = f_size((FIL*)fp);                                                 // get file size

    if(max == 0)                                                            // if the file is empty, return an OK value and set out_count to zero
    {
        *out_num = max;
        return FR_OK;
    }

    if(in_position == 0)                                                    // read from start of file, otherwise seek to start
    {
        if(in_count > max)                                                  // cannot read more than file size
        {
            to_read = max;
        }
        else
        {
            to_read = in_count;
        }

        res = f_lseek((FIL*)fp, in_position);                               // seek to start of file
        if(res) 
        {
            printf("fserr: seek failed %d in %s", res, __func__);
            return res;
        }

        res = f_read((FIL*)fp, out_buf, to_read, out_num);                  // read file
        if(res) 
        {
            printf("fserr: read failed %d in %s", res, __func__);
            return res;
        }

        return res;
    }
    else
    {
        if(in_position > max)                                               // check if seek position is valid
        {
            printf("fserr: cannot place cursor beyond max file size, use ftell first");
            return FR_INVALID_PARAMETER;
        }

        if(in_count > (max - in_position))                                  // check if in_count exceeds (size - cursor)
        {
            to_read = max - in_position;                                    // clamp to difference in bytes
        }
        else
        {
            to_read = in_count;                                             // otherwise set to the count
        }

        res = f_lseek((FIL*)fp, in_position);                               // seek to cursor
        if(res) 
        {
            printf("fserr: lseek failed %d in %s", res, __func__);
            return res;
        }

        res = f_read((FIL*)fp, out_buf, to_read, out_num);
        if(res) 
        {
            printf("fserr: read failed %d in %s", res, __func__);
            return res;
        }
    }

    return FR_OK;
}

int impl_flashfs_file_write(void* fp, const void* in_buf, uint32_t in_buflen, uint32_t in_bufstart, uint32_t in_writestart, bool bAppend, uint* out_num)
{
    if(fp == NULL)          return FR_INVALID_PARAMETER;                    // check if fp is valid
    if(in_buf == NULL)      return FR_INVALID_PARAMETER;                    // input buffer cannot be null
    if(in_buflen == 0)      return FR_INVALID_PARAMETER;                    // input buffer length cannot be null
    if(in_bufstart > in_buflen) return FR_INVALID_PARAMETER;                // bufstart cannot be more than buflen

    uint32_t write_pos = 0;
    if(bAppend == TRUE)
    {
        write_pos = f_tell((FIL*)fp);
    }
    else
    {
        write_pos = in_writestart;
    }

    int err = f_lseek((FIL*)fp, write_pos);                                 // seek to write position
    err = f_write((FIL*)fp, in_buf, in_buflen, out_num);                    // write the data

    // TODO IMPLEMENT LOOP WRITES IF MORE THAN FLASH BUFSIZE
    return err;
}

int impl_flashfs_file_reset(void* fp)
{
    if(fp == NULL)          return FR_INVALID_PARAMETER;

    return f_lseek((FIL*)fp, 0);
}

int impl_flashfs_file_list(char* path)
{
    int res;    // FRESULT
    FILINFO fno;
    DIR dir;
    char *fn;                                                               /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
    static char lfn[_MAX_LFN + 1];
    fno.lfname = lfn;
    fno.lfsize = sizeof(lfn);
#endif


    res = f_opendir(&dir, path);                                            /* Open the directory */
    if (res == FR_OK) {
        //i = strlen(path);
        for (;;) {
            res = f_readdir(&dir, &fno);                                    /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;                   /* Break on error or end of dir */
            if (fno.fname[0] == '.') continue;                              /* Ignore dot entry */
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif
            if (fno.fattrib & AM_DIR) {                                     /* It is a directory */
                sprintf(path, "/%s", fn);
                res = impl_flashfs_file_list(path);
                if (res != FR_OK) break;
                path = 0;
            } else {                                                        /* It is a file. */
                printf("%s/%s\r\n", path, fn);
            }
        }
    }

    return res;
}

#ifdef __cplusplus
}
#endif

#endif