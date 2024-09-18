#ifndef AMEBA_MATTER_API_LOG_H
#define AMEBA_MATTER_API_LOG_H

#pragma once

#include <platform_stdlib.h>
#include <platform_opts.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CHIP_LOG_DETAIL(MSG)    matter_chip_logging_wrapper(kDetail, MSG, __FILE__, __LINE__);
#define CHIP_LOG_PROGRESS(MSG)  matter_chip_logging_wrapper(kProgress, MSG, __FILE__, __LINE__);
#define CHIP_LOG_ERROR(MSG)     matter_chip_logging_wrapper(kError, MSG, __FILE__, __LINE__);

typedef enum MatterLogType
{
    kProgress,
    kDetail,
    kError,
} MatterLogType;

void matter_chip_logging_wrapper(MatterLogType logtype, char* msg, char* file, uint32_t line, ...);

#ifdef __cplusplus
}
#endif

#endif // AMEBA_MATTER_API_LOG_H