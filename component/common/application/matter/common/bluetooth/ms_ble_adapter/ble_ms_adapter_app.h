/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      central_client_app.h
   * @brief     This file handles BLE central client application routines.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

#ifndef _MS_ADAPTER_APP_H_
#define _MS_ADAPTER_APP_H_

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <profile_client.h>
#include <app_msg.h>
#include <ble_ms_adapter_app_flags.h>
#include <profile_server.h>
#include "ble_ms_adapter_service.h"

#define BLE_PRINT    printf
#define BD_ADDR_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define BD_ADDR_ARG(x) (x)[5],(x)[4],(x)[3],(x)[2],(x)[1],(x)[0]
#define UUID_128_FORMAT "0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X"
#define UUID_128(x)  x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7],x[8],x[9],x[10],x[11],x[12],x[13],x[14],x[15]

#define MAX_REGISRABLE_SERVICE_NUMBER 12

#if CONFIG_MS_MULTI_ADV
#define MAX_ADV_NUMBER     2
#define MS_MULTI_TASK_STACK_SIZE  256*10
#define MS_MULTI_TASK_PRIORITY  4
#define MS_MULTI_ADV_DATA_QUEUE_SIZE  0x60
#endif
/*============================================================================*
 *                              Variables
 *============================================================================*/
extern T_CLIENT_ID   ble_ms_adapter_gcs_client_id;         /**< General Common Services client client id*/

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

typedef struct {
	uint8_t      adv_evt_type;
	uint32_t     adv_intv_min;       /// Minimum advertising interval (in unit of 625us). Must be greater than 20ms
	uint32_t     adv_intv_max;       /// Maximum advertising interval (in unit of 625us). Must be greater than 20ms
	unsigned int adv_datalen;
	unsigned int scanrsp_datalen;
	uint8_t      adv_data[31];
	uint8_t      scanrsp_data[31];
} M_ADV_PARAM;
#if CONFIG_MS_MULTI_ADV
typedef struct {
	uint8_t 	 adv_datalen;
	uint8_t 	 scanrsp_datalen;
	uint8_t      adv_data[31];
	uint8_t      scanrsp_data[31];
	uint16_t 	 H_adv_intval;
	uint8_t 	 local_bd_type;
	uint8_t 	 is_used;
	uint8_t 	 type;        //1 means matter   2 means msmart
	uint8_t 	 adv_id;
	void 		*one_shot_timer;
	void 		*update_adv_mutex;
	uint16_t 	adv_int_min;
	uint16_t 	adv_int_max;
} M_MULTI_ADV_PARAM;

typedef enum {
	BMS_CALLBACK_MSG_CONNECTED,
	BMS_CALLBACK_MSG_CONNECTED_MATTER,
	BMS_CALLBACK_MSG_DISCONNECTED,
	BMS_CALLBACK_MSG_DISCONNECTED_MATTER,
	BMS_CALLBACK_MSG_ADV_OFF,
	BMS_CALLBACK_MSG_CMP_WRITE_RECV_MATTER,
	BMS_CALLBACK_MSG_CMP_WRITE_RECIEVED,
	//BMS_CALLBACK_MSG_CMP_CCCD_RECV_ENABLE_MATTER,
	//BMS_CALLBACK_MSG_CMP_CCCD_RECV_DISABLE_MATTER,
	BMS_CALLBACK_MSG_CMP_INDICATE_MATTER,
	BMS_CALLBACK_MSG_CMP_CCCD_RECV_MATTER,
	BMS_CALLBACK_MSG_CMP_INDICATE,
	//BMS_CALLBACK_MSG_SEND_DATA_COMPLETE_MATTER
} T_BMS_CALLBACK_MSG_TYPE;

typedef struct {
	T_BMS_CALLBACK_MSG_TYPE type;
	void *buf;
} T_BMS_CALLBACK_MSG;
#endif
typedef struct {
	void *task_handle;
	void *sem_handle;
	void *queue_handle;
	bool deinit_flag;
	uint8_t msmart_sta_sto_flag;   //for msmart
	uint8_t matter_sta_sto_flag;   //for matter
	uint8_t adv_id;
} T_MULTI_ADV_CONCURRENT;


