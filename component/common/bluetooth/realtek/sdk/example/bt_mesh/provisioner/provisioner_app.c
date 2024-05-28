/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      device_app.c
* @brief     Smart mesh demo application
* @details
* @author    bill
* @date      2015-11-12
* @version   v0.1
* *********************************************************************************************************
*/

#include <string.h>
#include <app_msg.h>
#include <trace.h>
#include <gap_scan.h>
#include <gap.h>
#include <gap_msg.h>
#include <gap_adv.h>
#include <gap_bond_le.h>
#include <profile_server.h>

#if defined(CONFIG_PLATFORM_8721D)
#include "ameba_soc.h"
#endif
#include "provisioner_app.h"
#include "trace_app.h"
#include "gap_wrapper.h"
#include "mesh_api.h"
#include "mesh_user_cmd_parse.h"
#include "mesh_cmd.h"
#include "ping_app.h"
#include "provisioner_cmd.h"
#include "provision_provisioner.h"
#include "provision_client.h"
#include "proxy_client.h"
#include "bt_flags.h"
#include "bt_mesh_provisioner_app_flags.h"
#include "platform_opts.h"
#include "vendor_cmd.h"
#include "vendor_cmd_bt.h"
#if defined(MESH_DFU) && MESH_DFU
#include "dfu_distributor_app.h"
#endif
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
#include "bt_mesh_user_api.h"
#include "bt_mesh_provisioner_api.h"
#endif
#if defined(CONFIG_EXAMPLE_BT_MESH_DEMO) && CONFIG_EXAMPLE_BT_MESH_DEMO
#include "bt_mesh_app_lib_intf.h"
extern struct BT_MESH_LIB_PRIV bt_mesh_lib_priv;
#endif
#include "mesh_data_dump.h"

#if F_BT_MESH_1_1_RPR_SUPPORT
#include "remote_provisioning.h"
#endif

bool prov_manual;
uint32_t prov_start_time;
prov_auth_value_type_t prov_auth_value_type;

/**
 * @brief  Application Link control block defination.
 */
typedef struct
{
    T_GAP_CONN_STATE        conn_state;          /**< Connection state. */
    T_GAP_REMOTE_ADDR_TYPE  bd_type;             /**< remote BD type*/
    uint8_t                 bd_addr[GAP_BD_ADDR_LEN]; /**< remote BD */
} T_APP_LINK;

T_GAP_DEV_STATE bt_mesh_provisioner_gap_dev_state = {0, 0, 0, 0, 0};                 /**< GAP device state */
T_APP_LINK app_link_table[APP_MAX_LINKS];

