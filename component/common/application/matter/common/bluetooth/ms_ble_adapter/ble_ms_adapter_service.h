#include <profile_server.h>
#include "ms_hal_ble.h"
#include "platform_opts_bt.h"

#define MS_READ_MAX_LEN 300
#define MS_WRITE_MAX_LEN 300
#define BMS_MAX_ATTR_NUM 12
#define BT_MATTER_ADAPTER_SERVICE_CHAR_WRITE_INDEX   0x2
#define BT_MATTER_ADAPTER_SERVICE_CHAR_INDICATE_INDEX  0x4
//#define BT_MATTER_ADAPTER_SERVICE_CHAR_INDICATE_CCCD_INDEX  BT_MATTER_ADAPTER_SERVICE_CHAR_INDICATE_INDEX + 1

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

/*============================================================================*
 *                              Constants
 *============================================================================*/
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
/*============================================================================*
 *                              Functions
 *============================================================================*/
T_APP_RESULT  ble_matter_adapter_service_attr_read_cb(uint8_t conn_id, T_SERVER_ID service_id,
                                            uint16_t attrib_index, uint16_t offset, uint16_t *p_length, uint8_t **pp_value);
                                            
T_APP_RESULT ble_matter_adapter_service_attr_write_cb(uint8_t conn_id, T_SERVER_ID service_id,
                                            uint16_t attrib_index, T_WRITE_TYPE write_type, uint16_t length, uint8_t *p_value,
                                            P_FUN_WRITE_IND_POST_PROC *p_write_ind_post_proc); 
                                                       
void ble_matter_adapter_service_cccd_update_cb(uint8_t conn_id, T_SERVER_ID service_id, uint16_t index,
                                     uint16_t cccbits);      
                                                               
T_SERVER_ID ble_matter_adapter_service_add_service(void *p_func);


