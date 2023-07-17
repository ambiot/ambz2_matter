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

typedef std::variant<std::uint8_t, std::int8_t, std::uint16_t, std::int16_t,
                                   std::uint32_t, std::int32_t, std::uint64_t, std::int64_t,
                                   std::string> AttributeValue;

// Configurations
struct AttributeConfig {
    std::uint32_t attributeId;
    std::uint8_t dataType; /* EmberAfAttributeType in string format */
    AttributeValue value = std::int32_t(0); /* default value, use ZAP_DEFAULT_EMPTY() */ // this will be default value, but the actual value should we use the one inside metadata?
    std::uint16_t size; /* attributeSize, use ZAP_TYPE(type) */
    std::uint8_t mask = 0; /* attribute flag */
};

struct EventConfig {
    // Add necessary attributes and methods for configuration
    std::uint32_t eventId;
};

struct CommandConfig {
    // Add necessary attributes and methods for configuration
    std::uint32_t commandId;
    std::uint8_t mask = 0; /* command flag */
    // callback?
};

struct ClusterConfig {
public:
    // Cluster configuration attributes
    std::uint32_t clusterId;
    std::vector<AttributeConfig> attributeConfigs;
    std::vector<EventConfig> eventConfigs;
    std::vector<CommandConfig> commandConfigs;
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
        value(attributeConfig.value) {};

    chip::AttributeId getAttributeId() const;

    uint16_t getAttributeSize() const;

    EmberAfAttributeType getAttributeType() const;

    EmberAfAttributeMask getAttributeMask() const;

    template<typename T>
    T getValue() const;

    template<typename T>
    void setValue(const T& newValue);

    void print(int indent = 0) const;
private:
    chip::AttributeId attributeId;
    uint16_t attributeSize;
    EmberAfAttributeType attributeType;
    EmberAfAttributeMask attributeMask;
    Cluster* parentCluster;
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
    Cluster(Endpoint* endpoint, ClusterConfig clusterConfig) :
        clusterId(clusterConfig.clusterId),
        clusterMask(clusterConfig.mask),
        parentEndpoint(endpoint) {};

    chip::ClusterId getClusterId() const;

    EmberAfClusterMask getClusterMask() const;

    Attribute *getAttribute(chip::AttributeId attributeId);

    Event *getEvent(chip::EventId eventId); 

    Command *getAcceptedCommand(chip::CommandId commandId);

    Command *getGeneratedCommand(chip::CommandId commandId);

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

    void print(int indent = 0) const;
private:
    chip::EndpointId endpointId;
    chip::EndpointId parentEndpointId;
    Node* parentNode;

    std::vector<Cluster> clusters;
};

// Node class
class Node {
public:
    static Node& getInstance();

    Endpoint *getEndpoint(chip::EndpointId endpointId);

    void addEndpoint(const EndpointConfig& endpointConfig);

    void removeEndpoint(chip::EndpointId endpointId);

    void print() const;

private:
    Node() {} /* singleton instance */
    chip::EndpointId endpointCount = 0;
    chip::EndpointId nextEndpointId = 0;
    std::vector<Endpoint> endpoints;
};