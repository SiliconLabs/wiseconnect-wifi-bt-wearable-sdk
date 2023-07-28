/*******************************************************************************
* @file  rsi_common_app_DEMO_69.c
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
/**
 * @file    rsi_common_app_DEMO_69.c
 * @version 0.1
 * @date    01 Feb 2020
 *
 *
 *  @section Licenseremote_name
 *  This program should be used on your own responsibility.
 *  Silicon Labs assumes no responsibility for any losses
 *  incurred by customers or third parties arising from the use of this file.
 *
 *  @brief : This file contains example application for device initialization
 *
 *  @section Description  This application initiates Silicon Labs device and create tasks.
 *
 */

/*=======================================================================*/
//   ! INCLUDES
/*=======================================================================*/

#include <rsi_common_app.h>
#if COEX_MAX_APP_BLE_2MAS_8SLAV
#include "rsi_bt_common_apis.h"
#include <stdio.h>
#include <string.h>
#include <rsi_ble.h>
#include "rsi_driver.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_config.h"
#include "rsi_common_app_DEMO_69.h"

/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/

/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/
//! flag to check bt power save
#if !RUN_TIME_CONFIG_ENABLE
rsi_parsed_conf_t rsi_parsed_conf = { 0 };
#endif
rsi_semaphore_handle_t ble_main_task_sem, ble_slave_conn_sem, bt_app_sem, prop_protocol_app_sem, wlan_app_sem,
  bt_inquiry_sem, ble_scan_sem;
#if WLAN_SYNC_REQ
rsi_semaphore_handle_t sync_coex_ble_sem, sync_coex_prop_protocol_sem, sync_coex_bt_sem;
bool other_protocol_activity_enabled;
#endif
#if SOCKET_ASYNC_FEATURE
rsi_semaphore_handle_t sock_wait_sem;
#endif
#if (RSI_TCP_IP_BYPASS && SOCKET_ASYNC_FEATURE)
rsi_semaphore_handle_t lwip_sock_async_sem;
#endif
rsi_task_handle_t ble_main_app_task_handle, bt_app_task_handle, prop_protocol_app_task_handle, wlan_app_task_handle,
  wlan_task_handle;
rsi_task_handle_t window_reset_notify_task_handle;
bool rsi_ble_running, rsi_bt_running, rsi_prop_protocol_running, rsi_wlan_running, wlan_radio_initialized,
  powersave_cmd_given;
rsi_mutex_handle_t power_cmd_mutex;
rsi_mutex_handle_t window_update_mutex;
bool rsi_window_update_sem_waiting;
rsi_semaphore_handle_t window_reset_notify_sem;
#if (SSL_TX_DATA || SSL_RX_DATA || (RX_DATA && HTTPS_DOWNLOAD))
rsi_semaphore_handle_t cert_sem, conn_sem;
rsi_task_handle_t cert_bypass_task_handle[SOCKTEST_INSTANCES_MAX];
#endif
/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/
extern rsi_task_handle_t common_task_handle;
#if RUN_TIME_CONFIG_ENABLE
extern rsi_semaphore_handle_t common_task_sem;
extern rsi_parsed_conf_t rsi_parsed_conf;
#endif

/*========================================================================*/
//!  CALLBACK FUNCTIONS
/*=======================================================================*/

/*=======================================================================*/
//   ! EXTERN FUNCTIONS
/*=======================================================================*/

/*=======================================================================*/
//   ! PROCEDURES
/*=======================================================================*/
int32_t set_power_config(void)
{
  int32_t status = RSI_SUCCESS;

  status = rsi_bt_power_save_profile(RSI_ACTIVE, RSI_MAX_PSP);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Failed to keep in ACTIVE MODE\r\n");
    return status;
  }

  status = rsi_wlan_power_save_profile(RSI_ACTIVE, RSI_MAX_PSP);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Failed to keep in ACTIVE MODE\r\n");
    return status;
  }
  return status;
}

/*==============================================*/
/**
 * @fn         rsi_initiate_power_save
 * @brief      send power save command to RS9116 module
 *
 * @param[out] none
 * @return     status of commands, success-> 0, failure ->-1
 * @section description
 * This function sends command to keep module in power save
 */
int32_t rsi_initiate_power_save(void)
{
  int32_t status = RSI_SUCCESS;
  //! enable wlan radio
  if (!wlan_radio_initialized) {
    status = rsi_wlan_radio_init();
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\n radio init failed,error = %d\n", status);
      return status;
    } else {
      wlan_radio_initialized = true;
    }
  }
  status = rsi_bt_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Failed in initiating power save\r\n");
    return status;
  }
  //! initiating power save in wlan mode
  status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Failed in initiating power save\r\n");
    return status;
  }
  return status;
}

