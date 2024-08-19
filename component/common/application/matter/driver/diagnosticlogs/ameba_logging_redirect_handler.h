#pragma once

#include <platform_stdlib.h>
#include <string>
#include "arch/cc.h"
#include "platform_opts_matter.h"
#include "chip_porting.h"
#include <core/ErrorStr.h>
#include <support/logging/TextOnlyLogging.h>
#include <platform/logging/LogV.h>

using namespace chip;

typedef struct __attribute__((__packed__)) AmebaLog {
    uint8_t     LogLen;            // 0x1
    uint32_t    Timestamp;         // 0x4
    uint8_t     ErrorRange : 3;     
    uint8_t     ErrorPart : 5;     // 0x1
    uint16_t    ErrorValue;        // 0x2
#if defined(CONFIG_AMEBA_MATTER_SHORT_LOG_FMT) && (CONFIG_AMEBA_MATTER_SHORT_LOG_FMT == 0)
    uint16_t    ErrorLineNo;       // 0x2
    uint8_t     ErrorFilename[CONFIG_AMEBA_MATTER_LOG_FILENAME_MAXSZ]; // 0x[ErrorFilenameSz]
#endif
} amebalog_t;

class AmebaLogRedirectHandler {
    public:
        static AmebaLogRedirectHandler & GetInstance() { 
            static AmebaLogRedirectHandler instance;
            return instance; 
        }

        void RegisterAmebaLogRedirect(void);
        void RegisterAmebaErrorFormatter(void);

        void DeregisterAmebaLogRedirect(void);
        void DeregisterAmebaErrorFormatter(void);

        static inline void InitAmebaLogSubsystem(void) 
        {
            bAmebaUseLogSubsystem = true;
        }

        static inline bool GetAmebaLogSubsystemInited(void)
        {
            return bAmebaUseLogSubsystem;
        }

        static inline bool GetAmebaLogRedirectInited(void) 
        {
            return bAmebaLogRedirected;
        }

        static inline bool GetAmebaErrorFormatterInited(void) 
        {
            return bAmebaCustomFormatterSet;
        }

        void AmebaEarlyError(char* buf, uint16_t bufSize, CHIP_ERROR err);
        static inline uint32_t cxxcount = 0;

    private:
        AmebaLogRedirectHandler();
        ~AmebaLogRedirectHandler();

        AmebaLogRedirectHandler(const AmebaLogRedirectHandler &)             = delete;
        AmebaLogRedirectHandler & operator=(const AmebaLogRedirectHandler &) = delete;

        static ErrorFormatter sAmebaErrorFormatter;

        static inline bool bAmebaLogRedirected = false;
        static inline bool bAmebaCustomFormatterSet = false;
        static inline bool bAmebaUseLogSubsystem = false;
};