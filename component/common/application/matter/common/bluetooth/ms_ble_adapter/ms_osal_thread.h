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
 
#ifndef __MS_OSAL_THREAD_H__
#define __MS_OSAL_THREAD_H__
#include <stdint.h>
#include "ms_osal.h"
#include "ms_osal_typedef.h"

#define MS_OSAL_THREAD_PRI_LOW     2
#define MS_OSAL_THREAD_PRI_NORMAL  1
#define MS_OSAL_THREAD_PRI_HIGH    0
/**
* @Function	Create a new task and add it to the list of tasks that are ready to run
* @Parameter	thandle: 		Used to pass back a handle by which the created task can be referenced
*			name: 		A descriptive name for the task
*			main_func: 	Pointer to the task entry function
*			arg: 			Pointer that will be used as the parameter for the task being created
*			stack_size: 	The size of the task stack
*			priority: 		The priority at which the task should run
* @return	OS_RESULT_SUCCESS: 	create successfully
*			OS_RESULT_ERROR  : 	create failed
**/
ms_osal_result_t ms_osal_thread_create(ms_osal_thread_t *thandle,
											const char *name,
		     								void (*main_func) (void *arg), 
		     								void *arg,
		     								uint32_t stack_size, 
		     								int32_t priority,
											uint8_t autorun);


/**
* @Function	Remove a task from the RTOS kernels management
* @Parameter	thandle: The handle of the task to be deleted. Passing NULL will cause the calling task to be deleted.
* @return	OS_RESULT_SUCCESS: 	delete successfully
*			OS_RESULT_ERROR:   	delete failed
**/
ms_osal_result_t ms_osal_thread_delete(ms_osal_thread_t *thandle);


/**
* @Function	sleep a task for a given number of ticks
* @Parameter	msecs: The amount of time, in millisecond, that the calling task should block
* @return	none
**/
void ms_osal_thread_sleep(int32_t msecs);


/**
* @Function	Yield the thread
* @return	OS_RESULT_SUCCESS: 	yield successfully
*			OS_RESULT_ERROR  : 	yield failed
**/
ms_osal_result_t ms_osal_thread_yield(void);


/**
* @Function	Run a thread if not autorun
* @Parameter	task: Task structure pointer
**/
void ms_osal_thread_run(ms_osal_thread_t *thandle);


/**
* @Function	Wake up the thread that is sleep
* @Parameter	thandle: The sleep thread to be waked up
* @return	OS_RESULT_SUCCESS: 	wake successfully
*			OS_RESULT_ERROR  : 	wake failed
**/
ms_osal_result_t ms_osal_thread_wake(ms_osal_result_t *thandle);


/**
* @Function	Suspend the thread
* @Parameter	thandle: Handle to the task being suspended. Passing a NULL handle will cause the calling task to be suspended
* @return	none
**/
void ms_osal_thread_suspend(ms_osal_thread_t *thandle);


/**
* @Function	Resume the thread
* @Parameter	thandle: Handle to the task being readied
* @return	none
**/
void ms_osal_thread_resume(ms_osal_thread_t *thandle);

/**
* @Function	Return thread name
* @Parameter	thandle: Handle to the task being readied
* @return	none
**/
const char *ms_osal_thread_get_name(void);


#endif /* __MS_OSAL_KERNEL_H__ */

