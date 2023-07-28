/*******************************************************************************
* @file  rsi_wlan_http_s_DEMO_69.c
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
 * @file    rsi_wlan_http_s_DEMO_69.c
 * @version 0.1
 * @date    01 May 2020
 *
 *
 *  @brief : This file contains example application for TCP+SSL client socket
 *
 *  @section Description  This file contains example application for TCP+SSL client socket
 *
 *
 */
/*=======================================================================*/
//  ! INCLUDES
/*=======================================================================*/
#include <rsi_common_app.h>
#include <rsi_common_app_DEMO_69.h>
#if (COEX_MAX_APP_BLE_2MAS_8SLAV && !WLAN_STA_TX_CASE)
#include "stdlib.h"
#include "rsi_driver.h"
#include "rsi_utils.h"
#include <rsi_wlan_non_rom.h>
#include <rsi_bt_common_apis.h>
#include "rsi_sock_test_DEMO_69.h"
#include "rsi_common_app_DEMO_69.h"
#include "rsi_socket.h"
#include "rsi_wlan_config_DEMO_69.h"
#if RSI_TCP_IP_BYPASS
#include "sys_arch.h"
#include "lwip/priv/tcpip_priv.h"
#include "lwip/timeouts.h"
#include "lwip/netif.h"
#endif
#if SSL
#include "servercert.pem" //! Include SSL CA certificate
#endif

/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/

/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/

//! Enumeration for states in application
typedef enum rsi_wlan_app_state_e {
  RSI_WLAN_INITIAL_STATE          = 0,
  RSI_WLAN_UNCONNECTED_STATE      = 1,
  RSI_WLAN_SCAN_DONE_STATE        = 2,
  RSI_WLAN_CONNECTED_STATE        = 3,
  RSI_WLAN_IPCONFIG_DONE_STATE    = 4,
  RSI_WLAN_DISCONNECTED_STATE     = 5,
  RSI_WLAN_SOCKET_CONNECTED_STATE = 6,
  RSI_POWER_SAVE_STATE            = 7
} rsi_wlan_app_state_t;

//! WLAN application control block
typedef struct rsi_wlan_app_cb_s {
  rsi_wlan_app_state_t state;       //! WLAN application state
  uint32_t length;                  //! length of buffer to copy
  uint8_t buffer[RSI_APP_BUF_SIZE]; //! application buffer
  uint8_t buf_in_use;               //! to check application buffer availability
  uint32_t event_map;               //! application events bit map
} rsi_wlan_app_cb_t;
struct rsi_sockaddr_in server_addr, client_addr; //! server and client IP addresses

rsi_task_handle_t http_socket_task_handle[SOCKTEST_INSTANCES_MAX] = { NULL };
extern rsi_semaphore_handle_t wlan_app_sem;
rsi_wlan_app_cb_t rsi_wlan_app_cb; //! application control block
int32_t client_socket;             //! client socket id
//! Throughput parameters
uint32_t pkts       = 0;
uint32_t num_bits   = 0;
uint32_t total_bits = 0;
uint32_t xfer_time;
uint32_t total_time = 0;
uint32_t xfer_time_usec;
uint32_t t_start = 0;
uint32_t t_end;
uint32_t throughput = 0, throughput_KBps = 0;
float throughput_mbps;
volatile uint8_t data_recvd = 0;
volatile uint32_t num_bytes = 0;
uint8_t no_of_iterations    = 0;

#if RSI_TCP_IP_BYPASS
uint8_t mac_address[6];
struct netif sta_netif;
extern sys_thread_t lwip_thread;
#endif
/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/
extern socktest_ctx_t socktest_ctx[SOCKTEST_INSTANCES_MAX];
extern rsi_semaphore_handle_t wlan_app_sem;
#if WLAN_SYNC_REQ
extern bool other_protocol_activity_enabled;
#endif
#if SOCKET_ASYNC_FEATURE
extern rsi_semaphore_handle_t sock_wait_sem;
#endif

