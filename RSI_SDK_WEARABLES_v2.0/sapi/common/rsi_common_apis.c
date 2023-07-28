/*******************************************************************************
* @file  rsi_common_apis.c
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

#include "rsi_driver.h"
#ifdef RSI_M4_INTERFACE
#include "rsi_ipmu.h"
#endif
#include "rsi_timer.h"
#ifdef RSI_BT_ENABLE
#include "rsi_bt.h"
#include "rsi_bt_config.h"
#endif
#ifdef RSI_BLE_ENABLE
#include "rsi_ble.h"
#endif
#ifdef RSI_PROP_PROTOCOL_ENABLE
#include "rsi_prop_protocol.h"
#endif
#include "rsi_wlan_non_rom.h"
extern rsi_socket_info_non_rom_t *rsi_socket_pool_non_rom;
extern rsi_socket_select_info_t *rsi_socket_select_info;
#ifdef PROCESS_SCAN_RESULTS_AT_HOST
extern struct wpa_scan_results_arr *scan_results_array;
#endif
/*
  Global Variables
 * */
rsi_driver_cb_t *rsi_driver_cb = NULL;
#ifdef RSI_M4_INTERFACE
extern efuse_ipmu_t global_ipmu_calib_data;
#endif
extern global_cb_t *global_cb;
extern rom_apis_t *rom_apis;
extern void rom_init(void);
int32_t rsi_driver_memory_estimate(void);

/** @addtogroup COMMON 
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_common_get_status(void)
 * @brief       Return common block status
 * @param[in]   void 
 * @return      0              -  Success \n 
 *              Non-Zero Value -  Failure
 */

int32_t rsi_common_get_status(void)
{
  return rsi_driver_cb->common_cb->status;
}

/*==============================================*/
/**
 * @fn          void rsi_common_set_status(int32_t status)
 * @brief       Set the common block status
 * @param[in]   status    -    status of common control block 
 * @return      0              - Success \n 
 *              Non-Zero Value - Failure
 */

void rsi_common_set_status(int32_t status)
{
  rsi_driver_cb->common_cb->status = status;
}

/*==============================================*/
/**
 *
 * @fn         int32_t rsi_driver_init(uint8_t *buffer, uint32_t length); 
 * @brief      Initialize the WiSeConnect driver.
 * @param[in]  buffer      -    This is the pointer to buffer from application. \n The driver uses this buffer to hold driver control for its operation.  
 * @param[in]  length      -    This is the length of the buffer. 
 * @return        0        -    Success \n
 *                              Returns the memory used, which is less than or equal to buffer length provided. \n
 *             Negative Value - Failure \n 
 *             Positive      -     returns total memory required by driver  \n
 *             -1          -    if UART initialization fails in SPI / UART mode   \n
 *             -2          -    If maximum sockets is greater than 10 
 */

