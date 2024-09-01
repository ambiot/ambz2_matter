#pragma once

#include <platform_opts.h>
#include <lfs.h>
#include <matter_fs.h>

#include <app/clusters/diagnostic-logs-server/DiagnosticLogsProviderDelegate.h>

#include <map>

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
    static AmebaDiagnosticLogsProvider & GetInstance()
    {
        static AmebaDiagnosticLogsProvider instance;
        return instance; 
    }
    static inline bool IsFSReady() { return bInitedFs; }

    // DiagnosticLogsProviderDelegate Interface
    CHIP_ERROR StartLogCollection(IntentEnum intent, LogSessionHandle & outHandle, Optional<uint64_t> & outTimeStamp,
                                Optional<uint64_t> & outTimeSinceBoot);
    CHIP_ERROR EndLogCollection(LogSessionHandle sessionHandle);
    CHIP_ERROR CollectLog(LogSessionHandle sessionHandle, MutableByteSpan & outBuffer, bool & outIsEndOfLog);
    size_t GetSizeForIntent(IntentEnum intent);
    CHIP_ERROR GetLogForIntent(IntentEnum intent, MutableByteSpan & outBuffer, Optional<uint64_t> & outTimeStamp,
                                Optional<uint64_t> & outTimeSinceBoot);

    static inline lfs_file_t* GetFpUserLog() { return &fpUserLog; }
    static inline lfs_file_t* GetFpNetdiagLog() { return &fpNetdiagLog; }
    static inline lfs_file_t* GetFpCrashLog() { return &fpCrashLog; }

private:
    //static AmebaDiagnosticLogsProvider sInstance;
    static inline bool bInitedFs = false;
    AmebaDiagnosticLogsProvider();
    ~AmebaDiagnosticLogsProvider();

    AmebaDiagnosticLogsProvider(const AmebaDiagnosticLogsProvider &)             = delete;
    AmebaDiagnosticLogsProvider & operator=(const AmebaDiagnosticLogsProvider &) = delete;

    static inline lfs_file_t fpUserLog;
    static inline lfs_file_t fpCrashLog;
    static inline lfs_file_t fpNetdiagLog;

    std::map<LogSessionHandle, lfs_file_t *> mLogFiles;

    LogSessionHandle mLogSessionHandle = kInvalidLogSessionHandle;
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
