#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <app/util/attribute-storage.h>
#include "matter_data_model.h"
#include <platform/Ameba/AmebaUtils.h>

using namespace ::chip;
using chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr;
using namespace chip::DeviceLayer::Internal;

/* emberAfExternalAttributeRead/WriteCallback are required for externally stored attributes */
EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint_id, ClusterId cluster_id,
                                                   const EmberAfAttributeMetadata *matter_attribute, uint8_t *buffer,
                                                   uint16_t max_read_length)
{
    Node & node = Node::getInstance();
    Endpoint *endpoint = node.getEndpoint(endpoint_id);
    Cluster *cluster = endpoint->getCluster(cluster_id);
    Attribute *attribute = cluster->getAttribute(matter_attribute->attributeId);

    if (attribute->getAttributeSize() > max_read_length)
    {
        ChipLogError(DeviceLayer, "[%s] Insufficient space to read Attribute 0x%08x from Cluster 0x%08x in Endpoint 0x%04x", __FUNCTION__, matter_attribute->attributeId, cluster_id, endpoint_id);
        return EMBER_ZCL_STATUS_RESOURCE_EXHAUSTED;
    }

    attribute->getValue(buffer);

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint_id, ClusterId cluster_id, const EmberAfAttributeMetadata *matter_attribute, uint8_t *buffer)
{
    Node & node = Node::getInstance();
    Endpoint *endpoint = node.getEndpoint(endpoint_id);
    Cluster *cluster = endpoint->getCluster(cluster_id);
    Attribute *attribute = cluster->getAttribute(matter_attribute->attributeId);

    attribute->setValue(buffer);

    return EMBER_ZCL_STATUS_SUCCESS;
}

/*                  Attributes                  */
chip::AttributeId Attribute::getAttributeId() const
{
    return attributeId;
}

chip::ClusterId Attribute::getParentClusterId() const
{
    return parentClusterId;
}

chip::ClusterId Attribute::getParentEndpointId() const
{
    return parentEndpointId;
}

std::uint16_t Attribute::getAttributeSize() const
{
    return attributeSize;
}

std::uint8_t Attribute::getAttributeType() const
{
    return attributeType;
}

std::uint8_t Attribute::getAttributeMask() const
{
    return attributeMask;
}

EmberAfDefaultOrMinMaxAttributeValue Attribute::getAttributeDefaultValue() const
{
    return defaultValue;
}

