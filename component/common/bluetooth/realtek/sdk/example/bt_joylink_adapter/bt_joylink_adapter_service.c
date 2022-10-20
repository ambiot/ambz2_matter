#include <gap.h>
#include <os_mem.h>
#include <profile_server.h>
#include "bt_joylink_adapter_peripheral_app.h"
#include <string.h>
#include "platform_stdlib.h"
#include "joylink_sdk.h"

static uint8_t bt_joylink_char_read_value[JOYLINK_READ_MAX_LEN];
static unsigned int bt_joylink_char_read_len = 0;

static P_FUN_SERVER_GENERAL_CB bt_joylink_service_cb = NULL;
T_SERVER_ID joylink_service_id;

bool bt_joylink_adapter_send_indication(uint8_t conn_id, uint8_t service_id, uint8_t handle,
										uint8_t *p_value, uint16_t length, T_GATT_PDU_TYPE type)
{
	if (p_value == NULL) {
		return false;
	}
	printf("\r\n[%s] service_id %d index %d", __FUNCTION__, service_id, handle);
	return server_send_data(conn_id, service_id, handle, p_value, length, type);
}

void bt_joylink_adapter_cccd_update_cb(uint8_t conn_id, T_SERVER_ID service_id, uint16_t attrib_index,
									   uint16_t cccbits)
{
	printf("\r\n[%s] service_id %d index 0x%x, cccbits = 0x%x ", __FUNCTION__, service_id, attrib_index, cccbits);
	T_JOYLINK_CALLBACK_DATA callback_data;
	callback_data.msg_type = SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION;
	callback_data.conn_id = conn_id;
	callback_data.srv_id = service_id;
	callback_data.msg_data.cccd.attr_index = attrib_index;
	callback_data.msg_data.cccd.ccc_val = cccbits;

	/* Notify Application. */
	if (bt_joylink_service_cb) {
		bt_joylink_service_cb(service_id, (void *)&callback_data);
	}
}

T_APP_RESULT bt_joylink_adapter_attr_write_cb(uint8_t conn_id, T_SERVER_ID service_id,
		uint16_t attrib_index, T_WRITE_TYPE write_type, uint16_t length, uint8_t *p_value,
		P_FUN_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
	T_APP_RESULT  cause = APP_RESULT_SUCCESS;
	printf("\r\n[%s] service_id %d index 0x%x", __FUNCTION__, service_id, attrib_index);
	T_JOYLINK_CALLBACK_DATA callback_data;

	if(BT_JOYLINK_ADAPTER_CHAR_WRITE_INDEX == attrib_index)
	{
		if (p_value == NULL) {
			cause  = APP_RESULT_INVALID_VALUE_SIZE;
		} else {
			/* Notify Application. */
			callback_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
			callback_data.conn_id  = conn_id;
			callback_data.srv_id = service_id;
			callback_data.msg_data.write.write_type = write_type;
			callback_data.msg_data.write.p_value = p_value;
			callback_data.msg_data.write.len = length;

			if (bt_joylink_service_cb) {
				bt_joylink_service_cb(service_id, (void *)&callback_data);
			}
		}
	}
	else
	{
		printf("simp_ble_service_attr_write_cb Error: attrib_index 0x%x, length %d",
                         attrib_index,
                         length);
        cause = APP_RESULT_ATTR_NOT_FOUND;
	
	}
	
	return (cause);
}


T_APP_RESULT  bt_joylink_adapter_attr_read_cb(uint8_t conn_id, T_SERVER_ID service_id,
		uint16_t attrib_index, uint16_t offset, uint16_t *p_length, uint8_t **pp_value)
{
	(void)offset;
	T_APP_RESULT  cause  = APP_RESULT_SUCCESS;

	printf("\r\n[%s] service_id %d index 0x%x", __FUNCTION__, service_id, attrib_index);
	bt_joylink_char_read_len = JOYLINK_READ_MAX_LEN;

	T_JOYLINK_CALLBACK_DATA callback_data;
	callback_data.msg_type = SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE;
	callback_data.conn_id = conn_id;
	callback_data.srv_id = service_id;
	callback_data.msg_data.read.p_value = bt_joylink_char_read_value;
	callback_data.msg_data.read.p_len = &bt_joylink_char_read_len;

	if (bt_joylink_service_cb) {
		bt_joylink_service_cb(service_id, (void *)&callback_data);
	}

	*pp_value = bt_joylink_char_read_value;
	*p_length = bt_joylink_char_read_len;

	return (cause);
}

const T_FUN_GATT_SERVICE_CBS bt_joylink_adapter_service_cbs = {
	bt_joylink_adapter_attr_read_cb,   /* Read callback function pointer */
	bt_joylink_adapter_attr_write_cb,  /* Write callback function pointer */
	bt_joylink_adapter_cccd_update_cb  /* CCCD update callback function pointer */
};

extern T_ATTRIB_APPL *joylink_service_table;
extern uint8_t CHAR_NUM;

