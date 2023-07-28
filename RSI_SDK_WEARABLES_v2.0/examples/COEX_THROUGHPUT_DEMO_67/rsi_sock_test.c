/********************************************************************/
/** \file rsi_sock_test.c
 *
 * \brief Test code to run HTTP/HTTPS download test using Silicon Labs
 *        implementation of BSD socket API
 *
 *   \par MODULE NAME:
 *       rsi_sock_test.c - Silabs sockets test
 *
 *   \par DESCRIPTION:
 *       This module exercises Silabs BSD socket API implementation
 *       by connecting to an HTTP/S server and downloading a test file
 *       with a known pattern. A few runtime knobs are provided via
 *       a few variables which control certain aspects of the test.
 *
 *
 * NOTICE:
 *   This code is intended for Slicon Labs internal use only. The code is
 *   considered Confidential Information under our NDA, and is not
 *   considered to be Feedback. This code may not be put into a
 *   public SDK.
 *
 *********************************************************************/

#include <rsi_common_app.h>
#if COEX_THROUGHPUT
#include "rsi_common_config.h"
#if (RSI_ENABLE_WLAN_TEST && !WLAN_THROUGHPUT_TEST)
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <rsi_driver.h>
#include "rsi_socket.h"
#include "rsi_wlan_config.h"
#include "rsi_wlan_common_config.h"
#include "rsi_wlan_non_rom.h"
#include "rsi_os.h"
#include "rsi_sock_test.h"

/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/
#define RSI_CHUNK_SIZE 1024

/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/

socktest_ctx_t socktest_ctx[SOCKTEST_INSTANCES_MAX];
// 192.168.165.78=NathanLaptop, 192.168.165.28=AlexLaptop
#if RX_DATA
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
int expected_bytes_to_send    = BYTES_TO_TRANSMIT;

/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/
extern rsi_task_handle_t http_socket_task_handle[SOCKTEST_INSTANCES_MAX];
extern uint32_t rsi_convert_4R_to_BIG_Endian_uint32(uint32_t *pw);
extern rsi_semaphore_handle_t wlan_app_sem;
extern bool rsi_ble_running, rsi_bt_running, rsi_prop_protocol_running, rsi_wlan_running, wlan_radio_initialized,
  powersave_cmd_given;
extern rsi_mutex_handle_t power_cmd_mutex;
extern volatile uint64_t num_bytes;
extern uint32_t t_start;
#if WLAN_SYNC_REQ
extern rsi_semaphore_handle_t sync_coex_ble_sem, sync_coex_prop_protocol_sem, sync_coex_bt_sem;
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

