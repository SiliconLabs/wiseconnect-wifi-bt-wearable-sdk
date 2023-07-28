/********************************************************************/
/** \file rsi_sock_test.c
 *
 * \brief Test code to run HTTP/HTTPS download test using Silicon Labs
 *        implementation of BSD socket API
 *
 *   \par MODULE NAME:
 *       rsi_sock_test.c - Silicon Labs sockets test
 *
 *   \par DESCRIPTION:
 *       This module exercises Silicon Labs BSD socket API implementation
 *       by connecting to an HTTP/S server and downloading a test file
 *       with a known pattern. A few runtime knobs are provided via
 *       a few variables which control certain aspects of the test.
 *
 * NOTICE:
 *   This code is intended for Silicon Labs internal use only. The code is
 *   considered Confidential Information under our NDA, and is not
 *   considered to be Feedback. This code may not be put into a
 *   public SDK.
 *
 *********************************************************************/
#include <rsi_common_app.h>
#if COEX_MAX_APP
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "rsi_socket.h"
#include "rsi_wlan_config_DEMO_57.h"
#include "rsi_wlan_common_config.h"
#include "rsi_wlan_non_rom.h"
#include "rsi_os.h"
#include "rsi_common_app_DEMO_57.h"
#include <rsi_sock_test_DEMO_57.h>

#if RSI_TCP_IP_BYPASS
#include "lwip/netif.h"
#include "netif/ethernet.h"
#include "lwip/etharp.h"
#include "lwip/apps/lwiperf.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "sys_arch.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#endif /* RSI_TCP_IP_BYPASS */
/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/
#if (SSL_TX_DATA || SSL_RX_DATA)
extern void compute_throughput();
#define RSI_CHUNK_SIZE 1370
#else
#define RSI_CHUNK_SIZE 1024
#endif

#if SSL_RX_DATA
extern volatile uint8_t data_recvd;
#endif
/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/
socktest_ctx_t socktest_ctx[SOCKTEST_INSTANCES_MAX];

#if (!SSL_RX_DATA && RX_DATA)
#if HTTPS_DOWNLOAD
const char http_request_str_first[] = "GET /" DOWNLOAD_FILENAME " HTTPS/1.1\r\n"
                                      "Host: " SERVER_IP_ADDRESS "\r\n"
                                      "User-Agent: sock_test\r\n"
                                      "Accept: text/plain\r\n"
                                      "Accept-Language: en-US,en;q=0.5\r\n"
                                      "Accept-Encoding: identity\r\n";
#else
const char http_request_str_first[] = "GET " DOWNLOAD_FILENAME " HTTP/1.1\r\n"
                                      "Host: " SERVER_IP_ADDRESS "\r\n"
                                      "User-Agent: sock_test\r\n"
                                      "Accept: text/plain\r\n"
                                      "Accept-Language: en-US,en;q=0.5\r\n"
                                      "Accept-Encoding: identity\r\n";
#endif
#endif
const char http_request_str_connection_close[] = "Connection: close\r\n";
const char http_request_str_end[]              = "\r\n";

int expected_bytes_to_receive = BYTES_TO_RECEIVE;
#if SSL_TX_DATA
int expected_bytes_to_send = (MAX_TX_PKTS * RSI_CHUNK_SIZE);
#else
int expected_bytes_to_send = BYTES_TO_TRANSMIT;
#endif
/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/
extern rsi_task_handle_t http_socket_task_handle[SOCKTEST_INSTANCES_MAX];
extern uint32_t rsi_convert_4R_to_BIG_Endian_uint32(uint32_t *pw);
extern rsi_semaphore_handle_t wlan_app_sem;
#if SOCKET_ASYNC_FEATURE
extern rsi_semaphore_handle_t sock_wait_sem;
#endif
#if (RSI_TCP_IP_BYPASS && SOCKET_ASYNC_FEATURE)
extern rsi_semaphore_handle_t lwip_sock_async_sem;
#endif
#if (WLAN_SYNC_REQ)
extern bool rsi_bt_running;
#endif
extern bool rsi_ble_running, rsi_prop_protocol_running;

extern volatile uint32_t num_bytes;
extern uint32_t t_start, t_end;

