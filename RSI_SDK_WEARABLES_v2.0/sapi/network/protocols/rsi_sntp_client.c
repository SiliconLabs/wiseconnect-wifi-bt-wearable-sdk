/*******************************************************************************
* @file  rsi_sntp_client.c
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

#include "rsi_sntp_client.h"

#include "rsi_nwk.h"

/** @addtogroup NETWORK12
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_sntp_client_create_async(uint8_t flags, uint8_t *server_ip, uint8_t sntp_method, uint16_t sntp_timeout,
                           void(*sntp_client_create_response_handler)(uint16_t status,const uint8_t cmd_type, const uint8_t *buffer ))
 * @brief      Create the sntp client.
 * @param[in]  flags                               - To select IP version and security BIT(0) - RSI_IPV6 \n
 *                                                   Set this bit to enable IPv6, by default it is configured to IPv4 \n
 *                                                   BIT(1) - RSI_SSL_ENABLE Set this bit to enable SSL feature  
 * @param[in]  server_ip                           - This is the server IP address
 * @param[in]  sntp_method                         - These are the SNTP methods to use \n
 *                                                   1-For Broadcast Method 2-For Unicast Method 
 * @param[in]  sntp_timeout                        - This is the SNTP timeout value 
 * @param[in]  sntp_client_create_response_handler - This is the callback function when asynchronous response comes for the request. \n
 *                                                   status: This is the status code \n
 *                                                   cmd_type: This is the command type \n
 *                                                   buffer: This is the buffer pointer
 * @return     0              -  Success  \n
 *             Negative Value - Failure \n
 *                         -2 - Invalid parameters, call back not registered  \n
 *                         -3 - Command given in wrong state \n
 *                         -4 - Buffer not available to serve the command
 *
 */

int32_t rsi_sntp_client_create_async(uint8_t flags,
                                     uint8_t *server_ip,
                                     uint8_t sntp_method,
                                     uint16_t sntp_timeout,
                                     void (*rsi_sntp_client_create_response_handler)(uint16_t status,
                                                                                     const uint8_t cmd_type,
                                                                                     const uint8_t *buffer))

{
  rsi_sntp_client_t *sntp_client;
  rsi_pkt_t *pkt;
  int32_t status     = RSI_SUCCESS;
  uint16_t send_size = 0;
  uint8_t *host_desc = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->opermode == RSI_WLAN_CONCURRENT_MODE || wlan_cb->opermode == RSI_WLAN_ACCESS_POINT_MODE) {
    // In concurrent mode or AP mode, state should be in RSI_WLAN_STATE_CONNECTED to accept this command
    if ((wlan_cb->state < RSI_WLAN_STATE_CONNECTED)) {
      //command given in wrong state
      return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
    }
  } else {
    // if state is not in ipconfig done state
    if ((wlan_cb->state < RSI_WLAN_STATE_IP_CONFIG_DONE)) {
      // Command given in wrong state
      return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
    }
  }

  status = rsi_check_and_update_cmd_state(NWK_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // register callback
    if (rsi_sntp_client_create_response_handler != NULL) {
      // Register pop3 client response notify call back handler
      rsi_wlan_cb_non_rom->nwk_callbacks.rsi_sntp_client_create_response_handler =
        rsi_sntp_client_create_response_handler;
    } else {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return invalid command error
      return RSI_ERROR_INVALID_PARAM;
    }

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    sntp_client = (rsi_sntp_client_t *)pkt->data;

    // memset the packet data
    memset(&pkt->data, 0, sizeof(rsi_sntp_client_t));

    // Fill command type
    sntp_client->command_type = RSI_SNTP_CREATE;

    if ((sntp_method != RSI_SNTP_BROADCAST_MODE) && (sntp_method != RSI_SNTP_UNICAST_MODE)) {
      // default SNTP is in  UNICAST mode
      sntp_method = RSI_SNTP_UNICAST_MODE;
    }

    // Fill SNTP method
    sntp_client->sntp_method = sntp_method;

    // sntp time out
    rsi_uint16_to_2bytes(sntp_client->sntp_timeout, sntp_timeout);

    // Check for IP version
    if (!(flags & RSI_IPV6)) {
      sntp_client->ip_version = RSI_IP_VERSION_4;
      memcpy(sntp_client->server_ip_address.ipv4_address, server_ip, RSI_IPV4_ADDRESS_LENGTH);
    } else {
      sntp_client->ip_version = RSI_IP_VERSION_6;
      memcpy(sntp_client->server_ip_address.ipv4_address, server_ip, RSI_IPV6_ADDRESS_LENGTH);
    }

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_sntp_client_t);

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

    rsi_wlan_cb_non_rom->nwk_cmd_rsp_pending |= SNTP_RESPONSE_PENDING;
    // send SNTP Get request command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SNTP_CLIENT, pkt);

  } else {
    // return nwk command error
    return status;
  }

  // Return the status
  return status;
}

