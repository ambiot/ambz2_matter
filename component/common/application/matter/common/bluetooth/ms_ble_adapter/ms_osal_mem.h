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
#ifndef __MS_OSAL_MEM_H__
#define __MS_OSAL_MEM_H__

#include <stdint.h>
#include "ms_osal.h"
#include "ms_osal_log.h"

typedef struct
{
   /* the number of bytes of free heap.*/
   	uint32_t free_heap_size; 
   /* the biggest block of bytes of free heap*/
    uint32_t biggest_free_block_size;
}ms_osal_mem_info_t;

/**
* @Function	Allocate memory
* @Parameter	size: Size of the memory to be allocated
* @return	Pointer to the allocated memory, NULL if allocation fails
**/
void *ms_osal_memory_alloc(int32_t size);
void *ms_osal_memory_calloc(size_t n, size_t size);

/**
* @Function	Free memory
* @Parameter	pmem: Pointer to the memory to be freed
**/
void ms_osal_memory_free (void *pmem);


/**
* @Function	Query memory information
* @Parameter	pmem_info: Pointer to the memory informaiton
* @return	OS_RESULT_SUCCESS: 	query successfully
*			OS_RESULT_ERROR  : 	query failed
**/
ms_osal_result_t ms_osal_memory_info (ms_osal_mem_info_t *pmem_info);

/**
* @Function	get thread stack space
* @Parameter	thread_id: thread id
* @return	OS_RESULT_SUCCESS: 	the available stack size
*			OS_RESULT_ERROR  : 	0
**/
uint32_t ms_osal_thread_stack_free(void *osThreadId_t);

/**
* @Function	    printf memory size
* @Parameter	null
* @return	    null
**/
void ms_osal_memory_printf(void);


/**
* @Function     printf memory verbose info
* @Parameter    null
* @return       null
**/
void ms_osal_memory_dump_verbose_info(void);


#endif

