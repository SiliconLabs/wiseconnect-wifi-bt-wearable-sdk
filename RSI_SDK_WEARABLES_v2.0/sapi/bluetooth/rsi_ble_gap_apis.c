/*******************************************************************************
* @file  rsi_ble_gap_apis.c
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

#ifdef RSI_BLE_ENABLE
#include "rsi_driver.h"
#include "rsi_ble.h"
#include "rsi_ble_config.h"
#include "rsi_utils.h"

/**
 * Structure for Power index and corresponding output power in db for all modules except WMS
 * The values are in dBm at Antenna
 * index 0 corresponds to -8 dBm , array size is 24
 */
uint8_t pwr_index_to_db_array[] = { 11, 13, 17, 23, 31, 44, 45, 47, 49, 52, 55, 59,
                                    63, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78 };

/**
 * Structure for Power index and corresponding output power in db for WMS module
 * The values are in dBm at Antenna
 * index 0 corresponds to -8 dBm , array size is 24
 */
uint8_t wms_pwr_index_to_db_array[] = { 8,  10, 13, 18, 29, 39, 40, 41, 42, 44, 45, 48,
                                        50, 54, 61, 70, 71, 72, 73, 74, 75, 76, 77, 78 };

/** @addtogroup BT-LOW-ENERGY1
* @{
*/
/**
 * @fn         uint8_t rsi_convert_db_to_powindex(int8_t tx_power_in_dBm)
 * @brief      Convert power from dBm to power index.
 * @param[in]  tx_power_in_dBm - tx power in  dBm( -8 dBm to 15 dBm)
 * @return     power index to controller
 *
 */
