/*******************************************************************************
* @file  rsi_socket.c
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
#include "rsi_wlan_non_rom.h"
#ifdef RSI_WLAN_ENABLE

// Socket information pool pointer
rsi_socket_info_t *rsi_socket_pool;
rsi_socket_info_non_rom_t *rsi_socket_pool_non_rom;
extern rsi_socket_select_info_t *rsi_socket_select_info;

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn          int rsi_fd_isset(uint32_t fd, struct rsi_fd_set_s *fds_p)
 * @brief       Check for the bit set in the bitmap.
 * @param[in]   fd     - socket id
 * @param[in]   fds_p  - pointer to the rsi_fd_set_structure 
 * @return     Positivwe Value - Success \n
 *              0              - Failure 
 *              
 */

int rsi_fd_isset(uint32_t fd, struct rsi_fd_set_s *fds_p)
{
  uint32_t mask = 1 << (fd % 32);
  return fds_p->fd_array[fd / 32] & mask;
}
/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn          void rsi_set_fd(uint32_t fd, struct rsi_fd_set_s *fds_p)
 * @brief       Set the bit for the file descriptor fd in the file descriptor set \ref rsi_fd_set.
 * @param[in]   fd    - socket id
 * @param[in]   fds_p - pointer to the rsi_fd_set_structure 
 * @return 		void 
 */

void rsi_set_fd(uint32_t fd, struct rsi_fd_set_s *fds_p)
{
  uint32_t mask = 1 << (fd % 32);
  fds_p->fd_array[fd / 32] |= mask;
}
/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn          void rsi_fd_clr(uint32_t fd, struct rsi_fd_set_s *fds_p) 
 * @brief       Clear the bit in the bitmap 
 * @param[in]   fd    - socket id
 * @param[in]   fds_p - pointer to the rsi_fd_set_structure 
 * @return 		Void
 */

void rsi_fd_clr(uint32_t fd, struct rsi_fd_set_s *fds_p)
{
  uint32_t mask = 1 << (fd % 32);
  fds_p->fd_array[fd / 32] &= ~mask;
}
/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_socket_async(int32_t protocolFamily, int32_t type, int32_t protocol, void (*callback)(uint32_t sock_no, uint8_t *buffer, uint32_t length))
 * @brief      Create a socket and register a callback which will be used by the driver to forward
 * 			   the received packets asynchronously to the application (on packet reception) without waiting for recv API call. 
 * @pre    \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  protocolFamily - Protocol family to select IPv4 or IPv6 AF_INET \n
 *							    (2) : to select IPv4 AF_INET6 \n
 *   							(3) : to select IPv6 
 * @param[in]  type           - Select socket type UDP or TCP SOCK_STREAM \n
 *                              (1) : to select TCP SOCK_DGRM \n
 *					            (2) : to select UDP 
 * @param[in]  protocol       - 0: non SSL sockets \n
 *                              1: SSL sockets \n
 * 			                    BIT(5) should be enabled for selecting the certificate index \n
 *                              0<<12 for index 0 \n
 *                              1<<12 for index 1 \n
 *                              Note: For selecting the certificate index feature, certificates should be loaded in to ram. 
 * @param[in]  callback       - This is the callback function to read data asynchronously from socket \n 
 *                              sock_no:  Application socket number \n
 *                              buffer:  Pointer to buffer to hold the data \n 
 *                              length: length of the buffer. 
 * @return     0              - Success \n
 *             Negative Value - Failure 
 *              
 *
 *
 */

int32_t rsi_socket_async(int32_t protocolFamily,
                         int32_t type,
                         int32_t protocol,
                         void (*callback)(uint32_t sock_no, uint8_t *buffer, uint32_t length))
{
  return rsi_socket_async_non_rom(protocolFamily, type, protocol, callback);
}
/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_socket(int32_t protocolFamily, int32_t type, int32_t protocol)
 * @brief       Create the socket.
 * @pre   \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]   protocolFamily - Protocol family to select IPv4 or IPv6 \n
 *                               AF_INET (2): to select IPv4 \n
 *                               AF_INET6 (3): to select IPv6 
 * @param[in]   type           - Select socket type UDP or TCP \n
 *                               SOCK_STREAM (1): to select \n
 *                               TCP SOCK_DGRM (2): to select UDP 
 * @param[in]  protocol        - 0: non SSL sockets \n 
 *                               1: SSL sockets \n
 * 			                     BIT(5) should be enabled for selecting the certificate index \n
 *                               0<<12 for index 0 \n
 *                               1<<12 for index 1 \n
 *                               Note: For selecting the certificate index feature, certificates should be loaded in to ram. 
 * @return      0              - Success \n
 *              Negative Value - Failure
 *
 *
 */

int32_t rsi_socket(int32_t protocolFamily, int32_t type, int32_t protocol)
{
#ifndef BSD_COMPATIBILITY
  if (protocol & BIT(0)) {
    if (protocol & BIT(13)) {
      rsi_wlan_cb_non_rom->tls_version = (RSI_SOCKET_FEAT_SSL | RSI_SSL_V_1_0);
    } else if (protocol & BIT(14)) {
      rsi_wlan_cb_non_rom->tls_version = (RSI_SOCKET_FEAT_SSL | RSI_SSL_V_1_1);
    } else if (protocol & BIT(15)) {
      rsi_wlan_cb_non_rom->tls_version = (RSI_SOCKET_FEAT_SSL | RSI_SSL_V_1_2);
    } else {
      rsi_wlan_cb_non_rom->tls_version = RSI_SOCKET_FEAT_SSL;
    }
  }
#endif
  return rsi_socket_async_non_rom(protocolFamily, type, protocol, NULL);
}

/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t  rsi_bind(int32_t sockID, struct rsi_sockaddr *localAddress, int32_t addressLength)
 * @brief       Assign an address to a socket. 
 * @pre   \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]   sockID        - This the socket descriptor ID 
 * @param[in]   localAddress  - This is the address assigned to the socket. The format is compatible with BSD socket 
 * @param[in]   addressLength - This is the length of the address measured in bytes 
 * @return		0              - Success \n
 *             Negative Value - Failure
 *               
 *
 *
 *
 */
int32_t rsi_bind(int32_t sockID, struct rsi_sockaddr *localAddress, int32_t addressLength)
{
  return rsi_socket_bind(sockID, localAddress, addressLength);
}
/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t  rsi_connect(int32_t sockID, struct rsi_sockaddr *remoteAddress, int32_t addressLength)
 * @brief       Connect the socket to the specified remote address. 
 * @pre   \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]   sockID        - This the socket descriptor ID 
 * @param[in]   remoteAddress - This is the remote peer address. The format is compatible with BSD socket  
 * @param[in]   addressLength - This is the length of the address in bytes  
 * @return 		0              - Success \n
 *              Negative Value - Failure
 *
 *
 */

int32_t rsi_connect(int32_t sockID, struct rsi_sockaddr *remoteAddress, int32_t addressLength)
{
  return rsi_socket_connect(sockID, remoteAddress, addressLength);
}
/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t  rsi_listen(int32_t sockID, int32_t backlog)
 * @brief      Make the socket to listen for a remote connection request in passive mode
 * @pre   \ref rsi_socket() / rsi_socket_async() API needs to be called before this API. 
 * @param[in]  sockID  - This is the socket descriptor ID 
 * @param[in]  backlog - The maximum length to which the queue of pending connections can be held  
 * @return     0              - Success \n
 *             Negative Value - Failure
 *
 *
 */

int32_t rsi_listen(int32_t sockID, int32_t backlog)
{
  return rsi_socket_listen(sockID, backlog);
}
/*==============================================*/
/**
 * @fn         int32_t  rsi_accept_non_rom (int32_t sockID, struct rsi_sockaddr *ClientAddress, int32_t *addressLength)
 * @brief      Accept the connection request from the remote peer.This API extracts the connection request from the queue of pending connections on listening socket and accepts it. 
 * @pre   \ref rsi_socket()/ \ref rsi_socket_async() API needs to be called before this API.
 * @param[in]  sockID        - This is the socket descriptor ID 
 * @param[in]  ClientAddress - This is the remote peer address. \n
 *   					       This parameter is an out parameter filled by driver on Success ful connection acceptance. \n
 *							   The format is compatible with BSD socket.  
 * @param[in]  addressLength - This is the length of the address measured in bytes 
 * @return 	   0              - Success \n
 *             Negative Value - Failure 
 *
 *
 */
/** @} */

int32_t rsi_accept_non_rom(int32_t sockID, struct rsi_sockaddr *ClientAddress, int32_t *addressLength)
{
  struct rsi_sockaddr_in peer4_address;
  struct rsi_sockaddr_in6 peer6_address;
  int32_t status = RSI_SUCCESS;
  rsi_req_socket_accept_t *accept;
  rsi_pkt_t *pkt = NULL;
  rsi_socket_info_t *sock_info;
  int8_t accept_sock_id = -1;

  rsi_driver_cb_t *rsi_driver_cb     = global_cb_p->rsi_driver_cb;
  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

#ifdef SOCKET_CLOSE_WAIT
  if (rsi_socket_pool[sockID].sock_state > RSI_SOCKET_STATE_INIT && rsi_socket_pool_non_rom[sockID].close_pending) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ECONNABORTED, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ECONNABORTED);
#endif
    return RSI_SOCK_ERROR;
  }
#endif

  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // If sockID is not in available range
  if (sockID < 0 || sockID >= NUMBER_OF_SOCKETS) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // Check socket is TCP or not
  if ((rsi_socket_pool[sockID].sock_type & 0xF) != SOCK_STREAM) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EPROTOTYPE, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EPROTOTYPE);
#endif
    return RSI_SOCK_ERROR;
  }

  // If socket is not binded
  if (rsi_socket_pool[sockID].sock_state != RSI_SOCKET_STATE_LISTEN) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  if (rsi_socket_pool[sockID].ltcp_socket_type != RSI_LTCP_PRIMARY_SOCKET) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EINVAL, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EINVAL);
#endif
    return RSI_SOCK_ERROR;
  }

  sock_info = &rsi_socket_pool[sockID];

  // check maximum backlogs count
  if (sock_info->backlogs == sock_info->backlog_current_count) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ENOBUFS, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ENOBUFS);
#endif
    return RSI_SOCK_ERROR;
  }

  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_LISTEN) {
    // Create new instance for socket
    accept_sock_id = rsi_socket_async((sock_info->sock_type >> 4),
                                      (sock_info->sock_type & 0xF),
                                      (sock_info->sock_bitmap),
                                      sock_info->sock_receive_callback);

    if ((accept_sock_id >= 0) && (accept_sock_id < NUMBER_OF_SOCKETS)) {

      // Set socket as secondary socket
      rsi_socket_pool[accept_sock_id].ltcp_socket_type = RSI_LTCP_SECONDARY_SOCKET;

      // Save local port number
      rsi_socket_pool[accept_sock_id].source_port = rsi_socket_pool[sockID].source_port;
    } else {
      // Set error
      rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
      rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
      return RSI_SOCK_ERROR;
    }
  }

  // Allocate packet
  pkt = rsi_pkt_alloc(&rsi_driver_cb->wlan_cb->wlan_tx_pool);
  if (pkt == NULL) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_PKT_ALLOCATION_FAILURE, sockID);
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    return RSI_SOCK_ERROR;
  }

  // send socket accept command
  accept = (rsi_req_socket_accept_t *)pkt->data;

  // Fill socket descriptor
  accept->socket_id = rsi_socket_pool[sockID].sock_id;

  // Fill local port number
  rsi_uint16_to_2bytes(accept->source_port, rsi_socket_pool[sockID].source_port);

#ifndef RSI_SOCK_SEM_BITMAP
  rsi_socket_pool_non_rom[accept_sock_id].socket_wait_bitmap |= BIT(0);
#endif

  // Send socket accept command
  status = RSI_DRIVER_WLAN_SEND_CMD(RSI_WLAN_REQ_SOCKET_ACCEPT, pkt);

  // wait on socket semaphore
  status =
    rsi_wait_on_socket_semaphore(&rsi_socket_pool_non_rom[accept_sock_id].socket_sem, RSI_ACCEPT_RESPONSE_WAIT_TIME);
  if (status != RSI_ERROR_NONE) {
    // get wlan/network command response status
    rsi_wlan_socket_set_status(status, sockID);
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    return RSI_SOCK_ERROR;
  }
  // get wlan/network command response status
  status = rsi_wlan_socket_get_status(accept_sock_id);
  if (status != RSI_SUCCESS) {
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    return RSI_SOCK_ERROR;
  }
  // check status,fill out params with remote peer address details if success and return status
  if (status == RSI_SUCCESS) {
    // Copy remote peer ip address
    if (ClientAddress && *addressLength != 0) {
      // Handle the IPv4 socket type
      if ((rsi_socket_pool[accept_sock_id].sock_type >> 4) == AF_INET) {

        // Update the Client address with socket family, remote host IPv4 address and port
        peer4_address.sin_family = AF_INET;
        memcpy(&peer4_address.sin_addr.s_addr,
               (ntohl(rsi_socket_pool[accept_sock_id].destination_ip_addr.ipv4)),
               RSI_IPV4_ADDRESS_LENGTH);
        peer4_address.sin_port = ntohs((uint16_t)rsi_socket_pool[accept_sock_id].destination_port);

        // Copy the peer address/port info to the ClientAddress.
        // Truncate if addressLength is smaller than the size of struct rsi_sockaddr_in
        if (*addressLength > (int32_t)sizeof(struct rsi_sockaddr_in))
          *addressLength = sizeof(struct rsi_sockaddr_in);
      }
      memcpy(ClientAddress, &peer4_address, *addressLength);
    } else if ((rsi_socket_pool[accept_sock_id].sock_type >> 4) == AF_INET6) {
      peer6_address.sin6_family = AF_INET6;
      peer6_address.sin6_port   = ntohs((uint16_t)rsi_socket_pool[accept_sock_id].destination_port);
      //! to avoid compiler warning replace uint32_t with uintptr_t
      /*peer6_address.sin6_addr._S6_un._S6_u32[0] =
          (uint32_t)ntohl(&rsi_socket_pool[accept_sock_id].destination_ip_addr.ipv6[0]);
        peer6_address.sin6_addr._S6_un._S6_u32[1] =
          (uint32_t)ntohl(&rsi_socket_pool[accept_sock_id].destination_ip_addr.ipv6[4]);
        peer6_address.sin6_addr._S6_un._S6_u32[2] =
          (uint32_t)ntohl(&rsi_socket_pool[accept_sock_id].destination_ip_addr.ipv6[8]);
        peer6_address.sin6_addr._S6_un._S6_u32[3] =
          (uint32_t)ntohl(&rsi_socket_pool[accept_sock_id].destination_ip_addr.ipv6[12]);*/
      //! to avoid compiler warning replace uint32_t with uintptr_t
      peer6_address.sin6_addr._S6_un._S6_u32[0] =
        (uintptr_t)ntohl(&rsi_socket_pool[accept_sock_id].destination_ip_addr.ipv6[0]);
      peer6_address.sin6_addr._S6_un._S6_u32[1] =
        (uintptr_t)ntohl(&rsi_socket_pool[accept_sock_id].destination_ip_addr.ipv6[4]);
      peer6_address.sin6_addr._S6_un._S6_u32[2] =
        (uintptr_t)ntohl(&rsi_socket_pool[accept_sock_id].destination_ip_addr.ipv6[8]);
      peer6_address.sin6_addr._S6_un._S6_u32[3] =
        (uintptr_t)ntohl(&rsi_socket_pool[accept_sock_id].destination_ip_addr.ipv6[12]);

      if ((*addressLength) > (int32_t)sizeof(struct rsi_sockaddr_in6)) {
        *addressLength = sizeof(struct rsi_sockaddr_in6);
      }

      memcpy(ClientAddress, &peer6_address, *addressLength);
    }
  }

  // Increase backlog current count
  rsi_socket_pool[sockID].backlog_current_count++;

  // Return status
  return accept_sock_id;
}

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_accept(int32_t sockID, struct rsi_sockaddr *ClientAddress, int32_t *addressLength)
 * @brief      TCP server mode to accept an incoming connection on given socket.
 * @pre   \ref rsi_socket()/ \ref rsi_socket_async() API needs to be called before this API. 
 * @param[in]  sockID        - This is the socket descriptor  
 * @param[in]  ClientAddress - This is the remote peer address.This parameter is an out parameter filled by driver on Success ful connection acceptance.  
 * @param[in]  addressLength - This is the length of the address measured in bytes 
 * @return 	   0              - Success \n
 *             Negative Value - Failure
 *
 *
 */
