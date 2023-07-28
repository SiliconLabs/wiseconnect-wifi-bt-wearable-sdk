/*******************************************************************************
* @file  rsi_http_client.c
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
#include "rsi_http_client.h"
/*
 * Global Variables
 * */
extern rsi_driver_cb_t *rsi_driver_cb;
uint8_t *rsi_itoa(uint32_t val, uint8_t *str);
/** @addtogroup NETWORK9
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_http_client_get_async(uint8_t flags, uint8_t *ip_address, uint16_t port, uint8_t *resource,
 *                                              uint8_t *host_name, uint8_t *extended_header, uint8_t *user_name, uint8_t *password,
 *                                               void (*http_client_response_handler)(uint16_t status, const uint8_t *buffer, const uint16_t length, const uint32_t moredata, uint16_t status_code)
 *												void (*http_client_response_handler)(uint16_t status, const uint8_t *buffer, const uint16_t length, const uint32_t moredata))
 * @brief      Send http get request to remote HTTP server. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  flags                            -  to select version and security , BIT(0) : 0 - IPv4 , 1 - IPv6, BIT(1): 0 - HTTP , 1 - HTTPS
 * @param[in]  ip_address                       - This is the server IP address 
 * @param[in]  port                             - This is the port number of HTTP server \n
 *					                               Note: HTTP server port is configurable on non-standard port also
 * @param[in]  resource                         - This is the URL string for requested resource 
 * @param[in]  host_name                        - This is the host name 
 * @param[in]  extended_header                  - This is the user defined extended header 
 * @param[in]  username                         - This is the username for server Authentication 
 * @param[in]  password                         - This is the password for server Authentication 
 * @param[in]  http_client_get_response_handler - This is the callback when asynchronous response comes for the request 
 * @param[in]  status                           - This is the status of response from module. This will return failure upon an internal error only. 
 * @param[in]  buffer                           - This is the buffer pointer 
 * @param[in]  length                           - This is the length of data 
 * @param[in]  more_data                        - 1 – No more data \n
 *												  0 – more data present 
 * @return      0              -  Success  \n
 *              Negative value - failure \n
 *                          -2 - Invalid parameters \n
 *				            -3 - Command given in wrong state \n
 *				            -4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16. 
 *
 */

int32_t rsi_http_client_get_async(uint8_t flags,
                                  uint8_t *ip_address,
                                  uint16_t port,
                                  uint8_t *resource,
                                  uint8_t *host_name,
                                  uint8_t *extended_header,
                                  uint8_t *user_name,
                                  uint8_t *password,
#if RSI_HTTP_STATUS_INDICATION_EN
                                  void (*http_client_response_handler)(uint16_t status,
                                                                       const uint8_t *buffer,
                                                                       const uint16_t length,
                                                                       const uint32_t moredata,
                                                                       uint16_t status_code)
#else
                                  void (*http_client_response_handler)(uint16_t status,
                                                                       const uint8_t *buffer,
                                                                       const uint16_t length,
                                                                       const uint32_t moredata)
#endif
)
{
  return rsi_http_client_async(RSI_HTTP_GET,
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
                               http_client_response_handler);
}

/** @} */

