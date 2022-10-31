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
#ifndef _BT_MATTER_ADAPTER_PERIPHERAL_APP__
#define _BT_MATTER_ADAPTER_PERIPHERAL_APP__

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <app_msg.h>
#include <gap_le.h>
#include <profile_server.h>
#include <chip_porting.h>

/*============================================================================*
 *                              Variables
 *============================================================================*/
extern T_SERVER_ID bt_matter_adapter_srv_id; /**< Simple ble service id*/
//extern T_SERVER_ID bas_srv_id;  /**< Battery service id */

/*============================================================================*
 *                              Constants
 *============================================================================*/
/** @addtogroup  PERIPHERAL_APP
    * @{
    */
#if (F_BT_LE_USE_RANDOM_ADDR==1)
typedef struct
{
	uint8_t 	 is_exist;
	uint8_t 	 reserved;		   /**< remote BD type*/
	uint8_t 	 bd_addr[GAP_BD_ADDR_LEN];	/**< remote BD */
} T_APP_STATIC_RANDOM_ADDR;
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
void bt_matter_adapter_app_handle_io_msg(T_IO_MSG  io_msg);

/**
 * @brief    All the BT Profile service callback events are handled in this function
 * @note     Then the event handling function shall be called according to the
 *           service_id.
 * @param[in] service_id  Profile service ID
 * @param[in] p_data      Pointer to callback data
 * @return   Indicates the function call is successful or not
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT bt_matter_adapter_app_profile_callback(T_SERVER_ID service_id, void *p_data);

/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT bt_matter_adapter_app_gap_callback(uint8_t cb_type, void *p_cb_data);

void bt_matter_adapter_app_set_adv_data(void);
#include "bt_matter_adapter_service.h"
typedef enum
{
    CB_PROFILE_CALLBACK = 0x01,  /**< bt_matter_adapter_app_profile_callback */
    CB_GAP_CALLBACK = 0x02, /**< bt_matter_adapter_app_gap_callback */
    CB_GAP_MSG_CALLBACK = 0x3, /**< bt_matter_adapter_app_handle_gap_msg */
} T_CHIP_BLEMGR_CALLBACK_TYPE;
typedef int (*chip_blemgr_callback)(void *param, void *cb_data, int type, T_CHIP_BLEMGR_CALLBACK_TYPE callback_type);
void chip_blemgr_set_callback_func(chip_blemgr_callback p, void *data);

#ifdef __cplusplus
}
#endif

#endif