#if !RUN_TIME_CONFIG_ENABLE
/*==============================================*/
/**
 * @fn         rsi_ble_initialize_conn_buffer
 * @brief      this function initializes the configurations for each connection
 * @param[out] none
 * @param[out] none
 * @return     none
 * @section description
 */
int8_t rsi_ble_initialize_conn_buffer(rsi_ble_conn_config_t *ble_conn_spec_conf)
{
  int8_t status = RSI_SUCCESS;
  if (ble_conn_spec_conf != NULL) {
    if (RSI_BLE_MAX_NBR_SLAVES > 0) {
      //! Initialize slave1 configurations
      ble_conn_spec_conf[SLAVE1].smp_enable        = SMP_ENABLE_S1;
      ble_conn_spec_conf[SLAVE1].add_to_whitelist  = ADD_TO_WHITELIST_S1;
      ble_conn_spec_conf[SLAVE1].profile_discovery = PROFILE_QUERY_S1;
      ble_conn_spec_conf[SLAVE1].data_transfer     = DATA_TRANSFER_S1;
      //ble_conn_spec_conf[SLAVE1].bidir_datatransfer = SMP_ENABLE_S1;
      ble_conn_spec_conf[SLAVE1].rx_notifications                 = RX_NOTIFICATIONS_FROM_S1;
      ble_conn_spec_conf[SLAVE1].rx_indications                   = RX_INDICATIONS_FROM_S1;
      ble_conn_spec_conf[SLAVE1].tx_notifications                 = TX_NOTIFICATIONS_TO_S1;
      ble_conn_spec_conf[SLAVE1].tx_write                         = TX_WRITES_TO_S1;
      ble_conn_spec_conf[SLAVE1].tx_write_no_response             = TX_WRITES_NO_RESP_TO_S1;
      ble_conn_spec_conf[SLAVE1].tx_indications                   = TX_INDICATIONS_TO_S1;
      ble_conn_spec_conf[SLAVE1].conn_param_update.conn_int       = CONN_INTERVAL_S1;
      ble_conn_spec_conf[SLAVE1].conn_param_update.conn_latncy    = CONN_LATENCY_S1;
      ble_conn_spec_conf[SLAVE1].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_S1;
      ble_conn_spec_conf[SLAVE1].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_S1;
      ble_conn_spec_conf[SLAVE1].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_S1;
      ble_conn_spec_conf[SLAVE1].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_S1;
    }

    if (RSI_BLE_MAX_NBR_SLAVES > 1) {
      //! Initialize slave2 configurations
      ble_conn_spec_conf[SLAVE2].smp_enable        = SMP_ENABLE_S2;
      ble_conn_spec_conf[SLAVE2].add_to_whitelist  = ADD_TO_WHITELIST_S2;
      ble_conn_spec_conf[SLAVE2].profile_discovery = PROFILE_QUERY_S2;
      ble_conn_spec_conf[SLAVE2].data_transfer     = DATA_TRANSFER_S2;
      //ble_conn_spec_conf[SLAVE2].bidir_datatransfer = SMP_ENABLE_S2;
      ble_conn_spec_conf[SLAVE2].rx_notifications                 = RX_NOTIFICATIONS_FROM_S2;
      ble_conn_spec_conf[SLAVE2].rx_indications                   = RX_INDICATIONS_FROM_S2;
      ble_conn_spec_conf[SLAVE2].tx_notifications                 = TX_NOTIFICATIONS_TO_S2;
      ble_conn_spec_conf[SLAVE2].tx_write                         = TX_WRITES_TO_S2;
      ble_conn_spec_conf[SLAVE2].tx_write_no_response             = TX_WRITES_NO_RESP_TO_S2;
      ble_conn_spec_conf[SLAVE2].tx_indications                   = TX_INDICATIONS_TO_S2;
      ble_conn_spec_conf[SLAVE2].conn_param_update.conn_int       = CONN_INTERVAL_S2;
      ble_conn_spec_conf[SLAVE2].conn_param_update.conn_latncy    = CONN_LATENCY_S2;
      ble_conn_spec_conf[SLAVE2].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_S2;
      ble_conn_spec_conf[SLAVE2].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_S2;
      ble_conn_spec_conf[SLAVE2].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_S2;
      ble_conn_spec_conf[SLAVE2].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_S2;
    }

    if (RSI_BLE_MAX_NBR_SLAVES > 2) {
      //! Initialize SLAVE3 configurations
      ble_conn_spec_conf[SLAVE3].smp_enable        = SMP_ENABLE_S3;
      ble_conn_spec_conf[SLAVE3].add_to_whitelist  = ADD_TO_WHITELIST_S3;
      ble_conn_spec_conf[SLAVE3].profile_discovery = PROFILE_QUERY_S3;
      ble_conn_spec_conf[SLAVE3].data_transfer     = DATA_TRANSFER_S3;
      //ble_conn_spec_conf[SLAVE3].bidir_datatransfer = SMP_ENABLE_S3;
      ble_conn_spec_conf[SLAVE3].rx_notifications                 = RX_NOTIFICATIONS_FROM_S3;
      ble_conn_spec_conf[SLAVE3].rx_indications                   = RX_INDICATIONS_FROM_S3;
      ble_conn_spec_conf[SLAVE3].tx_notifications                 = TX_NOTIFICATIONS_TO_S3;
      ble_conn_spec_conf[SLAVE3].tx_write                         = TX_WRITES_TO_S3;
      ble_conn_spec_conf[SLAVE3].tx_write_no_response             = TX_WRITES_NO_RESP_TO_S3;
      ble_conn_spec_conf[SLAVE3].tx_indications                   = TX_INDICATIONS_TO_S3;
      ble_conn_spec_conf[SLAVE3].conn_param_update.conn_int       = CONN_INTERVAL_S3;
      ble_conn_spec_conf[SLAVE3].conn_param_update.conn_latncy    = CONN_LATENCY_S3;
      ble_conn_spec_conf[SLAVE3].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_S3;
      ble_conn_spec_conf[SLAVE3].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_S3;
      ble_conn_spec_conf[SLAVE3].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_S3;
      ble_conn_spec_conf[SLAVE3].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_S3;
    }

    if (RSI_BLE_MAX_NBR_SLAVES > 3) {
      //! Initialize SLAVE4 configurations
      ble_conn_spec_conf[SLAVE4].smp_enable        = SMP_ENABLE_S4;
      ble_conn_spec_conf[SLAVE4].add_to_whitelist  = ADD_TO_WHITELIST_S4;
      ble_conn_spec_conf[SLAVE4].profile_discovery = PROFILE_QUERY_S4;
      ble_conn_spec_conf[SLAVE4].data_transfer     = DATA_TRANSFER_S4;
      //ble_conn_spec_conf[SLAVE4].bidir_datatransfer = SMP_ENABLE_S4;
      ble_conn_spec_conf[SLAVE4].rx_notifications                 = RX_NOTIFICATIONS_FROM_S4;
      ble_conn_spec_conf[SLAVE4].rx_indications                   = RX_INDICATIONS_FROM_S4;
      ble_conn_spec_conf[SLAVE4].tx_notifications                 = TX_NOTIFICATIONS_TO_S4;
      ble_conn_spec_conf[SLAVE4].tx_write                         = TX_WRITES_TO_S4;
      ble_conn_spec_conf[SLAVE4].tx_write_no_response             = TX_WRITES_NO_RESP_TO_S4;
      ble_conn_spec_conf[SLAVE4].tx_indications                   = TX_INDICATIONS_TO_S4;
      ble_conn_spec_conf[SLAVE4].conn_param_update.conn_int       = CONN_INTERVAL_S4;
      ble_conn_spec_conf[SLAVE4].conn_param_update.conn_latncy    = CONN_LATENCY_S4;
      ble_conn_spec_conf[SLAVE4].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_S4;
      ble_conn_spec_conf[SLAVE4].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_S4;
      ble_conn_spec_conf[SLAVE4].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_S4;
      ble_conn_spec_conf[SLAVE4].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_S4;
    }

    if (RSI_BLE_MAX_NBR_SLAVES > 4) {
      //! Initialize SLAVE5 configurations
      ble_conn_spec_conf[SLAVE5].smp_enable        = SMP_ENABLE_S5;
      ble_conn_spec_conf[SLAVE5].add_to_whitelist  = ADD_TO_WHITELIST_S5;
      ble_conn_spec_conf[SLAVE5].profile_discovery = PROFILE_QUERY_S5;
      ble_conn_spec_conf[SLAVE5].data_transfer     = DATA_TRANSFER_S5;
      //ble_conn_spec_conf[SLAVE5].bidir_datatransfer = SMP_ENABLE_S5;
      ble_conn_spec_conf[SLAVE5].rx_notifications                 = RX_NOTIFICATIONS_FROM_S5;
      ble_conn_spec_conf[SLAVE5].rx_indications                   = RX_INDICATIONS_FROM_S5;
      ble_conn_spec_conf[SLAVE5].tx_notifications                 = TX_NOTIFICATIONS_TO_S5;
      ble_conn_spec_conf[SLAVE5].tx_write                         = TX_WRITES_TO_S5;
      ble_conn_spec_conf[SLAVE5].tx_write_no_response             = TX_WRITES_NO_RESP_TO_S5;
      ble_conn_spec_conf[SLAVE5].tx_indications                   = TX_INDICATIONS_TO_S5;
      ble_conn_spec_conf[SLAVE5].conn_param_update.conn_int       = CONN_INTERVAL_S5;
      ble_conn_spec_conf[SLAVE5].conn_param_update.conn_latncy    = CONN_LATENCY_S5;
      ble_conn_spec_conf[SLAVE5].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_S5;
      ble_conn_spec_conf[SLAVE5].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_S5;
      ble_conn_spec_conf[SLAVE5].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_S5;
      ble_conn_spec_conf[SLAVE5].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_S5;
    }
    if (RSI_BLE_MAX_NBR_SLAVES > 5) {
      //! Initialize SLAVE6 configurations
      ble_conn_spec_conf[SLAVE6].smp_enable        = SMP_ENABLE_S6;
      ble_conn_spec_conf[SLAVE6].add_to_whitelist  = ADD_TO_WHITELIST_S6;
      ble_conn_spec_conf[SLAVE6].profile_discovery = PROFILE_QUERY_S6;
      ble_conn_spec_conf[SLAVE6].data_transfer     = DATA_TRANSFER_S6;
      //ble_conn_spec_conf[SLAVE6].bidir_datatransfer = SMP_ENABLE_S6;
      ble_conn_spec_conf[SLAVE6].rx_notifications                 = RX_NOTIFICATIONS_FROM_S6;
      ble_conn_spec_conf[SLAVE6].rx_indications                   = RX_INDICATIONS_FROM_S6;
      ble_conn_spec_conf[SLAVE6].tx_notifications                 = TX_NOTIFICATIONS_TO_S6;
      ble_conn_spec_conf[SLAVE6].tx_write                         = TX_WRITES_TO_S6;
      ble_conn_spec_conf[SLAVE6].tx_write_no_response             = TX_WRITES_NO_RESP_TO_S6;
      ble_conn_spec_conf[SLAVE6].tx_indications                   = TX_INDICATIONS_TO_S6;
      ble_conn_spec_conf[SLAVE6].conn_param_update.conn_int       = CONN_INTERVAL_S6;
      ble_conn_spec_conf[SLAVE6].conn_param_update.conn_latncy    = CONN_LATENCY_S6;
      ble_conn_spec_conf[SLAVE6].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_S6;
      ble_conn_spec_conf[SLAVE6].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_S6;
      ble_conn_spec_conf[SLAVE6].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_S6;
      ble_conn_spec_conf[SLAVE6].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_S6;
    }

    if (RSI_BLE_MAX_NBR_SLAVES > 6) {
      //! Initialize SLAVE7 configurations
      ble_conn_spec_conf[SLAVE7].smp_enable        = SMP_ENABLE_S7;
      ble_conn_spec_conf[SLAVE7].add_to_whitelist  = ADD_TO_WHITELIST_S7;
      ble_conn_spec_conf[SLAVE7].profile_discovery = PROFILE_QUERY_S7;
      ble_conn_spec_conf[SLAVE7].data_transfer     = DATA_TRANSFER_S7;
      //ble_conn_spec_conf[SLAVE7].bidir_datatransfer = SMP_ENABLE_S7;
      ble_conn_spec_conf[SLAVE7].rx_notifications                 = RX_NOTIFICATIONS_FROM_S7;
      ble_conn_spec_conf[SLAVE7].rx_indications                   = RX_INDICATIONS_FROM_S7;
      ble_conn_spec_conf[SLAVE7].tx_notifications                 = TX_NOTIFICATIONS_TO_S7;
      ble_conn_spec_conf[SLAVE7].tx_write                         = TX_WRITES_TO_S7;
      ble_conn_spec_conf[SLAVE7].tx_write_no_response             = TX_WRITES_NO_RESP_TO_S7;
      ble_conn_spec_conf[SLAVE7].tx_indications                   = TX_INDICATIONS_TO_S7;
      ble_conn_spec_conf[SLAVE7].conn_param_update.conn_int       = CONN_INTERVAL_S7;
      ble_conn_spec_conf[SLAVE7].conn_param_update.conn_latncy    = CONN_LATENCY_S7;
      ble_conn_spec_conf[SLAVE7].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_S7;
      ble_conn_spec_conf[SLAVE7].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_S7;
      ble_conn_spec_conf[SLAVE7].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_S7;
      ble_conn_spec_conf[SLAVE7].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_S7;
    }
    if (RSI_BLE_MAX_NBR_SLAVES > 7) {
      //! Initialize SLAVE8 configurations
      ble_conn_spec_conf[SLAVE8].smp_enable        = SMP_ENABLE_S8;
      ble_conn_spec_conf[SLAVE8].add_to_whitelist  = ADD_TO_WHITELIST_S8;
      ble_conn_spec_conf[SLAVE8].profile_discovery = PROFILE_QUERY_S8;
      ble_conn_spec_conf[SLAVE8].data_transfer     = DATA_TRANSFER_S8;
      //ble_conn_spec_conf[SLAVE8].bidir_datatransfer = SMP_ENABLE_S8;
      ble_conn_spec_conf[SLAVE8].rx_notifications                 = RX_NOTIFICATIONS_FROM_S8;
      ble_conn_spec_conf[SLAVE8].rx_indications                   = RX_INDICATIONS_FROM_S8;
      ble_conn_spec_conf[SLAVE8].tx_notifications                 = TX_NOTIFICATIONS_TO_S8;
      ble_conn_spec_conf[SLAVE8].tx_write                         = TX_WRITES_TO_S8;
      ble_conn_spec_conf[SLAVE8].tx_write_no_response             = TX_WRITES_NO_RESP_TO_S8;
      ble_conn_spec_conf[SLAVE8].tx_indications                   = TX_INDICATIONS_TO_S8;
      ble_conn_spec_conf[SLAVE8].conn_param_update.conn_int       = CONN_INTERVAL_S8;
      ble_conn_spec_conf[SLAVE8].conn_param_update.conn_latncy    = CONN_LATENCY_S8;
      ble_conn_spec_conf[SLAVE8].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_S8;
      ble_conn_spec_conf[SLAVE8].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_S8;
      ble_conn_spec_conf[SLAVE8].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_S8;
      ble_conn_spec_conf[SLAVE8].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_S8;
    }

    if (RSI_BLE_MAX_NBR_MASTERS > 0) {
      //! Initialize master1 configurations
      ble_conn_spec_conf[MASTER1].smp_enable        = SMP_ENABLE_M1;
      ble_conn_spec_conf[MASTER1].add_to_whitelist  = ADD_TO_WHITELIST_M1;
      ble_conn_spec_conf[MASTER1].profile_discovery = PROFILE_QUERY_M1;
      ble_conn_spec_conf[MASTER1].data_transfer     = DATA_TRANSFER_M1;
      //ble_conn_spec_conf[MASTER1].bidir_datatransfer = SMP_ENABLE_M1;
      ble_conn_spec_conf[MASTER1].rx_notifications                 = RX_NOTIFICATIONS_FROM_M1;
      ble_conn_spec_conf[MASTER1].rx_indications                   = RX_INDICATIONS_FROM_M1;
      ble_conn_spec_conf[MASTER1].tx_notifications                 = TX_NOTIFICATIONS_TO_M1;
      ble_conn_spec_conf[MASTER1].tx_write                         = TX_WRITES_TO_M1;
      ble_conn_spec_conf[MASTER1].tx_write_no_response             = TX_WRITES_NO_RESP_TO_M1;
      ble_conn_spec_conf[MASTER1].tx_indications                   = TX_INDICATIONS_TO_M1;
      ble_conn_spec_conf[MASTER1].conn_param_update.conn_int       = CONN_INTERVAL_M1;
      ble_conn_spec_conf[MASTER1].conn_param_update.conn_latncy    = CONN_LATENCY_M1;
      ble_conn_spec_conf[MASTER1].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_M1;
      ble_conn_spec_conf[MASTER1].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_M1;
      ble_conn_spec_conf[MASTER1].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_M1;
      ble_conn_spec_conf[MASTER1].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_M1;
    }

    if (RSI_BLE_MAX_NBR_MASTERS > 1) {
      //! Initialize master2 configurations
      ble_conn_spec_conf[MASTER2].smp_enable        = SMP_ENABLE_M2;
      ble_conn_spec_conf[MASTER2].add_to_whitelist  = ADD_TO_WHITELIST_M2;
      ble_conn_spec_conf[MASTER2].profile_discovery = PROFILE_QUERY_M2;
      ble_conn_spec_conf[MASTER2].data_transfer     = DATA_TRANSFER_M2;
      //ble_conn_spec_conf[MASTER2].bidir_datatransfer = SMP_ENABLE_M2;
      ble_conn_spec_conf[MASTER2].rx_notifications                 = RX_NOTIFICATIONS_FROM_M2;
      ble_conn_spec_conf[MASTER2].rx_indications                   = RX_INDICATIONS_FROM_M2;
      ble_conn_spec_conf[MASTER2].tx_notifications                 = TX_NOTIFICATIONS_TO_M2;
      ble_conn_spec_conf[MASTER2].tx_write                         = TX_WRITES_TO_M2;
      ble_conn_spec_conf[MASTER2].tx_write_no_response             = TX_WRITES_NO_RESP_TO_M2;
      ble_conn_spec_conf[MASTER2].tx_indications                   = TX_INDICATIONS_TO_M2;
      ble_conn_spec_conf[MASTER2].conn_param_update.conn_int       = CONN_INTERVAL_M2;
      ble_conn_spec_conf[MASTER2].conn_param_update.conn_latncy    = CONN_LATENCY_M2;
      ble_conn_spec_conf[MASTER2].conn_param_update.supervision_to = CONN_SUPERVISION_TIMEOUT_M2;
      ble_conn_spec_conf[MASTER2].buff_mode_sel.buffer_mode        = DLE_BUFFER_MODE_M2;
      ble_conn_spec_conf[MASTER2].buff_mode_sel.buffer_cnt         = DLE_BUFFER_COUNT_M2;
      ble_conn_spec_conf[MASTER2].buff_mode_sel.max_data_length    = RSI_BLE_MAX_DATA_LEN_M2;
    }

    /* Check the Total Number of Buffers allocated.*/
    if ((DLE_BUFFER_COUNT_S1 + DLE_BUFFER_COUNT_S2 + DLE_BUFFER_COUNT_S3 + DLE_BUFFER_COUNT_S4 + DLE_BUFFER_COUNT_S5
         + DLE_BUFFER_COUNT_S6 + DLE_BUFFER_COUNT_S7 + DLE_BUFFER_COUNT_S8 + DLE_BUFFER_COUNT_M1 + DLE_BUFFER_COUNT_M2)
        > RSI_BLE_NUM_CONN_EVENTS) {
      LOG_PRINT("\r\n Total number of per connection buffer count is more than the total number alllocated \r\n");
      status = RSI_FAILURE;
    }

  } else {
    LOG_PRINT("\r\n Invalid buffer passed \r\n");
    status = RSI_FAILURE;
  }
  return status;
}

