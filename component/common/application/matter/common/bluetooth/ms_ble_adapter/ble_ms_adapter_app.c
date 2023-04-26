/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      central_client_app.c
   * @brief     This file handles BLE central application routines.
   * @author    jane
   * @date      2017-06-06
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
#if defined(CONFIG_BT_MS_ADAPTER) && CONFIG_BT_MS_ADAPTER
#include <stdio.h>
#include <app_msg.h>
#include <string.h>
#include <trace_app.h>
#include <gap_scan.h>
#include <gap.h>
#include <gap_adv.h>
#include <gap_msg.h>
#include <gap_bond_le.h>
#include <ble_ms_adapter_app.h>
#include <ble_ms_adapter_link_mgr.h>
#include <gcs_client.h>
#include "gatt_builtin_services.h"
#include "ms_hal_ble.h"
#include "os_mem.h"
#include "os_msg.h"
#include "os_sync.h"
#include "os_queue.h"
#if CONFIG_MS_MULTI_ADV
#include "vendor_cmd_bt.h"
#include "matter_blemgr_common.h"
#include "os_timer.h"
#endif
/** @defgroup  CENTRAL_CLIENT_APP Central Client Application
    * @brief This file handles BLE central client application routines.
    * @{
    */
/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @addtogroup  CENTRAL_CLIIENT_CALLBACK
    * @{
    */
T_CLIENT_ID   ble_ms_adapter_gcs_client_id;         /**< General Common Services client client id*/
extern T_SERVER_ID ble_matter_adapter_service_id;				 /**< BT matter service id*/

/** @} */ /* End of group CENTRAL_CLIIENT_CALLBACK */

/** @defgroup  CENTRAL_CLIENT_GAP_MSG GAP Message Handler
    * @brief Handle GAP Message
    * @{
    */
T_GAP_DEV_STATE ble_ms_adapter_gap_dev_state = {0, 0, 0, 0, 0};                /**< GAP device state */
int ble_ms_adapter_peripheral_app_max_links = 0;
int ble_ms_adapter_central_app_max_links = 0;
T_OS_QUEUE scan_info_queue;
extern void *ms_add_service_sem;
extern uint8_t modify_whitelist_result;
extern void *modify_whitelist_sem;
extern bool M_accept;
extern ms_hal_ble_stack_callback_t callback_handler;
extern uint8_t adv_type;
extern uint8_t update_adv;
extern void *ble_ms_adapter_evt_queue_handle;  //!< Event queue handle
extern void *ble_ms_adapter_io_queue_handle;   //!< IO queue handle
extern void *ble_ms_adapter_callback_queue_handle;   //!< Callback queue handle
extern ms_hal_ble_scan_callback_t scan_result_callback;
extern T_APP_RESULT ble_ms_adapter_app_profile_callback(T_SERVER_ID service_id, void *p_data);
extern void ble_ms_adapter_switch_bt_address(uint8_t *address);
extern void ble_ms_adapter_search_and_free_service(T_SERVER_ID service_id);
int scan_info_flag = 0;
extern void *adv_start_sem;
#if CONFIG_MS_MULTI_ADV
uint8_t ms_local_public_addr[6] = {0};
uint8_t ms_local_static_random_addr[6] = {0};
M_MULTI_ADV_PARAM ms_multi_adv_param_array[MAX_ADV_NUMBER] = {0};
T_MULTI_ADV_CONCURRENT ms_multi_adapter = {0};

uint8_t link_customer = 0;    //1  means connection is matter   2 means connection is msmart
extern uint8_t msmart_adv_id;
extern matter_blemgr_callback matter_blemgr_callback_func;
extern void *matter_blemgr_callback_data;
extern uint16_t matter_adv_interval;
extern uint16_t matter_adv_int_min;
extern uint16_t matter_adv_int_max;
extern uint8_t matter_adv_data[31];
extern uint8_t matter_adv_data_length;
extern uint8_t matter_adv_id;
extern uint8_t matter_msmart_bt_deinit_id;
uint8_t ms_get_unused_adv_index(void)
{
	int i;
	for(i= 0; i < MAX_ADV_NUMBER; i++){
		if(!ms_multi_adv_param_array[i].is_used)
			return i;
	}
	return MAX_ADV_NUMBER;
}

bool ms_matter_ble_adv_stop_by_adv_id(uint8_t *adv_id)
{
	if(*adv_id < 0 || *adv_id >= MAX_ADV_NUMBER){
			printf("[%s] wrong input advId:%d\r\n",__func__, *adv_id);
			return 1;
	}	
	if(!ms_multi_adv_param_array[*adv_id].is_used){
		printf("[%s] adv id %d is already stop or not start\r\n",__func__, *adv_id);
		return 1;
	}

	if(ms_multi_adv_param_array[*adv_id].one_shot_timer){
		os_timer_delete(&ms_multi_adv_param_array[*adv_id].one_shot_timer);
	}
	if (ms_multi_adv_param_array[*adv_id].update_adv_mutex) {
		os_mutex_delete(ms_multi_adv_param_array[*adv_id].update_adv_mutex);
	}
	memset(&ms_multi_adv_param_array[*adv_id], 0, sizeof(M_MULTI_ADV_PARAM));
	*adv_id = MAX_ADV_NUMBER;

	return 0;
}

bool msmart_matter_ble_adv_start_by_adv_id(uint8_t *adv_id, uint8_t *adv_data, uint16_t adv_len, uint8_t *rsp_data, uint16_t rsp_len, uint8_t type)
{
	if (ble_ms_adapter_peripheral_app_max_links != 0) {
		printf("[%s] as slave role, BLE is conneted\r\n", __func__);
		return 1;
	}

	if ((MAX_ADV_NUMBER != *adv_id) && (ms_multi_adv_param_array[*adv_id].is_used == 1)) {
		os_timer_stop(&ms_multi_adv_param_array[*adv_id].one_shot_timer);
	} else {
		*adv_id = ms_get_unused_adv_index();
		if(MAX_ADV_NUMBER == *adv_id){
			printf("[%s] Extend the max adv num %d\r\n", __func__, MAX_ADV_NUMBER);
			return 1;
		}
	}

	uint8_t adv_index = *adv_id;
	M_MULTI_ADV_PARAM *h_adv_param;
	h_adv_param = ms_multi_adv_param_array + adv_index;
	if (!h_adv_param->update_adv_mutex) {
		os_mutex_create(&h_adv_param->update_adv_mutex);
	}
	if (type == 1) { //matter
		os_mutex_take(h_adv_param->update_adv_mutex, 0xFFFF);
		memcpy(h_adv_param->adv_data, matter_adv_data, matter_adv_data_length);
		h_adv_param->adv_datalen = matter_adv_data_length;
		h_adv_param->adv_int_min = matter_adv_int_min;
		h_adv_param->adv_int_max = matter_adv_int_max;
		h_adv_param->local_bd_type = GAP_LOCAL_ADDR_LE_RANDOM;
		h_adv_param->H_adv_intval = matter_adv_interval;
		h_adv_param->is_used = 1;
		h_adv_param->type = 1;
		ms_multi_adapter.matter_sta_sto_flag = false;
		os_mutex_give(h_adv_param->update_adv_mutex);
	} else if (type == 2) { //msmart
		os_mutex_take(h_adv_param->update_adv_mutex, 0xFFFF);
		memcpy(h_adv_param->adv_data, adv_data, adv_len);
		memcpy(h_adv_param->scanrsp_data, rsp_data, rsp_len);
		h_adv_param->adv_datalen = adv_len;
		h_adv_param->scanrsp_datalen = rsp_len;
		h_adv_param->local_bd_type = GAP_LOCAL_ADDR_LE_PUBLIC;
		h_adv_param->H_adv_intval = 320;
		h_adv_param->is_used = 1;
		h_adv_param->type = 2;
		ms_multi_adapter.msmart_sta_sto_flag = false;
		os_mutex_give(h_adv_param->update_adv_mutex);
	}

	if(h_adv_param->one_shot_timer == NULL){
		if (os_timer_create(&h_adv_param->one_shot_timer, "start adv_timer", adv_index, (int)(h_adv_param->H_adv_intval*0.625), 1, ble_ms_adapter_legacy_start_adv_callback) == false){
			printf("os_timer_create h_adv_param->one_shot_timer fail!\r\n");
			return 1;
		}
	}

	if (ms_multi_adapter.deinit_flag == true) { // If deinit, return 1
		printf("[%s]device ble deinit flag is set, return 1\r\n", __func__);
		return 1;
	}
	ble_ms_adapter_send_multi_adv_msg(adv_index);
	os_timer_start(&h_adv_param->one_shot_timer);
	return 0;
}

uint8_t ble_ms_adapter_judge_adv_stop(uint8_t adv_id)
{
	uint8_t flag = 0;
	if (adv_id == msmart_adv_id) {
		ms_multi_adapter.adv_id = adv_id;
		if (ms_multi_adapter.msmart_sta_sto_flag == true) {
			flag = 1;
		}
	} else if (adv_id == matter_adv_id) {
		ms_multi_adapter.adv_id = adv_id;
		if (ms_multi_adapter.matter_sta_sto_flag == true) {
			flag = 1;
		}
	}
	return flag;
}
void ble_ms_adapter_multi_adv_task_func(void *arg)
{
	(void)arg;
	uint8_t adv_id;
	uint8_t adv_stop_flag = 0;
	while (true) {
		if (os_msg_recv(ms_multi_adapter.queue_handle, &adv_id, 0xFFFFFFFF) == true) {
			if (adv_id == matter_msmart_bt_deinit_id) { // If deinit, break the outer while loop
				printf("[%s]device ble deinit flag is set, break\r\n", __func__);
				break;
			} else if (adv_id == msmart_adv_id) {
				ms_multi_adapter.adv_id = msmart_adv_id;
				if (ms_multi_adapter.msmart_sta_sto_flag == true) {
					printf("[%s]stop msmart conn adv flag is set[%d]continue\r\n", __func__, adv_id);
					continue;
				}
			} else if (adv_id == matter_adv_id) {
				ms_multi_adapter.adv_id = matter_adv_id;
				if (ms_multi_adapter.matter_sta_sto_flag == true) {
					printf("[%s]stop matter conn adv flag is set[%d], continue\r\n", __func__, adv_id);
					continue;
				}
			} else {
				printf("[%s]adv_id = %d\r\n", adv_id);
				continue;
			}

			ms_multi_adv_param_array[adv_id].adv_id = adv_id;
			if (ms_multi_adv_param_array[adv_id].local_bd_type == GAP_LOCAL_ADDR_LE_PUBLIC) {
				le_cfg_local_identity_address(ms_local_public_addr, GAP_IDENT_ADDR_PUBLIC);
			} else if (ms_multi_adv_param_array[adv_id].local_bd_type == GAP_LOCAL_ADDR_LE_RANDOM) {
				le_cfg_local_identity_address(ms_local_static_random_addr, GAP_IDENT_ADDR_RAND);
			}
			le_adv_set_param(GAP_PARAM_ADV_LOCAL_ADDR_TYPE, sizeof(ms_multi_adv_param_array[adv_id].local_bd_type), &ms_multi_adv_param_array[adv_id].local_bd_type);
			if (ms_multi_adv_param_array[adv_id].update_adv_mutex) {
				os_mutex_take(ms_multi_adv_param_array[adv_id].update_adv_mutex, 0xFFFF);
				le_adv_set_param(GAP_PARAM_ADV_DATA, ms_multi_adv_param_array[adv_id].adv_datalen, (void *)ms_multi_adv_param_array[adv_id].adv_data);
				if (ms_multi_adv_param_array[adv_id].type == 2) { //msmart
					le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, ms_multi_adv_param_array[adv_id].scanrsp_datalen, (void *)ms_multi_adv_param_array[adv_id].scanrsp_data);
				} else if (ms_multi_adv_param_array[adv_id].type == 1) { //matter
					le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(ms_multi_adv_param_array[adv_id].adv_int_min), &ms_multi_adv_param_array[adv_id].adv_int_min);
					le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(ms_multi_adv_param_array[adv_id].adv_int_max), &ms_multi_adv_param_array[adv_id].adv_int_max);
				}
				os_mutex_give(ms_multi_adv_param_array[adv_id].update_adv_mutex);
			} else {
				printf("[%s]update_adv_mutex is NULL[%d]\r\n", __func__, adv_id);
			}
			if (ble_ms_adapter_app_send_api_msg(BMS_API_MSG_UPDATE_ADV_PARAM, &ms_multi_adv_param_array[adv_id]) == false) {
				printf("[%s]send api msg fail\r\n", __func__);
				continue;
			}

			if (os_sem_take(ms_multi_adapter.sem_handle, 0xFFFFFFFF) == false) {
				printf("os_sem_take ms_multi_adapter.sem_handle fail!\r\n");
			}
		}
	}
	printf("delete multi_adv_task\r\n");
	os_sem_delete(ms_multi_adapter.sem_handle);
	os_msg_queue_delete(ms_multi_adapter.queue_handle);
	memset(&ms_multi_adapter, 0, sizeof(ms_multi_adapter));
	os_task_delete(NULL);
}

void ble_ms_adapter_multi_adv_init()
{
	memset(&ms_multi_adapter, 0, sizeof(ms_multi_adapter));
	ms_multi_adapter.deinit_flag = false;
	ms_multi_adapter.matter_sta_sto_flag = true;
	ms_multi_adapter.msmart_sta_sto_flag = true;
	ms_multi_adapter.adv_id = MAX_ADV_NUMBER;

	if (os_sem_create(&ms_multi_adapter.sem_handle, 0, 1) == false) {
		printf("os_sem_create ms_milti_adapter.sem_handle fail!\r\n");
	}
	if (os_msg_queue_create(&ms_multi_adapter.queue_handle, MS_MULTI_ADV_DATA_QUEUE_SIZE, sizeof(uint8_t)) == false) {
		printf("os_msg_queue_create ms_milti_adapter.queue_handle fail!\r\n");
	}

	if (os_task_create(&ms_multi_adapter.task_handle, "multi_adv_task", ble_ms_adapter_multi_adv_task_func, NULL, MS_MULTI_TASK_STACK_SIZE, MS_MULTI_TASK_PRIORITY) == false) {
		printf("os_task_create ms_milti_adapter.task_handle fail!\r\n");
	}

	memset(ms_multi_adv_param_array, 0, sizeof(M_MULTI_ADV_PARAM) * MAX_ADV_NUMBER);
	
}

void ble_ms_adapter_multi_adv_deinit()
{
	for(int i = 0; i < MAX_ADV_NUMBER; i ++){
		if(ms_multi_adv_param_array[i].one_shot_timer){
			os_timer_delete(&ms_multi_adv_param_array[i].one_shot_timer);
		}
	}
	memset(ms_multi_adv_param_array, 0, MAX_ADV_NUMBER * sizeof(M_MULTI_ADV_PARAM));
	ms_multi_adapter.deinit_flag = true;
	ble_ms_adapter_send_multi_adv_msg(matter_msmart_bt_deinit_id);

	if (ms_multi_adapter.sem_handle) {
		if (os_sem_give(ms_multi_adapter.sem_handle) == false) {
			printf("[%s]os_sem_give fail\r\n", __func__);
		}
	}
}

void ble_ms_adapter_send_multi_adv_msg(uint8_t adv_id)
{
	if (ms_multi_adapter.deinit_flag == true) {
		return;
	}
	if (ms_multi_adapter.queue_handle != NULL) {
		if(os_msg_send(ms_multi_adapter.queue_handle, &adv_id, 0) == false){
		printf("Send adv id to adv data queue fail[%d]\r\n", adv_id);
		}
	} else {
		printf("ms_multi_adapter.queue_handle is NULL\r\n");
	}
}

void ble_ms_adapter_legacy_start_adv_callback(void *data)
{
	int timerID;
	os_timer_id_get(&data,&timerID);
	if (ms_multi_adapter.deinit_flag == true) {
		return;
	}
	ble_ms_adapter_send_multi_adv_msg(timerID);
}

#endif

/*============================================================================*
 *                              Functions
 *============================================================================*/
void ble_ms_adapter_app_handle_gap_msg(T_IO_MSG  *p_gap_msg);
bool ble_ms_adapter_app_send_api_msg(T_BMS_API_MSG_TYPE sub_type, void *buf)
{
	T_IO_MSG io_msg;

	uint8_t event = EVENT_IO_TO_APP;

	io_msg.type = IO_MSG_TYPE_QDECODE;
	io_msg.subtype = sub_type;
	io_msg.u.buf = buf;

	if (ble_ms_adapter_evt_queue_handle != NULL && ble_ms_adapter_io_queue_handle != NULL) {
		if (os_msg_send(ble_ms_adapter_io_queue_handle, &io_msg, 0) == false) {
			printf("[%s] send io queue fail! type = 0x%x\r\n", __FUNCTION__, io_msg.subtype);
			return false;
		} else if (os_msg_send(ble_ms_adapter_evt_queue_handle, &event, 0) == false) {
			printf("[%s] send event queue fail! type = 0x%x\r\n", __FUNCTION__, io_msg.subtype);
			return false;
		}
	} else {
		printf("[%s] queue is empty! type = 0x%x\r\n", __FUNCTION__, io_msg.subtype);
		return false;
	}

	return true;
}
bool ble_ms_adapter_app_send_callback_msg(T_BMS_CALLBACK_MSG_TYPE type, void *buf)
{
	T_BMS_CALLBACK_MSG callback_msg;

	callback_msg.type = type;
	callback_msg.buf = buf;

	if (ble_ms_adapter_callback_queue_handle != NULL) {
		if (os_msg_send(ble_ms_adapter_callback_queue_handle, &callback_msg, 0) == false) {
			printf("[%s] send callback queue fail! type = 0x%x\r\n", __FUNCTION__, callback_msg.type);
			return false;
		}
	} else {
		printf("[%s] queue is empty! type = 0x%x\r\n", __FUNCTION__, callback_msg.type);
		return false;
	}

	return true;
}

