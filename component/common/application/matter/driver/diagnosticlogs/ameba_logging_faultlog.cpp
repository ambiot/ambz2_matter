#include "FreeRTOS.h"
#include "lfs.h"
#include "platform/platform_stdlib.h"
#include "platform_opts.h"
#include <flash_api.h>
#include <device_lock.h>

#include <string>

#include "ameba_diagnosticlogs_provider_delegate_impl.h"
//#include  "matter_flashfs.h"
#include "matter_fs.h"
#include "platform_opts_matter.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 1)
void read_last_fault_log(void)
{
	flash_t	fault_flash;
    FIL fp;
    int res;
    uint unused_writecount = 0;
    uint readsize = 0;

    int err = matter_flashfs_file_open("crash.log", &fp, FA_CREATE_ALWAYS | FA_WRITE);          // always make a new crash.log file
    if(err != 0) {
        ChipLogError(DeviceLayer, "open err: %d", err);
        return;
    }

	uint8_t *log = (uint8_t *)malloc(FAULT_FLASH_SECTOR_SIZE);
	if (!log)	{
		return;
	}
	memset(log, 0xFF, FAULT_FLASH_SECTOR_SIZE);
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	flash_stream_read(&fault_flash, FAULT_LOG1, FAULT_FLASH_SECTOR_SIZE, log);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);

	ChipLogProgress(DeviceLayer, ">>>>>> Copy fault log <<<<<<<<");
	if (log[0] != 0xFF) {
        while(1)    // scan for first 0xFF. replacement for strlen which only terminates at \0
        {
            if(log[readsize] == 0xFF) break;
            readsize++;
            if(readsize == FAULT_FLASH_SECTOR_SIZE) break;
        }

        res = matter_flashfs_file_write(&fp, log, readsize, 0, 0, false, &unused_writecount); // write to FS
        if(res == 0) 
        {
            ChipLogProgress(DeviceLayer, "wrote %d bytes to crashlog", unused_writecount);
        }
        else
        {
            ChipLogError(DeviceLayer, "write error!");
            matter_flashfs_file_close(&fp);
            return;
        }
	}

	device_mutex_lock(RT_DEV_LOCK_FLASH);
	flash_stream_read(&fault_flash, FAULT_LOG2, FAULT_FLASH_SECTOR_SIZE, log);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);

    readsize = 0;
    ChipLogProgress(DeviceLayer, ">>>>>>> Copy Backtrace log <<<<<<<<");
	if (log[0] != 0xFF) {
        while(1)
        {
            if(log[readsize] == 0xFF) break;
            readsize++;
            if(readsize == FAULT_FLASH_SECTOR_SIZE) break;
        }

        res = matter_flashfs_file_write(&fp, log, readsize, 0, 0, true, &unused_writecount);
        if(res == 0) 
        {
            ChipLogProgress(DeviceLayer, "wrote %d bytes to crashlog", unused_writecount);
        }
        else
        {
            ChipLogError(DeviceLayer, "write error!");
            matter_flashfs_file_close(&fp);
            return;
        }
	}
	free(log);
    matter_flashfs_file_close(&fp);

    ChipLogProgress(DeviceLayer, "crashlog rotated for next crash...");
}
#elif defined (CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 0)

// debugonly
#if 1
void write_debug_userlog(char* msg)
{
    lfs_file_t* fp = chip::app::Clusters::DiagnosticLogs::AmebaDiagnosticLogsProvider::GetFpUserLog();

    int err = matter_fs_fopen(USER_LOG_FILENAME, fp, LFS_O_RDWR | LFS_O_CREAT);
    if(err != 0) {
        ChipLogError(DeviceLayer, "open err: %d", err);
        return;
    }

    uint writeNum = 0;
    err = matter_fs_fwrite(fp, msg, strlen(msg), 0, 0, 0, &writeNum);
    matter_fs_fclose(fp);
}
#endif

void read_last_fault_log(void)
{
	flash_t	fault_flash;
    lfs_file_t* fp = chip::app::Clusters::DiagnosticLogs::AmebaDiagnosticLogsProvider::GetFpCrashLog();
    int res;
    uint unused_writecount = 0;
    uint readsize = 0;

    int err = matter_fs_fopen(CRASH_LOG_FILENAME, fp, LFS_O_RDWR | LFS_O_CREAT);          // always make a new crash.log file
    if(err != 0) {
        ChipLogError(DeviceLayer, "open err: %d", err);
        return;
    }

	uint8_t *log = (uint8_t *)malloc(FAULT_FLASH_SECTOR_SIZE);
	if (!log)	{
		return;
	}
	memset(log, 0xFF, FAULT_FLASH_SECTOR_SIZE);
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	flash_stream_read(&fault_flash, FAULT_LOG1, FAULT_FLASH_SECTOR_SIZE, log);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);

	ChipLogProgress(DeviceLayer, ">>>>>> Copy fault log <<<<<<<<");
	if (log[0] != 0xFF) {
        while(1)    // scan for first 0xFF. replacement for strlen which only terminates at \0
        {
            if(log[readsize] == 0xFF) break;
            readsize++;
            if(readsize == FAULT_FLASH_SECTOR_SIZE) break;
        }

        res = matter_fs_fwrite(fp, log, readsize, 0, 0, false, &unused_writecount); // write to FS
        if(res >= 0) 
        {
            ChipLogProgress(DeviceLayer, "wrote %d bytes to crashlog", unused_writecount);
        }
        else
        {
            ChipLogError(DeviceLayer, "write error! %d", res);
            matter_fs_fclose(fp);
            return;
        }
	}

	device_mutex_lock(RT_DEV_LOCK_FLASH);
	flash_stream_read(&fault_flash, FAULT_LOG2, FAULT_FLASH_SECTOR_SIZE, log);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);

    readsize = 0;
    ChipLogProgress(DeviceLayer, ">>>>>>> Copy Backtrace log <<<<<<<<");
	if (log[0] != 0xFF) {
        while(1)
        {
            if(log[readsize] == 0xFF) break;
            readsize++;
            if(readsize == FAULT_FLASH_SECTOR_SIZE) break;
        }

        res = matter_fs_fwrite(fp, log, readsize, 0, 0, true, &unused_writecount);
        if(res >= 0) 
        {
            ChipLogProgress(DeviceLayer, "wrote %d bytes to crashlog", unused_writecount);
        }
        else
        {
            ChipLogError(DeviceLayer, "write error! %d", res);
            matter_fs_fclose(fp);
            return;
        }
	}

	free(log);
    matter_fs_fclose(fp);

    // erase fault log area for new log
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_erase_sector(&fault_flash, FAULT_LOG1);
	flash_erase_sector(&fault_flash, FAULT_LOG2);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);

    return;
}

#endif

#ifdef __cplusplus
}
#endif