#pragma once

#include "matter_events.h"

#include <platform/CHIPDeviceLayer.h>
CHIP_ERROR matter_driver_thermostat_init(void);
CHIP_ERROR matter_driver_thermostat_ui_init(void);
CHIP_ERROR matter_driver_thermostat_ui_set_startup_value(void);
void matter_driver_attribute_update(AppEvent *aEvent);
