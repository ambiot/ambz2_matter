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
    Attribute(Cluster* cluster, AttributeConfig attributeConfig) : cluster(cluster), value(attributeConfig.value), metadata{static_cast<uint32_t>(0), attributeConfig.attributeId, attributeConfig.size, attributeConfig.dataType, attributeConfig.mask} {}

    template<typename T>
    T getValue() const;

    template<typename T>
    void setValue(const T& newValue);

    EmberAfDefaultOrMinMaxAttributeValue getAttributeDefaultValue() const;
    chip::AttributeId getAttributeId() const;

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
    Event(Cluster* cluster, EventConfig eventConfig) : eventId(eventConfig.eventId) {};

    chip::EventId getEventId() const;

    void print(int indent = 0) const;

private:
    Cluster* cluster;
    chip::EventId eventId;
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
    Command(Cluster* cluster, CommandConfig commandConfig) : commandId(commandConfig.commandId) {};

    chip::CommandId getCommandId() const;

    int getFlag() const;

    void print(int indent = 0) const;

private:
    Cluster* cluster;
    chip::CommandId commandId;
    int flag;
};

// Cluster class
class Cluster {
public:
    // change to pass in cluster config or change attribute constructor?
    Cluster(Endpoint* endpoint, ClusterConfig clusterConfig) : endpoint(endpoint), metadata{clusterConfig.clusterId, nullptr, 0, 0, clusterConfig.mask, nullptr, nullptr, nullptr, nullptr} {}

#if 0
    Cluster(const Cluster & other) : endpoint(other.endpoint), metadata(other.metadata) {
        // copy metadata values directly
        metadata.clusterId = other.metadata.clusterId;
        metadata.attributeCount = other.metadata.attributeCount;
        metadata.clusterSize = other.metadata.clusterSize;
        metadata.mask = other.metadata.mask;

        std::vector<EmberAfAttributeMetadata> newAttributes;
        newAttributes.reserve(metadata.attributeCount);     // Reserve space for existing attributes

        // Copy existing attribute metadata to new vector
        if (metadata.attributeCount > 0) {
            newAttributes.assign(metadata.attributes, metadata.attributes + metadata.attributeCount);
        }
    }
#endif

    Attribute *getAttribute(chip::AttributeId attributeId);

    Event *getEvent(chip::EventId eventId); 

    Command *getCommand(chip::CommandId commandId);

    chip::ClusterId getClusterId() const;

    const EmberAfAttributeMetadata *getAttributeMetadataList() const;

    uint16_t getAttributeCount() const;

    uint16_t getClusterSize() const;

    EmberAfClusterMask getClusterMask() const;

    const EmberAfGenericClusterFunction *getGenericClusterFunctionList() const;

    const chip::CommandId *getAcceptedCommandList() const;

    const chip::CommandId *getGeneratedCommandList() const;

    const chip::EventId *getEventList() const;

    uint16_t getEventCount() const;

    void addAttribute(AttributeConfig attributeConfig);

    void addAttribute(const Attribute& attribute);

    void removeAttribute(chip::AttributeId attributeId);

    void addEvent(EventConfig eventConfig);

    void addEvent(const Event& event);

    void removeEvent(chip::EventId eventId);

    void addCommand(CommandConfig commandConfig);

    void addCommand(const Command& command);

    void removeCommand(chip::CommandId commandId);

    void print(int indent = 0) const;

    const EmberAfCluster *getMetadata() const {return &metadata;}
private:
    Endpoint* endpoint;
    std::vector<Attribute> attributes;
    std::vector<Event> events;
    std::vector<Command> commands;
    EmberAfCluster metadata;
};

// Endpoint class
class Endpoint {
public:
    Endpoint(Node* node, chip::EndpointId endpointId) : node(node), endpointId(endpointId), parentEndpointId(0xFFFF /* chip::kInvalidEndpointId */), metadata{NULL, 0, 0} {}

#if 1
    Endpoint(const Endpoint & other) : node(other.node), endpointId(other.endpointId), parentEndpointId(other.parentEndpointId), metadata(other.metadata) {
        // Copy metadata values directly
        metadata.clusterCount = other.metadata.clusterCount;
        metadata.endpointSize = other.metadata.endpointSize;

        std::vector<EmberAfCluster> newClusters;
        newClusters.reserve(metadata.clusterCount);    // Reserve space for existing clusters

        // Copy the existing cluster metadata to the new vector
        if (metadata.clusterCount > 0) {
            newClusters.assign(metadata.cluster, metadata.cluster + metadata.clusterCount);
        }
        printf("\r\n%s %d size: %d\r\n", __FUNCTION__, __LINE__, other.metadata.cluster->attributes->size);

        metadata.cluster = newClusters.data();
        isDynamicallyAllocated = true;
        printf("\r\n%s %d size: %d\r\n", __FUNCTION__, __LINE__, metadata.cluster->attributes->size);
    }
#endif

    // Destructor TODO: since we don't use new keyword, do we even need this?
    ~Endpoint() {
        if (isDynamicallyAllocated) {
            delete[] metadata.cluster;
        }
    }

    Cluster *getCluster(chip::ClusterId clusterId);

    void addCluster(ClusterConfig & clusterConfig);

    void addCluster(const Cluster& cluster);

    void removeCluster(chip::ClusterId clusterId);

    chip::EndpointId getEndpointId() const;

    const EmberAfCluster *getMetadataClusterList() const;

    uint8_t getClusterCount() const;

    uint16_t getEndpointSize() const;

    chip::EndpointId getParentEndpointId() const;

    void setParentEndpointId(chip::EndpointId parentEndpointId);

    void print(int indent = 0) const;

    EmberAfEndpointType metadata;
private:
    Node* node;
    chip::EndpointId endpointId;
    chip::EndpointId parentEndpointId;
    std::vector<Cluster> clusters;
    bool isDynamicallyAllocated = false;    // to keep track if metadata.cluster is dynamically allocated
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
