/**
   WARNING: THIS FILE MAY BE MOVED INTO connectedhomeip/platform!! CHANGES ARE NOT FINAL (YET)
*/

#include "ameba_logging_redirect_handler.h"
#include "ameba_diagnosticlogs_provider_delegate_impl.h"
#include "chip_porting.h"
#include <core/ErrorStr.h>
#include <cstdint>
#include <deque>
#include <support/logging/TextOnlyLogging.h>
#include <platform/logging/LogV.h>
//#include "matter_flashfs_nofat.h"
#include "lfs.h"
#include "platform_opts_matter.h"
#include <stdlib.h>
#include <sys/time.h>

/* filesystem */
//#include "matter_flashfs.h"
#include "matter_fs.h"

#define EMPTY_PADDING_BYTE 0xCC

using namespace chip;
using namespace chip::Logging;

//std::deque<amebalog_t> mLogBuffer;

// hack. manually extend std namespace with own impl that does nothing (std::__throw_bad_array_new_length)
namespace std {
    void __throw_bad_array_new_length() {
        // not allowed to continue
        while (true) {}
        return;
    }
}

AmebaLogRedirectHandler::AmebaLogRedirectHandler() { }

AmebaLogRedirectHandler::~AmebaLogRedirectHandler() { }

#if 0
/**
 * Generates a human-readable NULL-terminated C string describing the provided error.
 *
 * @param[in] buf                   Buffer into which the error string will be placed.
 * @param[in] bufSize               Size of the supplied buffer in bytes.
 * @param[in] subsys                A short string describing the subsystem that originated
 *                                  the error, or NULL if the origin of the error is
 *                                  unknown/unavailable.  This string should be 10
 *                                  characters or less.
 * @param[in] err                   The error to be formatted.
 * @param[in] desc                  A string describing the cause or meaning of the error,
 *                                  or NULL if no such information is available.
 */
DLL_EXPORT void chip::FormatError(char * buf, uint16_t bufSize, const char * subsys, CHIP_ERROR err, const char * desc)
{
    const char * subsysSep = " ";
    const char * descSep   = ": ";

    if (subsys == nullptr)
    {
        subsys    = "";
        subsysSep = "";
    }
    if (desc == nullptr)
    {
        desc    = "";
        descSep = "";
    }

    (void) snprintf(buf, bufSize, "AMB>> %s%sError 0x%08" PRIX32 "%s%s", subsys, subsysSep, err.AsInteger(), descSep, desc);
}
#endif
/*
static uint32_t GetLogBufferByteSize()
{
    uint32_t size = 0;
    if(mLogBuffer.size() == 0) return size;

    for(int i = 0; i < mLogBuffer.size(); i++)
    {
        size += sizeof(mLogBuffer[i]);
    }

    return size;
}*/

/**
    Flush the log buffer to backing filesystem when threshold is reached. TODO change threshold

    @return true on success, false on error
*/
#if 0
static bool FlushBufferIfReady() 
{
    if(matter_flashfs_get_inited() == false)
        return false;

    // check if we have enough entries to flush
    uint32_t buffersz = GetLogBufferByteSize();
    if(buffersz >= SECTOR_SIZE_FLASH) {
        int err = 0;
        
        ChipLogProgress(DeviceLayer, "<*> size of mLogbuffer: %d | bytes: %d", mLogBuffer.size(), buffersz);
        // obtain the file descriptor to write to. for now everything will be thrown into netdiag
        auto fp = app::Clusters::DiagnosticLogs::AmebaDiagnosticLogsProvider::GetFpNetdiagLog();
        
        uint8_t* buf = (uint8_t*)malloc(SECTOR_SIZE_FLASH);                                 // allocate scratch memory to copy to (4KB)
        if(buf == nullptr || buf == NULL) {
            ChipLogError(DeviceLayer, "Unable to allocate memory for flushing log! %" CHIP_ERROR_FORMAT, CHIP_ERROR_NO_MEMORY);
            return false;
        }

        memset(buf, EMPTY_PADDING_BYTE, SECTOR_SIZE_FLASH);                                 // initialize this memory region. Unused areas will be marked with padding byte
        
        int offset = 0;
        while(!mLogBuffer.empty())                                                          // copy each item in the queue out. this is a FIFO, so pop is always the earliest log, thus we have sequential access
        {
            amebalog_t item = mLogBuffer.front();
            if(offset + sizeof(amebalog_t) >= SECTOR_SIZE_FLASH)                            // first check if we will exceed the size of one block
                break;

            memcpy(buf + offset, &item, sizeof(amebalog_t));                                // do NOT increment buf, an offset should always be used, otherwise free() will fail
            offset += sizeof(amebalog_t);                                                   // increment offset
            mLogBuffer.pop_front();                                                         // then pop the item from the deque
        }

        /*  
            check if we are about to write past the bounds of the file. if yes, it means old log must be cleared!
            the chosen strategy is to shift all data to write a new block at the end
            the upside is it is easier to write, but the downside is that more sectors need to be written over
            the consideration is that hopefully logs will be pulled in time before this major overwrite happens (only when log file is full)
        */
        if(matter_flashfs_file_size(fp) + SECTOR_SIZE_FLASH > (fp->addr_end - fp->addr_start))
        {
            ChipLogProgress(DeviceLayer, "Logfile full! Rotating old logs...");

            err = matter_flashfs_file_clearlastblock(fp);                                   // execute the log rotation strategy (NOT FINAL)
            if(err > 0) {
                ChipLogError(DeviceLayer, "Sector shift failed! %" CHIP_ERROR_FORMAT, CHIP_ERROR_PERSISTED_STORAGE_FAILED);
                return false;
            }
        }

        // for debug: write a marker showing this is a new block
        memcpy(buf+SECTOR_SIZE_FLASH-4, &AmebaLogRedirectHandler::cxxcount, sizeof(uint32_t));

        
        uint written = 0;
        err = matter_flashfs_file_write(fp, buf, SECTOR_SIZE_FLASH, 0, 0, true, &written);  // write buffer to flash
        if(err > 0) 
        {
            free(buf);                                                                      // clear memory
            ChipLogError(DeviceLayer, "fs driver write failed: %" CHIP_ERROR_FORMAT, CHIP_ERROR_WRITE_FAILED);
            return false;
        }

        free(buf);                                                                          // clear memory
        ChipLogProgress(DeviceLayer, "[LOG] successfully flushed to log store, number of events remain: %d | %d", mLogBuffer.size(), AmebaLogRedirectHandler::cxxcount);
        AmebaLogRedirectHandler::cxxcount++;                                                // debug: increment counter
        return true;
    }

    return false;
}
#endif