void bt_mesh_provisioner_app_handle_gap_msg(T_IO_MSG *p_gap_msg);
#if defined(MESH_DFU) && MESH_DFU
extern void dfu_dist_handle_timeout(void);
#endif
/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void bt_mesh_provisioner_app_handle_io_msg(T_IO_MSG io_msg)
{
    uint16_t msg_type = io_msg.type;
    switch (msg_type)
    {
    case IO_MSG_TYPE_BT_STATUS:
        bt_mesh_provisioner_app_handle_gap_msg(&io_msg);
        break;
    case IO_MSG_TYPE_UART:
        {
            /* We handle user command informations from Data UART in this branch. */
            uint8_t data = io_msg.subtype;
            mesh_user_cmd_collect(&data, sizeof(data), provisioner_cmd_table);
        }
        break;
    case PING_TIMEOUT_MSG:
        ping_handle_timeout();
        break;
    case PING_APP_TIMEOUT_MSG:
        ping_app_handle_timeout();
        break;
#if F_BT_MESH_1_1_MBT_SUPPORT
    case BLOB_CLIENT_PROCEDURE_TIMEOUT:
        blob_client_handle_procedure_timeout();
        break;
    case BLOB_CLIENT_RETRY_TIMEOUT:
        blob_client_handle_retry_timeout();
        break;
    case BLOB_CLIENT_CHUNK_TRANSFER:
        blob_client_active_chunk_transfer();
        break;
#endif
#if F_BT_MESH_1_1_DFU_SUPPORT
    case DFU_DIST_APP_TIMEOUT_MSG:
        dfu_dist_handle_timeout();
        break;
    case DFU_INIT_APP_TIMEOUT_MSG:
        dfu_init_handle_timeout();
        break;
#endif
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
void bt_mesh_provisioner_app_handle_dev_state_evt(T_GAP_DEV_STATE new_state, uint16_t cause)
{
    APP_PRINT_INFO4("bt_mesh_provisioner_app_handle_dev_state_evt: init state  %d, adv state %d, scan state %d, cause 0x%x",
                    new_state.gap_init_state, new_state.gap_adv_state,
                    new_state.gap_scan_state, cause);
    if (bt_mesh_provisioner_gap_dev_state.gap_init_state != new_state.gap_init_state)
    {
        if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
        {
            APP_PRINT_INFO0("GAP stack ready");
            uint8_t bt_addr[6];
            uint8_t net_key[16] = MESH_NET_KEY;
            uint8_t net_key1[16] = MESH_NET_KEY1;
            uint8_t app_key[16] = MESH_APP_KEY;
            uint8_t app_key1[16] = MESH_APP_KEY1;
            gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);
            printf("bt addr: 0x%02x%02x%02x%02x%02x%02x\r\n",
                            bt_addr[5], bt_addr[4], bt_addr[3],
                            bt_addr[2], bt_addr[1], bt_addr[0]);

            /** configure provisioner */
            mesh_node.node_state = PROV_NODE;
            mesh_node.unicast_addr = bt_addr[0] % 99 + 1;
            memcpy(&net_key[10], bt_addr, sizeof(bt_addr));
            memcpy(&net_key1[10], bt_addr, sizeof(bt_addr));
            memcpy(&app_key[10], bt_addr, sizeof(bt_addr));
            memcpy(&app_key1[10], bt_addr, sizeof(bt_addr));
            uint16_t net_key_index = net_key_add(0, net_key);
            app_key_add(net_key_index, 0, app_key);
            uint8_t net_key_index1 = net_key_add(1, net_key1);
            app_key_add(net_key_index1, 1, app_key1);
        	mesh_model_bind_all_key();
        }
    }
    if (bt_mesh_provisioner_gap_dev_state.gap_scan_state != new_state.gap_scan_state)
    {
        if (new_state.gap_scan_state == GAP_SCAN_STATE_IDLE)
        {
            APP_PRINT_INFO0("GAP scan stop");
            //printf("GAP scan stop\r\n");
        }
        else if (new_state.gap_scan_state == GAP_SCAN_STATE_SCANNING)
        {
            APP_PRINT_INFO0("GAP scan start");
            //printf("GAP scan start\r\n");
        }
    }
    
    bt_mesh_provisioner_gap_dev_state = new_state;
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
void bt_mesh_provisioner_app_handle_conn_state_evt(uint8_t conn_id, T_GAP_CONN_STATE new_state, uint16_t disc_cause)
{
    if (conn_id >= APP_MAX_LINKS)
    {
        return;
    }

    APP_PRINT_INFO4("bt_mesh_provisioner_app_handle_conn_state_evt: conn_id %d, conn_state(%d -> %d), disc_cause 0x%x",
                    conn_id, app_link_table[conn_id].conn_state, new_state, disc_cause);

    app_link_table[conn_id].conn_state = new_state;
    switch (new_state)
    {
    case GAP_CONN_STATE_DISCONNECTED:
        {
            if ((disc_cause != (HCI_ERR | HCI_ERR_REMOTE_USER_TERMINATE))
                && (disc_cause != (HCI_ERR | HCI_ERR_LOCAL_HOST_TERMINATE)))
            {
                APP_PRINT_ERROR2("bt_mesh_provisioner_app_handle_conn_state_evt: connection lost, conn_id %d, cause 0x%x", conn_id,
                                 disc_cause);
            }
            printf("Disconnect conn_id %d\r\n", conn_id);
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
            uint8_t ret = USER_API_RESULT_ERROR;
            ret = bt_mesh_indication(GEN_MESH_CODE(_connect), BT_MESH_USER_CMD_FAIL, NULL);
            if (ret != USER_API_RESULT_OK) {
                if (ret != USER_API_RESULT_INCORRECT_CODE) {
                    printf("[BT_MESH] %s(): user cmd %d fail !\r\n", __func__, GEN_MESH_CODE(_connect));
                    memset(&app_link_table[conn_id], 0, sizeof(T_APP_LINK));
                    break;
                }  
            } else {
                memset(&app_link_table[conn_id], 0, sizeof(T_APP_LINK));
                break;
            }
            ret = bt_mesh_indication(GEN_MESH_CODE(_disconnect), BT_MESH_USER_CMD_FAIL, NULL);
            if (ret != USER_API_RESULT_OK) {
                if (ret != USER_API_RESULT_INCORRECT_CODE) {
                    printf("[BT_MESH] %s(): user cmd %d fail !\r\n", __func__, GEN_MESH_CODE(_disconnect));
                    memset(&app_link_table[conn_id], 0, sizeof(T_APP_LINK));
                    break;
                }  
            } else {
                memset(&app_link_table[conn_id], 0, sizeof(T_APP_LINK));
                break;
            }
#endif
            memset(&app_link_table[conn_id], 0, sizeof(T_APP_LINK));
        }
        break;

    case GAP_CONN_STATE_CONNECTED:
        {
            uint16_t conn_interval;
            uint16_t conn_latency;
            uint16_t conn_supervision_timeout;

            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
            le_get_conn_addr(conn_id, app_link_table[conn_id].bd_addr,
                             &app_link_table[conn_id].bd_type);
            APP_PRINT_INFO5("GAP_CONN_STATE_CONNECTED:remote_bd %s, remote_addr_type %d, conn_interval 0x%x, conn_latency 0x%x, conn_supervision_timeout 0x%x",
                            TRACE_BDADDR(app_link_table[conn_id].bd_addr), app_link_table[conn_id].bd_type,
                            conn_interval, conn_latency, conn_supervision_timeout);
            printf("Connected success conn_id %d\r\n", conn_id);
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
            if (bt_mesh_indication(GEN_MESH_CODE(_connect), BT_MESH_USER_CMD_SUCCESS, NULL) != USER_API_RESULT_OK) {
                printf("[BT_MESH] %s(): user cmd %d fail !\r\n", __func__, GEN_MESH_CODE(_connect));  
            }
#endif
#if F_BT_LE_5_0_SET_PHY_SUPPORT
            {
                uint8_t tx_phy;
                uint8_t rx_phy;
                le_get_conn_param(GAP_PARAM_CONN_RX_PHY_TYPE, &rx_phy, conn_id);
                le_get_conn_param(GAP_PARAM_CONN_TX_PHY_TYPE, &tx_phy, conn_id);
                APP_PRINT_INFO2("GAP_CONN_STATE_CONNECTED: tx_phy %d, rx_phy %d", tx_phy, rx_phy);
            }
#endif

#if F_BT_LE_4_2_DATA_LEN_EXT_SUPPORT
            le_set_data_len(conn_id, 251, 2120);
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
void bt_mesh_provisioner_app_handle_authen_state_evt(uint8_t conn_id, uint8_t new_state, uint16_t cause)
{
    APP_PRINT_INFO2("bt_mesh_provisioner_app_handle_authen_state_evt:conn_id %d, cause 0x%x", conn_id, cause);

    switch (new_state)
    {
    case GAP_AUTHEN_STATE_STARTED:
        {
            APP_PRINT_INFO0("bt_mesh_provisioner_app_handle_authen_state_evt: GAP_AUTHEN_STATE_STARTED");
        }
        break;

    case GAP_AUTHEN_STATE_COMPLETE:
        {
            if (cause == GAP_SUCCESS)
            {
                printf("Pair success\r\n");
                APP_PRINT_INFO0("bt_mesh_provisioner_app_handle_authen_state_evt: GAP_AUTHEN_STATE_COMPLETE pair success");

            }
            else
            {
                printf("Pair failed: cause 0x%x\r\n", cause);
                APP_PRINT_INFO0("bt_mesh_provisioner_app_handle_authen_state_evt: GAP_AUTHEN_STATE_COMPLETE pair failed");
            }
        }
        break;

    default:
        {
            APP_PRINT_ERROR1("bt_mesh_provisioner_app_handle_authen_state_evt: unknown newstate %d", new_state);
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
void bt_mesh_provisioner_app_handle_conn_mtu_info_evt(uint8_t conn_id, uint16_t mtu_size)
{
    APP_PRINT_INFO2("bt_mesh_provisioner_app_handle_conn_mtu_info_evt: conn_id %d, mtu_size %d", conn_id, mtu_size);
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_PARAM_UPDATE
 * @note     All the connection parameter update change  events are pre-handled in this function.
 * @param[in] conn_id Connection ID
 * @param[in] status  New update state
 * @param[in] cause Use this cause when status is GAP_CONN_PARAM_UPDATE_STATUS_FAIL
 * @return   void
 */
void bt_mesh_provisioner_app_handle_conn_param_update_evt(uint8_t conn_id, uint8_t status, uint16_t cause)
{
    switch (status)
    {
    case GAP_CONN_PARAM_UPDATE_STATUS_SUCCESS:
        {
            uint16_t conn_interval;
            uint16_t conn_slave_latency;
            uint16_t conn_supervision_timeout;

            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_slave_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
            APP_PRINT_INFO4("bt_mesh_provisioner_app_handle_conn_param_update_evt update success:conn_id %d, conn_interval 0x%x, conn_slave_latency 0x%x, conn_supervision_timeout 0x%x",
                            conn_id, conn_interval, conn_slave_latency, conn_supervision_timeout);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_FAIL:
        {
            APP_PRINT_ERROR2("bt_mesh_provisioner_app_handle_conn_param_update_evt update failed: conn_id %d, cause 0x%x",
                             conn_id, cause);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_PENDING:
        {
            APP_PRINT_INFO1("bt_mesh_provisioner_app_handle_conn_param_update_evt update pending: conn_id %d", conn_id);
        }
        break;

    default:
        break;
    }
}

bool mesh_initial_state = FALSE;

/**
 * @brief    All the BT GAP MSG are pre-handled in this function.
 * @note     Then the event handling function shall be called according to the
 *           sub_type of T_IO_MSG
 * @param[in] p_gap_msg Pointer to GAP msg
 * @return   void
 */
void bt_mesh_provisioner_app_handle_gap_msg(T_IO_MSG *p_gap_msg)
{
    T_LE_GAP_MSG gap_msg;
    uint8_t conn_id;
    memcpy(&gap_msg, &p_gap_msg->u.param, sizeof(p_gap_msg->u.param));

    APP_PRINT_TRACE1("bt_mesh_provisioner_app_handle_gap_msg: sub_type %d", p_gap_msg->subtype);
    mesh_inner_msg_t mesh_inner_msg;
    mesh_inner_msg.type = MESH_BT_STATUS_UPDATE;
    mesh_inner_msg.sub_type = p_gap_msg->subtype;
    mesh_inner_msg.parm = p_gap_msg->u.param;
    gap_sched_handle_bt_status_msg(&mesh_inner_msg);
    switch (p_gap_msg->subtype)
    {
    case GAP_MSG_LE_DEV_STATE_CHANGE:
        {
            //static bool start = FALSE;
            if (!mesh_initial_state)
            {
                mesh_initial_state = TRUE;
                /** set device uuid according to bt address */
                uint8_t bt_addr[6];
                uint8_t dev_uuid[16] = MESH_DEVICE_UUID;
                gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);
                memcpy(dev_uuid, bt_addr, sizeof(bt_addr));
                device_uuid_set(dev_uuid);
                /** configure provisioner */
                mesh_node.unicast_addr = bt_addr[0] % 99 + 1;
            }
            bt_mesh_provisioner_app_handle_dev_state_evt(gap_msg.msg_data.gap_dev_state_change.new_state,
                                     gap_msg.msg_data.gap_dev_state_change.cause);
        }
        break;

    case GAP_MSG_LE_CONN_STATE_CHANGE:
        {
            bt_mesh_provisioner_app_handle_conn_state_evt(gap_msg.msg_data.gap_conn_state_change.conn_id,
                                      (T_GAP_CONN_STATE)gap_msg.msg_data.gap_conn_state_change.new_state,
                                      gap_msg.msg_data.gap_conn_state_change.disc_cause);
        }
        break;

    case GAP_MSG_LE_CONN_MTU_INFO:
        {
            bt_mesh_provisioner_app_handle_conn_mtu_info_evt(gap_msg.msg_data.gap_conn_mtu_info.conn_id,
                                         gap_msg.msg_data.gap_conn_mtu_info.mtu_size);
        }
        break;

    case GAP_MSG_LE_CONN_PARAM_UPDATE:
        {
            bt_mesh_provisioner_app_handle_conn_param_update_evt(gap_msg.msg_data.gap_conn_param_update.conn_id,
                                             gap_msg.msg_data.gap_conn_param_update.status,
                                             gap_msg.msg_data.gap_conn_param_update.cause);
        }
        break;

    case GAP_MSG_LE_AUTHEN_STATE_CHANGE:
        {
            bt_mesh_provisioner_app_handle_authen_state_evt(gap_msg.msg_data.gap_authen_state.conn_id,
                                        gap_msg.msg_data.gap_authen_state.new_state,
                                        gap_msg.msg_data.gap_authen_state.status);
        }
        break;

    case GAP_MSG_LE_BOND_JUST_WORK:
        {
            conn_id = gap_msg.msg_data.gap_bond_just_work_conf.conn_id;
            le_bond_just_work_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
            APP_PRINT_INFO0("GAP_MSG_LE_BOND_JUST_WORK");
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_DISPLAY:
        {
            uint32_t display_value = 0;
            conn_id = gap_msg.msg_data.gap_bond_passkey_display.conn_id;
            le_bond_get_display_key(conn_id, &display_value);
            APP_PRINT_INFO2("GAP_MSG_LE_BOND_PASSKEY_DISPLAY: conn_id %d, passkey %d",
                            conn_id, display_value);
            le_bond_passkey_display_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
            printf("GAP_MSG_LE_BOND_PASSKEY_DISPLAY: conn_id %d, passkey %06d\r\n",
                            conn_id,
                            display_value);
        }
        break;

    case GAP_MSG_LE_BOND_USER_CONFIRMATION:
        {
            uint32_t display_value = 0;
            conn_id = gap_msg.msg_data.gap_bond_user_conf.conn_id;
            le_bond_get_display_key(conn_id, &display_value);
            APP_PRINT_INFO2("GAP_MSG_LE_BOND_USER_CONFIRMATION: conn_id %d, passkey %d",
                            conn_id, display_value);
            printf("GAP_MSG_LE_BOND_USER_CONFIRMATION: conn_id %d, passkey %06d\r\n",
                            conn_id,
                            display_value);
            //le_bond_user_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_INPUT:
        {
            //uint32_t passkey = 888888;
            conn_id = gap_msg.msg_data.gap_bond_passkey_input.conn_id;
            APP_PRINT_INFO2("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d, key_press %d",
                            conn_id, gap_msg.msg_data.gap_bond_passkey_input.key_press);
            printf("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d\r\n", conn_id);
            //le_bond_passkey_input_confirm(conn_id, passkey, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

#if F_BT_LE_SMP_OOB_SUPPORT
    case GAP_MSG_LE_BOND_OOB_INPUT:
        {
            uint8_t oob_data[GAP_OOB_LEN] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            conn_id = gap_msg.msg_data.gap_bond_oob_input.conn_id;
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_OOB_INPUT: conn_id %d", conn_id);
            le_bond_set_param(GAP_PARAM_BOND_OOB_DATA, GAP_OOB_LEN, oob_data);
            le_bond_oob_input_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;
#endif

    default:
        APP_PRINT_ERROR1("bt_mesh_provisioner_app_handle_gap_msg: unknown sub_type %d", p_gap_msg->subtype);
        break;
    }
}

/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT bt_mesh_provisioner_app_gap_callback(uint8_t cb_type, void *p_cb_data)
{
    T_APP_RESULT result = APP_RESULT_SUCCESS;
    T_LE_CB_DATA *p_data = (T_LE_CB_DATA *)p_cb_data;

    switch (cb_type)
    {
    /* common msg*/
    case GAP_MSG_LE_READ_RSSI:
        APP_PRINT_INFO3("GAP_MSG_LE_READ_RSSI:conn_id 0x%x cause 0x%x rssi %d",
                        p_data->p_le_read_rssi_rsp->conn_id,
                        p_data->p_le_read_rssi_rsp->cause,
                        p_data->p_le_read_rssi_rsp->rssi);
        break;

#if F_BT_LE_4_2_DATA_LEN_EXT_SUPPORT
    case GAP_MSG_LE_DATA_LEN_CHANGE_INFO:
        APP_PRINT_INFO3("GAP_MSG_LE_DATA_LEN_CHANGE_INFO: conn_id %d, tx octets 0x%x, max_tx_time 0x%x",
                        p_data->p_le_data_len_change_info->conn_id,
                        p_data->p_le_data_len_change_info->max_tx_octets,
                        p_data->p_le_data_len_change_info->max_tx_time);
        break;
#endif

    case GAP_MSG_LE_BOND_MODIFY_INFO:
        APP_PRINT_INFO1("GAP_MSG_LE_BOND_MODIFY_INFO: type 0x%x",
                        p_data->p_le_bond_modify_info->type);
        break;

    case GAP_MSG_LE_MODIFY_WHITE_LIST:
        APP_PRINT_INFO2("GAP_MSG_LE_MODIFY_WHITE_LIST: operation %d, cause 0x%x",
                        p_data->p_le_modify_white_list_rsp->operation,
                        p_data->p_le_modify_white_list_rsp->cause);
        break;
    /* central reference msg*/
    case GAP_MSG_LE_SCAN_INFO:
        APP_PRINT_INFO5("GAP_MSG_LE_SCAN_INFO:adv_type 0x%x, bd_addr %s, remote_addr_type %d, rssi %d, data_len %d",
                        p_data->p_le_scan_info->adv_type,
                        TRACE_BDADDR(p_data->p_le_scan_info->bd_addr),
                        p_data->p_le_scan_info->remote_addr_type,
                        p_data->p_le_scan_info->rssi,
                        p_data->p_le_scan_info->data_len);
        gap_sched_handle_adv_report(p_data->p_le_scan_info);
        break;

#if F_BT_LE_GAP_CENTRAL_SUPPORT
    case GAP_MSG_LE_CONN_UPDATE_IND:
        APP_PRINT_INFO5("GAP_MSG_LE_CONN_UPDATE_IND: conn_id %d, conn_interval_max 0x%x, conn_interval_min 0x%x, conn_latency 0x%x,supervision_timeout 0x%x",
                        p_data->p_le_conn_update_ind->conn_id,
                        p_data->p_le_conn_update_ind->conn_interval_max,
                        p_data->p_le_conn_update_ind->conn_interval_min,
                        p_data->p_le_conn_update_ind->conn_latency,
                        p_data->p_le_conn_update_ind->supervision_timeout);
        /* if reject the proposed connection parameter from peer device, use APP_RESULT_REJECT. */
        result = APP_RESULT_ACCEPT;
        break;

    case GAP_MSG_LE_SET_HOST_CHANN_CLASSIF:
        APP_PRINT_INFO1("GAP_MSG_LE_SET_HOST_CHANN_CLASSIF: cause 0x%x",
                        p_data->p_le_set_host_chann_classif_rsp->cause);
        break;
#endif
    /* peripheral reference msg*/
    case GAP_MSG_LE_ADV_UPDATE_PARAM:
        APP_PRINT_INFO1("GAP_MSG_LE_ADV_UPDATE_PARAM: cause 0x%x",
                        p_data->p_le_adv_update_param_rsp->cause);
        gap_sched_adv_params_set_done();
        break;
        /*
    case GAP_MSG_LE_VENDOR_ONE_SHOT_ADV:
        APP_PRINT_INFO1("GAP_MSG_LE_VENDOR_ONE_SHOT_ADV: cause 0x%x",
                        p_data->le_cause.cause);
        gap_sched_adv_done(GAP_SCHED_ADV_END_TYPE_SUCCESS);
        break;
    case GAP_MSG_LE_DISABLE_SLAVE_LATENCY:
        APP_PRINT_INFO1("GAP_MSG_LE_DISABLE_SLAVE_LATENCY: cause 0x%x",
                        p_data->p_le_disable_slave_latency_rsp->cause);
        break;

    case GAP_MSG_LE_UPDATE_PASSED_CHANN_MAP:
        APP_PRINT_INFO1("GAP_MSG_LE_UPDATE_PASSED_CHANN_MAP:cause 0x%x",
                        p_data->p_le_update_passed_chann_map_rsp->cause);
        break;
        */
#if F_BT_LE_5_0_SET_PHY_SUPPORT
    case GAP_MSG_LE_PHY_UPDATE_INFO:
        APP_PRINT_INFO4("GAP_MSG_LE_PHY_UPDATE_INFO:conn_id %d, cause 0x%x, rx_phy %d, tx_phy %d",
                        p_data->p_le_phy_update_info->conn_id,
                        p_data->p_le_phy_update_info->cause,
                        p_data->p_le_phy_update_info->rx_phy,
                        p_data->p_le_phy_update_info->tx_phy);
        break;

    case GAP_MSG_LE_REMOTE_FEATS_INFO:
        {
            uint8_t  remote_feats[8];
            APP_PRINT_INFO3("GAP_MSG_LE_REMOTE_FEATS_INFO: conn id %d, cause 0x%x, remote_feats %b",
                            p_data->p_le_remote_feats_info->conn_id,
                            p_data->p_le_remote_feats_info->cause,
                            TRACE_BINARY(8, p_data->p_le_remote_feats_info->remote_feats));
            if (p_data->p_le_remote_feats_info->cause == GAP_SUCCESS)
            {
                memcpy(remote_feats, p_data->p_le_remote_feats_info->remote_feats, 8);
                if (remote_feats[LE_SUPPORT_FEATURES_MASK_ARRAY_INDEX1] & LE_SUPPORT_FEATURES_LE_2M_MASK_BIT)
                {
                    APP_PRINT_INFO0("GAP_MSG_LE_REMOTE_FEATS_INFO: support 2M");
                }
                if (remote_feats[LE_SUPPORT_FEATURES_MASK_ARRAY_INDEX1] & LE_SUPPORT_FEATURES_LE_CODED_PHY_MASK_BIT)
                {
                    APP_PRINT_INFO0("GAP_MSG_LE_REMOTE_FEATS_INFO: support CODED");
                }
            }
        }
        break;
#endif
    default:
        APP_PRINT_ERROR1("bt_mesh_provisioner_app_gap_callback: unhandled cb_type 0x%x", cb_type);
        break;
    }
    return result;
}

/**
 * @brief  Callback will be called when data sent from profile client layer.
 * @param  client_id the ID distinguish which module sent the data.
 * @param  conn_id connection ID.
 * @param  p_data  pointer to data.
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT bt_mesh_provisioner_app_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data)
{
    T_APP_RESULT  result = APP_RESULT_SUCCESS;
    APP_PRINT_INFO2("bt_mesh_provisioner_app_client_callback: client_id %d, conn_id %d",
                    client_id, conn_id);
    if (client_id == CLIENT_PROFILE_GENERAL_ID)
    {
        T_CLIENT_APP_CB_DATA *p_client_app_cb_data = (T_CLIENT_APP_CB_DATA *)p_data;
        switch (p_client_app_cb_data->cb_type)
        {
        case CLIENT_APP_CB_TYPE_DISC_STATE:
            if (p_client_app_cb_data->cb_content.disc_state_data.disc_state == DISC_STATE_SRV_DONE)
            {
                APP_PRINT_INFO0("Discovery All Service Procedure Done.");
            }
            else
            {
                APP_PRINT_INFO0("Discovery state send to application directly.");
            }
            break;
        case CLIENT_APP_CB_TYPE_DISC_RESULT:
            if (p_client_app_cb_data->cb_content.disc_result_data.result_type == DISC_RESULT_ALL_SRV_UUID16)
            {
                APP_PRINT_INFO3("Discovery All Primary Service: UUID16 0x%x, start handle 0x%x, end handle 0x%x.",
                                p_client_app_cb_data->cb_content.disc_result_data.result_data.p_srv_uuid16_disc_data->uuid16,
                                p_client_app_cb_data->cb_content.disc_result_data.result_data.p_srv_uuid16_disc_data->att_handle,
                                p_client_app_cb_data->cb_content.disc_result_data.result_data.p_srv_uuid16_disc_data->end_group_handle);
            }
            else
            {
                APP_PRINT_INFO0("Discovery result send to application directly.");
            }
            break;
        default:
            break;
        }

    }
    else if (client_id == prov_client_id)
    {
        prov_client_cb_data_t *pcb_data = (prov_client_cb_data_t *)p_data;
        switch (pcb_data->cb_type)
        {
        case PROV_CLIENT_CB_TYPE_DISC_STATE:
            switch (pcb_data->cb_content.disc_state)
            {
            case PROV_DISC_DONE:
                /* Discovery Simple BLE service procedure successfully done. */
                printf("Prov service discovery end!\r\n");
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
                if (bt_mesh_indication(GEN_MESH_CODE(_prov_discover), BT_MESH_USER_CMD_SUCCESS, NULL) != USER_API_RESULT_OK) {
                    printf("[BT_MESH] %s(): user cmd %d fail !\r\n", __func__, GEN_MESH_CODE(_prov_discover));  
                }
#endif
                break;
            case PROV_DISC_FAIL:
                /* Discovery Request failed. */
                printf("Prov service discovery fail!\r\n");
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
                if (bt_mesh_indication(GEN_MESH_CODE(_prov_discover), BT_MESH_USER_CMD_FAIL, NULL) != USER_API_RESULT_OK) {
                    printf("[BT_MESH] %s(): user cmd %d fail !\r\n", __func__, GEN_MESH_CODE(_prov_discover));  
                }
#endif
                break;
            case PROV_DISC_NOT_FOUND:
                /* Discovery Request failed. */
                printf("Prov service not found!\r\n");
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
                if (bt_mesh_indication(GEN_MESH_CODE(_prov_discover), BT_MESH_USER_CMD_FAIL, NULL) != USER_API_RESULT_OK) {
                    printf("[BT_MESH] %s(): user cmd %d fail !\r\n", __func__, GEN_MESH_CODE(_prov_discover));  
                }
#endif
                break;
            default:
                break;
            }
            break;
        case PROV_CLIENT_CB_TYPE_READ_RESULT:
            switch (pcb_data->cb_content.read_result.type)
            {
            case PROV_READ_DATA_OUT_CCCD:
                if (pcb_data->cb_content.read_result.cause == 0)
                {
                    printf("Prov data out cccd = %d.\r\n",
                                    pcb_data->cb_content.read_result.data.prov_data_out_cccd);
                }
                break;
            default:
                break;
            }
            break;
        case PROV_CLIENT_CB_TYPE_WRITE_RESULT:
            break;
        default:
            break;
        }
    }
    else if (client_id == proxy_client_id)
    {
        proxy_client_cb_data_t *pcb_data = (proxy_client_cb_data_t *)p_data;
        switch (pcb_data->cb_type)
        {
        case PROXY_CLIENT_CB_TYPE_DISC_STATE:
            switch (pcb_data->cb_content.disc_state)
            {
            case PROXY_DISC_DONE:
                /* Discovery Simple BLE service procedure successfully done. */
                printf("Proxy service discovery end!\r\n");
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
                if (bt_mesh_indication(GEN_MESH_CODE(_proxy_discover), BT_MESH_USER_CMD_SUCCESS, NULL) != USER_API_RESULT_OK) {
                    printf("[BT_MESH] %s(): user cmd %d fail !\r\n", __func__, GEN_MESH_CODE(_proxy_discover));  
                }
#endif
                break;
            case PROXY_DISC_FAIL:
                /* Discovery Request failed. */
                printf("Proxy service discovery fail!\r\n");
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
                if (bt_mesh_indication(GEN_MESH_CODE(_proxy_discover), BT_MESH_USER_CMD_FAIL, NULL) != USER_API_RESULT_OK) {
                    printf("[BT_MESH] %s(): user cmd %d fail !\r\n", __func__, GEN_MESH_CODE(_proxy_discover));  
                }
#endif
                break;
            case PROXY_DISC_NOT_FOUND:
                /* Discovery Request failed. */
                printf("Proxy service not found!\r\n");
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
                if (bt_mesh_indication(GEN_MESH_CODE(_proxy_discover), BT_MESH_USER_CMD_FAIL, NULL) != USER_API_RESULT_OK) {
                    printf("[BT_MESH] %s(): user cmd %d fail !\r\n", __func__, GEN_MESH_CODE(_proxy_discover));  
                }
#endif
                break;
            default:
                break;
            }
            break;
        case PROXY_CLIENT_CB_TYPE_READ_RESULT:
            switch (pcb_data->cb_content.read_result.type)
            {
            case PROXY_READ_DATA_OUT_CCCD:
                if (pcb_data->cb_content.read_result.cause == 0)
                {
                    printf("Proxy data out cccd = %d.\r\n",
                                    pcb_data->cb_content.read_result.data.proxy_data_out_cccd);
                }
                break;
            default:
                break;
            }
            break;
        case PROXY_CLIENT_CB_TYPE_WRITE_RESULT:
            break;
        default:
            break;
        }
    }
    return result;
}

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
T_APP_RESULT bt_mesh_provisioner_app_profile_callback(T_SERVER_ID service_id, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    if (service_id == SERVICE_PROFILE_GENERAL_ID)
    {
        T_SERVER_APP_CB_DATA *p_param = (T_SERVER_APP_CB_DATA *)p_data;
        switch (p_param->eventId)
        {
        case PROFILE_EVT_SRV_REG_COMPLETE:// srv register result event.
            APP_PRINT_INFO1("PROFILE_EVT_SRV_REG_COMPLETE: result %d",
                            p_param->event_data.service_reg_result);
            break;

        case PROFILE_EVT_SEND_DATA_COMPLETE:
            APP_PRINT_INFO5("PROFILE_EVT_SEND_DATA_COMPLETE: conn_id %d, cause 0x%x, service_id %d, attrib_idx 0x%x, credits %d",
                            p_param->event_data.send_data_result.conn_id,
                            p_param->event_data.send_data_result.cause,
                            p_param->event_data.send_data_result.service_id,
                            p_param->event_data.send_data_result.attrib_idx,
                            p_param->event_data.send_data_result.credits);
            if (p_param->event_data.send_data_result.cause == GAP_SUCCESS)
            {
                APP_PRINT_INFO0("PROFILE_EVT_SEND_DATA_COMPLETE success");
            }
            else
            {
                APP_PRINT_ERROR0("PROFILE_EVT_SEND_DATA_COMPLETE failed");
            }
            break;

        default:
            break;
        }
    }
    return app_result;
}

/******************************************************************
 * @fn      device_info_cb
 * @brief   device_info_cb callbacks are handled in this function.
 *
 * @param   cb_data  -  @ref prov_cb_data_t
 * @return  void
 */
void device_info_cb(uint8_t bt_addr[6], uint8_t bt_addr_type, int8_t rssi, device_info_t *pinfo)
{
    if (!dev_info_show_flag)
    {
        return;
    }
    printf("bt addr=0x%02x%02x%02x%02x%02x%02x type=%d rssi=%d ", bt_addr[5], bt_addr[4],
                    bt_addr[3], bt_addr[2], bt_addr[1], bt_addr[0], bt_addr_type, rssi);
    switch (pinfo->type)
    {
    case DEVICE_INFO_UDB:
        printf("udb=");
        mesh_data_dump(pinfo->pbeacon_udb->dev_uuid, 16);
        break;
    case DEVICE_INFO_SNB:
        printf("snb=");
        mesh_data_dump(pinfo->pbeacon_snb->net_id, 8);
        break;
    case DEVICE_INFO_PROV_ADV:
        printf("prov=");
        mesh_data_dump(pinfo->pservice_data->provision.dev_uuid, 16);
        break;
    case DEVICE_INFO_PROXY_ADV:
        switch (pinfo->pservice_data->proxy.type)
        {
        case PROXY_ADV_TYPE_NET_ID:
            printf("proxy net id=");
            break;
        case PROXY_ADV_TYPE_NODE_IDENTITY:
            printf("proxy node id=");
            break;
#if F_BT_MESH_1_1_PRB_SUPPORT
        case PROXY_ADV_TYPE_PRIVATE_NET_ID:
            printf("proxy private net id=");
            break;
        case PROXY_ADV_TYPE_PRIVATE_NODE_IDENTITY:
            printf("proxy private node id=");
            break;
#endif
        default:
            printf("proxy unknown=");
            break;
        }
        mesh_data_dump((uint8_t *)&pinfo->pservice_data->proxy, pinfo->len);
        break;
    default:
        break;
    }
}

/******************************************************************
 * @fn      prov_cb
 * @brief   Provisioning callbacks are handled in this function.
 *
 * @param   cb_data  -  @ref prov_cb_data_t
 * @return  the operation result
 */
bool prov_cb(prov_cb_type_t cb_type, prov_cb_data_t cb_data)
{
    APP_PRINT_INFO3("prov_cb: type = %d, bearer type %d link %d", cb_type, cb_data.bearer.type,
                    cb_data.bearer.link);

    switch (cb_type)
    {
    case PROV_CB_TYPE_PB_ADV_LINK_STATE:
        switch (cb_data.pb_generic_cb_type)
        {
        case PB_GENERIC_CB_LINK_OPENED:
            printf("PB-ADV Link Opened!\r\n");
            send_coex_mailbox_to_wifi_from_BtAPP(0);
#if defined(CONFIG_EXAMPLE_BT_MESH_DEMO) && CONFIG_EXAMPLE_BT_MESH_DEMO
            if (bt_mesh_lib_priv.connect_device_sema) {
                bt_mesh_lib_priv.connect_device_flag = 1;
                rtw_up_sema(&bt_mesh_lib_priv.connect_device_sema);
            }
#endif
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
            if (bt_mesh_indication(GEN_MESH_CODE(_pb_adv_con), BT_MESH_USER_CMD_SUCCESS, NULL) != USER_API_RESULT_OK) {
                printf("[BT_MESH] %s(): user cmd %d fail !\r\n", __func__, GEN_MESH_CODE(_pb_adv_con));  
            }
#endif
            break;
        case PB_GENERIC_CB_LINK_OPEN_FAILED:
            printf("PB-ADV Link Open Failed!\r\n>");
            send_coex_mailbox_to_wifi_from_BtAPP(0);
#if defined(CONFIG_EXAMPLE_BT_MESH_DEMO) && CONFIG_EXAMPLE_BT_MESH_DEMO
            if (bt_mesh_lib_priv.connect_device_sema) {
                bt_mesh_lib_priv.connect_device_flag = 0;
                rtw_up_sema(&bt_mesh_lib_priv.connect_device_sema);
            }
#endif
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
            if (bt_mesh_indication(GEN_MESH_CODE(_pb_adv_con), BT_MESH_USER_CMD_FAIL, NULL) != USER_API_RESULT_OK) {
                printf("[BT_MESH] %s(): user cmd %d fail !\r\n", __func__, GEN_MESH_CODE(_pb_adv_con));  
            }
#endif
            break;
        case PB_GENERIC_CB_LINK_CLOSED:
            printf("PB-ADV Link Closed!\r\n");
            send_coex_mailbox_to_wifi_from_BtAPP(0);
            break;
        default:
            break;
        }
        break;
    case PROV_CB_TYPE_PATH_CHOOSE:
        {
            if (prov_manual)
            {
                /* use cmd "authpath" to select oob/no oob public key and no oob/static oob/input oob/output oob auth data according to the device capabilities */
                prov_capabilities_p pprov_capabilities = cb_data.pprov_capabilities;
                printf("prov capabilities: en-%d al-%d pk-%d so-%d os-%d oa-%d is-%d ia-%d\r\n",
                                pprov_capabilities->element_num, pprov_capabilities->algorithm,
                                pprov_capabilities->public_key, pprov_capabilities->static_oob,
                                pprov_capabilities->output_oob_size, pprov_capabilities->output_oob_action,
                                pprov_capabilities->input_oob_size, pprov_capabilities->input_oob_action);
            }
            else
            {
                /* select no oob public key and no oob auth data as default provision method */
                prov_start_t prov_start;
                memset(&prov_start, 0, sizeof(prov_start_t));
                prov_path_choose(&prov_start);
            }
        }
        break;
    case PROV_CB_TYPE_PUBLIC_KEY:
        {
            APP_PRINT_INFO0("prov_cb: get the public key from the device");
            uint8_t public_key[64] = {0xf4, 0x65, 0xe4, 0x3f, 0xf2, 0x3d, 0x3f, 0x1b, 0x9d, 0xc7, 0xdf, 0xc0, 0x4d, 0xa8, 0x75, 0x81, 0x84, 0xdb, 0xc9, 0x66, 0x20, 0x47, 0x96, 0xec, 0xcf, 0x0d, 0x6c, 0xf5, 0xe1, 0x65, 0x00, 0xcc, 0x02, 0x01, 0xd0, 0x48, 0xbc, 0xbb, 0xd8, 0x99, 0xee, 0xef, 0xc4, 0x24, 0x16, 0x4e, 0x33, 0xc2, 0x01, 0xc2, 0xb0, 0x10, 0xca, 0x6b, 0x4d, 0x43, 0xa8, 0xa1, 0x55, 0xca, 0xd8, 0xec, 0xb2, 0x79};
            prov_device_public_key_set(public_key);
        }
        break;
    case PROV_CB_TYPE_AUTH_DATA:
        {
            prov_start_p pprov_start = cb_data.pprov_start;
            prov_auth_value_type = prov_auth_value_type_get(pprov_start);
            /* use cmd to set auth data */
            printf("auth method=%d[nsoi] action=%d size=%d type=%d[nbNa]\r\n",
                            pprov_start->auth_method,
                            pprov_start->auth_action, pprov_start->auth_size, prov_auth_value_type);
            //uint8_t auth_data[16] = {1};
            switch (pprov_start->auth_method)
            {
            case PROV_AUTH_METHOD_STATIC_OOB:
                //prov_auth_value_set(auth_data, sizeof(auth_data));
                APP_PRINT_INFO0("prov_cb: Please share the oob data with the device");
                break;
            case PROV_AUTH_METHOD_OUTPUT_OOB:
                //prov_auth_value_set(auth_data, pprov_start->auth_size.output_oob_size);
                APP_PRINT_INFO2("prov_cb: Please input the oob data provided by the device, output size = %d, action = %d",
                                pprov_start->auth_size.output_oob_size, pprov_start->auth_action.output_oob_action);
                break;
            case PROV_AUTH_METHOD_INPUT_OOB:
                //prov_auth_value_set(auth_data, pprov_start->auth_size.input_oob_size);
                APP_PRINT_INFO2("prov_cb: Please output the oob data to the device, input size = %d, action = %d",
                                pprov_start->auth_size.input_oob_size, pprov_start->auth_action.input_oob_action);
                break;
            default:
                break;
            }
        }
        break;
    case PROV_CB_TYPE_CONF_CHECK:
        {
            /* check confirmation value when using OOB */
            uint8_t rand[16] = {0};
            uint8_t confirmation[16] = {0};
            prov_check_conf_t prov_check_conf = cb_data.prov_check_conf;

            memcpy((void *)rand, (void *)prov_check_conf.rand, sizeof(rand));
            memcpy((void *)confirmation, (void *)prov_check_conf.conf, sizeof(confirmation));
            printf("confimation %02x %02x %02x %02x %02x %02x \r\n", confirmation[0], confirmation[1], confirmation[2], confirmation[3], confirmation[4], confirmation[5]);
            printf("rand %02x %02x %02x %02x %02x %02x \r\n", rand[0], rand[1], rand[2], rand[3], rand[4], rand[5]);
            
            /* if confirmation from device is correct invoke prov_send_prov_data() */
            /* if confirmation from device is wrong invoke prov_reject() */
            // if (success) {
            //     prov_send_prov_data();
            // } else {
            //     prov_reject();
            // }
        }
        break;
    case PROV_CB_TYPE_COMPLETE:
        {
            prov_data_p pprov_data = cb_data.pprov_data;
#if F_BT_MESH_1_1_RPR_SUPPORT
            if (rmt_prov_client_link_state() == RMT_PROV_LINK_STATE_OUTBOUND_PKT_TRANS)
            {
                if (rmt_prov_client_procedure() == RMT_PROV_PROCEDURE_DKRI)
                {
                    rmt_prov_dkri_procedure_t dkri_procedure = rmt_prov_dkri_procedure();
                    uint32_t time = plt_time_read_ms() - prov_start_time;
                    switch (dkri_procedure)
                    {
                    case RMT_PROV_DKRI_DEV_KEY_REFRESH:
                        printf("Refresh device key in %dms!\r\n", time);
                        break;
                    case RMT_PROV_DKRI_NODE_ADDR_REFRESH:
                        printf("Refresh node address to 0x%04x in %dms!\r\n", pprov_data->unicast_address, time);
                        break;
                    case RMT_PROV_DKRI_NODE_COMPO_REFRESH:
                        printf("Refresh node composition data in %dms!\r\n", time);
                        break;
                    default:
                        printf("Unknown dkri procedure %d done in %dms!\r\n", dkri_procedure, time);
                        break;
                    }
                }
                else
                {
                    printf("Has provisioned device with addr 0x%04x in %dms!\r\n", pprov_data->unicast_address,
                                    plt_time_read_ms() - prov_start_time);
                }
                /* the spec requires to disconnect, but you can remove it as you like! :) */
                rmt_prov_link_close(RMT_PROV_LINK_CLOSE_SUCCESS);
            }
            else 
#endif
            {
                printf("Has provisioned device with addr 0x%04x in %dms!\r\n", pprov_data->unicast_address,
                                plt_time_read_ms() - prov_start_time);
                /* the spec requires to disconnect, but you can remove it as you like! :) */
                prov_disconnect(PB_ADV_LINK_CLOSE_SUCCESS);
            }
#if defined(CONFIG_EXAMPLE_BT_MESH_DEMO) && CONFIG_EXAMPLE_BT_MESH_DEMO
            if (bt_mesh_lib_priv.connect_device_sema) {
                bt_mesh_lib_priv.connect_device_mesh_addr = pprov_data->unicast_address;
                rtw_up_sema(&bt_mesh_lib_priv.connect_device_sema);
            }
#endif
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
            printf("\r\n %s() pprov_data->unicast_address = %x\r\n",__func__,pprov_data->unicast_address);
            if (bt_mesh_indication(GEN_MESH_CODE(_prov), BT_MESH_USER_CMD_SUCCESS, (void *)&pprov_data->unicast_address) != USER_API_RESULT_OK) {
                printf("[BT_MESH] %s(): user cmd %d fail !\r\n", __func__, GEN_MESH_CODE(_prov));  
            }
#endif
        }
        break;
    case PROV_CB_TYPE_FAIL:
        printf("provision fail, type=%d!\r\n", cb_data.prov_fail.fail_type);
#if defined(CONFIG_EXAMPLE_BT_MESH_DEMO) && CONFIG_EXAMPLE_BT_MESH_DEMO
        if (bt_mesh_lib_priv.connect_device_sema) {
            bt_mesh_lib_priv.connect_device_mesh_addr = 0;
            rtw_up_sema(&bt_mesh_lib_priv.connect_device_sema);
        }
#endif
#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
        if (bt_mesh_indication(GEN_MESH_CODE(_prov), BT_MESH_USER_CMD_FAIL, NULL) != USER_API_RESULT_OK) {
            printf("[BT_MESH] %s(): user cmd %d fail !\r\n", __func__, GEN_MESH_CODE(_prov));  
        }
#endif

        break;
    case PROV_CB_TYPE_PROV:
        /* stack ready */
        printf("ms addr: 0x%04x\r\n", mesh_node.unicast_addr);
        break;
    default:
        break;
    }
    return true;
}

/******************************************************************
 * @fn      fn_cb
 * @brief   fn callbacks are handled in this function.
 *
 * @param   frnd_index
 * @param   type
 * @param   fn_addr
 * @return  void
 */
void fn_cb(uint8_t frnd_index, fn_cb_type_t type, uint16_t lpn_addr)
{
    /* avoid gcc compile warning */
    (void)frnd_index;
    
    char *string[] = {"establishing with lpn 0x%04x\r\n", "no poll from 0x%04x\r\n", "established with lpn 0x%04x\r\n", "lpn 0x%04x lost\r\n"};
    printf(string[type], lpn_addr);
    if (type == FN_CB_TYPE_ESTABLISH_SUCCESS || type == FN_CB_TYPE_FRND_LOST)
    {
        user_cmd_time(NULL);
    }
}

/******************************************************************
 * @fn      hb_cb
 * @brief   heartbeat callbacks are handled in this function.
 *
 * @param   type
 * @param   pdata
 * @return  void
 */
void hb_cb(hb_data_type_t type, void *pargs)
{
    switch (type)
    {
    case HB_DATA_PUB_TIMER_STATE:
        {
            hb_data_timer_state_t *pdata = pargs;
            if (HB_TIMER_STATE_START == pdata->state)
            {
                printf("heartbeat publish timer start, period = %d\r\n", pdata->period);
            }
            else
            {
                printf("heartbeat publish timer stop\r\n");
            }
        }
        break;
    case HB_DATA_SUB_TIMER_STATE:
        {
            hb_data_timer_state_t *pdata = pargs;
            if (HB_TIMER_STATE_START == pdata->state)
            {
                printf("heartbeat subscription timer start, period = %d\r\n", pdata->period);
            }
            else
            {
                printf("heartbeat subscription timer stop\r\n");
            }
        }
        break;
    case HB_DATA_PUB_COUNT_UPDATE:
        {
            hb_data_pub_count_update_t *pdata = pargs;
            printf("heartbeat publish count update: %d\r\n", pdata->count);
        }
        break;
    case HB_DATA_SUB_PERIOD_UPDATE:
        {
            hb_data_sub_period_update_t *pdata = pargs;
            printf("heartbeat subscription period update: %d\r\n", pdata->period);
        }
        break;
    case HB_DATA_SUB_RECEIVE:
        {
            hb_data_sub_receive_t *pdata = pargs;
            printf("receive heartbeat: src = %d, init_ttl = %d, features = %d-%d-%d-%d, ttl = %d\r\n",
                            pdata->src, pdata->init_ttl, pdata->features.relay, pdata->features.proxy,
                            pdata->features.frnd, pdata->features.lpn, pdata->ttl);
        }
        break;
    default:
        break;
    }
}

void bt_mesh_provisioner_app_vendor_callback(uint8_t cb_type, void *p_cb_data)
{
    T_GAP_VENDOR_CB_DATA cb_data;
    memcpy(&cb_data, p_cb_data, sizeof(T_GAP_VENDOR_CB_DATA));
    APP_PRINT_INFO1("bt_mesh_provisioner_app_vendor_callback: command 0x%x", cb_data.p_gap_vendor_cmd_rsp->command);
    switch (cb_type)
    {
    case GAP_MSG_VENDOR_CMD_RSP:
        switch(cb_data.p_gap_vendor_cmd_rsp->command)
        { 
#if BT_VENDOR_CMD_ONE_SHOT_SUPPORT
            case HCI_LE_VENDOR_EXTENSION_FEATURE2:
                //if(cb_data.p_gap_vendor_cmd_rsp->param[0] == HCI_EXT_SUB_ONE_SHOT_ADV)
                {
                    APP_PRINT_INFO1("One shot adv resp: cause 0x%x", cb_data.p_gap_vendor_cmd_rsp->cause);
                    gap_sched_adv_done(GAP_SCHED_ADV_END_TYPE_SUCCESS);
                }
                break;
#endif
            case HCI_LE_VENDOR_EXTENSION_FEATURE:
                switch(cb_data.p_gap_vendor_cmd_rsp->param[0])
                {
#if BT_VENDOR_CMD_ADV_TX_POWER_SUPPORT
                    case HCI_EXT_SUB_SET_ADV_TX_POWER:
                        APP_PRINT_INFO1("HCI_EXT_SUB_SET_ADV_TX_POWER: cause 0x%x", cb_data.p_gap_vendor_cmd_rsp->cause);
                        break;
#endif
#if BT_VENDOR_CMD_CONN_TX_POWER_SUPPORT
                    case HCI_EXT_SUB_SET_LINK_TX_POW:
                        APP_PRINT_INFO1("HCI_EXT_SUB_SET_LINK_TX_POW: cause 0x%x", cb_data.p_gap_vendor_cmd_rsp->cause);
                        break;
#endif
                }
                break;
            default:
                break;
        }
        break;

    default:
        break;
    }

    return;
}

#if F_BT_MESH_1_1_DF_SUPPORT
/******************************************************************
 * @fn      df_cb
 * @brief   df callbacks are handled in this function.
 *
 * @param   type
 * @param   pdata
 * @return  result
 */
uint16_t df_cb(uint8_t type, void *pdata)
{
    switch (type)
    {
    case MESH_MSG_DF_PATH_ACTION:
        {
            df_path_action_t *paction = (df_path_action_t *)pdata;
            char *state_str[] = {"discovering", "discovery failed", "established", "path released"};
            char *path_str =
                "master key index %d\r\nsrc 0x%04x(%d) [0x%04x(%d)] --> dst 0x%04x(%d) [0x%04x(%d)]\r\n";
            char info[120];
            sprintf(info, "%s, %s", state_str[paction->action_type], path_str);
            printf(info, paction->master_key_index,
                            paction->path_src, paction->path_src_sec_elem_num,
                            paction->dp_src, paction->dp_src_sec_elem_num,
                            paction->path_dst, paction->path_dst_sec_elem_num,
                            paction->dp_dst, paction->dp_dst_sec_elem_num);
            if (paction->action_type == DF_PATH_ACTION_TYPE_DISCOVERING ||
                paction->action_type == DF_PATH_ACTION_TYPE_DISCOVERY_FAIL ||
                paction->action_type == DF_PATH_ACTION_TYPE_ESTABLISHED ||
                paction->action_type == DF_PATH_ACTION_TYPE_PATH_RELEASE)
            {
                user_cmd_time(NULL);
            }
        }
        break;
    default:
        break;
    }

    return 0x00;
}
#endif

#if F_BT_MESH_1_1_MBT_SUPPORT
/******************************************************************
 * @fn      blob_client_cb
 * @brief   blob client callbacks are handled in this function.
 *
 * @param   pcb_data
 * @return  void
 */
void blob_client_cb(blob_transfer_cb_data_t *pcb_data)
{
    char *procedure_str[] = {"caps retrieve", "transfer", "cancel"};
    char *type_str[] = {"node fail", "success", "fail", "progress"};
    char info[30];
    sprintf(info, "%s, %s\r\n", procedure_str[pcb_data->procedure], type_str[pcb_data->type]);
    printf(info);
    printf("client phase %d\r\n", pcb_data->client_phase);

    if (pcb_data->type == BLOB_CB_TYPE_NODE_FAIL)
    {
        printf("addr 0x%04x", pcb_data->addr);
    }
    else if (pcb_data->type == BLOB_CB_TYPE_PROGRESS)
    {
        printf("---------- %d%% ---------", pcb_data->progress);
    }

    printf("\r\n");
}
#endif

#if F_BT_MESH_1_1_DFU_SUPPORT
/******************************************************************
 * @fn      dfu_dist_cb
 * @brief   dfu distributor callbacks are handled in this function.
 *
 * @param   pcb_data
 * @return  void
 */
void dfu_dist_cb(dfu_dist_cb_data_t *pcb_data)
{
    char *type_str[] = {"node fail", "transfer progress", "transfer success", "transfer fail", "verify", "complete"};
    printf("type %s, client phase %d\r\n", type_str[pcb_data->type], pcb_data->dist_phase);

    if (pcb_data->type == DFU_DIST_CB_TYPE_NODE_FAIL)
    {
        printf("addr 0x%04x", pcb_data->paddr[0]);
    }
    else if (pcb_data->type == DFU_DIST_CB_TYPE_TRANSFER_PROGRESS)
    {
        printf("---------- %d%% ---------", pcb_data->progress);
    }
    else if (pcb_data->type == DFU_DIST_CB_TYPE_TRANSFER_SUCCESS ||
             pcb_data->type == DFU_DIST_CB_TYPE_VERIFY ||
             pcb_data->type == DFU_DIST_CB_TYPE_COMPLETE)
    {
        uint8_t i = 0;
        while (i < pcb_data->addr_num)
        {
            printf("0x%04x ", pcb_data->paddr[i++]);
        }
    }

    printf("\r\n");
}

/******************************************************************
 * @fn      dfu_init_cb
 * @brief   dfu initiator callbacks are handled in this function.
 *
 * @param   pcb_data
 * @return  void
 */
void dfu_init_cb(dfu_init_cb_data_t *pcb_data)
{
    char *type_str[] = {"upload progress", "upload success", "upload fail"};
    printf("type %s, client phase %d\r\n", type_str[pcb_data->type], pcb_data->init_phase);

    if (pcb_data->type == DFU_INIT_CB_TYPE_UPLOAD_PROGRESS)
    {
        printf("---------- %d%% ---------", pcb_data->progress);
    }
    else if (pcb_data->type == DFU_INIT_CB_TYPE_UPLOAD_SUCCESS ||
             pcb_data->type == DFU_INIT_CB_TYPE_UPLOAD_FAIL)
    {
        printf("0x%04x ", pcb_data->addr);
    }

    printf("\r\n");
}
#endif
