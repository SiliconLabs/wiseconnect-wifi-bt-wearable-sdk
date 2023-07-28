/*******************************************************************************
* @file  rsi_common_app_DEMO_54.c
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
 * @file    rsi_common_app_DEMO_54.c
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
 *  @section Description  This application initiates Silabs device and create tasks.
 *
 */

/*=======================================================================*/
//   ! INCLUDES
/*=======================================================================*/

#include <rsi_common_app.h>

#if UNIFIED_PROTOCOL
#include "rsi_bt_common_apis.h"
#include <stdio.h>
#include <string.h>
#include <rsi_ble.h>
#include "rsi_driver.h"
#include "rsi_ble_apis.h"
#include "rsi_common_app_DEMO_54.h"
#include "rsi_ble_config_DEMO_54.h"

/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/

/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/
//! flag to check bt power save
bool bt_powersave_given                         = false;
rsi_task_handle_t bt_app_task_handle            = NULL;
rsi_task_handle_t prop_protocol_app_task_handle = NULL;
rsi_task_handle_t wlan_app_task_handle          = NULL;
rsi_task_handle_t ble_app_main_task_handle      = NULL;

#if (RSI_BLE_SANITY_TEST || RSI_BLE_POWER_NUM_TEST || RSI_BLE_ADVERTISING_TEST)
rsi_task_handle_t ble_app_sanity_task_handle = NULL;
rsi_semaphore_handle_t ble_sanity_task_sem;
#endif
rsi_task_handle_t ble_app_master1_task_handle = NULL;
rsi_task_handle_t ble_app_master2_task_handle = NULL;
rsi_task_handle_t ble_app_slave1_task_handle  = NULL;
rsi_task_handle_t ble_app_slave2_task_handle  = NULL;
rsi_task_handle_t ble_app_slave3_task_handle  = NULL;
#if (UNIFIED_PROTOCOL && RSI_GENERAL_POWER_TEST)
rsi_task_handle_t power_task_handle = NULL;
#endif

/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/
extern rsi_task_handle_t common_task_handle;
#if RSI_ENABLE_BT_TEST
extern rsi_semaphore_handle_t bt_sem;
#endif
#if RSI_ENABLE_WIFI_TEST
extern rsi_semaphore_handle_t suspend_sem;
#endif
#if RSI_ENABLE_PROP_PROTOCOL_TEST
extern rsi_semaphore_handle_t prop_protocol_sem;
#endif
#if RSI_ENABLE_BLE_TEST
extern rsi_semaphore_handle_t ble_main_task_sem, ble_slave1_sem, ble_slave2_sem, ble_slave3_sem, ble_master1_sem,
  ble_master2_sem;
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

/*==============================================*/
/**
 * @fn         rsi_common_app_task
 * @brief      This function creates the main tasks for selected protocol
 * @param[out] none
 * @return     none.
 * @section description
 * This function creates the main tasks for configured protocols
 */
void rsi_common_app_task(void)
{
  int32_t status = RSI_SUCCESS;

  while (1) {
    //! Silabs module initialization
    status = rsi_device_init(LOAD_NWP_FW);
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\r\n device init failed \n");
      return;
    }
    //! WiSeConnect initialization
    status = rsi_wireless_init(RSI_WLAN_MODE, RSI_COEX_MODE);
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\r\n wireless init failed \n");
      return;
    }
    //! Send Feature frame
    status = rsi_send_feature_frame();
    if (status != RSI_SUCCESS) {
      return;
    }

    //! initialize wlan radio
    status = rsi_wlan_radio_init();
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\n WLAN radio init failed \n");
      return;
    }

#if (RSI_ENABLE_BT_TEST)
    rsi_semaphore_create(&bt_sem, 0);
    rsi_task_create((void *)rsi_bt_app_task,
                    (uint8_t *)"bt_task",
                    RSI_BT_APP_TASK_SIZE + RSI_SBC_APP_ENCODE_SIZE,
                    NULL,
                    RSI_BT_APP_TASK_PRIORITY,
                    &bt_app_task_handle);