#if WLAN_SYNC_REQ
extern bool rsi_ble_running, rsi_bt_running, rsi_prop_protocol_running;
extern rsi_semaphore_handle_t sync_coex_prop_protocol_sem, sync_coex_ble_sem, sync_coex_bt_sem;
#endif
extern bool powersave_cmd_given;
extern rsi_mutex_handle_t power_cmd_mutex;
rsi_max_available_rx_window_t *max_available_rx_window;
#if (SSL_TX_DATA || SSL_RX_DATA || (RX_DATA && HTTPS_DOWNLOAD))
extern rsi_semaphore_handle_t cert_sem, conn_sem;
extern rsi_task_handle_t cert_bypass_task_handle[SOCKTEST_INSTANCES_MAX];
cert_bypass_struct_t rsi_cert_bypass[SOCKTEST_INSTANCES_MAX];
extern void certificate_response_handler(uint16_t status, uint8_t *buffer, const uint32_t length);
#endif
/*=======================================================================*/
//   ! EXTERN FUNCTIONS
/*=======================================================================*/

/*=======================================================================*/
//   ! PROCEDURES
/*=======================================================================*/

/*=======================================================================*/
//   ! VARIABLES
/*=======================================================================*/

/*************************************************************************/
//!  CALLBACK FUNCTIONS
/*************************************************************************/
#if RSI_TCP_IP_BYPASS
void wlan_data_receive_handler(uint16_t status, uint8_t *buffer, const uint32_t length);
extern const int lwip_num_cyclic_timers;
void lwip_deinit(void)
{
  struct sys_timeo **list_head = NULL;
  rsi_wlan_register_callbacks(RSI_WLAN_DATA_RECEIVE_NOTIFY_CB, NULL);
  dhcp_release_and_stop(&sta_netif);
  netif_set_down(&sta_netif);
  netif_remove(&sta_netif);
  rsi_task_destroy(lwip_thread.thread_handle);
  //! Free all timers
  for (int i = 0; i < lwip_num_cyclic_timers; i++) {
    list_head = sys_timeouts_get_next_timeout();
    if (*list_head != NULL)
      sys_untimeout((*list_head)->h, (*list_head)->arg);
  }
}
#endif
/*====================================================*/
/**
 * @fn          void rsi_wlan_app_callbacks_init(void)
 * @brief       To initialize WLAN application callback
 * @param[in]   void
 * @return      void
 * @section description
 * This callback is used to initialize WLAN
 * ==================================================*/
void rsi_wlan_app_callbacks_init(void)
{
  rsi_wlan_register_callbacks(RSI_JOIN_FAIL_CB, rsi_join_fail_handler);     //! Initialize join fail call back
  rsi_wlan_register_callbacks(RSI_IP_FAIL_CB, rsi_ip_renewal_fail_handler); //! Initialize IP renewal fail call back
  rsi_wlan_register_callbacks(RSI_REMOTE_SOCKET_TERMINATE_CB,
                              rsi_remote_socket_terminate_handler); //! Initialize remote terminate call back
  rsi_wlan_register_callbacks(RSI_IP_CHANGE_NOTIFY_CB,
                              rsi_ip_change_notify_handler); //! Initialize IP change notify call back
  rsi_wlan_register_callbacks(RSI_STATIONS_CONNECT_NOTIFY_CB,
                              rsi_stations_connect_notify_handler); //! Initialize IP change notify call back
  rsi_wlan_register_callbacks(RSI_STATIONS_DISCONNECT_NOTIFY_CB,
                              rsi_stations_disconnect_notify_handler); //! Initialize IP change notify call back
#if (SSL_TX_DATA || SSL_RX_DATA || (RX_DATA && HTTPS_DOWNLOAD))
  rsi_wlan_register_callbacks(RSI_WLAN_SERVER_CERT_RECEIVE_NOTIFY_CB, certificate_response_handler);
#endif
}
/*====================================================*/
/**
 * @fn         void socket_async_recive(uint32_t sock_no, uint8_t *buffer, uint32_t length)
 * @brief      Function to create Async socket
 * @param[in]  uint32_t sock_no, uint8_t *buffer, uint32_t length
 * @return     void
 * @section description
 * Callback for Socket Async Receive
 * ====================================================*/
void socket_async_recive(uint32_t sock_no, uint8_t *buffer, uint32_t length)
{
  //These statements are added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(sock_no);
  UNUSED_PARAMETER(buffer);

  num_bytes += length;
}

