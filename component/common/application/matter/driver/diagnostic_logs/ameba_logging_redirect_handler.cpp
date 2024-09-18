/**
   WARNING: THIS FILE MAY BE MOVED INTO connectedhomeip/platform!! CHANGES ARE NOT FINAL (YET)
*/
#include <platform_opts.h>
#include <cstdint>
#include <deque>
#include <stdlib.h>
#include <sys/time.h>

#if defined(CONFIG_ENABLE_AMEBA_DLOG) && (CONFIG_ENABLE_AMEBA_DLOG == 1)
#include <lfs.h>
#include <matter_timers.h>
#include <matter_fs.h>

#include <diagnostic_logs/ameba_logging_redirect_handler.h>
#include <diagnostic_logs/ameba_diagnosticlogs_provider_delegate_impl.h>

#include <core/ErrorStr.h>
#include <platform/logging/LogV.h>
#include <support/logging/TextOnlyLogging.h>

#define EMPTY_PADDING_BYTE 0xCC

using namespace chip;
using namespace chip::Logging;

// hack. manually extend std namespace with own impl that does nothing (std::__throw_bad_array_new_length)
namespace std {
    void __throw_bad_array_new_length() {
        // not allowed to continue
        while (true) {}
    }
}

AmebaLogRedirectHandler::AmebaLogRedirectHandler() { }

AmebaLogRedirectHandler::~AmebaLogRedirectHandler() { }

static bool ClearLogStrategy(void* fp)
{
    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    // Step 1: Seek to the most recent N logs
    int pos = matter_fs_ftell(lfs_fp);
    int region_size = sizeof(amebalog_t) * RETAIN_NLOGS_WHEN_FULL;
    int pos_to_read_from = pos - region_size;

    // Ensure the position is not negative
    if (pos_to_read_from < 0)
    {
        pos_to_read_from = 0;
    }

    if (matter_fs_fseek(lfs_fp, pos_to_read_from, LFS_SEEK_SET) != 0)
    {
        return false;
    }

    // Step 2: Read into temp buffer
    void* tmp = malloc(region_size);
    if (tmp == NULL)
    {
        return false;
    }

    uint out_num = 0;
    if (matter_fs_fread(lfs_fp, pos_to_read_from, region_size, tmp, region_size, &out_num) != 0)
    {
        free(tmp);
        return false;
    }

    // Step 3: Truncate the file
    if (matter_fs_fclear(lfs_fp) != 0)
    {
        free(tmp);
        return false;
    }

    // Step 4: Rewrite the copied contents back
    if (matter_fs_fwrite(lfs_fp, tmp, region_size, &out_num) != 0)
    {
        free(tmp);
        return false;
    }

    // Step 5: Free the temp buffer and return success
    free(tmp);
    return true;
}

/**
 * @brief Custom ErrorFormatter to intercept ChipLogErrors before they get transformed into the human-friendly string
 * Use this to redirect errors in subsystem (e.g BLE, Commissioning) to our log backend for DiagnosticLogs
 *
 * @param buf buffer containing the message
 * @param bufSize size of buffer
 * @param err the CHIP_ERROR, see CHIPError.cpp for modification
 * @return true if the error was handled by this formatter. false to let another formatter in the linkedlist parse it
 *
 *  Store the log format in the following structure
 *  [ 0x88 ][ 0xAA 0xAA 0xAA 0xAA ][ 0xBB ][ 0xCC 0xCC ] | [ 0xDD 0xDD ] [ 0xEE ... ]
 *     len        timestamp        ^                     ^   LineNumber    FileName
 *  _______________________________|                     |
 *  |                                                    --- end here if short format
 *  V
 *  [ X X X | X X X X X ][ A A A A A A A A ][ A A A A A A A A]
 *  |       |           ||                                   |
 *  |_______|___________||___________________________________|
 *    3 bit     5 bit              2 byte (uint16_t)
 *    Part   Subsys code         Subsystem Error Value
 *                                (see ERROR_CODES.md)
 */
