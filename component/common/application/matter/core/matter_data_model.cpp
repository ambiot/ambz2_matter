#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include "matter_data_model.h"
#include <app-common/zap-generated/attribute-type.h>

/*                  Attributes                  */
template<typename T>
T Attribute::getValue() const {
    if (std::holds_alternative<T>(value)) {
        return std::get<T>(value);
    } else {
        std::cout << "Unknown data type" << std::endl;
        return T();
    }
}

template<typename T>
void Attribute::setValue(const T& newValue) {
    value = newValue;
}

std::uint32_t Attribute::getAttributeId() const {
    return metadata.attributeId;
}

std::uint16_t Attribute::getAttributeSize() const {
    return metadata.size;
}

std::uint8_t Attribute::getAttributeType() const {
    return metadata.attributeType;
}

std::uint8_t Attribute::getAttributeMask() const {
    return metadata.mask;
}

void Attribute::print(int indent) const {
    std::string indentation(indent, ' ');
    if (metadata.attributeType == ZCL_INT8U_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint8_t>()) << std::endl;
    } else if (metadata.attributeType == ZCL_INT16U_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint16_t>()) << std::endl;
    } else if (metadata.attributeType == ZCL_INT32U_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint32_t>()) << std::endl;
    } else if (metadata.attributeType == ZCL_INT64U_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint64_t>()) << std::endl;
    } else if (metadata.attributeType == ZCL_INT8S_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int8_t>()) << std::endl;
    } else if (metadata.attributeType == ZCL_INT16S_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int16_t>()) << std::endl;
    } else if (metadata.attributeType == ZCL_INT32S_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int32_t>()) << std::endl;
    } else if (metadata.attributeType == ZCL_INT64S_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int64_t>()) << std::endl;
    } else if (metadata.attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << getValue<std::string>() << std::endl;
    } else {
        // Handle the case where the data type doesn't match
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value type undefined" << std::endl;
    }
}

/*                  Events                  */
Event::Event(Cluster* cluster, int eventId) : cluster(cluster), eventId(eventId) {}

int Event::getEventId() const {
    return eventId;
}

void Event::print(int indent) const {
    std::string indentation(indent, ' ');
    std::cout << indentation << "- Event " << eventId << std::endl;
}

/*                  Commands                  */
Command::Command(Cluster* cluster, int commandId) : cluster(cluster), commandId(commandId) {}

int Command::getCommandId() const {
    return commandId;
}

int Command::getFlag() const {
    return flag;
}

void Command::print(int indent) const {
    std::string indentation(indent, ' ');
    std::cout << indentation << "- Command " << commandId << std::endl;
}

/*                  Cluster                  */
int Cluster::getClusterId() const {
    return metadata.clusterId;
}

void Cluster::addAttribute(const Attribute& attribute) {
    attributes.push_back(attribute);

    const int oldAttributeCount = metadata.attributeCount;

    // Create a new vector for the updated attribute metadata
    std::vector<EmberAfAttributeMetadata> newAttributes;
    newAttributes.reserve(oldAttributeCount + 1);  // Reserve space for new attribute

    // Copy the existing attribute metadata to the new vector
    if (oldAttributeCount > 0) {
        newAttributes.assign(metadata.attributes, metadata.attributes + oldAttributeCount);
    }

    // Add the new attribute metadata to the vector
    newAttributes.emplace_back(
        EmberAfAttributeMetadata{
            static_cast<uint32_t>(0), /* default value */
            attribute.getAttributeId(),
            attribute.getAttributeSize(),
            attribute.getAttributeType(),
            attribute.getAttributeMask(),
            // Set other members of the new attribute metadata as required
        }
    );

    // Update the metadata with the new attribute vector
    metadata.attributeCount = oldAttributeCount + 1;
    metadata.attributes = newAttributes.data();
}

