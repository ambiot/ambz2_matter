/**
*****************************************************************************************
*     Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     bt_mesh_provisioner_api.h
  * @brief    Source file for provisioner cmd.
  * @details  User command interfaces.
  * @author   sherman
  * @date     2019-09-16
  * @version  v1.0
  * *************************************************************************************
  */
#ifndef _BT_MESH_PROV_API_H_
#define _BT_MESH_PROV_API_H_

#include "bt_mesh_user_api.h"

#define BT_MESH_PROV_CMD_RETRY_COUNT 2

/** @brief bt mesh provisioner command table */
enum bt_mesh_provisioner_cmd
{
	GEN_MESH_CODE(_pb_adv_con) ,	/*0*/
 	GEN_MESH_CODE(_prov) ,  
 	GEN_MESH_CODE(_prov_stop) ,
 	GEN_MESH_CODE(_app_key_add) , 
	GEN_MESH_CODE(_model_app_bind) , 
	GEN_MESH_CODE(_model_pub_set) ,  /*5*/
	GEN_MESH_CODE(_generic_on_off_set) , 
	GEN_MESH_CODE(_generic_on_off_get) ,
	GEN_MESH_CODE(_node_reset) ,
 	GEN_MESH_CODE(_model_sub_delete) ,
 	GEN_MESH_CODE(_model_sub_add) , /*10*/
 	GEN_MESH_CODE(_model_sub_get) , 
    GEN_MESH_CODE(_prov_discover) ,
    GEN_MESH_CODE(_prov_cccd_operate) ,
    GEN_MESH_CODE(_proxy_discover) ,
    GEN_MESH_CODE(_proxy_cccd_operate) , /*15*/
    GEN_MESH_CODE(_datatrans_write) , 
    GEN_MESH_CODE(_datatrans_read) ,
    GEN_MESH_CODE(_connect) , 
 	GEN_MESH_CODE(_disconnect) ,
 	GEN_MESH_CODE(_list) ,
 	GEN_MESH_CODE(_dev_info_show) ,
 	GEN_MESH_CODE(_fn_init) ,
 	GEN_MESH_CODE(_fn_deinit) ,
 	GEN_MESH_CODE(_light_lightness_get) ,
 	GEN_MESH_CODE(_light_lightness_set) ,
 	GEN_MESH_CODE(_light_lightness_linear_get) ,
 	GEN_MESH_CODE(_light_lightness_linear_set) ,
 	GEN_MESH_CODE(_light_lightness_last_get) ,
 	GEN_MESH_CODE(_light_lightness_default_get) ,
 	GEN_MESH_CODE(_light_lightness_default_set) ,
 	GEN_MESH_CODE(_light_lightness_range_get) ,
 	GEN_MESH_CODE(_light_lightness_range_set) ,
 	GEN_MESH_CODE(_light_ctl_get) ,
 	GEN_MESH_CODE(_light_ctl_set) ,
 	GEN_MESH_CODE(_light_ctl_temperature_get) ,
 	GEN_MESH_CODE(_light_ctl_temperature_set) ,
 	GEN_MESH_CODE(_light_ctl_temperature_range_get) ,
 	GEN_MESH_CODE(_light_ctl_temperature_range_set) ,
 	GEN_MESH_CODE(_light_ctl_default_get) ,
 	GEN_MESH_CODE(_light_ctl_default_set) ,
 	GEN_MESH_CODE(_light_hsl_get) ,
 	GEN_MESH_CODE(_light_hsl_set) ,
 	GEN_MESH_CODE(_light_hsl_target_get) ,
 	GEN_MESH_CODE(_light_hsl_hue_get) ,
 	GEN_MESH_CODE(_light_hsl_hue_set) ,
 	GEN_MESH_CODE(_light_hsl_saturation_get) ,
 	GEN_MESH_CODE(_light_hsl_saturation_set) ,
 	GEN_MESH_CODE(_light_hsl_default_get) ,
 	GEN_MESH_CODE(_light_hsl_default_set) ,
 	GEN_MESH_CODE(_light_hsl_range_get) ,
 	GEN_MESH_CODE(_light_hsl_range_set) ,
	GEN_MESH_CODE(_light_xyl_get) ,
	GEN_MESH_CODE(_light_xyl_set) ,
	GEN_MESH_CODE(_light_xyl_target_get) ,
	GEN_MESH_CODE(_light_xyl_default_get) ,
	GEN_MESH_CODE(_light_xyl_default_set) ,
	GEN_MESH_CODE(_light_xyl_range_get) ,
	GEN_MESH_CODE(_light_xyl_range_set) ,
	GEN_MESH_CODE(_time_set) ,
	GEN_MESH_CODE(_time_get) ,
	GEN_MESH_CODE(_time_zone_set) ,
	GEN_MESH_CODE(_time_zone_get) ,
	GEN_MESH_CODE(_time_tai_utc_delta_set) ,
	GEN_MESH_CODE(_time_tai_utc_delta_get) ,
	GEN_MESH_CODE(_time_role_set) ,
	GEN_MESH_CODE(_time_role_get) ,
	GEN_MESH_CODE(_scene_store) ,
	GEN_MESH_CODE(_scene_recall) ,
	GEN_MESH_CODE(_scene_get) ,
	GEN_MESH_CODE(_scene_register_get) ,
	GEN_MESH_CODE(_scene_delete) ,
	GEN_MESH_CODE(_scheduler_get) ,
	GEN_MESH_CODE(_scheduler_action_get) ,
	GEN_MESH_CODE(_scheduler_action_set) ,
	/******new model test*****/
	GEN_MESH_CODE(_gdtt_get) ,
	GEN_MESH_CODE(_gdtt_set) ,
	GEN_MESH_CODE(_generic_level_get) ,
	GEN_MESH_CODE(_generic_level_set) ,
	GEN_MESH_CODE(_generic_delta_set) ,
    GEN_MESH_CODE(_generic_move_set) ,
    GEN_MESH_CODE(_generic_on_powerup_get) ,
    GEN_MESH_CODE(_generic_on_powerup_set) ,
    GEN_MESH_CODE(_generic_power_level_get) ,
    GEN_MESH_CODE(_generic_power_level_set) ,
    GEN_MESH_CODE(_generic_power_last_get) ,
    GEN_MESH_CODE(_generic_power_default_get) ,
    GEN_MESH_CODE(_generic_power_default_set) ,
    GEN_MESH_CODE(_generic_power_range_get) ,
    GEN_MESH_CODE(_generic_power_range_set) ,
    GEN_MESH_CODE(_generic_battery_get) ,
    GEN_MESH_CODE(_sensor_descriptor_get) ,
    GEN_MESH_CODE(_sensor_cadence_get) ,
    GEN_MESH_CODE(_sensor_cadence_set) ,
    GEN_MESH_CODE(_sensor_settings_get) ,
    GEN_MESH_CODE(_sensor_setting_set) ,
    GEN_MESH_CODE(_sensor_setting_get) ,
    GEN_MESH_CODE(_sensor_get) ,
    GEN_MESH_CODE(_sensor_column_get) ,
    GEN_MESH_CODE(_sensor_series_get) ,
	GEN_MESH_CODE(_generic_location_global_get) ,
	GEN_MESH_CODE(_generic_location_global_set) ,
	GEN_MESH_CODE(_generic_location_local_get) ,
	GEN_MESH_CODE(_generic_location_local_set) ,
	GEN_MESH_CODE(_generic_user_properties_get) ,
	GEN_MESH_CODE(_generic_user_property_get) ,
	GEN_MESH_CODE(_generic_user_property_set) ,
	GEN_MESH_CODE(_generic_admin_properties_get) ,
	GEN_MESH_CODE(_generic_admin_property_get) ,
	GEN_MESH_CODE(_generic_admin_property_set) ,
	GEN_MESH_CODE(_generic_manufacturer_properties_get) ,
	GEN_MESH_CODE(_generic_manufacturer_property_get) ,
	GEN_MESH_CODE(_generic_manufacturer_property_set) ,
	GEN_MESH_CODE(_generic_client_properties_get) ,
	GEN_MESH_CODE(_light_lc_mode_get) ,
	GEN_MESH_CODE(_light_lc_mode_set) ,
	GEN_MESH_CODE(_light_lc_om_get) ,
	GEN_MESH_CODE(_light_lc_om_set) ,
	GEN_MESH_CODE(_light_lc_light_on_off_get) ,
	GEN_MESH_CODE(_light_lc_light_on_off_set) ,
	GEN_MESH_CODE(_light_lc_property_get) ,
	GEN_MESH_CODE(_light_lc_property_set) ,
#if F_BT_MESH_1_1_RPR_SUPPORT
	GEN_MESH_CODE(_rmt_prov_client_scan_start) ,
	GEN_MESH_CODE(_rmt_prov_client_link_open_prov) ,
	GEN_MESH_CODE(_rmt_prov_client_close) ,
#endif
#if defined(MESH_DFU) && MESH_DFU
	GEN_MESH_CODE(_fw_update_info_get) ,
	GEN_MESH_CODE(_fw_update_start) ,
	GEN_MESH_CODE(_fw_update_cancel) ,
#endif
#if F_BT_MESH_1_1_DF_SUPPORT
    GEN_MESH_CODE(_directed_control_set),
    GEN_MESH_CODE(_directed_publish_policy_set),
    GEN_MESH_CODE(_forwarding_table_add),
	GEN_MESH_CODE(_forwarding_table_dependents_add),
	GEN_MESH_CODE(_forwarding_table_delete),
	GEN_MESH_CODE(_forwarding_table_dependents_delete),
	GEN_MESH_CODE(_wanted_lanes_set),
	GEN_MESH_CODE(_two_way_path_set),
	GEN_MESH_CODE(_rssi_threshold_set),
	GEN_MESH_CODE(_discovery_table_capabilities_set),
	GEN_MESH_CODE(_path_echo_interval_set) ,
	GEN_MESH_CODE(_path_metric_set) ,
	GEN_MESH_CODE(_df_path_discovery) ,
  	GEN_MESH_CODE(_df_path_solicitation) ,
  	GEN_MESH_CODE(_df_path_dependents_update) ,
#endif
	MAX_MESH_PROVISIONER_CMD
};

/**
  * @brief initialize bt mesh provisioner api
  *
  * @return 1: success 2: fail
  */
uint8_t bt_mesh_provisioner_api_init(void);

/**
  * @brief deinitialize bt mesh provisioner api
  *
  * @return none
  */
void bt_mesh_provisioner_api_deinit(void);

#endif

