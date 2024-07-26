#ifndef MATTER_FLASHFS_FAT_H
#define MATTER_FLASHFS_FAT_H

#include "platform_opts.h"
#include <platform/platform_stdlib.h>


#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 1)

#ifdef __cplusplus
extern "C" {
#endif

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


#ifdef __cplusplus
}
#endif

#endif

#endif