static bool ClearLogStrategy(void* fp)
{
    lfs_file_t* lfs_fp = (lfs_file_t*)fp;

    // step 1: seek the most recent N logs
    int pos = matter_fs_ftell(lfs_fp);
    int region_size = (sizeof(amebalog_t) * RETAIN_NLOGS_WHEN_FULL);
    int pos_to_read_from = pos - region_size; // assuming that this lands on the start of a new AmebaLog_t message
    matter_fs_fseek(fp, pos_to_read_from, LFS_SEEK_SET);

    // step 2: read into temp buffer
    void* tmp = malloc(region_size);
    uint out_num = 0;
    if(tmp == NULL) {
        return false;
    }
    matter_fs_fread(lfs_fp, pos_to_read_from, region_size, tmp, region_size, &out_num);

    // step 3: truncate the file
    matter_fs_fclear(lfs_fp);

    // step 4: rewrite the copied contents back
    matter_fs_fwrite(fp, tmp, region_size, 0, 0, 0, &out_num);

    // step 5: free
    free(tmp);

    return true;
}

/**
    Custom ErrorFormatter to intercept ChipLogErrors before they get transformed into the human-friendly string
    Use this to redirect errors in subsystem (e.g BLE, Commissioning) to our log backend for DiagnosticLogs

    @param buf buffer containing the message
    @param bufSize size of buffer
    @param err the CHIP_ERROR, see CHIPError.cpp for modification
    @return true if the error was handled by this formatter. false to let another formatter in the linkedlist parse it
*/
bool AmebaErrorFormatter(char* buf, uint16_t bufSize, CHIP_ERROR err)
{
    // bypass all custom handling if the log subsystem was not Inited 
    if(AmebaLogRedirectHandler::GetAmebaLogSubsystemInited() == false) {
        return false;
    }

    // bypass file ops if the filesystem was not inited
    if(matter_fs_get_init() == false) {
        return false;
    }

    // warning: do NOT call err.Format() here as it will throw the format processor into infinite recursion
    if(!(
        err.IsPart(chip::ChipError::SdkPart::kBLE) ||
        err.IsPart(chip::ChipError::SdkPart::kInet) ||
        err.IsRange(chip::ChipError::Range::kPlatform)
    ))
    {
        return false;
    }

    /*
        Store the log format in the following structure
        [ 0x88 ][ 0xAA 0xAA 0xAA 0xAA ][ 0xBB ][ 0xCC 0xCC ] | [ 0xDD 0xDD ] [ 0xEE ... ]
           len        timestamp        ^                     ^   LineNumber    FileName
        _______________________________|                     |
        |                                                    --- end here if short format
        V
        [ X X X | X X X X X ][ A A A A A A A A ][ A A A A A A A A]
        |       |           ||                                   |
        |_______|___________||___________________________________|
          3 bit     5 bit              2 byte (uint16_t)
          Part   Subsys code         Subsystem Error Value
                                      (see ERROR_CODES.md)
    */

    // get timestamp
    auto time_s = ameba_get_clock_time();

    AmebaLog log = { 0 };                                                               // zero out entire struct
    log.LogLen      = sizeof(amebalog_t);
    log.Timestamp   = (uint32_t)time_s;
    log.ErrorRange  = (uint8_t)err.GetRange();
    log.ErrorPart   = (uint8_t)err.GetSdkCode();
    log.ErrorValue  = (uint8_t)err.GetValue();

#if defined(CONFIG_AMEBA_MATTER_SHORT_LOG_FMT) && (CONFIG_AMEBA_MATTER_SHORT_LOG_FMT == 0)
    
    auto path_cstr = err.GetFile();
    int namelen = 0;
    
    // set error filename. can be nullptr, in that case the filename is empty
    if(path_cstr != nullptr)
    {
        std::string path = err.GetFile();
        std::string base_filename = path.substr(path.find_last_of("/\\") + 1);
        namelen = (base_filename.length() > CONFIG_AMEBA_MATTER_LOG_FILENAME_MAXSZ) ? CONFIG_AMEBA_MATTER_LOG_FILENAME_MAXSZ : base_filename.length();
        
        memcpy(&log.ErrorFilename, base_filename.c_str(), namelen);
        log.ErrorFilename[namelen] = '\0';                                              // null terminate.
    }
    
    log.ErrorLineNo = (uint16_t)err.GetLine();                                          // set line number
    log.LogLen = 8 + 2 + namelen;                                                       // set new log length if long format is used

#endif // CONFIG_AMEBA_MATTER_SHORT_LOG_FMT

    //mLogBuffer.push_back(log);

    //FlushBufferIfReady();
    // write directly to FS
    auto fp = app::Clusters::DiagnosticLogs::AmebaDiagnosticLogsProvider::GetFpNetdiagLog();
    uint numWrite = 0;
    int status = matter_fs_fwrite(fp, &log, log.LogLen, 0, 0, true, &numWrite);
    if(status < 0) 
    {
        if(status == LFS_ERR_NOSPC) 
        {
            // no space left on device, attempt clear strategy
            if(ClearLogStrategy(fp) == false)
            {
                ChipLogError(DeviceLayer, "Failed to execute log clear strategy\n");
                return false;
            }
            else 
            {
                // reattempt the write
                matter_fs_fwrite(fp, &log, log.LogLen, 0, 0, true, &numWrite);
            }
        }
        else 
        {
            ChipLogError(DeviceLayer, "Error writing log to file: %d", status);
        }
    }
    else 
    {
        ChipLogProgress(DeviceLayer, "fs wrote %d bytes\n", numWrite);
    }
    
    return false;                                                                       // always return false because we need other formatters to parse
}

