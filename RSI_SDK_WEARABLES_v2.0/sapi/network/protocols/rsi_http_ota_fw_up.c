/*******************************************************************************
* @file  rsi_http_ota_fw_up.c
* @brief This file contains API's to get http data and post http data for requested URL
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
/** @addtogroup NETWORK9
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_http_fw_update(uint8_t flags, uint8_t *ip_address, uint16_t port_no, uint8_t *resource,
                                                uint8_t *host_name, uint8_t *extended_header, uint8_t *user_name, uint8_t *password,
                                                void(*http_otaf_response_handler)(uint16_t status, const uint8_t *buffer))
 * @brief      Initiate firmware update from a HTTP server
 * @param[in]  flags                      -  to select version and security , BIT(0) : 0 - IPv4 , 1 - IPv6, BIT(1): 0 - HTTP , 1 - HTTPS
 * @param[in]  ip_address                 - server IP address
 * @param[in]  port_no                    - port number : 80 - HTTP \n 443 - HTTPS
 * @param[in]  resource                   - URL string for requested resource
 * @param[in]  host_name                  - host name
 * @param[in]  extended_header            - extender header if present
 * @param[in]  username                   - username for server Authentication
 * @param[in]  password                   - password for server Authentication
 * @param[in]  http_otaf_response_handler - call back when asyncronous response comes for the request
 * @return      0              - Success \n
 *              Negative value - Failure 
 *
 */
int32_t rsi_http_fw_update(uint8_t flags,
                           uint8_t *ip_address,
                           uint16_t port,
                           uint8_t *resource,
                           uint8_t *host_name,
                           uint8_t *extended_header,
                           uint8_t *user_name,
                           uint8_t *password,
                           void (*http_otaf_response_handler)(uint16_t status, const uint8_t *buffer))
{
  return rsi_http_otaf_async(RSI_HTTP_OTAF,
                             flags,
                             ip_address,
                             port,
                             resource,
                             host_name,
                             extended_header,
                             user_name,
                             password,
                             NULL,
                             0,
                             http_otaf_response_handler);
}
/*==============================================*/
/**
 * @fn         int32_t rsi_http_otaf_async(uint8_t type, uint8_t flags, uint8_t *ip_address, uint16_t port, uint8_t *resource, uint8_t *host_name,
                                             uint8_t *extended_header, uint8_t *user_name, uint8_t *password, uint8_t *post_data, uint32_t post_data_length,
                                             void (*callback)(uint16_t status, const uint8_t *buffer))
 * @brief      Post the http data for the requested URL to http server
 * @param[in]  type             - 0 - HTTPGET \n 1 - HTTPPOST
 * @param[in]  flags            -  to select version and security , BIT(0) : 0 - IPv4 , 1 - IPv6, BIT(1): 0 - HTTP , 1 - HTTPS
 * @param[in]  ip_address       - server IP address
 * @param[in]  port             - port number : 80 - HTTP \n 443 - HTTPS
 * @param[in]  resource         - URL string for requested resource
 * @param[in]  host_name        - host name
 * @param[in]  extended_header  - extender header if present
 * @param[in]  username         - username for server Authentication
 * @param[in]  password         - password for server Authentication
 * @param[in]  post_data        - http data to be posted to server
 * @param[in]  post_data_length - http data length to be posted to server
 * @param[in]  callback         - call back when asyncronous response comes for the request
 * @return     0              - Success \n
 *             Negative value - Failure 
 */