void ble_ms_adapter_app_handle_api_msg(T_IO_MSG io_msg)
{
	//Handle API msg here
	uint16_t msg_type = io_msg.subtype;
	T_GAP_CAUSE cause = GAP_CAUSE_SUCCESS;
	switch (msg_type) {
#if CONFIG_MS_MULTI_ADV
	case BMS_API_MSG_UPDATE_ADV_PARAM: {
		M_MULTI_ADV_PARAM *adv_param = io_msg.u.buf;
		uint8_t adv_stop_flag = 0;
		adv_stop_flag = ble_ms_adapter_judge_adv_stop(adv_param->adv_id);
		if (adv_stop_flag) {
			printf("stop adv flag[%d] is set, give sem and break\r\n", adv_param->adv_id);
			if(ms_multi_adapter.sem_handle)
				os_sem_give(ms_multi_adapter.sem_handle);
			break;
		}
		cause = le_adv_update_param();
		if (cause != GAP_CAUSE_SUCCESS) {
			printf("le_adv_update_param fail! ret = 0x%x\r\n", cause);
		}
	}
	break;
#endif
	case BMS_API_MSG_START_ADV: {
		M_ADV_PARAM *m_adv_param = io_msg.u.buf;
		if (m_adv_param) {
			le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(m_adv_param->adv_evt_type), &m_adv_param->adv_evt_type);
			le_adv_set_param(GAP_PARAM_ADV_DATA, m_adv_param->adv_datalen, (void *)m_adv_param->adv_data);
			le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, m_adv_param->scanrsp_datalen, (void *)m_adv_param->scanrsp_data);

			cause = le_adv_start();
			if (cause != GAP_CAUSE_SUCCESS) {
				printf("[%s]call le_adv_start fail ret %d\r\n", __func__, cause);
				if (adv_start_sem) {
					os_sem_give(adv_start_sem);
				}
			}
			os_mem_free(m_adv_param);
		} else {
			printf("[%s] m_adv_param is NULL\r\n ", __func__);
			if (adv_start_sem) {
				os_sem_give(adv_start_sem);
			}
		}
	}
	break;
	case BMS_API_MSG_STOP_NON_ADV:
	case BMS_API_MSG_STOP_ADV:
		cause = le_adv_stop();
		if (cause != GAP_CAUSE_SUCCESS) {
			printf("[%s]call le_adv_stop fail ret %d\r\n", __func__, cause);
		}

		break;
	case BMS_API_MSG_START_NON_ADV: {
		M_ADV_PARAM *m_non_adv_param = io_msg.u.buf;
		if (m_non_adv_param) {
			le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(m_non_adv_param->adv_evt_type), &m_non_adv_param->adv_evt_type);
			le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(m_non_adv_param->adv_intv_min), &m_non_adv_param->adv_intv_min);
			le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(m_non_adv_param->adv_intv_max), &m_non_adv_param->adv_intv_max);
			le_adv_set_param(GAP_PARAM_ADV_DATA, m_non_adv_param->adv_datalen, (void *)m_non_adv_param->adv_data);

			cause = le_adv_start();
			if (cause != GAP_CAUSE_SUCCESS) {
				printf("[%s]call le_adv_start fail ret %d\r\n", __func__, cause);
			}

			os_mem_free(m_non_adv_param);
		} else {
			printf("[%s] m_non_adv_param is NULL\r\n", __func__);
		}
	}
	break;
	case BMS_API_MSG_CONNECT: {
		M_CON_PARAM *connect_param = io_msg.u.buf;
		if (connect_param) {
			T_GAP_LE_CONN_REQ_PARAM conn_req_param;
			conn_req_param.scan_interval = 0x520;	/* 820ms */
			conn_req_param.scan_window = 0x520;		/* 820ms */
			conn_req_param.conn_interval_min = 0x60;
			conn_req_param.conn_interval_max = 0x60;
			conn_req_param.conn_latency = 0;
			conn_req_param.supv_tout = 1000;
			conn_req_param.ce_len_min = 2 * (conn_req_param.conn_interval_min - 1);
			conn_req_param.ce_len_max = 2 * (conn_req_param.conn_interval_max - 1);

			le_set_conn_param(GAP_CONN_PARAM_1M, &conn_req_param);

			printf("cmd_con, DestAddr: 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",
				   connect_param->remote_bd[5], connect_param->remote_bd[4], connect_param->remote_bd[3], connect_param->remote_bd[2], connect_param->remote_bd[1],
				   connect_param->remote_bd[0]);

			cause = le_connect(0, connect_param->remote_bd, connect_param->remote_bd_type, connect_param->local_bd_type, 1000);
			if (cause != GAP_CAUSE_SUCCESS) {
				printf("[%s]call le_connect fail ret %d\r\n", __func__, cause);
			}

			os_mem_free(connect_param);
		} else {
			printf("[%s] connect_param is NULL\r\n", __func__);
		}
	}
	break;
	case BMS_API_MSG_DISCONNECT: {
		uint8_t *conn_id = (uint8_t *)io_msg.u.buf;//(uint8_t) io_msg.u.buf;
		if (conn_id) {
			cause = le_disconnect(*conn_id);
			if (cause != GAP_CAUSE_SUCCESS) {
				printf("[%s]call le_disconnect fail ret %d\r\n", __func__, cause);
			}
			os_mem_free(conn_id);
		} else {
			printf("[%s] conn_id is NULL\n\n", __func__);
		}

	}
	break;
	case BMS_API_MSG_START_SCAN: {
		M_SCAN_PARAM *scan_param = io_msg.u.buf;
		if (scan_param) {
			le_scan_set_param(GAP_PARAM_SCAN_MODE, sizeof(scan_param->scan_mode), &scan_param->scan_mode);
			le_scan_set_param(GAP_PARAM_SCAN_INTERVAL, sizeof(scan_param->scan_interval), &scan_param->scan_interval);
			le_scan_set_param(GAP_PARAM_SCAN_WINDOW, sizeof(scan_param->scan_window), &scan_param->scan_window);
			le_scan_set_param(GAP_PARAM_SCAN_FILTER_POLICY, sizeof(scan_param->scan_filter_policy),
							  &scan_param->scan_filter_policy);
			le_scan_set_param(GAP_PARAM_SCAN_FILTER_DUPLICATES, sizeof(scan_param->scan_filter_duplicate),
							  &scan_param->scan_filter_duplicate);

			cause = le_scan_start();
			if (cause != GAP_CAUSE_SUCCESS) {
				printf("[%s]call le_scan_start fail ret %d\r\n", __func__, cause);
			}

			os_mem_free(scan_param);
		} else {
			printf("[%s] scan_param is NULL\r\n", __func__);
		}
	}
	break;
	case BMS_API_MSG_STOP_SCAN:
		cause = le_scan_stop();
		if (cause != GAP_CAUSE_SUCCESS) {
			printf("[%s]call le_scan_stop fail ret %d\r\n", __func__, cause);
		}
		break;
	case BMS_API_MSG_INDICAT_SEND: {
		BMS_INDICATION_PARAM *param = io_msg.u.buf;
		int ret = 0;
		if (param) {
			ret = ble_ms_adapter_send_indication_notification(param->conn_id, param->srv_id, param->attrib_index, (uint8_t *)param->val, param->len, param->type);
			if (ret != true) {
				printf("[%s]call server_send_data fail \r\n", __func__);
			}

			os_mem_free(param->val);
			os_mem_free(param);
		} else {
			printf("[%s] param is NULL\r\n", __func__);
		}
	}
	break;
	case BMS_API_MSG_ADD_SERVICE: {
		BMS_SERVICE_INFO *srv_info = io_msg.u.buf;
		ble_ms_adapter_add_service(srv_info, ble_ms_adapter_app_profile_callback);
	}
	break;
	case BMS_API_MSG_DISC_ALL_SRV: {
		uint8_t *conn_id = (uint8_t *)io_msg.u.buf;
		if (conn_id) {
			cause = gcs_all_primary_srv_discovery(*conn_id);
			if (cause != GAP_CAUSE_SUCCESS) {
				printf("[%s]call gcs_all_primary_srv_discovery fail ret %d\r\n", __func__, cause);
			}
			os_mem_free(conn_id);
		} else {
			printf("[%s] conn_id is NULL\r\n", __func__);
		}

	}
	break;
	case BMS_API_MSG_DISC_SRV_BY_UUID: {
		M_DISC_SRV_UUID_PARAM *disc_srv_uuid = io_msg.u.buf;
		uint16_t uuid16;
		if (disc_srv_uuid) {
			if (disc_srv_uuid->uuid_size == 2) {
				uuid16 = (disc_srv_uuid->uuid[1] << 8) | disc_srv_uuid->uuid[0];
				cause = gcs_by_uuid_srv_discovery(disc_srv_uuid->conn_id, uuid16);
				if (cause != GAP_CAUSE_SUCCESS) {
					printf("[%s]call gcs_by_uuid_srv_discovery fail ret %d\r\n", __func__, cause);
				}

			} else if (disc_srv_uuid->uuid_size == 16) {
				cause = gcs_by_uuid128_srv_discovery(disc_srv_uuid->conn_id, disc_srv_uuid->uuid);
				if (cause != GAP_CAUSE_SUCCESS) {
					printf("[%s]call gcs_by_uuid128_srv_discovery fail ret %d\r\n", __func__, cause);
				}

			}
			os_mem_free(disc_srv_uuid->uuid);
			os_mem_free(disc_srv_uuid);
		} else {
			printf("[%s] disc_srv_uuid is NULL\r\n", __func__);
		}
	}
	break;
	case BMS_API_MSG_DISC_ALL_CHAR: {
		uint8_t *conn_id = (uint8_t *)io_msg.u.buf;
		uint16_t start_handle = 0x0001;
		uint16_t end_handle = 0xFFFF;
		if (conn_id) {
			cause = gcs_all_char_discovery(*conn_id, start_handle, end_handle);
			if (cause != GAP_CAUSE_SUCCESS) {
				printf("[%s]call gcs_all_char_discovery fail ret %d\r\n", __func__, cause);
			}

			os_mem_free(conn_id);
		} else {
			printf("[%s] conn_id is NULL\r\n", __func__);
		}

	}
	break;
	case BMS_API_MSG_DISC_CHAR_BY_UUID: {
		M_DISC_CHAR_UUID_PARAM *disc_char_uuid = io_msg.u.buf;
		uint8_t conn_id = disc_char_uuid->conn_id;
		uint16_t start_handle = disc_char_uuid->start_hdl;
		uint16_t end_handle = disc_char_uuid->end_hdl;
		uint16_t uuid16;

		if (disc_char_uuid) {
			if (disc_char_uuid->uuid_size == 2) {
				uuid16 = (disc_char_uuid->uuid[1] << 8) | disc_char_uuid->uuid[0];
				cause = gcs_by_uuid_char_discovery(conn_id, start_handle, end_handle, uuid16);
				if (cause != GAP_CAUSE_SUCCESS) {
					printf("[%s]call gcs_by_uuid_char_discovery fail ret %d\r\n", __func__, cause);
				}
			} else if (disc_char_uuid->uuid_size == 16) {
				cause = gcs_by_uuid128_char_discovery(conn_id, start_handle, end_handle, disc_char_uuid->uuid);
				if (cause != GAP_CAUSE_SUCCESS) {
					printf("[%s]call gcs_by_uuid128_char_discovery fail ret %d\r\n", __func__, cause);
				}
			}
			os_mem_free(disc_char_uuid->uuid);
			os_mem_free(disc_char_uuid);
		} else {
			printf("[%s] disc_char_uuid is NULL\r\n", __func__);
		}
	}
	break;
	case BMS_API_MSG_DISC_ALL_DESC: {
		M_DISC_DESC_PARAM *disc_desc = io_msg.u.buf;
		uint8_t conn_id = disc_desc->conn_id;
		uint16_t start_handle = disc_desc->start_hdl;
		uint16_t end_handle = disc_desc->end_hdl;

		if (disc_desc) {
			cause = gcs_all_char_descriptor_discovery(conn_id, start_handle, end_handle);
			if (cause != GAP_CAUSE_SUCCESS) {
				printf("[%s]call gcs_all_char_descriptor_discovery fail ret %d\r\n", __func__, cause);
			}
			os_mem_free(disc_desc);
		} else {
			printf("[%s] connection_param is NULL\r\n", __func__);
		}
	}
	break;
	case BMS_API_MSG_CONN_PARAM_REQ: {
		M_CONNECTION_REQ_PARAM *connection_req_param = io_msg.u.buf;
		if (connection_req_param) {
			cause = le_update_conn_param(connection_req_param->conn_id,
										 connection_req_param->conn_interval_min, connection_req_param->conn_interval_max,
										 connection_req_param->slave_latency, connection_req_param->supervision_timeout,
										 connection_req_param->ce_length_min, connection_req_param->ce_length_max
										);
			if (cause != GAP_CAUSE_SUCCESS) {
				printf("[%s]call le_update_conn_param fail ret %d\r\n", __func__, cause);
			}
			os_mem_free(connection_req_param);

		} else {
			printf("[%s] connection_param is NULL\r\n", __func__);
		}
	}
	break;
	case BMS_API_MSG_SET_CCCD:
	case BMS_API_MSG_DISC_WRITE: {
		M_WRITE_PARAM *write_param = io_msg.u.buf;
		if (write_param) {
			cause = gcs_attr_write(write_param->conn_id, write_param->type,
								   write_param->att_handle, write_param->value_size, write_param->value);
			if (cause != GAP_CAUSE_SUCCESS) {
				printf("[%s]call gcs_attr_write fail ret %d\r\n", __func__, cause);
			}

			os_mem_free(write_param->value);
			os_mem_free(write_param);
		} else {
			printf("[%s] write_cccd_param is NULL\r\n", __func__);
		}
	}
	break;
	case BMS_API_MSG_MODIFY_WHITELIST: {
		M_MODIFY_WHITELIST_PARAM *whitelist_add_param = io_msg.u.buf;
		if (whitelist_add_param) {
			cause = le_modify_white_list(whitelist_add_param->operation, whitelist_add_param->remote_bd, whitelist_add_param->remote_bd_type);
			if (cause != GAP_CAUSE_SUCCESS) {
				printf("[%s]call le_modify_white_list fail ret %d\r\n", __func__, cause);
				os_sem_give(modify_whitelist_sem);
			}
			os_mem_free(whitelist_add_param->remote_bd);
			os_mem_free(whitelist_add_param);
		} else {
			printf("[%s] whitelist_add_param is NULL\r\n", __func__);
			os_sem_give(modify_whitelist_sem);
		}
	}
	break;
	case BMS_API_MSG_SET_DEVICE_NAME: {
		M_SET_DEVICE_NAME *device_name_param = io_msg.u.buf;
		if (device_name_param) {
			le_set_gap_param(GAP_PARAM_DEVICE_NAME, device_name_param->device_name_len, (void *)device_name_param->device_name);
			os_mem_free(device_name_param->device_name);
			os_mem_free(device_name_param);
		} else {
			printf("[%s] device_name_param is NULL\r\n", __func__);

		}
	}
	break;
	default:
		break;
	}

}

void ble_ms_adapter_init_scan_info_queue(void)
{
	memset(&scan_info_queue, 0, sizeof(T_OS_QUEUE));
	os_queue_init(&scan_info_queue);
}

void ble_ms_adapter_deinit_scan_info_queue(void)
{
	M_SAVE_SCAN_INFO *sip = NULL;
	while (true) {
		sip = os_queue_out(&scan_info_queue);
		ble_ms_adapter_free_queue_scan_info(sip);
		if (sip == NULL) {
			memset(&scan_info_queue, 0, sizeof(T_OS_QUEUE));
			break;
		}
	}
}

void ble_ms_adapter_free_queue_scan_info(M_SAVE_SCAN_INFO *scan_info)
{
	if (scan_info) {
		if (scan_info->data) {
			os_mem_free(scan_info->data);
		}
		os_mem_free(scan_info);
	}
}

void ble_ms_adapter_free_scan_info(ms_hal_ble_scan_t *scan_info)
{
	if (scan_info) {
		if (scan_info->data) {
			os_mem_free(scan_info->data);
		}
		os_mem_free(scan_info);
	}
}