/** @addtogroup NETWORK9
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_http_client_post_async(uint8_t flags, uint8_t *ip_address, uint16_t port, uint8_t *resource, uint8_t *host_name,
 *                                          uint8_t *extended_header, uint8_t *user_name, uint8_t *password,uint8_t *post_data, uint32_t post_data_length,
 *                                          void(*http_client_response_handler)(uint16_t status, const uint8_t *buffer, const uint16_t length, const uint32_t moredata, uint16_t status_code),
 *										 void (*http_client_response_handler)(uint16_t status,  const uint8_t *buffer, const uint16_t length, const uint32_t moredata)) 
 * @brief      Send http post request to remote HTTP server. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  flags                             - to select version and security , BIT(0) : 0 - IPv4 , 1 - IPv6, BIT(1): 0 - HTTP , 1 - HTTPS
 * @param[in]  ip_address                        - This is the server IP address 
 * @param[in]  port                              - This is the port number of HTTP server 
 * @param[in]  resource                          - This is the URL string for requested resource 
 * @param[in]  host_name                         - This is the host name 
 * @param[in]  extended_header                   - This is the user defined extended header 
 * @param[in]  username                          - This is the username for server Authentication 
 * @param[in]  password                          - This is the password for server Authentication 
 * @param[in]  post_data                         - The is the HTTP data to be posted to server 
 * @param[in]  post_data_length                  - This is the post data length  
 * @param[in]  http_client_post_response_handler - This is the callback when asynchronous response comes for the request  
 * @param[in]  status                            - This is the status of response from module. This will return failure upon an internal error only. 
 * @param[in]  buffer                            - This is the buffer pointer 
 * @param[in]  length                            - This is the length of data 
 * @param[in]  moredata                          - 1 – No more data. \n
 *												   0 – more data present. \n  
 *												   2 – HTTP post Success  response. 
 * @return      0              -  Success  \n 
 *              Negative Value - Failure \n
 *                          -2 - Invalid parameters \n
 *				            -3 - Command given in wrong state \n
 *				            -4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16. 
 *
 */
int32_t rsi_http_client_post_async(uint8_t flags,
                                   uint8_t *ip_address,
                                   uint16_t port,
                                   uint8_t *resource,
                                   uint8_t *host_name,
                                   uint8_t *extended_header,
                                   uint8_t *user_name,
                                   uint8_t *password,
                                   uint8_t *post_data,
                                   uint32_t post_data_length,
#if RSI_HTTP_STATUS_INDICATION_EN
                                   void (*http_client_response_handler)(uint16_t status,
                                                                        const uint8_t *buffer,
                                                                        const uint16_t length,
                                                                        const uint32_t moredata,
                                                                        uint16_t status_code)
#else
                                   void (*http_client_response_handler)(uint16_t status,
                                                                        const uint8_t *buffer,
                                                                        const uint16_t length,
                                                                        const uint32_t moredata)
#endif
)
{
  return rsi_http_client_async(RSI_HTTP_POST,
                               flags,
                               ip_address,
                               port,
                               resource,
                               host_name,
                               extended_header,
                               user_name,
                               password,
                               post_data,
                               post_data_length,
                               http_client_response_handler);
}

/** @} */

/** @addtogroup NETWORK9
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_http_client_async(uint8_t type, uint8_t flags, uint8_t *ip_address, 
 *											uint16_t port, uint8_t *resource, uint8_t *host_name,
 *                                          uint8_t *extended_header, uint8_t *user_name, uint8_t *password,
 *											uint8_t *post_data, uint32_t post_data_length,
 *                                          void(*callback)(uint8_t status, const uint8_t *buffer, 
 *															const uint16_t length, const uint32_t moredata, uint16_t status_code),
 *											 void (*callback)(uint16_t status,  const uint8_t *buffer, 
 *															const uint16_t length,const uint32_t moredata))
 * @brief      Send http get request/http post request to remote HTTP server based on the type selected. 
 * @pre   \ref rsi_config_ipaddress() API needs to be called before this API.
 * @param[in]  type             - 0- RSI_HTTP_GET \n
 *								  1- RSI_HTTP_POST 
 * @param[in]  flags            - to select version and security , BIT(0) : 0 - IPv4 , 1 - IPv6, BIT(1): 0 - HTTP , 1 - HTTPS
 * @param[in]  ip_address       - This is the server IP address 
 * @param[in]  port             - This is the port number of HTTP server 
 * @param[in]  resource         - This is the URL string for requested resource
 *						          Note: HTTP server port is configurable on non-standard port also 
 * @param[in]  host_name        - This is the host name 
 * @param[in]  extended_header  - This is the user defined extended header 
 * @param[in]  username         - This is the username for server Authentication 
 * @param[in]  password         - This is the password for server Authentication 
 * @param[in]  post_data        - The is the HTTP data to be posted to server 
 * @param[in]  post_data_length - This is the post data length 
 * @param[in]  callback         - call back when asyncronous response comes for the request 
 * @param[in]  status           - This is the status of response from module. This will return failure upon an internal error only. 
 * @param[in]  buffer           - This is the buffer pointer 
 * @param[in]  length           - This is the length of data 
 * @param[in]  moredata         - 1 – No more data.
 * 								  0 – more data present. 
 * @return      0              -  Success  \n 
 *              Negative Value - Failure \n
 *                          -2 - Invalid parameters \n
 *				            -3 - Command given in wrong state \n
 *				            -4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16. 
 *
 */

