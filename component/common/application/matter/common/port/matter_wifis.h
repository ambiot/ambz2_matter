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

void wifi_btcoex_set_bt_on(void);
extern int CHIP_SetWiFiConfig(rtw_wifi_setting_t *config);
extern int CHIP_GetWiFiConfig(rtw_wifi_setting_t *config);
extern rtw_mode_t wifi_mode;

extern u32 apNum;
typedef int (*chip_connmgr_callback)(void *object);
void chip_connmgr_set_callback_func(chip_connmgr_callback p, void *data);
void matter_scan_networks(void);
void matter_scan_networks_with_ssid(const unsigned char *ssid, size_t length);
void matter_get_scan_results(rtw_scan_result_t *result_buf, uint8_t scanned_num);

#ifdef __cplusplus
}
#endif