typedef enum {
	BMS_CONN_ADV = 1,
	BMS_NONCONN_ADV,
} M_ADV_TYPE;

typedef struct {

	uint8_t  scan_mode;
	uint8_t  scan_filter_duplicate;
	uint8_t  scan_filter_policy;
	uint16_t scan_interval;
	uint16_t scan_window;
} M_SCAN_PARAM;

typedef struct {

	uint16_t conn_id;
	uint8_t uuid_size;
	uint8_t *uuid;
} M_DISC_SRV_UUID_PARAM;

typedef struct {

	uint16_t conn_id;
	uint16_t start_hdl;
	uint16_t end_hdl;
	uint8_t uuid_size;
	uint8_t *uuid;
} M_DISC_CHAR_UUID_PARAM;

typedef struct {

	uint16_t conn_id;
	uint16_t start_hdl;
	uint16_t end_hdl;
} M_DISC_DESC_PARAM;

typedef struct {
	uint16_t conn_id;
	T_GATT_WRITE_TYPE type;
	uint8_t *value;
	uint16_t value_size;
	uint16_t att_handle;
} M_WRITE_PARAM;

typedef struct {
	T_GAP_WHITE_LIST_OP operation;
	uint8_t *remote_bd;
	T_GAP_REMOTE_ADDR_TYPE remote_bd_type;
} M_MODIFY_WHITELIST_PARAM;

typedef struct {
	uint16_t conn_id;
	uint16_t conn_interval_min;
	uint16_t conn_interval_max;
	uint16_t slave_latency;
	uint16_t supervision_timeout;
	uint16_t ce_length_min;
	uint16_t ce_length_max;
} M_CONNECTION_REQ_PARAM; //connection param request

typedef struct {
	uint8_t remote_bd[GAP_BD_ADDR_LEN];
	T_GAP_REMOTE_ADDR_TYPE remote_bd_type;
	T_GAP_REMOTE_ADDR_TYPE local_bd_type;
} M_CON_PARAM;

typedef struct {
	uint8_t write_len;
	uint8_t write_value[MS_WRITE_MAX_LEN];
	ms_hal_ble_service_write_cb write_cb;
} T_MS_WRITE_DATA;

typedef struct {
	uint8_t device_name_len;
	uint8_t *device_name;
} M_SET_DEVICE_NAME;

typedef struct save_scan_info_t {
	struct save_scan_info_t *p_next; // Pointer to the next item, must be the first field.
	ms_hal_ble_report_type     type;                           // report ad type
	ms_hal_ble_addr_t     peer_addr;                           // peer addr
	int8_t                   tx_pwr;                           /// TX power (in dBm)
	int8_t                     rssi;                           // rssi
	uint16_t                    len;                           //data len
	uint8_t                   *data;                           //data
} M_SAVE_SCAN_INFO;

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
void ble_ms_adapter_app_handle_io_msg(T_IO_MSG io_msg);
/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT ble_ms_adapter_app_gap_callback(uint8_t cb_type, void *p_cb_data);

/**
 * @brief  Callback will be called when data sent from profile client layer.
 * @param  client_id the ID distinguish which module sent the data.
 * @param  conn_id connection ID.
 * @param  p_data  pointer to data.
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT ble_ms_adapter_gcs_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data);
#if F_BT_GAPS_CHAR_WRITEABLE
T_APP_RESULT ble_ms_adapter_gap_service_callback(T_SERVER_ID service_id, void *p_para);
#endif

void ble_ms_adapter_app_handle_callback_msg(T_IO_MSG callback_msg);



void ble_ms_adapter_app_vendor_callback(uint8_t cb_type, void *p_cb_data);
#if CONFIG_MS_MULTI_ADV
void ble_ms_adapter_multi_adv_init();
void ble_ms_adapter_multi_adv_deinit();
void ble_ms_adapter_send_multi_adv_msg(uint8_t adv_id);
void ble_ms_adapter_legacy_start_adv_callback(void *data);
#endif
#ifdef __cplusplus
}
#endif

#endif