int32_t rsi_accept(int32_t sockID, struct rsi_sockaddr *ClientAddress, int32_t *addressLength)
{
  return rsi_accept_non_rom(sockID, ClientAddress, addressLength);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_recv_large_data_sync(int32_t sockID,
                                 int8_t *buffer,
                                 int32_t requested_length,
                                 int32_t flags,
                                 struct rsi_sockaddr *fromAddr,
                                 int32_t *fromAddrLen)
 * @brief      Receive large data on a given socket synchronously
 * @param[in]  sockID    - socket descriptor
 * @param[in]  msg       - pointer to data which needs to be send to remote peer
 * @param[in]  msgLength - length of data to send
 * @param[in]  flags     - reserved
 * @param[out] status
 * @return     0              - Success \n
 *             Negative Value - Failure
 */
#ifdef RSI_PROCESS_MAX_RX_DATA
int32_t rsi_recv_large_data_sync(int32_t sockID,
                                 int8_t *buffer,
                                 int32_t requested_length,
                                 int32_t flags,
                                 struct rsi_sockaddr *fromAddr,
                                 int32_t *fromAddrLen)
{
  int32_t status         = 0;
  int32_t chunk_size     = 0;
  int32_t rem_len        = 0;
  int32_t offset         = 0;
  int32_t maximum_length = 0;
  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  if (rsi_socket_pool[sockID].sock_state != RSI_SOCKET_STATE_CONNECTED) {
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  if (requested_length > RX_DATA_LENGTH) {
    rem_len = RX_DATA_LENGTH;
  } else {
    rem_len = requested_length;
  }
  if (requested_length > MAX_RX_DATA_LENGTH) {
    maximum_length = MAX_RX_DATA_LENGTH;
  } else {
    maximum_length = requested_length;
  }
  do {
    rsi_socket_pool_non_rom[sockID].more_rx_data_pending = 0;
    chunk_size                                           = MIN(maximum_length, rem_len);
    status = rsi_socket_recvfrom(sockID, buffer + offset, chunk_size, flags, fromAddr, fromAddrLen);
    if (status < 0) {
      status = rsi_wlan_socket_get_status(sockID);
#ifdef RSI_WITH_OS
      status = rsi_get_error(sockID);
      rsi_set_os_errno(status);
#endif
      break;
    } else {
      rem_len -= status;
      offset += status;
    }
  } while (rem_len && rsi_socket_pool_non_rom[sockID].more_rx_data_pending);
  return offset;
}
#endif
/** @} */
/** @addtogroup NETWORK5
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t  rsi_recvfrom(int32_t sockID, int8_t *buffer, int32_t buffersize, int32_t flags,struct rsi_sockaddr *fromAddr, int32_t *fromAddrLen)
 * @brief      Retrieve the received data from the remote peer on a given socket descriptor.
 * @pre   \ref rsi_socket()/ \ref rsi_socket_async() API needs to be called before this API. 
 * @param[in]  sockID      - This is the socket descriptor  
 * @param[in]  buffer      - This is the pointer to buffer to hold receive data. This is an out parameter. 
 * @param[in]  buffersize  - This is the size of the buffer supplied 
 * @param[in]  flags       - reserved
 * @param[in]  fromAddr    - This is the address of remote peer, from where current packet was received. It is an out parameter. 
 * @param[in]  fromAddrLen - This is the pointer which contains remote peer address(fromAddr) length 
 * @return 	   0              - Success \n
 *             Negative Value - Failure
 *
 *
 */

int32_t rsi_recvfrom(int32_t sockID,
                     int8_t *buffer,
                     int32_t buffersize,
                     int32_t flags,
                     struct rsi_sockaddr *fromAddr,
                     int32_t *fromAddrLen)
{
  if ((buffersize == 0) || (buffer == NULL)) {
    rsi_wlan_socket_set_status(RSI_ERROR_EINVAL, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EINVAL);
#endif
    return RSI_SOCK_ERROR;
  }
#ifdef RSI_PROCESS_MAX_RX_DATA
  if (rsi_socket_pool[sockID].sock_type & SOCK_STREAM) {
    return rsi_recv_large_data_sync(sockID, buffer, buffersize, flags, fromAddr, fromAddrLen);
  } else {
    return rsi_socket_recvfrom(sockID, buffer, buffersize, flags, fromAddr, fromAddrLen);
  }
#else
  return rsi_socket_recvfrom(sockID, buffer, buffersize, flags, fromAddr, fromAddrLen);
#endif
}
/** @} */

/** @addtogroup NETWORK5 
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t  rsi_recv(int32_t sockID, void *rcvBuffer, int32_t bufferLength, int32_t flags)
 * @brief      Retrieve the data from the remote peer on a specified socket.
 * @pre   \ref rsi_socket()/ \ref rsi_socket_async() API needs to be called before this API.  
 * @param[in]  sockID     - This is the socket descriptor ID 
 * @param[in]  buffer     - This is the pointer to the buffer to hold the data received from the remote peer 
 * @param[in]  buffersize - This is the length of the buffer 
 * @param[in]  flags      - reserved
 * @return 	   0              - Success \n
 *             Negative Value - Failure
 *
 *
 */
int32_t rsi_recv(int32_t sockID, void *rcvBuffer, int32_t bufferLength, int32_t flags)
{
  int32_t fromAddrLen = 0;
  if ((bufferLength == 0) || (rcvBuffer == NULL)) {
    rsi_wlan_socket_set_status(RSI_ERROR_EINVAL, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EINVAL);
#endif
    return RSI_SOCK_ERROR;
  }
#ifdef RSI_PROCESS_MAX_RX_DATA
  if (rsi_socket_pool[sockID].sock_type & SOCK_STREAM) {
    return rsi_recv_large_data_sync(sockID, rcvBuffer, bufferLength, flags, NULL, &fromAddrLen);
  } else {
    return rsi_socket_recvfrom(sockID, rcvBuffer, bufferLength, flags, NULL, &fromAddrLen);
  }
#else
  return rsi_socket_recvfrom(sockID, rcvBuffer, bufferLength, flags, NULL, &fromAddrLen);
#endif
}

/** @} */

/** @addtogroup NETWORK5 
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_sendto(int32_t sockID, int8_t *msg, int32_t msgLength, int32_t flags, struct rsi_sockaddr *destAddr, int32_t destAddrLen)
 * @brief       Send data to specific remote peer on a given socket synchronously.
 * @pre   \ref rsi_socket()/ \ref rsi_socket_async() API needs to be called before this API. 
 * @param[in]   sockID      - This is the Socket Descriptor ID  
 * @param[in]   msg         - This is the pointer to data buffer containing data to send to remote peer 
 * @param[in]   msgLength   - This is the length of the buffer  
 * @param[in]   flags       - reserved 
 * @param[in]   destAddr    - This is the address of the remote peer to send data 
 * @param[in]   destAddrLen - This is the length of the address in bytes 
 * @return		0              - Success \n
 *              Negative Value - Failure 
 *
 */

int32_t rsi_sendto(int32_t sockID,
                   int8_t *msg,
                   int32_t msgLength,
                   int32_t flags,
                   struct rsi_sockaddr *destAddr,
                   int32_t destAddrLen)
{
  return rsi_sendto_async_non_rom(sockID, msg, msgLength, flags, destAddr, destAddrLen, NULL);
}

/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_sendto_async(int32_t sockID, 
 *                                       int8_t *msg, 
 *										 int32_t msgLength, 
 *										 int32_t flags, 
 *										 struct rsi_sockaddr *destAddr, 
 *										 int32_t destAddrLen,
 *                                       void (*data_transfer_complete_handler)(int32_t sockID, uint16_t length))
 * @brief       Send data to specific remote peer on a given socket asynchronously.
 * @param[in]   sockID						   - This is the Socket Descriptor ID  
 * @param[in]   msg 						   - This is the pointer to data buffer containing data to send to remote peer  
 * @param[in]   msgLength 					   - length of data to send 
 * @param[in]   flags 						   - reserved 
 * @param[in]   destAddr 					   - This is the address of the remote peer to send data  
 * @param[in]   destAddrLen 				   - This is the length of the address in bytes 
 * @param[in]   data_transfer_complete_handler - This is pointer to the callback function which will be called after data transfer completion 
 * @return		0              - Success \n
 *              Negative Value - Failure
 *
 *
 */
int32_t rsi_sendto_async(int32_t sockID,
                         int8_t *msg,
                         int32_t msgLength,
                         int32_t flags,
                         struct rsi_sockaddr *destAddr,
                         int32_t destAddrLen,
                         void (*data_transfer_complete_handler)(int32_t sockID, uint16_t length))
{
  return rsi_sendto_async_non_rom(sockID, msg, msgLength, flags, destAddr, destAddrLen, data_transfer_complete_handler);
}
/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_send(int32_t sockID, const int8_t *msg, int32_t msgLength, int32_t flags)
 * @brief      Send data on a given socket syncronously.
 * @pre    \ref rsi_socket()/ \ref rsi_socket_async() API needs to be called before this API. 
 * @param[in]  sockID    - This is the socket descriptor ID  
 * @param[in]  msg       - This is the pointer to the buffer containing data to send to the remote peer 
 * @param[in]  msgLength - lThis is the length of the buffer  
 * @param[in]  flags     - reserved 
 * @return	   0              - Success \n
 *             Negative Value - Failure
 *
 *
 */

int32_t rsi_send(int32_t sockID, const int8_t *msg, int32_t msgLength, int32_t flags)
{
#ifdef RSI_UART_INTERFACE
  rsi_wlan_cb_non_rom->rsi_uart_data_ack_check = sockID;
#endif
#ifdef LARGE_DATA_SYNC
  return rsi_send_large_data_sync(sockID, msg, msgLength, flags);
#else
  return rsi_send_async_non_rom(sockID, msg, msgLength, flags, NULL);
#endif
}

#ifndef RSI_M4_INTERFACE

/*==============================================*/
/**
 * @fn         void rsi_reset_per_socket_info(int32_t sockID)
 * @brief      Reset all the socket related information 
 * @param[in]  sockID - socket descriptor 
 * @return 		Void
 *
 */

void rsi_reset_per_socket_info(int32_t sockID)
{
  rsi_socket_info_non_rom_t *rsi_sock_info_non_rom_p = &rsi_socket_pool_non_rom[sockID];
  rsi_sock_info_non_rom_p->more_data                 = 0;
  rsi_sock_info_non_rom_p->offset                    = 0;
  rsi_sock_info_non_rom_p->buffer                    = 0;
  rsi_sock_info_non_rom_p->rem_len                   = 0;
  // reset socket_bitmap
  rsi_wlan_cb_non_rom->socket_bitmap &= ~BIT(sockID);
  rsi_sock_info_non_rom_p->rsi_sock_data_tx_done_cb = NULL;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_send_large_data_sync(int32_t sockID, const int8_t *msg, int32_t msgLength, int32_t flags)
 * @brief      Send large data on a given socket synchronously.
 * @param[in]  sockID    - socket descriptor 
 * @param[in]  msg       - pointer to data which needs to be send to remote peer 
 * @param[in]  msgLength - length of data to send 
 * @param[in]  flags     - reserved 
 * @return	   0              - Success \n
 *             Negative Value - Failure
 *
 */

int32_t rsi_send_large_data_sync(int32_t sockID, const int8_t *msg, int32_t msgLength, int32_t flags)
{
  int32_t status         = 0;
  int32_t chunk_size     = 0;
  int32_t rem_len        = msgLength;
  int32_t offset         = 0;
  int8_t *buffer         = (int8_t *)msg;
  int32_t maximum_length = 0;

  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  // Find maximum limit based on the protocol
  if ((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_DGRAM) {
    // If it is a UDP socket
    maximum_length = 1472;
  } else if (((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_STREAM)
             && ((rsi_socket_pool[sockID].sock_bitmap & RSI_SOCKET_FEAT_SSL)
                 || (rsi_socket_pool_non_rom[sockID].ssl_bitmap))) {
    // If it is a SSL socket/SSL websocket
    maximum_length = rsi_socket_pool_non_rom[sockID].mss - RSI_SSL_HEADER_SIZE;
  } else if (((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_STREAM)
             && (rsi_socket_pool[sockID].sock_bitmap & RSI_SOCKET_FEAT_WEBS_SUPPORT)) {
    // If it is a websocket
    maximum_length = 1450;
  } else {
    maximum_length = rsi_socket_pool_non_rom[sockID].mss;
  }

  while (rem_len) {
    chunk_size = MIN(maximum_length, rem_len);
    // Sends next chunk of data and returns the total data sent in success case
    status = rsi_send_async(sockID, buffer + offset, chunk_size, flags, NULL);
    if (status < 0) {
      status = rsi_wlan_socket_get_status(sockID);
#ifdef RSI_WITH_OS
      status = rsi_get_error(sockID);
      rsi_set_os_errno(status);
#endif
      break;
    } else {
      rem_len -= status;
      offset += status;
    }
  }
  return offset;
}

/*==============================================*/
/**
 * @fn         void rsi_chunk_data_tx_done_cb(int32_t sockID, const uint16_t length)
 * @brief      Callback function which is used to send one chunk of data at a time
 * @param[in]  sockID - socket descriptor 
 * @param[in]  length - This is the length 
 * @return     void
 *
 */

void rsi_chunk_data_tx_done_cb(int32_t sockID, const uint16_t length)
{
  UNUSED_CONST_PARAMETER(length); //This statement is added only to resolve compilation warning, value is unchanged
  int16_t status = 0;
  uint16_t total_data_sent, chunk_size = 0;
  rsi_socket_info_non_rom_t *rsi_sock_info_non_rom_p = &rsi_socket_pool_non_rom[sockID];
  int32_t maximum_length                             = 0;

  // Find maximum limit based on the protocol
  if ((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_DGRAM) {
    // If it is a UDP socket
    maximum_length = 1472;
  } else if (((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_STREAM)
             && ((rsi_socket_pool[sockID].sock_bitmap & RSI_SOCKET_FEAT_SSL)
                 || (rsi_socket_pool_non_rom[sockID].ssl_bitmap))) {
    // If it is a SSL socket/SSL websocket
    maximum_length = rsi_socket_pool_non_rom[sockID].mss - RSI_SSL_HEADER_SIZE;
  } else if (((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_STREAM)
             && (rsi_socket_pool[sockID].sock_bitmap & RSI_SOCKET_FEAT_WEBS_SUPPORT)) {
    // If it is a websocket
    maximum_length = 1450;
  } else {
    maximum_length = rsi_socket_pool_non_rom[sockID].mss;
  }

  if (rsi_sock_info_non_rom_p->rem_len) {
#ifndef WISEMCU
    // masking the socket event
    rsi_mask_event(RSI_SOCKET_EVENT);
#endif
    rsi_sock_info_non_rom_p->more_data = 1;
    chunk_size                         = MIN(maximum_length, rsi_sock_info_non_rom_p->rem_len);
    // Sends next chunk of data and returns the total data sent in success case
    status = rsi_send_async(sockID,
                            rsi_sock_info_non_rom_p->buffer + rsi_sock_info_non_rom_p->offset,
                            chunk_size,
                            rsi_sock_info_non_rom_p->flags,
                            rsi_chunk_data_tx_done_cb);
    if (status < 0) {
      status = rsi_wlan_socket_get_status(sockID);
#ifdef RSI_WITH_OS
      status = rsi_get_error(sockID);
      rsi_set_os_errno(status);
#endif
      total_data_sent = rsi_sock_info_non_rom_p->offset;
      if (status != RSI_TX_BUFFER_FULL) {
        if (rsi_sock_info_non_rom_p->rsi_sock_data_tx_done_cb != NULL) {
          rsi_sock_info_non_rom_p->rsi_sock_data_tx_done_cb(sockID, status, total_data_sent);
        }
        rsi_reset_per_socket_info(sockID);
      }
    } else {
      rsi_sock_info_non_rom_p->rem_len -= status;
      rsi_sock_info_non_rom_p->offset += status;
    }
#ifndef WISEMCU
    // unmasking the socket event
    rsi_unmask_event(RSI_SOCKET_EVENT);
#endif
  } else // Incase of total data is sent
  {
    total_data_sent = rsi_sock_info_non_rom_p->offset;
    if (rsi_sock_info_non_rom_p->rsi_sock_data_tx_done_cb != NULL) {
      rsi_sock_info_non_rom_p->rsi_sock_data_tx_done_cb(sockID, RSI_SUCCESS, total_data_sent);
    }
    rsi_reset_per_socket_info(sockID);
    if (rsi_wlan_cb_non_rom->socket_bitmap == 0) {
#ifndef WISEMCU
      // Clearing the socket event
      rsi_clear_event(RSI_SOCKET_EVENT);
#endif
    }
  }
}

/*==============================================*/
/**
 * @fn         int32_t rsi_send_large_data_async(int32_t sockID, const int8_t *msg, int32_t msgLength, int32_t flags, 
 *                                                void (*rsi_sock_data_tx_done_cb)(int32_t sockID, int16_t status,uint16_t total_data_sent))
 * @brief      Send large data on a given socket
 * @param[in]  sockID				    - socket descriptor 
 * @param[in]  msg 					    - pointer to data which needs to be send to remote peer 
 * @param[in]  msgLength			    - length of data to send 
 * @param[in]  flags				    - reserved 
 * @param[in]  rsi_sock_data_tx_done_cb - pointer to the callback function which will be called after one chunk of data transfer completion
 * @param[out] status
 * @return	   0              - Success \n
 *             Negative Value - Failure 
 *
 */

int32_t rsi_send_large_data_async(int32_t sockID,
                                  const int8_t *msg,
                                  int32_t msgLength,
                                  int32_t flags,
                                  void (*rsi_sock_data_tx_done_cb)(int32_t sockID,
                                                                   int16_t status,
                                                                   uint16_t total_data_sent))
{
  int32_t status                                     = 0;
  uint16_t chunk_size                                = 0;
  rsi_socket_info_non_rom_t *rsi_sock_info_non_rom_p = &rsi_socket_pool_non_rom[sockID];
  int32_t maximum_length                             = 0;

  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  // check if the socket is already doing data transfer, if so return busy
  if ((rsi_wlan_cb_non_rom->socket_bitmap & BIT(sockID))) {
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EEXIST);
#endif
    return RSI_SOCK_ERROR;
  }

  // Find maximum limit based on the protocol
  if ((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_DGRAM) {
    // If it is a UDP socket
    maximum_length = 1472;
  } else if (((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_STREAM)
             && ((rsi_socket_pool[sockID].sock_bitmap & RSI_SOCKET_FEAT_SSL)
                 || (rsi_socket_pool_non_rom[sockID].ssl_bitmap))) {
    // If it is a SSL socket/SSL websocket
    maximum_length = rsi_socket_pool_non_rom[sockID].mss - RSI_SSL_HEADER_SIZE;
  } else if (((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_STREAM)
             && (rsi_socket_pool[sockID].sock_bitmap & RSI_SOCKET_FEAT_WEBS_SUPPORT)) {
    // If it is a websocket
    maximum_length = 1450;
  } else {
    maximum_length = rsi_socket_pool_non_rom[sockID].mss;
  }

  chunk_size                                        = MIN(maximum_length, msgLength);
  rsi_sock_info_non_rom_p->rsi_sock_data_tx_done_cb = rsi_sock_data_tx_done_cb;
  rsi_sock_info_non_rom_p->buffer                   = (int8_t *)msg;
  rsi_sock_info_non_rom_p->flags                    = flags;
  rsi_wlan_cb_non_rom->socket_bitmap |= BIT(sockID);
  // returns total num of bytes sent in success case
  status = rsi_send_async(sockID, rsi_sock_info_non_rom_p->buffer, chunk_size, flags, rsi_chunk_data_tx_done_cb);
  // If chunk of data is not sent no need to update the values
  if (status < 0) {
    status = rsi_wlan_socket_get_status(sockID);
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    rsi_reset_per_socket_info(sockID);
    return status;
  } else // Incase of BUFFER FULL condition no need to update the values
  {
    rsi_sock_info_non_rom_p->rem_len = msgLength - status;
    rsi_sock_info_non_rom_p->offset  = status;
    // Setting the socket event
    rsi_set_event(RSI_SOCKET_EVENT);
  }

  return RSI_SUCCESS;
}

/*==============================================*/
/**
 * @fn         uint32_t rsi_find_socket_data_pending(uint32_t event_map)
 * @brief      Find on which socket the data is pending,
 *             according to the event which is set from the map
 * @param[in]   event_map - event map 
 * @return	   0              - Success \n
 *             Negative Value - Failure 
 *              
 */

uint32_t rsi_find_socket_data_pending(uint32_t event_map)
{
  uint8_t i;
  for (i = 0; i < RSI_NUMBER_OF_SOCKETS; i++) {
    if (event_map & BIT(i)) {
      break;
    }
  }
  return i;
}

/*==============================================*/
/**
 * @fn          void rsi_socket_event_handler(void)
 * @brief       Retrieve the packet from protocol TX data pending queue
 * 				and forwards to the module.
 * @param[in]   void  
 * @return		void 
 *
 *
 */

void rsi_socket_event_handler(void)
{
  int32_t sockID = 0;

  if (rsi_wlan_cb_non_rom->socket_bitmap) {
    sockID = rsi_find_socket_data_pending(rsi_wlan_cb_non_rom->socket_bitmap);

    rsi_chunk_data_tx_done_cb(sockID, 0);
  } else {
    rsi_clear_event(RSI_SOCKET_EVENT);
  }
}
/** @} */

#endif
/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_send_async(int32_t sockID, const int8_t *msg, int32_t msgLength, int32_t flags, 
                                      void (*data_transfer_complete_handler)(int32_t sockID, uint16_t length))
 * @brief      Send data on a given socket asynchronously. 
 * @pre  \ref rsi_socket()/ \ref rsi_socket_async() API needs to be called before this API. 
 * @param[in]  sockID 						  - This is the socket descriptor ID  
 * @param[in]  msg							  - This is the pointer to the buffer containing data to send to the remote peer  
 * @param[in]  msgLength 					  - This is the length of the buffer 
 * @param[in]  flags 						  - Reserved  
 * @param[in]  data_transfer_complete_handler - pointer to the callback function which will be called after data transfer completion 
 * 												  sockID - This is the socket descriptor ID.  \n length 
 * @return	   0              - Success \n
 *             Negative Value - Failure
 *
 */
int32_t rsi_send_async(int32_t sockID,
                       const int8_t *msg,
                       int32_t msgLength,
                       int32_t flags,
                       void (*data_transfer_complete_handler)(int32_t sockID, uint16_t length))
{
  return rsi_send_async_non_rom(sockID, msg, msgLength, flags, data_transfer_complete_handler);
}

/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t  rsi_shutdown(int32_t sockID, int32_t how)
 * @brief       Socket specified in a socket descriptor.
 * @pre  \ref rsi_socket()/ \ref rsi_socket_async() API needs to be called before this API. 
 * @param[in]   sockID - This is the socket descriptor ID  
 * @param[in]   how - 0: close the specified socket \n
 *                    1: close all the sockets open on specified socket's source port number. 
 * @return		0              - Success \n
 *              Negative Value - Failure
 *
 */
int32_t rsi_shutdown(int32_t sockID, int32_t how)
{
  return rsi_socket_shutdown(sockID, how);
}

/*==============================================*/
/**
 * @fn          int32_t rsi_check_state(int32_t type)
 * @brief       Check the wlan state
 * @param[in]   type - socket family ttype 
 * @return		0   - If in IP cofig state \n
 *              -20 - If not in IP config state 
 *
 */
int32_t rsi_check_state(int32_t type)
{
  rsi_driver_cb_t *rsi_driver_cb = global_cb_p->rsi_driver_cb;

  if (rsi_driver_cb->wlan_cb->opermode == RSI_WLAN_ACCESS_POINT_MODE) {
    if (RSI_CHECK_WLAN_STATE() != RSI_WLAN_STATE_CONNECTED) {
      return RSI_ERROR_NOT_IN_IPCONFIG_STATE;
    }
  } else {
    if (type == AF_INET) {
      if (RSI_CHECK_WLAN_STATE() != RSI_WLAN_STATE_IP_CONFIG_DONE) {
        return RSI_ERROR_NOT_IN_IPCONFIG_STATE;
      }
    } else {
      if (RSI_CHECK_WLAN_STATE() != RSI_WLAN_STATE_IPV6_CONFIG_DONE) {
        return RSI_ERROR_NOT_IN_IPCONFIG_STATE;
      }
    }
  }
  return RSI_SUCCESS;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_get_application_socket_descriptor(int32_t sock_id)
 * @brief       Get the application socket descriptor from module socket descriptor
 * @param[in]   sock_id - module's socket descriptor
 * @return		Positive Value - application index \n
 *              Negative Value - If index is not found 
 *
 */
int32_t rsi_get_application_socket_descriptor(int32_t sock_id)
{
  return rsi_application_socket_descriptor(sock_id);
}

/*==============================================*/
/**
 * @fn          void rsi_clear_sockets(int32_t sockID)
 * @brief       Clear socket information
 * @param[in]   sockID - socket descriptor
 * @return      void
 *
 */
void rsi_clear_sockets(int32_t sockID)
{
  rsi_clear_sockets_non_rom(sockID);
}

/*==============================================*/
/**
 * @fn          int rsi_fill_tls_extension(int32_t sockID, int extension_type, const void *option_value, rsi_socklen_t extension_len)
 * @brief       Configure TLS extensions
 * @param[in]   sockID         - socket descriptor 
 * @param[in]   extension_type -  Type of TLS extension
 * @param[in]   option_value   - TLS extension data
 * @param[in]   extension_len  - TLS extension data length
 * @return      0              - Success \n
 *              Negative Value - Failure    
 *
 *
 **/
int rsi_fill_tls_extension(int32_t sockID, int extension_type, const void *option_value, rsi_socklen_t extension_len)
{
  rsi_tls_tlv_t *tls_extension = NULL;

  if ((rsi_socket_pool_non_rom[sockID].extension_offset + extension_len + sizeof(rsi_tls_tlv_t))
      > MAX_SIZE_OF_EXTENSION_DATA) {
    return RSI_ERROR_INSUFFICIENT_BUFFER;
  }
  if (extension_type == SO_TLS_SNI) {
    extension_type = RSI_TLS_SNI;
  }

  tls_extension = (rsi_tls_tlv_t *)&rsi_socket_pool_non_rom[sockID]
                    .tls_extension_data[rsi_socket_pool_non_rom[sockID].extension_offset];
  tls_extension->type   = extension_type;
  tls_extension->length = extension_len;
  rsi_socket_pool_non_rom[sockID].extension_offset += sizeof(rsi_tls_tlv_t);
  memcpy(&rsi_socket_pool_non_rom[sockID].tls_extension_data[rsi_socket_pool_non_rom[sockID].extension_offset],
         option_value,
         extension_len);
  rsi_socket_pool_non_rom[sockID].extension_offset += extension_len;
  rsi_socket_pool_non_rom[sockID].no_of_tls_extensions++;
  return RSI_SUCCESS;
}
/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn           int rsi_setsockopt(int32_t sockID, int level, int option_name,const void *option_value, rsi_socklen_t option_len)
 * @brief        Set the socket options. 
 * @pre \ref rsi_setsockopt() API needs to be called after rsi_socket command only. 
 * @param[in]    sockID       - socket descriptor 
 * @param[in]    level        - To set the socket option take the socket level 
 * @param[in]    option_name  - provide the name of the ID \n
 *                              1.SO_MAX_RETRY - To select tcp max retry count \n
 *                              2.SO_SOCK_VAP_ID - To select the vap id \n
 *                              3.SO_TCP_KEEP_ALIVE-To configure the tcp keep alive initial time \n
 *                              4.SO_RCVBUF - To configure the application buffer for receiving server certificate \n
 *                              5.SO_SSL_ENABLE-To configure ssl socket \n
 *                              6.SO_HIGH_PERFORMANCE_SOCKET-To configure high performance socket 
 * @param[in]    option_value - value of the parameter 
 * @param[in]    option_len   - length of the parameter  
 * @return       0 - Success  \n
 *				-2 - Invalid parameters \n
 *				-3 - Command given in wrong state \n
 *				-4 - Buffer not available to serve the command
 *
 *
 */

int rsi_setsockopt(int32_t sockID, int level, int option_name, const void *option_value, rsi_socklen_t option_len)
{

  struct rsi_timeval *timeout = NULL;
  int32_t status              = RSI_SUCCESS;
  uint16_t timeout_val;
  uint32_t protocol;

#ifdef SOCKET_CLOSE_WAIT
  if (rsi_socket_pool[sockID].sock_state > RSI_SOCKET_STATE_INIT && rsi_socket_pool_non_rom[sockID].close_pending) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ECONNABORTED, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ECONNABORTED);
#endif
    return RSI_SOCK_ERROR;
  }
#endif

  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // If sockID is not in available range
  if (sockID < 0 || sockID >= RSI_NUMBER_OF_SOCKETS) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // Configure per socket maximum tcp retries count
  if ((option_name == SO_MAXRETRY)) {
    rsi_socket_pool_non_rom[sockID].max_tcp_retries = *(uint8_t *)option_value;
    return RSI_SUCCESS;
  }

  // Configure tcp keep alive time
  if ((option_name == SO_TCP_KEEP_ALIVE)) {
    rsi_uint16_to_2bytes(rsi_socket_pool_non_rom[sockID].tcp_keepalive_initial_time, *(uint16_t *)option_value);
    return RSI_SUCCESS;
  }

  // Configure ssl socket bit map
  if ((option_name == SO_SSL_ENABLE)) {
    rsi_socket_pool_non_rom[sockID].ssl_bitmap = RSI_SOCKET_FEAT_SSL;
    return RSI_SUCCESS;
  }

  // Configure ssl socket bit map with 1.0 version
  if ((option_name == SO_SSL_V_1_0_ENABLE)) {
    rsi_socket_pool_non_rom[sockID].ssl_bitmap = (RSI_SOCKET_FEAT_SSL | RSI_SSL_V_1_0);
    return RSI_SUCCESS;
  }

  // Configure ssl socket bit map with 1.1 version
  if ((option_name == SO_SSL_V_1_1_ENABLE)) {
    rsi_socket_pool_non_rom[sockID].ssl_bitmap = (RSI_SOCKET_FEAT_SSL | RSI_SSL_V_1_1);
    return RSI_SUCCESS;
  }

  // Configure ssl socket bit map with 1.2 version
  if ((option_name == SO_SSL_V_1_2_ENABLE)) {
    rsi_socket_pool_non_rom[sockID].ssl_bitmap = (RSI_SOCKET_FEAT_SSL | RSI_SSL_V_1_2);
    return RSI_SUCCESS;
  }
  // Configure tcp ack indication
  if ((option_name == SO_TCP_ACK_INDICATION)) {
    if (*(uint32_t *)option_value) {
      protocol = *(uint32_t *)option_value;
    } else {
      protocol = RSI_SOCKET_FEAT_TCP_ACK_INDICATION;
    }
    rsi_socket_pool[sockID].sock_bitmap |= RSI_SOCKET_FEAT_TCP_ACK_INDICATION;
    if (((protocol >> 3) & 0XF) == 0) {
      rsi_socket_pool[sockID].max_available_buffer_count = rsi_socket_pool[sockID].current_available_buffer_count = 4;
    } else {
      rsi_socket_pool[sockID].max_available_buffer_count = rsi_socket_pool[sockID].current_available_buffer_count =
        ((protocol >> 3) & 0xF);
    }
    return RSI_SUCCESS;
  }

  // Configure certificate index bit map
  if ((option_name == SO_CERT_INDEX)) {
    if (*(uint32_t *)option_value) {
      protocol = *(uint32_t *)option_value;
    } else {
      protocol = 0;
    }
    // Set certificate index bit
    rsi_socket_pool[sockID].sock_bitmap |= RSI_SOCKET_FEAT_CERT_INDEX;

    rsi_socket_pool[sockID].socket_cert_inx = protocol;
    return RSI_SUCCESS;
  }

  // Configure high performance socket
  if ((option_name == SO_HIGH_PERFORMANCE_SOCKET)) {
    rsi_socket_pool_non_rom[sockID].high_performance_socket = 1;
    return RSI_SUCCESS;
  }

  if ((option_name == SO_SOCK_VAP_ID)) {
    rsi_socket_pool_non_rom[sockID].vap_id = *(uint8_t *)option_value;
    return RSI_SUCCESS;
  }
  // Configure the TCP MSS size
  if (option_name == SO_TCP_MSS) {
    rsi_socket_pool_non_rom[sockID].tcp_mss = *(uint16_t *)option_value;
    return RSI_SUCCESS;
  }

  // Configure the TCP MSS size
  if (option_name == SO_TCP_RETRY_TRANSMIT_TIMER) {
    rsi_socket_pool_non_rom[sockID].tcp_retry_transmit_timer = *(uint8_t *)option_value;
    return RSI_SUCCESS;
  }
#ifndef RSI_M4_INTERFACE
  if ((option_name == SO_RCVBUF)) {
    rsi_socket_pool[sockID].recv_buffer        = (uint8_t *)option_value;
    rsi_socket_pool[sockID].recv_buffer_length = option_len;
    return RSI_SUCCESS;
  }
#endif
  if (option_name == SO_TLS_SNI) {
    status = rsi_fill_tls_extension(sockID, option_name, option_value, option_len);
    if (status != RSI_SUCCESS) {
      // Set error
      rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);

#ifdef RSI_WITH_OS
      rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
      return RSI_SOCK_ERROR;
    }
    return RSI_SUCCESS;
  }

  // For TOS
  if (option_name == IP_TOS) {
    rsi_socket_pool_non_rom[sockID].tos = *(uint32_t *)option_value;
    return RSI_SUCCESS;
  }

  if ((option_name != SO_RCVTIMEO) || (level != SOL_SOCKET)) {
    rsi_wlan_socket_set_status(RSI_ERROR_EINVAL, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EINVAL);
#endif
    return RSI_SOCK_ERROR;
  }
  // If socket is in not created state
  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);

#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  if (option_name == SO_RCVTIMEO) {
    timeout = (struct rsi_timeval *)option_value;
    if ((timeout->tv_sec == 0) && (timeout->tv_usec != 0) && (timeout->tv_usec < 1000)) {
      timeout->tv_usec = 1000;
    }
    timeout_val = (timeout->tv_usec / 1000) + (timeout->tv_sec * 1000);

    // This feature is available only if Synchrounous bitmap is set
    if (rsi_socket_pool[sockID].sock_bitmap & RSI_SOCKET_FEAT_SYNCHRONOUS) {
      rsi_socket_pool[sockID].read_time_out = timeout_val;
    } else {
      // Set error
      rsi_wlan_socket_set_status(RSI_ERROR_ENOPROTOOPT, sockID);
#ifdef RSI_WITH_OS
      rsi_set_os_errno(RSI_ERROR_ENOPROTOOPT);
#endif
      return RSI_SOCK_ERROR;
    }
  }
  return RSI_SUCCESS;
}
/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn           int rsi_getsockopt(int32_t sockID, int level, int option_name,const void *option_value, rsi_socklen_t option_len)
 * @brief        Get the socket options. 
 * @pre \ref rsi_getsockopt() API needs to be called after rsi_socket command only.   
 * @param[in]    sockID       - Socket descriptor 
 * @param[in]    level        - To set the socket option take the socket level 
 * @param[in]    option_name  - provided the name of the ID 
 * @param[in]    option_value - value of the parameter 
 * @param[in]    option_len   - length of the parameter  
 * @return       0              - Success \n
 *               Negative Value - Failure
 */

int rsi_getsockopt(int32_t sockID, int level, int option_name, const void *option_value, rsi_socklen_t option_len)
{
  UNUSED_PARAMETER(level);        //This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(option_value); //This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(option_len);   //This statement is added only to resolve compilation warning, value is unchanged
#ifdef RSI_WITH_OS
  int32_t status = 0;
#endif

#ifdef SOCKET_CLOSE_WAIT
  if (rsi_socket_pool[sockID].sock_state > RSI_SOCKET_STATE_INIT && rsi_socket_pool_non_rom[sockID].close_pending) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ECONNABORTED, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ECONNABORTED);
#endif
    return RSI_SOCK_ERROR;
  }
#endif

  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  // If sockID is not in available range
  if (sockID < 0 || sockID >= RSI_NUMBER_OF_SOCKETS) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  if ((option_name == SO_CHECK_CONNECTED_STATE)) {
    // If socket is in not created state
    if (rsi_socket_pool[sockID].sock_state != RSI_SOCKET_STATE_CONNECTED) {
      // Set error
      rsi_wlan_socket_get_status(sockID);
#ifdef RSI_WITH_OS
      status = rsi_get_error(sockID);
      rsi_set_os_errno(status);
#endif
      return RSI_SOCK_ERROR;
    }
  }

  return RSI_SUCCESS;
}

/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t  rsi_select(int32_t nfds, rsi_fd_set *readfds, rsi_fd_set *writefds, 
 *                                  rsi_fd_set *exceptfds, struct rsi_timeval *timeout,void 
 *                                 (*callback)(rsi_fd_set *fd_read, rsi_fd_set *fd_write, rsi_fd_set *fd_except, int32_t status)) 
 * @brief       Accept the connection request from the remote peer and register a callback which will be used by
 *  			the driver to forward the received connection request asynchronously to the application. 
 * @pre   \ref rsi_socket()/ \ref rsi_socket_async() API needs to be called before this API.  
 * @param[in]   nfds      - Maximum number of socket descriptors 
 * @param[in]   readfds   - This is the pointer to hold the bitmap for the select on read operation 
 * @param[in]   writefds  - This is the pointer to hold the bitmap for the select on write operation  
 * @param[in]   exceptfds - This is the pointer to hold the bitmap for exceptional cases for select request 
 * @param[in]   Timeout   - This is the timeout value to break the wait for select. 
 * @param[in]   callback  - This is the callback when asynchronous response comes for the select request. 
 * @param[in]   fd_read   - This is the pointer which holds the bitmap for the select on read operation 
 * @param[in]   fd_write  - This is the pointer which holds the bitmap for the select on write operation 
 * @param[in]   fd_except - This is the pointer which holds the bitmap for the select on exceptional operation 
 * @param[in]   status    - This is the status code 
 * @return	   0 - time out \n
 *             Positive Value - Indicates the total number of bits that are ready across all the descriptor sets \n 
 *             Negative Value - Failure
 *
 */
int32_t rsi_select(int32_t nfds,
                   rsi_fd_set *readfds,
                   rsi_fd_set *writefds,
                   rsi_fd_set *exceptfds,
                   struct rsi_timeval *timeout,
                   void (*callback)(rsi_fd_set *fd_read, rsi_fd_set *fd_write, rsi_fd_set *fd_except, int32_t status))
{
  int32_t status = RSI_SUCCESS;
  rsi_req_socket_select_t *select;
  rsi_pkt_t *pkt                 = NULL;
  rsi_driver_cb_t *rsi_driver_cb = global_cb_p->rsi_driver_cb;
  uint32_t read_select_bitmap = 0, write_select_bitmap = 0;
  int8_t socket_desc = 0;
  int8_t index       = 0;
  int i, read_count = 0, write_count = 0;
  int32_t rsi_select_response_wait_time = 0;
#ifdef SOCKET_CLOSE_WAIT
  rsi_fd_set rfds, wfds;
  RSI_FD_ZERO(&rfds);
  RSI_FD_ZERO(&wfds);
#endif
  // If nfds  is  not in available range
  if (nfds < 0 || nfds > NUMBER_OF_SOCKETS) {
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EINVAL);
#endif
    return RSI_SOCK_ERROR;
  }
  if (nfds > 0) {
    if ((readfds != NULL) && (readfds->fd_array[0] == 0)) {
      return RSI_SOCK_ERROR;
    }
    if ((writefds != NULL) && (writefds->fd_array[0] == 0)) {
      return RSI_SOCK_ERROR;
    }
  }
  RSI_MUTEX_LOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);
  for (i = 0; i < RSI_NUMBER_OF_SELECTS; i++) {
    if (rsi_socket_select_info[i].select_state == RSI_SOCKET_SELECT_STATE_INIT) {
      for (index = 0; index < nfds; index++) {

#ifdef SOCKET_CLOSE_WAIT
        if (rsi_socket_pool_non_rom[index].close_pending) {
          if ((readfds != NULL) && (readfds->fd_array[0]) & BIT(index)) {
            read_count += 1;
            RSI_FD_SET(index, &rfds);
          }
          if ((writefds != NULL) && (writefds->fd_array[0]) & BIT(index)) {
            write_count += 1;
            RSI_FD_SET(index, &wfds);
          }
        }
#endif

        if (readfds != NULL) {
          if (((readfds->fd_array[0]) & BIT(index))
              && (rsi_socket_pool[index].sock_state < RSI_SOCKET_STATE_CONNECTED)) {
            RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);
#ifdef RSI_WITH_OS
            rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
            return RSI_SOCK_ERROR;
          }
        }
        if (writefds != NULL) {
          if (((writefds->fd_array[0]) & BIT(index))
              && ((rsi_socket_pool[index].sock_state < RSI_SOCKET_STATE_CONNECTED)
                  && (!rsi_socket_pool_non_rom[index].sock_non_block))) {
            RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);
#ifdef RSI_WITH_OS
            rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
            return RSI_SOCK_ERROR;
          }
        }
      }
#ifdef SOCKET_CLOSE_WAIT
      if (read_count + write_count) {
        if (readfds != NULL) {
          readfds->fd_array[0] = rfds.fd_array[0];
        }
        if (writefds != NULL) {
          writefds->fd_array[0] = wfds.fd_array[0];
        }
        RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);
        return read_count + write_count;
      }
#endif
      memset(&rsi_socket_select_info[i], 0, sizeof(rsi_socket_select_info_t));
      if (!callback) {
        status = rsi_semaphore_create(&rsi_socket_select_info[i].select_sem, 0);
        if (status != RSI_ERROR_NONE) {
#ifdef RSI_WITH_OS
          rsi_set_os_errno(RSI_ERROR_ENOMEM);
#endif
          return RSI_SOCK_ERROR;
        }
      }
      // Allocate packet
      pkt = rsi_pkt_alloc(&rsi_driver_cb->wlan_cb->wlan_tx_pool);
      if (pkt == NULL) {
#ifdef RSI_WITH_OS
        rsi_set_os_errno(RSI_ERROR_ENOMEM);
#endif
        // Release mutex lock
        RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);
        return RSI_SOCK_ERROR;
      }

      rsi_socket_select_info[i].select_state = RSI_SOCKET_SELECT_STATE_CREATE;
      // Memset before filling
      select = (rsi_req_socket_select_t *)pkt->data;
      memset(select, 0, sizeof(rsi_req_socket_select_t));

      //updating the bitmap with the sock ids
      for (index = 0; index < nfds; index++) {
        if (readfds != NULL) {
          if ((readfds->fd_array[0]) & BIT(index)) {
            socket_desc = rsi_socket_pool[index].sock_id;
            read_select_bitmap |= BIT(socket_desc);
          }
        }
        if (writefds != NULL) {
          if ((writefds->fd_array[0]) & BIT(index)) {
            socket_desc = rsi_socket_pool[index].sock_id;
            write_select_bitmap |= BIT(socket_desc);
          }
        }
      }
      //saving the address to a pointer variable to access
      if (callback == NULL) {
        if (readfds != NULL) {
          rsi_socket_select_info[i].rsi_sel_read_fds = (rsi_fd_set *)readfds;
        }
        if (writefds != NULL) {
          rsi_socket_select_info[i].rsi_sel_write_fds = (rsi_fd_set *)writefds;
        }
      }
      // filling the select req struct
      select->num_fd = nfds;
      if (readfds != NULL) {
        select->rsi_read_fds.fd_array[0] = read_select_bitmap;
      }
      if (writefds != NULL) {
        select->rsi_write_fds.fd_array[0] = write_select_bitmap;
      }
      select->select_id = i;

      if (timeout != NULL) {
        select->rsi_select_timeval.tv_sec  = timeout->tv_sec;
        select->rsi_select_timeval.tv_usec = timeout->tv_usec;
        rsi_select_response_wait_time =
          ((select->rsi_select_timeval.tv_sec * 1000) + (select->rsi_select_timeval.tv_usec / 1000) + WAIT_TIMEOOUT
           + RSI_SELECT_RESPONSE_WAIT_TIME);
      } else {
        select->no_timeout            = 1;
        rsi_select_response_wait_time = 0;
      }
      if (callback) {
        // Set socket asynchronous receive callback
        rsi_socket_select_info[i].sock_select_callback = callback;
      }
      break;
    }
  }
  // If nfds  is  not in available range
  if (i >= RSI_NUMBER_OF_SELECTS) {
    RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EINVAL);
