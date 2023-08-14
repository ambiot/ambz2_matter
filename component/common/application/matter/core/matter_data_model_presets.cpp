#include <app/PluginApplicationCallbacks.h>
#include <app/util/endpoint-config-defines.h>
#include <app/att-storage.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/callback.h>
#include "matter_data_model.h"

using namespace chip::app::Clusters;

namespace Presets {
namespace Clusters {

// Attribute default values that are non trivial
EmberAfAttributeMinMaxValue onoffStartUpOnOffMinMaxValue = {(uint16_t)0xFF, (uint16_t)0x0, (uint16_t)0x2};
EmberAfAttributeMinMaxValue levelcontrolOptionsMinMaxValue = {(uint16_t)0x0, (uint16_t)0x0, (uint16_t)0x3};
uint8_t generalcommissioningBreadCrumbValue[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void matter_cluster_descriptor_server(ClusterConfig *clusterConfig)
{
    AttributeConfig descriptorDeviceTypeList(0x00000000, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig descriptorServerList(0x00000001, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig descriptorClientList(0x00000002, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig descriptorPartsList(0x00000003, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig descriptorFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(0), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig descriptorClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    clusterConfig->clusterId = 0x0000001D;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER);
    clusterConfig->attributeConfigs.push_back(descriptorDeviceTypeList);
    clusterConfig->attributeConfigs.push_back(descriptorServerList);
    clusterConfig->attributeConfigs.push_back(descriptorClientList);
    clusterConfig->attributeConfigs.push_back(descriptorPartsList);
    clusterConfig->attributeConfigs.push_back(descriptorFeatureMap);
    clusterConfig->attributeConfigs.push_back(descriptorClusterRevision);
}

void matter_cluster_acl_server(ClusterConfig *clusterConfig)
{
    AttributeConfig aclACL(0x00000000, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE));
    AttributeConfig aclExtension(0x00000001, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE));
    AttributeConfig aclSubjectsPerAccessControlEntry(0x00000002, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig aclTargetsPerAccessControlEntry(0x00000003, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig aclAccessControlEntriesPerFabric(0x00000004, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig aclFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(0), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig aclClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(1), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    EventConfig aclAccessControlEntryChanged(0x00000000);
    EventConfig aclAccessControlExtensionChanged(0x00000001);

    clusterConfig->clusterId = 0x0000001F;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER);
    clusterConfig->attributeConfigs.push_back(aclACL);
    clusterConfig->attributeConfigs.push_back(aclExtension);
    clusterConfig->attributeConfigs.push_back(aclSubjectsPerAccessControlEntry);
    clusterConfig->attributeConfigs.push_back(aclTargetsPerAccessControlEntry);
    clusterConfig->attributeConfigs.push_back(aclAccessControlEntriesPerFabric);
    clusterConfig->attributeConfigs.push_back(aclFeatureMap);
    clusterConfig->attributeConfigs.push_back(aclClusterRevision);
    clusterConfig->eventConfigs.push_back(aclAccessControlEntryChanged);
    clusterConfig->eventConfigs.push_back(aclAccessControlExtensionChanged);
}

void matter_cluster_basic_information_server(ClusterConfig *clusterConfig)
{
    AttributeConfig basicinfoDataModelRevision(0x00000000, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoVendorName(0x00000001, ZAP_TYPE(CHAR_STRING), ZAP_EMPTY_DEFAULT(), 33, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoVendorId(0x00000002, ZAP_TYPE(VENDOR_ID), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoProductName(0x00000003, ZAP_TYPE(CHAR_STRING), ZAP_EMPTY_DEFAULT(), 33, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoProductId(0x00000004, ZAP_TYPE(VENDOR_ID), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoNodeLabel(0x00000005, ZAP_TYPE(CHAR_STRING), ZAP_EMPTY_DEFAULT(), 33, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(TOKENIZE) | ZAP_ATTRIBUTE_MASK(SINGLETON) | ZAP_ATTRIBUTE_MASK(WRITABLE));
    AttributeConfig basicinfoLocation(0x00000006, ZAP_TYPE(CHAR_STRING), ZAP_EMPTY_DEFAULT(), 3, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON) | ZAP_ATTRIBUTE_MASK(WRITABLE));
    AttributeConfig basicinfoHardwareVersion(0x00000007, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoHardwareVersionString(0x00000008, ZAP_TYPE(CHAR_STRING), ZAP_EMPTY_DEFAULT(), 65, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoSoftwareVersion(0x00000009, ZAP_TYPE(INT32U), ZAP_EMPTY_DEFAULT(), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoSoftwareVersionString(0x0000000A, ZAP_TYPE(CHAR_STRING), ZAP_EMPTY_DEFAULT(), 65, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoManufacturingDate(0x0000000B, ZAP_TYPE(CHAR_STRING), ZAP_EMPTY_DEFAULT(), 17, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoPartNumber(0x0000000C, ZAP_TYPE(CHAR_STRING), ZAP_EMPTY_DEFAULT(), 33, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoProductUrl(0x0000000D, ZAP_TYPE(LONG_CHAR_STRING), ZAP_EMPTY_DEFAULT(), 258, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoProductLabel(0x0000000E, ZAP_TYPE(CHAR_STRING), ZAP_EMPTY_DEFAULT(), 65, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoSerialNumber(0x0000000F, ZAP_TYPE(CHAR_STRING), ZAP_EMPTY_DEFAULT(), 33, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoLocalConfigDisabled(0x00000010, ZAP_TYPE(BOOLEAN), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(TOKENIZE) | ZAP_ATTRIBUTE_MASK(SINGLETON) | ZAP_ATTRIBUTE_MASK(WRITABLE));
    AttributeConfig basicinfoUniqueId(0x00000012, ZAP_TYPE(CHAR_STRING), ZAP_EMPTY_DEFAULT(), 33, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));
    AttributeConfig basicinfoCapabilityMinima(0x00000013, ZAP_TYPE(STRUCT), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig basicinfoFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(0), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig basicinfoClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(1), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(SINGLETON));

    EventConfig basicinfoStartUp(0x00000000);
    EventConfig basicinfoShutDown(0x00000001);
    EventConfig basicinfoLeave(0x00000002);

    clusterConfig->clusterId = 0x00000028;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER);
    clusterConfig->attributeConfigs.push_back(basicinfoDataModelRevision);
    clusterConfig->attributeConfigs.push_back(basicinfoVendorName);
    clusterConfig->attributeConfigs.push_back(basicinfoVendorId);
    clusterConfig->attributeConfigs.push_back(basicinfoProductName);
    clusterConfig->attributeConfigs.push_back(basicinfoProductId);
    clusterConfig->attributeConfigs.push_back(basicinfoNodeLabel);
    clusterConfig->attributeConfigs.push_back(basicinfoLocation);
    clusterConfig->attributeConfigs.push_back(basicinfoHardwareVersion);
    clusterConfig->attributeConfigs.push_back(basicinfoHardwareVersionString);
    clusterConfig->attributeConfigs.push_back(basicinfoSoftwareVersion);
    clusterConfig->attributeConfigs.push_back(basicinfoSoftwareVersionString);
    clusterConfig->attributeConfigs.push_back(basicinfoManufacturingDate);
    clusterConfig->attributeConfigs.push_back(basicinfoPartNumber);
    clusterConfig->attributeConfigs.push_back(basicinfoProductUrl);
    clusterConfig->attributeConfigs.push_back(basicinfoProductLabel);
    clusterConfig->attributeConfigs.push_back(basicinfoSerialNumber);
    clusterConfig->attributeConfigs.push_back(basicinfoLocalConfigDisabled);
    clusterConfig->attributeConfigs.push_back(basicinfoUniqueId);
    clusterConfig->attributeConfigs.push_back(basicinfoFeatureMap);
    clusterConfig->attributeConfigs.push_back(basicinfoClusterRevision);
    clusterConfig->eventConfigs.push_back(basicinfoStartUp);
    clusterConfig->eventConfigs.push_back(basicinfoShutDown);
    clusterConfig->eventConfigs.push_back(basicinfoLeave);
}

void matter_cluster_ota_requestor_server(ClusterConfig *clusterConfig)
{
    AttributeConfig otarDefaultOtaProviders(0x00000000, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE));
    AttributeConfig otarUpdatePossible(0x00000001, ZAP_TYPE(BOOLEAN), ZAP_SIMPLE_DEFAULT(1), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig otarUpdateState(0x00000002, ZAP_TYPE(ENUM8), ZAP_SIMPLE_DEFAULT(0), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig otarUpdateStateProgress(0x00000003, ZAP_TYPE(INT8U), ZAP_SIMPLE_DEFAULT(0), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig otarFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(0), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig otarClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(1), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    CommandConfig otarAnnounceOtaProvider(0x00000000, COMMAND_MASK_ACCEPTED);
    CommandConfig otarEndOfAcceptedCommandList(chip::kInvalidCommandId, COMMAND_MASK_ACCEPTED);

    EventConfig otarStateTransition(0x00000000);
    EventConfig otarVersionApplied(0x00000001);
    EventConfig otarDownloadError(0x00000002);

    clusterConfig->clusterId = 0x0000002A;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER);
    clusterConfig->attributeConfigs.push_back(otarDefaultOtaProviders);
    clusterConfig->attributeConfigs.push_back(otarUpdatePossible);
    clusterConfig->attributeConfigs.push_back(otarUpdateState);
    clusterConfig->attributeConfigs.push_back(otarUpdateStateProgress);
    clusterConfig->attributeConfigs.push_back(otarFeatureMap);
    clusterConfig->attributeConfigs.push_back(otarClusterRevision);
    clusterConfig->commandConfigs.push_back(otarAnnounceOtaProvider);
    clusterConfig->commandConfigs.push_back(otarEndOfAcceptedCommandList);
    clusterConfig->eventConfigs.push_back(otarStateTransition);
    clusterConfig->eventConfigs.push_back(otarVersionApplied);
    clusterConfig->eventConfigs.push_back(otarDownloadError);
}

void matter_cluster_general_commissioning_server(ClusterConfig *clusterConfig)
{
    AttributeConfig gencomBreadcrumb(0x00000000, ZAP_TYPE(INT64U), uint32_t(0), 8, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE));
    AttributeConfig gencomBasicCommissioningInfo(0x00000001, ZAP_TYPE(STRUCT), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gencomRegulatoryConfig(0x00000002, ZAP_TYPE(ENUM8), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gencomLocationCapability(0x00000003, ZAP_TYPE(ENUM8), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gencomSupportsConcurrentConnection(0x00000004, ZAP_TYPE(BOOLEAN), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gencomFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(0), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gencomClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(1), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    CommandConfig gencomArmFailSafe(0x00000000, COMMAND_MASK_ACCEPTED);
    CommandConfig gencomSetRegulatoryConfig(0x00000002, COMMAND_MASK_ACCEPTED);
    CommandConfig gencomCommissioningComplete(0x00000004, COMMAND_MASK_ACCEPTED);
    CommandConfig gencomEndOfAcceptedCommandList(chip::kInvalidCommandId, COMMAND_MASK_ACCEPTED);

    CommandConfig gencomArmFailSafeResponse(0x00000001, COMMAND_MASK_GENERATED);
    CommandConfig gencomSetRegulatoryConfigResponse(0x00000003, COMMAND_MASK_GENERATED);
    CommandConfig gencomCommissioningCompleteResponse(0x00000005, COMMAND_MASK_GENERATED);
    CommandConfig gencomEndOfGeneratedCommandList(chip::kInvalidCommandId, COMMAND_MASK_GENERATED);

    clusterConfig->clusterId = 0x00000030;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER);
    clusterConfig->attributeConfigs.push_back(gencomBreadcrumb);
    clusterConfig->attributeConfigs.push_back(gencomBasicCommissioningInfo);
    clusterConfig->attributeConfigs.push_back(gencomRegulatoryConfig);
    clusterConfig->attributeConfigs.push_back(gencomLocationCapability);
    clusterConfig->attributeConfigs.push_back(gencomSupportsConcurrentConnection);
    clusterConfig->attributeConfigs.push_back(gencomFeatureMap);
    clusterConfig->attributeConfigs.push_back(gencomClusterRevision);
    clusterConfig->commandConfigs.push_back(gencomArmFailSafe);
    clusterConfig->commandConfigs.push_back(gencomSetRegulatoryConfig);
    clusterConfig->commandConfigs.push_back(gencomCommissioningComplete);
    clusterConfig->commandConfigs.push_back(gencomEndOfAcceptedCommandList);
    clusterConfig->commandConfigs.push_back(gencomArmFailSafeResponse);
    clusterConfig->commandConfigs.push_back(gencomSetRegulatoryConfigResponse);
    clusterConfig->commandConfigs.push_back(gencomCommissioningCompleteResponse);
    clusterConfig->commandConfigs.push_back(gencomEndOfGeneratedCommandList);
}

void matter_cluster_network_commissioning_server(ClusterConfig *clusterConfig)
{
    AttributeConfig netcomMaxNetworks(0x00000000, ZAP_TYPE(INT8U), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig netcomNetworks(0x00000001, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig netcomScanMaxTimeSeconds(0x00000002, ZAP_TYPE(INT8U), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig netcomConnectMaxTimeSeconds(0x00000003, ZAP_TYPE(INT8U), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig netcomInterfaceEnabled(0x00000004, ZAP_TYPE(BOOLEAN), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE));
    AttributeConfig netcomLastNetworkingStatus(0x00000005, ZAP_TYPE(ENUM8), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig netcomLastNetworkId(0x00000006, ZAP_TYPE(OCTET_STRING), ZAP_EMPTY_DEFAULT(), 33, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig netcomLastConnectErrorValue(0x00000007, ZAP_TYPE(INT32S), ZAP_EMPTY_DEFAULT(), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig netcomFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(1), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig netcomClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(1), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    CommandConfig netcomScanNetworks(0x00000000, COMMAND_MASK_ACCEPTED);
    CommandConfig netcomAddOrUpdateWiFiNetwork(0x00000002, COMMAND_MASK_ACCEPTED);
    CommandConfig netcomRemoveNetwork(0x00000004, COMMAND_MASK_ACCEPTED);
    CommandConfig netcomConnectNetwork(0x00000006, COMMAND_MASK_ACCEPTED);
    CommandConfig netcomReorderNetwork(0x00000008, COMMAND_MASK_ACCEPTED);
    CommandConfig netcomEndOfAcceptedCommandList(chip::kInvalidCommandId, COMMAND_MASK_ACCEPTED);

    CommandConfig netcomScanNetworksResponse(0x00000001, COMMAND_MASK_GENERATED);
    CommandConfig netcomNetworkConfigResponse(0x00000005, COMMAND_MASK_GENERATED);
    CommandConfig netcomConnectNetworkResponse(0x00000007, COMMAND_MASK_GENERATED);
    CommandConfig netcomEndOfGeneratedCommandList(chip::kInvalidCommandId, COMMAND_MASK_GENERATED);

    clusterConfig->clusterId = 0x00000031;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER);
    clusterConfig->attributeConfigs.push_back(netcomMaxNetworks);
    clusterConfig->attributeConfigs.push_back(netcomNetworks);
    clusterConfig->attributeConfigs.push_back(netcomScanMaxTimeSeconds);
    clusterConfig->attributeConfigs.push_back(netcomConnectMaxTimeSeconds);
    clusterConfig->attributeConfigs.push_back(netcomInterfaceEnabled);
    clusterConfig->attributeConfigs.push_back(netcomLastNetworkingStatus);
    clusterConfig->attributeConfigs.push_back(netcomLastNetworkId);
    clusterConfig->attributeConfigs.push_back(netcomLastConnectErrorValue);
    clusterConfig->attributeConfigs.push_back(netcomFeatureMap);
    clusterConfig->attributeConfigs.push_back(netcomClusterRevision);
    clusterConfig->commandConfigs.push_back(netcomScanNetworks);
    clusterConfig->commandConfigs.push_back(netcomAddOrUpdateWiFiNetwork);
    clusterConfig->commandConfigs.push_back(netcomRemoveNetwork);
    clusterConfig->commandConfigs.push_back(netcomConnectNetwork);
    clusterConfig->commandConfigs.push_back(netcomReorderNetwork);
    clusterConfig->commandConfigs.push_back(netcomEndOfAcceptedCommandList);
    clusterConfig->commandConfigs.push_back(netcomScanNetworksResponse);
    clusterConfig->commandConfigs.push_back(netcomNetworkConfigResponse);
    clusterConfig->commandConfigs.push_back(netcomConnectNetworkResponse);
    clusterConfig->commandConfigs.push_back(netcomEndOfGeneratedCommandList);
}

void matter_cluster_general_diagnostics_server(ClusterConfig *clusterConfig)
{
    AttributeConfig gendiagNetworkInterfaces(0x00000000, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gendiagRebootCount(0x00000001, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gendiagUpTime(0x00000002, ZAP_TYPE(INT64U), ZAP_EMPTY_DEFAULT(), 8, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gendiagTotalOperationalHours(0x00000003, ZAP_TYPE(INT32U), ZAP_EMPTY_DEFAULT(), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gendiagBootReason(0x00000004, ZAP_TYPE(ENUM8), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gendiagActiveHardwareFaults(0x00000005, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gendiagActiveRadioFaults(0x00000006, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gendiagActiveNetworkFaults(0x00000007, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gendiagTestEventTriggersEnabled(0x00000008, ZAP_TYPE(BOOLEAN), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gendiagFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(0), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gendiagClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(1), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    EventConfig gendiagHardwareFaultChange(0x00000000);
    EventConfig gendiagRadioFaultChange(0x00000001);
    EventConfig gendiagNetworkFaultChange(0x00000002);
    EventConfig gendiagBootReasonEvent(0x00000003);

    clusterConfig->clusterId = 0x00000033;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER);
    clusterConfig->attributeConfigs.push_back(gendiagNetworkInterfaces);
    clusterConfig->attributeConfigs.push_back(gendiagRebootCount);
    clusterConfig->attributeConfigs.push_back(gendiagUpTime);
    clusterConfig->attributeConfigs.push_back(gendiagTotalOperationalHours);
    clusterConfig->attributeConfigs.push_back(gendiagBootReason);
    clusterConfig->attributeConfigs.push_back(gendiagActiveHardwareFaults);
    clusterConfig->attributeConfigs.push_back(gendiagActiveRadioFaults);
    clusterConfig->attributeConfigs.push_back(gendiagActiveNetworkFaults);
    clusterConfig->attributeConfigs.push_back(gendiagTestEventTriggersEnabled);
    clusterConfig->attributeConfigs.push_back(gendiagFeatureMap);
    clusterConfig->attributeConfigs.push_back(gendiagClusterRevision);
    clusterConfig->eventConfigs.push_back(gendiagHardwareFaultChange);
    clusterConfig->eventConfigs.push_back(gendiagRadioFaultChange);
    clusterConfig->eventConfigs.push_back(gendiagNetworkFaultChange);
    clusterConfig->eventConfigs.push_back(gendiagBootReasonEvent);
}

void matter_cluster_software_diagnostics_server(ClusterConfig *clusterConfig)
{
    AttributeConfig swdiagThreadMetrics(0x00000000, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig swdiagCurrentHeapFree(0x00000001, ZAP_TYPE(INT64U), ZAP_EMPTY_DEFAULT(), 8, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig swdiagCurrentHeapUsed(0x00000002, ZAP_TYPE(INT64U), ZAP_EMPTY_DEFAULT(), 8, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig swdiagCurrentHeapHighWatermark(0x00000003, ZAP_TYPE(INT64U), ZAP_EMPTY_DEFAULT(), 8, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig swdiagFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(1), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig swdiagClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(1), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    clusterConfig->clusterId = 0x00000034;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER);
    clusterConfig->attributeConfigs.push_back(swdiagThreadMetrics);
    clusterConfig->attributeConfigs.push_back(swdiagCurrentHeapFree);
    clusterConfig->attributeConfigs.push_back(swdiagCurrentHeapUsed);
    clusterConfig->attributeConfigs.push_back(swdiagCurrentHeapHighWatermark);
    clusterConfig->attributeConfigs.push_back(swdiagFeatureMap);
    clusterConfig->attributeConfigs.push_back(swdiagClusterRevision);
}

void matter_cluster_wifi_diagnostics_server(ClusterConfig *clusterConfig)
{
    AttributeConfig wifidiagBssid(0x00000000, ZAP_TYPE(OCTET_STRING), ZAP_EMPTY_DEFAULT(), 7, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig wifidiagSecurityType(0x00000001, ZAP_TYPE(ENUM8), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig wifidiagWiFiVersion(0x00000002, ZAP_TYPE(ENUM8), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig wifidiagChannelNumber(0x00000003, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig wifidiagRssi(0x00000004, ZAP_TYPE(INT8S), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig wifidiagFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(3), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig wifidiagClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(1), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    EventConfig wifidiagDisconnection(0x00000000);
    EventConfig wifidiagAssociationFailure(0x00000001);
    EventConfig wifidiagConnectionStatus(0x00000002);

    clusterConfig->clusterId = 0x00000036;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER);
    clusterConfig->attributeConfigs.push_back(wifidiagBssid);
    clusterConfig->attributeConfigs.push_back(wifidiagSecurityType);
    clusterConfig->attributeConfigs.push_back(wifidiagWiFiVersion);
    clusterConfig->attributeConfigs.push_back(wifidiagChannelNumber);
    clusterConfig->attributeConfigs.push_back(wifidiagRssi);
    clusterConfig->attributeConfigs.push_back(wifidiagFeatureMap);
    clusterConfig->attributeConfigs.push_back(wifidiagClusterRevision);
    clusterConfig->eventConfigs.push_back(wifidiagDisconnection);
    clusterConfig->eventConfigs.push_back(wifidiagAssociationFailure);
    clusterConfig->eventConfigs.push_back(wifidiagConnectionStatus);
}

void matter_cluster_administrator_commissioning_server(ClusterConfig *clusterConfig)
{
    AttributeConfig admincomWindowStatus(0x00000000, ZAP_TYPE(ENUM8), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig admincomAdminFabricIndex(0x00000001, ZAP_TYPE(FABRIC_IDX), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig admincomAdminVendorId(0x00000002, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig admincomAdminFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(0), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig admincomAdminClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(1), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    clusterConfig->clusterId = 0x0000003C;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER);
    clusterConfig->attributeConfigs.push_back(admincomWindowStatus);
    clusterConfig->attributeConfigs.push_back(admincomAdminFabricIndex);
    clusterConfig->attributeConfigs.push_back(admincomAdminVendorId);
    clusterConfig->attributeConfigs.push_back(admincomAdminFeatureMap);
    clusterConfig->attributeConfigs.push_back(admincomAdminClusterRevision);
}

void matter_cluster_operational_credentials_server(ClusterConfig *clusterConfig)
{
    AttributeConfig opcredsNocs(0x00000000, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig opcredsFabrics(0x00000001, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig opcredsSupportedFabrics(0x00000002, ZAP_TYPE(INT8U), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig opcredsCommissionedFabrics(0x00000003, ZAP_TYPE(INT8U), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig opcredsTrustedRootCertificates(0x00000004, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig opcredsCurrentFabricIndex(0x00000005, ZAP_TYPE(INT8U), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig opcredsFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(0), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig opcredsClusterRevision(0x0000FFFD, ZAP_TYPE(INT8U), ZAP_SIMPLE_DEFAULT(1), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    CommandConfig opcredsAttestationRequest(0x00000000, COMMAND_MASK_ACCEPTED);
    CommandConfig opcredsCertificateChainRequest(0x00000002, COMMAND_MASK_ACCEPTED);
    CommandConfig opcredsCSRRequest(0x00000004, COMMAND_MASK_ACCEPTED);
    CommandConfig opcredsAddNOC(0x00000006, COMMAND_MASK_ACCEPTED);
    CommandConfig opcredsUpdateNOC(0x00000007, COMMAND_MASK_ACCEPTED);
    CommandConfig opcredsRemoveFabric(0x0000000A, COMMAND_MASK_ACCEPTED);
    CommandConfig opcredsAddTrustedRootCertificate(0x0000000B, COMMAND_MASK_ACCEPTED);
    CommandConfig opcredsEndOfAcceptedCommandList(chip::kInvalidCommandId, COMMAND_MASK_ACCEPTED);

    CommandConfig opcredsAttestationResponse(0x00000001, COMMAND_MASK_GENERATED);
    CommandConfig opcredsCertificateChainResponse(0x00000003, COMMAND_MASK_GENERATED);
    CommandConfig opcredsCSRResponse(0x00000005, COMMAND_MASK_GENERATED);
    CommandConfig opcredsNOCResponse(0x00000008, COMMAND_MASK_GENERATED);
    CommandConfig opcredsEndOfGeneratedCommandList(chip::kInvalidCommandId, COMMAND_MASK_GENERATED);

    clusterConfig->clusterId = 0x0000003E;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER);
    clusterConfig->attributeConfigs.push_back(opcredsNocs);
    clusterConfig->attributeConfigs.push_back(opcredsFabrics);
    clusterConfig->attributeConfigs.push_back(opcredsSupportedFabrics);
    clusterConfig->attributeConfigs.push_back(opcredsCommissionedFabrics);
    clusterConfig->attributeConfigs.push_back(opcredsTrustedRootCertificates);
    clusterConfig->attributeConfigs.push_back(opcredsCurrentFabricIndex);
    clusterConfig->attributeConfigs.push_back(opcredsFeatureMap);
    clusterConfig->attributeConfigs.push_back(opcredsClusterRevision);
    clusterConfig->commandConfigs.push_back(opcredsAttestationRequest);
    clusterConfig->commandConfigs.push_back(opcredsCertificateChainRequest);
    clusterConfig->commandConfigs.push_back(opcredsCSRRequest);
    clusterConfig->commandConfigs.push_back(opcredsAddNOC);
    clusterConfig->commandConfigs.push_back(opcredsUpdateNOC);
    clusterConfig->commandConfigs.push_back(opcredsRemoveFabric);
    clusterConfig->commandConfigs.push_back(opcredsAddTrustedRootCertificate);
    clusterConfig->commandConfigs.push_back(opcredsEndOfAcceptedCommandList);
    clusterConfig->commandConfigs.push_back(opcredsAttestationResponse);
    clusterConfig->commandConfigs.push_back(opcredsCertificateChainResponse);
    clusterConfig->commandConfigs.push_back(opcredsCSRResponse);
    clusterConfig->commandConfigs.push_back(opcredsNOCResponse);
    clusterConfig->commandConfigs.push_back(opcredsEndOfGeneratedCommandList);
}

void matter_cluster_group_key_management_server(ClusterConfig *clusterConfig)
{
    AttributeConfig gkmGroupKeyMap(0x00000000, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gkmGroupTable(0x00000001, ZAP_TYPE(ARRAY), ZAP_EMPTY_DEFAULT(), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gkmMaxGroupsPerFabric(0x00000002, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gkmMaxGroupKeysPerFabric(0x00000003, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gkmFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(0), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig gkmClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(1), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    clusterConfig->clusterId = 0x0000003F;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER);
    clusterConfig->attributeConfigs.push_back(gkmGroupKeyMap);
    clusterConfig->attributeConfigs.push_back(gkmGroupTable);
    clusterConfig->attributeConfigs.push_back(gkmMaxGroupsPerFabric);
    clusterConfig->attributeConfigs.push_back(gkmMaxGroupKeysPerFabric);
    clusterConfig->attributeConfigs.push_back(gkmFeatureMap);
    clusterConfig->attributeConfigs.push_back(gkmClusterRevision);
}

void matter_cluster_identify_server(ClusterConfig *clusterConfig)
{
    AttributeConfig identifyIdentifyTime(0x00000000, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(0x0000), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE));
    AttributeConfig identifyIdentifyType(0x00000001, ZAP_TYPE(ENUM8), ZAP_SIMPLE_DEFAULT(0x0), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig identifyFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(0), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig identifyClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(4), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    CommandConfig identifyIdentify(0x00000000, COMMAND_MASK_ACCEPTED);
    CommandConfig identifyTriggerEffect(0x00000040, COMMAND_MASK_ACCEPTED);
    CommandConfig identifyEndOfAcceptedCommandList(chip::kInvalidCommandId, COMMAND_MASK_ACCEPTED);

    clusterConfig->clusterId = 0x00000003;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION) | ZAP_CLUSTER_MASK(ATTRIBUTE_CHANGED_FUNCTION);
    clusterConfig->attributeConfigs.push_back(identifyIdentifyTime);
    clusterConfig->attributeConfigs.push_back(identifyIdentifyType);
    clusterConfig->attributeConfigs.push_back(identifyFeatureMap);
    clusterConfig->attributeConfigs.push_back(identifyClusterRevision);
    clusterConfig->commandConfigs.push_back(identifyIdentify);
    clusterConfig->commandConfigs.push_back(identifyTriggerEffect);
    clusterConfig->commandConfigs.push_back(identifyEndOfAcceptedCommandList);
    clusterConfig->functionConfigs.push_back((EmberAfGenericClusterFunction) emberAfIdentifyClusterServerInitCallback);
    clusterConfig->functionConfigs.push_back((EmberAfGenericClusterFunction) MatterIdentifyClusterServerAttributeChangedCallback);
}

void matter_cluster_groups_server(ClusterConfig *clusterConfig)
{
    AttributeConfig groupsNameSupport(0x00000000, ZAP_TYPE(BITMAP8), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig groupsFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(0), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig groupsClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(4), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    CommandConfig groupsAddGroup(0x00000000, COMMAND_MASK_ACCEPTED);
    CommandConfig groupsViewGroup(0x00000001, COMMAND_MASK_ACCEPTED);
    CommandConfig groupsGetGroupMembership(0x00000002, COMMAND_MASK_ACCEPTED);
    CommandConfig groupsRemoveGroup(0x00000003, COMMAND_MASK_ACCEPTED);
    CommandConfig groupsRemoveAllGroups(0x00000004, COMMAND_MASK_ACCEPTED);
    CommandConfig groupsAddGroupIfIdentifying(0x00000005, COMMAND_MASK_ACCEPTED);
    CommandConfig groupsEndOfAcceptedCommandList(chip::kInvalidCommandId, COMMAND_MASK_ACCEPTED);

    CommandConfig groupsAddGroupResponse(0x00000000, COMMAND_MASK_GENERATED);
    CommandConfig groupsViewGroupResponse(0x00000001, COMMAND_MASK_GENERATED);
    CommandConfig groupsGetGroupMembershipResponse(0x00000002, COMMAND_MASK_GENERATED);
    CommandConfig groupsRemoveGroupResponse(0x00000003, COMMAND_MASK_GENERATED);
    CommandConfig groupsEndOfGeneratedCommandList(chip::kInvalidCommandId, COMMAND_MASK_GENERATED);

    clusterConfig->clusterId = 0x00000004;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION);
    clusterConfig->attributeConfigs.push_back(groupsNameSupport);
    clusterConfig->attributeConfigs.push_back(groupsFeatureMap);
    clusterConfig->attributeConfigs.push_back(groupsClusterRevision);
    clusterConfig->commandConfigs.push_back(groupsAddGroup);
    clusterConfig->commandConfigs.push_back(groupsViewGroup);
    clusterConfig->commandConfigs.push_back(groupsGetGroupMembership);
    clusterConfig->commandConfigs.push_back(groupsRemoveGroup);
    clusterConfig->commandConfigs.push_back(groupsRemoveAllGroups);
    clusterConfig->commandConfigs.push_back(groupsAddGroupIfIdentifying);
    clusterConfig->commandConfigs.push_back(groupsEndOfAcceptedCommandList);
    clusterConfig->commandConfigs.push_back(groupsAddGroupResponse);
    clusterConfig->commandConfigs.push_back(groupsViewGroupResponse);
    clusterConfig->commandConfigs.push_back(groupsGetGroupMembershipResponse);
    clusterConfig->commandConfigs.push_back(groupsRemoveGroupResponse);
    clusterConfig->commandConfigs.push_back(groupsEndOfGeneratedCommandList);
    clusterConfig->functionConfigs.push_back((EmberAfGenericClusterFunction) emberAfGroupsClusterServerInitCallback);
}

void matter_cluster_scenes_server(ClusterConfig *clusterConfig)
{
    AttributeConfig scenesSceneCount(0x00000000, ZAP_TYPE(INT8U), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig scenesCurrentScene(0x00000001, ZAP_TYPE(INT8U), ZAP_SIMPLE_DEFAULT(0x00), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig scenesCurrentGroup(0x00000002, ZAP_TYPE(GROUP_ID), ZAP_SIMPLE_DEFAULT(0x0000), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig scenesSceneValid(0x00000003, ZAP_TYPE(BOOLEAN), ZAP_SIMPLE_DEFAULT(0x00), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig scenesNameSupport(0x00000004, ZAP_TYPE(BITMAP8), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig scenesLastConfiguredBy(0x00000005, ZAP_TYPE(NODE_ID), uint32_t(0), 8, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig scenesSceneTableSize(0x00000006, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig scenesRemainingCapacity(0x00000007, ZAP_TYPE(INT8U), ZAP_EMPTY_DEFAULT(), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig scenesFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(0), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig scenesClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(5), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    CommandConfig scenesAddScene(0x00000000, COMMAND_MASK_ACCEPTED);
    CommandConfig scenesViewScene(0x00000001, COMMAND_MASK_ACCEPTED);
    CommandConfig scenesRemoveScene(0x00000002, COMMAND_MASK_ACCEPTED);
    CommandConfig scenesRemoveAllScenes(0x00000003, COMMAND_MASK_ACCEPTED);
    CommandConfig scenesStoreScene(0x00000004, COMMAND_MASK_ACCEPTED);
    CommandConfig scenesRecallScene(0x00000005, COMMAND_MASK_ACCEPTED);
    CommandConfig scenesGetSceneMembership(0x00000006, COMMAND_MASK_ACCEPTED);
    CommandConfig scenesEndOfAcceptedCommandList(chip::kInvalidCommandId, COMMAND_MASK_ACCEPTED);
    
    CommandConfig scenesAddSceneResponse(0x00000000, COMMAND_MASK_GENERATED);
    CommandConfig scenesViewSceneResponse(0x00000001, COMMAND_MASK_GENERATED);
    CommandConfig scenesRemoveSceneResponse(0x00000002, COMMAND_MASK_GENERATED);
    CommandConfig scenesRemoveAllScenesResponse(0x00000003, COMMAND_MASK_GENERATED);
    CommandConfig scenesStoreSceneResponse(0x00000004, COMMAND_MASK_GENERATED);
    CommandConfig scenesGetSceneMembershipResponse(0x00000006, COMMAND_MASK_GENERATED);
    CommandConfig scenesEndOfGeneratedCommandList(chip::kInvalidCommandId, COMMAND_MASK_GENERATED);

    clusterConfig->clusterId = 0x00000005;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER);
    clusterConfig->attributeConfigs.push_back(scenesSceneCount);
    clusterConfig->attributeConfigs.push_back(scenesCurrentScene);
    clusterConfig->attributeConfigs.push_back(scenesCurrentGroup);
    clusterConfig->attributeConfigs.push_back(scenesSceneValid);
    clusterConfig->attributeConfigs.push_back(scenesNameSupport);
    clusterConfig->attributeConfigs.push_back(scenesLastConfiguredBy);
    clusterConfig->attributeConfigs.push_back(scenesSceneTableSize);
    clusterConfig->attributeConfigs.push_back(scenesRemainingCapacity);
    clusterConfig->attributeConfigs.push_back(scenesFeatureMap);
    clusterConfig->attributeConfigs.push_back(scenesClusterRevision);
    clusterConfig->commandConfigs.push_back(scenesAddScene);
    clusterConfig->commandConfigs.push_back(scenesViewScene);
    clusterConfig->commandConfigs.push_back(scenesRemoveScene);
    clusterConfig->commandConfigs.push_back(scenesRemoveAllScenes);
    clusterConfig->commandConfigs.push_back(scenesStoreScene);
    clusterConfig->commandConfigs.push_back(scenesRecallScene);
    clusterConfig->commandConfigs.push_back(scenesGetSceneMembership);
    clusterConfig->commandConfigs.push_back(scenesEndOfAcceptedCommandList);
    clusterConfig->commandConfigs.push_back(scenesAddSceneResponse);
    clusterConfig->commandConfigs.push_back(scenesViewSceneResponse);
    clusterConfig->commandConfigs.push_back(scenesRemoveSceneResponse);
    clusterConfig->commandConfigs.push_back(scenesRemoveAllScenesResponse);
    clusterConfig->commandConfigs.push_back(scenesStoreSceneResponse);
    clusterConfig->commandConfigs.push_back(scenesGetSceneMembershipResponse);
    clusterConfig->commandConfigs.push_back(scenesEndOfGeneratedCommandList);
}

void matter_cluster_onoff_server(ClusterConfig *clusterConfig)
{
    AttributeConfig onoffOnOff(0x00000000, ZAP_TYPE(BOOLEAN), ZAP_SIMPLE_DEFAULT(0x00), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(TOKENIZE));
    AttributeConfig onoffGlobalSceneControl(0x00004000, ZAP_TYPE(BOOLEAN), ZAP_SIMPLE_DEFAULT(0x01), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig onoffOnTime(0x00004001, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(0x0000), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE));
    AttributeConfig onoffOffWaitTime(0x00004002, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(0x0000), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE));
    AttributeConfig onoffStartUpOnOff(0x00004003, ZAP_TYPE(ENUM8), &onoffStartUpOnOffMinMaxValue, 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(TOKENIZE) | ZAP_ATTRIBUTE_MASK(WRITABLE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig onoffFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(1), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig onoffClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(4), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    CommandConfig onoffOff(0x00000000, COMMAND_MASK_ACCEPTED);
    CommandConfig onoffOn(0x00000001, COMMAND_MASK_ACCEPTED);
    CommandConfig onoffToggle(0x00000002, COMMAND_MASK_ACCEPTED);
    CommandConfig onoffOffWithEffect(0x00000040, COMMAND_MASK_ACCEPTED);
    CommandConfig onoffOnWithRecallGlobalScene(0x00000041, COMMAND_MASK_ACCEPTED);
    CommandConfig onoffOnWithTimedOff(0x00000042, COMMAND_MASK_ACCEPTED);
    CommandConfig onoffEndOfAcceptedCommandList(chip::kInvalidCommandId, COMMAND_MASK_ACCEPTED);

    CommandConfig onoffMoveToLevel(0x00000000, COMMAND_MASK_GENERATED);
    CommandConfig onoffMove(0x00000001, COMMAND_MASK_GENERATED);
    CommandConfig onoffStep(0x00000002, COMMAND_MASK_GENERATED);
    CommandConfig onoffStop(0x00000003, COMMAND_MASK_GENERATED);
    CommandConfig onoffMoveToLevelWithOnOff(0x00000004, COMMAND_MASK_GENERATED);
    CommandConfig onoffMoveWithOnOff(0x00000005, COMMAND_MASK_GENERATED);
    CommandConfig onoffStepWithOnOff(0x00000006, COMMAND_MASK_GENERATED);
    CommandConfig onoffStopWithOnOff(0x00000007, COMMAND_MASK_GENERATED);
    CommandConfig onoffEndOfGeneratedCommandList(chip::kInvalidCommandId, COMMAND_MASK_GENERATED);

    clusterConfig->clusterId = 0x00000006;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION) | ZAP_CLUSTER_MASK(SHUTDOWN_FUNCTION);
    clusterConfig->attributeConfigs.push_back(onoffOnOff);
    clusterConfig->attributeConfigs.push_back(onoffGlobalSceneControl);
    clusterConfig->attributeConfigs.push_back(onoffOnTime);
    clusterConfig->attributeConfigs.push_back(onoffOffWaitTime);
    clusterConfig->attributeConfigs.push_back(onoffStartUpOnOff);
    clusterConfig->attributeConfigs.push_back(onoffFeatureMap);
    clusterConfig->attributeConfigs.push_back(onoffClusterRevision);
    clusterConfig->commandConfigs.push_back(onoffOff);
    clusterConfig->commandConfigs.push_back(onoffOn);
    clusterConfig->commandConfigs.push_back(onoffToggle);
    clusterConfig->commandConfigs.push_back(onoffOffWithEffect);
    clusterConfig->commandConfigs.push_back(onoffOnWithRecallGlobalScene);
    clusterConfig->commandConfigs.push_back(onoffOnWithTimedOff);
    clusterConfig->commandConfigs.push_back(onoffEndOfAcceptedCommandList);
    clusterConfig->commandConfigs.push_back(onoffMoveToLevel);
    clusterConfig->commandConfigs.push_back(onoffMove);
    clusterConfig->commandConfigs.push_back(onoffStep);
    clusterConfig->commandConfigs.push_back(onoffStop);
    clusterConfig->commandConfigs.push_back(onoffMoveToLevelWithOnOff);
    clusterConfig->commandConfigs.push_back(onoffStepWithOnOff);
    clusterConfig->commandConfigs.push_back(onoffStopWithOnOff);
    clusterConfig->commandConfigs.push_back(onoffEndOfGeneratedCommandList);
    clusterConfig->functionConfigs.push_back((EmberAfGenericClusterFunction) emberAfOnOffClusterServerInitCallback);
    clusterConfig->functionConfigs.push_back((EmberAfGenericClusterFunction) MatterOnOffClusterServerShutdownCallback);
}

void matter_cluster_level_control_server(ClusterConfig *clusterConfig)
{
    AttributeConfig levelcontrolCurrentLevel(0x00000000, ZAP_TYPE(INT8U), ZAP_SIMPLE_DEFAULT(0x01), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(TOKENIZE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig levelcontrolRemainingTime(0x00000001, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(0x0000), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig levelcontrolMinLevel(0x00000002, ZAP_TYPE(INT8U), ZAP_SIMPLE_DEFAULT(0x01), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig levelcontrolMaxLevel(0x00000003, ZAP_TYPE(INT8U), ZAP_SIMPLE_DEFAULT(0xFE), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig levelcontrolCurrentFrequency(0x00000004, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(0x0000), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig levelcontrolMinFrequency(0x00000005, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(0x0000), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig levelcontrolMaxFrequency(0x00000006, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(0x0000), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig levelcontrolOptions(0x0000000F, ZAP_TYPE(BITMAP8), &levelcontrolOptionsMinMaxValue, 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(MIN_MAX) | ZAP_ATTRIBUTE_MASK(WRITABLE));
    AttributeConfig levelcontrolOnOffTransitionTime(0x00000010, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(0x0000), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE));
    AttributeConfig levelcontrolOnLevel(0x00000011, ZAP_TYPE(INT8U), ZAP_SIMPLE_DEFAULT(0xFF), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig levelcontrolOnTransitionTime(0x00000012, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig levelcontrolOffTransitionTime(0x00000013, ZAP_TYPE(INT16U), ZAP_EMPTY_DEFAULT(), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig levelcontrolDefaultMoveRate(0x00000014, ZAP_TYPE(INT8U), ZAP_SIMPLE_DEFAULT(50), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig levelcontrolStartUpCurrentLevel(0x00004000, ZAP_TYPE(INT8U), ZAP_SIMPLE_DEFAULT(255), 1, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(TOKENIZE) | ZAP_ATTRIBUTE_MASK(WRITABLE) | ZAP_ATTRIBUTE_MASK(NULLABLE));
    AttributeConfig levelcontrolFeatureMap(0x0000FFFC, ZAP_TYPE(BITMAP32), ZAP_SIMPLE_DEFAULT(3), 4, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));
    AttributeConfig levelcontrolClusterRevision(0x0000FFFD, ZAP_TYPE(INT16U), ZAP_SIMPLE_DEFAULT(5), 2, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE));

    CommandConfig levelcontrolMoveToLevel(0x00000000, COMMAND_MASK_ACCEPTED);
    CommandConfig levelcontrolMove(0x00000001, COMMAND_MASK_ACCEPTED);
    CommandConfig levelcontrolStep(0x00000002, COMMAND_MASK_ACCEPTED);
    CommandConfig levelcontrolStop(0x00000003, COMMAND_MASK_ACCEPTED);
    CommandConfig levelcontrolMoveToLevelWithOnOff(0x00000004, COMMAND_MASK_ACCEPTED);
    CommandConfig levelcontrolMoveWithOnOff(0x00000005, COMMAND_MASK_ACCEPTED);
    CommandConfig levelcontrolStepWithOnOff(0x00000006, COMMAND_MASK_ACCEPTED);
    CommandConfig levelcontrolStopWithOnOff(0x00000007, COMMAND_MASK_ACCEPTED);
    CommandConfig levelcontrolEndOfAcceptedCommandList(chip::kInvalidCommandId, COMMAND_MASK_ACCEPTED);

    clusterConfig->clusterId = 0x00000008;
    clusterConfig->mask = ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION) | ZAP_CLUSTER_MASK(SHUTDOWN_FUNCTION);
    clusterConfig->attributeConfigs.push_back(levelcontrolCurrentLevel);
    clusterConfig->attributeConfigs.push_back(levelcontrolRemainingTime);
    clusterConfig->attributeConfigs.push_back(levelcontrolMinLevel);
    clusterConfig->attributeConfigs.push_back(levelcontrolMaxLevel);
    clusterConfig->attributeConfigs.push_back(levelcontrolCurrentFrequency);
    clusterConfig->attributeConfigs.push_back(levelcontrolMinFrequency);
    clusterConfig->attributeConfigs.push_back(levelcontrolMaxFrequency);
    clusterConfig->attributeConfigs.push_back(levelcontrolOptions);
    clusterConfig->attributeConfigs.push_back(levelcontrolOnOffTransitionTime);
    clusterConfig->attributeConfigs.push_back(levelcontrolOnLevel);
    clusterConfig->attributeConfigs.push_back(levelcontrolOnTransitionTime);
    clusterConfig->attributeConfigs.push_back(levelcontrolOffTransitionTime);
    clusterConfig->attributeConfigs.push_back(levelcontrolDefaultMoveRate);
    clusterConfig->attributeConfigs.push_back(levelcontrolStartUpCurrentLevel);
    clusterConfig->attributeConfigs.push_back(levelcontrolFeatureMap);
    clusterConfig->attributeConfigs.push_back(levelcontrolClusterRevision);
    clusterConfig->commandConfigs.push_back(levelcontrolMoveToLevel);
    clusterConfig->commandConfigs.push_back(levelcontrolMove);
    clusterConfig->commandConfigs.push_back(levelcontrolStep);
    clusterConfig->commandConfigs.push_back(levelcontrolStop);
    clusterConfig->commandConfigs.push_back(levelcontrolMoveToLevelWithOnOff);
    clusterConfig->commandConfigs.push_back(levelcontrolMoveWithOnOff);
    clusterConfig->commandConfigs.push_back(levelcontrolStepWithOnOff);
    clusterConfig->commandConfigs.push_back(levelcontrolStopWithOnOff);
    clusterConfig->commandConfigs.push_back(levelcontrolEndOfAcceptedCommandList);
    clusterConfig->functionConfigs.push_back((EmberAfGenericClusterFunction) emberAfLevelControlClusterServerInitCallback);
    clusterConfig->functionConfigs.push_back((EmberAfGenericClusterFunction) MatterLevelControlClusterServerShutdownCallback);
}

} // Clusters

namespace Endpoints {

void matter_root_node_preset(EndpointConfig *rootNodeEndpointConfig)
{
    ClusterConfig descriptorServerCluster;
    ClusterConfig aclServerCluster;
    ClusterConfig basicInformationServerCluster;
    ClusterConfig otarServerCluster;
    ClusterConfig generalCommissioningServerCluster;
    ClusterConfig networkCommissioningServerCluster;
    ClusterConfig generalDiagnosticsServerCluster;
    ClusterConfig softwareDiagnosticsServerCluster;
    ClusterConfig wifiDiagnosticsServerCluster;
    ClusterConfig administratorCommissioningServerCluster;
    ClusterConfig operationalCredentialsServerCluster;
    ClusterConfig groupKeyManagementServerCluster;

    Presets::Clusters::matter_cluster_descriptor_server(&descriptorServerCluster);
    Presets::Clusters::matter_cluster_acl_server(&aclServerCluster);
    Presets::Clusters::matter_cluster_basic_information_server(&basicInformationServerCluster);
    Presets::Clusters::matter_cluster_ota_requestor_server(&otarServerCluster);
    Presets::Clusters::matter_cluster_general_commissioning_server(&generalCommissioningServerCluster);
    Presets::Clusters::matter_cluster_network_commissioning_server(&networkCommissioningServerCluster);
    Presets::Clusters::matter_cluster_general_diagnostics_server(&generalDiagnosticsServerCluster);
    Presets::Clusters::matter_cluster_software_diagnostics_server(&softwareDiagnosticsServerCluster);
    Presets::Clusters::matter_cluster_wifi_diagnostics_server(&wifiDiagnosticsServerCluster);
    Presets::Clusters::matter_cluster_administrator_commissioning_server(&administratorCommissioningServerCluster);
    Presets::Clusters::matter_cluster_operational_credentials_server(&operationalCredentialsServerCluster);
    Presets::Clusters::matter_cluster_group_key_management_server(&groupKeyManagementServerCluster);

    rootNodeEndpointConfig->clusterConfigs.push_back(descriptorServerCluster);
    rootNodeEndpointConfig->clusterConfigs.push_back(aclServerCluster);
    rootNodeEndpointConfig->clusterConfigs.push_back(basicInformationServerCluster);
    rootNodeEndpointConfig->clusterConfigs.push_back(otarServerCluster);
    rootNodeEndpointConfig->clusterConfigs.push_back(generalCommissioningServerCluster);
    rootNodeEndpointConfig->clusterConfigs.push_back(networkCommissioningServerCluster);
    rootNodeEndpointConfig->clusterConfigs.push_back(generalDiagnosticsServerCluster);
    rootNodeEndpointConfig->clusterConfigs.push_back(softwareDiagnosticsServerCluster);
    rootNodeEndpointConfig->clusterConfigs.push_back(wifiDiagnosticsServerCluster);
    rootNodeEndpointConfig->clusterConfigs.push_back(administratorCommissioningServerCluster);
    rootNodeEndpointConfig->clusterConfigs.push_back(operationalCredentialsServerCluster);
    rootNodeEndpointConfig->clusterConfigs.push_back(groupKeyManagementServerCluster);
}

void matter_dimmable_light_preset(EndpointConfig *dimmableLightEndpointConfig)
{
    ClusterConfig descriptorServerCluster;
    ClusterConfig identifyServerCluster;
    ClusterConfig groupsServerCluster;
    ClusterConfig scenesServerCluster;
    ClusterConfig onOffServerCluster;
    ClusterConfig levelControlServerCluster;

    Presets::Clusters::matter_cluster_descriptor_server(&descriptorServerCluster);
    Presets::Clusters::matter_cluster_identify_server(&identifyServerCluster);
    Presets::Clusters::matter_cluster_groups_server(&groupsServerCluster);
    Presets::Clusters::matter_cluster_scenes_server(&scenesServerCluster);
    Presets::Clusters::matter_cluster_onoff_server(&onOffServerCluster);
    Presets::Clusters::matter_cluster_level_control_server(&levelControlServerCluster);

    dimmableLightEndpointConfig->clusterConfigs.push_back(identifyServerCluster);
    dimmableLightEndpointConfig->clusterConfigs.push_back(groupsServerCluster);
    dimmableLightEndpointConfig->clusterConfigs.push_back(scenesServerCluster);
    dimmableLightEndpointConfig->clusterConfigs.push_back(onOffServerCluster);
    dimmableLightEndpointConfig->clusterConfigs.push_back(levelControlServerCluster);
}

}

} // Presets
