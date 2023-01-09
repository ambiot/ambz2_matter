/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     bt_matter_adapter_service.h
  * @brief    Demonstration of how to implement a self-definition service.
  * @details  Demonstration of different kinds of service interfaces.
  * @author
  * @date
  * @version
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _BT_MESH_DEVICE_MATTER_ADAPTER_SERVICE_H_
#define _BT_MESH_DEVICE_MATTER_ADAPTER_SERVICE_H_

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

/* Add Includes here */
#include <profile_server.h>


/** @defgroup SIMP_Service Simple Ble Service
  * @brief Simple BLE service
  * @{
  */
/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup SIMP_Service_Exported_Macros SIMP Service Exported Macros
  * @brief
  * @{
  */

/** @defgroup SIMP_Service_Application_Parameters SIMP Service Application Parameters
  * @brief  Type of parameters set/got from application.
  * @{
  */
typedef enum
{
	BTMATTER_SERVICE_PARAM_V1_READ_CHAR_VAL = 0x01,
} T_BTMATTER_PARAM_TYPE;

/** @} */


/** @defgroup SIMP_Service_Upstream_Message SIMP Service Upstream Message
  * @brief  Upstream message used to inform application.
  * @{
  */
#define MATTER_NOTIFY_INDICATE_V3_ENABLE     1
#define MATTER_NOTIFY_INDICATE_V3_DISABLE    2
//#define SIMP_NOTIFY_INDICATE_V4_ENABLE     3
//#define SIMP_NOTIFY_INDICATE_V4_DISABLE    4

#define BT_MATTER_ADAPTER_SERVICE_CHAR_RX_INDEX                 0x02
#define BT_MATTER_ADAPTER_SERVICE_CHAR_TX_INDEX                 0x04
#define BT_MATTER_ADAPTER_SERVICE_CHAR_INDICATE_CCCD_INDEX      (BT_MATTER_ADAPTER_SERVICE_CHAR_TX_INDEX + 1)
#define BT_MATTER_ADAPTER_SERVICE_C3_INDEX                      0x07

/** @} */

#define BT_MATTER_ADAPTER_READ_V1_MAX_LEN               300

/** @} End of SIMP_Service_Upstream_Message */

/** @} End of SIMP_Service_Exported_Macros */
/*============================================================================*
 *                              Types
 *============================================================================*/
/** @defgroup SIMP_Service_Exported_Types SIMP Service Exported Types
  * @brief
  * @{
  */

/** @defgroup TSIMP_WRITE_MSG TSIMP_WRITE_MSG
  * @brief Simple BLE service written msg to application.
  * @{
  */
typedef struct
{
    uint16_t len;
    uint8_t *p_value;
} T_MATTER_WRITE_READ_MSG;
/** @} End of TSIMP_WRITE_MSG */

/** @defgroup TSIMP_UPSTREAM_MSG_DATA TSIMP_UPSTREAM_MSG_DATA
  * @brief Simple BLE service callback message content.
  * @{
  */
typedef union
{
    uint8_t notification_indification_index; //!< ref: @ref SIMP_Service_Notify_Indicate_Info
    T_MATTER_WRITE_READ_MSG write_read;
} T_MATTER_UPSTREAM_MSG_DATA;
/** @} End of TSIMP_UPSTREAM_MSG_DATA */

/** @defgroup TSIMP_CALLBACK_DATA TSIMP_CALLBACK_DATA
  * @brief Simple BLE service data to inform application.
  * @{
  */
typedef struct
{
    uint8_t                 conn_id;
    T_SERVICE_CALLBACK_TYPE msg_type;
    T_MATTER_UPSTREAM_MSG_DATA msg_data;
} T_MATTER_CALLBACK_DATA;
/** @} End of TSIMP_CALLBACK_DATA */

/** @defgroup TSIMP_WRITE_MSG TSIMP_WRITE_MSG
  * @brief Simple BLE service written msg to application.
  * @{
  */
/*

*/
/** @} End of TSIMP_CALLBACK_DATA */

/** @} End of SIMP_Service_Exported_Types */
/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup SIMP_Service_Exported_Functions SIMP Service Exported Functions
  * @brief
  * @{
  */

/**
  * @brief Add simple BLE service to the BLE stack database.
  *
  * @param[in]   p_func  Callback when service attribute was read, write or cccd update.
  * @return Service id generated by the BLE stack: @ref T_SERVER_ID.
  * @retval 0xFF Operation failure.
  * @retval others Service id assigned by stack.
  *
  */
T_SERVER_ID bt_matter_adapter_service_add_service(void *p_func);

/** @} End of SIMP_Service_Exported_Functions */

/** @} End of SIMP_Service */

//extern T_SERVER_ID bt_matter_adapter_service_id;

#ifdef __cplusplus
}
#endif

#endif /* _BT_MATTER_ADAPTER_WIFI_SERVICE_H_ */

