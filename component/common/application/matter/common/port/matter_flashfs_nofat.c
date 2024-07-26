/**
    Implementation of Flash Filesystem with simple driver
*/

#include "ff.h"
#include "platform_opts.h"
#include "platform_opts_matter.h"

#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 0)

#ifdef __cplusplus
extern "C" {
#endif

#include <flash_api.h>
#include <device_lock.h>
#include "matter_flashfs_nofat.h"

int impl_flashfs_init(void)
{
    return 0;
}

int impl_flashfs_close(void)
{
    return 0;
}

int impl_flashfs_get_inited(void)
{
    return 1;
}

int impl_flashfs_file_open(char* filename, void* fp, uint8_t mode)
{
    /**
        The behavior of f_open for non-fatfs mode is as follows:
        It will attempt to read the header at the start of the flash address and get the file size
        If a size value is read, then the context is created with fptr at the end of the file, and fsize as the size of the data

        NOTE: fptr is the raw flash address, addr_start and addr_end defines the flash "file" boundary

        [len(4 byte): AA BB CC DD][data(N byte)...]
    */
    if(fp == NULL)
        return FR_INVALID_PARAMETER;
    amb_fsctx_t* ctx = (amb_fsctx_t*)fp;
    flash_t	device_flash;
    
    // read the size
    uint32_t sz = 0;

    // apply mutex and read to_read number of bytes
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_read_word(&device_flash, ctx->addr_start, &sz);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    if(sz == 0xFFFFFFFF) sz = 0;                                                                                    // this is an empty file
    printf("fs [%s] open: size: %d\n", ctx->label, sz);
    
    if(mode == FA_CREATE_ALWAYS) {                                                                                  // use mode field to determine if we should open this file from scratch
        sz = 0;                                                                                                     // setting size to 0 effectively clears the file
        printf("fs [%s] : size reset!\n", ctx->label);
    }

    ctx->fptr = ctx->addr_start + FLASH_SIZE_HEADER_LEN + sz;                                                       // place cursor at >>: [00 00 00 00]>>[data(N byte)...]
    ctx->fsize = sz;

    return 0;
}

int impl_flashfs_file_close(void* fp)
{
    /**
        The behavior of f_close for non-fatfs mode is as follows:
        Unlike regular posix-style fclose where the file pointer is deinitialized and deleted
        The fclose here simply resets the pointer back to the start of the data area, 
        acting as a "reset" of sorts when user wishes to overwite the file contents from beginning
    */
    if(fp == NULL)
        return FR_INVALID_PARAMETER;
    amb_fsctx_t* ctx = (amb_fsctx_t*)fp;
    ctx->fptr = (ctx->addr_start) + FLASH_SIZE_HEADER_LEN;                                                          // reset the fptr
    //ctx->fsize = 0;                                                                                                 // reset the file size since we are now at beginning

    return 0;
}

int impl_flashfs_file_lseek(void* fp, uint32_t to)
{
    /**
        The behavior of f_lseek for non-fatfs mode is similar to regular lseek, except it (should) take into account the header size
        When the file position is set, the fptr is simply offset from addr_start by (to) number of bytes
    */
    if(fp == NULL)
        return FR_INVALID_PARAMETER;
    amb_fsctx_t* ctx = (amb_fsctx_t *)fp;                                                                           // assume fp is a a fsctx type
    if(to > (ctx->addr_end - ctx->addr_start))                                                                      // tried to seek beyond bounds
    {
        return FR_INVALID_PARAMETER;
    }

    ctx->fptr = (ctx->addr_start) + to;                                                                             // set fptr to new position 

    return FR_OK;    
}

int impl_flashfs_file_tell(void* fp)
{
    /**
        The behavior of f_tell for non-fatfs mode simply returns the offset of the fptr from the start of region
        It is used to tell how far is read/written in the file area
    */
    if(fp == NULL)
        return -1;                                                                                                  // check if fp is valid
    amb_fsctx_t* ctx = (amb_fsctx_t *)fp;                                                                           // assume fp is a a fsctx type
    return ctx->fptr - ctx->addr_start;                                                                             // return cursor (fs should hide the internal address from top level for ease of use, 
                                                                                                                    // aka all offsets should be internally compensated for)
}

int impl_flashfs_file_size(void* fp)
{
    if(fp == NULL)
        return -1;                                                                                                  // check if fp is valid
    amb_fsctx_t* ctx = (amb_fsctx_t*)fp;                                                                            // assume fp is a a fsctx type
    return ctx->fsize;
}

int impl_flashfs_file_read(void* fp, uint32_t in_position, uint32_t in_count, void* out_buf, uint32_t out_buflen, uint* out_num)
{
    /**
        The behavior of f_read for non-fatfs mode is as follows:

        1) get the file position to read from. in_position should be called by passing in an f_tell call
        2) calculate and adjust the number of bytes to read
        3) read from flash area
        4) increment the fptr value by number of bytes read
    */

    int max, to_read;
    *out_num = 0;
    
    if(fp == NULL)          return FR_INVALID_PARAMETER;                                                            // check if fp is valid
    if(in_count == 0)       return FR_INVALID_PARAMETER;                                                            // cannot read zero bytes
    if(out_buf == NULL)     return FR_INVALID_PARAMETER;                                                            // output buffer cannot be null

    *out_num = 0;
    amb_fsctx_t* ctx = (amb_fsctx_t*)fp;
    flash_t	device_flash;

    max = impl_flashfs_file_size(ctx);                                                                              // get file size
    if(max == 0)                                                                                                    // if the file is empty, return an OK value and set out_count to zero
    {
        *out_num = max;
        return FR_OK;
    }

    int file_position = in_position - FLASH_SIZE_HEADER_LEN; 

    if(file_position < 0)                                                                                           // bounds check
    {
        printf("file position read bounds check failed!\n");
        return FR_INVALID_PARAMETER;
    } 
    else if(file_position > max)
    {
        printf("cannot read more than max bytes\n");
        return FR_INVALID_PARAMETER;
    }

    if((max - file_position) >= in_count)
    {
        to_read = in_count;
    }
    else
    {
        to_read = (max - file_position);                                                                            // there is only this many bytes left
    }

    // apply mutex and read to_read number of bytes
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_stream_read(&device_flash, ctx->fptr, to_read, out_buf);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    ctx->fptr += to_read;                                                                                           // set new fptr
    *out_num = to_read;                                                                                             // set new outsize

    return FR_OK;
}

int impl_flashfs_file_write(void* fp, const void* in_buf, uint32_t in_buflen, uint32_t in_bufstart, uint32_t in_writestart, bool bAppend, uint* out_num)
{
    if(fp == NULL)          return FR_INVALID_PARAMETER;                                                            // check if fp is valid
    if(in_buf == NULL)      return FR_INVALID_PARAMETER;                                                            // input buffer cannot be null
    if(in_buflen == 0)      return FR_INVALID_PARAMETER;                                                            // input buffer length cannot be null
    if(in_bufstart > in_buflen) return FR_INVALID_PARAMETER;                                                        // bufstart cannot be more than buflen

    /**
        The behavior of f_write for non-fatfs mode is as follows:

        1) calculate the max space of the memory region for this "file"
        2) seek to the write position
            - if append mode is false, fptr will be placed at the start of the file (aka position 0)
            - if append mode is true, the existing position of fptr is used
            - if append mode is true and fptr is at the start of the region, put fptr at position 0 after the header (this usually happen after a chip erase)
        3) adjust how many bytes should be written
            - number of bytes should not exceed the remainder space left in the memory region
        4) calculate the new size of the "file" after the write operation
        5) copy the first sector to a scratch buffer and erase it. within the copied sector, overwrite the first 4 bytes (header) with the new file size
        6) write the modified sector back
            - if the size required is more than 1 sector (4KB), then write the first sector, then continue writing the rest of data
            - if the size required is less than 1 sector (4KB), then copy existing data up to the old file size, then continue writing the rest of the data after 
        7) finally, set the new file size and new fptr
    */

    uint32_t to_write = 0;
    amb_fsctx_t* ctx = (amb_fsctx_t*)fp;
    uint32_t max_space = (ctx->addr_end - ctx->addr_start - FLASH_SIZE_HEADER_LEN);                                 // start address of flash memory region to write to [4][...]
    flash_t device_flash;

    /* seek */
    if(bAppend == FALSE)                                                                                            // add at cursor specified by in_writestart
    {
        if(in_writestart >= max_space)                                                                              // write position must be less than to max file size possible if not appending
        {
            return FR_INVALID_PARAMETER;
        }
        else 
        {
            impl_flashfs_file_lseek(ctx, in_writestart + FLASH_SIZE_HEADER_LEN);                                    // seek to new write position
        }
    }
    else 
    {
        if((ctx->fptr - ctx->addr_start) == 0)                                                                      // if this is fresh file (nothing written) then seek after header. if its already written, fptr should be elsewhere in the file
        {
            impl_flashfs_file_lseek(ctx, FLASH_SIZE_HEADER_LEN);                                                    // seek to end of header
        }
    }

    /* bounds check for input */
    if(in_buflen > ((max_space - ctx->fptr)))                                                                       // in_buflen must be less than to max file size possible if not appending
    {
        to_write = (max_space - ctx->fptr);                                                                         // truncate to max space available (after fptr) so we dont spill over into invalid memory
    }
    else 
    {
        to_write = in_buflen;
    }

    /* write */
    uint32_t new_size = to_write;
    if(bAppend == TRUE)
    {
        new_size = (ctx->fsize + to_write);                                                                         // in append mode, need to add existing data size + new (to_write)
    }

    //uint8_t sector_buf[4096];
    uint8_t* sector_buf = (uint8_t*)malloc(SECTOR_SIZE_FLASH);
    if(sector_buf == NULL)
        return FR_DISK_ERR;

    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_stream_read(&device_flash, ctx->addr_start, SECTOR_SIZE_FLASH, sector_buf);                               // copy the entire sector with the header
    flash_erase_sector(&device_flash, ctx->addr_start);                                                             // erase the sector and prepare it for writing again
    memcpy(sector_buf, &new_size, sizeof(uint32_t));                                                                // place the new size value at index 0 and overwrite the old header, while keeping the data

    if(new_size > SECTOR_SIZE_FLASH)                                                                                // if new size is more than 1 sector, just write the sector back in
        flash_burst_write(&device_flash, ctx->addr_start, SECTOR_SIZE_FLASH, sector_buf);                           // write sector with header modified
    else                                                                                                            // otherwise if new size is below one sector, we need to retain the data
        flash_burst_write(&device_flash, ctx->addr_start, ctx->fsize + FLASH_SIZE_HEADER_LEN, sector_buf);          // write new header + existing data (up till old size with fsize)

    flash_burst_write(&device_flash, ctx->fptr, to_write, in_buf);                                                  // write new data at fptr
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    ctx->fsize += to_write;                                                                                         // finally, increase the filesize after append
    ctx->fptr += to_write;                                                                                          // set the new fptr at end of file
    *out_num = to_write;                                                                                            // output the number of bytes written

    free(sector_buf);

    return FR_OK;
}

int impl_flashfs_file_reset(void* fp)
{
    if(fp == NULL)          return FR_INVALID_PARAMETER;

    amb_fsctx_t* ctx = (amb_fsctx_t*)fp;
    ctx->fptr = ctx->addr_start + FLASH_SIZE_HEADER_LEN;
    ctx->fsize = 0;

    // must also erase first sector, if not next reboot will be tainted as file structures are reinitialized!
    flash_t device_flash;
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_erase_sector(&device_flash, ctx->addr_start);
    
    // check that first word of sector is 0xFFFFFFFF
    uint32_t sz = 0;
    flash_read_word(&device_flash, ctx->addr_start, &sz);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    printf("########## reset word: 0x%08X ############\n", sz);

    return FR_OK;
}

int impl_flashfs_file_list(char* path)
{
    // no-op
    return 0;
}

static void hexdump(const uint8_t *in, size_t len) {
    printf("##DEBUG: ");
  for (size_t i = 0; i < len; i++) {
    printf("%02x ", in[i]);
  }
  printf("\n");
}

int impl_flashfs_file_clearlastblock(void* fp)
{
    /*
        Implements a strategy that shifts all blocks "up" by one and freeing the last block for use. Only in situations where the file is very full
        e,g for logging, rotating out old log for new one
    */
    if(fp == NULL)          return FR_INVALID_PARAMETER;

    amb_fsctx_t* ctx = (amb_fsctx_t*)fp;
    flash_t device_flash;

    // step 0: count the number of sectors this file has
    uint32_t num_sectors = (ctx->addr_end - ctx->addr_start) / SECTOR_SIZE_FLASH;

    printf("@@ num sectors: %d\n", num_sectors);

    // step 1: allocate scratch memory
    uint8_t* scratch_buf = (uint8_t*)malloc(SECTOR_SIZE_FLASH);
    if(scratch_buf == NULL)
    {
        return FR_DISK_ERR;
    }

    // step 2: copy only N-1 sectors, as last sector should be cleared for writing while the rest are copied
    for(int i = 0; i < num_sectors - 1; i++)
    {
        device_mutex_lock(RT_DEV_LOCK_FLASH);
        memset(scratch_buf, 0xDD, SECTOR_SIZE_FLASH);

        if(i == 0) {
            // copy header of sector 0 into scratch
            flash_read_word(&device_flash, ctx->addr_start, (uint32_t*)scratch_buf);
            hexdump(scratch_buf, 8);

            // copy 1st 4092 bytes of sector 1 into scratch+4
            flash_stream_read(&device_flash, ctx->addr_start + SECTOR_SIZE_FLASH + FLASH_SIZE_HEADER_LEN, SECTOR_SIZE_FLASH - FLASH_SIZE_HEADER_LEN, scratch_buf + sizeof(uint32_t));

            hexdump(scratch_buf, 32);
            hexdump(scratch_buf + SECTOR_SIZE_FLASH - 8, 8);
        } else {
            
            flash_stream_read(&device_flash, ctx->addr_start + ((i+1) * SECTOR_SIZE_FLASH), SECTOR_SIZE_FLASH, scratch_buf);

            printf("======== %d - %d ========\n", i, i+1);
            hexdump(scratch_buf, 32);
            hexdump(scratch_buf + SECTOR_SIZE_FLASH - 8, 8);
        }
        
        // there is now 4096 bytes copied. Erase sector i and write scratch
        flash_erase_sector(&device_flash, ctx->addr_start + (i * SECTOR_SIZE_FLASH));
        flash_burst_write(&device_flash, ctx->addr_start + (i * SECTOR_SIZE_FLASH), SECTOR_SIZE_FLASH, scratch_buf);

        device_mutex_unlock(RT_DEV_LOCK_FLASH);
    }

    // step 2c: copy final 4 bytes of sector N 
    memset(scratch_buf, 0, SECTOR_SIZE_FLASH);
    flash_read_word(&device_flash, ctx->addr_end - SECTOR_SIZE_FLASH - 4, scratch_buf);

    // step 3: erase the last sector 
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_erase_sector(&device_flash, ctx->addr_end - SECTOR_SIZE_FLASH);

    // step 4a: write 4 bytes back in last sector as leftover
    flash_burst_write(&device_flash, ctx->addr_end - SECTOR_SIZE_FLASH, sizeof(uint32_t), scratch_buf);

    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    // step 4: reset file pointer by 1 * SECTOR_SIZE_FLASH and file size by SECTOR_SIZE_FLASH
    ctx->fsize -= SECTOR_SIZE_FLASH;
    ctx->fptr -= SECTOR_SIZE_FLASH;

    // step 5: deinitialize
    if(scratch_buf != NULL)
    {
        free(scratch_buf);
        scratch_buf = NULL;
    }

    return FR_OK;
}

#ifdef __cplusplus
}
#endif

#endif // CONFIG_AMEBA_LOGS_USE_FATFS