/*==============================================*/
/**
 * @fn         rsi_fill_user_config
 * @brief      this function fills the compile time user inputs to local buffer
 * @param[out] none
 * @return     none.
 * @section description
 * this function fills the compile time userinputs to local buffer
 */
int8_t rsi_fill_user_config()
{
  int8_t status = RSI_SUCCESS;
  //! copy protocol selection macros
  rsi_parsed_conf.rsi_protocol_sel.is_ble_enabled           = RSI_ENABLE_BLE_TEST;
  rsi_parsed_conf.rsi_protocol_sel.is_bt_enabled            = RSI_ENABLE_BT_TEST;
  rsi_parsed_conf.rsi_protocol_sel.is_prop_protocol_enabled = RSI_ENABLE_PROP_PROTOCOL_TEST;
  rsi_parsed_conf.rsi_protocol_sel.is_wifi_enabled          = RSI_ENABLE_WIFI_TEST;

  //! copy ble connection specific configurations

  if ((RSI_BLE_MAX_NBR_MASTERS > 2) || (RSI_BLE_MAX_NBR_SLAVES > 8)) {
    LOG_PRINT("\r\n number of BLE MASTERS or BLE SLAVES Given wrong declaration\r\n");
    rsi_delay_ms(1000);
    return RSI_FAILURE;
  }

  if (rsi_parsed_conf.rsi_protocol_sel.is_ble_enabled) {
    status =
      rsi_ble_initialize_conn_buffer((rsi_ble_conn_config_t *)&rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config);
  }

  //! copy bt configurations
  if (rsi_parsed_conf.rsi_protocol_sel.is_bt_enabled) {
    rsi_parsed_conf.rsi_bt_config.rsi_bd_addr               = malloc(sizeof(uint8_t) * RSI_REM_DEV_ADDR_LEN);
    rsi_parsed_conf.rsi_bt_config.rsi_bd_addr               = RSI_BT_REMOTE_BD_ADDR;
    rsi_parsed_conf.rsi_bt_config.rsi_app_avdtp_role        = RSI_APP_AVDTP_ROLE;
    rsi_parsed_conf.rsi_bt_config.rsi_bt_avdtp_stats_enable = RSI_BT_AVDTP_STATS;
    rsi_parsed_conf.rsi_bt_config.rsi_ta_based_encoder      = TA_BASED_ENCODER;
    rsi_parsed_conf.rsi_bt_config.rsi_bt_inquiry_enable     = INQUIRY_ENABLE;
    rsi_parsed_conf.rsi_bt_config.rsi_inq_conn_simultaneous = INQUIRY_CONNECTION_SIMULTANEOUS;
  }
  return status;
}
#endif

