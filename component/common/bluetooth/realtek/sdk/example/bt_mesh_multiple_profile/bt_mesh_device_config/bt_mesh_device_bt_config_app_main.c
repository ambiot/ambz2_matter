/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      main.c
   * @brief     Source file for BLE scatternet project, mainly used for initialize modules
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
#include <platform_opts_bt.h>
#if defined(CONFIG_BT_MESH_DEVICE_CONFIG) && CONFIG_BT_MESH_DEVICE_CONFIG
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
#include "bt_mesh_device_bt_config_app_main.h"
#include "bt_config_wifi.h"
#include "bt_config_service.h"
#include "bt_fast_private_provision_service.h"
#include <osdep_service.h>
#include "trace_uart.h"
#include "bte.h"
#include "wifi_constants.h"
#include "wifi_conf.h"
#include "lwip_netconf.h"
#include "os_sched.h"

#include <stdlib.h>
#include <os_sched.h>
#include <gap_scan.h>
#include <gaps_client.h>
#include <profile_client.h>
#include <gatt_builtin_services.h>
#include <platform_utils.h>
#include "mesh_api.h"
#include "mesh_cmd.h"
#include "health.h"
#include "generic_on_off.h"
#include "ping.h"
#include "ping_app.h"
#include "tp.h"
#include "datatrans_server.h"
#include "datatrans_app.h"
#include <bt_mesh_device_bt_config_app_task.h>
#include "bt_mesh_device_bt_config_app_flags.h"
#include "bt_mesh_device_bt_config_app.h"
#include "vendor_cmd.h"
#include "vendor_cmd_bt.h"
#include "osdep_service.h"
#include "wifi_constants.h"
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
#include "bt_mesh_device_api.h"
#endif
#if defined(CONFIG_BT_MESH_IDLE_CHECK) && CONFIG_BT_MESH_IDLE_CHECK
#include "device_idle_check.h"
#endif
#include "trace_uart.h"
#include <gap_le_types.h>
#include <wifi/wifi_conf.h>
#include "rtk_coex.h"
#include <simple_ble_config.h>


extern bool bt_trace_uninit(void);
//extern uint8_t airsync_specific;


/*============================================================================*
 *                              Constants
 *============================================================================*/
/** @brief  Default minimum advertising interval when device is discoverable (units of 625us, 160=100ms) */
#define DEFAULT_ADVERTISING_INTERVAL_MIN            320
/** @brief  Default maximum advertising interval */
#define DEFAULT_ADVERTISING_INTERVAL_MAX            400

/** @brief Default scan interval (units of 0.625ms, 0x520=820ms) */
#define DEFAULT_SCAN_INTERVAL     0x520
/** @brief Default scan window (units of 0.625ms, 0x520=820ms) */
#define DEFAULT_SCAN_WINDOW       0x520

#define COMPANY_ID        0x005D
#define PRODUCT_ID        0x0000
#define VERSION_ID        0x0000

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
static const uint8_t bt_mesh_device_bt_config_adv_data[] =
{
    /* Flags */
    0x02,             /* length */
    GAP_ADTYPE_FLAGS, /* type="Flags" */
    GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
    /* Service */
    0x03,             /* length */
    GAP_ADTYPE_16BIT_COMPLETE,
    LO_WORD(GATT_UUID_BT_CONFIG_PROFILE),
    HI_WORD(GATT_UUID_BT_CONFIG_PROFILE),
    /* Local name */
    0x0D,             /* length */
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'A', 'm', 'e', 'b', 'a', '_', 'x', 'x', 'y', 'y', 'z', 'z',
};

//int array_count_of_adv_data = sizeof(bt_mesh_device_bt_config_adv_data) / sizeof(bt_mesh_device_bt_config_adv_data[0]);

plt_timer_t bt_mesh_device_bt_config_adv_timer = NULL;
uint8_t bt_mesh_device_bt_config_le_adv_start_enable = 0;
uint16_t bt_mesh_device_bt_config_adv_interval = 352;

mesh_model_info_t health_server_model;
mesh_model_info_t generic_on_off_server_model;

generic_on_off_t current_on_off = GENERIC_OFF;

extern void *bt_mesh_device_bt_config_evt_queue_handle;  //!< Event queue handle
extern void *bt_mesh_device_bt_config_io_queue_handle;   //!< IO queue handle

