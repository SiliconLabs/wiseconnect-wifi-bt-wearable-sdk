/*******************************************************************************
* @file  rsi_bt_apis.h
* @brief 
*******************************************************************************
* # License
* <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/

/*************************************************************************
 *
 */

#ifndef RSI_BT_APIS_H
#define RSI_BT_APIS_H

#include <rsi_data_types.h>
#include <rsi_utils.h>
#include <rsi_bt_common.h>
#include <rsi_bt_config.h>
#include <rsi_bt_sdp.h>

/******************************************************
 * *                      Macros
 * ******************************************************/
#define RSI_LINK_KEY_LEN 16

#define RSI_BUTTON_PUSHED   0x0
#define RSI_BUTTON_RELEASED 0x1
/******************************************************
 * *                    Constants
 * ******************************************************/

/******************************************************
 * *                   Enumerations
 * ******************************************************/

/******************************************************
 * *                 Type Definitions
 * ******************************************************/

/******************************************************
 * *                    Structures
 * ******************************************************/

//BT Classic GAP Command structures

typedef struct rsi_bt_req_query_services_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_query_services_t;

//BT Classic GAP Response structures

typedef struct rsi_bt_resp_query_services_s {
  uint8_t num_of_services;
  uint8_t reserved[3];
  uint32_t uuid[32];
} rsi_bt_resp_query_services_t;

typedef struct rsi_bt_resp_search_service_s {
  uint8_t service_status;
} rsi_bt_resp_search_service_t;

//BT Classic GAP Event structures

typedef struct rsi_bt_event_role_change_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t role;
} rsi_bt_event_role_change_t;

typedef struct rsi_bt_event_unbond_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_unbond_t;

typedef struct rsi_bt_event_bond_response_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_bond_t;

typedef struct rsi_bt_event_inquiry_response_s {
  uint8_t inquiry_type;
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t name_length;
  uint8_t remote_device_name[RSI_DEV_NAME_LEN];
  uint8_t cod[3];
  int8_t rssi;
} rsi_bt_event_inquiry_response_t;

typedef struct rsi_bt_event_remote_device_name_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t name_length;
  uint8_t remote_device_name[RSI_DEV_NAME_LEN];
} rsi_bt_event_remote_device_name_t;

typedef struct rsi_bt_event_remote_name_request_cancel_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_remote_name_request_cancel_t;

typedef struct rsi_bt_event_connection_initiated_s {
  uint8_t conn_initiated;
} rsi_bt_event_connection_initiated_t;

typedef struct rsi_bt_event_disconnect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_disconnect_t;

typedef struct rsi_bt_event_user_confirmation_request_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t reserved[2];
  uint8_t confirmation_value[4];
} rsi_bt_event_user_confirmation_request_t;

typedef struct rsi_bt_event_user_passkey_display_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t reserved[2];
  uint8_t passkey[4];
} rsi_bt_event_user_passkey_display_t;

typedef struct rsi_bt_event_user_passkey_notify_s {
  uint8_t dev_addr[6];
  uint8_t reserved[2];
  uint8_t passkey[4];
} rsi_bt_event_user_passkey_notify_t;

typedef struct rsi_bt_event_user_pincode_request_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_user_pincode_request_t;

typedef struct rsi_bt_event_user_passkey_request_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_user_passkey_request_t;

typedef struct rsi_bt_event_auth_complete_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_auth_complete_t;

typedef struct rsi_bt_event_user_linkkey_request_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_user_linkkey_request_t;

typedef struct rsi_bt_event_user_linkkey_save_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t linkKey[RSI_LINK_KEY_LEN];
  uint8_t local_dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_user_linkkey_save_t;

typedef struct rsi_bt_event_spp_receive_s {
  uint16_t data_len;
  uint8_t data[RSI_BT_MAX_PAYLOAD_SIZE];
} rsi_bt_event_spp_receive_t;

typedef struct rsi_bt_event_spp_connect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint16_t rem_mtu_size;
} rsi_bt_event_spp_connect_t;

typedef struct rsi_bt_event_spp_disconnect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_spp_disconnect_t;

typedef struct rsi_bt_event_l2cap_data_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t data_len[2];
  uint8_t data[RSI_BT_MAX_PAYLOAD_SIZE];
} rsi_bt_event_l2cap_data_t;
typedef struct rsi_bt_event_l2cap_connect_s {
#define BT_L2CAP_CONNECTED    1
#define BT_L2CAP_DISCONNECTED 0
  uint8_t conn_status;
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t reason_code[2];
  uint8_t rem_mtu_size[2];
} rsi_bt_event_l2cap_connect_t;

typedef struct rsi_bt_event_l2cap_disconnect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_l2cap_disconnect_t;

typedef struct rsi_bt_event_hid_receive_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t hid_channel;
  uint8_t data_len[2];
  uint8_t data[RSI_BT_MAX_PAYLOAD_SIZE];
} rsi_bt_event_hid_receive_t;

typedef struct rsi_bt_event_hid_connect_s {
#define BT_HID_CONNECTED    1
#define BT_HID_DISCONNECTED 0
  uint8_t conn_status;
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t reason_code[2];
  uint8_t rem_mtu_size[2];
} rsi_bt_event_hid_connect_t;

typedef struct rsi_bt_event_hid_disconnect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hid_disconnect_t;

typedef struct rsi_bt_event_mode_change_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t current_mode;
  uint8_t reserved;
  uint16_t mode_interval;
} rsi_bt_event_mode_change_t;

typedef struct rsi_bt_event_sniff_subrating_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint16_t max_tx_latency;
  uint16_t min_remote_timeout;
  uint16_t min_local_timeout;
} rsi_bt_event_sniff_subrating_t;

typedef struct rsi_bt_event_gatt_connect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_gatt_connect_t;

typedef struct rsi_bt_event_gatt_disconnect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_gatt_disconnect_t;

typedef struct rsi_bt_event_ssp_complete_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t status;
} rsi_bt_event_ssp_complete_t;

//PER structure paramters
typedef struct rsi_bt_tx_per_params_s {
  uint8_t cmd_id;
  uint8_t transmit_enable;
  uint8_t device_Addr[6]; //BT_BD_ADDR  peer_addr;
  uint8_t pkt_len[2];
  uint8_t pkt_type;
  uint8_t br_edr_mode;
  uint8_t rx_chnl_in;
  uint8_t tx_chnl_in;
  uint8_t link_type;
  uint8_t scrambler_seed;
  uint8_t hopping_type;
  uint8_t ant_sel;
  uint8_t pll_mode;
  uint8_t rf_type;
  uint8_t rf_chain;
  uint8_t payload_type;
  uint8_t tx_power;
  uint8_t transmit_mode;
  uint8_t inter_pkt_gap;
  uint8_t no_of_packets[4];
} rsi_bt_tx_per_params_t;

//PER structure paramters
typedef struct rsi_bt_rx_per_params_s {
  uint8_t cmd_id;
  uint8_t receive_enable;
  uint8_t device_Addr[6]; //BT_BD_ADDR  peer_addr;
  uint8_t pkt_len[2];
  uint8_t pkt_type;
  uint8_t br_edr_mode;
  uint8_t rx_chnl_in;
  uint8_t tx_chnl_in;
  uint8_t link_type;
  uint8_t scrambler_seed;
  uint8_t hopping_type;
  uint8_t ant_sel;
  uint8_t pll_mode;
  uint8_t rf_type;
  uint8_t rf_chain;
  uint8_t loop_back_mode;
} rsi_bt_rx_per_params_t;

typedef struct rsi_bt_event_avdtp_stats_s {
  uint32_t clock; //LMAC timer BT Clock
  /* BDR stats */
  uint16_t tx_poll;
  uint16_t tx_null;
  uint16_t rx_poll;
  uint16_t rx_null;
  uint16_t dm1_pkt[2];
  uint16_t dm3_pkt[2];
  uint16_t dm5_pkt[2];
  uint16_t dh1_pkt[2];
  uint16_t dh3_pkt[2];
  uint16_t dh5_pkt[2];

  /* EDR stats */
  uint16_t edr_2dh1_pkt[2];
  uint16_t edr_2dh3_pkt[2];
  uint16_t edr_2dh5_pkt[2];
  uint16_t edr_3dh1_pkt[2];
  uint16_t edr_3dh3_pkt[2];
  uint16_t edr_3dh5_pkt[2];
  /* Packet stats overall */
  uint16_t no_of_crc_fail;
  uint16_t no_of_retries;
  uint16_t no_of_hdr_err;
  uint16_t no_of_seq_repeat;
  uint16_t ack_delay[11];
  uint16_t ack_received;
  uint16_t nak_received;
  uint16_t no_of_rx_missed;
  uint16_t no_of_rx_error;
  uint16_t tx_attempts;
  uint16_t tx_done;
} rsi_bt_event_avdtp_stats_t;

#define _3M_STATE_CONTINUOUS_FAIL_THRESHOLD_REACHED 1
#define _3M_STATE_PER_THRESHOLD_REACHED             2
#define _2M_STATE_PASS_THRESHOLD_REACHED            3
#define _2M_STATE_CONTINUOUS_PASS_THRESHOLD_REACHED 4
#define _BT_AR_STATS_PERIODIC                       5

typedef struct rsi_bt_event_ar_stats_s {
  uint32_t fail_count_3m;
  uint32_t fail_count_2m;
  uint32_t pass_count_3m;
  uint32_t pass_count_2m;
  uint32_t reason_code;
  uint32_t current_state;
} rsi_bt_event_ar_stats_t;

typedef struct rsi_bt_event_pkt_change_s {
  uint8_t bd_addr[6];
  uint16_t pkt_type;

} rsi_bt_event_pkt_change_t;

typedef struct chip_bt_buffers_stats_s {
  uint8_t acl_buffer_utilization;
  uint8_t cp_buffer_utilization;
} chip_bt_buffers_stats_t;

typedef struct rsi_bt_event_iap_s {
  uint8_t status;
} rsi_bt_event_iap_t;

typedef struct rsi_bt_event_iap2_receive_s {
  uint16_t data_len;
  uint8_t data[1000];
} rsi_bt_event_iap2_receive_t;

/******************************************************
 * *       BT GAP Response Callbacks Declarations
 * ******************************************************/

/**
 * @callback   rsi_bt_on_role_change_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  role_change_status, contains the role_change_status information
 * @return      void
 * @section description
 * This callback function will be called if the role change status cmd response is received from the module.
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_role_change_t)(uint16_t resp_status, rsi_bt_event_role_change_t *role_change_status);

/**
 * @callback   rsi_bt_on_connect_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  bond_response, contains the connect/bond response information
 * @return      void
 * @section description
 * This callback function will be called if new connection indication is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_connect_t)(uint16_t resp_status, rsi_bt_event_bond_t *bond_response);

/**
 * @callback   rsi_bt_on_unbond_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  unbond_status, contains the disconnection information
 * @return      void
 * @section description
 * This callback function will be called if disconnection indication is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_unbond_t)(uint16_t resp_status, rsi_bt_event_unbond_t *unbond_status);

/**
 * @callback   rsi_bt_on_disconnect_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  bt_disconnect, contains the disconnection information
 * @return      void
 * @section description
 * This callback function will be called if disconnection indication is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_disconnect_t)(uint16_t resp_status, rsi_bt_event_disconnect_t *bt_disconnect);

/**
 * @callback   rsi_bt_on_scan_resp_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  rsi_ble_resp_profiles, contains the inquiry response information
 * @return      void
 * @section description
 * This callback function will be called if the single scan response is received from the module in response to inuiry command. 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_scan_resp_t)(uint16_t resp_status, rsi_bt_event_inquiry_response_t *single_scan_resp);

/**
 * @callback   rsi_bt_on_remote_name_resp_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  name_resp, contains the remote device name information
 * @return      void
 * @section description
 * This callback function will be called if the remote name request cmd response is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_remote_name_resp_t)(uint16_t resp_status, rsi_bt_event_remote_device_name_t *name_resp);

/**
 * @callback   rsi_bt_on_passkey_display_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  bt_event_user_passkey_display, contains the passkey information
 * @return      void
 * @section description
 * This callback function will be called if passkey display request is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_passkey_display_t)(uint16_t resp_status,
                                            rsi_bt_event_user_passkey_display_t *bt_event_user_passkey_display);

/**
 * @callback   rsi_bt_on_remote_name_request_cancel_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  remote_name_request_cancel, contains the response to remote name request cancel cmd.
 * @return      void
 * @section description
 * This callback function will be called if the remote name request canacel cmd response is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_remote_name_request_cancel_t)(
  uint16_t resp_status,
  rsi_bt_event_remote_name_request_cancel_t *remote_name_request_cancel);

/**
 * @callback   rsi_bt_on_confirm_request_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  user_confirmation_request, contains the confirmation request information
 * @return      void
 * @section description
 * This callback function will be called if user confirmation request is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_confirm_request_t)(uint16_t resp_status,
                                            rsi_bt_event_user_confirmation_request_t *user_confirmation_request);

/**
 * @callback   rsi_bt_on_pincode_request_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  user_pincode_request, contains the pincode request information
 * @return      void
 * @section description
 * This callback function will be called if pincode request is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_pincode_request_t)(uint16_t resp_status,
                                            rsi_bt_event_user_pincode_request_t *user_pincode_request);

/**
 * @callback   rsi_bt_on_passkey_request_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  user_passkey_request, contains the passkey request information
 * @return      void
 * @section description
 * This callback function will be called if passkey request is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_passkey_request_t)(uint16_t resp_status,
                                            rsi_bt_event_user_passkey_request_t *user_passkey_request);

/**
 * @callback   rsi_bt_on_inquiry_complete_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @return      void
 * @section description
 * This callback function will be called if inquiry complete status is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_inquiry_complete_t)(uint16_t resp_status);

/**
 * @callback   rsi_bt_on_auth_complete_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  auth_complete, contains the authentication complete information
 * @return      void
 * @section description
 * This callback function will be called if authentication complete indication is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_auth_complete_t)(uint16_t resp_status, rsi_bt_event_auth_complete_t *auth_complete);

/**
 * @callback   rsi_bt_on_linkkey_request_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  user_linkkey_request, contains the linkkey request information
 * @return      void
 * @section description
 * This callback function will be called if linkkey request is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_linkkey_request_t)(uint16_t resp_status,
                                            rsi_bt_event_user_linkkey_request_t *user_linkkey_request);

/**
 * @callback   rsi_bt_on_ssp_complete_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  status, contains the ssp complete status
 * @return      void
 * @section description
 * This callback function will be called if ssp complete status is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_ssp_complete_t)(uint16_t resp_status, rsi_bt_event_ssp_complete_t *ssp_complete);

/**
 * @callback   rsi_bt_on_linkkey_save_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  user_linkkey_save, contains the linkkey save information
 * @return      void
 * @section description
 * This callback function will be called if linkkey save request is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_linkkey_save_t)(uint16_t resp_status, rsi_bt_event_user_linkkey_save_t *user_linkkey_save);

/**
 * @callback   rsi_bt_on_get_services_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  service_list, contains the response information to get services cmd.
 * @return      void
 * @section description
 * This callback function will be called if the get services command response is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_get_services_t)(uint16_t resp_status, rsi_bt_resp_query_services_t *service_list);

/**
 * @callback   rsi_bt_on_search_service_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  status, contains the status response to search service cmd.
 * @return      void
 * @section description
 * This callback function will be called if the search services command response is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_search_service_t)(uint16_t resp_status, uint8_t *status);

/**
 * @callback   rsi_bt_on_mode_chnage_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  status, contains the status response to search service cmd.
 * @return      void
 * @section description
 * This callback function will be called if the search services command response is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_mode_chnage_t)(uint16_t resp_status, rsi_bt_event_mode_change_t *mode_change);

/**
 * @fn          rsi_bt_on_avdtp_stats_t 
 * @brief       Callback function to be called if AVDTP events are receicved from module
 * @param[in]   resp_status, contains the response status (Success or Error code)
 * @param[in]   avdtp_event, contains the avdtp status  parameters                 
 * @return      void
 * @section description
 * This function registers the avdtp stats callbacks
 * This callback function will be called when AVDTP stats event is received from the module 
 * This callback has to be registered using rsi_bt_avdtp_events_register_callbacks API

 */
