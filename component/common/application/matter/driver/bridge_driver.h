#pragma once

#include <platform_stdlib.h>
#include <functional>
#include <lib/core/DataModelTypes.h>
#include "matter_data_model.h"
#include "matter_data_model_presets.h"

// Device Type IDs
#define DEVICE_TYPE_ROOT_NODE 0x0016
#define DEVICE_TYPE_BRIDGE 0x000E
#define DEVICE_TYPE_LO_ON_OFF_LIGHT 0x0100
#define DEVICE_TYPE_BRIDGED_NODE 0x0013

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

#define BRIDGE_DEVICE_ADDRESS_LENGTH    20

// The bridge should also be a "provisioner" for the other non-matter network
class MatterBridge
{
public:
    struct EndpointInfo
    {
        uint8_t endpointId; // do we need to store this? can just create a new one on reboot
        char address[BRIDGE_DEVICE_ADDRESS_LENGTH];
    };
    void Init(Node& node);
    void addBridgedEndpoint(EndpointConfig bridgedConfig, Span<const EmberAfDeviceType> bridgedDeviceType);
    void removeBridgedEndpoint(chip::EndpointId endpointID);

private:
    Node* node;
    std::vector<EndpointInfo> endpointInfoList;
};
class MatterBridgeDevice
{
public:
    static const int kDeviceNameSize     = 32;
    static const int kDeviceLocationSize     = 32;

    enum Changed_t
    {
        kChanged_Reachable = 0x01,
        kChanged_State     = 0x02,
        kChanged_Location  = 0x04,
        kChanged_Name      = 0x08,
    } Changed;

    MatterBridgeDevice(const char * szDeviceName, const char * szLocation);
    virtual ~MatterBridgeDevice() {}

    bool IsReachable(void);
    void SetReachable(bool aReachable);
    void SetName(const char * szDeviceName);
    void SetLocation(const char * szLocation);
    inline void SetEndpointId(chip::EndpointId id) { mEndpointId = id; };
    inline chip::EndpointId GetEndpointId() { return mEndpointId; };
    inline char * GetName() { return mName; };
    inline char * GetLocation() { return mLocation; };

private:
    virtual void HandleDeviceChange(MatterBridgeDevice * device, MatterBridgeDevice::Changed_t changeMask) = 0;

protected:
    bool mReachable;
    char mName[kDeviceNameSize];
    char mLocation[kDeviceLocationSize];
    chip::EndpointId mEndpointId;
};

class MatterBridgedDeviceOnOff : public MatterBridgeDevice
{
public:
    enum Changed_t
    {
        kChanged_OnOff,
    } Changed;

    MatterBridgedDeviceOnOff(const char * szDeviceName, const char * szLocation);

    bool IsTurnedOn(void);
    void Set(bool state, int call_callback);

    using DeviceCallback_fn = std::function<void(MatterBridgedDeviceOnOff *, MatterBridgedDeviceOnOff::Changed_t)>;
    void SetChangeCallback(DeviceCallback_fn aChanged_CB);

private:
    void HandleDeviceChange(MatterBridgeDevice * device, MatterBridgeDevice::Changed_t changeMask);

private:
    bool mState;
    DeviceCallback_fn mChanged_CB;
};

