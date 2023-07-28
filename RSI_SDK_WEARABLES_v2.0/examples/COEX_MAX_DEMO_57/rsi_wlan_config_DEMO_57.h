/*******************************************************************************
* @file  rsi_wlan_config_DEMO_57.h
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
 * @file         rsi_wlan_config_DEMO_57.h
 * @version      0.1
 * @date         01 Feb 2020
 *
 *
 *
 *  @brief : This file contains user configurable details to configure the device  
 *
 *  @section Description  This file contains user configurable details to configure the device 
 */
#ifndef SAPIS_EXAMPLES_RSI_DEMO_APPS_INC_RSI_WLAN_CONFIG_COEX_MAX_APP_H
#define SAPIS_EXAMPLES_RSI_DEMO_APPS_INC_RSI_WLAN_CONFIG_COEX_MAX_APP_H

/*=======================================================================*/
//  ! INCLUDES
/*=======================================================================*/
#include <rsi_common_app.h>
#include "http_client.h"
#if COEX_MAX_APP
#include <rsi_bt.h>
/***********************************************************************************************************************************************/
//! APP CONFIG defines
/***********************************************************************************************************************************************/
#define SSID          "Tplink" //! Access point SSID to connect
#define SECURITY_TYPE RSI_WPA2 //! Security type -  RSI_WPA2 or RSI_OPEN
#define CHANNEL_NO    0
#define PSK           "12345678" //! Password
#define DHCP_MODE     1          //! DHCP mode 1- Enable 0- Disable
//! If DHCP mode is disabled give IP statically
#if !(DHCP_MODE)
#define DEVICE_IP 0x6500A8C0 //! IP address of the module  - E.g: 0x650AA8C0 == 192.168.0.101
#define GATEWAY   0x0100A8C0 //! IP address of Gateway  - E.g: 0x010AA8C0 == 192.168.0.1
#define NETMASK   0x00FFFFFF //! IP address of netmask - E.g: 0x00FFFFFF == 255.255.255.0
#endif
#define SERVER_PORT       5001            //! Server port number for SSL_RX_DATA and TX_DATA
#define SERVER_IP_ADDRESS "192.168.75.27" //"192.168.241.27" //! Server IP address
#define RSI_DNS_CLIENT    0               //! Enable if using DNS client (when using server hostname instead of ip addr)

#define SSL_TX_DATA 0 //! Enable for SSL Data Transfer
#if SSL_TX_DATA
#define TX_DATA     1
#define MAX_TX_PKTS 10000
#else
#define TX_DATA 0 //! Enable this to test TCP transmit
#endif

#define SSL_RX_DATA 0 //! Enable for SSL RX Data Transfer
#if SSL_RX_DATA
#define RX_DATA 1
#else
#define RX_DATA 1 //! Enable this to test HTTP/S download
#endif

#define HTTPS_DOWNLOAD 0 //! Enable this to test HTTPS download and also set RX_DATA to '1'
#if HTTPS_DOWNLOAD
#if RSI_DNS_CLIENT
//! TO-DO: This feature is to be implemeted
#define DOWNLOAD_FILENAME "https://gaana.com"
#else
#define DOWNLOAD_FILENAME "dltest.txt" //! HTTPs resource name
#define BYTES_TO_RECEIVE  6144         //! size of file to download from remote server
#endif
#else
#define DOWNLOAD_FILENAME "dltestdata32.txt" //"15MB.mp3"
#define BYTES_TO_RECEIVE  1048576            //14919768 //! size of DOWNLOAD_FILENAME
#endif

//! TCP IP BYPASS feature check
#define RSI_TCP_IP_BYPASS \
  0 //RSI_DISABLE //! Enable to verify http download with LWIP stack (TCP_IP_BYPASS mode) otherwise disable this macro.