/** @} */

/** @addtogroup NETWORK12
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_sntp_client_gettime(uint16_t length, uint8_t *sntp_time_rsp)
 * @brief       Get the current time parameters. 
 * @param[in]   length        - This is the length of the buffer 
 * @param[in]   sntp_time_rsp - This is the current time response 
 * @return     0              -  Success  \n
 *             Negative Value - Failure \n
 *                         -2 - Invalid parameters, call back not registered \n
 *                         -3 - Command given in wrong state \n
 *                         -4 - Buffer not available to serve the command
 *
 */

int32_t rsi_sntp_client_gettime(uint16_t length, uint8_t *sntp_time_rsp)
{

  rsi_sntp_client_t *sntp_client;
  rsi_pkt_t *pkt;
  int32_t status     = RSI_SUCCESS;
  uint16_t send_size = 0;
  uint8_t *host_desc = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->opermode == RSI_WLAN_CONCURRENT_MODE || wlan_cb->opermode == RSI_WLAN_ACCESS_POINT_MODE) {
    // In concurrent mode or AP mode, state should be in RSI_WLAN_STATE_CONNECTED to accept this command
    if ((wlan_cb->state < RSI_WLAN_STATE_CONNECTED)) {
      //command given in wrong state
      return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
    }
  } else {
    // if state is not in ipconfig done state
    if ((wlan_cb->state < RSI_WLAN_STATE_IP_CONFIG_DONE)) {
      // Command given in wrong state
      return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
    }
  }

  status = rsi_check_and_update_cmd_state(NWK_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // attach the buffer given by user
    rsi_driver_cb_non_rom->nwk_app_buffer = (uint8_t *)sntp_time_rsp;

    // length of the buffer provided by user
    rsi_driver_cb_non_rom->nwk_app_buffer_length = length;

    // memset the packet data
    memset(&pkt->data, 0, sizeof(rsi_sntp_client_t));

    sntp_client = (rsi_sntp_client_t *)pkt->data;

    // Fill command type
    sntp_client->command_type = RSI_SNTP_GETTIME;

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_sntp_client_t);

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif

    // send SNTP Get request command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SNTP_CLIENT, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_SNTP_RESPONSE_WAIT_TIME);
    // get wlan/network command response status
    status = rsi_wlan_get_nwk_status();

    //Changing the nwk state to allow
    rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);

  } else {
    // return nwk command error
    return status;
  }

  // Return the status
  return status;
}

/** @} */

/** @addtogroup NETWORK12
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_sntp_client_gettime_date(uint16_t length, uint8_t *sntp_time_date_rsp)
 * @brief       Get the current time in time date format parameters. 
 * @param[in]   length             - This is the length of the buffer 
 * @param[in]   sntp_time_date_rsp - This is the current time and date response  
 * @return     0              -  Success  \n
 *             Negative Value - Failure \n
 *                         -2 - Invalid parameters, call back not registered \n
 *                         -3 - Command given in wrong state \n
 *                         -4 - Buffer not available to serve the command
 *
 */

int32_t rsi_sntp_client_gettime_date(uint16_t length, uint8_t *sntp_time_date_rsp)
{
  rsi_sntp_client_t *sntp_client;
  rsi_pkt_t *pkt;
  int32_t status     = RSI_SUCCESS;
  uint16_t send_size = 0;
  uint8_t *host_desc = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->opermode == RSI_WLAN_CONCURRENT_MODE || wlan_cb->opermode == RSI_WLAN_ACCESS_POINT_MODE) {
    // In concurrent mode or AP mode, state should be in RSI_WLAN_STATE_CONNECTED to accept this command
    if ((wlan_cb->state < RSI_WLAN_STATE_CONNECTED)) {
      //command given in wrong state
      return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
    }
  } else {
    // if state is not in ipconfig done state
    if ((wlan_cb->state < RSI_WLAN_STATE_IP_CONFIG_DONE)) {
      // Command given in wrong state
      return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
    }
  }

  status = rsi_check_and_update_cmd_state(NWK_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // attach the buffer given by user
    rsi_driver_cb_non_rom->nwk_app_buffer = (uint8_t *)sntp_time_date_rsp;

    // length of the buffer provided by user
    rsi_driver_cb_non_rom->nwk_app_buffer_length = length;

    // memset the packet data
    memset(&pkt->data, 0, sizeof(rsi_sntp_client_t));

    sntp_client = (rsi_sntp_client_t *)pkt->data;

    // Fill command type
    sntp_client->command_type = RSI_SNTP_GETTIME_DATE;

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_sntp_client_t);

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif

    // send SNTP Get request command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SNTP_CLIENT, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_SNTP_RESPONSE_WAIT_TIME);
    // get wlan/network command response status
    status = rsi_wlan_get_nwk_status();

    //Changing the nwk state to allow
    rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);

  } else {
    // return nwk command error
    return status;
  }

  // Return the status
  return status;
}

