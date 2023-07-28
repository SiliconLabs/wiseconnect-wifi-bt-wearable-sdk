/*******************************************************************************
* @file  main.c
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
 * @file    main.c
 * @version 0.1
 * @date    8 May  2019
 *
 *
 *
 *  @brief : This file contains driver, module initialization and application execution
 *
 *  @section Description  This file contains driver, module initializations
 *
 *
 */

#include "rsi_common_app.h"
#include "rsi_driver.h"
#include "rsi_wlan.h"
#ifdef RSI_WITH_OS
//! OS include file to refer OS specific functionality
#include "rsi_os.h"

//! Wlan task priority
#if (BT_A2DP_SOURCE_WIFI_HTTP_S_RX || BT_A2DP_SOURCE_WIFI_HTTP_S_RX_DYN_COEX || WIFI_SOCKET_SELECT \
     || WIFI_HTTP_SOCKET_SELECT)
#define RSI_WLAN_TASK_PRIORITY 3
#else
#define RSI_WLAN_TASK_PRIORITY 1
#endif
#define RSI_DNS_TASK_PRIORITY 1

//! Wireless driver task priority
#if ((UNIFIED_PROTOCOL && (MULTITHREADED_HTTP_DOWNLOAD_TEST || MULTITHREADED_TCP_TX_TEST)))
#define RSI_DRIVER_TASK_PRIORITY 1
#elif (BLE_MULTI_SLAVE_MASTER || BLE_OVERLAPPING_PROCEDURES || UNIFIED_PROTOCOL || WIFI_SOCKET_SELECT \
       || WIFI_HTTP_SOCKET_SELECT)
#define RSI_DRIVER_TASK_PRIORITY 3
#elif (COEX_MAX_APP || COEX_THROUGHPUT || COEX_MAX_APP_BLE_2MAS_8SLAV)
#define RSI_DRIVER_TASK_PRIORITY 4
#else
#define RSI_DRIVER_TASK_PRIORITY 2
#endif

//! Wlan task stack size
#define RSI_WLAN_TASK_STACK_SIZE 1024
#define RSI_DNS_TASK_STACK_SIZE  512

#if RSI_ENABLE_PROP_PROTOCOL_TEST
#define RSI_PROP_PROTOCOL_TASK_STACK_SIZE 1024
#define RSI_PROP_PROTOCOL_TASK_PRIORITY   1
rsi_task_handle_t prop_protocol_task_handle = NULL;
rsi_semaphore_handle_t prop_protocol_sem;
#endif

#define RSI_SOCKET1_TASK_STACK_SIZE 768

#define RSI_SOCKET2_TASK_STACK_SIZE 768

//! Wireless driver task stack size
#define RSI_DRIVER_TASK_STACK_SIZE 1024

#define RSI_BT_TASK_STACK_SIZE 1024

#define RSI_SBC_ENCODE_STACK_SIZE 1024

#define RSI_PROP_PROTOCOL_TASK_STACK_SIZE 1024

#define RSI_BT_TASK_PRIORITY 1

#define RSI_SOCKET1_TASK_PRIORITY 2
#define RSI_SOCKET2_TASK_PRIORITY 2

#define RSI_PROP_PROTOCOL_TASK_PRIORITY 1

#define RSI_BLE_TASK_STACK_SIZE 1024

//! BT task priority
#if BLE_ADV_BT_A2DP_SOURCE
#define RSI_BLE_TASK_PRIORITY 3
#else
#define RSI_BLE_TASK_PRIORITY 1
#endif
#if (COEX_MAX_APP || COEX_MAX_APP_BLE_2MAS_8SLAV)
#define RSI_UI_TASK_STACK_SIZE 512 * 4
rsi_task_handle_t ui_app_task_handle = NULL;
#else
#define RSI_UI_TASK_STACK_SIZE 512
#endif

#define RSI_UI_TASK_PRIORITY 3

#if (COEX_MAX_APP || UNIFIED_PROTOCOL || COEX_THROUGHPUT || COEX_MAX_APP_BLE_2MAS_8SLAV)
rsi_task_handle_t common_task_handle = NULL;
#define RSI_COMMON_TASK_PRIORITY   1
#define RSI_COMMON_TASK_STACK_SIZE (512 * 2)
#endif
#if (COEX_MAX_APP || COEX_MAX_APP_BLE_2MAS_8SLAV)
rsi_semaphore_handle_t common_task_sem;
#endif

#if (BLE_MULTI_SLAVE_MASTER || BLE_MULTI_SLAVE_MASTER_BT_A2DP || BLE_MULTI_SLAVE_MASTER_BT_A2DP_WIFI_HTTP_S_RX \
     || BLE_OVERLAPPING_PROCEDURES || (PROP_PROTOCOL_BLE_DUAL_ROLE_BT_A2DP_SRC_APP))
#define RSI_BLE_COMMON_TASK_PRIORITY  2
#define RSI_BLE_MASTER1_TASK_PRIORITY 1
#define RSI_BLE_MASTER2_TASK_PRIORITY 1
#define RSI_BLE_SLAVE1_TASK_PRIORITY  1
#define RSI_BLE_SLAVE2_TASK_PRIORITY  1
#define RSI_BLE_SLAVE3_TASK_PRIORITY  1
#define RSI_BLE_COMMON_TASK_SIZE      (512 * 2)
#define RSI_BLE_MASTER1_TASK_SIZE     (512 * 2)
#define RSI_BLE_MASTER2_TASK_SIZE     (512 * 2)
#define RSI_BLE_SLAVE1_TASK_SIZE      (512 * 2)
#define RSI_BLE_SLAVE2_TASK_SIZE      (512 * 2)
#define RSI_BLE_SLAVE3_TASK_SIZE      (512 * 2)

rsi_semaphore_handle_t ble_main_task_sem, ble_slave1_sem, ble_slave2_sem, ble_slave3_sem, ble_master1_sem,
  ble_master2_sem;
rsi_task_handle_t ble_main_task_handle    = NULL;
rsi_task_handle_t ble_master1_task_handle = NULL;
rsi_task_handle_t ble_master2_task_handle = NULL;
rsi_task_handle_t ble_slave1_task_handle  = NULL;
rsi_task_handle_t ble_slave2_task_handle  = NULL;
rsi_task_handle_t ble_slave3_task_handle  = NULL;
#endif

#if (UNIFIED_PROTOCOL && RSI_ENABLE_BLE_TEST)
rsi_semaphore_handle_t ble_main_task_sem, ble_slave1_sem, ble_slave2_sem, ble_slave3_sem, ble_master1_sem,
  ble_master2_sem;
#endif
rsi_semaphore_handle_t prop_protocol_coex_sem;
rsi_semaphore_handle_t prop_protocol_sem;
rsi_semaphore_handle_t coex_sem;
rsi_semaphore_handle_t coex_sem1;
rsi_semaphore_handle_t socket_wait_sem1;
rsi_semaphore_handle_t socket_wait_sem2;

rsi_semaphore_handle_t event_sem;

rsi_semaphore_handle_t suspend_sem;

rsi_semaphore_handle_t wifi_task_sem;

rsi_semaphore_handle_t ui_task_sem;

rsi_semaphore_handle_t bt_sem;

rsi_task_handle_t ui_task_handle = NULL;
#endif

#define GLOBAL_BUFF_LEN 48000 //16900

//! Flag for infinite loop
#define RSI_FOREVER 1

//! Memory to initialize driver
uint8_t global_buf[GLOBAL_BUFF_LEN] = { 0 };
uint8_t powersave_d = 0, ble_powersave_d = 0, wifi_powersave_d = 0;
uint8_t tx_rx_Completed = 0;
uint8_t loopback_done   = 0;
#if (BLE_DUAL_MODE_BT_A2DP_SOURCE_WIFI_HTTP_S_RX || BLE_DUAL_MODE_BT_SPP_SLAVE)
extern uint16_t num_of_conn_slaves;
extern uint8_t num_of_conn_masters;
#endif

