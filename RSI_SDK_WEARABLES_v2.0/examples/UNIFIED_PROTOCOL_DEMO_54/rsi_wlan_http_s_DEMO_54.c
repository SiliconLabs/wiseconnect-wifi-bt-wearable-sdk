/*******************************************************************************
* @file  rsi_wlan_http_s_DEMO_54.c
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
 * @file    rsi_wlan_http_s_DEMO_54.c
 * @version 0.1
 * @date    13 Nov 2018
 *
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
#if (UNIFIED_PROTOCOL && RSI_ENABLE_WIFI_TEST)
#include "stdlib.h"
#include "rsi_driver.h"
#include "rsi_utils.h"
#include <rsi_wlan_non_rom.h>
#include "rsi_wlan_config_DEMO_54.h"
#if CHECK_WIFI_POWER_STATE
#include "rsi_bt_common_apis.h"
#endif
#if (MULTITHREADED_HTTP_DOWNLOAD_TEST || MULTITHREADED_TCP_TX_TEST)
#include "rsi_sock_test_DEMO_54.h"
#endif
#if SSL
#include "servercert.pem" //! Include SSL CA certificate
#endif
#if CHECK_WIFI_POWER_STATE
uint8_t wlan_power_save_given = 0;
#endif
//! Enumeration for states in application
typedef enum rsi_wlan_app_state_e {
  RSI_WLAN_INITIAL_STATE     = 0,
  RSI_WLAN_UNCONNECTED_STATE = 1,
#if (STA_SCAN_CONN_DISCNCT_TEST || CHECK_WIFI_POWER_STATE)
  RSI_WLAN_SCAN_DONE_STATE     = 2,
  RSI_WLAN_CONNECTED_STATE     = 3,
  RSI_WLAN_IPCONFIG_DONE_STATE = 4,
  RSI_WLAN_DISCONNECTED_STATE  = 5
#else
  RSI_WLAN_CONNECTED_STATE        = 2,
  RSI_WLAN_IPCONFIG_DONE_STATE    = 3,
  RSI_WLAN_SOCKET_CONNECTED_STATE = 4,
  RSI_POWER_SAVE_STATE            = 5
#endif
} rsi_wlan_app_state_t;

//! WLAN application control block
typedef struct rsi_wlan_app_cb_s {
  rsi_wlan_app_state_t state;       //! WLAN application state
  uint32_t length;                  //! length of buffer to copy
  uint8_t buffer[RSI_APP_BUF_SIZE]; //! application buffer
  uint8_t buf_in_use;               //! to check application buffer availability
  uint32_t event_map;               //! application events bit map
} rsi_wlan_app_cb_t;

#if (MULTITHREADED_HTTP_DOWNLOAD_TEST || MULTITHREADED_TCP_TX_TEST)
#define RSI_HTTP_SOCKET_TASK_STACK_SIZE 1024
#define RSI_HTTP_SOCKET_TASK_PRIORITY   2
rsi_task_handle_t http_socket_task_handle[SOCKTEST_INSTANCES_MAX] = { NULL };
extern rsi_semaphore_handle_t suspend_sem;
extern socktest_ctx_t socktest_ctx[SOCKTEST_INSTANCES_MAX];
#elif STA_SCAN_CONN_DISCNCT_TEST
//! WLAN application control block
struct rsi_wlan_con_discon_s {
  int8_t SSID_l[32]; //! application buffer
  int8_t PSK_l[64];
  rsi_security_mode_t SECURITY_TYPE;
};

struct rsi_wlan_con_discon_s rsi_wlan_con_discon[RSI_MAX_NO_OF_AP_TO_CONNECT] = {
  { "powersave_test", "testtest", RSI_WPA2 }, { "kill", "12345678", RSI_WPA2 }, { "rps", "12345678", RSI_WPA2 },
  { "Hotspot", "12345678", RSI_WPA2 },        { "Name", "12345678", RSI_WPA2 }, { "scan", "12345678", RSI_WPA2 },
  { "vivo", "12345678", RSI_WPA2 },           { "run", "12345678", RSI_WPA2 },  { "mark", "12345678", RSI_WPA2 },
  { "bssid", "12345678", RSI_OPEN }
};
#endif
/*=======================================================================*/
//   ! VARIABLES
/*=======================================================================*/
rsi_wlan_app_cb_t rsi_wlan_app_cb;               //! application control block
int32_t client_socket;                           //! client socket id
struct rsi_sockaddr_in server_addr, client_addr; //! server and client IP addresses

