/*
 * Copyright (c) 2019 - 2020 IoT Company of Midea Group.
 *
 * File Name 	    : 
 * Description	    : hal ble
 *
 * Version	        : v0.0.1
 * Author			: Moore
 * Date	            : 2020/03/14  refactoring
 * History	        : 
 */

#ifndef _MS_HAL_BLE_H_
#define _MS_HAL_BLE_H_

/**
 * !!! please see the config.mk to make sure your chip whether had those function !!!
 * MS_CONFIG_BLE_SUPPORT
 * !!! if not, do not adapter following interfaces
 */

#include "ms_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MS_CONFIG_BLE_SUPPORT

#define MS_HAL_BLE_HI_WORD(x)  ((uint8_t)((x & 0xFF00) >> 8))
#define MS_HAL_BLE_LO_WORD(x)  ((uint8_t)(x))

typedef enum {
    MS_HAL_BLE_STACK_EVENT_STACK_READY = 0,
    MS_HAL_BLE_STACK_EVENT_STACK_FAIL,
    MS_HAL_BLE_STACK_EVENT_ADV_ON,
    MS_HAL_BLE_STACK_EVENT_ADV_OFF,
    ///@deprecated Use MS_HAL_BLE_STACK_EVENT_CONNECTION_REPORT
    MS_HAL_BLE_STACK_EVENT_PERIPHERAL_CONNECTED, //!< Bluetooth connected as peripheral
    ///@deprecated Use MS_HAL_BLE_STACK_EVENT_CONNECTION_REPORT
    MS_HAL_BLE_STACK_EVENT_CENTRAL_CONNECTED,    //!< Bluetooth connected as central
    MS_HAL_BLE_STACK_EVENT_DISCONNECTED,         //!< Bluetooth disconnect
    MS_HAL_BLE_STACK_EVENT_SCAN_ON,              //!< ble scan start
    MS_HAL_BLE_STACK_EVENT_SCAN_OFF,             //!< ble scan stops
    MS_HAL_BLE_STACK_EVENT_NONCONN_ADV_ON,       //!< ble nonconn advertising start
    MS_HAL_BLE_STACK_EVENT_NONCONN_ADV_OFF,      //!< ble nonconn advertising stops
    //Part 2
    MS_HAL_BLE_STACK_EVENT_NOTIFY_REQ,           //!< ble notify data callback  //GATT_EVENT,Master get Ntf data form Slave
    MS_HAL_BLE_STACK_EVENT_INDICATE_REQ,         //!< ble indicate data callback //GATT_REQ_EVENT,Master get Ind data form Slave
    MS_HAL_BLE_STACK_EVENT_DISC_SVC_REPORT,      //!< discovery all service report
    MS_HAL_BLE_STACK_EVENT_DISC_CHAR_REPORT,     //!< discovery all char report
    MS_HAL_BLE_STACK_EVENT_DISC_DESC_REPORT,     //!< discovery all descriptor report
    MS_HAL_BLE_STACK_EVENT_MTU_CHANGED,          //!< MTU changed callback
    MS_HAL_BLE_STACK_EVENT_CONNECTION_REPORT,    //!< Connection information
    MS_HAL_BLE_STACK_EVENT_CONNECT_PARAM_UPDATE_REQ,    //!< Connection information


    MS_HAL_BLE_STACK_EVENT_CMP = 0X80,           //!< Complete event start ID, Do not use.
    MS_HAL_BLE_STACK_EVENT_CMP_MTU,              //!< MTU change complete
    MS_HAL_BLE_STACK_EVENT_CMP_SVC_DISC,         //!< discovery service done
    MS_HAL_BLE_STACK_EVENT_CMP_CHAR_DISC,        //!< discovery characteristic done
    MS_HAL_BLE_STACK_EVENT_CMP_DISC_DESC_CHAR,   //!< discover characteristic descriptor done
    MS_HAL_BLE_STACK_EVENT_CMP_WRITE_REQ,        //!< write request complete
    MS_HAL_BLE_STACK_EVENT_CMP_WRITE_CMD,        //!< write cmd complete
    MS_HAL_BLE_STACK_EVENT_CMP_NOTIFY,           //!< Notify complete
    MS_HAL_BLE_STACK_EVENT_CMP_INDICATE,         //!< Indicate complete
}matter_hal_ble_stack_event_t;

typedef struct{
    uint16_t conn_hdl;
    uint16_t att_hdl;
    uint16_t data_len;
    uint8_t *data;
}matter_hal_ble_gatt_msg_t;

typedef struct{
    uint16_t conn_hdl;
    uint16_t start_hdl;
    uint16_t end_hdl;
    uint8_t uuid_len;
    uint8_t *uuid;
}matter_hal_ble_disc_svc_msg_t;

typedef struct{
    uint16_t conn_hdl;
    uint16_t att_hdl;
    uint16_t pointer_hdl;
    uint8_t prop;
    uint8_t uuid_len;
    uint8_t *uuid;
}matter_hal_ble_disc_char_msg_t;

