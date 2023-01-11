#pragma once

#include <platform/CHIPDeviceLayer.h>

struct AppEvent;
typedef void (*EventHandler)(AppEvent *);

struct AppEvent
{
    enum AppEventTypes
    {
        kEventType_Uplink = 0,
        kEventType_Downlink_OnOff,
        kEventType_Downlink_Identify,
    };

    uint16_t Type;
    chip::app::ConcreteAttributePath path;
    uint8_t value;
    EventHandler mHandler;
};
