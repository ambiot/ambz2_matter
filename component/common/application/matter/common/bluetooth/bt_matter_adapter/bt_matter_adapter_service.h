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
#ifndef _BT_MATTER_ADAPTER_SERVICE_H_
#define _BT_MATTER_ADAPTER_SERVICE_H_

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

/* Add Includes here */
#include <profile_server.h>
#include "bt_matter_adapter_config.h"
#if CONFIG_BT_MATTER_ADAPTER
#include "simple_ble_service.h"
#endif
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
	BTCONFIG_SERVICE_PARAM_V1_READ_CHAR_VAL = 0x01,
} T_BTCONFIG_PARAM_TYPE;

/** @} */



/** @defgroup SIMP_Service_Upstream_Message SIMP Service Upstream Message
  * @brief  Upstream message used to inform application.
  * @{
  */
#if CONFIG_BT_MESH_DEVICE_MATTER
#define SIMP_NOTIFY_INDICATE_V3_ENABLE     1
#define SIMP_NOTIFY_INDICATE_V3_DISABLE    2
#define SIMP_NOTIFY_INDICATE_V4_ENABLE     3
#define SIMP_NOTIFY_INDICATE_V4_DISABLE    4
#endif
/** @defgroup SIMP_Service_Read_Info SIMP Service Read Info
  * @brief  Parameter for reading characteristic value.
  * @{
  */
#define BTCONFIG_READ_V1                                        1
/** @} */

/** @defgroup SIMP_Service_Write_Info SIMP Service Write Info
  * @brief  Parameter for writing characteristic value.
  * @{
  */
#define BTCONFIG_WRITE_V1                                       1

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
    uint8_t opcode; //!< ref:  @ref SIMP_Service_Write_Info
    T_WRITE_TYPE write_type;
    uint16_t len;
    uint8_t *p_value;
#if CONFIG_BT_MESH_DEVICE_MATTER
} TSIMP_WRITE_MSG;
#else
} TBTCONFIG_WRITE_MSG;
#endif
/** @} End of TSIMP_WRITE_MSG */


/** @defgroup TSIMP_UPSTREAM_MSG_DATA TSIMP_UPSTREAM_MSG_DATA
  * @brief Simple BLE service callback message content.
  * @{
  */
#if CONFIG_BT_MESH_DEVICE_MATTER
typedef union
{
    uint8_t notification_indification_index; //!< ref: @ref SIMP_Service_Notify_Indicate_Info
    uint8_t read_value_index; //!< ref: @ref SIMP_Service_Read_Info
    TSIMP_WRITE_MSG write;
    uint16_t read_offset;
} TSIMP_UPSTREAM_MSG_DATA;
#else
typedef union
{
    uint8_t read_value_index; //!< ref: @ref SIMP_Service_Read_Info
    TBTCONFIG_WRITE_MSG write;
	uint16_t read_offset;
} TBTCONFIG_MSG_DATA;
#endif
/** @} End of TSIMP_UPSTREAM_MSG_DATA */

/** @defgroup TSIMP_CALLBACK_DATA TSIMP_CALLBACK_DATA
  * @brief Simple BLE service data to inform application.
  * @{
  */
#if CONFIG_BT_MESH_DEVICE_MATTER
typedef struct
{
    uint8_t                 conn_id;
    T_SERVICE_CALLBACK_TYPE msg_type;
    TSIMP_UPSTREAM_MSG_DATA msg_data;
} TSIMP_CALLBACK_DATA;
#else
typedef struct
{
    uint8_t                 conn_id;
    T_SERVICE_CALLBACK_TYPE msg_type;
    TBTCONFIG_MSG_DATA msg_data;
} TBTCONFIG_CALLBACK_DATA;
#endif
/** @} End of TSIMP_CALLBACK_DATA */

/** @defgroup TSIMP_WRITE_MSG TSIMP_WRITE_MSG
  * @brief Simple BLE service written msg to application.
  * @{
  */
/*
#if CONFIG_BT_MATTER_ADAPTER
typedef struct
{
    uint8_t opcode; //!< ref:  @ref SIMP_Service_Write_Info
    T_WRITE_TYPE write_type;
    uint16_t len;
    uint8_t *p_value;
} TSIMP_WRITE_MSG;
*/
/** @} End of TSIMP_WRITE_MSG */


/** @defgroup TSIMP_UPSTREAM_MSG_DATA TSIMP_UPSTREAM_MSG_DATA
  * @brief Simple BLE service callback message content.
  * @{
  */
/*
typedef union
{
    uint8_t notification_indification_index; //!< ref: @ref SIMP_Service_Notify_Indicate_Info
    uint8_t read_value_index; //!< ref: @ref SIMP_Service_Read_Info
    TSIMP_WRITE_MSG write;
} TSIMP_UPSTREAM_MSG_DATA;
*/
/** @} End of TSIMP_UPSTREAM_MSG_DATA */

/** @defgroup TSIMP_CALLBACK_DATA TSIMP_CALLBACK_DATA
  * @brief Simple BLE service data to inform application.
  * @{
  */
/*
typedef struct
{
    uint8_t                 conn_id;
    T_SERVICE_CALLBACK_TYPE msg_type;
    TSIMP_UPSTREAM_MSG_DATA msg_data;
} TSIMP_CALLBACK_DATA;
#endif
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

/**
  * @brief  Set service related data from application.
  *
  * @param[in] param_type            parameter type to set.
  * @param[in] len                   value length to be set.
  * @param[in] p_value             value to set.
  * @return parameter set result.
  * @retval 0 false
  * @retval 1 true
  */
bool bt_matter_adapter_service_set_parameter(T_BTCONFIG_PARAM_TYPE param_type, uint16_t len, void *p_value);

/** @} End of SIMP_Service_Exported_Functions */

/** @} End of SIMP_Service */

#define BT_MATTER_ADAPTER_SERVICE_CHAR_INDICATE_CCCD_INDEX 0x5
extern T_SERVER_ID bt_matter_adapter_service_id;

#ifdef __cplusplus
}
#endif

#endif /* _BT_MATTER_ADAPTER_WIFI_SERVICE_H_ */

