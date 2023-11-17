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
void matter_cluster_identify_server(ClusterConfig *clusterConfig);
void matter_cluster_groups_server(ClusterConfig *clusterConfig);
void matter_cluster_scenes_server(ClusterConfig *clusterConfig);
void matter_cluster_onoff_server(ClusterConfig *clusterConfig);
void matter_cluster_level_control_server(ClusterConfig *clusterConfig);
void matter_cluster_thermostat_server(ClusterConfig *clusterConfig);
void matter_cluster_fan_control_server(ClusterConfig *clusterConfig);
void matter_cluster_temperature_measurement_server(ClusterConfig *clusterConfig);
void matter_cluster_relative_humidity_measurement_server(ClusterConfig *clusterConfig);
void matter_cluster_laundrymode_server(ClusterConfig *clusterConfig);
void matter_cluster_laundrywasher_control_server(ClusterConfig *clusterConfig);
void matter_cluster_temperature_control_server(ClusterConfig *clusterConfig);
void matter_cluster_operational_state_server(ClusterConfig *clusterConfig);
void matter_cluster_refrigerator_and_temperature_controlled_cabinet_mode_server(ClusterConfig *clusterConfig);
void matter_cluster_refrigerator_alarm_server(ClusterConfig *clusterConfig);

} // Clusters

namespace Endpoints {

void matter_root_node_preset(EndpointConfig *rootNodeEndpointConfig);
void matter_dimmable_light_preset(EndpointConfig *dimmableLightEndpointConfig);
void matter_room_air_conditioner_preset(EndpointConfig *dimmableLightEndpointConfig);
void matter_aggregator_preset(EndpointConfig *aggregatorEndpointConfig);
void matter_laundrywasher_preset(EndpointConfig *RefrigeratorEndpointConfig);
void matter_refrigerator_preset(EndpointConfig *RefrigeratorEndpointConfig);

} // Endpoints
} // Presets
