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
#include <time.h>

typedef u32 TickType_t;
int _nanosleep( const struct timespec * rqtp, struct timespec * rmtp );
int _vTaskDelay( const TickType_t xTicksToDelay );
time_t _time( time_t * tloc );
void matter_rtc_init(void);
long long matter_rtc_read(void);
void matter_rtc_write(long long time);

#ifdef __cplusplus
}
#endif
