/*
 * rsi_ble_device_info_DEMO_54.h
 *
 *  Created on: 01-Oct-2019
 *      Author: root
 */

#ifndef SAPIS_EXAMPLES_RSI_DEMO_APPS_INC_RSI_BLE_DEVICE_INFO_UNIFIED_PROTOCOL_H
#define SAPIS_EXAMPLES_RSI_DEMO_APPS_INC_RSI_BLE_DEVICE_INFO_UNIFIED_PROTOCOL_H
#include "rsi_common_app.h"
#if (UNIFIED_PROTOCOL && RSI_ENABLE_BLE_TEST)
#include "rsi_driver.h"
#include "rsi_ble_apis.h"
#define RSI_REM_DEV_ADDR_LEN 18
#define RSI_REM_DEV_NAME_LEN 31

// enable this, to send writes without responses
#define SEND_W_W_R                   0
#define SEND_NOTIFICATION            0
#define RSI_BLE_MAX_CHAR_DESCRIPTORS 5
#if RESOLVE_ENABLE
#define RSI_MAX_LIST_SIZE 0x05
#endif

#define MASTER_ROLE 2
#define SLAVE_ROLE  1

#define NO_SLAVE_FOUND      0
#define SLAVE_FOUND         1
#define SLAVE_CONNECTED     2
#define SLAVE_NOT_CONNECTED 3

typedef struct rsi_ble_conn_info_s {
  uint8_t conn_id;
  uint8_t conn_status;
  uint8_t remote_device_role; //! 1 - remote device is slave, 2 - remote device is master
  uint8_t remote_dev_addr[RSI_REM_DEV_ADDR_LEN];
  rsi_ble_event_remote_features_t remote_dev_feature;    //! -- rsi_ble_simple_peripheral_on_remote_features_event()
  rsi_ble_event_adv_report_t rsi_app_adv_reports_to_app; //! -- rsi_ble_simple_central_on_adv_report_event()
  rsi_ble_event_conn_update_t conn_update_resp;          //! -- ble_on_conn_update_complete_event()
  rsi_ble_event_remote_conn_param_req_t
    rsi_app_remote_device_conn_params; //! -- rsi_ble_on_remote_conn_params_request_event()
  rsi_ble_event_conn_status_t
    conn_event_to_app; //! -- rsi_ble_on_connect_event()  rsi_ble_on_disconnect_event() rsi_ble_on_enhance_conn_status_event()
  rsi_ble_event_enhance_conn_status_t rsi_enhc_conn_status;
  rsi_ble_event_write_t app_ble_write_event; //! -- rsi_ble_on_gatt_write_event()
  rsi_ble_set_att_resp_t rsi_ble_write_resp_event;
  rsi_ble_read_req_t app_ble_read_event;                      //! -- rsi_ble_on_read_req_event()
  rsi_ble_event_mtu_t app_ble_mtu_event;                      //! -- rsi_ble_on_mtu_event()
  rsi_ble_event_mtu_exchange_information_t mtu_exchange_info; //! --rsi_ble_on_mtu_exchange_info_t
  rsi_ble_event_disconnect_t rsi_ble_disconn_resp;            //! -- rsi_ble_on_disconnect_event()
  rsi_ble_event_profiles_list_t get_allprofiles;              //!	-- rsi_ble_profiles_list_event()
  rsi_ble_event_profile_by_uuid_t get_profile;                //!	-- rsi_ble_profile_event()
  rsi_ble_event_read_by_type1_t get_char_services;            //! -- rsi_ble_char_services_event()
  rsi_ble_event_att_value_t rsi_char_descriptors;
  rsi_ble_event_error_resp_t rsi_ble_gatt_err_resp;
  rsi_bt_event_smp_resp_t rsi_ble_event_smp_resp;
  rsi_bt_event_smp_req_t rsi_ble_event_smp_req;
  rsi_bt_event_smp_passkey_t rsi_ble_event_smp_passkey;
  rsi_bt_event_smp_passkey_display_t rsi_ble_smp_passkey_display;
  rsi_bt_event_sc_passkey_t rsi_event_sc_passkey;
  rsi_bt_event_smp_failed_t rsi_ble_smp_failed;
  rsi_bt_event_encryption_enabled_t rsi_encryption_enabled;
  rsi_bt_event_le_ltk_request_t rsi_le_ltk_resp;
  rsi_bt_event_le_security_keys_t rsi_le_security_keys;
#if (CONNECT_OPTION == CONN_BY_NAME)
  uint8_t *rsi_remote_name;
#endif
} rsi_ble_conn_info_t;

typedef struct rsi_ble_profile_list_by_conn_s {
  profile_descriptors_t profile_desc;
  rsi_ble_event_profile_by_uuid_t profile_info_uuid;
  rsi_ble_event_read_by_type1_t profile_char_info[RSI_BLE_MAX_RESP_LIST];
  rsi_ble_event_att_value_t rsi_char_descriptors[RSI_BLE_MAX_CHAR_DESCRIPTORS];
  uint8_t no_of_char_services;
} rsi_ble_profile_list_by_conn_t;

#if RESOLVE_ENABLE
//! user defined structure
//LE resolvlist group.
typedef struct rsi_ble_resolvlist_group_s {
  uint8_t used;
  uint8_t remote_dev_addr_type;
  uint8_t remote_dev_addr[18];
  uint8_t peer_irk[16];
  uint8_t local_irk[16];
  uint8_t Identity_addr_type;
  uint8_t Identity_addr[18];
} rsi_ble_resolvlist_group_t;

//LE resolvlist.
typedef struct rsi_ble_resolve_key_s {
  uint8_t remote_dev_addr_type;
  uint8_t remote_dev_addr[18];
  uint8_t peer_irk[16];
  uint8_t local_irk[16];
  uint16_t remote_ediv;
  uint8_t remote_rand[16];
  uint8_t remote_ltk[16];
  uint8_t Identity_addr_type;
  uint8_t Identity_addr[18];
} rsi_ble_resolve_key_t;

//LE resolvlist.
typedef struct rsi_ble_dev_ltk_list_s {
  uint8_t enc_enable;
  uint8_t sc_enable;
  uint8_t remote_dev_addr_type;
  uint8_t remote_dev_addr[6];
  uint8_t peer_irk[16];
  uint8_t local_irk[16];
  uint16_t remote_ediv;
  uint16_t local_ediv;
  uint8_t remote_rand[8];
  uint8_t localrand[8];
  uint8_t remote_ltk[16];
  uint8_t localltk[16];
  uint8_t Identity_addr_type;
  uint8_t Identity_addr[6];
  uint8_t used;
} rsi_ble_dev_ltk_list_t;
#endif

#if (CONNECT_OPTION == CONN_BY_NAME)
uint8_t rsi_get_ble_conn_id(uint8_t remote_dev_addr[18], uint8_t *remote_name, uint8_t size);
uint8_t rsi_add_ble_conn_id(uint8_t remote_dev_addr[18], uint8_t *remote_name, uint8_t size);
#else
uint8_t rsi_get_ble_conn_id(uint8_t remote_dev_addr[18]);
uint8_t rsi_add_ble_conn_id(uint8_t remote_dev_addr[18]);
#endif
uint8_t rsi_get_remote_device_role(uint8_t remote_dev_addr[18]);
uint8_t rsi_remove_ble_conn_id(uint8_t remote_dev_addr[18]);
uint8_t rsi_check_ble_conn_status(uint8_t connection_id);

#endif
#endif /* APPS_DEMO_BLE_MULTI_SLAVE_MASTER_DEMO_47_RSI_BLE_DEVICE_INFO_H_ */
