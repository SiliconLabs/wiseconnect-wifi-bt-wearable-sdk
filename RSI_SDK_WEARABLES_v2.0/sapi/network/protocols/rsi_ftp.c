/*******************************************************************************
* @file  rsi_ftp.c
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

/** @addtogroup NETWORK8
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ftp_connect(uint16_t flags, int8_t *server_ip, int8_t *username, int8_t *password, uint32_t server_port)
 * @brief      Create FTP objects and connects to the FTP server on the given server port.This should be the first command for accessing FTP server.
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]   flags      - This are the network flags. Each bit in the flag has its own significance \n
 *                           BIT(0) – RSI_IPV6 \n
 *                           Set this bit to enable IPv6. By default it is configured to IPv4. 
 *                           BIT(1) to BIT(15) are reserved for future use 
 * @param[in]  server_ip   - This is the FTP server IP address to connect   
 * @param[in]  username    - This is the username for server authentication 
 * @param[in]  password    - This is the password for server authentication 
 * @param[in]  server_port - This is the port number of FTP server \n
 *							 Note: FTP server port is configurable on nonstandard port also
 * @return      0              - Success \n
 *              Negative Value - Failure \n
 *				            -3 - Command given in wrong state \n
 *				            -4 - Buffer not availableto serve the command 
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16. 
 */

int32_t rsi_ftp_connect(uint16_t flags, int8_t *server_ip, int8_t *username, int8_t *password, uint32_t server_port)
{

  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;
  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_ftp_file_ops_t *file_ops = NULL;

  rsi_ftp_connect_t *ftp_connect = NULL;

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
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    file_ops = (rsi_ftp_file_ops_t *)pkt->data;

    // get command type as FTP Create
    file_ops->command_type = RSI_FTP_CREATE;

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
    // send set FTP Create command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_FTP_RESPONSE_WAIT_TIME);
    // get wlan/network command response status
    status = rsi_wlan_get_nwk_status();

    if (status != RSI_SUCCESS) {
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // Return the status if error in sending command occurs
      return status;
    }

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);
    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    ftp_connect = (rsi_ftp_connect_t *)pkt->data;

    // Memset the packet
    memset(pkt, 0, sizeof(rsi_ftp_connect_t));

    // Set command type as FTP connect
    ftp_connect->command_type = RSI_FTP_CONNECT;

    if (!(flags & RSI_IPV6)) {
      // fill IP version
      ftp_connect->ip_version = RSI_IP_VERSION_4;

      // Fill IP address
      memcpy(ftp_connect->server_ip_address.ipv4_address, server_ip, RSI_IPV4_ADDRESS_LENGTH);
    } else {
      // fill IP version
      ftp_connect->ip_version = RSI_IP_VERSION_6;

      // Fill IPv6 address
      memcpy(ftp_connect->server_ip_address.ipv6_address, server_ip, RSI_IPV6_ADDRESS_LENGTH);
    }
    if (username) {
      // Copy login username
      rsi_strcpy(ftp_connect->username, username);
    }

    if (password) {
      // Copy login password
      rsi_strcpy(ftp_connect->password, password);
    }

    // copy FTP Server port
    rsi_uint32_to_4bytes(ftp_connect->server_port, server_port);

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
    // send set FTP Connect command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_FTP_RESPONSE_WAIT_TIME);

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

/** @addtogroup NETWORK8
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ftp_disconnect()
 * @brief      Disconnect from the FTP server and also destroys the FTP objects.Once the FTP objects are
 * 			   destroyed, FTP server cannot be accessed.For further accessing,FTP objects should be created again. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @return      0              - Success \n
 *              Negative Value - Failure \n
 *				            -3 - Command given in wrong state \n
 *				            -4 - Buffer not availableto serve the command 
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16.
 *
 */
