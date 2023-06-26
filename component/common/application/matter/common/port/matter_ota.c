/************************** 
* Matter OTA Related 
**************************/
#include "platform_opts.h"
#include "platform/platform_stdlib.h"

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdbool.h"
#include "flash_api.h"
#include "sys_api.h"
#include "device_lock.h"
#include "ota_8710c.h"
#include "chip_porting.h"

#define MATTER_OTA_HEADER_SIZE 32
#define MATTER_OTA_SECTOR_SIZE 4096

static flash_t matter_ota_flash;
bool matter_ota_first_sector_written = false;
uint32_t matter_ota_flash_sector_base;
uint32_t matter_ota_new_firmware_addr;

uint8_t matter_ota_header[MATTER_OTA_HEADER_SIZE];
uint8_t matter_ota_header_size = 0; // variable to track size of ota header
uint8_t matter_ota_buffer[MATTER_OTA_SECTOR_SIZE]; // 4KB buffer to be written to one sector
uint16_t matter_ota_buffer_size = 0; // variable to track size of buffer

uint8_t matter_ota_get_header_size()
{
    return matter_ota_header_size;
}

void matter_ota_prepare_partition()
{
    // reset header and data buffer
    memset(matter_ota_buffer, 0, sizeof(matter_ota_buffer));
    memset(matter_ota_header, 0, sizeof(matter_ota_header));
    matter_ota_header_size = 0;
    matter_ota_buffer_size = 0;
    matter_ota_new_firmware_addr = sys_update_ota_prepare_addr();
    matter_ota_flash_sector_base = matter_ota_new_firmware_addr; // Note that the new fw address must be multiples of 4KB
}

int8_t matter_ota_store_header(uint8_t *data, uint32_t size)
{
    // check if overflow
    if (size + matter_ota_header_size > MATTER_OTA_HEADER_SIZE)
        return -1;

    memcpy(&(matter_ota_header[matter_ota_header_size]), data, size);
    matter_ota_header_size += size;

    return 1;
}

int8_t matter_ota_flash_burst_write(uint8_t *data, uint32_t size)
{
    if (size == 0)
    {
        return 1; // don't waste time, just return success
    }

    bool overflow = false;
    uint32_t sectorBase = matter_ota_flash_sector_base;
    uint32_t writeLength = MATTER_OTA_SECTOR_SIZE;
    int16_t bufferRemainSize = (int16_t) (MATTER_OTA_SECTOR_SIZE - matter_ota_buffer_size);

    if (!matter_ota_first_sector_written)
    {
        sectorBase += matter_ota_header_size; // leave first 32-bytes for header
        writeLength -= matter_ota_header_size;
        bufferRemainSize -= matter_ota_header_size;
    }

    if (bufferRemainSize >= size)
    {
        memcpy(matter_ota_buffer + matter_ota_buffer_size, data, size);
        matter_ota_buffer_size += size;
    }
    else
    {
        memcpy(matter_ota_buffer + matter_ota_buffer_size, data, bufferRemainSize);
        matter_ota_buffer_size += bufferRemainSize;
        overflow = true;
        size -= bufferRemainSize;
    }

    if (matter_ota_buffer_size == writeLength)
    {
        // buffer is full, time to erase sector and write buffer data to flash
        device_mutex_lock(RT_DEV_LOCK_FLASH);
        flash_erase_sector(&matter_ota_flash, matter_ota_flash_sector_base);
        flash_burst_write(&matter_ota_flash, sectorBase, writeLength, matter_ota_buffer);
        device_mutex_unlock(RT_DEV_LOCK_FLASH);

        if (!matter_ota_first_sector_written)
        {
            matter_ota_first_sector_written = true;
        }

        matter_ota_flash_sector_base += MATTER_OTA_SECTOR_SIZE; // point to next sector
        memset(matter_ota_buffer, 0, sizeof(matter_ota_buffer)); // clear buffer after writing
        matter_ota_buffer_size = 0;
    }

    if (overflow) // write remaining data into the newly cleared buffer
    {
        // TODO: what if it overflows twice?
        memcpy(matter_ota_buffer + matter_ota_buffer_size, data + bufferRemainSize, size);
        matter_ota_buffer_size += size;
    }

    return 1;
}

int8_t matter_ota_flush_last()
{
    if (matter_ota_buffer_size > 0)
    {
        device_mutex_lock(RT_DEV_LOCK_FLASH);
        flash_erase_sector(&matter_ota_flash, matter_ota_flash_sector_base);
        flash_burst_write(&matter_ota_flash, matter_ota_flash_sector_base, matter_ota_buffer_size, matter_ota_buffer);
        device_mutex_unlock(RT_DEV_LOCK_FLASH);

        matter_ota_flash_sector_base += MATTER_OTA_SECTOR_SIZE; // point to next sector
        memset(matter_ota_buffer, 0, sizeof(matter_ota_buffer)); // clear buffer after writing
        matter_ota_buffer_size = 0;
    }

    return 1;
}

int8_t matter_ota_update_signature()
{
    return (update_ota_signature(matter_ota_header, matter_ota_new_firmware_addr) == 0);
}

void matter_ota_platform_reset()
{
    ota_platform_reset();
}

static void matter_ota_abort_task(void *pvParameters)
{
    uint32_t newFWBlkSize = (AMEBA_OTA_FIRMWARE_LENGTH - 1) / 4096 + 1;
    printf("Cleaning up aborted OTA\r\n");
    printf("Erasing %d sectors\r\n", newFWBlkSize);

    if (matter_ota_new_firmware_addr != 0)
    {
        device_mutex_lock(RT_DEV_LOCK_FLASH);
        for (size_t i=0; i<newFWBlkSize; i++)
        {
            flash_erase_sector(&matter_ota_flash, matter_ota_new_firmware_addr + (i * 4096));
        }
        device_mutex_unlock(RT_DEV_LOCK_FLASH);
    }

    vTaskDelete(NULL);
}

void matter_ota_create_abort_task()
{
    if (xTaskCreate(matter_ota_abort_task, "matter_ota_abort", 2048, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        printf("[%s] Failed to create matter_ota_abort_task\r\n", __FUNCTION__);
    }
}

#ifdef __cplusplus
}
#endif
