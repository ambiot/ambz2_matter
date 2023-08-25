/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2021 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#include <platform_opts_bt.h>
#if (defined(CONFIG_BT_MATTER_ADAPTER) && CONFIG_BT_MATTER_ADAPTER)
#include <platform/platform_stdlib.h>
#include <string.h>
#include <trace_app.h>
#include <gap.h>
#include <gap_adv.h>
#include <gap_bond_le.h>
#include <gap_conn_le.h>
#include <profile_server.h>
#include <gap_msg.h>
#include <gap_config.h>
#include <bt_flags.h>
#include "bt_matter_adapter_app_main.h"
#include "bt_matter_adapter_wifi.h"
#include "bt_matter_adapter_service.h"
#include "bt_matter_adapter_app_flags.h"
#include "bt_matter_adapter_app_task.h"
#include "bt_matter_adapter_peripheral_app.h"
#include <osdep_service.h>
#include "trace_uart.h"
#include "bte.h"
#include "wifi_constants.h"
#include "wifi_conf.h"
#include "lwip_netconf.h"
#include "os_sched.h"
#if CONFIG_BT_MATTER_ADAPTER
extern bool bt_trace_uninit(void);
extern void wifi_btcoex_set_bt_on(void);
extern uint8_t airsync_specific;
#else
#include <os_mem.h>
#endif
/** @defgroup  PERIPH_DEMO_MAIN Peripheral Main
    * @brief Main file to initialize hardware and BT stack and start task scheduling
    * @{
    */

/*============================================================================*
 *                              Constants
 *============================================================================*/
/** @brief  Default minimum advertising interval when device is discoverable (units of 625us, 160=100ms) */
#define DEFAULT_ADVERTISING_INTERVAL_MIN            320
/** @brief  Default Maximum advertising interval */
#define DEFAULT_ADVERTISING_INTERVAL_MAX            400

#if CONFIG_BT_MATTER_ADAPTER
/*============================================================================*
 *                              Variables
 *============================================================================*/

/** @brief  GAP - scan response data (max size = 31 bytes) */
static const uint8_t scan_rsp_data[] =
{
    0x03,                             /* length */
    GAP_ADTYPE_APPEARANCE,            /* type="Appearance" */
    LO_WORD(GAP_GATT_APPEARANCE_UNKNOWN),
    HI_WORD(GAP_GATT_APPEARANCE_UNKNOWN),
};

/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */
static const uint8_t adv_data[] =
{
    /* Flags */
    0x02,             /* length */
    GAP_ADTYPE_FLAGS, /* type="Flags" */
    GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
    /* Service */
    0x03,             /* length */
    GAP_ADTYPE_16BIT_COMPLETE,
    LO_WORD(GATT_UUID_BT_MATTER_ADAPTER_PROFILE),
    HI_WORD(GATT_UUID_BT_MATTER_ADAPTER_PROFILE),
    /* Local name */
    0x0D,             /* length */
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'A', 'm', 'e', 'b', 'a', '_', 'x', 'x', 'y', 'y', 'z', 'z',
};
#endif
/*============================================================================*
 *                              Functions
 *============================================================================*/
/**
 * @brief  Config bt stack related feature
 *
 * NOTE: This function shall be called before @ref bte_init is invoked.
 * @return void
 */
extern void gap_config_hci_task_secure_context(uint32_t size);
void bt_matter_adapter_stack_config_init(void)
{
    gap_config_max_le_link_num(APP_MAX_LINKS);
    gap_config_max_le_paired_device(APP_MAX_LINKS);
    gap_config_hci_task_secure_context (280);
}
#if CONFIG_BT_MATTER_ADAPTER
/**
  * @brief  Initialize peripheral and gap bond manager related parameters
  * @return void
  */
