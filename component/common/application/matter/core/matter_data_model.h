#pragma once

#include "af-types.h"
#include <variant>
#include <vector>
#include <app-common/zap-generated/attribute-type.h>

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
    std::uint8_t mask; /* command flag */
    // callback?
};

struct ClusterConfig {
public:
    // Cluster configuration attributes
    std::uint32_t clusterId;
    std::vector<AttributeConfig> attributeConfigs;
    std::vector<EventConfig> eventConfigs;
    std::vector<CommandConfig> commandConfigs;
    std::uint8_t mask; /* cluster flag */
};

class EndpointConfig {
public:
    // Endpoint configuration attributes
    std::vector<ClusterConfig> clusterConfigs;
};

// Attribute class
class Attribute {
public:
    Attribute(Cluster* cluster, AttributeConfig attributeConfig) : cluster(cluster), value(attributeConfig.value), metadata{static_cast<uint32_t>(0), attributeConfig.attributeId, attributeConfig.size, attributeConfig.dataType, attributeConfig.mask} {}

    template<typename T>
    T getValue() const;

    template<typename T>
    void setValue(const T& newValue);

    std::uint32_t getAttributeId() const;

    std::uint16_t getAttributeSize() const;

    std::uint8_t getAttributeType() const;

    std::uint8_t getAttributeMask() const;

    void print(int indent = 0) const;

private:
    Cluster* cluster;
    AttributeValue value;
    EmberAfAttributeMetadata metadata;
};

// Event class
class Event {
public:
    Event(Cluster* cluster, int eventId);

    int getEventId() const;

    void print(int indent = 0) const;

private:
    Cluster* cluster;
    int eventId;
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
    Command(Cluster* cluster, int commandId);

    int getCommandId() const;

    int getFlag() const;

    void print(int indent = 0) const;

private:
    Cluster* cluster;
    int commandId;
    int flag;
};

// Cluster class
class Cluster {
public:
    Cluster(Endpoint* endpoint, std::uint32_t clusterId, std::uint8_t mask) : endpoint(endpoint), metadata{clusterId, nullptr, 0, 0, mask, nullptr, nullptr, nullptr, nullptr} {}

    int getClusterId() const;

    void addAttribute(const Attribute& attribute);

    void removeAttribute(int attributeId);

    void addEvent(const Event& event);

    void removeEvent(int eventId);

    void addCommand(const Command& command);

    void removeCommand(int commandId);

    void print(int indent = 0) const;

private:
    Endpoint* endpoint;
    EmberAfCluster metadata;
    // int clusterId;
    std::vector<Attribute> attributes;
    std::vector<Event> events;
    std::vector<Command> commands;
};

// Endpoint class
class Endpoint {
public:
    Endpoint(Node* node, int endpointId);

    void addCluster(const Cluster& cluster);

    int getEndpointId() const;

    int getNumClusters() const;

    int getParentEndpointId() const;

    void setParentEndpointId(int parentEndpointId);

    void print(int indent = 0) const;

private:
    Node* node;
    int endpointId;
    int parentEndpointId;
    std::vector<Cluster> clusters;
};

// Node class
class Node {
public:
    static Node& getInstance();

    void addEndpoint(const EndpointConfig& endpointConfig);

    void removeEndpoint(int endpointId);

    void print() const;

private:
    Node() {} /* singleton instance */
    int endpointCount = 0;
    int nextEndpointId = 0;
    std::vector<Endpoint> endpoints;
};
