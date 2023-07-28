/*******************************************************************************
* @file  rsi_prop_protocol.h
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

#ifndef RSI_PROP_PROTOCOL_BLE_H
#define RSI_PROP_PROTOCOL_BLE_H

#include <rsi_data_types.h>
#include <rsi_bt_common.h>
#include <rsi_ble_apis.h>
// enumeration for PROP_PROTOCOL command request codes
typedef enum rsi_prop_protocol_cmd_request_e {
  PROP_PROTOCOL_ATM_CMD            = 0x0001,
  PROP_PROTOCOL_GET_VERSION        = 0x0002,
  PROP_PROTOCOL_GET_STATS          = 0x0003,
  PROP_PROTOCOL_GET_ACTIVITY_STATS = 0x0004,
  PROP_PROTOCOL_PER_MODE_REQ       = 0X0020,
  PROP_PROTOCOL_DATA_CONFIG_REQ    = 0x0023,
  PROP_PROTOCOL_ENCRYPTION_REQ     = 0x0024,
} rsi_prop_protocol_cmd_request_t;

// enumeration for PROP_PROTOCOL command response codes
typedef enum rsi_prop_protocol_cmd_resp_e {
  PROP_PROTOCOL_ATM_CMD_RESP            = 0x0001,
  PROP_PROTOCOL_GET_VERSION_RESP        = 0x0002,
  PROP_PROTOCOL_GET_STATS_RESP          = 0x0003,
  PROP_PROTOCOL_GET_ACTIVITY_STATS_RESP = 0x0004,
  PROP_PROTOCOL_PER_MODE_RESP           = 0X0020,
  PROP_PROTOCOL_DATA_CONFIG_RESP        = 0x0023,
  PROP_PROTOCOL_ENCRYPTION_RESP         = 0x0024,
} rsi_prop_protocol_cmd_resp_t;

#if 1 /* Used in PROP_PROTOCOL PER Commands */

//PROP_PROTOCOL Per Trasmit Data  cmd_ix = 0xE000
typedef struct __attribute__((__packed__)) rsi_prop_protocol_tx_data_s {
  uint8_t cmd_ix;
  uint8_t prop_protocol_data_len;
  uint8_t prop_protocol_data_payload[20];
} rsi_prop_protocol_tx_data_t;
//PROP_PROTOCOL Per ENCYPTION Data, cmd_ix = 0xE000
typedef struct __attribute__((__packed__)) rsi_prop_protocol_encryption_data_s {
  uint8_t cmd_ix;
  uint8_t prop_protocol_enc_offset;
  uint8_t prop_protocol_enc_key[16];
  uint8_t prop_protocol_enc_ctr[16];
} rsi_prop_protocol_encryption_data_t;

//PROP_PROTOCOL Per mode Command, cmd_ix = 0xE000
typedef struct __attribute__((__packed__)) rsi_prop_protocol_start_atm_s {
  uint8_t cmd_ix;
  uint8_t enable;
  uint8_t dtm_mode;
  uint32_t mode_flags;
  uint8_t rf_channel;
  uint8_t tx_power;
  uint8_t sync_word[4];
  uint16_t pkt_length;
  uint32_t prop_protocol_mod_index;
} rsi_prop_protocol_start_atm_t;

//PROP_PROTOCOL Per mode Command, cmd_ix = 0xE000
typedef struct __attribute__((__packed__)) rsi_prop_protocol_per_stats_s {
  uint8_t cmd_ix;
  uint16_t crc_fail;
  uint16_t crc_pass;
  uint16_t tx_aborts;
  uint16_t cca_stk;
  uint16_t cca_not_stk;
  uint16_t fls_rx_start;
  uint16_t cca_idle;
  uint16_t tx_dones;
  uint16_t rssi;
  uint16_t id_pkts_rcvd;
} rsi_prop_protocol_per_stats_t;

#endif

// enumeration for PROP_PROTOCOL Asyncronous Events
typedef enum rsi_prop_protocol_event_e {
  RSI_PROP_PROTOCOL_SCHED_STATS    = 0x1530,
  RSI_PROP_PROTOCOL_ACTIVITY_STATS = 0x1531,
} rsi_prop_protocol_event_t;

typedef struct rsi_prop_protocol_stack_cmd_s {
  uint8_t data[256];
} rsi_prop_protocol_stack_cmd_t;

