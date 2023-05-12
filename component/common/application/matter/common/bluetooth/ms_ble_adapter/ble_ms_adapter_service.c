#include <platform_opts_bt.h>
#if defined(CONFIG_BT_MS_ADAPTER) && CONFIG_BT_MS_ADAPTER
#include <gap.h>
#include <os_mem.h>
#include <profile_server.h>
#include "ble_ms_adapter_app.h"
#include <string.h>
#include "platform_stdlib.h"
#include "ms_hal_ble.h"
#include "os_sync.h"

BMS_SERVICE_INFO ble_matter_adapter_srvs_head;
BMS_SERVICE_INFO *ble_matter_adapter_srv_p = &ble_matter_adapter_srvs_head;
static P_FUN_SERVER_GENERAL_CB ble_matter_adapter_service_cb = NULL;
static uint8_t bt_matter_char_write_value[MS_WRITE_MAX_LEN];
//extern void *matter_add_service_sem;
uint8_t ble_matter_adapter_srvs_num = 0;

void ble_matter_adapter_free_service_info(BMS_SERVICE_INFO *service_info)
{
	for (int i = 0; i < service_info->att_num; i ++) {
		if (service_info->att_tbl[i].p_value_context != NULL) {
			os_mem_free(service_info->att_tbl[i].p_value_context);
		}
	}
	os_mem_free(service_info->att_tbl);
	os_mem_free(service_info->cbInfo);
	os_mem_free(service_info);
}

void ble_matter_adapter_move_pointer_and_free_service(BMS_SERVICE_INFO *service_info)
{
	BMS_SERVICE_INFO *pnext = service_info->next;
	BMS_SERVICE_INFO *p_srv = &ble_matter_adapter_srvs_head;

	while (p_srv) {
		if (p_srv->next == service_info) {
			p_srv->next = pnext;
			if (pnext == NULL) {
				ble_matter_adapter_srv_p = p_srv;
			}
			break;
		} else {
			p_srv = p_srv->next;
		}
	}
	ble_matter_adapter_free_service_info(service_info);
}

void ble_matter_adapter_search_and_free_service(T_SERVER_ID service_id)
{
	BMS_SERVICE_INFO *p_srv = &ble_matter_adapter_srvs_head;
	BMS_SERVICE_INFO *pnext;

	while (p_srv) {
		pnext = p_srv->next;
		if (pnext->srvId == service_id) {
			p_srv->next = pnext->next;
			if (pnext->next == NULL) {
				ble_matter_adapter_srv_p = p_srv;
			}
			ble_matter_adapter_srvs_num --;
			ble_matter_adapter_free_service_info(pnext);
			break;
		} else {
			p_srv = p_srv->next;
		}
	}

}

bool ble_matter_adapter_send_indication_notification(uint8_t conn_id, uint8_t service_id, uint8_t handle,
		uint8_t *p_value, uint16_t length, bool type)
{
	if (p_value == NULL) {
		return false;
	}
	printf("[%s] service_id %d index %d\r\n", __FUNCTION__, service_id, handle);
	T_GATT_PDU_TYPE pdu_type;
	if (type == 1) {
		pdu_type = GATT_PDU_TYPE_INDICATION;
	} else {
		pdu_type = GATT_PDU_TYPE_NOTIFICATION;
	}
	return server_send_data(conn_id, service_id, handle, p_value, length, pdu_type);
}


T_APP_RESULT ble_matter_adapter_attr_write_cb(uint8_t conn_id, T_SERVER_ID service_id,
		uint16_t attrib_index, T_WRITE_TYPE write_type, uint16_t length, uint8_t *p_value,
		P_FUN_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
	T_APP_RESULT  cause = APP_RESULT_SUCCESS;
	printf("[%s] service_id %d index 0x%x\r\n", __FUNCTION__, service_id, attrib_index);
	T_MATTER_ADAPTER_CALLBACK_DATA callback_data;

	/* Make sure written value size is valid. */
	if (p_value == NULL) {
		cause  = APP_RESULT_INVALID_VALUE_SIZE;
	} else {
		/* Notify Application. */
		callback_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
		callback_data.conn_id  = conn_id;
		callback_data.srv_id = service_id;
		callback_data.msg_data.write.write_type = write_type;
		memcpy(bt_matter_char_write_value, p_value, length);
		callback_data.msg_data.write.p_value = bt_matter_char_write_value;
		callback_data.msg_data.write.len = length;

		BMS_SERVICE_INFO *p = ble_matter_adapter_srvs_head.next;
		while (p) {
			if (p->srvId == service_id) {
				break;
			} else {
				p = p->next;
			}
		}
		if (p) {
			callback_data.msg_data.write.write_cb = (p->cbInfo[attrib_index]).func.write_cb;
		} else {
			callback_data.msg_data.write.write_cb = NULL;
			printf("[%s] can not find write callback\r\n");
		}

		if (ble_matter_adapter_service_cb) {
			ble_matter_adapter_service_cb(service_id, (void *)&callback_data);
		}
	}

	return (cause);
}

