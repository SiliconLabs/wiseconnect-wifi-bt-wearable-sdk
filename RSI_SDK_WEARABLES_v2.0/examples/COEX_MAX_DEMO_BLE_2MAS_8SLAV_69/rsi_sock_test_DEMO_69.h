/********************************************************************/
/** \file rsi_sock_test_DEMO_57.h
*
* \brief Test code to run HTTP/HTTPS download test using Silicon Labs
*        implementation of BSD socket API
*
*   \par MODULE NAME:
*       rsi_sock_test.h - Silicon Labs sockets test header
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

#ifndef RSI_SOCK_TEST
#define RSI_SOCK_TEST
#include <rsi_common_app.h>
#include <rsi_socket.h>

/*DEFINES*/
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif
void init_sock_test(void);

void perform_sock_test(void *instance_ctx);
#if (SSL_TX_DATA || SSL_RX_DATA)
#define http_buffer_sz (1370)
#else
#define http_buffer_sz (1500)
#endif
#define ERRORTRACK_SIZE (17)

struct socktest_ctx_struct {
  int threadid;
  rsi_semaphore_handle_t http_soc_wait_sem;
  int num_successful_test;
  int num_failed_test;
};
typedef struct socktest_ctx_struct socktest_ctx_t;

struct socket_window_memory_ctx_struct {
  uint32_t Max_TCP_Window;
  uint32_t Min_TCP_Window;
  uint32_t Avaiable_TCP_Window;
  uint32_t Max_Window_threshold;
  uint32_t socket_window[SOCKTEST_INSTANCES_MAX];
};
typedef struct socket_window_memory_ctx_struct socket_window_memory_ctx_t;
struct cert_bypass_struct {
  uint32_t cert_valid;
  uint32_t sock_id;
};
typedef struct cert_bypass_struct cert_bypass_struct_t;
#define MIN_TCP_WINDOW_SIZE 4380
#endif
