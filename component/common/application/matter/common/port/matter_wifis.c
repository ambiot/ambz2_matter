/************************** 
* Matter WiFi Related 
**************************/
#include "platform_opts.h"
#include "platform/platform_stdlib.h"

#ifdef __cplusplus
 extern "C" {
#endif

#include "stddef.h"
#include "string.h"
#include "wifi_conf.h"
#include "chip_porting.h"
#include "osdep_service.h"

u32 apNum = 0; // no of total AP scanned
u8 matter_wifi_trigger = 0;
static rtw_scan_result_t matter_userdata[65] = {0};
static char *matter_ssid;
void* matter_param_indicator;
struct task_struct matter_wifi_autoreconnect_task;
struct matter_wifi_autoreconnect_param {
       rtw_security_t security_type;
       char *ssid;
       int ssid_len;
       char *password;
       int password_len;
       int key_id;
};

#if CONFIG_ENABLE_WPS
extern char wps_profile_ssid[33];
extern char wps_profile_password[65];
#endif

chip_connmgr_callback chip_connmgr_callback_func = NULL;
void *chip_connmgr_callback_data = NULL;
void chip_connmgr_set_callback_func(chip_connmgr_callback p, void *data)
{
    chip_connmgr_callback_func = p;
    chip_connmgr_callback_data = data;
}

void print_matter_scan_result( rtw_scan_result_t* record )
{
    RTW_API_INFO("%s\t ", ( record->bss_type == RTW_BSS_TYPE_ADHOC ) ? "Adhoc" : "Infra");
    RTW_API_INFO(MAC_FMT, MAC_ARG(record->BSSID.octet));
    RTW_API_INFO(" %d\t ", record->signal_strength);
    RTW_API_INFO(" %d\t  ", record->channel);
    RTW_API_INFO(" %d\t  ", record->wps_type);
    RTW_API_INFO("%s\t\t ", ( record->security == RTW_SECURITY_OPEN ) ? "Open" :
                                 ( record->security == RTW_SECURITY_WEP_PSK ) ? "WEP" :
                                 ( record->security == RTW_SECURITY_WPA_TKIP_PSK ) ? "WPA TKIP" :
                                 ( record->security == RTW_SECURITY_WPA_AES_PSK ) ? "WPA AES" :
                                 ( record->security == RTW_SECURITY_WPA_MIXED_PSK ) ? "WPA Mixed" :
                                 ( record->security == RTW_SECURITY_WPA2_AES_PSK ) ? "WPA2 AES" :
                                 ( record->security == RTW_SECURITY_WPA2_TKIP_PSK ) ? "WPA2 TKIP" :
                                 ( record->security == RTW_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
                                 ( record->security == RTW_SECURITY_WPA_WPA2_TKIP_PSK) ? "WPA/WPA2 TKIP" :
                                 ( record->security == RTW_SECURITY_WPA_WPA2_AES_PSK) ? "WPA/WPA2 AES" :
                                 ( record->security == RTW_SECURITY_WPA_WPA2_MIXED_PSK) ? "WPA/WPA2 Mixed" :
                                 ( record->security == RTW_SECURITY_WPA_TKIP_ENTERPRISE ) ? "WPA TKIP Enterprise" :
                                 ( record->security == RTW_SECURITY_WPA_AES_ENTERPRISE ) ? "WPA AES Enterprise" :
                                 ( record->security == RTW_SECURITY_WPA_MIXED_ENTERPRISE ) ? "WPA Mixed Enterprise" :
                                 ( record->security == RTW_SECURITY_WPA2_TKIP_ENTERPRISE ) ? "WPA2 TKIP Enterprise" :
                                 ( record->security == RTW_SECURITY_WPA2_AES_ENTERPRISE ) ? "WPA2 AES Enterprise" :
                                 ( record->security == RTW_SECURITY_WPA2_MIXED_ENTERPRISE ) ? "WPA2 Mixed Enterprise" :
                                 ( record->security == RTW_SECURITY_WPA_WPA2_TKIP_ENTERPRISE ) ? "WPA/WPA2 TKIP Enterprise" :
                                 ( record->security == RTW_SECURITY_WPA_WPA2_AES_ENTERPRISE ) ? "WPA/WPA2 AES Enterprise" :
                                 ( record->security == RTW_SECURITY_WPA_WPA2_MIXED_ENTERPRISE ) ? "WPA/WPA2 Mixed Enterprise" :
                                 "Unknown");

    RTW_API_INFO(" %s ", record->SSID.val);
    RTW_API_INFO("\r\n");
}

static rtw_result_t matter_scan_result_handler( rtw_scan_handler_result_t* malloced_scan_result )
{
    if (malloced_scan_result->scan_complete != RTW_TRUE)
    {
        if (malloced_scan_result->ap_details.SSID.len != 0)
        {
            rtw_scan_result_t* record = &malloced_scan_result->ap_details;
            record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */

            RTW_API_INFO("%d\t ", ++apNum);
            print_matter_scan_result(record);

            if(malloced_scan_result->user_data)
                memcpy((void *)((char *)malloced_scan_result->user_data+(apNum-1)*sizeof(rtw_scan_result_t)), (char *)record, sizeof(rtw_scan_result_t));
        }
    }
    else
    {
        if (chip_connmgr_callback_func && chip_connmgr_callback_data)
        {
            // inform matter
            chip_connmgr_callback_func(chip_connmgr_callback_data);
        }
        else
        {
            printf("chip_connmgr_callback_func is NULL\r\n");
            apNum = 0;
            return RTW_ERROR;
        }
    }
    return RTW_SUCCESS;
}

static rtw_result_t matter_scan_with_ssid_result_handler( rtw_scan_handler_result_t* malloced_scan_result )
{
    if (malloced_scan_result->scan_complete != RTW_TRUE)
    {
        rtw_scan_result_t* record = &malloced_scan_result->ap_details;
        record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */

        if((malloced_scan_result->user_data) && (!strcmp(matter_ssid, record->SSID.val)))
        {
            RTW_API_INFO("%d\t ", ++apNum);
            memcpy((void *)((char *)malloced_scan_result->user_data+(apNum-1)*sizeof(rtw_scan_result_t)), (char *)record, sizeof(rtw_scan_result_t));
            print_matter_scan_result(record);
        }
    }
    else
    {
        if (chip_connmgr_callback_func && chip_connmgr_callback_data)
        {
            // inform matter
            chip_connmgr_callback_func(chip_connmgr_callback_data);
            vPortFree(matter_ssid);
        }
        else
        {
            printf("chip_connmgr_callback_func is NULL\r\n");
            apNum = 0;
            vPortFree(matter_ssid);
            return RTW_ERROR;
        }
    }
    return RTW_SUCCESS;
}

void matter_scan_networks(void)
{
    volatile int ret = RTW_SUCCESS;
    apNum = 0; // reset counter at the start of scan
    if((ret = wifi_scan_networks(matter_scan_result_handler, matter_userdata)) != RTW_SUCCESS)
    {
        printf("ERROR: wifi scan failed\n\r");
    }
}

void matter_scan_networks_with_ssid(const unsigned char *ssid, size_t length)
{
    volatile int ret = RTW_SUCCESS;
    apNum = 0; // reset counter at the start of scan
    matter_ssid = (char*) pvPortMalloc(length+1);
    memset(matter_ssid, 0, length+1);
    memcpy(matter_ssid, ssid, length);
    matter_ssid[length] = '\0';
    if((ret = wifi_scan_networks(matter_scan_with_ssid_result_handler, matter_userdata)) != RTW_SUCCESS)
    {
        printf("ERROR: wifi scan failed\n\r");
    }
}

rtw_scan_result_t *matter_get_scan_results()
{
    return matter_userdata;
}

static int matter_find_ap_from_scan_buf(char*buf, int buflen, char *target_ssid, void *user_data)
{
    rtw_wifi_setting_t *pwifi = (rtw_wifi_setting_t *)user_data;
    int plen = 0;

    while(plen < buflen){
        u8 len, ssid_len, security_mode;
        char *ssid;

        // len offset = 0
        len = (int)*(buf + plen);
        // check end
        if(len == 0) break;
        // ssid offset = 14
        ssid_len = len - 14;
        ssid = buf + plen + 14 ;
        if((ssid_len == strlen(target_ssid))
            && (!memcmp(ssid, target_ssid, ssid_len)))
        {
            strncpy((char*)pwifi->ssid, target_ssid, 33);
            // channel offset = 13
            pwifi->channel = *(buf + plen + 13);
            // security_mode offset = 11
            security_mode = (u8)*(buf + plen + 11);
            if(security_mode == IW_ENCODE_ALG_NONE)
                pwifi->security_type = RTW_SECURITY_OPEN;
            else if(security_mode == IW_ENCODE_ALG_WEP)
                pwifi->security_type = RTW_SECURITY_WEP_PSK;
            else if(security_mode == IW_ENCODE_ALG_CCMP)
                pwifi->security_type = RTW_SECURITY_WPA2_AES_PSK;
            break;
        }
        plen += len;
    }
    return 0;
}

static int matter_get_ap_security_mode(IN char * ssid, OUT rtw_security_t *security_mode, OUT u8 * channel)
{
    rtw_wifi_setting_t wifi;
    u32 scan_buflen = 1000;

    memset(&wifi, 0, sizeof(wifi));

    if(wifi_scan_networks_with_ssid(matter_find_ap_from_scan_buf, (void*)&wifi, scan_buflen, ssid, strlen(ssid)) != RTW_SUCCESS){
        printf("Wifi scan failed!\n");
        return 0;
    }

    if(strcmp(wifi.ssid, ssid) == 0){
        *security_mode = wifi.security_type;
        *channel = wifi.channel;
        return 1;
    }

    return 0;
}

static void matter_wifi_autoreconnect_thread(void *param)
{
    int ret = RTW_ERROR;
    struct matter_wifi_autoreconnect_param *reconnect_param = (struct matter_wifi_autoreconnect_param *) param;
    RTW_API_INFO("\n\rmatter auto reconnect ...\n");
    char empty_bssid[6] = {0}, assoc_by_bssid = 0;
    extern unsigned char* rltk_wlan_get_saved_bssid(void);
    unsigned char* saved_bssid = rltk_wlan_get_saved_bssid();

    if(memcmp(saved_bssid, empty_bssid, ETH_ALEN)){
        assoc_by_bssid = 1;
    }

#if defined(CONFIG_SAE_SUPPORT) && (CONFIG_ENABLE_WPS==1)
    unsigned char is_wpa3_disable=0;
    if((strncmp(wps_profile_ssid, reconnect_param->ssid, reconnect_param->ssid_len) == 0) &&
        (strncmp(wps_profile_password, reconnect_param->password, reconnect_param->password_len) == 0) &&
        (wext_get_support_wpa3() == 1)){
        wext_set_support_wpa3(DISABLE);
        is_wpa3_disable=1;
    }
#endif

    if(assoc_by_bssid){
        ret = wifi_connect_bssid(saved_bssid, reconnect_param->ssid, reconnect_param->security_type,
                                    reconnect_param->password, ETH_ALEN, reconnect_param->ssid_len, reconnect_param->password_len, reconnect_param->key_id, NULL);
    }
    else{
        ret = wifi_connect(reconnect_param->ssid, reconnect_param->security_type, reconnect_param->password,
                            reconnect_param->ssid_len, reconnect_param->password_len, reconnect_param->key_id, NULL);
    }

#if defined(CONFIG_SAE_SUPPORT) && (CONFIG_ENABLE_WPS==1)
    if(is_wpa3_disable)
        wext_set_support_wpa3(ENABLE);
#endif

    matter_param_indicator = NULL;
    rtw_delete_task(&matter_wifi_autoreconnect_task);
}

void matter_wifi_autoreconnect_hdl(rtw_security_t security_type,
                            char *ssid, int ssid_len,
                            char *password, int password_len,
                            int key_id)
{
    static struct matter_wifi_autoreconnect_param param;
    matter_param_indicator = &param;
    param.security_type = security_type;
    param.ssid = ssid;
    param.ssid_len = ssid_len;
    param.password = password;
    param.password_len = password_len;
    param.key_id = key_id;

    if(matter_wifi_autoreconnect_task.task != NULL){
        dhcp_stop(&xnetif[0]);
        u32 start_tick = rtw_get_current_time();
        while(1){
            rtw_msleep_os(2);
            u32 passing_tick = rtw_get_current_time() - start_tick;
            if(rtw_systime_to_sec(passing_tick) >= 2){
                RTW_API_INFO("\r\n Create matter_wifi_autoreconnect_task timeout \r\n");
                return;
            }

            if(matter_wifi_autoreconnect_task.task == NULL){
                break;
            }
        }
    }

    rtw_create_task(&matter_wifi_autoreconnect_task, (const char *)"matter_wifi_autoreconnect", 512, tskIDLE_PRIORITY + 1, matter_wifi_autoreconnect_thread, &param);

}

int matter_wifi_connect(
    char              *ssid,
    rtw_security_t    security_type,
    char              *password,
    int               ssid_len,
    int               password_len,
    int               key_id,
    void              *semaphore)
{
    u8 connect_channel;
    int security_retry_count = 0;
    int ret = 0;
    int err = 0;

    if(strlen((const char *) password) != 0) {
        security_type = RTW_SECURITY_WPA_WPA2_MIXED_PSK;
    }
    else {
        security_type = RTW_SECURITY_OPEN;
    }

    if(security_type == RTW_SECURITY_WPA_WPA2_MIXED) {
        while (1) {
            if (matter_get_ap_security_mode((char*)ssid, &security_type, &connect_channel)) {
                break;
            }
            security_retry_count++;
            if(security_retry_count >= 3) {
                printf("Can't get AP security mode and channel. Use RTW_SECURITY_WPA_WPA2_MIXED\n");
                security_type = RTW_SECURITY_WPA_WPA2_MIXED;
                break;
            }
        }
        /* Don't set WEP Key ID, default use key_id = 0 for connection
         * If connection fails, use onnetwork (connect to AP with AT Command) instead of ble-wifi
         * This behavior matches other devices behavior as phone and laptop is unable to connect with key_id > 0
         * */
    }

    matter_wifi_trigger = 1;
    matter_set_autoreconnect(1);
    err = wifi_connect(ssid, security_type, password, strlen(ssid), strlen(password), key_id, NULL);

    return err;
}

int matter_get_sta_wifi_info(rtw_wifi_setting_t *pSetting)
{
    return wifi_get_setting((u8*)WLAN0_NAME, pSetting);
}

int matter_wifi_disconnect(void)
{
    return wifi_disconnect();
}

int matter_wifi_on(rtw_mode_t mode)
{
    return wifi_on(mode);
}

int matter_wifi_set_mode(rtw_mode_t mode)
{
    return wifi_set_mode(mode);
}

int matter_wifi_is_connected_to_ap(void)
{
    return wifi_is_connected_to_ap();
}

int matter_wifi_is_ready_to_transceive(rtw_interface_t interface)
{
    return wifi_is_ready_to_transceive(interface);
}

int matter_wifi_is_up(rtw_interface_t interface)
{
    return wifi_is_up(interface);
}

int matter_wifi_is_open_security (void)
{
    if(wifi_get_sta_security_type() == RTW_SECURITY_OPEN)
    {
        return 1;
    }
    return 0;
}

void matter_lwip_dhcp()
{
    netif_set_link_up(&xnetif[0]);
    matter_set_autoreconnect(0);

    LwIP_DHCP(0, DHCP_START);
}

void matter_lwip_dhcp6(void)
{
#if defined(LWIP_IPV6) && LWIP_IPV6
    LwIP_DHCP6(0, DHCP6_START);
#endif
}

void matter_lwip_releaseip(void)
{
    LwIP_ReleaseIP(0);
}

int matter_wifi_get_ap_bssid(unsigned char *bssid)
{
    return wifi_get_ap_bssid(bssid);
}

int matter_wifi_get_network_mode(rtw_network_mode_t *pmode)
{
    return wifi_get_network_mode(pmode);
}

int matter_wifi_get_security_type(const char *ifname, uint16_t *alg, uint8_t *key_idx, uint8_t *passphrase)
{
    if (wext_get_enc_ext(ifname, alg, key_idx, passphrase) < 0)
    {
        return RTW_ERROR;
    }
    return RTW_SUCCESS;
}

int matter_wifi_get_wifi_channel_number(const char *ifname, uint8_t *ch)
{
    if (wext_get_channel(ifname, ch) < 0)
    {
        return RTW_ERROR;
    }
    return RTW_SUCCESS;
}

int matter_wifi_get_rssi(int *prssi)
{
    return wifi_get_rssi(prssi);
}

int matter_wifi_get_mac_address(char *mac)
{
    return wifi_get_mac_address(mac);
}

int matter_wifi_get_last_error()
{
    return wifi_get_last_error();
}

void matter_set_autoreconnect(u8 mode)
{
    wifi_set_autoreconnect(mode);
}

uint8_t *matter_LwIP_GetIPv6_linklocal(uint8_t idx)
{
    return LwIP_GetIPv6_linklocal(&xnetif[idx]);
}

uint8_t *matter_LwIP_GetIPv6_global(uint8_t idx)
{
    return LwIP_GetIPv6_global(&xnetif[idx]);
}

unsigned char *matter_LwIP_GetIP(uint8_t idx)
{
    return LwIP_GetIP(&xnetif[idx]);
}

unsigned char *matter_LwIP_GetGW(uint8_t idx)
{
    return LwIP_GetGW(&xnetif[idx]);
}

uint8_t *matter_LwIP_GetMASK(uint8_t idx)
{
    return LwIP_GetMASK(&xnetif[idx]);
}

int matter_wifi_get_setting(unsigned char wlan_idx, rtw_wifi_setting_t *psetting)
{
    if (wlan_idx == WLAN0_IDX)
    {
    return wifi_get_setting(WLAN0_NAME, psetting);
    }
    else
    {
    return wifi_get_setting(WLAN1_NAME, psetting);
    }
}

#ifdef __cplusplus
}
#endif
