/*******************************************************************************
* @file  rsi_general_power_num_Demo_54.c
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
 * @file    rsi_general_power_num_Demo_54.c
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
#if (UNIFIED_PROTOCOL && RSI_GENERAL_POWER_TEST)
#include "stdlib.h"
#include "rsi_driver.h"
#include "rsi_bt_common_apis.h"
#include <rsi_wlan_non_rom.h>
#include "rsi_wlan_config_DEMO_54.h"
#ifdef RSI_WITH_OS
#include "rsi_os.h"
extern rsi_task_handle_t driver_task_handle;
#endif

static uint8_t local_dev_addr[RSI_DEV_ADDR_LEN] = { 0 };
uint8_t fw[20]                                  = { 0 };
//! Enumeration for states in application
typedef enum rsi_wlan_app_state_e {
  RSI_WLAN_INITIAL_STATE       = 0,
  RSI_WLAN_UNCONNECTED_STATE   = 1,
  RSI_WLAN_CONNECTED_STATE     = 2,
  RSI_WLAN_IPCONFIG_DONE_STATE = 3,
  RSI_POWER_SAVE_STATE         = 4,
} rsi_wlan_app_state_t;

//! WLAN application control block
typedef struct rsi_wlan_app_cb_s {
  rsi_wlan_app_state_t state;       //! WLAN application state
  uint32_t length;                  //! length of buffer to copy
  uint8_t buffer[RSI_APP_BUF_SIZE]; //! application buffer
  uint8_t buf_in_use;               //! to check application buffer availability
  uint32_t event_map;               //! application events bit map
} rsi_wlan_app_cb_t;

/*=======================================================================*/
//   ! VARIABLES
/*=======================================================================*/
rsi_wlan_app_cb_t rsi_wlan_app_cb; //! application control block

/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/

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
 * @fn         int32_t  rsi_app_task_WIFI_1(void)
 * @brief      Function to work with application task
 * @param[in]  void
 * @return     void
 *=====================================================*/
int32_t rsi_power_num_app_task(void)
{
  int32_t status = RSI_SUCCESS;
#if !(DHCP_MODE)
  uint32_t ip_addr      = DEVICE_IP;
  uint32_t network_mask = NETMASK;
  uint32_t gateway      = GATEWAY;
#endif
  while (1) {
    switch (rsi_wlan_app_cb.state) {
      case RSI_WLAN_INITIAL_STATE: {
        rsi_wlan_app_callbacks_init();
        rsi_wlan_app_cb.state = RSI_WLAN_UNCONNECTED_STATE; //! update WLAN application state to unconnected state
      }
      //no break
      case RSI_WLAN_UNCONNECTED_STATE: {
#if RSI_POWER_AFTER_CHIP_INIT
        //! Apply wlan power save profile
        status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("power save failure\n");
          break;
        }

        //! Apply bt power save profile with deep sleep
        status = rsi_bt_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("power save failure\n");
          break;
        }
        LOG_PRINT("In Disconnected Sleep mode...\n");
        rsi_wlan_app_cb.state = RSI_POWER_SAVE_STATE;
        break;
#elif RSI_DEEP_SLEEP_WITH_RAM_RET_AFTER_RESET
        //! Need to destroy and create the driver task before calling
        //! rsi_wireless_deinit();
        rsi_task_destroy(driver_task_handle);
        status = rsi_wireless_deinit();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n wireless deinit failed \n");
          break;
        }
        rsi_wireless_driver_task_create();

        LOG_PRINT("\n reset successfull \n");
        //! WiSeConnect initialization
        status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n wireless init failed \n");
          break;
        }

        //! WLAN Radio Init
        status = rsi_wlan_radio_init();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("wlan radio init failed\n");
          break;
        }
        LOG_PRINT("WLAN RADIO Init success\n");

        //! Apply wlan power save profile
        status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("power save failure\n");
          break;
        }

        //! Apply bt power save profile with deep sleep
        status = rsi_bt_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("power save failure\n");
          break;
        }
        LOG_PRINT("In DeepSleep mode...\n");
        rsi_wlan_app_cb.state = RSI_POWER_SAVE_STATE;
        break;
#elif (RSI_DEEP_SLEEP_WITH_N_WO_RAM_RET_AFTER_RESET || RSI_DEEP_SLEEP_WITHOUT_RAM_RET)
        LOG_PRINT("Setting ulp without ram retention power save\n");

        //! Apply wlan power save profile with deep sleep
        status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_10, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("power save failure\n");
          break;
        }

        //! Apply bt power save profile with deep sleep
        status = rsi_bt_power_save_profile(RSI_SLEEP_MODE_10, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("power save failure\n");
          break;
        }
        LOG_PRINT("In DeepSleep mode without ram retention...\n");
        rsi_delay_ms(5);
        rsi_wlan_app_cb.state = RSI_POWER_SAVE_STATE;
#endif
#if RSI_STANDBY
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

          //! Apply power save profile with standby
          status = rsi_bt_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("power save failure\n");
            break;
          }
          LOG_PRINT("In Standby mode...\n");
          rsi_wlan_app_cb.state = RSI_POWER_SAVE_STATE;
          break;
        }
#endif
      }
      //no break
      case RSI_POWER_SAVE_STATE: {
#if RSI_DEEP_SLEEP_WITH_N_WO_RAM_RET_AFTER_RESET
        status = rsi_req_wakeup();
        if (status != RSI_SUCCESS) {
          break;
        }
        LOG_PRINT("Wake up from sleep...\n");
        //! WiSeConnect initialization
        status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n wireless init failed \n");
          break;
        }
        //! Send Feature frame
        status = rsi_send_feature_frame();
        if (status != RSI_SUCCESS) {
          return status;
        }
        //! WLAN Radio Init
        status = rsi_wlan_radio_init();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("wlan radio init failed\n");
          break;
        }
        LOG_PRINT("WLAN RADIO Init success\n");
        LOG_PRINT("Setting ulp with ram retention power save\n");
        //! Apply bt power save profile with deep sleep
        status = rsi_bt_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("power save failure\n");
          break;
        }

        //! Apply wlan power save profile
        status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("power save failure\n");
          break;
        }
        rsi_delay_ms(5);

        status = rsi_wlan_get(RSI_FW_VERSION, fw, sizeof(fw));
        if (status != RSI_SUCCESS) {
          LOG_PRINT("reading fw version failed\n");
          break;
        }
        LOG_PRINT("fw version before upgrade is: %s\n", fw);

        status = rsi_bt_get_local_device_address(local_dev_addr);
        if (status != RSI_SUCCESS) {
          break;
        }

        rsi_wlan_app_cb.state = RSI_WLAN_UNCONNECTED_STATE; //! update WLAN application state to unconnected state

#else
        break;
#endif
      }
      default:
        break;
    }
  }
  return status;
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