#endif
    return RSI_SOCK_ERROR;
  }

  // Release mutex lock
  RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);
  status = RSI_DRIVER_WLAN_SEND_CMD(RSI_WLAN_REQ_SELECT_REQUEST, pkt);
  if (!callback) {
    if (rsi_wait_on_socket_semaphore(&rsi_socket_select_info[i].select_sem, (rsi_select_response_wait_time))
        != RSI_ERROR_NONE) {
      rsi_select_set_status(RSI_ERROR_RESPONSE_TIMEOUT, i);
      rsi_semaphore_destroy(&rsi_socket_select_info[i].select_sem);
      return RSI_SOCK_ERROR;
    }
    // Free packet
    status = rsi_semaphore_destroy(&rsi_socket_select_info[i].select_sem);
    if (status != RSI_ERROR_NONE) {
#ifdef RSI_WITH_OS
      rsi_set_os_errno(RSI_ERROR_EBUSY);
#endif
      return RSI_SOCK_ERROR;
    }

    //clear the except fd set
    if (exceptfds != NULL) {
      RSI_FD_ZERO(exceptfds);
    }
    rsi_socket_select_info[i].select_state = RSI_SOCKET_SELECT_STATE_INIT;
  }
  status = rsi_select_get_status(i);
  if (status == RSI_SUCCESS) {
    if (readfds != NULL) {
      // returning value of count
      read_count = readfds->fd_count;
    }
    if (writefds != NULL) {
      write_count = writefds->fd_count;
    }
    return read_count + write_count;
  } else {
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EINVAL);
#endif
    return RSI_SOCK_ERROR;
  }
}