#ifdef RSI_WITH_OS
rsi_task_handle_t driver_task_handle = NULL;
#endif
#if (BT_A2DP_SOURCE || BT_A2DP_SOURCE_SBC_CODEC || BT_A2DP_SOURCE_WIFI_HTTP_S_RX \
     || BT_A2DP_SOURCE_WIFI_HTTP_S_RX_DYN_COEX || PROP_PROTOCOL_BLE_DUAL_ROLE_BT_A2DP_SRC_APP)
extern int32_t rsi_switch_proto(uint8_t type, void (*callback)(uint16_t mode, uint8_t *bt_disabled_status));
extern void switch_proto_async(uint16_t mode, uint8_t *bt_disabled_status);
#endif
int main(void)
{
#ifdef RSI_WITH_OS

  rsi_task_handle_t wlan_task_handle = NULL;

  rsi_task_handle_t dns_task_handle = NULL;

  rsi_task_handle_t wlan_get_task_handle = NULL;

  rsi_task_handle_t socket1_task_handle = NULL;

  rsi_task_handle_t socket2_task_handle = NULL;

  rsi_task_handle_t bt_task_handle = NULL;

  rsi_task_handle_t ble_task_handle = NULL;

  rsi_task_handle_t prop_protocol_task_handle = NULL;

  rsi_task_handle_t ui_task_handle = NULL;

  rsi_task_handle_t cert_task_handle = NULL;

  (void)wlan_task_handle;
  (void)dns_task_handle;
  (void)wlan_get_task_handle;
  (void)socket1_task_handle;
  (void)socket2_task_handle;
  (void)bt_task_handle;
  (void)ble_task_handle;
  (void)prop_protocol_task_handle;
  (void)driver_task_handle;
  (void)ui_task_handle;
  (void)cert_task_handle;
#endif

  int32_t status = RSI_SUCCESS;

  //! Board Initialization
  rsi_hal_board_init();

  //! Driver initialization
  status = rsi_driver_init(global_buf, GLOBAL_BUFF_LEN);
  if ((status < 0) || (status > GLOBAL_BUFF_LEN)) {
    return status;
  }

#ifndef RSI_WITH_OS
  //! Redpine module initialization
#if FIRMWARE_UPGRADE_VIA_BOOTLOADER
  status = rsi_device_init(BURN_NWP_FW);
#else
  status = rsi_device_init(LOAD_NWP_FW);
#endif
  if (status != RSI_SUCCESS) {
    return status;
  }

#if !(FIRMWARE_UPGRADE_VIA_BOOTLOADER)
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return status;
  }

  //! Send Feature frame
#if !(RAM_DUMP_ASSERTION_CASE)
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return status;
  }
#endif
#endif
#if (BT_A2DP_SOURCE || BT_A2DP_SOURCE_SBC_ENC_IN_RS9116)
  //! switch to BT protocol
  status = rsi_switch_proto(1, NULL);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n bt enable fail \n");
    //return status;
  }

  //! BT A2DP Initialization
  status = rsi_bt_a2dp_source();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("bt_a2dp init failed\n");
    return -1;
  }
#endif
#if (BT_A2DP_SOURCE_SBC_CODEC)
  //! switch to BT protocol
  status = rsi_switch_proto(1, NULL);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n bt enable fail \n");
    //return status;
  }
  //! BT A2DP Initialization
  status = rsi_bt_a2dp_source_sbc_codec();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("bt_a2dp init failed\n");
    return -1;
  }
#endif
#endif

#ifdef RSI_WITH_OS
  //! OS case
  //! Task created for WLAN task
