#include "matter_fs.h"
#include "matter/api/matter_log_api.h"
#include "platform_opts_matter.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined (CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 0)

#include "littlefs/r2.50/lfs.h"
#include "littlefs/littlefs_adapter.h"

// variables used by the filesystem
lfs_t lfs;

uint32_t VFS1_FLASH_BASE_ADDR = LITTLEFS_START_ADDR;
uint32_t VFS1_FLASH_SIZE = LITTLEFS_MAX_SIZE;

static int bInited = 0;

// configuration of the filesystem is provided by this struct
// g_nor_lfs_cfg

int matter_fs_init() 
{
    char msg[32] = { 0 };
    int err = lfs_mount(&lfs, &g_nor_lfs_cfg);
    sprintf(msg, "lfs_mount1: %d\n", err);
    CHIP_LOG_DETAIL(msg);

    if (err) 
    {
        err = lfs_format(&lfs, &g_nor_lfs_cfg);
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "lfs_format: %d\n", err);
        CHIP_LOG_ERROR(msg);

        err = lfs_mount(&lfs, &g_nor_lfs_cfg);
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "lfs_mount2: %d\n", err);
        CHIP_LOG_ERROR(msg);
    }

    if(err == 0) 
    {
        bInited = 1;
    }

    return err;
}

int matter_fs_close() 
{
    bInited = 0;
    return lfs_unmount(&lfs);
}

int matter_fs_get_init() 
{
    return bInited;
}

// ====================================================================================

int matter_fs_fopen(char *filename, void *fp, int mode) 
{
    if(fp == NULL) 
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*) fp;

    int err = lfs_file_open(&lfs, lfs_fp, filename, mode);
    if(err < 0)
    {
        printf("fopen err: %s -> %d\n", filename, mode);
    } 
    else 
    {
        printf("fopen OK: %s\n", filename);
    }
    return err;
}

int matter_fs_fclose(void *fp) 
{
    if(fp == NULL) 
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*)fp;
    
    return lfs_file_close(&lfs, lfs_fp);
}

int matter_fs_fread(void *fp, uint32_t in_position, uint32_t in_count, void *out_buf, uint32_t out_buflen, uint *out_num) 
{
    if(fp == NULL) 
    {
        return LFS_ERR_INVAL;
    }

    if(out_buf == NULL)
    {
        return LFS_ERR_INVAL;
    }
    
    if(out_buflen == 0) 
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    int max = lfs_file_size(&lfs, lfs_fp);
    int to_read;
    int res = 0;

    if(max == 0)
    {
        *out_num = max;
        return LFS_ERR_OK;
    }

    if(in_position == 0)    // beginning of file
    {
        if(in_count > max)
        {
            to_read = max;
        } 
        else 
        {
            to_read = in_count;
        }

        // cannot read more than the provided buffer
        if(to_read > out_buflen)
        {
            to_read = out_buflen;
        }

        // seek
        res = lfs_file_seek(&lfs, lfs_fp, 0, LFS_SEEK_SET);
        if(res < 0)
        {
            printf("fserr: seek failed %d in %s", res, __func__);
            return res;
        }

        // read
        res = lfs_file_read(&lfs, lfs_fp, out_buf, to_read);
        if(res < 0)
        {
            printf("fserr: read failed %d in %s", res, __func__);
            return res;
        }
        *out_num = res;
    } 
    else 
    {
        // anywhere in file
        if(in_position > max)
        {
            return LFS_ERR_INVAL;               // cannot seek more than max
        }

        if(in_count > (max - in_position))
        {
            to_read = max - in_position;        // clamp to difference in bytes
        }
        else 
        {
            to_read = in_count;
        }

        // seek
        res = lfs_file_seek(&lfs, lfs_fp, in_position, LFS_SEEK_SET);
        if(res < 0)
        {
            printf("fserr: seek failed %d in %s", res, __func__);
            return res;
        }

        // read
        res = lfs_file_read(&lfs, lfs_fp, out_buf, to_read);
        if(res < 0)
        {
            printf("fserr: read failed %d in %s", res, __func__);
            return res;
        }
        *out_num = res;
    }

    return LFS_ERR_OK;
}

int matter_fs_fwrite(void *fp, const void *in_buf, uint32_t in_buflen, uint32_t in_bufstart, uint32_t in_writestart, bool bAppend, uint *out_num) 
{
    if(fp == NULL) 
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    int32_t status = lfs_file_write(&lfs, lfs_fp, in_buf, in_buflen);

    if(status >= 0) 
    {
        *out_num = status;
        lfs_file_sync(&lfs, lfs_fp);                // write changes after write successful
    } 
    else 
    {
        return status;
    }

    return LFS_ERR_OK;
}

int matter_fs_fseek(void *fp, uint32_t to, uint32_t whence) 
{
    if(fp == NULL) 
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    return lfs_file_seek(&lfs, lfs_fp, to, whence);  // whence: 0 = SEEK_SET, 1 = SEEK_CUR, 2 = SEEK_END
}

