/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      app_flags.h
   * @brief     This file is used to config app functions.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */
#ifndef _BT_MESH_DEVICE_MATTER_APP_FLAGS_H_
#define _BT_MESH_DEVICE_MATTER_APP_FLAGS_H_

#include "platform_opts_bt.h"

#include <app_common_flags.h>

/**
 * @addtogroup MESH_APP_CONFIG
 * @{
 */

/**
 * @defgroup  Mesh_App_Config Mesh App Configuration
 * @brief This file is used to config app functions.
 * @{
 */
/*============================================================================*
 *                              Constants
 *============================================================================*/
#if defined(CONFIG_PLATFORM_8721D)
#define BT_MESH_DEVICE_MATTER_APP_MAX_LINKS  4
#define BT_MESH_DEVICE_MATTER_PERIPHERAL_APP_MAX_LINKS   1 //for max slave link num
#define BT_MESH_DEVICE_MATTER_CENTRAL_APP_MAX_LINKS      3 //for max master link num
#elif defined(CONFIG_PLATFORM_8710C)
#define BT_MESH_DEVICE_MATTER_APP_MAX_LINKS  2
#define BT_MESH_DEVICE_MATTER_PERIPHERAL_APP_MAX_LINKS   1 //for max slave link num
#define BT_MESH_DEVICE_MATTER_CENTRAL_APP_MAX_LINKS      1 //for max master link num
#endif

/** @brief  Config the discovery table number of gcs_client */
#define BT_MESH_DEVICE_MATTER_SCATTERNET_APP_MAX_DISCOV_TABLE_NUM 40

/** @brief  Config airplane mode support: 0-Not built in, 1-built in, use user command to set*/
#define F_BT_AIRPLANE_MODE_SUPPORT          0

#endif