typedef void (*rsi_bt_on_avdtp_stats_t)(uint16_t resp_status, rsi_bt_event_avdtp_stats_t *avdtp_event);

/**
 * @fn          rsi_bt_on_ar_stats_t
 * @brief       Callback function to be called if AR events are received from module
 * @param[in]   resp_status, contains the response status (Success or Error code)
 * @param[in]   ar_event, contains the ar status  parameters
 * @return      void
 * @section description
 * This function registers the ar stats callbacks
 * This callback function will be called when AR stats event is received from the module
 * This callback has to be registered using rsi_bt_ar_events_register_callbacks API

 */
typedef void (*rsi_bt_on_ar_stats_t)(uint16_t resp_status, rsi_bt_event_ar_stats_t *ar_event);

/**
 * @fn          rsi_bt_pkt_change_stats_t 
 * @brief       Callback function to be called when packet type change receicved from module
 * @param[in]   resp_status, contains the response status (Success or Error code)
 * @param[in]   avdtp_event, contains the packet type                  
 * @return      void
 * @section description
 * This function registers rsi_bt_pkt_change_events_register_callbacks
 * * This callback function will be called when packet type event is received from the module 
 * This callback has to be registered using rsi_bt_pkt_change_events_register_callbacks API

 */

typedef void (*rsi_bt_pkt_change_stats_t)(uint16_t resp_status, rsi_bt_event_pkt_change_t *bt_pkt_change_stats);

/**
 * @fn          rsi_bt_on_chip_memory_stats_handler_t
 * @brief       Callback function to be called when memory stats received from module
 * @param[in]   resp_status, contains the response status (Success or Error code)
 * @param[in]   avdtp_event, contains the stats info
 * @return      void
 * @section description
 * This function registers rsi_bt_on_chip_memory_status_callbacks_register
 * * This callback function will be called when memory stats event is received from the module
 * This callback has to be registered using rsi_bt_on_chip_memory_status_callbacks_register API

 */
typedef void (*rsi_bt_on_chip_memory_stats_handler_t)(uint16_t resp_status,
                                                      struct chip_bt_buffers_stats_s *chip_bt_buffers_stats);

/**
 * @callback   rsi_bt_on_sniff_subrating_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  status, contains the status response to search service cmd.
 * @return      void
 * @section description
 * This callback function will be called if the search services command response is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_sniff_subrating_t)(uint16_t resp_status, rsi_bt_event_sniff_subrating_t *mode_change);
/**
 * @callback   rsi_bt_on_connection_initiated_t
 * @brief      Callback function to be called when the headset initiated the connection
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  status, contains the status response to search service cmd.
 * @return      void
 * @section description
 * This callback function will be called if the connection initiated response is recieved from the module
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_connection_initiated_t)(uint16_t resp_status,
                                                 rsi_bt_event_connection_initiated_t *mode_change);

/**
 * @callback   rsi_bt_on_gatt_connection_t
 * @brief      Callback function to be called if gatt connection response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  status, contains the status response to search service cmd.
 * @return      void
 * @section description
 * This callback function will be called if the gatt connection command response is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_gatt_connection_t)(uint16_t resp_status, rsi_bt_event_gatt_connect_t *gatt_conn);
/**
 * @callback   rsi_bt_on_gatt_disconnection_t
 * @brief      Callback function to be called if gatt disconnection response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  status, contains the status response to search service cmd.
 * @return      void
 * @section description
 * This callback function will be called if the gatt disconnection command response is received from the module 
 * This callback has to be registered using rsi_bt_register_gap_callbacks API
 */
typedef void (*rsi_bt_on_gatt_disconnection_t)(uint16_t resp_status, rsi_bt_event_gatt_disconnect_t *gatt_disconn);

/******************************************************
 * *       BT SPP Response Callbacks Declarations
 * ******************************************************/

/**
 * @callback   rsi_bt_on_spp_connect_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  spp_connect, contains the spp connect information
 * @return      void
 * @section description
 * This callback function will be called if spp connection happens.
 * This callback has to be registered using rsi_bt_spp_register_callbacks API
 */
typedef void (*rsi_bt_on_spp_connect_t)(uint16_t resp_status, rsi_bt_event_spp_connect_t *spp_connect);

/**
 * @callback   rsi_bt_on_spp_disconnect_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  spp_disconnect, contains the spp disconnect information
 * @return      void
 * @section description
 * This callback function will be called if spp disconnection happens.
 * This callback has to be registered using rsi_bt_spp_register_callbacks API
 */
typedef void (*rsi_bt_on_spp_disconnect_t)(uint16_t resp_status, rsi_bt_event_spp_disconnect_t *spp_disconnect);

/**
 * @callback   rsi_bt_on_spp_rx_data_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  bt_event_spp_receive, contains the spp rcvd data information.
 * @return      void
 * @section description
 * This callback function will be called if spp data is received.
 * This callback has to be registered using rsi_bt_spp_register_callbacks API
 */
typedef void (*rsi_bt_on_spp_rx_data_t)(uint16_t resp_status, rsi_bt_event_spp_receive_t *bt_event_spp_receive);

/******************************************************
 * *       BT L2CAP Response Callbacks Declarations
 * ******************************************************/

/**
 * @callback   rsi_bt_on_l2cap_connect_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  l2cap_connect, contains the l2cap connect information
 * @return      void
 * @section description
 * This callback function will be called if l2cap connection happens.
 * This callback has to be registered using rsi_bt_l2cap_register_callbacks API
 */
typedef void (*rsi_bt_on_l2cap_connect_t)(uint16_t resp_status, rsi_bt_event_l2cap_connect_t *l2cap_connect);

/**
 * @callback   rsi_bt_on_l2cap_disconnect_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  l2cap_disconnect, contains the l2cap disconnect information
 * @return      void
 * @section description
 * This callback function will be called if l2cap disconnection happens.
 * This callback has to be registered using rsi_bt_l2cap_register_callbacks API
 */
typedef void (*rsi_bt_on_l2cap_disconnect_t)(uint16_t resp_status, rsi_bt_event_l2cap_disconnect_t *l2cap_disconnect);

/**
 * @callback   rsi_bt_on_l2cap_rx_data_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  bt_event_hid_receive, contains the hid rcvd data information.
 * @return      void
 * @section description
 * This callback function will be called if hid data is received.
 * This callback has to be registered using rsi_bt_l2cap_register_callbacks API
 */
typedef void (*rsi_bt_on_l2cap_rx_data_t)(uint16_t resp_status, rsi_bt_event_l2cap_data_t *bt_event_l2cap_receive);

/******************************************************
 * *       BT HID Response Callbacks Declarations
 * ******************************************************/

/**
 * @callback   rsi_bt_on_hid_connect_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  hid_connect, contains the hid connect information
 * @return      void
 * @section description
 * This callback function will be called if hid connection happens.
 * This callback has to be registered using rsi_bt_hid_register_callbacks API
 */
typedef void (*rsi_bt_on_hid_connect_t)(uint16_t resp_status, rsi_bt_event_hid_connect_t *hid_connect);

/**
 * @callback   rsi_bt_on_hid_disconnect_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  hid_disconnect, contains the hid disconnect information
 * @return      void
 * @section description
 * This callback function will be called if hid disconnection happens.
 * This callback has to be registered using rsi_bt_hid_register_callbacks API
 */
typedef void (*rsi_bt_on_hid_disconnect_t)(uint16_t resp_status, rsi_bt_event_hid_disconnect_t *hid_disconnect);

/**
 * @callback   rsi_bt_on_hid_rx_data_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  bt_event_hid_receive, contains the hid rcvd data information.
 * @return      void
 * @section description
 * This callback function will be called if hid data is received.
 * This callback has to be registered using rsi_bt_hid_register_callbacks API
 */
typedef void (*rsi_bt_on_hid_rx_data_t)(uint16_t resp_status, rsi_bt_event_hid_receive_t *bt_event_hid_receive);
typedef void (*rsi_bt_on_hid_handshake_t)(uint16_t resp_status, rsi_bt_event_hid_receive_t *bt_event_hid_receive);
typedef void (*rsi_bt_on_hid_control_t)(uint16_t resp_status, rsi_bt_event_hid_receive_t *bt_event_hid_receive);
typedef void (*rsi_bt_on_hid_get_report_t)(uint16_t resp_status, rsi_bt_event_hid_receive_t *bt_event_hid_receive);
typedef void (*rsi_bt_on_hid_set_report_t)(uint16_t resp_status, rsi_bt_event_hid_receive_t *bt_event_hid_receive);
typedef void (*rsi_bt_on_hid_get_protocol_t)(uint16_t resp_status, rsi_bt_event_hid_receive_t *bt_event_hid_receive);
typedef void (*rsi_bt_on_hid_set_protocol_t)(uint16_t resp_status, rsi_bt_event_hid_receive_t *bt_event_hid_receive);
typedef void (*rsi_bt_app_iap_conn_t)(uint16_t resp_status, rsi_bt_event_iap_t *iap_disconn);
typedef void (*rsi_bt_app_iap_disconn_t)(uint16_t resp_status, rsi_bt_event_iap_t *iap_disconn);
typedef void (*rsi_bt_app_iap_acc_auth_strt_t)(uint16_t resp_status, rsi_bt_event_iap_t *iap_disconn);
typedef void (*rsi_bt_app_iap_acc_auth_fail_t)(uint16_t resp_status, rsi_bt_event_iap_t *iap_disconn);
typedef void (*rsi_bt_app_iap_acc_auth_complete_t)(uint16_t resp_status, rsi_bt_event_iap_t *iap_disconn);
typedef void (*rsi_bt_app_iap2_auth_start_t)(uint16_t resp_status, rsi_bt_event_iap_t *iap_disconn);
typedef void (*rsi_bt_app_iap2_auth_complete_t)(uint16_t resp_status, rsi_bt_event_iap_t *iap_disconn);
typedef void (*rsi_bt_app_iap2_auth_fail_t)(uint16_t resp_status, rsi_bt_event_iap_t *iap_disconn);
typedef void (*rsi_bt_app_iap2_identification_start_t)(uint16_t resp_status, rsi_bt_event_iap_t *iap_disconn);
typedef void (*rsi_bt_app_iap2_identification_complete_t)(uint16_t resp_status,
                                                          rsi_bt_event_iap2_receive_t *iap_disconn);
typedef void (*rsi_bt_app_iap2_identification_reject_t)(uint16_t resp_status, rsi_bt_event_iap2_receive_t *iap_disconn);
typedef void (*rsi_bt_app_iap2_File_Tx_state_t)(uint16_t resp_status, rsi_bt_event_iap2_receive_t *iap2_receive);
typedef void (*rsi_bt_app_on_iap2_data_rx_t)(uint16_t resp_status, rsi_bt_event_iap2_receive_t *iap2_receive);
typedef void (*rsi_bt_app_iap2_File_Tx_state_t)(uint16_t resp_status, rsi_bt_event_iap2_receive_t *iap2_receive);
typedef void (*rsi_bt_app_iap2_File_Transfer_rx_data_t)(uint16_t resp_status,
                                                        rsi_bt_event_iap2_receive_t *iap2_receive);

/******************************************************
 * * BT GAP Callbacks register function Declarations
 * ******************************************************/

/*==============================================*/
/**
 * @fn          rsi_bt_register_gap_callbacks 
 * @brief       This function registers the GAP callbacks
 * @param[in]   rsi_bt_on_connect_t                   bt_on_conn_status_event                         : Connection status callback
 * @param[in]   rsi_bt_on_unbond_t                    bt_on_unbond_status                         : Connection status callback
 * @param[in]   rsi_bt_on_disconnect_t                bt_on_disconnect_event                          : Disconnection status callback
 * @param[in]   rsi_bt_on_scan_resp_t                 bt_on_scan_resp_event                           : Scan report callback
 * @param[in]   rsi_bt_on_remote_name_resp_t          bt_on_remote_name_resp_event                    : Remote name report callback
 * @param[in]   rsi_bt_on_passkey_display_t           bt_on_passkey_display_event                     : Passkey display report callback
 * @param[in]   rsi_bt_on_remote_name_request_cancel_t          bt_on_remote_name_request_cancel_event: Remote name request cancel status callback
 * @param[in]   rsi_bt_on_confirm_request_t           bt_on_confirm_request_event                     : Authentication status callback
 * @param[in]   rsi_bt_on_pincode_request_t           bt_on_pincode_request_event                     : Pincode request status callback
 * @param[in]   rsi_bt_on_passkey_request_t           bt_on_passkey_request_event                     : Passkey request status callback
 * @param[in]   rsi_bt_on_inquiry_complete_t          bt_on_inquiry_complete_event                    : Inquiry report callback
 * @param[in]   rsi_bt_on_auth_complete_t             bt_on_auth_complete_event                       : Authentication status callback
 * @param[in]   rsi_bt_on_linkkey_request_t           bt_on_linkkey_request_event                     : Linkkey request report callback
 * @param[in]   rsi_bt_on_ssp_complete_t              bt_on_ssp_complete_event                        : SSP status callback
 * @param[in]   rsi_bt_on_linkkey_save_t              bt_on_linkkey_save_event                        : Linkkey save status callback
 * @param[in]   rsi_bt_on_get_services_t              bt_on_get_services_event                        : Get services report callback
 * @param[in]   rsi_bt_on_search_service_t            bt_on_search_service_event                      : Search service status callback
 * @return     
 *              Non zero - If fails
 *              0 - If success
 *
 *
 */
