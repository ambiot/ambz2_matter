#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include "matter_data_model.h"

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

EmberAfDefaultOrMinMaxAttributeValue Attribute::getAttributeDefaultValue() const {
    return metadata->defaultValue;
}

chip::AttributeId Attribute::getAttributeId() const {
    return metadata->attributeId;
}

std::uint16_t Attribute::getAttributeSize() const {
    return metadata->size;
}

std::uint8_t Attribute::getAttributeType() const {
    return metadata->attributeType;
}

std::uint8_t Attribute::getAttributeMask() const {
    return metadata->mask;
}

void Attribute::print(int indent) const {
    std::string indentation(indent, ' ');
    if (metadata->attributeType == ZCL_INT8U_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint8_t>()) << std::endl;
    } else if (metadata->attributeType == ZCL_INT16U_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint16_t>()) << std::endl;
    } else if (metadata->attributeType == ZCL_INT32U_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint32_t>()) << std::endl;
    } else if (metadata->attributeType == ZCL_INT64U_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<unsigned int>(getValue<std::uint64_t>()) << std::endl;
    } else if (metadata->attributeType == ZCL_INT8S_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int8_t>()) << std::endl;
    } else if (metadata->attributeType == ZCL_INT16S_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int16_t>()) << std::endl;
    } else if (metadata->attributeType == ZCL_INT32S_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int32_t>()) << std::endl;
    } else if (metadata->attributeType == ZCL_INT64S_ATTRIBUTE_TYPE) {
        std::cout << indentation << "- Attribute " << getAttributeId() << ": " << " value = " << static_cast<int>(getValue<std::int64_t>()) << std::endl;
    } else if (metadata->attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE) {
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
    return flag;
}

void Command::print(int indent) const {
    std::string indentation(indent, ' ');
    std::cout << indentation << "- Command " << commandId << std::endl;
}

/*                  Cluster                  */
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

Command *Cluster::getCommand(chip::CommandId commandId) {
    for (auto & cmd : commands) {
        if (cmd.getCommandId() == commandId) {
            return &cmd;
        }
    }

    return NULL;
}

chip::ClusterId Cluster::getClusterId() const {
    return metadata->clusterId;
}

const EmberAfAttributeMetadata *Cluster::getAttributeMetadataList() const {
    return metadata->attributes;
}

uint16_t Cluster::getAttributeCount() const {
    return metadata->attributeCount;
}

uint16_t Cluster::getClusterSize() const {
    return metadata->clusterSize;
}

EmberAfClusterMask Cluster::getClusterMask() const {
    return metadata->mask;
}

const EmberAfGenericClusterFunction *Cluster::getGenericClusterFunctionList() const {
    return metadata->functions;
}

const chip::CommandId *Cluster::getAcceptedCommandList() const {
    return metadata->acceptedCommandList;
}

const chip::CommandId *Cluster::getGeneratedCommandList() const {
    return metadata->generatedCommandList;
}

const chip::EventId *Cluster::getEventList() const {
    return metadata->eventList;
}

uint16_t Cluster::getEventCount() const {
    return metadata->eventCount;
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

    // Increment the attribute count
    ++metadata->attributeCount;

    // Create a new attribute list with the updated attribute count
    void *tempPtr = new uint8_t[(metadata->attributeCount) * sizeof(EmberAfAttributeMetadata)];

    // Copy the existing attributes to the new attributes list
    memcpy(tempPtr, metadata->attributes, (metadata->attributeCount) * sizeof(EmberAfAttributeMetadata));

    // Add the new attribute to the attributes list
    EmberAfAttributeMetadata attributeMetadata = EmberAfAttributeMetadata{
        static_cast<uint32_t>(0), /* default value */
        attribute.getAttributeId(),
        attribute.getAttributeSize(),
        attribute.getAttributeType(),
        attribute.getAttributeMask(),
    };
    memcpy(tempPtr + (metadata->attributeCount - 1) * sizeof(EmberAfAttributeMetadata), &attributeMetadata, sizeof(EmberAfAttributeMetadata));

    // Cast it to *EmberAfAttributeMetadata
    EmberAfAttributeMetadata *newAttributes = reinterpret_cast<EmberAfAttributeMetadata*>(tempPtr);

    // Free tempPtr
    delete[] reinterpret_cast<uint8_t*>(tempPtr);

    // TODO: deallocate metadata if this is the first addAttribute call

    // Assign the updated attributes list to the metadata
    metadata->attributes = newAttributes;

    // Inform the Endpoint about the change, so it can update its metadata
    printf("originalAttributeMetadata:\r\n");
    printf("\tattributeType: %d\r\n", attribute.getAttributeType());
    printf("attributeMetadata:\r\n");
    printf("\tattributeType: %d\r\n", attributeMetadata.attributeType);
}