void bt_matter_adapter_app_le_gap_init(void)
{
    /* Device name and device appearance */
    uint8_t  device_name[GAP_DEVICE_NAME_LEN] = "Ameba_xxyyzz";
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
    uint8_t  slave_init_mtu_req = true;

    /* Advertising parameters */
    uint8_t  adv_evt_type = GAP_ADTYPE_ADV_IND;
    uint8_t  adv_direct_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  adv_direct_addr[GAP_BD_ADDR_LEN] = {0};
    uint8_t  adv_chann_map = GAP_ADVCHAN_ALL;
    uint8_t  adv_filter_policy = GAP_ADV_FILTER_ANY;
    uint16_t adv_int_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint16_t adv_int_max = DEFAULT_ADVERTISING_INTERVAL_MIN;

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
    le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_evt_type), &adv_evt_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR_TYPE, sizeof(adv_direct_type), &adv_direct_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR, sizeof(adv_direct_addr), adv_direct_addr);
    le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_chann_map), &adv_chann_map);
    le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_int_min), &adv_int_min);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_int_max), &adv_int_max);
    le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data), (void *)adv_data);
    le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, sizeof(scan_rsp_data), (void *)scan_rsp_data);

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
    le_register_app_cb(bt_matter_adapter_app_gap_callback);
}
#endif
/**
 * @brief  Add GATT services and register callbacks
 * @return void
 */
void bt_matter_adapter_app_le_profile_init(void)
{
    server_init(1);
    bt_matter_adapter_srv_id = bt_matter_adapter_service_add_service((void *)bt_matter_adapter_app_profile_callback);
    server_register_app_cb(bt_matter_adapter_app_profile_callback);
}

/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BLE Peripheral APP, thus only one APP task is init here
 * @return   void
 */
void bt_matter_adapter_task_init(void)
{
	bt_matter_adapter_app_task_init();
}

void bt_matter_adapter_task_deinit(void)
{
	bt_matter_adapter_app_task_deinit();
}

extern T_GAP_DEV_STATE bt_matter_adapter_gap_dev_state;
extern T_GAP_CONN_STATE bt_matter_adapter_gap_conn_state;
#if CONFIG_BT_MESH_DEVICE_MATTER
uint8_t bt_matter_adapter_conn_id;
#endif
#if CONFIG_BT_MATTER_ADAPTER
extern uint8_t bt_matter_adapter_conn_id;
extern void bt_coex_init(void);
extern void bt_matter_adapter_app_set_adv_data(void);
static uint8_t bt_matter_adapter_state = BC_DEV_DISABLED;

uint8_t get_bt_matter_adapter_state(void)
{
	return bt_matter_adapter_state;
}

void set_bt_matter_adapter_state(uint8_t state)
{
	bt_matter_adapter_state = state;
}

int bt_matter_adapter_app_init(void)
{
	int bt_stack_already_on = 0;
	T_GAP_CONN_INFO conn_info;
	T_GAP_DEV_STATE new_state;

	/*Check WIFI init complete*/
	if(RTW_SUCCESS != wifi_is_connected_to_ap())
	{
		if( ! (wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
			BC_printf("WIFI is disabled\n\r");
			return -1;
		}
	}
	else
		return 0;

	set_bt_matter_adapter_state(BC_DEV_INIT); // BT Config on

#if CONFIG_AUTO_RECONNECT
	/* disable auto reconnect */
	wifi_set_autoreconnect(0);
#endif

	wifi_disconnect();
#if CONFIG_LWIP_LAYER
	LwIP_ReleaseIP(WLAN0_IDX);
#endif

	le_get_gap_param(GAP_PARAM_DEV_STATE , &new_state);
	le_get_conn_info(bt_matter_adapter_conn_id, &conn_info);
	bt_matter_adapter_gap_conn_state = conn_info.conn_state;

	if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY) {
		bt_stack_already_on = 1;
		BC_printf("BT Stack already on\n\r");
	}
	else{
		bt_trace_init();
		bt_matter_adapter_stack_config_init();
		bte_init();
		le_gap_init(APP_MAX_LINKS);
		bt_matter_adapter_app_le_profile_init();
	}

	bt_matter_adapter_app_le_gap_init();
	bt_matter_adapter_task_init();

	bt_coex_init();

	/*Wait BT init complete*/
	do {
		os_delay(100);
		le_get_gap_param(GAP_PARAM_DEV_STATE , &new_state);
	} while (new_state.gap_init_state != GAP_INIT_STATE_STACK_READY);

	/*Start BT WIFI coexistence*/
	wifi_btcoex_set_bt_on();

	if (bt_stack_already_on) {
		bt_matter_adapter_app_set_adv_data();
		bt_matter_adapter_send_msg(1); //Start ADV
		set_bt_matter_adapter_state(BC_DEV_IDLE); // BT Config Ready
	}

	return 0;
}

