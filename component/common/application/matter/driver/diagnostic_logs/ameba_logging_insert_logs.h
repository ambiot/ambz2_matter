#ifndef MATTER_AMEBA_FAULTLOG_H
#define MATTER_AMEBA_FAULTLOG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inserts user log data into the logging system.
 * 
 * @param data Pointer to the data to be logged.
 * @param data_len Length of the data in bytes.
 */
_WEAK void matter_insert_user_log(uint8_t* data, uint32_t data_len);

/**
 * @brief Inserts network log data into the logging system.
 * 
 * @param data Pointer to the network log data to be logged.
 * @param data_len Length of the data in bytes.
 */
_WEAK void matter_insert_network_log(uint8_t* data, uint32_t data_len);

#ifdef __cplusplus
}
#endif

#endif // MATTER_AMEBA_FAULTLOG_H
