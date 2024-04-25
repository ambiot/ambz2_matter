/******************************************************************************
  *
  * This module is a confidential and proprietary property of RealTek and
  * possession or use of this module requires written permission of RealTek.
  *
  * Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved. 
  *
******************************************************************************/

#ifndef LWIP_HDR_LWIPOPTS_MATTER_H
#define LWIP_HDR_LWIPOPTS_MATTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <platform/platform_stdlib.h>
#include "platform_opts.h"

#undef LWIP_TCPIP_CORE_LOCKING
#undef LWIP_COMPAT_MUTEX_ALLOWED
#undef LWIP_IPV6_ND
#undef LWIP_IPV6_SCOPES
#undef LWIP_PBUF_FROM_CUSTOM_POOLS
#define LWIP_TCPIP_CORE_LOCKING         1
#define LWIP_COMPAT_MUTEX_ALLOWED       1
#define LWIP_IPV6_ND                    1
#define LWIP_IPV6_SCOPES                0
#define LWIP_PBUF_FROM_CUSTOM_POOLS     0

#undef LWIP_IPV6
#define LWIP_IPV6                       1
#if LWIP_IPV6
#undef LWIP_IPV6_MLD
#undef LWIP_IPV6_AUTOCONFIG
#undef LWIP_ICMP6
#undef LWIP_IPV6_DHCP6
#undef MEMP_NUM_MLD6_GROUP
#define LWIP_IPV6_MLD                   1
#define LWIP_IPV6_AUTOCONFIG            1
#define LWIP_ICMP6                      1
#define LWIP_IPV6_DHCP6                 1
#define MEMP_NUM_MLD6_GROUP             6
#endif /* LWIP_IPV6 */

/* ---------- Hook options --------- */
#define LWIP_HOOK_FILENAME              "lwip_default_hooks.h"

#undef PBUF_POOL_SIZE
#undef PBUF_POOL_BUFSIZE
#undef MEMP_NUM_UDP_PCB
#undef MEMP_NUM_SYS_TIMEOUT
#define PBUF_POOL_SIZE                  40
#define PBUF_POOL_BUFSIZE               1500
#define MEMP_NUM_UDP_PCB                10
#define MEMP_NUM_SYS_TIMEOUT            14

#ifdef __cplusplus
}
#endif

#endif /* LWIP_HDR_LWIPOPTS_MATTER_H */
