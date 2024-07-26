#ifndef AMEBA_MATTER_API_LOG_H
#define AMEBA_MATTER_API_LOG_H

#pragma once

#include <platform_stdlib.h>
#include "platform_opts_matter.h"

#if defined(CONFIG_AMEBA_MATTER_ERROR_FORMATTER) && (CONFIG_AMEBA_MATTER_ERROR_FORMATTER == 1)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define CHIP_LOG_DETAIL(MSG)    matter_c_intf_chip_logging(kDetail, MSG, __FILE__, __LINE__);
#define CHIP_LOG_PROGRESS(MSG)  matter_c_intf_chip_logging(kProgress, MSG, __FILE__, __LINE__);
#define CHIP_LOG_ERROR(MSG)     matter_c_intf_chip_logging(kError, MSG, __FILE__, __LINE__);

typedef enum MatterLogType {
    kProgress,
    kDetail,
    kError,
} MatterLogType;

void matter_c_intf_chip_logging(MatterLogType logtype, char* msg, char* file, uint32_t line, ...);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CONFIG_AMEBA_MATTER_ERROR_FORMATTER

#endif // AMEBA_MATTER_API_LOG_H