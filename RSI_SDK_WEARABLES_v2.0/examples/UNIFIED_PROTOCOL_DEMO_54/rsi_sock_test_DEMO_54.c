/********************************************************************/
/** \file rsi_sock_test_DEMO_54.c
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
 *   This code is intended for Silabs internal use only. The code is
 *   considered Confidential Information under our NDA, and is not
 *   considered to be Feedback. This code may not be put into a
 *   public SDK.
 *
 *********************************************************************/
#include <rsi_common_app.h>
#if (UNIFIED_PROTOCOL && (MULTITHREADED_HTTP_DOWNLOAD_TEST || MULTITHREADED_TCP_TX_TEST))
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <rsi_sock_test_DEMO_54.h>
#include "rsi_socket.h"
#include "rsi_wlan_config_DEMO_54.h"
#include "rsi_wlan_common_config.h"
#include "rsi_wlan_non_rom.h"
#include "rsi_os.h"

#define BIG_ENDIAN_CONVERSION_REQUIRED 1
extern rsi_task_handle_t http_socket_task_handle[SOCKTEST_INSTANCES_MAX];
extern rsi_semaphore_handle_t suspend_sem;
extern volatile uint64_t num_bytes;
extern uint32_t rsi_convert_4R_to_BIG_Endian_uint32(uint32_t *pw);
socktest_ctx_t socktest_ctx[SOCKTEST_INSTANCES_MAX];

// 192.168.165.78=NathanLaptop, 192.168.165.28=AlexLaptop
const char http_request_str_first[] = "GET dltestdata32.txt HTTP/1.1\r\n"
                                      "Host: " SERVER_IP_ADDRESS "\r\n"
                                      "User-Agent: sock_test\r\n"
                                      "Accept: text/plain\r\n"
                                      "Accept-Language: en-US,en;q=0.5\r\n"
                                      "Accept-Encoding: identity\r\n";
const char http_request_str_connection_close[] = "Connection: close\r\n";
const char http_request_str_end[]              = "\r\n";

int expected_bytes_to_receive = BYTES_TO_RECEIVE;
int expected_bytes_to_send    = BYTES_TO_TRANSMIT;

#if MULTITHREADED_TCP_TX_TEST
#define SERVER_PORT    5001
#define RSI_CHUNK_SIZE 1024
#endif

extern void rsi_task_destroy(rsi_task_handle_t *task_handle);

void init_sock_test(void);
void perform_sock_test(void *instance_ctx);

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

#if MULTITHREADED_TCP_TX_TEST
  uint32_t i = 0, j = 0;
#endif
  uint32_t k = 0;
  uint32_t data_start_offs;
  int num_fds_ready;
  rsi_fd_set readfds;
  rsi_fd_set writefds;
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

  LOG_PRINT("\n Thread %d started", ctx->threadid);

  rsi_semaphore_wait(&ctx->http_soc_wait_sem, 0);

  /* Clear most of the context, except for the error tracking members */
  memset(ctx, 0, (uint32_t)(void *)&ctx->num_successful_test - (uint32_t)(void *)ctx);
  sock_fd = -1;

  /* RS9116 uses bit 1 of the protocol to signal TLS socket */
  sock_fd = rsi_socket(PF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    /* Failed to create socket */
    LOG_PRINT("\r\nSocket Create failed, status= 0x%x", status);
    return;
  }
  LOG_PRINT("\r\nSocket Create Success");
#if !MULTITHREADED_TCP_TX_TEST
  if ((ctx->threadid < TCP_RX_HIGH_PERFORMANCE_SOCKETS) && (high_performance_socket == 1)) {
    status = rsi_setsockopt(sock_fd, SOL_SOCKET, SO_HIGH_PERFORMANCE_SOCKET, NULL, 0);
    if (status != 0) {
      status = rsi_get_error(sock_fd);
      LOG_PRINT("\r\nSet Socket Opt Create failed, status= 0x%x", status);
      break_if((int)1);
    }
  }
#endif

  memset(&sa, 0x00, sizeof(sa));
  sa.sin_family = AF_INET;

  /* IP addr for server */
  //sa.sin_addr.s_addr = htonl(HTTPSVR_ADDR); // test server bsd apis
  sa.sin_addr.s_addr = ip_to_reverse_hex((char *)SERVER_IP_ADDRESS);
  /* Talk to port 80 required */
  sa.sin_port = htons(80);
#if MULTITHREADED_TCP_TX_TEST
  sa.sin_port = htons(SERVER_PORT);
#endif
  status = rsi_connect(sock_fd, (struct rsi_sockaddr *)&sa, sizeof(struct rsi_sockaddr_in));
  if (status != 0) {
    status = rsi_get_error(sock_fd);
    LOG_PRINT("\r\nSocket Connect failed, status= 0x%x", status);
    break_if((int)1);
    goto ERROR_EXIT_send_socket_data;
  }
  LOG_PRINT("\r\nSocket Connect Success");
  /***************************************************
TODO:
Basic wait loop to wait for the socket to connect to the server.
This loop requires that the server is available and we can connect to it.
Proper handling should be made using the rsi_wlan_rsp_socket_create callback notification.
	 ***************************************************/
  while (rsi_socket_pool[sock_fd].sock_state == 1) {
    //TSK_suspend(10);
    rsi_os_task_delay(10);
  };

#if MULTITHREADED_TCP_TX_TEST
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
      status = rsi_get_error(sock_fd);
      LOG_PRINT("\r\nSend failed, status= 0x%x", status);
      break_if((int)1);
      goto ERROR_EXIT_send_socket_data;
    }
    bytes_sent += RSI_CHUNK_SIZE;
  }
  break_if((int)1);
  goto ERROR_EXIT_send_socket_data;
#endif
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

  /* Receive data from server */
  headers_received = 0;
  bytes_received   = 0;

  LOG_PRINT("HTTP download started \r\n");

  while (bytes_received != expected_bytes_to_receive) {

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
          status = rsi_get_error(sock_fd);
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

ERROR_EXIT_send_socket_data:

  status = rsi_shutdown(sock_fd, 0);
  if (status != 0) {
    status = rsi_get_error(sock_fd);
    break_if((int)1);
  }

#if !MULTITHREADED_TCP_TX_TEST
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
  LOG_PRINT("\n Thread %d completed", ctx->threadid);
  http_socket_task_handle[ctx->threadid] = NULL;
  rsi_semaphore_destroy(&ctx->http_soc_wait_sem);
  rsi_semaphore_post(&suspend_sem);
  rsi_task_destroy(NULL);
}
#endif
