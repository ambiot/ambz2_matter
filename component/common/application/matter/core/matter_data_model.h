#pragma once

#include "af-types.h"
#include "endpoint_config.h"
#include <variant>
#include <vector>
#include <app-common/zap-generated/attribute-type.h>
#include <app/util/attribute-metadata.h>

using namespace ::chip;

class Node;
class Endpoint;
class Cluster;
class Attribute;
class Command;
class Event;

// Use variant to represent the different data types supported
typedef std::variant<uint8_t, uint8_t*, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t, float> AttributeValue;

// Configurations
struct AttributeConfig
{
    std::uint32_t attributeId;
    std::uint8_t dataType; /* use ZAP_TYPE(type) */
    EmberAfDefaultOrMinMaxAttributeValue value; /* use ZAP_EMPTY(), ZAP_SIMPLE(), etc */
    std::uint16_t size;
    std::uint8_t mask = 0; /* attribute flag */
    AttributeConfig(uint32_t attributeId, uint8_t dataType, EmberAfDefaultOrMinMaxAttributeValue value, uint16_t size, uint8_t mask) : attributeId(attributeId), dataType(dataType), value(value), size(size), mask(mask) {}
};

struct EventConfig
{
    std::uint32_t eventId;
    EventConfig(uint32_t eventId) : eventId(eventId) {}
};

struct CommandConfig
{
    std::uint32_t commandId;
    std::uint8_t mask = 0; /* command flag */
    CommandConfig(uint32_t commandId, uint8_t mask) : commandId(commandId), mask(mask) {}
};

struct ClusterConfig
{
public:
    std::uint32_t clusterId;
    std::vector<AttributeConfig> attributeConfigs;
    std::vector<EventConfig> eventConfigs;
    std::vector<CommandConfig> commandConfigs;
    std::vector<EmberAfGenericClusterFunction> functionConfigs;
    std::uint8_t mask = 0; /* cluster flag */
};

class EndpointConfig
{
public:
    std::vector<ClusterConfig> clusterConfigs;
};

// Attribute class
class Attribute
{
public:
    Attribute(chip::ClusterId clusterId, chip::EndpointId endpointId, AttributeConfig attributeConfig) : 
        attributeId(attributeConfig.attributeId),
        attributeSize(attributeConfig.size),
        attributeType(attributeConfig.dataType),
        attributeMask(attributeConfig.mask),
        parentClusterId(clusterId),
        parentEndpointId(endpointId),
        defaultValue(attributeConfig.value)
    {
        // Retrieve value from NVS if available, else
        // assign value to be of base type with default value from config
        switch(getAttributeBaseType())
        {
        case ZCL_INT8U_ATTRIBUTE_TYPE:
        case ZCL_BOOLEAN_ATTRIBUTE_TYPE:
            uint8_t value_uint8_t;
            if (retrieveValue(&value_uint8_t, sizeof(uint8_t)) == CHIP_NO_ERROR)
            {
                value = value_uint8_t;
            }
            else
            {
                value = uint8_t(attributeConfig.value.defaultValue);
            }
            break;
        case ZCL_INT16U_ATTRIBUTE_TYPE:
            uint16_t value_uint16_t;
            if (retrieveValue((uint8_t*) &value_uint16_t, sizeof(uint16_t)) == CHIP_NO_ERROR)
            {
                value = value_uint16_t;
            }
            else
            {
                value = uint16_t(attributeConfig.value.defaultValue);
            }
            break;
        case ZCL_INT32U_ATTRIBUTE_TYPE:
            uint32_t value_uint32_t;
            if (retrieveValue((uint8_t*) &value_uint32_t, sizeof(uint32_t)) == CHIP_NO_ERROR)
            {
                value = value_uint32_t;
            }
            else
            {
                value = uint32_t(attributeConfig.value.defaultValue);
            }
            break;
        case ZCL_INT64U_ATTRIBUTE_TYPE:
            uint64_t value_uint64_t;
            if (retrieveValue((uint8_t*) &value_uint64_t, sizeof(uint64_t)) == CHIP_NO_ERROR)
            {
                value = value_uint64_t;
            }
            else
            {
                value = uint64_t(attributeConfig.value.defaultValue);
            }
            break;
        case ZCL_INT8S_ATTRIBUTE_TYPE:
            int8_t value_int8_t;
            if (retrieveValue((uint8_t*) &value_int8_t, sizeof(int8_t)) == CHIP_NO_ERROR)
            {
                value = value_int8_t;
            }
            else
            {
                value = int8_t(attributeConfig.value.defaultValue);
            }
            break;
        case ZCL_INT16S_ATTRIBUTE_TYPE:
            int16_t value_int16_t;
            if (retrieveValue((uint8_t*) &value_int16_t, sizeof(int16_t)) == CHIP_NO_ERROR)
            {
                value = value_int16_t;
            }
            else
            {
                value = int16_t(attributeConfig.value.defaultValue);
            }
            break;
        case ZCL_INT32S_ATTRIBUTE_TYPE:
            int32_t value_int32_t;
            if (retrieveValue((uint8_t*) &value_int32_t, sizeof(int32_t)) == CHIP_NO_ERROR)
            {
                value = value_int32_t;
            }
            else
            {
                value = int32_t(attributeConfig.value.defaultValue);
            }
            break;
        case ZCL_INT64S_ATTRIBUTE_TYPE:
            int64_t value_int64_t;
            if (retrieveValue((uint8_t*) &value_int64_t, sizeof(int64_t)) == CHIP_NO_ERROR)
            {
                value = value_int64_t;
            }
            else
            {
                value = int64_t(attributeConfig.value.defaultValue);
            }
            break;
        case ZCL_SINGLE_ATTRIBUTE_TYPE:
            float value_float;
            if (retrieveValue((uint8_t*) &value_float, sizeof(float)) == CHIP_NO_ERROR)
            {
                value = value_float;
            }
            else
            {
                value = float(attributeConfig.value.defaultValue);
            }
            break;
        case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
        case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
        case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
            if (retrieveValue(valueBuffer, attributeSize) != CHIP_NO_ERROR)
            {
                memset(valueBuffer, 0, ATTRIBUTE_LARGEST);
                memcpy(valueBuffer, attributeConfig.value.ptrToDefaultValue, attributeSize);
            }
            break;
        case ZCL_ARRAY_ATTRIBUTE_TYPE:
        case ZCL_STRUCT_ATTRIBUTE_TYPE:
            break;
        default:
            ChipLogError(DeviceLayer, "Unknown attribute type, unable to assign value for attributeId: %d", attributeId);
            break;
        }
    }

