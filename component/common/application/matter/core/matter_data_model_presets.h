#pragma once

namespace Presets {
namespace Clusters {

void matter_cluster_descriptor_server(ClusterConfig *clusterConfig);
void matter_cluster_acl_server(ClusterConfig *clusterConfig);
void matter_cluster_basic_information_server(ClusterConfig *clusterConfig);
void matter_cluster_ota_requestor_server(ClusterConfig *clusterConfig);
void matter_cluster_general_commissioning_server(ClusterConfig *clusterConfig);
void matter_cluster_network_commissioning_server(ClusterConfig *clusterConfig);
void matter_cluster_general_diagnostics_server(ClusterConfig *clusterConfig);
void matter_cluster_software_diagnostics_server(ClusterConfig *clusterConfig);
void matter_cluster_wifi_diagnostics_server(ClusterConfig *clusterConfig);
void matter_cluster_administrator_commissioning_server(ClusterConfig *clusterConfig);
void matter_cluster_operational_credentials_server(ClusterConfig *clusterConfig);
void matter_cluster_group_key_management_server(ClusterConfig *clusterConfig);

} // Clusters
  //
namespace Endpoints {

void matter_root_node_preset(EndpointConfig *rootNodeEndpointConfig);

} // Endpoints
} // Presets