extern void ble_ms_adapter_search_scan_info_and_inqueue(ms_hal_ble_scan_t *scanned_device_info);
void ble_ms_adapter_app_handle_callback_msg(T_BMS_CALLBACK_MSG callback_msg)
{
	switch (callback_msg.type) {
	case BMS_CALLBACK_MSG_STACK_READY:
	case BMS_CALLBACK_MSG_CMP_WRITE:
	case BMS_CALLBACK_MSG_NON_ADV_ON:
	case BMS_CALLBACK_MSG_ADV_ON:
	case BMS_CALLBACK_MSG_NON_ADV_OFF:
	case BMS_CALLBACK_MSG_ADV_OFF:
	//case BMS_CALLBACK_MSG_SCAN_ON:
	//case BMS_CALLBACK_MSG_SCAN_OFF:
	case BMS_CALLBACK_MSG_CMP_NOTIFY://not use
	case BMS_CALLBACK_MSG_CMP_INDICATE:
	case BMS_CALLBACK_MSG_CMP_SVC_DISC:
	case BMS_CALLBACK_MSG_CMP_CHAR_DISC:
	case BMS_CALLBACK_MSG_CMP_DISC_DESC_CHAR:
	case BMS_CALLBACK_MSG_NOTIFY_REQ: //not use
	case BMS_CALLBACK_MSG_CMP_MTU:
	case BMS_CALLBACK_MSG_DISCONNECTED: {
		ms_hal_ble_stack_msg_t *cb_msg = callback_msg.buf;
		if (cb_msg) {
			if (callback_handler) {
				callback_handler(*cb_msg);
			} else {
				printf("[%s]callback_handler is NULL\r\n", __func__);
			}
			os_mem_free(cb_msg);
		} else {
			printf("[%s] callback_msg is NULL\r\n", __func__);
		}

	}
	break;
	case BMS_CALLBACK_MSG_SCAN_ON: {
		ble_ms_adapter_init_scan_info_queue();
		ms_hal_ble_stack_msg_t *cb_msg = callback_msg.buf;
		if (cb_msg) {
			if (callback_handler) {
				callback_handler(*cb_msg);
			} else {
				printf("[%s]callback_handler is NULL\r\n", __func__);
			}
			os_mem_free(cb_msg);
		} else {
			printf("[%s] callback_msg is NULL\r\n", __func__);
		}
	}
	break;
	case BMS_CALLBACK_MSG_SCAN_OFF: {
		ble_ms_adapter_deinit_scan_info_queue();
		ms_hal_ble_stack_msg_t *cb_msg = callback_msg.buf;
		if (cb_msg) {
			if (callback_handler) {
				callback_handler(*cb_msg);
			} else {
				printf("[%s]callback_handler is NULL\r\n", __func__);
			}
			os_mem_free(cb_msg);
		} else {
			printf("[%s] callback_msg is NULL\r\n", __func__);
		}
	}
	break;
	case BMS_CALLBACK_MSG_INDICATE_REQ: {
		ms_hal_ble_stack_msg_t *indication_msg = callback_msg.buf;
		if (indication_msg) {
			if (callback_handler) {
				callback_handler(*indication_msg);
			} else {
				printf("[%s] callback_handler is NULL\r\n", __func__);
			}
			os_mem_free(indication_msg->param.gatt_msg.data);
			os_mem_free(indication_msg);

		} else {
			printf("[%s] indication_msg is NULL\r\n", __func__);
		}
	}
	break;
	case BMS_CALLBACK_MSG_DISC_SVC_REPORT: {
		ms_hal_ble_stack_msg_t *disc_service_uuid_msg = callback_msg.buf;
		if (disc_service_uuid_msg) {
			if (callback_handler) {
				callback_handler(*disc_service_uuid_msg);
			} else {
				printf("[%s] callback_handler is NULL\r\n", __func__);
			}
			os_mem_free(disc_service_uuid_msg->param.disc_svc_msg.uuid);
			os_mem_free(disc_service_uuid_msg);
		} else {
			printf("[%s] disc_service_uuid_msg is NULL\r\n", __func__);
		}

	}
	break;
	case BMS_CALLBACK_MSG_DISC_CHAR_REPORT: {
		ms_hal_ble_stack_msg_t *disc_char_uuid_msg = callback_msg.buf;
		if (disc_char_uuid_msg) {
			if (callback_handler) {
				callback_handler(*disc_char_uuid_msg);
			} else {
				printf("[%s] callback_handler is NULL\r\n", __func__);
			}
			os_mem_free(disc_char_uuid_msg->param.disc_char_msg.uuid);
			os_mem_free(disc_char_uuid_msg);
		} else {
			printf("[%s] disc_char_uuid_msg is NULL\r\n", __func__);
		}

	}
	break;
	case BMS_CALLBACK_MSG_DISC_DESC_REPORT: {
		ms_hal_ble_stack_msg_t *disc_char_desc_msg = callback_msg.buf;
		if (disc_char_desc_msg) {
			if (callback_handler) {
				callback_handler(*disc_char_desc_msg);
			} else {
				printf("[%s] callback_handler is NULL\r\n", __func__);
			}
			os_mem_free(disc_char_desc_msg->param.disc_desc_msg.uuid);
			os_mem_free(disc_char_desc_msg);
		} else {
			printf("[%s] disc_char_desc_msg is NULL\r\n", __func__);
		}

	}
	break;
	case BMS_CALLBACK_MSG_CONNECTED: {
		ms_hal_ble_stack_msg_t *connected_msg = callback_msg.buf;
		if (connected_msg) {
			if (callback_handler) {
				callback_handler(*connected_msg);
			} else {
				printf("[%s] callback_handler is NULL\r\n", __func__);
			}
			os_mem_free(connected_msg->param.connection_msg.addr);
			os_mem_free(connected_msg);
		} else {
			printf("[%s] connected_msg is NULL\r\n", __func__);
		}

	}
	break;
	case BMS_CALLBACK_MSG_CMP_WRITE_RECIEVED: {
		T_MS_WRITE_DATA *write_req_msg = callback_msg.buf;
		ms_hal_ble_service_write_cb p_wirte_func = write_req_msg->write_cb;
		if (write_req_msg) {
			if (p_wirte_func) {
				p_wirte_func(write_req_msg->write_value, write_req_msg->write_len);
			} else {
				printf("[%s] ms_hal_ble_service_write_cb is NULL\r\n", __func__);
			}
			os_mem_free(write_req_msg);

		} else {
			printf("[%s] write_req_msg is NULL\r\n", __func__);
		}
	}
	break;
	case BMS_CALLBACK_MSG_SEND_SCAN_INFO: {
		ms_hal_ble_scan_t *scanned_device_info = callback_msg.buf;
		if (scanned_device_info) {
			if ((scanned_device_info->type == MS_HAL_BLE_REPORT_TYPE_DIRECT_IND) || (scanned_device_info->type == MS_HAL_BLE_REPORT_TYPE_SCAN_IND) ||
				(scanned_device_info->type == MS_HAL_BLE_REPORT_TYPE_NONCONN_IND)) {
				if (scan_result_callback) {
					scan_result_callback(scanned_device_info);
				} else {
					printf("[%s] scan_result_callback is NULL\r\n", __func__);
				}
			} else {
				ble_ms_adapter_search_scan_info_and_inqueue(scanned_device_info);
			}
			ble_ms_adapter_free_scan_info(scanned_device_info);
		} else {
			printf("[%s] scanned_device_info is NULL\r\n", __func__);
		}
	}
	break;
#if CONFIG_MS_MULTI_ADV
	case BMS_CALLBACK_MSG_CONNECTED_MATTER: {
		matter_blemgr_callback_func(matter_blemgr_callback_data, MATTER_BLEMGR_GAP_CONNECT_CB, callback_msg.buf);
		os_mem_free(callback_msg.buf);
	}
	break;
	case BMS_CALLBACK_MSG_DISCONNECTED_MATTER: {
		matter_blemgr_callback_func(matter_blemgr_callback_data, MATTER_BLEMGR_GAP_DISCONNECT_CB, callback_msg.buf);
		os_mem_free(callback_msg.buf);
	}
	break;
	case BMS_CALLBACK_MSG_CMP_WRITE_RECV_MATTER: {
		T_MATTER_BLEMGR_RX_CHAR_WRITE_CB_ARG * write_matter = callback_msg.buf;
		matter_blemgr_callback_func(matter_blemgr_callback_data, MATTER_BLEMGR_RX_CHAR_WRITE_CB, callback_msg.buf);
		if (write_matter->len) {
			os_mem_free((void *)write_matter->p_value);
		}
		os_mem_free(callback_msg.buf);
	}
	break;
	case BMS_CALLBACK_MSG_CMP_CCCD_RECV_MATTER: {
		matter_blemgr_callback_func(matter_blemgr_callback_data, MATTER_BLEMGR_TX_CHAR_CCCD_WRITE_CB, callback_msg.buf);
		os_mem_free(callback_msg.buf);
	}
	break;
	case BMS_CALLBACK_MSG_CMP_INDICATE_MATTER: {
		matter_blemgr_callback_func(matter_blemgr_callback_data, MATTER_BLEMGR_TX_COMPLETE_CB, callback_msg.buf);
		os_mem_free(callback_msg.buf);
	}
	break;
#endif
	default:
		printf("[%s] unknow type(%d) callback msg\r\n", __func__, callback_msg.type);
		break;
	}


}

void ble_ms_adapter_scan_info_inqueue(ms_hal_ble_scan_t *scanned_device_info)
{
	M_SAVE_SCAN_INFO *m_scan_info = (M_SAVE_SCAN_INFO *)os_mem_alloc(0, sizeof(M_SAVE_SCAN_INFO));
	if (m_scan_info == NULL) {
		printf("[%s]:m_scan_info malloc fail\r\n", __func__);
		return;
	}
	memset(m_scan_info, 0, sizeof(M_SAVE_SCAN_INFO));
	m_scan_info->type = scanned_device_info->type;
	m_scan_info->rssi = scanned_device_info->rssi;
	m_scan_info->tx_pwr = scanned_device_info->tx_pwr;
	m_scan_info->peer_addr.addr_type = scanned_device_info->peer_addr.addr_type;
	memcpy(m_scan_info->peer_addr.addr, scanned_device_info->peer_addr.addr, GAP_BD_ADDR_LEN);
	m_scan_info->len = scanned_device_info->len;
	if (m_scan_info->len > 0) {
		m_scan_info->data = (uint8_t *)os_mem_alloc(0, m_scan_info->len);
		if (m_scan_info->data == NULL) {
			ble_ms_adapter_free_scan_info(m_scan_info);
			printf("[%s]:m_scan_info->data malloc fail\r\n", __func__);
			return;
		}
		memset(m_scan_info->data, 0, m_scan_info->len);
		memcpy(m_scan_info->data, scanned_device_info->data, m_scan_info->len);
	}
	os_queue_in(&scan_info_queue, (void *)m_scan_info);
}

void ble_ms_adapter_search_scan_info_and_inqueue(ms_hal_ble_scan_t *scanned_device_info)
{
	M_SAVE_SCAN_INFO *scan_info_param = (M_SAVE_SCAN_INFO *)scan_info_queue.p_first;
	if (scanned_device_info->type == MS_HAL_BLE_REPORT_TYPE_IND) {
		while (scan_info_param != NULL) {
			if (!memcmp(scanned_device_info->peer_addr.addr, scan_info_param->peer_addr.addr, 6)) {
				scan_info_flag = 1;
				break;
			} else {
				scan_info_param = scan_info_param->p_next;
			}
		}

		if (scan_info_flag == 1) {
			ms_hal_ble_scan_t *new_scanned_device_info_1 = (ms_hal_ble_scan_t *)os_mem_alloc(0, sizeof(ms_hal_ble_scan_t));
			if (new_scanned_device_info_1 == NULL) {
				printf("[%s]new_scanned_device_info_1 malloc fail\r\n", __func__);
				return;
			}
			memset(new_scanned_device_info_1, 0, sizeof(ms_hal_ble_scan_t));
			new_scanned_device_info_1->type = scan_info_param->type;
			new_scanned_device_info_1->rssi = scan_info_param->rssi;
			new_scanned_device_info_1->tx_pwr = scan_info_param->tx_pwr;
			new_scanned_device_info_1->peer_addr.addr_type = scan_info_param->peer_addr.addr_type;
			memcpy(new_scanned_device_info_1->peer_addr.addr, scan_info_param->peer_addr.addr, GAP_BD_ADDR_LEN);
			new_scanned_device_info_1->len = scan_info_param->len;
			if (new_scanned_device_info_1->len > 0) {
				new_scanned_device_info_1->data = (uint8_t *)os_mem_alloc(0, new_scanned_device_info_1->len);
				if (new_scanned_device_info_1->data == NULL) {
					ble_ms_adapter_free_scan_info(new_scanned_device_info_1);
					printf("[%s]new_scanned_device_info_1->data malloc fail\r\n", __func__);
					return;
				}
				memset(new_scanned_device_info_1->data, 0, new_scanned_device_info_1->len);
				memcpy(new_scanned_device_info_1->data, scan_info_param->data, new_scanned_device_info_1->len);
			}

			if (scan_result_callback) {
				scan_result_callback(new_scanned_device_info_1);
			} else {
				printf("[%s] scan_result_callback is NULL\r\n", __func__);
			}
			os_queue_delete(&scan_info_queue, scan_info_param);
			//free old adv
			ble_ms_adapter_free_queue_scan_info(scan_info_param);
			ble_ms_adapter_free_scan_info(new_scanned_device_info_1);
			scan_info_flag = 0;
		}
		ble_ms_adapter_scan_info_inqueue(scanned_device_info);

	} else if (scanned_device_info->type == MS_HAL_BLE_REPORT_TYPE_SCAN_RSP) {
		while (scan_info_param != NULL) {
			if (!memcmp(scanned_device_info->peer_addr.addr, scan_info_param->peer_addr.addr, 6)) {
				scan_info_flag = 2;
				break;
			} else {
				scan_info_param = scan_info_param->p_next;
			}
		}

		if (scan_info_flag == 2) {
			ms_hal_ble_scan_t *new_scanned_device_info_2 = (ms_hal_ble_scan_t *)os_mem_alloc(0, sizeof(ms_hal_ble_scan_t));
			if (new_scanned_device_info_2 == NULL) {
				printf("[%s]new_scanned_device_info_2 malloc fail\r\n", __func__);
				return;
			}
			memset(new_scanned_device_info_2, 0, sizeof(ms_hal_ble_scan_t));
			new_scanned_device_info_2->type = scanned_device_info->type;
			new_scanned_device_info_2->rssi = scanned_device_info->rssi;
			new_scanned_device_info_2->tx_pwr = scanned_device_info->tx_pwr;
			new_scanned_device_info_2->peer_addr.addr_type = scanned_device_info->peer_addr.addr_type;
			memcpy(new_scanned_device_info_2->peer_addr.addr, scanned_device_info->peer_addr.addr, GAP_BD_ADDR_LEN);
			new_scanned_device_info_2->len = scanned_device_info->len + scan_info_param->len;
			if (new_scanned_device_info_2->len > 0) {
				new_scanned_device_info_2->data = (uint8_t *)os_mem_alloc(0, new_scanned_device_info_2->len);
				if (new_scanned_device_info_2->data == NULL) {
					ble_ms_adapter_free_scan_info(new_scanned_device_info_2);
					printf("[%s]new_scanned_device_info_2->data malloc fail\r\n", __func__);
					return;
				}
				memset(new_scanned_device_info_2->data, 0, new_scanned_device_info_2->len);
				memcpy(new_scanned_device_info_2->data, scan_info_param->data, scan_info_param->len);
				memcpy(new_scanned_device_info_2->data + scan_info_param->len, scanned_device_info->data, scanned_device_info->len);
			}
			if (scan_result_callback) {
				scan_result_callback(new_scanned_device_info_2);
			} else {
				printf("[%s] scan_result_callback is NULL\r\n", __func__);
			}
			os_queue_delete(&scan_info_queue, scan_info_param);
			ble_ms_adapter_free_queue_scan_info(scan_info_param);
			ble_ms_adapter_free_scan_info(new_scanned_device_info_2);
			scan_info_flag = 0;
		}
	}
}

/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void ble_ms_adapter_app_handle_io_msg(T_IO_MSG io_msg)
{
	uint16_t msg_type = io_msg.type;

	switch (msg_type) {
	case IO_MSG_TYPE_BT_STATUS: {
		ble_ms_adapter_app_handle_gap_msg(&io_msg);
	}
	break;
	case IO_MSG_TYPE_QDECODE: {
		ble_ms_adapter_app_handle_api_msg(io_msg);
	}
	break;
	default:
		break;
	}
}


/**
 * @brief    Handle msg GAP_MSG_LE_DEV_STATE_CHANGE
 * @note     All the gap device state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] new_state  New gap device state
 * @param[in] cause GAP device state change cause
 * @return   void
 */
