#pragma once

#include "matter_events.h"
#include "gpio_api.h"
#include "gpio_irq_api.h"
#include "gpio_irq_ex_api.h"

#include <platform/CHIPDeviceLayer.h>
#include <app/clusters/identify-server/identify-server.h>

CHIP_ERROR matter_driver_refrigerator_init(void);
CHIP_ERROR matter_driver_refrigerator_set_startup_value(void);
void matter_driver_set_mode_callback(uint32_t id);
void matter_driver_set_door_callback(uint32_t id);
void matter_driver_uplink_update_handler(AppEvent * event);
void matter_driver_downlink_update_handler(AppEvent * event);
