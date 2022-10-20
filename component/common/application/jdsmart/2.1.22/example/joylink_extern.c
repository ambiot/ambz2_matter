/* --------------------------------------------------
 * @brief:
 *
 * @version: 1.0
 *
 * @date: 08/01/2018
 *
 * @desc: Functions in this file must be implemented by the device developers when porting the Joylink SDK.
 *
 * --------------------------------------------------
 */
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#include "joylink.h"
#include "joylink_time.h"

#include "joylink_extern.h"
#include "joylink_extern_json.h"

#include "joylink_memory.h"
#include "joylink_socket.h"
#include "joylink_string.h"
#include "joylink_stdio.h"
//#include "joylink_stdint.h"
#include "joylink_log.h"
#include "joylink_time.h"
#include "joylink_thread.h"
#include "joylink_extern_ota.h"

#include "FreeRTOS.h"
#include "task.h"
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include "wifi/wifi_conf.h"
#include "flash_api.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "cJSON.h"
#include "sys_api.h"
#include "lwip_netconf.h"
#include "wlan_fast_connect/example_wlan_fast_connect.h"

#include <platform_opts.h>
#include <platform/platform_stdlib.h>
#include <errno.h>

#include "osdep_service.h"
#include <device_lock.h>

#if CONFIG_USE_POLARSSL
#include <lwip/sockets.h>
#include <polarssl/config.h>
#include <polarssl/memory.h>
#include <polarssl/ssl.h>
#elif CONFIG_USE_MBEDTLS /* CONFIG_USE_POLARSSL */
#include <mbedtls/config.h>
#include <mbedtls/platform.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#endif
#include "platform_opts_bt.h"
#if defined CONFIG_BT_JOYLINK_ADAPTER && CONFIG_BT_JOYLINK_ADAPTER
#include "joylink_sdk.h"
#include "bt_joylink_adapter_app_flags.h"
#endif
user_dev_status_t user_dev;
extern struct netif xnetif[NET_IF_NUM];

JLPInfo_t user_jlp = {
	.version = JLP_VERSION,
	.uuid = JLP_UUID,
	.devtype = JLP_DEV_TYPE,
	.lancon = JLP_LAN_CTRL,
	.cmd_tran_type = JLP_CMD_TYPE,
	//.prikey = JLP_PRI_KEY,
	.noSnapshot = JLP_SNAPSHOT,
};

jl2_d_idt_t user_idt = {
	.type = 0,
	.cloud_pub_key = IDT_CLOUD_PUB_KEY,
	.sig = "01234567890123456789012345678901",
	.pub_key = "01234567890123456789012345678901",
	.f_sig = "01234567890123456789012345678901",
	.f_pub_key = "01234567890123456789012345678901",
};

user_dev_status_t user_dev = {
	.Power = 1,
	.Mode = 0,
	.State = 0,
};

#if 1	
int joylink_set_feedid(const char *json_in)
{
	uint8_t backup[300]	= {0};
	uint8_t readfirst[50]	= {0};
	int writeok	= -1;
	int readok		= -1;
	flash_t flash;

	if (json_in == NULL) {
		return -1;
	}
	// read first
	readok = flash_stream_read(&flash, FLASH_ADD_JLP_INFO + 50 * 1, 50, readfirst);
	if (1 == readok) {
		// write directly
		if (*((uint32_t *)readfirst) == ~0x0) {
			writeok = flash_stream_write(&flash, FLASH_ADD_JLP_INFO + 50 * 1, 50, (unsigned char *)json_in);
			if (writeok != 1) {
				log_debug("[%s] write err", __func__);
				return -1;
			}
		} else {
			// the same thing, return directly
			if (!strcmp(readfirst, json_in)) {
				log_info("[%s] the same feedid", __func__);
			}
			// not the same, erase old -> write new
			else {
				flash_stream_read(&flash, FLASH_ADD_JLP_INFO, 300, backup);
				flash_erase_sector(&flash, FLASH_ADD_JLP_INFO);

				strncpy((char *)(backup + 50 * 1), json_in, 50);
				if (backup[50 * 2 - 1] != '\0' || flash_stream_write(&flash, FLASH_ADD_JLP_INFO, 300, backup) != 1) {
					log_debug("[%s] write err!", __func__);
					return -1;
				}
			}
		}
	} else {
		log_fatal("[%s] flash read first fail !", __func__);
		return -1;
	}
	return 0;
}

int joylink_set_accesskey(const char *json_in)
{
	uint8_t  backup[300]	= {0};
	uint8_t readfirst[50]	= {0};
	int writeok = -1;
	int readok		= -1;
	flash_t flash;

	if (json_in == NULL) {
		return -1;
	}
	// read first
	readok = flash_stream_read(&flash, FLASH_ADD_JLP_INFO + 50 * 2, 50, readfirst);
	if (1 == readok) {
		// write directly
		if (*((uint32_t *)readfirst) == ~0x0) {
			writeok = flash_stream_write(&flash, FLASH_ADD_JLP_INFO + 50 * 2, 50, (unsigned char *)json_in);
			if (writeok != 1) {
				log_debug("[%s] write err", __func__);
				return -1;
			}
		} else {
			// the same thing, return directly
			if (!strcmp(readfirst, json_in)) {
				log_info("[%s] the same accesskey", __func__);
			}
			// not the same, erase old -> write new
			else {
				flash_stream_read(&flash, FLASH_ADD_JLP_INFO, 300, backup);
				flash_erase_sector(&flash, FLASH_ADD_JLP_INFO);
				strncpy((char *)(backup + 50 * 2), json_in, 50);
				if (backup[50 * 3 - 1] != '\0' || flash_stream_write(&flash, FLASH_ADD_JLP_INFO, 300, backup) != 1) {
					log_debug("[%s] write err!", __func__);
					return -1;
				}
			}
		}
	} else {
		log_fatal("[%s] flash read first fail !", __func__);
		return -1;
	}
	return 0;
}