int32_t rsi_http_client_async(uint8_t type,
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
#if RSI_HTTP_STATUS_INDICATION_EN
                              void (*callback)(uint16_t status,
                                               const uint8_t *buffer,
                                               const uint16_t length,
                                               const uint32_t moredata,
                                               uint16_t status_code)
#else
                              void (*callback)(uint16_t status,
                                               const uint8_t *buffer,
                                               const uint16_t length,
                                               const uint32_t moredata)
#endif
)
{
  rsi_req_http_client_t *http_client;
  rsi_pkt_t *pkt;
  int32_t status       = RSI_SUCCESS;
  uint8_t https_enable = 0;
  uint16_t http_length = 0;
  uint32_t send_size   = 0;
  uint8_t *host_desc   = NULL;
  uint8_t tmp_str[7]   = { 0 };

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  // Register http client response notify call back handler to NULL
  rsi_wlan_cb_non_rom->nwk_callbacks.http_client_response_handler = NULL;

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

  status = rsi_check_and_update_cmd_state(NWK_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // register callback
    if (callback != NULL) {
      // Register http client response notify call back handler
      rsi_wlan_cb_non_rom->nwk_callbacks.http_client_response_handler = callback;
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

    if (flags & RSI_SUPPORT_HTTP_POST_DATA) {
      // set HTTP post big data feature bit
      https_enable |= RSI_SUPPORT_HTTP_POST_DATA;
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
      // Check for HTTP post data feature
      if (flags & RSI_SUPPORT_HTTP_POST_DATA) {
        post_data = rsi_itoa(post_data_length, tmp_str);

        post_data_length = rsi_strlen(post_data);
      }

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

    if (type) {
      // send HTTP Post request command
      status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_HTTP_CLIENT_POST, pkt);
    } else {
      // send HTTP Get request command
      status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_HTTP_CLIENT_GET, pkt);
    }

  } else {
    // return nwk command error
    return status;
  }

  // Return the status
  return status;
}

/** @} */

/** @addtogroup NETWORK9
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_http_client_abort(void)
 * @brief      Abort any ongoing HTTP request from the client.
 * @pre   \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  void 
 * @return      0              -  Success  \n
 *              Negative Value - Failure \n
 *				            -3 - Command given in wrong state \n
 *				            -4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16.
 */

int32_t rsi_http_client_abort(void)
{
  rsi_pkt_t *pkt;
  int32_t status = RSI_SUCCESS;

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

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif

    // send join command to start wps
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_HTTP_ABORT, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_HTTP_ABORT_RESPONSE_WAIT_TIME);
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

/** @addtogroup NETWORK9
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_http_client_put_create(void)
 * @brief      Create the http put client 
 * @pre   \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  void 
 * @return      0              -  Success  \n 
 *              Negative Value - Failure \n
 *				            -3 - Command given in wrong state \n
 *				            -4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16. 
 */

int32_t rsi_http_client_put_create(void)
{
  rsi_pkt_t *pkt;
  int32_t status     = RSI_SUCCESS;
  uint16_t send_size = 0;
  uint8_t *host_desc = NULL;

  rsi_http_client_put_req_t *http_put_req;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  // Register http client response notify call back handler to NULL
  rsi_wlan_cb_non_rom->nwk_callbacks.rsi_http_client_put_response_handler = NULL;

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

    http_put_req = (rsi_http_client_put_req_t *)pkt->data;

    // memset the packet data to insert NULL between fields
    memset(&pkt->data, 0, sizeof(rsi_http_client_put_req_t));

    // Fill command type
    http_put_req->command_type = HTTP_CLIENT_PUT_CREATE;

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_http_client_put_req_t) - HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH;

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif

    // send http put command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_HTTP_CLIENT_PUT, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_HTTP_CLIENT_PUT_RESPONSE_WAIT_TIME);

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