void bt_mesh_device_bt_config_adv_timer_handler(void *FunctionContext)
{
    /* avoid gcc compile warning */
    (void)FunctionContext;
    uint8_t event = EVENT_IO_TO_APP;
    T_IO_MSG io_msg;
    uint16_t adv_interval = bt_mesh_device_bt_config_adv_interval * 625 / 1000;

    io_msg.type = IO_MSG_TYPE_ADV;
    if (os_msg_send(bt_mesh_device_bt_config_io_queue_handle, &io_msg, 0) == false)
    {
    }
    else if (os_msg_send(bt_mesh_device_bt_config_evt_queue_handle, &event, 0) == false)
    {
    }
    if (bt_mesh_device_bt_config_le_adv_start_enable) {
        plt_timer_change_period(bt_mesh_device_bt_config_adv_timer, adv_interval, 0xFFFFFFFF);
    }
}

void bt_mesh_device_bt_config_le_adv_start(void)
{
    uint16_t adv_interval = bt_mesh_device_bt_config_adv_interval * 625 / 1000;

    bt_mesh_device_bt_config_le_adv_start_enable = 1;
    plt_timer_change_period(bt_mesh_device_bt_config_adv_timer, adv_interval, 0xFFFFFFFF);
}

void bt_mesh_device_bt_config_le_adv_stop(void)
{
    bt_mesh_device_bt_config_le_adv_start_enable = 0;
}


/*============================================================================*
 *                              Functions
 *============================================================================*/

static int32_t generic_on_off_server_data(const mesh_model_info_p pmodel_info, uint32_t type, void *pargs)
{
    /* avoid gcc compile warning */
    (void)pmodel_info;
    
    switch (type)
    {
        case GENERIC_ON_OFF_SERVER_GET:
            {
                generic_on_off_server_get_t *pdata = pargs;
                pdata->on_off = current_on_off;
            }
            break;
        case GENERIC_ON_OFF_SERVER_GET_DEFAULT_TRANSITION_TIME:
            break;
        case GENERIC_ON_OFF_SERVER_SET:
            {
                generic_on_off_server_set_t *pdata = pargs;
                if (pdata->total_time.num_steps == pdata->remaining_time.num_steps)
                {
                    if (pdata->on_off != current_on_off)
                    {
                        current_on_off = pdata->on_off;
                        if (current_on_off == GENERIC_OFF)
                        {
                            printf("Provisioner turn light OFF!\r\n");
                        }
                        else if (current_on_off == GENERIC_ON)
                        {
                            printf("Provisioner turn light ON!\r\n");
                        }
                    }
                }
            }
            break;
        default:
            break;
    }

    return 0;
}

void generic_on_off_server_model_init(void)
{
    generic_on_off_server_model.model_data_cb = generic_on_off_server_data;
    generic_on_off_server_reg(0, &generic_on_off_server_model);
}

/******************************************************************
 * @fn          Initial gap parameters
 * @brief      Initialize peripheral and gap bond manager related parameters
 *
 * @return     void
 */

