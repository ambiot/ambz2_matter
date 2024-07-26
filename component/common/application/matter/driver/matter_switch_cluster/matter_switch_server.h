#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/switch-server/switch-server.h>

#include <protocols/interaction_model/StatusCode.h>

namespace Ameba {
namespace Clusters {
namespace GenericSwitch {
namespace Event {

class GenericSwitchEventHandler;

struct SwitchEventData
{
    chip::EndpointId endpointId = 1;
    chip::ClusterId clusterId;
    chip::AttributeId attributeId;
    chip::CommandId commandId;
    chip::EventId eventId;
    uint8_t newPosition;
    uint8_t previousPosition;
    uint8_t count;
};

class GenericSwitchEventHandler
{
public:
    GenericSwitchEventHandler() = default;

    static void HandleCommand(intptr_t context);

    void TriggerCommand(SwitchEventData data);

    void OnSwitchLatchedHandler(chip::EndpointId endpoint, uint8_t newPosition);
    void OnSwitchInitialPressedHandler(chip::EndpointId endpoint, uint8_t newPosition);
    void OnSwitchLongPressedHandler(chip::EndpointId endpoint, uint8_t newPosition);
    void OnSwitchShortReleasedHandler(chip::EndpointId endpoint, uint8_t previousPosition);
    void OnSwitchLongReleasedHandler(chip::EndpointId endpoint, uint8_t previousPosition);
    void OnSwitchMultiPressOngoingHandler(chip::EndpointId endpoint, uint8_t newPosition, uint8_t count);
    void OnSwitchMultiPressCompleteHandler(chip::EndpointId endpoint, uint8_t previousPosition, uint8_t count);

    // Nested classes for switch events
    class SwitchLatch
    {
    public:
        explicit SwitchLatch(GenericSwitchEventHandler* handler);
        void Set(chip::EndpointId ep, uint8_t value);

    private:
        GenericSwitchEventHandler* mHandler;
    };

    class SwitchInitialPress
    {
    public:
        explicit SwitchInitialPress(GenericSwitchEventHandler* handler);
        void Set(chip::EndpointId ep, uint8_t value);

    private:
        GenericSwitchEventHandler* mHandler;
    };

    class SwitchLongPress
    {
    public:
        explicit SwitchLongPress(GenericSwitchEventHandler* handler);
        void Set(chip::EndpointId ep, uint8_t value);

    private:
        GenericSwitchEventHandler* mHandler;
    };

    class SwitchShortRelease
    {
    public:
        explicit SwitchShortRelease(GenericSwitchEventHandler* handler);
        void Set(chip::EndpointId ep, uint8_t value);

    private:
        GenericSwitchEventHandler* mHandler;
    };

    class SwitchLongRelease
    {
    public:
        explicit SwitchLongRelease(GenericSwitchEventHandler* handler);
        void Set(chip::EndpointId ep, uint8_t value);

    private:
        GenericSwitchEventHandler* mHandler;
    };

    class SwitchMultiPressOngoing
    {
    public:
        explicit SwitchMultiPressOngoing(GenericSwitchEventHandler* handler);
        void Set(chip::EndpointId ep, uint8_t value);

    private:
        GenericSwitchEventHandler* mHandler;
    };

    class SwitchMultiPressComplete
    {
    public:
        explicit SwitchMultiPressComplete(GenericSwitchEventHandler* handler);
        void Set(chip::EndpointId ep, uint8_t value);

    private:
        GenericSwitchEventHandler* mHandler;
    };

private:
    GenericSwitchEventHandler* mEventHandler = nullptr;
};

} // namespace Event
} // namespace GenericSwitch
} // namespace Clusters
} // namespace Ameba