void rsi_bt_gap_register_callbacks(rsi_bt_on_role_change_t bt_on_role_change_status_event,
                                   rsi_bt_on_connect_t bt_on_conn_status_event,
                                   rsi_bt_on_unbond_t bt_on_unbond_status,
                                   rsi_bt_on_disconnect_t bt_on_disconnect_event,
                                   rsi_bt_on_scan_resp_t bt_on_scan_resp_event,
                                   rsi_bt_on_remote_name_resp_t bt_on_remote_name_resp_event,
                                   rsi_bt_on_passkey_display_t bt_on_passkey_display_event,
                                   rsi_bt_on_remote_name_request_cancel_t bt_on_remote_name_request_cancel_event,
                                   rsi_bt_on_confirm_request_t bt_on_confirm_request_event,
                                   rsi_bt_on_pincode_request_t bt_on_pincode_request_event,
                                   rsi_bt_on_passkey_request_t bt_on_passkey_request_event,
                                   rsi_bt_on_inquiry_complete_t bt_on_inquiry_complete_event,
                                   rsi_bt_on_auth_complete_t bt_on_auth_complete_event,
                                   rsi_bt_on_linkkey_request_t bt_on_linkkey_request_event,
                                   rsi_bt_on_ssp_complete_t bt_on_ssp_complete_event,
                                   rsi_bt_on_linkkey_save_t bt_on_linkkey_save_event,
                                   rsi_bt_on_get_services_t bt_on_get_services_event,
                                   rsi_bt_on_search_service_t bt_on_search_service_event,
                                   rsi_bt_on_mode_chnage_t bt_on_mode_change_event,
                                   rsi_bt_on_sniff_subrating_t bt_on_sniff_subrating_event,
                                   rsi_bt_on_connection_initiated_t bt_on_connection_initiated);

void rsi_bt_iap_register_callbacks(rsi_bt_app_iap_conn_t bt_app_iap_conn,
                                   rsi_bt_app_iap_disconn_t bt_app_iap_disconn,
                                   rsi_bt_app_iap_acc_auth_strt_t bt_app_iap_acc_auth_strt,
                                   rsi_bt_app_iap_acc_auth_fail_t bt_app_iap_acc_auth_fail,
                                   rsi_bt_app_iap_acc_auth_complete_t bt_app_iap_acc_auth_complete,
                                   rsi_bt_app_iap2_auth_start_t bt_app_iap2_auth_start,
                                   rsi_bt_app_iap2_auth_complete_t bt_app_iap2_auth_complete,
                                   rsi_bt_app_iap2_auth_fail_t bt_app_iap2_auth_fail,
                                   rsi_bt_app_iap2_identification_start_t bt_app_iap2_identification_start,
                                   rsi_bt_app_iap2_identification_complete_t bt_app_iap2_identification_complete,
                                   rsi_bt_app_iap2_identification_reject_t bt_app_iap2_identification_reject,
                                   rsi_bt_app_on_iap2_data_rx_t bt_app_on_iap2_data_rx,
                                   rsi_bt_app_iap2_File_Tx_state_t bt_app_iap2_File_Transfer_state,
                                   rsi_bt_app_iap2_File_Transfer_rx_data_t bt_app_iap2_File_Transfer_data);

/**
 * @fn          rsi_bt_gatt_extended_register_callbacks 
 *
 *
 */

void rsi_bt_gatt_extended_register_callbacks(rsi_bt_on_gatt_connection_t bt_on_gatt_connection_event,
                                             rsi_bt_on_gatt_disconnection_t bt_on_gatt_disconnection_event);

/**
 * @fn          rsi_bt_avdtp_events_register_callbacks 
 *
 */

void rsi_bt_avdtp_events_register_callbacks(rsi_bt_on_avdtp_stats_t bt_on_avdtp_stats_event);

/**
 * @fn          rsi_bt_ar_events_register_callbacks
 * @brief       This function registers the AR Event callbacks
 * @param[in]   rsi_bt_on_avdtp_stats_t               bt_on_ar_stats_event
 * @return
 *              Non zero - If fails
 *              0 - If success
 *
 * @section description
 * This function registers the ar stats callbacks
 *
 */

void rsi_bt_ar_events_register_callbacks(rsi_bt_on_ar_stats_t bt_on_ar_stats_event);

/**
 * @fn          rsi_bt_pkt_change_events_register_callbacks
 *
 */
void rsi_bt_pkt_change_events_register_callbacks(rsi_bt_pkt_change_stats_t bt_pkt_change_stats_event);
/**
 * @fn          rsi_bt_on_chip_memory_status_callbacks_register
 *
 */
void rsi_bt_on_chip_memory_status_callbacks_register(
  rsi_bt_on_chip_memory_stats_handler_t bt_on_chip_memory_stats_event);
/******************************************************
 * * BT SPP Callbacks register function Declarations
 * ******************************************************/

/*==============================================*/

/**
 * @fn          rsi_bt_spp_register_callbacks 
 *
 */
void rsi_bt_spp_register_callbacks(rsi_bt_on_spp_connect_t bt_on_spp_connect_event,
                                   rsi_bt_on_spp_disconnect_t bt_on_spp_disconnect_event,
                                   rsi_bt_on_spp_rx_data_t bt_on_spp_rx_data_event);

void rsi_bt_l2cap_register_callbacks(rsi_bt_on_l2cap_connect_t bt_on_l2cap_connect_event,
                                     rsi_bt_on_l2cap_rx_data_t bt_on_l2cap_rx_data_event);
/*==============================================*/
/**
 * @fn         rsi_bt_set_local_class_of_device
 */
int32_t rsi_bt_set_local_class_of_device(uint32_t class_of_device);

/*==============================================*/
/**
 * @fn         rsi_bt_get_local_class_of_device
 */
int32_t rsi_bt_get_local_class_of_device(uint8_t *resp);

/*==============================================*/
/**
 * @fn         rsi_bt_start_discoverable
 */
int32_t rsi_bt_start_discoverable(void);

/*==============================================*/
/**
 * @fn         rsi_bt_start_limited_discoverable
 */
int32_t rsi_bt_start_limited_discoverable(int32_t time_out_ms);

/*==============================================*/
/**
 * @fn         rsi_bt_stop_discoverable
 */
int32_t rsi_bt_stop_discoverable(void);

/*==============================================*/
/**
 * @fn         rsi_bt_get_discoverable_status
 */
int32_t rsi_bt_get_discoverable_status(uint8_t *resp);

/*==============================================*/
/**
 * @fn         rsi_bt_set_connectable
 */
int32_t rsi_bt_set_connectable(void);

/*==============================================*/
/**
 * @fn         rsi_bt_set_non_connectable
 */
int32_t rsi_bt_set_non_connectable(void);

/*==============================================*/
/**
 * @fn         rsi_bt_get_connectable_status
 */
int32_t rsi_bt_get_connectable_status(uint8_t *resp);

/*==============================================*/
/**
 * @fn         rsi_bt_enable_authentication
 */
int32_t rsi_bt_enable_authentication(void);

/*==============================================*/
/**
 * @fn         rsi_bt_disable_authentication
 */
int32_t rsi_bt_disable_authentication(void);

/*==============================================*/
/**
 * @fn         rsi_bt_get_authentication
 */
int32_t rsi_bt_get_authentication(void);

/*==============================================*/
/**
 * @fn         rsi_bt_remote_name_request_async
 */
int32_t rsi_bt_remote_name_request_async(uint8_t *remote_dev_addr,
                                         rsi_bt_event_remote_device_name_t *bt_event_remote_device_name);

/*==============================================*/
/**
 * @fn         rsi_bt_remote_name_request_cancel
 */
int32_t rsi_bt_remote_name_request_cancel(uint8_t *remote_dev_addr);

/*==============================================*/
/**
 * @fn         rsi_bt_spp_connect
 * */
int32_t rsi_bt_spp_connect(uint8_t *remote_dev_addr);

/*==============================================*/
/**

 * @fn         rsi_ble_set_eir_data
 *
 */
int32_t rsi_bt_set_eir_data(uint8_t *data, uint16_t data_len);

/*==============================================*/
/**
 * @fn         rsi_bt_spp_init
 */
int32_t rsi_bt_spp_init(void);

/*==============================================*/
/**
 * @fn         rsi_bt_spp_transfer
 *
 */
int32_t rsi_bt_spp_transfer(uint8_t *remote_dev_addr, uint8_t *data, uint16_t length);

/*==============================================*/
/**
 * @fn         rsi_bt_inquiry
 */
int32_t rsi_bt_inquiry(uint8_t inquiry_type, uint32_t inquiry_duration, uint8_t max_devices);

/*==============================================*/
/**
 * @fn         rsi_bt_cancel_inquiry
 */
int32_t rsi_bt_cancel_inquiry(void);

/*==============================================*/
/**
 * @fn         rsi_bt_connect
 */
int32_t rsi_bt_connect(uint8_t *remote_dev_addr);

/*==============================================*/
/**
 * @fn         rsi_bt_cancel_connect
 *
 */
int32_t rsi_bt_cancel_connect(uint8_t *remote_dev_address);

/*==============================================*/
/**
 * @fn         rsi_bt_disconnect
 *
 */
int32_t rsi_bt_disconnect(uint8_t *remote_dev_address);

/*==============================================*/
/**
 * @fn         rsi_bt_accept_ssp_confirm
 *
 */
int32_t rsi_bt_accept_ssp_confirm(uint8_t *remote_dev_address);

/*==============================================*/
/**
 * @fn         rsi_bt_reject_ssp_confirm
 *
 */
int32_t rsi_bt_reject_ssp_confirm(uint8_t *remote_dev_address);

/*==============================================*/
/**
 * @fn         rsi_bt_passkey
 *
 */
int32_t rsi_bt_passkey(uint8_t *remote_dev_addr, uint32_t passkey, uint8_t reply_type);

/*==============================================*/
/**
 * @fn         rsi_bt_pincode_request_reply
 *
 */
int32_t rsi_bt_pincode_request_reply(uint8_t *remote_dev_addr, uint8_t *pin_code, uint8_t reply_type);

/*==============================================*/
/**
 * @fn         rsi_bt_linkkey_request_reply
 *
 */
int32_t rsi_bt_linkkey_request_reply(uint8_t *remote_dev_addr, uint8_t *linkkey, uint8_t reply_type);

/*==============================================*/
/**
 * @fn         rsi_bt_get_local_device_role
 *
 */
int32_t rsi_bt_get_local_device_role(uint8_t *remote_dev_addr, uint8_t *resp);

/*==============================================*/
/**
 * @fn         rsi_bt_get_services_async
 *
 */
int32_t rsi_bt_get_services_async(uint8_t *remote_dev_addr, rsi_bt_resp_query_services_t *bt_resp_query_services);

/*==============================================*/
/**
 * @fn         rsi_bt_search_service_async
 *
 */
int32_t rsi_bt_search_service_async(uint8_t *remote_dev_addr, uint32_t service_uuid);

/*==============================================*/
/**
 * @fn         rsi_bt_sniff_mode
 */
int32_t rsi_bt_sniff_mode(uint8_t *remote_dev_addr,
                          uint16_t sniff_max_intv,
                          uint16_t sniff_min_intv,
                          uint16_t sniff_attempt,
                          uint16_t sniff_tout);

/*==============================================*/
/**
 * @fn         rsi_bt_sniff_exit_mode
 */
int32_t rsi_bt_sniff_exit_mode(uint8_t *remote_dev_addr);

/*==============================================*/
/**
 * @fn         rsi_bt_sniff_subrating_mode
 */
int32_t rsi_bt_sniff_subrating_mode(uint8_t *remote_dev_addr,
                                    uint16_t max_latency,
                                    uint16_t min_remote_tout,
                                    uint16_t min_local_tout);

/*==============================================*/
/**
 * @fn         rsi_bt_set_ssp_mode
 */
int32_t rsi_bt_set_ssp_mode(uint8_t pair_mode, uint8_t IOcapability);
int32_t rsi_bt_iap_conn(uint8_t *remote_dev_addr, uint8_t version);
int32_t rsi_bt_iap_disconn(uint8_t *remote_dev_addr, uint8_t version);
int32_t rsi_bt_iap_set_accessory_info(uint8_t type, uint8_t length, uint8_t *data);
int32_t rsi_bt_iap_set_voice_over(uint8_t mode, uint8_t restore);
int32_t rsi_bt_iap1_identification(void);
int32_t rsi_bt_iap2_identification(void);
int32_t rsi_bt_iap1_device_authentication(void);
int32_t rsi_bt_iap_init(void);
int32_t rsi_bt_iap_set_protocol_type(uint8_t type);
int32_t rsi_bt_iap_find_protocol_type(uint8_t *resp);

/* A2DP Profile */
typedef struct rsi_bt_event_a2dp_connect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_a2dp_connect_t;

typedef struct rsi_bt_event_a2dp_disconnect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_a2dp_disconnect_t;

typedef struct rsi_bt_event_a2dp_configure_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_a2dp_configure_t;

typedef struct rsi_bt_event_a2dp_start_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint16_t sample_freq;
  uint16_t rem_mtu_size;
} rsi_bt_event_a2dp_start_t;

typedef struct rsi_bt_event_a2dp_open_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_a2dp_open_t;

typedef struct rsi_bt_event_a2dp_suspend_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_a2dp_suspend_t;

typedef struct rsi_bt_event_a2dp_abort_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_a2dp_abort_t;

typedef struct rsi_bt_event_a2dp_close_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_a2dp_close_t;

typedef struct rsi_bt_event_a2dp_encode_data_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint16_t encode_data_len;
  uint8_t encode_data[RSI_BT_MAX_PAYLOAD_SIZE];
} rsi_bt_event_a2dp_encode_data_t;

typedef struct rsi_bt_event_a2dp_pcm_data_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint16_t pcm_data_len;
  uint8_t pcm_data[RSI_BT_MAX_PAYLOAD_SIZE];
} rsi_bt_event_a2dp_pcm_data_t;

typedef struct rsi_bt_event_a2dp_more_data_req_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint16_t NbrBytesReqd;
} rsi_bt_event_a2dp_more_data_req_t;

typedef struct rsi_bt_event_a2dp_reconfig_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint16_t resp_status;
} rsi_bt_event_a2dp_reconfig_t;
/******************************************************
 * *       BT A2DP Response Callbacks Declarations
 * ******************************************************/

