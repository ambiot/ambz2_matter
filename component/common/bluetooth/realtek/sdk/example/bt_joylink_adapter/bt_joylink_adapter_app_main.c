/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      main.c
   * @brief     Source file for BLE peripheral project, mainly used for initialize modules
   * @author    jane
   * @date      2017-06-12
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
#include <bt_flags.h>
#include <os_sched.h>
#include <string.h>
#include <trace_app.h>
#include <gap.h>
#include <gap_adv.h>
#include <gap_bond_le.h>
#include <profile_server.h>
#include <gap_msg.h>
#include <bte.h>
#include <gap_config.h>
#include "bt_joylink_adapter_app_task.h"
#include "bt_joylink_adapter_peripheral_app.h"
#include "bt_joylink_adapter_app_flags.h"
#include "platform_stdlib.h"
#include "wifi_constants.h"
#include <wifi_conf.h>
#include "rtk_coex.h"
#include "os_timer.h"
#include <os_sync.h>
#include "joylink_sdk.h"
#include <os_mem.h>


/*============================================================================*
 *                              Constants
 *============================================================================*/
/** @brief  Default minimum advertising interval when device is discoverable (units of 625us, 160=100ms) */
#define DEFAULT_ADVERTISING_INTERVAL_MIN            352 //220ms
/** @brief  Default maximum advertising interval */
#define DEFAULT_ADVERTISING_INTERVAL_MAX            384 //240ms

/** @brief  GAP - scan response data (max size = 31 bytes) */
static const uint8_t scan_rsp_data[] = {
	0x03,                             /* length */
	GAP_ADTYPE_APPEARANCE,            /* type="Appearance" */
	LO_WORD(GAP_GATT_APPEARANCE_UNKNOWN),
	HI_WORD(GAP_GATT_APPEARANCE_UNKNOWN),
};
uint8_t *adv_data = NULL;
uint8_t adv_data_len = 0;

extern T_SERVER_ID joylink_service_id;
T_ATTRIB_APPL *joylink_service_table = NULL;
uint8_t CHAR_NUM = 6;
#if defined(BT_JOYLINK_ADAPTER_AUTO_DEINIT_BT) && BT_JOYLINK_ADAPTER_AUTO_DEINIT_BT
int connect_status;
#endif


/*============================================================================*
 *                              Functions
 *============================================================================*/

void bt_joylink_adapter_app_start_adv()
{
	jl_gap_data_t *gatt_data = (jl_gap_data_t *)os_mem_alloc(0,sizeof(jl_gap_data_t));
	jl_get_gap_config_data(gatt_data);

	uint8_t service_uuid_length = sizeof(gatt_data->service_uuid16);
	uint8_t manufacture_data_length = sizeof(gatt_data->manufacture_data);

	uint8_t local_name[] = {'J','D'};
	uint8_t local_name_lengh = sizeof(local_name);
	adv_data_len = 3
		         + 2 + service_uuid_length
			     + 2 + local_name_lengh
				 + 2 + manufacture_data_length;

	adv_data = (uint8_t *)os_mem_alloc(0,adv_data_len);
	uint8_t offset = 0;
	//Flags
	*(adv_data + offset) = 0x2;
	*(adv_data + offset + 1) = GAP_ADTYPE_FLAGS; /* type="Flags" */;
	*(adv_data + offset + 2) = GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED;
	//service
	offset += 3;
	*(adv_data + offset) = service_uuid_length + 1;
	*(adv_data + offset + 1) = GAP_ADTYPE_16BIT_COMPLETE;
	memcpy(adv_data + offset + 2,gatt_data->service_uuid16,service_uuid_length);
	offset += service_uuid_length + 2;
	//local name
	*(adv_data + offset) = local_name_lengh + 1;
	*(adv_data + offset + 1) = GAP_ADTYPE_LOCAL_NAME_COMPLETE;
	memcpy(adv_data + offset + 2,local_name,local_name_lengh);
	offset += local_name_lengh + 2;
	//manufacture data
	*(adv_data + offset) = manufacture_data_length + 1;
	*(adv_data + offset + 1) = GAP_ADTYPE_MANUFACTURER_SPECIFIC;
	memcpy(adv_data + offset + 2,gatt_data->manufacture_data,manufacture_data_length);
	
	if(gatt_data != NULL)
		os_mem_free(gatt_data);
}
/**
 * @brief  Config bt stack related feature
 *
 * NOTE: This function shall be called before @ref bte_init is invoked.
 * @return void
 */
