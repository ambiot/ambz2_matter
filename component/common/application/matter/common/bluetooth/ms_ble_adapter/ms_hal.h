/*
 * Copyright (c) 2019 - 2020 IoT Company of Midea Group.
 *
 * File Name 	    : 
 * Description	    : HAL adaptor
 *
 * Version	        : v0.1
 * Author			: 
 * Date	            :
 * History	        : 
 */

#ifndef __MS_HAL_H__
#define __MS_HAL_H__

//#include "ms_osal_log.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	MS_HAL_RESULT_SUCCESS            = 0x00,
	MS_HAL_RESULT_ERROR              = 0x01,
	MS_HAL_RESULT_FLASH_ITEM_EMPTY   = 0x02,
}ms_hal_result_t;


/** ms_hal_logic_partition_t
*   @brief:  Details of single partition
*/
typedef struct
{
    uint32_t partition_start_addr;
    uint32_t partition_length;
} ms_hal_logic_partition_t;

/** MS_PARTITION_T
*   @brief:  User defined flash partitions table
*/
typedef enum MS_HAL_PARTITION_T
{
	MS_OTA_PARTITION_MCU,           //to save dev-mcu ota firmware
	MS_OTA_PARTITION_MCU_PARA,      //to save dev-mcu para
	MS_OTA_PARTITION_HEAD_PARA,     //to save ota firmware header info
}ms_hal_partition_t;

#ifdef __cplusplus
}
#endif

#endif

