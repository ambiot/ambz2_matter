/************************** 
* Matter Timers Related 
**************************/
#include "platform_opts.h"
#include "platform/platform_stdlib.h"

#ifdef __cplusplus
 extern "C" {
#endif

#include "stddef.h"
#include "string.h"
#include "errno.h"
#include "FreeRTOS.h"
#include "chip_porting.h"
#include "time.h"
#include "rtc_api.h"
#include "timer_api.h"
#include "task.h"

#define MICROSECONDS_PER_SECOND    ( 1000000LL )                                   /**< Microseconds per second. */
#define NANOSECONDS_PER_SECOND     ( 1000000000LL )                                /**< Nanoseconds per second. */
#define NANOSECONDS_PER_TICK       ( NANOSECONDS_PER_SECOND / configTICK_RATE_HZ ) /**< Nanoseconds per FreeRTOS tick. */

#define US_OVERFLOW_MAX            (0xFFFFFFFF)
#define MATTER_SW_RTC_TIMER_ID     TIMER5

extern int FreeRTOS_errno;
#define errno FreeRTOS_errno

static gtimer_t matter_rtc_timer;
static volatile uint32_t rtc_counter = 0;
static uint64_t last_current_us = 0;
static uint64_t last_global_us = 0;

BOOL UTILS_ValidateTimespec( const struct timespec * const pxTimespec )
{
    BOOL xReturn = FALSE;

    if( pxTimespec != NULL )
    {
        /* Verify 0 <= tv_nsec < 1000000000. */
        if( ( pxTimespec->tv_nsec >= 0 ) &&
            ( pxTimespec->tv_nsec < NANOSECONDS_PER_SECOND ) )
        {
            xReturn = TRUE;
        }
    }

    return xReturn;
}

int _nanosleep( const struct timespec * rqtp,
               struct timespec * rmtp )
{
    int iStatus = 0;
    TickType_t xSleepTime = 0;

    /* Silence warnings about unused parameters. */
    ( void ) rmtp;

    /* Check rqtp. */
    if( UTILS_ValidateTimespec( rqtp ) == FALSE )
    {
        errno = EINVAL;
        iStatus = -1;
    }

    if( iStatus == 0 )
    {
        /* Convert rqtp to ticks and delay. */
        if( UTILS_TimespecToTicks( rqtp, &xSleepTime ) == 0 )
        {
            vTaskDelay( xSleepTime );
        }
    }

    return iStatus;
}

int __clock_gettime(struct timespec * tp)
{
    unsigned int update_tick = 0;
    long update_sec = 0, update_usec = 0, current_sec = 0, current_usec = 0;
    unsigned int current_tick = xTaskGetTickCount();

    sntp_get_lasttime(&update_sec, &update_usec, &update_tick);
    //if(update_tick) {
        long tick_diff_sec, tick_diff_ms;

        tick_diff_sec = (current_tick - update_tick) / configTICK_RATE_HZ;
        tick_diff_ms = (current_tick - update_tick) % configTICK_RATE_HZ / portTICK_RATE_MS;
        update_sec += tick_diff_sec;
        update_usec += (tick_diff_ms * 1000);
        current_sec = update_sec + update_usec / 1000000;
        current_usec = update_usec % 1000000;
    //}
    //else {
        //current_sec = current_tick / configTICK_RATE_HZ;
    //}
    tp->tv_sec = current_sec;
    tp->tv_nsec = current_usec*1000;
    //sntp_set_lasttime(update_sec,update_usec,update_tick);
    //printf("update_sec %d update_usec %d update_tick %d tvsec %d\r\n",update_sec,update_usec,update_tick,tp->tv_sec);
}

time_t _time( time_t * tloc )
{
#if 0
    /* Read the current FreeRTOS tick count and convert it to seconds. */
    time_t xCurrentTime = ( time_t ) ( xTaskGetTickCount() / configTICK_RATE_HZ );
#else
    time_t xCurrentTime;
    struct timespec tp;

    __clock_gettime(&tp);
    xCurrentTime = tp.tv_sec;
#endif
    /* Set the output parameter if provided. */
    if( tloc != NULL )
    {
        *tloc = xCurrentTime;
    }

    return xCurrentTime;
}

extern void vTaskDelay( const TickType_t xTicksToDelay );
int _vTaskDelay( const TickType_t xTicksToDelay )
{
    vTaskDelay(xTicksToDelay);

    return 0;
}

void matter_rtc_init()
{
    rtc_init();
}

long long matter_rtc_read()
{
    return rtc_read();
}

void matter_rtc_write(long long time)
{
    rtc_write(time);
}

uint64_t ameba_get_clock_time(void)
{
    uint64_t current_us = 0;
    uint64_t global_us = 0;

    // Read current timer value in microseconds
    current_us = gtimer_read_us(&matter_rtc_timer);

    // Check if the timer has wrapped around
    if (current_us < last_current_us)
    {
        // Timer wrapped around, increment global_us and adjust last_global_us
        global_us = last_global_us + 1;
    }
    else
    {
        // Calculate global_us based on the rtc_counter and current_us
        global_us = ((uint64_t)rtc_counter * US_OVERFLOW_MAX) + current_us;
        last_current_us = current_us;
    }

    last_global_us = global_us;

    return global_us;
}

static void matter_timer_rtc_callback(void)
{
    rtc_counter++;
    last_current_us = 0;
}

void matter_timer_init(void)
{
    gtimer_init(&matter_rtc_timer, MATTER_SW_RTC_TIMER_ID);
    hal_timer_set_cntmode(&matter_rtc_timer.timer_adp, 0); //use count up
    gtimer_start_periodical(&matter_rtc_timer, US_OVERFLOW_MAX, (void *)matter_timer_rtc_callback, (uint32_t) &matter_rtc_timer);
}

#ifdef __cplusplus
}
#endif
