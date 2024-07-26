#include "chip_porting.h"
#include <lib/support/logging/TextOnlyLogging.h>
#include <core/ErrorStr.h>
#include "matter_log_api.h"
#include <string>
#include "ameba_logging_redirect_handler.h"
#include "platform_opts_matter.h"

#if defined(CONFIG_AMEBA_MATTER_ERROR_FORMATTER) && (CONFIG_AMEBA_MATTER_ERROR_FORMATTER == 1)

using namespace std;
using CHIP_ERROR = ::chip::ChipError;
using Range = ::chip::ChipError::Range;

auto & instance = AmebaLogRedirectHandler::GetInstance(); 

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
* C wrapper for ChipLog* functions, allow to be called from driver code to unify logging
*
* @param logtype type of log. Either Progress or Detail or Error like ChipLogError
* @param content message to print to console
* @param file result of __FILE__ macro
* @param line result of __LINE__ macro
* @param vararg currently unimplemented
*/
void matter_c_intf_chip_logging(MatterLogType logtype, char* content, char* file, uint32_t line, ...)
{
    CHIP_ERROR err;
    string message = "[AMBDRV] ";
    message += content;

    switch(logtype)
    {
        case kProgress:
            ChipLogProgress(DeviceLayer, message.c_str());
            return;
        case kDetail:
            ChipLogDetail(DeviceLayer, message.c_str());
            return;
        case kError:
            err = CHIP_ERROR(Range::kPlatform, 0, file, line);
            if(instance.GetAmebaErrorFormatterInited() == true) 
            {
                ChipLogError(DeviceLayer, (message + " %" CHIP_ERROR_FORMAT).c_str(), err.Format());
            }
            else 
            {
                // for early error logging before CHIP stack is inited
                instance.AmebaEarlyError((char*)(message + " %" CHIP_ERROR_FORMAT).c_str(), 0, err);
            }
            return;
        default:
            return;
    }
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CONFIG_AMEBA_MATTER_ERROR_FORMATTER