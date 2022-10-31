/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     simple_ble_service.c
* @brief    simple BLE profile source file.
* @details  Demonstration of how to implement a self-definition profile.
* @author
* @date
* @version
*********************************************************************************************************
*/
#include <platform_opts_bt.h>
#if defined(CONFIG_BT_MATTER_ADAPTER) && CONFIG_BT_MATTER_ADAPTER
#include <string.h>
#include <trace_app.h>
#include "bt_matter_adapter_service.h"
#include <gap.h>
#include <diag.h>
#include "platform_stdlib.h"


#define SIMPLE_BLE_SERVICE_CHAR_V1_READ_INDEX           0x02
#define SIMPLE_BLE_SERVICE_CHAR_V2_WRITE_INDEX          0x05
#define SIMPLE_BLE_SERVICE_CHAR_V3_NOTIFY_INDEX         0x07
#define SIMPLE_BLE_SERVICE_CHAR_V4_INDICATE_INDEX       0x0a
#define SIMPLE_BLE_SERVICE_CHAR_NOTIFY_CCCD_INDEX       (SIMPLE_BLE_SERVICE_CHAR_V3_NOTIFY_INDEX + 1)
#define SIMPLE_BLE_SERVICE_CHAR_INDICATE_CCCD_INDEX     (SIMPLE_BLE_SERVICE_CHAR_V4_INDICATE_INDEX + 1)

#define BT_MATTER_ADAPTER_SERVICE_CHAR_V1_READ_WRITE_INDEX           0x02
#define BT_MATTER_ADAPTER_SERVICE_C3_INDEX           0x07
//#define UUID_RX	0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F, 0x9D, 0x11
//#define UUID_TX 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F, 0x9D, 0x12
#define UUID_RX		0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18
#define UUID_TX		0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18
#define UUID_C3		0x04, 0x8F, 0x21, 0x83, 0x8A, 0x74, 0x7D, 0xB8, 0xF2, 0x45, 0x72, 0x87, 0x38, 0x02, 0x63, 0x64

T_SERVER_ID bt_matter_adapter_service_id;
/**<  Value of bt config characteristic. */
static uint8_t bt_matter_adapter_char_read_value[BT_MATTER_ADAPTER_READ_V1_MAX_LEN];
static uint16_t bt_matter_adapter_char_read_len = 1;

/**<  Function pointer used to send event to application from ble config wifi profile. Initiated in bt_matter_adapter_service_add_service. */
static P_FUN_SERVER_GENERAL_CB pfn_bt_matter_adapter_service_cb = NULL;

/**< @brief  profile/service definition.  */
/**should changed according BT team **/
T_ATTRIB_APPL bt_matter_adapter_service_tbl[] =
{
/* <<Primary Service>>, .. */
{
(ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_LE), /* flags */
{ /* type_value */
LO_WORD(GATT_UUID_PRIMARY_SERVICE),
HI_WORD(GATT_UUID_PRIMARY_SERVICE),
LO_WORD(0xFFF6), /* service UUID */
HI_WORD(0xFFF6)
},
UUID_16BIT_SIZE, /* bValueLen */
NULL, /* p_value_context */
GATT_PERM_READ /* permissions */
},
/* <<Characteristic>> Data RX */
{
ATTRIB_FLAG_VALUE_INCL, /* flags */
{ /* type_value */
LO_WORD(GATT_UUID_CHARACTERISTIC),
HI_WORD(GATT_UUID_CHARACTERISTIC),
(GATT_CHAR_PROP_WRITE | GATT_CHAR_PROP_WRITE_NO_RSP) /* characteristic properties */
/* characteristic UUID not needed here, is UUID of next attrib. */
},
1, /* bValueLen */
NULL,
GATT_PERM_READ /* permissions */
},
{
ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT, /* flags */
{ /* type_value */
UUID_RX
},
0, /* bValueLen */
NULL,
GATT_PERM_WRITE /* permissions */
},
/* <<Characteristic>> Data TX */
{
ATTRIB_FLAG_VALUE_INCL, /* flags */
{ /* type_value */
LO_WORD(GATT_UUID_CHARACTERISTIC),
HI_WORD(GATT_UUID_CHARACTERISTIC),
(GATT_CHAR_PROP_READ | GATT_CHAR_PROP_NOTIFY) /* characteristic properties */
/* characteristic UUID not needed here, is UUID of next attrib. */
},
1, /* bValueLen */
NULL,
GATT_PERM_READ /* permissions */
},
{
ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT, /* flags */
{ /* type_value */
UUID_TX
},
0, /* bValueLen */
NULL,
GATT_PERM_READ //GATT_PERM_NONE // permissions 
},
/* client characteristic configuration */
{
ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_CCCD_APPL, /* flags */
{ /* type_value */
LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
/* NOTE: this value has an instantiation for each client, a write to */
/* this attribute does not modify this default value: */
LO_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT), /* client char. config. bit field */
HI_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT)
},
2, /* bValueLen */
NULL,
(GATT_PERM_READ | GATT_PERM_WRITE) /* permissions */
},

