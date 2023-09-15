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
        kEventType_Downlink_Opstate_State,
        kEventType_Downlink_Opstate_Error_State,
        kEventType_Downlink_LW_SpinSpeed,
        kEventType_Downlink_LW_NumberOfRinses,
        kEventType_Downlink_LW_Mode,
        kEventType_Downlink_Refrigerator_Alarm_SetMaskValue,
        kEventType_Downlink_Refrigerator_Alarm_SetStateValue,
        kEventType_Downlink_Refrigerator_Alarm_SetSupportedValue
    };

    uint16_t Type;
    chip::app::ConcreteAttributePath path;
    union
    {
       uint8_t _u8;
       uint16_t _u16;
       uint32_t _u32;
       uint64_t _u64;
       char _str[256];
    } value;
    EventHandler mHandler;
};