//! Throughput parameters
#if THROUGHPUT_EN
uint32_t pkts       = 0;
uint64_t num_bits   = 0;
uint64_t total_bits = 0;
uint32_t xfer_time;
uint32_t total_time = 0;
uint64_t xfer_time_usec;
uint32_t t_start = 0;
uint32_t t_end;
float throughput;
float throughput_mbps;
#endif

volatile uint8_t data_recvd = 0;
volatile uint64_t num_bytes = 0;

/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/
#if SSL_RX_MAX_THROUGHPUT_TEST
extern uint8_t tx_rx_Completed;
#endif
/*************************************************************************/
//!  CALLBACK FUNCTIONS
/*************************************************************************/
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
  num_bytes += length;
}

#if SSL_RX_MAX_THROUGHPUT_TEST
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
  num_bits   = num_bytes * 8;                   //! number of bits
  xfer_time  = t_end - t_start;                 //! data transfer time
  throughput = ((float)(num_bits) / xfer_time); //!Throughput calculation

  LOG_PRINT("Time taken in msec: %d\n", xfer_time);
  LOG_PRINT("Throughput: %d kbps \r\n", (uint32_t)(num_bits / xfer_time));
#ifdef RSI_WITH_OS
  while (1)
    ;
#endif
}
#endif
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
    LOG_PRINT("CA cert erase failed\n");
    return status;
  }

  status =
    rsi_wlan_set_certificate(RSI_SSL_CA_CERTIFICATE, servercert, (sizeof(servercert) - 1)); //! Load SSL CA certificate
  if (status != RSI_SUCCESS) {
    LOG_PRINT("CA cert load failed\n");
    return status;
  }
  return status;
}
#endif

/*====================================================*/
/**
 * @fn         int32_t rsi_app_wlan_socket_create()
 * @brief      Function to create socket
 * @param[in]  void
 * @return     int32_t
 *====================================================*/
int32_t rsi_app_wlan_socket_create()
{
  int32_t status = RSI_SUCCESS;

#if SOCKET_ASYNC_FEATURE
#if SSL
  client_socket = rsi_socket_async(AF_INET, SOCK_STREAM, 1, socket_async_recive);
#else
  client_socket = rsi_socket_async(AF_INET, SOCK_STREAM, 0, socket_async_recive);
#endif
#else
  client_socket = rsi_socket(AF_INET, SOCK_STREAM, 0);
#endif

  if (client_socket < 0) {
    LOG_PRINT("socket open failed\n");
    status = rsi_wlan_get_status();
    return status;
  }

  LOG_PRINT("socket create\n");

  status = rsi_setsockopt(client_socket, SOL_SOCKET, SO_HIGH_PERFORMANCE_SOCKET, NULL, 0);
  if (status != RSI_SUCCESS) {
    return status;
  }

  memset(&server_addr, 0, sizeof(server_addr)); //! Reset server structure
  server_addr.sin_family = AF_INET;             //! Set server address family
  server_addr.sin_port   = htons(SERVER_PORT); //! Set server port number, using htons function to use proper byte order
  server_addr.sin_addr.s_addr = ip_to_reverse_hex((char *)SERVER_IP_ADDRESS);

  LOG_PRINT("socket connect\n");

  status =
    rsi_connect(client_socket, (struct rsi_sockaddr *)&server_addr, sizeof(server_addr)); //! Connect to server socket
  if (status != RSI_SUCCESS) {
    status = rsi_wlan_get_status();
    rsi_shutdown(client_socket, 0);
    LOG_PRINT("socket connect failed\n");
    return status;
  }

  return status;
}