void ble_ms_adapter_app_handle_dev_state_evt(T_GAP_DEV_STATE new_state, uint16_t cause)
{
	int ret = 1;
	APP_PRINT_INFO3("ble_ms_adapter_dev_state_evt: init state  %d, scan state %d, cause 0x%x",
					new_state.gap_init_state,
					new_state.gap_scan_state, cause);
	if (ble_ms_adapter_gap_dev_state.gap_init_state != new_state.gap_init_state) {
		if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY) {
			uint8_t bt_addr[6];
			APP_PRINT_INFO0("GAP stack ready");
			/*stack ready*/
			gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);
			printf("local bd addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
				   bt_addr[5],
				   bt_addr[4],
				   bt_addr[3],
				   bt_addr[2],
				   bt_addr[1],
				   bt_addr[0]);
#if CONFIG_MS_MULTI_ADV
			memcpy(ms_local_public_addr, bt_addr, 6);
#endif
			//todo
			ms_hal_ble_stack_msg_t *stack_ready_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
			memset(stack_ready_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
			stack_ready_msg->event_type = MS_HAL_BLE_STACK_EVENT_STACK_READY;
			if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_STACK_READY, stack_ready_msg) == false) {
				printf("\n\r[%s] send callback msg fail\r\n", __func__);
				os_mem_free(stack_ready_msg);
			}
		}
	}

	if (ble_ms_adapter_gap_dev_state.gap_scan_state != new_state.gap_scan_state) {
		if (new_state.gap_scan_state == GAP_SCAN_STATE_IDLE) {
			APP_PRINT_INFO0("GAP scan stop");
			printf("GAP scan stop\r\n");
			//todo
			ms_hal_ble_stack_msg_t *scan_stop_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
			memset(scan_stop_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
			scan_stop_msg->event_type = MS_HAL_BLE_STACK_EVENT_SCAN_OFF;
			if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_SCAN_OFF, scan_stop_msg) == false) {
				printf("[%s] send callback msg fail\r\n", __func__);
				os_mem_free(scan_stop_msg);
			}
		} else if (new_state.gap_scan_state == GAP_SCAN_STATE_SCANNING) {
			APP_PRINT_INFO0("GAP scan start");
			printf("GAP scan start\r\n");
			//todo
			ms_hal_ble_stack_msg_t *scan_start_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
			memset(scan_start_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
			scan_start_msg->event_type = MS_HAL_BLE_STACK_EVENT_SCAN_ON;
			if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_SCAN_ON, scan_start_msg) == false) {
				printf("[%s] send callback msg fail\r\n", __func__);
				os_mem_free(scan_start_msg);
			}
		}
	}

	if (ble_ms_adapter_gap_dev_state.gap_adv_state != new_state.gap_adv_state) {
		if (new_state.gap_adv_state == GAP_ADV_STATE_IDLE) {
			if (new_state.gap_adv_sub_state == GAP_ADV_TO_IDLE_CAUSE_CONN) {
				APP_PRINT_INFO0("GAP adv stoped: because connection created");
				printf("GAP adv stoped: because connection created\r\n");
			} else {
				APP_PRINT_INFO0("GAP adv stoped");
				printf("GAP adv stopped\r\n");
			}
			if (update_adv != 1) {
				ms_hal_ble_stack_msg_t *adv_off_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
				memset(adv_off_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
				if (adv_type == GAP_ADTYPE_ADV_IND) {
					adv_off_msg->event_type = MS_HAL_BLE_STACK_EVENT_ADV_OFF;
				} else if (adv_type == GAP_ADTYPE_ADV_NONCONN_IND) {
					adv_off_msg->event_type = MS_HAL_BLE_STACK_EVENT_NONCONN_ADV_OFF;
				}
				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_ADV_OFF, adv_off_msg) == false) {
					printf("[%s] send callback msg fail\r\n", __func__);
					os_mem_free(adv_off_msg);
				}
			}
		} else if (new_state.gap_adv_state == GAP_ADV_STATE_ADVERTISING) {
			APP_PRINT_INFO0("GAP adv start");
			printf("GAP adv start\r\n");
			if (adv_start_sem) {
				os_sem_give(adv_start_sem);
			}

			if (update_adv != 1) {
				ms_hal_ble_stack_msg_t *adv_on_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
				memset(adv_on_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
				le_adv_get_param(GAP_PARAM_ADV_EVENT_TYPE, &adv_type);
				if (adv_type == GAP_ADTYPE_ADV_IND) {
					adv_on_msg->event_type = MS_HAL_BLE_STACK_EVENT_ADV_ON;
				} else if (adv_type == GAP_ADTYPE_ADV_NONCONN_IND) {
					adv_on_msg->event_type = MS_HAL_BLE_STACK_EVENT_NONCONN_ADV_ON;
				}
				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_ADV_ON, adv_on_msg) == false) {
					printf("[%s] send callback msg fail\r\n", __func__);
					os_mem_free(adv_on_msg);
				}
			}
		}
	}

	ble_ms_adapter_gap_dev_state = new_state;
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_STATE_CHANGE
 * @note     All the gap conn state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] conn_id Connection ID
 * @param[in] new_state  New gap connection state
 * @param[in] disc_cause Use this cause when new_state is GAP_CONN_STATE_DISCONNECTED
 * @return   void
 */
#if CONFIG_MS_MULTI_ADV
void ble_ms_adapter_delete_adv(uint8_t adv_id)
{
	if(ms_multi_adv_param_array[adv_id].one_shot_timer){
		os_timer_delete(&ms_multi_adv_param_array[adv_id].one_shot_timer);
	}
	if(ms_multi_adv_param_array[adv_id].update_adv_mutex) {
		os_mutex_delete(ms_multi_adv_param_array[adv_id].update_adv_mutex);
	}
	memset(&ms_multi_adv_param_array[adv_id], 0, sizeof(M_MULTI_ADV_PARAM));

}

void ble_ms_adapter_stop_all_adv(void)
{
	if (ms_multi_adapter.msmart_sta_sto_flag == false) {
		printf("stop mamsrt adv\r\n");
		ble_ms_adapter_delete_adv(msmart_adv_id);
		ms_multi_adapter.msmart_sta_sto_flag = true;
		msmart_adv_id = MAX_ADV_NUMBER;
		ms_hal_ble_stack_msg_t *adv_off_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
		memset(adv_off_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
		adv_off_msg->event_type = MS_HAL_BLE_STACK_EVENT_ADV_OFF;

		if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_ADV_OFF, adv_off_msg) == false) {
			printf("[%s] send callback msg fail\r\n", __func__);
			os_mem_free(adv_off_msg);
			return MS_HAL_RESULT_ERROR;
		}

	}
	if (ms_multi_adapter.matter_sta_sto_flag == false) {
		printf("stop matter adv\r\n");
		ble_ms_adapter_delete_adv(matter_adv_id);
		ms_multi_adapter.matter_sta_sto_flag = true;
		matter_adv_id = MAX_ADV_NUMBER;
	}
}
#endif
void ble_ms_adapter_app_handle_conn_state_evt(uint8_t conn_id, T_GAP_CONN_STATE new_state, uint16_t disc_cause)
{
	T_GAP_CONN_INFO conn_info;
	int ret = 1;

	if (conn_id >= BLE_MS_ADAPTER_APP_MAX_LINKS) {
		return;
	}

	APP_PRINT_INFO4("ble_ms_adapter_app_handle_conn_state_evt: conn_id %d, conn_state(%d -> %d), disc_cause 0x%x",
					conn_id, ble_ms_adapter_app_link_table[conn_id].conn_state, new_state, disc_cause);

	ble_ms_adapter_app_link_table[conn_id].conn_state = new_state;
	switch (new_state) {
	case GAP_CONN_STATE_DISCONNECTED: {
		if ((disc_cause != (HCI_ERR | HCI_ERR_REMOTE_USER_TERMINATE))
			&& (disc_cause != (HCI_ERR | HCI_ERR_LOCAL_HOST_TERMINATE))) {
			APP_PRINT_ERROR2("ble_ms_adapter_app_handle_conn_state_evt: connection lost, conn_id %d, cause 0x%x", conn_id,
							 disc_cause);
		}

		printf("Disconnect conn_id %d, cause 0x%x\r\n", conn_id, disc_cause);
///judge the type of disconnect is central or peripheral,if peripheral,start ADV
		if (ble_ms_adapter_app_link_table[conn_id].role == GAP_LINK_ROLE_SLAVE) {
			printf("As peripheral,recieve disconncect,please start ADV\r\n");
			//todo
			//start adv
		}

		if (ble_ms_adapter_app_link_table[conn_id].role == GAP_LINK_ROLE_MASTER) {
			ble_ms_adapter_central_app_max_links --;
		} else if (ble_ms_adapter_app_link_table[conn_id].role == GAP_LINK_ROLE_SLAVE) {
			ble_ms_adapter_peripheral_app_max_links --;
		}

		memset(&ble_ms_adapter_app_link_table[conn_id], 0, sizeof(T_APP_LINK));
#if CONFIG_MS_MULTI_ADV
		if (link_customer == 1) { //matter
			T_MATTER_BLEMGR_GAP_DISCONNECT_CB_ARG *disconnected_msg_matter = (T_MATTER_BLEMGR_GAP_DISCONNECT_CB_ARG *)os_mem_alloc(0, sizeof(T_MATTER_BLEMGR_GAP_DISCONNECT_CB_ARG));
			memset(disconnected_msg_matter, 0, sizeof(T_MATTER_BLEMGR_GAP_DISCONNECT_CB_ARG));
			disconnected_msg_matter->conn_id = conn_id;
			disconnected_msg_matter->disc_cause = disc_cause;
			if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_DISCONNECTED_MATTER, disconnected_msg_matter) == false) {
				printf("[%s] send callback msg fail\r\n", __func__);
				os_mem_free(disconnected_msg_matter);
			}
		} else if (link_customer == 2) { //msmart
			ms_hal_ble_stack_msg_t *disconnected_msg_msmart = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
			memset(disconnected_msg_msmart, 0, sizeof(ms_hal_ble_stack_msg_t));
			disconnected_msg_msmart->event_type = MS_HAL_BLE_STACK_EVENT_DISCONNECTED;
			disconnected_msg_msmart->param.disconnect_msg.conn_hdl = conn_id;
			disconnected_msg_msmart->param.disconnect_msg.reason = disc_cause;
			if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_DISCONNECTED, disconnected_msg_msmart) == false) {
				printf("[%s] send callback msg fail\r\n", __func__);
				os_mem_free(disconnected_msg_msmart);
			}
		}
#else
		//send callback msg
		ms_hal_ble_stack_msg_t *disconnected_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
		memset(disconnected_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
		disconnected_msg->event_type = MS_HAL_BLE_STACK_EVENT_DISCONNECTED;
		disconnected_msg->param.disconnect_msg.conn_hdl = conn_id;
		disconnected_msg->param.disconnect_msg.reason = disc_cause;
		if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_DISCONNECTED, disconnected_msg) == false) {
			printf("[%s] send callback msg fail\r\n", __func__);
			os_mem_free(disconnected_msg);
		}
#endif

	}
	break;

	case GAP_CONN_STATE_CONNECTED: {
		le_get_conn_addr(conn_id, ble_ms_adapter_app_link_table[conn_id].bd_addr,
						 (void *)&ble_ms_adapter_app_link_table[conn_id].bd_type);
		//get device role
		if (le_get_conn_info(conn_id, &conn_info)) {
			ble_ms_adapter_app_link_table[conn_id].role = conn_info.role;
			if (ble_ms_adapter_app_link_table[conn_id].role == GAP_LINK_ROLE_MASTER) {
				ble_ms_adapter_central_app_max_links ++;
			} else if (ble_ms_adapter_app_link_table[conn_id].role == GAP_LINK_ROLE_SLAVE) {
				ble_ms_adapter_peripheral_app_max_links ++;
			}
		}
		printf("Connected success conn_id %d\r\n", conn_id);
		uint8_t local_bd_type;
		uint8_t remote_bd_type;
		le_get_conn_param(GAP_PARAM_CONN_LOCAL_BD_TYPE, &local_bd_type, conn_id);
		le_get_conn_param(GAP_PARAM_CONN_BD_ADDR_TYPE, &remote_bd_type, conn_id);
		APP_PRINT_INFO3("GAP_CONN_STATE_CONNECTED: conn_id %d, local_bd_type %d, remote_bd_type %d\n",
						conn_id, local_bd_type, remote_bd_type);
		printf("GAP_CONN_STATE_CONNECTED: conn_id %d, local_bd_type %d, remote_bd_type %d\n",
			   conn_id, local_bd_type, remote_bd_type);
#if F_BT_LE_5_0_SET_PHY_SUPPORT
		{
			uint8_t tx_phy;
			uint8_t rx_phy;
			le_get_conn_param(GAP_PARAM_CONN_RX_PHY_TYPE, &rx_phy, conn_id);
			le_get_conn_param(GAP_PARAM_CONN_TX_PHY_TYPE, &tx_phy, conn_id);
			APP_PRINT_INFO2("GAP_CONN_STATE_CONNECTED: tx_phy %d, rx_phy %d\n", tx_phy, rx_phy);
			printf("GAP_CONN_STATE_CONNECTED: tx_phy %d, rx_phy %d\n", tx_phy, rx_phy);
		}
#endif
		//send callback msg
		uint16_t conn_interval;
		uint16_t conn_latency;
		uint16_t conn_supervision_timeout;
		uint8_t  remote_bd[GAP_BD_ADDR_LEN];
		le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
		le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_latency, conn_id);
		le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
		le_get_conn_addr(conn_id, remote_bd, (void *)&remote_bd_type);
#if CONFIG_MS_MULTI_ADV
		ble_ms_adapter_stop_all_adv();
		//get local adv bt type
		if (local_bd_type == GAP_LOCAL_ADDR_LE_RANDOM) {//matter
			link_customer = 1;
			T_MATTER_BLEMGR_GAP_CONNECT_CB_ARG *conn_msg_matter = (T_MATTER_BLEMGR_GAP_CONNECT_CB_ARG *)os_mem_alloc(0, sizeof(T_MATTER_BLEMGR_GAP_CONNECT_CB_ARG));
			memset(conn_msg_matter, 0, sizeof(T_MATTER_BLEMGR_GAP_CONNECT_CB_ARG));
			conn_msg_matter->conn_id = conn_id;
			if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CONNECTED_MATTER, conn_msg_matter) == false) {
				printf("[%s] send callback msg fail\r\n", __func__);
				os_mem_free(conn_msg_matter);
			}
		} else if (local_bd_type == GAP_LOCAL_ADDR_LE_PUBLIC) { //msmart
			link_customer = 2;
			ms_hal_ble_stack_msg_t *conn_msg_msmart = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
			memset(conn_msg_msmart, 0, sizeof(ms_hal_ble_stack_msg_t));
			conn_msg_msmart->event_type = MS_HAL_BLE_STACK_EVENT_CONNECTION_REPORT;
			conn_msg_msmart->param.connection_msg.conn_hdl = conn_id;
			conn_msg_msmart->param.connection_msg.clk_accuracy = 0; //not support
			conn_msg_msmart->param.connection_msg.con_interval = conn_interval;
			conn_msg_msmart->param.connection_msg.con_latency = conn_latency;
			conn_msg_msmart->param.connection_msg.peer_addr_type = local_bd_type;
			if (conn_info.role == GAP_LINK_ROLE_MASTER) {
				conn_msg_msmart->param.connection_msg.role = 0;
			} else if (conn_info.role == GAP_LINK_ROLE_SLAVE) {
				conn_msg_msmart->param.connection_msg.role = 1;
			} else {
				printf("[%s]:unknow role[%d]\r\n", __func__, conn_info.role);
			}
			conn_msg_msmart->param.connection_msg.sup_timeout = conn_supervision_timeout;
			conn_msg_msmart->param.connection_msg.addr = (uint8_t *)os_mem_alloc(0, GAP_BD_ADDR_LEN);
			memcpy(conn_msg_msmart->param.connection_msg.addr, remote_bd, GAP_BD_ADDR_LEN);
			ble_ms_adapter_switch_bt_address(conn_msg_msmart->param.connection_msg.addr);
			if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CONNECTED, conn_msg_msmart) == false) {
				printf("[%s] send callback msg fail\r\n", __func__);
				os_mem_free(conn_msg_msmart->param.connection_msg.addr);
				os_mem_free(conn_msg_msmart);
			}
		}
#else
		ms_hal_ble_stack_msg_t *connected_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
		memset(connected_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
		connected_msg->event_type = MS_HAL_BLE_STACK_EVENT_CONNECTION_REPORT;
		connected_msg->param.connection_msg.conn_hdl = conn_id;
		connected_msg->param.connection_msg.clk_accuracy = 0; //not support
		connected_msg->param.connection_msg.con_interval = conn_interval;
		connected_msg->param.connection_msg.con_latency = conn_latency;
		connected_msg->param.connection_msg.peer_addr_type = local_bd_type;
		if (conn_info.role == GAP_LINK_ROLE_MASTER) {
			connected_msg->param.connection_msg.role = 0;
		} else if (conn_info.role == GAP_LINK_ROLE_SLAVE) {
			connected_msg->param.connection_msg.role = 1;
		} else {
			printf("[%s]:unknow role[%d]\r\n", __func__, conn_info.role);
		}
		connected_msg->param.connection_msg.sup_timeout = conn_supervision_timeout;
		connected_msg->param.connection_msg.addr = (uint8_t *)os_mem_alloc(0, GAP_BD_ADDR_LEN);
		memcpy(connected_msg->param.connection_msg.addr, remote_bd, GAP_BD_ADDR_LEN);
		ble_ms_adapter_switch_bt_address(connected_msg->param.connection_msg.addr);
		if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CONNECTED, connected_msg) == false) {
			printf("[%s] send callback msg fail\r\n", __func__);
			os_mem_free(connected_msg->param.connection_msg.addr);
			os_mem_free(connected_msg);
		}
#endif
	}
	break;

	default:
		break;

	}

}

/**
 * @brief    Handle msg GAP_MSG_LE_AUTHEN_STATE_CHANGE
 * @note     All the gap authentication state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] conn_id Connection ID
 * @param[in] new_state  New authentication state
 * @param[in] cause Use this cause when new_state is GAP_AUTHEN_STATE_COMPLETE
 * @return   void
 */
