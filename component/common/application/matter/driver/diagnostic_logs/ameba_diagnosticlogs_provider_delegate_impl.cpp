#include <platform_stdlib.h>
#include <platform_opts.h>
#include <device_lock.h>
#include <flash_api.h>
#include <lfs.h>

#if defined(CONFIG_ENABLE_AMEBA_DLOG) && (CONFIG_ENABLE_AMEBA_DLOG == 1)
#include <matter_fs.h>
#include <diagnostic_logs/ameba_diagnosticlogs_provider_delegate_impl.h>

#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>

using namespace chip;
using namespace chip::app::Clusters::DiagnosticLogs;

/* Attach the log provider delegate to the server via callback */
void emberAfDiagnosticLogsClusterInitCallback(chip::EndpointId endpoint)
{
    auto & logProvider = AmebaDiagnosticLogsProvider::GetInstance();
    DiagnosticLogsServer::Instance().SetDiagnosticLogsProviderDelegate(endpoint, &logProvider);
}

size_t GetFileSize(char *path, void *fpp)
{
    uint sz = matter_fs_fsize(fpp);

    ChipLogProgress(DeviceLayer, "sz: %d", sz);

    return sz;
}

namespace {
    bool IsValidIntent(IntentEnum intent)
    {
        return intent != IntentEnum::kUnknownEnumValue;
    }
}

AmebaDiagnosticLogsProvider::AmebaDiagnosticLogsProvider()
{
    int res;

    if (AmebaDiagnosticLogsProvider::bInitedFs == false)
    {
        res = matter_fs_get_init();
        if (res < 0)
        {
            ChipLogError(DeviceLayer, "Fail to init flash fs for logging! %" CHIP_ERROR_FORMAT, CHIP_ERROR_PERSISTED_STORAGE_FAILED);
            goto exit;
        } 

        res = matter_fs_fopen(USER_LOG_FILENAME, &fpUserLog, LFS_O_RDWR | LFS_O_CREAT);
        if (res < 0)
        {
            ChipLogError(DeviceLayer, "Failed to open user.log file. Error code: %d", res);
            goto exit;
        }

        res = matter_fs_fopen(NET_LOG_FILENAME, &fpNetdiagLog, LFS_O_RDWR | LFS_O_CREAT);
        if (res < 0)
        {
            ChipLogError(DeviceLayer, "Failed to open netdiag.log file. Error code: %d", res);
            goto cleanup_user_log;
        }

        res = matter_fs_fopen(CRASH_LOG_FILENAME, &fpCrashLog, LFS_O_RDWR | LFS_O_CREAT);
        if (res < 0)
        {
            ChipLogError(DeviceLayer, "Failed to open crash.log file. Error code: %d", res);
            goto cleanup_netdiag_log;
        }

        AmebaDiagnosticLogsProvider::bInitedFs = true;
        ChipLogProgress(DeviceLayer, "Ameba LogProvider ready!");
        return;

    }

cleanup_netdiag_log:
    matter_fs_fclose(&fpNetdiagLog); // Cleanup netdiag.log if crash.log fails
cleanup_user_log:
    matter_fs_fclose(&fpUserLog);    // Cleanup user.log if netdiag.log fails
exit:
    return; // Early exit in case of errors

}

AmebaDiagnosticLogsProvider::~AmebaDiagnosticLogsProvider() { }

size_t AmebaDiagnosticLogsProvider::GetSizeForIntent(IntentEnum intent)
{
    VerifyOrReturnValue(IsValidIntent(intent), 0);
    VerifyOrReturnValue(AmebaDiagnosticLogsProvider::bInitedFs, 0);

    size_t logsize = 0;

    switch (intent) 
    {
    case IntentEnum::kEndUserSupport:
        // Get the size of the current user.log file
        logsize = GetFileSize(NULL, &fpUserLog);
        break;
    case IntentEnum::kNetworkDiag:
        // Get the size of the current netdiag.log file
        logsize = GetFileSize(NULL, &fpNetdiagLog);
        break;
    case IntentEnum::kCrashLogs:
        // Get the size of the current crash.log file
        logsize = GetFileSize(NULL, &fpCrashLog);
        break;
    default:
        return 0; // Unrecognized intent, return 0
    }

    return logsize;
}

