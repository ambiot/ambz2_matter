#include "platform_opts_bt.h"
#if defined(CONFIG_BT_MESH_PROVISIONER_OTA_CLIENT) && CONFIG_BT_MESH_PROVISIONER_OTA_CLIENT
#include <string.h>
#include <stdbool.h>
#include "log_service.h"
#if defined(CONFIG_BT_OTA_CENTRAL_CLIENT_W_REQ_CONFLICT) && CONFIG_BT_OTA_CENTRAL_CLIENT_W_REQ_CONFLICT
#include "insert_write.h"
#endif


uint8_t* bt_mesh_provisioner_ota_client_image = NULL;
uint8_t* bt_mesh_provisioner_ota_client_key = NULL;

extern void bt_mesh_provisioner_ota_client_app_discov_services(uint8_t conn_id, bool start);

/**
 * NOTE: OTA Image and AEK KEY
 */
bool bt_mesh_provisioner_ota_client_set_image(uint8_t* image)
{
	if (image == NULL) {
		printf("bt_mesh_provisioner_ota_client_set_image: image is NULL\r\n");
		return false;
	}

	bt_mesh_provisioner_ota_client_image = image;
	return true;
}

bool bt_mesh_provisioner_ota_client_set_key(uint8_t* key)
{
	if (key == NULL) {
		printf("bt_mesh_provisioner_ota_client_set_key: key is NULL\r\n");
		return false;
	}

	bt_mesh_provisioner_ota_client_key = key;
	return true;
}

/* start discovery services and ota process. */
int bt_mesh_provisioner_ota_client_start_ota(uint16_t subtype, void *arg)
{
	int argc = 0;
	char *argv[MAX_ARGC] = {0};
	int conn_id = 0;

	if (arg) {
		argc = parse_param(arg, argv);
	}

	conn_id = atoi(argv[1]);

#if defined(CONFIG_BT_OTA_CENTRAL_CLIENT_W_REQ_CONFLICT) && CONFIG_BT_OTA_CENTRAL_CLIENT_W_REQ_CONFLICT
	if(if_queue_in(1, conn_id, 0, 0, 0, NULL) == 0)
#endif
	{
		bt_mesh_provisioner_ota_client_app_discov_services(conn_id, true);
	}

	return 0;
}

/* ******************* wrap the function for conmon file of ota process ***************************** */
void bt_ota_central_client_app_discov_services(uint8_t conn_id, bool start)
{
	bt_mesh_provisioner_ota_client_app_discov_services(conn_id, start);
}

uint8_t* bt_ota_central_client_get_image(void)
{
	return bt_mesh_provisioner_ota_client_image;
}

uint8_t* bt_ota_central_client_get_key(void)
{
	return bt_mesh_provisioner_ota_client_key;
}

#endif
