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

#ifndef _MESH_APPLICATION__
#define _MESH_APPLICATION__

#ifdef __cplusplus
extern "C" {
#endif

#include <profile_client.h>
#include <profile_server.h>
#include "app_msg.h"
#include "mesh_api.h"
#if F_BT_MESH_1_1_MBT_SUPPORT
#include "blob_client_app.h"
#endif
#if F_BT_MESH_1_1_DFU_SUPPORT
#include "dfu_distributor_app.h"
#include "dfu_initiator_app.h"
#endif

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
void bt_mesh_device_app_handle_io_msg(T_IO_MSG io_msg);
/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT bt_mesh_device_app_gap_callback(uint8_t cb_type, void *p_cb_data);

/**
 * @brief  Callback will be called when data sent from profile client layer.
 * @param  client_id the ID distinguish which module sent the data.
 * @param  conn_id connection ID.
 * @param  p_data  pointer to data.
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT bt_mesh_device_app_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data);
T_APP_RESULT bt_mesh_device_app_profile_callback(T_SERVER_ID service_id, void *p_data);

void device_info_cb(uint8_t bt_addr[6], uint8_t bt_addr_type, int8_t rssi, device_info_t *pinfo);
bool prov_cb(prov_cb_type_t cb_type, prov_cb_data_t cb_data);
void fn_cb(uint8_t frnd_index, fn_cb_type_t type, uint16_t lpn_addr);
void lpn_cb(uint8_t frnd_index, lpn_cb_type_t type, uint16_t fn_addr);
void hb_cb(hb_data_type_t type, void *pargs);
void bt_mesh_device_app_vendor_callback(uint8_t cb_type, void *p_cb_data);
bool rpl_cb(mesh_rpl_fail_type_t type, uint8_t rpl_loop, uint16_t src, uint32_t iv_index,
            uint32_t rpl_seq, uint32_t seq);
#if F_BT_MESH_1_1_DF_SUPPORT
uint16_t df_cb(uint8_t type, void *pdata);
#endif
#if F_BT_MESH_1_1_MBT_SUPPORT
void blob_client_cb(blob_transfer_cb_data_t *pcb_data);
#endif
#if F_BT_MESH_1_1_DFU_SUPPORT
void dfu_dist_cb(dfu_dist_cb_data_t *pcb_data);
#endif
#ifdef __cplusplus
}
#endif

#endif /* _MESH_APPLICATION__S */