/*==============================================*/
/**
 * @fn         rsi_common_app_task
 * @brief      This function creates the main tasks for selected protocol
 * @param[out] none
 * @return     none.
 * @section description
 * This function creates the main tasks for enabled protocols based on local buffer 'rsi_parsed_conf'
 */
void rsi_common_app_task(void)
{
  int8_t status                 = RSI_SUCCESS;
  rsi_ble_running               = false;
  rsi_bt_running                = false;
  rsi_prop_protocol_running     = false;
  rsi_wlan_running              = false;
  wlan_radio_initialized        = false;
  powersave_cmd_given           = false;
  ble_main_app_task_handle      = NULL;
  bt_app_task_handle            = NULL;
  prop_protocol_app_task_handle = NULL;
  wlan_app_task_handle          = NULL;
  rsi_window_update_sem_waiting = false;
  uint8_t own_bd_addr[6]        = { 0x67, 0x55, 0x44, 0x33, 0x22, 0x11 };

  while (1) {
#if RUN_TIME_CONFIG_ENABLE
    //! wait untill completion of parsing input file
    rsi_semaphore_wait(&common_task_sem, 0);
#endif

    //! Silicon Labs module initialization
    status = rsi_device_init(LOAD_NWP_FW);
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\r\n device init failed \n");
      return;
    }
#if !RUN_TIME_CONFIG_ENABLE
    //! fill the configurations in local structure based on compilation macros
    status = rsi_fill_user_config();
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\r\n failed to fill the configurations in local buffer \r\n");
      return;
    }
