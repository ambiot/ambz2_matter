#ifndef PLATFORM_STDLIB_MATTER_H
#define PLATFORM_STDLIB_MATTER_H

#if defined(CONFIG_PLATFORM_8710C)
#include <assert.h>

extern size_t strnlen(const char *s, size_t count);
extern void *pvPortMalloc( size_t xWantedSize );

//def
#ifndef false
    #define false   0
#endif

#ifndef true
    #define true    1
#endif

#ifndef in_addr_t
    typedef __uint32_t in_addr_t;
#endif

#endif /* CONFIG_PLATFORM_XXXX */

#endif // PLATFORM_STDLIB_MATTER_H
