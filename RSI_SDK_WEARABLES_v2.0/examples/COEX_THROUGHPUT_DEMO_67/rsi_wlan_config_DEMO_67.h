/*******************************************************************************
* @file  rsi_wlan_config.h
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
 * @file         rsi_wlan_config.h
 * @version      0.1
 * @date         01 Feb 2020
 *
 *
 *
 *  @brief : This file contains user configurable details to configure the device
 *
 *  @section Description  This file contains user configurable details to configure the device
 */

#ifndef RSI_WLAN_CONFIG_H
#define RSI_WLAN_CONFIG_H

#include <rsi_common_app.h>
#if COEX_THROUGHPUT
#include "rsi_wlan_defines.h"
#include "rsi_os.h"
#include "rsi_common_config.h"

/*=======================================================================================================*/
//!	 APP CONFIG defines
/*=======================================================================================================*/
#define SSID               "Tplink"        //! Access point SSID to connect
#define SECURITY_TYPE      RSI_WPA2        //! Security type -  RSI_WPA2 or RSI_OPEN
#define CHANNEL_NO         0               //! Channel in which device should scan, 0 - scan all channels
#define PSK                "yelinki120891" //! Password
#define DEVICE_PORT        5001            //! Module port number
#define SERVER_PORT        5001            //! Server port number
#define SERVER_IP_ADDRESS  "192.168.0.102" //! Server IP address
#define SSL_RX_SERVER_PORT 5002            //! Server port number of SSL client
#define DHCP_MODE          1               //! DHCP mode 1- Enable 0- Disable
#if !(DHCP_MODE)                           //! If DHCP mode is disabled give IP statically
#define DEVICE_IP 0x6500A8C0               //! IP address of the module  - E.g: 0x650AA8C0 == 192.168.0.101
#define GATEWAY   0x0100A8C0               //! IP address of Gateway  - E.g: 0x010AA8C0 == 192.168.0.1
#define NETMASK   0x00FFFFFF               //! IP address of netmask - E.g: 0x00FFFFFF == 255.255.255.0
#endif

//! Type of throughput
#define UDP_RX            1
#define UDP_TX            2
#define TCP_TX            4
#define TCP_RX            8
#define SSL_TX            16
#define SSL_RX            32
#define UDP_BIDIRECTIONAL (UDP_TX + UDP_RX)
#define TCP_BIDIRECTIONAL (TCP_TX + TCP_RX)
#define SSL_BIDIRECTIONAL (SSL_TX + SSL_RX)

#define THROUGHPUT_TYPE TCP_TX //TCP_BIDIRECTIONAL

#define TOTAL_PROTOCOLS_CNT 6
#define TCP_BUFF_SIZE       1460
#define UDP_BUFF_SIZE       1470
#define SSL_BUFF_SIZE       1370
#define DEFAULT_BUFF_SIZE   1460

#define THROUGHPUT_AVG_TIME 60000 //! throughput average time in ms
#define MAX_TX_PKTS         10000 //! Applies in SSL TX and UDP_RX, calculate throughput after transmitting MAX_TX_PKTS
#define RSI_DNS_CLIENT      0     //! Enable if using DNS client (when using server hostname instead of ip addr)
#define TX_RX_RATIO_ENABLE  0
#define RSI_SSL_BIT_ENABLE  0

#define TX_DATA        0 //! Enable this to test TCP transmit
#define RX_DATA        1 //! Enable this to test HTTP/S download
#define HTTPS_DOWNLOAD 0 //! Enable this to test HTTPS download and also set RX_DATA to '1'
#if HTTPS_DOWNLOAD
#define SSL              1 //! Enable SSL or not
#define LOAD_CERTIFICATE 1 //! Load certificate to device flash :
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
#define CONTINUOUS_HTTP_DOWNLOAD 1
#define SOCKTEST_INSTANCES_MAX   1       //! No. of sockets to test
#define NO_OF_ITERATIONS         1       //! No. of iterations to repeat for each socket
#define BYTES_TO_TRANSMIT        1048576 //! size of file to send to remote server
#if WLAN_THROUGHPUT_TEST
#define SOCKET_ASYNC_FEATURE 1
#else
#define SOCKET_ASYNC_FEATURE 0
#endif
#define PING_TEST 0 //! Enable to test PING from Wlan otherwise disable to test https download.
#if PING_TEST
#define REMOTE_IP         0x0101A8C0 //! 192.168.1.1
#define PING_SIZE         20
#define PING_PACKET_COUNT 1000
#endif

#define HTTP_USER_AGENT_HEADER "User-Agent: redpine/1.0.4a"
#define HTTP_ACCEPT_HEADER     "Accept: */*"
#define RSI_APP_BUF_SIZE       1600
#define THROUGHPUT_EN          0  //!Disabled by default
#define STRING_HELPER(x)       #x // turns x into a string
// Two macros are required to convert a macro value to a string
// ex: #define THE_NUMBER_NINE 9
//     STRING_HELPER(THE_NUMBER_NINE) -> "THE_NUMBER_NINE"
//     TOSTRING(THE_NUMBER_NINE) -> STRING_HELPER(9) -> "9"
#define TOSTRING(x)                   STRING_HELPER(x)
#define MAX_DATA_PKTS                 100
#define SD_DEMO                       1
#define MAX_DATA_TO_WRITE_INTO_SDCARD (8 * 1024)
#define SD_WRITE_SZ                   (2 * 1024)
#define HTTP_DOWNLOAD_THROUGPUT_TEST  1 //! enable this for measuring http download throughput
/*=======================================================================*/
//!	Powersave configurations
/*=======================================================================*/
#define ENABLE_POWER_SAVE 0 //! Set to 1 for powersave mode
#define PSP_MODE          RSI_SLEEP_MODE_2
#define PSP_TYPE          RSI_MAX_PSP

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
#define TCP_RX_WINDOW_SIZE_CAP   20 //@ TCP RX Window size - 64K (44 * 1460)
#define TCP_RX_WINDOW_DIV_FACTOR 0