void ble_ms_adapter_app_handle_authen_state_evt(uint8_t conn_id, uint8_t new_state, uint16_t cause)
{
	APP_PRINT_INFO2("ble_ms_adapter_app_handle_authen_state_evt:conn_id %d, cause 0x%x", conn_id, cause);

	switch (new_state) {
	case GAP_AUTHEN_STATE_STARTED: {
		APP_PRINT_INFO0("ble_ms_adapter_app_handle_authen_state_evt: GAP_AUTHEN_STATE_STARTED");
	}
	break;

	case GAP_AUTHEN_STATE_COMPLETE: {
		if (cause == GAP_SUCCESS) {
			printf("Pair success\r\n");
			APP_PRINT_INFO0("ble_ms_adapter_app_handle_authen_state_evt: GAP_AUTHEN_STATE_COMPLETE pair success");
		} else {
			printf("Pair failed: cause 0x%x\r\n", cause);
			APP_PRINT_INFO0("ble_ms_adapter_app_handle_authen_state_evt: GAP_AUTHEN_STATE_COMPLETE pair failed");
		}
	}
	break;

	default: {
		APP_PRINT_ERROR1("ble_ms_adapter_app_handle_authen_state_evt: unknown newstate %d", new_state);
	}
	break;
	}
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_MTU_INFO
 * @note     This msg is used to inform APP that exchange mtu procedure is completed.
 * @param[in] conn_id Connection ID
 * @param[in] mtu_size  New mtu size
 * @return   void
 */
void ble_ms_adapter_app_handle_conn_mtu_info_evt(uint8_t conn_id, uint16_t mtu_size)
{
	APP_PRINT_INFO2("ble_ms_adapter_app_handle_conn_mtu_info_evt: conn_id %d, mtu_size %d", conn_id, mtu_size);
	//send callback msg
	ms_hal_ble_stack_msg_t *mtu_exchange_msg1 = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
	memset(mtu_exchange_msg1, 0, sizeof(ms_hal_ble_stack_msg_t));
	mtu_exchange_msg1->event_type = MS_HAL_BLE_STACK_EVENT_MTU_CHANGED;
	mtu_exchange_msg1->param.mtu_changed_msg.conn_hdl = conn_id;
	mtu_exchange_msg1->param.mtu_changed_msg.mtu = mtu_size;
	if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CMP_MTU, mtu_exchange_msg1) == false) {
		printf("[%s] send callback fail\r\n", __func__);
		os_mem_free(mtu_exchange_msg1);
	}


	ms_hal_ble_stack_msg_t *mtu_exchange_msg2 = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
	memset(mtu_exchange_msg2, 0, sizeof(ms_hal_ble_stack_msg_t));
	mtu_exchange_msg2->event_type = MS_HAL_BLE_STACK_EVENT_CMP_MTU;
	mtu_exchange_msg2->param.mtu_changed_msg.conn_hdl = conn_id;
	mtu_exchange_msg2->param.mtu_changed_msg.mtu = mtu_size;
	if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CMP_MTU, mtu_exchange_msg2) == false) {
		printf("[%s] send callback fail\r\n", __func__);
		os_mem_free(mtu_exchange_msg2);
	}
	//todo
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_PARAM_UPDATE
 * @note     All the connection parameter update change  events are pre-handled in this function.
 * @param[in] conn_id Connection ID
 * @param[in] status  New update state
 * @param[in] cause Use this cause when status is GAP_CONN_PARAM_UPDATE_STATUS_FAIL
 * @return   void
 */
void ble_ms_adapter_app_handle_conn_param_update_evt(uint8_t conn_id, uint8_t status, uint16_t cause)
{
	switch (status) {
	case GAP_CONN_PARAM_UPDATE_STATUS_SUCCESS: {
		uint16_t conn_interval;
		uint16_t conn_slave_latency;
		uint16_t conn_supervision_timeout;

		le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
		le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_slave_latency, conn_id);
		le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
		APP_PRINT_INFO4("ble_ms_adapter_app_handle_conn_param_update_evt update success:conn_id %d, conn_interval 0x%x, conn_slave_latency 0x%x, conn_supervision_timeout 0x%x",
						conn_id, conn_interval, conn_slave_latency, conn_supervision_timeout);
		printf("\n\rble_ms_adapter_app_handle_conn_param_update_evt update success:conn_id %d, conn_interval 0x%x, conn_slave_latency 0x%x, conn_supervision_timeout 0x%x \r\n",
			   conn_id, conn_interval, conn_slave_latency, conn_supervision_timeout);
	}
	break;

	case GAP_CONN_PARAM_UPDATE_STATUS_FAIL: {
		APP_PRINT_ERROR2("ble_ms_adapter_app_handle_conn_param_update_evt update failed: conn_id %d, cause 0x%x",
						 conn_id, cause);
		printf("\n\rble_ms_adapter_app_handle_conn_param_update_evt update failed: conn_id %d, cause 0x%x\r\n",
			   conn_id, cause);

	}
	break;

	case GAP_CONN_PARAM_UPDATE_STATUS_PENDING: {
		APP_PRINT_INFO1("ble_ms_adapter_app_handle_conn_param_update_evt update pending: conn_id %d", conn_id);
		printf("\n\rble_ms_adapter_app_handle_conn_param_update_evt update pending: conn_id %d\r\n", conn_id);

	}
	break;

	default:
		break;
	}
}

/**
 * @brief    All the BT GAP MSG are pre-handled in this function.
 * @note     Then the event handling function shall be called according to the
 *           subtype of T_IO_MSG
 * @param[in] p_gap_msg Pointer to GAP msg
 * @return   void
 */
void ble_ms_adapter_app_handle_gap_msg(T_IO_MSG *p_gap_msg)
{
	T_LE_GAP_MSG gap_msg;
	uint8_t conn_id;
	memcpy(&gap_msg, &p_gap_msg->u.param, sizeof(p_gap_msg->u.param));

	APP_PRINT_TRACE1("ble_ms_adapter_app_handle_gap_msg: subtype %d", p_gap_msg->subtype);
	switch (p_gap_msg->subtype) {
	case GAP_MSG_LE_DEV_STATE_CHANGE: {
		ble_ms_adapter_app_handle_dev_state_evt(gap_msg.msg_data.gap_dev_state_change.new_state,
												gap_msg.msg_data.gap_dev_state_change.cause);
	}
	break;

	case GAP_MSG_LE_CONN_STATE_CHANGE: {
		ble_ms_adapter_app_handle_conn_state_evt(gap_msg.msg_data.gap_conn_state_change.conn_id,
				(T_GAP_CONN_STATE)gap_msg.msg_data.gap_conn_state_change.new_state,
				gap_msg.msg_data.gap_conn_state_change.disc_cause);
	}
	break;

	case GAP_MSG_LE_CONN_MTU_INFO: {
		ble_ms_adapter_app_handle_conn_mtu_info_evt(gap_msg.msg_data.gap_conn_mtu_info.conn_id,
				gap_msg.msg_data.gap_conn_mtu_info.mtu_size);
	}
	break;

	case GAP_MSG_LE_CONN_PARAM_UPDATE: {
		ble_ms_adapter_app_handle_conn_param_update_evt(gap_msg.msg_data.gap_conn_param_update.conn_id,
				gap_msg.msg_data.gap_conn_param_update.status,
				gap_msg.msg_data.gap_conn_param_update.cause);
	}
	break;

	case GAP_MSG_LE_AUTHEN_STATE_CHANGE: {
		ble_ms_adapter_app_handle_authen_state_evt(gap_msg.msg_data.gap_authen_state.conn_id,
				gap_msg.msg_data.gap_authen_state.new_state,
				gap_msg.msg_data.gap_authen_state.status);
	}
	break;

	case GAP_MSG_LE_BOND_JUST_WORK: {
		conn_id = gap_msg.msg_data.gap_bond_just_work_conf.conn_id;
		le_bond_just_work_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
		APP_PRINT_INFO0("GAP_MSG_LE_BOND_JUST_WORK");
	}
	break;

	case GAP_MSG_LE_BOND_PASSKEY_DISPLAY: {
		uint32_t display_value = 0;
		conn_id = gap_msg.msg_data.gap_bond_passkey_display.conn_id;
		le_bond_get_display_key(conn_id, &display_value);
		APP_PRINT_INFO2("GAP_MSG_LE_BOND_PASSKEY_DISPLAY: conn_id %d, passkey %d",
						conn_id, display_value);
		le_bond_passkey_display_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
		printf("GAP_MSG_LE_BOND_PASSKEY_DISPLAY: conn_id %d, passkey %d\r\n",
			   conn_id,
			   display_value);
	}
	break;

	case GAP_MSG_LE_BOND_USER_CONFIRMATION: {
		uint32_t display_value = 0;
		conn_id = gap_msg.msg_data.gap_bond_user_conf.conn_id;
		le_bond_get_display_key(conn_id, &display_value);
		APP_PRINT_INFO2("GAP_MSG_LE_BOND_USER_CONFIRMATION: conn_id %d, passkey %d",
						conn_id, display_value);
		printf("GAP_MSG_LE_BOND_USER_CONFIRMATION: conn_id %d, passkey %d\r\n",
			   conn_id,
			   display_value);
		//le_bond_user_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
	}
	break;

	case GAP_MSG_LE_BOND_PASSKEY_INPUT: {
		//uint32_t passkey = 888888;
		conn_id = gap_msg.msg_data.gap_bond_passkey_input.conn_id;
		APP_PRINT_INFO1("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d", conn_id);
		printf("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d\r\n", conn_id);
		//le_bond_passkey_input_confirm(conn_id, passkey, GAP_CFM_CAUSE_ACCEPT);
	}
	break;
#if F_BT_LE_SMP_OOB_SUPPORT
	case GAP_MSG_LE_BOND_OOB_INPUT: {
		uint8_t oob_data[GAP_OOB_LEN] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		conn_id = gap_msg.msg_data.gap_bond_oob_input.conn_id;
		APP_PRINT_INFO1("GAP_MSG_LE_BOND_OOB_INPUT: conn_id %d", conn_id);
		le_bond_set_param(GAP_PARAM_BOND_OOB_DATA, GAP_OOB_LEN, oob_data);
		le_bond_oob_input_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
	}
	break;
#endif
	default:
		APP_PRINT_ERROR1("ble_ms_adapter_app_handle_gap_msg: unknown subtype %d", p_gap_msg->subtype);
		break;
	}
}

/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT ble_ms_adapter_app_gap_callback(uint8_t cb_type, void *p_cb_data)
{
	T_APP_RESULT result = APP_RESULT_SUCCESS;
	T_LE_CB_DATA *p_data = (T_LE_CB_DATA *)p_cb_data;
	char adv_type[20];
	char remote_addr_type[10];

	switch (cb_type) {
	case GAP_MSG_LE_SCAN_INFO:
		APP_PRINT_INFO5("GAP_MSG_LE_SCAN_INFO:adv_type 0x%x, bd_addr %s, remote_addr_type %d, rssi %d, data_len %d",
						p_data->p_le_scan_info->adv_type,
						TRACE_BDADDR(p_data->p_le_scan_info->bd_addr),
						p_data->p_le_scan_info->remote_addr_type,
						p_data->p_le_scan_info->rssi,
						p_data->p_le_scan_info->data_len);
		ms_hal_ble_scan_t *scanned_device_info = (ms_hal_ble_scan_t *)os_mem_alloc(0, sizeof(ms_hal_ble_scan_t));

		if (scanned_device_info) {
			memset(scanned_device_info, 0, sizeof(ms_hal_ble_scan_t));
			if (p_data->p_le_scan_info->adv_type == GAP_ADV_EVT_TYPE_UNDIRECTED) {
				scanned_device_info->type = MS_HAL_BLE_REPORT_TYPE_IND;
			} else if (p_data->p_le_scan_info->adv_type == GAP_ADV_EVT_TYPE_DIRECTED) {
				scanned_device_info->type = MS_HAL_BLE_REPORT_TYPE_DIRECT_IND;
			} else if (p_data->p_le_scan_info->adv_type == GAP_ADV_EVT_TYPE_SCANNABLE) {
				scanned_device_info->type = MS_HAL_BLE_REPORT_TYPE_SCAN_IND;
			} else if (p_data->p_le_scan_info->adv_type == GAP_ADV_EVT_TYPE_NON_CONNECTABLE) {
				scanned_device_info->type = MS_HAL_BLE_REPORT_TYPE_NONCONN_IND;
			} else if (p_data->p_le_scan_info->adv_type == GAP_ADV_EVT_TYPE_SCAN_RSP) {
				scanned_device_info->type = MS_HAL_BLE_REPORT_TYPE_SCAN_RSP;
			}
			scanned_device_info->rssi = p_data->p_le_scan_info->rssi;
			scanned_device_info->tx_pwr = 0;//not support
			scanned_device_info->peer_addr.addr_type = p_data->p_le_scan_info->remote_addr_type;
			memcpy(scanned_device_info->peer_addr.addr, p_data->p_le_scan_info->bd_addr, GAP_BD_ADDR_LEN);
			ble_ms_adapter_switch_bt_address(scanned_device_info->peer_addr.addr);
			scanned_device_info->len = p_data->p_le_scan_info->data_len;
			if (scanned_device_info->len > 0) {
				scanned_device_info->data = (uint8_t *)os_mem_alloc(0, p_data->p_le_scan_info->data_len);
				if (scanned_device_info->data == NULL) {
					ble_ms_adapter_free_scan_info(scanned_device_info);
					printf("[%s]scanned_device_info->data malloc fail\r\n", __func__);
					return 0;
				}
				memset(scanned_device_info->data, 0, scanned_device_info->len);
				memcpy(scanned_device_info->data, p_data->p_le_scan_info->data, scanned_device_info->len);
			}

			if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_SEND_SCAN_INFO, scanned_device_info) == false) {
				ble_ms_adapter_free_scan_info(scanned_device_info);
				printf("[%s] send callback msg fail\r\n", __func__);
			}

		}
		break;

	case GAP_MSG_LE_CONN_UPDATE_IND:
		APP_PRINT_INFO5("GAP_MSG_LE_CONN_UPDATE_IND: conn_id %d, conn_interval_max 0x%x, conn_interval_min 0x%x, conn_latency 0x%x,supervision_timeout 0x%x",
						p_data->p_le_conn_update_ind->conn_id,
						p_data->p_le_conn_update_ind->conn_interval_max,
						p_data->p_le_conn_update_ind->conn_interval_min,
						p_data->p_le_conn_update_ind->conn_latency,
						p_data->p_le_conn_update_ind->supervision_timeout);
		printf("GAP_MSG_LE_CONN_UPDATE_IND: conn_id %d, conn_interval_max 0x%x, conn_interval_min 0x%x, conn_latency 0x%x,supervision_timeout 0x%x\n\r",
			   p_data->p_le_conn_update_ind->conn_id,
			   p_data->p_le_conn_update_ind->conn_interval_max,
			   p_data->p_le_conn_update_ind->conn_interval_min,
			   p_data->p_le_conn_update_ind->conn_latency,
			   p_data->p_le_conn_update_ind->supervision_timeout);
		//todo
		//need to send callback to upper layer
		ms_hal_ble_stack_msg_t *conn_param_req_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
		conn_param_req_msg->param.conn_param_msg.conn_hdl = p_data->p_le_conn_update_ind->conn_id;
		conn_param_req_msg->param.conn_param_msg.conn_intv_max = p_data->p_le_conn_update_ind->conn_interval_max;
		conn_param_req_msg->param.conn_param_msg.conn_intv_min = p_data->p_le_conn_update_ind->conn_interval_min;
		conn_param_req_msg->param.conn_param_msg.slave_latency = p_data->p_le_conn_update_ind->conn_latency;
		conn_param_req_msg->param.conn_param_msg.timeout = p_data->p_le_conn_update_ind->supervision_timeout;
		conn_param_req_msg->event_type = MS_HAL_BLE_STACK_EVENT_CONNECT_PARAM_UPDATE_REQ;
		if (callback_handler) {
			callback_handler(*conn_param_req_msg);
		} else {
			printf("[%s] callback_handler is NULL\r\n", __func__);
		}
		os_mem_free(conn_param_req_msg);
		if (M_accept == true) {
			result = APP_RESULT_ACCEPT;
		} else {
			result = APP_RESULT_REJECT;
		}
		break;
#if F_BT_LE_5_0_SET_PHY_SUPPORT
	case GAP_MSG_LE_PHY_UPDATE_INFO:
		APP_PRINT_INFO4("GAP_MSG_LE_PHY_UPDATE_INFO:conn_id %d, cause 0x%x, rx_phy %d, tx_phy %d",
						p_data->p_le_phy_update_info->conn_id,
						p_data->p_le_phy_update_info->cause,
						p_data->p_le_phy_update_info->rx_phy,
						p_data->p_le_phy_update_info->tx_phy);
		printf("GAP_MSG_LE_PHY_UPDATE_INFO:conn_id %d, cause 0x%x, rx_phy %d, tx_phy %d\n\r",
			   p_data->p_le_phy_update_info->conn_id,
			   p_data->p_le_phy_update_info->cause,
			   p_data->p_le_phy_update_info->rx_phy,
			   p_data->p_le_phy_update_info->tx_phy);
		break;

	case GAP_MSG_LE_REMOTE_FEATS_INFO: {
		uint8_t  remote_feats[8];
		APP_PRINT_INFO3("GAP_MSG_LE_REMOTE_FEATS_INFO: conn id %d, cause 0x%x, remote_feats %b",
						p_data->p_le_remote_feats_info->conn_id,
						p_data->p_le_remote_feats_info->cause,
						TRACE_BINARY(8, p_data->p_le_remote_feats_info->remote_feats));
		if (p_data->p_le_remote_feats_info->cause == GAP_SUCCESS) {
			memcpy(remote_feats, p_data->p_le_remote_feats_info->remote_feats, 8);
			if (remote_feats[LE_SUPPORT_FEATURES_MASK_ARRAY_INDEX1] & LE_SUPPORT_FEATURES_LE_2M_MASK_BIT) {
				APP_PRINT_INFO0("GAP_MSG_LE_REMOTE_FEATS_INFO: support 2M");
				printf("GAP_MSG_LE_REMOTE_FEATS_INFO: support 2M\n\r");
			}
			if (remote_feats[LE_SUPPORT_FEATURES_MASK_ARRAY_INDEX1] & LE_SUPPORT_FEATURES_LE_CODED_PHY_MASK_BIT) {
				APP_PRINT_INFO0("GAP_MSG_LE_REMOTE_FEATS_INFO: support CODED");
				printf("GAP_MSG_LE_REMOTE_FEATS_INFO: support CODED\n\r");
			}
		}
	}
	break;
