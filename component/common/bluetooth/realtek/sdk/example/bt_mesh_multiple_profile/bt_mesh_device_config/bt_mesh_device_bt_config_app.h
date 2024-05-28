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

#ifndef _BT_MESH_DEVICE_BT_CONFIG_APP__
#define _BT_MESH_DEVICE_BT_CONFIG_APP__

#ifdef __cplusplus
extern "C" {
#endif

#include <profile_client.h>
#include <profile_server.h>
#include <gap_le.h>
#include "app_msg.h"
#include "mesh_api.h"
#include "bt_mesh_device_bt_config_app_flags.h"

/*============================================================================*
 *                              Variables
 *============================================================================*/
extern T_SERVER_ID bt_mesh_device_bt_config_srv_id; /**< Simple ble service id*/
extern T_SERVER_ID bt_private_provision_srv_id; /**< Simple ble service id*/
//extern T_SERVER_ID bas_srv_id;  /**< Battery service id */


/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void bt_mesh_device_bt_config_app_handle_io_msg(T_IO_MSG io_msg);
/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT bt_mesh_device_bt_config_app_gap_callback(uint8_t cb_type, void *p_cb_data);

/**
 * @brief  Callback will be called when data sent from profile client layer.
 * @param  client_id the ID distinguish which module sent the data.
 * @param  conn_id connection ID.
 * @param  p_data  pointer to data.
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT bt_mesh_device_bt_config_app_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data);

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
T_APP_RESULT bt_mesh_device_bt_config_app_profile_callback(T_SERVER_ID service_id, void *p_data);


void device_info_cb(uint8_t bt_addr[6], uint8_t bt_addr_type, int8_t rssi, device_info_t *pinfo);
bool prov_cb(prov_cb_type_t cb_type, prov_cb_data_t cb_data);
void fn_cb(uint8_t frnd_index, fn_cb_type_t type, uint16_t lpn_addr);
void lpn_cb(uint8_t frnd_index, lpn_cb_type_t type, uint16_t fn_addr);
void bt_mesh_device_bt_config_app_vendor_callback(uint8_t cb_type, void *p_cb_data);
bool rpl_cb(mesh_rpl_fail_type_t type, uint8_t rpl_loop, uint16_t src, uint32_t iv_index,
            uint32_t rpl_seq, uint32_t seq);
#if F_BT_MESH_1_1_DF_SUPPORT
uint16_t df_cb(uint8_t type, void *pdata);
#endif
#ifdef __cplusplus
}
#endif

#endif /* _MESH_APPLICATION__S */