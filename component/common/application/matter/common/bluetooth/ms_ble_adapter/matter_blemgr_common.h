#ifndef _MATTER_BLEMGR_COMMON_H_
#define _MATTER_BLEMGR_COMMON_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    MATTER_BLEMGR_GAP_CONNECT_CB,
    MATTER_BLEMGR_GAP_DISCONNECT_CB,
    MATTER_BLEMGR_RX_CHAR_WRITE_CB,
    MATTER_BLEMGR_TX_CHAR_CCCD_WRITE_CB,
    MATTER_BLEMGR_TX_COMPLETE_CB,
    MATTER_BLEMGR_C3_CHAR_READ_CB
} T_MATTER_BLEMGR_CALLBACK_TYPE;

typedef struct
{
    uint8_t conn_id;
} T_MATTER_BLEMGR_GAP_CONNECT_CB_ARG;

typedef struct
{
    uint8_t conn_id;
    uint16_t disc_cause;
} T_MATTER_BLEMGR_GAP_DISCONNECT_CB_ARG;

typedef struct
{
    uint8_t conn_id;
    uint8_t *p_value;
    uint16_t len;
} T_MATTER_BLEMGR_RX_CHAR_WRITE_CB_ARG;

typedef struct
{
    uint8_t conn_id;
    uint8_t indicationsEnabled;
    uint8_t notificationsEnabled;
} T_MATTER_BLEMGR_TX_CHAR_CCCD_WRITE_CB_ARG;

typedef struct
{
    uint8_t conn_id;
} T_MATTER_BLEMGR_TX_COMPLETE_CB_ARG;

typedef struct
{
    uint8_t **pp_value;
    uint16_t *p_len;
} T_MATTER_BLEMGR_C3_CHAR_READ_CB_ARG;

typedef int (*matter_blemgr_callback)(void *param, T_MATTER_BLEMGR_CALLBACK_TYPE cb_type, void *p_cb_data);

int matter_blemgr_init(void);
void matter_blemgr_set_callback_func(matter_blemgr_callback p, void *data);
int matter_blemgr_start_adv(void);
int matter_blemgr_stop_adv(void);
int matter_blemgr_config_adv(uint16_t adv_int_min, uint16_t adv_int_max, uint8_t *adv_data, uint8_t adv_data_length);
uint16_t matter_blemgr_get_mtu(uint8_t connect_id);
int matter_blemgr_set_device_name(char *device_name, uint8_t device_name_length);
int matter_blemgr_disconnect(uint8_t connect_id);
int matter_blemgr_send_indication(uint8_t connect_id, uint8_t *data, uint16_t data_length);

#ifdef __cplusplus
}
#endif

#endif