void Cluster::removeAttribute(chip::AttributeId attributeId) {
    // const int oldAttributeCount = metadata->attributeCount;

    // // Create a new vector for the updated attribute metadata
    // std::vector<EmberAfAttributeMetadata> newAttributes;
    // newAttributes.reserve(oldAttributeCount - 1);  // Reserve space for remaining attributes

    // bool removed = false;
    // // Copy the existing attribute metadata to the new vector, excluding the one to be removed
    // for (int i = 0; i < oldAttributeCount; ++i) {
    //     if (metadata.attributes[i].attributeId != attributeId) {
    //         newAttributes.push_back(metadata->attributes[i]);
    //     }
    //     else {
    //         removed = true;
    //     }
    // }

    // if (!removed) {
    //     // Attribute with the given attributeId not found
    //     return;
    // }

    // // Update the metadata with the new attribute vector
    // metadata.attributeCount = oldAttributeCount - 1;
    // metadata.attributes = newAttributes.data();

    // Remove the cluster from the vector
    auto it = std::find_if(attributes.begin(), attributes.end(), [&](const Attribute & attribute) {
        return attribute.getAttributeId() == attributeId;
    });

    if (it != attributes.end()) {
        attributes.erase(it);
    }

    // Inform the Endpoint about the change, so it can update its metadata
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

    // Increment the event count
    ++metadata->eventCount;

    // Create a new event list with the updated event count
    chip::EventId* newEventList = new chip::EventId[metadata->eventCount];

    // Copy the existing events to the new event list
    for (int i = 0; i < metadata->eventCount - 1; ++i) {
        newEventList[i] = metadata->eventList[i];
    }

    // Add the new event to the event list
    newEventList[metadata->eventCount - 1] = event.getEventId();

    // Free the previously allocated memory
    if (metadata->eventList != nullptr) {
        delete[] metadata->eventList;
    }

    // Assign the updated event list to the metadata
    metadata->eventList = newEventList;

    // Inform the Endpoint about the change, so it can update its metadata
}

void Cluster::removeEvent(chip::EventId eventId) {
    // Find the index of the event in the event list
    int eventIndex = -1;
    for (int i = 0; i < metadata->eventCount; ++i) {
        if (metadata->eventList[i] == eventId) {
            eventIndex = i;
            break;
        }
    }

    // If the event was found, remove it from the event list
    if (eventIndex != -1) {
        // Decrement the event count
        --metadata->eventCount;

        // Create a new event list with the updated event count
        chip::EventId* newEventList = new chip::EventId[metadata->eventCount];

        // Copy the events before the removed event
        for (int i = 0; i < eventIndex; ++i) {
            newEventList[i] = metadata->eventList[i];
        }

        // Copy the events after the removed event
        for (int i = eventIndex + 1; i < metadata->eventCount + 1; ++i) {
            newEventList[i - 1] = metadata->eventList[i];
        }

        // Free the previously allocated memory
        delete[] metadata->eventList;

        // Assign the updated event list to the metadata
        metadata->eventList = newEventList;

        // Remove the event from the vector
        auto it = std::find_if(events.begin(), events.end(), [&](const Event & event) {
            return event.getEventId() == eventId;
        });

        if (it != events.end()) {
            events.erase(it);
        }

        // Inform the Endpoint about the change, so it can update its metadata
    }
}