/*==============================================*/
/**
 * @fn          uint8_t calculate_buffers_required(uint8_t type, uint16_t length)
 * @brief       Calculate the buffer require. 
 * @param[in]   type - Type  
 * @param[in]   length - Lenght 
 * @return 		0              - Success \n
 *              Negative Value - Failure  
 *              
 *
 */
uint8_t calculate_buffers_required(uint8_t type, uint16_t length)
{
#ifdef ROM_WIRELESS
  return ROMAPI_WL->calculate_buffers_required(global_cb_p, type, length);
#else
  return api_wl->calculate_buffers_required(global_cb_p, type, length);
#endif
}
/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t  rsi_accept_async(int32_t sockID, void (*callback)(int32_t sock_id, int16_t dest_port,uint8_t *ip_addr, int16_t ip_version))
 * @brief      Accept the connection request from the remote peer and register a callback which will be used by
 * 			   the driver to forward the received connection request asynchronously to the application. 
 * @param[in]  sockID     - This is the socket descriptor ID 
 * @param[in]  callback   - callback function to indicate the socket parameters connected 
 * @param[in]  sock_id    - This is the socket descriptor ID 
 * @param[in]  dest_port  - Port number of remote peer 
 * @param[in]  ip_addr    - This is the remote peer address 
 * @parm[in]   ip_version - 4 if it is IPV4 connection  6 if it is IPV6 connection 
 * @return     0              - Success \n
 *             Negative Value - Failure
 *
 */

int32_t rsi_accept_async(int32_t sockID,
                         void (*callback)(int32_t sock_id, int16_t dest_port, uint8_t *ip_addr, int16_t ip_version))
{
  int32_t status = RSI_SUCCESS;
  rsi_req_socket_accept_t *accept;
  rsi_pkt_t *pkt = NULL;
  rsi_socket_info_t *sock_info;
  int8_t accept_sock_id = -1;

  rsi_driver_cb_t *rsi_driver_cb     = global_cb_p->rsi_driver_cb;
  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

#ifdef SOCKET_CLOSE_WAIT
  if (rsi_socket_pool[sockID].sock_state > RSI_SOCKET_STATE_INIT && rsi_socket_pool_non_rom[sockID].close_pending) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ECONNABORTED, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ECONNABORTED);
#endif
    return RSI_SOCK_ERROR;
  }
#endif

  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // If sockID is not in available range
  if (sockID < 0 || sockID >= NUMBER_OF_SOCKETS) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // Check socket is TCP or not
  if ((rsi_socket_pool[sockID].sock_type & 0xF) != SOCK_STREAM) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EPROTOTYPE, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EPROTOTYPE);
#endif
    return RSI_SOCK_ERROR;
  }

  // If socket is not in listen state and connected state
  if (rsi_socket_pool[sockID].sock_state != RSI_SOCKET_STATE_LISTEN) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  if (rsi_socket_pool[sockID].ltcp_socket_type != RSI_LTCP_PRIMARY_SOCKET) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EINVAL, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EINVAL);
#endif
    return RSI_SOCK_ERROR;
  }

  sock_info = &rsi_socket_pool[sockID];

  // check maximum backlogs count
  if (sock_info->backlogs == sock_info->backlog_current_count) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ENOBUFS, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ENOBUFS);
#endif
    return RSI_SOCK_ERROR;
  }

  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_LISTEN) {
    // Create new instance for socket
    accept_sock_id = rsi_socket_async((sock_info->sock_type >> 4),
                                      (sock_info->sock_type & 0xF),
                                      (sock_info->sock_bitmap & RSI_SOCKET_FEAT_SSL),
                                      sock_info->sock_receive_callback);

    if ((accept_sock_id >= 0) && (accept_sock_id < NUMBER_OF_SOCKETS)) {

      // Set socket as secondary socket
      rsi_socket_pool[accept_sock_id].ltcp_socket_type = RSI_LTCP_SECONDARY_SOCKET;

      // Save local port number
      rsi_socket_pool[accept_sock_id].source_port = rsi_socket_pool[sockID].source_port;
    } else {
      // Set error
      rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
      rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
      return RSI_SOCK_ERROR;
    }
  }

  // Allocate packet
  pkt = rsi_pkt_alloc(&rsi_driver_cb->wlan_cb->wlan_tx_pool);
  if (pkt == NULL) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_PKT_ALLOCATION_FAILURE, sockID);
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    return RSI_SOCK_ERROR;
  }
  // Set socket connection indication callback
  rsi_socket_pool_non_rom[accept_sock_id].accept_call_back_handler = callback;

  // send socket accept command
  accept = (rsi_req_socket_accept_t *)pkt->data;

  // Fill socket descriptor
  accept->socket_id = rsi_socket_pool[sockID].sock_id;

  // Fill local port number
  rsi_uint16_to_2bytes(accept->source_port, rsi_socket_pool[sockID].source_port);

  rsi_wlan_cb_non_rom->socket_cmd_rsp_pending |= BIT(accept_sock_id);
  // Send socket accept command
  status = RSI_DRIVER_WLAN_SEND_CMD(RSI_WLAN_REQ_SOCKET_ACCEPT, pkt);

  if (status != RSI_SUCCESS) {
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    return RSI_SOCK_ERROR;
  }

  // Return status
  return accept_sock_id;
}

/*==============================================*/
/**
 * @fn          int32_t  rsi_get_app_socket_descriptor(uint8_t *src_port)
 * @brief       Get the application socket descriptor from available socket pool
 * @param[in]  src_port - pointer to the socket source port
 * @return 		Positive Value - application index \n
 *              Negative Value - If index is not found 
 *
 */
int32_t rsi_get_app_socket_descriptor(uint8_t *src_port)
{
  int i;
  uint16_t source_port;

  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

  source_port = rsi_bytes2R_to_uint16(src_port);

  for (i = 0; i < NUMBER_OF_SOCKETS; i++) {
    if ((rsi_socket_pool[i].source_port == source_port) && (rsi_socket_pool[i].sock_state != RSI_SOCKET_STATE_CONNECTED)
        && (rsi_socket_pool[i].sock_state != RSI_SOCKET_STATE_LISTEN)) {
      break;
    }
  }

  if (i >= NUMBER_OF_SOCKETS) {
    return RSI_SOCK_ERROR;
  }

  return i;
}
/*==============================================*/
/**
 * @fn          int32_t rsi_get_socket_id(uint32_t src_port,uint32_t dst_port)
 * @brief       Function to get the primary socket ID from port number
 * @param[in]   src_port - the socket source port
 * @param[in]   dst_port - The destination port number
 * @return      0              - socket descriptor \n
 *              Negative Value - If socket is not found 
 */