void ble_matter_adapter_cccd_update_cb(uint8_t conn_id, T_SERVER_ID service_id, uint16_t attrib_index,
								   uint16_t cccbits)
{
	T_MATTER_ADAPTER_CALLBACK_DATA callback_data;
	callback_data.msg_type = SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION;
	callback_data.conn_id = conn_id;
	callback_data.srv_id = service_id;
	callback_data.msg_data.cccd.attr_index = attrib_index;
	callback_data.msg_data.cccd.ccc_val = cccbits;

	/* Notify Application. */
	if (ble_matter_adapter_service_cb) {
		ble_matter_adapter_service_cb(service_id, (void *)&callback_data);
	}
}
const T_FUN_GATT_SERVICE_CBS ble_matter_adapter_service_cbs = {
	NULL,   /* Read callback function pointer */
	ble_matter_adapter_attr_write_cb,  /* Write callback function pointer */
	ble_matter_adapter_cccd_update_cb  /* CCCD update callback function pointer */
};

T_SERVER_ID ble_matter_adapter_add_service(BMS_SERVICE_INFO *service_info, void *p_func)
{
	if (service_info == NULL || service_info->att_tbl == NULL) {
		return 0xff;
	}
	if (false == server_add_service(&(service_info->srvId),
									(uint8_t *) service_info->att_tbl,
									(service_info->att_num) * sizeof(T_ATTRIB_APPL),
									ble_matter_adapter_service_cbs)) {
		printf("\r\n[%s] add service fail", __FUNCTION__);
		ble_matter_adapter_move_pointer_and_free_service(service_info);
		return 0xff;
	} else {
		printf("[%s] add service %d success\n", __FUNCTION__, service_info->srvId);
		ble_matter_adapter_srvs_num ++;
	}
	if (ble_matter_adapter_service_cb == NULL) {

		ble_matter_adapter_service_cb = (P_FUN_SERVER_GENERAL_CB) p_func;
	}
	return service_info->srvId;
}

static uint32_t switch_perm(uint8_t perm)
{
	uint32_t permission = GATT_PERM_NONE;

	if (perm & 0x01) {
		permission = permission | GATT_PERM_READ;
	} else if (perm & 0x02) {
		permission = permission | GATT_PERM_WRITE;
	} else if (perm & 0x04) {
		permission = permission | GATT_PERM_NOTIF_IND;
	}

	return permission;
}


static int setup_ble_serv_dec_attr(T_ATTRIB_APPL *attr, uint8_t *value, uint8_t *uuid, uint8_t uuid_type)
{
	attr->type_value[0] = LO_WORD(GATT_UUID_PRIMARY_SERVICE);       /* type */
	attr->type_value[1] = HI_WORD(GATT_UUID_PRIMARY_SERVICE);

	if (uuid_type == ENUM_MS_HAL_BLE_UUID_TYPE_16_BIT) {
		attr->flags = ATTRIB_FLAG_LE | ATTRIB_FLAG_VALUE_INCL;
		attr->type_value[2] = uuid[0];        /* value */
		attr->type_value[3] = uuid[1];
		attr->p_value_context = NULL;
		attr->value_len = 2;
	} else if (uuid_type == ENUM_MS_HAL_BLE_UUID_TYPE_128_bit) {
		attr->flags = ATTRIB_FLAG_LE | ATTRIB_FLAG_VOID;
		attr->value_len = 16;
		attr->p_value_context = os_mem_alloc(0, attr->value_len);
		memset(attr->p_value_context, 0, attr->value_len);
		memcpy(attr->p_value_context, value, 16);
	}
	attr->permissions = GATT_PERM_READ;

	return 0;
}

static int setup_ble_char_dec_attr(T_ATTRIB_APPL *attr, uint8_t prop)
{
	attr->flags = ATTRIB_FLAG_VALUE_INCL;
	attr->type_value[0] = LO_WORD(GATT_UUID_CHARACTERISTIC);
	attr->type_value[1] = HI_WORD(GATT_UUID_CHARACTERISTIC);
	attr->type_value[2] = prop;
	attr->value_len = 1;
	attr->p_value_context = NULL;
	attr->permissions = GATT_PERM_READ;
	return 0;
}


