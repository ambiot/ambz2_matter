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
#include <platform_opts_bt.h>
#if defined(CONFIG_BT_MESH_DEVICE_MATTER) && CONFIG_BT_MESH_DEVICE_MATTER
#include <os_msg.h>
#include <os_task.h>
#include <gap.h>
#include <gap_le.h>
#include <gap_msg.h>
#include <trace.h>
#include <app_msg.h>
#include "mesh_api.h"
#include "mesh_data_uart.h"
#include "mesh_user_cmd_parse.h"
#include "device_cmd.h"
#include "platform_opts.h"
#include "ble_scatternet_user_cmd.h"
#include "bt_mesh_device_matter_app.h"
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
#include "bt_mesh_user_api.h"
#endif
#if defined(CONFIG_PLATFORM_8721D)
#include "ameba_soc.h"
#endif
/*============================================================================*
 *                              Macros
 *============================================================================*/
#define EVENT_MESH                    0x80
#define APP_TASK_PRIORITY             4         //!< Task priorities
#define APP_TASK_STACK_SIZE           256 * 10  //!< Task stack size
#define MAX_NUMBER_OF_GAP_MESSAGE     0x20      //!< GAP message queue size
#define MAX_NUMBER_OF_IO_MESSAGE      0x20      //!< IO message queue size
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
#define MAX_NUMBER_OF_EVENT_MESSAGE   (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE + MESH_INNER_MSG_NUM) //!< Event message queue size
#else
#define MAX_NUMBER_OF_EVENT_MESSAGE   (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE + MESH_INNER_MSG_NUM) //!< Event message queue size
#endif

//ble matter
#define BT_MATTER_APP_CALLBACK_TASK_PRIORITY        APP_TASK_PRIORITY
#define BT_MATTER_APP_CALLBACK_TASK_STACK_SIZE      1024*4
#define BT_MATTER_MAX_NUMBER_OF_CALLBACK_MESSAGE    0x20

#if defined(CONFIG_PLATFORM_8721D)
#define UART_TX    _PA_18
#define UART_RX    _PA_19
#elif defined(CONFIG_PLATFORM_8710C)
#include "serial_api.h"
#define UART_TX    PA_14
#define UART_RX    PA_13
#endif

/*============================================================================*
 *                              Variables
 *============================================================================*/
void *bt_mesh_device_matter_app_task_handle;   //!< APP Task handle
void *bt_mesh_device_matter_evt_queue_handle;  //!< Event queue handle
void *bt_mesh_device_matter_io_queue_handle;   //!< IO queue handle
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
void *bt_mesh_device_matter_user_cmd_io_queue_handle;   //!< user cmd queue handle
#endif

void *bt_mesh_device_matter_callback_task_handle = NULL;   /* Callback Task handle */
void *bt_mesh_device_matter_callback_queue_handle = NULL;  /* Callback Queue handle */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/*note: [input]subtype: set as BT_MATTER_SERVER_MSG_TYPE*/
bool bt_mesh_device_matter_adapter_send_msg(uint16_t sub_type, void *arg)
{
	uint8_t event = EVENT_IO_TO_APP;

	T_IO_MSG io_msg;

	io_msg.type = IO_MSG_TYPE_QDECODE;
	io_msg.subtype = sub_type;
	io_msg.u.buf = arg;

	if (bt_mesh_device_matter_evt_queue_handle != NULL && bt_mesh_device_matter_io_queue_handle != NULL) {
		if (os_msg_send(bt_mesh_device_matter_io_queue_handle, &io_msg, 0) == false) {
			printf("bt matter adapter send msg fail: subtype 0x%x", io_msg.subtype);
		} else if (os_msg_send(bt_mesh_device_matter_evt_queue_handle, &event, 0) == false) {
			printf("bt matter adapter event fail: subtype 0x%x", io_msg.subtype);
		}
	}
	return true;
}