int32_t rsi_get_socket_id(uint32_t src_port, uint32_t dst_port)
{
  int i;

  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

  for (i = 0; i < NUMBER_OF_SOCKETS; i++) {
    if (rsi_socket_pool[i].source_port == src_port) {
      break;
    } else if (rsi_socket_pool[i].destination_port == dst_port) {
      break;
    }
  }

  if (i >= NUMBER_OF_SOCKETS) {
    return RSI_SOCK_ERROR;
  }
  return i;
}
/*==============================================*/
/**
 * @fn          int32_t rsi_get_primary_socket_id(uint8_t *port_num)
 * @brief       Get the primary socket ID from port number
 * @param[in]   port_num - pointer to port number
 * @return      0              - socket descriptor \n
 *              Negative Value - If socket is not found 
 *
 */
int32_t rsi_get_primary_socket_id(uint8_t *port_num)
{
  int i;
  uint16_t port_number;

  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

  port_number = rsi_bytes2R_to_uint16(port_num);

  for (i = 0; i < NUMBER_OF_SOCKETS; i++) {
    if ((rsi_socket_pool[i].source_port == port_number)
        && (rsi_socket_pool[i].ltcp_socket_type == RSI_LTCP_PRIMARY_SOCKET)) {
      break;
    }
  }

  if (i >= NUMBER_OF_SOCKETS) {
    return RSI_SOCK_ERROR;
  }
  return i;
}
/** @} */

#ifndef RSI_M4_INTERFACE
/** @addtogroup WLAN
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_certificate_valid(uint16_t valid, uint16_t socket_id)
 * @brief      After validating the server certificate received from module, host can indicate the status to module using this API 
 * @param[in]  valid     - This field indicates whether the server certificate is valid or not \n
 *											1 - indicates that the server certificate is valid \n
 *											0 - indicates that the server certificate is not valid 
 * @param[in]  socket_id - Socket identifier 
 * @return		0              - Success \n
 *              Negative Value - Failure 
 */

int32_t rsi_certificate_valid(uint16_t valid, uint16_t socket_id)
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt = NULL;
  rsi_req_cert_valid_t *cert_valid;
  int32_t sockID = 0;
  sockID         = rsi_get_application_socket_descriptor(socket_id);
#ifdef SOCKET_CLOSE_WAIT
  if (rsi_socket_pool[sockID].sock_state > RSI_SOCKET_STATE_INIT && rsi_socket_pool_non_rom[sockID].close_pending) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ECONNABORTED, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ECONNABORTED);
#endif
    return RSI_SOCK_ERROR;
  }
#endif
  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  // Allocate packet
  pkt = rsi_pkt_alloc(&rsi_driver_cb->wlan_cb->wlan_tx_pool);
  if (pkt == NULL) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_PKT_ALLOCATION_FAILURE, socket_id);
    return RSI_SOCK_ERROR;
  }
  rsi_driver_cb_non_rom->socket_state = RSI_SOCKET_CMD_IN_PROGRESS;

  cert_valid = (rsi_req_cert_valid_t *)pkt->data;

  cert_valid->socket_id = socket_id;
  cert_valid->status    = valid;

  // Send cert valid command
  status = RSI_DRIVER_WLAN_SEND_CMD(RSI_WLAN_REQ_CERT_VALID, pkt);

  return status;
}
#endif

/*==============================================*/
/**
 * @fn          int32_t  rsi_socket_create_async(int32_t sockID, int32_t type, int32_t backlog)
 * @brief       Send socket create request to the module and receives the response asynchronously
 * @param[in]   global_cb_p - pointer to the global control block
 * @param[in]   sockID      - socket descriptor 
 * @param[in]   type        - type of socket to create
 * @param[in]   backlog     - number of backlogs for LTCP socket
 * @return 		0              - Success \n
 *              Negative Value - Failure
 *
 */

int32_t rsi_socket_create_async(int32_t sockID, int32_t type, int32_t backlog)
{
  rsi_pkt_t *pkt = NULL;
  rsi_req_socket_t *socket_create;
  int32_t status = 0;

  rsi_driver_cb_t *rsi_driver_cb     = global_cb_p->rsi_driver_cb;
  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

#ifdef SOCKET_CLOSE_WAIT
  if (rsi_socket_pool[sockID].sock_state > RSI_SOCKET_STATE_INIT && rsi_socket_pool_non_rom[sockID].close_pending) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ECONNABORTED, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ECONNABORTED);
#endif
    return RSI_SOCK_ERROR;
  }
#endif

  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
    return RSI_SOCK_ERROR;
  }

  // Socket Parameters
  pkt = rsi_pkt_alloc(&rsi_driver_cb->wlan_cb->wlan_tx_pool);
  if (pkt == NULL) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_PKT_ALLOCATION_FAILURE, sockID);
    return RSI_SOCK_ERROR;
  }

  // Get data pointer
  socket_create = (rsi_req_socket_t *)pkt->data;

  // Memset before filling
  memset(socket_create, 0, sizeof(rsi_req_socket_t));

  // Fill IP verison and destination IP address
  if ((rsi_socket_pool[sockID].sock_type >> 4) == AF_INET) {
    rsi_uint16_to_2bytes(socket_create->ip_version, 4);
    memcpy(socket_create->dest_ip_addr.ipv4_address,
           rsi_socket_pool[sockID].destination_ip_addr.ipv4,
           RSI_IPV4_ADDRESS_LENGTH);
  } else {
    rsi_uint16_to_2bytes(socket_create->ip_version, 6);
    memcpy(socket_create->dest_ip_addr.ipv6_address,
           rsi_socket_pool[sockID].destination_ip_addr.ipv6,
           RSI_IPV6_ADDRESS_LENGTH);
  }

  // Fill local port
  rsi_uint16_to_2bytes(socket_create->module_socket, rsi_socket_pool[sockID].source_port);

  // Fill destination port
  rsi_uint16_to_2bytes(socket_create->dest_socket, rsi_socket_pool[sockID].destination_port);

  // Fill socket type
  rsi_uint16_to_2bytes(socket_create->socket_type, type);

  // Set backlogs
  if (type == RSI_SOCKET_TCP_SERVER) {
    rsi_uint16_to_2bytes(socket_create->max_count, backlog);
    socket_create->socket_bitmap |= RSI_SOCKET_FEAT_LTCP_ACCEPT;
  } else {
    rsi_uint16_to_2bytes(socket_create->max_count, 0);
  }

  if (rsi_socket_pool_non_rom[sockID].sock_non_block) {
    type = (type | O_NONBLOCK);
    rsi_uint16_to_2bytes(socket_create->socket_type, type);
  }
  // Check for SSL feature and fill it in ssl bitmap
  if (rsi_socket_pool[sockID].sock_bitmap & RSI_SOCKET_FEAT_SSL) {
    socket_create->ssl_bitmap |= RSI_SOCKET_FEAT_SSL;
  }

  // Check for SYNCHRONOUS feature and fill it in socket bitmap
  if (rsi_socket_pool[sockID].sock_bitmap & RSI_SOCKET_FEAT_SYNCHRONOUS) {
    socket_create->socket_bitmap |= BIT(0);
  }

  // Check TCP ACK indication bit
  if (rsi_socket_pool[sockID].sock_bitmap & RSI_SOCKET_FEAT_TCP_ACK_INDICATION) {
    socket_create->socket_bitmap |= RSI_SOCKET_FEAT_TCP_ACK_INDICATION;
  }
  // Check TCP socket certificate index bit
  if (rsi_socket_pool[sockID].sock_bitmap & RSI_SOCKET_FEAT_CERT_INDEX) {
    socket_create->socket_bitmap |= RSI_SOCKET_FEAT_CERT_INDEX;
  }

  socket_create->socket_bitmap |= RSI_SOCKET_FEAT_TCP_RX_WINDOW;

  // Set the RX window size
  socket_create->rx_window_size = rsi_socket_pool[sockID].rx_window_buffers;

  // Get certificate index
  socket_create->socket_cert_inx = rsi_socket_pool[sockID].socket_cert_inx;

  // Fill VAP_ID
  socket_create->vap_id = RSI_VAP_ID;

  socket_create->vap_id = rsi_socket_pool_non_rom[sockID].vap_id;
  if (rsi_socket_pool_non_rom[sockID].tos) {
    rsi_uint32_to_4bytes(socket_create->tos, rsi_socket_pool_non_rom[sockID].tos);
  } else {
    rsi_uint32_to_4bytes(socket_create->tos, RSI_TOS);
  }
  if (rsi_socket_pool_non_rom[sockID].max_tcp_retries) {
    socket_create->max_tcp_retries_count = rsi_socket_pool_non_rom[sockID].max_tcp_retries;
  } else {
    socket_create->max_tcp_retries_count = 10;
  }
  if (*(uint16_t *)rsi_socket_pool_non_rom[sockID].tcp_keepalive_initial_time) {
    memcpy(socket_create->tcp_keepalive_initial_time, rsi_socket_pool_non_rom[sockID].tcp_keepalive_initial_time, 2);
  } else {
    rsi_uint16_to_2bytes(socket_create->tcp_keepalive_initial_time, RSI_SOCKET_KEEPALIVE_TIMEOUT);
  }
  if (rsi_socket_pool_non_rom[sockID].ssl_bitmap) {
    socket_create->ssl_bitmap = rsi_socket_pool_non_rom[sockID].ssl_bitmap;
  }
  if (rsi_wlan_cb_non_rom->tls_version & RSI_SOCKET_FEAT_SSL) {
    socket_create->ssl_bitmap |= rsi_wlan_cb_non_rom->tls_version;
    rsi_wlan_cb_non_rom->tls_version = 0;
  }
  if (rsi_socket_pool_non_rom[sockID].high_performance_socket) {
    socket_create->ssl_bitmap |= RSI_HIGH_PERFORMANCE_SOCKET;
  }
  //configuring ssl_ciphers_bitmap
  socket_create->ssl_ciphers_bitmap = RSI_SSL_CIPHERS;
  if (rsi_socket_pool_non_rom[sockID].tcp_mss) {
    socket_create->tcp_mss = rsi_socket_pool_non_rom[sockID].tcp_mss;
  }
  if (rsi_socket_pool_non_rom[sockID].tcp_retry_transmit_timer) {
    socket_create->tcp_retry_transmit_timer = rsi_socket_pool_non_rom[sockID].tcp_retry_transmit_timer;
  }

  if (!(rsi_wlan_cb_non_rom->callback_list.socket_connect_response_handler != NULL)) {
#ifndef RSI_SOCK_SEM_BITMAP
    rsi_socket_pool_non_rom[sockID].socket_wait_bitmap |= BIT(0);
#endif
  } else {
    rsi_wlan_cb_non_rom->socket_cmd_rsp_pending |= BIT(sockID);
  }

  if (rsi_socket_pool_non_rom[sockID].extension_offset) {
    socket_create->no_of_tls_extensions   = rsi_socket_pool_non_rom[sockID].no_of_tls_extensions;
    socket_create->total_extension_length = rsi_socket_pool_non_rom[sockID].extension_offset;
    memcpy(socket_create->tls_extension_data,
           rsi_socket_pool_non_rom[sockID].tls_extension_data,
           MAX_SIZE_OF_EXTENSION_DATA);
  }
  // Send socket create command
  status = RSI_DRIVER_WLAN_SEND_CMD(RSI_WLAN_REQ_SOCKET_CREATE, pkt);
  if (rsi_wlan_cb_non_rom->callback_list.socket_connect_response_handler != NULL) {
    // In case of ssl certificate bypass connect behaves as asynchronous cmd for ssl sockets
    rsi_wlan_socket_set_status(RSI_SUCCESS, sockID);
  } else {
    // wait on socket semaphore
    status =
      rsi_wait_on_socket_semaphore(&rsi_socket_pool_non_rom[sockID].socket_sem, RSI_SOCKET_CREATE_RESPONSE_WAIT_TIME);
    if (status != RSI_ERROR_NONE) {
      // get wlan/network command response status
      rsi_wlan_socket_set_status(status, sockID);
      return RSI_SOCK_ERROR;
    }
    // get wlan/network command response status
    status = rsi_wlan_socket_get_status(sockID);
  }
  return status;
}
/*==============================================*/
/**
 * @fn          int32_t rsi_tcp_window_update(uint32_t sockID, uint32_t new_size_bytes)
 * @brief       Update TCP window size from host
 * @param[in]   sockID - socket descriptor
 * @param[in]  	new_size_bytes - window size to update
 * @param[out]  Returns the value the window size got updated
 * @return      Positive Value - window size updated \n
 *              Negative Value - Failure 
 *             
 *
 *
 */

int32_t rsi_tcp_window_update(uint32_t sockID, uint32_t new_size_bytes)
{
  rsi_pkt_t *pkt = NULL;
  rsi_req_tcp_window_update_t *tcp_window_update;
  int32_t status = 0;

  rsi_driver_cb_t *rsi_driver_cb     = global_cb_p->rsi_driver_cb;
  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

#ifdef SOCKET_CLOSE_WAIT
  if (rsi_socket_pool[sockID].sock_state > RSI_SOCKET_STATE_INIT && rsi_socket_pool_non_rom[sockID].close_pending) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ECONNABORTED, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ECONNABORTED);
#endif
    return RSI_SOCK_ERROR;
  }
#endif
  if (rsi_socket_pool_non_rom[sockID].socket_terminate_indication) {
    rsi_wlan_socket_set_status(RSI_ERROR_ECONNABORTED, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ECONNABORTED);
#endif
    return RSI_SOCK_ERROR;
  }
  if (rsi_socket_pool[sockID].sock_state != RSI_SOCKET_STATE_CONNECTED) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  pkt = rsi_pkt_alloc(&rsi_driver_cb->wlan_cb->wlan_tx_pool);
  if (pkt == NULL) {
    rsi_wlan_socket_set_status(RSI_ERROR_PKT_ALLOCATION_FAILURE, sockID);
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    return RSI_SOCK_ERROR;
  }

  // Get data pointer
  tcp_window_update = (rsi_req_tcp_window_update_t *)pkt->data;

  // Memset before filling
  memset(tcp_window_update, 0, sizeof(rsi_req_tcp_window_update_t));
  tcp_window_update->socket_id   = rsi_socket_pool[sockID].sock_id;
  tcp_window_update->window_size = new_size_bytes;

#ifndef RSI_SOCK_SEM_BITMAP
  rsi_socket_pool_non_rom[sockID].socket_wait_bitmap |= BIT(0);
#endif

  status = RSI_DRIVER_WLAN_SEND_CMD(RSI_WLAN_REQ_UPDATE_TCP_WINDOW, pkt);

  // wait on socket semaphore
  status =
    rsi_wait_on_socket_semaphore(&rsi_socket_pool_non_rom[sockID].socket_sem, RSI_WLAN_TCP_WINDOW_RESPONSE_WAIT_TIME);
  if (status != RSI_ERROR_NONE) {
    // get wlan/network command response status
    rsi_wlan_socket_set_status(status, sockID);
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    return RSI_SOCK_ERROR;
  }
  // get wlan/network command response status
  status = rsi_wlan_socket_get_status(sockID);
  if (status != RSI_SUCCESS) {
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ERANGE);
#endif
    return RSI_SOCK_ERROR;
  }
  return rsi_socket_pool_non_rom[sockID].window_size;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_get_socket_descriptor(uint8_t *src_port, uint8_t *dst_port, uint8_t *ip_addr, uint16_t ip_version,uint16_t sockid)
 * @brief       Get the primary socket ID based on source port number, destination port number, destination ip address and the ip version.
 * @param[in]   src_port   - pointer to source port number
 * @param[in]   dst_port   - pointer to destination port number
 * @param[in]   ip_addr    - pointer to destinaion ip address
 * @param[in]   ip_version - ip version either IPV4/IPV6

 * @return		0              - socket descriptor \n
 *              Negative Value - If socket is not found 
 *
 */

