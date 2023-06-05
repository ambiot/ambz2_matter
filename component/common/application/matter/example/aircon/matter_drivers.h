#pragma once

#include "matter_events.h"

#include <platform/CHIPDeviceLayer.h>
#include <app/clusters/identify-server/identify-server.h>

// CHIP_ERROR matter_driver_button_init(void);
CHIP_ERROR matter_driver_fan_init(void);
CHIP_ERROR matter_driver_fan_set_startup_value(void);
void matter_driver_on_identify_start(Identify * identify);
void matter_driver_on_identify_stop(Identify * identify);
void matter_driver_on_trigger_effect(Identify * identify);
void matter_driver_uplink_update_handler(AppEvent * event);
// void matter_driver_downlink_update_handler(AppEvent *aEvent);