void perform_sock_test(void *instance_ctx)
{

  uint32_t i = 0, j = 0;
  uint32_t k = 0;
  uint32_t data_start_offs;
  int num_fds_ready;
  rsi_fd_set readfds;
  rsi_fd_set writefds;
  //uint32_t t_start=0;
  struct rsi_timeval timeout = { 0 };
  int32_t status             = 0;
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

  /* Clear most of the context, except for the error tracking members */
  memset(ctx, 0, (uint32_t)(void *)&ctx->num_successful_test - (uint32_t)(void *)ctx);
  sock_fd = -1;

  /* RS9116 uses bit 1 of the protocol to signal TLS socket */
#if (HIGH_PERFORMANCE_ENABLE && SOCKET_ASYNC_FEATURE)
  sock_fd = rsi_socket_async(AF_INET, SOCK_STREAM, 1, socket_async_recive);
#else
  sock_fd = rsi_socket(PF_INET, SOCK_STREAM, 0);
#endif
  if (sock_fd == -1) {
    /* Failed to create socket */
    LOG_PRINT("\r\n Socket Create failed");
    return;
  }
#if RX_DATA
  if ((ctx->threadid < TCP_RX_HIGH_PERFORMANCE_SOCKETS) && (high_performance_socket == 1)) {
    status = rsi_setsockopt(sock_fd, SOL_SOCKET, SO_HIGH_PERFORMANCE_SOCKET, NULL, 0);
    if (status != RSI_SUCCESS) {
      status = rsi_get_error(sock_fd);
      LOG_PRINT("\r\nSet Socket Opt Create failed, status= 0x%x", status);
      break_if((int)1);
    }
  }
#endif

  memset(&sa, 0x00, sizeof(sa));
  sa.sin_family = AF_INET;

  /* IP addr for server */
  sa.sin_addr.s_addr = ip_to_reverse_hex((char *)SERVER_IP_ADDRESS);

#if (RX_DATA && HTTPS_DOWNLOAD)
  /* Talk to port 443 required */
  sa.sin_port = htons(443);
#elif (RX_DATA && (!HTTPS_DOWNLOAD))
  /* Talk to port 80 required */
  sa.sin_port = htons(80);
#else
  sa.sin_port = htons(SERVER_PORT);
#endif

#if (RX_DATA && HTTPS_DOWNLOAD)
  status = rsi_setsockopt(sock_fd, SOL_SOCKET, SO_SSL_ENABLE, NULL, 0);
  if (status != RSI_SUCCESS) {
    status = rsi_get_error(sock_fd);
    LOG_PRINT("\r\nSet Socket Opt Create failed, status= 0X%x", status);
    break_if((int)1);
    goto ERROR_EXIT_send_socket_data;
  }
#endif
  status = rsi_connect(sock_fd, (struct rsi_sockaddr *)&sa, sizeof(struct rsi_sockaddr_in));
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Failed to connect to remote server, threadid:%d", ctx->threadid);
    status = rsi_wlan_socket_get_status(sock_fd);
    status = rsi_get_error(sock_fd);
    LOG_PRINT("\r\nSocket Connect failed, status= 0x%x", status);
    break_if((int)1);
    goto ERROR_EXIT_send_socket_data;
  }
#if (WLAN_SYNC_REQ && !WLAN_TRANSIENT_CASE)
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

//! suspend the task if WLAN connect only is configured
#if WLAN_CONNECT_ONLY
  rsi_task_suspend(NULL);
#endif

  t_start = rsi_hal_gettickcount();

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

#if TX_DATA

  bytes_sent = 0;
  while (bytes_sent != expected_bytes_to_send) {

    j = 0;
    for (k = 0; k < (RSI_CHUNK_SIZE) / 4; k++) {
      memcpy(http_buffer + j, &i, 4);
      j += 4;
      i++;
    }

    status = rsi_send(sock_fd, (int8_t *)http_buffer, RSI_CHUNK_SIZE, 0);
    /* Check for error */

    if (status < 0) {
      status = rsi_wlan_socket_get_status(sock_fd);
      status = rsi_get_error(sock_fd);
      LOG_PRINT(" Send data failed, error: %x \r\n", status);
      break_if((int)1);
      goto ERROR_EXIT_send_socket_data;
    }
    bytes_sent += RSI_CHUNK_SIZE;
  }
  break_if((int)1);
  goto ERROR_EXIT_send_socket_data;
#endif
#if RX_DATA
  /* Send first set of headers to server */
  bytes_sent = 0;
  while (bytes_sent != strlen(http_request_str_first)) {
    status = rsi_send(sock_fd,
                      (int8_t *)(http_request_str_first + bytes_sent),
                      strlen(http_request_str_first) - bytes_sent,
                      0);
    /* Check for error */
    if (status < 0) {
      status = rsi_get_error(sock_fd);
      LOG_PRINT("\r\nSend HTTP Request first failed, status = 0x%x", status);
      break_if((int)1);
      goto ERROR_EXIT_send_socket_data;
    }
    bytes_sent += status;
  }

  /* Send last set of headers to server */