int32_t rsi_get_socket_descriptor(uint8_t *src_port,
                                  uint8_t *dst_port,
                                  uint8_t *ip_addr,
                                  uint16_t ip_version,
                                  uint16_t sockid)
{
  int i;
  uint16_t source_port;
  uint16_t destination_port;

  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

  source_port      = rsi_bytes2R_to_uint16(src_port);
  destination_port = rsi_bytes2R_to_uint16(dst_port);

  for (i = 0; i < NUMBER_OF_SOCKETS; i++) {
    if (rsi_socket_pool_non_rom[i].sock_non_block) {
      if ((rsi_socket_pool[i].sock_id == sockid) && (rsi_socket_pool[i].sock_state != RSI_SOCKET_STATE_CONNECTED)) {
        return i; // goto here;
      }
      if (rsi_socket_pool[i].source_port != 0) {
        if ((rsi_socket_pool[i].source_port == source_port)
            && (rsi_socket_pool[i].sock_state != RSI_SOCKET_STATE_CONNECTED)
            && (rsi_socket_pool[i].sock_state != RSI_SOCKET_STATE_LISTEN)
            && rsi_socket_pool_non_rom[i].wait_to_connect) {
          break;
        }
      }
      // Compare source port and destination port
      else if ((rsi_socket_pool[i].destination_port == destination_port)
               && (rsi_socket_pool[i].sock_state != RSI_SOCKET_STATE_CONNECTED)
               && (rsi_socket_pool[i].sock_state != RSI_SOCKET_STATE_LISTEN)
               && rsi_socket_pool_non_rom[i].wait_to_connect) {

        // Compare destination IP addess
        if (ip_version == 4) {
          if (!(memcmp(rsi_socket_pool[i].destination_ip_addr.ipv4, ip_addr, RSI_IPV4_ADDRESS_LENGTH))) {
            break;
          }
        } else {
          if (!(memcmp(rsi_socket_pool[i].destination_ip_addr.ipv6, ip_addr, RSI_IPV6_ADDRESS_LENGTH))) {
            break;
          }
        }
      }
    } else {
      if (rsi_socket_pool[i].source_port != 0) {
        if ((rsi_socket_pool[i].source_port == source_port)
            && (rsi_socket_pool[i].sock_state != RSI_SOCKET_STATE_CONNECTED)
            && (rsi_socket_pool[i].sock_state != RSI_SOCKET_STATE_LISTEN)) {
          break;
        }
      }
      // Compare source port and destination port
      else if ((rsi_socket_pool[i].destination_port == destination_port)
               && (rsi_socket_pool[i].sock_state != RSI_SOCKET_STATE_CONNECTED)
               && (rsi_socket_pool[i].sock_state != RSI_SOCKET_STATE_LISTEN)) {

        // Compare destination IP addess
        if (ip_version == 4) {
          if (!(memcmp(rsi_socket_pool[i].destination_ip_addr.ipv4, ip_addr, RSI_IPV4_ADDRESS_LENGTH))) {
            break;
          }
        } else {
          if (!(memcmp(rsi_socket_pool[i].destination_ip_addr.ipv6, ip_addr, RSI_IPV6_ADDRESS_LENGTH))) {
            break;
          }
        }
      }
    }
  }
  if (i >= NUMBER_OF_SOCKETS) {
    return RSI_SOCK_ERROR;
  }

  return i;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_socket_async_non_rom(int32_t protocolFamily, int32_t type, int32_t protocol, void (*callback)(uint32_t sock_no, uint8_t *buffer, uint32_t length))
 * @brief      Create a socket and registers a callback which will be used by the driver 
 * 			   to forward the received packets asynchronously to the application (on packet reception) without waiting for recv API call. 
 * @pre  \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  protocolFamily - Protocol family to select IPv4 or IPv6 \n
 *                              AF_INET (2) : to select IPv4 \n
 *                              AF_INET6 (3) : to select IPv6
 * @param[in]  type           - Select socket type UDP or TCP \n  
 *                              SOCK_STREAM (1) : to select TCP \n 
 *                              SOCK_DGRM (2) : to select UDP  
 * @param[in]  protocol       - 0: non SSL sockets, 1: SSL sockets \n 
 *                              BIT(5) should be enabled for selecting the certificate index \n
 *                              0<<12 for index 0 \n 
 *                              1<<12 for index 1 
 * @param[in]  callback       - This is the callback function to read data asynchronously from socket \n
 *                              sock_no:  Application socket number \n
 *                              buffer:  Pointer to buffer to hold the data \n
 *                              length: length of the buffer.
 * @return     0              - Success \n
 *             Negative Value - Failure  
 *
 */

int32_t rsi_socket_async_non_rom(int32_t protocolFamily,
                                 int32_t type,
                                 int32_t protocol,
                                 void (*callback)(uint32_t sock_no, uint8_t *buffer, uint32_t length))
{
  int i;

  rsi_driver_cb_t *rsi_driver_cb     = global_cb_p->rsi_driver_cb;
  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;
  int32_t status                     = RSI_SUCCESS;

  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if ((wlan_cb->opermode != RSI_WLAN_CONCURRENT_MODE) && (wlan_cb->state < RSI_WLAN_STATE_IP_CONFIG_DONE)) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  // Check supported protocol family
  if ((protocolFamily != AF_INET) && (protocolFamily != AF_INET6)) {
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EAFNOSUPPORT);
#endif
    return RSI_SOCK_ERROR;
  }

  // Check supported socket type
  if ((type != SOCK_STREAM) && (type != SOCK_DGRAM) && (type != (SOCK_STREAM | O_NONBLOCK)) /*&& (type != SOCK_RAW)*/) {
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EPROTOTYPE);
#endif
    return RSI_SOCK_ERROR;
  }

  // Acquire mutex lock
  RSI_MUTEX_LOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);

  // search for free entry in socket pool
  for (i = 0; i < NUMBER_OF_SOCKETS; i++) {
    if (rsi_socket_pool[i].sock_state == RSI_SOCKET_STATE_INIT) {
      // Memset socket info
      memset(&rsi_socket_pool[i], 0, sizeof(rsi_socket_info_t));
#ifdef RSI_WITH_OS
      status = rsi_semaphore_check_and_destroy(&rsi_socket_pool_non_rom[i].socket_sem);
      if (status != RSI_ERROR_NONE) {
#ifdef RSI_WITH_OS
        rsi_set_os_errno(RSI_ERROR_EBUSY);
#endif
        return RSI_SOCK_ERROR;
      }
      status = rsi_semaphore_check_and_destroy(&rsi_socket_pool_non_rom[i].sock_send_sem);
      if (status != RSI_ERROR_NONE) {
#ifdef RSI_WITH_OS
        rsi_set_os_errno(RSI_ERROR_EBUSY);
#endif
        return RSI_SOCK_ERROR;
      }
      status = rsi_semaphore_check_and_destroy(&rsi_socket_pool_non_rom[i].sock_recv_sem);
      if (status != RSI_ERROR_NONE) {
#ifdef RSI_WITH_OS
        rsi_set_os_errno(RSI_ERROR_EBUSY);
#endif
        return RSI_SOCK_ERROR;
      }
#endif
      memset(&rsi_socket_pool_non_rom[i], 0, sizeof(rsi_socket_info_non_rom_t));

      status = rsi_semaphore_create(&rsi_socket_pool_non_rom[i].socket_sem, 0);
      if (status != RSI_ERROR_NONE) {
#ifdef RSI_WITH_OS
        rsi_set_os_errno(RSI_ERROR_ENOMEM);
#endif
        return RSI_SOCK_ERROR;
      }
      status = rsi_semaphore_create(&rsi_socket_pool_non_rom[i].sock_send_sem, 0);
      if (status != RSI_ERROR_NONE) {
#ifdef RSI_WITH_OS
        rsi_set_os_errno(RSI_ERROR_ENOMEM);
#endif
        return RSI_SOCK_ERROR;
      }
      status = rsi_semaphore_create(&rsi_socket_pool_non_rom[i].sock_recv_sem, 0);
      if (status != RSI_ERROR_NONE) {
#ifdef RSI_WITH_OS
        rsi_set_os_errno(RSI_ERROR_ENOMEM);
#endif
        return RSI_SOCK_ERROR;
      }

      // Change the socket state to create
      rsi_socket_pool[i].sock_state = RSI_SOCKET_STATE_CREATE;

      // Set socket type
      rsi_socket_pool[i].sock_type = type;

      // Set protocol type
      rsi_socket_pool[i].sock_type |= (protocolFamily << 4);

      if (callback) {
        // Set socket asynchronous receive callback
        rsi_socket_pool[i].sock_receive_callback = callback;
      } else {
        // If callback is registered set socket as asynchronous
        rsi_socket_pool[i].sock_bitmap |= RSI_SOCKET_FEAT_SYNCHRONOUS;
      }

      if (type & O_NONBLOCK) {
        type                                       = SOCK_STREAM;
        rsi_socket_pool_non_rom[i].wait_to_connect = 1;
        rsi_socket_pool_non_rom[i].sock_non_block  = 1;
      }
#ifndef BSD_COMPATIBILITY
      if (protocol & RSI_SOCKET_FEAT_SSL) {
        // Check SSL enabled or not
        rsi_socket_pool[i].sock_bitmap |= RSI_SOCKET_FEAT_SSL;
      }

      if (protocol & RSI_SOCKET_FEAT_TCP_ACK_INDICATION) {
        // Set TCP ACK indication bit
        rsi_socket_pool[i].sock_bitmap |= RSI_SOCKET_FEAT_TCP_ACK_INDICATION;

        if (((protocol >> 3) & 0XF) == 0) {
          rsi_socket_pool[i].max_available_buffer_count = rsi_socket_pool[i].current_available_buffer_count = 4;
        } else {
          rsi_socket_pool[i].max_available_buffer_count = rsi_socket_pool[i].current_available_buffer_count =
            ((protocol >> 3) & 0xF);
        }
      }

      if (protocol & RSI_SOCKET_FEAT_CERT_INDEX) {
        // Set certificate index bit
        rsi_socket_pool[i].sock_bitmap |= RSI_SOCKET_FEAT_CERT_INDEX;

        rsi_socket_pool[i].socket_cert_inx = (protocol >> 12);

      } else {
        rsi_socket_pool[i].socket_cert_inx = 0;
      }

      // Get RX window buffers
      rsi_socket_pool[i].rx_window_buffers = ((protocol >> 7) & 0xF);
#endif
      break;
    }
  }

  if (i >= NUMBER_OF_SOCKETS) {
    // Release mutex lock
    RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ENFILE);
#endif
    return RSI_SOCK_ERROR;
  }

  // Release mutex lock
  RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);

  // Return available index
  return i;
}
/*==============================================*/
/**
 * @fn          int32_t  rsi_socket_connect(int32_t sockID, struct rsi_sockaddr *remoteAddress, int32_t addressLength)
 * @brief       Connect the socket to specified remote address
 * @param[in]   sockID        - socket descriptor 
 * @param[in]   remoteAddress - remote peer address structure 
 * @param[in]   addressLength - remote peer address structrue length 

 * @return		0              - Success \n
 *              Negative Value - Failure
 *               
 */

int32_t rsi_socket_connect(int32_t sockID, struct rsi_sockaddr *remoteAddress, int32_t addressLength)
{
  int32_t status = RSI_SUCCESS;

  rsi_driver_cb_t *rsi_driver_cb     = global_cb_p->rsi_driver_cb;
  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

#ifdef SOCKET_CLOSE_WAIT
  if (rsi_socket_pool[sockID].sock_state > RSI_SOCKET_STATE_INIT && rsi_socket_pool_non_rom[sockID].close_pending) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ECONNABORTED, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ECONNABORTED);
#endif
    return RSI_SOCK_ERROR;
  }
#endif

  // If sockID is not in available range
  if (sockID < 0 || sockID >= NUMBER_OF_SOCKETS) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // If socket is in init state
  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // Check whether supplied address structure
  if (remoteAddress == RSI_NULL) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EFAULT, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EFAULT);
#endif
    return RSI_SOCK_ERROR;
  }

  // Check address length
  if (((remoteAddress->sa_family == AF_INET) && (addressLength != sizeof(struct rsi_sockaddr_in)))
      || ((remoteAddress->sa_family == AF_INET6) && (addressLength != sizeof(struct rsi_sockaddr_in6)))) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EAFNOSUPPORT, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EAFNOSUPPORT);
#endif
    return RSI_SOCK_ERROR;
  }

  // Check if the remote address family matches the local BSD socket address family
  if (remoteAddress->sa_family != (rsi_socket_pool[sockID].sock_type >> 4)) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EAFNOSUPPORT, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EAFNOSUPPORT);
#endif
    return RSI_SOCK_ERROR;
  }

  // Acquire mutex lock
  RSI_MUTEX_LOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);

  // save destination port number ip address and port number to use in udp send()
  if (remoteAddress->sa_family == AF_INET) {
    // Save destination IPv4 address
    memcpy(rsi_socket_pool[sockID].destination_ip_addr.ipv4,
           &(((struct rsi_sockaddr_in *)remoteAddress)->sin_addr.s_addr),
           RSI_IPV4_ADDRESS_LENGTH);
    rsi_socket_pool[sockID].destination_port = htons(((struct rsi_sockaddr_in *)remoteAddress)->sin_port);
  } else {
    memcpy(rsi_socket_pool[sockID].destination_ip_addr.ipv6,
           ((struct rsi_sockaddr_in6 *)remoteAddress)->sin6_addr._S6_un._S6_u32,
           RSI_IPV6_ADDRESS_LENGTH);
    rsi_socket_pool[sockID].destination_port = htons(((struct rsi_sockaddr_in6 *)remoteAddress)->sin6_port);
  }
  // Release mutex lock
  RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);

  if ((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_STREAM) {
    // send socket create command
    status = rsi_socket_create_async(sockID, RSI_SOCKET_TCP_CLIENT, 0);
  } else if (((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_DGRAM)
             && (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_CREATE)) {
    // send socket create command
    status = rsi_socket_create_async(sockID, RSI_SOCKET_LUDP, 0);
  }

  if (status != RSI_SUCCESS) {
    // Set error
    rsi_wlan_socket_set_status(status, sockID);
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    return RSI_SOCK_ERROR;
  }

  return RSI_SUCCESS;
}
/*==============================================*/
/**
 * @fn         int32_t  rsi_socket_recvfrom(int32_t sockID, int8_t *buffer, int32_t buffersize, int32_t flags,struct rsi_sockaddr *fromAddr, int32_t *fromAddrLen)
 * @brief      Retrieve the received data from the remote peer on a given socket descriptor. 
 * @pre  \ref rsi_socket() \ref rsi_socket_async() API needs to be called before this API. 
 * @param[in]  sockID      - This is the socket descriptor 
 * @param[in]  buffer      - This is the pointer to buffer to hold receive data. This is an out parameter. 
 * @param[in]  buffersize  - This is the size of the buffer supplied 
 * @param[in]  flags       - Reserved 
 * @param[in]  fromAddr    - This is the address of remote peer, from where current packet was received. It is an out parameter. 
 * @param[in]  fromAddrLen - This is the pointer which contains remote peer address(fromAddr) length 
 * @return	   Positive Value - Number of bytes received Success fully \n 
 *             0              - Socket close error \n
 *             Negative Value - Failure     
 *
 */

int32_t rsi_socket_recvfrom(int32_t sockID,
                            int8_t *buffer,
                            int32_t buffersize,
                            int32_t flags,
                            struct rsi_sockaddr *fromAddr,
                            int32_t *fromAddrLen)
{
  UNUSED_PARAMETER(flags); //This statement is added only to resolve compilation warning, value is unchanged

  struct rsi_sockaddr_in peer4_address;
  struct rsi_sockaddr_in6 peer6_address;
  int32_t status                      = RSI_SUCCESS;
  int32_t rsi_read_response_wait_time = 0;

  int32_t copy_length = 0;
  rsi_req_socket_read_t *data_recv;
  rsi_pkt_t *pkt = NULL;

  rsi_driver_cb_t *rsi_driver_cb     = global_cb_p->rsi_driver_cb;
  rsi_socket_info_t *sock_info       = &global_cb_p->rsi_socket_pool[sockID];
  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

#ifdef SOCKET_CLOSE_WAIT
  if (rsi_socket_pool[sockID].sock_state > RSI_SOCKET_STATE_INIT && rsi_socket_pool_non_rom[sockID].close_pending) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ECONNABORTED, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ECONNABORTED);
#endif
    return RSI_SOCK_ERROR;
  }
#endif

  // If socket is in init state
  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  // If sockID is not in available range
  if (sockID < 0 || sockID >= NUMBER_OF_SOCKETS) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // If socket is not in connected state
  if (rsi_socket_pool[sockID].sock_state != RSI_SOCKET_STATE_CONNECTED) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  if (rsi_socket_pool_non_rom[sockID].socket_terminate_indication) {
    rsi_socket_pool_non_rom[sockID].socket_terminate_indication = 0;
    return copy_length;
  }
  // Acquire mutex lock
  RSI_MUTEX_LOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);

  if (global_cb_p->rx_buffer_mem_copy != 1) {
    sock_info->recv_buffer = (uint8_t *)buffer;
#ifdef RSI_PROCESS_MAX_RX_DATA
    if (rsi_socket_pool[sockID].sock_type & SOCK_STREAM) {
      sock_info->recv_buffer_length = 0;
    } else {
      sock_info->recv_buffer_length = buffersize;
    }
#else
    sock_info->recv_buffer_length = buffersize;
#endif
  }
  // If nothing is left
  if (sock_info->sock_recv_available_length == 0 || global_cb_p->rx_buffer_mem_copy != 1) {
    // Allocate packet
    pkt = rsi_pkt_alloc(&rsi_driver_cb->wlan_cb->wlan_tx_pool);
    if (pkt == NULL) {
      // Release mutex lock
      RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);
      // set error
      rsi_wlan_socket_set_status(RSI_ERROR_PKT_ALLOCATION_FAILURE, sockID);
#ifdef RSI_WITH_OS
      status = rsi_get_error(sockID);
      rsi_set_os_errno(status);
#endif
      return RSI_SOCK_ERROR;
    }

    // Send data recieve command
    data_recv = (rsi_req_socket_read_t *)pkt->data;

    // memset the recv packet
    memset(data_recv, 0, sizeof(rsi_req_socket_read_t));

    // Fill socket ID
    data_recv->socket_id = sock_info->sock_id;
#ifdef RSI_PROCESS_MAX_RX_DATA
    if (rsi_socket_pool[sockID].sock_type & SOCK_STREAM) {
      if (buffersize > MAX_RX_DATA_LENGTH) {
        buffersize = MAX_RX_DATA_LENGTH;
      }
    } else {
      if (buffersize > 1472) {
        buffersize = 1472;
      }
    }
#else
    if (rsi_socket_pool[sockID].sock_type & SOCK_STREAM) {
      if (buffersize > 1460) {
        buffersize = 1460;
      }
    } else {
      if (buffersize > 1472) {
        buffersize = 1472;
      }
    }