void Cluster::removeAttribute(int attributeId) {
    const int oldAttributeCount = metadata.attributeCount;

    // Create a new vector for the updated attribute metadata
    std::vector<EmberAfAttributeMetadata> newAttributes;
    newAttributes.reserve(oldAttributeCount - 1);  // Reserve space for remaining attributes

    bool removed = false;
    // Copy the existing attribute metadata to the new vector, excluding the one to be removed
    for (int i = 0; i < oldAttributeCount; ++i) {
        if (metadata.attributes[i].attributeId != attributeId) {
            newAttributes.push_back(metadata.attributes[i]);
        } else {
            removed = true;
        }
    }

    if (!removed) {
        // Attribute with the given attributeId not found
        return;
    }

    // Update the metadata with the new attribute vector
    metadata.attributeCount = oldAttributeCount - 1;
    metadata.attributes = newAttributes.data();
}

void Cluster::addEvent(const Event& event) {
    events.push_back(event);

    // Increment the event count
    ++metadata.eventCount;

    // Create a new event list with the updated event count
    chip::EventId* newEventList = new chip::EventId[metadata.eventCount];

    // Copy the existing events to the new event list
    for (int i = 0; i < metadata.eventCount - 1; ++i) {
        newEventList[i] = metadata.eventList[i];
    }

    // Add the new event to the event list
    newEventList[metadata.eventCount - 1] = event.getEventId();

    // Free the previously allocated memory
    delete[] metadata.eventList;

    // Assign the updated event list to the metadata
    metadata.eventList = newEventList;
}

void Cluster::removeEvent(int eventId) {
    // Find the index of the event in the event list
    int eventIndex = -1;
    for (int i = 0; i < metadata.eventCount; ++i) {
        if (metadata.eventList[i] == eventId) {
            eventIndex = i;
            break;
        }
    }

    // If the event was found, remove it from the event list
    if (eventIndex != -1) {
        // Decrement the event count
        --metadata.eventCount;

        // Create a new event list with the updated event count
        chip::EventId* newEventList = new chip::EventId[metadata.eventCount];

        // Copy the events before the removed event
        for (int i = 0; i < eventIndex; ++i) {
            newEventList[i] = metadata.eventList[i];
        }

        // Copy the events after the removed event
        for (int i = eventIndex + 1; i < metadata.eventCount + 1; ++i) {
            newEventList[i - 1] = metadata.eventList[i];
        }

        // Free the previously allocated memory
        delete[] metadata.eventList;

        // Assign the updated event list to the metadata
        metadata.eventList = newEventList;
    }
}

void Cluster::addCommand(const Command& command) {
    commands.push_back(command);

    if (command.getFlag() & COMMAND_MASK_ACCEPTED) {
        // Add the commandId to the acceptedCommandList
        std::vector<int> newAcceptedCommands;
        if (metadata.acceptedCommandList) {
            // Copy the existing acceptedCommandList
            const chip::CommandId* ptr = metadata.acceptedCommandList;
            while (*ptr != 0xFFFFFFFF) {
                newAcceptedCommands.push_back(*ptr);
                ++ptr;
            }
        }

        // Add the new commandId
        newAcceptedCommands.push_back(command.getCommandId());
        newAcceptedCommands.push_back(0xFFFFFFFF);  // Terminate the list

        // Free previously assigned list
        delete[] metadata.acceptedCommandList;

        // Assign the updated acceptedCommandList
        metadata.acceptedCommandList = new chip::CommandId[newAcceptedCommands.size()];
        std::copy(newAcceptedCommands.begin(), newAcceptedCommands.end(), const_cast<chip::CommandId*>(metadata.acceptedCommandList));
    }

    if (command.getFlag() & COMMAND_MASK_GENERATED) {
        // Add the commandId to the generatedCommandList
        std::vector<int> newGeneratedCommands;
        if (metadata.generatedCommandList) {
            // Copy the existing generatedCommandList
            const chip::CommandId* ptr = metadata.generatedCommandList;
            while (*ptr != 0xFFFFFFFF) {
                newGeneratedCommands.push_back(*ptr);
                ++ptr;
            }
        }

        // Add the new commandId
        newGeneratedCommands.push_back(command.getCommandId());
        newGeneratedCommands.push_back(0xFFFFFFFF);  // Terminate the list

        // Free previously assigned list
        delete[] metadata.generatedCommandList;

        // Assign the updated generatedCommandList
        metadata.generatedCommandList = new chip::CommandId[newGeneratedCommands.size()];
        std::copy(newGeneratedCommands.begin(), newGeneratedCommands.end(), const_cast<chip::CommandId*>(metadata.generatedCommandList));
    }
}

