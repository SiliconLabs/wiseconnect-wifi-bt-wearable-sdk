/*******************************************************************************
* @file  rsi_ble_config_DEMO_54.h
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
 * @file         rsi_ble_config_DEMO_54.h
 * @version      0.1
 * @date         15 Aug 2015 *
 *
 *
 *  @brief : This file contains user configurable details to configure the device  
 *
 *  @section Description  This file contains user configurable details to configure the device 
 */

#ifndef SAPIS_EXAMPLES_RSI_DEMO_APPS_INC_RSI_BLE_CONFIG_UNIFIED_PROTOCOL_H
#define SAPIS_EXAMPLES_RSI_DEMO_APPS_INC_RSI_BLE_CONFIG_UNIFIED_PROTOCOL_H
/***********************************************************************************************************************************************/
//  ! INCLUDES
/***********************************************************************************************************************************************/
#include "rsi_common_app.h"
#include "stdint.h"
#include <string.h>
#include <stdlib.h>
#if UNIFIED_PROTOCOL
#include "rsi_ble_apis.h"

/***********************************************************************************************************************************************/
//! application events list
/***********************************************************************************************************************************************/
typedef enum rsi_ble_state_e {
  RSI_BLE_DISCONN_EVENT = 0,
  RSI_APP_EVENT_ADV_REPORT,
  RSI_BLE_CONN_EVENT,
  RSI_BLE_ENHC_CONN_EVENT,
  RSI_BLE_MTU_EXCHANGE_INFORMATION,
  RSI_BLE_MTU_EVENT,
  RSI_BLE_MORE_DATA_REQ_EVT,
  RSI_BLE_RECEIVE_REMOTE_FEATURES,
  RSI_BLE_SCAN_RESTART_EVENT,
  RSI_APP_EVENT_DATA_LENGTH_CHANGE,
  RSI_APP_EVENT_PHY_UPDATE_COMPLETE,
  RSI_BLE_SMP_REQ_EVENT,
  RSI_BLE_SMP_RESP_EVENT,
  RSI_BLE_SMP_PASSKEY_EVENT,
  RSI_BLE_SMP_FAILED_EVENT,
  RSI_BLE_ENCRYPT_STARTED_EVENT,
  RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT,
  RSI_BLE_SC_PASSKEY_EVENT,
  RSI_BLE_LTK_REQ_EVENT,
  RSI_BLE_SECURITY_KEYS_EVENT,
  RSI_BLE_CONN_UPDATE_COMPLETE_EVENT,
  RSI_APP_EVENT_REMOTE_CONN_PARAM_REQ,
  RSI_BLE_CONN_UPDATE_REQ,
  RSI_BLE_GATT_ERROR,
  RSI_BLE_READ_REQ_EVENT,
  RSI_BLE_GATT_DESC_SERVICES,
  RSI_BLE_GATT_CHAR_SERVICES,
  RSI_BLE_GATT_PROFILE,
  RSI_BLE_GATT_PROFILES,
  RSI_BLE_REQ_GATT_PROFILE,
  RSI_BLE_GATT_WRITE_EVENT,
  RSI_BLE_GATT_PREPARE_WRITE_EVENT,
  RSI_BLE_GATT_EXECUTE_WRITE_EVENT,
  RSI_BLE_BUFF_CONF_EVENT,
  RSI_DATA_TRANSFER_EVENT,
  RSI_CONN_UPDATE_REQ_EVENT,
  RSI_BLE_WRITE_EVENT_RESP,
  RSI_BLE_REQ_WHITELIST,
  RSI_BLE_APP_USER_INPUT,
} rsi_ble_state_t;

#define UPDATE_CONN_PARAMETERS 0 //! To update connection parameters of remote master connection

#define MITM_ENABLE 1

//! by default this macro is disable
//! Enable it to place connected BLE remote device to whitelist
#if RSI_BLE_MULTICONN_TEST
#define RSI_ENABLE_WHITELIST 0
#endif

#define RSI_MAX_PROFILE_CNT 9
#define SMP_ENABLE          1
#define SIG_VUL             0
#if RSI_ENABLE_WHITELIST
#define SMP_ENABLE_M1 1
#else
#define SMP_ENABLE_M1 0
#endif
#define SMP_ENABLE_M2 0
#define SMP_ENABLE_S1 0
#define SMP_ENABLE_S2 0
#define SMP_ENABLE_S3 0

