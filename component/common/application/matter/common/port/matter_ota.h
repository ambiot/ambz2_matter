#ifdef __cplusplus
extern "C" {
#endif

uint8_t matter_ota_get_total_header_size();
uint8_t matter_ota_get_current_header_size();
void matter_ota_prepare_partition();
int8_t matter_ota_store_header(uint8_t *data, uint32_t size);
int8_t matter_ota_flash_burst_write(uint8_t *data, uint32_t size);
int8_t matter_ota_flush_last();
int8_t matter_ota_update_signature();
void matter_ota_platform_reset();
void matter_ota_create_abort_task();

#ifdef __cplusplus
}
#endif