/** @} */

/** @addtogroup NETWORK12
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_sntp_client_server_info(uint16_t length, uint8_t *sntp_server_response)
 * @brief       Create SNTP client
 * @param[in]   length               - Reponse buffer length
 * @param[in]   sntp_server_response - Pointer to the SNTP Reponse buffer 
 * @return     0              -  Success \n
 *             Negative Value - Failure 
 */

int32_t rsi_sntp_client_server_info(uint16_t length, uint8_t *sntp_server_response)
{

  rsi_sntp_client_t *sntp_client;
  rsi_pkt_t *pkt;
  int32_t status     = RSI_SUCCESS;
  uint16_t send_size = 0;
  uint8_t *host_desc = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  // if state is not in card ready received state
  if (wlan_cb->opermode == RSI_WLAN_CONCURRENT_MODE || wlan_cb->opermode == RSI_WLAN_ACCESS_POINT_MODE) {
    // In concurrent mode or AP mode, state should be in RSI_WLAN_STATE_CONNECTED to accept this command
    if ((wlan_cb->state < RSI_WLAN_STATE_CONNECTED)) {
      //command given in wrong state
      return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
    }
  } else {
    // if state is not in ipconfig done state
    if ((wlan_cb->state < RSI_WLAN_STATE_IP_CONFIG_DONE)) {
      // Command given in wrong state
      return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
    }
  }

  status = rsi_check_and_update_cmd_state(NWK_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // attach the buffer given by user
    rsi_driver_cb_non_rom->nwk_app_buffer = (uint8_t *)sntp_server_response;

    // length of the buffer provided by user
    rsi_driver_cb_non_rom->nwk_app_buffer_length = length;

    // memset the packet data
    memset(&pkt->data, 0, sizeof(rsi_sntp_client_t));

    sntp_client = (rsi_sntp_client_t *)pkt->data;

    // Fill command type
    sntp_client->command_type = RSI_SNTP_GET_SERVER_INFO;

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_sntp_client_t);

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif

    // send SNTP  Get request command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SNTP_CLIENT, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_SNTP_RESPONSE_WAIT_TIME);
    // get wlan/network command response status
    status = rsi_wlan_get_nwk_status();

    //Changing the nwk state to allow
    rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);

  } else {
    // return nwk command error
    return status;
  }

  // Return the status
  return status;
}

/** @} */

/** @addtogroup NETWORK12
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_sntp_client_delete_async(void)
 * @brief       Delete the SNTP client. 
 * @param[in]   void 
 * @return     0              -  Success  \n
 *             Negative Value - Failure \n
 *                         -2 - Invalid parameters, call back not registered \n
 *                         -3 - Command given in wrong state \n
 *                         -4 - Buffer not available to serve the command
 *
 */

int32_t rsi_sntp_client_delete_async(void)
{
  rsi_sntp_client_t *sntp_client;
  rsi_pkt_t *pkt;
  int32_t status     = RSI_SUCCESS;
  uint16_t send_size = 0;
  uint8_t *host_desc = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->opermode == RSI_WLAN_CONCURRENT_MODE || wlan_cb->opermode == RSI_WLAN_ACCESS_POINT_MODE) {
    // In concurrent mode or AP mode, state should be in RSI_WLAN_STATE_CONNECTED to accept this command
    if ((wlan_cb->state < RSI_WLAN_STATE_CONNECTED)) {
      //command given in wrong state
      return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
    }
  } else {
    // if state is not in ipconfig done state
    if ((wlan_cb->state < RSI_WLAN_STATE_IP_CONFIG_DONE)) {
      // Command given in wrong state
      return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
    }
  }

  status = rsi_check_and_update_cmd_state(NWK_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the common state to allow state
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // memset the packet data
    memset(&pkt->data, 0, sizeof(rsi_sntp_client_t));

    sntp_client = (rsi_sntp_client_t *)pkt->data;

    // Fill command type
    sntp_client->command_type = RSI_SNTP_DELETE;

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_sntp_client_t);

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

    rsi_wlan_cb_non_rom->nwk_cmd_rsp_pending |= SNTP_RESPONSE_PENDING;
    // send SNTP Get request command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SNTP_CLIENT, pkt);

  } else {
    // return nwk command error
    return status;
  }

  // Return the status
  return status;
}
/** @} */