EmberAfAttributeType Attribute::getAttributeBaseType() const
{
    switch (attributeType)
    {
    case ZCL_ACTION_ID_ATTRIBUTE_TYPE:  // Action Id
    case ZCL_FABRIC_IDX_ATTRIBUTE_TYPE: // Fabric Index
    case ZCL_BITMAP8_ATTRIBUTE_TYPE:    // 8-bit bitmap
    case ZCL_ENUM8_ATTRIBUTE_TYPE:      // 8-bit enumeration
    case ZCL_STATUS_ATTRIBUTE_TYPE:     // Status Code
    case ZCL_PERCENT_ATTRIBUTE_TYPE:    // Percentage
        static_assert(std::is_same<chip::Percent, uint8_t>::value,
                      "chip::Percent is expected to be uint8_t, change this when necessary");
        return ZCL_INT8U_ATTRIBUTE_TYPE;

    case ZCL_ENDPOINT_NO_ATTRIBUTE_TYPE:   // Endpoint Number
    case ZCL_GROUP_ID_ATTRIBUTE_TYPE:      // Group Id
    case ZCL_VENDOR_ID_ATTRIBUTE_TYPE:     // Vendor Id
    case ZCL_ENUM16_ATTRIBUTE_TYPE:        // 16-bit enumeration
    case ZCL_BITMAP16_ATTRIBUTE_TYPE:      // 16-bit bitmap
    case ZCL_PERCENT100THS_ATTRIBUTE_TYPE: // 100ths of a percent
        static_assert(std::is_same<chip::EndpointId, uint16_t>::value,
                      "chip::EndpointId is expected to be uint16_t, change this when necessary");
        static_assert(std::is_same<chip::GroupId, uint16_t>::value,
                      "chip::GroupId is expected to be uint16_t, change this when necessary");
        static_assert(std::is_same<chip::Percent100ths, uint16_t>::value,
                      "chip::Percent100ths is expected to be uint16_t, change this when necessary");
        return ZCL_INT16U_ATTRIBUTE_TYPE;

    case ZCL_CLUSTER_ID_ATTRIBUTE_TYPE: // Cluster Id
    case ZCL_ATTRIB_ID_ATTRIBUTE_TYPE:  // Attribute Id
    case ZCL_FIELD_ID_ATTRIBUTE_TYPE:   // Field Id
    case ZCL_EVENT_ID_ATTRIBUTE_TYPE:   // Event Id
    case ZCL_COMMAND_ID_ATTRIBUTE_TYPE: // Command Id
    case ZCL_TRANS_ID_ATTRIBUTE_TYPE:   // Transaction Id
    case ZCL_DEVTYPE_ID_ATTRIBUTE_TYPE: // Device Type Id
    case ZCL_DATA_VER_ATTRIBUTE_TYPE:   // Data Version
    case ZCL_BITMAP32_ATTRIBUTE_TYPE:   // 32-bit bitmap
    case ZCL_EPOCH_S_ATTRIBUTE_TYPE:    // Epoch Seconds
    case ZCL_ELAPSED_S_ATTRIBUTE_TYPE:  // Elapsed Seconds
        static_assert(std::is_same<chip::ClusterId, uint32_t>::value,
                      "chip::Cluster is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::AttributeId, uint32_t>::value,
                      "chip::AttributeId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::AttributeId, uint32_t>::value,
                      "chip::AttributeId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::EventId, uint32_t>::value,
                      "chip::EventId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::CommandId, uint32_t>::value,
                      "chip::CommandId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::TransactionId, uint32_t>::value,
                      "chip::TransactionId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::DeviceTypeId, uint32_t>::value,
                      "chip::DeviceTypeId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::DataVersion, uint32_t>::value,
                      "chip::DataVersion is expected to be uint32_t, change this when necessary");
        return ZCL_INT32U_ATTRIBUTE_TYPE;

    case ZCL_EVENT_NO_ATTRIBUTE_TYPE:   // Event Number
    case ZCL_FABRIC_ID_ATTRIBUTE_TYPE:  // Fabric Id
    case ZCL_NODE_ID_ATTRIBUTE_TYPE:    // Node Id
    case ZCL_BITMAP64_ATTRIBUTE_TYPE:   // 64-bit bitmap
    case ZCL_EPOCH_US_ATTRIBUTE_TYPE:   // Epoch Microseconds
    case ZCL_POSIX_MS_ATTRIBUTE_TYPE:   // POSIX Milliseconds
    case ZCL_SYSTIME_MS_ATTRIBUTE_TYPE: // System time Milliseconds
    case ZCL_SYSTIME_US_ATTRIBUTE_TYPE: // System time Microseconds
        static_assert(std::is_same<chip::EventNumber, uint64_t>::value,
                      "chip::EventNumber is expected to be uint64_t, change this when necessary");
        static_assert(std::is_same<chip::FabricId, uint64_t>::value,
                      "chip::FabricId is expected to be uint64_t, change this when necessary");
        static_assert(std::is_same<chip::NodeId, uint64_t>::value,
                      "chip::NodeId is expected to be uint64_t, change this when necessary");
        return ZCL_INT64U_ATTRIBUTE_TYPE;

    case ZCL_TEMPERATURE_ATTRIBUTE_TYPE: // Temperature
        return ZCL_INT16S_ATTRIBUTE_TYPE;

    default:
        return attributeType;
    }
}