void bt_matter_adapter_app_deinit(void)
{
	T_GAP_DEV_STATE new_state;

	set_bt_matter_adapter_state(BC_DEV_DEINIT);
	bt_matter_adapter_task_deinit();

	le_get_gap_param(GAP_PARAM_DEV_STATE , &new_state);
	if (new_state.gap_init_state != GAP_INIT_STATE_STACK_READY) {
		BC_printf("BT Stack is not running\n\r");
	}
#if F_BT_DEINIT
	else {
		bte_deinit();
		bt_trace_uninit();
		BC_printf("BT Stack deinitalized\n\r");
	}
#endif
	set_bt_matter_adapter_state(BC_DEV_DISABLED); // BT Config off
}
#endif
/*************************************** CHIP API *************************************************/

/** @brief  GAP - scan response data (max size = 31 bytes) */
static const uint8_t scan_rsp_data_chip[] =
{
    0x03,                             /* length */
    GAP_ADTYPE_APPEARANCE,            /* type="Appearance" */
    LO_WORD(GAP_GATT_APPEARANCE_UNKNOWN),
    HI_WORD(GAP_GATT_APPEARANCE_UNKNOWN),
};

/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */
static const uint8_t adv_data_chip[] =
{
    /* Flags */
    0x02,             /* length */
    GAP_ADTYPE_FLAGS, /* type="Flags" */
    GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
    /* Service */
    0x16,             /* length */
    GAP_ADTYPE_16BIT_COMPLETE,
    LO_WORD(0xFFF6),
    HI_WORD(0xFFF6),

    // Local name
    0x07,             // length
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'M', 'A', 'T', 'T', 'E', 'R',
};

/**
  * @brief  Initialize peripheral and gap bond manager related parameters
  * @return void
  */
void bt_matter_adapter_app_le_gap_init_chip(void)
{
    /* Device name and device appearance */
    //uint8_t  device_name[GAP_DEVICE_NAME_LEN] = "Ameba_xxyyzz";
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
    uint8_t  slave_init_mtu_req = true;

    /* Advertising parameters */
    uint8_t  adv_evt_type = GAP_ADTYPE_ADV_IND;
    uint8_t  adv_direct_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  adv_direct_addr[GAP_BD_ADDR_LEN] = {0};
    uint8_t  adv_chann_map = GAP_ADVCHAN_ALL;
    uint8_t  adv_filter_policy = GAP_ADV_FILTER_ANY;
    uint16_t adv_int_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint16_t adv_int_max = DEFAULT_ADVERTISING_INTERVAL_MIN;

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
    //le_set_gap_param(GAP_PARAM_DEVICE_NAME, GAP_DEVICE_NAME_LEN, device_name);
    le_set_gap_param(GAP_PARAM_APPEARANCE, sizeof(appearance), &appearance);
    le_set_gap_param(GAP_PARAM_SLAVE_INIT_GATT_MTU_REQ, sizeof(slave_init_mtu_req),
                     &slave_init_mtu_req);

    /* Set advertising parameters */
    le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_evt_type), &adv_evt_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR_TYPE, sizeof(adv_direct_type), &adv_direct_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR, sizeof(adv_direct_addr), adv_direct_addr);
    le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_chann_map), &adv_chann_map);
    le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_int_min), &adv_int_min);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_int_max), &adv_int_max);
    //le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data_chip), (void *)adv_data_chip);
    //le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, sizeof(scan_rsp_data_chip), (void *)scan_rsp_data_chip);

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
    le_register_app_cb(bt_matter_adapter_app_gap_callback);