int32_t rsi_http_otaf_async(uint8_t type,
                            uint8_t flags,
                            uint8_t *ip_address,
                            uint16_t port,
                            uint8_t *resource,
                            uint8_t *host_name,
                            uint8_t *extended_header,
                            uint8_t *user_name,
                            uint8_t *password,
                            uint8_t *post_data,
                            uint32_t post_data_length,
                            void (*callback)(uint16_t status, const uint8_t *buffer))
{
  rsi_req_http_client_t *http_client = NULL;
  rsi_pkt_t *pkt                     = NULL;
  int32_t status                     = RSI_SUCCESS;
  uint8_t https_enable               = 0;
  uint16_t http_length               = 0;
  uint32_t send_size                 = 0;
  uint8_t *host_desc                 = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  // Register http client response notify call back handler to NULL
  rsi_wlan_cb_non_rom->nwk_callbacks.http_otaf_response_handler = NULL;

  if (wlan_cb->opermode == RSI_WLAN_CONCURRENT_MODE || wlan_cb->opermode == RSI_WLAN_ACCESS_POINT_MODE) {
    // In concurrent mode and AP mode state should be in RSI_WLAN_STATE_CONNECTED to accept this command
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

  if (rsi_check_and_update_cmd_state(NWK_CMD, IN_USE) == RSI_SUCCESS) {
    // register callback
    if (callback != NULL) {
      // Register http client response notify call back handler
      rsi_wlan_cb_non_rom->nwk_callbacks.http_otaf_response_handler = callback;
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

    http_client = (rsi_req_http_client_t *)pkt->data;

    // memset the packet data to insert NULL between fields
    memset(&pkt->data, 0, sizeof(rsi_req_http_client_t));

    // Fill ipversion
    if (flags & RSI_IPV6) {
      // Set ipv6 version
      rsi_uint16_to_2bytes(http_client->ip_version, RSI_IP_VERSION_6);
    } else {
      // Set ipv4 version
      rsi_uint16_to_2bytes(http_client->ip_version, RSI_IP_VERSION_4);
    }

    if (flags & RSI_SSL_ENABLE) {
      // Set https feature
      https_enable = 1;
    }

    // set default by NULL delimiter
    https_enable |= BIT(1);

    //ssl versions

    if (flags & RSI_SSL_V_1_0) {
      https_enable |= RSI_SSL_V_1_0;
    }
    if (flags & RSI_SSL_V_1_1) {
      https_enable |= RSI_SSL_V_1_1;
    }

    if (flags & RSI_SUPPORT_HTTP_V_1_1) {
      // set HTTP version 1.1 feature bit
      https_enable |= RSI_SUPPORT_HTTP_V_1_1;
    }

    // Fill https features parameters
    rsi_uint16_to_2bytes(http_client->https_enable, https_enable);

    // Fill port no
    http_client->port = port;

    // Copy username
    rsi_strcpy(http_client->buffer, user_name);
    http_length = rsi_strlen(user_name) + 1;

    // Copy  password
    rsi_strcpy((http_client->buffer) + http_length, password);
    http_length += rsi_strlen(password) + 1;

    // Check for HTTP_V_1.1 and Empty host name
    if ((flags & RSI_SUPPORT_HTTP_V_1_1) && (rsi_strlen(host_name) == 0)) {
      host_name = ip_address;
    }

    // Copy  Host name
    rsi_strcpy((http_client->buffer) + http_length, host_name);
    http_length += rsi_strlen(host_name) + 1;

    // Copy IP address
    rsi_strcpy((http_client->buffer) + http_length, ip_address);
    http_length += rsi_strlen(ip_address) + 1;

    // Copy URL resource
    rsi_strcpy((http_client->buffer) + http_length, resource);
    http_length += rsi_strlen(resource) + 1;

    if (extended_header != NULL) {

      // Copy Extended header
      rsi_strcpy((http_client->buffer) + http_length, extended_header);
      http_length += rsi_strlen(extended_header);
    }

    if (type) {
      // Copy Httppost data
      memcpy((http_client->buffer) + http_length + 1, post_data, post_data_length);

      http_length += (post_data_length + 1);
    }

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_req_http_client_t) - RSI_HTTP_BUFFER_LEN + http_length;

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint32_to_4bytes(host_desc, (send_size & 0xFFF));

    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_HTTP_OTAF, pkt);
  } else {
    // return nwk command error
    return RSI_ERROR_NWK_CMD_IN_PROGRESS;
  }

  // Return the status
  return status;
}

/** @} */