#if RSI_TCP_IP_BYPASS
#define CONTINUOUS_HTTP_DOWNLOAD      0
#define SOCKTEST_INSTANCES_MAX        1       //! No. of sockets to test
#define NO_OF_ITERATIONS              11      //! No. of iterations to repeat for each socket
#define BYTES_TO_TRANSMIT             1048576 //! size of file to send to remote server
#define SOCKET_ASYNC_FEATURE          0
#define WLAN_THROUGHPUT_ENABLE        0
#define SSL                           0 //! Enable SSL or not
#define LOAD_CERTIFICATE              0 //! Load certificate to device flash
#define CONFIGURE_TIMEOUT             0 //! Configuring timeout value
#define HIGH_PERFORMANCE_ENABLE       0
#define WLAN_SYNC_REQ                 1
#define RSI_HTTP_SOCKET_TASK_PRIORITY 2

#if (RSI_TCP_IP_BYPASS                                                                                           \
     && (SELECT_ON_WRITEFDS || USE_CONNECTION_CLOSE || TEST_SOCKET_SHUTDOWN || TLS_SNI_FEATURE || SOCK_NON_BLOCK \
         || WINDOW_UPDATE_FEATURE || PER_TEST_TX_ENABLE || PER_TEST_RX_ENABLE || SSL || LOAD_CERTIFICATE         \
         || CONFIGURE_TIMEOUT || HIGH_PERFORMANCE_ENABLE))
#error "Feature combinations are not supported as per the configuration of TCP_IP_BYPASS mode"
#endif

#else /*RSI_TCP_IP_BYPASS */
#define CONTINUOUS_HTTP_DOWNLOAD 1
#define SOCKTEST_INSTANCES_MAX   1       //! No. of sockets to test
#define NO_OF_ITERATIONS         1       //! No. of iterations to repeat for each socket
#define BYTES_TO_TRANSMIT        1048576 //! size of file to send to remote server
#define SOCKET_ASYNC_FEATURE     0
#define WLAN_THROUGHPUT_ENABLE   0
#define SSL                      0 //! Enable SSL or not
#define LOAD_CERTIFICATE         0 //! Load certificate to device flash
#define CONFIGURE_TIMEOUT        0 //! Configuring timeout value
#endif                             /*RSI_TCP_IP_BYPASS */

#define PING_TEST 0 //! Enable to test PING from Wlan otherwise disable to test https download.
#if PING_TEST
#define REMOTE_IP         0x0101A8C0 //! 192.168.1.1
#define PING_SIZE         20
#define PING_PACKET_COUNT 1000
#endif

#define HTTP_USER_AGENT_HEADER "User-Agent: redpine/1.0.4a"
#define HTTP_ACCEPT_HEADER     "Accept: */*"
#define RSI_APP_BUF_SIZE       1600
#if RSI_TCP_IP_BYPASS
#define THROUGHPUT_EN 0 //!Disabled by default
#else
#define THROUGHPUT_EN 0 //!Disabled by default
#endif
#define STRING_HELPER(x) #x // turns x into a string
// Two macros are required to convert a macro value to a string
// ex: #define THE_NUMBER_NINE 9
//     STRING_HELPER(THE_NUMBER_NINE) -> "THE_NUMBER_NINE"
//     TOSTRING(THE_NUMBER_NINE) -> STRING_HELPER(9) -> "9"
#define TOSTRING(x)                   STRING_HELPER(x)
#define MAX_DATA_PKTS                 100
#define SD_DEMO                       1
#define MAX_DATA_TO_WRITE_INTO_SDCARD (8 * 1024)
#define SD_WRITE_SZ                   (2 * 1024)
//! By default the app does scan,connect,ipdone,http/https downloads. To restrict the wlan scan only or wlan connect only case need to enable either or manner.
#define WLAN_SCAN_ONLY          0 //! make it 1 for wlan scan in loop.
#define WLAN_CONNECT_ONLY       0 //! make it 1 for be wlan as connected stand-by.
#define COMPUTE_WLAN_THROUGHPUT 1 //! for throughput calculations
/*=======================================================================*/
//!	Powersave configurations
/*=======================================================================*/
#if RSI_TCP_IP_BYPASS
#define ENABLE_POWER_SAVE 0 //! Set to 1 for powersave mode
#else
#if WLAN_THROUGHPUT_ENABLE
#define ENABLE_POWER_SAVE 0 //! Set to 1 for powersave mode
#else
#define ENABLE_POWER_SAVE 0 //! Set to 1 for powersave mode
#endif
#endif
#define PSP_MODE RSI_SLEEP_MODE_2
#define PSP_TYPE RSI_MAX_PSP