/** @} */
uint8_t *buffer_addr = NULL;
int32_t rsi_driver_init(uint8_t *buffer, uint32_t length)
{
#if (defined RSI_WLAN_ENABLE) || (defined RSI_UART_INTERFACE) | (defined LINUX_PLATFORM)
  int32_t status = RSI_SUCCESS;
#endif

  uint32_t actual_length = 0;

  //if (((uint32_t)buffer & 3) != 0)
  if (((uintptr_t)buffer & 3) != 0) // to avoid compiler warning replace uint32_t with uintptr_t
  {
    // Making buffer 4 byte aligned
    //length -= (4 - ((uint32_t)buffer & 3));
    // to avoid compiler warning replace uint32_t with uintptr_t
    length -= (4 - ((uintptr_t)buffer & 3));
    // buffer = (uint8_t *)(((uint32_t)buffer + 3) & ~3);// to avoid compiler warning replace uint32_t with uintptr_t
    buffer = (uint8_t *)(((uintptr_t)buffer + 3) & ~3);
  }

  // Memset user buffer
  memset(buffer, 0, length);

  actual_length += rsi_driver_memory_estimate();

  // If length is not sufficient
  if (length < actual_length) {
    return actual_length;
  }
  buffer_addr = buffer;

  // Store length minus any alignment bytes to first 32 bit address in buffer.
  *(uint32_t *)buffer = length;
  buffer += sizeof(uint32_t);

  // Designate memory for driver cb
  rsi_driver_cb = (rsi_driver_cb_t *)buffer;
  buffer += sizeof(rsi_driver_cb_t);
  global_cb = (global_cb_t *)buffer;
  buffer += sizeof(global_cb_t);
  rom_apis = (rom_apis_t *)buffer;
  buffer += sizeof(rom_apis_t);
#ifdef RSI_WLAN_ENABLE
  // Memory for sockets
  rsi_socket_pool = (rsi_socket_info_t *)buffer;
  buffer += RSI_SOCKET_INFO_POOL_SIZE;
  rsi_socket_pool_non_rom = (rsi_socket_info_non_rom_t *)buffer;
  buffer += RSI_SOCKET_INFO_POOL_ROM_SIZE;
  rsi_socket_select_info = (rsi_socket_select_info_t *)buffer;
  buffer += RSI_SOCKET_SELECT_INFO_POOL_SIZE;
  rsi_wlan_cb_non_rom = (rsi_wlan_cb_non_rom_t *)buffer;
  buffer += RSI_WLAN_CB_NON_ROM_POOL_SIZE;
#ifdef PROCESS_SCAN_RESULTS_AT_HOST
  scan_results_array = (struct wpa_scan_results_arr *)buffer;
  buffer += sizeof(struct wpa_scan_results_arr);
#endif

  // Check for max no of sockets
  if (RSI_NUMBER_OF_SOCKETS > (10 + RSI_NUMBER_OF_LTCP_SOCKETS)) {
    status = RSI_ERROR_INVALID_PARAM;
    return status;
  }
#endif

//This will be disabled in M4 A11 condition,rest all it will be enabled.
#if !(defined(RSI_M4_INTERFACE) && !defined(A11_ROM))
  rsi_driver_cb->event_list = (rsi_event_cb_t *)buffer;
  buffer += RSI_EVENT_INFO_POOL_SIZE;
#endif

  rom_init();

  // Designate memory for rx_pool
#if !((defined RSI_SDIO_INTERFACE) && (!defined LINUX_PLATFORM))
  rsi_pkt_pool_init(&rsi_driver_cb->rx_pool, buffer, RSI_DRIVER_RX_POOL_SIZE, RSI_DRIVER_RX_PKT_LEN);
  buffer += RSI_DRIVER_RX_POOL_SIZE;
#endif
  // Designate memory for common_cb
  rsi_driver_cb_non_rom = (rsi_driver_cb_non_rom_t *)buffer;
  buffer += sizeof(rsi_driver_cb_non_rom_t);

  // Designate memory for common_cb
  rsi_driver_cb->common_cb = (rsi_common_cb_t *)buffer;
  buffer += sizeof(rsi_common_cb_t);

  // Initialize common cb
  rsi_common_cb_init(rsi_driver_cb->common_cb);

  // Designate pool for common block
  rsi_pkt_pool_init(&rsi_driver_cb->common_cb->common_tx_pool, buffer, RSI_COMMON_POOL_SIZE, RSI_COMMON_CMD_LEN);
  buffer += RSI_COMMON_POOL_SIZE;

  // Designate memory for wlan block
  rsi_driver_cb->wlan_cb = (rsi_wlan_cb_t *)buffer;
  buffer += sizeof(rsi_wlan_cb_t);

#ifdef RSI_M4_INTERFACE
  // Designate memory for efuse ipmu block
  rsi_driver_cb->common_cb->ipmu_calib_data_cb = (efuse_ipmu_t *)buffer;
  //ipmu_calib_data_cb = (efuse_ipmu_t *)buffer;
  //efuse_size = sizeof(efuse_ipmu_t);
  buffer += sizeof(efuse_ipmu_t);
#endif
#ifdef RSI_WLAN_ENABLE
  // Initialize wlan cb
  rsi_wlan_cb_init(rsi_driver_cb->wlan_cb);
#endif

  // Designate memory for wlan_cb pool
  rsi_pkt_pool_init(&rsi_driver_cb->wlan_cb->wlan_tx_pool, buffer, RSI_WLAN_POOL_SIZE, RSI_WLAN_CMD_LEN);
  buffer += RSI_WLAN_POOL_SIZE;

  // Initialize scheduler
  rsi_scheduler_init(&rsi_driver_cb->scheduler_cb);

  // Initialize events
  rsi_events_init();

  rsi_queues_init(&rsi_driver_cb->wlan_tx_q);

  rsi_queues_init(&rsi_driver_cb->common_tx_q);
#ifdef RSI_M4_INTERFACE
  rsi_queues_init(&rsi_driver_cb->m4_tx_q);

  rsi_queues_init(&rsi_driver_cb->m4_rx_q);

#endif
#ifdef RSI_ZB_ENABLE
  rsi_driver_cb->zigb_cb = (rsi_zigb_cb_t *)buffer;
  buffer += sizeof(rsi_zigb_cb_t);

  // Initialize zigb cb
  rsi_zigb_cb_init(rsi_driver_cb->zigb_cb);

  // Designate memory for zigb_cb buffer pool
  rsi_pkt_pool_init(&rsi_driver_cb->zigb_cb->zigb_tx_pool, buffer, RSI_ZIGB_POOL_SIZE, RSI_ZIGB_CMD_LEN);
  buffer += RSI_ZIGB_POOL_SIZE;

#ifdef ZB_MAC_API
  rsi_driver_cb->zigb_cb->zigb_global_mac_cb = (rsi_zigb_global_mac_cb_t *)buffer;
  buffer += sizeof(rsi_zigb_global_mac_cb_t);
  // Fill in zigb_global_cb
  buffer += rsi_zigb_global_mac_cb_init(buffer);
#else
  rsi_driver_cb->zigb_cb->zigb_global_cb = (rsi_zigb_global_cb_t *)buffer;
  buffer += sizeof(rsi_zigb_global_cb_t);

  // Fill in zigb_global_cb
  buffer += rsi_zigb_global_cb_init(buffer);
#endif
  rsi_queues_init(&rsi_driver_cb->zigb_tx_q);
#ifdef ZB_DEBUG
  printf("\n ZIGB POOL INIT \n");
#endif
#endif

#if defined(RSI_BT_ENABLE) || defined(RSI_BLE_ENABLE) || defined(RSI_PROP_PROTOCOL_ENABLE)
  // Designate memory for bt_common_cb
  rsi_driver_cb->bt_common_cb = (rsi_bt_cb_t *)buffer;
  buffer += ((sizeof(rsi_bt_cb_t) + 3) & ~3);

  // Initialize bt_common_cb
  rsi_bt_cb_init(rsi_driver_cb->bt_common_cb, RSI_PROTO_BT_COMMON);

  // Designate memory for bt_common_cb pool
  rsi_pkt_pool_init(&rsi_driver_cb->bt_common_cb->bt_tx_pool, buffer, RSI_BT_COMMON_POOL_SIZE, RSI_BT_COMMON_CMD_LEN);
  buffer += ((RSI_BT_COMMON_POOL_SIZE + 3) & ~3);

  rsi_queues_init(&rsi_driver_cb->bt_single_tx_q);
#endif

#ifdef RSI_BT_ENABLE
  // Designate memory for bt_classic_cb
  rsi_driver_cb->bt_classic_cb = (rsi_bt_cb_t *)buffer;
  buffer += ((sizeof(rsi_bt_cb_t) + 3) & ~3);

  // Initialize bt_classic_cb
  rsi_bt_cb_init(rsi_driver_cb->bt_classic_cb, RSI_PROTO_BT_CLASSIC);

  // Designate memory for bt_classic_cb pool
  rsi_pkt_pool_init(&rsi_driver_cb->bt_classic_cb->bt_tx_pool,
                    buffer,
                    RSI_BT_CLASSIC_POOL_SIZE,
                    RSI_BT_CLASSIC_CMD_LEN);
  buffer += ((RSI_BT_CLASSIC_POOL_SIZE + 3) & ~3);
#endif

#ifdef RSI_BLE_ENABLE
  // Designate memory for ble_cb
  rsi_driver_cb->ble_cb = (rsi_bt_cb_t *)buffer;
  buffer += ((sizeof(rsi_bt_cb_t) + 3) & ~3);

  // Initialize ble_cb
  rsi_bt_cb_init(rsi_driver_cb->ble_cb, RSI_PROTO_BLE);

  // Designate memory for ble_cb pool
  rsi_pkt_pool_init(&rsi_driver_cb->ble_cb->bt_tx_pool, buffer, RSI_BLE_POOL_SIZE, RSI_BLE_CMD_LEN);
  buffer += ((RSI_BLE_POOL_SIZE + 3) & ~3);
#endif

#ifdef RSI_PROP_PROTOCOL_ENABLE
  // Designate memory for prop_protocol_cb
  rsi_driver_cb->prop_protocol_cb = (rsi_bt_cb_t *)buffer;
  buffer += ((sizeof(rsi_bt_cb_t) + 3) & ~3);

  // Initialize prop_protocol_cb
  rsi_bt_cb_init(rsi_driver_cb->prop_protocol_cb, RSI_PROTO_PROP_PROTOCOL);

  // Designate memory for prop_protocol_cb pool
  rsi_pkt_pool_init(&rsi_driver_cb->prop_protocol_cb->bt_tx_pool,
                    buffer,
                    RSI_PROP_PROTOCOL_POOL_SIZE,
                    RSI_PROP_PROTOCOL_CMD_LEN);
  buffer += ((RSI_PROP_PROTOCOL_POOL_SIZE + 3) & ~3);

  rsi_queues_init(&rsi_driver_cb->prop_protocol_tx_q);
#endif

#ifdef SAPIS_BT_STACK_ON_HOST
  // Designate memory for bt_classic_cb
  rsi_driver_cb->bt_ble_stack_cb = (rsi_bt_cb_t *)buffer;
  buffer += sizeof(rsi_bt_cb_t);

  // Initialize bt_classic_cb
  rsi_bt_cb_init(rsi_driver_cb->bt_ble_stack_cb, RSI_PROTO_BT_BLE_STACK);

  // Designate memory for bt_classic_cb pool
  rsi_pkt_pool_init(&rsi_driver_cb->bt_ble_stack_cb->bt_tx_pool,
                    buffer,
                    RSI_BT_CLASSIC_POOL_SIZE,
                    RSI_BT_CLASSIC_CMD_LEN);
  buffer += RSI_BT_CLASSIC_POOL_SIZE;

  rsi_queues_init(&rsi_driver_cb->bt_ble_stack_tx_q);

#endif

#if defined(RSI_BT_ENABLE) || defined(RSI_BLE_ENABLE) || defined(RSI_PROP_PROTOCOL_ENABLE)
  // Designate memory for bt_common_cb
  rsi_driver_cb->bt_global_cb = (rsi_bt_global_cb_t *)buffer;
  buffer += sizeof(rsi_bt_global_cb_t);

  // Fill in bt_global_cb
  buffer += rsi_bt_global_cb_init(rsi_driver_cb, buffer);
#endif

  if (length < (uint32_t)(buffer - buffer_addr)) {
    return buffer - buffer_addr;
  }

#ifndef LINUX_PLATFORM
#ifdef RSI_SPI_INTERFACE
  rsi_timer_start(RSI_TIMER_NODE_0,
                  RSI_HAL_TIMER_TYPE_PERIODIC,
                  RSI_HAL_TIMER_MODE_MILLI,
                  1,
                  rsi_timer_expiry_interrupt_handler);
#endif

#if (defined(RSI_SPI_INTERFACE) || defined(RSI_SDIO_INTERFACE))
  // Configure power save GPIOs
  rsi_powersave_gpio_init();
#endif
#endif

#ifdef LINUX_PLATFORM
#if (defined(RSI_USB_INTERFACE) || defined(RSI_SDIO_INTERFACE))
  status = rsi_linux_app_init();
  if (status != RSI_SUCCESS) {
    return status;
  }

#endif
#endif

#ifdef RSI_UART_INTERFACE
  // UART initialisation
  status = rsi_uart_init();
  if (status != RSI_SUCCESS) {
    return status;
  }
#endif

  // Updating the state
  rsi_driver_cb_non_rom->device_state = RSI_DRIVER_INIT_DONE;

  return actual_length;
}
/** @addtogroup COMMON 
* @{
*/

/*==============================================*/

