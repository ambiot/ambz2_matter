#ifndef _MESH_DATA_DUMP_H_
#define _MESH_DATA_DUMP_H_

#include <stdint.h>
#include <stdio.h>

#define data_uart_debug  printf

#define data_uart_dump  mesh_data_dump

void mesh_data_dump(uint8_t *pbuffer, uint32_t len);

#endif //_MESH_DATA_DUMP_H_