void Attribute::getValue(uint8_t *buffer) const
{
    switch(getAttributeBaseType())
    {
    case ZCL_INT8U_ATTRIBUTE_TYPE:
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE:
        memcpy(buffer, &(std::get<uint8_t>(value)), sizeof(uint8_t));
        break;
    case ZCL_INT16U_ATTRIBUTE_TYPE:
        memcpy(buffer, &(std::get<uint16_t>(value)), sizeof(uint16_t));
        break;
    case ZCL_INT32U_ATTRIBUTE_TYPE:
        memcpy(buffer, &(std::get<uint32_t>(value)), sizeof(uint32_t));
        break;
    case ZCL_INT64U_ATTRIBUTE_TYPE:
        memcpy(buffer, &(std::get<uint64_t>(value)), sizeof(uint64_t));
        break;
    case ZCL_INT8S_ATTRIBUTE_TYPE:
        memcpy(buffer, &(std::get<int8_t>(value)), sizeof(int8_t));
        break;
    case ZCL_INT16S_ATTRIBUTE_TYPE:
        memcpy(buffer, &(std::get<int16_t>(value)), sizeof(int16_t));
        break;
    case ZCL_INT32S_ATTRIBUTE_TYPE:
        memcpy(buffer, &(std::get<int32_t>(value)), sizeof(int32_t));
        break;
    case ZCL_INT64S_ATTRIBUTE_TYPE:
        memcpy(buffer, &(std::get<int64_t>(value)), sizeof(int64_t));
        break;
    case ZCL_SINGLE_ATTRIBUTE_TYPE:
        memcpy(buffer, &(std::get<float>(value)), sizeof(float));
        break;
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
        memcpy(buffer, valueBuffer, attributeSize);
        break;
    case ZCL_ARRAY_ATTRIBUTE_TYPE:
    case ZCL_STRUCT_ATTRIBUTE_TYPE:
        // Do nothing here, these types of attributes won't be handled here
        break;
    default:
        ChipLogError(DeviceLayer, "[%s] Unknown data type for attributeId: %d from clusterId: 0x%x", __FUNCTION__, getAttributeId(), getParentClusterId());
    }
}

void Attribute::setValue(uint8_t *buffer)
{
    switch(getAttributeBaseType())
    {
    case ZCL_INT8U_ATTRIBUTE_TYPE:
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE:
        uint8_t value_uint8_t;
        memcpy(&value_uint8_t, buffer, sizeof(uint8_t));
        value = value_uint8_t;
        persistValue(&value_uint8_t, sizeof(uint8_t));
        break;
    case ZCL_INT16U_ATTRIBUTE_TYPE:
        uint16_t value_uint16_t;
        memcpy(&value_uint16_t, buffer, sizeof(uint16_t));
        value = value_uint16_t;
        persistValue((uint8_t*) &value_uint16_t, sizeof(uint16_t));
        break;
    case ZCL_INT32U_ATTRIBUTE_TYPE:
        uint32_t value_uint32_t;
        memcpy(&value_uint32_t, buffer, sizeof(uint32_t));
        value = value_uint32_t;
        persistValue((uint8_t*) &value_uint32_t, sizeof(uint32_t));
        break;
    case ZCL_INT64U_ATTRIBUTE_TYPE:
        uint64_t value_uint64_t;
        memcpy(&value_uint64_t, buffer, sizeof(uint64_t));
        value = value_uint64_t;
        persistValue((uint8_t*) &value_uint64_t, sizeof(uint64_t));
        break;
    case ZCL_INT8S_ATTRIBUTE_TYPE:
        int8_t value_int8_t;
        memcpy(&value_int8_t, buffer, sizeof(int8_t));
        value = value_int8_t;
        persistValue((uint8_t*) &value_int8_t, sizeof(int8_t));
        break;
    case ZCL_INT16S_ATTRIBUTE_TYPE:
        int16_t value_int16_t;
        memcpy(&value_int16_t, buffer, sizeof(int16_t));
        value = value_int16_t;
        persistValue((uint8_t*) &value_int16_t, sizeof(int16_t));
        break;
    case ZCL_INT32S_ATTRIBUTE_TYPE:
        int32_t value_int32_t;
        memcpy(&value_int32_t, buffer, sizeof(int32_t));
        value = value_int32_t;
        persistValue((uint8_t*) &value_int32_t, sizeof(int32_t));
        break;
    case ZCL_INT64S_ATTRIBUTE_TYPE:
        int64_t value_int64_t;
        memcpy(&value_int64_t, buffer, sizeof(int64_t));
        value = value_int64_t;
        persistValue((uint8_t*) &value_int64_t, sizeof(int64_t));
        break;
    case ZCL_SINGLE_ATTRIBUTE_TYPE:
        float value_float;
        memcpy(&value_float, buffer, sizeof(float));
        value = value_float;
        persistValue((uint8_t*) &value_float, sizeof(float));
        break;
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
        memcpy(valueBuffer, buffer, attributeSize);
        persistValue(valueBuffer, attributeSize);
        break;
    case ZCL_ARRAY_ATTRIBUTE_TYPE:
    case ZCL_STRUCT_ATTRIBUTE_TYPE:
        break;
    default:
        break;
    }
}