/**
 * @callback   rsi_bt_on_a2dp_connect_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  a2dp_connect, contains the a2dp connect information
 * @return      void
 * @section description
 * This callback function will be called if a2dp connection happens.
 * This callback has to be registered using rsi_bt_a2dp_register_callbacks API
 */
typedef void (*rsi_bt_on_a2dp_connect_t)(uint16_t resp_status, rsi_bt_event_a2dp_connect_t *a2dp_connect);

/**
 * @callback   rsi_bt_on_a2dp_disconnect_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  a2dp_disconnect, contains the a2dp disconnect information
 * @return      void
 * @section description
 * This callback function will be called if a2dp disconnection happens.
 * This callback has to be registered using rsi_bt_a2dp_register_callbacks API
 */
typedef void (*rsi_bt_on_a2dp_disconnect_t)(uint16_t resp_status, rsi_bt_event_a2dp_disconnect_t *a2dp_disconnect);

/**
 * @callback   rsi_bt_on_a2dp_configure_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  a2dp_configure, contains the a2dp configure information
 * @return      void
 * @section description
 * This callback function will be called if a2dp connection happens.
 * This callback has to be registered using rsi_bt_a2dp_register_callbacks API
 */
typedef void (*rsi_bt_on_a2dp_configure_t)(uint16_t resp_status, rsi_bt_event_a2dp_configure_t *a2dp_configure);

/**
 * @callback   rsi_bt_on_a2dp_open_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  a2dp_open, contains the a2dp open information
 * @return      void
 * @section description
 * This callback function will be called if a2dp open happens.
 * This callback has to be registered using rsi_bt_a2dp_register_callbacks API
 */
typedef void (*rsi_bt_on_a2dp_open_t)(uint16_t resp_status, rsi_bt_event_a2dp_open_t *a2dp_open);

/**
 * @callback   rsi_bt_on_a2dp_start_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  a2dp_start, contains the a2dp start information
 * @return      void
 * @section description
 * This callback function will be called if a2dp start happens.
 * This callback has to be registered using rsi_bt_a2dp_register_callbacks API
 */
typedef void (*rsi_bt_on_a2dp_start_t)(uint16_t resp_status, rsi_bt_event_a2dp_start_t *a2dp_start);

/**
 * @callback   rsi_bt_on_a2dp_suspend_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  a2dp suspend, contains the a2dp suspend information
 * @return      void
 * @section description
 * This callback function will be called if a2dp suspend happens.
 * This callback has to be registered using rsi_bt_a2dp_register_callbacks API
 */
typedef void (*rsi_bt_on_a2dp_suspend_t)(uint16_t resp_status, rsi_bt_event_a2dp_suspend_t *a2dp_suspend);

/**
 * @callback   rsi_bt_on_a2dp_abort_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  a2dp_abort, contains the a2dp abort information
 * @return      void
 * @section description
 * This callback function will be called if a2dp abort happens.
 * This callback has to be registered using rsi_bt_a2dp_register_callbacks API
 */
typedef void (*rsi_bt_on_a2dp_abort_t)(uint16_t resp_status, rsi_bt_event_a2dp_abort_t *a2dp_abort);

/**
 * @callback   rsi_bt_on_a2dp_close_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  a2dp_close, contains the a2dp close information
 * @return      void
 * @section description
 * This callback function will be called if a2dp close happens.
 * This callback has to be registered using rsi_bt_a2dp_register_callbacks API
 */
typedef void (*rsi_bt_on_a2dp_close_t)(uint16_t resp_status, rsi_bt_event_a2dp_close_t *a2dp_close);

/**
 * @callback   rsi_bt_on_a2dp_encode_data_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  a2dp_encode_data, contains the SBC encoded packet information
 * @return      void
 * @section description
 * This callback function will be called if SBC data received from TA happens.
 * This callback has to be registered using rsi_bt_a2dp_register_callbacks API
 */
typedef void (*rsi_bt_on_a2dp_encode_data_t)(uint16_t resp_status, rsi_bt_event_a2dp_encode_data_t *a2dp_encode_data);

/**
 * @callback   rsi_bt_on_a2dp_pcm_data_t
 * @brief      Callback function to be called if PCM data is received from TA
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  a2dp_pcm_data, contains the PCM audio data information
 * @return      void
 * @section description
 * This callback function will be called if a2dp PCM data receive happens.
 * This callback has to be registered using rsi_bt_a2dp_register_callbacks API
 */
typedef void (*rsi_bt_on_a2dp_pcm_data_t)(uint16_t resp_status, rsi_bt_event_a2dp_pcm_data_t *a2dp_pcm_data);

/**
 * @callback   rsi_bt_on_a2dp_data_req_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  a2dp_more_data_req, contains the a2dp_more_data_req information
 * @return      void
 * @section description
 * This callback function will be called if a2dp connection happens.
 * This callback has to be registered using rsi_bt_a2dp_register_callbacks API
 */
typedef void (*rsi_bt_on_a2dp_data_req_t)(uint16_t resp_status, rsi_bt_event_a2dp_more_data_req_t *a2dp_more_data_req);

/**
 * @callback   rsi_bt_on_a2dp_reconfig_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  a2dp suspend, contains the a2dp reconfig information
 * @return      void
 * @section description
 * This callback function will be called if a2dp reconfig happens.
 * This callback has to be registered using rsi_bt_a2dp_register_callbacks API
 */
typedef void (*rsi_bt_on_a2dp_reconfig_t)(uint16_t resp_status, rsi_bt_event_a2dp_reconfig_t *a2dp_reconfig);

/******************************************************
 * * BT A2DP Callbacks register function Declarations
 * ******************************************************/

/*==============================================*/

/**
 * @fn          rsi_bt_spp_register_callbacks 
 * @brief       This function registers the SPP callbacks
 * @param[in]   rsi_bt_on_a2dp_connect_t         bt_on_a2dp_connect_event              : a2dp connection status callback
 * @param[in]   rsi_bt_on_a2dp_disconnect_t      bt_on_a2dp_disconnect_event           : a2dp disconnection status callback
 * @param[in]   rsi_bt_on_a2dp_configure_t       bt_on_a2dp_configure_event            : a2dp configure status callback
 * @param[in]   rsi_bt_on_a2dp_open_t            bt_on_a2dp_open_event                 : a2dp open status callback
 * @param[in]   rsi_bt_on_a2dp_start_t           bt_on_a2dp_start_event                : a2dp start status callback
 * @param[in]   rsi_bt_on_a2dp_suspend_t         bt_on_a2dp_suspend_event              : a2dp suspend status callback
 * @param[in]   rsi_bt_on_a2dp_abort_t           bt_on_a2dp_abort_event                : a2dp abort status callback
 * @param[in]   rsi_bt_on_a2dp_close_t           bt_on_a2dp_close_event                : a2dp close status callback
 * @param[in]   rsi_bt_on_a2dp_encode_data_t     bt_on_a2dp_encode_data_event          : a2dp SBC packet received callback
 * @param[in]   rsi_bt_on_a2dp_pcm_data_t        bt_on_a2dp_pcm_data_event             : a2dp PCM packet received callback
 * @param[in]   rsi_bt_on_a2dp_data_req_t        rsi_bt_on_a2dp_data_req_t             : a2dp data request received callback
 * @param[in]   rsi_bt_on_a2dp_reconfig_t        bt_on_a2dp_reconfig_event             : a2dp reconfig status callback
 * @return     
 *              Non zero - If fails
 *              0 - If success
 *
 *
 */
void rsi_bt_a2dp_register_callbacks(rsi_bt_on_a2dp_connect_t bt_on_a2dp_connect_event,
                                    rsi_bt_on_a2dp_disconnect_t bt_on_a2dp_disconnect_event,
                                    rsi_bt_on_a2dp_configure_t bt_on_a2dp_configure_event,
                                    rsi_bt_on_a2dp_open_t bt_on_a2dp_open_event,
                                    rsi_bt_on_a2dp_start_t bt_on_a2dp_start_event,
                                    rsi_bt_on_a2dp_suspend_t bt_on_a2dp_suspend_event,
                                    rsi_bt_on_a2dp_abort_t bt_on_a2dp_abort_event,
                                    rsi_bt_on_a2dp_close_t bt_on_a2dp_close_event,
                                    rsi_bt_on_a2dp_encode_data_t bt_on_a2dp_encode_data_event,
                                    rsi_bt_on_a2dp_pcm_data_t bt_on_a2dp_pcm_data_event,
                                    rsi_bt_on_a2dp_data_req_t bt_on_a2dp_data_req_t,
                                    rsi_bt_on_a2dp_reconfig_t bt_on_a2dp_reconfig_event);

/* AVRCP Profile */
typedef struct rsi_bt_event_avrcp_connect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_avrcp_connect_t;

typedef struct rsi_bt_event_avrcp_disconnect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_avrcp_disconnect_t;

typedef struct rsi_bt_event_avrcp_play_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t status_flag;
} rsi_bt_event_avrcp_play_t;

typedef struct rsi_bt_event_avrcp_pause_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t status_flag;
} rsi_bt_event_avrcp_pause_t;

typedef struct rsi_bt_event_avrcp_stop_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t status_flag;
} rsi_bt_event_avrcp_stop_t;

typedef struct rsi_bt_event_avrcp_next_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t status_flag;
} rsi_bt_event_avrcp_next_t;

typedef struct rsi_bt_event_avrcp_previous_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t status_flag;
} rsi_bt_event_avrcp_previous_t;

typedef struct rsi_bt_event_avrcp_vol_up_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t status_flag;
} rsi_bt_event_avrcp_vol_up_t;

typedef struct rsi_bt_event_avrcp_vol_down_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t status_flag;
} rsi_bt_event_avrcp_vol_down_t;

typedef struct rsi_bt_event_avrcp_get_elem_attr_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t pkt_type;
  uint16_t param_len;
  uint8_t param[512];
} rsi_bt_event_avrcp_get_elem_attr_resp_t;

typedef struct player_att_value_s {
  uint8_t attid;
  uint8_t attvalue;
  uint8_t reserved[2];
} player_att_value_t;

typedef struct player_att_val_list_s {
  uint8_t nbr_att;
  player_att_value_t att_list[5];
} player_att_val_list_t;

typedef struct att_val_s {
  uint8_t att_id;
  uint8_t att_val;
  uint8_t reserved[2];
} att_val_t;
#define MAX_ATT_VALS 5
typedef struct att_val_list_s {
  att_val_t att_vals[MAX_ATT_VALS];
  uint8_t nbr_atts;
} att_val_list_t;

typedef union notify_val_s {
  uint8_t player_status;
  uint32_t track_num;
  uint32_t track_pos;
  uint8_t battery_status;
  uint8_t system_status;
  uint8_t abs_vol;
  uint16_t playerid;
  uint16_t uidcounter;
  att_val_list_t att_val_list;
} notify_val_t;

typedef struct avrcp_notify_s {
  uint8_t eventid;
  uint8_t rtype;
  uint8_t Reserved[2];
  notify_val_t notify_val;
  /*    union notify_val_s {
        uint8_t    player_status;
        uint32_t   track_num;
        uint32_t   track_pos;
        uint8_t    batter_status;
        uint8_t    system_status;
        player_att_val_list_t  att_val_list;
    } notify_val;*/
} avrcp_notify_t;

typedef struct rsi_bt_event_avrcp_notify_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  avrcp_notify_t notify_val;
} rsi_bt_event_avrcp_notify_t;

/**
 * @callback   rsi_bt_on_avrcp_connect_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  avrcp_conn, contains the avrcp connected information.
 * @return      void
 * @section description
 *This callback function will be called if avrcp connection happens.
 * This callback has to be registered using rsi_bt_avrcp_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_connect_t)(uint16_t resp_status, rsi_bt_event_avrcp_connect_t *bt_event_avrcp_connect);

/**
 * @callback   rsi_bt_on_avrcp_disconnect_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  avrcp_disconn, contains the avrcp disconnected information.
 * @return      void
 * @section description
 * This callback function will be called if avrcp disconnection happens.
 * This callback has to be registered using rsi_bt_avrcp_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_disconnect_t)(uint16_t resp_status,
                                             rsi_bt_event_avrcp_disconnect_t *bt_event_avrcp_disconnect);

/**
 * @callback   rsi_bt_on_avrcp_paly_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  avrcp_play, contains the avrcp play information.
 * @return      void
 * @section description
 * This callback function will be called if avrcp play event is received.
 * This callback has to be registered using rsi_bt_avrcp_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_play_t)(uint16_t resp_status, rsi_bt_event_avrcp_play_t *bt_event_avrcp_play);

/**
 * @callback   rsi_bt_on_avrcp_pause_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  avrcp_pause, contains the avrcp pause information.
 * @return      void
 * @section description
 * This callback function will be called if avrcp pause event is received.
 * This callback has to be registered using rsi_bt_avrcp_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_pause_t)(uint16_t resp_status, rsi_bt_event_avrcp_pause_t *bt_event_avrcp_pause);

/**
 * @callback   rsi_bt_on_avrcp_stop_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  avrcp_stop, contains the avrcp pause information.
 * @return      void
 * @section description
 * This callback function will be called if avrcp stop event is received.
 * This callback has to be registered using rsi_bt_avrcp_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_stop_t)(uint16_t resp_status, rsi_bt_event_avrcp_stop_t *bt_event_avrcp_stop);

/**
 * @callback   rsi_bt_on_avrcp_next_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  avrcp_next, contains the spp rcvd data information.
 * @return      void
 * @section description
 * This callback function will be called if avrcp next event is received.
 * This callback has to be registered using rsi_bt_avrcp_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_next_t)(uint16_t resp_status, rsi_bt_event_avrcp_next_t *bt_event_avrcp_next);

/**
 * @callback   rsi_bt_on_avrcp_previous_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  avrcp_previous, contains the spp rcvd data information.
 * @return      void
 * @section description
 * This callback function will be called if avrcp previous event is received.
 * This callback has to be registered using rsi_bt_avrcp_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_previous_t)(uint16_t resp_status,
                                           rsi_bt_event_avrcp_previous_t *bt_event_avrcp_previous);

/**
 * @callback   rsi_bt_on_avrcp_vol_up_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  avrcp_vol_up, contains avrcp vol up info.
 * @return      void
 * @section description
 * This callback function will be called if avrcp vol up event is received.
 * This callback has to be registered using rsi_bt_avrcp_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_vol_up_t)(uint16_t resp_status, rsi_bt_event_avrcp_vol_up_t *bt_event_avrcp_vol_up);

/**
 * @callback   rsi_bt_on_avrcp_vol_down_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  avrcp_vol_down, contains the avrcp vol down info.
 * @return      void
 * @section description
 * This callback function will be called if avrcp vol down event is received.
 * This callback has to be registered using rsi_bt_avrcp_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_vol_down_t)(uint16_t resp_status,
                                           rsi_bt_event_avrcp_vol_down_t *bt_event_avrcp_vol_down);

/**
 * @callback   rsi_bt_on_avrcp_get_elem_attr_resp_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  avrcp_get_elem_attr_resp, contains the avrcp get elem attr resp info.
 * @return      void
 * @section description
 * This callback function will be called if avrcp get elem resp event is received.
 * This callback has to be registered using rsi_bt_avrcp_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_get_elem_attr_resp_t)(uint16_t resp_status,
                                                     rsi_bt_event_avrcp_get_elem_attr_resp_t *avrcp_get_elem_attr_resp);

/**
 * @callback   rsi_bt_on_avrcp_notify_event_t
 * @brief      Callback function is invoke when we receive notification from remote device.
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[out  bt_event_avrcp_notify, contains the spp rcvd data information.
 * @return      void
 * @section description
 * This callback function will be called if avrcp notification event is received.
 * This callback has to be registered using rsi_bt_avrcp_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_notify_event_t)(uint16_t resp_status,
                                               rsi_bt_event_avrcp_notify_t *bt_event_avrcp_notify);

/*==============================================*/

