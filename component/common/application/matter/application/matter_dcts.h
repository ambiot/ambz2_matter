/**
 * @brief High resolution sleep.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/nanosleep.html
 *
 * @note rmtp is ignored, as signals are not implemented.
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <wifi_conf.h>

#ifdef CHIP_PROJECT
// for AmebaConfig
s32 initPref(void);
s32 deinitPref(void);
s32 registerPref(const char * ns);
s32 registerPref2(const char * ns);
s32 clearPref(const char * ns);
s32 deleteKey(const char *domain, const char *key);
BOOL checkExist(const char *domain, const char *key);
s32 setPref_new(const char *domain, const char *key, u8 *value, size_t byteCount);
s32 getPref_bool_new(const char *domain, const char *key, u8 *val);
s32 getPref_u32_new(const char *domain, const char *key, u32 *val);
s32 getPref_u64_new(const char *domain, const char *key, u64 *val);
s32 getPref_str_new(const char *domain, const char *key, char * buf, size_t bufSize, size_t *outLen);
s32 getPref_bin_new(const char *domain, const char *key, u8 * buf, size_t bufSize, size_t *outLen);
#endif /* CHIP_PROJECT */

#ifdef __cplusplus
}
#endif