void Attribute::persistValue(uint8_t *buffer, size_t size)
{
    // Only store if value is set to be stored in NVS (represented by ATTRIBUTE_MASK_TOKENIZE flag)
    if (getAttributeMask() & ATTRIBUTE_MASK_TOKENIZE)
    {
        char key[64];
        sprintf(key, "g/a/%x/%x/%x", parentEndpointId, parentClusterId, attributeId); // g/a/endpoint_id/cluster_id/attribute_id
        setPref_new(key, key, buffer, size);
    }
}

CHIP_ERROR Attribute::retrieveValue(uint8_t *buffer, size_t size)
{
    int32_t error = -1;

    // Only retrieved if value is set to be stored in NVS (represented by ATTRIBUTE_MASK_TOKENIZE flag)
    if (!(getAttributeMask() & ATTRIBUTE_MASK_TOKENIZE))
    {
        return CHIP_ERROR_INTERNAL;
    }

    char key[64];
    size_t len;
    sprintf(key, "g/a/%x/%x/%x", parentEndpointId, parentClusterId, attributeId); // g/a/endpoint_id/cluster_id/attribute_id
    return AmebaUtils::MapError(getPref_bin_new(key, key, buffer, size, &len), AmebaErrorType::kDctError);
}

/*                  Events                  */
chip::EventId Event::getEventId() const
{
    return eventId;
}

chip::ClusterId Event::getParentClusterId() const
{
    return parentClusterId;
}

/*                  Commands                  */
chip::CommandId Command::getCommandId() const
{
    return commandId;
}

chip::ClusterId Command::getParentClusterId() const
{
    return parentClusterId;
}

uint8_t Command::getFlag() const
{
    return commandFlag;
}

/*                  Cluster                  */
chip::ClusterId Cluster::getClusterId() const
{
    return clusterId;
}

EmberAfClusterMask Cluster::getClusterMask() const
{
    return clusterMask;
}

chip::EndpointId Cluster::getParentEndpointId() const
{
    return parentEndpointId;
}

Attribute *Cluster::getAttribute(chip::AttributeId attributeId)
{
    for (auto & att : attributes)
    {
        if (att.getAttributeId() == attributeId)
        {
            return &att;
        }
    }

    return NULL;
}

Event *Cluster::getEvent(chip::EventId eventId)
{
    for (auto & evt : events)
    {
        if (evt.getEventId() == eventId)
        {
            return &evt;
        }
    }

    return NULL;
}

Command *Cluster::getAcceptedCommand(chip::CommandId commandId)
{
    for (auto & cmd : acceptedCommands)
    {
        if (cmd.getCommandId() == commandId)
        {
            return &cmd;
        }
    }

    return NULL;
}

Command *Cluster::getGeneratedCommand(chip::CommandId commandId)
{
    for (auto & cmd : generatedCommands)
    {
        if (cmd.getCommandId() == commandId)
        {
            return &cmd;
        }
    }

    return NULL;
}

void Cluster::addAttribute(AttributeConfig attributeConfig)
{
    Attribute attribute(clusterId, parentEndpointId, attributeConfig);
    addAttribute(attribute);
}

