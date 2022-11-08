/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      bt_matter_adapter_app_task.c
   * @brief     Routines to create App task and handle events & messages
   * @author    jane
   * @date      2017-06-02
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <platform_opts_bt.h>
#if (defined(CONFIG_BT_MATTER_ADAPTER) && CONFIG_BT_MATTER_ADAPTER) || (defined(CONFIG_BT_AIRSYNC_CONFIG) && CONFIG_BT_AIRSYNC_CONFIG)
#include <osdep_service.h>
#include <os_msg.h>
#include <os_task.h>
#include <gap.h>
#include <gap_le.h>
#include <app_msg.h>
#include "bt_matter_adapter_wifi.h"
#include "bt_matter_adapter_app_task.h"
#include "bt_matter_adapter_peripheral_app.h"
#include "platform_stdlib.h"
#include <gap_msg.h>

/** @defgroup  BT_MATTER_ADAPTER_PERIPH_APP_TASK BT Config Peripheral App Task
    * @brief This file handles the implementation of application task related functions.
    *
    * Create App task and handle events & messages
    * @{
    */
/*============================================================================*
 *                              Macros
 *============================================================================*/
#define APP_TASK_PRIORITY             1         //!< Task priorities
#if CONFIG_BT_MESH_DEVICE_MATTER
#define APP_TASK_STACK_SIZE           1024 * 4   //!<  Task stack size
#else
#define APP_TASK_STACK_SIZE           1024 * 2   //!<  Task stack size
#endif
#define MAX_NUMBER_OF_GAP_MESSAGE     0x20      //!<  GAP message queue size
#define MAX_NUMBER_OF_IO_MESSAGE      0x20      //!<  IO message queue size
#define MAX_NUMBER_OF_EVENT_MESSAGE   (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE)    //!< Event message queue size

#if CONFIG_BT_MESH_DEVICE_MATTER
#define APP_CALLBACK_TASK_PRIORITY        APP_TASK_PRIORITY
#define APP_CALLBACK_TASK_STACK_SIZE      1024*4
#define MAX_NUMBER_OF_CALLBACK_MESSAGE    0x20
#endif
/*============================================================================*
 *                              Variables
 *============================================================================*/
static void *app_task_handle = NULL;   //!< APP Task handle
static void *evt_queue_handle = NULL;;  //!< Event queue handle
static void *io_queue_handle = NULL;;   //!< IO queue handle
#if CONFIG_BT_MESH_DEVICE_MATTER
void *bt_matter_callback_task_handle = NULL;   /* Callback Task handle */
void *bt_matter_callback_queue_handle = NULL;  /* Callback Queue handle */
#endif
extern T_GAP_DEV_STATE bt_matter_adapter_gap_dev_state;

#if CONFIG_BT_MESH_DEVICE_MATTER
bool bt_matter_adapter_send_msg(uint16_t sub_type, void *arg)
#else
void bt_matter_adapter_send_msg(uint16_t sub_type)
#endif
{
	uint8_t event = EVENT_IO_TO_APP;

	T_IO_MSG io_msg;

	io_msg.type = IO_MSG_TYPE_QDECODE;
	io_msg.subtype = sub_type;
#if CONFIG_BT_MESH_DEVICE_MATTER
	io_msg.u.buf = arg;
#endif
	if (evt_queue_handle != NULL && io_queue_handle != NULL) {
		if (os_msg_send(io_queue_handle, &io_msg, 0) == false) {
			BC_printf("bt config send msg fail: subtype 0x%x", io_msg.subtype);
		} else if (os_msg_send(evt_queue_handle, &event, 0) == false) {
			BC_printf("bt config send event fail: subtype 0x%x", io_msg.subtype);
		}
	}
#if CONFIG_BT_MESH_DEVICE_MATTER
	return true;
#endif
}
#if CONFIG_BT_MESH_DEVICE_MATTER
void bt_matter_adapter_send_callback_msg(uint16_t msg_type, uint8_t cb_type, void *arg)
{

	T_IO_MSG callback_msg;
	callback_msg.type = msg_type;

	if( (msg_type==BT_MATTER_SEND_CB_MSG_SEND_DATA_COMPLETE) || (msg_type==BT_MATTER_SEND_CB_MSG_IND_NTF_ENABLE) || 		(msg_type==BT_MATTER_SEND_CB_MSG_IND_NTF_DISABLE) || (msg_type==BT_MATTER_SEND_CB_MSG_READ_WRITE_CHAR) )
	{
		callback_msg.subtype = cb_type;
	}

	callback_msg.u.buf = arg;
	if (bt_matter_callback_queue_handle != NULL) {
		if (os_msg_send(bt_matter_callback_queue_handle, &callback_msg, 0) == false) {
			printf("bt config send msg fail: subtype 0x%x", callback_msg.type);
		}
	}
}

