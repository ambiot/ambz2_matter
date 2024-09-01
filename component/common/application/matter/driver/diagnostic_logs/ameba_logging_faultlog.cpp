#include <FreeRTOS.h>
#include <platform_stdlib.h>
#include <platform_opts.h>
#include <device_lock.h>
#include <flash_api.h>
#include <string>

#if defined(CONFIG_ENABLE_AMEBA_DLOG) && (CONFIG_ENABLE_AMEBA_DLOG == 1)
#include <lfs.h>
#include <matter_fs.h>

#include <diagnostic_logs/ameba_diagnosticlogs_provider_delegate_impl.h>

#ifdef __cplusplus
extern "C" {
#endif

void matter_fault_log(char *msg, int len)
{
    flash_t fault_flash;
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_erase_sector(&fault_flash, FAULT_LOG1);
    flash_stream_write(&fault_flash, FAULT_LOG1, len, (uint8_t*)msg);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
}

void matter_bt_log(char *msg, int len)
{
    flash_t fault_flash;
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_erase_sector(&fault_flash, FAULT_LOG2);
    flash_stream_write(&fault_flash, FAULT_LOG2, len, (uint8_t*)msg);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
}

void matter_read_last_fault_log(void)
{
    flash_t fault_flash;
    lfs_file_t* fp = chip::app::Clusters::DiagnosticLogs::AmebaDiagnosticLogsProvider::GetFpCrashLog();
    int res;
    uint unused_writecount = 0;
    uint readsize = 0;

    //always make a new crash.log file
    int err = matter_fs_fopen(CRASH_LOG_FILENAME, fp, LFS_O_RDWR | LFS_O_CREAT);
    if (err != 0)
    {
        ChipLogError(DeviceLayer, "Open err: %d", err);
        return;
    }

    uint8_t *log = (uint8_t *)malloc(FAULT_FLASH_SECTOR_SIZE);
    if (!log)
    {
        ChipLogError(DeviceLayer, "Memory allocation failed");
        matter_fs_fclose(fp);
        return;
    }

    memset(log, 0xFF, FAULT_FLASH_SECTOR_SIZE);
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_stream_read(&fault_flash, FAULT_LOG1, FAULT_FLASH_SECTOR_SIZE, log);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    if (log[0] != 0xFF)
    {
        ChipLogProgress(DeviceLayer, "Copy Fault log.....");

        // find the first 0xFF. replacement for strlen which only terminates at \0
        while (readsize < FAULT_FLASH_SECTOR_SIZE)
        {
            if (log[readsize] == 0xFF) break;
            readsize++;
        }

        res = matter_fs_fwrite(fp, log, readsize, &unused_writecount); // write to FS
        if (res >= 0)
        {
            ChipLogProgress(DeviceLayer, "Wrote %d bytes to crashlog", unused_writecount);
        }
        else
        {
            ChipLogError(DeviceLayer, "Write error! %d", res);
            free(log);
            matter_fs_fclose(fp);
            return;
        }
    }

    device_mutex_lock(RT_DEV_LOCK_FLASH);
    flash_stream_read(&fault_flash, FAULT_LOG2, FAULT_FLASH_SECTOR_SIZE, log);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    readsize = 0;
    if (log[0] != 0xFF)
    {
        ChipLogProgress(DeviceLayer, "Copy Backtrace log......");

        while (readsize < FAULT_FLASH_SECTOR_SIZE)
        {
            if (log[readsize] == 0xFF) break;
            readsize++;
        }

        res = matter_fs_fwrite(fp, log, readsize, &unused_writecount);
        if (res >= 0) 
        {
            ChipLogProgress(DeviceLayer, "wrote %d bytes to crashlog", unused_writecount);
        }
        else
        {
            ChipLogError(DeviceLayer, "write error! %d", res);
            free(log);
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

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_ENABLE_AMEBA_DLOG */