/**
 * @fn         int32_t rsi_wireless_init(uint16_t opermode, uint16_t coex_mode)
 * @brief      Initialize the WiSeConnect features.
 * @pre        \ref rsi_driver_init() API needs to be called before this API. 
 * @param[in]  opermode        -    WLAN Operating mode \n
 *			                        0 - Client mode \n
 *			                        2 - Enterprise security client mode \n
 *			                        6 - Access point mode \n
 *			                        8 - Transmit test mode \n
 *			                        9 - Concurrent mode 
 * @param[in]  coex_mode       -    Coexistence mode 
 *                                  0 - WLAN only mode \n
 *                                  1 - WLAN \n
 *                                  4 - Bluetooth \n
 *                                  5 - WLAN + Bluetooth \n
 *                                  8 - Dual Mode (Bluetooth and BLE) \n
 *                                  9 - WLAN + Dual Mode \n
 *                                  12- BLE mode \n
 *                                  13- WLAN + BLE \n
 *                                  Note: Coex modes are supported only in 384K memory configuration.  
 * @return      0              -    Success  \n
 *              Non-Zero Value -    Failure \n
 *                                  If return value is lesser than 0 \n
 *	                                -2 - Invalid parameters \n 
 *				                    -3 - Command given in wrong state \n
 *				                    -4 - Buffer not available to serve the command \n
 *				                    If return value is greater than 0 \n
 *					                0x0021,0x0025,0xFF73,0x002C,0xFF6E,0xFF6F, 0xFF70,0xFFC5
 * @note        Refer to Error Codes section for above error codes \ref SP16.
 */

int32_t rsi_wireless_init(uint16_t opermode, uint16_t coex_mode)
{
  rsi_pkt_t *pkt;
  rsi_opermode_t *rsi_opermode;
  int32_t status = RSI_SUCCESS;
#if !((defined(RSI_UART_INTERFACE) && defined(RSI_STM32)))
  rsi_timer_instance_t timer_instance;
#endif
#ifdef RSI_M4_INTERFACE
  //  int32_t        ipmu_status = RSI_SUCCESS;

#endif
  // Get common cb pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;
  rsi_wlan_cb_t *wlan_cb     = rsi_driver_cb->wlan_cb;

  if ((rsi_driver_cb_non_rom->device_state < RSI_DEVICE_INIT_DONE)) {
    //command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  common_cb->ps_coex_mode = coex_mode;
  common_cb->ps_coex_mode &= ~BIT(0);

#if (defined(RSI_UART_INTERFACE) && defined(RSI_STM32))
  common_cb->state = RSI_COMMON_CARDREADY;
#else
  rsi_init_timer(&timer_instance, RSI_CARD_READY_WAIT_TIME);
  // if state is not in card ready received state
  while (common_cb->state == RSI_COMMON_STATE_NONE) {
#ifndef RSI_WITH_OS
    // Wait untill receives card ready
    rsi_scheduler(&rsi_driver_cb->scheduler_cb);

    if (rsi_timer_expired(&timer_instance)) {
      return RSI_ERROR_CARD_READY_TIMEOUT;
    }
#else
#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(3);
#endif
#ifdef RSI_WITH_OS
    if (rsi_wait_on_common_semaphore(&common_cb->common_card_ready_sem, RSI_CARD_READY_WAIT_TIME) != RSI_ERROR_NONE) {
      return RSI_ERROR_RESPONSE_TIMEOUT;
    }
#endif
#endif
  }
#endif
  if (wlan_cb->auto_config_state != RSI_WLAN_STATE_NONE) {
    while (1) {
      // Check auto config state

      if ((wlan_cb->auto_config_state == RSI_WLAN_STATE_AUTO_CONFIG_DONE)
          || (wlan_cb->auto_config_state == RSI_WLAN_STATE_AUTO_CONFIG_FAILED)) {
        if (wlan_cb->state >= RSI_WLAN_STATE_INIT_DONE) {
          common_cb->ps_coex_mode |= BIT(0);
        }
        wlan_cb->auto_config_state = 0;
        break;
      }
#ifndef RSI_WLAN_SEM_BITMAP
      rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif

#ifdef RSI_WLAN_ENABLE
      // signal the wlan semaphore
      if (rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_AUTO_JOIN_RESPONSE_WAIT_TIME)
          != RSI_ERROR_NONE) {
        return RSI_ERROR_RESPONSE_TIMEOUT;
      }
#endif
    }
#ifdef RSI_WLAN_ENABLE
    status = rsi_common_get_status();
    if (status) {
      // return error
      return status;
    }
    status = rsi_wlan_get_status();
    if (status) {
      // return error
      return status;
    }
#endif
    // auto configuration status.
    return RSI_USER_STORE_CFG_STATUS;
  } else if (common_cb->state != RSI_COMMON_CARDREADY) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }

  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);
    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);
      // return packet allocation failure
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // memset the packet data
    memset(&pkt->data, 0, sizeof(rsi_opermode_t));

    // Take the user provided data and fill it in opermode mode structure
    rsi_opermode = (rsi_opermode_t *)pkt->data;

    // Save opermode command
    wlan_cb->opermode = opermode;

    // Fill coex and opermode parameters
    rsi_uint32_to_4bytes(rsi_opermode->opermode, (coex_mode << 16 | opermode));

#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif
#if defined(RSI_BT_ENABLE) || defined(RSI_BLE_ENABLE) || defined(RSI_PROP_PROTOCOL_ENABLE)
    // Save expected response type
    rsi_driver_cb->bt_common_cb->expected_response_type = RSI_BT_EVENT_CARD_READY;
    rsi_driver_cb->bt_common_cb->sync_rsp               = 1;
#endif
    // send opermode command to driver
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_OPERMODE, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_OPERMODE_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response stattus
  status = rsi_common_get_status();

#if defined(RSI_BT_ENABLE) || defined(RSI_BLE_ENABLE) || defined(RSI_PROP_PROTOCOL_ENABLE)
  if (!status) {
    if ((coex_mode == RSI_OPERMODE_WLAN_BLE) || (coex_mode == RSI_OPERMODE_WLAN_BT_CLASSIC)
        || (coex_mode == RSI_OPERMODE_WLAN_BT_DUAL_MODE)
#if defined(RSI_PROP_PROTOCOL_ENABLE)
        || (coex_mode == RSI_OPERMODE_PROP_PROTOCOL_GARDEN)
#endif
    ) {
      //WC waiting for BT Classic/ZB/BLE card ready
      rsi_bt_common_init();
    }
  }
#endif

#ifdef RSI_M4_INTERFACE
  //ipmu_status = rsi_cmd_m4_ta_secure_handshake(2,0,NULL,sizeof(efuse_ipmu_t),(uint8_t *)&global_ipmu_calib_data);
  //RSI_IPMU_UpdateIpmuCalibData(&global_ipmu_calib_data);
#endif

  // Return the status
  return status;
}
/** @} */
/*==============================================*/

#ifdef RSI_CHIP_MFG_EN

#define RSI_WAIT_TIMEOUT 50000000
#define RSI_INC_TIMER    timer_count++
int32_t rsi_wait_for_card_ready(uint32_t queue_type)
{
  uint32_t timer_count = 0;
  uint32_t status      = RSI_SUCCESS;

  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;
  rsi_wlan_cb_t *wlan_cb     = rsi_driver_cb->wlan_cb;
  volatile rsi_common_state_t *state_p;

  if (queue_type == RSI_COMMON_Q) {
    state_p = &common_cb->state;
  } else {
    state_p = &wlan_cb->state;
  }
  // if state is not in card ready received state
  while (*state_p == RSI_COMMON_STATE_NONE) {
    if (RSI_INC_TIMER > RSI_WAIT_TIMEOUT) {
      status = RSI_FAILURE;
      break;
    }
#ifndef RSI_WITH_OS
    // Wait untill receives card ready
    rsi_scheduler(&rsi_driver_cb->scheduler_cb);
#else
#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(3);
#endif
    if (rsi_wait_on_common_semaphore(&common_cb->common_card_ready_sem, RSI_WAIT_FOREVER) != RSI_ERROR_NONE) {
      return RSI_ERROR_RESPONSE_TIMEOUT;
    }
#endif
  }
  return status;
}