void Cluster::addCommand(CommandConfig commandConfig) {
    Command command(this, commandConfig);
    addCommand(command);
}

void Cluster::addCommand(const Command& command) {
    // Check if command already exist
    if (getCommand(command.getCommandId()) != NULL) {
        return;
    }

    commands.push_back(command);

    if (command.getFlag() & COMMAND_MASK_ACCEPTED) {
        // Count how many accepted commands
        int count = 0;
        const chip::CommandId* ptr = metadata->acceptedCommandList;
        while (*ptr != 0xFFFFFFFF) {
            ++ptr;
            count++;
        }

        // Increment the command count for new command
        count += 1;

        // Create a new accepted command list with updated command count
        chip::CommandId *newAcceptedCommandList = new chip::CommandId[count];

        // Copy existing accepted commands into the new command list
        for (int i=0; i<count-1; i++) {
            newAcceptedCommandList[i] = metadata->acceptedCommandList[i];
        }

        // Add the new accepted command to the accepted command list
        newAcceptedCommandList[count - 1] = command.getCommandId();

        // Free the previously allocated memory
        if (metadata->acceptedCommandList != nullptr) {
            delete[] metadata->acceptedCommandList;
        }

        // Assign new metadata
        metadata->acceptedCommandList = newAcceptedCommandList;
    }

    if (command.getFlag() & COMMAND_MASK_GENERATED) {
        int count = 0;
        const chip::CommandId* ptr = metadata->generatedCommandList;
        while (*ptr != 0xFFFFFFFF) {
            ++ptr;
            count++;
        }

        // Increment the command count for new command
        count += 1;

        // Create a new generated command list with updated command count
        chip::CommandId *newGeneratedCommandList = new chip::CommandId[count];

        // Copy existing generated commands into the new command list
        for (int i=0; i<count-1; i++) {
            newGeneratedCommandList[i] = metadata->generatedCommandList[i];
        }

        // Add the new accepted command to the accepted command list
        newGeneratedCommandList[count - 1] = command.getCommandId();

        // Free the previously allocated memory
        if (metadata->generatedCommandList != nullptr) {
            delete[] metadata->generatedCommandList;
        }

        // Assign new metadata
        metadata->acceptedCommandList = newGeneratedCommandList;
    }

    // Inform the Endpoint about the change, so it can update its metadata
}

