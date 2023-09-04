#pragma once

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
        void Init(Node node);
        void addBridgedEndpoint();
        void removeBridgedEndpoint();

    private:
        Node node;
        std::vector<EndpointInfo> endpointInfoList;
};

class MatterBridgedDevice
{
    public:
        void bridgeAttributeChangeCallback(void *data);
        void setCallbackFunction(/* callback function pointer */);

    private:
        // callback function pointer to translate matter attribute changes to non-matter protocols

};
