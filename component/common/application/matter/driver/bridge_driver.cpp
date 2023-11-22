#include <cJSON.h>
#include "bridge_driver.h"
#include <support/logging/CHIPLogging.h>
#include <algorithm>

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

EmberAfDeviceType gRootNodeDeviceTypes[] = {
    { DEVICE_TYPE_ROOT_NODE, DEVICE_VERSION_DEFAULT },
};

EmberAfDeviceType gAggregatorDeviceTypes[] = {
    { DEVICE_TYPE_BRIDGE, DEVICE_VERSION_DEFAULT },
};

void MatterBridge::Init(Node& mNode)
{
    // start polling task to poll for messages from bridged device

    if ( &mNode == NULL ) {
        ChipLogError(DeviceLayer, "Node is null");
        return;
    }

    node = &mNode;
    Endpoint *ep0, *ep1;

    ep0 = node->getEndpoint(0);
    ep1 = node->getEndpoint(1);

    if (ep0 != NULL) {
        ep0->disableEndpoint();
        node->removeEndpoint(0);
    }

    if (ep1 != NULL) {
        ep1->disableEndpoint();
        node->removeEndpoint(1);
    }

    EndpointConfig rootNodeEndpointConfig;
    EndpointConfig aggregatorEndpointConfig;

    Presets::Endpoints::matter_root_node_preset(&rootNodeEndpointConfig);
    Presets::Endpoints::matter_aggregator_preset(&aggregatorEndpointConfig);

    // Initialization for Bridge: Root Node on ep0 and Aggregator on ep1
    node->addEndpoint(rootNodeEndpointConfig, Span<const EmberAfDeviceType>(gRootNodeDeviceTypes));
    node->addEndpoint(aggregatorEndpointConfig, Span<const EmberAfDeviceType>(gAggregatorDeviceTypes));

    // Enable endpoints
    node->enableAllEndpoints();
}

void MatterBridge::addBridgedEndpoint(EndpointConfig bridgedConfig, Span<const EmberAfDeviceType> bridgedDeviceType)
{
    node->addEndpoint(bridgedConfig, bridgedDeviceType);
    node->enableAllEndpoints();
}

void MatterBridge::removeBridgedEndpoint(chip::EndpointId endpointID)
{
    Endpoint *ep = node->getEndpoint(endpointID);
    ep->disableEndpoint();
    node->removeEndpoint(endpointID);
}

MatterBridgeDevice::MatterBridgeDevice(const char * szDeviceName, const char * szLocation)
{
    memcpy(mName, szDeviceName, sizeof(mName));
    memcpy(mLocation, szLocation, sizeof(mLocation));
    ChipLogProgress(DeviceLayer, "Setting up Device[%s] at %s", mName, mLocation);

    mReachable  = false;
    mEndpointId = 0;
}

bool MatterBridgeDevice::IsReachable()
{
    return this->mReachable;
}

void MatterBridgeDevice::SetReachable(bool aReachable)
{
    bool changed = (mReachable != aReachable);

    if (mReachable == aReachable)
        return;

    mReachable = aReachable;

    if (mReachable)
    {
        ChipLogProgress(DeviceLayer, "Device[%s]: ONLINE", mName);
    } 
    else
    {
        ChipLogProgress(DeviceLayer, "Device[%s]: OFFLINE", mName);
    }

    if (changed)
    {
        HandleDeviceChange(this, kChanged_Reachable);
    }
}

void MatterBridgeDevice::SetName(const char * szDeviceName)
{
    bool changed = (strncmp(mName, szDeviceName, sizeof(mName)) != 0);

    ChipLogProgress(DeviceLayer, "Device[%s]: New Name=\"%s\"", mName, szDeviceName);

    memcpy(mName, szDeviceName, sizeof(mName));

    if (changed)
    {
        HandleDeviceChange(this, kChanged_Name);
    }
}

void MatterBridgeDevice::SetLocation(const char * szLocation)
{
    bool changed = (strncmp(mLocation, szLocation, sizeof(mLocation)) != 0);

    memcpy(mLocation, szLocation, sizeof(mLocation));

    ChipLogProgress(DeviceLayer, "Device[%s]: Location=\"%s\"", mName, mLocation);

    if (changed)
    {
        HandleDeviceChange(this, kChanged_Location);
    }
}

/***** Device Type: ONOFF *****/

MatterBridgedDeviceOnOff::MatterBridgedDeviceOnOff(const char * szDeviceName, const char * szLocation) : MatterBridgeDevice(szDeviceName, szLocation)
{
    mState = false;
}

bool MatterBridgedDeviceOnOff::IsTurnedOn()
{
    return this->mState;
}

void MatterBridgedDeviceOnOff::Set(bool state, int call_callback)
{
    bool changed;

    changed = (mState != state);
    mState = state;

    ChipLogProgress(DeviceLayer, "Device[%s]: %s", mName, state ? "ON" : "OFF");

    if ((changed) && (mChanged_CB && call_callback))
    {
        mChanged_CB(this, kChanged_OnOff);
    }
}

void MatterBridgedDeviceOnOff::SetChangeCallback(DeviceCallback_fn aChanged_CB)
{
    mChanged_CB = aChanged_CB;
}

void MatterBridgedDeviceOnOff::HandleDeviceChange(MatterBridgeDevice * device, MatterBridgeDevice::Changed_t changeMask)
{
    if (mChanged_CB)
    {
        mChanged_CB(this, (MatterBridgedDeviceOnOff::Changed_t) changeMask);
    }
}