/*====================================================*/
/**
 * @fn         void Throughput(void)
 * @brief      Function to calculate throughput
 * @param[in]  void
 * @return     void
 * @section description
 *====================================================*/
void compute_throughput(void)
{
  uint32_t num_bytes_l = 0, throughput_l = 0;
  num_bytes_l  = (num_bytes * 8);                    //! number of bytes
  xfer_time    = (t_end - t_start);                  //! data transfer time sec
  throughput_l = ((float)(num_bytes_l / xfer_time)); //!Throughput calculation

  LOG_PRINT("Time taken in msec: %d \r\n", xfer_time);
  LOG_PRINT("Throughput: %d KBps\r\n", throughput_l);
  num_bytes = 0;
}

#if SSL
/*====================================================*/
/**
 * @fn         int32_t rsi_app_load_ssl_cert()
 * @brief      Function to load SSL certificate
 * @param[in]  void
 * @return     void
 *====================================================*/
int32_t rsi_app_load_ssl_cert()
{
  int32_t status = RSI_SUCCESS;
  status         = rsi_wlan_set_certificate(RSI_SSL_CA_CERTIFICATE, NULL, 0); //! erase existing certificate
  if (status != RSI_SUCCESS) {
    LOG_PRINT("CA cert erase failed \r\n");
    return status;
  }

  status =
    rsi_wlan_set_certificate(RSI_SSL_CA_CERTIFICATE, servercert, (sizeof(servercert) - 1)); //! Load SSL CA certificate
  if (status != RSI_SUCCESS) {
    LOG_PRINT("CA cert load failed \r\n");
    return status;
  }
  return status;
}
#endif

/*====================================================*/
/**
 * @fn         int32_t  rsi_wlan_app_task(void)
 * @brief      Function to work with application task
 * @param[in]  void
 * @return     void
 *=====================================================*/
int32_t rsi_wlan_app_task(void)
{
  int32_t status = RSI_SUCCESS;
  uint8_t ip[20] = { 0 };
#if !(DHCP_MODE)
  uint32_t ip_addr      = DEVICE_IP;
  uint32_t network_mask = NETMASK;
  uint32_t gateway      = GATEWAY;
#else
  uint8_t dhcp_mode = (RSI_DHCP | RSI_DHCP_UNICAST_OFFER);
#endif
  uint32_t i;
  volatile uint8_t download_complete = 0;
#if WLAN_SYNC_REQ
  other_protocol_activity_enabled = false;
#endif

  while (1) {
    switch (rsi_wlan_app_cb.state) {
      case RSI_WLAN_INITIAL_STATE: {
        rsi_wlan_app_callbacks_init();                      //! register callback to initialize WLAN
        rsi_wlan_app_cb.state = RSI_WLAN_UNCONNECTED_STATE; //! update WLAN application state to unconnected state
#if ENABLE_POWER_SAVE
        rsi_mutex_lock(&power_cmd_mutex);
        if (!powersave_cmd_given) {
          status = rsi_initiate_power_save();
          if (status != RSI_SUCCESS) {
            LOG_PRINT("failed to keep module in power save \r\n");
            return status;
          }
          powersave_cmd_given = true;
          LOG_PRINT("WLAN kept Module in sleep \r\n");
        }
        rsi_mutex_unlock(&power_cmd_mutex);

#endif
      }
      //fall through
      //no break
      case RSI_WLAN_UNCONNECTED_STATE: {
        LOG_PRINT("WLAN scan started \r\n");
        status = rsi_wlan_scan((int8_t *)SSID, (uint8_t)CHANNEL_NO, NULL, 0);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("scan failed %x\r\n", status);
          break;
        } else {
          rsi_wlan_app_cb.state = RSI_WLAN_SCAN_DONE_STATE; //! update WLAN application state to connected state
          LOG_PRINT("scan done state \r\n");
        }

#if WLAN_SCAN_ONLY
        rsi_wlan_app_cb.state = RSI_WLAN_UNCONNECTED_STATE;
#if !WLAN_SYNC_REQ
        break;
#else
        if (other_protocol_activity_enabled == false) {
          //! unblock other protocol activities
          if (rsi_bt_running) {
            rsi_semaphore_post(&sync_coex_bt_sem);
          }
          if (rsi_ble_running) {
            rsi_semaphore_post(&sync_coex_ble_sem);
          }
          if (rsi_prop_protocol_running) {
            rsi_semaphore_post(&sync_coex_prop_protocol_sem);
          }
          other_protocol_activity_enabled = true;
        }
        break;
#endif
#endif
      }
      //fall through
      case RSI_WLAN_SCAN_DONE_STATE: {
        status = rsi_wlan_connect((int8_t *)SSID, SECURITY_TYPE, PSK);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("WLAN connection failed %x\r\n", status);
          break;
        } else {
          rsi_wlan_app_cb.state = RSI_WLAN_CONNECTED_STATE; //! update WLAN application state to connected state
          LOG_PRINT("WLAN connected state \r\n");
        }

#if (RX_DATA && ENABLE_POWER_SAVE)
        status = rsi_wlan_power_save_profile(RSI_ACTIVE, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n Failed in initiating power save\r\n");
          return status;
        }
        status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, RSI_UAPSD);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n Failed in initiating power save\r\n");
          return status;
        }
        LOG_PRINT("Kept uapsd powersave \r\n");