/*=======================================================================*/
//!WMM PS parameters
/*=======================================================================*/

#define RSI_WMM_PS_ENABLE        RSI_ENABLE //! set WMM enable or disable
#define RSI_WMM_PS_TYPE          1          //! set WMM enable or disable  //! 0- TX BASED 1 - PERIODIC
#define RSI_WMM_PS_WAKE_INTERVAL 30         //! set WMM wake up interval
#define RSI_WMM_PS_UAPSD_BITMAP  15         //! set WMM UAPSD bitmap

/*=======================================================================*/
//! Feature frame parameters
/*=======================================================================*/
#if (ENABLE_POWER_SAVE)
#define FEATURE_ENABLES \
  (RSI_FEAT_FRAME_PREAMBLE_DUTY_CYCLE | RSI_FEAT_FRAME_LP_CHAIN | RSI_FEAT_FRAME_IN_PACKET_DUTY_CYCLE)
#else
#define FEATURE_ENABLES 0
#endif

/***********************************************************************************************************************************************/
//! WLAN SAPI CONFIG DEFINES
/***********************************************************************************************************************************************/
#define RSI_ENABLE      1           //! Enable feature
#define RSI_DISABLE     0           //! Disable feature
#define CONCURRENT_MODE RSI_DISABLE //! To enable concurrent mode

/*=======================================================================*/
//! BT power control
/*=======================================================================*/
#define BT_PWR_CTRL_DISABLE 0
#define BT_PWR_CTRL         1
#define BT_PWR_INX          10

/*=======================================================================*/
//! Power save command parameters
/*=======================================================================*/
#define RSI_HAND_SHAKE_TYPE GPIO_BASED //! set handshake type of power mode

/*=======================================================================*/
//! Socket configuration
/*=======================================================================*/
#define TOTAL_SOCKETS       SOCKTEST_INSTANCES_MAX //@ Total number of sockets. TCP TX + TCP RX + UDP TX + UDP RX
#define TOTAL_TCP_SOCKETS   SOCKTEST_INSTANCES_MAX //@ Total TCP sockets. TCP TX + TCP R
#define TCP_RX_ONLY_SOCKETS SOCKTEST_INSTANCES_MAX //@ Total TCP RX only sockets. TCP RX
#if WLAN_THROUGHPUT_ENABLE
#define TCP_RX_WINDOW_SIZE_CAP   20 //@ TCP RX Window size - 64K (44 * 1460)
#define TCP_RX_WINDOW_DIV_FACTOR 0
#else
#define TCP_RX_WINDOW_SIZE_CAP   90 //@ TCP RX Window size - 64K (44 * 1460)
#define TCP_RX_WINDOW_DIV_FACTOR 90
#endif
#define BT_PACKET_TYPE_SEL         BT_EDR_3DH3_PACKETS_ONLY
#define BT_AIRPODS_PER_FEAT_ENABLE 1

/*=======================================================================*/
//! opermode command paramaters
/*=======================================================================*/
//! To set wlan feature select bit map
#define RSI_FEATURE_BIT_MAP (FEAT_ULP_GPIO_BASED_HANDSHAKE | FEAT_DEV_TO_HOST_ULP_GPIO_1 | FEAT_SECURITY_OPEN)

//! TCP/IP feature select bitmap for selecting TCP/IP features
#if PING_TEST
#define RSI_TCP_IP_FEATURE_BIT_MAP                                                                    \
  (TCP_IP_FEAT_DHCPV4_CLIENT | TCP_IP_FEAT_SSL | TCP_IP_FEAT_DNS_CLIENT | TCP_IP_FEAT_EXTENSION_VALID \
   | TCP_IP_FEAT_ICMP)
#else
#if RSI_TCP_IP_BYPASS
#define RSI_TCP_IP_FEATURE_BIT_MAP                                                                    \
  (TCP_IP_FEAT_DHCPV4_CLIENT | TCP_IP_FEAT_SSL | TCP_IP_FEAT_DNS_CLIENT | TCP_IP_FEAT_EXTENSION_VALID \
   | TCP_IP_FEAT_BYPASS)

