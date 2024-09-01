#include <platform_stdlib.h>
#include <platform_opts.h>
#include <string>
#if defined(CONFIG_ENABLE_AMEBA_DLOG) && (CONFIG_ENABLE_AMEBA_DLOG == 1)
#include <matter_fs.h>
#include <matter_log_api.h>

#include <diagnostic_logs/ameba_logging_redirect_handler.h>

#include <core/ErrorStr.h>
#include <lib/support/logging/TextOnlyLogging.h>

using namespace std;
using CHIP_ERROR = ::chip::ChipError;
using Range = ::chip::ChipError::Range;

auto & instance = AmebaLogRedirectHandler::GetInstance(); 

#ifdef __cplusplus
extern "C" {
#endif

/**
* C wrapper for chip loggings; Can be called from the driver layer to unify logging
*
* @param logtype type of log. Either Progress or Detail or Error like ChipLogError
* @param content message to print to console
* @param file result of __FILE__ macro
* @param line result of __LINE__ macro
* @param vararg currently unimplemented
*/
void matter_chip_logging_wrapper(MatterLogType logtype, char* content, char* file, uint32_t line, ...)
{
    CHIP_ERROR err;
    string message = "[AMBDRV] ";
    message += content;

    switch (logtype)
    {
    case kProgress:
        ChipLogProgress(DeviceLayer, message.c_str());
        break;
    case kDetail:
        ChipLogDetail(DeviceLayer, message.c_str());
        break;
    case kError:
        err = CHIP_ERROR(Range::kPlatform, 0, file, line);
        if(instance.GetAmebaErrorFormatterInited() == true) 
        {
            ChipLogError(DeviceLayer, (message + " %" CHIP_ERROR_FORMAT).c_str(), err.Format());
        }
        else 
        {
            // capture early error logging before CHIP stack is inited
            instance.AmebaEarlyError((char*)(message + " %" CHIP_ERROR_FORMAT).c_str(), 0, err);
        }
        break;
    default:
        break;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_ENABLE_AMEBA_DLOG */