int32_t rsi_prop_protocol_send_cmd(void *prop_protocol_cmd, void *prop_protocol_cmd_resp);
int32_t rsi_prop_protocol_send_cmd_per(void *prop_protocol_cmd, void *prop_protocol_cmd_resp);
typedef struct rsi_prop_protocol_atm_mode_s {
  uint8_t cmd_ix;
  uint8_t flags;
} rsi_prop_protocol_atm_mode_t;

//PROP_PROTOCOL get versions cmd structure
typedef struct rsi_prop_protocol_get_ver_s {
  uint8_t cmd_ix;
} rsi_prop_protocol_get_ver_t;

//PROP_PROTOCOL scheduling stats
typedef struct rsi_prop_protocol_schedule_stats_s {
  uint16_t reserved;
  uint16_t activities_blocked;
  uint16_t activities_aborted;
  uint16_t schedules_blocked;
  uint16_t radio_ops_complete;
  uint16_t schedules_attempted;
} rsi_prop_protocol_schedule_stats_t;

//PROP_PROTOCOL activity stats
typedef struct rsi_prop_protocol_activity_stats_s {
  uint16_t reserved;
  uint16_t activity_status_ok;
  uint16_t activities_aborted;
  uint16_t activity_status_too_late;
  uint16_t activity_status_no_sync;
  uint16_t activity_status_incomplete;
  uint16_t activity_status_bad_crc;
  uint16_t activity_status_vnd_error_start;
} rsi_prop_protocol_activity_stats_t;

/*==============================================*/
/**
 * @fn         rsi_prop_protocol_resp_handler_register_callbacks
 * @brief      post the prop_protocol rx async events to the application 
 * @param[in]  pointer to the data packet start address
 *  * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function registers the function pointers for GATT responses
 */
//PROP_PROTOCOL get stats cmd structure
typedef struct rsi_prop_protocol_get_stats_s {
  uint8_t cmd_ix;
} rsi_prop_protocol_get_stats_t;

typedef void (*rsi_prop_protocol_resp_handler_t)(uint8_t *data);
typedef void (*rsi_prop_protocol_data_request_callback_t)(void);

void rsi_prop_protocol_register_callbacks(
  rsi_prop_protocol_resp_handler_t prop_protocol_async_resp_handler,
  rsi_prop_protocol_data_request_callback_t rsi_prop_protocol_data_request_callback);

// Driver PROP_PROTOCOL control block
typedef struct rsi_prop_protocol_cb_s {
  //PROP_PROTOCOL CBFC callbacks
  rsi_prop_protocol_resp_handler_t prop_protocol_async_resp_handler;
  rsi_prop_protocol_data_request_callback_t rsi_prop_protocol_data_request_callback;
} rsi_prop_protocol_cb_t;

int32_t rsi_prop_protocol_driver_send_cmd(uint16_t cmd, void *cmd_struct, void *resp);
void rsi_prop_protocol_common_tx_done(rsi_pkt_t *pkt);
uint32_t rsi_prop_protocol_app(void);
void rsi_prop_protocol_app_task(void);
uint16_t rsi_bt_prepare_prop_protocol_pkt(uint16_t cmd_type, void *cmd_struct, rsi_pkt_t *pkt);
int32_t rsi_prop_protocol_driver_send_cmd(uint16_t cmd, void *cmd_struct, void *resp);
void rsi_prop_protocol_tx_done(rsi_bt_cb_t *prop_protocol_cb, rsi_pkt_t *pkt);
void rsi_prop_protocol_callbacks_handler(rsi_bt_cb_t *prop_protocol_cb,
                                         uint16_t rsp_type,
                                         uint8_t *payload,
                                         uint16_t payload_length);
int32_t rsi_driver_process_prop_protocol_resp(
  rsi_bt_cb_t *prop_protocol_cb,
  rsi_pkt_t *pkt,
  void (*rsi_bt_async_callback_handler)(rsi_bt_cb_t *cb, uint16_t type, uint8_t *data, uint16_t length));
void rsi_bt_clear_wait_bitmap(uint16_t protocol_type, uint8_t sem_type);
void rsi_bt_set_wait_bitmap(uint16_t protocol_type, uint8_t sem_type);

// PROP_PROTOCOL protocol more data request threshold.
#define PROP_PROTOCOL_MORE_DATA_REQUEST_THRESHOLD 0x02
#define RSI_OPERMODE_PROP_PROTOCOL_GARDEN         0x109

#endif
