/**
 * @brief High resolution sleep.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/nanosleep.html
 *
 * @note rmtp is ignored, as signals are not implemented.
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <wifi_conf.h>
#include <lwip_netconf.h>

#define JOIN_HANDSHAKE_DONE (uint32_t)(1 << 7)
extern uint32_t rtw_join_status;

void wifi_btcoex_set_bt_on(void);
extern int CHIP_SetWiFiConfig(rtw_wifi_setting_t *config);
extern int CHIP_GetWiFiConfig(rtw_wifi_setting_t *config);
extern rtw_mode_t wifi_mode;

extern u32 apNum;
typedef int (*chip_connmgr_callback)(void *object);
void chip_connmgr_set_callback_func(chip_connmgr_callback p, void *data);
void matter_scan_networks(void);
void matter_scan_networks_with_ssid(const unsigned char *ssid, size_t length);
rtw_scan_result_t *matter_get_scan_results(void);
void matter_wifi_autoreconnect_hdl(
    rtw_security_t security_type,
    char *ssid, int ssid_len,
    char *password, int password_len,
    int key_id);
void matter_set_autoreconnect(u8 mode);
int matter_wifi_connect(
    char              *ssid,
    rtw_security_t    security_type,
    char              *password,
    int               ssid_len,
    int               password_len,
    int               key_id,
    void              *semaphore);
int matter_get_sta_wifi_info(rtw_wifi_setting_t *pSetting);
int matter_wifi_disconnect(void);
int matter_wifi_on(rtw_mode_t mode);
int matter_wifi_set_mode(rtw_mode_t mode);
int matter_wifi_is_connected_to_ap(void);
int matter_wifi_is_open_security (void);
int matter_wifi_is_ready_to_transceive(rtw_interface_t interface);
int matter_wifi_is_up(rtw_interface_t interface);
void matter_lwip_dhcp(void);
void matter_lwip_dhcp6(void);
void matter_lwip_releaseip(void);
int matter_wifi_get_ap_bssid(unsigned char*);
int matter_wifi_get_network_mode(rtw_network_mode_t *pmode);
int matter_wifi_get_security_type(const char *ifname, uint16_t *alg, uint8_t *key_idx, uint8_t *passphrase);
int matter_wifi_get_wifi_channel_number(const char *ifname, uint8_t *ch);
int matter_wifi_get_rssi(int *prssi);
int matter_wifi_get_mac_address(char *mac);
int matter_wifi_get_last_error(void);
#if LWIP_VERSION_MAJOR > 2 || LWIP_VERSION_MINOR > 0
#if LWIP_IPV6
uint8_t *matter_LwIP_GetIPv6_linklocal(uint8_t idx);
uint8_t *matter_LwIP_GetIPv6_global(uint8_t idx);
#endif // LWIP_IPV6
#endif // LWIP_VERSION_MAJOR > 2 || LWIP_VERSION_MINOR > 0
unsigned char *matter_LwIP_GetIP(uint8_t idx);
unsigned char *matter_LwIP_GetGW(uint8_t idx);
uint8_t *matter_LwIP_GetMASK(uint8_t idx);
int matter_wifi_get_setting(unsigned char wlan_idx, rtw_wifi_setting_t *psetting);

#ifdef __cplusplus
}
#endif
