#include "matter_flashfs.h"
#include "ff.h"
#include "platform_opts.h"
#include "platform/platform_stdlib.h"
#include "platform_opts_matter.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "chip_porting.h"

#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 1)

#include "matter_flashfs_fat.h"

#elif defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 0)

#include "matter_flashfs_nofat.h"

#endif // CONFIG_AMEBA_LOGS_USE_FATFS

/**
 * initialize filesystem
 * @return result of operation, 0 if not using FATFS
*/
int matter_flashfs_init() 
{
    return impl_flashfs_init();
}

/**
 * deinitialize filesystem
 * @return result of operation, 0 if not using FATFS
*/
int matter_flashfs_close()
{
    return impl_flashfs_close();
}

/**
 * get filesystem init status
 * @return result of operation, 1 if not using FS as flash is always inited
*/
int matter_flashfs_get_inited() 
{
    return impl_flashfs_get_inited();
}

/**
 * implementation of f_open
 * @param filename filename
 * @param fp (type:FIL* or amb_fsctx_t*) handle to file if opened successfully
 * @return FRESULT of operation
*/
int matter_flashfs_file_open(char* filename, void* fp, uint8_t mode) 
{
    return impl_flashfs_file_open(filename, fp, mode);
}

/**
 * implementation of f_close
 * @param fp (type:FIL* or amb_fsctx_t*) handle to file
 * @return FRESULT of operation
*/
int matter_flashfs_file_close(void* fp)
{
    return impl_flashfs_file_close(fp);
}

/**
 * implementation of f_lseek
 * @param fp (type: FIL* or amb_fsctx_t*) handle to file
 * @param to position to seek to
 * @return FRESULT of operation
*/
int matter_flashfs_file_lseek(void* fp, uint32_t to)
{
    return impl_flashfs_file_lseek(fp, to);
}

/**
 * implementation of f_tell
 * @param fp (type: FIL* or amb_fsctx_t*) handle to file
 * @return current cursor in file, or -1 if err
*/
int matter_flashfs_file_tell(void* fp)
{
    return impl_flashfs_file_tell(fp);
}

/**
 * implementation of f_size
 * @param fp (type: FIL* or amb_fsctx_t*) handle to file
 * @return size of file, or -1 if err
*/
int matter_flashfs_file_size(void* fp)
{
    return impl_flashfs_file_size(fp);
}

/**
 * implementation of f_read
 * @param fp (type:FIL* or amb_fsctx_t*) handle of file to read
 * @param in_position starting cursor position to read from
 * @param in_count number of bytes to read
 * @param out_buf (type:char*) buffer to write to
 * @param out_buflen size of buffer
 * @param out_num number of bytes read
 * @return FRESULT of operation
*/
int matter_flashfs_file_read(void* fp, uint32_t in_position, uint32_t in_count, void* out_buf, uint32_t out_buflen, uint* out_num)
{
    return impl_flashfs_file_read(fp, in_position, in_count, out_buf, out_buflen, out_num);
}


/**
 * implementation of f_write
 * @param fp (type:FIL* or amb_fsctx_t*) handle of file to write
 * @param in_buf buffer of data (as utf8 / uint8_t)
 * @param in_buflen size of buffer
 * @param in_bufstart start of buffer to read from
 * @param in_writestart position in file to write to (if bAppend is false)
 * @param bAppend if append is set, write will place bytes at end of file and ignore in_writestart, otherwise write bytes starting at in_writestart
 * @param out_num number of bytes written
 * @return FRESULT of operation
*/
int matter_flashfs_file_write(void* fp, const void* in_buf, uint32_t in_buflen, uint32_t in_bufstart, uint32_t in_writestart, bool bAppend, uint* out_num)
{
    return impl_flashfs_file_write(fp, in_buf, in_buflen, in_bufstart, in_writestart, bAppend, out_num);
}

/**
 * reset the file by setting filepointer back to front. Used to "delete" file contents without closing the handle
 * @param fp (type:FIL* or amb_fsctx_t*) handle of file to write
*/
int matter_flashfs_file_reset(void* fp)
{
    return impl_flashfs_file_reset(fp);
}

/**
 * list files in path recursively
 * @param path start path
*/
int matter_flashfs_scanfiles(char* path)
{
    return impl_flashfs_file_list(path);
}

#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 0)
int matter_flashfs_file_clearlastblock(void* fp)
{
    return impl_flashfs_file_clearlastblock(fp);
}
#endif

#ifdef __cplusplus
}
#endif