/**
    Early log capture for driver or board init. see matter/api/matter_log_api.h for usage

    @param buf buffer containing the message
    @param bufSize unused
    @param err the CHIP_ERROR, see CHIPError.cpp for modification
*/
void AmebaLogRedirectHandler::AmebaEarlyError(char* buf, uint16_t bufSize, CHIP_ERROR err)
{
    // bypass all custom handling if the log subsystem was not Inited 
    if(AmebaLogRedirectHandler::GetAmebaLogSubsystemInited() == true) {
        AmebaErrorFormatter(buf, bufSize, err);
    }
    
    ChipLogError(DeviceLayer, "%s %" CHIP_ERROR_FORMAT, buf, err.Format());
    return;
}

/**
    Log callback to intercept and modify log / call other logging subsystem
    This method can also act as a "coarse" sieve to operate on the final string message

    @param module Short name of module as listed in lib/support/logging/TextOnlyLogging.cpp::ModuleNames. Can be custom
    @param category Log level, 0 to 4 (0 is to not print) based on enum lib/support/logging/Constants.h::LogCategory
    @param msg format string like printf
    @param args variable list to fill up the format string
*/
static void AmebaLogRedirect(const char* module, uint8_t category, const char* msg, va_list args)
{

    /*
    // print our own message
    int len;
    char * buf;

    len = strlen(module) + 1 + vsnprintf(NULL, 0, msg, args) + 1;
    buf = (char *) malloc(len);
    if (buf)
    {
        len = sprintf(buf, "%s ", module);
        vsprintf(buf + len, msg, args);
        printf("[REDIR] %s\n", buf);
        free(buf);
    }*/
    
    // debug: print a copy of the message
    chip::Logging::Platform::LogV(module, category, msg, args);
}

void AmebaLogRedirectHandler::RegisterAmebaLogRedirect(void) 
{
    if(this->bAmebaLogRedirected) return;

    printf("[AMEBA] Ameba Log Redirector registered: redirecting logs after this line\n");
    SetLogRedirectCallback(AmebaLogRedirect);
    this->bAmebaLogRedirected = true;
}

/* ========= ERROR FORMATTER ========== */

ErrorFormatter AmebaLogRedirectHandler::sAmebaErrorFormatter = { AmebaErrorFormatter, nullptr };

void AmebaLogRedirectHandler::RegisterAmebaErrorFormatter(void)
{
    if(this->bAmebaCustomFormatterSet) return;

    RegisterErrorFormatter(&sAmebaErrorFormatter);

    printf("[AMEBA] Ameba Error Formatter registered\n");
    this->bAmebaCustomFormatterSet = true;
}