CHIP_ERROR AmebaDiagnosticLogsProvider::StartLogCollection(IntentEnum intent, LogSessionHandle & outHandle, Optional<uint64_t> & outTimeStamp, Optional<uint64_t> & outTimeSinceBoot)
{
    VerifyOrReturnValue(IsValidIntent(intent), CHIP_ERROR_INVALID_ARGUMENT);

    lfs_file_t* fp = nullptr;

    switch(intent)
    {
    case IntentEnum::kEndUserSupport:
        fp = &fpUserLog;
        break;
    case IntentEnum::kNetworkDiag:
        fp = &fpNetdiagLog;
        break;
    case IntentEnum::kCrashLogs:
        fp = &fpCrashLog;
        break;
    default:
        return CHIP_ERROR_INCORRECT_STATE;
    }

    VerifyOrReturnValue(fp != nullptr, CHIP_ERROR_INTERNAL);

    // Seek file back to the start for reading
    int err = matter_fs_fseek(fp, 0, LFS_SEEK_SET);
    VerifyOrReturnError(err == 0, CHIP_ERROR_INTERNAL);

    mLogSessionHandle++;
    // If the session handle rolls over to UINT16_MAX which is invalid, reset to 0.
    VerifyOrDo(mLogSessionHandle != kInvalidLogSessionHandle, mLogSessionHandle = 0);

    outHandle = mLogSessionHandle;
    mLogFiles[mLogSessionHandle] = fp;
    
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaDiagnosticLogsProvider::EndLogCollection(LogSessionHandle sessionHandle)
{
    VerifyOrReturnValue(sessionHandle != kInvalidLogSessionHandle, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(mLogFiles.count(sessionHandle), CHIP_ERROR_INVALID_ARGUMENT);

    lfs_file_t* fp = mLogFiles[sessionHandle];
    VerifyOrReturnError(fp != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // WARNING: If the handle is closed, it needs to be reopened or will assert in LFS layer on future calls
    matter_fs_fclear(fp);  // Since the file has been read, it is safe to clear it

    mLogFiles.erase(sessionHandle);  // Remove the session handle from the map

    ChipLogProgress(DeviceLayer, "Log Collection Ended");

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaDiagnosticLogsProvider::CollectLog(LogSessionHandle sessionHandle, MutableByteSpan & outBuffer, bool & outIsEndOfLog)
{
    lfs_file_t* fp = mLogFiles[sessionHandle]; // Obtain the file handle
    VerifyOrReturnError(fp != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    int filesize = matter_fs_fsize(fp); // Get the filesize
    VerifyOrReturnError(filesize >= 0, CHIP_ERROR_INTERNAL);

    int count = min<int>(filesize, outBuffer.size()); // Clamp the size we want to read
    uint bytesRead = 0;

    ChipLogProgress(DeviceLayer, "Cur/max: %d / %d", matter_fs_ftell(fp), filesize);

    int err = matter_fs_fread(fp, matter_fs_ftell(fp), count, outBuffer.data(), outBuffer.size(), &bytesRead);
    VerifyOrReturnError(err == 0, CHIP_ERROR_INTERNAL, outBuffer.reduce_size(0)); // Truncate buffer on error

    outBuffer.reduce_size(bytesRead); // Truncate remainder buffer space

    // Required for BDX transaction to tell it when to read next block
    outIsEndOfLog = filesize == matter_fs_ftell(fp);

    return CHIP_NO_ERROR; 
}

CHIP_ERROR AmebaDiagnosticLogsProvider::GetLogForIntent(IntentEnum intent, MutableByteSpan & outBuffer, Optional<uint64_t> & outTimeStamp, Optional<uint64_t> & outTimeSinceBoot)
{
    VerifyOrReturnValue(IsValidIntent(intent), CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err = CHIP_NO_ERROR;
    LogSessionHandle sessionHandle = kInvalidLogSessionHandle;

    err = StartLogCollection(intent, sessionHandle, outTimeStamp, outTimeSinceBoot);
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, outBuffer.reduce_size(0)); // return empty buffer if error

    bool unusedOutIsEndOfLog;
    err = CollectLog(sessionHandle, outBuffer, unusedOutIsEndOfLog);
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, outBuffer.reduce_size(0));

    err = EndLogCollection(sessionHandle);
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, outBuffer.reduce_size(0));

    return CHIP_NO_ERROR;
}
#endif /* CONFIG_ENABLE_AMEBA_DLOG */