int32_t rsi_common_dev_params()
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  common_dev_config_params_t *rsi_common_dev_params_p;

  // Get common cb structure pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  if ((common_cb->state < RSI_COMMON_OPERMODE_DONE)) {
    //command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from common pool
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Take the user provided data and fill it in debug uart print select structure
    rsi_common_dev_params_p = (common_dev_config_params_t *)pkt->data;

    rsi_common_dev_params_p->lp_sleep_handshake           = 0x0;
    rsi_common_dev_params_p->ulp_sleep_handshake          = 0x1;
    rsi_common_dev_params_p->sleep_config_param           = 0x0;
    rsi_common_dev_params_p->host_wakeup_intr_enable      = 0xf;
    rsi_common_dev_params_p->host_wakeup_intr_active_high = 0x4;
    rsi_common_dev_params_p->lp_wakeup_threshold          = 0x100;
    rsi_common_dev_params_p->ulp_wakeup_threshold         = 0x2030200;
    rsi_common_dev_params_p->wakeup_threshold             = 0x0;
    rsi_common_dev_params_p->unused_soc_gpio_bitmap       = 0x10000;
    rsi_common_dev_params_p->unused_ulp_gpio_bitmap       = 0x13;
    rsi_common_dev_params_p->ext_pa_or_bt_coex_en         = 0x0;
    rsi_common_dev_params_p->opermode                     = 0x0;
    rsi_common_dev_params_p->driver_mode                  = 0x0;
    rsi_common_dev_params_p->no_of_stations_supported     = 0x1;
    rsi_common_dev_params_p->peer_distance                = 0x0;
    rsi_common_dev_params_p->bt_feature_bitmap            = 0x13;

    // Send firmware version query request
    status = rsi_driver_common_send_cmd(RSI_COMMON_DEV_CONFIG, pkt);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // Return the status
  return status;
}
#endif
/** @addtogroup COMMON 
* @{
*/
/**
 * @fn         int32_t rsi_cmd_uart_flow_ctrl(uint8_t uartflow_en)
 * @brief      Enable or disable UART flow control. 
 * @param[in]  uartflow_en  -    Enable or Disable UART hardware flow control \n
 *                               1/2 - Enable and Pin set to be used to for RTS/CTS purpose. \n
 *                               0   - Disable  \n
 *                               If uart_hw_flowcontrol_enable parameter is 0, uart flow control is disabled. \n
 *                               If the parameter is given as 1 or 2, then UART hardware flow control is enabled and Pin set to be used \n
 *                               If parameter is given as 1: Pin set used for RTS/CTS functionality is: \n
 *                                     UART_CTS:    GPIO - 11 \n
 *                                     UART_RTS:    GPIO - 7 \n
 *                               If parameter is given as 2: Pin set used for RTS/CTS functionality is: \n
 *                                     UART_CTS:    GPIO - 15 \n
 *                                     UART_RTS:    GPIO - 12 
 * @return     0              - Success \n 
 *             Negative Value - Failure 
 */
int32_t rsi_cmd_uart_flow_ctrl(uint8_t uartflow_en)
{
  rsi_pkt_t *pkt;

  int32_t status = RSI_SUCCESS;

  // Get common cb pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    pkt->data[0] = uartflow_en;
#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif
    // send  antenna select command
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_UART_FLOW_CTRL_ENABLE, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_UART_FLOW_CTRL_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response stattus
  status = rsi_common_get_status();

  // Return the status
  return status;
}

/*==============================================*/
/**
 * @fn           int32_t rsi_cmd_m4_ta_secure_handshake(uint8_t sub_cmd_type,
 *                                                      uint8_t input_len,
 *                                                      uint8_t *input_data,
 *                                                      uint8_t output_len,
 *                                                      uint8_t *output_data)
 * @brief      Secure handshake 
 * @param[in]  sub_cmd_type - Sub command 
 * @param[in]  input_data  - Input data 
 * @param[in]  input_len  - Length length
 * @param[in]  output_len  - Output length 
 * @param[in]  output_data - Output data 
 * @return     0              - Success \n 
 *             Negative Value - Failure 
 *
 *
 */

#ifdef RSI_M4_INTERFACE
int32_t rsi_cmd_m4_ta_secure_handshake(uint8_t sub_cmd_type,
                                       uint8_t input_len,
                                       uint8_t *input_data,
                                       uint8_t output_len,
                                       uint8_t *output_data)
{
  rsi_pkt_t *pkt;

  int32_t status = RSI_SUCCESS;

  // Get common cb pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Take the sub_cmd_type for TA and M4 commands
    pkt->data[0] = sub_cmd_type;

    pkt->data[1] = input_len;

    memcpy(&pkt->data[2], input_data, input_len);

    // attach the buffer given by user
    common_cb->app_buffer = output_data;

    // length of the buffer provided by user
    common_cb->app_buffer_length = output_len;

#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif
    // send  antenna select command
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_TA_M4_COMMANDS, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_TA_M4_COMMAND_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response stattus
  status = rsi_common_get_status();

  // Return the status
  return status;
}
#endif
/** @} */

/** @addtogroup COMMON
* @{
*/
/*==============================================*/
/**
 * @fn           int32_t rsi_wireless_deinit(void)
 * @brief        Reset WiSeConnect module, loads the firmware and restarts the driver.This API should be called before
 * \ref rsi_wireless_init command, if user wants to change  the previous configuration given through \ref rsi_wireless_init.       
 * @param[in]    void  
 * @return       0              - Success \n
 *               Negative Value - Failure \n
 *                         -2: Invalid parameters \n
 *                         -3: Command given in wrong state \n
 *                         -4: Buffer not available to serve the command 
 * @note        For restarting RS9116W module from host, application needs to call \ref rsi_driver_deinit() followed by 
 *              \ref rsi_driver_init() and \ref rsi_device_init(). For OS cases, additionally needs to call 
 *              \ref rsi_task_destroy(driver_task_handle) to delete the driver task before calling \ref rsi_driver_deinit() and should 
 *              create again after \ref rsi_device_init() using \ref rsi_task_create()
 */

int32_t rsi_wireless_deinit(void)
{
  int32_t status = RSI_SUCCESS;
#ifndef RSI_M4_INTERFACE
  int32_t length = 0;
#endif
#ifdef RSI_M4_INTERFACE
  rsi_pkt_t *pkt;
  // Get common cb structure pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // send softreset command
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_SOFT_RESET, pkt);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }
#ifdef SOFT_RESET_ENABLE
  // Mask interrupts
  rsi_hal_intr_mask();
#endif
#ifdef RSI_SPI_INTERFACE
  // poll for interrupt status
  while (!rsi_hal_intr_pin_status())
    ;

  // SPI interface initialization
  status = rsi_spi_iface_init();
  if (status != RSI_SUCCESS) {
    // Return the status
    return status;
  }
#endif
#ifdef SOFT_RESET_ENABLE
  // Unmask interrupts
  rsi_hal_intr_unmask();
#endif

#ifdef RSI_UART_INTERFACE
  rsi_uart_deinit();

#endif
#ifndef RSI_COMMON_SEM_BITMAP
  rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(2);
#endif

  // Wait on common semaphore
  if (rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_DEINIT_RESPONSE_WAIT_TIME)
      != RSI_ERROR_NONE) {
    return RSI_ERROR_RESPONSE_TIMEOUT;
  }

  // get common status
  status = rsi_common_get_status();
#else

  if (buffer_addr != NULL) {
    length = *(uint32_t *)buffer_addr;
    // Driver initialization
    status = rsi_driver_init(buffer_addr, length);
    if ((status < 0) || (status > length)) {
      return status;
    }
  } else {
    return -1;
  }

  // state update
  rsi_driver_cb->common_cb->state = RSI_COMMON_STATE_NONE;
#ifdef RSI_WLAN_ENABLE
  rsi_driver_cb->wlan_cb->state   = RSI_WLAN_STATE_NONE;
#endif
  // Redpine module initialization
  status = rsi_device_init(LOAD_NWP_FW);
  if (status != RSI_SUCCESS) {
    return status;
  }

#endif
  return status;
}

/*==============================================*/
/**
 * @fn           int32_t rsi_wireless_antenna(uint8_t type,uint8_t gain_2g, uint8_t gain_5g)
 * @brief        Select antenna type on the device
 * @param[in]    type              -  0 : RF_OUT_2/Internal Antenna is selected \n 
 * 				    	              1 : RF_OUT_1/uFL connector is selected. 
 * @param[in]    gain_2g           -  Currently not supported 
 * @param[in]    gain_5g           -  Currently not supported 
 * @note         Currently ignoring the gain_2g, gain_5g, antenna_path, antenna_type values. 
 *               Currently wireless_antenna selection is not supported. 
 * @return       0                 -  Success \n
 *               Non-Zero Value    -  failure \n
 *                                    If return value is lesser than 0 \n
 *                                    -4: Buffer not available to serve the command \n
 *                                    If return value is greater than 0 \n
 *                                    0x0025, 0x002C 
 * @note        Refer to Error Codes section for above error codes \ref SP16. 
 */

