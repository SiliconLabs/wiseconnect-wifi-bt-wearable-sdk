/*******************************************************************************
* @file  rsi_mdnsd.c
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

// Include Driver header file
#include "rsi_driver.h"
#include "rsi_mdnsd.h"
/** @addtogroup NETWORK15
* @{
*/
/*==============================================*/
/**
 * @fn               int32_t rsi_mdnsd_init(uint8_t ip_version, uint16_t ttl, uint8_t *host_name)
 * @brief            Initialize the MDNSD service in WiSeConnect Device.It creates MDNS daemon. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]        ip_version - To select the IP version. 4 – To select IPv4 \n 6 – To select IPv6  
 * @param[in]        ttl        - This is the time to live. THis is the time in seconds for which service should be active 
 * @param[in]        host_name  - This is the host name which is used as host name in Type A record. 
 * @return      0              - Success \n
 *              Negative value - Failure 
 */
int32_t rsi_mdnsd_init(uint8_t ip_version, uint16_t ttl, uint8_t *host_name)
{
  rsi_req_mdnsd_t *mdnsd;
  rsi_pkt_t *pkt;
  int32_t status     = RSI_SUCCESS;
  uint16_t send_size = 0;
  uint8_t *host_desc = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if ((rsi_strlen(host_name) + 1) > MDNSD_BUFFER_SIZE) {
    return RSI_ERROR_INSUFFICIENT_BUFFER;
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

    mdnsd = (rsi_req_mdnsd_t *)pkt->data;

    // memset the packet data to insert NULL between fields
    memset(&pkt->data, 0, sizeof(rsi_req_mdnsd_t));

    // Fill command type
    mdnsd->command_type = RSI_MDNSD_INIT;

    // Fill ip version
    mdnsd->mdnsd_struct.mdnsd_init.ip_version = ip_version;

    // Fill time to live
    rsi_uint16_to_2bytes(mdnsd->mdnsd_struct.mdnsd_init.ttl, ttl);

    // Copy host name
    rsi_strcpy(mdnsd->buffer, host_name);

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_req_mdnsd_t) - MDNSD_BUFFER_SIZE + rsi_strlen(mdnsd->buffer) + 1;

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif

    // send MDNSD request command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_MDNSD, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_MDNSD_RESPONSE_WAIT_TIME);
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

/** @addtogroup NETWORK15
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_mdnsd_register_service(uint16_t port,
 *                                  uint16_t ttl,
 *                                 uint8_t more,
 *                                 uint8_t *service_ptr_name,
 *                                 uint8_t *service_name,
 *                                 uint8_t *service_text)
 * @brief      Add a service / start service discovery.  
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API 
 * @param[in]  port             - This is the port number on which service should be added. 
 * @param[in]  ttl              - This is the time to live. This is the time in seconds for which service should be active 
 * @param[in]  more             - This byte should be set to '1' when there are more services to add \n
 *                                 0 - This is last service, starts MDNS service. \n
 *                                 1 - Still more services will be added. 
 * @param[in]  service_ptr_name - This is the name to be added in Type-PTR record 
 * @param[in]  service_name     - This is the name to be added in Type-SRV record(Service name) 
 * @param[in]  service_text     - This is the text field to be added in Type-TXT record 
 * @return      0              - Success \n
 *              Negative value - Failure  
 *
 */
int32_t rsi_mdnsd_register_service(uint16_t port,
                                   uint16_t ttl,
                                   uint8_t more,
                                   uint8_t *service_ptr_name,
                                   uint8_t *service_name,
                                   uint8_t *service_text)
{
  rsi_req_mdnsd_t *mdnsd;
  rsi_pkt_t *pkt;
  int32_t status     = RSI_SUCCESS;
  uint16_t send_size = 0;
  uint8_t *host_desc = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if ((rsi_strlen(service_ptr_name) + rsi_strlen(service_name) + rsi_strlen(service_text) + 3) > MDNSD_BUFFER_SIZE) {
    return RSI_ERROR_INSUFFICIENT_BUFFER;
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

    mdnsd = (rsi_req_mdnsd_t *)pkt->data;

    // memset the packet data to insert NULL between fields
    memset(&pkt->data, 0, sizeof(rsi_req_mdnsd_t));

    // Fill command type
    mdnsd->command_type = RSI_MDNSD_REGISTER_SERVICE;

    // Fill port number
    rsi_uint16_to_2bytes(mdnsd->mdnsd_struct.mdnsd_register_service.port, port);

    // Fill time to live
    rsi_uint16_to_2bytes(mdnsd->mdnsd_struct.mdnsd_register_service.ttl, ttl);

    // more
    mdnsd->mdnsd_struct.mdnsd_register_service.more = more;

    // Copy service pointer name
    rsi_strcpy(mdnsd->buffer, service_ptr_name);
    send_size = rsi_strlen(service_ptr_name) + 1;

    // Copy service name
    rsi_strcpy((mdnsd->buffer) + send_size, service_name);
    send_size += rsi_strlen(service_name) + 1;

    // Copy service text
    rsi_strcpy((mdnsd->buffer) + send_size, service_text);
    send_size += rsi_strlen(service_text) + 1;

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_req_mdnsd_t) - MDNSD_BUFFER_SIZE + send_size;

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif

    // send MDNSD request command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_MDNSD, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_MDNSD_RESPONSE_WAIT_TIME);
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

/** @addtogroup NETWORK15
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_mdnsd_deinit(void)
 * @brief      Delete the mdnsd service. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  void  
 * @return     0              - Success \n
 *             Negative value - Failure  
 */

int32_t rsi_mdnsd_deinit(void)
{
  rsi_req_mdnsd_t *mdnsd;
  rsi_pkt_t *pkt;
  int32_t status     = RSI_SUCCESS;
  uint16_t send_size = 0;
  uint8_t *host_desc = NULL;

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

    mdnsd = (rsi_req_mdnsd_t *)pkt->data;

    // memset the packet data to insert NULL between fields
    memset(&pkt->data, 0, sizeof(rsi_req_mdnsd_t));

    // Fill command type
    mdnsd->command_type = RSI_MDNSD_DEINIT;

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_req_mdnsd_t) - MDNSD_BUFFER_SIZE;

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif

    // send MDNSD request command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_MDNSD, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_MDNSD_RESPONSE_WAIT_TIME);
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
