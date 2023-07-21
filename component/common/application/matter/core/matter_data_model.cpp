#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <app/util/attribute-storage.h>
#include "matter_data_model.h"

using namespace ::chip;

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
        ChipLogError(DeviceLayer, "Insufficient space to read Attribute 0x%08x from Cluster 0x%08x in Endpoint 0x%04x", matter_attribute->attributeId, cluster_id, endpoint_id);
        return EMBER_ZCL_STATUS_RESOURCE_EXHAUSTED;
    }

    switch(attribute->getAttributeBaseType())
    {
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
    case ZCL_ARRAY_ATTRIBUTE_TYPE:
    case ZCL_STRUCT_ATTRIBUTE_TYPE:
        // memcpy(buffer, attribute->getValue<uint8_t*>(), attribute->getAttributeSize());
        memcpy(buffer, std::get<uint8_t*>(attribute->getValue()), attribute->getAttributeSize());
        break;
    default:
        ChipLogError(DeviceLayer, "");
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint_id, ClusterId cluster_id, const EmberAfAttributeMetadata *matter_attribute, uint8_t *buffer)
{
    Node & node = Node::getInstance();
    Endpoint *endpoint = node.getEndpoint(endpoint_id);
    Cluster *cluster = endpoint->getCluster(cluster_id);
    Attribute *attribute = cluster->getAttribute(matter_attribute->attributeId);

    switch(attribute->getAttributeBaseType())
    {
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
    case ZCL_ARRAY_ATTRIBUTE_TYPE:
    case ZCL_STRUCT_ATTRIBUTE_TYPE:
        // memcpy(attribute->getValue<uint8_t*>(), buffer, attribute->getAttributeSize());
        memcpy(std::get<uint8_t*>(attribute->getValue()), buffer, attribute->getAttributeSize());
        break;
    default:
        ChipLogError(DeviceLayer, "");
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

/*                  Attributes                  */
chip::AttributeId Attribute::getAttributeId() const {
    return attributeId;
}

std::uint16_t Attribute::getAttributeSize() const {
    return attributeSize;
}

std::uint8_t Attribute::getAttributeType() const {
    return attributeType;
}

std::uint8_t Attribute::getAttributeMask() const {
    return attributeMask;
}

EmberAfDefaultOrMinMaxAttributeValue Attribute::getAttributeDefaultValue() const {
    return defaultValue;
}

EmberAfAttributeType Attribute::getAttributeBaseType() {
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

AttributeValue Attribute::getValue() const {
    return value;
}

void Attribute::setValue(AttributeValue & newValue) {
    switch(getAttributeBaseType())
    {
    case ZCL_INT8U_ATTRIBUTE_TYPE:
        value = std::get<uint8_t>(newValue);
        break;
    case ZCL_INT16U_ATTRIBUTE_TYPE:
        value = std::get<uint16_t>(newValue);
        break;
    case ZCL_INT32U_ATTRIBUTE_TYPE:
        value = std::get<uint32_t>(newValue);
        break;
    case ZCL_INT64U_ATTRIBUTE_TYPE:
        value = std::get<uint64_t>(newValue);
        break;
    case ZCL_INT8S_ATTRIBUTE_TYPE:
        value = std::get<int8_t>(newValue);
        break;
    case ZCL_INT16S_ATTRIBUTE_TYPE:
        value = std::get<int16_t>(newValue);
        break;
    case ZCL_INT32S_ATTRIBUTE_TYPE:
        value = std::get<int32_t>(newValue);
        break;
    case ZCL_INT64S_ATTRIBUTE_TYPE:
        value = std::get<int64_t>(newValue);
        break;
    case ZCL_SINGLE_ATTRIBUTE_TYPE:
        value = std::get<float>(newValue);
        break;
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
    case ZCL_ARRAY_ATTRIBUTE_TYPE:
    case ZCL_STRUCT_ATTRIBUTE_TYPE:
        value = std::get<uint8_t*>(newValue);
        break;
    default:
        ChipLogError(DeviceLayer, "Unknown attribute type, unable to assign value");
    }
}

void Attribute::print(int indent) const {
    // std::string indentation(indent, ' ');
    // if (attributeType == ZCL_INT8U_ATTRIBUTE_TYPE) {
    //     std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint8_t>()) << std::endl;
    // } else if (attributeType == ZCL_INT16U_ATTRIBUTE_TYPE) {
    //     std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint16_t>()) << std::endl;
    // } else if (attributeType == ZCL_INT32U_ATTRIBUTE_TYPE) {
    //     std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint32_t>()) << std::endl;
    // } else if (attributeType == ZCL_INT64U_ATTRIBUTE_TYPE) {
    //     std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint64_t>()) << std::endl;
    // } else if (attributeType == ZCL_INT8S_ATTRIBUTE_TYPE) {
    //     std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int8_t>()) << std::endl;
    // } else if (attributeType == ZCL_INT16S_ATTRIBUTE_TYPE) {
    //     std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int16_t>()) << std::endl;
    // } else if (attributeType == ZCL_INT32S_ATTRIBUTE_TYPE) {
    //     std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int32_t>()) << std::endl;
    // } else if (attributeType == ZCL_INT64S_ATTRIBUTE_TYPE) {
    //     std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int64_t>()) << std::endl;
    // } else if (attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE) {
    //     std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << getValue<std::string>() << std::endl;
    // } else {
    //     // Handle the case where the data type doesn't match
    //     std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value type undefined" << std::endl;
    // }
}

/*                  Events                  */
chip::EventId Event::getEventId() const {
    return eventId;
}

void Event::print(int indent) const {
    std::string indentation(indent, ' ');
    std::cout << indentation << "- Event " << eventId << std::endl;
}

/*                  Commands                  */
chip::CommandId Command::getCommandId() const {
    return commandId;
}

int Command::getFlag() const {
    return commandFlag;
}

void Command::print(int indent) const {
    std::string indentation(indent, ' ');
    std::cout << indentation << "- Command " << commandId << std::endl;
}

/*                  Cluster                  */
chip::ClusterId Cluster::getClusterId() const {
    return clusterId;
}

EmberAfClusterMask Cluster::getClusterMask() const {
    return clusterMask;
}

Attribute *Cluster::getAttribute(chip::AttributeId attributeId) {
    for (auto & att : attributes) {
        if (att.getAttributeId() == attributeId) {
            return &att;
        }
    }

    return NULL;
}

Event *Cluster::getEvent(chip::EventId eventId) {
    for (auto & evt : events) {
        if (evt.getEventId() == eventId) {
            return &evt;
        }
    }

    return NULL;
}

Command *Cluster::getAcceptedCommand(chip::CommandId commandId) {
    for (auto & cmd : acceptedCommands) {
        if (cmd.getCommandId() == commandId) {
            return &cmd;
        }
    }

    return NULL;
}

Command *Cluster::getGeneratedCommand(chip::CommandId commandId) {
    for (auto & cmd : generatedCommands) {
        if (cmd.getCommandId() == commandId) {
            return &cmd;
        }
    }

    return NULL;
}

void Cluster::addAttribute(AttributeConfig attributeConfig) {
    Attribute attribute(this, attributeConfig);
    addAttribute(attribute);
}

void Cluster::addAttribute(const Attribute& attribute) {
    // Check if attribute already exist
    if (getAttribute(attribute.getAttributeId()) != NULL) {
        return;
    }

    attributes.push_back(attribute);
}

void Cluster::removeAttribute(chip::AttributeId attributeId) {
    auto it = std::find_if(attributes.begin(), attributes.end(), [&](const Attribute & attribute) {
        return attribute.getAttributeId() == attributeId;
    });

    if (it != attributes.end()) {
        attributes.erase(it);
    }
}

void Cluster::addEvent(EventConfig eventConfig) {
    Event event(this, eventConfig);
    addEvent(event);
}

void Cluster::addEvent(const Event& event) {
    // Check if event already exist
    if (getEvent(event.getEventId()) != NULL) {
        return;
    }

    events.push_back(event);
}

void Cluster::removeEvent(chip::EventId eventId) {
    auto it = std::find_if(events.begin(), events.end(), [&](const Event & event) {
        return event.getEventId() == eventId;
    });

    if (it != events.end()) {
        events.erase(it);
    }
}

void Cluster::addAcceptedCommand(CommandConfig commandConfig) {
    Command command(this, commandConfig);
    addAcceptedCommand(command);
}

void Cluster::addAcceptedCommand(const Command& command) {
    // Check if command already exist
    if (getAcceptedCommand(command.getCommandId()) != NULL) {
        return;
    }

    if (command.getFlag() & COMMAND_MASK_ACCEPTED) {
        acceptedCommands.push_back(command);
    }
}

void Cluster::addGeneratedCommand(CommandConfig commandConfig) {
    Command command(this, commandConfig);
    addGeneratedCommand(command);
}

void Cluster::addGeneratedCommand(const Command& command) {
    // Check if command already exist
    if (getGeneratedCommand(command.getCommandId()) != NULL) {
        return;
    }

    if (command.getFlag() & COMMAND_MASK_GENERATED) {
        generatedCommands.push_back(command);
    }
}

void Cluster::removeAcceptedCommand(chip::CommandId commandId) {
    auto it = std::find_if(acceptedCommands.begin(), acceptedCommands.end(), [commandId](const Command& command) {
        return command.getCommandId() == commandId;
    });

    if (it != acceptedCommands.end()) {
        acceptedCommands.erase(it);
    }
}

void Cluster::removeGeneratedCommand(chip::CommandId commandId) {
    auto it = std::find_if(generatedCommands.begin(), generatedCommands.end(), [commandId](const Command& command) {
        return command.getCommandId() == commandId;
    });

    if (it != generatedCommands.end()) {
        generatedCommands.erase(it);
    }
}

void Cluster::print(int indent) const {
    std::string indentation(indent, ' ');
    std::cout << indentation << "Cluster " << getClusterId() << ": " << std::endl;
    for (const Attribute& attribute : attributes) {
        attribute.print(indent + 2);
    }
    for (const Event& event : events) {
        event.print(indent + 2);
    }
    for (const Command& command : acceptedCommands) {
        command.print(indent + 2);
    }
    for (const Command& command : generatedCommands) {
        command.print(indent + 2);
    }
}

/*                  Endpoint                  */
chip::EndpointId Endpoint::getEndpointId() const {
    return endpointId;
}

Cluster *Endpoint::getCluster(chip::ClusterId clusterId) {
    for (auto & cls : clusters) {
        if (cls.getClusterId() == clusterId) {
            return &cls;
        }
    }

    return NULL;
}

void Endpoint::addCluster(ClusterConfig & clusterConfig) {
    Cluster cluster(this, clusterConfig);
    for (const AttributeConfig & attributeConfig : clusterConfig.attributeConfigs) {
        Attribute attribute(&cluster, attributeConfig);
        cluster.addAttribute(attribute);
    }
    for (const EventConfig & eventConfig : clusterConfig.eventConfigs) {
        Event event(&cluster, eventConfig);
        cluster.addEvent(event);
    }
    for (const CommandConfig & commandConfig : clusterConfig.commandConfigs) {
        Command command(&cluster, commandConfig);
        if (command.getFlag() & COMMAND_MASK_ACCEPTED) {
            cluster.addAcceptedCommand(command);
        }
        if (command.getFlag() & COMMAND_MASK_GENERATED) {
            cluster.addGeneratedCommand(command);
        }
    }
    addCluster(cluster);
}

void Endpoint::addCluster(const Cluster& cluster) {
    // Check if cluster already exist
    if (getCluster(cluster.getClusterId()) != NULL) {
        return;
    }

    clusters.push_back(cluster);
}

void Endpoint::removeCluster(chip::ClusterId clusterId) {
    // Remove the cluster from the vector
    auto it = std::find_if(clusters.begin(), clusters.end(), [&](const Cluster& cluster) {
        return cluster.getClusterId() == clusterId;
    });

    if (it != clusters.end()) {
        clusters.erase(it);
    }
}

chip::EndpointId Endpoint::getParentEndpointId() const {
    return parentEndpointId;
}

void Endpoint::setParentEndpointId(chip::EndpointId newParentEndpointId) {
    parentEndpointId = newParentEndpointId;
}

void Endpoint::enableEndpoint(Span<const EmberAfDeviceType> deviceTypeList) {
    // dataVersion = new chip::DataVersion[clusters.size()]; // dataVersion should be as big as the number of clusters in this endpoint
    dataVersion = (chip::DataVersion*) calloc(clusters.size(), sizeof(chip::DataVersion));
    // index - to setup a counter, maybe use nextEndpointId-1?
    // endpointId - member
    // EmberAfEndpointType - to be created
    // dataVersionStorage - allocated, member
    // deviceTypeList - argument
    // parentEndpointId - member

    // Store dynamic allocated objects here, then delete it at the end of this function
    // TODO: do we need this garbage collectors? Can we loop through the list?
    // std::vector<EmberAfCluster*> clusterGarbageCollector;
    // std::vector<EmberAfAttributeMetadata*> attributeGarbageCollector;
    // std::vector<EmberAfGenericClusterFunction*> functionGarbageCollector;
    // std::vector<chip::CommandId*> acceptedCommandGarbageCollector;
    // std::vector<chip::CommandId*> generatedCommandGarbageCollector;
    // std::vector<chip::EventId*> eventGarbageCollector;

    EmberAfEndpointType *endpointType = nullptr;
    EmberAfCluster *clusterType = nullptr;
    EmberAfAttributeMetadata *attributeType = nullptr;
    EmberAfGenericClusterFunction *functionType = nullptr;
    chip::CommandId *acceptedCommandType = nullptr;
    chip::CommandId *generatedCommandType = nullptr;
    chip::EventId *eventType = nullptr;

    // Setup cluster type
    if (clusters.size() > 0) {
        clusterType = (EmberAfCluster*) calloc(clusters.size(), sizeof(EmberAfCluster));
        clusterGarbageCollector.push_back(clusterType);
    }

    for (size_t i=0; i<clusters.size(); i++) {
        Cluster cluster = clusters[i];

        // Setup attributes
        if (cluster.attributes.size() > 0) {
            attributeType = (EmberAfAttributeMetadata*) calloc(cluster.attributes.size(), sizeof(EmberAfAttributeMetadata));
            attributeGarbageCollector.push_back(attributeType);
        }

        for (size_t j=0; j<cluster.attributes.size(); j++) {
            attributeType[j].defaultValue = cluster.attributes[j].getAttributeDefaultValue(); 
            attributeType[j].attributeId = cluster.attributes[j].getAttributeId();
            attributeType[j].size = cluster.attributes[j].getAttributeSize();
            attributeType[j].attributeType = cluster.attributes[j].getAttributeType();
            attributeType[j].mask = cluster.attributes[j].getAttributeMask();
        }

        // Setup cluster functions
        if (cluster.functions.size() > 0) {
            functionType = (EmberAfGenericClusterFunction*) calloc(cluster.functions.size(), sizeof(EmberAfGenericClusterFunction));
            functionGarbageCollector.push_back(functionType);
        }

        for (size_t j=0; j<cluster.functions.size(); j++) {
            functionType[j] = cluster.functions[j];
        }

        // Setup accepted commands
        if (cluster.acceptedCommands.size() > 0) {
            acceptedCommandType = (chip::CommandId*) calloc(cluster.acceptedCommands.size(), sizeof(chip::CommandId));
            acceptedCommandGarbageCollector.push_back(acceptedCommandType);
        }

        for (size_t j=0; j<cluster.acceptedCommands.size(); j++) {
            acceptedCommandType[j] = cluster.acceptedCommands[j].getCommandId();
        }

        // Setup generated commands
        if (cluster.generatedCommands.size() > 0) {
            generatedCommandType = (chip::CommandId*) calloc(cluster.generatedCommands.size(), sizeof(chip::CommandId));
            generatedCommandGarbageCollector.push_back(generatedCommandType);
        }

        for (size_t j=0; j<cluster.generatedCommands.size(); j++) {
            generatedCommandType[j] = cluster.generatedCommands[j].getCommandId();
        }

        // Setup events
        if (cluster.events.size() > 0) {
            eventType = (chip::EventId*) calloc(cluster.events.size(), sizeof(chip::EventId));
            eventGarbageCollector.push_back(eventType);
        }

        for (size_t j=0; j<cluster.events.size(); j++) {
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

    // Register endpoint as Matter dynamic endpoint
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    EmberAfStatus status = emberAfSetDynamicEndpoint(parentNode->getNextEndpointId() - 1, endpointId, endpointType, chip::Span<chip::DataVersion>(dataVersion, clusters.size()), deviceTypeList, parentEndpointId);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (status == EMBER_ZCL_STATUS_SUCCESS) {
        ChipLogProgress(DeviceLayer, "Set dynamic endpoint %d success", endpointId);
        endpointMetadata = endpointType;
        return;
    }
    else {
        ChipLogError(DeviceLayer, "Failed to create dynamic endpoint %d with status %d", endpointId, status);
    }

    // free allocated memory if error
    for (EmberAfCluster *cls : clusterGarbageCollector) {
        free(cls);
    }

    for (EmberAfAttributeMetadata *att : attributeGarbageCollector) {
        free(att);
    }

    for (EmberAfGenericClusterFunction *fun : functionGarbageCollector) {
        free(fun);
    }

    for (chip::CommandId *acmd : acceptedCommandGarbageCollector) {
        free(acmd);
    }

    for (chip::CommandId *gcmd : generatedCommandGarbageCollector) {
        free(gcmd);
    }

    for (chip::EventId *evt : eventGarbageCollector) {
        free(evt);
    }

    free(dataVersion);
    free(endpointType);
}

void Endpoint::disableEndpoint() {

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    int endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpointId);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if (endpointIndex == 0xFFFF) {
        ChipLogError(DeviceLayer, "Could not find endpoint index");
        return;
    }

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    emberAfClearDynamicEndpoint(endpointIndex);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    // free allocated memory if error
    for (EmberAfCluster *cls : clusterGarbageCollector) {
        free(cls);
    }

    for (EmberAfAttributeMetadata *att : attributeGarbageCollector) {
        free(att);
    }

    for (EmberAfGenericClusterFunction *fun : functionGarbageCollector) {
        free(fun);
    }

    for (chip::CommandId *acmd : acceptedCommandGarbageCollector) {
        free(acmd);
    }

    for (chip::CommandId *gcmd : generatedCommandGarbageCollector) {
        free(gcmd);
    }

    for (chip::EventId *evt : eventGarbageCollector) {
        free(evt);
    }

    free(dataVersion);
    free(endpointMetadata);

    // TODO: clear persistent data on this endpoint
    ChipLogProgress(DeviceLayer, "Successfully disabled dynamic endpoint %d", endpointId);
}

void Endpoint::print(int indent) const {
    std::string indentation(indent, ' ');
    std::cout << indentation << "Endpoint_" << endpointId << " parentEndpoint: " << getParentEndpointId() << std::endl;
    for (const Cluster& cluster : clusters) {
        cluster.print(indent + 2);
    }
}

/*                  Node                  */
Node& Node::getInstance() {
    static Node instance;
    return instance;
}

Endpoint *Node::getEndpoint(chip::EndpointId endpointId) {
    int i=0;
    for (auto & ep : endpoints) {
        if (ep.getEndpointId() == endpointId) {
            return &ep;
        }
        i++;
    }
    return NULL;
}

chip::EndpointId Node::getNextEndpointId() const {
    return nextEndpointId;
}

void Node::addEndpoint(const EndpointConfig& endpointConfig) {
    Endpoint endpoint(this, nextEndpointId);
    // Set parentEndpointId based on the previous endpoint's endpointId
    if (!endpoints.empty()) {
        endpoint.setParentEndpointId(endpoints.back().getEndpointId());
    }

    for (const ClusterConfig& clusterConfig : endpointConfig.clusterConfigs) {
        Cluster cluster(&endpoint, clusterConfig);
        for (const AttributeConfig& attributeConfig : clusterConfig.attributeConfigs) {
            Attribute attribute(&cluster, attributeConfig);
            cluster.addAttribute(attribute);
        }
        for (const EventConfig& eventConfig : clusterConfig.eventConfigs) {
            Event event(&cluster, eventConfig);
            cluster.addEvent(event);
        }
        for (const CommandConfig& commandConfig : clusterConfig.commandConfigs) {
            Command command(&cluster, commandConfig);
            if (command.getFlag() & COMMAND_MASK_ACCEPTED) {
                cluster.addAcceptedCommand(command);
            }
            if (command.getFlag() & COMMAND_MASK_GENERATED) {
                cluster.addGeneratedCommand(command);
            }
        }
        endpoint.addCluster(cluster);
    }
    endpoints.push_back(endpoint);
    endpointCount++;
    nextEndpointId++;

}

void Node::removeEndpoint(chip::EndpointId endpointId) {
    auto it = std::find_if(endpoints.begin(), endpoints.end(), [&](const Endpoint& endpoint) {
        return endpoint.getEndpointId() == endpointId;
    });

    if (it != endpoints.end()) {
        // Get the index of the endpoint in the vector
        int index = std::distance(endpoints.begin(), it);

        // Remove the endpoint from the vector
        endpoints.erase(it);

        // Update the parentEndpointId of subsequent endpoints
        for (int i = index; i < endpoints.size(); ++i) {
            if (i > 0) {
                endpoints[i].setParentEndpointId(endpoints[i - 1].getEndpointId());
            } else {
                // If the removed endpoint was the first one, set the parentEndpointId to 0xFFFF
                endpoints[i].setParentEndpointId(0xFFFF);
            }
        }
    }
    endpointCount--;
}

void Node::print() const {
    std::cout << "Node" << std::endl;
    for (const Endpoint& endpoint : endpoints) {
        endpoint.print(2);
    }
}

// EndpointConfig createEndpointConfig() {
//     // Create Attribute configurations
//     AttributeConfig attributeConfig1;
//     attributeConfig1.attributeId = 0;
//     attributeConfig1.dataType = ZCL_INT8U_ATTRIBUTE_TYPE;
//     attributeConfig1.value = std::uint8_t(10);
//     attributeConfig1.size = 1;

//     AttributeConfig attributeConfig2;
//     attributeConfig2.attributeId = 1;
//     attributeConfig2.dataType = ZCL_CHAR_STRING_ATTRIBUTE_TYPE;
//     attributeConfig2.value = std::string("Hello");

//     AttributeConfig attributeConfig3;
//     attributeConfig3.attributeId = 2;
//     attributeConfig3.dataType = ZCL_INT32U_ATTRIBUTE_TYPE;
//     attributeConfig3.value = std::uint32_t(1000000);

//     // Create Event configurations
//     EventConfig eventConfig1;
//     eventConfig1.eventId = 0;
//     // Set necessary attributes for eventConfig1

//     EventConfig eventConfig2;
//     eventConfig2.eventId = 1;
//     // Set necessary attributes for eventConfig2

//     // Create Command configurations
//     CommandConfig commandConfig1;
//     commandConfig1.commandId = 0;
//     commandConfig1.mask = 1;
//     // Set necessary attributes for commandConfig1

//     CommandConfig commandConfig2;
//     commandConfig2.commandId = 1;
//     // Set necessary attributes for commandConfig2

//     // Create Cluster configurations
//     ClusterConfig clusterConfig1;
//     clusterConfig1.clusterId = 0;
//     clusterConfig1.attributeConfigs.push_back(attributeConfig1);
//     clusterConfig1.attributeConfigs.push_back(attributeConfig2);
//     clusterConfig1.attributeConfigs.push_back(attributeConfig3);
//     clusterConfig1.eventConfigs.push_back(eventConfig1);
//     clusterConfig1.eventConfigs.push_back(eventConfig2);
//     clusterConfig1.commandConfigs.push_back(commandConfig1);
//     clusterConfig1.commandConfigs.push_back(commandConfig2);
//     clusterConfig1.mask = 0;

//     // Create another Cluster configuration
//     ClusterConfig clusterConfig2;
//     clusterConfig2.clusterId = 1;
//     clusterConfig2.attributeConfigs.push_back(attributeConfig1);
//     clusterConfig2.attributeConfigs.push_back(attributeConfig2);
//     clusterConfig2.eventConfigs.push_back(eventConfig1);
//     clusterConfig2.eventConfigs.push_back(eventConfig2);
//     clusterConfig2.commandConfigs.push_back(commandConfig1);
//     clusterConfig2.commandConfigs.push_back(commandConfig2);
//     clusterConfig2.mask = 0;

//     // Create Endpoint configuration
//     EndpointConfig endpointConfig;
//     endpointConfig.clusterConfigs.push_back(clusterConfig1);
//     endpointConfig.clusterConfigs.push_back(clusterConfig2);

//     return endpointConfig;
// }

// int main() {
//     Node& node = Node::getInstance();

//     EndpointConfig endpointConfig0 = createEndpointConfig();
//     EndpointConfig endpointConfig1 = createEndpointConfig();
//     EndpointConfig endpointConfig2 = createEndpointConfig();
//     EndpointConfig endpointConfig3 = createEndpointConfig();
//     EndpointConfig endpointConfig4 = createEndpointConfig();
//     EndpointConfig endpointConfig5 = createEndpointConfig();

//     node.addEndpoint(endpointConfig0);
//     node.addEndpoint(endpointConfig1);
//     node.addEndpoint(endpointConfig2);
//     node.addEndpoint(endpointConfig3);
//     node.addEndpoint(endpointConfig4);

//     node.print();

//     node.removeEndpoint(2);

//     node.print();

//     node.removeEndpoint(1);

//     node.print();

//     node.removeEndpoint(4);

//     node.print();

//     node.addEndpoint(endpointConfig5);

//     node.print();

//     return 0;
// }