int32_t rsi_wireless_antenna(uint8_t type, uint8_t gain_2g, uint8_t gain_5g)
{

  rsi_pkt_t *pkt;
  rsi_antenna_select_t *rsi_antenna;
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  // Get common cb pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  // pre-condition for antenna selection.
  if ((wlan_cb->state < RSI_WLAN_STATE_INIT_DONE)) {
    //command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }

  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Take the user provided data and fill it in antenna select structure
    rsi_antenna = (rsi_antenna_select_t *)pkt->data;

    // antenna type
    rsi_antenna->antenna_value = type;

    // antena gain in 2.4GHz
    rsi_antenna->gain_2g = gain_2g;

    // antenna gain in 5GHz
    rsi_antenna->gain_5g = gain_5g;

#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif
    // send  antenna select command
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_ANTENNA_SELECT, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_ANTENNA_SEL_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response stattus
  status = rsi_common_get_status();

  // Return the status
  return status;
}
/*==============================================*/
/**
 * @fn           int32_t rsi_send_feature_frame_dyn(uint32_t feature_enables)
 * @brief        Send feature frame to connection command parser
 * @param[in]    feature_enables - Feature enables
 * @return       0              - Success \n 
 *               Negative Value - Failure 
 *               
 * @note Refer Error Codes section for above error codes \ref SP16. 
 *
 */
int32_t rsi_send_feature_frame_dyn(uint32_t feature_enables)
{

  rsi_pkt_t *pkt;
  rsi_feature_frame_t *rsi_feature_frame;
  int32_t status = RSI_SUCCESS;

  // Get common cb pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  if ((common_cb->state < RSI_COMMON_OPERMODE_DONE)) {
    //command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Take the user provided data and fill it in antenna select structure
    rsi_feature_frame = (rsi_feature_frame_t *)pkt->data;

    // pll mode value
    rsi_feature_frame->pll_mode = PLL_MODE;

    // rf type
    rsi_feature_frame->rf_type = RF_TYPE;

    // wireless mode
    rsi_feature_frame->wireless_mode = WIRELESS_MODE;

    // enable ppp
    rsi_feature_frame->enable_ppp = ENABLE_PPP;

    // afe type
    rsi_feature_frame->afe_type = AFE_TYPE;

    // feature enables
    rsi_feature_frame->feature_enables = feature_enables;

    // send antenna select command
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_FEATURE_FRAME, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_FEATURE_FRAME_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response stattus
  status = rsi_common_get_status();

  // Return the status
  return status;
}
/*==============================================*/
/**
 * @fn            int32_t rsi_send_feature_frame(void)
 * @brief         Select internal RF type or External RF type and clock frequency. 
 * @pre           \ref rsi_wireless_init() API needs to be called before this API 
 * @return        0              - Success \n
 *                Non-Zero Value - Failure \n
 *                                 If return value is lesser than 0 \n
 *                                 -2 : Invalid parameters \n
 *                                 -3 : Command given in wrong state \n
 *                                 -4 : Buffer not available to serve the command \n
 *                                 If return value is greater than 0 \n
 *                                 0x0021, 0xFF74 
 * @note         Refer Error Codes section for above error codes \ref SP16. 
 */

int32_t rsi_send_feature_frame(void)
{

  rsi_pkt_t *pkt;
  rsi_feature_frame_t *rsi_feature_frame;
  int32_t status = RSI_SUCCESS;

  // Get common cb pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  if ((common_cb->state < RSI_COMMON_OPERMODE_DONE)) {
    //command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Take the user provided data and fill it in antenna select structure
    rsi_feature_frame = (rsi_feature_frame_t *)pkt->data;

    // pll mode value
    rsi_feature_frame->pll_mode = PLL_MODE;

    // rf type
    rsi_feature_frame->rf_type = RF_TYPE;

    // wireless mode
    rsi_feature_frame->wireless_mode = WIRELESS_MODE;

    // enable ppp
    rsi_feature_frame->enable_ppp = ENABLE_PPP;

    // afe type
    rsi_feature_frame->afe_type = AFE_TYPE;

    // feature enables
    rsi_feature_frame->feature_enables =
      (RSI_WLAN_TRANSMIT_TEST_MODE == rsi_driver_cb->wlan_cb->opermode) ? 0 : FEATURE_ENABLES;

#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif
    // send antenna select command
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_FEATURE_FRAME, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_FEATURE_FRAME_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response stattus
  status = rsi_common_get_status();

  // Return the status
  return status;
}
/*==============================================*/
/**
 * @fn          int32_t rsi_get_fw_version(uint8_t *response, uint16_t length)
 * @brief       Get the firmware version present in the device. 
 * @param[in]   length         - This is the length of the response buffer in bytes to hold result.  
 * @param[out]  response       - This is the response of the requested command. This is an output parameter. 
 * @return      0              - Success \n
 *				Non-Zero Value - Failure \n
 *                               If return value is lesser than 0 \n
 *                               -3: Command given in wrong state \n
 *                               -4: Buffer not available to serve the command \n
 *                               -6: Insufficient input buffer given \n
 *                               If return value is greater than 0 \n
 *                               0x0021, 0x0025, 0x002c
 * @note       Refer Error Codes section for above error codes \ref SP16 . 
 */

int32_t rsi_get_fw_version(uint8_t *response, uint16_t length)
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;

  // Get common cb structure pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  // if state is not in card ready received state
  if (common_cb->state == RSI_COMMON_STATE_NONE) {
    while (common_cb->state != RSI_COMMON_CARDREADY) {
#ifndef RSI_WITH_OS
      rsi_scheduler(&rsi_driver_cb->scheduler_cb);
#endif
    }
  }

  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from common pool
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    if (response != NULL) {
      // attach the buffer given by user
      common_cb->app_buffer = response;

      // length of the buffer provided by user
      common_cb->app_buffer_length = length;
    } else {
      // Assign NULL to the app_buffer to avoid junk
      common_cb->app_buffer = NULL;

      // length of the buffer to 0
      common_cb->app_buffer_length = 0;
    }

#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif
    // Send firmware version query request
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_FW_VERSION, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_FWUP_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response stattus
  status = rsi_common_get_status();

  // Return the status
  return status;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_get_module_type(uint8_t *response, uint16_t length)
 * @brief       Get the module type
 * @pre         Need to call after the device initialization
 * @param[in]   length         - This is the length of the response buffer in bytes to hold result.
 * @param[out]  response       - This is the response of the requested command. This is an output parameter.
 * @return      0              - Success \n
 *				Non-Zero Value - Failure \n
 *                               If return value is lesser than 0 \n
 *                               -3: Command given in wrong state \n
 *                               -4: Buffer not available to serve the command \n
 *                               -6: Insufficient input buffer given \n
 *                               If return value is greater than 0 \n
 *                               0x0021, 0x0025, 0x002c
 * @note       Refer Error Codes section for above error codes \ref SP16 .
 */

int32_t rsi_get_module_type(uint8_t *response)
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  if (rsi_driver_cb_non_rom->module_type_valid) {
    *response = rsi_driver_cb_non_rom->module_type_id;
    return RSI_SUCCESS;
  }
  // Get common cb structure pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  // if state is not in card ready received state
  if (common_cb->state == RSI_COMMON_STATE_NONE) {
    while (common_cb->state != RSI_COMMON_CARDREADY) {
#ifndef RSI_WITH_OS
      rsi_scheduler(&rsi_driver_cb->scheduler_cb);
#endif
    }
  }

  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from common pool
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    if (response != NULL) {
      // attach the buffer given by user
      common_cb->app_buffer = response;

      // length of the buffer provided by user
      common_cb->app_buffer_length = 1;
    } else {
      // Assign NULL to the app_buffer to avoid junk
      common_cb->app_buffer = NULL;

      // length of the buffer to 0
      common_cb->app_buffer_length = 0;
    }

#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif
    // Send module type query request
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_MODULE_TYPE, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_MTYPE_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response status
  status = rsi_common_get_status();

  if (status == RSI_SUCCESS) {
    rsi_driver_cb_non_rom->module_type_id    = *response;
    rsi_driver_cb_non_rom->module_type_valid = 1;
  }

  // Return the status
  return status;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_common_debug_log(int32_t assertion_type, int32_t assertion_level)
 * @brief       Debug prints on UART interfaces 1 and 2. Host can get 5 types of debug prints based on the assertion level and assertion type. 
 * @param[in]   assertion_type          - Assertion_type (Possible values 0 - 15) \n
 * 				                          0000 - LMAC core, 0001 - SME, 0010 - UMAC, 0100 - NETX, 1000 - Enables assertion indication and \n
 *				                          provides ram dump in critical assertion. \n
 * @param[in]   assertion_level         - Assertion_level (Possible values 0 - 15). value 1 is least value/only specific prints, 15 is the highest \n
 *				                          level/Enable all prints.  \n
 *				                          0000 - Assertion required, 0010 - Recoverable, 0100 - Information \n
 * @return      0                       - Success \n
 *				Non-Zero value          - Failure \n
 *				-2                      - Parameters invalid
 * @note        1. To minimize the debug prints host is supposed to give the same command with assertion type and assertion level as 0. 
 *              2. Baud rate for UART 2 on host application side should be 460800. 
 *              3. Enable few Feature bit map for getting debug logs on UART:   
 *                 // To set custom feature select bit map 
 *                 #define RSI_CUSTOM_FEATURE_BIT_MAP   FEAT_CUSTOM_FEAT_EXTENTION_VALID   
 *                 // To set Extended custom feature select bit map   
 *                 #define RSI_EXT_CUSTOM_FEATURE_BIT_MAP    EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS  
 */

int32_t rsi_common_debug_log(int32_t assertion_type, int32_t assertion_level)
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  rsi_debug_uart_print_t *debug_uart_print;
  // Get common cb structure pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;
  if ((common_cb->state < RSI_COMMON_OPERMODE_DONE)) {
    //command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  if (((assertion_type > 15) || (assertion_type < 0)) || ((assertion_level < 0) || (assertion_level > 15))) {
    return RSI_ERROR_INVALID_PARAM;
  }

  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from common pool
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Take the user provided data and fill it in debug uart print select structure
    debug_uart_print = (rsi_debug_uart_print_t *)pkt->data;

    // assertion_type
    debug_uart_print->assertion_type = assertion_type;

    // rf type
    debug_uart_print->assertion_level = assertion_level;

#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif
    // Send firmware version query request
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_DEBUG_LOG, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_DEBUG_LOG_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response stattus
  status = rsi_common_get_status();

  // Return the status
  return status;
}