int joylink_set_localkey(const char *json_in)
{
	uint8_t  backup[300]	= {0};
	uint8_t readfirst[50]	= {0};
	int writeok = -1;
	int readok		= -1;
	flash_t flash;

	if (json_in == NULL) {
		return -1;
	}
	// read first
	readok = flash_stream_read(&flash, FLASH_ADD_JLP_INFO + 50 * 3, 50, readfirst);
	if (1 == readok) {
		// write directly
		if (*((uint32_t *)readfirst) == ~0x0) {
			writeok = flash_stream_write(&flash, FLASH_ADD_JLP_INFO + 50 * 3, 50, (unsigned char *)json_in);
			if (writeok != 1) {
				log_debug("[%s] write err", __func__);
				return -1;
			}
		} else {
			// the same thing, return directly
			if (!strcmp(readfirst, json_in)) {
				log_info("[%s] the same localkey", __func__);
			}
			// not the same, erase old -> write new
			else {
				flash_stream_read(&flash, FLASH_ADD_JLP_INFO, 300, backup);
				flash_erase_sector(&flash, FLASH_ADD_JLP_INFO);
				strncpy((char *)(backup + 50 * 3), json_in, 50);
				if (backup[50 * 4 - 1] != '\0' || flash_stream_write(&flash, FLASH_ADD_JLP_INFO, 300, backup) != 1) {
					log_debug("[%s] write err!", __func__);
					return -1;
				}
			}
		}
	} else {
		log_fatal("[%s] flash read first fail !", __func__);
		return -1;
	}
	return 0;
}

int joylink_set_server_info(const char *json_in)
{
	uint8_t backup[300]	= {0};
	uint8_t readfirst[50]	= {0};
	int writeok = -1;
	int readok		= -1;
	flash_t flash;

	if (json_in == NULL) {
		return -1;
	}
	// read first
	readok = flash_stream_read(&flash, FLASH_ADD_JLP_INFO + 50 * 5, 50, readfirst);
	if (1 == readok) {
		// write directly
		if (*((uint32_t *)readfirst) == ~0x0) {
			writeok = flash_stream_write(&flash, FLASH_ADD_JLP_INFO + 50 * 5, 50, (unsigned char *)json_in);
			if (writeok != 1) {
				log_debug("[%s] write err", __func__);
				return -1;
			}
		} else {
			// the same thing, return directly
			if (!strcmp(readfirst, json_in)) {
				log_info("[%s] the same server_info", __func__);
			}
			// not the same, erase old -> write new
			else {
				flash_stream_read(&flash, FLASH_ADD_JLP_INFO, 300, backup);
				flash_erase_sector(&flash, FLASH_ADD_JLP_INFO);

				strncpy((char *)(backup + 50 * 5), json_in, 50);
				if (backup[50 * 6 - 1] != '\0' || flash_stream_write(&flash, FLASH_ADD_JLP_INFO, 300, backup) != 1) {
					log_debug("[%s] write err!", __func__);
					return -1;
				}
			}
		}
	} else {
		log_fatal("[%s] flash read first fail !", __func__);
		return -1;
	}
	return 0;
}

int joylink_set_version(const char *json_in)
{
	uint8_t backup[300]	= {0};
	uint8_t  readfirst[50]	= {0};
	int writeok = -1;
	int readok		= -1;
	flash_t flash;

	if (json_in == NULL) {
		return -1;
	}
	// read first
	readok = flash_stream_read(&flash, FLASH_ADD_JLP_INFO + 50 * 4, 50, readfirst);
	if (1 == readok) {
		// write directly
		if (*((uint32_t *)readfirst) == ~0x0) {
			writeok = flash_stream_write(&flash, FLASH_ADD_JLP_INFO + 50 * 4, 50, (unsigned char *)json_in);
			if (writeok != 1) {
				log_debug("[%s] write err", __func__);
				return -1;
			}
		} else {
			// the same thing, return directly
			if (!strcmp(readfirst, json_in)) {
				log_info("[%s] the same version", __func__);
			}
			// not the same, erase old -> write new
			else {
				flash_stream_read(&flash, FLASH_ADD_JLP_INFO, 300, backup);
				flash_erase_sector(&flash, FLASH_ADD_JLP_INFO);
				strncpy((char *)(backup + 50 * 4), json_in, 50);
				if (backup[50 * 5 - 1] != '\0' || flash_stream_write(&flash, FLASH_ADD_JLP_INFO, 300, backup) != 1) {
					log_debug("[%s] write err!", __func__);
					return -1;
				}
			}
		}
	} else {
		log_fatal("[%s] flash read first fail !", __func__);
		return -1;
	}
	return 0;
}

int joylink_get_accesskey(char *buf, unsigned int buf_sz)
{
	int readok = -1;
	flash_t flash;

	readok = flash_stream_read(&flash, FLASH_ADD_JLP_INFO + 50 * 2, 50, (uint8_t *)buf);
	if (1 == readok) {
		if (*((uint32_t *)buf) == ~0x0) {
			log_fatal("[%s] flash read success, but no valid data !", __func__);
			return -1;
		} else {
			return 0;
		}
	} else {
		log_fatal("[%s] flash read fail !", __func__);
		return -1;
	}
}

int joylink_get_localkey(char *buf, unsigned int buf_sz)
{
	int readok = -1;
	flash_t flash;

	readok = flash_stream_read(&flash, FLASH_ADD_JLP_INFO + 50 * 3, 50, (uint8_t *)buf);
	if (1 == readok) {
		if (*((uint32_t *)buf) == ~0x0) {
			log_fatal("[%s] flash read success, but no valid data !", __func__);
			return -1;
		} else {
			return 0;
		}
	} else {
		log_fatal("[%s] flash read fail !", __func__);
		return -1;
	}
}