/**
 * @fn          rsi_bt_avrcp_register_callbacks 
 * @brief       This function registers the AVRCP profile callbacks
 * @param[in]   rsi_bt_on_avrcp_connect_t         bt_on_avrcp_connect_event              : avrcp connection status callback
 * @param[in]   rsi_bt_on_avrcp_disconnect_t      bt_on_avrcp_disconnect_event           : avrcp disconnection status callback
 * @param[in]   rsi_bt_on_avrcp_play_t            bt_on_avrcp_play_event                 : avrcp play status callback
 * @param[in]   rsi_bt_on_avrcp_pause_t           bt_on_avrcp_pause_event                : avrcp pause status callback
 * @param[in]   rsi_bt_on_avrcp_stop_t            bt_on_avrcp_stop_event                 : avrcp stop status callback
 * @param[in]   rsi_bt_on_avrcp_next_t            bt_on_avrcp_next_event                 : avrcp next status callback
 * @param[in]   rsi_bt_on_avrcp_previous_t        bt_on_avrcp_previous_event             : avrcp previous status callback
 * @param[in]   rsi_bt_on_avrcp_vol_up_t          bt_on_avrcp_vol_up_event               : avrcp vol up status callback
 * @param[in]   rsi_bt_on_avrcp_vol_down_t        bt_on_avrcp_vol_down_event             : avrcp vol down status callback
 * @return     
 *              Non zero - If fails
 *              0 - If success
 *
 *
 */
void rsi_bt_avrcp_register_callbacks(rsi_bt_on_avrcp_connect_t bt_on_avrcp_connect_event,
                                     rsi_bt_on_avrcp_disconnect_t bt_on_avrcp_disconnect_event,
                                     rsi_bt_on_avrcp_play_t bt_on_avrcp_play_event,
                                     rsi_bt_on_avrcp_pause_t bt_on_avrcp_pause_event,
                                     rsi_bt_on_avrcp_stop_t bt_on_avrcp_stop_event,
                                     rsi_bt_on_avrcp_next_t bt_on_avrcp_next_event,
                                     rsi_bt_on_avrcp_previous_t bt_on_avrcp_previous_event,
                                     rsi_bt_on_avrcp_vol_up_t bt_on_avrcp_vol_up_event,
                                     rsi_bt_on_avrcp_vol_down_t bt_on_avrcp_vol_down_event,
                                     rsi_bt_on_avrcp_get_elem_attr_resp_t bt_on_avrcp_get_elem_attr_resp_event,
                                     rsi_bt_on_avrcp_notify_event_t bt_on_avrcp_notify_event);

#define AVRCP_MAX_APP_SUPP_ATTS      5
#define AVRCP_MAX_APP_SUPP_ATT_VALS  5
#define AVRCP_MAX_APP_SUPP_CHAR_SETS 5
#define AVRCP_MAX_SONG_ATTS          10

typedef struct rsi_bt_event_avrcp_get_cur_att_val_s {
  uint8_t dev_addr[6];
  uint8_t nbr_atts;
  uint8_t atts[AVRCP_MAX_APP_SUPP_ATTS];
} rsi_bt_event_avrcp_get_cur_att_val_t;

typedef struct rsi_bt_event_avrcp_set_att_val_s {
  uint8_t dev_addr[6];
  uint8_t nbr_atts;
  uint8_t reserved;
  player_att_value_t att_list[AVRCP_MAX_APP_SUPP_ATTS];
} rsi_bt_event_avrcp_set_att_val_t;

typedef struct rsi_bt_event_avrcp_vals_text_req_s {
  uint8_t dev_addr[6];
  uint8_t att_id;
  uint8_t nbr_vals;
  uint8_t vals[AVRCP_MAX_APP_SUPP_ATT_VALS];
} rsi_bt_event_avrcp_vals_text_req_t;

typedef struct rsi_bt_event_avrcp_char_sets_s {
  uint8_t dev_addr[6];
  uint8_t nbr_sets;
  uint8_t reserved;
  uint16_t char_sets[AVRCP_MAX_APP_SUPP_CHAR_SETS];
} rsi_bt_event_avrcp_char_sets_t;

typedef struct rsi_bt_event_avrcp_ele_att_s {
  uint8_t dev_addr[6];
  uint8_t nbr_atts;
  uint8_t reserved;
  uint32_t ele_atts[AVRCP_MAX_SONG_ATTS];
} rsi_bt_event_avrcp_ele_att_t;

typedef struct rsi_bt_event_avrcp_set_abs_vol_s {
  uint8_t dev_addr[6];
  uint8_t abs_vol;
  uint8_t remote_req_or_cmd_resp; //0 - remote request event, 1 - host cmd resp
} rsi_bt_event_avrcp_set_abs_vol_t;
typedef struct rsi_bt_event_avrcp_set_addr_player_s {
  uint8_t dev_addr[6];
  uint16_t player_id;
} rsi_bt_event_avrcp_set_addr_player_t;

typedef struct rsi_bt_event_avrcp_get_folder_items_s {
  uint8_t dev_addr[6];
  uint8_t scope;
  uint16_t start_item;
  uint16_t end_item;
  uint8_t attr_cnt;
} rsi_bt_event_avrcp_get_folder_items_t;

typedef struct rsi_bt_event_avrcp_get_tot_num_items_s {
  uint8_t dev_addr[6];
  uint8_t scope;
} rsi_bt_event_avrcp_get_tot_num_items_t;

/**
 * @callback   rsi_bt_on_avrcp_get_cap_event_t
 * @brief      Callback function is invoke when we receive get capabilities request from remote device.
 * @param[in]  bd_addr, Remote bd address.
 * @param[in]  cap_type, avrcp capabilities type.
 * @return      void
 * @section description
 * This callback function will be called if avrcp get capabilities request is received.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_get_cap_event_t)(uint8_t *bd_addr, uint8_t cap_type);

/**
 * @callback   rsi_bt_on_avrcp_get_app_supp_att_event_t
 * @brief      Callback function is invoke when we receive get applicaiton support attribute list request from remote device.
 * @param[in]  bd_addr, Remote bd address.
 * @return     void
 * @section description
 * This callback function will be called if avrcp get the application support attributes request is received.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_get_app_supp_atts_event_t)(uint8_t *bd_addr);

/**
 * @callback   rsi_bt_on_avrcp_get_app_suup_att_vals_event_t
 * @brief      Callback function is invoke when we receive get application support attribute values request from remote device.
 * @param[in]  bd_addr, Remote bd address.
 * @param[in]  att_id, application attribute id.
 * @return      void
 * @section description
 * This callback function will be called if avrcp get application support attribute values request is received.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_get_app_suup_att_vals_event_t)(uint8_t *bd_addr, uint8_t att_id);

/**
 * @callback   rsi_bt_on_avrcp_get_app_cur_att_val_event_t
 * @brief      Callback function is invoke when we receive get application curent attributes value request from remote device.
 * @param[in]  p_att_list, attribute list.
 * @return      void
 * @section description
 * This callback function will be called if avrcp get application curent attribute value request is received.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_get_app_cur_att_val_event_t)(rsi_bt_event_avrcp_get_cur_att_val_t *p_att_list);

/**
 * @callback   rsi_bt_on_avrcp_get_app_cur_att_val_event_t
 * @brief      Callback function is invoke when we receive get application curent attributes value request from remote device.
 * @param[in]  p_att_list, attribute list.
 * @return      void
 * @section description
 * This callback function will be called if avrcp get application curent attribute value request is received.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_set_app_cur_att_val_event_t)(rsi_bt_event_avrcp_set_att_val_t *p_att_list);

/**
 * @callback   rsi_bt_on_avrcp_get_app_att_text_event_t
 * @brief      Callback function is invoke when we receive get application attributes text request from remote device.
 * @param[in]  p_att_list, attribute list.
 * @return      void
 * @section description
 * This callback function will be called if avrcp get application attribute text request is received.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_get_app_att_text_event_t)(rsi_bt_event_avrcp_get_cur_att_val_t *p_att_list);

/**
 * @callback   rsi_bt_on_avrcp_get_app_att_vals_text_event_t
 * @brief      Callback function is invoke when we receive get application attribute values text request from remote device.
 * @param[in]  p_vals_list, attribute values list.
 * @return      void
 * @section description
 * This callback function will be called if avrcp get application attribute values text request is received.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_get_app_att_vals_text_event_t)(rsi_bt_event_avrcp_vals_text_req_t *p_vals_list);

/**
 * @callback   rsi_bt_on_avrcp_supp_char_sets_event_t
 * @brief      Callback function is invoke when we receive inform character sets request from remote device.
 * @param[in]  p_char_sets, support character sets list.
 * @return      void
 * @section description
 * This callback function will be called if avrcp support character sets command is received.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_supp_char_sets_event_t)(rsi_bt_event_avrcp_char_sets_t *p_char_sets);

/**
 * @callback   rsi_bt_on_avrcp_batt_status_event_t
 * @brief      Callback function is invoke when we receive battery status from remote device.
 * @param[in]  bd_addr, Remote bd address.
 * @param[in]  batt_status, avrcp capabilities type.
 * @return      void
 * @section description
 * This callback function will be called if avrcp controller battery status is received.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_batt_status_event_t)(uint8_t *bd_addr, uint8_t batt_status);

/**
 * @callback   rsi_bt_on_avrcp_get_song_atts_event_t
 * @brief      Callback function is invoke when we receive element/song attribute request from remote device.
 * @param[in]  rsi_bt_event_avrcp_ele_att_t, requested song attribute list.
 * @return      void
 * @section description
 * This callback function will be called if avrcp song attributes request is received.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_get_song_atts_event_t)(rsi_bt_event_avrcp_ele_att_t *p_song_atts);

/**
 * @callback   rsi_bt_on_avrcp_get_play_status_event_t
 * @brief      Callback function is invoke when we receive get player status from remote device.
 * @param[in]  bd_addr, Remote bd address.
 * @return      void
 * @section description
 * This callback function will be called if avrcp controller requested the player status.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_get_play_status_event_t)(uint8_t *bd_addr);

/**
 * @callback   rsi_bt_on_avrcp_reg_notify_event_t
 * @brief      Callback function is invoke when we receive register notify event from remote device.
 * @param[in]  bd_addr, Remote bd address.
 * @param[in]  batt_status, avrcp capabilities type.
 * @return      void
 * @section description
 * This callback function will be called if register notification event is received.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_reg_notify_event_t)(uint8_t *bd_addr, uint8_t event_id);

/**
 * @callback   rsi_bt_on_avrcp_set_abs_vol_event_t
 * @brief      Callback function is invoke when we receive set absolute volume event from remote device.
 * @param[in]  bd_addr, Remote bd address.
 * @param[in]  vol, avrcp absolute vol.
 * @return      void
 * @section description
 * This callback function will be called if set absolute volume event is received.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_set_abs_vol_event_t)(uint16_t resp_status, rsi_bt_event_avrcp_set_abs_vol_t *p_abs_vol);

/**
 * @callback   rsi_bt_on_avrcp_set_addr_player_event_t
 * @brief      Callback function is invoke when we receive set absolute volume event from remote device.
 * @param[in]  bd_addr, Remote bd address.
 * @param[in]  vol, avrcp absolute vol.
 * @return      void
 * @section description
 * This callback function will be called if set absolute volume event is received.
 * This callback has to be registered using rsi_bt_avrcp_target_register_callbacks API
 */
typedef void (*rsi_bt_on_avrcp_set_addr_player_event_t)(rsi_bt_event_avrcp_set_addr_player_t *p_set_addr_player);

typedef void (*rsi_bt_on_avrcp_get_folder_items_event_t)(rsi_bt_event_avrcp_get_folder_items_t *p_get_folder_items);

typedef void (*rsi_bt_on_avrcp_get_tot_num_items_event_t)(rsi_bt_event_avrcp_get_tot_num_items_t *p_get_tot_num_items);

/*==============================================*/

/**
 * @fn          rsi_bt_avrcp_target_register_callbacks 
 * @brief       This function registers the avrcp target callbacks
 * @param[in]   rsi_bt_on_avrcp_get_cap_event_t                  bt_on_avrcp_gat_cap,
 * @param[in]   rsi_bt_on_avrcp_get_app_supp_atts_event_t        bt_on_avrcp_get_app_att_list,
 * @param[in]   rsi_bt_on_avrcp_get_app_suup_att_vals_event_t    bt_on_avrcp_get_app_att_vals_list,
 * @param[in]   rsi_bt_on_avrcp_get_app_cur_att_val_event_t      bt_on_avrcp_get_app_cur_att_val,
 * @param[in]   rsi_bt_on_avrcp_set_app_cur_att_val_event_t      bt_on_avrcp_set_app_cur_att_val,
 * @param[in]   rsi_bt_on_avrcp_get_app_att_text_event_t         bt_on_avrcp_get_app_att_text,
 * @param[in]   rsi_bt_on_avrcp_get_app_att_vals_text_event_t    bt_on_avrcp_get_app_att_vals_text,
 * @param[in]   rsi_bt_on_avrcp_supp_char_sets_event_t           bt_on_avrcp_supp_char_sets,
 * @param[in]   rsi_bt_on_avrcp_batt_status_event_t              bt_on_avrcp_batt_status,
 * @param[in]   rsi_bt_on_avrcp_get_song_atts_event_t            bt_on_avrcp_get_song_atts,
 * @param[in]   rsi_bt_on_avrcp_get_play_status_event_t          bt_on_avrcp_get_player_status,
 * @param[in]   rsi_bt_on_avrcp_reg_notify_event_t               bt_on_avrcp_reg_notifivation);
 * @param[in]   rsi_bt_on_avrcp_set_abs_vol_event_t              bt_on_avrcp_set_abs_vol);
 *
 * @return     
 *              Non zero - If fails
 *              0 - If success
 *
 *
 */
