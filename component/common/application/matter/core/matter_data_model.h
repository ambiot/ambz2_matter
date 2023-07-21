#pragma once

#include "af-types.h"
#include <variant>
#include <vector>
#include <app-common/zap-generated/attribute-type.h>
#include <app/util/attribute-metadata.h>

class Node;
class Endpoint;
class Cluster;
class Attribute;
class Command;
class Event;

typedef std::variant<uint8_t, uint8_t*, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t, float> AttributeValue;

// Configurations
struct AttributeConfig {
    std::uint32_t attributeId;
    std::uint8_t dataType; /* EmberAfAttributeType in string format */
    // AttributeValue value = std::int32_t(0); /* default value, use ZAP_DEFAULT_EMPTY() */ // this will be default value, but the actual value should we use the one inside metadata?
    EmberAfDefaultOrMinMaxAttributeValue value;
    std::uint16_t size; /* attributeSize, use ZAP_TYPE(type) */
    std::uint8_t mask = 0; /* attribute flag */

    AttributeConfig(uint32_t attributeId, uint8_t dataType, EmberAfDefaultOrMinMaxAttributeValue value, uint16_t size, uint8_t mask) : attributeId(attributeId), dataType(dataType), value(value), size(size), mask(mask) {}
};

struct EventConfig {
    // Add necessary attributes and methods for configuration
    std::uint32_t eventId;

    EventConfig(uint32_t eventId) : eventId(eventId) {}
};

struct CommandConfig {
    // Add necessary attributes and methods for configuration
    std::uint32_t commandId;
    std::uint8_t mask = 0; /* command flag */
    // callback?

    CommandConfig(uint32_t commandId, uint8_t mask) : commandId(commandId), mask(mask) {}
};

struct ClusterConfig {
public:
    // Cluster configuration attributes
    std::uint32_t clusterId;
    std::vector<AttributeConfig> attributeConfigs;
    std::vector<EventConfig> eventConfigs;
    std::vector<CommandConfig> commandConfigs;
    std::vector<EmberAfGenericClusterFunction> functionConfigs;
    std::uint8_t mask = 0; /* cluster flag */
};

class EndpointConfig {
public:
    // Endpoint configuration attributes
    std::vector<ClusterConfig> clusterConfigs;
};

// Attribute class
class Attribute {
public:
    Attribute(Cluster* cluster, AttributeConfig attributeConfig) : 
        attributeId(attributeConfig.attributeId),
        attributeSize(attributeConfig.size),
        attributeType(attributeConfig.dataType),
        attributeMask(attributeConfig.mask),
        parentCluster(cluster),
        defaultValue(attributeConfig.value) {
            // Change attributeType to base type
            // Assign value to be of base type with default value
            // TODO: get value from NVS
            switch(getAttributeBaseType())
            {
            case ZCL_INT8U_ATTRIBUTE_TYPE:
                value = uint8_t(attributeConfig.value.defaultValue);
                break;
            case ZCL_INT16U_ATTRIBUTE_TYPE:
                value = uint16_t(attributeConfig.value.defaultValue);
                break;
            case ZCL_INT32U_ATTRIBUTE_TYPE:
                value = uint32_t(attributeConfig.value.defaultValue);
                break;
            case ZCL_INT64U_ATTRIBUTE_TYPE:
                value = uint64_t(attributeConfig.value.defaultValue);
                break;
            case ZCL_INT8S_ATTRIBUTE_TYPE:
                value = int8_t(attributeConfig.value.defaultValue);
                break;
            case ZCL_INT16S_ATTRIBUTE_TYPE:
                value = int16_t(attributeConfig.value.defaultValue);
                break;
            case ZCL_INT32S_ATTRIBUTE_TYPE:
                value = int32_t(attributeConfig.value.defaultValue);
                break;
            case ZCL_INT64S_ATTRIBUTE_TYPE:
                value = int64_t(attributeConfig.value.defaultValue);
                break;
            case ZCL_SINGLE_ATTRIBUTE_TYPE:
                value = float(attributeConfig.value.defaultValue);
                break;
            case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
            case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
            case ZCL_ARRAY_ATTRIBUTE_TYPE:
            case ZCL_STRUCT_ATTRIBUTE_TYPE:
                value = (uint8_t*)(attributeConfig.value.ptrToDefaultValue);
                break;
            default:
                ChipLogError(DeviceLayer, "Unknown attribute type, unable to assign value");
            }
        }

