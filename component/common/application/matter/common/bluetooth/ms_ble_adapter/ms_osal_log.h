/*
 * Copyright (c) 2019 - 2020 IoT Company of Midea Group.
 *
 * File Name 	    : 
 * Description	    : rtos adaptor
 *
 * Version	        : v0.1
 * Author			:
 * Date	            : 2019/09/15  refactoring
 * History	        : 2020/06/15  modify by pargo ,add log level
 */
#ifndef __MS_OSAL_LOG_H__
#define __MS_OSAL_LOG_H__

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#ifdef MS_USE_ULOG
#include "ulog/ulog.h"
#endif
#include "ms_osal.h"
#include "ms_osal_thread.h"
#include "ms_osal_mem.h"

/**
 * @brief log print mutex initialize
 * 
 * @param route_info 
 * @return MS_OSAL_RESULT_SUCCESS/MS_OSAL_RESULT_ERROR
 */
ms_osal_result_t ms_osal_print_mutex_init(void);
/**
 * @brief log print mutex get
 * 
 * @param none 
 * @return MS_OSAL_RESULT_SUCCESS/MS_OSAL_RESULT_ERROR 
 */
ms_osal_result_t ms_osal_print_mutex_lock(void);
/**
 * @brief log print mutex put
 * 
 * @param none 
 * @return MS_OSAL_RESULT_SUCCESS/MS_OSAL_RESULT_ERROR 
 */
ms_osal_result_t ms_osal_print_mutex_unlock(void);

/**
 * @brief print system perror
 *
 * @param none
 * @return Returns a pointer to the error string, the error string describes the error errnum
 */
const char *ms_osal_log_error(void);

//#define MS_LOG_PRINT_MUTEX_ENABLE

//#define MS_USE_ULOG
/**
 * @brief log level
 * 
 */
#define MS_LOG_LEVEL_DEBUG (3)
#define MS_LOG_LEVEL_INFO (2)
#define MS_LOG_LEVEL_WARNING (1)
#define MS_LOG_LEVEL_ERROR (0)

#ifndef MS_RELEASE_DOMAIN
#define MS_LOG_LEVEL MS_LOG_LEVEL_DEBUG
#else
//Here
#define MS_LOG_LEVEL MS_LOG_LEVEL_INFO
#endif
/**
 * @brief log format
 * 
 */
#ifdef MS_USE_ULOG
#define ms_osal_logd LOGD
#define ms_osal_logi LOGI
#define ms_osal_logw LOGW
#define ms_osal_loge LOGE
#else
#if 0
#define ms_osal_logd(format, arg...) printf("\033[0m<D>" format "\033[0m\n", ##arg)
#define ms_osal_logi(format, arg...) printf("\033[0;36m<I>" format "\033[0m\n", ##arg)
#define ms_osal_logw(format, arg...) printf("\033[1;33m<W>" format "\033[0m\n", ##arg)
#define ms_osal_loge(format, arg...) printf("\033[0;31m<E>" format "\033[0m\n", ##arg)
#else
#ifdef MS_LOG_PRINT_MUTEX_ENABLE
#define PRINT_BUF_SIZE 512
#define ms_osal_logd(format, arg...)    do{ \
                                            char *printbuf = ms_osal_memory_alloc(PRINT_BUF_SIZE);\
                                            snprintf(printbuf, PRINT_BUF_SIZE, "<D>" format "\n", ms_osal_thread_get_name(),##arg);\
                                            ms_osal_print_mutex_lock(); \
                                            printf("%s", printbuf); \
                                            ms_osal_print_mutex_unlock(); \
                                            ms_osal_memory_free(printbuf);\
                                        }while(0);

#define ms_osal_logi(format, arg...)    do{ \
                                            char *printbuf = ms_osal_memory_alloc(PRINT_BUF_SIZE);\
                                            snprintf(printbuf, PRINT_BUF_SIZE, "<I>" format "\n", ms_osal_thread_get_name(),##arg);\
                                            ms_osal_print_mutex_lock(); \
                                            printf("%s", printbuf); \
                                            ms_osal_print_mutex_unlock(); \
                                            ms_osal_memory_free(printbuf);\
                                        }while(0);
    

#define ms_osal_logw(format, arg...)    do{ \
                                            char *printbuf = ms_osal_memory_alloc(PRINT_BUF_SIZE);\
                                            snprintf(printbuf, PRINT_BUF_SIZE, "<W>" format "\n", ms_osal_thread_get_name(),##arg);\
                                            ms_osal_print_mutex_lock(); \
                                            printf("%s", printbuf); \
                                            ms_osal_print_mutex_unlock(); \
                                            ms_osal_memory_free(printbuf);\
                                        }while(0);

