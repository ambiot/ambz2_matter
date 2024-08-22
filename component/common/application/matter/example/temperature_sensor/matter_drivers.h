#pragma once

#include <matter_events.h>

#include <platform/CHIPDeviceLayer.h>
#include <app/clusters/identify-server/identify-server.h>

void matter_driver_on_identify_start(Identify *identify);
void matter_driver_on_identify_stop(Identify *identify);
void matter_driver_on_trigger_effect(Identify *identify);

CHIP_ERROR matter_driver_temperature_sensor_init(void);
void matter_driver_uplink_update_handler(AppEvent *aEvent);
void matter_driver_downlink_update_handler(AppEvent *aEvent);