/*====================================================*/
/**
 * @fn         int32_t  rsi_app_task_WIFI_1(void)
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
#if STA_SCAN_CONN_DISCNCT_TEST
  int8_t i = 0, j = 0, retry = 0, retry_cnt = 0;
#elif (MULTITHREADED_HTTP_DOWNLOAD_TEST || MULTITHREADED_TCP_TX_TEST)
  uint32_t i;
  volatile uint8_t download_complete = 0;
  uint8_t no_of_iterations = 0;
#endif

#if CHECK_WIFI_POWER_STATE
  status = rsi_bt_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
  if (status != RSI_SUCCESS) {
    return status;
  }
  status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
  if (status != RSI_SUCCESS) {
    return status;
  }
  LOG_PRINT("\n In disconnected sleep \n");
  rsi_delay_ms(30000);
#endif

#if !STA_SCAN_CONN_DISCNCT_TEST
  while (1) {
    switch (rsi_wlan_app_cb.state) {
      case RSI_WLAN_INITIAL_STATE: {
#if !(MULTITHREADED_HTTP_DOWNLOAD_TEST || MULTITHREADED_TCP_TX_TEST)
        rsi_wlan_app_callbacks_init(); //! register callback to initialize WLAN
#endif
        rsi_wlan_app_cb.state = RSI_WLAN_UNCONNECTED_STATE; //! update WLAN application state to unconnected state
      }
      //no break
      case RSI_WLAN_UNCONNECTED_STATE: {
#if CHECK_WIFI_POWER_STATE

        LOG_PRINT("\n start scanning \n");
        status = rsi_wlan_scan((int8_t *)SSID, (uint8_t)CHANNEL_NO, NULL, 0);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("WLAN scan failed\n");
          break;
        } else {
          rsi_wlan_app_cb.state = RSI_WLAN_SCAN_DONE_STATE;

          LOG_PRINT("WLAN scan done state\n");

          rsi_delay_ms(30000);
        }
#elif WIFI_STA_AND_SSL_CONN_DISCONN_TEST
#if SECURE_CON_DISCON
        while (1) {
          do {
            status = rsi_wlan_connect((int8_t *)SSID, SECURITY_TYPE, PSK); //! Connect to an Access point
          } while (status != RSI_SUCCESS);

          if (status != RSI_SUCCESS) {
            LOG_PRINT("WLAN connection failed\n");
            break;
          } else {
            LOG_PRINT("WLAN connected successfully\n");
          }

          status = rsi_wlan_disconnect();
          if (status != RSI_SUCCESS) {
            LOG_PRINT("WLAN disconnection failed\n");
            break;
          }
          LOG_PRINT("WLAN disconnected successfully\n");
        }
#endif
#if TCP_TLS_CON_DISCON
        status = rsi_wlan_connect((int8_t *)SSID, SECURITY_TYPE, PSK); //! Connect to an Access point
        if (status != RSI_SUCCESS) {
          LOG_PRINT("WLAN connection failed\n");
          break;
        } else {
          rsi_wlan_app_cb.state = RSI_WLAN_CONNECTED_STATE; //! update WLAN application state to connected state
          LOG_PRINT("WLAN connected state\n");
        }
#endif
#elif (SSL_RX_MAX_THROUGHPUT_TEST || MULTITHREADED_HTTP_DOWNLOAD_TEST || MULTITHREADED_TCP_TX_TEST)
        status = rsi_wlan_connect((int8_t *)SSID, SECURITY_TYPE, PSK); //! Connect to an Access point
        if (status != RSI_SUCCESS) {
          LOG_PRINT("WLAN connection failed\n");
          break;
        } else {
          rsi_wlan_app_cb.state = RSI_WLAN_CONNECTED_STATE; //! update WLAN application state to connected state
          LOG_PRINT("WLAN connected state\n");
        }
#elif WIFI_DEEPSLEEP_STANDBY_TEST
#if WLAN_DEEPSLEEP
        //! Apply wlan power save profile
        status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          break;
        }

        LOG_PRINT("In Disconnected Sleep mode...\n");
        rsi_wlan_app_cb.state = RSI_POWER_SAVE_STATE;
        break;
#endif
#if WLAN_STANDBY
        //! Connect to an Access point
        status = rsi_wlan_connect((int8_t *)SSID, SECURITY_TYPE, PSK);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("wlan connection failed\n");
          break;
        } else {
          LOG_PRINT("wlan connected \n");

          //!Broadcast filter
          status = rsi_wlan_filter_broadcast(5000, 1, 1);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("Filter broadcast failure\n");
            break;
          }

          //! Apply power save profile with standby
          status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("power save failure\n");
            break;
          }
          LOG_PRINT("In Standby mode...\n");
          rsi_wlan_app_cb.state = RSI_POWER_SAVE_STATE;
          break;
        }
#endif
#endif //! end of WIFI_DEEPSLEEP_STANDBY_TEST
      }
      //no break
#if CHECK_WIFI_POWER_STATE
      case RSI_WLAN_SCAN_DONE_STATE: {
        //! Connect to an Access point
        status = rsi_wlan_connect((int8_t *)SSID, SECURITY_TYPE, PSK);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("wlan connection failed\n");
          break;
        } else {
          LOG_PRINT("wlan connected state\n");
          LOG_PRINT("\n in standby \n");
          //! wait for 10 sec and disconnect from connected accesspoint
          rsi_delay_ms(10000);
          status = rsi_wlan_disconnect();
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n Failed to disconnect from accesspoint %s \n", SSID);
          } else {
            //! update wlan application state
            rsi_wlan_app_cb.state = RSI_WLAN_DISCONNECTED_STATE;
          }
        }
        break;
      }

      case RSI_WLAN_DISCONNECTED_STATE: {
        LOG_PRINT("\n disconnected from accesspoint: %s\n", SSID);

        //! Keep TA in sleep
        status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          return status;
        }
        LOG_PRINT("\n in disconnected sleep \n");
        rsi_delay_ms(30000);
        while (1)
          ;
      }
#endif
      case RSI_WLAN_CONNECTED_STATE: {
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
          LOG_PRINT("IP Config failed\n");
          break;
        } else {
          rsi_wlan_app_cb.state = RSI_WLAN_IPCONFIG_DONE_STATE;
          LOG_PRINT("WLAN ipconfig done state\n");
          LOG_PRINT("RSI_STA IP ADDR: %d.%d.%d.%d\n", ip[6], ip[7], ip[8], ip[9]);
        }
      }
      //no break
      case RSI_WLAN_IPCONFIG_DONE_STATE: {
#if SSL_RX_MAX_THROUGHPUT_TEST
        if (data_recvd) {
          rsi_shutdown(client_socket, 0);
          LOG_PRINT("TCP/SSL client closed\n");
          LOG_PRINT("TCP/SSL RX test end\n");
          data_recvd = 0; //! Clear data receive flag
          compute_throughput();
          tx_rx_Completed = 1;
          break;
        }
#endif

#if !(MULTITHREADED_HTTP_DOWNLOAD_TEST || MULTITHREADED_TCP_TX_TEST)
#if (SSL && LOAD_CERTIFICATE)
        status = rsi_app_load_ssl_cert(); //Function to load certificate
        if (status != RSI_SUCCESS) {
          break;
        }
#endif
#endif
#if HIGH_PERFORMANCE_ENABLE
        status = rsi_socket_config();
        if (status < 0) {
          LOG_PRINT("\r\nhigh-performance socket config failed");
          status = rsi_wlan_get_status();
          break;
        }
        LOG_PRINT("\r\nhigh-performance socket config success");
#endif
#if SSL_RX_MAX_THROUGHPUT_TEST
        status = rsi_app_wlan_socket_create(); //! Create socket
        if (status != RSI_SUCCESS) {
          break;
        }
        t_start               = rsi_hal_gettickcount();
        rsi_wlan_app_cb.state = RSI_WLAN_SOCKET_CONNECTED_STATE; //! update WLAN application state
        LOG_PRINT("WLAN socket connected state\n");
        LOG_PRINT("TCP/SSL RX test started...\n");

#elif (MULTITHREADED_HTTP_DOWNLOAD_TEST || MULTITHREADED_TCP_TX_TEST)
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
            LOG_PRINT("\n Thread creation failed %d", socktest_ctx[i].threadid);
            while (1)
              ;
          }
        }
        for (i = 0; i < SOCKTEST_INSTANCES_MAX; i++) {
          rsi_semaphore_post(&socktest_ctx[i].http_soc_wait_sem);
        }

        do {
          rsi_semaphore_wait(&suspend_sem, 0);
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
          no_of_iterations++;
          if (no_of_iterations == NO_OF_ITERATIONS) {
            for (i = 0; i < SOCKTEST_INSTANCES_MAX; i++) {
              LOG_PRINT("\n Thread id %d", socktest_ctx[i].threadid);
              LOG_PRINT("\n Tests Success %d", socktest_ctx[i].num_successful_test);
              LOG_PRINT("\n Tests failed %d", socktest_ctx[i].num_failed_test);
            }

            LOG_PRINT("\n Demo completed");
            //Demo completed
            while (1)
              ;
          }
          rsi_wlan_app_cb.state = RSI_WLAN_IPCONFIG_DONE_STATE;
        }
        break;
#elif WIFI_STA_AND_SSL_CONN_DISCONN_TEST
        while (1) {
          rsi_app_wlan_socket_create();
          if (status != RSI_SUCCESS) {
            status = rsi_wlan_get_status();
            rsi_shutdown(client_socket, 0);
            LOG_PRINT("SSL Socket connection failed\n");
            break;
          } else {
            LOG_PRINT("SSL Socket connected\n");
            rsi_shutdown(client_socket, 0);
            LOG_PRINT("SSL Socket disconnected\n");
          }
        }
        break;
#endif
      }
#if !CHECK_WIFI_POWER_STATE
      case RSI_WLAN_SOCKET_CONNECTED_STATE: {
        break;
      }
      //no break
      case RSI_POWER_SAVE_STATE: {
        break;
      }
#endif
      default:
        break;
    }
  }
#else
  LOG_PRINT("\n started demo \n");
  //! code for scan/connect/disconnect test
  while (j < RSI_MAX_NO_OF_ITERATIONS) {
    i = 0;
    for (i = 0; i < RSI_MAX_NO_OF_AP_TO_CONNECT; i++) {
      do {

        switch (rsi_wlan_app_cb.state) {
          case RSI_WLAN_INITIAL_STATE: {
            rsi_wlan_app_callbacks_init();                      //! register call backs
            rsi_wlan_app_cb.state = RSI_WLAN_UNCONNECTED_STATE; //! update wlan application state
          }
          //no break
          case RSI_WLAN_UNCONNECTED_STATE: {
            status = rsi_wlan_scan((int8_t *)rsi_wlan_con_discon[i].SSID_l, (uint8_t)CHANNEL_NO, NULL, 0);
            if (status != RSI_SUCCESS) {
              if ((retry == 0) && (retry_cnt == 0)) {
                retry = 1;
                //!Need to retry scan for same AP
                retry_cnt = RSI_NO_OF_RETRIES;
              }
              LOG_PRINT("WLAN scan failed\n");
              break;
            } else {
              retry = 0;
              retry_cnt = 0;
              rsi_wlan_app_cb.state = RSI_WLAN_SCAN_DONE_STATE;
              LOG_PRINT("WLAN scan done state\n");
            }
          }
          //no break
          case RSI_WLAN_SCAN_DONE_STATE: {
            status = rsi_wlan_connect((int8_t *)rsi_wlan_con_discon[i].SSID_l,
                                      rsi_wlan_con_discon[i].SECURITY_TYPE,
                                      rsi_wlan_con_discon[i].PSK_l);
            if (status != RSI_SUCCESS) {
              if ((retry == 0) && (retry_cnt == 0)) {
                retry = 1;
                //!Need to retry connection to same AP
                retry_cnt = RSI_NO_OF_RETRIES;
              }
              LOG_PRINT("WLAN connection failed\n");
              break;
            } else {
              retry = 0;
              retry_cnt = 0;
              rsi_wlan_app_cb.state = RSI_WLAN_CONNECTED_STATE;
              LOG_PRINT("WLAN connected state\n");
            }
          }
          //no break
          case RSI_WLAN_CONNECTED_STATE: {
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
              if ((retry == 0) && (retry_cnt == 0)) {
                retry = 1;
                //!Need to retry DHCP to same AP
                retry_cnt = RSI_NO_OF_RETRIES;
              }
              LOG_PRINT("IP Config failed\n");
              break;
            } else {
              retry = 0;
              retry_cnt = 0;
              rsi_wlan_app_cb.state = RSI_WLAN_IPCONFIG_DONE_STATE; //! update wlan application state
              LOG_PRINT("wlan ipconfig done state\n");
              LOG_PRINT("RSI_STA IP ADDR: %d.%d.%d.%d\n", ip[6], ip[7], ip[8], ip[9]);
            }
          }
          //no break
          case RSI_WLAN_IPCONFIG_DONE_STATE: {
            status = rsi_wlan_disconnect();
            if (status != RSI_SUCCESS) {
              if ((retry == 0) && (retry_cnt == 0)) {
                retry = 1;
                //!Need to retry Disconnect to same AP
                retry_cnt = RSI_NO_OF_RETRIES;
              }
              LOG_PRINT("WLAN disconnection failed\n");
              break;
            } else {
              retry = 0;
              retry_cnt = 0;
              LOG_PRINT("WLAN disconnected successfully\n");
              rsi_wlan_app_cb.state = RSI_WLAN_INITIAL_STATE; //! update wlan application state
            }
          } break;
          default:
            break;
        }
        if (retry_cnt > 0)
          retry_cnt--;
        if ((retry == 1) && (retry_cnt == 0) && (rsi_wlan_app_cb.state == RSI_WLAN_CONNECTED_STATE)) {
          rsi_wlan_app_cb.state = RSI_WLAN_IPCONFIG_DONE_STATE;
          retry_cnt = 1;
        }

      } while (retry_cnt);
      retry = 0;
      rsi_wlan_app_cb.state = RSI_WLAN_INITIAL_STATE; //! update wlan application state
#if RSI_STATUS_RETURN_ON_FAILURE
      if (status != RSI_SUCCESS) {
        LOG_PRINT("error status = 0x%x\n", status);
        while (1)
          ;
      }
#endif
    }
    j++;
  }
  if (j == RSI_MAX_NO_OF_ITERATIONS) {
    LOG_PRINT("DEMO Complete\n");
    while (1)
      ;
  }
#endif
  return status;
}

#if (MULTITHREADED_HTTP_DOWNLOAD_TEST || MULTITHREADED_TCP_TX_TEST)
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
#else
/*====================================================*/
/**
 * @fn         void rsi_join_fail_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
 * @brief      Callback handler in station mode at rejoin failure
 * @param[in]  uint16_t status, uint8_t *buffer, const uint32_t length
 * @return     void
 *=====================================================*/
void rsi_join_fail_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
{
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
#if SSL_RX_MAX_THROUGHPUT_TEST
  data_recvd = 1;                                       //Set data receive flag
#if (THROUGHPUT_EN)
  t_end = rsi_hal_gettickcount();                       //! capture time-stamp after data transfer is completed
#endif
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
}
#endif
#endif