void Cluster::addAttribute(const Attribute& attribute)
{
    // Check if attribute already exist
    if (getAttribute(attribute.getAttributeId()) != NULL)
    {
        return;
    }

    attributes.push_back(attribute);
}

void Cluster::removeAttribute(chip::AttributeId attributeId)
{
    auto it = std::find_if(attributes.begin(), attributes.end(), [&](const Attribute & attribute)
    {
        return attribute.getAttributeId() == attributeId;
    });

    if (it != attributes.end())
    {
        attributes.erase(it);
    }
}

void Cluster::addEvent(EventConfig eventConfig)
{
    Event event(clusterId, parentEndpointId, eventConfig);
    addEvent(event);
}

void Cluster::addEvent(const Event& event)
{
    // Check if event already exist
    if (getEvent(event.getEventId()) != NULL)
    {
        return;
    }

    events.push_back(event);
}

void Cluster::removeEvent(chip::EventId eventId)
{
    auto it = std::find_if(events.begin(), events.end(), [&](const Event & event)
    {
        return event.getEventId() == eventId;
    });

    if (it != events.end())
    {
        events.erase(it);
    }
}

void Cluster::addAcceptedCommand(CommandConfig commandConfig)
{
    Command command(clusterId, parentEndpointId, commandConfig);
    addAcceptedCommand(command);
}

void Cluster::addAcceptedCommand(const Command& command)
{
    // Check if command already exist
    if (getAcceptedCommand(command.getCommandId()) != NULL)
    {
        return;
    }

    if (command.getFlag() & COMMAND_MASK_ACCEPTED)
    {
        acceptedCommands.push_back(command);
    }
}

void Cluster::addGeneratedCommand(CommandConfig commandConfig)
{
    Command command(clusterId, parentEndpointId, commandConfig);
    addGeneratedCommand(command);
}

void Cluster::addGeneratedCommand(const Command& command)
{
    // Check if command already exist
    if (getGeneratedCommand(command.getCommandId()) != NULL)
    {
        return;
    }

    if (command.getFlag() & COMMAND_MASK_GENERATED)
    {
        generatedCommands.push_back(command);
    }
}

void Cluster::removeAcceptedCommand(chip::CommandId commandId)
{
    auto it = std::find_if(acceptedCommands.begin(), acceptedCommands.end(), [commandId](const Command& command)
    {
        return command.getCommandId() == commandId;
    });

    if (it != acceptedCommands.end())
    {
        acceptedCommands.erase(it);
    }
}

void Cluster::removeGeneratedCommand(chip::CommandId commandId)
{
    auto it = std::find_if(generatedCommands.begin(), generatedCommands.end(), [commandId](const Command& command)
    {
        return command.getCommandId() == commandId;
    });

    if (it != generatedCommands.end())
    {
        generatedCommands.erase(it);
    }
}

void Cluster::addFunction(const EmberAfGenericClusterFunction function)
{
    functions.push_back(function);
}

void Cluster::removeFunction()
{
    // not implemented
}

/*                  Endpoint                  */
chip::EndpointId Endpoint::getEndpointId() const
{
    return endpointId;
}

Cluster *Endpoint::getCluster(chip::ClusterId clusterId)
{
    for (auto & cls : clusters)
    {
        if (cls.getClusterId() == clusterId)
        {
            return &cls;
        }
    }

    return NULL;
}

void Endpoint::addCluster(ClusterConfig & clusterConfig)
{
    Cluster cluster(endpointId, clusterConfig);
    for (const AttributeConfig & attributeConfig : clusterConfig.attributeConfigs)
    {
        Attribute attribute(cluster.getClusterId(), endpointId, attributeConfig);
        cluster.addAttribute(attribute);
    }
    for (const EventConfig & eventConfig : clusterConfig.eventConfigs)
    {
        Event event(cluster.getClusterId(), endpointId, eventConfig);
        cluster.addEvent(event);
    }
    for (const CommandConfig & commandConfig : clusterConfig.commandConfigs)
    {
        Command command(cluster.getClusterId(), endpointId, commandConfig);
        if (command.getFlag() & COMMAND_MASK_ACCEPTED)
        {
            cluster.addAcceptedCommand(command);
        }
        if (command.getFlag() & COMMAND_MASK_GENERATED)
        {
            cluster.addGeneratedCommand(command);
        }
    }
    for (const EmberAfGenericClusterFunction & functionConfig : clusterConfig.functionConfigs)
    {
        cluster.addFunction(functionConfig);
    }
    addCluster(cluster);
}

