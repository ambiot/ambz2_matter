/*
 * Copyright (c) 2019 - 2020 M-SMART Research Institute of Midea Group.
 * All rights reserved.	
 *
 * File Name 		: ms_osal_typedef.h
 * Introduction	: 
 *
 * Current Version	: v0.1
 * Author			: Zachary Chau(zhouzh6@midea.com)
 * Create Time	: 2019/12/18
 * Change Log	     	:
 *
 * All software, firmware and related documentation herein ("M-Smart Software") are
 * intellectual property of M-SMART Research Institute of Midea Group and protected 
 * by law, including, but not limited to, copyright law and international treaties.
 *
 * Any use, modification, reproduction, retransmission, or republication of all
 * or part of M-Smart Software is expressly prohibited, unless prior written
 * permission has been granted by M-Smart.
 *
 */
 
#ifndef __MS_OSAL_TYPEDEF_H__
#define __MS_OSAL_TYPEDEF_H__

typedef void*  ms_osal_timer_t;
typedef int    ms_osal_timer_tick_t;
typedef unsigned long  ms_osal_timer_time_t;

typedef void*  ms_osal_mutex_t;

typedef void*  ms_osal_queue_t;

typedef void*  ms_osal_semaphore_t;

typedef void*  ms_osal_thread_t;

typedef void*  ms_osal_ringbuf_t;

#ifndef MS_CONFIG_MODULE_HI3861
#define CRYPTO_RAM_TEXT_SECTION
#else
#include "hi_types_base.h"
#endif /* MS_CONFIG_MODULE_HI3861 */

#endif

