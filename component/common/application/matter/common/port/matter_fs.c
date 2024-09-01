#include <platform_opts.h>
#include <matter_fs.h>
#include <matter/api/matter_log_api.h>
#include <lfs.h>
#include <littlefs_adapter.h>

#if defined(CONFIG_ENABLE_AMEBA_LFS) && (CONFIG_ENABLE_AMEBA_LFS == 1)

#ifdef __cplusplus
extern "C" {
#endif

lfs_t lfs;
static int bInited = 0;

int matter_fs_init(void)
{
    char msg[32] = { 0 };

    // Attempt to mount the fs
    int err = lfs_mount(&lfs, &g_lfs_cfg);
    sprintf(msg, "lfs_mount1: %d\n", err);
    CHIP_LOG_DETAIL(msg);

    // If mount failes, format and mount the fs again
    if (err)
    {
        err = lfs_format(&lfs, &g_lfs_cfg);
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "lfs_format: %d\n", err);
        CHIP_LOG_DETAIL(msg);

        err = lfs_mount(&lfs, &g_lfs_cfg);
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "lfs_mount2: %d\n", err);
        if (err)
        {
            CHIP_LOG_ERROR(msg);  // Only log as error if it fails again
        }
        else
        {
            CHIP_LOG_DETAIL(msg);  // Log success at detail level
        }
    }

    if (err == 0) 
    {
        bInited = 1;
    }

    return err;
}


int matter_fs_close(void)
{
    bInited = 0;
    return lfs_unmount(&lfs);
}

int matter_fs_get_init()
{
    return bInited;
}

int matter_fs_fopen(char *filename, void *fp, int mode)
{
    if (fp == NULL)
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*) fp;

    int err = lfs_file_open(&lfs, lfs_fp, filename, mode);
    if (err < 0)
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
    if (fp == NULL) 
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    return lfs_file_close(&lfs, lfs_fp);
}

int matter_fs_fread(void *fp, uint32_t in_position, uint32_t in_count, void *out_buf, uint32_t out_buflen, uint *out_num)
{
    if (fp == NULL || out_buf == NULL || out_buflen == 0) 
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    // check file size to be read
    int max = lfs_file_size(&lfs, lfs_fp);
    int to_read;
    int res = 0;

    // if file is empty; set out_num = 0 indicating nothing is read
    if (max == 0)
    {
        *out_num = 0;
        return LFS_ERR_OK;
    }

    // beginning of the file
    if (in_position == 0)
    {
        if (in_count > max)
        {
            to_read = max;
        }
        else
        {
            to_read = in_count;
        }

        // cannot read more than the provided buffer
        if (to_read > out_buflen)
        {
            to_read = out_buflen;
        }
    }
    else
    {
        // reading from anywhere in the file
        if (in_position >= max)
        {
            // cannot seek more than max
            return LFS_ERR_INVAL;
        }

        if (in_count > (max - in_position))
        {
            // clamp to difference in bytes
            to_read = max - in_position;
        }
        else 
        {
            to_read = in_count;
        }
    }

    // seek
    res = lfs_file_seek(&lfs, lfs_fp, in_position, LFS_SEEK_SET);
    if (res < 0)
    {
        printf("fserr: seek failed %d in %s", res, __func__);
        return res;
    }

    // read
    res = lfs_file_read(&lfs, lfs_fp, out_buf, to_read);
    if (res < 0)
    {
        printf("fserr: read failed %d in %s", res, __func__);
        return res;
    }
    *out_num = res;

    return LFS_ERR_OK;
}

int matter_fs_fwrite(void *fp, const void *in_buf, uint32_t in_buflen, uint *out_num)
{
    if (fp == NULL) 
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    int32_t status = lfs_file_write(&lfs, lfs_fp, in_buf, in_buflen);

    if (status >= 0)
    {
        *out_num = status;
        // Sync to ensure all changes are saved to fs
        lfs_file_sync(&lfs, lfs_fp);
        return LFS_ERR_OK;
    }
    else
    {
        return status;
    }
}

int matter_fs_fseek(void *fp, uint32_t to, uint32_t whence)
{
    if (fp == NULL)
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    // whence: 0 = `SEEK_SET`, 1 = `SEEK_CUR`, 2 = `SEEK_END`.
    return lfs_file_seek(&lfs, lfs_fp, to, whence);
}


int matter_fs_ftell(void* fp)
{
    if (fp == NULL)
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    return lfs_file_tell(&lfs, lfs_fp);
}


int matter_fs_fsize(void *fp)
{
    if (fp == NULL)
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    return lfs_file_size(&lfs, lfs_fp);
}

int matter_fs_fclear(void* fp)
{
    if (fp == NULL)
    {
        return LFS_ERR_INVAL;
    }

    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    // truncate to 0 effectively clears the file
    return lfs_file_truncate(&lfs, lfs_fp, 0);
}

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_ENABLE_AMEBA_LFS */
