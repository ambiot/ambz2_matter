#pragma once

#include "matter_events.h"

#include <platform/CHIPDeviceLayer.h>
#include <app/clusters/identify-server/identify-server.h>

CHIP_ERROR matter_driver_tempsensor_init();
CHIP_ERROR matter_driver_tempsensor_set_startup_value();
CHIP_ERROR matter_driver_tempsensor_start();
void matter_driver_uplink_update_handler(AppEvent *aEvent);
void matter_driver_downlink_update_handler(AppEvent *event);