int joylink_get_feedid(char *buf, unsigned int buf_sz)
{
	int readok = -1;
	flash_t flash;

	readok = flash_stream_read(&flash, FLASH_ADD_JLP_INFO + 50 * 1, 50, (uint8_t *)buf);
	if (1 == readok) {
		if (*((uint32_t *)buf) == ~0x0) {
			log_fatal("[%s] flash read success, but no valid data !", __func__);
			return -1;
		} else {
			return 0;
		}
	} else {
		log_fatal("[%s] flash read fail !", __func__);
		return -1;
	}
}

int joylink_get_server_info(char *buf, unsigned int buf_sz)
{
	int readok = -1;
	flash_t flash;

	readok = flash_stream_read(&flash, FLASH_ADD_JLP_INFO + 50 * 5, 50, (uint8_t *)buf);
	if (1 == readok) {
		if (*((uint32_t *)buf) == ~0x0) {
			log_fatal("[%s] flash read success, but no valid data !", __func__);
			return -1;
		} else {
			return 0;
		}
	} else {
		log_fatal("[%s] flash read fail !", __func__);
		return -1;
	}
}

int joylink_get_version(char *buf, unsigned int buf_sz)
{
	int readok = -1;
	flash_t flash;

	readok = flash_stream_read(&flash, FLASH_ADD_JLP_INFO + 50 * 4, 50, (uint8_t *)buf);
	if (1 == readok) {
		if (*((uint32_t *)buf) == ~0x0) {
			log_fatal("[%s] flash read success, but no valid data !", __func__);
			return -1;
		} else {
			return 0;
		}
	} else {
		log_fatal("[%s] flash read fail !", __func__);
		return -1;
	}
}

/**
 * brief:
 *
 * @Param: jlp
 * @Param: pMsg
 *
 * @Returns:
 */
int
joylink_parse_jlp(JLPInfo_t *jlp, char *pMsg)
{
	int ret = -1;
	if (NULL == pMsg || NULL == jlp) {
		return ret;
	}
	cJSON *pVal;
	cJSON *pJson = cJSON_Parse(pMsg);

	if (NULL == pJson) {
		log_error("--->:ERROR: pMsg is NULL\n");
		goto RET;
	}

	pVal = cJSON_GetObjectItem(pJson, "uuid");
	if (NULL != pVal) {
		jl_platform_strcpy(jlp->uuid, pVal->valuestring);
	}

	pVal = cJSON_GetObjectItem(pJson, "feedid");
	if (NULL != pVal) {
		jl_platform_strcpy(jlp->feedid, pVal->valuestring);
	}

	pVal = cJSON_GetObjectItem(pJson, "accesskey");
	if (NULL != pVal) {
		jl_platform_strcpy(jlp->accesskey, pVal->valuestring);
	}

	pVal = cJSON_GetObjectItem(pJson, "localkey");
	if (NULL != pVal) {
		jl_platform_strcpy(jlp->localkey, pVal->valuestring);
	}

	pVal = cJSON_GetObjectItem(pJson, "version");
	if (NULL != pVal) {
		jlp->version = pVal->valueint;
	}

	cJSON_Delete(pJson);
	ret = 0;
RET:
	return ret;
}

#endif

/**
 * @brief: 用以返回一个整型随机数
 *
 * @param: 无
 *
 * @returns: 整型随机数
 */
int
joylink_dev_get_random()
{
	/**
	 *FIXME:must to do
	 */
	static unsigned long int next = 1;
	next = next * 1103515245 + 12345;
	return (int)(next / 65536) % (1134);
}

/**
 * @brief: 返回是否可以访问互联网
 *
 * @returns: E_JL_TRUE 可以访问, E_JL_FALSE 不可访问
 */
E_JLBOOL_t joylink_dev_is_net_ok()
{
	if (RTW_SUCCESS == wifi_is_ready_to_transceive(RTW_STA_INTERFACE)) {
		return E_JL_TRUE;
	} else {
		return E_JL_FALSE;
	}
}

/**
 * @brief: 此函数用作通知应用层设备与云端的连接状态.
 *
 * @param: st - 当前连接状态  0-Socket init, 1-Authentication, 2-Heartbeat
 *
 * @returns:
 */
E_JLRetCode_t
joylink_dev_set_connect_st(int st)
{
	/**
	*FIXME:must to do
	*/
	char buff[64] = {0};
	int ret = 0;

	jl_platform_sprintf(buff, "{\"conn_status\":\"%d\"}", st);
	log_info("--set_connect_st:%s\n", buff);
#if defined CONFIG_BT_JOYLINK_ADAPTER && CONFIG_BT_JOYLINK_ADAPTER
	static int iot_st = 0;
	if(iot_st != st){
		if(st == JL_SERVER_ST_WORK){
			JLPInfo_t jlp;
#if defined(BT_JOYLINK_ADAPTER_AUTO_DEINIT_BT) && BT_JOYLINK_ADAPTER_AUTO_DEINIT_BT
			extern int connect_status;
			connect_status = 1;
#endif
			memset(&jlp,0,sizeof(jlp));
			joylink_dev_get_jlp_info(&jlp);
			jl_send_net_config_state(E_JL_NET_CONF_ST_IOT_CONNECT_SUCCEED,jlp.feedid,jl_platform_strlen(jlp.feedid));	
			jl_send_net_config_state(E_JL_NET_CONF_ST_CLOUD_CONNECT_SUCCEED,NULL,0);
		}
		iot_st = st;
	}
#endif
	return ret;
}

/**
 * @brief: 传出激活信息
 *
 * @param[in] message: 激活信息
 *
 * @returns: 0:设置成功
 */
E_JLRetCode_t joylink_dev_active_message(char *message)
{
	log_info("message = %s", message);
	return 0;
}