#define BIT64(a) ((long long int)1 << a)

//! By default this macro is enabled, if disabled connection update request wont start while other procedures executing
#define RSI_CONNECTION_UPDATE_REQ 1

//! By default this macro is disabled, if enabled profile query executes continuously with other procedures
#define RSI_PROFILE_QUERY_AGAIN 0

#define STOP_DATATRANSFER 1
//! ERROR CASES
//! By default these are disabled
//! enable this to check prevention of one procedure from causing an error response on an other unrelated procedure
#define ERROR_IN_PROCEDURE 1
//! enable this macro, to check error for procedure in progress
#define SAME_PROCEDURE_IN_PROGRESS 1

/* enable this, to perform basic connectivity test */
#define CONNECTIVITY_TEST 0

#if !RSI_BLE_SANITY_TEST
//! By default this macro is enabled
#define DLE_ON 1
#else
#define DLE_ON 0
#endif
//! by default this macro is disabled, enable this macro to check negative case of transmitting 232 bytes in DLE OFF mode
#define DLE_OFF_NEGATIVE_CASE 0

#if DLE_ON
#define DLE_BUFFER_MODE  1
#define DLE_BUFFER_COUNT 2
//! [RSI_BLE_NUM_CONN_EVENTS (no: BLE notifications) - no: of connections]/no: of connections (20-5)/5
#define RSI_BLE_MAX_DATA_LEN 230 //! max data length
#else
#define DLE_BUFFER_MODE  0
#define DLE_BUFFER_COUNT 2
#if !DLE_OFF_NEGATIVE_CASE
#define RSI_BLE_MAX_DATA_LEN 20 //! max data length
#else
#define RSI_BLE_MAX_DATA_LEN 230
#endif
#endif

#define MAX_MTU_SIZE 240

#define RSI_ERROR_IN_BUFFER_ALLOCATION 0x4e65
#define RSI_END_OF_PROFILE_QUERY       0x4a0a

#define RSI_TO_CHECK_ALL_CMD_IN_PROGRESS 0

//! BY default this macro is enabled
#define RSI_USER_INPUTS_ENABLED 1
#define CONN_BY_ADDR            1
#define CONN_BY_NAME            2
#define CONNECT_OPTION          CONN_BY_NAME //CONN_BY_ADDR

#if (CONNECT_OPTION == CONN_BY_NAME)
/***********************************************************************************************************************************************/
//! Remote Device Name to connect
/***********************************************************************************************************************************************/
#define RSI_REMOTE_DEVICE_NAME1 "HotspotX" //"Hotspot"//"Skv"//"Hotspot"
#define RSI_REMOTE_DEVICE_NAME2 "slave22"
#define RSI_REMOTE_DEVICE_NAME3 "slave3"
#else

#define RSI_BLE_DEV_ADDR_TYPE \
  LE_RANDOM_ADDRESS //! Address type of the device to connect - LE_PUBLIC_ADDRESS / LE_RANDOM_ADDRESS
/***********************************************************************************************************************************************/
//! Address of the devices to connect
/***********************************************************************************************************************************************/
#define RSI_BLE_DEV_1_ADDR "00:1A:7D:DA:71:16"
#define RSI_BLE_DEV_2_ADDR "00:1A:7D:DA:71:74"
#define RSI_BLE_DEV_3_ADDR "00:1A:7D:DA:73:13"
#endif

#if RSI_ENABLE_PROP_PROTOCOL_TEST
#define RSI_BLE_DEV_ADDR_TYPE LE_PUBLIC_ADDRESS //! Address type of the device to connect
#endif

#define RESOLVE_ENABLE 0

#if RESOLVE_ENABLE

#define RSI_BLE_DEV_ADDR_RESOLUTION_ENABLE    0x01
#define RSI_BLE_SET_RESOLVABLE_PRIV_ADDR_TOUT 0x120

#define PRIVACY_ENABLE 0

#endif

#define SET_SMP_CONFIGURATION 0x01
#if SET_SMP_CONFIGURATION
#define LOCAL_OOB_DATA_FLAG_NOT_PRESENT (0x00)