/*==============================================*/
/**
 * @fn           int32_t rsi_switch_proto(uint8_t type, void (*callback)(uint16_t mode, uint8_t *bt_disabled_status))
 * @brief        Enable or disable BT 
 * @param[in]    type - Type 
 * @return        0              - Success \n
 *                Negative Value - Failure 
 */

int32_t rsi_switch_proto(uint8_t type, void (*callback)(uint16_t mode, uint8_t *bt_disabled_status))
{
  rsi_pkt_t *pkt;
  rsi_switch_proto_t *ptr;
  int32_t status = RSI_SUCCESS;

  // Get common cb pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  if ((type == 1) && (callback != NULL)) {
    /* in ENABLE case, callback must be NULL */
    return RSI_ERROR_INVALID_PARAM;
  }
  if ((common_cb->state < RSI_COMMON_OPERMODE_DONE)) {
    //command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }

  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }
    // Take the user provided data and fill it in  swicth proto structure
    ptr = (rsi_switch_proto_t *)pkt->data;

    // type
    ptr->mode_value = type;

    common_cb->sync_mode = type;
#ifdef RSI_WLAN_ENABLE
    if (callback != NULL) {
      // Set socket asynchronous receive callback
      rsi_wlan_cb_non_rom->switch_proto_callback = callback;

    } else {
      // Set socket asynchronous receive callback
      rsi_wlan_cb_non_rom->switch_proto_callback = NULL;
    }
#endif
#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif

    // send switch proto command
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_SWITCH_PROTO, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_SWITCH_PROTO_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response stattus
  status = rsi_common_get_status();

  // Return the status
  return status;
}
/** @} */
/** @addtogroup COMMON
* @{
*/
/*==============================================*/
/**
 *
 * @fn          void rsi_wireless_driver_task(void)
 * @brief       Handle the driver events. It should be called in application main loop for non-OS platforms.
 * @param[in]   void
 * @return      void
 */

void rsi_wireless_driver_task(void)
{
#ifdef RSI_WITH_OS
  while (1)
#endif
  {
    rsi_scheduler(&rsi_driver_cb->scheduler_cb);
  }
}
//======================================================
/**
 *
 * @fn          int32_t rsi_driver_deinit(void)
 * @brief       De-Initialize the driver components. Clears all the memory given for driver operations in \ref rsi_driver_init() API. 
 * In OS case,  User need to take care of OS variables initialized in \ref rsi_driver_init(). 
 * This API has to be called by the thread/task/Master thread which is not dependent on.
 * OS variables allocated/initialized in \ref rsi_driver_init() API.
 * @pre 		Need to call after the driver initialization
 * @param[in]   void
 * @return      0              - Success \n
 *              Non-Zero Value - Failure
 */