/* <<Characteristic>> C3 Data TX */
{
ATTRIB_FLAG_VALUE_INCL, /* flags */
{ /* type_value */
LO_WORD(GATT_UUID_CHARACTERISTIC),
HI_WORD(GATT_UUID_CHARACTERISTIC),
(GATT_CHAR_PROP_READ) /* characteristic properties */
/* characteristic UUID not needed here, is UUID of next attrib. */
},
1, /* bValueLen */
NULL,
GATT_PERM_READ /* permissions */
},
{
ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT, /* flags */
{ /* type_value */
UUID_C3
},
0, /* bValueLen */
NULL,
GATT_PERM_READ //GATT_PERM_NONE // permissions
},
};


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
bool bt_matter_adapter_service_set_parameter(T_BTCONFIG_PARAM_TYPE param_type, uint16_t len, void *p_value)
{
	bool ret = true;

	switch (param_type) {
		case BTCONFIG_SERVICE_PARAM_V1_READ_CHAR_VAL:
			if  (len <= BT_MATTER_ADAPTER_READ_V1_MAX_LEN) {
				memcpy(bt_matter_adapter_char_read_value, p_value, len);
				bt_matter_adapter_char_read_len = len;
			} else {
				ret = false;
			}
		break;
		
		default:
			ret = false;
		break;
	}

	if (!ret) {
		APP_PRINT_ERROR0("bt_matter_adapter_service_set_parameter failed");
	}

	return ret;
}

/**
 * @brief read characteristic data from service.
 *
 * @param service_id          ServiceID of characteristic data.
 * @param attrib_index        Attribute index of getting characteristic data.
 * @param offset              Used for Blob Read.
 * @param p_length            length of getting characteristic data.
 * @param pp_value            data got from service.
 * @return Profile procedure result
*/
T_APP_RESULT  bt_matter_adapter_service_attr_read_cb(uint8_t conn_id, T_SERVER_ID service_id,
                                            uint16_t attrib_index, uint16_t offset, uint16_t *p_length, uint8_t **pp_value)
{
    T_APP_RESULT  cause  = APP_RESULT_SUCCESS;

    switch (attrib_index)
    {
    default:
        printf("bt_matter_adapter_service_attr_read_cb, Attr not found, index %d", attrib_index);
        cause = APP_RESULT_ATTR_NOT_FOUND;
        break;
    case BT_MATTER_ADAPTER_SERVICE_CHAR_V1_READ_WRITE_INDEX:
        {
            TBTCONFIG_CALLBACK_DATA callback_data;
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE;
			callback_data.msg_data.read_value_index = BTCONFIG_READ_V1;
			callback_data.msg_data.read_offset = offset;
            callback_data.conn_id = conn_id;
            if (pfn_bt_matter_adapter_service_cb)
            {
                pfn_bt_matter_adapter_service_cb(service_id, (void *)&callback_data);
            }
            *pp_value = bt_matter_adapter_char_read_value;
            *p_length = bt_matter_adapter_char_read_len;
			//printf("[BT_MATTER_ADAPTER] Read %d\n\r", bt_matter_adapter_char_read_len);
        }
        break;

    case BT_MATTER_ADAPTER_SERVICE_C3_INDEX:
        {
            TBTCONFIG_CALLBACK_DATA callback_data;
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE;
			callback_data.msg_data.read_value_index = BTCONFIG_READ_V1;
			callback_data.msg_data.read_offset = offset;
            callback_data.conn_id = conn_id;
            if (pfn_bt_matter_adapter_service_cb)
            {
                pfn_bt_matter_adapter_service_cb(service_id, (void *)&callback_data);
            }
            *pp_value = callback_data.msg_data.write.p_value;
            *p_length = callback_data.msg_data.write.len;
        }
        break;
    }

    return (cause);
}