#endif

	case GAP_MSG_LE_MODIFY_WHITE_LIST:
		APP_PRINT_INFO2("GAP_MSG_LE_MODIFY_WHITE_LIST: operation  0x%x, cause 0x%x",
						p_data->p_le_modify_white_list_rsp->operation,
						p_data->p_le_modify_white_list_rsp->cause);
		printf("GAP_MSG_LE_MODIFY_WHITE_LIST: operation  0x%x, cause 0x%x\r\n",
			   p_data->p_le_modify_white_list_rsp->operation,
			   p_data->p_le_modify_white_list_rsp->cause);
		modify_whitelist_result = p_data->p_le_modify_white_list_rsp->cause;
		os_sem_give(modify_whitelist_sem);
		break;
#if CONFIG_MS_MULTI_ADV
	case GAP_MSG_LE_ADV_UPDATE_PARAM:
		APP_PRINT_INFO1("GAP_MSG_LE_ADV_UPDATE_PARAM: cause 0x%x",
					  p_data->p_le_adv_update_param_rsp->cause);
		//printf("GAP_MSG_LE_ADV_UPDATE_PARAM: cause 0x%x\r\n",
					 // p_data->p_le_adv_update_param_rsp->cause);
		if (p_data->p_le_adv_update_param_rsp->cause == 0) {
			uint8_t adv_stop_flag = 0;
			adv_stop_flag = ble_ms_adapter_judge_adv_stop(ms_multi_adapter.adv_id);
			if (adv_stop_flag) {
				printf("[%s]stop adv flag[%d] is set, give sem and break\r\n", __func__, ms_multi_adapter.adv_id);
				ms_multi_adapter.adv_id = MAX_ADV_NUMBER;
				if(ms_multi_adapter.sem_handle)
					os_sem_give(ms_multi_adapter.sem_handle);
				break;
			}
#if BT_VENDOR_CMD_ONE_SHOT_SUPPORT
			T_GAP_CAUSE ret = GAP_CAUSE_SUCCESS;
			ret = le_vendor_one_shot_adv();
			if (ret != GAP_CAUSE_SUCCESS) {
				printf("le_vendor_one_shot_adv fail! ret = 0x%x\r\n", ret);
			}
#endif
		} else
			printf("GAP_MSG_LE_ADV_UPDATE_PARAM: cause 0x%x\r\n", p_data->p_le_adv_update_param_rsp->cause);
	break;
#endif
	default:
		APP_PRINT_ERROR1("ble_ms_adapter_app_gap_callback: unhandled cb_type 0x%x", cb_type);
		break;
	}
	return result;
}
/** @} */ /* End of group CENTRAL_CLIENT_GAP_CALLBACK */
/** @defgroup  GCS_CLIIENT_CALLBACK GCS Client Callback Event Handler
    * @brief Handle profile client callback event
    * @{
    */
