#pragma once

#include <platform_stdlib.h>
#include <platform_opts.h>
#include <arch/cc.h>
#include <string>

#include <core/ErrorStr.h>
#include <platform/logging/LogV.h>
#include <support/logging/TextOnlyLogging.h>

using namespace chip;

typedef struct __attribute__((__packed__)) AmebaLog {
    uint8_t     LogLen;
    uint32_t    Timestamp;
    uint8_t     ErrorRange : 3;
    uint8_t     ErrorPart : 5;
    uint16_t    ErrorValue;
#if defined(CONFIG_ENABLE_AMEBA_SHORT_LOGGING) && (CONFIG_ENABLE_AMEBA_SHORT_LOGGING == 0)
    uint16_t    ErrorLineNo;
    uint8_t     ErrorFilename[CONFIG_AMEBA_LOG_FILENAME_MAXSZ];
#endif
} amebalog_t;

class AmebaLogRedirectHandler
{
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
