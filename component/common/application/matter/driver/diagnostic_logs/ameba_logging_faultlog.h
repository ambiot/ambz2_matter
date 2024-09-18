#ifndef MATTER_AMEBA_FAULTLOG_H
#define MATTER_AMEBA_FAULTLOG_H

#ifdef __cplusplus
extern "C" {
#endif

extern void fault_handler_override(void(*fault_log)(char *msg, int len), void(*bt_log)(char *msg, int len));

/**
 * @brief  The callback functions to handle fault event logs, which includes register and stack memory dump logs
 * @param  msg:  fault event logs which includes register and stack memory dump logs
 * @param  len:  indicates the length of msg
 */
void matter_fault_log(char *msg, int len);

/**
 * @brief  The callback functions to handle stack backtrace logs
 * @param  msg:  backtrace logs which includes msp and psp information
 * @param  len:  indicates the length of msg
 */
void matter_bt_log(char *msg, int len);

/**
 * @brief  Reads the last crash logs from LFS
 */
void matter_read_last_fault_log(void);

#ifdef __cplusplus
}
#endif

#endif // MATTER_AMEBA_FAULTLOG_H
