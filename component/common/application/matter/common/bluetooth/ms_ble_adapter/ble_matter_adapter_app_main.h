/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      bt_matter_adapter_peripheral_app.h
   * @brief     This file handles BLE peripheral application routines.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */
#ifndef _BT_MESH_DEVICE_MATTER_APP_MAIN_H__
#define _BT_MESH_DEVICE_MATTER_APP_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <app_msg.h>
#include <profile_server.h>

/*============================================================================*
 *                              Variables
 *============================================================================*/
typedef struct
{
	uint8_t conn_id;
	uint8_t service_id;
	uint16_t attrib_index;
	uint8_t *p_data;
	uint16_t data_len;
	uint8_t type;
} BT_MATTER_SERVER_SEND_DATA;

/*============================================================================*
 *                              Functions
 *============================================================================*/
uint16_t ble_att_mtu_z2(uint16_t conn_id);

bool ble_matter_netmgr_server_send_data(uint8_t conn_id, T_SERVER_ID service_id, uint16_t attrib_index,
					  uint8_t *p_data, uint16_t data_len, T_GATT_PDU_TYPE type);

bool ble_matter_netmgr_adv_param_handler(uint16_t adv_int_min, uint16_t adv_int_max, void *advData, uint8_t advData_len);

bool ble_matter_netmgr_adv_start_handler(void);

bool ble_matter_netmgr_adv_stop_handler(void);

bool ble_matter_netmgr_adapter_init_handler(void);


#ifdef __cplusplus
};
#endif

#endif
