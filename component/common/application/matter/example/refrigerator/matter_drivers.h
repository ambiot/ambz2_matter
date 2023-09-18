#pragma once

#include "matter_events.h"
// #include "gpio_irq_api.h"

#include <platform/CHIPDeviceLayer.h>
#include <app/clusters/identify-server/identify-server.h>

// void matter_driver_button_trigger_fall();
// void matter_driver_button_trigger_rise();
// void matter_driver_button_fall_callback(uint32_t id, gpio_irq_event event);
// void matter_driver_button_rise_callback(uint32_t id, gpio_irq_event event);
CHIP_ERROR matter_driver_refrigerator_init(void);
CHIP_ERROR matter_driver_refrigerator_set_startup_value(void);
void matter_driver_set_temperature_callback(uint32_t id);
void matter_driver_uplink_update_handler(AppEvent * event);
void matter_driver_downlink_update_handler(AppEvent * event);