#if WLAN_SYNC_REQ
extern rsi_semaphore_handle_t sync_coex_ble_sem, sync_coex_prop_protocol_sem, sync_coex_bt_sem;
#endif
#if (SSL_TX_DATA || SSL_RX_DATA || (RX_DATA && HTTPS_DOWNLOAD))
extern rsi_semaphore_handle_t cert_sem, conn_sem;
extern rsi_task_handle_t cert_bypass_task_handle[SOCKTEST_INSTANCES_MAX];
extern cert_bypass_struct_t rsi_cert_bypass[SOCKTEST_INSTANCES_MAX];
#endif
/*=======================================================================*/
//   ! EXTERN FUNCTIONS
/*=======================================================================*/
extern void rsi_task_destroy(rsi_task_handle_t *task_handle);
#if SOCKET_ASYNC_FEATURE
extern void socket_async_recive(uint32_t sock_no, uint8_t *buffer, uint32_t length);
#endif

/*=======================================================================*/
//   ! PROCEDURES
/*=======================================================================*/
void init_sock_test(void);
void perform_sock_test(void *instance_ctx);

/*=======================================================================*/
//   ! VARIABLES
/*=======================================================================*/

/*=======================================================================*/
//!  CALLBACK FUNCTIONS
/*=======================================================================*/

static void break_if(int should_break)
{
  if (should_break) {
    should_break = FALSE;
  }
}

void init_sock_test(void)
{
  memset(socktest_ctx, 0, sizeof(socktest_ctx));
}

#if (SSL_TX_DATA || SSL_RX_DATA || (RX_DATA && HTTPS_DOWNLOAD))
uint8_t cert_buffer[8192];
void certificate_response_handler(uint16_t status, uint8_t *buffer, const uint32_t length)
{
  rsi_cert_recv_t *cert_recev     = (rsi_cert_recv_t *)buffer;
  uint32_t sockID                 = 0;
  sockID                          = rsi_get_socket_descriptor(cert_recev->src_port,
                                     cert_recev->dst_port,
                                     cert_recev->ip_address.ipv6_address,
                                     rsi_bytes2R_to_uint16(cert_recev->ip_version),
                                     rsi_bytes2R_to_uint16(cert_recev->sock_desc));
  rsi_cert_bypass[sockID].sock_id = rsi_bytes2R_to_uint16(cert_recev->sock_desc);
  //! User needs to copy the certificate into the certificate buffer
  if (status != RSI_SUCCESS) {
    LOG_PRINT("error status 0x%x", status);
    while (1)
      ;
  } else {
    if (cert_recev->more_chunks == 0) {
      rsi_cert_bypass[sockID].cert_valid = 1;
      //! Complete certificate received
      rsi_semaphore_post(&cert_sem);
    }
  }

  return;
}
void rsi_app_task_send_certificates()
{

  int32_t status = RSI_SUCCESS;
  int i          = 0;
  while (1) {
    ////////////////////////////////////////
    //! User certificate validation code ///
    ////////////////////////////////////////
    /* User need to take care of validating the received certificate
					 and then only should send the certificate valid reponse to the module */
    rsi_semaphore_wait(&cert_sem, 0);
    for (i = 0; i < SOCKTEST_INSTANCES_MAX; i++) {
      if (rsi_cert_bypass[i].cert_valid) {
        rsi_cert_bypass[i].cert_valid = 0;
        //! Send certificate valid response
        status = rsi_certificate_valid(1, rsi_cert_bypass[i].sock_id);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("SSL certificate valid send failed\n");
          //return status;
        }
        break;
      } else {
        continue;
      }
    }
    rsi_task_destroy(NULL);
  }
}
#endif
#if (RSI_TCP_IP_BYPASS && SOCKET_ASYNC_FEATURE)
uint8_t started               = 0;
struct tcp_pcb *connected_pcb = NULL;
static err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  /* close socket if the peer has sent the FIN packet  */
  if (p == NULL) {
    tcp_close(tpcb);
    return ERR_OK;
  }
  if (!started) {
    t_start = rsi_hal_gettickcount();
    started = 1;
  }
  num_bytes += p->tot_len;
  /* all we do is say we've received the packet */
  /* we don't actually make use of it */

  tcp_recved(tpcb, p->tot_len);

  pbuf_free(p);
  return ERR_OK;
}

uint32_t tcp_client_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  LOG_PRINT("Connected to iperf server\r\n");
  /* store state */
  connected_pcb = tpcb;
  if (err == ERR_OK) {
    /* set callback values & functions */
    tcp_arg(tpcb, NULL);
    //! register call_back for asynchronous data transfer
    tcp_recv(tpcb, tcp_recv_callback);
  }
  //! posting semaphore
  rsi_semaphore_post(&lwip_sock_async_sem);
  return ERR_OK;
}

