#pragma once

#include "matter_events.h"
#include <app/clusters/identify-server/identify-server.h>
#include <platform/CHIPDeviceLayer.h>

CHIP_ERROR matter_driver_thermostat_init(void);
CHIP_ERROR matter_driver_thermostat_ui_init(void);
CHIP_ERROR matter_driver_thermostat_ui_set_startup_value(void);
void matter_driver_on_identify_start(Identify * identify);
void matter_driver_on_identify_stop(Identify * identify);
void matter_driver_on_trigger_effect(Identify * identify);
void matter_driver_uplink_update_handler(AppEvent *aEvent);