#endif
      }
      //fall through
      //no break
      case RSI_WLAN_CONNECTED_STATE: {

#if RSI_TCP_IP_BYPASS
        //! getting MAC address of module
        status = rsi_wlan_get(RSI_MAC_ADDRESS, mac_address, sizeof(mac_address));
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\nMac address Request Failed, Error Code : 0x%X\r\n", status);
          return status;
        }
        LOG_PRINT("\r\nMac address is :%x:%x:%x:%x:%x:%x\r\n",
                  mac_address[0],
                  mac_address[1],
                  mac_address[2],
                  mac_address[3],
                  mac_address[4],
                  mac_address[5]);

        //! LWIP init
        lwip_init_os();
        rsi_wlan_register_callbacks(RSI_WLAN_DATA_RECEIVE_NOTIFY_CB, wlan_data_receive_handler);

        LOG_PRINT("\nLWIP Thread started..\n");
        /* Start DHCP  */
        status = dhcp_start(&sta_netif);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("LWIP IP Config failed %x\r\n", status);
          break;
        }
        //! Looping until IP for the module
        while (sta_netif.ip_addr.addr == NULL) {
          vTaskDelay(10);
        }
        LOG_PRINT("Device IP address : %d.%d.%d.%d\r\n",
                  (uint8_t)(sta_netif.ip_addr.addr & 0xff),
                  (uint8_t)(sta_netif.ip_addr.addr >> 8),
                  (uint8_t)(sta_netif.ip_addr.addr >> 16),
                  (uint8_t)(sta_netif.ip_addr.addr >> 24));

#else /* RSI_TCP_IP_BYPASS */
        //! Configure IP
#if DHCP_MODE
        status = rsi_config_ipaddress(RSI_IP_VERSION_4, dhcp_mode, 0, 0, 0, ip, sizeof(ip), 0);
#else
        status = rsi_config_ipaddress(RSI_IP_VERSION_4,
                                      RSI_STATIC,
                                      (uint8_t *)&ip_addr,
                                      (uint8_t *)&network_mask,
                                      (uint8_t *)&gateway,
                                      NULL,
                                      0,
                                      0);
#endif
        if (status != RSI_SUCCESS) {
          LOG_PRINT("IP Config failed %x\r\n", status);
          break;
        } else {
          rsi_wlan_app_cb.state = RSI_WLAN_IPCONFIG_DONE_STATE;
          LOG_PRINT("WLAN ipconfig done state \r\n");
          LOG_PRINT("RSI_STA IP ADDR: %d.%d.%d.%d \r\n", ip[6], ip[7], ip[8], ip[9]);
#if (SSL && LOAD_CERTIFICATE)
          status = rsi_app_load_ssl_cert(); //Function to load certificate
          if (status != RSI_SUCCESS) {
            break;
          }
#endif
        }
