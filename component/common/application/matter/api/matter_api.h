#pragma once

bool matter_server_is_commissioned(void);
void matter_get_fabric_indexes(uint16_t *pFabricIndexes, uint8_t bufSize);
void matter_open_basic_commissioning_window(void);