bool bt_mesh_device_matter_adapter_send_callback_msg(uint16_t msg_type, uint8_t cb_type, void *arg)
{
	T_IO_MSG callback_msg;
	callback_msg.type = msg_type;
	callback_msg.u.buf = arg;

	if( (msg_type==BT_MATTER_SEND_CB_MSG_SEND_DATA_COMPLETE) || (msg_type==BT_MATTER_SEND_CB_MSG_IND_NTF_ENABLE) ||\
		(msg_type==BT_MATTER_SEND_CB_MSG_IND_NTF_DISABLE) || (msg_type==BT_MATTER_SEND_CB_MSG_WRITE_CHAR) )
	{
		callback_msg.subtype = cb_type;
	}

	if (bt_mesh_device_matter_callback_queue_handle != NULL) {
		if (os_msg_send(bt_mesh_device_matter_callback_queue_handle, &callback_msg, 0) == false) {
			printf("bt matter send msg fail: subtype 0x%x", callback_msg.type);
			return false;
		}

		return true;
	}

	return false;
}

void bt_mesh_device_matter_adapter_callback_main_task(void *p_param)
{
	(void)p_param;
	T_IO_MSG callback_msg;

	os_msg_queue_create(&bt_mesh_device_matter_callback_queue_handle, BT_MATTER_MAX_NUMBER_OF_CALLBACK_MESSAGE, sizeof(T_IO_MSG));

	while (true)
	{
		if (os_msg_recv(bt_mesh_device_matter_callback_queue_handle, &callback_msg, 0xFFFFFFFF) == true)
		{
			bt_mesh_device_matter_handle_callback_msg(callback_msg);
		}
	}
}

/*************************************************************************************************************/
void app_send_uart_msg(uint8_t data)
{
    uint8_t event = EVENT_IO_TO_APP;
    T_IO_MSG msg;
    msg.type = IO_MSG_TYPE_UART;
    msg.subtype = data;
    if (os_msg_send(bt_mesh_device_matter_io_queue_handle, &msg, 0) == false)
    {
    }
    else if (os_msg_send(bt_mesh_device_matter_evt_queue_handle, &event, 0) == false)
    {
    }
}

#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
extern CMD_MOD_INFO_S btMeshCmdPriv;
int bt_mesh_send_io_msg(T_IO_MSG *p_io_msg)
{
    uint8_t event = EVENT_USER_API_REQ;
    CMD_ITEM_S *pmeshCmdItem_s = NULL;
    PUSER_ITEM puserItem = NULL;

    pmeshCmdItem_s = (CMD_ITEM_S *)p_io_msg->u.buf;
    puserItem = (PUSER_ITEM)pmeshCmdItem_s->pmeshCmdItem->userData;
    //trace commands transmitted to mesh stack
    btMeshCmdPriv.cmdTransmittedNum ++;
    if (os_msg_send(bt_mesh_device_matter_user_cmd_io_queue_handle, p_io_msg, 0) == false)
    {
        pmeshCmdItem_s->msgRecvFlag = 1;
        if (btMeshCmdPriv.cmdTransmittedNum) {
            btMeshCmdPriv.cmdTransmittedNum --;
        }
        printf("[BT_MESH] Send io_msg to bt_mesh_device_matter_user_cmd_io_queue_handle fail!\r\n");
        return 1;
    }
    else if (os_msg_send(bt_mesh_device_matter_evt_queue_handle, &event, 0) == false)
    {
        pmeshCmdItem_s->msgRecvFlag = 1;
        if (btMeshCmdPriv.cmdTransmittedNum) {
            btMeshCmdPriv.cmdTransmittedNum --;
        }
        printf("[BT_MESH] Send io_msg to bt_mesh_device_matter_evt_queue_handle fail!\r\n");
        return 1;
    }

    return 0;
}
#endif

/**
 * @brief        App task to handle events & messages
 * @param[in]    p_param    Parameters sending to the task
 * @return       void
 */