#endif /* RSI_TCP_IP_BYPASS */
      }
      //fall through
      //no break
      case RSI_WLAN_IPCONFIG_DONE_STATE: {
#if WLAN_CONNECT_ONLY
#if !WLAN_SYNC_REQ
        break;
#else
        if (other_protocol_activity_enabled == false) {
          //! unblock other protocol activities
          if (rsi_bt_running) {
            rsi_semaphore_post(&sync_coex_bt_sem);
          }
          if (rsi_ble_running) {
            rsi_semaphore_post(&sync_coex_ble_sem);
          }
          if (rsi_prop_protocol_running) {
            rsi_semaphore_post(&sync_coex_prop_protocol_sem);
          }
          other_protocol_activity_enabled = true;
        }
        break;
#endif
#endif
#if HIGH_PERFORMANCE_ENABLE
        status = rsi_socket_config();
        if (status < 0) {
          LOG_PRINT("high-performance socket config failed \r\n");
          status = rsi_wlan_get_status();
          break;
        }
        LOG_PRINT("high-performance socket config success \r\n");
#endif
#if (!(SSL_TX_DATA || SSL_RX_DATA) && COMPUTE_WLAN_THROUGHPUT)
        compute_throughput();
#endif

        for (i = 0; i < SOCKTEST_INSTANCES_MAX; i++) {
          socktest_ctx[i].threadid = i;
          rsi_semaphore_create(&socktest_ctx[i].http_soc_wait_sem, 0);
          status = rsi_task_create((void *)perform_sock_test,
                                   (uint8_t *)"socket_task1",
                                   RSI_HTTP_SOCKET_TASK_STACK_SIZE,
                                   &socktest_ctx[i],
                                   RSI_HTTP_SOCKET_TASK_PRIORITY,
                                   &http_socket_task_handle[i]);
          if (status != RSI_ERROR_NONE) {
            LOG_PRINT("Thread creation failed %d \r\r", socktest_ctx[i].threadid);
            while (1)
              ;
          }
#if (SSL_TX_DATA || SSL_RX_DATA || (RX_DATA && HTTPS_DOWNLOAD))
          status = rsi_task_create((void *)rsi_app_task_send_certificates,
                                   (uint8_t *)"cert_task",
                                   RSI_CERT_BYPASS_TASK_STACK_SIZE,
                                   NULL,
                                   RSI_CERT_BYPASS_TASK_PRIORITY,
                                   &cert_bypass_task_handle[i]);
          if (status != RSI_ERROR_NONE) {
            LOG_PRINT("\n Thread creation failed %d", socktest_ctx[i].threadid);
            while (1)
              ;
          }
#endif
        }
        for (i = 0; i < SOCKTEST_INSTANCES_MAX; i++) {
          rsi_semaphore_post(&socktest_ctx[i].http_soc_wait_sem);
        }

        do {
          rsi_semaphore_wait(&wlan_app_sem, 0);

          for (i = 0; i < SOCKTEST_INSTANCES_MAX; i++) {
            if (http_socket_task_handle[i] == NULL) {
              download_complete = 1;
            } else {
              download_complete = 0;
              break;
            }
          }
        } while (!download_complete);

        if (download_complete) {
          rsi_os_task_delay(50);
          download_complete = 0;
#if !CONTINUOUS_HTTP_DOWNLOAD
          no_of_iterations++;
#endif

          if (no_of_iterations == NO_OF_ITERATIONS) {
#if SOCKET_ASYNC_FEATURE
            rsi_semaphore_destroy(&sock_wait_sem);
#endif
            for (i = 0; i < SOCKTEST_INSTANCES_MAX; i++) {
              LOG_PRINT("Thread id: %d \r\n", socktest_ctx[i].threadid);
              LOG_PRINT("Tests Success: %d \r\n", socktest_ctx[i].num_successful_test);
              LOG_PRINT("Tests failed: %d \r\n", socktest_ctx[i].num_failed_test);
            }
#if SSL_TX_DATA
            LOG_PRINT("SSL-TX Demo Completed \r\n");
#elif SSL_RX_DATA
            LOG_PRINT("SSL-RX Demo Completed \r\n");
#else
            LOG_PRINT("Download completed \r\n");
#endif
            //! Demo completed
            //! required execution completed, so destroy the task
#if RSI_TCP_IP_BYPASS
            lwip_deinit();
#endif
            rsi_task_destroy(NULL);
          } else {
#if (!SSL_RX_DATA && RX_DATA)
#if HTTPS_DOWNLOAD
            LOG_PRINT("HTTPS download completed \r\n");
#else
            LOG_PRINT("HTTP download completed \r\n");
#endif
#endif
          }
#if !SOCKET_ASYNC_FEATURE
          rsi_wlan_app_cb.state = RSI_WLAN_IPCONFIG_DONE_STATE;
#endif
        }
        break;
      }
      case RSI_WLAN_SOCKET_CONNECTED_STATE: {
        break;
      }
      default:
        break;
    }
  }
  return status;
}