void Endpoint::addCluster(const Cluster& cluster)
{
    // Check if cluster already exist
    if (getCluster(cluster.getClusterId()) != NULL)
    {
        return;
    }

    clusters.push_back(cluster);
}

void Endpoint::removeCluster(chip::ClusterId clusterId)
{
    // Remove the cluster from the vector
    auto it = std::find_if(clusters.begin(), clusters.end(), [&](const Cluster& cluster)
    {
        return cluster.getClusterId() == clusterId;
    });

    if (it != clusters.end())
    {
        clusters.erase(it);
    }
}

chip::EndpointId Endpoint::getParentEndpointId() const
{
    return parentEndpointId;
}

void Endpoint::setParentEndpointId(chip::EndpointId newParentEndpointId)
{
    parentEndpointId = newParentEndpointId;
}

void Endpoint::enableEndpoint()
{
    if (enabled)
    {
        ChipLogDetail(DeviceLayer, "Endpoint %d already enabled", endpointId);
        return;
    }

    dataVersion = (chip::DataVersion*) calloc(clusters.size(), sizeof(chip::DataVersion));

    EmberAfEndpointType *endpointType = nullptr;
    EmberAfCluster *clusterType = nullptr;
    EmberAfAttributeMetadata *attributeType = nullptr;
    EmberAfGenericClusterFunction *functionType = nullptr;
    chip::CommandId *acceptedCommandType = nullptr;
    chip::CommandId *generatedCommandType = nullptr;
    chip::EventId *eventType = nullptr;

    // Setup cluster type
    if (clusters.size() > 0)
    {
        clusterType = (EmberAfCluster*) calloc(clusters.size(), sizeof(EmberAfCluster));
        clusterCollector.push_back(clusterType);
    }

    for (size_t i=0; i<clusters.size(); i++)
    {
        Cluster cluster = clusters[i];

        // Setup attributes
        if (cluster.attributes.size() > 0)
        {
            attributeType = (EmberAfAttributeMetadata*) calloc(cluster.attributes.size(), sizeof(EmberAfAttributeMetadata));
            attributeCollector.push_back(attributeType);
        }

        for (size_t j=0; j<cluster.attributes.size(); j++)
        {
            attributeType[j].defaultValue = cluster.attributes[j].getAttributeDefaultValue(); 
            attributeType[j].attributeId = cluster.attributes[j].getAttributeId();
            attributeType[j].size = cluster.attributes[j].getAttributeSize();
            attributeType[j].attributeType = cluster.attributes[j].getAttributeType();
            attributeType[j].mask = cluster.attributes[j].getAttributeMask();
        }

        // Setup cluster functions
        if (cluster.functions.size() > 0)
        {
            functionType = (EmberAfGenericClusterFunction*) calloc(cluster.functions.size(), sizeof(EmberAfGenericClusterFunction));
            functionCollector.push_back(functionType);
        }

        for (size_t j=0; j<cluster.functions.size(); j++)
        {
            functionType[j] = cluster.functions[j];
        }

        // Setup accepted commands
        if (cluster.acceptedCommands.size() > 0)
        {
            acceptedCommandType = (chip::CommandId*) calloc(cluster.acceptedCommands.size(), sizeof(chip::CommandId));
            acceptedCommandCollector.push_back(acceptedCommandType);
        }

        for (size_t j=0; j<cluster.acceptedCommands.size(); j++)
        {
            acceptedCommandType[j] = cluster.acceptedCommands[j].getCommandId();
        }

        // Setup generated commands
        if (cluster.generatedCommands.size() > 0)
        {
            generatedCommandType = (chip::CommandId*) calloc(cluster.generatedCommands.size(), sizeof(chip::CommandId));
            generatedCommandCollector.push_back(generatedCommandType);
        }

        for (size_t j=0; j<cluster.generatedCommands.size(); j++)
        {
            generatedCommandType[j] = cluster.generatedCommands[j].getCommandId();
        }

        // Setup events
        if (cluster.events.size() > 0)
        {
            eventType = (chip::EventId*) calloc(cluster.events.size(), sizeof(chip::EventId));
            eventCollector.push_back(eventType);
        }

        for (size_t j=0; j<cluster.events.size(); j++)
        {
            eventType[j] = cluster.events[j].getEventId();
        }

        clusterType[i].clusterId = cluster.getClusterId();    
        clusterType[i].attributeCount = cluster.attributes.size();
        clusterType[i].clusterSize = 0;   // default value
        clusterType[i].mask = cluster.getClusterMask();
        clusterType[i].eventCount = cluster.events.size();
        clusterType[i].attributes = attributeType;
        clusterType[i].functions = functionType;
        clusterType[i].acceptedCommandList = acceptedCommandType;
        clusterType[i].generatedCommandList = generatedCommandType;
        clusterType[i].eventList = eventType;
    }

    // Setup endpoint type
    endpointType = (EmberAfEndpointType*) calloc(1, sizeof(EmberAfEndpointType));
    endpointType->clusterCount = clusters.size();
    endpointType->endpointSize = 0;   // set to 0 as default
    endpointType->cluster = clusterType;

    // Register endpoint as dynamic endpoint in matter stack
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    EmberAfStatus status = emberAfSetDynamicEndpoint(endpointIndex, endpointId, endpointType, chip::Span<chip::DataVersion>(dataVersion, clusters.size()), deviceTypeList, parentEndpointId);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "Set dynamic endpoint %d success", endpointId);
        endpointMetadata = endpointType;
        enabled = true;
        return;
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to set dynamic endpoint %d with status %d", endpointId, status);
    }

    // free allocated memory if error
    for (EmberAfCluster *cls : clusterCollector)
    {
        free(cls);
    }

    for (EmberAfAttributeMetadata *att : attributeCollector)
    {
        free(att);
    }

    for (EmberAfGenericClusterFunction *fun : functionCollector)
    {
        free(fun);
    }

    for (chip::CommandId *acmd : acceptedCommandCollector)
    {
        free(acmd);
    }

    for (chip::CommandId *gcmd : generatedCommandCollector)
    {
        free(gcmd);
    }

    for (chip::EventId *evt : eventCollector)
    {
        free(evt);
    }

    free(dataVersion);
    free(endpointType);
}

