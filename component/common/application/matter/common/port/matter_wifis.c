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

u32 apNum = 0; // no of total AP scanned
static rtw_scan_result_t matter_userdata[65] = {0};
static char *matter_ssid;

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
                printf("Can't get AP security mode and channel.\n");
                ret = RTW_NOTFOUND;
                return RTW_ERROR;
            }
        }
        /* Don't set WEP Key ID, default use key_id = 0 for connection
         * If connection fails, use onnetwork (connect to AP with AT Command) instead of ble-wifi
         * This behavior matches other devices behavior as phone and laptop is unable to connect with key_id > 0
         * */
    }

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

void matter_lwip_dhcp(uint8_t idx, uint8_t dhcp_state)
{
    if (dhcp_state == DHCP_START)
    {
        LwIP_DHCP(0, DHCP_START);
    }
    else if (dhcp_state == DHCP6_START)
    {
        LwIP_DHCP6(0, DHCP6_START);
    }
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

#ifdef __cplusplus
}
#endif