#if (F_BT_LE_USE_RANDOM_ADDR==1)
    T_APP_STATIC_RANDOM_ADDR random_addr;
    bool gen_addr = true;
    uint8_t local_bd_type = GAP_LOCAL_ADDR_LE_RANDOM;
#if 0
    if (ble_peripheral_app_load_static_random_address(&random_addr) == 0)
    {
        if (random_addr.is_exist == true)
        {
            gen_addr = false;
        }
    }
#endif
    if (gen_addr)
    {
        if (le_gen_rand_addr(GAP_RAND_ADDR_STATIC, random_addr.bd_addr) == GAP_CAUSE_SUCCESS)
        {
            random_addr.is_exist = true;
            // Don't save, we use a newly generated random address every boot
            //ble_peripheral_app_save_static_random_address(&random_addr);
            printf("bd addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n", 
                    random_addr.bd_addr[5],
                    random_addr.bd_addr[4],
                    random_addr.bd_addr[3],
                    random_addr.bd_addr[2],
                    random_addr.bd_addr[1],
                    random_addr.bd_addr[0]
                  );
        }
    }
    le_cfg_local_identity_address(random_addr.bd_addr, GAP_IDENT_ADDR_RAND);
    int ret1 = le_set_gap_param(GAP_PARAM_RANDOM_ADDR, 6, random_addr.bd_addr);
    //only for peripheral,broadcaster
    int ret2 = le_adv_set_param(GAP_PARAM_ADV_LOCAL_ADDR_TYPE, sizeof(local_bd_type), &local_bd_type);
    //only for central,observer
    //le_scan_set_param(GAP_PARAM_SCAN_LOCAL_ADDR_TYPE, sizeof(local_bd_type), &local_bd_type);
#endif
}

int bt_stack_already = 0;
int bt_matter_adapter_init(void)
{
	T_GAP_CONN_INFO conn_info;
	T_GAP_DEV_STATE new_state;

	/*Check WIFI init complete*/
	if(RTW_SUCCESS != wifi_is_connected_to_ap())
	{
		if( ! (wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
			BC_printf("WIFI is disabled\n\r");
			return -1;
		}
	}
	else
		return 0;

#if CONFIG_BT_MATTER_ADAPTER
	set_bt_matter_adapter_state(BC_DEV_INIT); // BT Config on
#endif

#if CONFIG_AUTO_RECONNECT
	/* disable auto reconnect */
	wifi_set_autoreconnect(0);
#endif

	wifi_disconnect();
#if CONFIG_LWIP_LAYER
	LwIP_ReleaseIP(WLAN0_IDX);
#endif

	le_get_gap_param(GAP_PARAM_DEV_STATE , &new_state);
	le_get_conn_info(bt_matter_adapter_conn_id, &conn_info);
	bt_matter_adapter_gap_conn_state = conn_info.conn_state;

	if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY) {
		bt_stack_already = 1;
		BC_printf("BT Stack already on\n\r");
	}
	else{
		bt_trace_init();
		bt_matter_adapter_stack_config_init();
		bte_init();
		le_gap_init(APP_MAX_LINKS);
		bt_matter_adapter_app_le_profile_init();
	}

	bt_matter_adapter_app_le_gap_init_chip();
	bt_matter_adapter_task_init();

	bt_coex_init();

	//Wait BT init complete*
	do {
		os_delay(100);
		le_get_gap_param(GAP_PARAM_DEV_STATE , &new_state);
	} while (new_state.gap_init_state != GAP_INIT_STATE_STACK_READY);

#if CONFIG_BT_MATTER_ADAPTER
	//Start BT WIFI coexistence
	wifi_btcoex_set_bt_on();
#endif
	return 0;
}

int bt_matter_adapter_adv(void)
{
	if (bt_stack_already) {
		bt_matter_adapter_app_set_adv_data();
		bt_matter_adapter_send_msg(1); //Start ADV
		set_bt_matter_adapter_state(BC_DEV_IDLE); // BT Config Ready
	}
	return 0;
}

