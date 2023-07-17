#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include "matter_data_model.h"

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

void Attribute::print(int indent) const {
    std::string indentation(indent, ' ');
    if (attributeType == ZCL_INT8U_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint8_t>()) << std::endl;
    } else if (attributeType == ZCL_INT16U_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint16_t>()) << std::endl;
    } else if (attributeType == ZCL_INT32U_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint32_t>()) << std::endl;
    } else if (attributeType == ZCL_INT64U_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint64_t>()) << std::endl;
    } else if (attributeType == ZCL_INT8S_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int8_t>()) << std::endl;
    } else if (attributeType == ZCL_INT16S_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int16_t>()) << std::endl;
    } else if (attributeType == ZCL_INT32S_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int32_t>()) << std::endl;
    } else if (attributeType == ZCL_INT64S_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int64_t>()) << std::endl;
    } else if (attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << getValue<std::string>() << std::endl;
    } else {
        // Handle the case where the data type doesn't match
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value type undefined" << std::endl;
    }
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

#if 0
    printf("\r\n=================metadata=============\r\n");
    for (const Endpoint & endpoint : endpoints) {
        printf("Endpoint Id: %d\r\n", endpoint.getEndpointId());
        printf("\tClusterCount: %d\r\n", endpoint.getClusterCount());
        printf("\tEndpointSize: %d\r\n", endpoint.getEndpointSize());

        const EmberAfCluster *cluster_md = endpoint.getMetadataClusterList();
        for (size_t i=0; i<endpoint.getClusterCount(); i++) {
            printf("\tCluster Id: %d\r\n", cluster_md->clusterId);
            printf("\t\tClusterSize: %d\r\n", cluster_md->clusterSize);
            printf("\t\tMask: %d\r\n", cluster_md->mask);

            printf("\t\tEventCount: %d\r\n", cluster_md->eventCount);
            printf("\t\tEventList\r\n");
            // for (size_t j=0; j<cluster_md->eventCount; j++) {
            //     printf("\t\t\tEvend Id: %d\r\n", *(cluster_md->eventList + j));
            // }

            printf("\t\tAcceptedCommandList\r\n");
            const chip::CommandId* ptr = cluster_md->acceptedCommandList;
            // while (*ptr != 0xFFFFFFFF) {
            //     printf("\t\t\tCommand Id: %d\r\n", *(ptr));
            //     ptr++;
            //     break;
            // }

            printf("\t\tGeneratedCommandList\r\n");
            ptr = cluster_md->generatedCommandList;
            // while (*ptr != 0xFFFFFFFF) {
            //     printf("\t\t\tCommand Id: %d\r\n", *(ptr));
            //     ptr++;
            //     break;
            // }

            printf("\t\tAttributeCount: %d\r\n", cluster_md->attributeCount);
            printf("\t\tAttributeList\r\n");
            const EmberAfAttributeMetadata *attribute_md = cluster_md->attributes;
            // for (size_t j=0; j<cluster_md->attributeCount; j++) {
            //     printf("\t\t\tAttribute Id: %d\r\n", attribute_md->attributeId);
            //     printf("\t\t\tAttributeSize : %d\r\n", attribute_md->size);
            //     printf("\t\t\tAttributeType : %d\r\n", attribute_md->attributeType);
            //     printf("\t\t\tAttributeMask : %d\r\n", attribute_md->mask);
            // }
        }
    }
#endif
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