void Cluster::removeCommand(int commandId) {
    auto it = std::find_if(commands.begin(), commands.end(), [commandId](const Command& command) {
        return command.getCommandId() == commandId;
    });

    if (it != commands.end()) {
        commands.erase(it);

        if (it->getFlag() & COMMAND_MASK_ACCEPTED) {
            // Remove the commandId from the acceptedCommandList
            std::vector<int> newAcceptedCommands;
            if (metadata.acceptedCommandList) {
                // Copy the existing acceptedCommandList except for the removed commandId
                const chip::CommandId* ptr = metadata.acceptedCommandList;
                while (*ptr != 0xFFFFFFFF) {
                    if (*ptr != commandId) {
                        newAcceptedCommands.push_back(*ptr);
                    }
                    ++ptr;
                }
            }

            // Free the previously allocated memory
            delete[] metadata.acceptedCommandList;

            // Assign the updated acceptedCommandList
            metadata.acceptedCommandList = new chip::CommandId[newAcceptedCommands.size()];
            std::copy(newAcceptedCommands.begin(), newAcceptedCommands.end(), const_cast<chip::CommandId*>(metadata.acceptedCommandList));
        }

        if (it->getFlag() & COMMAND_MASK_GENERATED) {
            // Remove the commandId from the generatedCommandList
            std::vector<int> newGeneratedCommands;
            if (metadata.generatedCommandList) {
                // Copy the existing generatedCommandList except for the removed commandId
                const chip::CommandId* ptr = metadata.generatedCommandList;
                while (*ptr != 0xFFFFFFFF) {
                    if (*ptr != commandId) {
                        newGeneratedCommands.push_back(*ptr);
                    }
                    ++ptr;
                }
            }

            // Free the previously allocated memory
            delete[] metadata.generatedCommandList;

            // Assign the updated generatedCommandList
            metadata.generatedCommandList = new chip::CommandId[newGeneratedCommands.size()];
            std::copy(newGeneratedCommands.begin(), newGeneratedCommands.end(), const_cast<chip::CommandId*>(metadata.generatedCommandList));
        }
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
    for (const Command& command : commands) {
        command.print(indent + 2);
    }
}

/*                  Endpoint                  */
Endpoint::Endpoint(Node* node, int endpointId) : node(node), endpointId(endpointId), parentEndpointId(0xFFFF /* chip::kInvalidEndpointId */) {}

void Endpoint::addCluster(const Cluster& cluster) {
    clusters.push_back(cluster);
}

int Endpoint::getEndpointId() const {
    return endpointId;
}

int Endpoint::getNumClusters() const {
    return clusters.size();
}

int Endpoint::getParentEndpointId() const {
    return parentEndpointId;
}

void Endpoint::setParentEndpointId(int newParentEndpointId) {
    parentEndpointId = newParentEndpointId;
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

void Node::addEndpoint(const EndpointConfig& endpointConfig) {
    Endpoint endpoint(this, nextEndpointId);
    // Set parentEndpointId based on the previous endpoint's endpointId
    if (!endpoints.empty()) {
        endpoint.setParentEndpointId(endpoints.back().getEndpointId());
    }

    for (const ClusterConfig& clusterConfig : endpointConfig.clusterConfigs) {
        Cluster cluster(&endpoint, clusterConfig.clusterId, clusterConfig.mask);
        for (const AttributeConfig& attributeConfig : clusterConfig.attributeConfigs) {
            Attribute attribute(&cluster, attributeConfig);
            cluster.addAttribute(attribute);
        }
        for (const EventConfig& eventConfig : clusterConfig.eventConfigs) {
            Event event(&cluster, eventConfig.eventId);
            cluster.addEvent(event);
        }
        for (const CommandConfig& commandConfig : clusterConfig.commandConfigs) {
            Command command(&cluster, commandConfig.commandId);
            cluster.addCommand(command);
        }
        endpoint.addCluster(cluster);
    }
    endpoints.push_back(endpoint);
    endpointCount++;
    nextEndpointId++;
}

void Node::removeEndpoint(int endpointId) {
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
