#include "matter_switch_cluster/matter_switch_server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <app/util/att-storage.h>
#include <app/util/attribute-storage.h>
#include <platform/PlatformManager.h>

#include <utility> // For std::pair
#include <memory>  // For std::unique_ptr

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Switch;
using namespace chip::DeviceLayer;

using namespace Ameba::Clusters::GenericSwitch::Event;
using Ameba::Clusters::GenericSwitch::Event::SwitchEventData;

void GenericSwitchEventHandler::HandleCommand(intptr_t context)
{
    auto* pair = reinterpret_cast<std::pair<GenericSwitchEventHandler*, SwitchEventData*>*>(context);

    GenericSwitchEventHandler* handler = pair->first;
    SwitchEventData* data = pair->second;

    switch (data->eventId)
    {
        case Events::SwitchLatched::Id:
            handler->OnSwitchLatchedHandler(data->endpointId, data->newPosition);
            break;
        case Events::InitialPress::Id:
            handler->OnSwitchInitialPressedHandler(data->endpointId, data->newPosition);
            break;
        case Events::LongPress::Id:
            handler->OnSwitchLongPressedHandler(data->endpointId, data->newPosition);
            break;
        case Events::ShortRelease::Id:
            handler->OnSwitchShortReleasedHandler(data->endpointId, data->previousPosition);
            break;
        case Events::LongRelease::Id:
            handler->OnSwitchLongReleasedHandler(data->endpointId, data->previousPosition);
            break;
        case Events::MultiPressOngoing::Id:
            handler->OnSwitchMultiPressOngoingHandler(data->endpointId, data->newPosition, data->count);
            break;
        case Events::MultiPressComplete::Id:
            handler->OnSwitchMultiPressCompleteHandler(data->endpointId, data->previousPosition, data->count);
            break;
        default:
            ChipLogError(NotSpecified, "Unhandled event ID: %d", data->eventId);
            break;
    }

    Platform::Delete(data);
    delete pair;
}

void GenericSwitchEventHandler::OnSwitchLatchedHandler(EndpointId endpoint, uint8_t newPosition)
{
    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, newPosition);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The latching switch is moved to a new position:%d", newPosition);

    chip::app::Clusters::SwitchServer::Instance().OnSwitchLatch(endpoint, newPosition);
}

void GenericSwitchEventHandler::OnSwitchInitialPressedHandler(EndpointId endpoint, uint8_t newPosition)
{
    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, newPosition);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The new position when the momentary switch starts to be pressed:%d", newPosition);

    chip::app::Clusters::SwitchServer::Instance().OnInitialPress(endpoint, newPosition);
}

void GenericSwitchEventHandler::OnSwitchLongPressedHandler(EndpointId endpoint, uint8_t newPosition)
{
    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, newPosition);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The new position when the momentary switch has been pressed for a long time:%d", newPosition);

    chip::app::Clusters::SwitchServer::Instance().OnLongPress(endpoint, newPosition);
}

void GenericSwitchEventHandler::OnSwitchShortReleasedHandler(EndpointId endpoint, uint8_t previousPosition)
{
    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, 0);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to reset CurrentPosition attribute"));

    chip::app::Clusters::SwitchServer::Instance().OnShortRelease(endpoint, previousPosition);
}

void GenericSwitchEventHandler::OnSwitchLongReleasedHandler(EndpointId endpoint, uint8_t previousPosition)
{
    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, 0);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to reset CurrentPosition attribute"));

    chip::app::Clusters::SwitchServer::Instance().OnLongRelease(endpoint, previousPosition);
}

void GenericSwitchEventHandler::OnSwitchMultiPressOngoingHandler(EndpointId endpoint, uint8_t newPosition, uint8_t count)
{
    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, newPosition);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));

    chip::app::Clusters::SwitchServer::Instance().OnMultiPressOngoing(endpoint, newPosition, count);
}

void GenericSwitchEventHandler::OnSwitchMultiPressCompleteHandler(EndpointId endpoint, uint8_t previousPosition, uint8_t count)
{
    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, 0);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to reset CurrentPosition attribute"));

    chip::app::Clusters::SwitchServer::Instance().OnMultiPressComplete(endpoint, previousPosition, count);
}

void GenericSwitchEventHandler::TriggerCommand(SwitchEventData data)
{
    auto temp = std::make_unique<SwitchEventData>(data);
    auto pair = std::make_unique<std::pair<GenericSwitchEventHandler*, SwitchEventData*>>(this, temp.release());

    chip::DeviceLayer::PlatformMgr().ScheduleWork(GenericSwitchEventHandler::HandleCommand, reinterpret_cast<intptr_t>(pair.release()));
}

// SwitchLatch class methods