/**
 * @brief: 存储JLP(Joylink Parameters)信息,将入参jlp结构中的信息持久化存储,如文件、设备flash等方式
 *
 * @param [in]: jlp-JLP structure pointer
 *
 * @returns:
 */
// write feedid, accesskey, localkey, joylink_server, server_port, gURLStr, gTokenStr into flash

E_JLRetCode_t
joylink_dev_set_attr_jlp(JLPInfo_t *jlp)
{
	if (NULL == jlp) {
		return E_RET_ERROR;
	}
	/**
	*FIXME:must to do
	*Must save jlp info to flash
	*/
	int ret = E_RET_ERROR;
#if 1
	char buff[256];

	JLPInfo_t *pJLPInfo = &(user_jlp);

	memset(buff, 0, sizeof(buff));
	if (strlen(jlp->feedid)) {
		strcpy(pJLPInfo->feedid, jlp->feedid);
		sprintf(buff, "{\"feedid\":\"%s\"}", jlp->feedid);
		log_debug("--set buff:%s", buff);
		if (joylink_set_feedid(buff)) {
			goto RET;
		}
	}

	memset(buff, 0, sizeof(buff));
	if (strlen(jlp->accesskey)) {
		strcpy(pJLPInfo->accesskey, jlp->accesskey);
		sprintf(buff, "{\"accesskey\":\"%s\"}", jlp->accesskey);
		log_debug("--set buff:%s", buff);
		if (joylink_set_accesskey(buff)) {
			goto RET;
		}
	}

	memset(buff, 0, sizeof(buff));
	if (strlen(jlp->localkey)) {
		strcpy(pJLPInfo->localkey, jlp->localkey);
		sprintf(buff, "{\"localkey\":\"%s\"}", jlp->localkey);
		log_debug("--set buff:%s", buff);
		if (joylink_set_localkey(buff)) {
			goto RET;
		}
	}

	memset(buff, 0, sizeof(buff));
	if (strlen(jlp->joylink_server)) {
		strcpy(pJLPInfo->joylink_server, jlp->joylink_server);
		pJLPInfo->server_port = jlp->server_port;
		sprintf(buff, "%s:%d", jlp->joylink_server, jlp->server_port);
		log_debug("--set buff:%s", buff);
		if (joylink_set_server_info(buff)) {
			goto RET;
		}
	}

	pJLPInfo->version = jlp->version;
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "{\"version\":%d}", jlp->version);
	log_debug("--set buff:%s", buff);
	if (joylink_set_version(buff)) {
		goto RET;
	}
#else
	memcpy(&user_jlp, jlp, sizeof(JLPInfo_t));
#ifdef _SAVE_FILE_
	FILE *outfile;
	outfile = fopen(file, "wb+");
	fwrite(&user_jlp, sizeof(JLPInfo_t), 1, outfile);
	fclose(outfile);
#endif
#endif
	ret = E_RET_OK;
	return ret;

#if 1
RET:
	log_error("set error");
#endif
	return ret;

}

/**
 * @brief: 设置设备认证信息
 *
 * @param[out]: pidt--设备认证信息结构体指针,需填入必要信息sig,pub_key,f_sig,f_pub_key,cloud_pub_key
 *
 * @returns: 返回设置成功或失败
 */
E_JLRetCode_t
joylink_dev_get_idt(jl2_d_idt_t *pidt)
{
	if (NULL == pidt) {
		return E_RET_ERROR;
	}
	pidt->type = 0;
	jl_platform_strcpy(pidt->sig, user_idt.sig);
	jl_platform_strcpy(pidt->pub_key, user_idt.pub_key);
	//jl_platform_strcpy(pidt->rand, user_idt.rand);
	jl_platform_strcpy(pidt->f_sig, user_idt.f_sig);
	jl_platform_strcpy(pidt->f_pub_key, user_idt.f_pub_key);
	jl_platform_strcpy(pidt->cloud_pub_key, user_idt.cloud_pub_key);

	return E_RET_OK;
}

/**
 * @brief: 此函数需返回设备的MAC地址
 *
 * @param[out] out: 将设备MAC地址赋值给此参数
 *
 * @returns: E_RET_OK 成功, E_RET_ERROR 失败
 */