uint16_t ble_att_mtu_z2(uint16_t conn_id)
{
	int ret;
	uint16_t mtu_size;

	ret = le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, &mtu_size, conn_id);
	if (ret == 0)
	{
		printf("printing MTU size\r\n");
		return mtu_size;
	}
	else
		return 0;
}
#if CONFIG_BT_MESH_DEVICE_MATTER
bool ble_matter_netmgr_adapter_init_handler(void)
{
	return bt_matter_adapter_init();
}

bool ble_matter_netmgr_adv_param_handler(uint16_t adv_int_min, uint16_t adv_int_max, void *advData, uint8_t advData_len)
{
	int ret = 0;
	le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_int_min), &adv_int_min);
	le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_int_max), &adv_int_max);
	le_adv_set_param(GAP_PARAM_ADV_DATA, advData_len, advData); // set advData
	return ret;
}

bool ble_matter_netmgr_adv_start_handler(void)
{
	//Stop adv before start
	ble_matter_netmgr_stop_adv();
	ble_matter_netmgr_start_adv();
	return 0;
}

bool ble_matter_netmgr_adv_stop_handler(void)
{
	ble_matter_netmgr_stop_adv();
	return 0;
}
bool ble_matter_netmgr_start_adv(void)
{
	T_GAP_DEV_STATE new_state;
	//check BLE stack state
	le_get_gap_param(GAP_PARAM_DEV_STATE , &new_state);
	if(new_state.gap_init_state != GAP_INIT_STATE_STACK_READY)
	{
		printf("Waiting for ble stack ready...\n");
		do{
			os_delay(100);
			le_get_gap_param(GAP_PARAM_DEV_STATE , &new_state);
		}while(new_state.gap_init_state != GAP_INIT_STATE_STACK_READY);
	}

	//check adv state
	if(new_state.gap_adv_state != GAP_ADV_STATE_IDLE)
	{
		printf("Waiting for adv ready \n");
		do{
			os_delay(100);
			le_get_gap_param(GAP_PARAM_DEV_STATE , &new_state);
		}while(new_state.gap_adv_state != GAP_ADV_STATE_IDLE);
	}

	//send adv cmd
	if(bt_matter_adapter_send_msg(BT_MATTER_MSG_START_ADV, NULL) == false)
	{
		printf("msg send fail \n");
		return false;
	}

	//while(new_state.gap_adv_state != GAP_ADV_STATE_ADVERTISING);
	return true;
}

bool ble_matter_netmgr_stop_adv(void)
{
	T_GAP_DEV_STATE new_state;

	//check adv state
	le_get_gap_param(GAP_PARAM_DEV_STATE , &new_state);
	if(new_state.gap_adv_state != GAP_ADV_STATE_ADVERTISING)
	{
		printf("adv not start \n");
	} else {
	//send adv cmd
		if(bt_matter_adapter_send_msg(BT_MATTER_MSG_STOP_ADV, NULL) == false)
		{
			printf("msg send fail \n");
			return false;
		}
	}
	return true;
}

bool ble_matter_netmgr_server_send_data(uint8_t conn_id, T_SERVER_ID service_id, uint16_t attrib_index,
					  uint8_t *p_data, uint16_t data_len, T_GATT_PDU_TYPE type)
{
		BT_MATTER_SERVER_SEND_DATA *param = os_mem_alloc(0, sizeof(BT_MATTER_SERVER_SEND_DATA));
		if(param)
		{
				param->conn_id = conn_id;
				param->service_id = service_id;
				param->attrib_index = attrib_index;
				param->data_len = data_len;
				param->type = type;
				if (param->data_len  !=0)
				{
					param->p_data = os_mem_alloc(0, param->data_len);
					memcpy(param->p_data, p_data, param->data_len);
				}
				if(bt_matter_adapter_send_msg(BT_MATTER_MSG_SEND_DATA, param)==false)
				{
					printf("os_mem_free\r\n");
					os_mem_free(param);
					os_mem_free(param->p_data);
					return false;
				}
		}
		else
				printf("Malloc failed\r\n");
		return true;
}
#endif /*CONFIG_BT_MESH_DEVICE_MATTER*/
#endif