#define ms_osal_loge(format, arg...)    do{ \
                                            char *printbuf = ms_osal_memory_alloc(PRINT_BUF_SIZE);\
                                            snprintf(printbuf, PRINT_BUF_SIZE, "<E>" format "\n", ms_osal_thread_get_name(),##arg);\
                                            ms_osal_print_mutex_lock(); \
                                            printf("%s", printbuf); \
                                            ms_osal_print_mutex_unlock(); \
                                            ms_osal_memory_free(printbuf);\
                                        }while(0);
#else
#define ms_osal_logd(format, arg...)    do{ \
                                                printf("<D>" format "\r\n", ms_osal_thread_get_name(),##arg);\
                                            }while(0);
    
#define ms_osal_logi(format, arg...)    do{ \
                                                printf("<I>" format "\r\n", ms_osal_thread_get_name(),##arg);\
                                            }while(0);
        
    
#define ms_osal_logw(format, arg...)    do{ \
                                                printf("<W>" format "\r\n", ms_osal_thread_get_name(),##arg);\
                                            }while(0);
    
#define ms_osal_loge(format, arg...)    do{ \
                                                printf("<E>" format "\r\n", ms_osal_thread_get_name(),##arg);\
                                            }while(0);
#endif

#endif
#endif

#ifdef MS_USE_ULOG
111
#define MS_LOGD(...) ms_osal_logd("ms:", ##__VA_ARGS__)
#define MS_LOGI(...) ms_osal_logi("ms:", ##__VA_ARGS__)
#define MS_LOGW(...) ms_osal_logw("ms:", ##__VA_ARGS__)
#define MS_LOGE(...) ms_osal_loge("ms:", ##__VA_ARGS__)
#else
#define MS_LOGD(fmt, arg...)                    \
    do                                          \
    {                                           \
        if (MS_LOG_LEVEL >= MS_LOG_LEVEL_DEBUG) \
        {                                       \
            ms_osal_logd("ms [Task_%s]:" fmt, ##arg);     \
        }                                       \
    } while (0)
#define MS_LOGI(fmt, arg...)                   \
    do                                         \
    {                                          \
        if (MS_LOG_LEVEL >= MS_LOG_LEVEL_INFO) \
        {                                      \
            ms_osal_logi("ms [Task_%s]:" fmt, ##arg);    \
        }                                      \
    } while (0)
#define MS_LOGW(fmt, arg...)                      \
    do                                            \
    {                                             \
        if (MS_LOG_LEVEL >= MS_LOG_LEVEL_WARNING) \
        {                                         \
            ms_osal_logw("ms [Task_%s]:" fmt, ##arg);       \
        }                                         \
    } while (0)
#define MS_LOGE(fmt, arg...)                    \
    do                                          \
    {                                           \
        if (MS_LOG_LEVEL >= MS_LOG_LEVEL_ERROR) \
        {                                       \
            ms_osal_loge("ms [Task_%s]:" fmt, ##arg);     \
        }                                       \
    } while (0)
#define MS_TRACE(fmt, arg...)                  \
    do                                         \
    {                                          \
        if (MS_LOG_LEVEL >= MS_LOG_LEVEL_INFO) \
        {                                      \
            ms_osal_logi("ms [Task_%s]:" fmt, ##arg);    \
        }                                      \
    } while (0)
#endif

//#define MS_TRACE MS_LOGI
/**
 * @brief no use
 * 
 */
#ifdef MS_LOG_TEST
#define MS_LOG_FUNC_ENTER() ms_osal_logd("ms: enter %s", __FUNCTION__)
#define MS_LOG_FUNC_EXIT() ms_osal_logd("ms: exit %s", __FUNCTION__)
#else
#define MS_LOG_FUNC_ENTER()
#define MS_LOG_FUNC_EXIT()
#endif
/**
 * @brief strings format
 * 
 * @todo optimize
 */
#ifndef MS_USE_ULOG
#ifdef MS_LOG_PRINT_MUTEX_ENABLE
#define MS_LOG_FORMAT(task, name, buf, len, format, level)                                          \
    do                                                                                        \
    {                                                                                         \
        if (level <= MS_LOG_LEVEL)                                                            \
        {                                                                                     \
            char *printbuf = ms_osal_memory_alloc(PRINT_BUF_SIZE);                            \
            snprintf(printbuf, PRINT_BUF_SIZE, "<%s>ms [Task_%s]: %s size=%d\n[", (level == MS_LOG_LEVEL_DEBUG) ? "D" : "I", task,(char *)name, len); \
            for (unsigned int __index = 0; __index < len; __index++)                                            \
            {                                                                                 \
                if (0 == (__index % 16) && 0 != __index)                                                  \
                    snprintf(printbuf+strlen(printbuf), PRINT_BUF_SIZE-strlen(printbuf), "\n"); \
                char *p = (char *)buf;                                                        \
                snprintf(printbuf+strlen(printbuf), PRINT_BUF_SIZE-strlen(printbuf), format, (p[__index] & 0xff));                                                \
            }                                                                                 \
            snprintf(printbuf+strlen(printbuf), PRINT_BUF_SIZE-strlen(printbuf), "]\n");                                                                    \
            ms_osal_print_mutex_lock();                                                       \
            printf("%s", printbuf);                                                           \
            ms_osal_print_mutex_unlock();                                                     \
            ms_osal_memory_free(printbuf);                                                    \
        }                                                                                     \
    } while (0)
#else
#define MS_LOG_FORMAT(task, name, buf, len, format, level)                                          \
    do                                                                                        \
    {                                                                                         \
        if (level <= MS_LOG_LEVEL)                                                            \
        {                                                                                     \
            printf("<%s>ms [Task_%s]: %s size=%d\r\n[", (level == MS_LOG_LEVEL_DEBUG) ? "D" : "I", task,(char *)name, len); \
            for (unsigned int __index_t = 0; __index_t < (unsigned int)len; __index_t++)                                            \
            {                                                                                 \
                if (0 == (__index_t % 16) && 0 != __index_t)                                                  \
                    printf("\r\n");                                                             \
                char *p = (char *)buf;                                                        \
                printf(format, (p[__index_t] & 0xff));                                                \
                }                                                                                 \
            printf("]\r\n");                                                                    \
        }                                                                                     \
    } while (0)

#endif
#else
#define MS_LOG_FORMAT(name, buf, len, format)
do
{
    //int i;
    log_get_mutex();
    log_release_mutex();
} while (0)
#endif
//#define MS_LOG_HEX(name,buf, len)				MS_LOG_FORMAT(name,buf, len, "%02x ")
//#define MS_LOG_DEC(name,buf, len)				MS_LOG_FORMAT(name,buf, len, "%02d ")
//#define MS_LOG_CHAR(name,buf, len)			MS_LOG_FORMAT(name,buf, len, "%c")
#define MS_LOG_HEX(name, buf, len) MS_LOG_FORMAT(ms_osal_thread_get_name(), name, buf, len, "%02x ", MS_LOG_LEVEL_DEBUG)
#define MS_LOG_DEC(name, buf, len) MS_LOG_FORMAT(ms_osal_thread_get_name(), name, buf, len, "%02d ", MS_LOG_LEVEL_DEBUG)
#define MS_LOG_CHAR(name, buf, len) MS_LOG_FORMAT(ms_osal_thread_get_name(), name, buf, len, "%c", MS_LOG_LEVEL_DEBUG)
#define MS_LOGI_HEX(name, buf, len) MS_LOG_FORMAT(ms_osal_thread_get_name(), name, buf, len, "%02x ", MS_LOG_LEVEL_INFO)
#define MS_LOGI_CHAR(name, buf, len) MS_LOG_FORMAT(ms_osal_thread_get_name(), name, buf, len, "%c", MS_LOG_LEVEL_INFO)
#define PRINT_BUF(buf,len) MS_LOG_HEX("net:",buf,len)
/**
 * @brief assert for NULL
 * 
 */
#ifndef MS_RELEASE_DOMAIN
#define MS_LOG_ASSERT assert(0)
#else
#define MS_LOG_ASSERT                             \
    do                                            \
    {                                             \
        MS_LOGE("%s;%d", __FUNCTION__, __LINE__); \
    } while (0)
#endif
#define MS_NULL_CHECK(p)   \
    do                     \
    {                      \
        if (NULL == p)     \
        {                  \
            MS_LOG_ASSERT; \
            return;        \
        }                  \
    } while (0)
#define MS_NULL_CHECK_WITH_VALUE(p,v)   \
    do                     \
    {                      \
        if (NULL == p)     \
        {                  \
            MS_LOG_ASSERT; \
            return v;      \
        }                  \
    } while (0)
#endif
