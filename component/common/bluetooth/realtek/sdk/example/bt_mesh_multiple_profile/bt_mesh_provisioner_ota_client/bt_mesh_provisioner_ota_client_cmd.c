/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     provisioner_cmd.c
  * @brief    Source file for provisioner cmd.
  * @details  User command interfaces.
  * @author   bill
  * @date     2017-3-31
  * @version  v1.0
  * *************************************************************************************
  */
#include <platform_opts_bt.h>
#if defined(CONFIG_BT_MESH_PROVISIONER_OTA_CLIENT) && CONFIG_BT_MESH_PROVISIONER_OTA_CLIENT
#include <string.h>
#include "trace.h"
#include "gap_wrapper.h"
#include "provisioner_cmd.h"
#include "provisioner_app.h"
#include "provision_client.h"
#include "provision_provisioner.h"
#include "mesh_api.h"
#include "mesh_cmd.h"
#include "test_cmd.h"
#include "client_cmd.h"
#include "generic_client_app.h"
#include "light_client_app.h"
#include "datatrans_model.h"
#include "datatrans_app.h"
#include "remote_provisioning.h"

extern bool aes128_ecb_encrypt(uint8_t *input, const uint8_t *key, uint8_t *output);

static user_cmd_parse_result_t user_cmd_prov_discover(user_cmd_parse_value_t *pparse_value)
{
    printf("Prov Start Discover\r\n");
    uint8_t conn_id = pparse_value->dw_parameter[0];
    prov_client_start_discovery(conn_id);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_prov_read_char(user_cmd_parse_value_t *pparse_value)
{
    /* Indicate which char to be read. */
    prov_read_type_t read_char_type = (prov_read_type_t)pparse_value->dw_parameter[0];
    /* Read by handle or UUID, 1--by UUID, 0--by handle. */
    uint8_t read_pattern = (uint8_t)pparse_value->dw_parameter[1];
    uint8_t conn_id = pparse_value->dw_parameter[2];
    printf("Pro Read Char\r\n");
    if (read_pattern)
    {
        prov_client_read_by_uuid(conn_id, read_char_type);
    }
    else
    {
        prov_client_read_by_handle(conn_id, read_char_type);
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_prov_cccd_operate(user_cmd_parse_value_t *pparse_value)
{
    /* Indicate which char CCCD command. */
    uint8_t cmd_type = (uint8_t)pparse_value->dw_parameter[0];
    /* Enable or disable, 1--enable, 0--disable. */
    bool cmd_data = (bool)pparse_value->dw_parameter[1];
    gap_sched_link_t link = pparse_value->dw_parameter[2];
    printf("Prov Cccd Operate\r\n");
    switch (cmd_type)
    {
    case 0:/* V3 Notify char notif enable/disable. */
        {
            prov_client_data_out_cccd_set(link, cmd_data);
            uint8_t ctx_id = prov_service_alloc_proxy_ctx(link);
            if (ctx_id == MESH_PROXY_PROTOCOL_RSVD_CTX_ID)
            {
                printf("fail, no proxy resource for prov\r\n");
            }
        }
        break;
    default:
        return USER_CMD_RESULT_WRONG_PARAMETER;
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_prov_list(user_cmd_parse_value_t *pparse_value)
{
    uint8_t conn_id = pparse_value->dw_parameter[0];
    printf("Prov Server Handle List: link %d\r\nidx\thandle\r\n", conn_id);
    for (prov_handle_type_t hdl_idx = HDL_PROV_SRV_START; hdl_idx < HDL_PROV_CACHE_LEN; hdl_idx++)
    {
        printf("%d\t0x%x\r\n", hdl_idx, prov_client_handle_get(conn_id, hdl_idx));
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_pb_adv_con(user_cmd_parse_value_t *pparse_value)
{
    uint8_t dev_uuid[16];
    
    send_coex_mailbox_to_wifi_from_BtAPP(1);
    plt_hex_to_bin(dev_uuid, (uint8_t *)pparse_value->pparameter[0], sizeof(dev_uuid));
    if (!pb_adv_link_open(0, dev_uuid))
    {
        printf("PB_ADV: Link Busy!\r\n");

    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_pb_adv_disc(user_cmd_parse_value_t *pparse_value)
{
    /* avoid gcc compile warning */
    (void)pparse_value;
    
    if (pb_adv_link_close(0, PB_ADV_LINK_CLOSE_SUCCESS))
    {
        printf("PB_ADV: Link Closed!\r\n");
    }
    else
    {
        printf("PB_ADV: Link Closed Already!\r\n");
    }
    return USER_CMD_RESULT_OK;
}

#if F_BT_MESH_1_1_RPR_SUPPORT
static user_cmd_parse_result_t user_cmd_rmt_prov_client_scan_start(user_cmd_parse_value_t *pparse_value)
{
    uint8_t dev_uuid[16];
    uint16_t dst = pparse_value->dw_parameter[0];
    uint8_t net_key_index = pparse_value->dw_parameter[1];
    uint8_t scanned_items_limit = pparse_value->dw_parameter[2];
    uint8_t scan_timeout = pparse_value->dw_parameter[3];
    uint8_t ret = 0;

    if (pparse_value->para_count == 5) {
        plt_hex_to_bin(dev_uuid, (uint8_t *)pparse_value->pparameter[4], sizeof(dev_uuid));
        ret = rmt_prov_client_scan_start(dst, net_key_index, scanned_items_limit, scan_timeout, dev_uuid);
    } else if (pparse_value->para_count == 4) {
        ret = rmt_prov_client_scan_start(dst, net_key_index, scanned_items_limit, scan_timeout, NULL);
    } else {
        printf("invalid param\r\n");
        return USER_CMD_RESULT_WRONG_PARAMETER;
    }
    if (ret == MESH_MSG_SEND_CAUSE_SUCCESS) {
        return USER_CMD_RESULT_OK;
    } else {
        printf("fail\r\n");
        return USER_CMD_RESULT_ERROR;
    }   
}

static user_cmd_parse_result_t user_cmd_rmt_prov_client_link_open_prov(user_cmd_parse_value_t *pparse_value)
{
    uint8_t dev_uuid[16];
    uint16_t dst = pparse_value->dw_parameter[0];
    uint8_t net_key_index = pparse_value->dw_parameter[1];
    uint8_t link_open_timeout = pparse_value->dw_parameter[3];
    
    if (pparse_value->para_count == 4) {
        plt_hex_to_bin(dev_uuid, (uint8_t *)pparse_value->pparameter[2], sizeof(dev_uuid));
    } else {
        printf("invalid uuid\r\n");
        return USER_CMD_RESULT_WRONG_PARAMETER;
    }
    return rmt_prov_client_link_open_prov(dst, net_key_index, dev_uuid, link_open_timeout) == MESH_MSG_SEND_CAUSE_SUCCESS ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
}

static user_cmd_parse_result_t user_cmd_rmt_prov_client_close(user_cmd_parse_value_t *pparse_value)
{
	rmt_prov_link_close_reason_t reason;
    uint16_t dst = pparse_value->dw_parameter[0];
    uint8_t net_key_index = pparse_value->dw_parameter[1];

    if (pparse_value->para_count == 3) {
        reason = (rmt_prov_link_close_reason_t)pparse_value->dw_parameter[2];
    } else {
        printf("invalid reason\r\n");
        return USER_CMD_RESULT_WRONG_PARAMETER;
    }

	return rmt_prov_client_link_close(dst, net_key_index, reason) == MESH_MSG_SEND_CAUSE_SUCCESS ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
}
#endif

static user_cmd_parse_result_t user_cmd_prov(user_cmd_parse_value_t *pparse_value)
{
    printf("provision...\r\n");
    uint32_t attn_dur = pparse_value->dw_parameter[0];
    prov_manual = pparse_value->dw_parameter[1];
    prov_start_time = plt_time_read_ms();
    return prov_invite(attn_dur) ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
}

static user_cmd_parse_result_t user_cmd_prov_stop(user_cmd_parse_value_t *pparse_value)
{
    /* avoid gcc compile warning */
    (void)pparse_value;
    
    return prov_reject() ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
}

static user_cmd_parse_result_t user_cmd_prov_auth_path(user_cmd_parse_value_t *pparse_value)
{
    prov_start_algorithm_t algo = (prov_start_algorithm_t)pparse_value->dw_parameter[0];
    prov_start_public_key_t public_key = (prov_start_public_key_t)pparse_value->dw_parameter[1];
    prov_auth_method_t auth_method = (prov_auth_method_t)pparse_value->dw_parameter[2];
    uint8_t oob_action = pparse_value->dw_parameter[3];
    uint8_t oob_size = pparse_value->dw_parameter[4];

    prov_start_t prov_start;
    prov_start.algorithm = algo;
    prov_start.public_key = public_key;
    prov_start.auth_method = auth_method;
    prov_start.auth_action.oob_action = oob_action;
    prov_start.auth_size.oob_size = oob_size;
    return prov_path_choose(&prov_start) == true ? USER_CMD_RESULT_OK : USER_CMD_RESULT_WRONG_PARAMETER;
}

static user_cmd_parse_result_t user_cmd_unprov(user_cmd_parse_value_t *pparse_value)
{
    /* avoid gcc compile warning */
    (void)pparse_value;
    
    printf("Unprovision...\r\n");
#if MESH_UNPROVISIONING_SUPPORT
    prov_unprovisioning();
#endif
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_cfg_client_key_set(user_cmd_parse_value_t
                                                           *pparse_value)
{
    uint8_t key_index = pparse_value->dw_parameter[0];
    bool use_app_key = pparse_value->dw_parameter[1] ? TRUE : FALSE;
    mesh_node.features.cfg_model_use_app_key = use_app_key;
    return cfg_client_key_set(key_index) ? USER_CMD_RESULT_OK :
           USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
}

static user_cmd_parse_result_t user_cmd_compo_data_get(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    uint8_t page = pparse_value->dw_parameter[1];
    if (MESH_IS_UNASSIGNED_ADDR(dst))
    {
        printf("CDP0 len=%d, data=", mesh_node.compo_data_size[0]);
        mesh_data_dump(mesh_node.compo_data[0], mesh_node.compo_data_size[0]);
    }
    else
    {
        cfg_compo_data_get(dst, page);
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_node_reset(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    bool clear = pparse_value->dw_parameter[1];
    if (MESH_IS_UNASSIGNED_ADDR(dst))
    {
        mesh_node_reset();
    }
    else
    {
        cfg_node_reset(dst);
        if (clear)
        {
            int dev_key_index = dev_key_find(dst);
            if (dev_key_index >= 0)
            {
                dev_key_delete(dev_key_index);
                mesh_flash_store(MESH_FLASH_PARAMS_DEV_KEY, &dev_key_index);
            }
        }
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_net_key_add(user_cmd_parse_value_t *pparse_value)
{
    if (pparse_value->para_count != 2)
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    uint16_t dst = pparse_value->dw_parameter[0];
    uint16_t net_key_index = pparse_value->dw_parameter[1];
    if (net_key_index >= mesh_node.net_key_num ||
        mesh_node.net_key_list[net_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    cfg_net_key_add(dst, mesh_node.net_key_list[net_key_index].net_key_index_g,
                    mesh_node.net_key_list[net_key_index].pnet_key[key_state_to_new_loop(
                                                                       mesh_node.net_key_list[net_key_index].key_state)]->net_key);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_net_key_update(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    uint16_t net_key_index = pparse_value->dw_parameter[1];
    if (net_key_index >= mesh_node.net_key_num ||
        mesh_node.net_key_list[net_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    if (MESH_IS_UNASSIGNED_ADDR(dst))
    {
        if (pparse_value->para_count != 3)
        {
            return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
        }
        uint8_t net_key[MESH_COMMON_KEY_SIZE];
        plt_hex_to_bin(net_key, (uint8_t *)pparse_value->pparameter[2], MESH_COMMON_KEY_SIZE);
        return net_key_update(net_key_index, mesh_node.net_key_list[net_key_index].net_key_index_g,
                              net_key) ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
    }
    else
    {
        cfg_net_key_update(dst, mesh_node.net_key_list[net_key_index].net_key_index_g,
                           mesh_node.net_key_list[net_key_index].pnet_key[key_state_to_new_loop(
                                                                              mesh_node.net_key_list[net_key_index].key_state)]->net_key);
        return USER_CMD_RESULT_OK;
    }
}

static user_cmd_parse_result_t user_cmd_app_key_add(user_cmd_parse_value_t *pparse_value)
{
    if (pparse_value->para_count != 3)
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    uint16_t dst = pparse_value->dw_parameter[0];
    uint16_t net_key_index = pparse_value->dw_parameter[1];
    uint16_t app_key_index = pparse_value->dw_parameter[2];
    if (net_key_index >= mesh_node.net_key_num ||
        mesh_node.net_key_list[net_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    if (app_key_index >= mesh_node.app_key_num ||
        mesh_node.app_key_list[app_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    printf("%s() dst = %x\r\n",__func__,dst);
    cfg_app_key_add(dst, mesh_node.net_key_list[net_key_index].net_key_index_g,
                    mesh_node.app_key_list[app_key_index].app_key_index_g,
                    mesh_node.app_key_list[app_key_index].papp_key[key_state_to_new_loop(
                                                                       mesh_node.app_key_list[app_key_index].key_state)]->app_key);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_app_key_update(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    uint16_t net_key_index = pparse_value->dw_parameter[1];
    uint16_t app_key_index = pparse_value->dw_parameter[2];
    if (net_key_index >= mesh_node.net_key_num ||
        mesh_node.net_key_list[net_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    if (app_key_index >= mesh_node.app_key_num ||
        mesh_node.app_key_list[app_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    if (MESH_IS_UNASSIGNED_ADDR(dst))
    {
        if (pparse_value->para_count != 4)
        {
            return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
        }
        uint8_t app_key[MESH_COMMON_KEY_SIZE];
        plt_hex_to_bin(app_key, (uint8_t *)pparse_value->pparameter[3], MESH_COMMON_KEY_SIZE);
        return app_key_update(app_key_index, net_key_index,
                              mesh_node.app_key_list[app_key_index].app_key_index_g,
                              app_key) ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
    }
    else
    {
        cfg_app_key_update(dst, mesh_node.net_key_list[net_key_index].net_key_index_g,
                           mesh_node.app_key_list[app_key_index].app_key_index_g,
                           mesh_node.app_key_list[app_key_index].papp_key[key_state_to_new_loop(
                                                                              mesh_node.app_key_list[app_key_index].key_state)]->app_key);
        return USER_CMD_RESULT_OK;
    }
}

static user_cmd_parse_result_t user_cmd_model_app_bind(user_cmd_parse_value_t *pparse_value)
{
    if (pparse_value->para_count != 4)
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    uint16_t dst = pparse_value->dw_parameter[0];
    uint8_t element_index = pparse_value->dw_parameter[1];
    uint32_t model_id = pparse_value->dw_parameter[2];
    uint16_t app_key_index = pparse_value->dw_parameter[3];
    if (app_key_index >= mesh_node.app_key_num ||
        mesh_node.app_key_list[app_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    cfg_model_app_bind(dst, dst + element_index, mesh_node.app_key_list[app_key_index].app_key_index_g,
                       model_id);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_model_pub_set(user_cmd_parse_value_t *pparse_value)
{
    if (pparse_value->para_count != 11)
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    uint16_t dst = pparse_value->dw_parameter[0];
    pub_key_info_t pub_key_info = {.app_key_index = pparse_value->dw_parameter[4], .frnd_flag = pparse_value->dw_parameter[5], 0};
    pub_period_t pub_period = {pparse_value->dw_parameter[7] & 0x3f, pparse_value->dw_parameter[7] >> 6};
    pub_retrans_info_t pub_retrans_info = {pparse_value->dw_parameter[8], pparse_value->dw_parameter[9]};
    uint8_t addr[16];
    if (0 == pparse_value->dw_parameter[2])
    {
        uint16_t element_addr = (uint16_t)(pparse_value->dw_parameter[3]);
        LE_WORD2EXTRN(addr, element_addr);
    }
    else
    {
        plt_hex_to_bin(addr, (uint8_t *)pparse_value->pparameter[3], 16);
    }
    cfg_model_pub_set(dst, pparse_value->dw_parameter[1], pparse_value->dw_parameter[2],
                      addr, pub_key_info, pparse_value->dw_parameter[6], pub_period,
                      pub_retrans_info, pparse_value->dw_parameter[10]);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_model_sub_add(user_cmd_parse_value_t *pparse_value)
{
    if (pparse_value->para_count != 4)
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    uint16_t dst = pparse_value->dw_parameter[0];
    uint8_t element_index = pparse_value->dw_parameter[1];
    uint32_t model_id = pparse_value->dw_parameter[2];
    uint16_t group_addr = pparse_value->dw_parameter[3];
    if (MESH_NOT_GROUP_ADDR(group_addr))
    {
        return USER_CMD_RESULT_WRONG_PARAMETER;
    }
    cfg_model_sub_add(dst, dst + element_index, false, (uint8_t *)&group_addr, model_id);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_model_sub_get(user_cmd_parse_value_t *pparse_value)
{
    if (pparse_value->para_count != 3)
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    uint16_t dst = pparse_value->dw_parameter[0];
    uint8_t element_index = pparse_value->dw_parameter[1];
    uint32_t model_id = pparse_value->dw_parameter[2];
    cfg_model_sub_get(dst, dst + element_index, model_id);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_model_sub_delete(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    uint8_t element_index = pparse_value->dw_parameter[1];
    uint32_t model_id = pparse_value->dw_parameter[2];
    uint16_t group_addr = pparse_value->dw_parameter[3];
    if (pparse_value->para_count == 4)
    {
        if (MESH_NOT_GROUP_ADDR(group_addr))
        {
            return USER_CMD_RESULT_WRONG_PARAMETER;
        }
        cfg_model_sub_delete(dst, dst + element_index, false, (uint8_t *)&group_addr, model_id);
    }
    else if (pparse_value->para_count == 3)
    {
        cfg_model_sub_delete_all(dst, dst + element_index, model_id);
    }
    else
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_key_refresh_phase_get(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    uint16_t net_key_index = pparse_value->dw_parameter[1];
    cfg_key_refresh_phase_get(dst, net_key_index_to_global(net_key_index));
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_key_refresh_phase_set(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    uint16_t net_key_index = pparse_value->dw_parameter[1];
    uint8_t phase = pparse_value->dw_parameter[2];
    cfg_key_refresh_phase_set(dst, net_key_index_to_global(net_key_index), phase);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_datatrans_write(user_cmd_parse_value_t *pparse_value)
{
    uint8_t para_count = pparse_value->para_count;
    uint8_t data[18];

    for (uint8_t i = 0; i < para_count - 3; ++i)
    {
        data[i] = pparse_value->dw_parameter[i + 1];
    }
    datatrans_write(&datatrans, pparse_value->dw_parameter[0],
                    pparse_value->dw_parameter[para_count - 2], para_count - 3, data,
                    pparse_value->dw_parameter[para_count - 1]);

    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_datatrans_read(user_cmd_parse_value_t *pparse_value)
{
    datatrans_read(&datatrans, pparse_value->dw_parameter[0],
                   pparse_value->dw_parameter[2], pparse_value->dw_parameter[1]);

    return USER_CMD_RESULT_OK;
}

#if F_BT_MESH_1_1_RPR_SUPPORT
static user_cmd_parse_result_t user_cmd_rmt_prov_scan_capabilities_get(
    user_cmd_parse_value_t *pparse_value)
{
    rmt_prov_client_scan_caps_get(pparse_value->dw_parameter[0], pparse_value->dw_parameter[1]);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_rmt_prov_scan_get(user_cmd_parse_value_t *pparse_value)
{
    rmt_prov_client_scan_get(pparse_value->dw_parameter[0], pparse_value->dw_parameter[1]);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_rmt_prov_scan_start(user_cmd_parse_value_t *pparse_value)
{
    uint8_t *puuid = NULL;
    uint8_t uuid[16];
    if (pparse_value->para_count >= 5)
    {
        puuid = uuid;
        plt_hex_to_bin(uuid, (uint8_t *)pparse_value->pparameter[4], 16);
    }

    rmt_prov_client_scan_start(pparse_value->dw_parameter[0], pparse_value->dw_parameter[1],
                               pparse_value->dw_parameter[2], pparse_value->dw_parameter[3],
                               puuid);

    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_rmt_prov_scan_stop(user_cmd_parse_value_t *pparse_value)
{
    rmt_prov_client_scan_stop(pparse_value->dw_parameter[0], pparse_value->dw_parameter[1]);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_rmt_prov_extended_scan_start(user_cmd_parse_value_t
                                                                     *pparse_value)
{
    uint8_t *puuid = NULL;
    uint8_t uuid[16];
    uint8_t *pad_type_filter = NULL;
    uint8_t ad_type_filter[16];
    uint8_t timeout = 0;
    if (pparse_value->para_count == 3)
    {
    }
    else if (pparse_value->para_count == 4)
    {
        pad_type_filter = ad_type_filter;
        plt_hex_to_bin(ad_type_filter, (uint8_t *)pparse_value->pparameter[3],
                       pparse_value->dw_parameter[2]);
    }
    else if (pparse_value->para_count == 6)
    {
        pad_type_filter = ad_type_filter;
        plt_hex_to_bin(ad_type_filter, (uint8_t *)pparse_value->pparameter[3],
                       pparse_value->dw_parameter[2]);
        puuid = uuid;
        plt_hex_to_bin(uuid, (uint8_t *)pparse_value->pparameter[4], 16);
        timeout = pparse_value->dw_parameter[5];
    }
    else
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    rmt_prov_client_extended_scan_start(pparse_value->dw_parameter[0], pparse_value->dw_parameter[1],
                                        pparse_value->dw_parameter[2], pad_type_filter, puuid, timeout);

    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_rmt_prov_link_get(user_cmd_parse_value_t *pparse_value)
{
    rmt_prov_client_link_get(pparse_value->dw_parameter[0], pparse_value->dw_parameter[1]);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_rmt_prov_link_open_prov(user_cmd_parse_value_t
                                                                *pparse_value)
{
    uint8_t uuid[16];
    plt_hex_to_bin(uuid, (uint8_t *)pparse_value->pparameter[2], 16);
    rmt_prov_client_link_open_prov(pparse_value->dw_parameter[0], pparse_value->dw_parameter[1], uuid,
                                   pparse_value->dw_parameter[3]);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_rmt_prov_link_open_dkri(user_cmd_parse_value_t
                                                                *pparse_value)
{
    rmt_prov_client_link_open_dkri(pparse_value->dw_parameter[0], pparse_value->dw_parameter[1],
                                   pparse_value->dw_parameter[2]);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_rmt_prov_link_close(user_cmd_parse_value_t *pparse_value)
{
    rmt_prov_client_link_close(pparse_value->dw_parameter[0], pparse_value->dw_parameter[1],
                               pparse_value->dw_parameter[2]);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_rmt_prov_pdu_send(user_cmd_parse_value_t *pparse_value)
{
    uint8_t data_len = pparse_value->dw_parameter[3];
    uint8_t data[128];
    plt_hex_to_bin(data, (uint8_t *)pparse_value->pparameter[4], data_len);
    rmt_prov_client_pdu_send(pparse_value->dw_parameter[0], pparse_value->dw_parameter[1],
                             pparse_value->dw_parameter[2], data, data_len);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_rmt_prov_refresh_device_key(user_cmd_parse_value_t
                                                                    *pparse_value)
{
    printf("refresh device key...\r\n");
    uint32_t attn_dur = pparse_value->dw_parameter[0];
    prov_manual = pparse_value->dw_parameter[1];
    prov_start_time = plt_time_read_ms();
    return rmt_prov_client_refresh_dev_key(attn_dur) ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
}

static user_cmd_parse_result_t user_cmd_rmt_prov_refresh_node_addr(user_cmd_parse_value_t
                                                                   *pparse_value)
{
    uint16_t prov_node_addr = pparse_value->dw_parameter[0];
    if (!MESH_IS_UNICAST_ADDR(prov_node_addr))
    {
        printf("invalid node addr 0x%04x\r\n", prov_node_addr);
        return USER_CMD_RESULT_ERROR;
    }
    printf("refresh node address...\r\n");
    uint32_t attn_dur = pparse_value->dw_parameter[1];
    prov_manual = pparse_value->dw_parameter[2];
    prov_start_time = plt_time_read_ms();
    return rmt_prov_client_refresh_node_addr(prov_node_addr,
                                             attn_dur) ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
}

static user_cmd_parse_result_t user_cmd_rmt_prov_refresh_compo_data(user_cmd_parse_value_t
                                                                    *pparse_value)
{
    printf("refresh composition data...\r\n");
    uint32_t attn_dur = pparse_value->dw_parameter[0];
    prov_manual = pparse_value->dw_parameter[1];
    prov_start_time = plt_time_read_ms();
    return rmt_prov_client_refresh_compo_data(attn_dur) ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
}
#endif


/*----------------------------------------------------
 * command table
 * --------------------------------------------------*/
const user_cmd_table_entry_t provisioner_cmd_table[] =
{
    // mesh common cmd
    MESH_COMMON_CMD,
    CLIENT_CMD,
    TEST_CMD,
    // provisioner cmd
    // pb-adv
    {
        "pbadvcon",
        "pbadvcon [dev uuid]\n\r",
        "create a pb-adv link with the device uuid\n\r",
        user_cmd_pb_adv_con
    },
    {
        "pbadvdisc",
        "pbadvdisc\n\r",
        "disconnect the pb-adv link\n\r",
        user_cmd_pb_adv_disc
    },
    // pb-gatt
    {
        "provdis",
        "provdis [conn id]\n\r",
        "Start discovery provisioning service\n\r",
        user_cmd_prov_discover
    },
    {
        "provread",
        "provread [char] [pattern: handle/UUID16]\n\r",
        "Read all related chars by user input\n\r",
        user_cmd_prov_read_char
    },
    {
        "provcmd",
        "provcmd [char CCCD] [command: enable/disable]\n\r",
        "Provisioning notify/ind switch command\n\r",
        user_cmd_prov_cccd_operate
    },
    {
        "provls",
        "provls\n\r",
        "Provision server handle list\n\r",
        user_cmd_prov_list
    },
    // provisioner
    {
        "prov",
        "prov [attn_dur] [manual]\n\r",
        "provision a new mesh device\n\r",
        user_cmd_prov
    },
    {
        "provs",
        "provs\n\r",
        "provision stop\n\r",
        user_cmd_prov_stop
    },
    {
        "pap",
        "pap [algorithm] [pubkey] [method: nsoi] [action] [size]\n\r",
        "provision authentication path\n\r",
        user_cmd_prov_auth_path
    },
    {
        "unprov",
        "unprov\n\r",
        "unprovision the mesh device\n\r",
        user_cmd_unprov
    },
    // cfg client key set
    {
        "ccks",
        "ccks [key_index] [use_app_key]\n\r",
        "cfg client key set\n\r",
        user_cmd_cfg_client_key_set
    },
    // cfg client or local setting
    {
        "cdg",
        "cdg [dst]\n\r",
        "compo data get\n\r",
        user_cmd_compo_data_get
    },
    {
        "nr",
        "nr [dst] [clear]\n\r",
        "node reset\n\r",
        user_cmd_node_reset
    },
    {
        "nka",
        "nka [dst] [net_key_index]\n\r",
        "net key add\n\r",
        user_cmd_net_key_add
    },
    {
        "nku",
        "nku [dst] [net_key_index] [net key]\n\r",
        "net key update\n\r",
        user_cmd_net_key_update
    },
    {
        "aka",
        "aka [dst] [net_key_index] [app_key_index]\n\r",
        "app key add\n\r",
        user_cmd_app_key_add
    },
    {
        "aku",
        "aku [dst] [net_key_index] [app_key_index] [app key]\n\r",
        "app key update\n\r",
        user_cmd_app_key_update
    },
    {
        "mab",
        "mab [dst] [element index] [model_id] [app_key_index]\n\r",
        "model app bind\n\r",
        user_cmd_model_app_bind
    },
    {
        "msa",
        "msa [dst] [element index] [model_id] [group addr]\n\r",
        "model subsribe add\n\r",
        user_cmd_model_sub_add
    },
    {
        "msd",
        "msd [dst] [element index] [model_id] <group addr>\n\r",
        "model subsribe delete\n\r",
        user_cmd_model_sub_delete
    },
    {
        "krpg",
        "krpg [dst] [net key index]\n\r",
        "key refresh phase get\n\r",
        user_cmd_key_refresh_phase_get
    },
    {
        "krps",
        "krps [dst] [net key index] [phase]\n\r",
        "key refresh phase set\n\r",
        user_cmd_key_refresh_phase_set
    },
    {
        "dtw",
        "dtw [dst] [data...] [app_key_index] [ack] \n\r",
        "data transmission write data\n\r",
        user_cmd_datatrans_write
    },
    {
        "dtr",
        "dtr [dst] [len] [app_key_index]\n\r",
        "data transmission read data\n\r",
        user_cmd_datatrans_read
    },
#if F_BT_MESH_1_1_RPR_SUPPORT
    {
        "rmtscan",
        "rmtscan [dst] [net key index] [scanned items limit] [scan timeout] [dev uuid]\n\r",
        "romte provision scan start\n\r",
        user_cmd_rmt_prov_client_scan_start
    },
    {
        "rmtcon",
        "rmtcon [dst] [net key index] [dev uuid] [link open timeout]\n\r",
        "romte link open for provision\n\r",
        user_cmd_rmt_prov_client_link_open_prov
    },
    {
    	"rmtdisc",
		"rmtdisc [dst] [net_key_index] [reason]\n\r",
		"romte link close for provision\n\r",
		user_cmd_rmt_prov_client_close
    },
    {
        "rpscg",
        "rpscg [dst] [net key index]\n\r",
        "remote provision scan capabilities get\n\r",
        user_cmd_rmt_prov_scan_capabilities_get
    },
    {
        "rpsg",
        "rpsg [dst] [net key index]\n\r",
        "remote provision scan get\n\r",
        user_cmd_rmt_prov_scan_get
    },
    {
        "rpsst",
        "rpsst [dst] [net key index] [scanned items limit] [scan timeout] [uuid]\n\r",
        "remote provision scan start\n\r",
        user_cmd_rmt_prov_scan_start
    },
    {
        "rpssp",
        "rpssp [dst] [net key index]\n\r",
        "remote provision scan stop\n\r",
        user_cmd_rmt_prov_scan_stop
    },
    {
        "rpess",
        "rpess [dst] [net key index] [ad type filter cnt] [ad type filter] [uuid] [timeout]\n\r",
        "remote provision extended scan start\n\r",
        user_cmd_rmt_prov_extended_scan_start
    },
    {
        "rplg",
        "rplg [dst] [net key index]\n\r",
        "remote provision link get\n\r",
        user_cmd_rmt_prov_link_get
    },
    {
        "rplop",
        "rplop [dst] [net key index] [uuid] [timeout]\n\r",
        "remote provision link open for provision\n\r",
        user_cmd_rmt_prov_link_open_prov
    },
    {
        "rplod",
        "rplod [dst] [net key index] [dkri]\n\r",
        "remote provision link open for dkri\n\r",
        user_cmd_rmt_prov_link_open_dkri
    },
    {
        "rplnc",
        "rplnc [dst] [net key index] [reason]\n\r",
        "remote provision link close\n\r",
        user_cmd_rmt_prov_link_close
    },
    {
        "rpps",
        "rpps [dst] [net key index] [outbound_pdu_num] [data len] [data]\n\r",
        "remote provision pdu send\n\r",
        user_cmd_rmt_prov_pdu_send
    },
    {
        "rprdk",
        "rprdk [attn_dur] [manual]\n\r",
        "remote provision refresh device key\n\r",
        user_cmd_rmt_prov_refresh_device_key
    },
    {
        "rprna",
        "rprna [node addr] [attn_dur] [manual]\n\r",
        "remote provision refresh node address\n\r",
        user_cmd_rmt_prov_refresh_node_addr
    },
    {
        "rprcd",
        "rprcd [attn_dur] [manual]\n\r",
        "remote provision refresh composition data\n\r",
        user_cmd_rmt_prov_refresh_compo_data
    },
#endif
    /* MUST be at the end: */
    {
        0,
        0,
        0,
        0
    }
};

#if defined(CONFIG_BT_MESH_USER_API) && CONFIG_BT_MESH_USER_API
const struct bt_mesh_api_hdl provisionercmds[] = 
{
    GEN_MESH_HANDLER(_pb_adv_con)    
    GEN_MESH_HANDLER(_prov)  
    GEN_MESH_HANDLER(_prov_stop)
    GEN_MESH_HANDLER(_app_key_add)
    GEN_MESH_HANDLER(_model_app_bind)
    GEN_MESH_HANDLER(_model_pub_set)
    GEN_MESH_HANDLER(_generic_on_off_set)
    GEN_MESH_HANDLER(_generic_on_off_get)
    GEN_MESH_HANDLER(_node_reset)
    GEN_MESH_HANDLER(_model_sub_delete)
    GEN_MESH_HANDLER(_model_sub_add)
    GEN_MESH_HANDLER(_model_sub_get)
    GEN_MESH_HANDLER(_prov_discover)
    GEN_MESH_HANDLER(_prov_cccd_operate)
    GEN_MESH_HANDLER(_proxy_discover)
    GEN_MESH_HANDLER(_proxy_cccd_operate)
    GEN_MESH_HANDLER(_datatrans_write)
    GEN_MESH_HANDLER(_datatrans_read)
    GEN_MESH_HANDLER(_connect)
    GEN_MESH_HANDLER(_disconnect)
    GEN_MESH_HANDLER(_list)
    GEN_MESH_HANDLER(_dev_info_show)
    GEN_MESH_HANDLER(_fn_init)
    GEN_MESH_HANDLER(_fn_deinit)
    GEN_MESH_HANDLER(_light_lightness_get)
    GEN_MESH_HANDLER(_light_lightness_set)
    GEN_MESH_HANDLER(_light_lightness_linear_get)
    GEN_MESH_HANDLER(_light_lightness_linear_set)
    GEN_MESH_HANDLER(_light_lightness_last_get)
    GEN_MESH_HANDLER(_light_lightness_default_get)
    GEN_MESH_HANDLER(_light_lightness_default_set)
    GEN_MESH_HANDLER(_light_lightness_range_get)
    GEN_MESH_HANDLER(_light_lightness_range_set)
    GEN_MESH_HANDLER(_light_ctl_get)
    GEN_MESH_HANDLER(_light_ctl_set)
    GEN_MESH_HANDLER(_light_ctl_temperature_get)
    GEN_MESH_HANDLER(_light_ctl_temperature_set)
    GEN_MESH_HANDLER(_light_ctl_temperature_range_get)
    GEN_MESH_HANDLER(_light_ctl_temperature_range_set)
    GEN_MESH_HANDLER(_light_ctl_default_get)
    GEN_MESH_HANDLER(_light_ctl_default_set)
    GEN_MESH_HANDLER(_light_hsl_get)
    GEN_MESH_HANDLER(_light_hsl_set)
    GEN_MESH_HANDLER(_light_hsl_target_get)
    GEN_MESH_HANDLER(_light_hsl_hue_get)
    GEN_MESH_HANDLER(_light_hsl_hue_set)
    GEN_MESH_HANDLER(_light_hsl_saturation_get)
    GEN_MESH_HANDLER(_light_hsl_saturation_set)
    GEN_MESH_HANDLER(_light_hsl_default_get)
    GEN_MESH_HANDLER(_light_hsl_default_set)
    GEN_MESH_HANDLER(_light_hsl_range_get)
    GEN_MESH_HANDLER(_light_hsl_range_set)
    GEN_MESH_HANDLER(_light_xyl_get)
    GEN_MESH_HANDLER(_light_xyl_set)
    GEN_MESH_HANDLER(_light_xyl_target_get)
    GEN_MESH_HANDLER(_light_xyl_default_get)
    GEN_MESH_HANDLER(_light_xyl_default_set)
    GEN_MESH_HANDLER(_light_xyl_range_get)
    GEN_MESH_HANDLER(_light_xyl_range_set)
    GEN_MESH_HANDLER(_time_set)
    GEN_MESH_HANDLER(_time_get)
    GEN_MESH_HANDLER(_time_zone_set)
    GEN_MESH_HANDLER(_time_zone_get)
    GEN_MESH_HANDLER(_time_tai_utc_delta_set)
    GEN_MESH_HANDLER(_time_tai_utc_delta_get)
    GEN_MESH_HANDLER(_time_role_set)
    GEN_MESH_HANDLER(_time_role_get)
	GEN_MESH_HANDLER(_scene_store)
	GEN_MESH_HANDLER(_scene_recall)
	GEN_MESH_HANDLER(_scene_get)
	GEN_MESH_HANDLER(_scene_register_get)
	GEN_MESH_HANDLER(_scene_delete)
	GEN_MESH_HANDLER(_scheduler_get)
	GEN_MESH_HANDLER(_scheduler_action_get)
	GEN_MESH_HANDLER(_scheduler_action_set)
    /*******new model test********/
    GEN_MESH_HANDLER(_gdtt_get)
    GEN_MESH_HANDLER(_gdtt_set)
    GEN_MESH_HANDLER(_generic_level_get) 
	GEN_MESH_HANDLER(_generic_level_set)
    GEN_MESH_HANDLER(_generic_delta_set)
    GEN_MESH_HANDLER(_generic_move_set)
    GEN_MESH_HANDLER(_generic_on_powerup_get)
    GEN_MESH_HANDLER(_generic_on_powerup_set)
    GEN_MESH_HANDLER(_generic_power_level_get)
    GEN_MESH_HANDLER(_generic_power_level_set)
    GEN_MESH_HANDLER(_generic_power_last_get)
    GEN_MESH_HANDLER(_generic_power_default_get)
    GEN_MESH_HANDLER(_generic_power_default_set)
    GEN_MESH_HANDLER(_generic_power_range_get)
    GEN_MESH_HANDLER(_generic_power_range_set)
    GEN_MESH_HANDLER(_generic_battery_get)
    GEN_MESH_HANDLER(_sensor_descriptor_get)
    GEN_MESH_HANDLER(_sensor_cadence_get)
    GEN_MESH_HANDLER(_sensor_cadence_set)
    GEN_MESH_HANDLER(_sensor_settings_get)
    GEN_MESH_HANDLER(_sensor_setting_set)
    GEN_MESH_HANDLER(_sensor_setting_get)
    GEN_MESH_HANDLER(_sensor_get)
    GEN_MESH_HANDLER(_sensor_column_get)
    GEN_MESH_HANDLER(_sensor_series_get)
    GEN_MESH_HANDLER(_generic_location_global_get)
	GEN_MESH_HANDLER(_generic_location_global_set)
	GEN_MESH_HANDLER(_generic_location_local_get)
	GEN_MESH_HANDLER(_generic_location_local_set)
	GEN_MESH_HANDLER(_generic_user_properties_get)
	GEN_MESH_HANDLER(_generic_user_property_get)
	GEN_MESH_HANDLER(_generic_user_property_set)
	GEN_MESH_HANDLER(_generic_admin_properties_get)
	GEN_MESH_HANDLER(_generic_admin_property_get)
	GEN_MESH_HANDLER(_generic_admin_property_set)
	GEN_MESH_HANDLER(_generic_manufacturer_properties_get)
	GEN_MESH_HANDLER(_generic_manufacturer_property_get)
	GEN_MESH_HANDLER(_generic_manufacturer_property_set)
	GEN_MESH_HANDLER(_generic_client_properties_get)
    GEN_MESH_HANDLER(_light_lc_mode_get)
	GEN_MESH_HANDLER(_light_lc_mode_set)
	GEN_MESH_HANDLER(_light_lc_om_get)
	GEN_MESH_HANDLER(_light_lc_om_set)
	GEN_MESH_HANDLER(_light_lc_light_on_off_get)
	GEN_MESH_HANDLER(_light_lc_light_on_off_set)
	GEN_MESH_HANDLER(_light_lc_property_get)
	GEN_MESH_HANDLER(_light_lc_property_set)
#if F_BT_MESH_1_1_RPR_SUPPORT
	GEN_MESH_HANDLER(_rmt_prov_client_scan_start)
	GEN_MESH_HANDLER(_rmt_prov_client_link_open_prov)
	GEN_MESH_HANDLER(_rmt_prov_client_close)
#endif
#if defined(MESH_DFU) && MESH_DFU
    GEN_MESH_HANDLER(_fw_update_info_get)
    GEN_MESH_HANDLER(_fw_update_start)
    GEN_MESH_HANDLER(_fw_update_cancel)
#endif
#if F_BT_MESH_1_1_DF_SUPPORT
    GEN_MESH_HANDLER(_directed_control_set)
    GEN_MESH_HANDLER(_directed_publish_policy_set)
    GEN_MESH_HANDLER(_forwarding_table_add)
    GEN_MESH_HANDLER(_forwarding_table_dependents_add)
    GEN_MESH_HANDLER(_forwarding_table_delete)
    GEN_MESH_HANDLER(_forwarding_table_dependents_delete)
    GEN_MESH_HANDLER(_wanted_lanes_set)
    GEN_MESH_HANDLER(_two_way_path_set)
    GEN_MESH_HANDLER(_rssi_threshold_set)
    GEN_MESH_HANDLER(_discovery_table_capabilities_set)
    GEN_MESH_HANDLER(_path_echo_interval_set)
    GEN_MESH_HANDLER(_path_metric_set)
    GEN_MESH_HANDLER(_df_path_discovery)
    GEN_MESH_HANDLER(_df_path_solicitation)
    GEN_MESH_HANDLER(_df_path_dependents_update)
#endif
};
#endif

#endif
