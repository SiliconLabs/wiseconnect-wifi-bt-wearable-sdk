/*******************************************************************************
* @file  rsi_wlan_config_DEMO_54.h
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

 * @file         rsi_wlan_config_DEMO_54.h
 * @version      0.1
 * @date         15 Aug 2015
 *
 *
 *
 *  @brief : This file contains user configurable details to configure the device  
 *
 *  @section Description  This file contains user configurable details to configure the device 
 */
#ifndef SAPIS_EXAMPLES_RSI_DEMO_APPS_INC_RSI_WLAN_CONFIG_UNIFIED_PROTOCOL_H
#define SAPIS_EXAMPLES_RSI_DEMO_APPS_INC_RSI_WLAN_CONFIG_UNIFIED_PROTOCOL_H

/*=======================================================================*/
//  ! INCLUDES
/*=======================================================================*/
#include <rsi_common_app.h>
#include "http_client.h"
#if UNIFIED_PROTOCOL
#include <rsi_bt.h>
#include <rsi_bt_config_DEMO_54.h>

#if (RSI_GENERAL_POWER_TEST && RSI_DEEP_SLEEP_WITH_RAM_RET_AFTER_RESET)
void rsi_wireless_driver_task_create();
#endif
/***********************************************************************************************************************************************/
//! BT_A2DP_SOURCE_WIFI_HTTP_S_RX APP CONFIG defines
/***********************************************************************************************************************************************/
#define RSI_APP_BUF_SIZE 1600

#if (RSI_ENABLE_WIFI_TEST && (SSL_RX_MAX_THROUGHPUT_TEST || MULTITHREADED_TCP_TX_TEST))
#define THROUGHPUT_EN 1 //!Disabled by default
#endif
#define STRING_HELPER(x) #x // turns x into a string

// Two macros are required to convert a macro value to a string
// ex: #define THE_NUMBER_NINE 9
//     STRING_HELPER(THE_NUMBER_NINE) -> "THE_NUMBER_NINE"
//     TOSTRING(THE_NUMBER_NINE) -> STRING_HELPER(9) -> "9"
#define TOSTRING(x) STRING_HELPER(x)

#define SSID "silk_1" //! Access point SSID to connect

#define SECURITY_TYPE RSI_WPA2 //! Security type -  RSI_WPA2 or RSI_OPEN

#define PSK "12345678" //! Password

#define CHANNEL_NO 0

#define DHCP_MODE 1 //! DHCP mode 1- Enable 0- Disable

//! If DHCP mode is disabled give IP statically
#if !(DHCP_MODE)

#define DEVICE_IP 0x6500A8C0 //! IP address of the module  - E.g: 0x650AA8C0 == 192.168.0.101

#define GATEWAY 0x0100A8C0 //! IP address of Gateway  - E.g: 0x010AA8C0 == 192.168.0.1

#define NETMASK 0x00FFFFFF //! IP address of netmask - E.g: 0x00FFFFFF == 255.255.255.0
#endif

#define SOCKET_ASYNC_FEATURE 1

#define SERVER_PORT 5001 //! Server port number

#define SERVER_IP_ADDRESS "192.168.0.5" //! Server IP address

#define HTTPSVR_ADDR (0x0500a8c0) // Alex laptop

#define RSI_DNS_CLIENT 1 //! Enable if using DNS client (when using server hostname instead of ip addr)

#if !RSI_DNS_CLIENT
#define DOWNLOAD_FILENAME "15MB.mp3"

#define HTTP_URL "http://" SERVER_IP_ADDRESS ":" TOSTRING(SERVER_PORT) "/" DOWNLOAD_FILENAME //! HTTP resource name
#else
#define SSL              1 //! Enable SSL or not

//! Load certificate to device flash :
//! Certificate could be loaded once and need not be loaded for every boot up
#define LOAD_CERTIFICATE 1

#define HTTP_URL "https://gaana.com/" //! HTTP resource name
#endif

#define HTTP_USER_AGENT_HEADER "User-Agent: redpine/1.0.4a"

#define HTTP_ACCEPT_HEADER "Accept: */*"

#define MAX_DATA_PKTS 100

#define AUDIO_FILE_NAME  "/Song1.bin"
#define AUDIO_FILE_NAME2 "/Song2.bin"
#define AUDIO_FILE_NAME3 "/Song3.bin"

#define CONTINUOUS_HTTP_DOWNLOAD 0

#define SD_DEMO 1

#define MAX_DATA_TO_WRITE_INTO_SDCARD (8 * 1024)

#define SD_WRITE_SZ (2 * 1024)

//!Powersave configurations
#define ENABLE_POWER_SAVE 0 //! Set to 1 for powersave mode

#if ENABLE_POWER_SAVE
#define MEASURE_AUDIO_POWER_NUMBER 1
#endif

#define PSP_MODE RSI_SLEEP_MODE_2
#define PSP_TYPE RSI_MAX_PSP
/*=======================================================================*/
//!WMM PS parameters
/*=======================================================================*/

