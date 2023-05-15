/*
 * Copyright (c) 2019 - 2020 IoT Company of Midea Group.
 *
 * File Name 	    : 
 * Description	    : rtos adaptor
 *
 * Version	        : v0.1
 * Author			: 
 * Date	            : 2019/09/15  refactoring
 * History	        : 
 */

#ifndef __MS_OSAL_H__
#define __MS_OSAL_H__

// #include <k_api.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "ms_osal_typedef.h"

typedef signed char 	int8_t;
typedef unsigned char   uint8_t;

typedef signed short	int16_t;
typedef unsigned short  uint16_t;

typedef enum
{
	MS_OSAL_RESULT_SUCCESS            = 0x00,
	MS_OSAL_RESULT_ERROR              = 0x01,
}ms_osal_result_t;

// ms_osal_result_t ms_osal_err_map (kstat_t err_code);
ms_osal_result_t ms_osal_err_map (int err_code);
/**
 * \brief CLR_STRUCT(p) \n
 * 宏函数，清空结构体�?
 * \param p  结构体指�?
 * \return 肯定执行成功.
 */
#define CLR_STRUCT(p)					do {memset(p, 0x0, sizeof(*p));} while(0)

/**
* @Function	Force a context switch
* @Parameter	none
* @return	none
**/
void ms_osal_context_force_switch(void);


/**
* @Function	Enter critical section
* @Parameter	none
* @return	none
**/
void ms_osal_enter_critical_section(void);


/**
* @Function	Exit critical section
* @Parameter	none
* @return	none
**/
void ms_osal_exit_critical_section(void);


/**
* @Function	Disable all maskable interrupts
* @Parameter	none
* @return	none
**/
void ms_osal_disable_all_interrupts(void);


/**
* @Function	Enable microcontroller interrupts
* @Parameter	none
* @return	none
**/
void ms_osal_enable_all_interrupts(void);


/**
* @Function	Start the RTOS scheduler
* @Parameter	none
* @return	none
**/
void ms_osal_scheduler_start(void);


/**
* @Function	Stop the RTOS scheduler
* @Parameter	none
* @return	none
**/
void ms_osal_scheduler_stop(void);


/**
* @Function	Suspends the scheduler without disabling interrupts
* @Parameter	none
* @return	none
**/
void ms_osal_scheduler_suspend(void);


/**
* @Function	Resume the RTOS scheduler
* @Parameter	none
* @return	none
**/
void ms_osal_scheduler_resume(void);

/**
* @Function	Get Realtek WIFI2.0 Version
* @Parameter sVersion ( Min Size 20)
* @return	none
**/
void ms_osal_version(char* sVersion);

#endif

