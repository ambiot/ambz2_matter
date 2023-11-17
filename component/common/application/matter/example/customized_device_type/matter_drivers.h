#pragma once

#include "matter_events.h"
#include "gpio_api.h"
#include "gpio_irq_api.h"
#include "gpio_irq_ex_api.h"

#include <platform/CHIPDeviceLayer.h>
#include <app/clusters/identify-server/identify-server.h>

CHIP_ERROR matter_driver_fan_init(void);
CHIP_ERROR matter_driver_fan_set_startup_value(void);
CHIP_ERROR matter_driver_temphumsensor_init();
CHIP_ERROR matter_driver_temphumsensor_start();
CHIP_ERROR matter_driver_laundry_washer_init();
CHIP_ERROR matter_driver_laundry_washer_set_startup_value();
CHIP_ERROR matter_driver_refrigerator_init(void);
CHIP_ERROR matter_driver_refrigerator_set_startup_value(void);
void matter_driver_set_mode_callback(uint32_t id);
void matter_driver_set_door_callback(uint32_t id);
void matter_driver_on_identify_start(Identify * identify);
void matter_driver_on_identify_stop(Identify * identify);
void matter_driver_on_trigger_effect(Identify * identify);
void matter_driver_uplink_update_handler(AppEvent * event);
void matter_driver_downlink_update_handler(AppEvent *event);