#if WIFI_SSL_RX_THROUGHPUT
  rsi_task_create((void *)rsi_app_task_wifi_ssl_rx_tpt,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif MBR_BURN_ERASE
  rsi_task_create((void *)rsi_app_task_mbr_burn_erase,
                  (uint8_t *)"mbr_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif WIFI_TCP_BI_DIR
  rsi_task_create((void *)rsi_app_task_wifi_tcp_bi_dir,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif WIFI_SSL_TX_THROUGHPUT
  rsi_task_create((void *)rsi_app_task_wifi_ssl_tx_tpt,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif WIFI_TCP_RX_POWERSAVE
  rsi_task_create((void *)rsi_app_task_wifi_tcp_rx_ps,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif WIFI_CONNECTED_STANDBY
  rsi_task_create((void *)rsi_app_task_wifi_connected_standby,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif WIFI_STA_BGSCAN
  rsi_task_create((void *)rsi_app_task_wifi_sta_bgscan,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif WIFI_STA_CONN_DISCONN_SSL_CONN_DISCONN
  rsi_task_create((void *)rsi_app_task_wifi_sta_ssl_conn_disconn,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif WIFI_STA_SCAN_CONN_DISCONN
  rsi_task_create((void *)rsi_app_task_wifi_sta_scan_conn_disconn,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif WIFI_TCP_TX_POWERSAVE
  rsi_task_create((void *)rsi_app_task_wifi_tcp_tx_ps,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif WIFI_TCP_DNS_APP
  rsi_semaphore_create(&suspend_sem, 0);
  rsi_semaphore_create(&event_sem, 0);
  rsi_task_create((void *)rsi_app_task_wifi_tcp_tx,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
  rsi_task_create((void *)rsi_app_task_dns,
                  (uint8_t *)"dns_task",
                  RSI_DNS_TASK_STACK_SIZE,
                  NULL,
                  RSI_DNS_TASK_PRIORITY,
                  &dns_task_handle);
#elif WIFI_TCP_WLAN_GET_APP
  rsi_semaphore_create(&suspend_sem, 0);
  rsi_task_create((void *)rsi_app_task_wifi_tcp_tx,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
  rsi_task_create((void *)rsi_app_task_wlan_get,
                  (uint8_t *)"wlan_get_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_get_task_handle);
#elif WIFI_TCP_TX_APP
  rsi_task_create((void *)rsi_app_task_wifi_tcp_tx,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif WIFI_DEEPSLEEP_STANDBY
  rsi_task_create((void *)rsi_app_task_wifi_deepsleep_standby,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif FIRMWARE_UPGRADE
  rsi_task_create((void *)rsi_app_task_fw_update,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif FIRMWARE_UPGRADE_VIA_BOOTLOADER
  rsi_task_create((void *)rsi_app_task_fw_update_via_bootloader,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif RAM_DUMP_ASSERTION_CASE
  rsi_task_create((void *)rsi_app_task_ram_dump_assertion_case,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
#elif WIFI_SSL_SERVER_CERT_BYPASS
  rsi_task_create((void *)rsi_app_task_wifi_ssl_server_cert_bypass,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
  rsi_task_create((void *)rsi_app_task_send_certificates,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &cert_task_handle);
  rsi_semaphore_create(&suspend_sem, 0);
#elif WIFI_SOCKET_SELECT
  rsi_semaphore_create(&socket_wait_sem1, 0);
  rsi_semaphore_create(&socket_wait_sem2, 0);
  rsi_semaphore_create(&suspend_sem, 0);
  rsi_task_create((void *)rsi_app_task_wifi_socket_select,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
  rsi_task_create((void *)rsi_app_wlan_socket_create1,
                  (uint8_t *)"socket_task1",
                  RSI_SOCKET1_TASK_STACK_SIZE,
                  NULL,
                  RSI_SOCKET1_TASK_PRIORITY,
                  &socket1_task_handle);
  rsi_task_create((void *)rsi_app_wlan_socket_create2,
                  (uint8_t *)"socket_task2",
                  RSI_SOCKET2_TASK_STACK_SIZE,
                  NULL,
                  RSI_SOCKET2_TASK_PRIORITY,
                  &socket2_task_handle);
#elif WIFI_HTTP_SOCKET_SELECT
  rsi_semaphore_create(&socket_wait_sem1, 0);
  rsi_semaphore_create(&socket_wait_sem2, 0);
  rsi_semaphore_create(&suspend_sem, 0);
  rsi_task_create((void *)rsi_app_task_wifi_http_socket_select,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
  rsi_task_create((void *)rsi_app_wlan_socket_create1,
                  (uint8_t *)"socket_task1",
                  RSI_SOCKET1_TASK_STACK_SIZE,
                  NULL,
                  RSI_SOCKET1_TASK_PRIORITY,
                  &socket1_task_handle);
#elif BT_A2DP_SOURCE_WIFI_HTTP_S_RX
  rsi_semaphore_create(&coex_sem, 0);
  rsi_semaphore_create(&event_sem, 0);
  rsi_semaphore_create(&suspend_sem, 0);
  rsi_task_create((void *)rsi_wlan_app_task,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
  rsi_task_create((void *)rsi_bt_app_task,
                  (uint8_t *)"bt_task",
                  RSI_BT_TASK_STACK_SIZE,
                  NULL,
                  RSI_BT_TASK_PRIORITY,
                  &bt_task_handle);
#elif BT_A2DP_SOURCE_WIFI_HTTP_S_RX_DYN_COEX
  rsi_semaphore_create(&wifi_task_sem, 0);
  rsi_semaphore_create(&ui_task_sem, 0);
  rsi_semaphore_create(&coex_sem, 0);
  rsi_semaphore_create(&event_sem, 0);
  rsi_semaphore_create(&suspend_sem, 0);
  rsi_task_create((void *)rsi_ui_app_task,
                  (uint8_t *)"ui_task",
                  RSI_UI_TASK_STACK_SIZE,
                  NULL,
                  RSI_UI_TASK_PRIORITY,
                  &ui_task_handle);
  rsi_task_create((void *)rsi_wlan_app_task,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
  rsi_task_create((void *)rsi_bt_app_task,
                  (uint8_t *)"bt_task",
                  RSI_BT_TASK_STACK_SIZE,
                  NULL,
                  RSI_BT_TASK_PRIORITY,
                  &bt_task_handle);
#elif WIFI_HTTP_S_5MB_RX_WITH_STANDBY
  rsi_task_create((void *)rsi_app_task_wifi_https_rx_standby,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);

#elif WIFI_PROP_PROTOCOL_HTTP_S_5MB_RX_WITH_STANDBY
  rsi_semaphore_create(&bt_sem, 0);
  rsi_task_create((void *)rsi_app_task_wifi_prop_protocol_https_rx_standby,
                  (uint8_t *)"wlan_prop_protocol_task",
                  RSI_PROP_PROTOCOL_TASK_STACK_SIZE,
                  NULL,
                  RSI_PROP_PROTOCOL_TASK_PRIORITY,
                  &prop_protocol_task_handle);
  rsi_task_create((void *)rsi_prop_protocol_task,
                  (uint8_t *)"bt_prop_protocol_task",
                  RSI_BT_TASK_STACK_SIZE,
                  NULL,
                  RSI_BT_TASK_PRIORITY,
                  &bt_task_handle);

#elif (BT_A2DP_SOURCE || BT_A2DP_SOURCE_SBC_ENC_IN_RS9116)
  rsi_task_create((void *)rsi_app_task_bt_a2dp_source,
                  (uint8_t *)"bt_task",
                  RSI_BT_TASK_STACK_SIZE,
                  NULL,
                  RSI_BT_TASK_PRIORITY,
                  &bt_task_handle);
#elif BT_A2DP_SOURCE_SBC_CODEC
  rsi_task_create((void *)rsi_app_task_bt_a2dp_source_sbc_codec,
                  (uint8_t *)"bt_task",
                  RSI_BT_TASK_STACK_SIZE + RSI_SBC_ENCODE_STACK_SIZE,
                  NULL,
                  RSI_BT_TASK_PRIORITY,
                  &bt_task_handle);
#elif BLE_ADV_BT_SPP_THROUGHPUT
  //! Task created for BLE task
  rsi_task_create((void *)rsi_app_task_ble_adv_bt_spp_tpt,
                  (uint8_t *)"bt_task",
                  RSI_BT_TASK_STACK_SIZE,
                  NULL,
                  RSI_BT_TASK_PRIORITY,
                  &bt_task_handle);
#elif BLE_ADV_BT_A2DP_SOURCE
  rsi_semaphore_create(&coex_sem, 0);
  rsi_semaphore_create(&suspend_sem, 0);
  //! Task created for BLE task
  rsi_task_create((void *)rsi_bt_app_task,
                  (uint8_t *)"bt_task",
                  RSI_BT_TASK_STACK_SIZE,
                  NULL,
                  RSI_BT_TASK_PRIORITY,
                  &bt_task_handle);
  rsi_task_create((void *)rsi_ble_app_task,
                  (uint8_t *)"ble_task",
                  RSI_BLE_TASK_STACK_SIZE,
                  NULL,
                  RSI_BLE_TASK_PRIORITY,
                  &ble_task_handle);
#elif BT_EIR_SNIFF_SPP_SLAVE
  rsi_task_create((void *)rsi_app_task_eir_sniff_spp,
                  (uint8_t *)"bt_task",
                  RSI_BT_TASK_STACK_SIZE,
                  NULL,
                  RSI_BT_TASK_PRIORITY,
                  &bt_task_handle);
#elif BLE_PERIPHERAL
  rsi_task_create((void *)rsi_app_task_ble_peripheral,
                  (uint8_t *)"ble_task",
                  RSI_BLE_TASK_STACK_SIZE,
                  NULL,
                  RSI_BLE_TASK_PRIORITY,
                  &ble_task_handle);
#elif BLE_INDICATION
  rsi_task_create((void *)rsi_app_task_ble_peripheral,
                  (uint8_t *)"ble_task",
                  RSI_BLE_TASK_STACK_SIZE,
                  NULL,
                  RSI_BLE_TASK_PRIORITY,
                  &ble_task_handle);
#elif BLE_SIMPLE_GATT
  rsi_task_create((void *)rsi_app_task_ble_simple_gatt,
                  (uint8_t *)"ble_task",
                  RSI_BLE_TASK_STACK_SIZE,
                  NULL,
                  RSI_BLE_TASK_PRIORITY,
                  &ble_task_handle);
#elif BLE_CENTRAL
  rsi_task_create((void *)rsi_app_task_ble_central,
                  (uint8_t *)"ble_task",
                  RSI_BLE_TASK_STACK_SIZE,
                  NULL,
                  RSI_BLE_TASK_PRIORITY,
                  &ble_task_handle);
#elif BLE_DUAL_ROLE
  rsi_task_create((void *)rsi_app_task_ble_dual_role,
                  (uint8_t *)"ble_task",
                  RSI_BLE_TASK_STACK_SIZE,
                  NULL,
                  RSI_BLE_TASK_PRIORITY,
                  &ble_task_handle);
#elif BLE_POWERSAVE
  rsi_task_create((void *)rsi_app_task_ble_powersave,
                  (uint8_t *)"ble_task",
                  RSI_BLE_TASK_STACK_SIZE,
                  NULL,
                  RSI_BLE_TASK_PRIORITY,
                  &ble_task_handle);
#elif BLE_GATT_INDICATION_STATUS
  rsi_task_create((void *)rsi_app_task_ble_gatt_indication_status,
                  (uint8_t *)"ble_task",
                  RSI_BLE_TASK_STACK_SIZE,
                  NULL,
                  RSI_BLE_TASK_PRIORITY,
                  &ble_task_handle);
#elif BLE_PRIVACY
  rsi_task_create((void *)rsi_app_task_ble_privacy,
                  (uint8_t *)"ble_task",
                  RSI_BLE_TASK_STACK_SIZE,
                  NULL,
                  RSI_BLE_TASK_PRIORITY,
                  &ble_task_handle);
#elif BLE_MULTI_SLAVE_MASTER
  rsi_semaphore_create(&ble_main_task_sem, 0);
  rsi_semaphore_create(&ble_slave1_sem, 0);
  rsi_semaphore_create(&ble_slave2_sem, 0);
  rsi_semaphore_create(&ble_slave3_sem, 0);
  rsi_semaphore_create(&ble_master1_sem, 0);
  rsi_semaphore_create(&ble_master2_sem, 0);
  rsi_semaphore_create(&ui_task_sem, 0);
  rsi_task_create((void *)rsi_ble_main_app_task,
                  (uint8_t *)"ble_main_task",
                  RSI_BLE_COMMON_TASK_SIZE,
                  NULL,
                  RSI_BLE_COMMON_TASK_PRIORITY,
                  &ble_main_task_handle);
  rsi_task_create((void *)rsi_ble_master1_app_task,
                  (uint8_t *)"ble_master1_task",
                  RSI_BLE_MASTER1_TASK_SIZE,
                  NULL,
                  RSI_BLE_MASTER1_TASK_PRIORITY,
                  &ble_master1_task_handle);
  rsi_task_create((void *)rsi_ble_master2_app_task,
                  (uint8_t *)"ble_master2_task",
                  RSI_BLE_MASTER2_TASK_SIZE,
                  NULL,
                  RSI_BLE_MASTER2_TASK_PRIORITY,
                  &ble_master2_task_handle);
  rsi_task_create((void *)rsi_ble_slave1_app_task,
                  (uint8_t *)"ble_slave1_task",
                  RSI_BLE_SLAVE1_TASK_SIZE,
                  NULL,
                  RSI_BLE_SLAVE1_TASK_PRIORITY,
                  &ble_slave1_task_handle);
  rsi_task_create((void *)rsi_ble_slave2_app_task,
                  (uint8_t *)"ble_slave2_task",
                  RSI_BLE_SLAVE2_TASK_SIZE,
                  NULL,
                  RSI_BLE_SLAVE2_TASK_PRIORITY,
                  &ble_slave2_task_handle);
  rsi_task_create((void *)rsi_ble_slave3_app_task,
                  (uint8_t *)"ble_slave3_task",
                  RSI_BLE_SLAVE3_TASK_SIZE,
                  NULL,
                  RSI_BLE_SLAVE3_TASK_PRIORITY,
                  &ble_slave3_task_handle);
  rsi_task_create((void *)rsi_ui_app_task,
                  "ui_task",
                  RSI_UI_TASK_STACK_SIZE,
                  NULL,
                  RSI_UI_TASK_PRIORITY,
                  &ui_task_handle);
#elif BLE_OVERLAPPING_PROCEDURES
  rsi_semaphore_create(&ble_main_task_sem, 0);
  rsi_semaphore_create(&ble_slave1_sem, 0);
  rsi_semaphore_create(&ble_slave2_sem, 0);
  rsi_semaphore_create(&ble_slave3_sem, 0);
  rsi_semaphore_create(&ble_master1_sem, 0);
  rsi_semaphore_create(&ble_master2_sem, 0);
  rsi_task_create((void *)rsi_ble_main_app_task,
                  (uint8_t *)"ble_main_task",
                  RSI_BLE_COMMON_TASK_SIZE,
                  NULL,
                  RSI_BLE_COMMON_TASK_PRIORITY,
                  &ble_main_task_handle);
  rsi_task_create((void *)rsi_ble_master1_app_task,
                  (uint8_t *)"ble_master1_task",
                  RSI_BLE_MASTER1_TASK_SIZE,
                  NULL,
                  RSI_BLE_MASTER1_TASK_PRIORITY,
                  &ble_master1_task_handle);
  rsi_task_create((void *)rsi_ble_master2_app_task,
                  (uint8_t *)"ble_master2_task",
                  RSI_BLE_MASTER2_TASK_SIZE,
                  NULL,
                  RSI_BLE_MASTER2_TASK_PRIORITY,
                  &ble_master2_task_handle);
  rsi_task_create((void *)rsi_ble_slave1_app_task,
                  (uint8_t *)"ble_slave1_task",
                  RSI_BLE_SLAVE1_TASK_SIZE,
                  NULL,
                  RSI_BLE_SLAVE1_TASK_PRIORITY,
                  &ble_slave1_task_handle);
  rsi_task_create((void *)rsi_ble_slave2_app_task,
                  (uint8_t *)"ble_slave2_task",
                  RSI_BLE_SLAVE2_TASK_SIZE,
                  NULL,
                  RSI_BLE_SLAVE2_TASK_PRIORITY,
                  &ble_slave2_task_handle);
  rsi_task_create((void *)rsi_ble_slave3_app_task,
                  (uint8_t *)"ble_slave3_task",
                  RSI_BLE_SLAVE3_TASK_SIZE,
                  NULL,
                  RSI_BLE_SLAVE3_TASK_PRIORITY,
                  &ble_slave3_task_handle);
#elif BLE_MULTI_SLAVE_MASTER_BT_A2DP
  rsi_semaphore_create(&coex_sem, 0);
  rsi_semaphore_create(&ble_main_task_sem, 0);
  rsi_semaphore_create(&ble_slave1_sem, 0);
  rsi_semaphore_create(&ble_slave2_sem, 0);
  rsi_semaphore_create(&ble_slave3_sem, 0);
  rsi_semaphore_create(&ble_master1_sem, 0);
  rsi_semaphore_create(&ble_master2_sem, 0);
  rsi_semaphore_create(&ui_task_sem, 0);
  rsi_task_create((void *)rsi_bt_app_task,
                  (uint8_t *)"bt_task",
                  RSI_BT_TASK_STACK_SIZE,
                  NULL,
                  RSI_BT_TASK_PRIORITY,
                  &bt_task_handle);
  rsi_task_create((void *)rsi_ble_main_app_task,
                  (uint8_t *)"ble_main_task",
                  RSI_BLE_COMMON_TASK_SIZE,
                  NULL,
                  RSI_BLE_COMMON_TASK_PRIORITY,
                  &ble_main_task_handle);
  rsi_task_create((void *)rsi_ble_master1_app_task,
                  (uint8_t *)"ble_master1_task",
                  RSI_BLE_MASTER1_TASK_SIZE,
                  NULL,
                  RSI_BLE_MASTER1_TASK_PRIORITY,
                  &ble_master1_task_handle);
  rsi_task_create((void *)rsi_ble_master2_app_task,
                  (uint8_t *)"ble_master2_task",
                  RSI_BLE_MASTER2_TASK_SIZE,
                  NULL,
                  RSI_BLE_MASTER2_TASK_PRIORITY,
                  &ble_master2_task_handle);
  rsi_task_create((void *)rsi_ble_slave1_app_task,
                  (uint8_t *)"ble_slave1_task",
                  RSI_BLE_SLAVE1_TASK_SIZE,
                  NULL,
                  RSI_BLE_SLAVE1_TASK_PRIORITY,
                  &ble_slave1_task_handle);
  rsi_task_create((void *)rsi_ble_slave2_app_task,
                  (uint8_t *)"ble_slave2_task",
                  RSI_BLE_SLAVE2_TASK_SIZE,
                  NULL,
                  RSI_BLE_SLAVE2_TASK_PRIORITY,
                  &ble_slave2_task_handle);
  rsi_task_create((void *)rsi_ble_slave3_app_task,
                  (uint8_t *)"ble_slave3_task",
                  RSI_BLE_SLAVE3_TASK_SIZE,
                  NULL,
                  RSI_BLE_SLAVE3_TASK_PRIORITY,
                  &ble_slave3_task_handle);
#elif BLE_MULTI_SLAVE_MASTER_BT_A2DP_WIFI_HTTP_S_RX
  rsi_semaphore_create(&coex_sem, 0);
  rsi_semaphore_create(&coex_sem1, 0);
  rsi_semaphore_create(&suspend_sem, 0);
  rsi_semaphore_create(&ble_main_task_sem, 0);
  rsi_semaphore_create(&ble_slave1_sem, 0);
  rsi_semaphore_create(&ble_slave2_sem, 0);
  rsi_semaphore_create(&ble_slave3_sem, 0);
  rsi_semaphore_create(&ble_master1_sem, 0);
  rsi_semaphore_create(&ble_master2_sem, 0);
  rsi_semaphore_create(&ui_task_sem, 0);
  rsi_task_create((void *)rsi_wlan_app_task,
                  (uint8_t *)"wlan_task",
                  RSI_WLAN_TASK_STACK_SIZE,
                  NULL,
                  RSI_WLAN_TASK_PRIORITY,
                  &wlan_task_handle);
  rsi_task_create((void *)rsi_bt_app_task,
                  (uint8_t *)"bt_task",
                  RSI_BT_TASK_STACK_SIZE,
                  NULL,
                  RSI_BT_TASK_PRIORITY,
                  &bt_task_handle);
  rsi_task_create((void *)rsi_ble_main_app_task,
                  (uint8_t *)"ble_main_task",
                  RSI_BLE_COMMON_TASK_SIZE,
                  NULL,
                  RSI_BLE_COMMON_TASK_PRIORITY,
                  &ble_main_task_handle);
  rsi_task_create((void *)rsi_ble_master1_app_task,
                  (uint8_t *)"ble_master1_task",
                  RSI_BLE_MASTER1_TASK_SIZE,
                  NULL,
                  RSI_BLE_MASTER1_TASK_PRIORITY,
                  &ble_master1_task_handle);
  rsi_task_create((void *)rsi_ble_master2_app_task,
                  (uint8_t *)"ble_master2_task",
                  RSI_BLE_MASTER2_TASK_SIZE,
                  NULL,
                  RSI_BLE_MASTER2_TASK_PRIORITY,
                  &ble_master2_task_handle);
  rsi_task_create((void *)rsi_ble_slave1_app_task,
                  (uint8_t *)"ble_slave1_task",
                  RSI_BLE_SLAVE1_TASK_SIZE,
                  NULL,
                  RSI_BLE_SLAVE1_TASK_PRIORITY,
                  &ble_slave1_task_handle);
  rsi_task_create((void *)rsi_ble_slave2_app_task,
                  (uint8_t *)"ble_slave2_task",
                  RSI_BLE_SLAVE2_TASK_SIZE,
                  NULL,
                  RSI_BLE_SLAVE2_TASK_PRIORITY,
                  &ble_slave2_task_handle);
  rsi_task_create((void *)rsi_ble_slave3_app_task,
                  (uint8_t *)"ble_slave3_task",
                  RSI_BLE_SLAVE3_TASK_SIZE,
                  NULL,
                  RSI_BLE_SLAVE3_TASK_PRIORITY,
                  &ble_slave3_task_handle);
#elif COEX_MAX_APP
#if RUN_TIME_CONFIG_ENABLE
  rsi_semaphore_create(&common_task_sem, 0);
  rsi_semaphore_create(&ui_task_sem, 0);
  rsi_task_create((void *)rsi_ui_app_task,
                  (uint8_t *)"ui_task",
                  RSI_UI_TASK_STACK_SIZE,
                  NULL,
                  RSI_UI_TASK_PRIORITY,
                  &ui_app_task_handle);
#endif
  rsi_task_create((void *)rsi_common_app_task,
                  (uint8_t *)"common_task",
                  RSI_COMMON_TASK_STACK_SIZE,
                  NULL,
                  RSI_COMMON_TASK_PRIORITY,
                  &common_task_handle);
#elif COEX_MAX_APP_BLE_2MAS_8SLAV
#if RUN_TIME_CONFIG_ENABLE
  rsi_semaphore_create(&common_task_sem, 0);
  rsi_semaphore_create(&ui_task_sem, 0);
  rsi_task_create((void *)rsi_ui_app_task,
                  (uint8_t *)"ui_task",
                  RSI_UI_TASK_STACK_SIZE,
                  NULL,
                  RSI_UI_TASK_PRIORITY,
                  &ui_app_task_handle);
#endif
  rsi_task_create((void *)rsi_common_app_task,
                  (uint8_t *)"common_task",
                  RSI_COMMON_TASK_STACK_SIZE,
                  NULL,
                  RSI_COMMON_TASK_PRIORITY,
                  &common_task_handle);
#elif UNIFIED_PROTOCOL
  rsi_task_create((void *)rsi_common_app_task,
                  (uint8_t *)"common_task",
                  RSI_COMMON_TASK_STACK_SIZE,
                  NULL,
                  RSI_COMMON_TASK_PRIORITY,
                  &common_task_handle);

#elif PROP_PROTOCOL_BLE_DUAL_ROLE_BT_A2DP_SRC_APP

#if ENABLE_BT
  rsi_semaphore_create(&coex_sem, 0);
  rsi_task_create((void *)rsi_bt_app_task,
                  (uint8_t *)"bt_task",
                  RSI_BT_TASK_STACK_SIZE,
                  NULL,
                  RSI_BT_TASK_PRIORITY,
                  &bt_task_handle);
#endif
#if ENABLE_BLE
  rsi_semaphore_create(&ble_main_task_sem, 0);
  rsi_task_create((void *)rsi_ble_main_app_task,
                  (uint8_t *)"ble_main_task",
                  RSI_BLE_COMMON_TASK_SIZE,
                  NULL,
                  RSI_BLE_TASK_PRIORITY,
                  &ble_main_task_handle);
#endif
#if ENABLE_PROP_PROTOCOL
  rsi_semaphore_create(&prop_protocol_coex_sem, 0);
  rsi_semaphore_create(&prop_protocol_sem, 0);
  rsi_task_create((void *)rsi_prop_protocol_task,
                  (uint8_t *)"prop_protocol_task",
                  RSI_PROP_PROTOCOL_TASK_STACK_SIZE,
                  NULL,
                  RSI_PROP_PROTOCOL_TASK_PRIORITY,
                  &prop_protocol_task_handle);
#endif
#elif BT_CW_MODE
  rsi_semaphore_create(&coex_sem, 0);
  rsi_task_create((void *)rsi_app_task_bt_cw_mode,
                  "bt_task",
                  RSI_BT_TASK_STACK_SIZE,
                  NULL,
                  RSI_BT_TASK_PRIORITY,
                  &bt_task_handle);
#elif BLE_CW_MODE
  rsi_semaphore_create(&coex_sem, 0);
  rsi_task_create((void *)rsi_ble_cw_mode,
                  "ble_task",
                  RSI_BLE_TASK_STACK_SIZE,
                  NULL,
                  RSI_BLE_TASK_PRIORITY,
                  &ble_task_handle);
#elif PROP_PROTOCOL_CW_MODE
  rsi_semaphore_create(&coex_sem, 0);
  rsi_task_create((void *)rsi_prop_protocol_cw_mode,
                  "ble_task",
                  RSI_BLE_TASK_STACK_SIZE,
                  NULL,
                  RSI_BLE_TASK_PRIORITY,
                  &ble_task_handle);
#elif WLAN_STANDBY_WITH_PROP_PROTOCOL
  rsi_task_create((void *)rsi_wlan_standby_with_prop_protocol_task,
                  (uint8_t *)"prop_protocol_task",
                  RSI_PROP_PROTOCOL_TASK_STACK_SIZE,
                  NULL,
                  RSI_PROP_PROTOCOL_TASK_PRIORITY,
                  &prop_protocol_task_handle);
#elif COEX_THROUGHPUT
  void rsi_common_app_task(void);
  rsi_task_create((void *)rsi_common_app_task,
                  (uint8_t *)"common_task",
                  RSI_COMMON_TASK_STACK_SIZE,
                  NULL,
                  RSI_COMMON_TASK_PRIORITY,
                  &common_task_handle);
#elif BT_BLE_UPDATE_GAIN_TABLE
  rsi_task_create((void *)rsi_app_task_update_gain_table,
                  (uint8_t *)"ble_task",
                  RSI_BLE_TASK_STACK_SIZE,
                  NULL,
                  RSI_BLE_TASK_PRIORITY,
                  &ble_task_handle);
#endif
  //! Task created for Driver task
  rsi_task_create((void *)rsi_wireless_driver_task,
                  (uint8_t *)"driver_task",
                  RSI_DRIVER_TASK_STACK_SIZE,
                  NULL,
                  RSI_DRIVER_TASK_PRIORITY,
                  &driver_task_handle);

  //! OS TAsk Start the scheduler
  rsi_start_os_scheduler();
#else
  while (RSI_FOREVER) {
    //! Execute demo
    rsi_demo_app();

    //! wireless driver task
    rsi_wireless_driver_task();
  }
#endif
  return 0;
}

#ifdef RSI_WITH_OS
void rsi_wireless_driver_task_create()
{
  //! Task created for Driver task
  rsi_task_create((void *)rsi_wireless_driver_task,
                  (uint8_t *)"driver_task",
                  RSI_DRIVER_TASK_STACK_SIZE,
                  NULL,
                  RSI_DRIVER_TASK_PRIORITY,
                  &driver_task_handle);
}
#endif

void rsi_demo_app(void)
{

#if (BT_A2DP_SOURCE || BT_A2DP_SOURCE_SBC_ENC_IN_RS9116)
  rsi_app_task_bt_a2dp_source();
#elif BT_A2DP_SOURCE_SBC_CODEC
  rsi_app_task_bt_a2dp_source_sbc_codec();
#elif BT_PER
  rsi_app_task_bt_per();
#elif BT_SNIFF
  rsi_app_task_bt_connected_sniff();
#elif BT_SPP_RX_TX
  rsi_app_task_bt_spp_rx_tx();
#elif BLE_ADV_BT_SPP_THROUGHPUT
  rsi_app_task_ble_adv_bt_spp_tpt();
#elif BT_EIR_SNIFF_SPP_SLAVE
  rsi_app_task_eir_sniff_spp();
#elif BLE_ADV_BT_A2DP_SOURCE
  rsi_app_task_ble_adv_bt_a2dp();
#elif BLE_INDICATION
  rsi_app_task_ble_peripheral();
#elif BLE_PERIPHERAL
  rsi_app_task_ble_peripheral();
#elif BLE_SIMPLE_GATT
  rsi_app_task_ble_simple_gatt();
#elif BLE_CENTRAL
  rsi_app_task_ble_central();
#elif BLE_DUAL_ROLE
  rsi_app_task_ble_dual_role();
#elif BLE_POWERSAVE
  rsi_app_task_ble_powersave();
#elif BT_PAGE_INQUIRY
  rsi_app_task_bt_page_inquiry();
#elif BT_INQUIRY_SCAN
  rsi_app_task_bt_inquiry_scan();
#elif BT_PAGE_SCAN
  rsi_app_task_bt_page_scan();
#elif BT_SPP_MASTER_SNIFF
  rsi_app_task_bt_spp_master_sniff();
#elif BT_SPP_SLAVE_SNIFF
  rsi_app_task_bt_spp_slave_sniff();
#elif BLE_ADV
  rsi_app_task_ble_peripheral_adv();
#elif BLE_SCAN
  rsi_app_task_ble_central_scan();
#elif BLE_IDLE_POWERSAVE
  if (!ble_powersave_d)
    rsi_app_task_ble_ilde_powersave();
#elif BT_BLE_IDLE_POWERSAVE
  if (!powersave_d)
    rsi_app_task_bt_ble_ilde_powersave();
#elif WIFI_SSL_RX_THROUGHPUT
  if (!tx_rx_Completed)
    rsi_app_task_wifi_ssl_rx_tpt();
#elif WIFI_TCP_BI_DIR
  if (!loopback_done)
    rsi_app_task_wifi_tcp_bi_dir();
#elif WIFI_SSL_TX_THROUGHPUT
  if (!tx_rx_Completed)
    rsi_app_task_wifi_ssl_tx_tpt();
#elif WIFI_TCP_RX_POWERSAVE
  if (!tx_rx_Completed)
    rsi_app_task_wifi_tcp_rx_ps();
#elif WIFI_CONNECTED_STANDBY
  if (!powersave_d)
    rsi_app_task_wifi_connected_standby();
#elif WIFI_STA_BGSCAN
  rsi_app_task_wifi_sta_bgscan();
#elif WIFI_STA_CONN_DISCONN_SSL_CONN_DISCONN
  rsi_app_task_wifi_sta_ssl_conn_disconn();
#elif WIFI_TCP_TX_POWERSAVE
  rsi_app_task_wifi_tcp_tx_ps();
#elif WIFI_DEEPSLEEP_STANDBY
  if (!powersave_d)
    rsi_app_task_wifi_deepsleep_standby();
#elif WIFI_SSL_SERVER_CERT_BYPASS
  rsi_app_task_wifi_ssl_server_cert_bypass();
#elif BT_A2DP_SOURCE_WIFI_HTTP_S_RX
  rsi_app_task_bt_a2dp_wifi_http_s();
#elif BT_A2DP_SOURCE_WIFI_HTTP_S_RX_DYN_COEX
  rsi_app_task_bt_a2dp_wifi_http_s_dyn_coex();
#elif WIFI_HTTP_S_5MB_RX_WITH_STANDBY
  rsi_app_task_wifi_https_rx_standby();
#elif 0 //WIFI_PROP_PROTOCOL_HTTP_S_5MB_RX_WITH_STANDBY
  rsi_app_task_wifi_prop_protocol_https_rx_standby();
#elif BLE_DUAL_MODE_BT_A2DP_SOURCE_WIFI_HTTP_S_RX
  rsi_app_task_bt_ble_dual_mode_wifi_https();
#elif BLE_DUAL_MODE_BT_SPP_SLAVE
  rsi_app_task_bt_ble_dualmode();
#elif FIRMWARE_UPGRADE
  rsi_app_task_fw_update();
#elif FIRMWARE_UPGRADE_VIA_BOOTLOADER
  rsi_app_task_fw_update_via_bootloader();
#elif RAM_DUMP_ASSERTION_CASE
  int32_t rsi_app_task_ram_dump_assertion_case(void);
#elif BLE_PER
  rsi_ble_per();
#elif BLE_DATA_TRANSFER
  rsi_app_task_ble_data_transfer();
#elif BLE_PRIVACY
  rsi_app_task_ble_privacy();
#elif BLE_LONG_READ
  rsi_app_task_ble_long_read();
#elif BLE_L2CAP_CBFC
  rsi_app_task_le_l2cap_cbfc();
#elif BLE_GATT_TEST_ASYNC
  rsi_app_task_ble_simple_gatt_test();
#elif BLE_GATT_INDICATION_STATUS
  rsi_app_task_ble_gatt_indication_status();
#elif PROP_PROTOCOL_APP
  rsi_prop_protocol();
#elif PROP_PROTOCOL_APP_POWERSAVE
  rsi_app_task_prop_protocol_app_power_save();
#elif BLE_CENTRAL_PROP_PROTOCOL_APP
  rsi_prop_protocol_ble_central_coex_task();
#elif BLE_PERIPHERAL_PROP_PROTOCOL_APP
  rsi_prop_protocol_ble_peripheral_coex_task();
#elif PROP_PROTOCOL_BLE_PERI_BT_A2DP_SRC_APP
  rsi_app_task_prop_protocol_ble_peri_bt_a2dp_coex_task();
#elif PROP_PROTOCOL_BLE_CENT_BT_A2DP_SRC_APP
  rsi_app_task_prop_protocol_ble_cent_bt_a2dp_coex_task();
#elif PROP_PROTOCOL_BLE_DUAL_ROLE_BT_A2DP_SRC_APP
  rsi_app_task_prop_protocol_ble_dual_role_bt_a2dp_coex_task();
#elif 0 //WLAN_STANDBY_WITH_PROP_PROTOCOL
  rsi_wlan_standby_with_prop_protocol_task();
#elif (WLAN_STANDBY_WITH_PROP_PROTOCOL || WIFI_PROP_PROTOCOL_HTTP_S_5MB_RX_WITH_STANDBY)
  rsi_wlan_prop_protocol_common_task();
#elif BT_CW_MODE
  rsi_app_task_bt_cw_mode();
#elif BLE_CW_MODE
  rsi_ble_cw_mode();
#elif PROP_PROTOCOL_CW_MODE
  rsi_prop_protocol_cw_mode();
#endif
}

#if (WLAN_STANDBY_WITH_PROP_PROTOCOL || WIFI_PROP_PROTOCOL_HTTP_S_5MB_RX_WITH_STANDBY)
void rsi_wlan_prop_protocol_common_task()
{

  while (RSI_FOREVER) {
    //! BT Task
    //rsi_bt_app_task();

    //! BLE Task
    //rsi_ble_app_task();

    //! WLAN PROP_PROTOCOL COMMON TASK
#if WLAN_STANDBY_WITH_PROP_PROTOCOL
    rsi_wlan_standby_with_prop_protocol_task();
#elif WIFI_PROP_PROTOCOL_HTTP_S_5MB_RX_WITH_STANDBY
    rsi_app_task_wifi_prop_protocol_https_rx_standby();
#endif

    //! PROP_PROTOCOL Task
    rsi_ble_prop_protocol_app_task();

    //! wireless driver task
    rsi_wireless_driver_task();
  }
}
#endif
#if PROP_PROTOCOL_APP
void rsi_app_task_prop_protocol_app()
{
  rsi_prop_protocol();
}
#endif

#if PROP_PROTOCOL_APP_POWERSAVE
void rsi_app_task_prop_protocol_app_power_save()
{
  rsi_prop_protocol_power_save();
}
#endif

#if BLE_CENTRAL_PROP_PROTOCOL_APP
void rsi_prop_protocol_ble_central_coex_task()
{
  rsi_ble_central();
}
#endif

#if BLE_PERIPHERAL_PROP_PROTOCOL_APP
void rsi_prop_protocol_ble_peripheral_coex_task()
{
  rsi_ble_peripheral();
}
#endif
#if PROP_PROTOCOL_BLE_PERI_BT_A2DP_SRC_APP
void rsi_app_task_prop_protocol_ble_peri_bt_a2dp_coex_task(void)
{
  int32_t status = RSI_SUCCESS;

  //! BT A2DP Initialization
  status = rsi_bt_a2dp_source();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("bt_a2dp init failed\n");
    return;
  }

  // Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! BLE dual role Initialization
  status = rsi_ble_peripheral_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("ble peripheral role init failed\n");
    return;
  }

  //! PROP_PROTOCOL  role Initialization
  status = rsi_prop_protocol_app();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("PROP_PROTOCOL role init got failed\n");
    return;
  }

  while (RSI_FOREVER) {
    //! BT Task
    rsi_bt_app_task();

    //! BLE Task
    rsi_ble_app_task();

    //! PROP_PROTOCOL Task
    rsi_prop_protocol_app_task();

    //! wireless driver task
    rsi_wireless_driver_task();
  }
}
#endif

#if PROP_PROTOCOL_BLE_CENT_BT_A2DP_SRC_APP
void rsi_app_task_prop_protocol_ble_cent_bt_a2dp_coex_task(void)
{
  int32_t status = RSI_SUCCESS;

  //! BT A2DP Initialization
  status = rsi_bt_a2dp_source();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("bt_a2dp init failed\n");
    return;
  }

  // Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! BLE dual role Initialization
  status = rsi_ble_central_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("ble central role init failed\n");
    return;
  }

  //! PROP_PROTOCOL  role Initialization
  status = rsi_prop_protocol_app();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("PROP_PROTOCOL role init got failed\n");
    return;
  }

  while (RSI_FOREVER) {
    //! BT Task
    rsi_bt_app_task();

    //! BLE Task
    rsi_ble_app_task();

    //! PROP_PROTOCOL Task
    rsi_prop_protocol_app_task();

    //! wireless driver task
    rsi_wireless_driver_task();
  }
}
#endif

#if PROP_PROTOCOL_BLE_DUAL_ROLE_BT_A2DP_SRC_APP
void rsi_app_task_prop_protocol_ble_dual_role_bt_a2dp_coex_task(void)
{
  int32_t status = RSI_SUCCESS;

#ifndef RSI_WITH_OS
#if 1
  status = rsi_switch_proto(1, NULL);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n enable fail \n");
    //return status;
  }
  //! BT A2DP Initialization
  status = rsi_bt_a2dp_source();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("bt_a2dp init failed\n");
    return;
  }
#endif
  // Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! BLE dual role Initialization
  status = rsi_ble_dual_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("ble dual role init failed\n");
    return;
  }

  //! PROP_PROTOCOL  role Initialization
  status = rsi_ble_prop_protocol_app();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("PROP_PROTOCOL role init got failed\n");
    return;
  }

  while (RSI_FOREVER) {
#if 1
    //! BT Task
    rsi_bt_app_task();
#endif
    //! BLE Task
    rsi_ble_app_task();

    //! PROP_PROTOCOL Task
    rsi_ble_prop_protocol_app_task();

    //! wireless driver task
    rsi_wireless_driver_task();
  }
#endif
}
#endif

#if BT_A2DP_SOURCE_WIFI_HTTP_S_RX
void rsi_app_task_bt_a2dp_wifi_http_s(void)
{
  int32_t status = RSI_SUCCESS;

  //! switch to BT protocol
  status = rsi_switch_proto(1, NULL);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n bt enable fail \n");
    //return status;
  }

  //! BT A2DP Initialization
  status = rsi_bt_a2dp_source();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("bt_a2dp init failed\n");
    return;
  }

  while (RSI_FOREVER) {
    //! WLAN Task
    rsi_wlan_app_task();

    //! BT Task
    rsi_bt_app_task();

    //! wireless driver task
    rsi_wireless_driver_task();
  }
}
#endif

#if BT_A2DP_SOURCE_WIFI_HTTP_S_RX_DYN_COEX
void rsi_app_task_bt_a2dp_wifi_http_s_dyn_coex(void)
{
  int32_t status = RSI_SUCCESS;

  status = rsi_switch_proto(1, NULL);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n bt enable fail \n");
    //return status;
  }

  //! BT A2DP Initialization
  status = rsi_bt_a2dp_source();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("bt_a2dp init failed\n");
    return;
  }

  while (RSI_FOREVER) {
    //! WLAN Task
    rsi_wlan_app_task();

    //! BT Task
    rsi_bt_app_task();

    //! wireless driver task
    rsi_wireless_driver_task();
  }
}
#endif

#if BLE_PERIPHERAL
void rsi_app_task_ble_peripheral(void)
{
  int32_t status = RSI_SUCCESS;

#ifdef RSI_WITH_OS
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! //! BLE dual role Initialization
  status = rsi_ble_dual_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("BLE dual role init failed\n");
    return;
  }

  while (RSI_FOREVER) {

    status = rsi_ble_app_task();

#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif

#if BLE_INDICATION
void rsi_app_task_ble_peripheral(void)
{
  int32_t status = RSI_SUCCESS;

#ifdef RSI_WITH_OS
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! //! BLE dual role Initialization
  status = rsi_ble_dual_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("BLE dual role init failed\n");
    return;
  }

  while (RSI_FOREVER) {

    status = rsi_ble_app_task();

#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif
#if BLE_SIMPLE_GATT
void rsi_app_task_ble_simple_gatt(void)
{
  int32_t status = RSI_SUCCESS;

#ifdef RSI_WITH_OS
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! //! BLE dual role Initialization
  status = rsi_ble_dual_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("BLE dual role init failed\n");
    return;
  }

  while (RSI_FOREVER) {

    status = rsi_ble_app_task();

#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif

#if BLE_PRIVACY
void rsi_app_task_ble_privacy(void)
{
  int32_t status = RSI_SUCCESS;

#ifdef RSI_WITH_OS
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! //! BLE dual role Initialization
  status = rsi_ble_dual_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("BLE dual role init failed\n");
    return;
  }

  while (RSI_FOREVER) {

    status = rsi_ble_app_task();
#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif

#if BLE_GATT_TEST_ASYNC
void rsi_app_task_ble_simple_gatt_test()
{
  int32_t status = RSI_SUCCESS;

#ifdef RSI_WITH_OS
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! BLE dual role Initialization
  status = rsi_ble_simple_gatt_test_app_init();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("BLE dual role init failed\n");
    return;
  }

  while (RSI_FOREVER) {

    status = rsi_ble_simple_gatt_test_app_task();
#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif

#if BLE_CENTRAL
void rsi_app_task_ble_central(void)
{
  int32_t status = RSI_SUCCESS;
#ifdef RSI_WITH_OS
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! //! BLE dual role Initialization
  status = rsi_ble_dual_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("BLE dual role init failed\n");
    return;
  }

  while (RSI_FOREVER) {

    status = rsi_ble_app_task();
#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif

#if BLE_DUAL_ROLE
void rsi_app_task_ble_dual_role(void)
{
#ifdef RSI_WITH_OS
  int32_t status = RSI_SUCCESS;
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! //! BLE dual role Initialization
  rsi_ble_dual_role();

  while (RSI_FOREVER) {

    rsi_ble_app_task();
#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif

#if BLE_POWERSAVE
void rsi_app_task_ble_powersave(void)
{
#ifdef RSI_WITH_OS
  int32_t status = RSI_SUCCESS;
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! //! BLE dual role Initialization
  rsi_ble_dual_role();

  while (RSI_FOREVER) {

    rsi_ble_app_task();

#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif

#if BLE_GATT_INDICATION_STATUS
void rsi_app_task_ble_gatt_indication_status(void)
{
#ifdef RSI_WITH_OS
  int32_t status = RSI_SUCCESS;
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! //! BLE dual role Initialization
  rsi_ble_gatt_indication_status_app_init();

  while (RSI_FOREVER) {

    rsi_ble_gatt_indication_status_app_task();

#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif

#if BLE_DUAL_MODE_BT_A2DP_SOURCE_WIFI_HTTP_S_RX
void rsi_app_task_bt_ble_dual_mode_wifi_https(void)
{
  int32_t status                  = RSI_SUCCESS;
  uint8_t initialize_bt_wlan_task = 0;
  uint8_t initialize_bt_a2dp      = 0;

  // Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! BLE dual role Initialization
  status = rsi_ble_dual_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("ble dual role init failed\n");
    return;
  }

  while (RSI_FOREVER) {
    //! BLE Task
    rsi_ble_app_task();

    if (initialize_bt_a2dp == 1) {
      //! BT A2DP Initialization
      status = rsi_bt_a2dp_source();
      if (status != RSI_SUCCESS) {
        LOG_PRINT("bt_a2dp init failed\n");
        return;
      }
      initialize_bt_a2dp      = 2;
      initialize_bt_wlan_task = 1;
    }

    if (((num_of_conn_slaves == RSI_BLE_MAX_NBR_SLAVES) && (num_of_conn_masters == RSI_BLE_MAX_NBR_MASTERS))
        || (initialize_bt_wlan_task == 1)) {
      if (initialize_bt_wlan_task == 1) {
        //! WLAN Task
        rsi_wlan_app_task();
        //! BT Task
        rsi_bt_app_task();
      } else {
        initialize_bt_a2dp = 1;
      }
    }
    //! wireless driver task
    rsi_wireless_driver_task();
  }
}
#endif

#if BLE_DUAL_MODE_BT_SPP_SLAVE
void rsi_app_task_bt_ble_dualmode(void)
{
  int32_t status                  = RSI_SUCCESS;
  uint8_t initialize_bt_wlan_task = 0;

  /// Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! BLE dual role Initialization
  status = rsi_ble_dual_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("ble dual role init failed\n");
    return;
  }
  status = rsi_bt_spp();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("bt init failed\n");
    return;
  }

  while (RSI_FOREVER) {
    //! BLE Task
    rsi_ble_app_task();

    if (((num_of_conn_slaves == RSI_BLE_MAX_NBR_SLAVES) && (num_of_conn_masters == RSI_BLE_MAX_NBR_MASTERS))
        || (initialize_bt_wlan_task != 0)) {
      initialize_bt_wlan_task = 1;
      //! BT Task
      rsi_bt_app_task();
    }
    //! wireless driver task
    rsi_wireless_driver_task();
  }
}
#endif

#if BLE_ADV_BT_A2DP_SOURCE
void rsi_app_task_ble_adv_bt_a2dp(void)
{
  int32_t status = RSI_SUCCESS;

  //! BT A2DP Initialization
  status = rsi_bt_a2dp_source();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("bt_a2dp init failed\n");
    return;
  }

  // Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! BLE dual role Initialization
  status = rsi_ble_adv_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("ble dual role init failed\n");
    return;
  }

  while (RSI_FOREVER) {
    //! BT Task
    rsi_bt_app_task();

    //! BLE Task
    rsi_ble_app_task();

    //! wireless driver task
    rsi_wireless_driver_task();
  }
}
#endif

#if BLE_ADV
void rsi_app_task_ble_peripheral_adv(void)
{
  int32_t status = RSI_SUCCESS;
#ifdef RSI_WITH_OS
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return;
  }

  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! //! BLE dual role Initialization
  status = rsi_ble_peripheral_adv();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("BLE init failed\n");
    return;
  }

  while (RSI_FOREVER) {

    status = rsi_ble_adv();

#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif

#if BLE_SCAN
void rsi_app_task_ble_central_scan(void)
{
  int32_t status = RSI_SUCCESS;
#ifdef RSI_WITH_OS
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! //! BLE dual role Initialization
  status = rsi_ble_central_scan();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("BLE init failed\n");
    return;
  }

  while (RSI_FOREVER) {

    status = rsi_ble_scan();

#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif

#if BLE_LONG_READ
void rsi_app_task_ble_long_read()
{
  int32_t status = RSI_SUCCESS;
#ifdef RSI_WITH_OS
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! //! BLE dual role Initialization
  status = rsi_app_init_ble_simple_gatt_test();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("BLE init failed\n");
    return;
  }

  while (RSI_FOREVER) {
    status = rsi_ble_app_task();
#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif

#if BLE_DATA_TRANSFER
void rsi_app_task_ble_data_transfer(void)
{
  int32_t status = RSI_SUCCESS;
#ifdef RSI_WITH_OS
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return;
  }
  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! //! BLE dual role Initialization
  status = rsi_ble_peripheral_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("BLE dual role init failed\n");
    return;
  }

  while (RSI_FOREVER) {

    status = rsi_ble_app_task();
#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif

#if BLE_L2CAP_CBFC
void rsi_app_task_le_l2cap_cbfc(void)
{
  int32_t status = RSI_SUCCESS;
#ifdef RSI_WITH_OS
  //! Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return status;
  }
  //! WiSeConnect initialization
  status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
  if (status != RSI_SUCCESS) {
    return status;
  }
  //! Send Feature frame
  status = rsi_send_feature_frame();
  if (status != RSI_SUCCESS) {
    return status;
  }
#endif
  //Start BT Stack
  intialize_bt_stack(STACK_BTLE_MODE);

  //! BLE initialization
  status = rsi_ble_central_l2cap_cbfc();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("BLE init failed\n");
    return;
  }

  while (RSI_FOREVER) {

    status = rsi_ble_app_task();
#ifndef RSI_WITH_OS
    //! wireless driver task
    rsi_wireless_driver_task();
#endif
  }
}
#endif