#else
#define RSI_TCP_IP_FEATURE_BIT_MAP \
  (TCP_IP_FEAT_DHCPV4_CLIENT | TCP_IP_FEAT_SSL | TCP_IP_FEAT_DNS_CLIENT | TCP_IP_FEAT_EXTENSION_VALID)
#endif
#endif

#if BT_PWR_CTRL_DISABLE
#define RSI_BT_FEATURE_BITMAP                                                                            \
  (A2DP_PROFILE_ENABLE | A2DP_SOURCE_ROLE_ENABLE | BT_PACKET_TYPE_SEL | BT_RF_TYPE | ENABLE_BLE_PROTOCOL \
   | (BT_PWR_CTRL << 2) | (BT_PWR_INX << 3) | (BT_AIRPODS_PER_FEAT_ENABLE << 28))
#else
#define RSI_BT_FEATURE_BITMAP \
  (A2DP_PROFILE_ENABLE | A2DP_SOURCE_ROLE_ENABLE | BT_PACKET_TYPE_SEL | BT_RF_TYPE | ENABLE_BLE_PROTOCOL)
#endif

#if WLAN_THROUGHPUT_ENABLE
#define RSI_EXT_TCPIP_FEATURE_BITMAP                                                                           \
  (EXT_TCP_IP_WAIT_FOR_SOCKET_CLOSE | EXT_TCP_IP_CERT_BYPASS | EXT_DYNAMIC_COEX_MEMORY | EXT_TCP_IP_WINDOW_DIV \
   | EXT_TCP_IP_TOTAL_SELECTS_4 | EXT_TCP_IP_BI_DIR_ACK_UPDATE | EXT_TCP_IP_WINDOW_SCALING                     \
   | EXT_TCP_IP_SSL_16K_RECORD)
#else
#define RSI_EXT_TCPIP_FEATURE_BITMAP                                                                           \
  (EXT_TCP_IP_WAIT_FOR_SOCKET_CLOSE | EXT_TCP_IP_CERT_BYPASS | EXT_DYNAMIC_COEX_MEMORY | EXT_TCP_IP_WINDOW_DIV \
   | EXT_TCP_IP_TOTAL_SELECTS_4 | EXT_TCP_IP_BI_DIR_ACK_UPDATE)
#endif

//! To set custom feature select bit map
#define RSI_CUSTOM_FEATURE_BIT_MAP FEAT_CUSTOM_FEAT_EXTENTION_VALID

//! To set Extended custom feature select bit map
#if ENABLE_1P8V
#if (RSI_ENABLE_BT_TEST && TA_BASED_ENCODER)
#define RSI_EXT_CUSTOM_FEATURE_BIT_MAP (EXT_FEAT_XTAL_CLK_ENABLE | EXT_FEAT_384K_MODE | EXT_FEAT_1P8V_SUPPORT)
#else
#define RSI_EXT_CUSTOM_FEATURE_BIT_MAP \
  (EXT_FEAT_LOW_POWER_MODE | EXT_FEAT_XTAL_CLK_ENABLE | EXT_FEAT_384K_MODE | EXT_FEAT_1P8V_SUPPORT)
#endif
#else
#if (RSI_ENABLE_BT_TEST && TA_BASED_ENCODER)
#define RSI_EXT_CUSTOM_FEATURE_BIT_MAP (EXT_FEAT_XTAL_CLK_ENABLE | EXT_FEAT_384K_MODE)
#else
#define RSI_EXT_CUSTOM_FEATURE_BIT_MAP (EXT_FEAT_LOW_POWER_MODE | EXT_FEAT_XTAL_CLK_ENABLE | EXT_FEAT_384K_MODE)
#endif
#endif

//! including the common defines
#include "rsi_wlan_common_config.h"
#endif
#endif /* SAPIS_EXAMPLES_RSI_DEMO_APPS_INC_RSI_CONFIG_BLE_DUAL_MODE_BT_A2DP_SOURCE_WIFI_HTTP_S_RX_H */