void ble_ms_adapter_gcs_handle_discovery_result(uint8_t conn_id, T_GCS_DISCOVERY_RESULT discov_result)
{
	uint16_t i;
	T_GCS_DISCOV_RESULT *p_result_table;
	uint16_t    properties;
	switch (discov_result.discov_type) {
	case GCS_ALL_PRIMARY_SRV_DISCOV:
		APP_PRINT_INFO2("conn_id %d, GCS_ALL_PRIMARY_SRV_DISCOV, is_success %d",
						conn_id, discov_result.is_success);
		for (i = 0; i < discov_result.result_num; i++) {
			p_result_table = &(discov_result.p_result_table[i]);
			switch (p_result_table->result_type) {
			case DISC_RESULT_ALL_SRV_UUID16:
				APP_PRINT_INFO4("ALL SRV UUID16[%d]: service range: 0x%x-0x%x, uuid16 0x%x",
								i, p_result_table->result_data.srv_uuid16_disc_data.att_handle,
								p_result_table->result_data.srv_uuid16_disc_data.end_group_handle,
								p_result_table->result_data.srv_uuid16_disc_data.uuid16);
				printf("ALL SRV UUID16[%d]: service range: 0x%x-0x%x, uuid16 0x%x\r\n",
					   i, p_result_table->result_data.srv_uuid16_disc_data.att_handle,
					   p_result_table->result_data.srv_uuid16_disc_data.end_group_handle,
					   p_result_table->result_data.srv_uuid16_disc_data.uuid16);

				ms_hal_ble_stack_msg_t *disc_service_uuid16_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
				memset(disc_service_uuid16_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
				disc_service_uuid16_msg->event_type = MS_HAL_BLE_STACK_EVENT_DISC_SVC_REPORT;
				disc_service_uuid16_msg->param.disc_svc_msg.uuid = (uint8_t *)os_mem_alloc(0, 2);
				disc_service_uuid16_msg->param.disc_svc_msg.conn_hdl = conn_id;
				disc_service_uuid16_msg->param.disc_svc_msg.start_hdl = p_result_table->result_data.srv_uuid16_disc_data.att_handle;
				disc_service_uuid16_msg->param.disc_svc_msg.end_hdl = p_result_table->result_data.srv_uuid16_disc_data.end_group_handle;
				disc_service_uuid16_msg->param.disc_svc_msg.uuid_len = 2;
				disc_service_uuid16_msg->param.disc_svc_msg.uuid[0] = p_result_table->result_data.srv_uuid16_disc_data.uuid16 & 0xFF;
				disc_service_uuid16_msg->param.disc_svc_msg.uuid[1] = (p_result_table->result_data.srv_uuid16_disc_data.uuid16 >> 8) & 0xFF;
				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_DISC_SVC_REPORT, disc_service_uuid16_msg) == false) {
					printf("[%s] send callback msg fail\r\n", __func__);
					os_mem_free(disc_service_uuid16_msg->param.disc_svc_msg.uuid);
					os_mem_free(disc_service_uuid16_msg);
				}
				break;
			case DISC_RESULT_ALL_SRV_UUID128:
				APP_PRINT_INFO4("ALL SRV UUID128[%d]: service range: 0x%x-0x%x, service=<%b>",
								i, p_result_table->result_data.srv_uuid128_disc_data.att_handle,
								p_result_table->result_data.srv_uuid128_disc_data.end_group_handle,
								TRACE_BINARY(16, p_result_table->result_data.srv_uuid128_disc_data.uuid128));
				printf("ALL SRV UUID128[%d]: service range: 0x%x-0x%x, service="UUID_128_FORMAT"\n\r",
					   i, p_result_table->result_data.srv_uuid128_disc_data.att_handle,
					   p_result_table->result_data.srv_uuid128_disc_data.end_group_handle,
					   UUID_128(p_result_table->result_data.srv_uuid128_disc_data.uuid128));
				ms_hal_ble_stack_msg_t *disc_service_uuid128_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
				memset(disc_service_uuid128_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
				disc_service_uuid128_msg->event_type = MS_HAL_BLE_STACK_EVENT_DISC_SVC_REPORT;
				disc_service_uuid128_msg->param.disc_svc_msg.uuid = (uint8_t *)os_mem_alloc(0, 16);
				disc_service_uuid128_msg->param.disc_svc_msg.conn_hdl = conn_id;
				disc_service_uuid128_msg->param.disc_svc_msg.start_hdl = p_result_table->result_data.srv_uuid128_disc_data.att_handle;
				disc_service_uuid128_msg->param.disc_svc_msg.end_hdl = p_result_table->result_data.srv_uuid128_disc_data.end_group_handle;
				disc_service_uuid128_msg->param.disc_svc_msg.uuid_len = 16;
				memcpy(disc_service_uuid128_msg->param.disc_svc_msg.uuid, p_result_table->result_data.srv_uuid128_disc_data.uuid128, 16);
				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_DISC_SVC_REPORT, disc_service_uuid128_msg) == false) {
					printf("[%s] send callback msg fail\r\n", __func__);
					os_mem_free(disc_service_uuid128_msg->param.disc_svc_msg.uuid);
					os_mem_free(disc_service_uuid128_msg);
				}
				break;

			default:
				APP_PRINT_ERROR0("Invalid Discovery Result Type!");
				printf("Invalid Discovery Result Type!\n\r");
				break;
			}
		}
		ms_hal_ble_stack_msg_t *disc_service_done_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
		memset(disc_service_done_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
		disc_service_done_msg->event_type = MS_HAL_BLE_STACK_EVENT_CMP_SVC_DISC;
		if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CMP_SVC_DISC, disc_service_done_msg) == false) {
			printf("[%s] send callback msg fail\r\n", __func__);
			os_mem_free(disc_service_done_msg);
		}
		break;

	case GCS_BY_UUID128_SRV_DISCOV:
		APP_PRINT_INFO2("conn_id %d, GCS_BY_UUID128_SRV_DISCOV, is_success %d",
						conn_id, discov_result.is_success);
		printf("conn_id %d, GCS_BY_UUID128_SRV_DISCOV, is_success %d\n\r",
			   conn_id, discov_result.is_success);

		for (i = 0; i < discov_result.result_num; i++) {
			p_result_table = &(discov_result.p_result_table[i]);
			switch (p_result_table->result_type) {
			case DISC_RESULT_SRV_DATA:
				APP_PRINT_INFO3("SRV DATA[%d]: service range: 0x%x-0x%x",
								i, p_result_table->result_data.srv_disc_data.att_handle,
								p_result_table->result_data.srv_disc_data.end_group_handle);
				printf("SRV DATA[%d]: service range: 0x%x-0x%x\n\r",
					   i, p_result_table->result_data.srv_disc_data.att_handle,
					   p_result_table->result_data.srv_disc_data.end_group_handle);

				break;

			default:
				APP_PRINT_ERROR0("Invalid Discovery Result Type!");
				printf("Invalid Discovery Result Type!\n\r");
				break;
			}
		}
		break;

	case GCS_BY_UUID_SRV_DISCOV:
		APP_PRINT_INFO2("conn_id %d, GCS_BY_UUID_SRV_DISCOV, is_success %d",
						conn_id, discov_result.is_success);
		printf("conn_id %d, GCS_BY_UUID_SRV_DISCOV, is_success %d\n\r",
			   conn_id, discov_result.is_success);

		for (i = 0; i < discov_result.result_num; i++) {
			p_result_table = &(discov_result.p_result_table[i]);
			switch (p_result_table->result_type) {
			case DISC_RESULT_SRV_DATA:
				APP_PRINT_INFO3("SRV DATA[%d]: service range: 0x%x-0x%x",
								i, p_result_table->result_data.srv_disc_data.att_handle,
								p_result_table->result_data.srv_disc_data.end_group_handle);
				printf("SRV DATA[%d]: service range: 0x%x-0x%x\n\r",
					   i, p_result_table->result_data.srv_disc_data.att_handle,
					   p_result_table->result_data.srv_disc_data.end_group_handle);

				break;

			default:
				APP_PRINT_ERROR0("Invalid Discovery Result Type!");
				printf("Invalid Discovery Result Type!\n\r");
				break;
			}
		}
		break;

	case GCS_ALL_CHAR_DISCOV:
		APP_PRINT_INFO2("conn_id %d, GCS_ALL_CHAR_DISCOV, is_success %d",
						conn_id, discov_result.is_success);
		printf("conn_id %d, GCS_ALL_CHAR_DISCOV, is_success %d\n\r",
			   conn_id, discov_result.is_success);

		for (i = 0; i < discov_result.result_num; i++) {
			p_result_table = &(discov_result.p_result_table[i]);
			switch (p_result_table->result_type) {
			case DISC_RESULT_CHAR_UUID16:
				properties = p_result_table->result_data.char_uuid16_disc_data.properties;
				APP_PRINT_INFO5("CHAR UUID16[%d]: decl_handle 0x%x, properties 0x%x, value_handle 0x%x, uuid16 0x%x",
								i, p_result_table->result_data.char_uuid16_disc_data.decl_handle,
								p_result_table->result_data.char_uuid16_disc_data.properties,
								p_result_table->result_data.char_uuid16_disc_data.value_handle,
								p_result_table->result_data.char_uuid16_disc_data.uuid16);
				APP_PRINT_INFO5("properties:indicate %d, read %d, write cmd %d, write %d, notify %d\r\n",
								properties & GATT_CHAR_PROP_INDICATE,
								properties & GATT_CHAR_PROP_READ,
								properties & GATT_CHAR_PROP_WRITE_NO_RSP,
								properties & GATT_CHAR_PROP_WRITE,
								properties & GATT_CHAR_PROP_NOTIFY);
				printf("CHAR UUID16[%d]: decl_handle 0x%x, properties 0x%x, value_handle 0x%x, uuid16 0x%x\n\r",
					   i, p_result_table->result_data.char_uuid16_disc_data.decl_handle,
					   p_result_table->result_data.char_uuid16_disc_data.properties,
					   p_result_table->result_data.char_uuid16_disc_data.value_handle,
					   p_result_table->result_data.char_uuid16_disc_data.uuid16);
				printf("properties:indicate %d, read %d, write cmd %d, write %d, notify %d\n\r",
					   properties & GATT_CHAR_PROP_INDICATE,
					   properties & GATT_CHAR_PROP_READ,
					   properties & GATT_CHAR_PROP_WRITE_NO_RSP,
					   properties & GATT_CHAR_PROP_WRITE,
					   properties & GATT_CHAR_PROP_NOTIFY);
				ms_hal_ble_stack_msg_t *disc_char_uuid16_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
				memset(disc_char_uuid16_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
				disc_char_uuid16_msg->event_type = MS_HAL_BLE_STACK_EVENT_DISC_CHAR_REPORT;
				disc_char_uuid16_msg->param.disc_char_msg.uuid = (uint8_t *)os_mem_alloc(0, 2);
				disc_char_uuid16_msg->param.disc_char_msg.conn_hdl = conn_id;
				disc_char_uuid16_msg->param.disc_char_msg.att_hdl = p_result_table->result_data.char_uuid16_disc_data.decl_handle;
				disc_char_uuid16_msg->param.disc_char_msg.pointer_hdl = p_result_table->result_data.char_uuid16_disc_data.value_handle;
				disc_char_uuid16_msg->param.disc_char_msg.prop = p_result_table->result_data.char_uuid16_disc_data.properties;
				disc_char_uuid16_msg->param.disc_char_msg.uuid_len = 2;
				disc_char_uuid16_msg->param.disc_char_msg.uuid[0] = p_result_table->result_data.char_uuid16_disc_data.uuid16 & 0xFF;
				disc_char_uuid16_msg->param.disc_char_msg.uuid[1] = (p_result_table->result_data.char_uuid16_disc_data.uuid16 >> 8) & 0xFF;
				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_DISC_CHAR_REPORT, disc_char_uuid16_msg) == false) {
					printf("[%s] send callback msg fail\r\n", __func__);
					os_mem_free(disc_char_uuid16_msg->param.disc_char_msg.uuid);
					os_mem_free(disc_char_uuid16_msg);
				}
				break;

			case DISC_RESULT_CHAR_UUID128:
				properties = p_result_table->result_data.char_uuid128_disc_data.properties;
				APP_PRINT_INFO5("CHAR UUID128[%d]:  decl hndl=0x%x, prop=0x%x, value hndl=0x%x, uuid128=<%b>",
								i, p_result_table->result_data.char_uuid128_disc_data.decl_handle,
								p_result_table->result_data.char_uuid128_disc_data.properties,
								p_result_table->result_data.char_uuid128_disc_data.value_handle,
								TRACE_BINARY(16, p_result_table->result_data.char_uuid128_disc_data.uuid128));
				APP_PRINT_INFO5("properties:indicate %d, read %d, write cmd %d, write %d, notify %d",
								properties & GATT_CHAR_PROP_INDICATE,
								properties & GATT_CHAR_PROP_READ,
								properties & GATT_CHAR_PROP_WRITE_NO_RSP,
								properties & GATT_CHAR_PROP_WRITE,
								properties & GATT_CHAR_PROP_NOTIFY
							   );
				printf("CHAR UUID128[%d]:  decl hndl=0x%x, prop=0x%x, value hndl=0x%x, uuid128="UUID_128_FORMAT"\n\r",
					   i, p_result_table->result_data.char_uuid128_disc_data.decl_handle,
					   p_result_table->result_data.char_uuid128_disc_data.properties,
					   p_result_table->result_data.char_uuid128_disc_data.value_handle,
					   UUID_128(p_result_table->result_data.char_uuid128_disc_data.uuid128));
				printf("properties:indicate %d, read %d, write cmd %d, write %d, notify %d\n\r",
					   properties & GATT_CHAR_PROP_INDICATE,
					   properties & GATT_CHAR_PROP_READ,
					   properties & GATT_CHAR_PROP_WRITE_NO_RSP,
					   properties & GATT_CHAR_PROP_WRITE,
					   properties & GATT_CHAR_PROP_NOTIFY
					  );
				ms_hal_ble_stack_msg_t *disc_char_uuid128_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
				memset(disc_char_uuid128_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
				disc_char_uuid128_msg->event_type = MS_HAL_BLE_STACK_EVENT_DISC_CHAR_REPORT;
				disc_char_uuid128_msg->param.disc_char_msg.uuid = (uint8_t *)os_mem_alloc(0, 16);
				disc_char_uuid128_msg->param.disc_char_msg.conn_hdl = conn_id;
				disc_char_uuid128_msg->param.disc_char_msg.att_hdl = p_result_table->result_data.char_uuid128_disc_data.decl_handle;
				disc_char_uuid128_msg->param.disc_char_msg.pointer_hdl = p_result_table->result_data.char_uuid128_disc_data.value_handle;
				disc_char_uuid128_msg->param.disc_char_msg.prop = p_result_table->result_data.char_uuid128_disc_data.properties;
				disc_char_uuid128_msg->param.disc_char_msg.uuid_len = 16;
				memcpy(disc_char_uuid128_msg->param.disc_char_msg.uuid, p_result_table->result_data.char_uuid128_disc_data.uuid128, 16);
				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_DISC_CHAR_REPORT, disc_char_uuid128_msg) == false) {
					printf("[%s] send callback msg fail\r\n", __func__);
					os_mem_free(disc_char_uuid128_msg->param.disc_char_msg.uuid);
					os_mem_free(disc_char_uuid128_msg);
				}
				break;
			default:
				APP_PRINT_ERROR0("Invalid Discovery Result Type!");
				printf("Invalid Discovery Result Type!\n\r");
				break;
			}
		}

		ms_hal_ble_stack_msg_t *disc_all_char_done_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
		memset(disc_all_char_done_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
		disc_all_char_done_msg->event_type = MS_HAL_BLE_STACK_EVENT_CMP_CHAR_DISC;
		if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CMP_CHAR_DISC, disc_all_char_done_msg) == false) {
			printf("[%s] send callback msg fail\r\n", __func__);
			os_mem_free(disc_all_char_done_msg);
		}
		break;

	case GCS_BY_UUID_CHAR_DISCOV:
		APP_PRINT_INFO2("conn_id %d, GCS_BY_UUID_CHAR_DISCOV, is_success %d",
						conn_id, discov_result.is_success);
		printf("conn_id %d, GCS_BY_UUID_CHAR_DISCOV, is_success %d\n\r",
			   conn_id, discov_result.is_success);

		for (i = 0; i < discov_result.result_num; i++) {
			p_result_table = &(discov_result.p_result_table[i]);
			switch (p_result_table->result_type) {
			case DISC_RESULT_BY_UUID16_CHAR:
				properties = p_result_table->result_data.char_uuid16_disc_data.properties;
				APP_PRINT_INFO5("UUID16 CHAR[%d]: Characteristics by uuid16, decl hndl=0x%x, prop=0x%x, value hndl=0x%x, uuid16=<0x%x>",
								i, p_result_table->result_data.char_uuid16_disc_data.decl_handle,
								p_result_table->result_data.char_uuid16_disc_data.properties,
								p_result_table->result_data.char_uuid16_disc_data.value_handle,
								p_result_table->result_data.char_uuid16_disc_data.uuid16);
				APP_PRINT_INFO5("properties:indicate %d, read %d, write cmd %d, write %d, notify %d",
								properties & GATT_CHAR_PROP_INDICATE,
								properties & GATT_CHAR_PROP_READ,
								properties & GATT_CHAR_PROP_WRITE_NO_RSP,
								properties & GATT_CHAR_PROP_WRITE,
								properties & GATT_CHAR_PROP_NOTIFY
							   );
				printf("UUID16 CHAR[%d]: Characteristics by uuid16, decl hndl=0x%x, prop=0x%x, value hndl=0x%x, uuid16=<0x%x>\n\r",
					   i, p_result_table->result_data.char_uuid16_disc_data.decl_handle,
					   p_result_table->result_data.char_uuid16_disc_data.properties,
					   p_result_table->result_data.char_uuid16_disc_data.value_handle,
					   p_result_table->result_data.char_uuid16_disc_data.uuid16);
				printf("properties:indicate %d, read %d, write cmd %d, write %d, notify %d\n\r",
					   properties & GATT_CHAR_PROP_INDICATE,
					   properties & GATT_CHAR_PROP_READ,
					   properties & GATT_CHAR_PROP_WRITE_NO_RSP,
					   properties & GATT_CHAR_PROP_WRITE,
					   properties & GATT_CHAR_PROP_NOTIFY
					  );

				break;

			default:
				APP_PRINT_ERROR0("Invalid Discovery Result Type!");
				printf("Invalid Discovery Result Type!");
				break;
			}
		}
		break;

	case GCS_BY_UUID128_CHAR_DISCOV:
		APP_PRINT_INFO2("conn_id %d, GCS_BY_UUID128_CHAR_DISCOV, is_success %d",
						conn_id, discov_result.is_success);
		printf("conn_id %d, GCS_BY_UUID128_CHAR_DISCOV, is_success %d\n\r",
			   conn_id, discov_result.is_success);

		for (i = 0; i < discov_result.result_num; i++) {
			p_result_table = &(discov_result.p_result_table[i]);
			switch (p_result_table->result_type) {
			case DISC_RESULT_BY_UUID128_CHAR:
				properties = p_result_table->result_data.char_uuid128_disc_data.properties;
				APP_PRINT_INFO5("UUID128 CHAR[%d]: Characteristics by uuid128, decl hndl=0x%x, prop=0x%x, value hndl=0x%x, uuid128=<%b>",
								i, p_result_table->result_data.char_uuid128_disc_data.decl_handle,
								p_result_table->result_data.char_uuid128_disc_data.properties,
								p_result_table->result_data.char_uuid128_disc_data.value_handle,
								TRACE_BINARY(16, p_result_table->result_data.char_uuid128_disc_data.uuid128));
				APP_PRINT_INFO5("properties:indicate %d, read %d, write cmd %d, write %d, notify %d",
								properties & GATT_CHAR_PROP_INDICATE,
								properties & GATT_CHAR_PROP_READ,
								properties & GATT_CHAR_PROP_WRITE_NO_RSP,
								properties & GATT_CHAR_PROP_WRITE,
								properties & GATT_CHAR_PROP_NOTIFY
							   );
				printf("UUID128 CHAR[%d]: Characteristics by uuid128, decl hndl=0x%x, prop=0x%x, value hndl=0x%x, uuid128="UUID_128_FORMAT"\n\r",
					   i, p_result_table->result_data.char_uuid128_disc_data.decl_handle,
					   p_result_table->result_data.char_uuid128_disc_data.properties,
					   p_result_table->result_data.char_uuid128_disc_data.value_handle,
					   UUID_128(p_result_table->result_data.char_uuid128_disc_data.uuid128));
				printf("properties:indicate %d, read %d, write cmd %d, write %d, notify %d\n\r",
					   properties & GATT_CHAR_PROP_INDICATE,
					   properties & GATT_CHAR_PROP_READ,
					   properties & GATT_CHAR_PROP_WRITE_NO_RSP,
					   properties & GATT_CHAR_PROP_WRITE,
					   properties & GATT_CHAR_PROP_NOTIFY
					  );

				break;

			default:
				APP_PRINT_ERROR0("Invalid Discovery Result Type!");
				BLE_PRINT("Invalid Discovery Result Type!\n\r");
				break;
			}
		}
		break;

	case GCS_ALL_CHAR_DESC_DISCOV:
		APP_PRINT_INFO2("conn_id %d, GCS_ALL_CHAR_DESC_DISCOV, is_success %d\r\n",
						conn_id, discov_result.is_success);
		printf("conn_id %d, GCS_ALL_CHAR_DESC_DISCOV, is_success %d\n\r",
			   conn_id, discov_result.is_success);
		for (i = 0; i < discov_result.result_num; i++) {
			p_result_table = &(discov_result.p_result_table[i]);
			switch (p_result_table->result_type) {
			case DISC_RESULT_CHAR_DESC_UUID16:
				APP_PRINT_INFO3("DESC UUID16[%d]: Descriptors handle=0x%x, uuid16=<0x%x>",
								i, p_result_table->result_data.char_desc_uuid16_disc_data.handle,
								p_result_table->result_data.char_desc_uuid16_disc_data.uuid16);
				printf("DESC UUID16[%d]: Descriptors handle=0x%x, uuid16=<0x%x>\n\r",
					   i, p_result_table->result_data.char_desc_uuid16_disc_data.handle,
					   p_result_table->result_data.char_desc_uuid16_disc_data.uuid16);
				ms_hal_ble_stack_msg_t *disc_char_desc_uuid16_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
				memset(disc_char_desc_uuid16_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
				disc_char_desc_uuid16_msg->event_type = MS_HAL_BLE_STACK_EVENT_DISC_DESC_REPORT;
				disc_char_desc_uuid16_msg->param.disc_desc_msg.uuid = (uint8_t *)os_mem_alloc(0, 2);
				disc_char_desc_uuid16_msg->param.disc_desc_msg.conn_hdl = conn_id;
				disc_char_desc_uuid16_msg->param.disc_desc_msg.att_hdl = p_result_table->result_data.char_desc_uuid16_disc_data.handle;
				disc_char_desc_uuid16_msg->param.disc_desc_msg.uuid_len = 2;
				disc_char_desc_uuid16_msg->param.disc_desc_msg.uuid[0] = p_result_table->result_data.char_desc_uuid16_disc_data.uuid16 & 0xFF;
				disc_char_desc_uuid16_msg->param.disc_desc_msg.uuid[1] = (p_result_table->result_data.char_desc_uuid16_disc_data.uuid16 >> 8) & 0xFF;
				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_DISC_DESC_REPORT, disc_char_desc_uuid16_msg) == false) {
					printf("[%s] send callback msg fail\r\n", __func__);
					os_mem_free(disc_char_desc_uuid16_msg->param.disc_desc_msg.uuid);
					os_mem_free(disc_char_desc_uuid16_msg);

				}
				break;
			case DISC_RESULT_CHAR_DESC_UUID128:
				APP_PRINT_INFO3("DESC UUID128[%d]: Descriptors handle=0x%x, uuid128=<%b>",
								i, p_result_table->result_data.char_desc_uuid128_disc_data.handle,
								TRACE_BINARY(16, p_result_table->result_data.char_desc_uuid128_disc_data.uuid128));
				printf("DESC UUID128[%d]: Descriptors handle=0x%x, uuid128="UUID_128_FORMAT"\n\r",
					   i, p_result_table->result_data.char_desc_uuid128_disc_data.handle,
					   UUID_128(p_result_table->result_data.char_desc_uuid128_disc_data.uuid128));
				ms_hal_ble_stack_msg_t *disc_char_desc_uuid128_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
				memset(disc_char_desc_uuid128_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
				disc_char_desc_uuid128_msg->event_type = MS_HAL_BLE_STACK_EVENT_DISC_DESC_REPORT;
				disc_char_desc_uuid128_msg->param.disc_desc_msg.uuid = (uint8_t *)os_mem_alloc(0, 16);
				disc_char_desc_uuid128_msg->param.disc_desc_msg.conn_hdl = conn_id;
				disc_char_desc_uuid128_msg->param.disc_desc_msg.att_hdl = p_result_table->result_data.char_desc_uuid16_disc_data.handle;
				disc_char_desc_uuid128_msg->param.disc_desc_msg.uuid_len = 16;
				memcpy(disc_char_desc_uuid128_msg->param.disc_desc_msg.uuid, p_result_table->result_data.char_desc_uuid128_disc_data.uuid128, 16);
				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_DISC_DESC_REPORT, disc_char_desc_uuid128_msg) == false) {
					printf("[%s] send callback msg fail\r\n", __func__);
					os_mem_free(disc_char_desc_uuid128_msg->param.disc_desc_msg.uuid);
					os_mem_free(disc_char_desc_uuid128_msg);

				}
				break;

			default:
				APP_PRINT_ERROR0("Invalid Discovery Result Type!");
				printf("Invalid Discovery Result Type!\n\r");
				break;
			}
		}
		ms_hal_ble_stack_msg_t *disc_all_char_desc_done_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
		disc_all_char_desc_done_msg->event_type = MS_HAL_BLE_STACK_EVENT_CMP_DISC_DESC_CHAR;
		if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CMP_DISC_DESC_CHAR, disc_all_char_desc_done_msg) == false) {
			printf("[%s] send callback msg fail\r\n", __func__);
			os_mem_free(disc_all_char_desc_done_msg);

		}
		break;

	default:
		APP_PRINT_ERROR2("Invalid disc type: conn_id %d, discov_type %d",
						 conn_id, discov_result.discov_type);
		printf("Invalid disc type: conn_id %d, discov_type %d\n\r",
			   conn_id, discov_result.discov_type);
		break;
	}
}
/**
 * @brief  Callback will be called when data sent from gcs client.
 * @param  client_id the ID distinguish which module sent the data.
 * @param  conn_id connection ID.
 * @param  p_data  pointer to data.
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT ble_ms_adapter_gcs_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data)
{
	T_APP_RESULT  result = APP_RESULT_SUCCESS;
	APP_PRINT_INFO2("ble_ms_adapter_gcs_client_callback: client_id %d, conn_id %d",
					client_id, conn_id);
	if (client_id == ble_ms_adapter_gcs_client_id) {
		T_GCS_CLIENT_CB_DATA *p_gcs_cb_data = (T_GCS_CLIENT_CB_DATA *)p_data;
		switch (p_gcs_cb_data->cb_type) {
		case GCS_CLIENT_CB_TYPE_DISC_RESULT:
			ble_ms_adapter_gcs_handle_discovery_result(conn_id, p_gcs_cb_data->cb_content.discov_result);
			break;
		case GCS_CLIENT_CB_TYPE_READ_RESULT:
			APP_PRINT_INFO3("READ RESULT: cause 0x%x, handle 0x%x, value_len %d",
							p_gcs_cb_data->cb_content.read_result.cause,
							p_gcs_cb_data->cb_content.read_result.handle,
							p_gcs_cb_data->cb_content.read_result.value_size);
			printf("READ RESULT: cause 0x%x, handle 0x%x, value_len %d\n\r",
				   p_gcs_cb_data->cb_content.read_result.cause,
				   p_gcs_cb_data->cb_content.read_result.handle,
				   p_gcs_cb_data->cb_content.read_result.value_size);

			if (p_gcs_cb_data->cb_content.read_result.cause == GAP_SUCCESS) {
				APP_PRINT_INFO1("READ VALUE: %b",
								TRACE_BINARY(p_gcs_cb_data->cb_content.read_result.value_size,
											 p_gcs_cb_data->cb_content.read_result.p_value));
				printf("READ VALUE: ");
				for (int i = 0; i < p_gcs_cb_data->cb_content.read_result.value_size; i++) {
					printf("0x%2x ", *(p_gcs_cb_data->cb_content.read_result.p_value + i));
				}
				printf("\n\r");
			}

			break;
		case GCS_CLIENT_CB_TYPE_WRITE_RESULT:
			APP_PRINT_INFO3("WRITE RESULT: cause 0x%x, handle 0x%x, type %d",
							p_gcs_cb_data->cb_content.write_result.cause,
							p_gcs_cb_data->cb_content.write_result.handle,
							p_gcs_cb_data->cb_content.write_result.type);
			printf("WRITE RESULT: cause 0x%x, handle 0x%x, type %d\n\r",
				   p_gcs_cb_data->cb_content.write_result.cause,
				   p_gcs_cb_data->cb_content.write_result.handle,
				   p_gcs_cb_data->cb_content.write_result.type);
			//send callback msg
			ms_hal_ble_stack_msg_t *write_done_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
			memset(write_done_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
			if (p_gcs_cb_data->cb_content.write_result.type == GATT_WRITE_TYPE_REQ) {
				write_done_msg->event_type = MS_HAL_BLE_STACK_EVENT_CMP_WRITE_REQ;

			} else if (p_gcs_cb_data->cb_content.write_result.type == GATT_WRITE_TYPE_CMD) {
				write_done_msg->event_type = MS_HAL_BLE_STACK_EVENT_CMP_WRITE_CMD;
			}

			if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CMP_WRITE, write_done_msg) == false) {
				printf("[%s] send callback msg fail\r\n", __func__);
				os_mem_free(write_done_msg);
			}
			break;
		case GCS_CLIENT_CB_TYPE_NOTIF_IND:
			if (p_gcs_cb_data->cb_content.notif_ind.notify == false) {
				APP_PRINT_INFO2("INDICATION: handle 0x%x, value_size %d",
								p_gcs_cb_data->cb_content.notif_ind.handle,
								p_gcs_cb_data->cb_content.notif_ind.value_size);
				APP_PRINT_INFO1("INDICATION VALUE: %b",
								TRACE_BINARY(p_gcs_cb_data->cb_content.notif_ind.value_size,
											 p_gcs_cb_data->cb_content.notif_ind.p_value));
				printf("INDICATION: handle 0x%x, value_size %d\r\n",
					   p_gcs_cb_data->cb_content.notif_ind.handle,
					   p_gcs_cb_data->cb_content.notif_ind.value_size);
				printf("INDICATION VALUE: ");
				for (int i = 0; i < p_gcs_cb_data->cb_content.notif_ind.value_size; i++) {
					printf("0x%2x ", *(p_gcs_cb_data->cb_content.notif_ind.p_value + i));
				}
				printf("\n\r");

				ms_hal_ble_stack_msg_t *indication_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
				memset(indication_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
				indication_msg->event_type = MS_HAL_BLE_STACK_EVENT_INDICATE_REQ;
				indication_msg->param.gatt_msg.att_hdl = p_gcs_cb_data->cb_content.notif_ind.handle;
				indication_msg->param.gatt_msg.conn_hdl = conn_id;
				indication_msg->param.gatt_msg.data_len = p_gcs_cb_data->cb_content.notif_ind.value_size;
				indication_msg->param.gatt_msg.data = (uint8_t *)os_mem_alloc(0, indication_msg->param.gatt_msg.data_len);
				memcpy(indication_msg->param.gatt_msg.data, p_gcs_cb_data->cb_content.notif_ind.p_value, indication_msg->param.gatt_msg.data_len);
				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_INDICATE_REQ, indication_msg) == false) {
					printf("[%s] send callback msg fail\r\n", __func__);
					os_mem_free(indication_msg->param.gatt_msg.data);
					os_mem_free(indication_msg);

				}

			} else {
				APP_PRINT_INFO2("NOTIFICATION: handle 0x%x, value_size %d",
								p_gcs_cb_data->cb_content.notif_ind.handle,
								p_gcs_cb_data->cb_content.notif_ind.value_size);
				APP_PRINT_INFO1("NOTIFICATION VALUE: %b",
								TRACE_BINARY(p_gcs_cb_data->cb_content.notif_ind.value_size,
											 p_gcs_cb_data->cb_content.notif_ind.p_value));
				printf("NOTIFICATION: handle 0x%x, value_size %d\r\n",
					   p_gcs_cb_data->cb_content.notif_ind.handle,
					   p_gcs_cb_data->cb_content.notif_ind.value_size);
				printf("NOTIFICATION VALUE: ");
				for (int i = 0; i < p_gcs_cb_data->cb_content.notif_ind.value_size; i++) {
					printf("0x%2x ", *(p_gcs_cb_data->cb_content.notif_ind.p_value + i));
				}
				printf("\n\r");
			}
			break;
		default:
			break;
		}
	}

	return result;
}

/** @} */ /* End of group PERIPH_GAP_CALLBACK */

