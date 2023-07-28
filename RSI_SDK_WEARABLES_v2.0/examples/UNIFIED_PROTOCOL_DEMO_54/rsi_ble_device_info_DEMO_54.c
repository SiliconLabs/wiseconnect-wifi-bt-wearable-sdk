/*
 * rsi_ble_device_info.c
 *
 *  Created on: 01-Oct-2019
 *      Author: root
 */
#include <rsi_common_app.h>
#if (UNIFIED_PROTOCOL && RSI_ENABLE_BLE_TEST)
#include "rsi_driver.h"
#include "rsi_ble_apis.h"
#include <rsi_ble_config_DEMO_54.h>
#include <rsi_ble_device_info_DEMO_54.h>

uint8_t RSI_NULL_BLE_ADDR[18]                                                           = { 0 };
rsi_ble_conn_info_t rsi_ble_conn_info[RSI_BLE_MAX_NBR_MASTERS + RSI_BLE_MAX_NBR_SLAVES] = { 0 };

#if (CONNECT_OPTION == CONN_BY_NAME)
uint8_t rsi_get_ble_conn_id(uint8_t *remote_dev_addr, uint8_t *remote_name, uint8_t size)
#else
uint8_t rsi_get_ble_conn_id(uint8_t *remote_dev_addr)
#endif
{
  uint8_t conn_id = 0xFF; //! Max connections (0xFF -1)
  uint8_t i       = 0;

  for (i = 0; i < (RSI_BLE_MAX_NBR_MASTERS + RSI_BLE_MAX_NBR_SLAVES); i++) {
    if (!memcmp(rsi_ble_conn_info[i].remote_dev_addr, remote_dev_addr, RSI_REM_DEV_ADDR_LEN)) {
      conn_id = i;
      break;
    }
  }

  //! if bd_addr not found, add to the list
  if (conn_id == 0xFF) {
#if (CONNECT_OPTION == CONN_BY_NAME)
    conn_id = rsi_add_ble_conn_id(remote_dev_addr, remote_name, size);
#else
    conn_id = rsi_add_ble_conn_id(remote_dev_addr);
#endif
  }

  return conn_id;
}

uint8_t rsi_get_remote_device_role(uint8_t *remote_dev_addr)
{
  uint8_t role = MASTER_ROLE, i;

  //! Loop all structures and if the device addr is matched for slave structure, then return slave role or else master role
  for (i = 0; i < (RSI_BLE_MAX_NBR_SLAVES); i++) {
    if (memcmp(rsi_ble_conn_info[i].remote_dev_addr, remote_dev_addr, RSI_REM_DEV_ADDR_LEN) == 0) {
      return rsi_ble_conn_info[i].remote_device_role;
    }
  }
  return role; //! Returning role as master
}

#if (CONNECT_OPTION == CONN_BY_NAME)
uint8_t rsi_add_ble_conn_id(uint8_t *remote_dev_addr, uint8_t *remote_name, uint8_t size)
#else
uint8_t rsi_add_ble_conn_id(uint8_t *remote_dev_addr)
#endif
{
  uint8_t conn_id = 0xFF; //! Max connections (0xFF -1)
  uint8_t i       = 0;

  for (i = 0; i < (RSI_BLE_MAX_NBR_SLAVES); i++) {
    if (!memcmp(rsi_ble_conn_info[i].remote_dev_addr, RSI_NULL_BLE_ADDR, RSI_REM_DEV_ADDR_LEN)) {
      memcpy(rsi_ble_conn_info[i].remote_dev_addr, remote_dev_addr, RSI_REM_DEV_ADDR_LEN);
#if (CONNECT_OPTION == CONN_BY_NAME)
      rsi_ble_conn_info[i].rsi_remote_name = (uint8_t *)malloc((size + 1) * sizeof(uint8_t));
      memset(rsi_ble_conn_info[i].rsi_remote_name, 0, size + 1);
      memcpy(rsi_ble_conn_info[i].rsi_remote_name, remote_name, size);
#endif
      rsi_ble_conn_info[i].remote_device_role = SLAVE_ROLE; //! remote device is slave
      conn_id                                 = i;
      break;
    }
  }

  rsi_ble_conn_info[conn_id].conn_id = conn_id;

  return conn_id;
}

uint8_t rsi_remove_ble_conn_id(uint8_t *remote_dev_addr)
{
  uint8_t conn_id = 0xFF; //! Max connections (0xFF -1)
  uint8_t i       = 0;

  for (i = 0; i < (RSI_BLE_MAX_NBR_MASTERS + RSI_BLE_MAX_NBR_SLAVES); i++) {
    if (!memcmp(rsi_ble_conn_info[i].remote_dev_addr, remote_dev_addr, RSI_REM_DEV_ADDR_LEN)) {
      memset(rsi_ble_conn_info[i].remote_dev_addr, 0, RSI_REM_DEV_ADDR_LEN);
#if (CONNECT_OPTION == CONN_BY_NAME)
      memset(rsi_ble_conn_info[i].rsi_remote_name, 0, sizeof(uint32_t));
#endif
      conn_id = i;
      break;
    }
  }

  return conn_id;
}

uint8_t rsi_check_ble_conn_status(uint8_t connection_id)
{
  uint8_t i = 0;

  for (i = 0; i < (RSI_BLE_MAX_NBR_MASTERS + RSI_BLE_MAX_NBR_SLAVES); i++) {
    if (rsi_ble_conn_info[i].conn_id == connection_id) {
      return RSI_SUCCESS;
    }
  }

  return 0xFF;
}

#endif