//extern void gap_config_hci_task_secure_context(uint32_t size);
void bt_joylink_adapter_stack_config_init(void)
{
	gap_config_max_le_link_num(BT_JOYLINK_ADAPTER_APP_MAX_LINKS);
	gap_config_max_le_paired_device(BT_JOYLINK_ADAPTER_APP_MAX_LINKS);
}

/**
  * @brief  Initialize peripheral and gap bond manager related parameters
  * @return void
  */
void bt_joylink_adapter_app_le_gap_init(void)
{
	/* Device name and device appearance */
	uint8_t  device_name[GAP_DEVICE_NAME_LEN] = "JD";
	uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
	uint8_t  slave_init_mtu_req = false;

	/* Advertising parameters */
    uint8_t  adv_evt_type = GAP_ADTYPE_ADV_IND;
    uint8_t  adv_direct_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  adv_direct_addr[GAP_BD_ADDR_LEN] = {0};
    uint8_t  adv_chann_map = GAP_ADVCHAN_ALL;
    uint8_t  adv_filter_policy = GAP_ADV_FILTER_ANY;
    uint16_t adv_int_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint16_t adv_int_max = DEFAULT_ADVERTISING_INTERVAL_MAX;

	/* GAP Bond Manager parameters */
	uint8_t  auth_pair_mode = GAP_PAIRING_MODE_PAIRABLE;
	uint16_t auth_flags = GAP_AUTHEN_BIT_BONDING_FLAG;
	uint8_t  auth_io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
#if F_BT_LE_SMP_OOB_SUPPORT
	uint8_t  auth_oob = false;
#endif
	uint8_t  auth_use_fix_passkey = false;
	uint32_t auth_fix_passkey = 0;
	uint8_t  auth_sec_req_enable = false;
	uint16_t auth_sec_req_flags = GAP_AUTHEN_BIT_BONDING_FLAG;

	/* Set device name and device appearance */
	le_set_gap_param(GAP_PARAM_DEVICE_NAME, GAP_DEVICE_NAME_LEN, device_name);
	le_set_gap_param(GAP_PARAM_APPEARANCE, sizeof(appearance), &appearance);
	le_set_gap_param(GAP_PARAM_SLAVE_INIT_GATT_MTU_REQ, sizeof(slave_init_mtu_req),
					 &slave_init_mtu_req);
	/* Set advertising parameters */
	bt_joylink_adapter_app_start_adv();
    le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_evt_type), &adv_evt_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR_TYPE, sizeof(adv_direct_type), &adv_direct_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR, sizeof(adv_direct_addr), adv_direct_addr);
    le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_chann_map), &adv_chann_map);
    le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_int_min), &adv_int_min);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_int_max), &adv_int_max);
    le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, sizeof(scan_rsp_data), (void *)scan_rsp_data);
	le_adv_set_param(GAP_PARAM_ADV_DATA, adv_data_len, (void *)adv_data);

	/* Setup the GAP Bond Manager */
	gap_set_param(GAP_PARAM_BOND_PAIRING_MODE, sizeof(auth_pair_mode), &auth_pair_mode);
	gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(auth_flags), &auth_flags);
	gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(auth_io_cap), &auth_io_cap);
#if F_BT_LE_SMP_OOB_SUPPORT
	gap_set_param(GAP_PARAM_BOND_OOB_ENABLED, sizeof(auth_oob), &auth_oob);