/** @addtogroup NETWORK9
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_http_client_put_delete(void)
 * @brief      Delete the created http put client  
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API
 * @param[in]  void 
 * @return      0              -  Success  \n 
 *              Negative Value - Failure \n
 *				            -3 - Command given in wrong state \n
 *				            -4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16. 
 */

int32_t rsi_http_client_put_delete(void)
{
  rsi_pkt_t *pkt;
  int32_t status     = RSI_SUCCESS;
  uint16_t send_size = 0;
  uint8_t *host_desc = NULL;

  rsi_http_client_put_req_t *http_put_req;

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

    http_put_req = (rsi_http_client_put_req_t *)pkt->data;

    // memset the packet data to insert NULL between fields
    memset(&pkt->data, 0, sizeof(rsi_http_client_put_req_t));

    // Fill command type
    http_put_req->command_type = HTTP_CLIENT_PUT_DELETE;

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_http_client_put_req_t) - HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH;

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

    // send http put command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_HTTP_CLIENT_PUT, pkt);

  } else {
    // return nwk command error
    return status;
  }

  // Return the status
  return status;
}

/** @} */

/** @addtogroup NETWORK9
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_http_client_put_start(uint8_t flags, uint8_t *ip_address, uint32_t port_number, 
 *                           uint8_t *resource, uint8_t *host_name, uint8_t *extended_header, 
 *                           uint8_t *user_name, uint8_t *password, uint32_t content_length,
 *                           void (*callback)(uint16_t status, uint8_t type, const uint8_t *buffer, 
 *						   	 uint16_t length, const uint8_t end_of_put_pkt))
 * @brief 	   Start the http client put process  
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  flags            -  to select version and security , BIT(0) : 0 - IPv4 , 1 - IPv6, BIT(1): 0 - HTTP , 1 - HTTPS
 * @param[in]  ip_address       - This is the server IP address 
 * @param[in]  port_number      - This is the port number of HTTP server 
 * @param[in]  resource         - This is the URL string for requested resource 
 * @param[in]  host_name        - This is the host name 
 * @param[in]  extended_header  - This is the user defined extended header 
 * @param[in]  user_name        - This is the username for server Authentication 
 * @param[in]  password         - This is the password for server Authentication 
 * @param[in]  content_length   - This is the total length of http data 
 * @param[in]  post_data_length - http data length to be posted to server
 * @param[in]  callback         - call back when asyncronous response comes for the request
 * @param[in]  status           - This is the status code 
 * @param[in]  type             - This is the HTTP Client PUT command type 
 * @param[in]  buffer           - This is the buffer pointer 
 * @param[in]  length           - This is the length of data 
 * @param[in]  end_of_put_pkt   -  This is the End of file or HTTP resource content 
 * @return      0              -  Success  \n 
 *              Negative Value - Failure \n
 *				            -3 - Command given in wrong state \n
 *				            -4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16. 
 */