#endif
#if (RSI_ENABLE_BLE_TEST)
#if RSI_BLE_MULTICONN_TEST
    rsi_semaphore_create(&ble_main_task_sem, 0);
    rsi_semaphore_create(&ble_slave1_sem, 0);
    rsi_semaphore_create(&ble_slave2_sem, 0);
    rsi_semaphore_create(&ble_slave3_sem, 0);
    rsi_semaphore_create(&ble_master1_sem, 0);
    rsi_semaphore_create(&ble_master2_sem, 0);
    rsi_task_create((void *)rsi_ble_main_app_task,
                    (uint8_t *)"ble_main_task",
                    RSI_BLE_APP_MAIN_TASK_SIZE,
                    NULL,
                    RSI_BLE_MAIN_TASK_PRIORITY,
                    &ble_app_main_task_handle);
    rsi_task_create((void *)rsi_ble_master1_app_task,
                    (uint8_t *)"ble_master1_task",
                    RSI_BLE_APP_TASK_SIZE,
                    NULL,
                    RSI_BLE_APP_TASK_PRIORITY,
                    &ble_app_master1_task_handle);
    rsi_task_create((void *)rsi_ble_master2_app_task,
                    (uint8_t *)"ble_master2_task",
                    RSI_BLE_APP_TASK_SIZE,
                    NULL,
                    RSI_BLE_APP_TASK_PRIORITY,
                    &ble_app_master2_task_handle);
    rsi_task_create((void *)rsi_ble_slave1_app_task,
                    (uint8_t *)"ble_slave1_task",
                    RSI_BLE_APP_TASK_SIZE,
                    NULL,
                    RSI_BLE_APP_TASK_PRIORITY,
                    &ble_app_slave1_task_handle);
    rsi_task_create((void *)rsi_ble_slave2_app_task,
                    (uint8_t *)"ble_slave2_task",
                    RSI_BLE_APP_TASK_SIZE,
                    NULL,
                    RSI_BLE_APP_TASK_PRIORITY,
                    &ble_app_slave2_task_handle);
    rsi_task_create((void *)rsi_ble_slave3_app_task,
                    (uint8_t *)"ble_slave3_task",
                    RSI_BLE_APP_TASK_SIZE,
                    NULL,
                    RSI_BLE_APP_TASK_PRIORITY,
                    &ble_app_slave3_task_handle);
#elif (RSI_BLE_SANITY_TEST || RSI_BLE_POWER_NUM_TEST || RSI_BLE_ADVERTISING_TEST)
    rsi_semaphore_create(&ble_sanity_task_sem, 0);
    rsi_task_create((void *)rsi_ble_sanity_app_task,
                    (uint8_t *)"ble_sanity_task",
                    RSI_BLE_APP_MAIN_TASK_SIZE,
                    NULL,
                    RSI_BLE_MAIN_TASK_PRIORITY,
                    &ble_app_sanity_task_handle);
#endif
#endif
#if (RSI_ENABLE_WIFI_TEST)
    rsi_semaphore_create(&suspend_sem, 0);
    rsi_task_create((void *)rsi_wlan_app_task,
                    (uint8_t *)"wlan_task",
                    RSI_WLAN_APP_TASK_SIZE,
                    NULL,
                    RSI_WLAN_APP_TASK_PRIORITY,
                    &wlan_app_task_handle);
#endif
#if RSI_GENERAL_POWER_TEST
    rsi_task_create((void *)rsi_power_num_app_task,
                    (uint8_t *)"power_num_task",
                    RSI_WLAN_APP_TASK_SIZE,
                    NULL,
                    RSI_WLAN_APP_TASK_PRIORITY,
                    &power_task_handle);
#endif
#if (RSI_ENABLE_PROP_PROTOCOL_TEST)
    rsi_semaphore_create(&prop_protocol_sem, 0);
    rsi_task_create((void *)rsi_prop_protocol_task,
                    (uint8_t *)"prop_protocol_task",
                    RSI_PROP_PROTOCOL_APP_TASK_SIZE,
                    NULL,
                    RSI_PROP_PROTOCOL_APP_TASK_PRIORITY,
                    &prop_protocol_app_task_handle);
#endif
    //! delete the task as initialization is completed
    rsi_task_destroy(common_task_handle);
  }
}
#endif
