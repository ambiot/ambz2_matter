/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      bt_matter_adapter_app_task.h
   * @brief     Routines to create App task and handle events & messages
   * @author    
   * @date      
   * @version   
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */
#ifndef _BT_MATTER_ADAPTER_APP_TASK_H_
#define _BT_MATTER_ADAPTER_APP_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize App task
 * @return void
 */
void bt_matter_adapter_send_msg(uint16_t sub_type);
void bt_matter_adapter_app_task_init(void);
void bt_matter_adapter_app_task_deinit(void);

#ifdef __cplusplus
}
#endif
#endif