int32_t rsi_driver_deinit(void)
{
  int32_t status         = RSI_SUCCESS;
  uint32_t actual_length = 0;

  if ((rsi_driver_cb_non_rom->device_state < RSI_DRIVER_INIT_DONE)) {
    //command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  // check buffer is enough or not for driver components
  actual_length += rsi_driver_memory_estimate();
  if (buffer_addr == NULL) {
    return RSI_FAILURE;
  }
#ifndef RSI_M4_INTERFACE
  rsi_hal_intr_mask();
#else
  mask_ta_interrupt(RX_PKT_TRANSFER_DONE_INTERRUPT | TX_PKT_TRANSFER_DONE_INTERRUPT);
#endif
#ifndef RSI_CHECK_PKT_QUEUE
  rsi_check_pkt_queue_and_dequeue();
#endif
#ifdef RSI_WITH_OS
  rsi_vport_enter_critical();
#endif
#ifndef RSI_RELEASE_SEMAPHORE
  rsi_release_waiting_semaphore();
#endif
  rsi_semaphore_destroy(&rsi_driver_cb->scheduler_cb.scheduler_sem);
#ifdef WAKEUP_GPIO_INTERRUPT_METHOD
  rsi_semaphore_destroy(&rsi_driver_cb->common_cb->wakeup_gpio_sem);
#endif
#ifdef RSI_WITH_OS
  rsi_semaphore_destroy(&rsi_driver_cb->common_cb->common_card_ready_sem);
#endif
  status = rsi_semaphore_destroy(&rsi_driver_cb->common_cb->common_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  rsi_mutex_destroy(&rsi_driver_cb->common_cb->common_mutex);
  rsi_mutex_destroy(&rsi_driver_cb_non_rom->tx_mutex);
#ifdef RSI_ZB_ENABLE
  rsi_mutex_destroy(&rsi_driver_cb->zigb_tx_q.queue_mutex);
#endif
#ifdef RSI_PROP_PROTOCOL_ENABLE
  rsi_mutex_destroy(&rsi_driver_cb->prop_protocol_tx_q.queue_mutex);
#endif
#if (defined RSI_BT_ENABLE || defined RSI_BLE_ENABLE || defined RSI_PROP_PROTOCOL_ENABLE)
  rsi_mutex_destroy(&rsi_driver_cb->bt_single_tx_q.queue_mutex);
#endif
#if (defined RSI_BLE_ENABLE || defined RSI_BT_ENABLE)
  rsi_mutex_destroy(&rsi_driver_cb->bt_common_tx_q.queue_mutex);
#endif
// Added WLAN define
#ifdef RSI_WLAN_ENABLE
  rsi_mutex_destroy(&rsi_driver_cb->wlan_tx_q.queue_mutex);
#endif
  rsi_mutex_destroy(&rsi_driver_cb->common_tx_q.queue_mutex);
// Added WLAN define
#ifdef RSI_WLAN_ENABLE
  status = rsi_semaphore_destroy(&rsi_driver_cb_non_rom->send_data_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
#endif
  status = rsi_semaphore_destroy(&rsi_driver_cb->rx_pool.pkt_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb->common_cb->common_tx_pool.pkt_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb_non_rom->common_cmd_send_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb_non_rom->common_cmd_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
#ifdef RSI_WLAN_ENABLE
  // Create wlan semaphore
  status = rsi_semaphore_destroy(&rsi_driver_cb_non_rom->nwk_cmd_send_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb_non_rom->nwk_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb_non_rom->wlan_cmd_send_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb_non_rom->wlan_cmd_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb->wlan_cb->wlan_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb->wlan_cb->wlan_tx_pool.pkt_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  rsi_mutex_destroy(&rsi_driver_cb->wlan_cb->wlan_mutex);
  rsi_socket_pool         = NULL;
  rsi_socket_pool_non_rom = NULL;
#endif
#if (defined RSI_BT_ENABLE || defined RSI_BLE_ENABLE || defined RSI_PROP_PROTOCOL_ENABLE)
  // Create bt semaphore
  status = rsi_semaphore_destroy(&rsi_driver_cb->bt_common_cb->bt_cmd_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb->bt_common_cb->bt_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb->bt_common_cb->bt_tx_pool.pkt_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
#ifdef SAPIS_BT_STACK_ON_HOST
  status = rsi_semaphore_destroy(&rsi_driver_cb->bt_ble_stack_cb->bt_tx_pool.pkt_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
#endif
#ifdef RSI_BT_ENABLE
  status = rsi_semaphore_destroy(&rsi_driver_cb->bt_classic_cb->bt_cmd_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb->bt_classic_cb->bt_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb->bt_classic_cb->bt_tx_pool.pkt_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
#endif
#ifdef RSI_BLE_ENABLE
  status = rsi_semaphore_destroy(&rsi_driver_cb->ble_cb->bt_cmd_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb->ble_cb->bt_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb->ble_cb->bt_tx_pool.pkt_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
#endif
#ifdef RSI_PROP_PROTOCOL_ENABLE
  status = rsi_semaphore_destroy(&rsi_driver_cb->prop_protocol_cb->bt_cmd_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb->prop_protocol_cb->bt_tx_pool.pkt_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
#endif
#endif
#ifdef RSI_ZB_ENABLE
  status = rsi_semaphore_destroy(&rsi_driver_cb->zigb_cb->zigb_tx_pool.pkt_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  status = rsi_semaphore_destroy(&rsi_driver_cb->zigb_cb->zigb_sem);
  if (status != RSI_ERROR_NONE) {
    return RSI_ERROR_SEMAPHORE_DESTROY_FAILED;
  }
  rsi_mutex_destroy(&rsi_driver_cb->zigb_cb->zigb_mutex);
#endif
#ifdef RSI_WITH_OS
  rsi_vport_exit_critical();
#endif
  // state update
  rsi_driver_cb->common_cb->state = RSI_COMMON_STATE_NONE;
#ifdef RSI_WLAN_ENABLE
  rsi_driver_cb->wlan_cb->state = RSI_WLAN_STATE_NONE;
#endif
  rsi_driver_cb_non_rom->device_state = RSI_DEVICE_STATE_NONE;
  return RSI_SUCCESS;
}

/*=============================================================================*/
/**
 * @fn          int32_t rsi_driver_version(uint8_t *request)
 * @brief       Get current driver version
 * @param[in]   request - pointer to fill driver version
 * @return      0              - Success \n
 * 				Non-Zero Value - Failure
 */

int32_t rsi_driver_version(uint8_t *request)
{
  memcpy(request, (uint8_t *)RSI_DRIVER_VERSION, sizeof(RSI_DRIVER_VERSION));
  return RSI_SUCCESS;
}
/** @} */
/** @addtogroup COMMON 
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_set_rtc_timer(module_rtc_time_t *timer)
 * @brief       Set the host rtc timer 
 * @pre 		\ref rsi_wireless_init() API needs to be called before this API
 * @param[in]   timer          - pointer to fill rtc time
 * @return      0              - Success \n
 * 				Non-Zero Value - Failure \n 
 * @note        Hour is 24-hour format only (valid values are 0 to 23). Valid values for Month are 0 to 11 (January to December). 
 */

int32_t rsi_set_rtc_timer(module_rtc_time_t *timer)
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  module_rtc_time_t *rtc;
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  if ((common_cb->state < RSI_COMMON_OPERMODE_DONE)) {
    //command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from common pool
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Take the user provided data and fill it in module rtc time structure
    rtc = (module_rtc_time_t *)pkt->data;

    memcpy(&rtc->tm_sec, &timer->tm_sec, 4);
    memcpy(&rtc->tm_min, &timer->tm_min, 4);
    memcpy(&rtc->tm_hour, &timer->tm_hour, 4);
    memcpy(&rtc->tm_mday, &timer->tm_mday, 4);
    memcpy(&rtc->tm_mon, &timer->tm_mon, 4);
    memcpy(&rtc->tm_year, &timer->tm_year, 4);
    memcpy(&rtc->tm_wday, &timer->tm_wday, 4);

#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif
    // Send set rtc timer request
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_SET_RTC_TIMER, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_SET_RTC_TIMER_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response stattus
  status = rsi_common_get_status();

  // Return the status
  return status;
}

//*==============================================*/
/**
 * @fn          int32_t rsi_get_ram_log(uint32_t addr,uint32_t length)
 * @brief       Get ram log on UART/UART2 
 * @pre 		\ref rsi_wireless_init() API needs to be called before this API.
 * @param[in]   addr           - Address in RS9116 module
 * @param[in]   length         - Chunk length to read from RS9116 module
 * @return 		0              - Success \n
 *				Non-Zero Value - Failure \n
 *				                 If return value is lesser than 0  \n
 *				                 -2: Invalid parameters  \n 
 *				                 -3: Command given in wrong state \n
 *				                 If return value is greater than 0 \n
 *								 0x0021,0x003E
 * @note       Refer Error Codes section for above error codes \ref SP16. 
 */

int32_t rsi_get_ram_log(uint32_t addr, uint32_t length)
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  rsi_ram_dump_t *ram;
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  if ((common_cb->state < RSI_COMMON_OPERMODE_DONE)) {
    //command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from common pool
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Take the user provided data and fill it in ram_dump structure
    ram = (rsi_ram_dump_t *)pkt->data;
    // address
    ram->addr = addr;
    // length
    ram->length = length;
#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif
    // Send ram dump request
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_GET_RAM_DUMP, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_GET_RAM_DUMP_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response stattus
  status = rsi_common_get_status();

  // Return the status
  return status;
}

/*==============================================*/
/**
 *
 * @fn         int32_t rsi_driver_memory_estimate(void)
 * @brief      Provide the memory required by the application
 * @param[in]  void
 * @return     Driver pool size 
 *        	   
 */

int32_t rsi_driver_memory_estimate(void)
{
  uint32_t actual_length = 0;

  // Calculate the Memory length of the application
  actual_length += RSI_DRIVER_POOL_SIZE;
  return actual_length;
}

/*==============================================*/
/**
 *
 * @fn         void rsi_uregister_events_callbacks(void (*callback_handler_ptr)(uint32_t event_num))
 * @brief      Deregister event handler for the given event
 * @param[in]  event_num - This is event numbera
 * @return     0              - Success \n 
 *             Non-Zero Value - Failure \n 
 */
void rsi_uregister_events_callbacks(void (*callback_handler_ptr)(uint32_t event_num))
{
  global_cb_p->rsi_driver_cb->unregistered_event_callback = callback_handler_ptr;
}
#ifndef RSI_WAIT_TIMEOUT_EVENT_HANDLE_TIMER_DISABLE
/*==============================================*/
/**
 *
 * @fn         void rsi_register_wait_timeout_error_callbacks(void (*callback_handler_ptr)(uint32_t status, uint32_t cmd_type))
 * @brief      Register SAPI wait timeout handler
 * @param[in]  cmd_type - This is Command 
 * @param[in]  status - Status 
 * @return     0        - Success \n 
 *             Non-Zero - Failure \n 
 */
void rsi_register_wait_timeout_error_callbacks(void (*callback_handler_ptr)(int32_t status, uint32_t cmd_type))
{
  rsi_driver_cb_non_rom->rsi_wait_timeout_handler_error_cb = callback_handler_ptr;
}
#endif
/** @} */
/** @addtogroup COMMON 
* @{
*/
//*==============================================*/
/**
 * @fn          int32_t rsi_get_rtc_timer(module_rtc_time_t *response)
 * @brief       Set the host rtc timer
 * @pre        \ref rsi_set_rtc_timer() API needs to be called before this API. 
 * @param[in] response       - This is the response of the requested command. \n This is an output parameter. 
 * @return     0              - Success \n 
 *             Non-Zero Value - Failure \n 
 *                              0x0021, 0x0025
 * @note       Hour is 24-hour format only (valid values are 0 to 23). Valid values for Month are 0 to 11 (January to December).
 */

int32_t rsi_get_rtc_timer(module_rtc_time_t *response)
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;
  if ((common_cb->state < RSI_COMMON_OPERMODE_DONE)) {
    //command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer  from common pool
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      // unlock mutex
      rsi_mutex_unlock(&common_cb->common_mutex);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    if (response != NULL) {
      // attach the buffer given by user
      common_cb->app_buffer = (uint8_t *)response;

      // length of the buffer provided by user
      common_cb->app_buffer_length = sizeof(module_rtc_time_t);
    } else {
      // Assign NULL to the app_buffer to avoid junk
      common_cb->app_buffer = NULL;

      // length of the buffer to 0
      common_cb->app_buffer_length = 0;
    }
#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif

    // Send get rtc timer request
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_GET_RTC_TIMER, pkt);
    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_GET_RTC_TIMER_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response stattus
  status = rsi_common_get_status();

  // Return the status
  return status;
}

/*==============================================*/
/**
 * @fn                int32_t rsi_set_config( uint32_t code, uint8_t value)
 * @brief             Configure XO Ctune value from host
 * @param[in]         Code - XO_CTUNE_FROM_HOST
 * @param[in]         value - Value to be configured
 * @return            0 		- Success \n
 *                    Non-Zero Value  - Failure
 */

int32_t rsi_set_config(uint32_t code, uint8_t value)
{
  rsi_pkt_t *pkt;
  rsi_set_config_t *set_config = NULL;
  int32_t status               = RSI_SUCCESS;

  // Get common cb structure pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  if (common_cb->state < RSI_COMMON_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }

  if (code != XO_CTUNE_FROM_HOST) {
    // if flag set is other than xo_ctune from host
    return RSI_ERROR_INVALID_SET_CONFIG_FLAG;
  }
  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer  from common pool
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      // Changing the common cmd state to allow
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    set_config = (rsi_set_config_t *)pkt->data;

    //code
    set_config->code = code;

    //xo ctune value from host
    set_config->values.xo_ctune = value;

#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif
    // send  set config request command
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_SET_CONFIG, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_SET_CONFIG_RESPONSE_WAIT_TIME);

    // Changing the common cmd state to allow
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  } else {
    // return common command error
    return status;
  }
  // get common command response status
  status = rsi_common_get_status();

  // Return the status
  return status;
}
#ifdef RSI_ASSERT_API
/*==============================================*/
/**
 * @fn          int32_t rsi_assert(void)
 * @brief       Trigger an assert
 * @param[in]   void
 * @return      0              - Success \n 
 *              Non-Zero Value - Failure
 */

int32_t rsi_assert(void)
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;
  if ((common_cb->state < RSI_COMMON_OPERMODE_DONE)) {
    //command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(COMMON_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    //command given in wrong state
    pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }
#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap |= BIT(0);
#endif

    // Send assert request
    status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_ASSERT, pkt);

    // wait on common semaphore
    rsi_wait_on_common_semaphore(&rsi_driver_cb_non_rom->common_cmd_sem, RSI_ASSERT_RESPONSE_WAIT_TIME);

    //Changing the common state to allow state
    rsi_check_and_update_cmd_state(COMMON_CMD, ALLOW);

  }

  else {
    //return common command error
    return status;
  }

  // get common command response stattus
  status = rsi_common_get_status();

  // Return the status
  return status;
}
#endif
#ifdef CONFIGURE_GPIO_FROM_HOST

//*==============================================*/
/**
 * @fn         int32_t rsi_gpio_pininit(uint8_t pin_num,uint8_t configuration)
 * @brief      Configure TA GPIO's using Command from host
 * @param[in]  pin_num       - GPIO Number : Valid values  0 - 15 
 * @param[in]  configuration - BIT[4]-  1 - input mode   0 - Output mode \n
 *                             BIT[0 - 1] - Drive strength : 0 - 2mA , 1 - 4mA , 2 - 8mA, 3 - 12mA  \n
 *                             BIT[6 - 7] : 0 - Hi-Z, 1- Pull-up, 2- Pull-down
 * @return 	   0              - Success \n
 *             Non-Zero Value - Failure
 */

int32_t rsi_gpio_pininit(uint8_t pin_num, uint8_t configuration)
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;
  rsi_gpio_pin_config_t *config_gpio;
  // take lock on common control block
  rsi_mutex_lock(&common_cb->common_mutex);

  // allocate command buffer  from common pool
  pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

  // If allocation of packet fails
  if (pkt == NULL) {
    // unlock mutex
    rsi_mutex_unlock(&common_cb->common_mutex);
    // return packet allocation failure error
    return RSI_ERROR_PKT_ALLOCATION_FAILURE;
  }

  config_gpio = (rsi_gpio_pin_config_t *)pkt->data;

  config_gpio->config_mode   = RSI_CONFIG_GPIO;
  config_gpio->pin_num       = pin_num;
  config_gpio->output_value  = 0;
  config_gpio->config_values = configuration;

  // Send GPIO configuration request
  status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_GPIO_CONFIG, pkt);

  // free the packet
  rsi_pkt_free(&common_cb->common_tx_pool, pkt);

  // unlock mutex
  rsi_mutex_unlock(&common_cb->common_mutex);

  // Return the status
  return status;
}

//*==============================================*/
/**
 * @fn          int32_t rsi_gpio_writepin(uint8_t pin_num,uint8_t value)
 * @brief       Write the  TA GPIO's high or low using Command from host 
 *@param[in]   	pin_num        -  GPIO Number : Valid values  0 - 15 
 *@param[in]    value          -  Value to be driven on GPIO \n
 *                                1 - Drive high \n
 *                                0 - Drive Low  \n
 *@return 	    0              -  Success \n
 *              Non-Zero Value -  Failure	
 */

int32_t rsi_gpio_writepin(uint8_t pin_num, uint8_t value)
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;
  rsi_gpio_pin_config_t *config_gpio;

  // take lock on common control block
  rsi_mutex_lock(&common_cb->common_mutex);

  // allocate command buffer  from common pool
  pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

  // If allocation of packet fails
  if (pkt == NULL) {
    // unlock mutex
    rsi_mutex_unlock(&common_cb->common_mutex);
    // return packet allocation failure error
    return RSI_ERROR_PKT_ALLOCATION_FAILURE;
  }
  config_gpio = (rsi_gpio_pin_config_t *)pkt->data;

  config_gpio->config_mode   = RSI_SET_GPIO;
  config_gpio->pin_num       = pin_num;
  config_gpio->output_value  = value;
  config_gpio->config_values = 0;

  // Send GPIO configuration request
  status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_GPIO_CONFIG, pkt);

  // free the packet
  rsi_pkt_free(&common_cb->common_tx_pool, pkt);

  // unlock mutex
  rsi_mutex_unlock(&common_cb->common_mutex);

  // Return the status
  return status;
}

//*==============================================*/
/**
 * @fn          int32_t rsi_gpio_readpin(uint8_t pin_num,uint8_t *gpio_value)
 * @brief       Read status of TA GPIO's using Command from host 
 * @param[in]   pin_num    -  GPIO Number : Valid values  0 - 15 
 * @param[in]   gpio_value - Address of variable to store the value
 * @return 		0              - Success \n 
 *				Non-Zero Value - Failure        
 */

int32_t rsi_gpio_readpin(uint8_t pin_num, uint8_t *gpio_value)
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;
  rsi_gpio_pin_config_t *config_gpio;

  // take lock on common control block
  rsi_mutex_lock(&common_cb->common_mutex);

  // allocate command buffer  from common pool
  pkt = rsi_pkt_alloc(&common_cb->common_tx_pool);

  // If allocation of packet fails
  if (pkt == NULL) {
    // unlock mutex
    rsi_mutex_unlock(&common_cb->common_mutex);
    // return packet allocation failure error
    return RSI_ERROR_PKT_ALLOCATION_FAILURE;
  }
  config_gpio = (rsi_gpio_pin_config_t *)pkt->data;

  config_gpio->config_mode   = RSI_GET_GPIO;
  config_gpio->pin_num       = pin_num;
  config_gpio->output_value  = 0;
  config_gpio->config_values = 0;

  // attach the buffer given by user
  common_cb->app_buffer = (uint8_t *)gpio_value;
  // length of the buffer provided by user
  common_cb->app_buffer_length = 1;

  // Send GPIO configuration request
  status = rsi_driver_common_send_cmd(RSI_COMMON_REQ_GPIO_CONFIG, pkt);

  // free the packet
  rsi_pkt_free(&common_cb->common_tx_pool, pkt);

  // unlock mutex
  rsi_mutex_unlock(&common_cb->common_mutex);

  // Return the status
  return status;
}

#endif
/** @} */