void rsi_bt_avrcp_target_register_callbacks(
  rsi_bt_on_avrcp_get_cap_event_t bt_on_avrcp_gat_cap,
  rsi_bt_on_avrcp_get_app_supp_atts_event_t bt_on_avrcp_get_app_att_list,
  rsi_bt_on_avrcp_get_app_suup_att_vals_event_t bt_on_avrcp_get_app_att_vals_list,
  rsi_bt_on_avrcp_get_app_cur_att_val_event_t bt_on_avrcp_get_app_cur_att_val,
  rsi_bt_on_avrcp_set_app_cur_att_val_event_t bt_on_avrcp_set_app_cur_att_val,
  rsi_bt_on_avrcp_get_app_att_text_event_t bt_on_avrcp_get_app_att_text,
  rsi_bt_on_avrcp_get_app_att_vals_text_event_t bt_on_avrcp_get_app_att_vals_text,
  rsi_bt_on_avrcp_supp_char_sets_event_t bt_on_avrcp_supp_char_sets,
  rsi_bt_on_avrcp_batt_status_event_t bt_on_avrcp_batt_status,
  rsi_bt_on_avrcp_get_song_atts_event_t bt_on_avrcp_get_song_atts,
  rsi_bt_on_avrcp_get_play_status_event_t bt_on_avrcp_get_player_status,
  rsi_bt_on_avrcp_reg_notify_event_t bt_on_avrcp_reg_notifivation,
  rsi_bt_on_avrcp_set_abs_vol_event_t bt_on_avrcp_set_abs_vol,
  rsi_bt_on_avrcp_set_addr_player_event_t bt_on_avrcp_set_addr_player,
  rsi_bt_on_avrcp_get_folder_items_event_t bt_on_avrcp_get_folder_items,
  rsi_bt_on_avrcp_get_tot_num_items_event_t bt_on_avrcp_get_tot_num_items);

/* HFP Profile */
typedef struct rsi_bt_event_hfp_connect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_connect_t;

typedef struct rsi_bt_event_hfp_disconnect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_disconnect_t;

typedef struct rsi_bt_event_hfp_ring_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_ring_t;
typedef struct rsi_bt_event_hfp_callstatus_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t call_status;
} rsi_bt_event_hfp_callstatus_t;

typedef struct rsi_bt_event_hfp_callsetup_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t call_setup;
} rsi_bt_event_hfp_callsetup_t;

typedef struct rsi_bt_event_hfp_audioconn_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_audioconn_t;

typedef struct rsi_bt_event_hfp_audiodisconn_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_audiodisconn_t;

typedef struct rsi_bt_event_hfp_signalstrength_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t signal_strength;
} rsi_bt_event_hfp_signalstrength_t;

typedef struct rsi_bt_event_hfp_batterylevel_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t battery_status;
} rsi_bt_event_hfp_batterylevel_t;

typedef struct rsi_bt_event_hfp_dialcomp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_dialcomp_t;

typedef struct rsi_bt_event_hfp_anscomp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_anscomp_t;

typedef struct rsi_bt_event_hfp_hangupcomp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_hangupcomp_t;

typedef struct rsi_bt_event_hfp_senddtmfcomp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_senddtmfcomp_t;

typedef struct rsi_bt_event_hfp_phoneservice_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t service;
} rsi_bt_event_hfp_phoneservice_t;

typedef struct rsi_bt_event_hfp_roamstatus_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t status;
} rsi_bt_event_hfp_roamstatus_t;

typedef struct rsi_bt_event_hfp_calheldrelease_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_calheldrelease_t;

typedef struct rsi_bt_event_hfp_callheld_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t held_status;

} rsi_bt_event_hfp_callheld_t;

typedef struct rsi_bt_event_hfp_calwait_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_calwait_t;
typedef struct rsi_bt_event_hfp_servnotfound_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_servnotfound_t;

typedef struct rsi_bt_event_hfp_voicerecogactivated_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_voicerecogactivated_t;

typedef struct rsi_bt_event_hfp_voicerecogdeactivated_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_hfp_voicerecogdeactivated_t;

typedef struct rsi_bt_event_hfp_dialnum_s {
  uint8_t dial_num[10];
} rsi_bt_event_hfp_dialnum_t;

typedef struct rsi_bt_event_hfp_spkgain_s {
  uint8_t spk_gain;
} rsi_bt_event_hfp_spkgain_t;

typedef struct rsi_bt_event_hfp_micgain_s {
  uint8_t mic_gain;
} rsi_bt_event_hfp_micgain_t;

typedef struct rsi_bt_event_hfp_phonenumber_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t PhoneNbr[21];
  uint8_t PhoneType;
  uint8_t PhoneNbrType;
} rsi_bt_event_phonenum_t;

/******************************************************
 * *       BT HFP Response Callbacks Declarations
 * ******************************************************/

/**
 * @callback   rsi_bt_on_hfp_connect_t
 * @brief      Callback function to be called if profiles list response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  hfp_connect, contains the hfp connect information
 * @return      void
 * @section description
 * This callback function will be called if hfp connection happens.
 * This callback has to be registered using rsi_bt_spp_register_callbacks API
 */
typedef void (*rsi_bt_on_hfp_connect_t)(uint16_t resp_status, rsi_bt_event_hfp_connect_t *hfp_connect);
typedef void (*rsi_bt_on_hfp_disconnect_t)(uint16_t resp_status, rsi_bt_event_hfp_disconnect_t *hfp_disconn);
typedef void (*rsi_bt_on_hfp_ring_t)(uint16_t resp_status, rsi_bt_event_hfp_ring_t *hfp_ring);
typedef void (*rsi_bt_on_hfp_callcallerid_t)(uint16_t resp_status, rsi_bt_event_phonenum_t *phonenum);
typedef void (*rsi_bt_on_hfp_audioconnected_t)(uint16_t resp_status, rsi_bt_event_hfp_audioconn_t *audioconn);
typedef void (*rsi_bt_on_hfp_audiodisconnected_t)(uint16_t resp_status, rsi_bt_event_hfp_audiodisconn_t *audiodisconn);
typedef void (*rsi_bt_on_hfp_dialcomplete_t)(uint16_t resp_status, rsi_bt_event_hfp_dialcomp_t *dialcomp);
typedef void (*rsi_bt_on_hfp_answercomplete_t)(uint16_t resp_status, rsi_bt_event_hfp_anscomp_t *anscomp);
typedef void (*rsi_bt_on_hfp_hangupcomplete_t)(uint16_t resp_status, rsi_bt_event_hfp_hangupcomp_t *hangupcomp);
typedef void (*rsi_bt_on_hfp_senddtmfcomplete_t)(uint16_t resp_status, rsi_bt_event_hfp_senddtmfcomp_t *senddtmfcomp);
typedef void (*rsi_bt_on_hfp_callwait_t)(uint16_t resp_status, rsi_bt_event_hfp_calwait_t *callwait);
typedef void (*rsi_bt_on_hfp_callvoicerecogdeactivated_t)(
  uint16_t resp_status,
  rsi_bt_event_hfp_voicerecogdeactivated_t *voicerecog_deactivated);
typedef void (*rsi_bt_on_hfp_callvoicerecogactivated_t)(uint16_t resp_status,
                                                        rsi_bt_event_hfp_voicerecogactivated_t *voicerecog_activated);
typedef void (*rsi_bt_on_hfp_servicenotfound_t)(uint16_t resp_status, rsi_bt_event_hfp_servnotfound_t *servnotfound);
typedef void (*rsi_bt_app_on_hfp_callstatus_t)(uint16_t resp_status, rsi_bt_event_hfp_callstatus_t *callstatus);
typedef void (*rsi_bt_app_on_hfp_signalstrength_t)(uint16_t resp_status,
                                                   rsi_bt_event_hfp_signalstrength_t *signalstrength);
typedef void (*rsi_bt_app_on_hfp_batterylevel_t)(uint16_t resp_status, rsi_bt_event_hfp_batterylevel_t *batterylevel);
typedef void (*rsi_bt_app_on_hfp_phoneservice_t)(uint16_t resp_status, rsi_bt_event_hfp_phoneservice_t *phoneservice);
typedef void (*rsi_bt_app_on_hfp_roamingstatus_t)(uint16_t resp_status, rsi_bt_event_hfp_roamstatus_t *roamstatus);
typedef void (*rsi_bt_app_on_hfp_callsetup_t)(uint16_t resp_status, rsi_bt_event_hfp_callsetup_t *callsetup);
typedef void (*rsi_bt_app_on_hfp_callheld_t)(uint16_t resp_status, rsi_bt_event_hfp_callheld_t *callheld);

/******************************************************
 * * BT HFP Callbacks register function Declarations
 * ******************************************************/

/*==============================================*/
/**
 * @fn          rsi_bt_hfp_register_callbacks 
 * @brief       This function registers the HFP callbacks
 * @param[in]   rsi_bt_on_hfp_connect_t         bt_on_hfp_connect_event              : HFP connection status callback
 * @param[in]   rsi_bt_on_hfp_disconnect_t      bt_on_hfp_disconnect_event           : HFP disconnection status callback
 * @param[in]   rsi_bt_on_hfp_ring_t            bt_on_hfp_ring_event                 : HFP call ring status callback
 * @return     
 *              Non zero - If fails
 *              0 - If success
 *
 */

void rsi_bt_hfp_register_callbacks(rsi_bt_on_hfp_connect_t bt_on_hfp_connect_event,
                                   rsi_bt_on_hfp_disconnect_t bt_on_hfp_disconnect_event,
                                   rsi_bt_on_hfp_ring_t bt_on_hfp_ring_event,
                                   rsi_bt_on_hfp_callcallerid_t bt_on_hfp_callcallerid_event,
                                   rsi_bt_on_hfp_audioconnected_t bt_on_hfp_audioconnected_event,
                                   rsi_bt_on_hfp_audiodisconnected_t bt_on_hfp_audiodisconnected_event,
                                   rsi_bt_on_hfp_dialcomplete_t bt_on_hfp_dialcomplete_event,
                                   rsi_bt_on_hfp_answercomplete_t bt_on_hfp_answercomplete_event,
                                   rsi_bt_on_hfp_hangupcomplete_t bt_on_hfp_hangupcomplete_event,
                                   rsi_bt_on_hfp_senddtmfcomplete_t bt_on_hfp_senddtmfcomplete_event,
                                   rsi_bt_on_hfp_callwait_t bt_on_hfp_callwait_event,
                                   rsi_bt_on_hfp_callvoicerecogdeactivated_t bt_on_hfp_callvoicerecogdeactivated_event,
                                   rsi_bt_on_hfp_callvoicerecogactivated_t bt_on_hfp_callvoicerecogactivated_event,
                                   rsi_bt_on_hfp_servicenotfound_t bt_on_hfp_servicenotfound_event,
                                   rsi_bt_app_on_hfp_callstatus_t bt_on_hfp_callstatus_event,
                                   rsi_bt_app_on_hfp_signalstrength_t bt_on_hfp_signalstrength_event,
                                   rsi_bt_app_on_hfp_batterylevel_t bt_on_hfp_batterylevel_event,
                                   rsi_bt_app_on_hfp_phoneservice_t bt_on_hfp_phoneservice_event,
                                   rsi_bt_app_on_hfp_roamingstatus_t bt_on_hfp_roamingstatus_event,
                                   rsi_bt_app_on_hfp_callsetup_t bt_on_hfp_callsetup_event,
                                   rsi_bt_app_on_hfp_callheld_t bt_on_hfp_callheld_event);

/* PBAP Profile */
typedef struct rsi_bt_event_pbap_connect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_pbap_connect_t;

typedef struct rsi_bt_event_pbap_disconnect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_event_pbap_disconnect_t;

typedef struct rsi_bt_event_pbap_data_s {
  uint8_t PhoneNbr[13];
  uint8_t Name[30];
} rsi_bt_event_pbap_data_t;

/******************************************************
 * *       BT PBAP Response Callbacks Declarations
 * ******************************************************/

/**
 * @callback   rsi_bt_on_pbap_connect_t
 * @brief      Callback function to be called if pbap connect response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  pbap_connect, contains the pbap connect information
 * @return      void
 * @section description
 * This callback function will be called if pbap connection happens.
 * This callback has to be registered using rsi_bt_pbap_register_callbacks API
 */
typedef void (*rsi_bt_on_pbap_connect_t)(uint16_t resp_status, rsi_bt_event_pbap_connect_t *pbap_connect);
/**
 * @callback   rsi_bt_on_pbap_disconnect_t
 * @brief      Callback function to be called if pbap disconnect response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  pbap_disconnect, contains the pbap connect information
 * @return      void
 * @section description
 * This callback function will be called if pbap disconnection happens.
 * This callback has to be registered using rsi_bt_pbap_register_callbacks API
 */

typedef void (*rsi_bt_on_pbap_disconnect_t)(uint16_t resp_status, rsi_bt_event_pbap_disconnect_t *pbap_disconn);
/**
 * @callback   rsi_bt_on_pbap_data_t
 * @brief      Callback function to be called if pbap data response is received
 * @param[in]  resp_status, contains the response status (Success or Error code)
 * @param[in]  pbap_data, contains the pbap data information
 * @return      void
 * @section description
 * This callback function will be called if pbap data happens.
 * This callback has to be registered using rsi_bt_pbap_register_callbacks API
 */

typedef void (*rsi_bt_on_pbap_data_t)(uint16_t resp_status, rsi_bt_event_pbap_data_t *pbap_data);

/******************************************************
 * * BT PBAP Callbacks register function Declarations
 * ******************************************************/

/*==============================================*/
/**
 * @fn          rsi_bt_pbap_register_callbacks 
 * @brief       This function registers the PBAP callbacks
 * @param[in]   rsi_bt_on_pbap_connect_t         bt_on_pbap_connect_event              : PBAP connection status callback
 * @param[in]   rsi_bt_on_pbap_disconnect_t      bt_on_pbap_disconnect_event           : PBAP disconnection status callback
 * @param[in]   rsi_bt_on_pbap_data_t            bt_on_pbap_data_event                 : PBAP data status callback
 * @return     
 *              Non zero - If fails
 *              0 - If success
 *
 *
 */

void rsi_bt_pbap_register_callbacks(rsi_bt_on_pbap_connect_t bt_on_pbap_connect_event,
                                    rsi_bt_on_pbap_disconnect_t bt_on_pbap_disconnect_event,
                                    rsi_bt_on_pbap_data_t bt_on_pbap_data_event);
