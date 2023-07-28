/*******************************************************************************
* @file  rsi_socket_rom.c
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
#ifndef ROM_WIRELESS

/** @addtogroup NETWORK16
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t  ROM_WL_rsi_get_application_socket_descriptor(global_cb_t *global_cb_p, int32_t sock_id)
 * @brief       Get the application socket descriptor from module socket descriptor
 * @param[in]   global_cb_p - pointer to the global control block
 * @param[in]   sock_id    	- module's socket descriptor
 * @return	    Positive value - application index \n
 *              Negative value - If index is not found       
 */
int32_t ROM_WL_rsi_get_application_socket_descriptor(global_cb_t *global_cb_p, int32_t sock_id)
{
  int i;

  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

  for (i = 0; i < NUMBER_OF_SOCKETS; i++) {
    if (rsi_socket_pool[i].sock_id == sock_id) {
      break;
    }
  }

  if (i >= NUMBER_OF_SOCKETS) {
    return -1;
  }

  return i;
}

/*==============================================*/
/**
 * @fn          int32_t ROM_WL_rsi_get_primary_socket_id(global_cb_t *global_cb_p, uint16_t port_number)
 * @brief       Clear socket information
 * @param[in]   global_cb_p - pointer to the global control block
 * @param[in]   port_number - port number
 * @return 		Positive value - socket descriptor \n
 *              Negative value - If socket is not found 
 *             
 */
int32_t ROM_WL_rsi_get_primary_socket_id(global_cb_t *global_cb_p, uint16_t port_number)
{
  int i;

  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

  for (i = 0; i < NUMBER_OF_SOCKETS; i++) {
    if ((rsi_socket_pool[i].source_port == port_number)
        && (rsi_socket_pool[i].ltcp_socket_type == RSI_LTCP_PRIMARY_SOCKET)) {
      break;
    }
  }

  if (i >= NUMBER_OF_SOCKETS) {
    return -1;
  }
  return i;
}

/*==============================================*/
/**
 * @fn          uint8_t ROM_WL_calculate_buffers_required(global_cb_t *global_cb_p, uint8_t type, uint16_t length)
 * @brief       Calculate number of buffers required for the data packet
 * @param[in]   global_cb_p - pointer to the global control block
 * @param[in]   type 		- type of socket to create
 * @param[in]   length 		- length of the message
 * @return 		The number of buffers required for a data packet
 */
uint8_t ROM_WL_calculate_buffers_required(global_cb_t *global_cb_p, uint8_t type, uint16_t length)
{
  //This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(global_cb_p);

  uint8_t header_size, buffers_required = 1;
  uint16_t first_buffer_available_size, remaining_length;

  // Calculate header size including extra 2bytes based on the proto type
  if ((type & 0xF) == SOCK_STREAM) {
    header_size = 56;
  } else {
    header_size = 44;
  }

  // Increase header size by 20 for IPv6 case
  if ((type >> 4) == AF_INET6) {
    header_size += 20;
  }

  remaining_length = length;

  first_buffer_available_size = (512 - header_size - 252);

  if (length <= first_buffer_available_size) {
    return 1;
  }

  remaining_length -= first_buffer_available_size;

  do {
    buffers_required++;
    if (remaining_length > 512) {
      remaining_length -= 512;
    } else {
      remaining_length = 0;
    }

  } while (remaining_length);

  return buffers_required;
}
/*==============================================*/
/**
 * @fn         uint16_t ROM_WL_calculate_length_to_send(global_cb_t *global_cb_p, uint8_t type, uint8_t buffers)
 * @brief      Caluculate the msg length sent using available buffers    
 * @param[in]  global_cb_p  - pointer to the global control block
 * @param[in]  type 		- type of the socket stream
 * @param[in]  buffers 		- available buffers
 * @return     void 
 */

uint16_t ROM_WL_calculate_length_to_send(global_cb_t *global_cb_p, uint8_t type, uint8_t buffers)
{
  //This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(global_cb_p);
  uint8_t header_size;
  uint16_t length;

  // Calculate header size including extra 2bytes based on the proto type
  if ((type & 0xF) == SOCK_STREAM) {
    header_size = 56;
  } else {
    header_size = 44;
  }

  // Increase header size by 20 for IPv6 case
  if ((type >> 4) == AF_INET6) {
    header_size += 20;
  }

  length = (512 - header_size - 252);

  if (buffers == 1) {
    return length;
  }

  buffers--;

  while (buffers) {
    length += 512;
    buffers--;
  }

  return length;
}
#endif
/** @} */