typedef struct{
    uint16_t conn_hdl;
    uint16_t att_hdl;
    uint8_t uuid_len;
    uint8_t *uuid;
}matter_hal_ble_disc_desc_msg_t;

typedef struct{
    uint16_t conn_hdl;
    uint16_t mtu;
}matter_hal_ble_mtu_changed_msg_t;

typedef struct{
    uint16_t conn_hdl;
    uint8_t *addr;
    uint16_t con_interval;
    uint16_t con_latency;
    uint16_t sup_timeout;
    uint8_t clk_accuracy;
    uint8_t peer_addr_type;
    uint8_t role;
}matter_hal_ble_connetion_msg_t;

typedef struct{
    uint16_t conn_hdl;
    uint8_t reason;
}matter_hal_ble_disconnect_msg_t;

typedef struct{
    uint16_t conn_hdl;
    uint16_t conn_intv_min;
    uint16_t conn_intv_max;
    uint16_t slave_latency;
    uint16_t timeout;
}matter_hal_ble_conn_param_msg_t;

typedef struct{
    matter_hal_ble_stack_event_t event_type;
    union{
        uint16_t conn_hdl;
        matter_hal_ble_gatt_msg_t gatt_msg;
        matter_hal_ble_disc_svc_msg_t disc_svc_msg;
        matter_hal_ble_disc_char_msg_t disc_char_msg;
        matter_hal_ble_disc_desc_msg_t disc_desc_msg;
        matter_hal_ble_mtu_changed_msg_t mtu_changed_msg;
        matter_hal_ble_connetion_msg_t connection_msg;
        matter_hal_ble_disconnect_msg_t disconnect_msg;
        matter_hal_ble_conn_param_msg_t conn_param_msg;
    }param;
}matter_hal_ble_stack_msg_t;

typedef int (*matter_hal_ble_stack_callback_t)(matter_hal_ble_stack_msg_t);



// UUID Service
// service +characteristic +charc_user_description +client_charc_config
typedef void (*matter_hal_ble_service_null_cb)(void);
typedef void (*matter_hal_ble_service_read_cb)(uint8_t *data, uint16_t size);
typedef void (*matter_hal_ble_service_write_cb)(uint8_t *data, uint16_t size);
typedef void (*matter_hal_ble_service_indicate_cb)(void);

#define MS_HAL_BLE_ATTRIB_CHAR_PROP_NONE                0x00
#define MS_HAL_BLE_ATTRIB_CHAR_PROP_BROADCAST           0x01
#define MS_HAL_BLE_ATTRIB_CHAR_PROP_READ                0x02
#define MS_HAL_BLE_ATTRIB_CHAR_PROP_WRITE_NO_RSP        0x04
#define MS_HAL_BLE_ATTRIB_CHAR_PROP_WRITE               0x08
#define MS_HAL_BLE_ATTRIB_CHAR_PROP_NOTIFY              0x10
#define MS_HAL_BLE_ATTRIB_CHAR_PROP_INDICATE            0x20
#define MS_HAL_BLE_ATTRIB_CHAR_PROP_WRITE_AUTHEN_SIGNED 0x40
#define MS_HAL_BLE_ATTRIB_CHAR_PROP_EXT_PROP            0x80

#define MS_HAL_BLE_ATTRIB_PERM_NONE              0x00
#define MS_HAL_BLE_ATTRIB_PERM_READ              0x01
#define MS_HAL_BLE_ATTRIB_PERM_WRITE             0x02
#define MS_HAL_BLE_ATTRIB_PERM_NOTIF_IND         0x04

typedef enum _matter_hal_ble_uuid_type_
{
    ENUM_MS_HAL_BLE_UUID_TYPE_NULL = 0,
    ENUM_MS_HAL_BLE_UUID_TYPE_16_BIT,
    ENUM_MS_HAL_BLE_UUID_TYPE_128_bit,
} matter_hal_ble_uuid_type_t;

typedef enum _matter_hal_ble_attrib_type_
{
    ENUM_MS_HAL_BLE_ATTRIB_TYPE_SERVICE = 1,
    ENUM_MS_HAL_BLE_ATTRIB_TYPE_CHAR,
    ENUM_MS_HAL_BLE_ATTRIB_TYPE_CHAR_VALUE,
    ENUM_MS_HAL_BLE_ATTRIB_TYPE_CHAR_CLIENT_CONFIG,
    ENUM_MS_HAL_BLE_ATTRIB_TYPE_CHAR_USER_DESCR,
} matter_hal_ble_attrib_type_t;

typedef union _matter_hal_ble_attrib_callback_
{
    matter_hal_ble_service_null_cb null_cb;
    matter_hal_ble_service_read_cb read_cb;
    matter_hal_ble_service_write_cb write_cb;
    matter_hal_ble_service_indicate_cb indicate_cb;
}matter_hal_ble_attrib_callback_t;

typedef struct _matter_hal_ble_service_attrib_
{
    uint8_t att_type;
    uint8_t uuid_type;
    uint8_t uuid[2 + 14];
    uint8_t prop;
    uint16_t value_len;
    uint8_t *value_context;
    uint8_t perm;
    matter_hal_ble_attrib_callback_t callback;
} matter_hal_ble_service_attrib_t;