int32_t rsi_http_client_put_start(
  uint8_t flags,
  uint8_t *ip_address,
  uint32_t port_number,
  uint8_t *resource,
  uint8_t *host_name,
  uint8_t *extended_header,
  uint8_t *user_name,
  uint8_t *password,
  uint32_t content_length,
  void (*callback)(uint16_t status, uint8_t type, const uint8_t *buffer, uint16_t length, const uint8_t end_of_put_pkt))
{
  rsi_pkt_t *pkt;
  int32_t status       = RSI_SUCCESS;
  uint8_t https_enable = 0;
  uint16_t http_length = 0;
  uint16_t send_size   = 0;
  uint8_t *host_desc   = NULL;
  rsi_http_client_put_req_t *http_put_req;
  rsi_http_client_put_start_t *http_put_start;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(NWK_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // register callback
    if (callback != NULL) {
      // Register http client response notify call back handler
      rsi_wlan_cb_non_rom->nwk_callbacks.rsi_http_client_put_response_handler = callback;
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

    http_put_req = (rsi_http_client_put_req_t *)pkt->data;

    // memset the packet data to insert NULL between fields
    memset(&pkt->data, 0, sizeof(rsi_http_client_put_req_t));

    http_put_start = &http_put_req->http_client_put_struct.http_client_put_start;

    // Fill command type
    http_put_req->command_type = HTTP_CLIENT_PUT_START;

    // Fill ipversion
    if (flags & RSI_IPV6) {
      // Set ipv6 version
      http_put_start->ip_version = RSI_IP_VERSION_6;
    } else {
      // Set ipv4 version
      http_put_start->ip_version = RSI_IP_VERSION_4;
    }

    if (flags & RSI_SSL_ENABLE) {
      // Set https feature
      https_enable = 1;

      // set default by NULL delimiter
      https_enable |= BIT(1);
    }
    if (flags & RSI_HTTP_USER_DEFINED_CONTENT_TYPE) {
      // Enable user given content type in extended header
      https_enable |= RSI_HTTP_USER_DEFINED_CONTENT_TYPE;
    }
    // Fill https features parameters
    rsi_uint16_to_2bytes(http_put_start->https_enable, https_enable);
    // Fill http server port number
    rsi_uint32_to_4bytes(http_put_start->port_number, port_number);

    // Fill Total resource content length
    rsi_uint32_to_4bytes(http_put_start->content_length, content_length);

    // Copy username
    rsi_strcpy(http_put_req->http_put_buffer, user_name);
    http_length = rsi_strlen(user_name) + 1;

    // Copy  password
    rsi_strcpy((http_put_req->http_put_buffer) + http_length, password);
    http_length += rsi_strlen(password) + 1;

    // Copy  Host name
    rsi_strcpy((http_put_req->http_put_buffer) + http_length, host_name);
    http_length += rsi_strlen(host_name) + 1;

    // Copy IP address
    rsi_strcpy((http_put_req->http_put_buffer) + http_length, ip_address);
    http_length += rsi_strlen(ip_address) + 1;

    // Copy URL resource
    rsi_strcpy((http_put_req->http_put_buffer) + http_length, resource);
    http_length += rsi_strlen(resource) + 1;

    if (extended_header != NULL) {
      // Copy Extended header
      rsi_strcpy((http_put_req->http_put_buffer) + http_length, extended_header);
      http_length += rsi_strlen(extended_header);
    }

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_http_client_put_req_t) - HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH + http_length;

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

    // send http put command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_HTTP_CLIENT_PUT, pkt);

  } else {
    // return nwk command error
    return status;
  }

  // Return the status
  return status;
}

/** @} */

/** @addtogroup NETWORK9
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_http_client_put_pkt(uint8_t *file_content, uint16_t current_chunk_length)
 * @brief      Put http data onto the http server for the created URL resource. 
 * @pre   \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  file_content         - HTTP data content
 * @param[in]  current_chunk_length - HTTP data current chunk length
 * @return      0              -  Success  \n 
 *              Negative Value - Failure \n
 *				            -3 - Command given in wrong state \n
 *				            -4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16.
 */

int32_t rsi_http_client_put_pkt(uint8_t *file_content, uint16_t current_chunk_length)
{
  rsi_pkt_t *pkt;
  int32_t status = RSI_SUCCESS;
  rsi_http_client_put_req_t *http_put_req;
  rsi_http_client_put_data_req_t *http_put_data;
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

    http_put_req = (rsi_http_client_put_req_t *)pkt->data;

    // memset the packet data to insert NULL between fields
    memset(&pkt->data, 0, sizeof(rsi_http_client_put_req_t));

    http_put_data = &http_put_req->http_client_put_struct.http_client_put_data_req;

    // Fill command type
    http_put_req->command_type = HTTP_CLIENT_PUT_PKT;

    // Fill http put current_chunk_length
    rsi_uint16_to_2bytes(http_put_data->current_length, current_chunk_length);

    // Fill resource content
    memcpy((uint8_t *)http_put_req->http_put_buffer, file_content, current_chunk_length);

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_http_client_put_req_t) - HTTP_CLIENT_PUT_MAX_BUFFER_LENGTH + current_chunk_length;

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

    // send http put command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_HTTP_CLIENT_PUT, pkt);

    if (rsi_wlan_cb_non_rom->nwk_callbacks.rsi_http_client_put_response_handler == NULL) {
#ifndef RSI_NWK_SEM_BITMAP
      rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
      // wait on nwk semaphore
      rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_HTTP_CLIENT_PUT_RESPONSE_WAIT_TIME);
      // get wlan/network command response status
      status = rsi_wlan_get_nwk_status();

      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
    }
  } else {
    // return nwk command error
    return status;
  }

  // Return the status
  return status;
}