uint8_t rsi_convert_db_to_powindex(int8_t tx_power_in_dBm)
{
  module_type mod_type_temp;
  int16_t status;

  status = rsi_get_module_type(&mod_type_temp);
  if (status == RSI_SUCCESS) {
    tx_power_in_dBm += BLE_OUTPUT_POWER_FRONT_END_LOSS;
    if ((tx_power_in_dBm < RSI_MIN_OUTPUT_POWER_IN_DBM) || (tx_power_in_dBm > RSI_MAX_OUTPUT_POWER_IN_DBM)) {
      return 0;
    }
    /* Converting At antenna  dBm to On screen dBm  */
    tx_power_in_dBm -= RSI_MIN_OUTPUT_POWER_IN_DBM;
    switch (mod_type_temp) {
      case RSI_MODULE_TYPE_WMS: {
        return (wms_pwr_index_to_db_array[tx_power_in_dBm]);
        //no break
      }
      case RSI_MODULE_TYPE_Q7:
      case RSI_MODULE_TYPE_SB:
      case RSI_MODULE_TYPE_M7DB:
      case RSI_MODULE_TYPE_M4SB:
      default: {
        return (pwr_index_to_db_array[tx_power_in_dBm]);
        //no break
      }
    }
  }
  return 0;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_random_address(void)
 * @brief      Request the local device to set a random address.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  void
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_set_random_address(void)
{
  rsi_ble_req_rand_t ble_rand = { { 0 } };

  // Send set random address command
  return rsi_bt_driver_send_cmd(RSI_BLE_SET_RANDOM_ADDRESS, &ble_rand, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_random_address_with_value(uint8_t *random_addr)
 * @brief      Request the local device to set a given random address.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  random_addr - random address of the device to be set
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_set_random_address_with_value(uint8_t *random_addr)
{
  rsi_ble_req_rand_t ble_rand = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(ble_rand.rand_addr, random_addr);
#else
  memcpy((int8_t *)&(ble_rand.rand_addr), (int8_t *)random_addr, 6);
#endif

  // Send set random address command
  return rsi_bt_driver_send_cmd(RSI_BLE_SET_RANDOM_ADDRESS, &ble_rand, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_advertising(void)
 * @brief      Request the local device to start advertising.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  void
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_start_advertising(void)
{
  rsi_ble_req_adv_t ble_adv = { 0 };

  ble_adv.status = RSI_BLE_START_ADV;

  ble_adv.adv_type         = RSI_BLE_ADV_TYPE;
  ble_adv.filter_type      = RSI_BLE_ADV_FILTER_TYPE;
  ble_adv.direct_addr_type = RSI_BLE_ADV_DIR_ADDR_TYPE;
  rsi_ascii_dev_address_to_6bytes_rev(ble_adv.direct_addr, (int8_t *)RSI_BLE_ADV_DIR_ADDR);
  if (ble_adv.adv_int_min == 0) {
    ble_adv.adv_int_min = RSI_BLE_ADV_INT_MIN;
  }
  if (ble_adv.adv_int_max == 0) {
    ble_adv.adv_int_max = RSI_BLE_ADV_INT_MAX;
  }
  ble_adv.own_addr_type   = LE_PUBLIC_ADDRESS;
  ble_adv.adv_channel_map = RSI_BLE_ADV_CHANNEL_MAP;
  // Send start advertise command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_ADV, &ble_adv, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_advertising_with_values(void *rsi_ble_adv)
 * @brief      Request the local device to start advertising with specified values.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  rsi_ble_adv - This structure pointer holds the information of advertising values \n
 *                           This variable is the pointer of the \ref rsi_ble_req_adv_s structure
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */
int32_t rsi_ble_start_advertising_with_values(void *rsi_ble_adv)
{
  rsi_ble_req_adv_t ble_adv = { 0 };
  if (rsi_ble_adv != NULL) {
    memcpy(&ble_adv, rsi_ble_adv, sizeof(rsi_ble_req_adv_t));
  }

  ble_adv.status = RSI_BLE_START_ADV;

  // Send start advertise command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_ADV, &ble_adv, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_encrypt(uint8_t *key, uint8_t *data, uint8_t *resp)
 * @brief      Encrypt data.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  key - 16 Bytes key for Encryption of data.
 * @param[in]  data - 16 Bytes of Data request to encrypt.
 * @param[out] resp - Encrypted data
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */
int32_t rsi_ble_encrypt(uint8_t *key, uint8_t *data, uint8_t *resp)
{
  rsi_ble_encrypt_t ble_en;
  memcpy(ble_en.key, key, 16);
  memcpy(ble_en.data, data, 16);

  // Send Encrypt command
  return rsi_bt_driver_send_cmd(RSI_BLE_ENCRYPT, &ble_en, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_stop_advertising(void)
 * @brief      Stop advertising.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  void
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */
int32_t rsi_ble_stop_advertising(void)
{
  rsi_ble_req_adv_t ble_adv = { 0 };

  ble_adv.status = RSI_BLE_STOP_ADV;

  // Send stop advertise command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_ADV, &ble_adv, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_advertise_data(uint8_t *data, uint16_t data_len)
 * @brief      Set the advertising data.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  data - Advertising data.
 * @param[in]  data_len - Total length of advertising data.
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 * @note      1. The maximum length of advertising data payload is 31 bytes. \n
 *            2. The basic format of advertising payload record contains length and data.
 */
int32_t rsi_ble_set_advertise_data(uint8_t *data, uint16_t data_len)
{
  rsi_ble_req_adv_data_t ble_adv_data = { 0 };

  ble_adv_data.data_len = RSI_MIN(data_len, sizeof(ble_adv_data.adv_data));
  memcpy(ble_adv_data.adv_data, data, ble_adv_data.data_len);

  // Send stop advertise command
  return rsi_bt_driver_send_cmd(RSI_BLE_SET_ADVERTISE_DATA, &ble_adv_data, NULL);
}

/*========================================================*/
/**
 * @fn         int32_t rsi_ble_set_scan_response_data(uint8_t *data, uint16_t data_len)
 * @brief      Request the local device to set the scan response data.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  data - Data about to be sent
 * @param[in]  data_len - Length of data, which is about to be sent
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */
int32_t rsi_ble_set_scan_response_data(uint8_t *data, uint16_t data_len)
{
  rsi_ble_req_scanrsp_data_t ble_scanrsp_data = { 0 };
  ble_scanrsp_data.data_len                   = RSI_MIN(data_len, sizeof(ble_scanrsp_data.scanrsp_data));
  memcpy(ble_scanrsp_data.scanrsp_data, data, ble_scanrsp_data.data_len);

  return rsi_bt_driver_send_cmd(RSI_BLE_SET_SCAN_RESPONSE_DATA, &ble_scanrsp_data, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_scanning(void)
 * @brief      Start scanning.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  void
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */
int32_t rsi_ble_start_scanning(void)
{
  rsi_ble_req_scan_t ble_scan = { 0 };

  ble_scan.status = RSI_BLE_START_SCAN;

  ble_scan.scan_type     = RSI_BLE_SCAN_TYPE;
  ble_scan.filter_type   = RSI_BLE_SCAN_FILTER_TYPE;
  ble_scan.scan_int      = LE_SCAN_INTERVAL;
  ble_scan.scan_win      = LE_SCAN_WINDOW;
  ble_scan.own_addr_type = LE_PUBLIC_ADDRESS;

  // Send start scanning command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_SCAN, &ble_scan, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_scanning_with_values(void *rsi_ble_scan_params)
 * @brief      Start scanning with values.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  rsi_ble_scan_params - BLE scan parameters structure
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 */
int32_t rsi_ble_start_scanning_with_values(void *rsi_ble_scan_params)
{
  rsi_ble_req_scan_t ble_scan = { 0 };

  if (rsi_ble_scan_params != NULL) {
    memcpy(&ble_scan, (rsi_ble_req_scan_t *)rsi_ble_scan_params, sizeof(rsi_ble_req_scan_t));
  }

  ble_scan.status = RSI_BLE_START_SCAN;

  // Send start scanning command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_SCAN, &ble_scan, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_stop_scanning(void)
 * @brief      Stop scanning.
 * @pre        \ref rsi_ble_start_scanning() API needs to be called before this API.
 * @param[in]  void
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */
int32_t rsi_ble_stop_scanning(void)
{
  rsi_ble_req_scan_t ble_scan = { 0 };

  ble_scan.status = RSI_BLE_STOP_SCAN;

  // Send stop scanning command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_SCAN, &ble_scan, NULL);
}
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_connect_with_params(uint8_t remote_dev_addr_type,
 *                                              int8_t *remote_dev_addr,
 *                                              uint16_t scan_interval,
 *                                              uint16_t scan_window,
 *                                              uint16_t conn_interval_max,
 *                                              uint16_t conn_interval_min,
 *                                              uint16_t conn_latency,
 *                                              uint16_t supervision_tout)
 * @brief      Connect to the remote BLE device with the user configured parameters.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  remote_dev_addr_type - AddressType - Specifies the type of the address mentioned in BD Address \n
 *                                  0 - Public Address \n
 *                                  1 - Random Address
 * @param[in]  remote_dev_addr - This parameter describes the device address of remote device
 * @param[in]  scan_interval - LE Scan Interval : N=0xXXXX \n
 *                             It is defined as the time interval from when the Controller
 *                             started its last LE scan until it \n
 *                             begins the subsequent LE scan. \n
 *                             Range: 0x0004 to 0x4000 \n
 *                             Time = N * 0.625 msec \n
 *                             Time Range: 2.5 msec to 10 . 24 seconds
 * @param[in]  scan_window - LE Scan Window : N=0xXXXX \n
 *                           Amount of time for the duration of the LE scan. LE_Scan_Window
 *                           must be less than or equal to LE_Scan_Interval \n
 *                           Range: 0x0004 to 0x4000 \n
 *                           Time = N * 0.625 msec \n
 *                           Time Range: 2.5 msec to 10 . 24 seconds
 * @param[in]  conn_interval_max - Max Connection Interval : N=0xXXXX \n
 *                                 Minimum value for the connection event interval, which must \n
 *                                 be greater than or equal to Conn_Interval_Min. \n
 *                                 Range: 0x0006 to 0x0C80 \n
 *                                 Time = N * 1.25 msec \n
 *                                 Time Range: 7.5 msec to 4 seconds. \n
 *                                 0x0000 - 0x0005 and 0x0C81 - 0xFFFF - Reserved for future use
 * @param[in]  conn_interval_min - Min Connection Interval : N=0xXXXX \n
 *                                 Minimum value for the connection event interval, which must
 *                                 be greater than or equal to Conn_Interval_Max. \n
 *                                 Range: 0x0006 to 0x0C80 \n
 *                                 Time = N * 1.25 msec \n
 *                                 Time Range: 7.5 msec to 4 seconds. \n
 *                                 0x0000 - 0x0005 and 0x0C81 - 0xFFFF - Reserved for future use
 * @param[in]  conn_latency - Connection Latency : N = 0xXXXX \n
 *                            Slave latency for the connection in number of connection events. \n
 *                            Range: 0x0000 to 0x01F4
 * @param[in]  supervision_tout - Supervision Timeout : N = 0xXXXX \n
 *                                Supervision timeout for the LE Link. \n
 *                                Range: 0x000A to 0x0C80 \n
 *                                Time = N * 10 msec \n
 *                                Time Range: 100 msec to 32 seconds \n
 *                                0x0000 - 0x0009 and 0x0C81 - 0xFFFF - Reserved for future use
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_connect_with_params(uint8_t remote_dev_addr_type,
                                    int8_t *remote_dev_addr,
                                    uint16_t scan_interval,
                                    uint16_t scan_window,
                                    uint16_t conn_interval_max,
                                    uint16_t conn_interval_min,
                                    uint16_t conn_latency,
                                    uint16_t supervision_tout)
{

  rsi_ble_req_conn_t ble_conn = { 0 };

  ble_conn.dev_addr_type = remote_dev_addr_type;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(ble_conn.dev_addr, remote_dev_addr);
#else
  memcpy(ble_conn.dev_addr, remote_dev_addr, 6);
#endif
  ble_conn.le_scan_interval  = scan_interval;
  ble_conn.le_scan_window    = scan_window;
  ble_conn.conn_interval_min = conn_interval_min;
  ble_conn.conn_interval_max = conn_interval_max;
  ble_conn.conn_latency      = conn_latency;
  ble_conn.supervision_tout  = supervision_tout;

  // Send connect command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_CONN, &ble_conn, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_connect(uint8_t remote_dev_addr_type, int8_t *remote_dev_addr)
 * @brief      Connect to the remote BLE device.
 * @pre        rsi_wireless_init() API needs to be called before this API.
 * @param[in]  remote_dev_addr_type - This parameter describes the address type of the remote device
 * @param[in]  remote_dev_addr - This parameter describes the device address of the remote device
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_connect(uint8_t remote_dev_addr_type, int8_t *remote_dev_addr)
{
  rsi_ble_req_conn_t ble_conn = { 0 };

  ble_conn.dev_addr_type = remote_dev_addr_type;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(ble_conn.dev_addr, remote_dev_addr);
#else
  memcpy(ble_conn.dev_addr, remote_dev_addr, 6);
#endif
  ble_conn.le_scan_interval  = LE_SCAN_INTERVAL;
  ble_conn.le_scan_window    = LE_SCAN_WINDOW;
  ble_conn.conn_interval_min = CONNECTION_INTERVAL_MIN;
  ble_conn.conn_interval_max = CONNECTION_INTERVAL_MAX;
  ble_conn.conn_latency      = CONNECTION_LATENCY;
  ble_conn.supervision_tout  = SUPERVISION_TIMEOUT;

  // Send connect command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_CONN, &ble_conn, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_connect_cancel(int8_t *remote_dev_address)
 * @brief      Cancel the connection to the remote BLE device.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - This parameter describes the device address of the remote device
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */
int32_t rsi_ble_connect_cancel(int8_t *remote_dev_address)
{
  rsi_ble_req_disconnect_t ble_disconnect = { { 0 } };

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(ble_disconnect.dev_addr, remote_dev_address);
#else
  memcpy(ble_disconnect.dev_addr, remote_dev_address, 6);
#endif
  ble_disconnect.type = BLE_CONNECT_CANCEL;

  // Send connect cancel command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_DISCONNECT, &ble_disconnect, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_disconnect(int8_t *remote_dev_address)
 * @brief      Disconnect with the remote BLE device.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - This parameter describes the device address of the remote device
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */
int32_t rsi_ble_disconnect(int8_t *remote_dev_address)
{
  rsi_ble_req_disconnect_t ble_disconnect = { { 0 } };

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(ble_disconnect.dev_addr, remote_dev_address);
#else
  memcpy(ble_disconnect.dev_addr, remote_dev_address, 6);
#endif
  ble_disconnect.type = BLE_DISCONNECT;
  // Send disconnect command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_DISCONNECT, &ble_disconnect, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_device_state(uint8_t *resp)
 * @brief      Get the local device state. The state value is filled in "resp".
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[out] resp - This is an output parameter which consists of local device state. \n
 *                    This is a 1-byte value. The possible states are described in the below table
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */
int32_t rsi_ble_get_device_state(uint8_t *resp)
{
  return rsi_bt_driver_send_cmd(RSI_BLE_GET_DEV_STATE, NULL, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_smp_pairing_cap_data(rsi_ble_set_smp_pairing_capabilty_data_t *smp_pair_cap_data)
 * @brief      Set the SMP Pairing Capability of local device.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  smp_pair_cap_data - This structure pointer holds the information of the SMP capability data values \n
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */
int32_t rsi_ble_set_smp_pairing_cap_data(rsi_ble_set_smp_pairing_capabilty_data_t *smp_pair_cap_data)
{
  return rsi_bt_driver_send_cmd(RSI_BLE_SET_SMP_PAIRING_CAPABILITY_DATA, smp_pair_cap_data, NULL);
}
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_local_irk_value(uint8_t *l_irk)
 * @brief      Set the IRK value to the local device.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  l_irk -  l_irk  Pointer to local_irk
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 */

int32_t rsi_ble_set_local_irk_value(uint8_t *l_irk)
{
  rsi_ble_set_local_irk_t local_device_irk = { 0 };

  memcpy(local_device_irk.irk, l_irk, 16);

  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_SET_LOCAL_IRK, &local_device_irk, NULL);
}

/** @} */

/** @addtogroup BT-LOW-ENERGY5
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_conn_param_resp(uint8_t *remote_dev_address, uint8_t status)
 * @brief      Give the response for the remote device connection parameter request.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - Address of the remote device
 * @param[in]  status - accept or reject status \n
 *                    (0-ACCEPT 1-REJECT)
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 *
 */

int32_t rsi_ble_conn_param_resp(uint8_t *remote_dev_address, uint8_t status)
{
  rsi_ble_cmd_conn_param_resp_t conn_param_resp_cmd = { 0 };

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(conn_param_resp_cmd.dev_addr, remote_dev_address);
#else
  memcpy(conn_param_resp_cmd.dev_addr, remote_dev_address, 6);
#endif
  conn_param_resp_cmd.status = status;

  // Send connect cancel command
  return rsi_bt_driver_send_cmd(RSI_BLE_CONN_PARAM_RESP_CMD, &conn_param_resp_cmd, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_enhance_connect_with_params(void* ble_enhance_conn_params)
 * @brief      Connect to the remote BLE device with the user configured parameters.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  ble_enhance_conn_params - BLE enhance connection parameter structure
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_enhance_connect_with_params(void *ble_enhance_conn_params)
{
  rsi_ble_req_enhance_conn_t ble_enhance_conn = { 0 };

  if (ble_enhance_conn_params != NULL) {
    memcpy(&ble_enhance_conn,
           (rsi_ble_req_enhance_conn_t *)ble_enhance_conn_params,
           sizeof(rsi_ble_req_enhance_conn_t));
  }
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_CONN_ENHANCE, &ble_enhance_conn, NULL);
}

/** @} */
/** @addtogroup BT-LOW-ENERGY1
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_smp_pair_request(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req)
 * @brief      Request the SMP pairing process with the remote device.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[in]  io_capability - This is the device input output capability \n
 *                            0x00 - Display Only \n
 *                            0x01 - Display Yes/No \n
 *                            0x02 - Keyboard Only \n
 *                            0x03 - No Input No Output
 * @param[in]  mitm_req - MITM enable/disable \n
 *                       0 - Disable \n
 *                       1 - Enable
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_smp_pair_request(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req)
{

  rsi_ble_req_smp_pair_t smp_pair_req;
  memset(&smp_pair_req, 0, sizeof(smp_pair_req));

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(smp_pair_req.dev_addr, remote_dev_address);
#else
  memcpy(smp_pair_req.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  smp_pair_req.io_capability = io_capability;
  smp_pair_req.mitm_req      = mitm_req;

  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_SMP_PAIR, &smp_pair_req, NULL);
}

/** @} */
/** @addtogroup BT-LOW-ENERGY1
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_smp_pair_failed(uint8_t *remote_dev_address ,uint8_t reason)
 * @brief      Send SMP pairing failure reason to the remote device.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[in]  reason - This is the reason for SMP Pairing Failure \n
 *                            0x05 - Pairing Not Supported \n
 *                            0x09 - Repeated Attempts \n
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_smp_pair_failed(uint8_t *remote_dev_address, uint8_t reason)
{

  rsi_ble_req_smp_pair_failed_t smp_pair_failed;
  memset(&smp_pair_failed, 0, sizeof(smp_pair_failed));

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(smp_pair_failed.dev_addr, remote_dev_address);
#else
  memcpy(smp_pair_failed.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  smp_pair_failed.reason = reason;

  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_SMP_PAIRING_FAILED, &smp_pair_failed, NULL);
}

/** @} */
/** @addtogroup BT-LOW-ENERGY5
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_ltk_req_reply(uint8_t *remote_dev_address,
 *                                           uint8_t reply_type, uint8_t *ltk)
 * @brief      Send the local long term key of its associated local EDIV and local Rand.
 * @param[in]  remote_dev_address -  BD address of remote device
 * @param[in]  reply_type - 0 - Negative reply \n
 *                          1 - Positive Reply
 * @param[in]  ltk - Long Term Key 16 bytes
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_ltk_req_reply(uint8_t *remote_dev_address, uint8_t reply_type, uint8_t *ltk)
{
  rsi_ble_set_le_ltkreqreply_t le_ltk_req_reply;
  memset(&le_ltk_req_reply, 0, sizeof(le_ltk_req_reply));

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(le_ltk_req_reply.dev_addr, remote_dev_address);
#else
  memcpy(le_ltk_req_reply.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  le_ltk_req_reply.replytype = reply_type;
  if (ltk != NULL) {
    memcpy(le_ltk_req_reply.localltk, ltk, 16);
  }

  return rsi_bt_driver_send_cmd(RSI_BLE_LE_LTK_REQ_REPLY, &le_ltk_req_reply, NULL);
}
/** @} */
/** @addtogroup BT-LOW-ENERGY1
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_smp_pair_response(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req)
 * @brief      Send SMP pairing response during the process of pairing with the remote device.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[in]  io_capability - This is the device input output capability \n
 *                            0x00 - Display Only \n
 *                            0x01 - Display Yes/No \n
 *                            0x02 - Keyboard Only \n
 *                            0x03 - No Input No Output
 * @param[in]  mitm_req -  MITM Request info \n
 *                        0 - Disable \n
 *                        1 - Enable
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_smp_pair_response(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req)
{
  rsi_ble_smp_response_t smp_pair_resp;
  memset(&smp_pair_resp, 0, sizeof(smp_pair_resp));

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(smp_pair_resp.dev_addr, remote_dev_address);
#else
  memcpy(smp_pair_resp.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  smp_pair_resp.io_capability = io_capability;
  smp_pair_resp.mitm_req      = mitm_req;

  return rsi_bt_driver_send_cmd(RSI_BLE_SMP_PAIR_RESPONSE, &smp_pair_resp, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_smp_passkey(uint8_t *remote_dev_address, uint32_t passkey)
 * @brief      Send SMP passkey during SMP pairing process with the remote device.
 * @pre        Call \ref rsi_ble_smp_pair_request and \ref rsi_ble_smp_pair_response
 *             before calling this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[in]  passkey - This is the key required in pairing process
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_smp_passkey(uint8_t *remote_dev_address, uint32_t passkey)
{
  rsi_ble_smp_passkey_t smp_passkey;
  memset(&smp_passkey, 0, sizeof(smp_passkey));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(smp_passkey.dev_addr, remote_dev_address);
#else
  memcpy(smp_passkey.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  smp_passkey.passkey = passkey;
  return rsi_bt_driver_send_cmd(RSI_BLE_SMP_PASSKEY, &smp_passkey, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_le_ping_timeout(uint8_t *remote_dev_address, uint16_t *time_out)
 * @brief      Get the timeout value of the LE ping.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - This is the remote device address
 * @param[out] time_out - This a response parameter which holds timeout value for \n
 *                        authentication payload command.
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 * @note     Currently Get ping is not supported.
 */

int32_t rsi_ble_get_le_ping_timeout(uint8_t *remote_dev_address, uint16_t *time_out)
{
  uint16_t status;
  rsi_ble_get_le_ping_timeout_t leping_cmd;
  rsi_ble_rsp_get_le_ping_timeout_t le_ping_rsp;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(leping_cmd.dev_addr, remote_dev_address);
#else
  memcpy(leping_cmd.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  status = rsi_bt_driver_send_cmd(RSI_BLE_GET_LE_PING, &leping_cmd, &le_ping_rsp);
  if (status == 0) {
    *time_out = le_ping_rsp.time_out;
  }

  return status;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_le_ping_timeout(uint8_t *remote_dev_address, uint16_t time_out)
 * @brief      Set the timeout value of the LE ping.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address -  This is the remote device address
 * @param[out] timeout - This input parameter sets timeout value for authentication \n
 *                       payload command.(in milliseconds)
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_set_le_ping_timeout(uint8_t *remote_dev_address, uint16_t time_out)
{
  rsi_ble_set_le_ping_timeout_t leping_cmd;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(leping_cmd.dev_addr, remote_dev_address);
#else
  memcpy(leping_cmd.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  leping_cmd.time_out = time_out;

  return rsi_bt_driver_send_cmd(RSI_BLE_SET_LE_PING, &leping_cmd, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_clear_whitelist(void)
 * @brief      Clear all the BD address present in white list.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  void
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_clear_whitelist(void)
{
  rsi_ble_white_list_t le_whitelist = { 0 };
  le_whitelist.addordeltowhitlist   = CLEAR_WHITELIST;

  return rsi_bt_driver_send_cmd(RSI_BLE_LE_WHITE_LIST, &le_whitelist, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_addto_whitelist(int8_t *dev_address, uint8_t dev_addr_type)
 * @brief      Add BD address to white list.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  dev_address - Address of the device which is going to add in white list
 * @param[in]  dev_addr_type - address type of BD address
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_addto_whitelist(int8_t *dev_address, uint8_t dev_addr_type)
{
  rsi_ble_white_list_t le_whitelist = { 0 };
  le_whitelist.addordeltowhitlist   = ADD_DEVICE_TO_WHITELIST;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(le_whitelist.dev_addr, dev_address);
#else
  memcpy(le_whitelist.dev_addr, dev_address, 6);
#endif
  le_whitelist.bdaddressType = dev_addr_type;

  return rsi_bt_driver_send_cmd(RSI_BLE_LE_WHITE_LIST, &le_whitelist, NULL);
}
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_deletefrom_whitelist(int8_t *dev_address, uint8_t dev_addr_type)
 * @brief      Delete particular BD address from white list.
 * @pre        \ref rsi_ble_addto_whitelist() API needs to be called before this API.
 * @param[in]  dev_address - Address of the device which is going to delete from white list
 * @param[in]  dev_addr_type - address type of BD address
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_deletefrom_whitelist(int8_t *dev_address, uint8_t dev_addr_type)
{
  rsi_ble_white_list_t le_whitelist = { 0 };
  le_whitelist.addordeltowhitlist   = DELETE_DEVICE_FROM_WHITELIST;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(le_whitelist.dev_addr, dev_address);
#else
  memcpy(le_whitelist.dev_addr, dev_address, 6);
#endif
  le_whitelist.bdaddressType = dev_addr_type;

  return rsi_bt_driver_send_cmd(RSI_BLE_LE_WHITE_LIST, &le_whitelist, NULL);
}

/** @} */

/** @addtogroup BT-LOW-ENERGY5
* @{
*/
/*==============================================*/
/**
 * @fn        int32_t rsi_ble_resolvlist(uint8_t process_type,
 *                                      uint8_t remote_dev_addr_type,
 *                                      uint8_t *remote_dev_address,
 *                                      uint8_t *peer_irk,
 *                                      uint8_t *local_irk)
 * @brief     Add a device to resolving list device address, address type and irk's.
 * @pre       Call \ref rsi_wireless_init() before calling this API.
 * @param[in] process_type - Indicates which type of process this is, as follows: \n
 *                          1 to add a device to the resolve list \n
 *                          2 to remove a device from the resolve list \n
 *                          3 to clear the entire resolve list
 * @param[in] remote_dev_addr_type - This is the remote device address
 * @param[in] peer_irk - 16-byte IRK of the peer device
 * @param[in] local_irk - 16-byte IRK of the local device
 * @param[in] remote_dev_address - This is remote address
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_resolvlist(uint8_t process_type,
                           uint8_t remote_dev_addr_type,
                           uint8_t *remote_dev_address,
                           uint8_t *peer_irk,
                           uint8_t *local_irk)
{
  rsi_ble_resolvlist_t resolvlist;

  resolvlist.process_type         = process_type;
  resolvlist.remote_dev_addr_type = remote_dev_addr_type;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(resolvlist.remote_dev_addr, remote_dev_address);
#else
  memcpy(resolvlist.remote_dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  memcpy(resolvlist.peer_irk, peer_irk, 16);
  memcpy(resolvlist.local_irk, local_irk, 16);
  return rsi_bt_driver_send_cmd(RSI_BLE_PROCESS_RESOLV_LIST, &resolvlist, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_get_resolving_list_size(uint8_t *resp)
 * @brief      Request to get resolving list size.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
  * @param[in] resp - This is an output parameter which consists of resolving the list size.
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 : Buffer not available to serve the command
 */

int32_t rsi_ble_get_resolving_list_size(uint8_t *resp)
{

  return rsi_bt_driver_send_cmd(RSI_BLE_GET_RESOLVING_LIST_SIZE, NULL, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_addr_resolution_enable(uint8_t enable, uint16_t tout)
 * @brief      Request to enable address resolution, and to set resolvable private address timeout.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  enable - 1 for enable address resolution \n
 *                      0 for disable address resolution
 * @param[in]  tout -  the period for changing address of our local device in seconds.
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 : Buffer not available to serve the command
 */

int32_t rsi_ble_set_addr_resolution_enable(uint8_t enable, uint16_t tout)
{
  rsi_ble_set_addr_resolution_enable_t addr_resolution_enable;

  addr_resolution_enable.enable = enable;
  addr_resolution_enable.tout   = tout;

  return rsi_bt_driver_send_cmd(RSI_BLE_SET_ADDRESS_RESOLUTION_ENABLE, &addr_resolution_enable, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_privacy_mode(uint8_t remote_dev_addr_type,
 *                                              uint8_t *remote_dev_address, uint8_t privacy_mode)
 * @brief      Request to set privacy mode for particular device
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  remote_dev_addr_type - This is the remote device address \n
 *                                    0x00 Public Identity Address \n
 *                                    0x01 Random (static) Identity Address
 * @param[in]  remote_dev_address - BD address of remote device
 * @param[in]  privacy_mode - 0-Network privacy mode \n
 *                            1-Device privacy mode
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 : Buffer not available to serve the command
 */

int32_t rsi_ble_set_privacy_mode(uint8_t remote_dev_addr_type, uint8_t *remote_dev_address, uint8_t privacy_mode)
{
  rsi_ble_set_privacy_mode_t set_privacy_mode;
  set_privacy_mode.remote_dev_addr_type = remote_dev_addr_type;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(set_privacy_mode.remote_dev_addr, remote_dev_address);
#else
  memcpy(set_privacy_mode.remote_dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  set_privacy_mode.privacy_mode = privacy_mode;
  return rsi_bt_driver_send_cmd(RSI_BLE_SET_PRIVACY_MODE, &set_privacy_mode, NULL);
}
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_readphy(int8_t *remote_dev_address, rsi_ble_resp_read_phy_t *resp)
 * @brief      Read TX and RX PHY.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - remote device address
 * @param[in]  resp - pointer to store the response
 * @return     Connection_Handle:  Connection_Handle
 *             TX_PHY:             0x01 The transmitter PHY for the connection is LE 1M \n
 *                                 0x02 The transmitter PHY for the connection is LE 2M \n
 *                                 0x03 The transmitter PHY for the connection is LE Coded \n
 *                                 All other values Reserved for future use
 *             RX_PHY:             0x01 The receiver PHY for the connection is LE 1M \n
 *                                 0x02 The receiver PHY for the connection is LE 2M \n
 *                                 0x03 The receiver PHY for the connection is LE Coded \n
 *                                 All other values Reserved for future use
 */
int32_t rsi_ble_readphy(int8_t *remote_dev_address, rsi_ble_resp_read_phy_t *resp)
{
  rsi_ble_req_read_phy_t ble_read_phy = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(ble_read_phy.dev_addr, remote_dev_address);
#else
  memcpy(ble_read_phy.dev_addr, remote_dev_address, 6);
#endif

  // Send connect cancel command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_READ_PHY, &ble_read_phy, resp);
}
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_setphy(int8_t *remote_dev_address, uint8_t tx_phy, uint8_t rx_phy,
 *                                    uint16_t coded_phy)
 * @brief      Set TX and RX PHY.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - Address of remote device \n
 * @param[in]  tx_phy - 0 The Host prefers to use the LE 1M transmitter PHY (possibly among others) \n
 *                      1 The Host prefers to use the LE 2M transmitter PHY (possibly among others) \n
 *                      2 The Host prefers to use the LE Coded transmitter PHY (possibly among others) \n
 *                      3 - 7 Reserved for future use \n
 * @param[in]  rx_phy - 0 The Host prefers to use the LE 1M receiver PHY (possibly among others) \n
 *                      1 The Host prefers to use the LE 2M receiver PHY (possibly among others) \n
 *                      2 The Host prefers to use the LE Coded receiver PHY (possibly among others) \n
 *                      3 - 7 Reserved for future use \n
 * @param[in]  coded_phy - 0 = the Host has no preferred coding when transmitting on the LE Coded PHY \n
 *                         1 = the Host prefers that S=2 coding be used when transmitting on the LE Coded PHY \n
 *                         2 = the Host prefers that S=8 coding be used when transmitting on the LE Coded PHY \n
 *                         3 = Reserved for future use \n
 * @return     0   -  Success \n
 *             Non-Zero Value - Failure
 */

int32_t rsi_ble_setphy(int8_t *remote_dev_address, uint8_t tx_phy, uint8_t rx_phy, uint16_t coded_phy)
{

  rsi_ble_set_phy_t set_phy;
  memset(&set_phy, 0, sizeof(set_phy));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(set_phy.dev_addr, remote_dev_address);
#else
  memcpy(set_phy.dev_addr, remote_dev_address, 6);
#endif
  set_phy.all_phy     = ALL_PHYS;
  set_phy.tx_phy      = tx_phy;
  set_phy.rx_phy      = rx_phy;
  set_phy.phy_options = coded_phy;

  // Send connect cancel command
  return rsi_bt_driver_send_cmd(RSI_BLE_REQ_SET_PHY, &set_phy, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_conn_params_update(uint8_t *remote_dev_address,
 *                                             uint16_t min_int,
 *                                             uint16_t max_int,
 *                                             uint16_t latency,
 *                                             uint16_t timeout)
 * @brief      Request the connection parameters with the remote device
 * when RSI device is acting as Slave and update the connection params with the remote device
 * when RSI device is acting as Master.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - Address of remote device
 * @param[in]  min_int - Minimum value for the connection interval. \n
 *                       This shall be less than or equal to Conn_Interval_Max.
 * @param[in]  max_int - Conn_Interval_Max (in dec) - Maximum value for the connection interval. \n
 *                       This shall be greater than or equal to Conn_Interval_Min.
 * @param[in]  latency - Conn_Latency (in dec) - Slave latency for the connection in \n
 *                       number of connection events. Range: 0 to 499
 * @param[in]  timeout - Supervision_Timeout (in dec) - Supervision timeout for the LE Link. \n
 *                       Range: 10 to 3200 (Time = N * 10 ms, Time Range: 100 ms to 32 s)
 * @return     0   -  Success \n
 *             Non-Zero Value - Failure
 * @note       Min and max interval Range: 6 to 3200 (Time = N * 1.25 ms, Time Range: 7.5 ms to 4 s).
 *
 */
int32_t rsi_ble_conn_params_update(uint8_t *remote_dev_address,
                                   uint16_t min_int,
                                   uint16_t max_int,
                                   uint16_t latency,
                                   uint16_t timeout)
{
  rsi_ble_cmd_conn_params_update_t conn_params_update_cmd = { 0 };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(conn_params_update_cmd.dev_addr, remote_dev_address);
#else
  memcpy(conn_params_update_cmd.dev_addr, remote_dev_address, 6);
#endif
  conn_params_update_cmd.min_interval = min_int;
  conn_params_update_cmd.max_interval = max_int;
  conn_params_update_cmd.latency      = latency;
  conn_params_update_cmd.timeout      = timeout;

  // Send connect cancel command
  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_CONN_PARAMS_UPDATE, &conn_params_update_cmd, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_set_data_len(uint8_t *remote_dev_address, uint16_t tx_octets, uint16_t tx_time)
 * @brief      Set TX octets and the TX time.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  remote_dev_address - tAddress of remote device \n
 * @param[in]  tx_octets - Preferred maximum number of payload octets that the local Controller \n
 *                         should include in a single Link Layer packet on this connection.
 * @param[in]  tx_time - Preferred maximum number of microseconds that the local Controller \n
 *                       should use to transmit a single Link Layer packet on this connection.
 * @return     0 - LE_Set_Data_Length command succeeded. \n
 *             Non-Zero Value - Failure \n
 *             Connection_Handle - Connection_HandleRange 0x0000-0x0EFF
 *
 */

int32_t rsi_ble_set_data_len(uint8_t *remote_dev_address, uint16_t tx_octets, uint16_t tx_time)
{
  rsi_ble_setdatalength_t lesetdatalen;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(lesetdatalen.dev_addr, remote_dev_address);
#else
  memcpy(lesetdatalen.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  lesetdatalen.txoctets = tx_octets;
  lesetdatalen.txtime   = tx_time;

  return rsi_bt_driver_send_cmd(RSI_BLE_SET_DATA_LEN, &lesetdatalen, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_read_max_data_len(rsi_ble_read_max_data_length_t *blereaddatalen)
 * @brief      Set TX octets and the TX time.
 * @pre        \ref rsi_ble_connect() API needs to be called before this API.
 * @param[in]  blereaddatalen - structure variable of the type \ref rsi_ble_read_max_data_length_t.
 * @return     parameters    Description \n
 *             tx_octets  -  Preferred maximum number of payload octets that the local Controller should
 *                            include in a single Link Layer packet on this connection. \n
 *             tx_time    -  Preferred maximum number of microseconds that the local Controller
 *                           should use to transmit a single Link Layer packet on this connection \n
 *             maxrxoctets-  Maximum number of payload octets that the local Controller supports
 *                           for reception of a single Link Layer packet on a data connection. \n
 *             maxrxtime  -  Maximum time, in microseconds, that the local Controller supports for
 *                           reception of a single Link Layer packet on a data connection.
 */
int32_t rsi_ble_read_max_data_len(rsi_ble_read_max_data_length_t *blereaddatalen)
{
  return rsi_bt_driver_send_cmd(RSI_BLE_READ_MAX_DATA_LEN, NULL, blereaddatalen);
}
/** @} */

/** @addtogroup BT-LOW-ENERGY6
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_rx_test_mode(uint8_t rx_channel, uint8_t phy, uint8_t modulation)
 * @brief      Start the BLE RX test mode in controller.
 * @param[in]  rx_channel - Channel in which packet have to be received (0 - 39)
 * @param[in]  phy - 0x00  Reserved for future use \n
 *                   0x01  Receiver set to use the LE 1M PHY \n
 *                   0x02  Receiver set to use the LE 2M PHY \n
 *                   0x03  Receiver set to use the LE Coded PHY \n
 *                   (0x04 - 0xFF)   Reserved for future use.
 * @param[in]  modulation - 0x00  Assume transmitter will have a standard standard modulation index \n
 *                          0x01  Assume transmitter will have a stable modulation index \n
 *                          (0x02 - 0xFF)  Reserved for future use
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 */

int32_t rsi_ble_rx_test_mode(uint8_t rx_channel, uint8_t phy, uint8_t modulation)
{
  rsi_ble_rx_test_mode_t rx_test_mode;

  rx_test_mode.rx_channel = rx_channel;
  rx_test_mode.phy        = phy;
  rx_test_mode.modulation = modulation;

  return rsi_bt_driver_send_cmd(RSI_BLE_RX_TEST_MODE, &rx_test_mode, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_tx_test_mode(uint8_t tx_channel, uint8_t phy,
 *                                          uint8_t tx_len, uint8_t mode)
 * @brief      Start the BLE TX test mode in controller.
 * @param[in]  tx_channel -  RF Channel (0-39). \n
 * @param[in]  phy - 0x00  Reserved for future use \n
 *                   0x01  Transmitter set to use the LE 1M PHY \n
 *                   0x02  Transmitter set to use the LE 2M PHY \n
 *                   0x03  Transmitter set to use the LE Coded PHY with S=8 data coding \n
 *                   0x04  Transmitter set to use the LE Coded PHY with S=2 data coding \n
 *                   (0x05 - 0xFF)  Reserved for future use.
 * @param[in] tx_len - Length in bytes of payload data in each packet ( 1 - 251 bytes).
 * @param[in] mode - 0x00  PRBS9 sequence '11111111100000111101...' \n
 *                   0x01  Repeated '11110000' \n
 *                   0x02  Repeated '10101010' \n
 *                   0x03  PRBS15 \n
 *                   0x04  Repeated '11111111' \n
 *                   0x05  Repeated '00000000'
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 */

int32_t rsi_ble_tx_test_mode(uint8_t tx_channel, uint8_t phy, uint8_t tx_len, uint8_t mode)
{
  rsi_ble_tx_test_mode_t tx_test_mode;

  tx_test_mode.tx_channel   = tx_channel;
  tx_test_mode.phy          = phy;
  tx_test_mode.tx_len       = tx_len;
  tx_test_mode.tx_data_mode = mode;

  return rsi_bt_driver_send_cmd(RSI_BLE_TX_TEST_MODE, &tx_test_mode, NULL);
}

/**
 * @fn        int32_t rsi_ble_end_test_mode(uint16_t *num_of_pkts)
 * @brief     Stop the BLE TX / RX test mode in controller.
 * @param[in] num_of_pkts - Number of RX packets received are displayed  \n
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 */

int32_t rsi_ble_end_test_mode(uint16_t *num_of_pkts)
{
  return rsi_bt_driver_send_cmd(RSI_BLE_END_TEST_MODE, NULL, num_of_pkts);
}
/** @} */

/** @addtogroup BT-LOW-ENERGY6
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ble_per_transmit(struct rsi_ble_per_transmit_s *rsi_ble_per_tx)
 * @brief      Initiate the BLE transmit PER mode in the controller.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  rsi_ble_per_tx - This parameter is the buffer to hold the structure values \n
 *                            This is a structure variable of struct \ref rsi_ble_per_transmit_s
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 */

int32_t rsi_ble_per_transmit(struct rsi_ble_per_transmit_s *rsi_ble_per_tx)
{
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_PER_CMD, rsi_ble_per_tx, NULL);
}

/**
 * @fn         int32_t rsi_ble_per_receive(struct rsi_ble_per_receive_s *rsi_ble_per_rx)
 * @brief      Initiate the BLE receive PER mode in the controller.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  rsi_ble_per_rx - This parameter is the buffer to hold the structure values \n
 *             This is a structure variable of struct \ref rsi_ble_per_receive_s
 * @return     0 - Success \n
 *             Non-Zero Value - Failure
 */

int32_t rsi_ble_per_receive(struct rsi_ble_per_receive_s *rsi_ble_per_rx)
{
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_PER_CMD, rsi_ble_per_rx, NULL);
}
/** @} */

/** @addtogroup BT-LOW-ENERGY1
* @{
*/
/**
 * @fn         int32_t rsi_ble_vendor_rf_type(uint8_t ble_power_index)
 * @brief      Issue a vendor-specific command to set the RF type in the controller on given inputs.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @prama[in]  ble_power_index - power index\n
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *               -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_vendor_rf_type(uint8_t ble_power_index)
{
  rsi_ble_vendor_rf_type_t ble_vendor_rf_type;

  ble_vendor_rf_type.opcode[0]       = (BLE_VENDOR_RF_TYPE_CMD_OPCODE & 0xFF);
  ble_vendor_rf_type.opcode[1]       = ((BLE_VENDOR_RF_TYPE_CMD_OPCODE >> 8) & 0xFF);
  ble_vendor_rf_type.ble_power_index = ble_power_index;

  return rsi_bt_driver_send_cmd(RSI_BT_VENDOR_SPECIFIC, &ble_vendor_rf_type, NULL);
}

/**
 * @fn         int32_t rsi_ble_white_list_using_adv_data(uint8_t enable,
 *                                                    uint8_t data_compare_index,
 *                                                    uint8_t len_for_compare_data,
 *                                                    uint8_t *payload)
 * @brief      Give vendor-specific command to set the whitelist feature based on
 *             the advertisers advertising payload.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  enable - enable/disable
 * @param[in]  data_compare_index - From which index onwards compare
 * @param[in]  len_for_compare_data - total length of data to compare
 * @param[in]  payload - Payload
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *               -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_white_list_using_adv_data(uint8_t enable,
                                          uint8_t data_compare_index,
                                          uint8_t len_for_compare_data,
                                          uint8_t *payload)
{
  rsi_ble_req_whitelist_using_payload_t whitelist_using_payload;
  memset(&whitelist_using_payload, 0, sizeof(whitelist_using_payload));

  whitelist_using_payload.opcode[0] = (BLE_VENDOR_WHITELIST_USING_ADV_DATA_PAYLOAD & 0xFF);
  whitelist_using_payload.opcode[1] = ((BLE_VENDOR_WHITELIST_USING_ADV_DATA_PAYLOAD >> 8) & 0xFF);

  whitelist_using_payload.enable               = enable;
  whitelist_using_payload.total_len            = 31;
  whitelist_using_payload.data_compare_index   = data_compare_index;
  whitelist_using_payload.len_for_compare_data = len_for_compare_data;

  memcpy(whitelist_using_payload.adv_data_payload, payload, 31);

  return rsi_bt_driver_send_cmd(RSI_BT_VENDOR_SPECIFIC, &whitelist_using_payload, NULL);
}

/*==============================================*/
/**
 * @fn         void BT_LE_ADPacketExtract(uint8_t *remote_name, uint8_t *pbuf, uint8_t buf_len)
 * @brief      Extract the advertised packet.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  remote_name - device name
 * @param[in]  pbuf -  advertise data packet buffer pointer
 * @param[in]  buf_len - buffer length
 * @return     void
 */

void BT_LE_ADPacketExtract(uint8_t *remote_name, uint8_t *pbuf, uint8_t buf_len)
{
  uint8_t adv_len;
  uint8_t adv_type;
  uint8_t ix;
  uint8_t name_len;

  ix = 0;

  while (ix < buf_len) {
    adv_len = pbuf[ix];
    ix += sizeof(uint8_t);
    adv_type = pbuf[ix];

    switch (adv_type) {
      case 0x08:
      case 0x09: {
        name_len = adv_len - 1;
        memcpy(remote_name, &pbuf[ix + 1], name_len);
        remote_name[name_len] = 0;
        return;
      }
        // no break
      default:
        break;
    }

    ix += adv_len;
  }
}
/** @} */

/** @addtogroup BT-LOW-ENERGY1
* @{
*/
/*==============================================*/
/**
 * @fn         void rsi_ble_update_directed_address(uint8_t *remote_dev_addr)
 * @brief      Update the direct address with the remote device address.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  remote_dev_addr - Remote device BD address
 * @return     void
 */

void rsi_ble_update_directed_address(uint8_t *remote_dev_addr)
{
  rsi_bt_cb_t *le_cb = rsi_driver_cb->ble_cb;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(le_cb->directed_addr, remote_dev_addr);
#else
  memcpy(le_cb->directed_addr, (int8_t *)remote_dev_addr, 6);
#endif
}

/*==============================================*/
/**
 * @fn         int32_t rsi_ble_start_encryption(uint8_t *remote_dev_address, uint16_t ediv,
 *                                              uint8_t *rand, uint8_t *ltk)
 * @brief      Start the encryption process with the remote device.
 * @pre        Encryption enabled event should come before calling this API for second time SMP connection.
 * @param[in]  remote_dev_address - Remote BD address in string format
 * @param[in]  ediv - remote device ediv value.
 * @param[in]  rand - remote device rand value.
 * @param[in]  ltk  - remote device ltk value.
 * @return     0 - Success \n
 *             Non-Zero Value - Failure \n
 *             If the return value is less than 0 \n
 *             -4 - Buffer not available to serve the command
 */

int32_t rsi_ble_start_encryption(uint8_t *remote_dev_address, uint16_t ediv, uint8_t *rand, uint8_t *ltk)
{
  rsi_ble_strat_encryption_t encrypt;
  memset(&encrypt, 0, sizeof(encrypt));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(encrypt.dev_addr, remote_dev_address);
#else
  memcpy(encrypt.dev_addr, (int8_t *)remote_dev_address, 6);
#endif
  encrypt.ediv = ediv;
  memcpy(encrypt.rand, rand, 8);
  memcpy(encrypt.ltk, ltk, 16);

  return rsi_bt_driver_send_cmd(RSI_BLE_RSP_START_ENCRYPTION, &encrypt, NULL);
}

/*==============================================*/
/**
 * @fn          int32_t rsi_ble_set_ble_tx_power(uint8_t role, uint8_t *remote_dev_address,
 *                                               int8_t tx_power)
 * @brief       Set the TX power value per GAP role.
 * @param[in]   role
 *              ADV_ROLE               0x01 \n
 *              SCAN_AND_CENTRAL_ROLE  0x02 \n
 *              PERIPHERAL_ROLE        0x03 \n
 *              CONN_ROLE              0x04
 * @param[in]   remote_dev_addr - Remote device address
 * @param[in]   tx-power - power value ( -8 dBm to 15 dBm)
 * @return      0 - Success \n
 *              Non-Zero Value - Failure
 *
 */

int32_t rsi_ble_set_ble_tx_power(uint8_t role, uint8_t *remote_dev_address, int8_t tx_power)
{
  rsi_ble_set_ble_tx_power_t ble_tx_power = { 0 };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(ble_tx_power.dev_addr, remote_dev_address);
#else
  memcpy(ble_tx_power.dev_addr, remote_dev_address, RSI_DEV_ADDR_LEN);
#endif
  ble_tx_power.role = role;
#if RSI_BLE_PWR_INX_DBM
  ble_tx_power.tx_power = rsi_convert_db_to_powindex(tx_power);
  if (ble_tx_power.tx_power == 0) {
    return RSI_ERROR_INVALID_PARAM;
  }
#else
  ble_tx_power.tx_power                        = tx_power;
#endif

  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_SET_BLE_TX_POWER, &ble_tx_power, NULL);
}
/*==============================================*/
/**
 * @fn          int32_t rsi_ble_set_prop_protocol_ble_bandedge_tx_power(uint8_t protocol, int8_t bandedge_tx_power)
 * @brief       Set the Proprietary Protocol and  BLE protocol bandedge tx power
 * @param[in]   protocol
 *              BLE_PROTOCOL                         0x01 \n
 *              PROP_PROTOCOL_PROTOCOL               0x02 \n
 * @param[in]   tx-power - power value ( -8 dBm to 15 dBm)
 * @note        #define RSI_BLE_PWR_INX_DBM  0  indicate tx_power in index \n
 *              Default Value for BLE Tx Power Index is 31, The range for the BLE Tx Power Index is 1 to 75 (0, 32 indexes are invalid) \n
 *                      1 - 31    BLE - 0DBM Mode.  \n
 *                     33 - 63    BLE - 10DBM Mode. \n
 *                     64 - 75    BLE - HP Mode.    \n
 *              Currently this API is supports only BLE LP mode . i.e. 1 to  63 BLE LP MODE \n
 *              #define RSI_BLE_PWR_INX_DBM  1  indicate tx_power in dBm \n
 *              tx_power in dBm (-8dBm to 15 dBm) \n
 *              Currently this API is supports only BLE LP mode . i.e. -8 dBm to  4dBm BLE LP MODE \n
 * @return      0 - Success \n
 *              Non-Zero Value - Failure \n
 *              0x4046 	Invalid Arguments \n 
 *              0x4D14	BLE parameter out of mandatory range 
 * @note        Refer Error Codes section for above error codes \ref error-codes.
 * 
 */

int32_t rsi_ble_set_prop_protocol_ble_bandedge_tx_power(uint8_t protocol, int8_t bandedge_tx_power)
{
  rsi_ble_set_prop_protocol_ble_bandedge_tx_power_t prop_protocol_ble_bandedge_tx_power = { 0 };

  prop_protocol_ble_bandedge_tx_power.protocol = protocol;
#if RSI_BLE_PWR_INX_DBM
  prop_protocol_ble_bandedge_tx_power.tx_power = rsi_convert_db_to_powindex(bandedge_tx_power);
  if (prop_protocol_ble_bandedge_tx_power.tx_power == 0) {
    return RSI_ERROR_INVALID_PARAM;
  }
#else
  prop_protocol_ble_bandedge_tx_power.tx_power = bandedge_tx_power;
#endif

  return rsi_bt_driver_send_cmd(RSI_BLE_CMD_SET_PROP_PROTOCOL_BLE_BANDEDGE_TXPOWER,
                                &prop_protocol_ble_bandedge_tx_power,
                                NULL);
}
#endif
/** @} */