int32_t rsi_ftp_disconnect(void)
{
  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;
  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_ftp_file_ops_t *ftp_ops = NULL;

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
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    ftp_ops = (rsi_ftp_file_ops_t *)pkt->data;

    // get command type as FTP Create
    ftp_ops->command_type = RSI_FTP_DISCONNECT;

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
    // send set FTP Create command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_FTP_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_nwk_status();

    if (status != RSI_SUCCESS) {
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // Return the status if error in sending command occurs
      return status;
    }
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    ftp_ops = (rsi_ftp_file_ops_t *)pkt->data;

    // Memset the packet
    memset(pkt, 0, sizeof(rsi_ftp_file_ops_t));

    // get command type as FTP Create
    ftp_ops->command_type = RSI_FTP_DESTROY;

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
    // send set FTP Create command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_FTP_RESPONSE_WAIT_TIME);

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

/** @addtogroup NETWORK8
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ftp_file_write(int8_t *file_name)
 * @brief      Open a file in the specified path on the FTP server. 
 * @pre   \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  file_name - This is the file name or filename including path can be given. 
 * @return      0              - Success \n
 *              Negative Value - Failure \n
 *				            -3 - Command given in wrong state \n
 *				            -4 - Buffer not availableto serve the command 
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16. 
 */
int32_t rsi_ftp_file_write(int8_t *file_name)
{

  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_ftp_file_ops_t *ftp_ops = NULL;

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
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    ftp_ops = (rsi_ftp_file_ops_t *)pkt->data;

    // get command type as FTP file write
    ftp_ops->command_type = RSI_FTP_FILE_WRITE;

    // copy the filename/path
    rsi_strcpy(ftp_ops->file_name, file_name);

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
    // send  FTP  command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_FTP_RESPONSE_WAIT_TIME);

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

/** @addtogroup NETWORK8
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_ftp_file_write_content(uint16_t flags, int8_t *file_content,int16_t content_length,uint8_t end_of_file)
 * @brief       Write the content into the file which is opened using \ref rsi_ftp_file_write() API. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]   flags          - These are the network flags. Each bit in the flag has its own significance \n
 *                                BIT(0) – RSI_IPV6 \n
 *                                Set this bit to enable IPv6. By default, it is configured to IPv4 \n
 *                                BIT(1) to BIT(15) are reserved for future use 
 * @param[in]   file_content   - This is the data stream to be written into the file 
 * @param[in]   content_length - This is the file content length 
 * @param[in]   end_of_file    - This flag indicates the end of file \n
 *                                1 – This chunk represents the end of content to be written into the file \n
 *                                0 – This are the extra data which is pending to write into the file \n
 *							      Note: This API can be called multiple times to append data into the same file and at the last chunk
 *							      this flag should be 1.
 * @return      0              -  Success \n 
 *              Negative Value -  Failure 
 */
int32_t rsi_ftp_file_write_content(uint16_t flags, int8_t *file_content, int16_t content_length, uint8_t end_of_file)
{

  int32_t status = RSI_SUCCESS;
  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_ftp_file_write_t *ftp_file_write = NULL;

  rsi_pkt_t *pkt;

  uint32_t file_offset = 0;

  uint32_t chunk_size = 0;

  uint32_t send_size = 0;

  uint8_t head_room = 0;

  uint8_t *host_desc = NULL;

  if (!(flags & RSI_IPV6)) {
    // Headroom for IPv4
    head_room = RSI_TCP_FRAME_HEADER_LEN;
  } else {
    // Headroom for IPv6
    head_room = RSI_TCP_V6_FRAME_HEADER_LEN;
  }

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
    while (content_length) {
      // allocate command buffer  from wlan pool
      pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);
      // If allocation of packet fails
      if (pkt == NULL) {
        //Changing the nwk state to allow
        rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
        // return packet allocation failure error
        return RSI_ERROR_PKT_ALLOCATION_FAILURE;
      }

      ftp_file_write = (rsi_ftp_file_write_t *)pkt->data;

      // Memset the packet
      memset(pkt, 0, (sizeof(pkt) + sizeof(rsi_ftp_file_write_t)));

      // get command type as file delete
      ftp_file_write->command_type = RSI_FTP_FILE_WRITE_CONTENT;

      if (content_length > RSI_FTP_MAX_CHUNK_LENGTH) {
        chunk_size = RSI_FTP_MAX_CHUNK_LENGTH;
#ifndef RSI_UART_INTERFACE
        rsi_driver_cb->wlan_cb->expected_response = RSI_WLAN_RSP_ASYNCHRONOUS;
#endif

        // copy end of file
        ftp_file_write->end_of_file = 0;

      } else {
        chunk_size = content_length;

        // copy end of file
        ftp_file_write->end_of_file = end_of_file;

#ifndef RSI_UART_INTERFACE
        if (!end_of_file) {
          rsi_driver_cb->wlan_cb->expected_response = RSI_WLAN_RSP_ASYNCHRONOUS;
        }
#endif
      }
      // Copy file content
      memcpy((((uint8_t *)ftp_file_write) + head_room), (file_content + file_offset), chunk_size);

      // Add headroom to send size
      send_size = chunk_size + head_room;

      // get the host descriptor
      host_desc = (pkt->desc);

      // Fill data length in the packet host descriptor
      rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

      if (rsi_driver_cb->wlan_cb->expected_response != RSI_WLAN_RSP_ASYNCHRONOUS) {
#ifndef RSI_NWK_SEM_BITMAP
        rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
      }
      // send set FTP Create command
      status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

      if (rsi_driver_cb->wlan_cb->expected_response != RSI_WLAN_RSP_ASYNCHRONOUS) {
        // wait on nwk semaphore
        rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_FTP_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_nwk_status();
        // If  fails ,donot send other chunks
        if (status != RSI_SUCCESS) {
          break;
        }
      }
      // Increase file offset
      file_offset += chunk_size;

      // Decrese file remaining size
      content_length -= chunk_size;
    }

    if (rsi_driver_cb->wlan_cb->expected_response != RSI_WLAN_RSP_ASYNCHRONOUS) {
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
    }
  } else {
    // return nwk command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/** @} */

/** @addtogroup NETWORK8
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_ftp_file_read_aysnc(int8_t *file_name,
 * 												void (*call_back_handler_ptr)(uint16_t status, uint8_t *file_content, uint16_t content_length, uint8_t end_of_file))
 * @brief       Read the content from the specified file on the FTP server. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]   file_name              - This is the file name or filename including path can be given. 
 * @param[in]   call_back_handler_ptr  - This is the callback function when asynchronous response comes for the file read request 
 * @param[in]   status                 -  This is the status code. The other parameters are valid only if status is 0 
 * @param[in]   file_content           - file content 
 * @param[in]   content_length         -  This is the length of file content 
 * @param[in]   end_of_file            -  This indicates the end of file \n
 *                                        1 – No more data \n
 *                                        0 – more data present 
 * @return       0             - Success \n
 *              Negative value - Failure \n
 *                          -2 - Invalid parameter,expects call back handler \n
 *                          -3 - Command given in wrong state \n
 *                          -4 - Buffer not available to serve the command 
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16.
 */
int32_t rsi_ftp_file_read_aysnc(
  int8_t *file_name,
  void (*call_back_handler_ptr)(uint16_t status, uint8_t *file_content, uint16_t content_length, uint8_t end_of_file))
{

  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_ftp_file_ops_t *ftp_ops = NULL;

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
    if (call_back_handler_ptr != NULL) {
      // Register FTP file read response notify call back handler
      rsi_wlan_cb_non_rom->nwk_callbacks.ftp_file_read_call_back_handler = call_back_handler_ptr;
    } else {
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return invalid command error
      return RSI_ERROR_INVALID_PARAM;
    }
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);
    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    ftp_ops = (rsi_ftp_file_ops_t *)pkt->data;

    // get command type as FTP file Read
    ftp_ops->command_type = RSI_FTP_FILE_READ;

    // copy the filename/path
    rsi_strcpy(ftp_ops->file_name, file_name);

    // send  FTP  command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

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

/** @addtogroup NETWORK8
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ftp_file_delete(int8_t *file_name)
 * @brief      Delete the file which is present in the specified path on the FTP server. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  file_name - This is the file name or filename including path can be given to delete 
 * @return      0             - Success \n
 *             Negative Value - Failure \n
 *                         -3 - Command given in wrong state \n
 *                         -4 - Buffer not available to serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16.
 */
int32_t rsi_ftp_file_delete(int8_t *file_name)
{

  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_ftp_file_ops_t *ftp_ops = NULL;

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
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    ftp_ops = (rsi_ftp_file_ops_t *)pkt->data;

    // get command type as file delete
    ftp_ops->command_type = RSI_FTP_FILE_DELETE;

    // copy the filename/path to delete
    rsi_strcpy(ftp_ops->file_name, file_name);

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
    // send set FTP Create command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_FTP_RESPONSE_WAIT_TIME);

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

/** @addtogroup NETWORK8
* @{
*/
/*==============================================*/
/**
 * @fn         rsi_ftp_file_rename(int8_t *old_file_name, int8_t *new_file_name)
 * @brief      Rename the file with a new name on the FTP server. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API 
 * @param[in]  old_file_name - This is the filename/file name which has to be renamed 
 * @param[in]  new_file_name - This is the new file name 
 * @return     0              -  Success  \n
 *             Negative Value - Failure \n
 *                         -3 - Command given in wrong state \n 
 *                         -4 - Buffer not available to serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16.
 */
int32_t rsi_ftp_file_rename(int8_t *old_file_name, int8_t *new_file_name)
{

  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_ftp_file_rename_t *file_rename = NULL;

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
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    file_rename = (rsi_ftp_file_rename_t *)pkt->data;

    // get command type as file rename
    file_rename->command_type = RSI_FTP_FILE_RENAME;

    // copy the filename/path of old file name
    rsi_strcpy(file_rename->old_file_name, old_file_name);

    // copy the filename/path of new file name
    rsi_strcpy(file_rename->new_file_name, new_file_name);

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
    // send set FTP Create command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_FTP_RESPONSE_WAIT_TIME);

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

/** @addtogroup NETWORK8
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t  rsi_ftp_directory_create(int8_t *directory_name)
 * @brief      Create a directory on the FTP server. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  directory_name - This is the directory name (with path if required) to create 
 * @return     0              -  Success  \n
 *             Negative Value - Failure \n
 *                         -3 - Command given in wrong state \n 
 *                         -4 - Buffer not available to serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16.
 */
int32_t rsi_ftp_directory_create(int8_t *directory_name)
{

  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_ftp_file_ops_t *ftp_ops = NULL;

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
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    ftp_ops = (rsi_ftp_file_ops_t *)pkt->data;

    // get command type as directory create
    ftp_ops->command_type = RSI_FTP_DIRECTORY_CREATE;

    // copy the directory name to create
    rsi_strcpy(ftp_ops->file_name, directory_name);

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
    // send set FTP Create command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_FTP_RESPONSE_WAIT_TIME);

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

/** @addtogroup NETWORK8
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t  rsi_ftp_directory_delete(int8_t *directory_name)
 * @brief      Delete the directory on the FTP server. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API 
 * @param[in]  directory_name - directory name (with path if required) to delete 
 * @return     0              -  Success  \n
 *             Negative Value - Failure \n
 *                         -3 - Command given in wrong state \n 
 *                         -4 - Buffer not available to serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16.
 */
int32_t rsi_ftp_directory_delete(int8_t *directory_name)
{
  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_ftp_file_ops_t *ftp_ops = NULL;

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
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    ftp_ops = (rsi_ftp_file_ops_t *)pkt->data;

    // get command type as directory delete
    ftp_ops->command_type = RSI_FTP_DIRECTORY_DELETE;

    // copy the filename/path to delete
    rsi_strcpy(ftp_ops->file_name, directory_name);

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
    // send set FTP  command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_FTP_RESPONSE_WAIT_TIME);

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

/** @addtogroup NETWORK8
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t  rsi_ftp_directory_set(int8_t *directory_path)
 * @brief      Change the current working directory to the specified directory path on the FTP server. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  directory_path - This is the directory name (with path if required) to create 
 * @return     0              -  Success  \n
 *             Negative Value - Failure \n
 *                         -3 - Command given in wrong state \n 
 *                         -4 - Buffer not available to serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16.
 */
int32_t rsi_ftp_directory_set(int8_t *directory_path)
{
  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_ftp_file_ops_t *ftp_ops = NULL;

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
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    ftp_ops = (rsi_ftp_file_ops_t *)pkt->data;

    // get command type as directory delete
    ftp_ops->command_type = RSI_FTP_DIRECTORY_SET;

    // copy the filename/path to delete
    rsi_strcpy(ftp_ops->file_name, directory_path);

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
    // send set FTP Create command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_FTP_RESPONSE_WAIT_TIME);

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

/** @addtogroup NETWORK8
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ftp_directory_list_async(int8_t *directory_path,void (*call_back_handler_ptr)(uint16_t status, uint8_t *directory_list, uint16_t length , uint8_t end_of_list)) 
 * @brief      Get the list of directories present in the specified directory on the FTP server. 
 * @pre   \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  directory_path        - directory path(with path if required) to list
 * @param[in]  call_back_handler_ptr -  This is the callback function when asynchronous response comes for the directory list request 
 * @param[in]  status                -  status code.Other parameters are valid only if status is 0 
 * @param[in]  directory_list        -  This is the stream of data with directory list as content 
 * @param[in]  length                -  This is the length of content 
 * @param[in]  end_of_list           -  This indicates end of list \n
 *                                      1 – No more data \n
 *                                      0 – more data present  
 * @return     0              -  Success  \n
 *             Negative Value - Failure \n
 *                         -2 - Invalid parameter, expects call back handler \n
 *                         -3 - Command given in wrong state \n 
 *                         -4 - Buffer not available to serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16.
 */
int32_t rsi_ftp_directory_list_async(
  int8_t *directory_path,
  void (*call_back_handler_ptr)(uint16_t status, uint8_t *directory_list, uint16_t length, uint8_t end_of_list))
{

  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_ftp_file_ops_t *ftp_ops = NULL;

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
    if (call_back_handler_ptr != NULL) {
      // Register FTP directory list response notify call back handler
      rsi_wlan_cb_non_rom->nwk_callbacks.ftp_directory_list_call_back_handler = call_back_handler_ptr;
    } else {
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return invalid command error
      return RSI_ERROR_INVALID_PARAM;
    }
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);
    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    ftp_ops = (rsi_ftp_file_ops_t *)pkt->data;

    // get command type as directory list
    ftp_ops->command_type = RSI_FTP_DIRECTORY_LIST;

    // copy the directory path to list
    rsi_strcpy(ftp_ops->file_name, directory_path);

    // send set FTP  command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

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

/** @addtogroup NETWORK8 
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_ftp_mode_set(uint8_t mode)
 * @brief      Set the FTP client mode - either in Passive mode or Active Mode. 
 * @pre   \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  mode - Used to select the mode of FTP client if FTP is enabled \n
 *                    0-Active Mode \n
 *				  	  1-Passive Mode.  
 * @return     0              -  Success  \n
 *             Negative Value - Failure \n
 *                         -2 - Invalid parameter, expects call back handler \n
 *                         -3 - Command given in wrong state \n 
 *                         -4 - Buffer not available to serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16. 
 */
int32_t rsi_ftp_mode_set(uint8_t mode)
{
  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_ftp_file_ops_t *ftp_ops = NULL;

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
      //Changing the nwk state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    ftp_ops = (rsi_ftp_file_ops_t *)pkt->data;

    // Check for PASSIVE mode
    if (mode) {
      // get command type as FTP PASSIVE
      ftp_ops->command_type = RSI_FTP_PASSIVE;
    } else {
      // get command type as FTP ACTIVE
      ftp_ops->command_type = RSI_FTP_ACTIVE;
    }

#ifndef RSI_NWK_SEM_BITMAP
    rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
    // send set FTP Create command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FTP, pkt);

    // wait on nwk semaphore
    rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_FTP_RESPONSE_WAIT_TIME);

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