#if !HTTPS_DOWNLOAD
  bytes_sent = 0;
  while (bytes_sent != strlen(http_request_str_end)) {

    status =
      rsi_send(sock_fd, (int8_t *)(http_request_str_end + bytes_sent), strlen(http_request_str_end) - bytes_sent, 0);
    /* Check for error */
    if (status < 0) {
      status = rsi_get_error(sock_fd);
      LOG_PRINT("\r\nSend HTTP Request end failed, status = 0x%x", status);
      break_if((int)1);
      goto ERROR_EXIT_send_socket_data;
    }
    bytes_sent += status;
  }
#endif
  /* Receive data from server */
  headers_received = 0;
  bytes_received   = 0;
#if (HTTPS_DOWNLOAD && RX_DATA)
  LOG_PRINT("HTTPS download started \r\n");
#elif ((!HTTPS_DOWNLOAD) && RX_DATA)
  LOG_PRINT("HTTP download started \r\n");
#endif
  num_bytes = 0;
  while (bytes_received != expected_bytes_to_receive) {
    //		if(ctx->bytes_received > 14919700)
    //		{
    //			LOG_PRINT("stop \r\n");
    //		}
    int bytes_read;
    /* Configure fd set */
    RSI_FD_ZERO(&readfds);
    RSI_FD_SET(sock_fd, &readfds);

    /* Configure timeout */
    even_odd++;
    timeout.tv_sec = (even_odd % 2) ? 10 : 0; /* alternate between 10 and 0 seconds */
    num_fds_ready  = rsi_select(sock_fd + 1, &readfds, NULL, NULL, &timeout, NULL);

    /* Check for error */
    if (num_fds_ready < 0) {
      break_if((int)1);
      num_fds_ready = 0;
      break;
    }

    /* Read data from socket */
    if (RSI_FD_ISSET(sock_fd, &readfds)) {
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
          //ctx->res = rsi_get_error(ctx->sock_fd);
          if (status == RSI_ERROR_ENOBUFS) {
            continue;
          }
        }
        bytes_read = 0;
        //TSK_suspend( 10 );
        rsi_os_task_delay(20);
      }
      break_if(k >= 3);
      if (k >= 3) {
        break;
      }

      if (http_buffer_bytes_saved) {
        bytes_read += http_buffer_bytes_saved;
        http_buffer_bytes_saved = 0;
      }

      /* Null terminate the http_buffer */
      http_buffer[bytes_read] = '\0';
    } else {
      /* Not ready to read, go back to select */
      continue;
    }

    /* Loop through looking for data*/
    if (!headers_received) {
      /* http_buffer has already been null terminated to assist with processing */
      char *tmp_ptr = strstr(http_buffer, "\r\n\r\n");

      /* Header separator found */
      if (tmp_ptr) {
        tmp_ptr += 4;
        headers_received = 1;
        bytes_read -= (tmp_ptr - http_buffer);
        data_start_offs = tmp_ptr - http_buffer;
      } else {

        /* Separator not found, save last 3 bytes to account for "\r\n\r" being at the end of the first msg and "\n" being at the start of the next */
        http_buffer_bytes_saved = 3;
        memmove(http_buffer, http_buffer + bytes_read - http_buffer_bytes_saved, http_buffer_bytes_saved);
      }
    } else {
      data_start_offs = 0;
    }

    if (headers_received) {
      break_if((int)(bytes_read < 0));
      break_if((int)((bytes_received + bytes_read) > expected_bytes_to_receive));
      bytes_received += bytes_read;
      bytes_read = 0;
    }
  }
  num_bytes = bytes_received;
#endif
ERROR_EXIT_send_socket_data:

  status = rsi_shutdown(sock_fd, 0);
  if (status != RSI_SUCCESS) {
    status = rsi_get_error(sock_fd);
    break_if((int)1);
  }

#if RX_DATA
  if (bytes_received != expected_bytes_to_receive) {
    ctx->num_failed_test++;
  } else {
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
#endif
