/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */
#ifndef _BT_MESH_DEVICE_MATTER_APP_TASK_H_
#define _BT_MESH_DEVICE_MATTER_APP_TASK_H_

#include "mesh_config.h"
#include "app_msg.h"

extern void *bt_mesh_device_matter_evt_queue_handle;  //!< Event queue handle
extern void *bt_mesh_device_matter_io_queue_handle;   //!< IO queue handle

bool bt_mesh_device_matter_adapter_send_msg(uint16_t sub_type, void *arg);

bool bt_mesh_device_matter_adapter_send_callback_msg(uint16_t msg_type, uint8_t cb_type, void *arg);

void app_send_uart_msg(uint8_t data);

int bt_mesh_send_io_msg(T_IO_MSG *p_io_msg);



/**
 * @brief  Initialize App task
 * @return void
 */
void bt_mesh_device_matter_app_task_init(void);

/**
 * @brief  Deinitialize App task
 * @return void
 */
void bt_mesh_device_matter_app_task_deinit(void);

#endif

