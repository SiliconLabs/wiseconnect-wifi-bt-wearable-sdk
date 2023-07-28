/*******************************************************************************
* @file  rsi_user_input_DEMO_69.h
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
 * @file         rsi_user_input_DEMO_69.h
 * @version      0.1
 * @date         01 Feb 2020 *
 *
 *
 *  @brief : This file contains prototypes and macros which are used while parsing the runtime inputs
 *
 *  @section Description  This file contains prototypes and macros which are used while parsing the runtime inputs
 */

#ifndef RSI_USER_INPUT_H
#define RSI_USER_INPUT_H
void rsi_ui_app_task(void);
#define PROTOCOL_SEL          "protocol <"
#define BLE_CONF              "ble_conf <"
#define BLE_REMOTE_NAME       "ble_remote_names <"
#define BLE_REMOTE_ADDR       "ble_remote_addr <"
#define SLAVE1_CONF           "slave1 <"
#define SLAVE2_CONF           "slave2 <"
#define SLAVE3_CONF           "slave3 <"
#define SLAVE4_CONF           "slave4 <"
#define SLAVE5_CONF           "slave5 <"
#define SLAVE6_CONF           "slave6 <"
#define SLAVE7_CONF           "slave7 <"
#define SLAVE8_CONF           "slave8 <"
#define MASTER1_CONF          "master1 <"
#define MASTER2_CONF          "master2 <"
#define BT_CONFIG             "bt_conf <"
#define LINE1                 1
#define LINE2                 2
#define LINE3                 3
#define LINE4                 4
#define LINE5                 5
#define LINE6                 6
#define LINE7                 7
#define LINE8                 8
#define LINE9                 9
#define LINE10                10
#define DEMO_RING_BUFFER_SIZE 1000 //300
#endif