void bt_mesh_device_bt_config_stack_init(void)
{
    /** set mesh stack log level, default all on, disable the log of level LEVEL_TRACE */
    uint32_t module_bitmap[MESH_LOG_LEVEL_SIZE] = {0};
    diag_level_set(TRACE_LEVEL_TRACE, module_bitmap);

    /** mesh stack needs rand seed */
    plt_srand(platform_random(0xffffffff));

    /** configure provisioning parameters */
    prov_capabilities_t prov_capabilities =
    {
        .algorithm = PROV_CAP_ALGO_FIPS_P256_ELLIPTIC_CURVE
#if F_BT_MESH_1_1_EPA_SUPPORT
        | PROV_CAP_ALGO_BTM_ECDH_P256_HMAC_SHA256_AES_CCM
#endif
        ,
        .public_key = 0,
        .static_oob = 0,
        .output_oob_size = 0,
        .output_oob_action = 0,
        .input_oob_size = 0,
        .input_oob_action = 0
    };
    prov_params_set(PROV_PARAMS_CAPABILITIES, &prov_capabilities, sizeof(prov_capabilities_t));
    prov_params_set(PROV_PARAMS_CALLBACK_FUN, (void *)prov_cb, sizeof(prov_cb_pf));

    /** config node parameters */
    mesh_node_features_t features =
    {
        .role = MESH_ROLE_DEVICE,
        .relay = 1,
        .proxy = 1,
        .fn = 1,
        .lpn = 1,
        .prov = 1,
        .udb = 1,
        .snb = 1,
        .bg_scan = 1,
        .flash = 1,
        .flash_rpl = 1,
#if F_BT_MESH_1_1_PRB_SUPPORT
        .prb = 1,
        .private_proxy = 1,
#endif
#if F_BT_MESH_1_1_SBR_SUPPORT
        .sbr = 1,
#endif
#if F_BT_MESH_1_1_DF_SUPPORT
        .df = 1,
#endif
    };

    mesh_node_cfg_t node_cfg =
    {
        .dev_key_num = 2,
        .net_key_num = 10,
        .master_key_num = 3, // shall <= net_key_num
        .app_key_num = 3,
        .vir_addr_num = 3,
        .rpl_num = 20,
        .sub_addr_num = 5,
        .proxy_num = 1,
        .prov_interval = 2,
        .udb_interval = 2,
        .proxy_interval = 5,
#if F_BT_MESH_1_1_SBR_SUPPORT
        .bridging_table_size = 5,
#endif
#if F_BT_MESH_1_1_DF_SUPPORT
        .df_fixed_path_size = 5,
#endif
    };

    mesh_node_cfg(features, &node_cfg);
    proxy_server_support_prov_on_proxy(true);
    mesh_node.net_trans_count = 6;
    mesh_node.relay_retrans_count = 2;
    mesh_node.trans_retrans_count = 4;
    mesh_node.ttl = 5;
#if MESH_LPN
    mesh_node.frnd_poll_retry_times = 32;
#endif

    le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, sizeof(scan_rsp_data), (void *)scan_rsp_data);

    /** create elements and register models */
    mesh_element_create(GATT_NS_DESC_UNKNOWN);

    health_server_reg(0, &health_server_model);// must be reg so that can promise request composition data ok
    health_server_set_company_id(&health_server_model, COMPANY_ID);
    ping_control_reg(ping_app_ping_cb, pong_receive);
    tp_control_reg(tp_receive);
    datatrans_model_init();
    generic_on_off_server_model_init();

    bt_mesh_device_bt_config_adv_timer = plt_timer_create("bt_mesh_device_bt_config_adv_timer", 0xFFFFFFFF, FALSE, NULL, bt_mesh_device_bt_config_adv_timer_handler);
    if (!bt_mesh_device_bt_config_adv_timer) {
        printf("[BT Mesh Device Config] Create adv timer failed\n\r");
    }

    compo_data_page0_header_t compo_data_page0_header = {COMPANY_ID, PRODUCT_ID, VERSION_ID};
    compo_data_page0_gen(&compo_data_page0_header);

    /** init mesh stack */
    mesh_init();

    /** register proxy adv callback */
    device_info_cb_reg(device_info_cb);
    rpl_cb_reg(rpl_cb);
#if F_BT_MESH_1_1_DF_SUPPORT
    df_cb_reg(df_cb);
#endif
}

/**
  * @brief  Initialize gap related parameters
  * @return void
  */
void bt_mesh_device_bt_config_app_le_gap_init(void)
{
    /* GAP Bond Manager parameters */
    uint8_t  auth_pair_mode = GAP_PAIRING_MODE_PAIRABLE;
    uint16_t auth_flags = GAP_AUTHEN_BIT_BONDING_FLAG;
    uint8_t  auth_io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8_t  auth_oob = false;
    uint8_t  auth_use_fix_passkey = false;
    uint32_t auth_fix_passkey = 0;
    uint8_t  auth_sec_req_enable = false;
    uint16_t auth_sec_req_flags = GAP_AUTHEN_BIT_BONDING_FLAG;

    uint16_t scan_window = 0x100; /* 160ms */
    uint16_t scan_interval = 0x120; /* 180ms */

    gap_sched_params_set(GAP_SCHED_PARAMS_INTERWAVE_SCAN_WINDOW, &scan_window, sizeof(scan_window));
    gap_sched_params_set(GAP_SCHED_PARAMS_INTERWAVE_SCAN_INTERVAL, &scan_interval, sizeof(scan_interval));
    gap_sched_params_set(GAP_SCHED_PARAMS_SCAN_WINDOW, &scan_window, sizeof(scan_window));
    gap_sched_params_set(GAP_SCHED_PARAMS_SCAN_INTERVAL, &scan_interval, sizeof(scan_interval));
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

#if F_BT_LE_5_0_SET_PHY_SUPPORT
    uint8_t phys_prefer = GAP_PHYS_PREFER_ALL;
    uint8_t tx_phys_prefer = GAP_PHYS_PREFER_1M_BIT;
    uint8_t rx_phys_prefer = GAP_PHYS_PREFER_1M_BIT;
    le_set_gap_param(GAP_PARAM_DEFAULT_PHYS_PREFER, sizeof(phys_prefer), &phys_prefer);
    le_set_gap_param(GAP_PARAM_DEFAULT_TX_PHYS_PREFER, sizeof(tx_phys_prefer), &tx_phys_prefer);
    le_set_gap_param(GAP_PARAM_DEFAULT_RX_PHYS_PREFER, sizeof(rx_phys_prefer), &rx_phys_prefer);
#endif

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
    le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(bt_mesh_device_bt_config_adv_data), (void *)bt_mesh_device_bt_config_adv_data);
    le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, sizeof(scan_rsp_data), (void *)scan_rsp_data);

    /* register gap message callback */
   // le_register_app_cb(bt_config_app_gap_callback);


    vendor_cmd_init(bt_mesh_device_bt_config_app_vendor_callback);
    /* register gap message callback */
    le_register_app_cb(bt_mesh_device_bt_config_app_gap_callback);
    
}