T_SERVER_ID bt_joylink_adapter_add_service(void *p_func)
{
	uint16_t bt_joylink_attr_tbl_size = CHAR_NUM * sizeof(T_ATTRIB_APPL);
	if (false == server_add_service(&joylink_service_id,
									(uint8_t *) joylink_service_table,
									bt_joylink_attr_tbl_size,
									bt_joylink_adapter_service_cbs)) {
		printf("\r\n[%s] add service fail", __FUNCTION__);
		return 0xff;
	} else {
		printf("\r\n[%s] add service %d success", __FUNCTION__, joylink_service_id);
	}

	if (bt_joylink_service_cb == NULL) {
		bt_joylink_service_cb = (P_FUN_SERVER_GENERAL_CB) p_func;
	}
	return joylink_service_id;
}


static int setup_ble_char_indication_attr(uint8_t *uuid,T_ATTRIB_APPL *attr)
{
	///characteristic dec
	attr->flags = ATTRIB_FLAG_VALUE_INCL;
	attr->type_value[0] = LO_WORD(GATT_UUID_CHARACTERISTIC);
	attr->type_value[1] = HI_WORD(GATT_UUID_CHARACTERISTIC);
	attr->type_value[2] = GATT_CHAR_PROP_INDICATE;
	attr->value_len = 1;
	attr->p_value_context = NULL;
	attr->permissions = GATT_PERM_READ;
	//characteristic value dec
	attr ++;
	
	attr->permissions = GATT_PERM_NONE;
	attr->flags = ATTRIB_FLAG_UUID_128BIT | ATTRIB_FLAG_VALUE_APPL;
	memcpy(attr->type_value, uuid, 16);
	
	attr->value_len = 0;
	attr->p_value_context = NULL;
	//cccd
	attr ++;
	attr->flags = ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_CCCD_APPL;
	attr->type_value[0] = LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG);
	attr->type_value[1] = HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG);
	attr->type_value[2] = 0;
	attr->type_value[3] = 0;
	attr->p_value_context = NULL;
	attr->value_len = 2;
	attr->permissions = GATT_PERM_READ | GATT_PERM_WRITE;

	return 0;
}


static int setup_ble_char_write_attr(uint8_t *uuid,T_ATTRIB_APPL *attr)
{
	///characteristic dec
	attr->flags = ATTRIB_FLAG_VALUE_INCL;
	attr->type_value[0] = LO_WORD(GATT_UUID_CHARACTERISTIC);
	attr->type_value[1] = HI_WORD(GATT_UUID_CHARACTERISTIC);
	attr->type_value[2] = GATT_CHAR_PROP_WRITE;
	attr->value_len = 1;
	attr->p_value_context = NULL;
	attr->permissions = GATT_PERM_READ;
	//characteristic value dec
	attr ++;
	
	attr->permissions = GATT_PERM_WRITE;
	attr->flags = ATTRIB_FLAG_UUID_128BIT | ATTRIB_FLAG_VALUE_APPL;
	memcpy(attr->type_value, uuid, 16);
	
	attr->value_len = 0;
	attr->p_value_context = NULL;

	return 0;

}


static int setup_ble_serv_dec_attr(uint8_t *uuid,T_ATTRIB_APPL *attr)
{

	attr->type_value[0] = LO_WORD(GATT_UUID_PRIMARY_SERVICE);       /* type */
	attr->type_value[1] = HI_WORD(GATT_UUID_PRIMARY_SERVICE);
	
	attr->flags = ATTRIB_FLAG_LE | ATTRIB_FLAG_VOID;
	attr->value_len = 16;
	attr->p_value_context = os_mem_alloc(0, attr->value_len);
	memset(attr->p_value_context, 0, attr->value_len);
	memcpy(attr->p_value_context, (void *)uuid, attr->value_len);
	 
	attr->permissions = GATT_PERM_READ;
	
	return 0;
}
bool bt_joylink_adapter_parse_service_info()
{
	//get service data form JD ble sdk
	jl_gatt_data_t *service_data = (jl_gatt_data_t *)os_mem_alloc(0,sizeof(jl_gatt_data_t));
	jl_get_gatt_config_data(service_data);
	////joylink service include characteristic(indication) and characteristic(write)
	joylink_service_table = (T_ATTRIB_APPL *)os_mem_alloc(0,CHAR_NUM * sizeof(T_ATTRIB_APPL)); 
	memset(joylink_service_table,0,CHAR_NUM * sizeof(T_ATTRIB_APPL));
	//parse service data
	int i = 0;
	setup_ble_serv_dec_attr(service_data->service_uuid128,&joylink_service_table[i]);
	i ++;
	setup_ble_char_write_attr(service_data->chra_uuid128_write,&joylink_service_table[i]);
	i = i + 2;
	setup_ble_char_indication_attr(service_data->chra_uuid128_indicate,&joylink_service_table[i]);
	if(service_data != NULL)
		os_mem_free(service_data);
	return true;
}

