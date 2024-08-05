#pragma once

#include <app/clusters/diagnostic-logs-server/DiagnosticLogsProviderDelegate.h>
#include <map>

#include "ff.h"
#include "matter_flashfs.h"
#include "platform_opts_matter.h"

size_t GetFileSize(char* path, void* fpp);

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {
/**
 * The LogProvider class serves as the sole instance delegate for handling diagnostic logs.
 *
 * It implements the DiagnosticLogsProviderDelegate interface
 */
class AmebaDiagnosticLogsProvider : public DiagnosticLogsProviderDelegate
{
public:
    static AmebaDiagnosticLogsProvider & GetInstance() { 
        static AmebaDiagnosticLogsProvider instance;
        return instance; 
    }
    static inline bool IsFSReady() { return bInitedFs; }

    /////////// DiagnosticLogsProviderDelegate Interface /////////
    CHIP_ERROR StartLogCollection(IntentEnum intent, LogSessionHandle & outHandle, Optional<uint64_t> & outTimeStamp,
                                  Optional<uint64_t> & outTimeSinceBoot);
    CHIP_ERROR EndLogCollection(LogSessionHandle sessionHandle);
    CHIP_ERROR CollectLog(LogSessionHandle sessionHandle, MutableByteSpan & outBuffer, bool & outIsEndOfLog);
    size_t GetSizeForIntent(IntentEnum intent);
    CHIP_ERROR GetLogForIntent(IntentEnum intent, MutableByteSpan & outBuffer, Optional<uint64_t> & outTimeStamp,
                               Optional<uint64_t> & outTimeSinceBoot);

#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 0)
    static inline amb_fsctx_t* GetFpUserLog() { return &fpUserLog; }
    static inline amb_fsctx_t* GetFpNetdiagLog() { return &fpNetdiagLog; }
    static inline amb_fsctx_t* GetFpCrashLog() { return &fpCrashLog; }
#endif

private:
    //static AmebaDiagnosticLogsProvider sInstance;
    static inline bool bInitedFs = false;
    AmebaDiagnosticLogsProvider();
    ~AmebaDiagnosticLogsProvider();

    AmebaDiagnosticLogsProvider(const AmebaDiagnosticLogsProvider &)             = delete;
    AmebaDiagnosticLogsProvider & operator=(const AmebaDiagnosticLogsProvider &) = delete;

    /* ameba specific */
#if defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 1)
    static inline FIL fpUserLog;
    static inline FIL fpNetdiagLog;
    static inline FIL fpCrashLog;

    std::map<LogSessionHandle, FIL *> mLogFiles;
#elif defined(CONFIG_AMEBA_LOGS_USE_FATFS) && (CONFIG_AMEBA_LOGS_USE_FATFS == 0)
    
    /**
        Define memory regions to hold "files" that you can read/write to (simple fs only)
        addr_start  -> start of region, must be 4KB aligned!
        addr_end    -> end of region
        fptr        -> always set this to the start of the region
        fsize       -> always 0, this is automatically calculated by the API
    */

    static inline amb_fsctx_t fpUserLog = {
        .label =        "LOG_USER", 
        .addr_start =   USER_LOG_ADDR,      
        .addr_end =     (USER_LOG_ADDR + USER_LOG_MAXSZ),
        .fptr =         USER_LOG_ADDR,
        .fsize =        0
    };
    static inline amb_fsctx_t fpNetdiagLog = { 
        .label =        "LOG_NETDIAG",
        .addr_start =   NETDIAG_LOG_ADDR,      
        .addr_end =     (NETDIAG_LOG_ADDR + NETDIAG_LOG_MAXSZ),
        .fptr =         NETDIAG_LOG_ADDR,
        .fsize =        0
    };
    static inline amb_fsctx_t fpCrashLog = {
        .label =        "LOG_CRASH", 
        .addr_start =   CRASH_LOG_ADDR,      
        .addr_end =     (CRASH_LOG_ADDR + CRASH_LOG_MAXSZ),
        .fptr =         CRASH_LOG_ADDR,
        .fsize =        0
    };

    std::map<LogSessionHandle, amb_fsctx_t *> mLogFiles;
#endif
    LogSessionHandle mLogSessionHandle = kInvalidLogSessionHandle;

    
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