#define AUTH_REQ_BONDING_BITS   ((1 << 0))
#define AUTH_REQ_MITM_BIT       (1 << 2)
#define AUTH_REQ_SC_BIT         (1 << 3)
#define AUTH_REQ_BITS           (AUTH_REQ_BONDING_BITS | AUTH_REQ_MITM_BIT | AUTH_REQ_SC_BIT)
#define MAXIMUM_ENC_KEY_SIZE_16 (16)

// BLUETOOTH SPECIFICATION Version 5.0 | Vol 3, Part H 3.6.1 Key Distribution and Generation
#define ENC_KEY_DIST  (1 << 0)
#define ID_KEY_DIST   (1 << 1)
#define SIGN_KEY_DIST (1 << 2)
#define LINK_KEY_DIST (1 << 3)
#if RESOLVE_ENABLE
#define RESPONDER_KEYS_TO_DIST (ENC_KEY_DIST | SIGN_KEY_DIST | ID_KEY_DIST)
#else
#define RESPONDER_KEYS_TO_DIST (ENC_KEY_DIST | SIGN_KEY_DIST)
#endif
#if RESOLVE_ENABLE
#define INITIATOR_KEYS_TO_DIST (ID_KEY_DIST | ENC_KEY_DIST | SIGN_KEY_DIST)
#else
#define INITIATOR_KEYS_TO_DIST (ENC_KEY_DIST | SIGN_KEY_DIST)
#endif
#endif

#define RSI_BLE_APP_GATT_TEST (void *)"BLE_UNIFIED_DEMO" //! local device name

/***********************************************************************************************************************************************/
//! Notification related macros
/***********************************************************************************************************************************************/
#define HEART_RATE_PROFILE_UUID                0x180D     //! Heart rate profile UUID
#define HEART_RATE_MEASUREMENT                 0x2A37     //! Heart rate measurement
#define APPLE_NOTIFICATION_CENTER_SERVICE_UUID 0x7905F431 //! Apple notification center service UUID
#define APPLE_NOTIFICATION_SOURCE_UUID         0x9FBF120D //! Apple Notification source uuid

#define ENABLE_NOTIFICATION_SERVICE HEART_RATE_PROFILE_UUID

#if ENABLE_NOTIFICATION_SERVICE == HEART_RATE_PROFILE_UUID
#define ENABLE_NOTIFICATION_UUID HEART_RATE_MEASUREMENT
#elif ENABLE_NOTIFICATION_SERVICE == APPLE_NOTIFICATION_CENTER_SERVICE_UUID
#define ENABLE_NOTIFICATION_UUID APPLE_NOTIFICATION_SOURCE_UUID
#endif

/***********************************************************************************************************************************************/
//! BLE client characteristic service and attribute uuid.
/***********************************************************************************************************************************************/
#define RSI_BLE_NEW_CLIENT_SERVICE_UUID 0x180D //! immediate alert service uuid
#define RSI_BLE_CLIENT_ATTRIBUTE_1_UUID 0x2A37 //! Alert level characteristic

#define BLE_ATT_REC_SIZE 500
#define NO_OF_VAL_ATT    5 //! Attribute value count

/***********************************************************************************************************************************************/
//! user defined structure
/***********************************************************************************************************************************************/
typedef struct rsi_ble_att_list_s {
  uuid_t char_uuid;
  uint16_t handle;
  uint16_t value_len;
  uint16_t max_value_len;
  uint8_t char_val_prop;
  void *value;
} rsi_ble_att_list_t;

typedef struct rsi_ble_s {
  uint8_t DATA[BLE_ATT_REC_SIZE];
  uint16_t DATA_ix;
  uint16_t att_rec_list_count;
  rsi_ble_att_list_t att_rec_list[NO_OF_VAL_ATT];
} rsi_ble_t;

/***********************************************************************************************************************************************/
//! BLE_DUAL_MODE_BT_A2DP_SOURCE_WIFI_HTTP_S_RX SAPI BLE CONFIG DEFINES
/***********************************************************************************************************************************************/

#define RSI_OPERMODE_WLAN_BLE (0x109)

#define RSI_BLE_MAX_NBR_SLAVES  3
#define RSI_BLE_MAX_NBR_MASTERS 2

#define RSI_BLE_GATT_ASYNC_ENABLE 1

#define RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST 1
#define RSI_BLE_MTU_EXCHANGE_FROM_HOST          1
#define LOCAL_MTU_SIZE                          232
//! Notify status
#define NOTIFY_DISABLE 0x00
#define NOTIFY_ENABLE  0x01

