#include <platform_opts.h>
#include <platform_stdlib.h>
#include <lfs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the LFS for the Matter application.
 * @return 0 for Success; otherwise non-zero for failure
 */
int matter_fs_init(void);

/**
 * @brief Closes the LittleFS file system and resets the initialization flag.
 * @return 0 for Success; otherwise non-zero for failure
 */
int matter_fs_close(void);

/**
 * @brief Returns the current initialization status of the LFS.
 * @return 1 if initialized, 0 if not initialized
 */
int matter_fs_get_init();

/**
 * @brief Opens a file in the LFS.
 *
 * @param filename Name of the file to open.
 * @param fp Pointer to a file handle where the file descriptor will be stored.
 * @param mode File access mode (e.g., read, write).
 * @return 0 for success; otherwise, non-zero for failure.
 */
int matter_fs_fopen(char* filename, void* fp, int mode);

/**
 * @brief Closes an open file in the LittleFS file system.
 * 
 * @param fp Pointer to the file handle to be closed.
 * @return 0 for success; otherwise, non-zero for failure.
 */
int matter_fs_fclose(void* fp);

/**
 * @brief Reads data from a file in the LFS.
 * 
 * @param fp Pointer to the file handle.
 * @param in_position Position in the file to start reading from.
 * @param in_count Number of bytes to read.
 * @param out_buf Buffer to store the read data.
 * @param out_buflen Length of the output buffer.
 * @param out_num Pointer to store the number of bytes read.
 * @return 0 for success; otherwise, non-zero for failure.
 */
int matter_fs_fread(void* fp, uint32_t in_position, uint32_t in_count, void* out_buf, uint32_t out_buflen, uint* out_num);

/**
 * @brief Writes data to a file in the LFS.
 * 
 * @param fp Pointer to the file handle.
 * @param in_buf Pointer to the buffer containing the data to write.
 * @param in_buflen Length of the data in `in_buf` to write.
 * @param out_num Pointer to store the number of bytes written.
 * @return 0 for success; otherwise, non-zero for failure.
 */
int matter_fs_fwrite(void* fp, const void* in_buf, uint32_t in_buflen, uint* out_num);

/**
 * @brief Seeks to a specific position in a file within the LFS.
 * 
 * @param fp Pointer to the file handle.
 * @param to Position to seek to.
 * @param whence 0 = `SEEK_SET`, 1 = `SEEK_CUR`, 2 = `SEEK_END`.
 * @return 0 for success; otherwise, non-zero for failure.
 */
int matter_fs_fseek(void* fp, uint32_t to, uint32_t whence);

/**
 * @brief Gets the current position of the file pointer.
 * 
 * @param fp Pointer to the file handle.
 * @return The current position of the file pointer; otherwise, non-zero for failure.
 */
int matter_fs_ftell(void* fp);

/**
 * @brief Gets the size of the file.
 * 
 * @param fp Pointer to the file handle.
 * @return The size of the file in bytes; otherwise, non-zero for failure.
 */
int matter_fs_fsize(void* fp);

/**
 * @brief Clears the file by truncating it to zero length.
 * 
 * @param fp Pointer to the file handle.
 * @return int 0 for success; otherwise, non-zero for failure.
 */
int matter_fs_fclear(void* fp);

#ifdef __cplusplus
}
#endif