#endif
    // Fill number of requesting bytes for receive
    rsi_uint32_to_4bytes(data_recv->requested_bytes, buffersize);

    // Fill number of requesting bytes for receive
    rsi_uint16_to_2bytes(data_recv->read_timeout, rsi_socket_pool[sockID].read_time_out);

    if (rsi_socket_pool[sockID].read_time_out) {
      rsi_read_response_wait_time =
        (rsi_socket_pool[sockID].read_time_out + WAIT_TIMEOOUT + RSI_SOCKET_RECVFROM_RESPONSE_WAIT_TIME);
    } else {
      rsi_read_response_wait_time = 0;
    }

    // Release mutex lock
    RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);

#ifndef RSI_SOCK_SEM_BITMAP
    rsi_socket_pool_non_rom[sockID].socket_wait_bitmap |= BIT(1);
#endif

    // Send socket receive request command
    status = RSI_DRIVER_WLAN_SEND_CMD(RSI_WLAN_REQ_SOCKET_READ_DATA, pkt);

    status =
      rsi_wait_on_socket_semaphore(&rsi_socket_pool_non_rom[sockID].sock_recv_sem, (rsi_read_response_wait_time));

    /* Check if we need to invalidate the recv buffer pointer */
    if (global_cb_p->rx_buffer_mem_copy != 1) {
      sock_info->recv_buffer = NULL;
    }

    if (status != RSI_ERROR_NONE) {
      // get wlan/network command response status
      rsi_wlan_socket_set_status(status, sockID);
#ifdef RSI_WITH_OS
      status = rsi_get_error(sockID);
      rsi_set_os_errno(status);
#endif
      return RSI_SOCK_ERROR;
    }

    // get wlan/network command response status
    status = rsi_wlan_socket_get_status(sockID);

    if (status != RSI_SUCCESS) {
      rsi_socket_pool_non_rom[sockID].socket_terminate_indication = 0;
#ifdef RSI_WITH_OS
      status = rsi_get_error(sockID);
      rsi_set_os_errno(status);
#endif
      return RSI_SOCK_ERROR;
    } else {
      if (rsi_socket_pool_non_rom[sockID].recv_pending_bit & BIT(0)) {
        rsi_socket_pool_non_rom[sockID].recv_pending_bit = 0;
      } else if (rsi_socket_pool_non_rom[sockID].recv_pending_bit & BIT(1)) {
        rsi_socket_pool_non_rom[sockID].recv_pending_bit = 0;
        return 0;
      }
    }
  }

  if (global_cb_p->rx_buffer_mem_copy == 1) {
    // Get minimum of requested length and available length
    if (buffersize > (int32_t)sock_info->sock_recv_available_length) {
      copy_length = sock_info->sock_recv_available_length;
    } else {
      copy_length = buffersize;
    }

    // Copy available buffer
    buffer = (int8_t *)sock_info->sock_recv_buffer;

    // Increase buffer offset
    sock_info->sock_recv_buffer_offset += copy_length;

    if ((int32_t)(sock_info->sock_recv_available_length) >= copy_length) {
      // Decrease available length
      sock_info->sock_recv_available_length -= copy_length;
    } else {
      sock_info->sock_recv_available_length = 0;
    }

    if (sock_info->sock_recv_available_length == 0) {
      // Reset offset value after reading total packet
      sock_info->sock_recv_buffer_offset = 0;
    }
  } else {
    copy_length = sock_info->recv_buffer_length;
  }

  // If fromAddr is not NULL then copy the IP address
  if (fromAddr && (*fromAddrLen != 0)) {
    if ((sock_info->sock_type >> 4) == AF_INET) {
      // Update the Client address with socket family, remote host IPv4 address and port
      peer4_address.sin_family = AF_INET;
      memcpy(&peer4_address.sin_addr.s_addr,
             (ntohl(rsi_socket_pool[sockID].destination_ip_addr.ipv4)),
             RSI_IPV4_ADDRESS_LENGTH);
      peer4_address.sin_port = ntohs((uint16_t)rsi_socket_pool[sockID].destination_port);

      // Copy the peer address/port info to the ClientAddress
      // Truncate if addressLength is smaller than the size of struct rsi_sockaddr_in
      if (*fromAddrLen > (int32_t)sizeof(struct rsi_sockaddr_in)) {
        *fromAddrLen = sizeof(struct rsi_sockaddr_in);
      }
      memcpy(fromAddr, &peer4_address, *fromAddrLen);
    } else {
      peer6_address.sin6_family = AF_INET6;
      peer6_address.sin6_port   = ntohs((uint16_t)rsi_socket_pool[sockID].destination_port);

      peer6_address.sin6_addr._S6_un._S6_u32[0] =
        (uintptr_t)ntohl(&rsi_socket_pool[sockID].destination_ip_addr.ipv6[0]);
      peer6_address.sin6_addr._S6_un._S6_u32[1] =
        (uintptr_t)ntohl(&rsi_socket_pool[sockID].destination_ip_addr.ipv6[4]);
      peer6_address.sin6_addr._S6_un._S6_u32[2] =
        (uintptr_t)ntohl(&rsi_socket_pool[sockID].destination_ip_addr.ipv6[8]);
      peer6_address.sin6_addr._S6_un._S6_u32[3] =
        (uintptr_t)ntohl(&rsi_socket_pool[sockID].destination_ip_addr.ipv6[12]);

      // Copy the peer address/port info to the ClientAddress
      // Truncate if addressLength is smaller than the size of struct rsi_sockaddr_in
      if ((*fromAddrLen) > (int32_t)sizeof(struct rsi_sockaddr_in6)) {
        *fromAddrLen = sizeof(struct rsi_sockaddr_in6);
      }
      memcpy(fromAddr, &peer6_address, *fromAddrLen);
    }
  }

  return copy_length;
}
/*==============================================*/
/**
 * @fn         int32_t rsi_socket_listen(int32_t sockID, int32_t backlog)
 * @brief      Make socket to listen for remote connecion request in passive mode
 * @param[in]  sockID  - socket descriptor 
 * @param[in]  backlog - maximum number of pending connections requests
 * @return 	   0              - Success \n
 *             Negative Value - Failure 
 *
 */

int32_t rsi_socket_listen(int32_t sockID, int32_t backlog)
{
  int32_t status                     = RSI_SUCCESS;
  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

#ifdef SOCKET_CLOSE_WAIT
  if (rsi_socket_pool[sockID].sock_state > RSI_SOCKET_STATE_INIT && rsi_socket_pool_non_rom[sockID].close_pending) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ECONNABORTED, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ECONNABORTED);
#endif
    return RSI_SOCK_ERROR;
  }
#endif

  // If socket is in init state
  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  // If sockID is not in available range
  if (sockID < 0 || sockID >= NUMBER_OF_SOCKETS) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // Check socket is TCP or not
  if ((rsi_socket_pool[sockID].sock_type & 0xF) != SOCK_STREAM) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EPROTOTYPE, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EPROTOTYPE);
#endif
    return RSI_SOCK_ERROR;
  }

  // If socket is not binded
  if (rsi_socket_pool[sockID].sock_state != RSI_SOCKET_STATE_BIND) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // Set minimum backlog
  if (backlog < TCP_LISTEN_MIN_BACKLOG) {
    backlog = TCP_LISTEN_MIN_BACKLOG;
  }

  // Create socket
  status = rsi_socket_create_async(sockID, RSI_SOCKET_TCP_SERVER, backlog);

  if (status == RSI_SUCCESS) {
    // If fails memset socket info
    // Save backlogs
    rsi_socket_pool[sockID].backlogs = backlog;
    // Set as master socket
    rsi_socket_pool[sockID].ltcp_socket_type = RSI_LTCP_PRIMARY_SOCKET;
  } else {
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
    status = RSI_SOCK_ERROR;
#endif
  }
  return status;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_socket_shutdown(int32_t sockID, int32_t how)
 * @brief       Close the socket specified in a socket descriptor. 
 * @pre  \ref rsi_socket()/ \ref rsi_socket_async() API needs to be called before this API. 
 * @param[in]   sockID - This is the socket descriptor ID 
 * @param[in]   how    - 0: close the specified socket \n
 *                       1: close all the sockets open on specified socket's source port number. 
 * @return		0              - Success \n
 *              Negative Value - Failure 
 *
 */
int32_t rsi_socket_shutdown(int32_t sockID, int32_t how)
{
  int32_t status = RSI_SUCCESS;

  rsi_req_socket_close_t *close;
  rsi_pkt_t *pkt = NULL;

  rsi_driver_cb_t *rsi_driver_cb     = global_cb_p->rsi_driver_cb;
  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

  // If socket is in init state
  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  // If sockID is not in available range
  if (sockID < 0 || sockID >= NUMBER_OF_SOCKETS) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

#ifdef SOCKET_CLOSE_WAIT
  if (rsi_socket_pool[sockID].sock_state > RSI_SOCKET_STATE_INIT && rsi_socket_pool_non_rom[sockID].close_pending) {
    // Memeset socket info
    memset(&rsi_socket_pool[sockID], 0, sizeof(rsi_socket_info_t));

    rsi_socket_pool_non_rom[sockID].close_pending = 0;
    rsi_wlan_socket_set_status(RSI_SUCCESS, sockID);
    // Return success
    return RSI_SUCCESS;
  }
#endif

  // If socket is not in connected state dont send socket close command
  if (rsi_socket_pool[sockID].sock_state != RSI_SOCKET_STATE_CONNECTED) {
    if (rsi_socket_pool[sockID].ltcp_socket_type != RSI_LTCP_PRIMARY_SOCKET) {
      // Memeset socket info
      memset(&rsi_socket_pool[sockID], 0, sizeof(rsi_socket_info_t));

      rsi_wlan_socket_set_status(RSI_SUCCESS, sockID);
      // Return success
      return RSI_SUCCESS;
    } else {
      if (rsi_socket_pool[sockID].sock_state != RSI_SOCKET_STATE_LISTEN) {
        // Memeset socket info
        memset(&rsi_socket_pool[sockID], 0, sizeof(rsi_socket_info_t));

        rsi_wlan_socket_set_status(RSI_SUCCESS, sockID);
        // Return success
        return RSI_SUCCESS;
      }
      how = 1;
    }
  }

  // Allocate packet
  pkt = rsi_pkt_alloc(&rsi_driver_cb->wlan_cb->wlan_tx_pool);
  if (pkt == NULL) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_PKT_ALLOCATION_FAILURE, sockID);
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    return RSI_SOCK_ERROR;
  }

  // Fill socket close command
  close = (rsi_req_socket_close_t *)pkt->data;

  if (how == 1) {
    // Port based close for LTCP
    rsi_uint16_to_2bytes(close->socket_id, 0);
    rsi_uint16_to_2bytes(close->port_number, rsi_socket_pool[sockID].source_port);
  } else {
    // Socket descriptor based close
    rsi_uint16_to_2bytes(close->socket_id, rsi_socket_pool[sockID].sock_id);
    rsi_uint16_to_2bytes(close->port_number, 0);
  }

#ifndef RSI_SOCK_SEM_BITMAP
  rsi_socket_pool_non_rom[sockID].socket_wait_bitmap |= BIT(0);
#endif

  // Send socket close command
  status = RSI_DRIVER_WLAN_SEND_CMD(RSI_WLAN_REQ_SOCKET_CLOSE, pkt);

  // wait on socket semaphore
  status =
    rsi_wait_on_socket_semaphore(&rsi_socket_pool_non_rom[sockID].socket_sem, RSI_SOCKET_CLOSE_RESPONSE_WAIT_TIME);
  if (status != RSI_ERROR_NONE) {
    // get wlan/network command response status
    rsi_wlan_socket_set_status(status, sockID);
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    return RSI_SOCK_ERROR;
  }
  // Clear socket info
  rsi_clear_sockets(sockID);
  // get socket command response status
  status = rsi_wlan_socket_get_status(sockID);
  if (status != RSI_SUCCESS) {
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    return RSI_SOCK_ERROR;
  }

  // Return status
  return status;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_socket_bind(int32_t sockID, struct rsi_sockaddr *localAddress, int32_t addressLength)
 * @brief       Assign address to the socket
 * @param[in]   sockID        - socket descriptor
 * @param[in]   localAddress  - address which needs to be assign 
 * @param[in]   addressLength - length of the socket address 
 * @return		0              - Success \n
 *              Negative Value - Failure 
 *
 *
 */
int32_t rsi_socket_bind(int32_t sockID, struct rsi_sockaddr *localAddress, int32_t addressLength)
{
  int32_t local_port;
  int32_t status = RSI_SUCCESS;
  uint8_t i;

  rsi_driver_cb_t *rsi_driver_cb     = global_cb_p->rsi_driver_cb;
  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

#ifdef SOCKET_CLOSE_WAIT
  if (rsi_socket_pool[sockID].sock_state > RSI_SOCKET_STATE_INIT && rsi_socket_pool_non_rom[sockID].close_pending) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ECONNABORTED, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ECONNABORTED);
#endif
    return RSI_SOCK_ERROR;
  }
#endif

  // If socket is in init state
  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  // If sockID is not in available range
  if (sockID < 0 || sockID >= NUMBER_OF_SOCKETS) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // If socket is not created
  if (rsi_socket_pool[sockID].sock_state != RSI_SOCKET_STATE_CREATE) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // Check for a valid input local address and address length input buffer
  if ((localAddress == RSI_NULL) || (addressLength == 0)) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // Check address length
  if (((localAddress->sa_family == AF_INET) && (addressLength != sizeof(struct rsi_sockaddr_in)))
      || ((localAddress->sa_family == AF_INET6) && (addressLength != sizeof(struct rsi_sockaddr_in6)))) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // Check address family
  if ((rsi_socket_pool[sockID].sock_type >> 4) != localAddress->sa_family) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);

#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // Acquire mutex lock
  RSI_MUTEX_LOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);

  // Pickup the local port
  if (localAddress->sa_family == AF_INET) {
    local_port = ntohs(((struct rsi_sockaddr_in *)localAddress)->sin_port);
  } else {
    local_port = ntohs(((struct rsi_sockaddr_in6 *)localAddress)->sin6_port);
  }

  // Check whether local port is already used or not
  for (i = 0; i < NUMBER_OF_SOCKETS; i++) {
    if (rsi_socket_pool[i].source_port == local_port) {
      // Release mutex lock
      RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);

      // Set error
      rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);

#ifdef RSI_WITH_OS
      rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
      return RSI_SOCK_ERROR;
    }
  }

  // Port number is not used save the port number
  rsi_socket_pool[sockID].source_port = local_port;

  // Release mutex lock
  RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);

  // Send socket create if it is UDP
  if ((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_DGRAM) {
    status = rsi_socket_create_async(sockID, RSI_SOCKET_LUDP, 0);

    // Set socket state as connected
    rsi_socket_pool[sockID].sock_state = RSI_SOCKET_STATE_CONNECTED;
  } else {
    // Set socket state as bind
    rsi_socket_pool[sockID].sock_state = RSI_SOCKET_STATE_BIND;
  }

  if (status != RSI_SUCCESS) {
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    return RSI_SOCK_ERROR;
  }

  // return success
  return RSI_SUCCESS;
}
/*==============================================*/
/**
 * @fn          rsi_error_t rsi_wait_on_socket_semaphore(rsi_semaphore_handle_t *semaphore, uint32_t timeout_ms ) 
 * @brief       Wireless Library to acquire or wait for nwk semaphore.
 * @param[in]   semaphore  - Semaphore handle pointer  
 * @param[in]   timeout_ms - Maximum time to wait to acquire semaphore. If timeout_ms is 0 then wait
 *              till acquire semaphore.
 * @return      0              - Success \n
 *              Negative Value - Failure 
 */

rsi_error_t rsi_wait_on_socket_semaphore(rsi_semaphore_handle_t *semaphore, uint32_t timeout_ms)
{
  // Wait on wlan semaphore
  if (rsi_semaphore_wait(semaphore, timeout_ms) != RSI_ERROR_NONE) {
#ifndef RSI_WAIT_TIMEOUT_EVENT_HANDLE_TIMER_DISABLE
    if (rsi_driver_cb_non_rom->rsi_wait_timeout_handler_error_cb != NULL) {
      rsi_driver_cb_non_rom->rsi_wait_timeout_handler_error_cb(RSI_ERROR_RESPONSE_TIMEOUT, SOCKET_CMD);
    }
#endif
    return RSI_ERROR_RESPONSE_TIMEOUT;
  }
  return RSI_ERROR_NONE;
}
/*==============================================*/
/**
 * @fn          int32_t  rsi_application_socket_descriptor(int32_t sock_id)
 * @brief       Get the application socket descriptor from module socket descriptor
 * @param[in]   sock_id - module's socket descriptor
 * @return		Positive Value - application index \n
 *              Negative Value - If index is not found 
 *
 */
int32_t rsi_application_socket_descriptor(int32_t sock_id)
{
  int i;

  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

  for (i = 0; i < NUMBER_OF_SOCKETS; i++) {
    if (rsi_socket_pool[i].sock_id == sock_id && rsi_socket_pool[i].sock_state == RSI_SOCKET_STATE_CONNECTED
#ifdef SOCKET_CLOSE_WAIT
        && !(rsi_socket_pool_non_rom[i].close_pending)
#endif
    ) {
      break;
    }
  }

  if (i >= NUMBER_OF_SOCKETS) {
    return RSI_SOCK_ERROR;
  }

  return i;
}
/*==============================================*/
/**
 * @fn          int32_t rsi_wlan_socket_get_status(int32_t sockID)
 * @brief       Return wlan status
 * @param[in]   sockID - Application socket ID 
 * @return      wlan status
 *
 */