/**
 * @brief  Add GATT services, clients and register callbacks
 * @return void
 */
void bt_mesh_device_bt_config_app_le_profile_init(void)
{
    server_init(MESH_GATT_SERVER_COUNT + 3);

    /* Register Server Callback */
    server_register_app_cb(bt_mesh_device_bt_config_app_profile_callback);

    client_init(MESH_GATT_CLIENT_COUNT);
    /* Add Client Module */

    /* Register Client Callback--App_ClientCallback to handle events from Profile Client layer. */
    client_register_general_client_cb(bt_mesh_device_bt_config_app_client_callback);
    
    //server_init(1);
    bt_mesh_device_bt_config_srv_id = bt_config_service_add_service((void *)bt_mesh_device_bt_config_app_profile_callback);

    bt_private_provision_srv_id = bt_fast_private_provision_service_add_service((void *)bt_mesh_device_bt_config_app_profile_callback);
}

/**
 * @brief    Contains the initialization of pinmux settings and pad settings
 * @note     All the pinmux settings and pad settings shall be initiated in this function,
 *           but if legacy driver is used, the initialization of pinmux setting and pad setting
 *           should be peformed with the IO initializing.
 * @return   void
 */
void bt_mesh_device_bt_config_board_init(void)
{

}

/**
 * @brief    Contains the initialization of peripherals
 * @note     Both new architecture driver and legacy driver initialization method can be used
 * @return   void
 */
void bt_mesh_device_bt_config_driver_init(void)
{

}

/**
 * @brief    Contains the power mode settings
 * @return   void
 */
void bt_mesh_device_bt_config_pwr_mgr_init(void)
{
}

/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BLE Scatternet APP, thus only one APP task is init here
 * @return   void
 */
void bt_mesh_device_bt_config_task_init(void)
{
    bt_mesh_device_bt_config_app_task_init();
	bt_config_wifi_init();
}

void bt_mesh_device_bt_config_task_deinit(void)
{
    bt_config_wifi_deinit();
    bt_mesh_device_bt_config_app_task_deinit();
}


extern void gap_config_hci_task_secure_context(uint32_t size);
void bt_mesh_device_bt_config_stack_config_init(void)
{
    gap_config_max_le_link_num(BT_MESH_DEVICE_BT_CONFIG_APP_MAX_LINKS);
    gap_config_max_le_paired_device(BT_MESH_DEVICE_BT_CONFIG_APP_MAX_LINKS);
    gap_config_hci_task_secure_context (280);
}

/**
 * @brief    Entry of APP code
 * @return   int (To avoid compile warning)
 */
int bt_mesh_device_bt_config_app_main(void)
{
	bt_trace_init();
    bt_mesh_device_bt_config_stack_config_init();
    bte_init();
    bt_mesh_device_bt_config_board_init();
    bt_mesh_device_bt_config_driver_init();
    le_gap_init(BT_MESH_DEVICE_BT_CONFIG_APP_MAX_LINKS);
    bt_mesh_device_bt_config_app_le_gap_init();
    bt_mesh_device_bt_config_app_le_profile_init();
    bt_mesh_device_bt_config_stack_init();
    bt_mesh_device_bt_config_pwr_mgr_init();
    bt_mesh_device_bt_config_task_init();

    return 0;
}


/***********unused function in bt_config_app_main.c*******************/
extern T_GAP_DEV_STATE bt_mesh_device_bt_config_gap_dev_state;
extern T_GAP_CONN_STATE bt_mesh_device_bt_config_gap_conn_state;
extern uint8_t bt_mesh_device_bt_config_conn_id;
extern void bt_coex_init(void);
extern void bt_mesh_device_bt_config_app_set_adv_data(void);
static uint8_t bt_mesh_device_bt_config_state = BC_DEV_DISABLED;

