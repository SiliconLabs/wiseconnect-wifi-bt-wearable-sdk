/*******************************************************************************
* @file  rsi_common_config.h
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
 * @file         rsi_common_config.h
 * @version      0.1
 * @date         01 Feb 2020*
 *
 *
 *  @brief : This file contains user configurable details to configure the device
 *
 *  @section Description  This file contains user configurable details to configure the device
 */

#ifndef RSI_APP_COMMON_CONFIG_H
#define RSI_APP_COMMON_CONFIG_H
#include <stdio.h>
#include "rsi_ble_config.h"

/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/

//! Enable/Disable individual protocol activities
#define RSI_ENABLE_BLE_TEST           1 //! enable to test BLE functionality
#define RSI_ENABLE_BT_TEST            1 //! enable to test BT functionality
#define RSI_ENABLE_WLAN_TEST          1 //! enable to test WLAN functionality
#define RSI_ENABLE_PROP_PROTOCOL_TEST 0 //! enable to test PROP_PROTOCOL functionality

//! configure below macro for measuring WLAN throughput
#if RSI_ENABLE_WLAN_TEST
#define WLAN_THROUGHPUT_TEST 1 //! 1- TO Measure SSL/TCP/UDP tx/rx throughputs, 0- HTTP/s download
#endif

//! configure below macro for measuring BLE throughput
#if RSI_ENABLE_BLE_TEST
#define BLE_THROUGHPUT_TEST 0 //! 1- To Measure BLE throughput using sniffer, 0- default BLE operation
#endif

#if RSI_ENABLE_BT_TEST
#define BT_THROUGHPUT_TEST 0 //! 1- To Test BT throughput using sniffer, 0- default BT A2DP operation
#endif

//! BT controls
#if (RSI_ENABLE_BT_TEST && (WLAN_THROUGHPUT_TEST || BLE_THROUGHPUT_TEST))
#define BT_A2DP_INIT_DONE 0 //! make it 1 for BT init only
#define BT_A2DP_CONNECTED 0 //! make it 1 for BT A2DP connection only
#define BT_A2DP_START     1 //! make it 1 for BT A2DP streaming
#endif

//! BLE controls
#if (RSI_ENABLE_BLE_TEST && (WLAN_THROUGHPUT_TEST || BT_THROUGHPUT_TEST))
#define BLE_INIT_DONE           0 //! make it 1 for BLE init only
#define BLE_CONNECTED           0 //! make it 1 for BLE connection only
#define BLE_DATA_TRANSFER_START 1 //! make it 1 for BLE data transfer
#endif

//! WLAN controls
#if (RSI_ENABLE_WLAN_TEST && (BT_THROUGHPUT_TEST || BLE_THROUGHPUT_TEST))
#define WLAN_SCAN_ONLY    0 //! make it 1 for wlan scan only
#define WLAN_CONNECT_ONLY 0 //! make it 1 for wlan connection only
#define WLAN_DATATRANSFER 1 //! make it 1 for wlan http download
#endif

//! PROP_PROTOCOL controls
#define PROP_PROTOCOL_INIT_DONE           0
#define PROP_PROTOCOL_DATA_TRANSFER_START 1

#define WLAN_SYNC_REQ \
  1 //! Enable this macro to start bt and ble activites after wlan connection, else all activities starts concurrently

//! Application Task priorities
#define RSI_BLE_MAIN_TASK_PRIORITY          1
#define RSI_BT_APP_TASK_PRIORITY            2
#define RSI_HTTP_SOCKET_TASK_PRIORITY       1
#define RSI_BLE_APP_TASK_PRIORITY           1
#define RSI_PROP_PROTOCOL_APP_TASK_PRIORITY 1
#define RSI_WLAN_APP_TASK_PRIORITY          1
#define RSI_WLAN_THROUGHPUT_TASK_PRIORITY   1

//! Application Task sizes
#define RSI_BLE_APP_MAIN_TASK_SIZE       (512 * 2)
#define RSI_BLE_APP_TASK_SIZE            (512 * 4)
#define RSI_BT_APP_TASK_SIZE             (512 * 2)
#define RSI_SBC_APP_ENCODE_SIZE          (512 * 2)
#define RSI_PROP_PROTOCOL_APP_TASK_SIZE  (512 * 2)
#define RSI_WLAN_APP_TASK_SIZE           (512 * 2)
#define RSI_HTTP_SOCKET_TASK_STACK_SIZE  (512 * 2)
#define RSI_WLAN_THRGPUT_TASK_STACK_SIZE (512 * 2)

/*=======================================================================*/
//   ! Data type declarations
/*=======================================================================*/
typedef struct rsi_parsed_conf_s {
#if RSI_ENABLE_BLE_TEST
  struct rsi_ble_config_t {
    rsi_ble_conn_config_t rsi_ble_conn_config[TOTAL_CONNECTIONS];
  } rsi_ble_config;
#endif
#if RSI_ENABLE_BT_TEST
  //! To be defined
  struct rsi_bt_config_t {
    uint8_t *rsi_bd_addr;
    uint8_t rsi_app_avdtp_role;
    bool rsi_bt_avdtp_stats_enable;
    bool rsi_ta_based_encoder;
    bool rsi_bt_inquiry_enable;
    bool rsi_inq_rem_name_req;
    bool rsi_inq_conn_simultaneous;
  } rsi_bt_config;
#endif
#if RSI_ENABLE_WLAN_TEST
  bool dummy; //! added to resolve compilation error in Keil IDE
#endif
} rsi_parsed_conf_t;

/*=======================================================================*/
//   ! Function prototype declarations
/*=======================================================================*/
int32_t rsi_initiate_power_save(void);
int32_t set_power_config(void);
int32_t rsi_wlan_app_task(void);
int32_t rsi_bt_spp_task(void);
int32_t rsi_bt_a2dp_task(void);
int32_t wlan_throughput_task();
void rsi_ble_main_app_task(void);
void rsi_ble_task_on_conn(void *parameters);
void rsi_ble_slave_app_task(void);
void rsi_prop_protocol_app_task(void);
void rsi_ui_app_task(void);
void rsi_common_app_task(void);
#endif