/*==============================================*/
/*
 * @fn         rsi_bt_add_device_id
 * @brief      To add device identification for local device
 * @param[in]  Specification ID
 *             Vendor ID
 *             Product ID
 *             Version
 *             Primary record
 *             Vendor ID source
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used to add the device id in the SDP.
  
-------------------------------------------------------------------------------------------------*/
int32_t rsi_bt_add_device_id(uint16_t spec_id,
                             uint16_t vendor_id,
                             uint16_t product_id,
                             uint16_t version,
                             int primary_rec,
                             uint16_t vendor_id_source);

/**
 * @fn         rsi_bt_set_sdp_attr_id
 */
uint32_t rsi_bt_set_sdp_attr_id(rsi_sdp_att_record_t *att_rec,
                                uint16_t attr_id,
                                uint16_t pattr_buf_idx,
                                uint16_t attr_buf_len);

/**
 * @fn         rsi_bt_add_sdp_attribute
 */
uint32_t rsi_bt_add_sdp_attribute(rsi_sdp_att_record_t *att_rec,
                                  uint16_t attr_id,
                                  uint8_t att_data_8,
                                  uint16_t att_data_16,
                                  uint8_t is_boolean,
                                  uint8_t param_len);

/**
 * @fn         rsi_bt_add_sdp_hid_language_attribute
 */
uint32_t rsi_bt_add_sdp_hid_language_attribute(rsi_sdp_att_record_t *att_record_data,
                                               uint16_t lang_id,
                                               uint16_t lang_attr_base);

/**
 * @fn         rsi_bt_add_sdp_hid_descriptor_list
 */
uint32_t rsi_bt_add_sdp_hid_descriptor_list(rsi_sdp_att_record_t *att_record_data, uint8_t *buff_ptr, uint8_t buff_len);

/**
 * @fn         rsi_bt_add_sdp_service_attribute
 */
uint32_t rsi_bt_add_sdp_service_attribute(rsi_sdp_att_record_t *att_rec,
                                          char *service_name,
                                          uint8_t name_len,
                                          uint16_t attr_id);

/**
 * @fn         rsi_bt_add_sdp_service_classid
 */
uint32_t rsi_bt_add_sdp_service_classid(rsi_sdp_att_record_t *att_rec, uint16_t serv_class_uuid);

/**
 * @fn         rsi_bt_add_sdp_service_handle
 */
uint32_t rsi_bt_add_sdp_service_handle(rsi_sdp_att_record_t *att_rec, uint32_t serv_hndl);

/**
 * @fn         rsi_bt_add_sdp_protocol_list
 */
uint32_t rsi_bt_add_sdp_protocol_list(rsi_sdp_att_record_t *att_rec,
                                      bt_sdp_proto_desc_list_elem_t *list,
                                      uint8_t list_cnt,
                                      uint16_t attr_id);

/**
 * @fn         rsi_bt_add_sdp_language_base_attributeid_list
 */
uint32_t rsi_bt_add_sdp_language_base_attributeid_list(rsi_sdp_att_record_t *att_rec,
                                                       bt_sdp_lang_attr_id_elem_t *list,
                                                       uint8_t list_cnt);

/**
 * @fn         rsi_bt_add_sdp_profile_descriptor_list
 */
uint32_t rsi_bt_add_sdp_profile_descriptor_list(rsi_sdp_att_record_t *att_rec,
                                                uint16_t profile_uuid,
                                                uint16_t profile_version);

/**
 * @fn         rsi_bt_add_sdp_service_record_handle
 */
uint32_t rsi_bt_add_sdp_service_record_handle(rsi_sdp_att_record_t *att_rec, uint32_t serv_hndl);

// A2DP Connect command
typedef struct rsi_bt_req_a2dp_connect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_a2dp_connect_t;

// A2DP disconnect command
typedef struct rsi_bt_req_a2dp_disconnect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_a2dp_disconnect_t;

// A2DP pcm/mp3 data structure
typedef struct rsi_bt_req_a2dp_pcm_mp3_data_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint16_t pcm_mp3_data_len : 12;
  uint16_t audio_type : 4;
  uint8_t pcm_mp3_data[512 * 7]; //[RSI_BT_MAX_PAYLOAD_SIZE];
} rsi_bt_req_a2dp_pcm_mp3_data_t;

// A2DP sbc data command
typedef struct rsi_bt_req_a2dp_sbc_aac_data_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint16_t sbc_aac_data_len : 12;
  uint16_t audio_type : 4;
  uint8_t reserved[13]; // RTP_HEADER_LEN
  uint8_t sbc_aac_data[RSI_BT_MAX_PAYLOAD_SIZE];
} rsi_bt_req_a2dp_sbc_aac_data_t;

// A2DP request start command
typedef struct rsi_bt_req_a2dp_start_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_a2dp_start_t;

// A2DP request suspend command
typedef struct rsi_bt_req_a2dp_suspend_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_a2dp_suspend_t;

// A2DP request close command
typedef struct rsi_bt_req_a2dp_close_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_a2dp_close_t;

// A2DP request abort command
typedef struct rsi_bt_req_a2dp_abort_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_a2dp_abort_t;

// AVRCP profile command
// AVRCP profile connect command
typedef struct rsi_bt_req_avrcp_conn_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_avrcp_conn_t;

// AVRCP profile disconnect command
typedef struct rsi_bt_req_avrcp_disconnect_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_avrcp_disconnect_t;

typedef struct rsi_bt_a2dp_sbc_codec_cap_s {
  uint8_t ChannelMode : 4;
  uint8_t Freq : 4;
  uint8_t AllocMethod : 2;
  uint8_t SubBands : 2;
  uint8_t BlockLength : 4;
  uint8_t MinBitPool;
  uint8_t MaxBitPool;
} rsi_bt_a2dp_sbc_codec_cap_t;

typedef struct rsi_bt_req_a2dp_get_config_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_a2dp_get_config_t;

typedef struct rsi_bt_resp_a2dp_get_config_s {
  rsi_bt_a2dp_sbc_codec_cap_t sbc_cap;
  uint32_t status;
} rsi_bt_resp_a2dp_get_config_t;

typedef struct rsi_bt_req_a2dp_set_config_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  rsi_bt_a2dp_sbc_codec_cap_t sbc_cap;
} rsi_bt_req_a2dp_set_config_t;

typedef struct rsi_bt_resp_a2dp_set_config_s {
  uint32_t err_status;
} rsi_bt_resp_a2dp_set_config_t;

// AVRCP profile play song command
typedef struct rsi_bt_req_avrcp_play_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_avrcp_play_t;

// AVRCP profile pause song command
typedef struct rsi_bt_req_avrcp_pause_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_avrcp_pause_t;

// AVRCP profile stop song command
typedef struct rsi_bt_req_avrcp_stop_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_avrcp_stop_t;

// AVRCP profile next song command
typedef struct rsi_bt_req_avrcp_next_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_avrcp_next_t;

// AVRCP profile previous song command
typedef struct rsi_bt_req_avrcp_previous_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_avrcp_previous_t;

// AVRCP profile vol up command
typedef struct rsi_bt_req_avrcp_vol_up_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_avrcp_vol_up_t;

// AVRCP profile vol down command
typedef struct rsi_bt_req_avrcp_vol_down_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_avrcp_vol_down_t;

// AVRCP profile get capabilities command
typedef struct rsi_bt_req_avrcp_get_capabilities_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t type;
} rsi_bt_req_avrcp_get_capabilities_t;

typedef struct rsi_bt_rsp_avrcp_get_capabilities_s {
  uint32_t nbr_ids;
  uint32_t ids[RSI_MAX_ATT];
} rsi_bt_rsp_avrcp_get_capabilities_t;

// AVRCP profile get attributes list command
typedef struct rsi_bt_req_avrcp_get_att_list_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_avrcp_get_att_list_t;

typedef struct rsi_bt_rsp_avrcp_get_atts_list_s {
  uint8_t nbr_ids;
  uint8_t att_ids[RSI_MAX_ATT];
} rsi_bt_rsp_avrcp_get_atts_list_t;

// AVRCP profile get attribute values list command
typedef struct rsi_bt_req_avrcp_get_att_vals_list_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t att_id;
} rsi_bt_req_avrcp_get_att_vals_list_t;

typedef struct rsi_bt_rsp_avrcp_get_att_vals_list_s {
  uint8_t nbr_vals;
  uint8_t att_vals[RSI_MAX_ATT];
} rsi_bt_rsp_avrcp_get_att_vals_list_t;

// AVRCP profile get current attribute value command
typedef struct rsi_bt_req_avrcp_get_cur_att_val_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t nbr_atts;
  uint8_t att_list[RSI_MAX_ATT];
} rsi_bt_req_avrcp_get_cur_att_val_t;

typedef struct rsi_bt_rsp_avrcp_get_cur_att_val_s {
  uint8_t nbr_vals;
  uint8_t att_vals[RSI_MAX_ATT];
} rsi_bt_rsp_avrcp_get_cur_att_val_t;

// AVRCP profile Set current attribute value command

typedef struct rsi_bt_req_avrcp_set_cur_att_val_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t nbr_atts;
  uint8_t reserved;
  att_val_t att_val_list[RSI_MAX_ATT];
} rsi_bt_req_avrcp_set_cur_att_val_t;

// AVRCP profile get song attribute value command
typedef struct rsi_bt_req_avrcp_get_ele_att_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t nbr_atts;
  uint8_t reserved;
  uint32_t att_list[RSI_MAX_ATT];
} rsi_bt_req_avrcp_get_ele_att_t;

typedef struct normal_time_s {
  uint32_t org_time;
  uint32_t min;
  uint32_t sec;
} normal_time_t;

typedef struct att_text_s {
  uint8_t id;
  uint8_t att_text[31];
} att_text_t;
#define MAX_TEXT_LIST 7
typedef struct player_att_text_s {
  att_text_t list[MAX_TEXT_LIST];
  uint8_t nbr_atts;
} player_att_text_t;

typedef struct attr_list_s {
  uint32_t id;
  uint16_t char_set_id;
  uint16_t attr_len;
  /** Limiting to 50 bytes*/
  uint8_t attr_val[50]; //499
} attr_list_t;
#define MAX_ATT_LIST 7
typedef struct elem_attr_list_s {
  uint8_t num_attrs;
  attr_list_t attr_list[MAX_ATT_LIST];
} elem_attr_list_t;

typedef struct rsi_bt_rsp_avrcp_get_ele_att_s {
  player_att_text_t player_attr;
} rsi_bt_rsp_avrcp_get_ele_att_t;

typedef struct rsi_bt_rsp_avrcp_elem_attr_s {
  elem_attr_list_t elem_attr_list;
} rsi_bt_rsp_avrcp_elem_attr_t;

// AVRCP profile get player status command
typedef struct rsi_bt_req_avrcp_get_player_status_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_avrcp_get_player_status_t;

typedef struct rsi_bt_rsp_avrcp_get_player_status_s {
  uint32_t song_len;
  uint32_t song_pos;
  uint8_t play_status;
} rsi_bt_rsp_avrcp_get_player_status_t;

// AVRCP profile register notification event command
typedef struct rsi_bt_req_avrcp_reg_notification_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t event_id;
} rsi_bt_req_avrcp_reg_notification_t;

// AVRCP profile get player status command
typedef struct rsi_bt_req_avrcp_remote_version_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
} rsi_bt_req_avrcp_remote_version_t;

typedef struct rsi_bt_rsp_avrcp_remote_version_s {
  uint16_t version;
  uint16_t reserved;
} rsi_bt_rsp_avrcp_remote_version_t;

// AVRCP profile get attribute value text command
typedef struct rsi_bt_req_avrcp_get_att_val_text_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t att_id;
  uint8_t nbr_vals;
  uint8_t vals[RSI_MAX_ATT];
} rsi_bt_req_avrcp_get_att_val_text_t;

// AVRCP profile send the battery status command
typedef struct rsi_bt_req_avrcp_batt_status_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t batt_status;
} rsi_bt_req_avrcp_batt_status_t;

// AVRCP profile send the support character sets command
#define MAX_SUPP_VALS 5
typedef struct char_sets_s {
  uint16_t supp_vals[MAX_SUPP_VALS];
  uint8_t nbr_sets;
} char_sets_t;

typedef struct rsi_bt_req_avrcp_char_sets_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t Reserved[2];
  char_sets_t char_sets;
} rsi_bt_req_avrcp_char_sets_t;

typedef struct rsi_bt_avrcp_set_abs_vol_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t rev;
  uint8_t abs_vol;
} rsi_bt_avrcp_set_abs_vol_t;

#define AVRCP_SUPP_CMP_CAP_ID     2
#define AVRCP_SUPP_NOTIFYS_CAP_ID 3
#define MAX_CAPS                  10
typedef struct rsi_bt_avrcp_cap_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t cap_type;
  uint8_t nbr_caps;
  uint32_t caps[MAX_CAPS];
} rsi_bt_avrcp_cap_resp_t;

typedef struct rsi_bt_avrcp_att_list_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t nbr_atts;
  uint8_t atts[MAX_CAPS];
} rsi_bt_avrcp_att_list_resp_t;

typedef struct rsi_bt_avrcp_att_vals_list_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t nbr_vals;
  uint8_t vals[MAX_CAPS];
} rsi_bt_avrcp_att_vals_list_resp_t;

typedef struct rsi_bt_avrcp_cur_att_vals_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t reserved[2];
  att_val_list_t att_list;
} rsi_bt_avrcp_cur_att_vals_resp_t;

typedef struct rsi_bt_avrcp_set_att_vals_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t status;
} rsi_bt_avrcp_set_att_vals_resp_t;

typedef struct rsi_bt_avrcp_elem_attr_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t reserved[2];
  elem_attr_list_t elem_attr_list;
} rsi_bt_avrcp_elem_attr_resp_t;

typedef struct rsi_bt_avrcp_play_status_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t play_status;
  uint8_t reserved;
  uint32_t song_len;
  uint32_t song_pos;
} rsi_bt_avrcp_play_status_resp_t;

typedef union reg_notify_val_u {
  uint8_t play_status;
  uint8_t curr_track_idx[8];
  uint8_t curr_playback_pos[4];
  uint8_t batter_status;
  uint8_t system_status;
  uint8_t player_app_setting[3];
  uint16_t playerid;
  uint16_t uidcounter;
  uint8_t abs_vol;
} reg_notify_val_t;

typedef struct rsi_bt_avrcp_reg_notify_interim_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t event_id;
  uint8_t reserved;
  reg_notify_val_t reg_notify_val;
} rsi_bt_avrcp_reg_notify_interim_resp_t;

typedef struct rsi_bt_avrcp_reg_notifiy_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t status;
} rsi_bt_avrcp_reg_notify_resp_t;