#endif
	le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY, sizeof(auth_fix_passkey), &auth_fix_passkey);
	le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY_ENABLE, sizeof(auth_use_fix_passkey),
					  &auth_use_fix_passkey);
	le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_ENABLE, sizeof(auth_sec_req_enable), &auth_sec_req_enable);
	le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_REQUIREMENT, sizeof(auth_sec_req_flags),
					  &auth_sec_req_flags);

	/* register gap message callback */
	le_register_app_cb(bt_joylink_adapter_app_gap_callback);
}

/**
 * @brief  Add GATT services and register callbacks
 * @return void
 */
void bt_joylink_adapter_app_le_profile_init(void)
{
	server_init(1);

	bt_joylink_adapter_parse_service_info();
	joylink_service_id = bt_joylink_adapter_add_service((void *)bt_joylink_adapter_app_profile_callback);
	server_register_app_cb(bt_joylink_adapter_app_profile_callback);
}

/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BLE Peripheral APP, thus only one APP task is init here
 * @return   void
 */
void bt_joylink_adapter_task_init(void)
{
	bt_joylink_adapter_app_task_init();
}

/**
 * @brief    Entry of APP code
 * @return   int (To avoid compile warning)
 */
int bt_joylink_adapter_app_main(void)
{
	bt_trace_init();
	bt_joylink_adapter_stack_config_init();
	bte_init();
	le_gap_init(BT_JOYLINK_ADAPTER_APP_MAX_LINKS);
	bt_joylink_adapter_app_le_gap_init();
	bt_joylink_adapter_app_le_profile_init();
	bt_joylink_adapter_task_init();

	return 0;
}

int bt_joylink_adapter_app_init(void)
{
	T_GAP_DEV_STATE new_state;

	/*Wait WIFI init complete*/
	while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
		os_delay(1000);
	}

	//judge BT joylink Adapter is already on
	le_get_gap_param(GAP_PARAM_DEV_STATE, &new_state);
	if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY) {
		printf("[BT Joylink Adapter] BT Stack already on\r\n");
		return 0;
	} else {
		bt_joylink_adapter_app_main();
	}

	bt_coex_init();

	/*Wait BT init complete*/
	do {
		os_delay(100);
		le_get_gap_param(GAP_PARAM_DEV_STATE, &new_state);
	} while (new_state.gap_init_state != GAP_INIT_STATE_STACK_READY);

	return 0;
}

#if defined(BT_JOYLINK_ADAPTER_AUTO_DEINIT_BT) && BT_JOYLINK_ADAPTER_AUTO_DEINIT_BT
void bt_joylink_adapter_deint_BT(void *p_param) 
{
	(void)p_param;
	connect_status = 0;
	bt_joylink_adapter_app_deinit();
	os_task_delete(NULL);
}
#endif
extern bool bt_trace_uninit(void);
extern T_GAP_DEV_STATE bt_joylink_adapter_gap_dev_state;

void bt_joylink_adapter_app_deinit(void)
{
	if(joylink_service_table != NULL){
		for(int i = 0;i < CHAR_NUM; i ++){
			if(joylink_service_table[i].p_value_context != NULL)
				os_mem_free(joylink_service_table[i].p_value_context);
		}
		os_mem_free(joylink_service_table);
		joylink_service_table = NULL;
	}
	if(adv_data != NULL){
		os_mem_free(adv_data);
		adv_data = NULL;
	}
	bt_joylink_adapter_app_task_deinit();
	T_GAP_DEV_STATE state;
	le_get_gap_param(GAP_PARAM_DEV_STATE, &state);
	if (state.gap_init_state != GAP_INIT_STATE_STACK_READY) {
		printf("[BT Joylink Adapter] BT Stack is not running\r\n");
	}
#if F_BT_DEINIT
	else {
		bte_deinit();
		bt_trace_uninit();
		memset(&bt_joylink_adapter_gap_dev_state, 0, sizeof(T_GAP_DEV_STATE));
		printf("[BT Joylink Adapter] BT Stack deinitalized\r\n");
	}
#endif
}
#endif
