# -------------------------------------------------------------------
# CHIP options
# -------------------------------------------------------------------
CFLAGS += -DCHIP_PROJECT=1

# for chip device options
CFLAGS += -DCHIP_DEVICE_LAYER_TARGET=Ameba
CFLAGS += -DCHIP_DEVICE_LAYER_NONE=0

# for chip System options
CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF=0
CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS=0
CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS=0
CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_LWIP=1
CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_SOCKETS=0
CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK=0
CFLAGS += -DCHIP_SYSTEM_CONFIG_POSIX_LOCKING=0

# for chip IPv4 
CFLAGS += -DINET_CONFIG_ENABLE_IPV4=0

# LwIP options
CFLAGS += -DLWIP_IPV6_ND=1
CFLAGS += -DLWIP_IPV6_SCOPES=0
CFLAGS += -DLWIP_PBUF_FROM_CUSTOM_POOLS=0
CFLAGS += -DLWIP_IPV6_ROUTE_TABLE_SUPPORT=1

# Mbedtls options
CFLAGS += -DMBEDTLS_CONFIG_FILE=\"mbedtls_config.h\"

# -------------------------------------------------------------------
# Ameba Matter options
# -------------------------------------------------------------------
CFLAGS += -DCONFIG_MATTER=1
CFLAGS += -DCONFIG_BT=1

# for matter factory data
CFLAGS += -DCONFIG_ENABLE_AMEBA_FACTORY_DATA=0

# for matter additional secure feature
CFLAGS += -DCONFIG_ENABLE_AMEBA_CRYPTO=0

# for matter TestEvent Trigger EnableKey
CFLAGS += -DCONFIG_ENABLE_AMEBA_TEST_EVENT_TRIGGER=0

# for matter blemgr adapter
#CFLAGS += -DCONFIG_MATTER_BLEMGR_ADAPTER=1