#endif

void perform_sock_test(void *instance_ctx)
{

  int32_t status = 0;
  uint32_t k     = 0;
#if RSI_TCP_IP_BYPASS
  struct sockaddr_in addr;
#if SOCKET_ASYNC_FEATURE
  struct tcp_pcb *pcb;
  err_t err = 0;
#endif
#endif
#if TX_DATA
  uint32_t i = 0, j = 0;
#endif
  int32_t num_fds_ready;
  rsi_fd_set readfds;
  struct rsi_timeval timeout = { 0 };
  socktest_ctx_t *ctx        = instance_ctx;
  int32_t sock_fd;
  uint8_t high_performance_socket = 0;
  struct rsi_sockaddr_in sa;
  char http_buffer[http_buffer_sz + 1];
  int32_t bytes_sent              = 0;
  int32_t bytes_received          = 0;
  int32_t headers_received        = 0;
  int32_t even_odd                = 0;
  int32_t selrdfd_ret_cnt         = 0;
  int32_t http_buffer_bytes_saved = 0;

  LOG_PRINT("Thread %d started \r\n", ctx->threadid);

  rsi_semaphore_wait(&ctx->http_soc_wait_sem, 0);
  sock_fd = -1;

  /* RS9116 uses bit 1 of the protocol to signal TLS socket */
#if (HIGH_PERFORMANCE_ENABLE && SOCKET_ASYNC_FEATURE)
  sock_fd = rsi_socket_async(AF_INET, SOCK_STREAM, 0, socket_async_recive);
#else
#if RSI_TCP_IP_BYPASS
#if SOCKET_ASYNC_FEATURE
  /* create new TCP PCB structure */
  pcb = tcp_new();

#else
  //! LWIP socker creation.
  sock_fd = lwip_socket(AF_INET, SOCK_STREAM, 0);
#endif
#else  /* RSI_TCP_IP_BYPASS */
  sock_fd = rsi_socket(PF_INET, SOCK_STREAM, 0);
#endif /* RSI_TCP_IP_BYPASS */
#endif
#if (RSI_TCP_IP_BYPASS && SOCKET_ASYNC_FEATURE)
  if (!pcb) {
    printf("Error socket creating PCB. Out of Memory\r\n");
    return;
  }
#else
  if (sock_fd == -1) {
    /* Failed to create socket */
    printf("Failed to create socket in thread %d\r\n", ctx->threadid);
    return;
  }
#endif
#if WLAN_THROUGHPUT_ENABLE
  high_performance_socket = 1;
#endif
#if (RX_DATA && (!RSI_TCP_IP_BYPASS))
  if ((ctx->threadid < TCP_RX_HIGH_PERFORMANCE_SOCKETS) && (high_performance_socket == 1)) {
    status = rsi_setsockopt(sock_fd, SOL_SOCKET, SO_HIGH_PERFORMANCE_SOCKET, NULL, 0);
    if (status != 0) {
      status = rsi_get_error(sock_fd);
      break_if((int)1);
    }
  }
#endif

#if RSI_TCP_IP_BYPASS
  /* set up address to connect to */
  memset(&addr, 0, sizeof(addr));
  addr.sin_len    = sizeof(addr);
  addr.sin_family = AF_INET;
#if (TX_DATA || SOCKET_ASYNC_FEATURE)
  addr.sin_port = htons(SERVER_PORT);
#else
  addr.sin_port = htons(80);
#endif
  addr.sin_addr.s_addr = ip_to_reverse_hex((char *)SERVER_IP_ADDRESS);
#else /* RSI_TCP_IP_BYPASS */
  memset(&sa, 0x00, sizeof(sa));
  sa.sin_family = AF_INET;

  /* IP addr for server */
  sa.sin_addr.s_addr = ip_to_reverse_hex((char *)SERVER_IP_ADDRESS);

#if ((!SSL_RX_DATA) && RX_DATA && HTTPS_DOWNLOAD)
  /* Talk to port 443 required */
  sa.sin_port = htons(443);
#elif ((!SSL_RX_DATA) && RX_DATA && (!HTTPS_DOWNLOAD))
  /* Talk to port 80 required */
  sa.sin_port = htons(80);
#else
  sa.sin_port = htons(SERVER_PORT + sock_fd);
#endif
#endif /* RSI_TCP_IP_BYPASS */

#if (SSL_TX_DATA || SSL_RX_DATA || (RX_DATA && HTTPS_DOWNLOAD))
  rsi_semaphore_wait(&conn_sem, 0);
  status = rsi_setsockopt(sock_fd, SOL_SOCKET, SO_SSL_ENABLE, NULL, 0);
  if (status != 0) {
    status = rsi_get_error(sock_fd);
    break_if((int)1);
    goto ERROR_EXIT_send_socket_data;
  }
  status = rsi_setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, cert_buffer, sizeof(cert_buffer));
  if (status != 0) {
    status = rsi_get_error(sock_fd);
    break_if((int)1);
    goto ERROR_EXIT_send_socket_data;
  }