#endif
    //! WiSeConnect initialization
    status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\r\n wireless init failed \n");
      return;
    }

    status = rsi_bt_set_bd_addr(own_bd_addr);
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\nSET BD ADDR FAILED : 0x%x\n", status);
    } else {
      LOG_PRINT("\nSET BD ADDR SUCCESS : 0x%x\n", status);
    }

    //! Send Feature frame
    status = rsi_send_feature_frame();
    if (status != RSI_SUCCESS) {
      return;
    }
#if CONFIGURE_TIMEOUT
    status = rsi_config_timeout(KEEP_ALIVE_TIMEOUT, 60);
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\n Timeout Configuration Failed : 0x%x \n", status);
      return status;
    } else {
      LOG_PRINT("\nTimeout Configuration SUCCESS");
    }
#endif

    //! initialize wlan radio
    status = rsi_wlan_radio_init();
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\n WLAN radio init failed \n");
      return;
    } else {
      wlan_radio_initialized = true;
    }
    LOG_PRINT("\n Basic Init Done i.e deviceinit=>wirelessinit=>feature=>wlanradioinit \n");

#if ENABLE_POWER_SAVE
    //! create mutex
    status = rsi_mutex_create(&power_cmd_mutex);
    if (status != RSI_ERROR_NONE) {
      LOG_PRINT("failed to create mutex object, error = %d \r\n", status);
      return;
    }
