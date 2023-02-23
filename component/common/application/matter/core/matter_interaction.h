#pragma once

#include <stdint.h>
#include "matter_events.h"
#include <platform/CHIPDeviceLayer.h>

void PostDownlinkEvent(const AppEvent * aEvent);
CHIP_ERROR matter_interaction_start_downlink(void);
CHIP_ERROR matter_interaction_start_uplink(void);
// void matter_interaction_onoff_handler(AppEvent *aEvent);