#endif

#if RSI_TCP_IP_BYPASS
  /* connect to iperf server */
#if SOCKET_ASYNC_FEATURE
  err = tcp_connect(pcb, &addr.sin_addr.s_addr, SERVER_PORT, tcp_client_connected_callback);
#else
  status        = lwip_connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr));
#endif
#else  /* RSI_TCP_IP_BYPASS */
  status      = rsi_connect(sock_fd, (struct rsi_sockaddr *)&sa, sizeof(struct rsi_sockaddr_in));
#endif /* RSI_TCP_IP_BYPASS */

  if (status != 0) {
#if (!RSI_TCP_IP_BYPASS)
    status = rsi_wlan_socket_get_status(sock_fd);
    status = rsi_get_error(sock_fd);
#endif /* RSI_TCP_IP_BYPASS */
    printf("\n Socket connection failed :0x%x\n", status);
    break_if((int)1);
    goto ERROR_EXIT_send_socket_data;
  }

#if (SOCKET_ASYNC_FEATURE && RSI_TCP_IP_BYPASS)
  //! waiting until connected or response for connect
  rsi_semaphore_wait(&lwip_sock_async_sem, 0);
  rsi_semaphore_wait(&sock_wait_sem, 0);
#endif

#if (SSL_TX_DATA || SSL_RX_DATA || (RX_DATA && HTTPS_DOWNLOAD))
  rsi_semaphore_post(&conn_sem);
#endif

#if (WLAN_SYNC_REQ)
  //! unblock other protocol activities
  if (rsi_bt_running) {
    rsi_semaphore_post(&sync_coex_bt_sem);
  }
  if (rsi_ble_running) {
    rsi_semaphore_post(&sync_coex_ble_sem);
  }
  if (rsi_prop_protocol_running) {
    rsi_semaphore_post(&sync_coex_prop_protocol_sem);
  }
#endif

#if (!RSI_TCP_IP_BYPASS)
  t_start = rsi_hal_gettickcount();
#endif
  /***************************************************
TODO:
Basic wait loop to wait for the socket to connect to the server.
This loop requires that the server is available and we can connect to it.
Proper handling should be made using the rsi_wlan_rsp_socket_create callback notification.
	 ***************************************************/

#if (!(THROUGHPUT_EN && HIGH_PERFORMANCE_ENABLE))
  while (rsi_socket_pool[sock_fd].sock_state == 1) {
    //TSK_suspend(10);
    rsi_os_task_delay(10);
  };

#if SSL_TX_DATA
  // Memset buffer
  memset(&http_buffer, sock_fd, sizeof(http_buffer));
  num_bytes = 0;
#endif

#if TX_DATA

  bytes_sent = 0;
  while (bytes_sent != expected_bytes_to_send) {
#if (!SSL_TX_DATA)
    j = 0;
    for (k = 0; k < (RSI_CHUNK_SIZE) / 4; k++) {
      memcpy(http_buffer + j, &i, 4);
      j += 4;
      i++;
    }
#endif
#if RSI_TCP_IP_BYPASS
    status = lwip_write(sock_fd, (int8_t *)http_buffer, RSI_CHUNK_SIZE);
#else
    status = rsi_send(sock_fd, (int8_t *)http_buffer, RSI_CHUNK_SIZE, 0);
    /* Check for error */
#endif
    if (status < 0) {
#if (!RSI_TCP_IP_BYPASS)
      status = rsi_wlan_socket_get_status(sock_fd);
      LOG_PRINT("Error in Sending data %x \r\n", status);
      status = rsi_get_error(sock_fd);
#else
      LOG_PRINT("Error %x \r\n", status);
#endif
      break_if((int)1);
      goto ERROR_EXIT_send_socket_data;
    }
    bytes_sent += RSI_CHUNK_SIZE;
  }