GenericSwitchEventHandler::SwitchLatch::SwitchLatch(GenericSwitchEventHandler* handler)
    : mHandler(handler) {}

void GenericSwitchEventHandler::SwitchLatch::Set(EndpointId ep, uint8_t value)
{
    ChipLogProgress(DeviceLayer, "SwitchLatch Set endpoint%d, value=%x", ep, value);
    SwitchEventData data;
    data.endpointId = ep;
    data.clusterId = chip::app::Clusters::Switch::Id;
    data.eventId = Events::SwitchLatched::Id;
    data.newPosition = value;

    mHandler->TriggerCommand(data);
}

// SwitchInitialPress class methods

GenericSwitchEventHandler::SwitchInitialPress::SwitchInitialPress(GenericSwitchEventHandler* handler)
    : mHandler(handler) {}

void GenericSwitchEventHandler::SwitchInitialPress::Set(EndpointId ep, uint8_t value)
{
    ChipLogProgress(DeviceLayer, "SwitchInitialPress Set endpoint%d, value=%x", ep, value);
    SwitchEventData data;
    data.endpointId = ep;
    data.clusterId = chip::app::Clusters::Switch::Id;
    data.eventId = Events::InitialPress::Id;
    data.newPosition = value;

    mHandler->TriggerCommand(data);
}

// SwitchLongPress class methods

GenericSwitchEventHandler::SwitchLongPress::SwitchLongPress(GenericSwitchEventHandler* handler)
    : mHandler(handler) {}

void GenericSwitchEventHandler::SwitchLongPress::Set(EndpointId ep, uint8_t value)
{
    ChipLogProgress(DeviceLayer, "SwitchLongPress Set endpoint%d, value=%x", ep, value);
    SwitchEventData data;
    data.endpointId = ep;
    data.clusterId = chip::app::Clusters::Switch::Id;
    data.eventId = Events::LongPress::Id;
    data.newPosition = value;

    mHandler->TriggerCommand(data);
}

// SwitchShortRelease class methods

GenericSwitchEventHandler::SwitchShortRelease::SwitchShortRelease(GenericSwitchEventHandler* handler)
    : mHandler(handler) {}

void GenericSwitchEventHandler::SwitchShortRelease::Set(EndpointId ep, uint8_t value)
{
    ChipLogProgress(DeviceLayer, "SwitchShortRelease Set endpoint%d, value=%x", ep, value);
    SwitchEventData data;
    data.endpointId = ep;
    data.clusterId = chip::app::Clusters::Switch::Id;
    data.eventId = Events::ShortRelease::Id;
    data.previousPosition = value;

    mHandler->TriggerCommand(data);
}

// SwitchLongRelease class methods

GenericSwitchEventHandler::SwitchLongRelease::SwitchLongRelease(GenericSwitchEventHandler* handler)
    : mHandler(handler) {}

void GenericSwitchEventHandler::SwitchLongRelease::Set(EndpointId ep, uint8_t value)
{
    ChipLogProgress(DeviceLayer, "SwitchLongRelease Set endpoint%d, value=%x", ep, value);
    SwitchEventData data;
    data.endpointId = ep;
    data.clusterId = chip::app::Clusters::Switch::Id;
    data.eventId = Events::LongRelease::Id;
    data.previousPosition = value;

    mHandler->TriggerCommand(data);
}

// SwitchMultiPressOngoing class methods

GenericSwitchEventHandler::SwitchMultiPressOngoing::SwitchMultiPressOngoing(GenericSwitchEventHandler* handler)
    : mHandler(handler) {}

void GenericSwitchEventHandler::SwitchMultiPressOngoing::Set(EndpointId ep, uint8_t value)
{
    ChipLogProgress(DeviceLayer, "SwitchMultiPressOngoing Set endpoint%d, value=%x", ep, value);
    SwitchEventData data;
    data.endpointId = ep;
    data.clusterId = chip::app::Clusters::Switch::Id;
    data.eventId = Events::MultiPressOngoing::Id;
    data.newPosition = value;

    mHandler->TriggerCommand(data);
}

// SwitchMultiPressComplete class methods

GenericSwitchEventHandler::SwitchMultiPressComplete::SwitchMultiPressComplete(GenericSwitchEventHandler* handler)
    : mHandler(handler) {}

void GenericSwitchEventHandler::SwitchMultiPressComplete::Set(EndpointId ep, uint8_t value)
{
    ChipLogProgress(DeviceLayer, "SwitchMultiPressComplete Set endpoint%d, value=%x", ep, value);
    SwitchEventData data;
    data.endpointId = ep;
    data.clusterId = chip::app::Clusters::Switch::Id;
    data.eventId = Events::MultiPressComplete::Id;
    data.newPosition = value;

    mHandler->TriggerCommand(data);
}