int matter_fs_ftell(void* fp) 
{
    if(fp == NULL) 
    {
        return LFS_ERR_INVAL;
    }
    
    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    return lfs_file_tell(&lfs, lfs_fp);
}

int matter_fs_fsize(void *fp) 
{
    if(fp == NULL) 
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    return lfs_file_size(&lfs, lfs_fp);
}

int matter_fs_fclear(void* fp) 
{
    if(fp == NULL) return LFS_ERR_INVAL;
    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    // truncate to 0 effectively clears the file
    return lfs_file_truncate(&lfs, lfs_fp, 0);
}

#else  // CONFIG_AMEBA_LOGS_USE_FATFS == 1

#include "ff.h"
#include "fatfs_flash_api.h"

static int bInited = 0;

int matter_fs_init() 
{
    int err = fatfs_flash_init();
    if(err == FALSE)
    {
        bInited = 1;
    }

    return err;
}

int matter_fs_close() 
{
    bInited = 0;
    return fatfs_flash_close();
}

int matter_fs_get_init() 
{
    return bInited;
}

// ====================================================================================

int matter_fs_fopen(char* filename, void* fp, uint8_t mode) 
{
    if(fp == NULL) {
        return FR_INVALID_PARAMETER;
    }

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

int matter_fs_fclose(void* fp)
{
    if(fp == NULL) {
        return FR_INVALID_PARAMETER;
    }
    
    int res = f_close((FIL*)fp);
    if(res) 
    {
        printf("fserr: close file fail res: %d\n", res);
        return res;
    }
}

int matter_fs_fread(void* fp, uint32_t in_position, uint32_t in_count, void* out_buf, uint32_t out_buflen, uint* out_num)
{
    int res = 0;
    int max, to_read;
    *out_num = 0;
    
    if(fp == NULL) {
        return FR_INVALID_PARAMETER;                    // check if fp is valid
    }
    
    if(in_count == 0) {
        return FR_INVALID_PARAMETER;                    // cannot read zero bytes
    }
    
    if(out_buf == NULL) {
        return FR_INVALID_PARAMETER;                    // output buffer cannot be null
    }

    max = f_size((FIL*)fp);                             // get file size

    if(max == 0)                                        // if the file is empty, return an OK value and set out_count to zero
    {
        *out_num = max;
        return FR_OK;
    }

    if(in_position == 0)                                // read from start of file, otherwise seek to start
    {
        if(in_count > max)                              // cannot read more than file size
        {
            to_read = max;
        }
        else
        {
            to_read = in_count;
        }

        res = f_lseek((FIL*)fp, in_position);            // seek to start of file
        if(res) 
        {
            printf("fserr: seek failed %d in %s", res, __func__);
            return res;
        }

        res = f_read((FIL*)fp, out_buf, to_read, out_num);  // read file
        if(res) 
        {
            printf("fserr: read failed %d in %s", res, __func__);
            return res;
        }

        return res;
    }
    else
    {
        if(in_position > max)                             // check if seek position is valid
        {
            printf("fserr: cannot place cursor beyond max file size, use ftell first");
            return FR_INVALID_PARAMETER;
        }

        if(in_count > (max - in_position))                // check if in_count exceeds (size - cursor)
        {
            to_read = max - in_position;                  // clamp to difference in bytes
        }
        else
        {
            to_read = in_count;                           // otherwise set to the count
        }

        res = f_lseek((FIL*)fp, in_position);             // seek to cursor
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

int matter_fs_fwrite(void* fp, const void* in_buf, uint32_t in_buflen, uint32_t in_bufstart, uint32_t in_writestart, bool bAppend, uint* out_num)
{
    if(fp == NULL) {
        return FR_INVALID_PARAMETER;                    // check if fp is valid
    }
    
    if(in_buf == NULL) {
        return FR_INVALID_PARAMETER;                    // input buffer cannot be null
    }
    
    if(in_buflen == 0) {
        return FR_INVALID_PARAMETER;                    // input buffer length cannot be null
    }
    
    if(in_bufstart > in_buflen) { 
        return FR_INVALID_PARAMETER;                    // bufstart cannot be more than buflen
    }

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

int matter_fs_fseek(void* fp, uint32_t to)
{
    if(fp == NULL) 
    {
        return FR_INVALID_PARAMETER;                                        // check if fp is valid
    }

    return f_lseek((FIL*)fp, to);   
}

int matter_fs_ftell(void* fp)
{
    if(fp == NULL)
    {
        return -1;                                                          // check if fp is valid
    }
    
    return f_tell((FIL*)fp);
}

int matter_fs_fsize(void* fp)
{
    if(fp == NULL) 
    {
        return -1;                                                          // check if fp is valid
    }
    
    return f_size((FIL*)fp);
}

#endif  // CONFIG_AMEBA_LOGS_USE_FATFS

#ifdef __cplusplus
}
#endif