#if SSL_TX_DATA
  t_end     = rsi_hal_gettickcount();
  num_bytes = bytes_sent;
  compute_throughput();
#endif
  break_if((int)1);
  goto ERROR_EXIT_send_socket_data;
#endif

#if RX_DATA
#if (!SSL_RX_DATA)
  /* Send first set of headers to server */
  bytes_sent = 0;
  while (bytes_sent != strlen(http_request_str_first)) {

#if RSI_TCP_IP_BYPASS
    status =
      lwip_write(sock_fd, (int8_t *)(http_request_str_first + bytes_sent), strlen(http_request_str_first) - bytes_sent);
#else
    status = rsi_send(sock_fd,
                      (int8_t *)(http_request_str_first + bytes_sent),
                      strlen(http_request_str_first) - bytes_sent,
                      0);
#endif
    /* Check for error */
    if (status < 0) {
#if (!RSI_TCP_IP_BYPASS)
      status = rsi_get_error(sock_fd);
#endif
      break_if((int)1);
      goto ERROR_EXIT_send_socket_data;
    }
    bytes_sent += status;
  }

  /* Send last set of headers to server */
#if !HTTPS_DOWNLOAD
  bytes_sent = 0;
  while (bytes_sent != strlen(http_request_str_end)) {
#if RSI_TCP_IP_BYPASS
    status =
      lwip_write(sock_fd, (int8_t *)(http_request_str_end + bytes_sent), strlen(http_request_str_end) - bytes_sent);
#else
    status =
      rsi_send(sock_fd, (int8_t *)(http_request_str_end + bytes_sent), strlen(http_request_str_end) - bytes_sent, 0);
#endif
    /* Check for error */
    if (status < 0) {
#if (!RSI_TCP_IP_BYPASS)
      status = rsi_get_error(sock_fd);
#endif
      break_if((int)1);
      goto ERROR_EXIT_send_socket_data;
    }
    bytes_sent += status;
  }
#endif /* !HTTPS_DOWNLOAD */

#endif /*(!SSL_RX_DATA)*/

  /* Receive data from server */
  headers_received = 0;
  bytes_received   = 0;

#if (!SSL_RX_DATA)
#if (HTTPS_DOWNLOAD && RX_DATA)
  LOG_PRINT("HTTPS download started \r\n");
#elif ((!HTTPS_DOWNLOAD) && RX_DATA)
  LOG_PRINT("HTTP download started \r\n");
#endif
#endif /*(!SSL_RX_DATA)*/

#if RSI_TCP_IP_BYPASS
  t_start = rsi_hal_gettickcount();
#endif

#if !SOCKET_ASYNC_FEATURE
  num_bytes = 0;