typedef struct rsi_bt_avrcp_att_text_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t reserved[2];
  player_att_text_t atts_text;
} rsi_bt_avrcp_att_text_resp_t;

/* typedef union notify_val_s {
  uint8_t   player_status;
  uint32_t  track_num;
  uint32_t  track_pos;
  uint8_t   battery_status;
  uint8_t   system_status;
  att_val_list_t  att_val_list;
} notify_val_t; */

typedef struct rsi_bt_avrcp_notify_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t event_id;
  uint8_t reserved;
  notify_val_t notify_val;
} rsi_bt_avrcp_notify_t;

typedef struct rsi_bt_avrcp_cmd_reject_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t pdu_id;
  uint8_t status;
} rsi_bt_avrcp_cmd_reject_t;

typedef struct rsi_bt_avrcp_set_abs_vol_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t abs_vol;
} rsi_bt_avrcp_set_abs_vol_resp_t;

typedef struct rsi_bt_avrcp_set_addr_player_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t status;
} rsi_bt_avrcp_set_addr_player_resp_t;

typedef struct folder_items_resp_s {
  uint16_t uidcntr;
  uint16_t numofitems;
  uint8_t itemtype;
  uint16_t itemlen;
  uint16_t playerid;
  uint8_t majorplayertype;
  uint32_t playersubtype;
  uint8_t playstatus;
  uint8_t featurebitmask[16];
  uint16_t charsetid;
  uint16_t disnamelen;
  uint8_t disname[11];
} folder_items_resp_t;

typedef struct rsi_bt_avrcp_get_folder_items_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  folder_items_resp_t fldr_items;
  uint8_t status;
} rsi_bt_avrcp_get_folder_items_resp_t;

typedef struct rsi_bt_avrcp_get_tot_num_items_resp_s {
  uint8_t dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t status;
  uint16_t uidcntr;
  uint32_t numofitems;
} rsi_bt_avrcp_get_tot_num_items_resp_t;
#define RSI_BT_IAP_MAX_DATA_LEN 300
typedef struct rsi_bt_iap_payload_s {
  uint16_t len;
  uint8_t data[1000];
} rsi_bt_iap_payload_t;

typedef struct rsi_bt_iap_cmds_info_s {
  uint16_t len;
  uint16_t cmd[20];
} rsi_bt_iap_cmds_info_t;

typedef struct rsi_bt_iap_File_transfer_info_s {
  uint16_t len;
  uint8_t state;
  uint8_t FileTransferId;
  uint8_t FileTransferSessionId;
  uint8_t data[RSI_BT_IAP_MAX_DATA_LEN];
} rsi_bt_iap_File_transfer_info_t;

int32_t rsi_bt_a2dp_init(rsi_bt_a2dp_sbc_codec_cap_t *sbc_cap);
int32_t rsi_bt_a2dp_connect(uint8_t *remote_dev_addr);
int32_t rsi_bt_a2dp_disconnect(uint8_t *remote_dev_addr);

int32_t rsi_bt_a2dp_send_pcm_mp3_data(uint8_t *remote_dev_addr,
                                      uint8_t *pcm_mp3_data,
                                      uint16_t pcm_mp3_data_len,
                                      uint8_t audio_type);
int32_t rsi_bt_a2dp_send_sbc_aac_data(uint8_t *remote_dev_addr,
                                      uint8_t *sbc_aac_data,
                                      uint16_t sbc_aac_data_len,
                                      uint8_t audio_type);
int32_t rsi_bt_a2dp_start(uint8_t *remote_dev_addr);
int32_t rsi_bt_a2dp_suspend(uint8_t *remote_dev_addr);
int32_t rsi_bt_a2dp_close(uint8_t *remote_dev_addr);
int32_t rsi_bt_a2dp_abort(uint8_t *remote_dev_addr);
int32_t rsi_bt_a2dp_get_config(uint8_t *remote_dev_addr, rsi_bt_resp_a2dp_get_config_t *sbc_resp_cap);
int32_t rsi_bt_a2dp_set_config(uint8_t *remote_dev_addr, rsi_bt_a2dp_sbc_codec_cap_t *set_sbc_cap, int32_t *status);
int32_t rsi_bt_app_sbc_encode(void);
int32_t rsi_sbc_encode(void);
int32_t rsi_bt_set_local_device_role(uint8_t *remote_dev_addr, uint8_t set_role, uint8_t *resp);
int32_t rsi_switch_proto(uint8_t type, void (*callback)(uint16_t mode, uint8_t *bt_disable));

int32_t rsi_bt_avrcp_init(uint8_t *avrcp_feature);
int32_t rsi_bt_avrcp_conn(uint8_t *remote_dev_addr);
int32_t rsi_bt_avrcp_disconn(uint8_t *remote_dev_addr);
int32_t rsi_bt_avrcp_play(uint8_t *remote_dev_addr);
int32_t rsi_bt_avrcp_pause(uint8_t *remote_dev_addr);
int32_t rsi_bt_avrcp_stop(uint8_t *remote_dev_addr);
int32_t rsi_bt_avrcp_next(uint8_t *remote_dev_addr);
int32_t rsi_bt_avrcp_previous(uint8_t *remote_dev_addr);
int32_t rsi_bt_avrcp_vol_up(uint8_t *remote_dev_addr);
int32_t rsi_bt_avrcp_vol_down(uint8_t *remote_dev_addr);
int32_t rsi_bt_avrcp_get_capabilities(uint8_t *remote_dev_addr,
                                      uint8_t capability_type,
                                      rsi_bt_rsp_avrcp_get_capabilities_t *cap_list);
int32_t rsi_bt_avrcp_get_att_list(uint8_t *remote_dev_addr, rsi_bt_rsp_avrcp_get_atts_list_t *att_list);
int32_t rsi_bt_avrcp_get_att_vals_list(uint8_t *remote_dev_addr,
                                       uint8_t att_id,
                                       rsi_bt_rsp_avrcp_get_att_vals_list_t *att_vals_list);
int32_t rsi_bt_avrcp_get_cur_att_val(uint8_t *remote_dev_addr,
                                     uint8_t *att_list,
                                     uint8_t nbr_atts,
                                     rsi_bt_rsp_avrcp_get_cur_att_val_t *att_vals_list);
int32_t rsi_bt_avrcp_set_cur_att_val(uint8_t *remote_dev_addr, att_val_t *val_list, uint8_t nbr_atts);
int32_t rsi_bt_avrcp_get_element_att(uint8_t *remote_dev_addr,
                                     uint8_t *att_ids,
                                     uint8_t nbr_atts,
                                     rsi_bt_rsp_avrcp_get_ele_att_t *att_vals);
int32_t rsi_bt_avrcp_get_play_status(uint8_t *remote_dev_addr, rsi_bt_rsp_avrcp_get_player_status_t *play_status);
int32_t rsi_bt_avrcp_reg_notification(uint8_t *remote_dev_addr, uint8_t event_id, uint8_t *p_resp_val);
int32_t rsi_bt_avrcp_get_remote_version(uint8_t *remote_dev_addr, rsi_bt_rsp_avrcp_remote_version_t *version);
int32_t rsi_bt_avrcp_get_att_text(uint8_t *remote_dev_addr,
                                  uint8_t nbr_atts,
                                  uint8_t *p_atts,
                                  player_att_text_t *p_att_text_resp);
int32_t rsi_bt_avrcp_get_att_val_text(uint8_t *remote_dev_addr,
                                      uint8_t att_id,
                                      uint8_t nbr_vals,
                                      uint8_t *p_vals,
                                      player_att_text_t *p_att_text_resp);
int32_t rsi_bt_avrcp_batt_status(uint8_t *remote_dev_addr, uint8_t batt_level);
int32_t rsi_bt_avrcp_supp_char_sets(uint8_t *remote_dev_addr, uint8_t nbr_sets, uint16_t *p_sets);
int32_t rsi_bt_avrcp_set_abs_vol(uint8_t *remote_dev_addr, uint8_t abs_vol, uint8_t *p_resp_abs_vol);
int32_t rsi_bt_avrcp_cap_resp(uint8_t *remote_dev_addr, uint8_t cap_type, uint8_t nbr_caps, uint32_t *p_caps);
int32_t rsi_bt_avrcp_att_list_resp(uint8_t *remote_dev_addr, uint8_t nbr_atts, uint8_t *p_atts);
int32_t rsi_bt_avrcp_att_val_list_resp(uint8_t *remote_dev_addr, uint8_t nbr_vals, uint8_t *p_vals);
int32_t rsi_bt_avrcp_cur_att_val_resp(uint8_t *remote_dev_addr, uint8_t nbr_atts, att_val_t *p_att_vals);
int32_t rsi_bt_avrcp_set_cur_att_val_resp(uint8_t *remote_dev_addr, uint8_t status);
int32_t rsi_bt_avrcp_ele_att_resp(uint8_t *remote_dev_addr, uint8_t num_attrs, attr_list_t *p_attr_list);
int32_t rsi_bt_avrcp_play_status_resp(uint8_t *remote_dev_addr,
                                      uint8_t play_status,
                                      uint32_t song_len,
                                      uint32_t song_pos);
int32_t rsi_bt_avrcp_reg_notify_resp(uint8_t *remote_dev_addr,
                                     uint8_t event_id,
                                     uint8_t event_data_len,
                                     uint8_t *event_data);
int32_t rsi_bt_avrcp_att_text_resp(uint8_t *remote_dev_addr, uint8_t nbr_atts, att_text_t *p_att_text);
int32_t rsi_bt_avrcp_att_vals_text_resp(uint8_t *remote_dev_addr, uint8_t nbr_vals, att_text_t *p_vals_text);
int32_t rsi_bt_avrcp_batt_status_resp(uint8_t *remote_dev_addr, uint8_t status);
int32_t rsi_bt_avrcp_char_set_status_resp(uint8_t *remote_dev_addr, uint8_t status);
int32_t rsi_bt_avrcp_notify(uint8_t *remote_dev_addr, uint8_t event_id, notify_val_t *p_notify_val);
int32_t rsi_bt_avrcp_cmd_reject(uint8_t *remote_dev_addr, uint8_t pdu_id, uint8_t status);
int32_t rsi_bt_avrcp_set_abs_vol_resp(uint8_t *remote_dev_addr, uint8_t abs_vol);
int32_t rsi_bt_avrcp_set_addr_player_resp(uint8_t *remote_dev_addr, uint8_t status);
int32_t rsi_bt_avrcp_get_folder_items_resp(uint8_t *remote_dev_addr,
                                           uint8_t status,
                                           folder_items_resp_t folder_items_resp);
int32_t rsi_bt_avrcp_get_tot_num_items_resp(uint8_t *remote_dev_addr,
                                            uint8_t status,
                                            uint16_t uidcntr,
                                            uint32_t numofitems);

int32_t rsi_bt_write_current_iac_lap(uint8_t no_of_iaps, uint8_t *iap_lap_list);
int32_t rsi_bt_set_afh_host_channel_classification(uint8_t enable, uint8_t *channel_map);
int32_t rsi_bt_get_afh_host_channel_classification(uint8_t *status);
int32_t rsi_bt_enable_device_under_testmode(void);
int32_t rsi_bt_gatt_connect(uint8_t *remote_dev_addr);
int32_t rsi_bt_per_rx(uint32_t *bt_perrx);
int32_t rsi_bt_per_tx(uint32_t *bt_pertx);
int32_t rsi_bt_change_pkt_type(uint8_t *remote_dev_addr, uint16_t pkt_type);
int32_t rsi_bt_ptt_req(uint8_t mode);

int32_t rsi_bt_per_stats(uint8_t cmd_type, struct rsi_bt_per_stats_s *per_stats);
int32_t rsi_bt_vendor_avdtp_stats_enable(uint16_t avdtp_stats_enable, uint32_t avdtp_stats_rate);
int32_t rsi_bt_vendor_ar_enable(uint16_t enable);
int32_t rsi_memory_stats_enable(uint8_t protocol, uint8_t memory_stats_enable, uint32_t memory_stats_interval_ms);
int32_t rsi_bt_vendor_dynamic_pwr(uint16_t enable,
                                  uint8_t *remote_dev,
                                  uint8_t power_index_br,
                                  uint8_t power_index_2m,
                                  uint8_t power_index_3m);
int32_t rsi_bt_vendor_set_afh_classification_intervals(uint16_t afh_min, uint16_t afh_max);

int32_t rsi_bt_iap_send_control_session_data(rsi_bt_iap_payload_t *payload);
int32_t rsi_bt_iap2_update_send_cmds_info(rsi_bt_iap_cmds_info_t *cmd_list);
int32_t rsi_bt_iap2_update_recv_cmds_info(rsi_bt_iap_cmds_info_t *cmd_list);
int32_t rsi_bt_iap2_Init_File_Transfer(void);
int32_t rsi_bt_iap2_deInit_File_Transfer(void);
int32_t rsi_bt_iap2_Send_File_Transfer_State(rsi_bt_iap_File_transfer_info_t *payload_info);
int32_t rsi_bt_iap2_Send_File_Transfer_data(rsi_bt_iap_File_transfer_info_t *payload_info);
int32_t rsi_bt_spp_disconnect(uint8_t *remote_dev_addr);
int32_t rsi_bt_spp_transfer(uint8_t *remote_dev_addr, uint8_t *data, uint16_t length);
int32_t rsi_bt_hfp_init(void);
int32_t rsi_bt_hfp_connect(uint8_t *remote_dev_addr);
int32_t rsi_bt_hfp_disconnect(uint8_t *remote_dev_addr);
int32_t rsi_bt_hfp_phoneoperator(uint8_t *operator);
int32_t rsi_bt_hfp_callaccept(void);
int32_t rsi_bt_hfp_callreject(void);
int32_t rsi_bt_hfp_dialnum(uint8_t *phone_no);
int32_t rsi_bt_hfp_dialmem(uint8_t *mem_ix);
int32_t rsi_bt_hfp_redial(void);
int32_t rsi_bt_hfp_voicerecognitionactive(void);
int32_t rsi_bt_hfp_voicerecognitiondeactive(void);
int32_t rsi_bt_hfp_spkgain(uint8_t spk_gain);
int32_t rsi_bt_hfp_micgain(uint8_t mic_gain);
int32_t rsi_bt_hfp_getcalls(void);
int32_t rsi_bt_hfp_audio(void);
int32_t rsi_bt_pbap_init(void);
int32_t rsi_bt_pbap_connect(uint8_t *remote_dev_addr);
int32_t rsi_bt_pbap_disconnect(uint8_t *remote_dev_addr);
int32_t rsi_bt_pbap_contacts(uint8_t *remote_dev_addr, uint8_t folder_id, uint8_t nbr_contacts, uint8_t start_ix);
#endif
