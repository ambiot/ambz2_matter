#include "stdio.h"
#include "mesh_data_dump.h"

void mesh_data_dump(uint8_t *pbuffer, uint32_t len)
{
	printf("0x");
	for (uint32_t loop = 0; loop < len; loop++) {
		char data = "0123456789ABCDEF"[pbuffer[loop] >> 4];
		printf("%c", data);
		data = "0123456789ABCDEF"[pbuffer[loop] & 0x0f];
		printf("%c", data);
	}
	printf("\r\n");
}
