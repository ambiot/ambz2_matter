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

//#if CONFIG_BLE_MATTER_MULTI_ADV     /*To fix ble_ms_adapter_service.h:36:2: error: unknown type name 'T_MS_READ_MSG'*/
typedef struct {
	unsigned int *p_len;
	uint8_t *p_value;
} T_MS_READ_MSG;
//#endif

typedef struct {
	T_WRITE_TYPE write_type;
	unsigned int len;
	uint8_t *p_value;
	ms_hal_ble_service_write_cb write_cb;
} T_MS_WRITE_MSG;

typedef struct {
	uint16_t attr_index;
	uint16_t ccc_val;
} T_MS_CCCD_MSG;

typedef union {
//#if CONFIG_BLE_MATTER_MULTI_ADV  /*To fix ble_matter_adapter_service.c:157:24: error: 'T_MS_MSG_DATA' has no member named 'read'*/
	T_MS_READ_MSG read;
//#endif
	T_MS_CCCD_MSG cccd;
	T_MS_WRITE_MSG write;
} T_MS_MSG_DATA;


typedef struct {
	uint8_t conn_id;
	T_SERVICE_CALLBACK_TYPE msg_type;
	T_MS_MSG_DATA msg_data;
	T_SERVER_ID srv_id;
} T_MS_ADAPTER_CALLBACK_DATA;

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

typedef struct {
	uint8_t type;
	uint8_t conn_id;
	uint8_t srv_id;
	uint16_t attrib_index;
	uint8_t *val;
	uint16_t len;
} BMS_INDICATION_PARAM;


typedef struct {
	uint8_t att_handle;
	ms_hal_ble_attrib_callback_t func;
} BMS_SERVICE_CALLBACK_INFO;


typedef struct BMS_SERVICE_INFO {
	uint8_t srvId;
	uint8_t att_num;
	T_ATTRIB_APPL *att_tbl;
	BMS_SERVICE_CALLBACK_INFO *cbInfo;
	struct BMS_SERVICE_INFO *next;
} BMS_SERVICE_INFO;

BMS_SERVICE_INFO *ble_ms_adapter_parse_srv_tbl(ms_hal_ble_service_attrib_t **profile, uint16_t attrib_count);
bool ble_ms_adapter_send_indication_notification(uint8_t conn_id, uint8_t service_id, uint8_t handle, uint8_t *p_value, uint16_t length, bool type);
T_SERVER_ID ble_ms_adapter_add_service(BMS_SERVICE_INFO *service_info, void *p_func);


