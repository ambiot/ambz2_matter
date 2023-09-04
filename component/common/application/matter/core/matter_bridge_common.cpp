#include <cJSON.h>
#include "matter_data_model.h"
#include "matter_data_model_presets.h"
#include "matter_bridge_common.h"

void MatterBridge::Init(Node node)
{
    // check for null node
    // check if rootnode endpoint has been added to ep0
    // check if aggregator endpoint has been added to ep1
    // retrieve previously saved endpointInfoList in string format from DCT
    // parse the endpointInfoList string and populate the endpointInfoList vector
    // reach out to the bridged device using the address and read the config file (bridged device need to maintain and update their config file with the up to date values)
    // create the new endpoint based on the config file (this means we don't need to store bridged devices' attribute values in nvs) (but this means we need to re-enable it on matter stack - normal)
    // start polling task to poll for messages from bridged device
}

void MatterBridge::addBridgedEndpoint()
{
    // parse the endpointConfig and create an endpoint
    // add the created endpoint to the node
    // enable the endpoint
    // append the endpointId of the created endpoint to the endpointIdList vector
    // format the endpointIdList vector into a string and store it in DCT
}

void MatterBridge::removeBridgedEndpoint()
{
    // remove the endpoint from the node
    // remove the endpointId from the endpointIdList vector
    // update and store the vector into DCT
}