#if F_BT_GAPS_CHAR_WRITEABLE
/** @defgroup  SCATTERNET_GAPS_WRITE GAP Service Callback Handler
    * @brief Use @ref F_BT_GAPS_CHAR_WRITEABLE to open
    * @{
    */
/**
 * @brief    All the BT GAP service callback events are handled in this function
 * @param[in] service_id  Profile service ID
 * @param[in] p_para      Pointer to callback data
 * @return   Indicates the function call is successful or not
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT ble_ms_adapter_gap_service_callback(T_SERVER_ID service_id, void *p_para)
{
	(void) service_id;
	T_APP_RESULT  result = APP_RESULT_SUCCESS;
	T_GAPS_CALLBACK_DATA *p_gap_data = (T_GAPS_CALLBACK_DATA *)p_para;
	APP_PRINT_INFO2("gap_service_callback: conn_id = %d msg_type = %d\n", p_gap_data->conn_id,
					p_gap_data->msg_type);
	APP_PRINT_INFO2("gap_service_callback: len = 0x%x,opcode = %d\n", p_gap_data->msg_data.len,
					p_gap_data->msg_data.opcode);
	if (p_gap_data->msg_type == SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE) {
		switch (p_gap_data->msg_data.opcode) {
		case GAPS_WRITE_DEVICE_NAME: {
			T_LOCAL_NAME device_name;
			memcpy(device_name.local_name, p_gap_data->msg_data.p_value, p_gap_data->msg_data.len);
			device_name.local_name[p_gap_data->msg_data.len] = 0;
			printf("GAPS_WRITE_DEVICE_NAME:device_name = %s\r\n", device_name.local_name);
			flash_save_local_name(&device_name);
		}
		break;

		case GAPS_WRITE_APPEARANCE: {
			uint16_t appearance_val;
			T_LOCAL_APPEARANCE appearance;
			LE_ARRAY_TO_UINT16(appearance_val, p_gap_data->msg_data.p_value);
			appearance.local_appearance = appearance_val;
			//printf("GAPS_WRITE_APPEARANCE:appearance = %s\r\n",appearance.local_appearance);
			flash_save_local_appearance(&appearance);
		}
		break;
		default:
			APP_PRINT_ERROR1("gap_service_callback: unhandled msg_data.opcode 0x%x", p_gap_data->msg_data.opcode);
			//printf("gap_service_callback: unhandled msg_data.opcode 0x%x\r\n", p_gap_data->msg_data.opcode);
			break;
		}
	}
	return result;
}
#endif

/** @defgroup  PERIPH_SEVER_CALLBACK Profile Server Callback Event Handler
    * @brief Handle profile server callback event
    * @{
    */
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
T_APP_RESULT ble_ms_adapter_app_profile_callback(T_SERVER_ID service_id, void *p_data)
{
	T_APP_RESULT app_result = APP_RESULT_SUCCESS;
	if (service_id == SERVICE_PROFILE_GENERAL_ID) {
		T_SERVER_APP_CB_DATA *p_param = (T_SERVER_APP_CB_DATA *)p_data;
		switch (p_param->eventId) {
		case PROFILE_EVT_SRV_REG_COMPLETE:// srv register result event.
			APP_PRINT_INFO1("PROFILE_EVT_SRV_REG_COMPLETE: result %d",
							p_param->event_data.service_reg_result);
			break;
		case PROFILE_EVT_SRV_REG_AFTER_INIT_COMPLETE:// srv register result event.
			printf("[%s]PROFILE_EVT_SRV_REG_AFTER_INIT_COMPLETE: result %d\r\n", __func__,
				   p_param->event_data.server_reg_after_init_result.result);
			if (p_param->event_data.server_reg_after_init_result.result != GATT_SERVER_SUCCESS) {
				//free service info
				ble_ms_adapter_search_and_free_service(p_param->event_data.server_reg_after_init_result.service_id);
			}
			if (ms_add_service_sem) {
				os_sem_give(ms_add_service_sem);
			}

			break;

		case PROFILE_EVT_SEND_DATA_COMPLETE:
			APP_PRINT_INFO5("PROFILE_EVT_SEND_DATA_COMPLETE: conn_id %d, cause 0x%x, service_id %d, attrib_idx 0x%x, credits %d",
							p_param->event_data.send_data_result.conn_id,
							p_param->event_data.send_data_result.cause,
							p_param->event_data.send_data_result.service_id,
							p_param->event_data.send_data_result.attrib_idx,
							p_param->event_data.send_data_result.credits);
			printf("PROFILE_EVT_SEND_DATA_COMPLETE: conn_id %d, cause 0x%x, service_id %d, attrib_idx 0x%x, credits %d\r\n",
				   p_param->event_data.send_data_result.conn_id,
				   p_param->event_data.send_data_result.cause,
				   p_param->event_data.send_data_result.service_id,
				   p_param->event_data.send_data_result.attrib_idx,
				   p_param->event_data.send_data_result.credits);
			if (p_param->event_data.send_data_result.cause == GAP_SUCCESS) {
				APP_PRINT_INFO0("PROFILE_EVT_SEND_DATA_COMPLETE success");
				printf("PROFILE_EVT_SEND_DATA_COMPLETE success\r\n");
			} else {
				APP_PRINT_ERROR0("PROFILE_EVT_SEND_DATA_COMPLETE failed");
				printf("PROFILE_EVT_SEND_DATA_COMPLETE failed\r\n");
			}
#if CONFIG_MS_MULTI_ADV
			if (p_param->event_data.send_data_result.service_id == ble_matter_adapter_service_id) {
				T_MATTER_BLEMGR_TX_COMPLETE_CB_ARG *indication_complete_msg_matter = (T_MATTER_BLEMGR_TX_COMPLETE_CB_ARG *) os_mem_alloc(0, sizeof(T_MATTER_BLEMGR_TX_COMPLETE_CB_ARG));
				memset(indication_complete_msg_matter, 0, sizeof(T_MATTER_BLEMGR_TX_COMPLETE_CB_ARG));
				indication_complete_msg_matter->conn_id = p_param->event_data.send_data_result.conn_id;
				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CMP_INDICATE_MATTER, indication_complete_msg_matter) == false) {
					printf("\n\r[%s] send callback msg fail\r\n", __func__);
					os_mem_free(indication_complete_msg_matter);
				}
			} else {
				ms_hal_ble_stack_msg_t *indication_complete_msg_msmart = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
				memset(indication_complete_msg_msmart, 0, sizeof(ms_hal_ble_stack_msg_t));
				indication_complete_msg_msmart->event_type = MS_HAL_BLE_STACK_EVENT_CMP_INDICATE;
				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CMP_INDICATE, indication_complete_msg_msmart) == false) {
					printf("\n\r[%s] send callback msg fail\r\n", __func__);
					os_mem_free(indication_complete_msg_msmart);
				}
			}
#else
			ms_hal_ble_stack_msg_t *indication_complete_msg = (ms_hal_ble_stack_msg_t *)os_mem_alloc(0, sizeof(ms_hal_ble_stack_msg_t));
			memset(indication_complete_msg, 0, sizeof(ms_hal_ble_stack_msg_t));
			indication_complete_msg->event_type = MS_HAL_BLE_STACK_EVENT_CMP_INDICATE;
			if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CMP_INDICATE, indication_complete_msg) == false) {
				printf("\n\r[%s] send callback msg fail\r\n", __func__);
				os_mem_free(indication_complete_msg);
			}
#endif

			break;

		default:
			break;
		}
	} else {
		T_MS_ADAPTER_CALLBACK_DATA *p_ms_cb_data = (T_MS_ADAPTER_CALLBACK_DATA *)p_data;
		switch (p_ms_cb_data->msg_type) {
		case SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION: {
#if CONFIG_MS_MULTI_ADV
			if (service_id == ble_matter_adapter_service_id) {
				T_MATTER_BLEMGR_TX_CHAR_CCCD_WRITE_CB_ARG *cccd_write_msg_matter = (T_MATTER_BLEMGR_TX_CHAR_CCCD_WRITE_CB_ARG *)os_mem_alloc(0, sizeof(T_MATTER_BLEMGR_TX_CHAR_CCCD_WRITE_CB_ARG));
				memset(cccd_write_msg_matter, 0, sizeof(T_MATTER_BLEMGR_TX_CHAR_CCCD_WRITE_CB_ARG));
				cccd_write_msg_matter->conn_id = p_ms_cb_data->conn_id;

				if (p_ms_cb_data->msg_data.cccd.ccc_val & GATT_CLIENT_CHAR_CONFIG_NOTIFY) {
					printf("[%s] cccd 0x%x update : notify enable\r\n", __FUNCTION__, p_ms_cb_data->msg_data.cccd.attr_index);
					cccd_write_msg_matter->notificationsEnabled = 1;
				} else {
					printf("[%s] cccd 0x%x update : notify disable\r\n", __FUNCTION__, p_ms_cb_data->msg_data.cccd.attr_index);
					cccd_write_msg_matter->notificationsEnabled = 0;
				}
				if (p_ms_cb_data->msg_data.cccd.ccc_val & GATT_CLIENT_CHAR_CONFIG_INDICATE) {
					printf("[%s] cccd 0x%x update : indicate enable\r\n", __FUNCTION__, p_ms_cb_data->msg_data.cccd.attr_index);
					cccd_write_msg_matter->indicationsEnabled = 1;
				} else {
					printf("[%s] cccd 0x%x update : indicate disable\r\n", __FUNCTION__, p_ms_cb_data->msg_data.cccd.attr_index);
					cccd_write_msg_matter->indicationsEnabled = 0;
				}

				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CMP_CCCD_RECV_MATTER, cccd_write_msg_matter) == false) {
					printf("\n\r[%s] send callback msg fail\r\n", __func__);
					os_mem_free(cccd_write_msg_matter);
				}
			}else {
				if (p_ms_cb_data->msg_data.cccd.ccc_val & GATT_CLIENT_CHAR_CONFIG_NOTIFY) {
				printf("[%s] cccd 0x%x update : notify enable\r\n", __FUNCTION__, p_ms_cb_data->msg_data.cccd.attr_index);
				} else {
					printf("[%s] cccd 0x%x update : notify disable\r\n", __FUNCTION__, p_ms_cb_data->msg_data.cccd.attr_index);
				}
				if (p_ms_cb_data->msg_data.cccd.ccc_val & GATT_CLIENT_CHAR_CONFIG_INDICATE) {
					printf("[%s] cccd 0x%x update : indicate enable\r\n", __FUNCTION__, p_ms_cb_data->msg_data.cccd.attr_index);
				} else {
					printf("[%s] cccd 0x%x update : indicate disable\r\n", __FUNCTION__, p_ms_cb_data->msg_data.cccd.attr_index);
				}
			}
#else
			if (p_ms_cb_data->msg_data.cccd.ccc_val & GATT_CLIENT_CHAR_CONFIG_NOTIFY) {
				printf("[%s] cccd 0x%x update : notify enable\r\n", __FUNCTION__, p_ms_cb_data->msg_data.cccd.attr_index);
			} else {
				printf("[%s] cccd 0x%x update : notify disable\r\n", __FUNCTION__, p_ms_cb_data->msg_data.cccd.attr_index);
			}
			if (p_ms_cb_data->msg_data.cccd.ccc_val & GATT_CLIENT_CHAR_CONFIG_INDICATE) {
				printf("[%s] cccd 0x%x update : indicate enable\r\n", __FUNCTION__, p_ms_cb_data->msg_data.cccd.attr_index);
			} else {
				printf("[%s] cccd 0x%x update : indicate disable\r\n", __FUNCTION__, p_ms_cb_data->msg_data.cccd.attr_index);
			}

#endif	
			}
			break;
		case SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE:
			//todo
			break;
		case SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE:
			printf("\n\r[%s] write type %d len %d \r\n", __func__, p_ms_cb_data->msg_data.write.write_type, p_ms_cb_data->msg_data.write.len);
#if CONFIG_MS_MULTI_ADV
			if (service_id == ble_matter_adapter_service_id) {
				T_MATTER_BLEMGR_RX_CHAR_WRITE_CB_ARG *write_msg_matter = (T_MATTER_BLEMGR_RX_CHAR_WRITE_CB_ARG *)os_mem_alloc(0, sizeof(T_MATTER_BLEMGR_RX_CHAR_WRITE_CB_ARG));
				memset(write_msg_matter, 0, sizeof(T_MATTER_BLEMGR_RX_CHAR_WRITE_CB_ARG));
				write_msg_matter->conn_id = p_ms_cb_data->conn_id;
				write_msg_matter->len = p_ms_cb_data->msg_data.write.len;
				if (write_msg_matter->len) {
					write_msg_matter->p_value = (uint8_t *)os_mem_alloc(0, write_msg_matter->len);
				}
				memcpy(write_msg_matter->p_value, p_ms_cb_data->msg_data.write.p_value, write_msg_matter->len);
				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CMP_WRITE_RECV_MATTER, write_msg_matter) == false) {
					printf("[%s] send callback msg fail\r\n", __func__);
					if (write_msg_matter->len) {
						os_mem_free(write_msg_matter->p_value);
					}
					os_mem_free(write_msg_matter);
				}
			} else {
				if (p_ms_cb_data->msg_data.write.write_cb) {
					T_MS_WRITE_DATA *write_msg_msmart = (T_MS_WRITE_DATA *)os_mem_alloc(0, sizeof(T_MS_WRITE_DATA));
					memset(write_msg_msmart, 0, sizeof(T_MS_WRITE_DATA));
					memcpy(write_msg_msmart->write_value, p_ms_cb_data->msg_data.write.p_value, p_ms_cb_data->msg_data.write.len);
					write_msg_msmart->write_len = p_ms_cb_data->msg_data.write.len;
					write_msg_msmart->write_cb = p_ms_cb_data->msg_data.write.write_cb;
					if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CMP_WRITE_RECIEVED, write_msg_msmart) == false) {
						printf("[%s] send callback msg fail\r\n", __func__);
						os_mem_free(write_msg_msmart);
					}
				} else {
					printf("\r\n[%s] User's write callback is NULL\r\n", __FUNCTION__);
				}
			}		
#else
			if (p_ms_cb_data->msg_data.write.write_cb) {
				T_MS_WRITE_DATA *MSG_DATA = (T_MS_WRITE_DATA *)os_mem_alloc(0, sizeof(T_MS_WRITE_DATA));
				memset(MSG_DATA, 0, sizeof(T_MS_WRITE_DATA));
				memcpy(MSG_DATA->write_value, p_ms_cb_data->msg_data.write.p_value, p_ms_cb_data->msg_data.write.len);
				MSG_DATA->write_len = p_ms_cb_data->msg_data.write.len;
				MSG_DATA->write_cb = p_ms_cb_data->msg_data.write.write_cb;
				if (ble_ms_adapter_app_send_callback_msg(BMS_CALLBACK_MSG_CMP_WRITE_RECIEVED, MSG_DATA) == false) {
					printf("[%s] send callback msg fail\r\n", __func__);
					os_mem_free(MSG_DATA);
				}
			} else {
				printf("\r\n[%s] User's write callback is NULL\r\n", __FUNCTION__);
			}
#endif
			break;

		default:
			break;
		}
	}
	return app_result;
}
#if CONFIG_MS_MULTI_ADV
void ble_ms_adapter_app_vendor_callback(uint8_t cb_type, void *p_cb_data)
{
	T_GAP_VENDOR_CB_DATA cb_data;
	memcpy(&cb_data, p_cb_data, sizeof(T_GAP_VENDOR_CB_DATA));
	APP_PRINT_INFO1("app_vendor_callback: command 0x%x", cb_data.p_gap_vendor_cmd_rsp->command);
	//printf();
	switch (cb_type)
	{
		case GAP_MSG_VENDOR_CMD_RSP:
			switch(cb_data.p_gap_vendor_cmd_rsp->command)
			{
#if BT_VENDOR_CMD_ONE_SHOT_SUPPORT
				case HCI_LE_VENDOR_EXTENSION_FEATURE2:
					//if(cb_data.p_gap_vendor_cmd_rsp->param[0] == HCI_EXT_SUB_ONE_SHOT_ADV)
					{
						APP_PRINT_ERROR1("One shot adv resp: cause 0x%x", cb_data.p_gap_vendor_cmd_rsp->cause);
						if (cb_data.p_gap_vendor_cmd_rsp->cause == 0) {
							if(ms_multi_adapter.sem_handle)
								os_sem_give(ms_multi_adapter.sem_handle);
						} else
							printf("One shot adv resp: cause 0x%x\r\n", cb_data.p_gap_vendor_cmd_rsp->cause);
					}
					break;
#endif
				default:
					break;
			}
			break;

		default:
			break;
	}

	return;
}
#endif
/** @} */ /* End of group PERIPH_SEVER_CALLBACK */
/** @} */ /* End of group PERIPH_APP */
#endif