uint8_t bt_mesh_device_bt_config_get_bt_config_state(void)
{
	return bt_mesh_device_bt_config_state;
}

void bt_mesh_device_bt_config_set_bt_config_state(uint8_t state)
{
	bt_mesh_device_bt_config_state = state;
}
/****************end*********************/

int bt_mesh_device_bt_config_app_init(void)
{
   // int bt_stack_already_on = 0;
	T_GAP_CONN_INFO conn_info;
	T_GAP_DEV_STATE new_state;

	/*Wait WIFI init complete*/
	while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
		os_delay(1000);
	}

    bt_mesh_device_bt_config_set_bt_config_state(BC_DEV_INIT); // BT Config on
#if CONFIG_AUTO_RECONNECT
	/* disable auto reconnect */
	wifi_set_autoreconnect(0);
#endif
	
	
	wifi_disconnect();
#if CONFIG_LWIP_LAYER
	LwIP_ReleaseIP(WLAN0_IDX);
#endif

	//judge BLE central is already on
	le_get_gap_param(GAP_PARAM_DEV_STATE , &new_state);
    le_get_conn_info(bt_mesh_device_bt_config_conn_id, &conn_info);
	bt_mesh_device_bt_config_gap_conn_state = conn_info.conn_state;
    
	if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY) {
	//	bt_stack_already_on = 1;
		printf("[BT Mesh Device]BT Stack already on\n\r");
		return 0;
	}
	else
		bt_mesh_device_bt_config_app_main();
	bt_coex_init();

	/*Wait BT init complete*/
	do {
		os_delay(100);
		le_get_gap_param(GAP_PARAM_DEV_STATE , &new_state);
	}while(new_state.gap_init_state != GAP_INIT_STATE_STACK_READY);

#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
    if (bt_mesh_device_api_init()) {
        printf("[BT Mesh Device] bt_mesh_device_api_init fail ! \n\r");
        return 1;
    }
#endif

#if defined(CONFIG_BT_MESH_IDLE_CHECK) && CONFIG_BT_MESH_IDLE_CHECK
    bt_mesh_idle_check_init();
#endif

    /*if (bt_stack_already_on) {
		bt_mesh_device_bt_config_app_set_adv_data();
		bt_mesh_device_bt_config_send_msg(1); //Start ADV
		bt_mesh_device_bt_config_set_bt_config_state(BC_DEV_IDLE); // BT Config Ready
	}*/
	return 0;
}

extern void mesh_deinit(void);
extern bool mesh_initial_state;
extern bool bt_trace_uninit(void);


extern T_GAP_DEV_STATE bt_mesh_device_bt_config_gap_dev_state;

void bt_mesh_device_bt_config_app_deinit(void)
{
    T_GAP_DEV_STATE new_state;

#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
    bt_mesh_device_api_deinit();
#endif
    bt_mesh_device_bt_config_set_bt_config_state(BC_DEV_DEINIT);
    bt_mesh_device_bt_config_task_deinit();
    le_get_gap_param(GAP_PARAM_DEV_STATE , &new_state);
	if (new_state.gap_init_state != GAP_INIT_STATE_STACK_READY) {
		printf("[BT Mesh Device] BT Stack is not running\n\r");
        mesh_initial_state = FALSE;
        bt_mesh_device_bt_config_gap_dev_state.gap_init_state = GAP_INIT_STATE_INIT;
        return;
	}
#if F_BT_DEINIT
	else {
		bte_deinit();
		printf("[BT Mesh Device] BT Stack deinitalized\n\r");
	}
#endif
    mesh_deinit();
#if F_BT_MESH_1_1_RPR_SUPPORT
    prov_client_deinit();
#endif
    bt_trace_uninit();

#if defined(CONFIG_BT_MESH_IDLE_CHECK) && CONFIG_BT_MESH_IDLE_CHECK
    bt_mesh_idle_check_deinit();
#endif
    bt_mesh_device_bt_config_set_bt_config_state(BC_DEV_DISABLED);

    plt_timer_delete(bt_mesh_device_bt_config_adv_timer, 0xFFFFFFFF);
	bt_mesh_device_bt_config_adv_timer = NULL;

    mesh_initial_state = FALSE;
	bt_mesh_device_bt_config_gap_dev_state.gap_init_state = GAP_INIT_STATE_INIT;
}

#endif
