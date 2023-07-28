/*******************************************************************************
* @file  rsi_common_app_DEMO_54.h
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
 * @file         rsi_common_app_DEMO_57.h
 * @version      0.1
 * @date         01 Feb 2020*
 *
 *
 *  @brief : This file contains user configurable details to configure the device
 *
 *  @section Description  This file contains user configurable details to configure the device
 */

#ifndef SAPIS_EXAMPLES_RSI_DEMO_APPS_INC_RSI_COMMON_APP_DEMO_57_H
#define SAPIS_EXAMPLES_RSI_DEMO_APPS_INC_RSI_COMMON_APP_DEMO_57_H

#include <stdio.h>
#include <rsi_os.h>
#define RSI_BLE_MAIN_TASK_PRIORITY          2
#define RSI_BT_APP_TASK_PRIORITY            1
#define RSI_BLE_APP_TASK_PRIORITY           1
#define RSI_PROP_PROTOCOL_APP_TASK_PRIORITY 1
#if (UNIFIED_PROTOCOL && (MULTITHREADED_HTTP_DOWNLOAD_TEST || MULTITHREADED_TCP_TX_TEST))
#define RSI_WLAN_APP_TASK_PRIORITY 3
#else
#define RSI_WLAN_APP_TASK_PRIORITY 1
#endif

#define RSI_BLE_APP_MAIN_TASK_SIZE      (512 * 2)
#define RSI_BLE_APP_TASK_SIZE           (512 * 2)
#define RSI_BT_APP_TASK_SIZE            (512 * 2)
#define RSI_SBC_APP_ENCODE_SIZE         (512 * 2)
#define RSI_PROP_PROTOCOL_APP_TASK_SIZE (512 * 2)
#define RSI_WLAN_APP_TASK_SIZE          (512 * 2)

#endif