E_JLRetCode_t
joylink_dev_get_user_mac(char *out)
{
	unsigned char *tmp;
	unsigned char mac[6] = {0};
	/*get mac address*/
	tmp = LwIP_GetMAC(&xnetif[0]);
	mac[0] = tmp[0];
	mac[1] = tmp[1];
	mac[2] = tmp[2];
	mac[3] = tmp[3];
	mac[4] = tmp[4];
	mac[5] = tmp[5];

	sprintf((char *)out, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	// backup in user_jlp
	strcpy(user_jlp.mac, out);
	return E_RET_OK;
}

/**
 * @brief: 此函数需返回设备私钥private key,该私钥可从小京鱼后台获取
 *
 * @param[out] out: 将私钥字符串赋值给该参数返回
 *
 * @returns: E_RET_OK:成功, E_RET_ERROR:发生错误
 */
E_JLRetCode_t
joylink_dev_get_private_key(char *out)
{
	strcpy(out, JLP_PRIV_KEY);
	printf("private_key=out: %s\n", out);
	return E_RET_OK;
}

/**
 * @brief: 从永久存储介质(文件或flash)中读取jlp信息,并赋值给参数jlp,其中feedid, accesskey,localkey,joylink_server,server_port必须正确赋值
 *
 * @param[out] jlp: 将JLP(Joylink Parameters)读入内存,并赋值给该参数
 *
 * @returns: E_RET_OK:成功, E_RET_ERROR:发生错误
 */
// read feedid, accesskey, localkey, joylink_server, server_port, gURLStr, gTokenStr from flash
E_JLRetCode_t
joylink_dev_get_jlp_info(JLPInfo_t *jlp)
{
	if (NULL == jlp) {
		return -1;
	}
	int ret = E_RET_OK;
#if 1
	char buff[256];
	memset(buff, 0, sizeof(buff));
	if (!joylink_get_accesskey(buff, sizeof(buff))) {
		log_debug("-->accesskey:%s\n", buff);
		joylink_parse_jlp(jlp, buff);
	} else {
		log_error("get accesskey error");
	}

	memset(buff, 0, sizeof(buff));
	if (!joylink_get_localkey(buff, sizeof(buff))) {
		log_debug("-->localkey:%s\n", buff);
		joylink_parse_jlp(jlp, buff);
	} else {
		log_error("get localkey error");
	}

	memset(buff, 0, sizeof(buff));
	if (!joylink_get_feedid(buff, sizeof(buff))) {
		log_debug("-->feedid:%s\n", buff);
		joylink_parse_jlp(jlp, buff);
	} else {
		log_error("get feedid error");
	}

	memset(buff, 0, sizeof(buff));
	if (!joylink_get_server_info(buff, sizeof(buff))) {
		log_info("-->server_info:%s\n", buff);
		joylink_util_cut_ip_port(buff, jlp->joylink_server, &jlp->server_port);
	} else {
		log_error("get server_info error");
	}

	memset(buff, 0, sizeof(buff));
	if (!joylink_get_version(buff, sizeof(buff))) {
		log_info("-->version:%s\n", buff);
		joylink_parse_jlp(jlp, buff);
	} else {
		log_error("get version error");
	}
#else

	JLPInfo_t fjlp;
	memset(&fjlp, 0, sizeof(JLPInfo_t));

#ifdef _SAVE_FILE_
	FILE *infile;
	infile = fopen(file, "rb+");
	if (infile > 0) {
		fread(&fjlp, sizeof(fjlp), 1, infile);
		fclose(infile);

		strcpy(user_jlp.feedid, fjlp.feedid);
		strcpy(user_jlp.accesskey, fjlp.accesskey);
		strcpy(user_jlp.localkey, fjlp.localkey);
		strcpy(user_jlp.joylink_server, fjlp.joylink_server);
		user_jlp.server_port = fjlp.server_port;
	}
#endif
	strcpy(jlp->feedid, user_jlp.feedid);
	strcpy(jlp->accesskey, user_jlp.accesskey);
	strcpy(jlp->localkey, user_jlp.localkey);
	strcpy(jlp->joylink_server, user_jlp.joylink_server);
	jlp->server_port = user_jlp.server_port;
#endif

	if (joylink_dev_get_user_mac(jlp->mac) < 0) {
		strcpy(jlp->mac, user_jlp.mac);
	}

	if (joylink_dev_get_private_key(jlp->prikey) < 0) {
		strcpy(jlp->prikey, user_jlp.prikey);
	}

	jlp->version = user_jlp.version;
	strcpy(jlp->uuid, user_jlp.uuid);
	jlp->devtype = user_jlp.devtype;
	jlp->lancon = user_jlp.lancon;
	jlp->cmd_tran_type = user_jlp.cmd_tran_type;

	jlp->noSnapshot = user_jlp.noSnapshot;
	//strcpy(jlp->joySdkVersion, user_jlp.joySdkVersion);//2.1.21 delete
	return ret;
}

/**
 * @brief: 返回设备状态,通过填充user_data参数,返回设备当前状态
 *
 * @param[out] user_data: 设备状态结构体指针
 *
 * @returns: 0
 */
int
joylink_dev_user_data_get(user_dev_status_t *user_data)
{
	user_data->Power = user_dev.Power;
	user_data->Mode = user_dev.Mode;
	user_data->State = user_dev.State;
	log_info("dev current Power: %d  Mode: %d  State: %d  Heap: %d", user_data->Power, user_data->Mode, user_data->State, xPortGetFreeHeapSize());
	return 0;
}

/**
 * @brief: 获取设备快照json结构,结构中包含返回状态码
 *
 * @param[in] ret_code: 返回状态码
 * @param[out] out_snap: 序列化为字符串的设备快照json结构
 * @param[in] out_max: out_snap可写入的最大长度
 *
 * @returns: 实际写入out_snap的数据长度
 */
int
joylink_dev_get_snap_shot_with_retcode(int32_t ret_code, char *out_snap, int32_t out_max)
{
	if (NULL == out_snap || out_max < 0) {
		return 0;
	}
	int len = 0;
	joylink_dev_user_data_get(&user_dev);
	char *packet_data =  joylink_dev_package_info(ret_code, &user_dev);
	if (NULL !=  packet_data) {
		len = jl_platform_strlen(packet_data);
		log_info("------>%s:len:%d\n", packet_data, len);
		if (len < out_max) {
			jl_platform_memcpy(out_snap, packet_data, len);
		} else {
			len = 0;
		}
	}

	if (NULL !=  packet_data) {
		jl_platform_free(packet_data);
	}
	return len;
}

/**
 * @brief: 获取设备快照json结构
 *
 * @param[out] out_snap: 序列化为字符串的设备快照json结构
 * @param[in] out_max: out_snap可写入的最大长度
 *
 * @returns: 实际写入out_snap的数据长度
 */
int
joylink_dev_get_snap_shot(char *out_snap, int32_t out_max)
{
	return joylink_dev_get_snap_shot_with_retcode(0, out_snap, out_max);
}

/**
 * @brief: 获取向App返回的设备快照json结构
 *
 * @param[out] out_snap: 序列化为字符串的设备快照json结构
 * @param[in] out_max: out_snap允许写入的最大长度
 * @param[in] code: 返回状态码
 * @param[in] feedid: 设备的feedid
 *
 * @returns:
 */
int
joylink_dev_get_json_snap_shot(char *out_snap, int32_t out_max, int code, char *feedid)
{
	jl_platform_sprintf(out_snap, "{\"code\":%d, \"feedid\":\"%s\"}", code, feedid);
	return jl_platform_strlen(out_snap);
}

/**
 * @brief: 通过App控制设备,需要实现此函数,根据传入的json_cmd对设备进行控制
 *
 * @param[in] json_cmd: 设备控制命令
 *
 * @returns: E_RET_OK:控制成功, E_RET_ERROR:发生错误
 */
E_JLRetCode_t
joylink_dev_lan_json_ctrl(const char *json_cmd)
{
	log_debug("json ctrl:%s", json_cmd);
	return E_RET_OK;
}

/**
 * @brief: 根据传入的cmd值,设置对应设备属性
 *
 * @param[in] cmd: 设备属性名称
 * @param[out] user_data: 设备状态结构体
 *
 * @returns: E_RET_OK 设置成功
 */
E_JLRetCode_t
joylink_dev_user_data_set(char *cmd, user_dev_status_t *user_data)
{
	user_dev.Power = user_data->Power;
	user_dev.Mode = user_data->Mode;
	user_dev.State = user_data->State;
	log_info("set dev Power: %d  Mode: %d  State: %d", user_dev.Power, user_dev.Mode, user_dev.State);
	return E_RET_OK;
}

/**
 * @brief:根据src参数传入的控制命令数据包对设备进行控制.调用joylink_dev_parse_ctrl进行控制命令解析,并更改设备属性值
 *
 * @param[in] src: 控制指令数据包
 * @param[in] src_len: src长度
 * @param[in] ctr: 控制码
 * @param[in] from_server: 是否来自server控制 0-App,2-Server
 *
 * @returns: E_RET_OK 成功, E_RET_ERROR 失败
 */
E_JLRetCode_t
joylink_dev_script_ctrl(const char *src, int src_len, JLContrl_t *ctr, int from_server)
{
	if (NULL == src || NULL == ctr) {
		return E_RET_ERROR;
	}

	int ret = -1;
	ctr->biz_code = (int)(*((int *)(src + 4)));
	ctr->serial = (int)(*((int *)(src + 8)));

	uint32_t tt = jl_time_get_timestamp(NULL);
	log_info("bcode:%d:server:%d:time:%ld", ctr->biz_code, from_server, (long)tt);

	if (ctr->biz_code == JL_BZCODE_GET_SNAPSHOT) {
		ret = E_RET_OK;
	} else if (ctr->biz_code == JL_BZCODE_CTRL) {
		joylink_dev_parse_ctrl(src + 12, &user_dev);
		return E_RET_OK;
	} else if (ctr->biz_code == JL_BZCODE_MENU) {
		joylink_dev_parse_ctrl(src + 12, &user_dev);
		return E_RET_OK;
	} else {
		char buf[50];
		jl_platform_sprintf(buf, "Unknown biz_code:%d", ctr->biz_code);
		log_error("%s", buf);
	}
	return ret;
}

/**
 * @brief: 实现接收到ota命令和相关参数后的动作,可使用otaOrder提供的参数进行具体的OTA操作
 *
 * @param[in] otaOrder: OTA命令结构体
 *
 * @returns: E_RET_OK 成功, E_RET_ERROR 发生错误
 */
E_JLRetCode_t
joylink_dev_ota(JLOtaOrder_t *otaOrder)
{
	jl_dev_ota_z2(otaOrder);
	return E_RET_OK;
}

/**
 * @brief: OTA执行状态上报,无需返回值
 */
void
joylink_dev_ota_status_upload()
{
	return;
}

static int joylink_dev_http_parse_content(
	char *response,
	int response_len,
	char *content,
	int content_len)
{
	int length = 0;
	content[0] = 0;
	char *p = jl_platform_strstr(response, "\r\n\r\n");
	if (p == NULL) {
		return -1;
	}
	p += 4;
	length = response_len - (p - response);
	length = length > content_len ? content_len - 1 : length;
	jl_platform_strncpy(content, p, length);
	content[length] = 0;
	// log_info("content = \r\n%s", content);

	return length;
}

#if CONFIG_USE_POLARSSL
static int jl_my_random(void *p_rng, unsigned char *output, size_t output_len)
{
	rtw_get_random_bytes(output, output_len);
	return 0;
}

int joylink_dev_https_post(char *host, char *query, char *revbuf, int buflen)
{
	int ret = 0;
	int server_fd = -1;
	struct sockaddr_in server_addr;
	ssl_context ssl;

	memory_set_own(pvPortMalloc, vPortFree);
	memset(&ssl, 0, sizeof(ssl_context));

	if ((ret = net_connect(&server_fd, host, 443)) != 0) {
		log_error("ERROR: net_connect ret(%d)", ret);
		goto exit;
	}

	if ((ret = ssl_init(&ssl)) != 0) {
		log_error("ERROR: ssl_init ret(%d)", ret);
		goto exit;
	}

	ssl_set_endpoint(&ssl, SSL_IS_CLIENT);
	ssl_set_authmode(&ssl, SSL_VERIFY_NONE);
	ssl_set_rng(&ssl, jl_my_random, NULL);
	ssl_set_bio(&ssl, net_recv, &server_fd, net_send, &server_fd);

	if ((ret = ssl_handshake(&ssl)) != 0) {
		log_error("ERROR: ssl_handshake ret(-0x%x)", -ret);
		goto exit;
	} else {
		char buf[1400] = {0};
		int read_size = 0, resource_size = 0, content_len = 0, header_removed = 0, recv_finish = 0;

		log_info("SSL ciphersuite %s", ssl_get_ciphersuite(&ssl));
		ssl_write(&ssl, query, strlen(query));
		log_info("query=[%s]", query); //
		while ((!recv_finish) && ((read_size = ssl_read(&ssl, buf, 1400 - 1)) > 0)) {

			char *response_end = NULL;
			response_end = strstr(buf, "\"code\":0}}");
			if (response_end) {
				recv_finish = 1;
			}

			if (header_removed == 0) {
				char *header = NULL;
				header = strstr(buf, "\r\n\r\n");

				if (header) {
					char *body, *content_len_pos;
					body = header + strlen("\r\n\r\n");
					*(body - 2) = 0;
					header_removed = 1;

					// Remove header size to get first read size of data from body head
					strcpy(revbuf, body);
					resource_size = read_size - (body - buf);

					content_len_pos = strstr(revbuf, "Content-Length: ");
					if (content_len_pos) {
						content_len_pos += strlen("Content-Length: ");
						*(char *)(strstr(content_len_pos, "\r\n")) = 0;
						content_len = atoi(content_len_pos);
					}
					memset(buf, 0, 1400);
				} else {
					memset(buf, 0, 1400);
				}
			} else {
				strcpy(revbuf + resource_size, buf);
				resource_size += read_size;
				memset(buf, 0, 1400);
			}
		}

		log_debug("https response is %s", revbuf);
		log_debug("https content-length = %d bytes, download resource size = %d bytes", content_len, resource_size);
	}

exit:

	if (server_fd >= 0) {
		net_close(server_fd);
	}

	ssl_free(&ssl);
	return ret;
}

#elif CONFIG_USE_MBEDTLS
static int jl_my_random(void *p_rng, unsigned char *output, size_t output_len)
{
	rtw_get_random_bytes(output, output_len);
	return 0;
}

static void *jl_my_calloc(size_t nelements, size_t elementSize)
{
	size_t size;
	void *ptr = NULL;
	size = nelements * elementSize;
	ptr = pvPortMalloc(size);
	if (ptr) {
		memset(ptr, 0, size);
	}
	return ptr;
}
static char *jl_atcmd_lwip_itoa(int value)
{
	char *val_str;
	int tmp = value, len = 1;

	while ((tmp /= 10) > 0) {
		len ++;
	}
	val_str = (char *) pvPortMalloc(len + 1);
	sprintf(val_str, "%d", value);
	return val_str;
}

int joylink_dev_https_post(char *host, char *query, char *revbuf, int buflen)
{

	int ret = 0;
	mbedtls_net_context server_fd;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;

	mbedtls_platform_set_calloc_free(jl_my_calloc, vPortFree);
	mbedtls_net_init(&server_fd);
	mbedtls_ssl_init(&ssl);
	mbedtls_ssl_config_init(&conf);

	int port = 443;
	char *jl_port_str = jl_atcmd_lwip_itoa(port);
	if ((ret = mbedtls_net_connect(&server_fd, host, jl_port_str, MBEDTLS_NET_PROTO_TCP)) != 0) {
		log_error("ERROR: mbedtls_net_connect ret(%d)", ret);
		goto exit;
	}

	mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
	if ((ret = mbedtls_ssl_config_defaults(&conf,
										   MBEDTLS_SSL_IS_CLIENT,
										   MBEDTLS_SSL_TRANSPORT_STREAM,
										   MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
		log_error("ERRPR: mbedtls_ssl_config_defaults ret(%d)", ret);
		goto exit;
	}

	mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
	mbedtls_ssl_conf_rng(&conf, jl_my_random, NULL);
	if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
		log_error("ERRPR: mbedtls_ssl_setup ret(%d)", ret);
		goto exit;
	}

	if ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
		log_error("ERROR: mbedtls_ssl_handshake ret(-0x%x)", -ret);
		goto exit;
	} else {
		char buf[1400] = {0};
		int read_size = 0, resource_size = 0, content_len = 0, header_removed = 0, recv_finish = 0;

		log_info("SSL ciphersuite %s", mbedtls_ssl_get_ciphersuite(&ssl));
		mbedtls_ssl_write(&ssl, query, strlen(query));
		log_info("query=\r\n%s", query);
		while ((!recv_finish) && ((read_size = mbedtls_ssl_read(&ssl, buf, 1400 - 1)) > 0)) {
			log_info("read buf=\r\n%s", buf);

			char *response_end = NULL;
			response_end = strstr(buf, "\"code\":0}}");
			if (response_end) {
				recv_finish = 1;
			}

			if (header_removed == 0) {
				char *header = NULL;
				//header = strstr(revbuf, "\r\n\r\n");
				header = strstr(buf, "\r\n\r\n");//fix

				if (header) {
					char *body, *content_len_pos;
					body = header + strlen("\r\n\r\n");
					*(body - 2) = 0;
					header_removed = 1;

					// Remove header size to get first read size of data from body head
					strcpy(revbuf, body);
					resource_size = read_size - (body - buf);

					content_len_pos = strstr(revbuf, "Content-Length: ");
					if (content_len_pos) {
						content_len_pos += strlen("Content-Length: ");
						*(strstr(content_len_pos, "\r\n")) = 0;
						content_len = atoi(content_len_pos);
					}
					memset(buf, 0, 1400);
				} else {
					memset(buf, 0, 1400);
				}
			} else {
				strcpy(revbuf + resource_size, buf);
				resource_size += read_size;
				memset(buf, 0, 1400);
			}
		}

		log_debug("https response is %s", revbuf);
		log_debug("https content-length = %d bytes, download resource size = %d bytes", content_len, resource_size);
	}

exit:
	mbedtls_net_free(&server_fd);
	mbedtls_ssl_free(&ssl);
	mbedtls_ssl_config_free(&conf);
	return ret;
}
#endif

/**
 * @brief 实现HTTP的POST请求,请求响应填入revbuf参数.
 *
 * @param[in]  host: POST请求的目标主机
 * @param[in]  query: POST请求的路径、HEADER和Payload
 * @param[out] revbuf: 填入请求的响应信息的Body
 * @param[in]  buflen: revbuf的最大长度
 *
 * @returns: 负值 - 发生错误, 非负 - 实际填充revbuf的长度
 *
 * @note: 此函数必须正确实现,否者设备无法校时,无法正常激活绑定
 *
 * */
int joylink_dev_http_post(char *host, char *query, char *revbuf, int buflen)
{
#if 1
	int log_socket = -1;
	int len = 0;
	int ret = -1;
	char *recv_buf = NULL;
	jl_sockaddr_in saServer;
	char ip[20] = {0};

	if (host == NULL || query == NULL || revbuf == NULL) {
		log_error("DNS lookup failed");
		goto RET;
	}

	memset(ip, 0, sizeof(ip));
	// ret = joylink_dev_get_host_ip(host, ip);
	ret = jl_platform_gethostbyname(host, ip, SOCKET_IP_ADDR_LEN);
	if (ret < 0) {
		log_error("get ip error");
		ret = -1;
		goto RET;
	}

	memset(&saServer, 0, sizeof(saServer));
	saServer.sin_family = jl_platform_get_socket_proto_domain(JL_SOCK_PROTO_DOMAIN_AF_INET);
	saServer.sin_port = jl_platform_htons(80);
	saServer.sin_addr.s_addr = jl_platform_inet_addr(ip);

	log_socket = jl_platform_socket(JL_SOCK_PROTO_DOMAIN_AF_INET, JL_SOCK_PROTO_TYPE_SOCK_STREAM, JL_SOCK_PROTO_PROTO_IPPROTO_TCP);
	if (log_socket < 0) {
		log_error("... Failed to allocate socket.");
		goto RET;
	}
	int reuseaddrEnable = 1;
	if (jl_platform_setsockopt(log_socket,
							   JL_SOCK_OPT_LEVEL_SOL_SOCKET,
							   JL_SOCK_OPT_NAME_SO_REUSEADDR,
							   (uint8_t *)&reuseaddrEnable,
							   sizeof(reuseaddrEnable)) < 0) {
		log_error("set SO_REUSEADDR error");
	}

	/*fcntl(log_socket,F_SETFL,fcntl(log_socket,F_GETFL,0)|O_NONBLOCK);*/

	if (jl_platform_connect(log_socket, (jl_sockaddr *)&saServer, sizeof(saServer)) != 0) {
		log_error("... socket connect failed");
		goto RET;
	}

	if (jl_platform_send(log_socket, query, jl_platform_strlen(query), 5000, 0) < 0) {
		log_error("... socket send failed");
		goto RET;
	}

	struct timeval receiving_timeout;
	receiving_timeout.tv_sec = 5;
	receiving_timeout.tv_usec = 0;
	if (jl_platform_setsockopt(log_socket,
							   JL_SOCK_OPT_LEVEL_SOL_SOCKET,
							   JL_SOCK_OPT_NAME_SO_RCVTIMEO,
							   &receiving_timeout,
							   sizeof(receiving_timeout)) < 0) {
		log_error("... failed to set socket receiving timeout");
		goto RET;
	}

	int recv_buf_len = 1024; //2048;
	recv_buf = (char *)jl_platform_malloc(recv_buf_len);
	if (recv_buf == NULL) {
		goto RET;
	}
	jl_platform_memset(recv_buf, 0, recv_buf_len);
	len = jl_platform_recv(log_socket, recv_buf, recv_buf_len, 0, 0);
	if (len <= 0) {
		ret = -1;
		goto RET;
	}
	log_info("... read data length = %d, response data = \r\n%s", len, recv_buf);
	ret = joylink_dev_http_parse_content(recv_buf, len, revbuf, buflen);

RET:
	if (-1 != log_socket) {
		jl_platform_close(log_socket);
	}
	if (recv_buf) {
		/* code */
		jl_platform_free(recv_buf);
	}

	return ret;
#else
	return -1;
#endif

}

/**
 * @brief: SDK main loop 运行状态报告,正常情况下此函数每5秒会被调用一次,可以用来判断SDK主任务的运行状态.
 *
 * @param[in] status: SDK main loop运行状态 0正常, -1异常
 *
 * @return: reserved 当前此函数仅做通知,调用方不关心返回值.
 */
int joylink_dev_run_status(JLRunStatus_t status)
{
	return -1;
}

/**
 * @brief: 每间隔1个main loop周期此函数将在SDK main loop中被调用,让用户有机会将代码逻辑运行在核心任务中.
 *
 * @note:  正常情况下一个main loop周期为1s(取决于socket等待接收数据的timeout时间),但不保证精度,请勿用作定时器
 * @note:  仅用作关键的非阻塞任务执行,例如OTA状态上报或设备状态上报.
 * @note:  执行阻塞或耗时较多的代码,将会妨碍主task运行.
 */
void joylink_dev_run_user_code()
{
	//You may add some code run in the main loop if necessary.
}

/**
 * @brief: Erase wifi and joylink info.
 * @usage: Input command "ATCJ"
 */

// for ATCJ
void joylink_erase()
{
	flash_t flash;
	if (_g_pdev->lan_socket != -1) {
		lwip_close(_g_pdev->lan_socket);
	}
	if (_g_pdev->server_socket != -1) {
		lwip_close(_g_pdev->server_socket);
	}
	LwIP_DHCP(0, DHCP_RELEASE_IP);
	LwIP_DHCP(0, DHCP_STOP);
	vTaskDelay(10);
	fATWD(NULL);
	vTaskDelay(20);
	flash_erase_sector(&flash, FAST_RECONNECT_DATA);
	//	flash_erase_sector(&flash, FLASH_ADD_WIFI_ATTR);
	flash_erase_sector(&flash, FLASH_ADD_JLP_INFO);
	sys_reset();

}