bool AmebaErrorFormatter(char* buf, uint16_t bufSize, CHIP_ERROR err)
{
    // Bypass all custom handling if the log subsystem was not Inited
    if (AmebaLogRedirectHandler::GetAmebaLogSubsystemInited() == false)
    {
        return false;
    }

    // Bypass file operations if the filesystem was not initialized
    if (matter_fs_get_init() == false)
    {
        return false;
    }

    // Do not call err.Format() here as it may cause infinite recursion
    if (!(
        err.IsPart(chip::ChipError::SdkPart::kBLE) ||
        err.IsPart(chip::ChipError::SdkPart::kInet) ||
        err.IsRange(chip::ChipError::Range::kPlatform)
    ))
    {
        return false;
    }

    // Get timestamp
    auto time_s = ameba_get_clock_time();

    AmebaLog log = { 0 }; // Zero out entire struct
    log.LogLen      = sizeof(amebalog_t);
    log.Timestamp   = (uint32_t)time_s;
    log.ErrorRange  = (uint8_t)err.GetRange();
    log.ErrorPart   = (uint8_t)err.GetSdkCode();
    log.ErrorValue  = (uint8_t)err.GetValue();

#if defined(CONFIG_ENABLE_AMEBA_SHORT_LOGGING) && (CONFIG_ENABLE_AMEBA_SHORT_LOGGING == 0)

    auto path_cstr = err.GetFile();
    int namelen = 0;

    // set error filename. can be nullptr, in that case the filename is empty
    if (path_cstr != nullptr)
    {
        std::string path = err.GetFile();
        std::string base_filename = path.substr(path.find_last_of("/\\") + 1);
        namelen = (base_filename.length() > CONFIG_AMEBA_LOG_FILENAME_MAXSZ) ? CONFIG_AMEBA_LOG_FILENAME_MAXSZ : base_filename.length();

        memcpy(&log.ErrorFilename, base_filename.c_str(), namelen);
        log.ErrorFilename[namelen] = '\0'; // null terminate.
    }

    log.ErrorLineNo = (uint16_t)err.GetLine(); // set line number
    log.LogLen = 8 + 2 + namelen; // set new log length if long format is used

#endif // CONFIG_ENABLE_AMEBA_SHORT_LOGGING

    // Write directly to FS
    auto fp = app::Clusters::DiagnosticLogs::AmebaDiagnosticLogsProvider::GetFpNetdiagLog();
    if (fp == nullptr)
    {
        ChipLogError(DeviceLayer, "File pointer is null");
        return false;
    }

    uint numWrite = 0;
    int status = matter_fs_fwrite(fp, &log, log.LogLen, &numWrite);
    if (status < 0)
    {
        if (status == LFS_ERR_NOSPC)
        {
            // No space left on device, attempt clear strategy
            if (ClearLogStrategy(fp) == false)
            {
                ChipLogError(DeviceLayer, "Failed to execute log clear strategy");
                return false;
            }
            else
            {
                // Retry writing after clearing
                int status = matter_fs_fwrite(fp, &log, log.LogLen, &numWrite);
                if (status < 0)
                {
                    ChipLogError(DeviceLayer, "Error writing log to file after clearing: %d", status);
                    return false;
                }
            }
        }
        else
        {
            ChipLogError(DeviceLayer, "Error writing log to file: %d", status);
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "fs wrote %d bytes", numWrite);
    }

    return false; // always return false because we need other formatters to parse
}

/**
 * @briefEarly Log capture for driver or board init. see matter/api/matter_log_api.h for usage
 *
 * @param buf buffer containing the message
 * @param bufSize unused
 * @param err the CHIP_ERROR, see CHIPError.cpp for modification
*/
void AmebaLogRedirectHandler::AmebaEarlyError(char* buf, uint16_t bufSize, CHIP_ERROR err)
{
    // bypass all custom handling if the log subsystem was not initialized
    if (AmebaLogRedirectHandler::GetAmebaLogSubsystemInited() == true)
    {
        AmebaErrorFormatter(buf, bufSize, err);
    }

    ChipLogError(DeviceLayer, "%s %" CHIP_ERROR_FORMAT, buf, err.Format());

    return;
}

/**
 * @brief Log callback to intercept and modify log / call other logging subsystem
 *        This method can also act as a "coarse" sieve to operate on the final string message

 * @param module Short name of module as listed in lib/support/logging/TextOnlyLogging.cpp::ModuleNames. Can be custom
 * @param category Log level, 0 to 4 (0 is to not print) based on enum lib/support/logging/Constants.h::LogCategory
 * @param msg format string like printf
 * @param args variable list to fill up the format string
*/
static void AmebaLogRedirect(const char* module, uint8_t category, const char* msg, va_list args)
{
    chip::Logging::Platform::LogV(module, category, msg, args);
}

void AmebaLogRedirectHandler::RegisterAmebaLogRedirect(void) 
{
    if (this->bAmebaLogRedirected)
    {
        return;
    }

    ChipLogProgress(DeviceLayer, "Ameba Log Redirector registered: redirecting logs after this line");

    SetLogRedirectCallback(AmebaLogRedirect);

    this->bAmebaLogRedirected = true;
}

ErrorFormatter AmebaLogRedirectHandler::sAmebaErrorFormatter = { AmebaErrorFormatter, nullptr };

void AmebaLogRedirectHandler::RegisterAmebaErrorFormatter(void)
{
    if (this->bAmebaLogRedirected)
    {
        return;
    }

    RegisterErrorFormatter(&sAmebaErrorFormatter);

    ChipLogProgress(DeviceLayer, "Ameba Error Formatter registered");

    this->bAmebaCustomFormatterSet = true;
}

#endif /* CONFIG_ENABLE_AMEBA_DLOG */