    // Copy constructor to perform a deep copy of string values
    Attribute(const Attribute & other) :
        attributeId(other.attributeId),
        attributeSize(other.attributeSize),
        attributeType(other.attributeType),
        attributeMask(other.attributeMask),
        parentClusterId(other.parentClusterId),
        parentEndpointId(other.parentEndpointId),
        defaultValue(other.defaultValue),
        value(other.value)
    {
        switch (getAttributeBaseType())
        {
        case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
        case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
        case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
            memcpy(valueBuffer, other.valueBuffer, ATTRIBUTE_LARGEST);
            break;
        default:
            break;
        }
    }

    chip::AttributeId getAttributeId() const;
    chip::ClusterId getParentClusterId() const;
    chip::ClusterId getParentEndpointId() const;
    uint16_t getAttributeSize() const;
    EmberAfAttributeType getAttributeType() const;
    EmberAfAttributeMask getAttributeMask() const;
    EmberAfDefaultOrMinMaxAttributeValue getAttributeDefaultValue() const;
    EmberAfAttributeType getAttributeBaseType() const;
    void getValue(uint8_t *buffer) const;
    void setValue(uint8_t *buffer);
    void persistValue(uint8_t *buffer, size_t size);
    CHIP_ERROR retrieveValue(uint8_t *buffer, size_t size);

private:
    chip::AttributeId attributeId;
    uint16_t attributeSize;
    EmberAfAttributeType attributeType;
    EmberAfAttributeMask attributeMask;
    chip::ClusterId parentClusterId;
    chip::EndpointId parentEndpointId;
    EmberAfDefaultOrMinMaxAttributeValue defaultValue;
    AttributeValue value;
    uint8_t valueBuffer[ATTRIBUTE_LARGEST];
};

// Event class
class Event
{
public:
    Event(chip::ClusterId clusterId, chip::EndpointId endpointId, EventConfig eventConfig) :
        eventId(eventConfig.eventId),
        parentClusterId(clusterId),
        parentEndpointId(endpointId) {}
    chip::EventId getEventId() const;
    chip::ClusterId getParentClusterId() const;

private:
    chip::EventId eventId;
    chip::ClusterId parentClusterId;
    chip::EndpointId parentEndpointId;
};

/** Command flags */
/** The command is not a standard command */
#define COMMAND_MASK_CUSTOM (0x01)
/** The command is client generated */
#define COMMAND_MASK_ACCEPTED (0x02)
/** The command is server generated */
#define COMMAND_MASK_GENERATED (0x04)

// Command class
class Command
{
public:
    Command(chip::ClusterId clusterId, chip::EndpointId endpointId, CommandConfig commandConfig) :
        commandId(commandConfig.commandId),
        commandFlag(commandConfig.mask),
        parentClusterId(clusterId),
        parentEndpointId(endpointId) {}
    chip::CommandId getCommandId() const;
    chip::ClusterId getParentClusterId() const;
    uint8_t getFlag() const;

private:
    chip::CommandId commandId;
    uint8_t commandFlag;
    chip::ClusterId parentClusterId;
    chip::EndpointId parentEndpointId;
};