void Cluster::removeCommand(chip::CommandId commandId) {
    auto it = std::find_if(commands.begin(), commands.end(), [commandId](const Command& command) {
        return command.getCommandId() == commandId;
    });

    if (it != commands.end()) {
        commands.erase(it);

        if (it->getFlag() & COMMAND_MASK_ACCEPTED) {
            // Remove the commandId from the acceptedCommandList
            std::vector<int> newAcceptedCommands;
            if (metadata->acceptedCommandList) {
                // Copy the existing acceptedCommandList except for the removed commandId
                const chip::CommandId* ptr = metadata->acceptedCommandList;
                while (*ptr != 0xFFFFFFFF) {
                    if (*ptr != commandId) {
                        newAcceptedCommands.push_back(*ptr);
                    }
                    ++ptr;
                }
            }

            // Free the previously allocated memory
            delete[] metadata->acceptedCommandList;

            // Assign the updated acceptedCommandList
            metadata->acceptedCommandList = new chip::CommandId[newAcceptedCommands.size()];
            std::copy(newAcceptedCommands.begin(), newAcceptedCommands.end(), const_cast<chip::CommandId*>(metadata->acceptedCommandList));

            // Inform the Endpoint about the change, so it can update its metadata
        }

        if (it->getFlag() & COMMAND_MASK_GENERATED) {
            // Remove the commandId from the generatedCommandList
            std::vector<int> newGeneratedCommands;
            if (metadata->generatedCommandList) {
                // Copy the existing generatedCommandList except for the removed commandId
                const chip::CommandId* ptr = metadata->generatedCommandList;
                while (*ptr != 0xFFFFFFFF) {
                    if (*ptr != commandId) {
                        newGeneratedCommands.push_back(*ptr);
                    }
                    ++ptr;
                }
            }

            // Free the previously allocated memory
            delete[] metadata->generatedCommandList;

            // Assign the updated generatedCommandList
            metadata->generatedCommandList = new chip::CommandId[newGeneratedCommands.size()];
            std::copy(newGeneratedCommands.begin(), newGeneratedCommands.end(), const_cast<chip::CommandId*>(metadata->generatedCommandList));

            // Inform the Endpoint about the change, so it can update its metadata
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
        cluster.addCommand(command);
    }
    addCluster(cluster);
}

void Endpoint::addCluster(const Cluster& cluster) {
    // Check if cluster already exist
    if (getCluster(cluster.getClusterId()) != NULL) {
        return;
    }

    clusters.push_back(cluster);

    // Increment the cluster count
    ++metadata->clusterCount;

    // Create a new cluster list with the updated cluster count
    void *tempPtr = new uint8_t[(metadata->clusterCount) * sizeof(EmberAfCluster)];

    // Copy the existing clusters to the new clusters list
    memcpy(tempPtr, metadata->cluster, (metadata->clusterCount) * sizeof(EmberAfCluster));

    // Add the new cluster to the clusters list
    EmberAfCluster clusterMetadata = EmberAfCluster{
        cluster.getClusterId(),
        cluster.getAttributeMetadataList(),
        cluster.getAttributeCount(),
        cluster.getClusterSize(),
        cluster.getClusterMask(),
        cluster.getGenericClusterFunctionList(),
        cluster.getAcceptedCommandList(),
        cluster.getGeneratedCommandList(),
        cluster.getEventList(),
        cluster.getEventCount(),
    };
    memcpy(tempPtr + (metadata->clusterCount - 1) * sizeof(EmberAfCluster), &clusterMetadata, sizeof(EmberAfCluster));

    // Cast it to *EmberAfCluster
    EmberAfCluster *newClusters = reinterpret_cast<EmberAfCluster*>(tempPtr);

    // Free tempPtr
    delete[] reinterpret_cast<uint8_t*>(tempPtr);

    // Assign the updated clusters list to the metadata
    metadata->cluster = newClusters;
}

void Endpoint::removeCluster(chip::ClusterId clusterId) {
    const int oldClusterCount = metadata->clusterCount;

    // Create a new vector for the updated attribute metadata
    std::vector<EmberAfCluster> newClusters;
    newClusters.reserve(oldClusterCount - 1);   // Reserve space for remaining clusters

    bool removed = false;
    // Copy the existing cluster metadata to the new vector, excluding the one to be removed
    for (int i=0; i<oldClusterCount; ++i) {
        if (metadata->cluster[i].clusterId != clusterId) {
            newClusters.push_back(metadata->cluster[i]);
        }
        else {
            removed = true;
        }
    }

    if (!removed) {
        // Cluster with the given clusterId not found
        printf("Cluster not found, cannot remove!\r\n");
        return;
    }

    // Update the metadata with the new cluster vector
    metadata->clusterCount = oldClusterCount - 1;
    metadata->cluster = newClusters.data();

    // Remove the cluster from the vector
    auto it = std::find_if(clusters.begin(), clusters.end(), [&](const Cluster& cluster) {
        return cluster.getClusterId() == clusterId;
    });

    if (it != clusters.end()) {
        clusters.erase(it);
    }
}

chip::EndpointId Endpoint::getEndpointId() const {
    return endpointId;
}

const EmberAfCluster *Endpoint::getMetadataClusterList() const {
    return metadata->cluster;
}

uint8_t Endpoint::getClusterCount() const {
    return metadata->clusterCount;
}

uint16_t Endpoint::getEndpointSize() const {
    return metadata->endpointSize;
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
            cluster.addCommand(command);
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
