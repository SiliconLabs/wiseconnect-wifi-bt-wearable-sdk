/*******************************************************************************
* @file  rsi_ota_fw_up.c
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
#include "rsi_firmware_upgradation.h"

/** @addtogroup NETWORK20
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_ota_firmware_upgradation(uint8_t flags,
 *                                                   uint8_t *server_ip,
 *                                                   uint32_t server_port,
 *                                                   uint16_t chunk_number,
 *                                                   uint16_t timeout,
 *                                                   uint16_t tcp_retry_count,
 *                                       void (*ota_fw_up_response_handler)(uint16_t status, uint16_t chunk_number))
 * @brief       Create an otaf client.This initializes the client with given configuration. 
 * @pre         \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]   flags                      - To select IPv6 version, a bit in flags is set. By default IP version is set to IPV4. \n
 *                                           RSI_IPV6 - BIT(0) To select IPv6 version 
 * @param[in]   server_ip                  - This is the OTAF server IP address 
 * @param[in]   server_port                - This is the OTAF server port number 
 * @param[in]   chunk_number               - This is the firmware content request chunk number 
 * @param[in]   timeout                    - This is the TCP receive packet timeout 
 * @param[in]   tcp_retry_count            - This is the TCP retransmissions count 
 * @param[in]   ota_fw_up_response_handler - This is the callback when asynchronous response comes for the firmware upgrade request. 
 * @param[in]   status                     - This is the status code 
 * @param[in]   chunk_number               - This is the chunk number of the firmware content 
 * @return      0              -  Success  \n
 *              Negative Value -  failure \n
 *                          -3 - Command given in wrong state \n
 *                          -4 - Buffer not available to serve the command
 */

int32_t rsi_ota_firmware_upgradation(uint8_t flags,
                                     uint8_t *server_ip,
                                     uint32_t server_port,
                                     uint16_t chunk_number,
                                     uint16_t timeout,
                                     uint16_t tcp_retry_count,
                                     void (*ota_fw_up_response_handler)(uint16_t status, uint16_t chunk_number))
{
  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_req_ota_fwup_t *otaf_fwup = NULL;

  status = rsi_check_and_update_cmd_state(NWK_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // register callback
    if (ota_fw_up_response_handler != NULL) {
      // Register smtp client response notify call back handler
      rsi_wlan_cb_non_rom->nwk_callbacks.rsi_ota_fw_up_response_handler = ota_fw_up_response_handler;
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

    otaf_fwup = (rsi_req_ota_fwup_t *)pkt->data;

    // Check ip version
    if (!(flags & RSI_IPV6)) {
      // Fill the IP version
      otaf_fwup->ip_version = RSI_IP_VERSION_4;

      // Set IP address to localhost
      memcpy(otaf_fwup->server_ip_address.ipv4_address, server_ip, RSI_IPV4_ADDRESS_LENGTH);
    } else {
      // Fill the IP version
      otaf_fwup->ip_version = RSI_IP_VERSION_6;

      // Set IP address to localhost
      memcpy(otaf_fwup->server_ip_address.ipv6_address, server_ip, RSI_IPV6_ADDRESS_LENGTH);
    }

    // Fill server port number
    rsi_uint32_to_4bytes(otaf_fwup->server_port, server_port);

    // Fill chunk number
    rsi_uint16_to_2bytes(otaf_fwup->chunk_number, chunk_number);

    // Fill timeout
    rsi_uint16_to_2bytes(otaf_fwup->timeout, timeout);

    // Fill tcp retry count
    rsi_uint16_to_2bytes(otaf_fwup->retry_count, tcp_retry_count);

    // Send OTAF command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_OTA_FWUP, pkt);

  } else {
    // return nwk command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}
/** @} */
