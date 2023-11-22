#pragma once

#include "matter_events.h"
#include <lwip/ip_addr.h>
#include <platform/CHIPDeviceLayer.h>

CHIP_ERROR matter_driver_bridge_light_init(void);
void matter_driver_uplink_update_handler(AppEvent * event);
void matter_driver_downlink_update_handler(AppEvent *aEvent);
void matter_customer_bridge_code(void *param);