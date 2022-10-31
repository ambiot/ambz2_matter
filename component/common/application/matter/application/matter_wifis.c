/************************** 
* Matter WiFi Related 
**************************/
#include "platform_opts.h"
#include "platform/platform_stdlib.h"

#ifdef CHIP_PROJECT

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

void matter_get_scan_results(rtw_scan_result_t *result_buf, uint8_t scanned_num)
{
    memcpy(result_buf, matter_userdata, sizeof(rtw_scan_result_t) * scanned_num);
}

#ifdef __cplusplus
}
#endif
#endif /* CHIP_PROJECT */
