#ifndef MATTER_FLASHFS_NOFAT_H
#define MATTER_FLASHFS_NOFAT_H

#include "platform_opts.h"
#include <platform/platform_stdlib.h>

#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 0)

#ifdef __cplusplus
extern "C" {
#endif

#define FLASH_SIZE_HEADER_LEN 4

typedef struct amb_fsctx {
    char label[16];                // "label"
    const uint32_t addr_start;     // start address constant
    const uint32_t addr_end;       // end address constant
    uint32_t fptr;                 // cursor
    uint32_t fsize;                // size of "file"
} amb_fsctx_t;

int impl_flashfs_init(void);
int impl_flashfs_close(void);
int impl_flashfs_get_inited(void);
int impl_flashfs_file_open(char* filename, void* fp, uint8_t mode);
int impl_flashfs_file_close(void* fp);
int impl_flashfs_file_lseek(void* fp, uint32_t to);
int impl_flashfs_file_tell(void* fp);
int impl_flashfs_file_size(void* fp);
int impl_flashfs_file_read(void* fp, uint32_t in_position, uint32_t in_count, void* out_buf, uint32_t out_buflen, uint* out_num);
int impl_flashfs_file_write(void* fp, const void* in_buf, uint32_t in_buflen, uint32_t in_bufstart, uint32_t in_writestart, bool bAppend, uint* out_num);
int impl_flashfs_file_list(char* path);
int impl_flashfs_file_reset(void* fp);
int impl_flashfs_file_clearlastblock(void* fp);

#ifdef __cplusplus
}
#endif

#endif

#endif