#endif
void bt_matter_adapter_app_main_task(void *p_param)
{
	(void) p_param;
	uint8_t event;

#if defined(configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1)
	osif_create_secure_context(configMINIMAL_SECURE_STACK_SIZE + 256);
#endif

	os_msg_queue_create(&io_queue_handle, MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
	os_msg_queue_create(&evt_queue_handle, MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint8_t));

	gap_start_bt_stack(evt_queue_handle, io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);

	while (1) {
		if (os_msg_recv(evt_queue_handle, &event, 0xFFFFFFFF) == true) {
			if (event == EVENT_IO_TO_APP) {
				T_IO_MSG io_msg;
				if (os_msg_recv(io_queue_handle, &io_msg, 0) == true) {
					bt_matter_adapter_app_handle_io_msg(io_msg);
				}
			} else {
				gap_handle_msg(event);
			}
		}
	}
}
#if CONFIG_BT_MESH_DEVICE_MATTER
void bt_matter_adapter_callback_main_task(void *p_param)
{
		(void)p_param;
		T_IO_MSG callback_msg;

		while (true)
		{
				if (os_msg_recv(bt_matter_callback_queue_handle, &callback_msg, 0xFFFFFFFF) == true)
				{
						bt_matter_handle_callback_msg(callback_msg);
				}
		}
}
#endif
void bt_matter_adapter_app_task_init()
{
	if (app_task_handle == NULL) {
		BC_printf("bt_matter_adapter_app_task_init\n\r");
		os_task_create(&app_task_handle, "bt_matter_adapter_app", bt_matter_adapter_app_main_task, 0, APP_TASK_STACK_SIZE,
					   APP_TASK_PRIORITY);
	}
	else {
		BC_printf("bt_matter_adapter_app_main_task already on\n\r");
	}
#if CONFIG_BT_MESH_DEVICE_MATTER
	os_msg_queue_create(&bt_matter_callback_queue_handle, MAX_NUMBER_OF_CALLBACK_MESSAGE, sizeof(T_IO_MSG));
	if (bt_matter_callback_task_handle == NULL) {
				os_task_create(&bt_matter_callback_task_handle, "bt_matter_adapter_callback", bt_matter_adapter_callback_main_task, 0, APP_CALLBACK_TASK_STACK_SIZE, APP_CALLBACK_TASK_PRIORITY);
		}
		else {
				printf("bt_matter_adapter_callback_main_task already on\n\r");
		}
#endif
}

void bt_matter_adapter_app_task_deinit()
{
	//gap_stop_bt_stack
	if (app_task_handle) {
		os_task_delete(app_task_handle);
	}
	if (io_queue_handle) {
		os_msg_queue_delete(io_queue_handle);
	}
	if (evt_queue_handle) {
		os_msg_queue_delete(evt_queue_handle);
	}
#if CONFIG_BT_MESH_DEVICE_MATTER
	if (bt_matter_callback_queue_handle) {
				os_msg_queue_delete(bt_matter_callback_queue_handle);
	}
	if (bt_matter_adapter_callback_main_task) {
			os_task_delete(bt_matter_adapter_callback_main_task);
	}
#endif
	io_queue_handle = NULL;
	evt_queue_handle = NULL;
	app_task_handle = NULL;
#if CONFIG_BT_MESH_DEVICE_MATTER
	bt_matter_callback_task_handle = NULL;   /* Callback Task handle */
	bt_matter_callback_queue_handle = NULL;
#endif
	bt_matter_adapter_gap_dev_state.gap_init_state = 0;
	bt_matter_adapter_gap_dev_state.gap_adv_sub_state = 0;
	bt_matter_adapter_gap_dev_state.gap_adv_state = 0;
	bt_matter_adapter_gap_dev_state.gap_scan_state = 0;
	bt_matter_adapter_gap_dev_state.gap_conn_state = 0;
}

#endif
