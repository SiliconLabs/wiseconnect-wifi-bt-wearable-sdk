/*******************************************************************************
* @file  rsi_multicast.c
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
#include "rsi_multicast.h"
/** @addtogroup NETWORK3
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_multicast(uint8_t flags, int8_t *ip_address, uint8_t command_type)
 * @brief       Helper function for actual APIs.
 * @param[in]   flags        -  to select version and security, BIT(0) : 0 - IPv4 , 1 - IPv6 
 * @param[in]   ip_address   - multicast IP address
 * @param[in]   command_type - type of the command JOIN/LEAVE
 * @return      0              -  Success \n
 *              Negative value -  Failure 
 *
 */

static int32_t rsi_multicast(uint8_t flags, int8_t *ip_address, uint8_t command_type)
{
  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;
  rsi_req_multicast_t *multicast;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

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

    multicast = (rsi_req_multicast_t *)pkt->data;

    // Fill IP version and IP address
    if (flags & RSI_IPV6) {
      // Fill IPv6 version
      rsi_uint16_to_2bytes(multicast->ip_version, RSI_IP_VERSION_6);

      // Fill IPv6 address
      memcpy(multicast->multicast_address.ipv6_address, ip_address, RSI_IPV6_ADDRESS_LENGTH);
    } else {
      // Fill IPv4 version
      rsi_uint16_to_2bytes(multicast->ip_version, RSI_IP_VERSION_4);

      // Fill IPv4 address
      memcpy(multicast->multicast_address.ipv4_address, ip_address, RSI_IPV4_ADDRESS_LENGTH);
    }

    // Fill command type
    rsi_uint16_to_2bytes(multicast->type, command_type);

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif

    // send set FTP Create command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_MULTICAST, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_MULTICAST_RESPONSE_WAIT_TIME);
    // get wlan/network command response status
    status = rsi_wlan_get_nwk_status();

    //Changing the nwk state to allow
    rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);

  } else {
    // return nwk command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/** @} */

/** @addtogroup NETWORK3
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_multicast_join(uint8_t flags, int8_t *ip_address)
 * @brief       Join to a multicast group. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]   flags      - To select the IP version. \n
 *                           BIT(0) – RSI_IPV6.Set this bit to enable IPv6 \n
 * 							 by default it is configured to IPv4. 
 * @param[in]   ip_address - IPv4/IPv6 address of multicast group. 
 * @return	    0              -  Success  \n
 *              Negative Value -  failure \n
 *                          -3 - Command given in wrong state \n
 *                          -4 - Buffer not available to serve the command 
 */
int32_t rsi_multicast_join(uint8_t flags, int8_t *ip_address)
{
  int32_t status = RSI_SUCCESS;

  status = rsi_multicast(flags, ip_address, RSI_MULTICAST_JOIN);

  return status;
}

/** @} */

/** @addtogroup NETWORK3
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_multicast_leave(uint8_t flags, int8_t *ip_address)
 * @brief       Leave the multicast group. 
 * @pre   \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]   flags - To select the IP version. \n
 *                      BIT(0) – RSI_IPV6
 *                      Set this bit to enable IPv6 \n by default it is configured to IPv4. 
 * @param[in]   ip_address - IPv4/IPv6 address of multicast group. 
 * @return      0              - Success \n
 *              Negative Value - Failure 
 *
 */
int32_t rsi_multicast_leave(uint8_t flags, int8_t *ip_address)
{
  int32_t status = RSI_SUCCESS;

  status = rsi_multicast(flags, ip_address, RSI_MULTICAST_LEAVE);

  return status;
}
/** @} */
