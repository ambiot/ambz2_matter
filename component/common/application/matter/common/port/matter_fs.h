#include "platform_opts.h"
#include <platform/platform_stdlib.h>
#include "littlefs/r2.50/lfs.h"

#ifdef __cplusplus
extern "C" {
#endif

int matter_fs_init();
int matter_fs_close();

int matter_fs_get_init();
int matter_fs_fopen(char* filename, void* fp, int mode);
int matter_fs_fclose(void* fp);
int matter_fs_fseek(void* fp, uint32_t to, uint32_t whence);
int matter_fs_ftell(void* fp);
int matter_fs_fsize(void* fp);
int matter_fs_fread(void* fp, uint32_t in_position, uint32_t in_count, void* out_buf, uint32_t out_buflen, uint* out_num);
int matter_fs_fwrite(void* fp, const void* in_buf, uint32_t in_buflen, uint32_t in_bufstart, uint32_t in_writestart, bool bAppend, uint* out_num);
int matter_fs_fclear(void* fp);

#ifdef __cplusplus
}
#endif