#endif

    if (rsi_parsed_conf.rsi_protocol_sel.is_wifi_enabled) {
      rsi_wlan_running = true;                //! Making sure wlan got triggered.
      rsi_semaphore_create(&wlan_app_sem, 0); //! This lock will be used from one download complete notification.
#if (SSL_TX_DATA || SSL_RX_DATA || (RX_DATA && HTTPS_DOWNLOAD))
      rsi_semaphore_create(&cert_sem, 0);
      rsi_semaphore_create(&conn_sem, 0);
      rsi_semaphore_post(&conn_sem);
#endif
#if WLAN_SYNC_REQ
      rsi_semaphore_create(&sync_coex_bt_sem, 0); //! This lock will be used from wlan task to be done.
      rsi_semaphore_create(&sync_coex_prop_protocol_sem, 0);
      rsi_semaphore_create(&sync_coex_ble_sem, 0);
#endif
#if SOCKET_ASYNC_FEATURE
      rsi_semaphore_create(&sock_wait_sem, 0);
#endif
#if (RSI_TCP_IP_BYPASS && SOCKET_ASYNC_FEATURE)
      rsi_semaphore_create(&lwip_sock_async_sem, 0);
#endif
#if WLAN_STA_TX_CASE
      status = rsi_task_create((void *)rsi_app_task_wifi_tcp_tx_ps,
                               (uint8_t *)"wlan_task",
                               RSI_WLAN_TASK_STACK_SIZE,
                               NULL,
                               RSI_WLAN_APP_TASK_PRIORITY,
                               &wlan_task_handle);
      if (status != RSI_ERROR_NONE) {
        LOG_PRINT("\r\n rrsi_app_task_wifi_tcp_tx_ps failed to create \r\n");
        break;
      }
#else
      status = rsi_task_create((void *)rsi_wlan_app_task,
                               (uint8_t *)"wlan_task",
                               RSI_WLAN_APP_TASK_SIZE,
                               NULL,
                               RSI_WLAN_APP_TASK_PRIORITY,
                               &wlan_app_task_handle);
      if (status != RSI_ERROR_NONE) {
        LOG_PRINT("\r\n rsi_wlan_app_task failed to create \r\n");
        break;
      }
#endif
    }
    //! create ble main task if ble protocol is selected
    if (rsi_parsed_conf.rsi_protocol_sel.is_ble_enabled) {
      rsi_ble_running = 1;
      rsi_semaphore_create(&ble_main_task_sem, 0);
      rsi_semaphore_create(&ble_scan_sem, 0);
      if (RSI_BLE_MAX_NBR_SLAVES > 0) {
        rsi_semaphore_create(&ble_slave_conn_sem, 0);
      }
      status = rsi_task_create((void *)rsi_ble_main_app_task,
                               (uint8_t *)"ble_main_task",
                               RSI_BLE_APP_MAIN_TASK_SIZE,
                               NULL,
                               RSI_BLE_MAIN_TASK_PRIORITY,
                               &ble_main_app_task_handle);
      if (status != RSI_ERROR_NONE) {
        LOG_PRINT("\r\n ble main task failed to create \r\n");
        return;
      }
    }

    //! create bt task if bt protocol is selected
    if (rsi_parsed_conf.rsi_protocol_sel.is_bt_enabled) {
      rsi_bt_running = 1;
      rsi_semaphore_create(&bt_app_sem, 0);
      rsi_semaphore_create(&bt_inquiry_sem, 0);
      status = rsi_task_create((void *)rsi_bt_app_task,
                               (uint8_t *)"bt_task",
                               RSI_BT_APP_TASK_SIZE + RSI_SBC_APP_ENCODE_SIZE,
                               NULL,
                               RSI_BT_APP_TASK_PRIORITY,
                               &bt_app_task_handle);
      if (status != RSI_ERROR_NONE) {
        LOG_PRINT("\r\n rsi_bt_app_task failed to create \r\n");
        return;
      }
    }

    //! create prop_protocol task if prop_protocol protocol is selected
    if (rsi_parsed_conf.rsi_protocol_sel.is_prop_protocol_enabled) {
      rsi_prop_protocol_running = 1;
      rsi_semaphore_create(&prop_protocol_app_sem, 0);
      status = rsi_task_create((void *)rsi_prop_protocol_app_task,
                               (uint8_t *)"prop_protocol_task",
                               RSI_PROP_PROTOCOL_APP_TASK_SIZE,
                               NULL,
                               RSI_PROP_PROTOCOL_APP_TASK_PRIORITY,
                               &prop_protocol_app_task_handle);
      if (status != RSI_ERROR_NONE) {
        LOG_PRINT("\r\n rsi_prop_protocol_app_task failed to create \r\n");
        return;
      }
    }

    //! delete the task as initialization is completed
    rsi_task_destroy(common_task_handle);
  }
}
#endif