static int setup_ble_char_value_desc_attr(T_ATTRIB_APPL *attr, uint8_t *value, uint8_t *uuid, uint8_t uuid_type, uint8_t perm)
{
	attr->permissions = switch_perm(perm);
	if (uuid_type == ENUM_MS_HAL_BLE_UUID_TYPE_16_BIT) {
		attr->flags = ATTRIB_FLAG_VALUE_APPL;
		attr->type_value[0] = uuid[0];        /* value */
		attr->type_value[1] = uuid[1];
	} else if (uuid_type == ENUM_MS_HAL_BLE_UUID_TYPE_128_bit) {
		attr->flags = ATTRIB_FLAG_UUID_128BIT | ATTRIB_FLAG_VALUE_APPL;
		memcpy(attr->type_value, uuid, 16);
	}
	attr->value_len = 0;
	attr->p_value_context = NULL;
	return 0;
}

static int setup_ble_char_cccd_attr(T_ATTRIB_APPL *attr)
{
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

static int setup_ble_char_user_desc_attr(T_ATTRIB_APPL *attr, uint8_t *value, uint16_t value_size, uint8_t perm)
{
	attr->permissions = switch_perm(perm);
	attr->flags = ATTRIB_FLAG_VOID | ATTRIB_FLAG_ASCII_Z;
	attr->type_value[0] = LO_WORD(GATT_UUID_CHAR_USER_DESCR);
	attr->type_value[1] = HI_WORD(GATT_UUID_CHAR_USER_DESCR);
	attr->value_len = value_size;
	attr->p_value_context = os_mem_alloc(0, attr->value_len);
	memset(attr->p_value_context, 0, attr->value_len);
	memcpy(attr->p_value_context, (void *)value, attr->value_len);

	return 0;
}
BMS_SERVICE_INFO *ble_matter_adapter_parse_srv_tbl(matter_hal_ble_service_attrib_t **profile, uint16_t attrib_count)
{
	BMS_SERVICE_INFO *new_srv = (BMS_SERVICE_INFO *) os_mem_alloc(0, sizeof(BMS_SERVICE_INFO));
	memset(new_srv, 0, sizeof(BMS_SERVICE_INFO));
	new_srv->att_tbl = (T_ATTRIB_APPL *) os_mem_alloc(0, attrib_count * sizeof(T_ATTRIB_APPL));
	new_srv->cbInfo = (BMS_SERVICE_CALLBACK_INFO *)os_mem_alloc(0, attrib_count * sizeof(BMS_SERVICE_CALLBACK_INFO));
	memset(new_srv->cbInfo, 0, sizeof(BMS_SERVICE_CALLBACK_INFO));
	new_srv->att_num = attrib_count;

	uint16_t i = 0; //attribute handle

	for (int j = 0 ; j < attrib_count; j ++) {
		if (profile[j]->att_type == ENUM_MS_HAL_BLE_ATTRIB_TYPE_SERVICE) {
			setup_ble_serv_dec_attr(&new_srv->att_tbl[i], profile[j]->value_context, profile[j]->uuid, profile[j]->uuid_type);
			i ++;
		} else if (profile[j]->att_type == ENUM_MS_HAL_BLE_ATTRIB_TYPE_CHAR) {
			setup_ble_char_dec_attr(&new_srv->att_tbl[i], profile[j]->prop);
			i ++;
		} else if (profile[j]->att_type == ENUM_MS_HAL_BLE_ATTRIB_TYPE_CHAR_VALUE) {
			setup_ble_char_value_desc_attr(&new_srv->att_tbl[i], profile[j]->value_context, profile[j]->uuid, profile[j]->uuid_type, profile[j]->perm);
			new_srv->cbInfo[i].att_handle = i;
			memcpy(&(new_srv->cbInfo[i].func), &(profile[j]->callback), sizeof(matter_hal_ble_attrib_callback_t));
			i ++;
		} else if (profile[j]->att_type == ENUM_MS_HAL_BLE_ATTRIB_TYPE_CHAR_CLIENT_CONFIG) {
			setup_ble_char_cccd_attr(&new_srv->att_tbl[i]);
			new_srv->cbInfo[i].att_handle = i;
			memcpy(&(new_srv->cbInfo[i].func), &(profile[j]->callback), sizeof(matter_hal_ble_attrib_callback_t));
			i ++;
		} else if (profile[j]->att_type == ENUM_MS_HAL_BLE_ATTRIB_TYPE_CHAR_USER_DESCR) {
			setup_ble_char_user_desc_attr(&new_srv->att_tbl[i], profile[j]->value_context, profile[j]->value_len, profile[j]->perm);
			new_srv->cbInfo[i].att_handle = i;
			memcpy(&(new_srv->cbInfo[i].func), &(profile[j]->callback), sizeof(matter_hal_ble_attrib_callback_t));
			i ++;
		} else {
			printf("\n\r[%s]:Unknow Attribute Type...\r\n", __func__);
		}
	}
	ble_matter_adapter_srv_p->next = new_srv;
	ble_matter_adapter_srv_p = ble_matter_adapter_srv_p->next;
	return new_srv;

}

#endif