/** @} */

/** @addtogroup NETWORK9
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_http_client_post_data(uint8_t *file_content, uint16_t current_chunk_length, 
 *                         void(* rsi_http_post_data_response_handler)(uint16_t status, const uint8_t *buffer, const uint16_t length, const uint32_t moredata),
 *                         void (*rsi_http_post_data_response_handler)(uint16_t status, const uint8_t *buffer, const uint16_t length,  const uint32_t moredata))
 * @brief      Send the http post data packet to remote HTTP server. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API.
 * @param[in]  file_content         - This is the user given http file content 
 * @param[in]  current_chunk_length - This is the length of the current http data 
 * @param[in]  callback             - This is the callback when asynchronous response comes for the request. 
 * @param[in]  status               - This is the status of response from module. This will return failure upon an internal error only. 
 * @param[in]  buffer               - This is the buffer pointer 
 * @param[in]  length               - This is the length of data
 * @param[in]  more_data            - 1 – No more data \n 0 – more data \n 4 – HTTP post data response \n 8 – HTTP post data receive response 
 * @return      0              -  Success  \n 
 *              Negative Value - Failure \n
 *                          -2 - Invalid parameters \n
 *				            -3 - Command given in wrong state \n
 *				            -4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16. 
 */

int32_t rsi_http_client_post_data(uint8_t *file_content,
                                  uint16_t current_chunk_length,
#if RSI_HTTP_STATUS_INDICATION_EN
                                  void (*rsi_http_post_data_response_handler)(uint16_t status,
                                                                              const uint8_t *buffer,
                                                                              const uint16_t length,
                                                                              const uint32_t moredata)
#else
                                  void (*rsi_http_post_data_response_handler)(uint16_t status,
                                                                              const uint8_t *buffer,
                                                                              const uint16_t length,
                                                                              const uint32_t moredata)
#endif
)
{
  rsi_pkt_t *pkt;
  int32_t status = RSI_SUCCESS;
  rsi_http_client_post_data_req_t *http_post_data;
  uint16_t send_size = 0;
  uint8_t *host_desc = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(NWK_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // register callback
    if (rsi_http_post_data_response_handler != NULL) {
      // Register http client response notify call back handler
      rsi_wlan_cb_non_rom->nwk_callbacks.rsi_http_client_post_data_response_handler =
        rsi_http_post_data_response_handler;
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

    http_post_data = (rsi_http_client_post_data_req_t *)pkt->data;

    // memset the packet data to insert NULL between fields
    memset(&pkt->data, 0, sizeof(rsi_http_client_post_data_req_t));

    // Fill http post data current_chunk_length
    rsi_uint16_to_2bytes(http_post_data->current_length, current_chunk_length);

    // Fill resource content
    memcpy((uint8_t *)http_post_data->http_post_data_buffer, file_content, current_chunk_length);

    // Using host descriptor to set payload length
    send_size =
      sizeof(rsi_http_client_post_data_req_t) - HTTP_CLIENT_POST_DATA_MAX_BUFFER_LENGTH + current_chunk_length;

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

    // send http put command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_HTTP_CLIENT_POST_DATA, pkt);

  } else {
    // return nwk command error
    return status;
  }

  // Return the status
  return status;
}
/** @} */
