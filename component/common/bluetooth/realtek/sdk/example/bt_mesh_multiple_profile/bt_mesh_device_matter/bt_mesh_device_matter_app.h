/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      device_app.h
* @brief     Smart mesh demo application
* @details
* @author    bill
* @date      2015-11-12
* @version   v0.1
* *********************************************************************************************************
*/

#ifndef _BT_MESH_DEVICE_MATTER_APP_H__
#define _BT_MESH_DEVICE_MATTER_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <profile_client.h>
#include <profile_server.h>
#include "app_msg.h"
#include "mesh_api.h"
#include "bt_mesh_device_matter_app_flags.h"

/*============================================================================*
 *                              Variables
 *============================================================================*/
#define BLE_PRINT	printf
#define BD_ADDR_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define BD_ADDR_ARG(x) (x)[5],(x)[4],(x)[3],(x)[2],(x)[1],(x)[0]
#define UUID_128_FORMAT "0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X"
#define UUID_128(x)  x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7],x[8],x[9],x[10],x[11],x[12],x[13],x[14],x[15]

extern T_CLIENT_ID bt_mesh_device_matter_gcs_client_id;  /**< General Common Services client client id*/
extern T_SERVER_ID bt_mesh_device_matter_adapter_srv_id;        /**< Matter service id */

extern int bt_mesh_device_matter_peripheral_app_max_links;
extern int bt_mesh_device_matter_central_app_max_links;

extern bool mesh_initial_state;
extern T_GAP_DEV_STATE bt_mesh_device_matter_gap_dev_state;
extern uint8_t bt_mesh_device_matter_config_adv_flag;

typedef struct
{
	uint8_t conn_id;
	uint8_t new_state;
	uint16_t disc_cause;
} BT_MATTER_CONN_EVENT;

typedef enum
{
	BT_MATTER_MSG_START_ADV = 12,
	BT_MATTER_MSG_STOP_ADV,
	BT_MATTER_MSG_SEND_DATA,
} BT_MATTER_SERVER_MSG_TYPE;

typedef enum
{
	BT_MATTER_SEND_CB_MSG_DISCONNECTED = 1,
	BT_MATTER_SEND_CB_MSG_CONNECTED,
	BT_MATTER_SEND_CB_MSG_SEND_DATA_COMPLETE,
	BT_MATTER_SEND_CB_MSG_IND_NTF_ENABLE,
	BT_MATTER_SEND_CB_MSG_IND_NTF_DISABLE,
	BT_MATTER_SEND_CB_MSG_WRITE_CHAR,
} BT_MATTER_SEND_MSG_TYPE;

typedef enum
{
	CB_PROFILE_CALLBACK = 0x01,  /**< bt_matter_adapter_app_profile_callback */
	CB_GAP_CALLBACK = 0x02, /**< bt_matter_adapter_app_gap_callback */
	CB_GAP_MSG_CONN_EVENT = 0x3, /**< bt_matter_adapter_app_handle_gap_msg */
} T_CHIP_BLEMGR_CALLBACK_TYPE;
/*============================================================================*
 *                              Functions
 *============================================================================*/
typedef int (*chip_blemgr_callback)(void *param, void *cb_data, int type, T_CHIP_BLEMGR_CALLBACK_TYPE callback_type);
void chip_blemgr_set_callback_func(chip_blemgr_callback p, void *data);

void bt_mesh_device_matter_handle_callback_msg(T_IO_MSG callback_msg);

void bt_mesh_device_matter_app_handle_gap_msg(T_IO_MSG *p_gap_msg);

/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void bt_mesh_device_matter_app_handle_io_msg(T_IO_MSG io_msg);
/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT bt_mesh_device_matter_app_gap_callback(uint8_t cb_type, void *p_cb_data);

/**
 * @brief  Callback will be called when data sent from profile client layer.
 * @param  client_id the ID distinguish which module sent the data.
 * @param  conn_id connection ID.
 * @param  p_data  pointer to data.
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT bt_mesh_device_matter_app_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data);

/**
    * @brief    All the BT Profile service callback events are handled in this function
    * @note     Then the event handling function shall be called according to the
    *           service_id
    * @param    service_id  Profile service ID
    * @param    p_data      Pointer to callback data
    * @return   T_APP_RESULT, which indicates the function call is successful or not
    * @retval   APP_RESULT_SUCCESS  Function run successfully
    * @retval   others              Function run failed, and return number indicates the reason
    */
T_APP_RESULT bt_mesh_device_matter_app_profile_callback(T_SERVER_ID service_id, void *p_data);

/**
 * @brief  Callback will be called when data sent from profile client layer.
 * @param  client_id the ID distinguish which module sent the data.
 * @param  conn_id connection ID.
 * @param  p_data  pointer to data.
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT bt_mesh_device_matter_gcs_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data);

#if F_BT_GAPS_CHAR_WRITEABLE
T_APP_RESULT bt_mesh_device_matter_gap_service_callback(T_SERVER_ID service_id, void *p_para);
#endif

void device_info_cb(uint8_t bt_addr[6], uint8_t bt_addr_type, int8_t rssi, device_info_t *pinfo);
bool prov_cb(prov_cb_type_t cb_type, prov_cb_data_t cb_data);
void fn_cb(uint8_t frnd_index, fn_cb_type_t type, uint16_t lpn_addr);
void lpn_cb(uint8_t frnd_index, lpn_cb_type_t type, uint16_t fn_addr);
void hb_cb(hb_data_type_t type, void *pargs);
void bt_mesh_device_matter_app_vendor_callback(uint8_t cb_type, void *p_cb_data);
#ifdef __cplusplus
}
#endif

#endif /* _MESH_APPLICATION__S */