int32_t rsi_wlan_socket_get_status(int32_t sockID)
{
  return rsi_socket_pool_non_rom[sockID].socket_status;
}

/*==============================================*/
/**
 * @fn          void rsi_wlan_socket_set_status(int32_t status, int32_t sockID)
 * @brief       Set wlan status
 * @param[in]   status - status value to be set 
 * @param[in]   sockID - Application socket ID 
 * @return      void
 *
 */
void rsi_wlan_socket_set_status(int32_t status, int32_t sockID)
{
  rsi_socket_pool_non_rom[sockID].socket_status = status;
#ifndef RSI_WLAN_STATUS
  rsi_wlan_set_status(status);
#endif
}
/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn          int32_t rsi_select_get_status(int32_t selectid)
 * @brief       Return wlan status
 * @param[in]   selectid - This is the socket ID to get the status 
 * @return      0              - Success \n
 *              Negative Value - Failure  
 */
int32_t rsi_select_get_status(int32_t selectid)
{
  return rsi_socket_select_info[selectid].select_status;
}

/** @} */

/** @addtogroup NETWORK5
* @{
*/
/*==============================================*/
/**
 * @fn          void rsi_select_set_status(int32_t status, int32_t selectid)
 * @brief       Set the status of the socket specified in the select ID. 
 * @pre  \ref rsi_socket()/ \ref rsi_socket_async() API needs to be called before this API. 
 * @param[in]   status   - Status value to be set 
 * @param[in]   selectid - This is the socket ID on which the status is set 
 * @return      void
 *
 */
void rsi_select_set_status(int32_t status, int32_t selectid)
{
  rsi_socket_select_info[selectid].select_status = status;
#ifndef RSI_WLAN_STATUS
  rsi_wlan_set_status(status);
#endif
}
/*==============================================*/
/**
 * @fn         int32_t rsi_send_async_non_rom(int32_t sockID, const int8_t *msg, int32_t msgLength, int32_t flags, 
 *                 void (*data_transfer_complete_handler)(int32_t sockID, uint16_t length))
 * @brief      Send data on a given socket asynchronously
 * @param[in]  sockID                         - socket descriptor 
 * @param[in]  msg                            - pointer to data which needs to be send to remote peer 
 * @param[in]  msgLength                      - length of data to send 
 * @param[in]  flags                          - reserved 
 * @param[in]  data_transfer_complete_handler - pointer to the callback function which will be called after data transfer completion
 * @return	   0              - Success \n
 *             Negative Value - Failure  
 *
 *
 */
int32_t rsi_send_async_non_rom(int32_t sockID,
                               const int8_t *msg,
                               int32_t msgLength,
                               int32_t flags,
                               void (*data_transfer_complete_handler)(int32_t sockID, uint16_t length))
{
  struct rsi_sockaddr_in fromAddr4;
  struct rsi_sockaddr_in6 fromAddr6;
  int32_t status                     = 0;
  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  if ((rsi_socket_pool[sockID].sock_type >> 4) == AF_INET) {

    // Set socket family for IPv4
    fromAddr4.sin_family = AF_INET;

    // Set destination port
    fromAddr4.sin_port = rsi_socket_pool[sockID].destination_port;

    // Set destination IPv4 address
    memcpy(&fromAddr4.sin_addr.s_addr, rsi_socket_pool[sockID].destination_ip_addr.ipv4, RSI_IPV4_ADDRESS_LENGTH);

    status = rsi_sendto_async(sockID,
                              (int8_t *)msg,
                              msgLength,
                              flags,
                              (struct rsi_sockaddr *)&fromAddr4,
                              sizeof(fromAddr4),
                              data_transfer_complete_handler);
  } else {

    // Set socket family for IPv6
    fromAddr6.sin6_family = AF_INET6;

    // Set destination port
    fromAddr6.sin6_port = rsi_socket_pool[sockID].destination_port;

    // Set destination IPv6 address
    memcpy(fromAddr6.sin6_addr._S6_un._S6_u8,
           rsi_socket_pool[sockID].destination_ip_addr.ipv6,
           RSI_IPV6_ADDRESS_LENGTH);

    status = rsi_sendto_async(sockID,
                              (int8_t *)msg,
                              msgLength,
                              flags,
                              (struct rsi_sockaddr *)&fromAddr6,
                              sizeof(fromAddr6),
                              data_transfer_complete_handler);
  }

  // Return status
  return status;
}

/*==============================================*/
/**
 * @fn          int32_t  rsi_sendto_async_non_rom(int32_t sockID, int8_t *msg,
 *                                           int32_t msgLength, int32_t flags, struct rsi_sockaddr *destAddr, int32_t destAddrLen,
 *                                           void (*data_transfer_complete_handler)(int32_t sockID, uint16_t length))
 *
 * @brief       Send data to specific remote peer on a given socket
 * @param[in]   sockID                         - socket descriptor 
 * @param[in]   msg                            - pointer to data which needs to be send to remote peer 
 * @param[in]   msgLength                      - length of data to send 
 * @param[in]   flags                          - reserved 
 * @param[in]   destAddr                       - remote peer address to send data 
 * @param[in]   destAddrLen                    - rmeote peer address length
 * @param[in]   data_transfer_complete_handler - pointer to the callback function which will be called after data transfer completion
 * @return		0              - Success \n
 *              Negative Value - Failure  
 *
 *
 */
int32_t rsi_sendto_async_non_rom(int32_t sockID,
                                 int8_t *msg,
                                 int32_t msgLength,
                                 int32_t flags,
                                 struct rsi_sockaddr *destAddr,
                                 int32_t destAddrLen,
                                 void (*data_transfer_complete_handler)(int32_t sockID, uint16_t length))
{
  UNUSED_PARAMETER(destAddrLen); //This statement is added only to resolve compilation warning, value is unchanged
  int32_t status        = RSI_SUCCESS;
  int32_t maximum_limit = 0;
  uint8_t buffers_required;
  UNUSED_PARAMETER(flags); //This statement is added only to resolve compilation warning, value is unchanged
  rsi_driver_cb_t *rsi_driver_cb     = global_cb_p->rsi_driver_cb;
  rsi_socket_info_t *sock_info       = &global_cb_p->rsi_socket_pool[sockID];
  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

#ifdef SOCKET_CLOSE_WAIT
  if (rsi_socket_pool[sockID].sock_state > RSI_SOCKET_STATE_INIT && rsi_socket_pool_non_rom[sockID].close_pending) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_ECONNABORTED, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_ECONNABORTED);
#endif
    return RSI_SOCK_ERROR;
  }
#endif

  if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }
  // If sockID is not in available range
  if (sockID < 0 || sockID >= NUMBER_OF_SOCKETS) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EBADF, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EBADF);
#endif
    return RSI_SOCK_ERROR;
  }

  // If socket is in init state for UDP
  if (((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_DGRAM)
      && (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_INIT)) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EPROTOTYPE, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EPROTOTYPE);
#endif
    return RSI_SOCK_ERROR;
  }
  if (((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_STREAM)
      && (rsi_socket_pool[sockID].sock_state != RSI_SOCKET_STATE_CONNECTED)) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EPROTOTYPE, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EPROTOTYPE);
#endif
    return RSI_SOCK_ERROR;
  }

  // Check socket family
  if (destAddr->sa_family != (rsi_socket_pool[sockID].sock_type >> 4)) {
    // Set error
    rsi_wlan_socket_set_status(RSI_ERROR_EFAULT, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EFAULT);
#endif
    return RSI_SOCK_ERROR;
  }

  // Register the callback
  rsi_wlan_cb_non_rom->nwk_callbacks.data_transfer_complete_handler = data_transfer_complete_handler;

  // Acquire mutex lock
  RSI_MUTEX_LOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);

  if (((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_DGRAM)
      && (rsi_socket_pool[sockID].sock_state != RSI_SOCKET_STATE_CONNECTED)) {
    if ((rsi_socket_pool[sockID].sock_type >> 4) == AF_INET) {
      memcpy(rsi_socket_pool[sockID].destination_ip_addr.ipv4,
             &(((struct rsi_sockaddr_in *)destAddr)->sin_addr.s_addr),
             RSI_IPV4_ADDRESS_LENGTH);
      rsi_socket_pool[sockID].destination_port = ntohs(((struct rsi_sockaddr_in *)destAddr)->sin_port);
    } else {
      memcpy(rsi_socket_pool[sockID].destination_ip_addr.ipv6,
             ((struct rsi_sockaddr_in6 *)destAddr)->sin6_addr._S6_un._S6_u32,
             RSI_IPV6_ADDRESS_LENGTH);
      rsi_socket_pool[sockID].destination_port = ntohs(((struct rsi_sockaddr_in6 *)destAddr)->sin6_port);
    }
    // Create socket
    status = rsi_socket_create_async(sockID, RSI_SOCKET_LUDP, 0);
    if (status != RSI_SUCCESS) {
      // Release mutex lock
      RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);
#ifdef RSI_WITH_OS
      status = rsi_get_error(sockID);
      rsi_set_os_errno(status);
#endif
      return status;
    }

    // Change state to connected
    rsi_socket_pool[sockID].sock_state = RSI_SOCKET_STATE_CONNECTED;
  }

  // Find maximum limit based on the protocol
  if ((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_DGRAM) {
    // If it is a UDP socket
    maximum_limit = 1472;
  } else if (((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_STREAM)
             && ((rsi_socket_pool[sockID].sock_bitmap & RSI_SOCKET_FEAT_SSL)
                 || (rsi_socket_pool_non_rom[sockID].ssl_bitmap))) {
    // If it is a SSL socket/SSL websocket
    maximum_limit = rsi_socket_pool_non_rom[sockID].mss - RSI_SSL_HEADER_SIZE;
  } else if (((rsi_socket_pool[sockID].sock_type & 0xF) == SOCK_STREAM)
             && (rsi_socket_pool[sockID].sock_bitmap & RSI_SOCKET_FEAT_WEBS_SUPPORT)) {
    // If it is a websocket
    maximum_limit = 1450;
  } else {
    maximum_limit = rsi_socket_pool_non_rom[sockID].mss;
  }

  // Check maximum allowed length value
  if (msgLength > maximum_limit) {
    rsi_wlan_socket_set_status(RSI_ERROR_EMSGSIZE, sockID);
#ifdef RSI_WITH_OS
    rsi_set_os_errno(RSI_ERROR_EMSGSIZE);
#endif
    RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);
    return RSI_SOCK_ERROR;
  }
  if (sock_info->sock_bitmap & RSI_SOCKET_FEAT_TCP_ACK_INDICATION) {
    // Return if buffers are not available
    if (sock_info->current_available_buffer_count == 0) {
      // Set no buffers status and return
      rsi_wlan_socket_set_status(RSI_ERROR_ENOBUFS, sockID);
      // Release mutex lock
      RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);
#ifdef RSI_WITH_OS
      rsi_set_os_errno(RSI_ERROR_ENOBUFS);
#endif
      return RSI_SOCK_ERROR;
    }

    // Calculate buffers required for sending this length
    buffers_required = calculate_buffers_required(sock_info->sock_type, msgLength);

    if (buffers_required > sock_info->current_available_buffer_count) {
      // Calculate length can be sent with available buffers
      msgLength                                 = global_cb_p->rom_apis_p->ROM_WL_calculate_length_to_send(global_cb_p,
                                                                           sock_info->sock_type,
                                                                           sock_info->current_available_buffer_count);
      sock_info->current_available_buffer_count = 0;
    } else {
      // Subtract buffers required to send this packet
      if (sock_info->current_available_buffer_count > buffers_required) {
        sock_info->current_available_buffer_count -= buffers_required;
      } else {
        sock_info->current_available_buffer_count = 0;
      }
    }

    // Set expected response based on the callback
    if (data_transfer_complete_handler != NULL) {
#ifdef RSI_UART_INTERFACE
      rsi_driver_cb->wlan_cb->expected_response = RSI_WLAN_RSP_UART_DATA_ACK;
#else
      rsi_driver_cb->wlan_cb->expected_response = RSI_WLAN_RSP_ASYNCHRONOUS;
#endif
    } else {
      rsi_driver_cb->wlan_cb->expected_response = RSI_WLAN_RSP_TCP_ACK_INDICATION;
    }
  } else {
#ifdef RSI_UART_INTERFACE
    rsi_driver_cb->wlan_cb->expected_response = RSI_WLAN_RSP_UART_DATA_ACK;
#else
    rsi_driver_cb->wlan_cb->expected_response = RSI_WLAN_RSP_ASYNCHRONOUS;
#endif
  }

  // Send data send command
  status = rsi_driver_send_data(sockID, (uint8_t *)msg, msgLength, destAddr);

  // Release mutex lock
  RSI_MUTEX_UNLOCK(&rsi_driver_cb->wlan_cb->wlan_mutex);

  if (status == RSI_SUCCESS) {
    return msgLength;
  } else {
#ifdef RSI_WITH_OS
    status = rsi_get_error(sockID);
    rsi_set_os_errno(status);
#endif
    return RSI_SOCK_ERROR;
  }
}
/*==============================================*/
/**
 * @fn          void rsi_clear_sockets_non_rom(int32_t sockID)
 * @brief       Clear socket information
 * @param[in]   global_cb_p - pointer to the global control block
 * @param[in]   sockID      - socket descriptor
 * @return      void
 *
 *
 */
void rsi_clear_sockets_non_rom(int32_t sockID)
{
  int i;
  int32_t sock_id = -1;
  uint16_t port_number;
  int32_t status = RSI_SUCCESS;

  //  rsi_driver_cb_t   *rsi_driver_cb   = global_cb_p->rsi_driver_cb;
  rsi_socket_info_t *rsi_socket_pool = global_cb_p->rsi_socket_pool;

  if (sockID == RSI_CLEAR_ALL_SOCKETS) {
    for (i = 0; i < NUMBER_OF_SOCKETS; i++) {
      // Memset socket info
      memset(&rsi_socket_pool[i], 0, sizeof(rsi_socket_info_t));
    }
  } else if (rsi_socket_pool[sockID].sock_state == RSI_SOCKET_STATE_LISTEN) {
    port_number = rsi_socket_pool[sockID].source_port;

    for (i = 0; i < NUMBER_OF_SOCKETS; i++) {
      if ((rsi_socket_pool[i].source_port == port_number) && ((rsi_socket_pool[i].sock_type & 0xF) == SOCK_STREAM)) {
        // Memset socket info
        memset(&rsi_socket_pool[i], 0, sizeof(rsi_socket_info_t));
        rsi_wlan_socket_set_status(status, i);
        // Wait on select semaphore
        rsi_post_waiting_socket_semaphore(i);
      }
    }

  } else {
    if (sockID < 0 || sockID >= NUMBER_OF_SOCKETS) {
      return;
    }
    if ((rsi_socket_pool[sockID].ltcp_socket_type == RSI_LTCP_PRIMARY_SOCKET)) {
      if (rsi_socket_pool[sockID].backlog_current_count) {
        rsi_socket_pool[sockID].sock_state = RSI_SOCKET_STATE_LISTEN;
        rsi_socket_pool[sockID].backlog_current_count--;
      }
    } else if (rsi_socket_pool[sockID].ltcp_socket_type == RSI_LTCP_SECONDARY_SOCKET) {
      sock_id = rsi_get_primary_socket_id((uint8_t *)&rsi_socket_pool[sockID].source_port);
      if (sock_id >= 0) {
        // Decrease backlog current count
        rsi_socket_pool[sock_id].backlog_current_count--;
      }
      // Memset socket info
      memset(&rsi_socket_pool[sockID], 0, sizeof(rsi_socket_info_t));

    } else {
      // Memset socket info
      memset(&rsi_socket_pool[sockID], 0, sizeof(rsi_socket_info_t));
    }
  }
}
/** @} */

/** @addtogroup DRIVER10
* @{
*/
/*==============================================*/
/**
 * @fn          void rsi_post_waiting_socket_semaphore(int32_t sockID)
 * @brief       Set wlan status
 * @param[in]   status - status value to be set 
 * @return      void
 *
 */
#ifndef RSI_SOCK_SEM_BITMAP
void rsi_post_waiting_socket_semaphore(int32_t sockID)
{
  if (rsi_socket_pool_non_rom[sockID].socket_wait_bitmap & BIT(0)) {
    rsi_semaphore_post(&rsi_socket_pool_non_rom[sockID].socket_sem);
  }
  if (rsi_socket_pool_non_rom[sockID].socket_wait_bitmap & BIT(1)) {
    rsi_socket_pool_non_rom[sockID].recv_pending_bit |= BIT(1);
    rsi_semaphore_post(&rsi_socket_pool_non_rom[sockID].sock_recv_sem);
  }
  if (rsi_socket_pool_non_rom[sockID].socket_wait_bitmap & BIT(2)) {
    rsi_semaphore_post(&rsi_socket_pool_non_rom[sockID].sock_send_sem);
  }
  rsi_socket_pool_non_rom[sockID].socket_wait_bitmap = 0;
}
#else
void rsi_post_waiting_socket_semaphore(int32_t sockID)
{
  rsi_semaphore_post(&rsi_socket_pool_non_rom[sockID].socket_sem);
  rsi_semaphore_post(&rsi_socket_pool_non_rom[sockID].sock_recv_sem);
  rsi_semaphore_post(&rsi_socket_pool_non_rom[sockID].sock_send_sem);
}
#endif

#endif
/** @} */
