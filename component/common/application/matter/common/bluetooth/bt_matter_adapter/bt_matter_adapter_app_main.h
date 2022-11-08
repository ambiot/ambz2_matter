/******************************************************************************
 * Copyright (c) 2013-2016 Realtek Semiconductor Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
  ******************************************************************************
  * @file    bt_matter_adapter_app_main.h
  * @author
  * @version
  * @brief
  ******************************************************************************
  */
#ifndef __BT_MATTER_ADAPTER_APP_MAIN_H_
#define __BT_MATTER_ADAPTER_APP_MAIN_H_

#ifdef __cplusplus
extern "C" {
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
void bt_matter_adapter_stack_config_init(void);

/**
 * @brief  Add GATT services and register callbacks
 * @return void
 */
void bt_matter_adapter_app_le_profile_init(void);

/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BLE Peripheral APP, thus only one APP task is init here
 * @return   void
 */
void bt_matter_adapter_task_init(void);

void bt_matter_adapter_task_deinit(void);

void bt_matter_adapter_app_le_gap_init_chip(void);

int bt_matter_adapter_init(void);

uint16_t ble_att_mtu_z2(uint16_t conn_id);
#if CONFIG_BT_MATTER_ADAPTER
uint8_t get_bt_matter_adapter_state(void);

void set_bt_matter_adapter_state(uint8_t state);

int bt_matter_adapter_app_init(void);

void bt_matter_adapter_app_deinit(void);

void bt_matter_adapter_app_le_gap_init(void);

int bt_matter_adapter_adv(void);
#elif CONFIG_BT_MESH_DEVICE_MATTER

typedef struct
{
    uint8_t conn_id;
    uint8_t service_id;
    uint16_t attrib_index;
    uint8_t *p_data;
    uint16_t data_len;
    uint8_t type;
} BT_MATTER_SERVER_SEND_DATA;

bool ble_matter_netmgr_adapter_init_handler(void);

bool ble_matter_netmgr_adv_param_handler(uint16_t adv_int_min, uint16_t adv_int_max, void *advData, uint8_t advData_len);

bool ble_matter_netmgr_adv_start_handler(void);

bool ble_matter_netmgr_adv_stop_handler(void);

bool ble_matter_netmgr_start_adv(void);

bool ble_matter_netmgr_stop_adv(void);

bool ble_matter_netmgr_server_send_data(uint8_t conn_id, T_SERVER_ID service_id, uint16_t attrib_index,
                      uint8_t *p_data, uint16_t data_len, T_GATT_PDU_TYPE type);
#endif
#ifdef __cplusplus
}
#endif
#endif // __BT_MATTER_ADAPTER_APP_MAIN_H_