// Cluster class
class Cluster
{
public:
    friend class Endpoint;

    Cluster(chip::EndpointId endpointId, ClusterConfig clusterConfig) :
        clusterId(clusterConfig.clusterId),
        clusterMask(clusterConfig.mask),
        parentEndpointId(endpointId) {};
    chip::ClusterId getClusterId() const;
    EmberAfClusterMask getClusterMask() const;
    chip::EndpointId getParentEndpointId() const;
    Attribute *getAttribute(chip::AttributeId attributeId);
    uint32_t getAttributeCount() const;
    Event *getEvent(chip::EventId eventId); 
    uint32_t getEventCount() const;
    Command *getAcceptedCommand(chip::CommandId commandId);
    uint32_t getAcceptedCommandCount() const;
    Command *getGeneratedCommand(chip::CommandId commandId);
    uint32_t getGeneratedCommandCount() const;
    uint32_t getFunctionCount() const;
    void addAttribute(AttributeConfig attributeConfig);
    void addAttribute(const Attribute& attribute);
    void removeAttribute(chip::AttributeId attributeId);
    void addEvent(EventConfig eventConfig);
    void addEvent(const Event& event);
    void removeEvent(chip::EventId eventId);
    void addAcceptedCommand(CommandConfig commandConfig);
    void addAcceptedCommand(const Command& command);
    void addGeneratedCommand(CommandConfig commandConfig);
    void addGeneratedCommand(const Command& command);
    void removeAcceptedCommand(chip::CommandId commandId);
    void removeGeneratedCommand(chip::CommandId commandId);
    void addFunction(const EmberAfGenericClusterFunction function);
    void removeFunction();

private:
    chip::ClusterId clusterId;
    EmberAfClusterMask clusterMask;
    chip::EndpointId parentEndpointId;
    std::vector<Attribute> attributes;
    std::vector<Event> events;
    std::vector<Command> acceptedCommands;
    std::vector<Command> generatedCommands;
    std::vector<EmberAfGenericClusterFunction> functions;
};

// Endpoint class
class Endpoint
{
public:
    friend class Node;

    Endpoint(Node* node, chip::EndpointId endpointId, uint16_t endpointCount, Span<const EmberAfDeviceType> deviceTypeList) :
        endpointId(endpointId),
        endpointIndex(endpointCount),
        parentEndpointId(0xFFFF /* chip::kInvalidEndpointId */),
        parentNode(node),
        deviceTypeList(deviceTypeList) {}
    chip::EndpointId getEndpointId() const;
    Cluster *getCluster(chip::ClusterId clusterId);
    void addCluster(ClusterConfig & clusterConfig);
    void addCluster(const Cluster& cluster);
    void removeCluster(chip::ClusterId clusterId);
    chip::EndpointId getParentEndpointId() const;   // This returns the endpointId of the previous endpoint, not to be confused with Cluster::getParentEndpointId
    void setParentEndpointId(chip::EndpointId parentEndpointId);
    void enableEndpoint();
    void disableEndpoint();

private:
    chip::EndpointId endpointId;
    uint16_t endpointIndex;
    chip::EndpointId parentEndpointId;
    Node* parentNode;
    chip::DataVersion *dataVersion = nullptr;
    Span<const EmberAfDeviceType> deviceTypeList;
    EmberAfEndpointType *endpointMetadata;
    std::vector<Cluster> clusters;
    bool enabled = false;

    // Garbage collectors
    // Store dynamic allocated objects here when endpoint is enabled, then delete it when disabled
    std::vector<EmberAfCluster*> clusterGarbageCollector;
    std::vector<EmberAfAttributeMetadata*> attributeGarbageCollector;
    std::vector<EmberAfGenericClusterFunction*> functionGarbageCollector;
    std::vector<chip::CommandId*> acceptedCommandGarbageCollector;
    std::vector<chip::CommandId*> generatedCommandGarbageCollector;
    std::vector<chip::EventId*> eventGarbageCollector;
};

// Node class
class Node
{
public:
    static Node& getInstance();
    Endpoint *getEndpoint(chip::EndpointId endpointId);
    chip::EndpointId getNextEndpointId() const;
    chip::EndpointId addEndpoint(const EndpointConfig& endpointConfig, Span<const EmberAfDeviceType> deviceTypeList);
    void removeEndpoint(chip::EndpointId endpointId);
    void enableAllEndpoints();

private:
    Node() {} /* singleton instance */
    chip::EndpointId endpointCount = 0;
    chip::EndpointId nextEndpointId = 0;
    std::vector<Endpoint> endpoints;
};
