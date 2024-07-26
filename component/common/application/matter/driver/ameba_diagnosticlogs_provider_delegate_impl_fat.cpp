
#include "ameba_diagnosticlogs_provider_delegate_impl.h"
#include "chip_porting.h"
#include "matter_flashfs.h"
#include "platform_opts_matter.h"
#include <flash_api.h>
#include <device_lock.h>

#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 1)

size_t GetFileSize(char* path, void* fpp)
{
    FIL fp;
    int err = matter_flashfs_file_open(path, &fp, FA_OPEN_ALWAYS | FA_READ);
    if(err != 0) {
        ChipLogError(DeviceLayer, "open err: %d", err);
        return 0;
    }

    uint sz = matter_flashfs_file_size(&fp);
    matter_flashfs_file_close(&fp);

    ChipLogProgress(DeviceLayer, "sz: %d", sz);

    return sz;
}

using namespace chip;
using namespace chip::app::Clusters::DiagnosticLogs;

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
        // init filesystem
        // TODO: fix init order
        int res = matter_flashfs_get_inited();
        if(res < 0) 
        {
            ChipLogError(DeviceLayer, "Fail to init flash fs for logging!");
        } 
        else 
        {
            matter_flashfs_scanfiles("0:/");
        }

        AmebaDiagnosticLogsProvider::bInitedFs = true;
        ChipLogProgress(DeviceLayer, "Ameba LogProvider ready!");
    }
}

AmebaDiagnosticLogsProvider::~AmebaDiagnosticLogsProvider() { }

size_t AmebaDiagnosticLogsProvider::GetSizeForIntent(IntentEnum intent)
{
    /**
     * Tells the remote how much to expect for a given log
    */
    VerifyOrReturnValue(IsValidIntent(intent), 0);
    VerifyOrReturnValue(AmebaDiagnosticLogsProvider::bInitedFs, 0);

    size_t logsize = 0;

    switch(intent) 
    {
        // size is currently not checked in server! send it something non zero
        case IntentEnum::kEndUserSupport:
            ChipLogProgress(Zcl, "@@@ type: kEndUserSupport");
            logsize = GetFileSize("user.log", NULL);                      // get the size of the current user.log file
            return logsize;

        case IntentEnum::kNetworkDiag:
            ChipLogProgress(Zcl, "@@@ type: kNetworkDiag");
            logsize = GetFileSize("netdiag.log", NULL);                   // get the size of the current netdiag.log file
#if 0
        /**
         * Debug use only
         * Forces the device to crash because the instruction being executed is not aligned to a 4-byte value as required by ARM core
         * Use only to generate crash log for testing
         * TODO: Move this into AT command OR matter shell command
        */
            ((void (*)(void))2)();
#endif
            return logsize;

        case IntentEnum::kCrashLogs:
            ChipLogProgress(Zcl, "@@@ type: kCrashLogs");
            logsize = GetFileSize("crash.log", NULL);                     // get the size of the current crash.log file
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
    FIL* fp; 

    switch(intent)
    {
        case IntentEnum::kEndUserSupport:
            fp = &AmebaDiagnosticLogsProvider::fpUserLog;
            err = matter_flashfs_file_open("user.log", fp, FA_OPEN_ALWAYS | FA_READ);
            break;
        case IntentEnum::kNetworkDiag:
            fp = &AmebaDiagnosticLogsProvider::fpNetdiagLog;
            err = matter_flashfs_file_open("netdiag.log", fp, FA_OPEN_ALWAYS | FA_READ);
            break;
        case IntentEnum::kCrashLogs:
            fp = &AmebaDiagnosticLogsProvider::fpCrashLog;
            err = matter_flashfs_file_open("crash.log", fp, FA_OPEN_ALWAYS | FA_READ);
            break;
        default:
            return CHIP_ERROR_INCORRECT_STATE;
    }

    VerifyOrReturnValue(nullptr != fp, CHIP_ERROR_INTERNAL);    // handle could not be opened (fp is null)
    VerifyOrReturnValue(err == 0, CHIP_ERROR_INTERNAL);         // error occured in fs

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

    FIL* fp; 
    
    fp = mLogFiles[sessionHandle];

#if defined(CONFIG_EXAMPLE_MATTER_LIGHT_LOGREDIRECT) && (CONFIG_EXAMPLE_MATTER_LIGHT_LOGREDIRECT == 1)
    // set fp back to start
    //matter_flashfs_file_lseek(fp, 0);
    matter_flashfs_file_reset(fp);
#endif

    int err = matter_flashfs_file_close(fp);
    VerifyOrReturnError(err == 0, CHIP_ERROR_INTERNAL);

    mLogFiles.erase(sessionHandle);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaDiagnosticLogsProvider::CollectLog(LogSessionHandle sessionHandle, MutableByteSpan & outBuffer, bool & outIsEndOfLog)
{
    /**
     * Read the file here based on which handle was provided
     * Context for tracking read operation is not required because we fallback to using f_tell in the fs driver
     */
    FIL* fp; 

    fp = mLogFiles[sessionHandle];                                                          // obtain the file handle
    int filesize = matter_flashfs_file_size(fp);                                        // get the filesize
    int count = min<int>(filesize, outBuffer.size());                                   // clamp the size we want to read

    uint bytesRead = 0;
    int err = matter_flashfs_file_read(fp, matter_flashfs_file_tell(fp), count, outBuffer.data(), outBuffer.size(), &bytesRead);    // start from the fp position
    VerifyOrReturnError(err == 0, CHIP_ERROR_INTERNAL, outBuffer.reduce_size(0));

    outBuffer.reduce_size(bytesRead);                                              // truncate remainder buffer space

    /* required for BDX transaction to tell it when to read next block */
    outIsEndOfLog = filesize == matter_flashfs_file_tell(fp);

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