void Endpoint::disableEndpoint()
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    int endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpointId);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (endpointIndex == 0xFFFF)
    {
        ChipLogError(DeviceLayer, "Could not find endpoint index");
        return;
    }

    // clear dynamic endpoint on matter stack
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    emberAfClearDynamicEndpoint(endpointIndex);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    enabled = false;

    // free allocated memory
    for (EmberAfCluster *cls : clusterCollector)
    {
        free(cls);
    }

    for (EmberAfAttributeMetadata *att : attributeCollector)
    {
        free(att);
    }

    for (EmberAfGenericClusterFunction *fun : functionCollector)
    {
        free(fun);
    }

    for (chip::CommandId *acmd : acceptedCommandCollector)
    {
        free(acmd);
    }

    for (chip::CommandId *gcmd : generatedCommandCollector)
    {
        free(gcmd);
    }

    for (chip::EventId *evt : eventCollector)
    {
        free(evt);
    }

    free(dataVersion);
    free(endpointMetadata);

    char key[64];

    // Clear persistent data on this endpoint
    for (size_t i=0; i<clusters.size(); i++)
    {
        Cluster cluster = clusters[i];
        for (size_t j=0; j<cluster.attributes.size(); j++)
        {
            Attribute attribute = cluster.attributes[j];
            if (cluster.attributes[j].getAttributeMask() & ATTRIBUTE_MASK_TOKENIZE)
            {
                sprintf(key, "g/a/%x/%x/%x", attribute.getParentEndpointId(), attribute.getParentClusterId(), attribute.getAttributeId());
                deleteKey(key, key);
            }
        }
    }
    ChipLogProgress(DeviceLayer, "Successfully disabled dynamic endpoint %d", endpointId);
}