void bt_mesh_device_matter_app_main_task(void *p_param)
{
    /* avoid gcc compile warning */
    (void)p_param;
    uint8_t event;

#if defined(configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1)
    osif_create_secure_context(configMINIMAL_SECURE_STACK_SIZE + 256);
#endif

    os_msg_queue_create(&bt_mesh_device_matter_io_queue_handle, MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
    os_msg_queue_create(&bt_mesh_device_matter_evt_queue_handle, MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint8_t));
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
    os_msg_queue_create(&bt_mesh_device_matter_user_cmd_io_queue_handle, MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
#endif
    gap_start_bt_stack(bt_mesh_device_matter_evt_queue_handle, bt_mesh_device_matter_io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);

    mesh_start(EVENT_MESH, EVENT_IO_TO_APP, bt_mesh_device_matter_evt_queue_handle, bt_mesh_device_matter_io_queue_handle);

    mesh_data_uart_init(UART_TX, UART_RX, app_send_uart_msg);
    mesh_user_cmd_init("MeshDevice");

    while (true)
    {
        if (os_msg_recv(bt_mesh_device_matter_evt_queue_handle, &event, 0xFFFFFFFF) == true)
        {
            if (event == EVENT_IO_TO_APP)
            {
                T_IO_MSG io_msg;
                if (os_msg_recv(bt_mesh_device_matter_io_queue_handle, &io_msg, 0) == true)
                {
                    bt_mesh_device_matter_app_handle_io_msg(io_msg);
                }
            }
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
            else if (event == EVENT_USER_API_REQ) {
                T_IO_MSG io_msg;
                if (os_msg_recv(bt_mesh_device_matter_user_cmd_io_queue_handle, &io_msg, 0) == true)
                {
                    bt_mesh_io_msg_handler(io_msg);
                }
            }
#endif
#if defined(CONFIG_EXAMPLE_BT_MESH_DEMO) && CONFIG_EXAMPLE_BT_MESH_DEMO
            else if (event == EVENT_USER_HTTP_SERVER)
            {
                T_IO_MSG io_msg;
                if (os_msg_recv(bt_mesh_device_matter_io_queue_handle, &io_msg, 0) == true)
                {
                    bt_mesh_demo_io_msg_handler(io_msg);
                }
            }
#endif
            else if (event == EVENT_MESH)
            {
                mesh_inner_msg_handle(event);
            }
            else
            {
                gap_handle_msg(event);
            }
        }
    }
}

/**
 * @brief  Initialize App task
 * @return void
 */
void bt_mesh_device_matter_app_task_init()
{
	/*app main task*/
	os_task_create(&bt_mesh_device_matter_app_task_handle, "bt_mesh_device_matter_app_task", bt_mesh_device_matter_app_main_task, 
						0, APP_TASK_STACK_SIZE, APP_TASK_PRIORITY);

	/*ble matter task*/
	if (bt_mesh_device_matter_callback_task_handle == NULL) {
		os_task_create(&bt_mesh_device_matter_callback_task_handle, "bt_mesh_device_matter_adapter_callback", bt_mesh_device_matter_adapter_callback_main_task, 
						0, BT_MATTER_APP_CALLBACK_TASK_STACK_SIZE, BT_MATTER_APP_CALLBACK_TASK_PRIORITY);
	}

}

/**
 * @brief  Deinitialize App task
 * @return void
 */
void bt_mesh_device_matter_app_task_deinit(void)
{
	if (bt_mesh_device_matter_app_task_handle) {
		os_task_delete(bt_mesh_device_matter_app_task_handle);
	}
	if (bt_mesh_device_matter_io_queue_handle) {
		os_msg_queue_delete(bt_mesh_device_matter_io_queue_handle);
	}
	if (bt_mesh_device_matter_evt_queue_handle) {
		os_msg_queue_delete(bt_mesh_device_matter_evt_queue_handle);
	}
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
	if (bt_mesh_device_matter_user_cmd_io_queue_handle) {
		os_msg_queue_delete(bt_mesh_device_matter_user_cmd_io_queue_handle);
	}
	bt_mesh_device_matter_user_cmd_io_queue_handle = NULL;
#endif
	bt_mesh_device_matter_io_queue_handle = NULL;
	bt_mesh_device_matter_evt_queue_handle = NULL;
	bt_mesh_device_matter_app_task_handle = NULL;

	bt_mesh_device_matter_peripheral_app_max_links = 0;
	bt_mesh_device_matter_central_app_max_links = 0;

	//bt matter
	if (bt_mesh_device_matter_callback_task_handle) {
		os_task_delete(bt_mesh_device_matter_callback_task_handle);
	}
	if (bt_mesh_device_matter_callback_queue_handle) {
		os_msg_queue_delete(bt_mesh_device_matter_callback_queue_handle);
	}
	bt_mesh_device_matter_callback_task_handle = NULL;
	bt_mesh_device_matter_callback_queue_handle = NULL;
	bt_mesh_device_matter_config_adv_flag = 0; //matter moudle disable config adv

	mesh_user_cmd_deinit("MeshDevice");
}

#endif
