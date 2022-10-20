/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      app_task.c
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
#include "platform_opts_bt.h"
#if defined(CONFIG_BT_JOYLINK_ADAPTER) && CONFIG_BT_JOYLINK_ADAPTER
#include <os_msg.h>
#include <os_task.h>
#include <gap.h>
#include <gap_le.h>
#include "bt_joylink_adapter_app_task.h"
#include <app_msg.h>
#include "bt_joylink_adapter_peripheral_app.h"
#include "platform_stdlib.h"
#include <basic_types.h>
#include <gap_msg.h>
#include <os_mem.h>

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define BT_JOYLINK_ADAPTER_APP_TASK_PRIORITY             4         //!< Task priorities
#define BT_JOYLINK_ADAPTER_APP_TASK_STACK_SIZE           256 * 4   //!<  Task stack size
#define BT_JOYLINK_ADAPTER_CALLBACK_TASK_PRIORITY        4         //!< Task priorities
#define BT_JOYLINK_ADAPTER_CALLBACK_TASK_STACK_SIZE      256 * 8   //!<  Task stack size

#define MAX_NUMBER_OF_GAP_MESSAGE                        0x20      //!<  GAP message queue size
#define MAX_NUMBER_OF_IO_MESSAGE                         0x20      //!<  IO message queue size
#define MAX_NUMBER_OF_EVENT_MESSAGE                      (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE)    //!< Event message queue size
#define MAX_NUMBER_OF_CALLBACK_MESSAGE                   0x20      //!< Callback message queue size


/*============================================================================*
 *                              Variables
 *============================================================================*/
void *bt_joylink_adapter_app_task_handle = NULL;         //!< APP task handle
void *bt_joylink_adapter_evt_queue_handle = NULL;        //!< Event queue handle
void *bt_joylink_adapter_io_queue_handle = NULL;         //!< IO queue handle
void *bt_joylink_adapter_callback_task_handle = NULL;    //!< Callback task handle
void *bt_joylink_adapter_callback_queue_handle = NULL;   //!< Callback queue handle


extern T_GAP_DEV_STATE bt_joylink_adapter_gap_dev_state;

/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief        App task to handle events & messages
 * @param[in]    p_param    Parameters sending to the task
 * @return       void
 */
void bt_joylink_adapter_app_main_task(void *p_param)
{
	(void)p_param;
	uint8_t event;

#if defined(configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1)
	osif_create_secure_context(configMINIMAL_SECURE_STACK_SIZE + 256);
#endif

	os_msg_queue_create(&bt_joylink_adapter_io_queue_handle, MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
	os_msg_queue_create(&bt_joylink_adapter_evt_queue_handle, MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint8_t));

	gap_start_bt_stack(bt_joylink_adapter_evt_queue_handle, bt_joylink_adapter_io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);

	while (true) {
		if (os_msg_recv(bt_joylink_adapter_evt_queue_handle, &event, 0xFFFFFFFF) == true) {
			if (event == EVENT_IO_TO_APP) {
				T_IO_MSG io_msg;
				if (os_msg_recv(bt_joylink_adapter_io_queue_handle, &io_msg, 0) == true) {
					bt_joylink_adapter_app_handle_io_msg(io_msg);
				}
			} else {
				gap_handle_msg(event);
			}
		}
	}
}

void bt_joylink_adapter_callback_main_task(void *p_param)
{
	(void)p_param;
	T_BJA_CALLBACK_MSG callback_msg;

	os_msg_queue_create(&bt_joylink_adapter_callback_queue_handle, MAX_NUMBER_OF_CALLBACK_MESSAGE, sizeof(T_BJA_CALLBACK_MSG));

	while (true) {
		if (os_msg_recv(bt_joylink_adapter_callback_queue_handle, &callback_msg, 0xFFFFFFFF) == true) {
			bt_joylink_adapter_app_handle_callback_msg(callback_msg);
		}
	}
}

/**
 * @brief  Initialize App task
 * @return void
 */
void bt_joylink_adapter_app_task_init(void)
{
	os_task_create(&bt_joylink_adapter_app_task_handle, "bt_joylink_adapter_app", bt_joylink_adapter_app_main_task, 0, BT_JOYLINK_ADAPTER_APP_TASK_STACK_SIZE,
				   BT_JOYLINK_ADAPTER_APP_TASK_PRIORITY);
	os_task_create(&bt_joylink_adapter_callback_task_handle, "bt_joylink_adapter_callback", bt_joylink_adapter_callback_main_task, 0,
				   BT_JOYLINK_ADAPTER_CALLBACK_TASK_STACK_SIZE,
				   BT_JOYLINK_ADAPTER_CALLBACK_TASK_PRIORITY);
}

void bt_joylink_adapter_app_task_deinit(void)
{
	if (bt_joylink_adapter_app_task_handle) {
		os_task_delete(bt_joylink_adapter_app_task_handle);
	}
	if (bt_joylink_adapter_callback_task_handle) {
		os_task_delete(bt_joylink_adapter_callback_task_handle);
	}
	if (bt_joylink_adapter_io_queue_handle) {
		os_msg_queue_delete(bt_joylink_adapter_io_queue_handle);
	}
	if (bt_joylink_adapter_evt_queue_handle) {
		os_msg_queue_delete(bt_joylink_adapter_evt_queue_handle);
	}
	if (bt_joylink_adapter_callback_queue_handle) {
		os_msg_queue_delete(bt_joylink_adapter_callback_queue_handle);
	}
	bt_joylink_adapter_app_task_handle = NULL;
	bt_joylink_adapter_callback_task_handle = NULL;
	bt_joylink_adapter_io_queue_handle = NULL;
	bt_joylink_adapter_evt_queue_handle = NULL;
	bt_joylink_adapter_callback_queue_handle = NULL;

	bt_joylink_adapter_gap_dev_state.gap_init_state = 0;
	bt_joylink_adapter_gap_dev_state.gap_adv_sub_state = 0;
	bt_joylink_adapter_gap_dev_state.gap_adv_state = 0;
	bt_joylink_adapter_gap_dev_state.gap_scan_state = 0;
	bt_joylink_adapter_gap_dev_state.gap_conn_state = 0;

}

#endif