#if SSL_RX_DATA
  while (!data_recvd) {
#else
  while (bytes_received != expected_bytes_to_receive) {
#endif
    int bytes_read;
#if (!RSI_TCP_IP_BYPASS)
    /* Configure fd set */
    RSI_FD_ZERO(&readfds);
    RSI_FD_SET(sock_fd, &readfds);

    /* Configure timeout */
    even_odd++;
    timeout.tv_sec = (even_odd % 2) ? 10 : 0; /* alternate between 10 and 0 seconds */
    num_fds_ready  = rsi_select(sock_fd + 1, &readfds, NULL, NULL, &timeout, NULL);
    selrdfd_ret_cnt++;

    /* Check for error */
    if (num_fds_ready < 0) {
      break_if((int)1);
      num_fds_ready = 0;
      break;
    }

    /* Read data from socket */
    if (RSI_FD_ISSET(sock_fd, &readfds)) {
#endif /* !RSI_TCP_IP_BYPASS */

#if RSI_TCP_IP_BYPASS
      bytes_read = lwip_read(sock_fd, http_buffer + http_buffer_bytes_saved, http_buffer_sz - http_buffer_bytes_saved);

      if (bytes_read < 0) {
        bytes_read = 0;
        rsi_os_task_delay(20);
      }
#else  /* RSI_TCP_IP_BYPASS */
    for (k = 0; k < 3; k++) {
      bytes_read =
        rsi_recv(sock_fd, http_buffer + http_buffer_bytes_saved, http_buffer_sz - http_buffer_bytes_saved, 0);
      if (bytes_read >= 0 && bytes_read <= http_buffer_sz) {
        if (k > 0) {
        } /* Track recoveries after error. */
        break;
      }
      if (bytes_read < 0) {
        status = rsi_wlan_socket_get_status(sock_fd);
        if (status == RSI_ERROR_ENOBUFS) {
          continue;
        }
      }
      bytes_read = 0;
      //TSK_suspend( 10 );
      rsi_os_task_delay(20);
    }
    num_bytes += bytes_read;
    break_if(k >= 3);
    if (k >= 3) {
      break;
    }
#endif /* RSI_TCP_IP_BYPASS */

      if (http_buffer_bytes_saved) {
        bytes_read += http_buffer_bytes_saved;
        http_buffer_bytes_saved = 0;
      }

      /* Null terminate the http_buffer */
      http_buffer[bytes_read] = '\0';
#if (!RSI_TCP_IP_BYPASS)
    } else {
      /* Not ready to read, go back to select */
      continue;
    }
#endif /* !RSI_TCP_IP_BYPASS */

#if (!SSL_RX_DATA)
    /* Loop through looking for data*/
    if (!headers_received) {
      /* http_buffer has already been null terminated to assist with processing */
      char *tmp_ptr = strstr(http_buffer, "\r\n\r\n");

      /* Header separator found */
      if (tmp_ptr) {
        tmp_ptr += 4;
        headers_received = 1;
        bytes_read -= (tmp_ptr - http_buffer);
        //This statement is added to resolve the warning: [-unused-but-set-variable],so declared the variable in a macro in which the variable is used.
      } else {

        /* Separator not found, save last 3 bytes to account for "\r\n\r" being at the end of the first msg and "\n" being at the start of the next */
        http_buffer_bytes_saved = 3;
        memmove(http_buffer, http_buffer + bytes_read - http_buffer_bytes_saved, http_buffer_bytes_saved);
      }
    } else {
    }

    if (headers_received) {
      break_if((int)(bytes_read < 0));
      break_if((int)((bytes_received + bytes_read) > expected_bytes_to_receive));
      bytes_received += bytes_read;
      bytes_read = 0;
    }
#endif
  } /* SSL_RX_DATA*/

#if (!SSL_RX_DATA)
  num_bytes = bytes_received;
#endif
#endif /*!SOCKET_ASYNC_FEATURE*/

#endif /* RX_DATA */

#endif /* (!(THROUGHPUT_EN && HIGH_PERFORMANCE_ENABLE)) */

#if SOCKET_ASYNC_FEATURE
  rsi_semaphore_wait(&sock_wait_sem, 0);
#endif

#if SSL_RX_DATA
  data_recvd = 0;
  compute_throughput();
#endif
ERROR_EXIT_send_socket_data:

#if RSI_TCP_IP_BYPASS
  /* close the LWIP socket */
  status = lwip_close(sock_fd);
#else  /* RSI_TCP_IP_BYPASS */
  status      = rsi_shutdown(sock_fd, 0);
#endif /* RSI_TCP_IP_BYPASS */

  if (status != 0) {
#if (!RSI_TCP_IP_BYPASS)
    status = rsi_get_error(sock_fd);
#endif /* RSI_TCP_IP_BYPASS */
    break_if((int)1);
  }
#if (!SSL_RX_DATA && !SOCKET_ASYNC_FEATURE)
#if RX_DATA
  if (bytes_received != expected_bytes_to_receive) {
    ctx->num_failed_test++;
  } else {
#if RSI_TCP_IP_BYPASS
    t_end = rsi_hal_gettickcount();
#endif /* RSI_TCP_IP_BYPASS */
    ctx->num_successful_test++;
  }

  break_if((int)(bytes_received != expected_bytes_to_receive));

#else
  if (bytes_sent != expected_bytes_to_send) {
    ctx->num_failed_test++;
  } else {
    ctx->num_successful_test++;
  }
#endif
#endif
  LOG_PRINT("Thread %d completed \r\n", ctx->threadid);
  http_socket_task_handle[ctx->threadid] = NULL;
  rsi_semaphore_destroy(&ctx->http_soc_wait_sem);
  rsi_semaphore_post(&wlan_app_sem);
  rsi_task_destroy(NULL);
}
#endif
