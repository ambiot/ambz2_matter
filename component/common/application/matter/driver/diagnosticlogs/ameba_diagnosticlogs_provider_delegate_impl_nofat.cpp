#include "ameba_diagnosticlogs_provider_delegate_impl.h"
#include "chip_porting.h"
//#include "matter_flashfs.h"
#include "lfs.h"
#include "matter_fs.h"
#include "platform_opts_matter.h"
#include <flash_api.h>
#include <device_lock.h>

#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>

using namespace chip;
using namespace chip::app::Clusters::DiagnosticLogs;

/* Attach the log provider delegate to the server via callback */
void emberAfDiagnosticLogsClusterInitCallback(chip::EndpointId endpoint) {
    auto & logProvider = AmebaDiagnosticLogsProvider::GetInstance();
    DiagnosticLogsServer::Instance().SetDiagnosticLogsProviderDelegate(endpoint, &logProvider);
}

#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 0)

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
    if(AmebaDiagnosticLogsProvider::bInitedFs == false)
    {
        int res = matter_fs_get_init();
        if(res < 0) 
        {
            ChipLogError(DeviceLayer, "Fail to init flash fs for logging! %" CHIP_ERROR_FORMAT, CHIP_ERROR_PERSISTED_STORAGE_FAILED);
        } 
        else
        {
            matter_fs_fopen(USER_LOG_FILENAME,     &fpUserLog,     LFS_O_RDWR | LFS_O_CREAT);
            matter_fs_fopen(NET_LOG_FILENAME,      &fpNetdiagLog,  LFS_O_RDWR | LFS_O_CREAT);
            matter_fs_fopen(CRASH_LOG_FILENAME,    &fpCrashLog,    LFS_O_RDWR | LFS_O_CREAT);

            AmebaDiagnosticLogsProvider::bInitedFs = true;
            ChipLogProgress(DeviceLayer, "Ameba LogProvider ready!");
        }
    }
}

AmebaDiagnosticLogsProvider::~AmebaDiagnosticLogsProvider() { }

size_t AmebaDiagnosticLogsProvider::GetSizeForIntent(IntentEnum intent)
{
    VerifyOrReturnValue(IsValidIntent(intent), 0);
    VerifyOrReturnValue(AmebaDiagnosticLogsProvider::bInitedFs, 0);

    size_t logsize = 0;

    switch(intent) 
    {
        // size is currently not checked in server! send it something non zero
        case IntentEnum::kEndUserSupport:
            logsize = GetFileSize(NULL, &fpUserLog);                        // get the size of the current user.log file
            return logsize;

        case IntentEnum::kNetworkDiag:
            logsize = GetFileSize(NULL, &fpNetdiagLog);                     // get the size of the current netdiag.log file
            return logsize;

        case IntentEnum::kCrashLogs:
            logsize = GetFileSize(NULL, &fpCrashLog);                       // get the size of the current crash.log file
            return logsize;
        default:
            break;
    }

    return 0;
}

/**
    Open file handles for reading logs from here
*/
CHIP_ERROR AmebaDiagnosticLogsProvider::StartLogCollection(IntentEnum intent, LogSessionHandle & outHandle, Optional<uint64_t> & outTimeStamp, Optional<uint64_t> & outTimeSinceBoot)
{
    VerifyOrReturnValue(IsValidIntent(intent), CHIP_ERROR_INVALID_ARGUMENT);

    int err;

    lfs_file_t* fp;

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

    VerifyOrReturnValue(nullptr != fp, CHIP_ERROR_INTERNAL);                    // handle could not be opened (fp is null)

    // seek file back to top to read
    matter_fs_fseek(fp, 0, LFS_SEEK_SET);

    mLogSessionHandle++;
    // If the session handle rolls over to UINT16_MAX which is invalid, reset to 0.
    VerifyOrDo(mLogSessionHandle != kInvalidLogSessionHandle, mLogSessionHandle = 0);

    outHandle = mLogSessionHandle;
    mLogFiles[mLogSessionHandle] = fp;
    
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaDiagnosticLogsProvider::EndLogCollection(LogSessionHandle sessionHandle)
{
    /* Close the file handles here */
    VerifyOrReturnValue(sessionHandle != kInvalidLogSessionHandle, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnValue(mLogFiles.count(sessionHandle), CHIP_ERROR_INVALID_ARGUMENT);

    lfs_file_t* fp;

    fp = mLogFiles[sessionHandle];

    // WARNING: if handle is closed, it needs to be reopened or assert in LFS layer when a future call is made
    matter_fs_fclear(fp);   // because the file has been read it is safe to clear it

    mLogFiles.erase(sessionHandle);

    ChipLogProgress(DeviceLayer, ">>> EndLogCollection");

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaDiagnosticLogsProvider::CollectLog(LogSessionHandle sessionHandle, MutableByteSpan & outBuffer, bool & outIsEndOfLog)
{
    lfs_file_t* fp;

    fp = mLogFiles[sessionHandle];                                                 // obtain the file handle
    int filesize = matter_fs_fsize(fp);                                             // get the filesize
    int count = min<int>(filesize, outBuffer.size());                              // clamp the size we want to read

    uint bytesRead = 0;
    printf("Cur/max: %d / %d \n", matter_fs_ftell(fp), matter_fs_fsize(fp));
    int err = matter_fs_fread(fp, matter_fs_ftell(fp), count, outBuffer.data(), outBuffer.size(), &bytesRead);
    VerifyOrReturnError(err == 0, CHIP_ERROR_INTERNAL, outBuffer.reduce_size(0));

    outBuffer.reduce_size(bytesRead);                                              // truncate remainder buffer space

    /* required for BDX transaction to tell it when to read next block */
    outIsEndOfLog = filesize == matter_fs_ftell(fp);

    return CHIP_NO_ERROR; 
}

CHIP_ERROR AmebaDiagnosticLogsProvider::GetLogForIntent(IntentEnum intent, MutableByteSpan & outBuffer, Optional<uint64_t> & outTimeStamp, Optional<uint64_t> & outTimeSinceBoot)
{
    VerifyOrReturnValue(IsValidIntent(intent), CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err = CHIP_NO_ERROR;
    LogSessionHandle sessionHandle = kInvalidLogSessionHandle;
    err = StartLogCollection(intent, sessionHandle, outTimeStamp, outTimeSinceBoot);
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, outBuffer.reduce_size(0));       // return empty buffer if error

    bool unusedOutIsEndOfLog;
    err = CollectLog(sessionHandle, outBuffer, unusedOutIsEndOfLog);
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, outBuffer.reduce_size(0));

    err = EndLogCollection(sessionHandle);
    VerifyOrReturnError(CHIP_NO_ERROR == err, err, outBuffer.reduce_size(0));

    return CHIP_NO_ERROR;
}
#endif