/**
 * @brief write characteristic data from service.
 *
 * @param conn_id
 * @param service_id        ServiceID to be written.
 * @param attrib_index      Attribute index of characteristic.
 * @param length            length of value to be written.
 * @param p_value           value to be written.
 * @return Profile procedure result
*/
T_APP_RESULT bt_matter_adapter_service_attr_write_cb(uint8_t conn_id, T_SERVER_ID service_id,
                                            uint16_t attrib_index, T_WRITE_TYPE write_type, uint16_t length, uint8_t *p_value,
                                            P_FUN_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
	//printf("[BT_MATTER_ADAPTER] Write %d\n\r", length);
	TBTCONFIG_CALLBACK_DATA callback_data;
	T_APP_RESULT  cause = APP_RESULT_SUCCESS;
	APP_PRINT_INFO1("bt_matter_adapter_service_attr_write_cb write_type = 0x%x", write_type);
	*p_write_ind_post_proc = NULL;

	if (BT_MATTER_ADAPTER_SERVICE_CHAR_V1_READ_WRITE_INDEX == attrib_index) {
		/* Make sure written value size is valid. */
		if (p_value == NULL) {
			cause  = APP_RESULT_INVALID_VALUE_SIZE;
		} else {
			/* Notify Application. */
			callback_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
			callback_data.conn_id  = conn_id;
			callback_data.msg_data.write.opcode = BTCONFIG_WRITE_V1;
			callback_data.msg_data.write.write_type = write_type;
			callback_data.msg_data.write.len = length;
			callback_data.msg_data.write.p_value = p_value;

			//handle_bt_matter_adapter_app_data(p_value, length);
			if (pfn_bt_matter_adapter_service_cb) {
				pfn_bt_matter_adapter_service_cb(service_id, (void *)&callback_data);
			}
		}
	} else {
			APP_PRINT_ERROR2("bt_matter_adapter_service_attr_write_cb Error: attrib_index 0x%x, length %d",
			 attrib_index, length);
			cause = APP_RESULT_ATTR_NOT_FOUND;
	}

	return cause;
}

/**
 * @brief update CCCD bits from stack.
 *
 * @param conn_id           connection id.
 * @param service_id          Service ID.
 * @param index          Attribute index of characteristic data.
 * @param cccbits         CCCD bits from stack.
 * @return None
*/
void bt_matter_adapter_service_cccd_update_cb(uint8_t conn_id, T_SERVER_ID service_id, uint16_t index,
                                     uint16_t cccbits)
{
    TSIMP_CALLBACK_DATA callback_data;
    bool is_handled = false;
    callback_data.conn_id = conn_id;
    callback_data.msg_type = SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION;
    //printf("simp_ble_service_cccd_update_cb: index = %d, cccbits 0x%x\r\n", index, cccbits);
    switch (index)
    {
    case BT_MATTER_ADAPTER_SERVICE_CHAR_NOTIFY_CCCD_INDEX:
        {
            if (cccbits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
            {
                // Enable Notification
                callback_data.msg_data.notification_indification_index = SIMP_NOTIFY_INDICATE_V3_ENABLE;
            }
            else
            {
                // Disable Notification
                callback_data.msg_data.notification_indification_index = SIMP_NOTIFY_INDICATE_V3_DISABLE;
            }
            is_handled =  true;
        }
        break;
    case SIMPLE_BLE_SERVICE_CHAR_INDICATE_CCCD_INDEX:
        {
            if (cccbits & GATT_CLIENT_CHAR_CONFIG_INDICATE)
            {
                // Enable Indication
                callback_data.msg_data.notification_indification_index = SIMP_NOTIFY_INDICATE_V4_ENABLE;
            }
            else
            {
                // Disable Indication
                callback_data.msg_data.notification_indification_index = SIMP_NOTIFY_INDICATE_V4_DISABLE;
            }
            is_handled =  true;
        }
        break;

    default:
        break;
    }
    /* Notify Application. */
    if (pfn_bt_matter_adapter_service_cb && (is_handled == true))
    {
        pfn_bt_matter_adapter_service_cb(service_id, (void *)&callback_data);
    }
}

/**
 * @brief Simple ble Service Callbacks.
*/
const T_FUN_GATT_SERVICE_CBS bt_matter_adapter_service_cbs =
{
    bt_matter_adapter_service_attr_read_cb,  // Read callback function pointer
    bt_matter_adapter_service_attr_write_cb, // Write callback function pointer
    bt_matter_adapter_service_cccd_update_cb // CCCD update callback function pointer
};

/**
  * @brief Add simple BLE service to the BLE stack database.
  *
  * @param[in]   p_func  Callback when service attribute was read, write or cccd update.
  * @return Service id generated by the BLE stack: @ref T_SERVER_ID.
  * @retval 0xFF Operation failure.
  * @retval others Service id assigned by stack.
  *
  */
T_SERVER_ID bt_matter_adapter_service_add_service(void *p_func)
{
    if (false == server_add_service(&bt_matter_adapter_service_id,
                                    (uint8_t *)bt_matter_adapter_service_tbl,
                                    sizeof(bt_matter_adapter_service_tbl),
                                    bt_matter_adapter_service_cbs))
    {
   // 	 DBG_8195A("bt_matter_adapter_service_add_service: fail\n\r");
        APP_PRINT_ERROR0("bt_matter_adapter_service_add_service: fail");
        bt_matter_adapter_service_id = 0xff;
        return bt_matter_adapter_service_id;
    }

    pfn_bt_matter_adapter_service_cb = (P_FUN_SERVER_GENERAL_CB)p_func;
    return bt_matter_adapter_service_id;
}
#endif