//write status
#define WRITE_ENABLE  0x01
#define WRITE_DISABLE 0x00

/***********************************************************************************************************************************************/
//! Advertising command parameters
/***********************************************************************************************************************************************/
#if RSI_ENABLE_PROP_PROTOCOL_TEST
#define RSI_BLE_ADV_INT_MIN 0x06a8
#define RSI_BLE_ADV_INT_MAX 0x06a8
#else
#if !RSI_BLE_ADVERTISING_TEST
#define RSI_BLE_ADV_INT_MIN 0x100
#define RSI_BLE_ADV_INT_MAX 0x200
#else
//! advertising interval set to 20ms
#define RSI_BLE_ADV_INT_MIN 0x20
#define RSI_BLE_ADV_INT_MAX 0x20
#endif
#endif

//! advertise command parameters after one connection
#define RSI_BLE_ADV_INT_MIN_AFTER 0x21
#define RSI_BLE_ADV_INT_MAX_AFTER 0x21
/***********************************************************************************************************************************************/

/***********************************************************************************************************************************************/
//! Connection parameters
/***********************************************************************************************************************************************/
#define LE_SCAN_INTERVAL 0x0100
#define LE_SCAN_WINDOW   0x0050

#if RSI_ENABLE_PROP_PROTOCOL_TEST
#define CONNECTION_INTERVAL_MIN 0x00A0
#define CONNECTION_INTERVAL_MAX 0x00A0
#else
#define CONNECTION_INTERVAL_MIN 0x00A0
#define CONNECTION_INTERVAL_MAX 0x00A0
#endif

#define SUPERVISION_TIMEOUT 0x0C80

#define LE_SCAN_INTERVAL_CONN 0x0050
#define LE_SCAN_WINDOW_CONN   0x0050

//! Connection parameters after scanning
#define LE_SCAN_INTERVAL_AFTER 0x0035
#define LE_SCAN_WINDOW_AFTER   0x0015

/***********************************************************************************************************************************************/
//! Connection parameters for RSI as slave to remote device as master connection
/***********************************************************************************************************************************************/
#define S2M_CONNECTION_INTERVAL_MIN 0x00C8
#define S2M_CONNECTION_INTERVAL_MAX 0x00C8

#define S2M_CONNECTION_LATENCY  0x0000
#define S2M_SUPERVISION_TIMEOUT (4 * S2M_CONNECTION_INTERVAL_MAX)

/***********************************************************************************************************************************************/
//! Connection parameters for RSI as master to remote device as slave connection
/***********************************************************************************************************************************************/
#define M2S12_CONNECTION_INTERVAL_MIN 0x00C8
#define M2S12_CONNECTION_INTERVAL_MAX 0x00C8

#define M2S12_CONNECTION_LATENCY  0x0000
#define M2S12_SUPERVISION_TIMEOUT (4 * M2S12_CONNECTION_INTERVAL_MIN)

/***********************************************************************************************************************************************/
//! Connection parameters for RSI as master to remote device as slave connection
/***********************************************************************************************************************************************/
#define M2S34_CONNECTION_INTERVAL_MIN 0x0190
#define M2S34_CONNECTION_INTERVAL_MAX 0x0190

#define M2S34_CONNECTION_LATENCY  0x0000
#define M2S34_SUPERVISION_TIMEOUT (4 * M2S34_CONNECTION_INTERVAL_MIN)

/***********************************************************************************************************************************************/
//! Connection parameters for RSI as master to remote device as slave connection
/***********************************************************************************************************************************************/
#define M2S56_CONNECTION_INTERVAL_MIN 0x0320
#define M2S56_CONNECTION_INTERVAL_MAX 0x0320

#define M2S56_CONNECTION_LATENCY  0x0000
#define M2S56_SUPERVISION_TIMEOUT (4 * M2S56_CONNECTION_INTERVAL_MIN)

/***********************************************************************************************************************************************/
#define BLE_OUTPUT_POWER_FRONT_END_LOSS \
  0 /* This define value is 2 incase of EVK on screen verification, 0 at Antenna (used by wearables). Used in dBm to index conversion, per role Tx power  */

#include "rsi_ble_common_config.h"
#endif
#endif /* SAPIS_EXAMPLES_RSI_DEMO_APPS_INC_RSI_BLE_CONFIG_UNIFIED_PROTOCOL_H */
