#ifndef MATTER_FLASHFS_H
#define MATTER_FLASHFS_H

#include "platform_opts.h"
#include <platform/platform_stdlib.h>

#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 1)
/*
#define FLASH_SIZE_HEADER_LEN 4

typedef struct amb_fsctx {
    const uint32_t addr_start;     // start address constant
    const uint32_t addr_end;       // end address constant
    uint32_t fptr;                 // cursor
    uint32_t fsize;                 // size of "file"
} amb_fsctx_t;
*/
#include "matter_flashfs_fat.h"
#else
#include "matter_flashfs_nofat.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int matter_flashfs_init();
int matter_flashfs_close();

int matter_flashfs_get_inited();
int matter_flashfs_file_open(char* filename, void* fp, uint8_t mode);
int matter_flashfs_file_close(void* fp);
int matter_flashfs_file_lseek(void* fp, uint32_t to);
int matter_flashfs_file_tell(void* fp);
int matter_flashfs_file_size(void* fp);
int matter_flashfs_file_read(void* fp, uint32_t in_position, uint32_t in_count, void* out_buf, uint32_t out_buflen, uint* out_num);
int matter_flashfs_file_write(void* fp, const void* in_buf, uint32_t in_buflen, uint32_t in_bufstart, uint32_t in_writestart, bool bAppend, uint* out_num);
int matter_flashfs_file_reset(void* fp);
int matter_flashfs_scanfiles(char* path);

#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 0)
int matter_flashfs_file_clearlastblock(void* fp);
#endif // CONFIG_AMEBA_LOGS_USE_FATFS

#ifdef __cplusplus
}
#endif

#endif