/*                  Node                  */
Node& Node::getInstance()
{
    static Node instance;
    return instance;
}

Endpoint *Node::getEndpoint(chip::EndpointId endpointId)
{
    int i=0;
    for (auto & ep : endpoints)
    {
        if (ep.getEndpointId() == endpointId)
        {
            return &ep;
        }
        i++;
    }
    return NULL;
}

chip::EndpointId Node::getNextEndpointId() const
{
    return nextEndpointId;
}

chip::EndpointId Node::addEndpoint(const EndpointConfig& endpointConfig, Span<const EmberAfDeviceType> deviceTypeList)
{
    Endpoint endpoint(this, nextEndpointId, endpointCount, deviceTypeList);
    // Set parentEndpointId based on the previous endpoint's endpointId
    if (!endpoints.empty())
    {
        endpoint.setParentEndpointId(endpoints.back().getEndpointId());
    }

    for (const ClusterConfig& clusterConfig : endpointConfig.clusterConfigs)
    {
        Cluster cluster(endpoint.getEndpointId(), clusterConfig);
        for (const AttributeConfig& attributeConfig : clusterConfig.attributeConfigs)
        {
            Attribute attribute(cluster.getClusterId(), endpoint.getEndpointId(), attributeConfig);
            cluster.addAttribute(attribute);
        }
        for (const EventConfig& eventConfig : clusterConfig.eventConfigs)
        {
            Event event(cluster.getClusterId(), endpoint.getEndpointId(), eventConfig);
            cluster.addEvent(event);
        }
        for (const CommandConfig& commandConfig : clusterConfig.commandConfigs)
        {
            Command command(cluster.getClusterId(), endpoint.getEndpointId(), commandConfig);
            if (command.getFlag() & COMMAND_MASK_ACCEPTED)
            {
                cluster.addAcceptedCommand(command);
            }
            if (command.getFlag() & COMMAND_MASK_GENERATED)
            {
                cluster.addGeneratedCommand(command);
            }
        }
        for (const EmberAfGenericClusterFunction & functionConfig : clusterConfig.functionConfigs)
        {
            cluster.addFunction(functionConfig);
        }
        endpoint.addCluster(cluster);
    }
    endpoints.push_back(endpoint);
    endpointCount++;
    nextEndpointId++;

    return endpoint.getEndpointId();
}

void Node::removeEndpoint(chip::EndpointId endpointId)
{
    auto it = std::find_if(endpoints.begin(), endpoints.end(), [&](const Endpoint& endpoint)
    {
        return endpoint.getEndpointId() == endpointId;
    });

    if (it != endpoints.end())
    {
        // Get the index of the endpoint in the vector
        int index = std::distance(endpoints.begin(), it);

        if (it->enabled)
        {
            ChipLogError(DeviceLayer, "Endpoint not yet disabled!");
            return;
        }

        // Remove the endpoint from the vector
        endpoints.erase(it);

        // Update the parentEndpointId of subsequent endpoints
        for (int i = index; i < endpoints.size(); ++i)
        {
            if (i > 0)
            {
                endpoints[i].setParentEndpointId(endpoints[i - 1].getEndpointId());
            }
            else
            {
                // If the removed endpoint was the first one, set the parentEndpointId to 0xFFFF
                endpoints[i].setParentEndpointId(0xFFFF);
            }
        }
    }
    endpointCount--;
}

void Node::enableAllEndpoints()
{
    for (Endpoint & endpoint: endpoints)
    {
        endpoint.enableEndpoint();
    }
}
