#pragma once

#include "matter_events.h"

#include <platform/CHIPDeviceLayer.h>
CHIP_ERROR matter_driver_button_init(void);
CHIP_ERROR matter_driver_led_init(void);
CHIP_ERROR matter_driver_led_toggle(void);
bool matter_driver_led_get_onoff(void);
uint8_t matter_driver_led_get_level(void);
CHIP_ERROR matter_driver_led_set_startup_value(void);
void matter_driver_attribute_update(AppEvent *aEvent);