#define RSI_WMM_PS_ENABLE        RSI_DISABLE //! set WMM enable or disable
#define RSI_WMM_PS_TYPE          0           //! set WMM enable or disable  //! 0- TX BASED 1 - PERIODIC
#define RSI_WMM_PS_WAKE_INTERVAL 20          //! set WMM wake up interval
#define RSI_WMM_PS_UAPSD_BITMAP  15          //! set WMM UAPSD bitmap

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
#if RSI_ENABLE_PROP_PROTOCOL_TEST
#define TCP_RX_WINDOW_SIZE_CAP   44 //@ TCP RX Window size - 64K (44 * 1460)
#define TCP_RX_WINDOW_DIV_FACTOR 44
#else
#define TCP_RX_WINDOW_SIZE_CAP   2 //@ TCP RX Window size - 64K (44 * 1460)
#define TCP_RX_WINDOW_DIV_FACTOR 0
#endif

#if (!ENABLE_POWER_SAVE)
#define BT_PACKET_TYPE_SEL 0
#elif (ENABLE_POWER_SAVE)
#if MEASURE_AUDIO_POWER_NUMBER
#define BT_PACKET_TYPE_SEL BT_EDR_2DH3_PACKETS_ONLY
#else
#define BT_PACKET_TYPE_SEL BT_EDR_2DH5_PACKETS_ONLY
#endif
#endif //ENABLE_POWER_SAVE
#endif

/*=======================================================================*/
//! opermode command paramaters
/*=======================================================================*/
//! To set wlan feature select bit map
#define RSI_FEATURE_BIT_MAP (FEAT_ULP_GPIO_BASED_HANDSHAKE | FEAT_DEV_TO_HOST_ULP_GPIO_1 | FEAT_SECURITY_OPEN)

//! TCP IP BYPASS feature check
#define RSI_TCP_IP_BYPASS RSI_DISABLE

//! TCP/IP feature select bitmap for selecting TCP/IP features
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

#define RSI_EXT_TCPIP_FEATURE_BITMAP \
  (EXT_DYNAMIC_COEX_MEMORY | EXT_TCP_IP_WINDOW_DIV | EXT_TCP_IP_TOTAL_SELECTS_4 | EXT_TCP_IP_BI_DIR_ACK_UPDATE)

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

#if (RSI_ENABLE_WIFI_TEST && (MULTITHREADED_HTTP_DOWNLOAD_TEST || MULTITHREADED_TCP_TX_TEST))
#define TOTAL_SOCKETS       4 //@ Total number of sockets. TCP TX + TCP RX + UDP TX + UDP RX
#define TOTAL_TCP_SOCKETS   4 //@ Total TCP sockets. TCP TX + TCP RX
#define TCP_RX_ONLY_SOCKETS 4 //@ Total TCP RX only sockets. TCP RX

#elif (RSI_ENABLE_WIFI_TEST && WIFI_STA_AND_SSL_CONN_DISCONN_TEST)
#define SECURE_CON_DISCON  1
#define TCP_TLS_CON_DISCON 0

#elif (RSI_ENABLE_WIFI_TEST && WIFI_DEEPSLEEP_STANDBY_TEST)
/***********************************************************************************************************************************************/
//! WIFI_DEEPSLEEP_STANDBY APP CONFIG defines
/***********************************************************************************************************************************************/

//!Wlan powersave configurations
/**
    --------------------------------------------------------------------------
    |          POWERSAVE MODE              |      Configurations             |
    --------------------------------------------------------------------------
    |    DEEPSLEEP with RAM retention      |    WLAN_DEEPSLEEP = 1           |
    | 									   |    DP_SLEEP_WITH_RAM_RET = 1    |
    |                                      |    WLAN_STANDBY = 0             |
    --------------------------------------------------------------------------
    |    DEEPSLEEP without RAM retention   |    WLAN_DEEPSLEEP = 1           |
    | 									   |    DP_SLEEP_WITH_RAM_RET = 0    |
    |                                      |    WLAN_STANDBY = 0             |
    --------------------------------------------------------------------------
    |            STANDBY                   |    WLAN_DEEPSLEEP = 0           |
    | 									   |    DP_SLEEP_WITH_RAM_RET = 0    |
    |                                      |    WLAN_STANDBY = 1             |
    --------------------------------------------------------------------------
 **/
//! Configure following MACROS as shown in the above table as per the Powersave Mode
#define WLAN_DEEPSLEEP        1
#define DP_SLEEP_WITH_RAM_RET 1
#define WLAN_STANDBY          0
/*=======================================================================*/
//! Opermode command parameters
/*=======================================================================*/

#if (WLAN_DEEPSLEEP)
/*=======================================================================*/
//! Power save command parameters
/*=======================================================================*/
#if !DP_SLEEP_WITH_RAM_RET
#define RSI_SELECT_LP_OR_ULP_MODE RSI_ULP_WITHOUT_RAM_RET
#endif
#endif

#elif (RSI_ENABLE_WIFI_TEST && STA_SCAN_CONN_DISCNCT_TEST)
/***********************************************************************************************************************************************/
//! WIFI_STA_SCAN_CONN_DISCONN APP CONFIG defines
/***********************************************************************************************************************************************/
#define SECURE_CON_DISCON            0
#define TCP_TLS_CON_DISCON           1
#define RSI_MAX_NO_OF_AP_TO_CONNECT  10
#define RSI_MAX_NO_OF_ITERATIONS     10
#define RSI_STATUS_RETURN_ON_FAILURE 0
#define RSI_NO_OF_RETRIES            2
#define PSECURITY_TYPE               2
#endif
//! including the common defines
#include "rsi_wlan_common_config.h"
#endif /* SAPIS_EXAMPLES_RSI_DEMO_APPS_INC_RSI_WLAN_CONFIG_UNIFIED_PROTOCOL_H */
