/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */
#ifndef _BT_MESH_PROVISIONER_OTA_CLIENT_APP_TASK_H_
#define _BT_MESH_PROVISIONER_OTA_CLIENT_APP_TASK_H_

#include "mesh_config.h"

extern void *bt_mesh_provisioner_ota_client_evt_queue_handle;  //!< Event queue handle
extern void *bt_mesh_provisioner_ota_client_io_queue_handle;   //!< IO queue handle

/**
 * @brief  Initialize App task
 * @return void
 */
void bt_mesh_provisioner_ota_client_app_task_init(void);

/**
 * @brief  Deinitialize App task
 * @return void
 */
void bt_mesh_provisioner_ota_client_app_task_deinit(void);

#endif