    chip::AttributeId getAttributeId() const;

    uint16_t getAttributeSize() const;

    EmberAfAttributeType getAttributeType() const;

    EmberAfAttributeMask getAttributeMask() const;

    EmberAfDefaultOrMinMaxAttributeValue getAttributeDefaultValue() const;

    EmberAfAttributeType getAttributeBaseType();

    AttributeValue getValue() const;

    void setValue(AttributeValue & newValue);

    void print(int indent = 0) const;
private:
    chip::AttributeId attributeId;
    uint16_t attributeSize;
    EmberAfAttributeType attributeType;
    EmberAfAttributeMask attributeMask;
    Cluster* parentCluster;
    EmberAfDefaultOrMinMaxAttributeValue defaultValue;
    AttributeValue value;
};

// Event class
class Event {
public:
    Event(Cluster* cluster, EventConfig eventConfig) :
        eventId(eventConfig.eventId),
        parentCluster(cluster) {}

    chip::EventId getEventId() const;

    void print(int indent = 0) const;

private:
    chip::EventId eventId;
    Cluster* parentCluster;
};

/** Command flags */
/** The command is not a standard command */
#define COMMAND_MASK_CUSTOM (0x01)
/** The command is client generated */
#define COMMAND_MASK_ACCEPTED (0x02)
/** The command is server generated */
#define COMMAND_MASK_GENERATED (0x04)

// Command class
class Command {
public:
    Command(Cluster* cluster, CommandConfig commandConfig) :
        commandId(commandConfig.commandId),
        commandFlag(commandConfig.mask),
        parentCluster(cluster) {}

    chip::CommandId getCommandId() const;

    int getFlag() const;

    void print(int indent = 0) const;

private:
    chip::CommandId commandId;
    int commandFlag;
    Cluster* parentCluster;
};

// Cluster class
class Cluster {
public:
    friend class Endpoint;

    Cluster(Endpoint* endpoint, ClusterConfig clusterConfig) :
        clusterId(clusterConfig.clusterId),
        clusterMask(clusterConfig.mask),
        parentEndpoint(endpoint) {};

    chip::ClusterId getClusterId() const;

    EmberAfClusterMask getClusterMask() const;

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

    void print(int indent = 0) const;

private:
    chip::ClusterId clusterId;
    EmberAfClusterMask clusterMask;
    Endpoint* parentEndpoint;

    std::vector<Attribute> attributes;
    std::vector<Event> events;
    std::vector<Command> acceptedCommands;
    std::vector<Command> generatedCommands;
    std::vector<EmberAfGenericClusterFunction> functions; // TODO
};

// Endpoint class
class Endpoint {
public:
    Endpoint(Node* node, chip::EndpointId endpointId) : 
        endpointId(endpointId),
        parentEndpointId(0xFFFF /* chip::kInvalidEndpointId */),
        parentNode(node) {}

    chip::EndpointId getEndpointId() const;

    Cluster *getCluster(chip::ClusterId clusterId);

    void addCluster(ClusterConfig & clusterConfig);

    void addCluster(const Cluster& cluster);

    void removeCluster(chip::ClusterId clusterId);

    chip::EndpointId getParentEndpointId() const;

    void setParentEndpointId(chip::EndpointId parentEndpointId);

    void enableEndpoint(chip::Span<const EmberAfDeviceType> deviceTypeList);

    void disableEndpoint();

    void print(int indent = 0) const;
private:
    chip::EndpointId endpointId;
    chip::EndpointId parentEndpointId;
    Node* parentNode;
    chip::DataVersion *dataVersion = nullptr;
    std::vector<Cluster> clusters;
    EmberAfEndpointType *endpointMetadata;

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
class Node {
public:
    static Node& getInstance();

    Endpoint *getEndpoint(chip::EndpointId endpointId);

    chip::EndpointId getNextEndpointId() const;

    void addEndpoint(const EndpointConfig& endpointConfig);

    void removeEndpoint(chip::EndpointId endpointId);

    void enableAllEndpoints();

    void print() const;

private:
    Node() {} /* singleton instance */
    chip::EndpointId endpointCount = 0;
    chip::EndpointId nextEndpointId = 0;
    std::vector<Endpoint> endpoints;
};