typedef struct
{
    uint32_t                    adv_intv_min;       /// Minimum advertising interval (in unit of 625us). Must be greater than 20ms
    uint32_t                    adv_intv_max;       /// Maximum advertising interval (in unit of 625us). Must be greater than 20ms
} matter_hal_ble_gap_adv_params;




/*!
    @brief BLE ddress Type
*/

typedef enum
{
    MS_HAL_BLE_ADDR_TYPE_PUBLIC = 0,               ///< public address
    MS_HAL_BLE_ADDR_TYPE_RANDOM_STATIC = 1,        ///< random static address
    MS_HAL_BLE_ADDR_TYPE_RANDOM_RESOLVABLE = 2,    ///< random resolvable addresss
    MS_HAL_BLE_ADDR_TYPE_RANDOM_NON_RESOLVABLE = 3, ///< random non resolvable address
}matter_hal_ble_addr_type;

/*!
    @brief BLE address structure
*/
typedef struct
{
    matter_hal_ble_addr_type addr_type;             ///< address type.
    uint8_t                addr[6];                        ///< address byte array.
} matter_hal_ble_addr_t;


/*!
    @brief BLE Report Type
    The report type is for the scanned peers' type. It could be AD type of passive scan or active scan(SCAN_RSP).
    It was used in structure ble_gap_evt_adv_report_t.
*/
typedef enum
{
    MS_HAL_BLE_REPORT_TYPE_IND = 0x00,                 ///< Type for ADV_IND found (passive)
    MS_HAL_BLE_REPORT_TYPE_DIRECT_IND = 0x01,          ///< Type for ADV_DIRECT_IND found (passive)
    MS_HAL_BLE_REPORT_TYPE_SCAN_IND    = 0x02,         ///< Type for ADV_SCAN_IND found (passive)
    MS_HAL_BLE_REPORT_TYPE_NONCONN_IND  = 0x03,        ///< Type for ADV_NONCONN_IND found (passive)
    MS_HAL_BLE_REPORT_TYPE_SCAN_RSP = 0x04             ///< Type for SCAN_RSP found (active)
}matter_hal_ble_report_type;


typedef struct
{
    matter_hal_ble_report_type     type;                           // report ad type
    matter_hal_ble_addr_t     peer_addr;                           // peer addr
    int8_t                   tx_pwr;                           /// TX power (in dBm)
    int8_t                     rssi;                           // rssi
    uint16_t                    len;                           //data len   
    uint8_t                   *data;                           //data       
}matter_hal_ble_scan_t;
 

//扫描回调
typedef void (*matter_hal_ble_scan_callback_t)(matter_hal_ble_scan_t *data);


/*!
    @brief BLE while listing.
*/
typedef struct
{
    uint8_t                 addr_count;    ///< device count
    matter_hal_ble_addr_t *        p_addrs;       ///< device address's array. Its size is equal to addr_count.
} matter_hal_ble_whitelist_t;


typedef enum
{
    MS_HAL_BLE_SCAN_FP_ACCEPT_ALL                          = 0x00,  /**< Accept all advertising packets except directed advertising packets
                                                                    not addressed to this device. */
    MS_HAL_BLE_SCAN_FP_WHITELIST                           = 0x01,  /**< Accept advertising packets from devices in the whitelist except directed
                                                                    packets not addressed to this device. */
    MS_HAL_BLE_SCAN_FP_ALL_NOT_RESOLVED_DIRECTED           = 0x02,  /**< Accept all advertising packets specified in @ref BLE_GAP_SCAN_FP_ACCEPT_ALL.
                                                                    In addition, accept directed advertising packets, where the advertiser's
                                                                    address is a resolvable private address that cannot be resolved. */
    MS_HAL_BLE_SCAN_FP_WHITELIST_NOT_RESOLVED_DIRECTED     = 0x03,  /**< Accept all advertising packets specified in @ref BLE_GAP_SCAN_FP_WHITELIST.
                                                                    In addition, accept directed advertising packets, where the advertiser's
                                                                    address is a resolvable private address that cannot be resolved. */
}matter_hal_ble_scan_filter_policy_t;


typedef struct
{
    bool                                       active;			    //scan active or passive
    bool                            filter_duplicated;              ///< Filter duplicated device
    matter_hal_ble_scan_filter_policy_t     filter_policy;
    matter_hal_ble_whitelist_t *              p_whitelist;              ///< Pointer to whitelist, NULL if no whitelist or the current active whitelist is to be used.
    uint16_t                               scan_intvl;              ///< Scan interval between 0x0004 and 0x4000 in 0.625ms units (2.5ms to 10.24s).
    uint16_t                              scan_window;              ///< Scan window between 0x0004 and 0x4000 in 0.625ms units (2.5ms to 10.24s).
} matter_hal_ble_scan_params_t;


#endif /* MS_CONFIG_BLE_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif //__MS_HAL_BLE_H__