#define BT_PACKET_TYPE_SEL BT_EDR_2DH3_PACKETS_ONLY

/*=======================================================================*/
//! opermode command paramaters
/*=======================================================================*/
//! To set wlan feature select bit map
#define RSI_FEATURE_BIT_MAP \
  (FEAT_ULP_GPIO_BASED_HANDSHAKE | FEAT_DEV_TO_HOST_ULP_GPIO_1 | FEAT_SECURITY_OPEN /*| FEAT_AGGREGATION*/)

//! TCP IP BYPASS feature check
#define RSI_TCP_IP_BYPASS RSI_DISABLE

#define RSI_TCP_IP_FEATURE_BIT_MAP \
  (TCP_IP_FEAT_DHCPV4_CLIENT | TCP_IP_FEAT_SSL | TCP_IP_FEAT_DNS_CLIENT | TCP_IP_FEAT_EXTENSION_VALID)

#if BT_PWR_CTRL_DISABLE
#define RSI_BT_FEATURE_BITMAP                                                                            \
  (A2DP_PROFILE_ENABLE | A2DP_SOURCE_ROLE_ENABLE | BT_PACKET_TYPE_SEL | BT_RF_TYPE | ENABLE_BLE_PROTOCOL \
   | (BT_PWR_CTRL << 2) | (BT_PWR_INX << 3))
#else
#define RSI_BT_FEATURE_BITMAP \
  (A2DP_PROFILE_ENABLE | A2DP_SOURCE_ROLE_ENABLE | BT_PACKET_TYPE_SEL | BT_RF_TYPE | ENABLE_BLE_PROTOCOL)
#endif
#define RSI_EXT_TCPIP_FEATURE_BITMAP                                                      \
  (BIT(16) | EXT_DYNAMIC_COEX_MEMORY | EXT_TCP_IP_WINDOW_DIV | EXT_TCP_IP_TOTAL_SELECTS_4 \
   | EXT_TCP_IP_BI_DIR_ACK_UPDATE)

//! Enable CUSTOM_FEAT_SOC_CLK_CONFIG_160MHZ only in Wlan mode
//! To set custom feature select bit map
#if (RSI_ENABLE_BT_TEST || RSI_ENABLE_BLE_TEST)
#define RSI_CUSTOM_FEATURE_BIT_MAP FEAT_CUSTOM_FEAT_EXTENTION_VALID
#else
#define RSI_CUSTOM_FEATURE_BIT_MAP FEAT_CUSTOM_FEAT_EXTENTION_VALID | CUSTOM_FEAT_SOC_CLK_CONFIG_160MHZ
#endif
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

//! Enumeration for states in application
typedef enum rsi_wlan_app_state_e {
  RSI_WLAN_INITIAL_STATE          = 0,
  RSI_WLAN_UNCONNECTED_STATE      = 1,
  RSI_WLAN_SCAN_DONE_STATE        = 2,
  RSI_WLAN_CONNECTED_STATE        = 3,
  RSI_WLAN_IPCONFIG_DONE_STATE    = 4,
  RSI_WLAN_DISCONNECTED_STATE     = 5,
  RSI_WLAN_SOCKET_CONNECTED_STATE = 6,
  RSI_POWER_SAVE_STATE            = 7,
  RSI_WLAN_IDLE_STATE             = 8,

} rsi_wlan_app_state_t;

//! WLAN application control block
typedef struct rsi_wlan_app_cb_s {
  rsi_wlan_app_state_t state;       //! WLAN application state
  uint32_t length;                  //! length of buffer to copy
  uint8_t buffer[RSI_APP_BUF_SIZE]; //! application buffer
  uint8_t buf_in_use;               //! to check application buffer availability
  uint32_t event_map;               //! application events bit map
} rsi_wlan_app_cb_t;

typedef struct wlan_throughput_config_s {
  rsi_semaphore_handle_t wlan_app_sem;
  rsi_semaphore_handle_t wlan_app_sem1;
  uint8_t thread_id;
  uint8_t throughput_type;
  //uint32_t server_port;
  //uint8_t *server_addr;
} wlan_throughput_config_t;

#if 0
/*=======================================================================*/
//! Feature frame parameters
/*=======================================================================*/
#if (RSI_ENABLE_BT_TEST || RSI_ENABLE_BLE_TEST)
#define PLL_MODE 0
#else
#define PLL_MODE 1
#endif

#define RF_TYPE       1 //! 0 - External RF 1- Internal RF
#define WIRELESS_MODE 0
#define ENABLE_PPP    0
#define AFE_TYPE      1

/*=======================================================================*/
//! High Throughput Capabilies related information
/*=======================================================================*/
//! HT caps supported
//! HT caps bit map.
#define RSI_HT_CAPS_BIT_MAP \
  (RSI_HT_CAPS_NUM_RX_STBC | RSI_HT_CAPS_SHORT_GI_20MHZ | RSI_HT_CAPS_GREENFIELD_EN | RSI_HT_CAPS_SUPPORT_CH_WIDTH)
/*=======================================================================*/
//! Rejoin parameters
/*=======================================================================*/
//! RSI_ENABLE or RSI_DISABLE rejoin params
#define RSI_REJOIN_PARAMS_SUPPORT RSI_ENABLE
//! Rejoin retry count. If 0 retries infinity times
#define RSI_REJOIN_MAX_RETRY      1
#endif
#include <rsi_wlan_common_config.h>
#endif
#endif