uint32_t rsi_convert_4R_to_BIG_Endian_uint32(uint32_t *pw)
{
  uint32_t val;
  uint8_t *pw1 = (uint8_t *)pw;
  val          = pw1[0];
  val <<= 8;
  val |= pw1[1] & 0x000000ff;
  val <<= 8;
  val |= pw1[2] & 0x000000ff;
  val <<= 8;
  val |= pw1[3] & 0x000000ff;
  return val;
}

/*====================================================*/
/**
 * @fn         void rsi_join_fail_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
 * @brief      Callback handler in station mode at rejoin failure
 * @param[in]  uint16_t status, uint8_t *buffer, const uint32_t length
 * @return     void
 *=====================================================*/
void rsi_join_fail_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
{
  //These statements are added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
  UNUSED_PARAMETER(buffer);
  UNUSED_CONST_PARAMETER(length);
  rsi_wlan_app_cb.state = RSI_WLAN_UNCONNECTED_STATE; //! update wlan application state
}

/*====================================================*/
/**
 * @fn         void rsi_ip_renewal_fail_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
 * @brief      IP renewal failure call back handler in station mode
 * @param[in]  uint16_t status, uint8_t *buffer, const uint32_t length
 * @return     void
 *=====================================================*/
void rsi_ip_renewal_fail_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
{
  //These statements are added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
  UNUSED_PARAMETER(buffer);
  UNUSED_CONST_PARAMETER(length);
  //! update wlan application state
  rsi_wlan_app_cb.state = RSI_WLAN_CONNECTED_STATE;
}

/*====================================================*/
/**
 * @fn         void rsi_remote_socket_terminate_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
 * @brief      Callback handler to terminate stations remote socket
 * @param[in]  uint16_t status, uint8_t *buffer, const uint32_t length
 * @return     void
 *=====================================================*/
void rsi_remote_socket_terminate_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
{
  //These statements are added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
  UNUSED_PARAMETER(buffer);
  UNUSED_CONST_PARAMETER(length);
  data_recvd = 1;                      //Set data receive flag
  t_end      = rsi_hal_gettickcount(); //! capture time-stamp after data transfer is completed
#if SOCKET_ASYNC_FEATURE
  rsi_semaphore_post(&sock_wait_sem);
#endif
  rsi_wlan_app_cb.state = RSI_WLAN_IPCONFIG_DONE_STATE; //! update wlan application state
}

/*====================================================*/
/**
 * @fn         void rsi_ip_change_notify_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
 * @brief      Callback handler to notify IP change in Station mode
 * @param[in]  uint16_t status, uint8_t *buffer, const uint32_t length
 * @return     void
 *=====================================================*/
void rsi_ip_change_notify_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
{
  //These statements are added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
  UNUSED_PARAMETER(buffer);
  UNUSED_CONST_PARAMETER(length);
  //! update wlan application state
  rsi_wlan_app_cb.state = RSI_WLAN_IPCONFIG_DONE_STATE;
}

/*====================================================*/
/**
 * @fn         void rsi_stations_connect_notify_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
 * @brief      Callback handler to notify stations connect in AP mode
 * @param[in]  uint16_t status, uint8_t *buffer, const uint32_t length
 * @return     void
 *=====================================================*/
void rsi_stations_connect_notify_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
{
  //These statements are added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
  UNUSED_PARAMETER(buffer);
  UNUSED_CONST_PARAMETER(length);
}

/*====================================================*/
/**
 * @fn         void rsi_stations_disconnect_notify_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
 * @brief      Callback handler to notify stations disconnect in AP mode
 * @param[in]  uint16_t status, uint8_t *buffer, const uint32_t length
 * @return     void
 *=====================================================*/
void rsi_stations_disconnect_notify_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
{
  //These statements are added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
  UNUSED_PARAMETER(buffer);
  UNUSED_CONST_PARAMETER(length);
}
#endif
