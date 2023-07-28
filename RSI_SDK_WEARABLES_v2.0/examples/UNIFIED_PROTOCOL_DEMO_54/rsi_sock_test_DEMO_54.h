/********************************************************************/
/** \file rsi_sock_test_DEMO_54
*
* \brief Test code to run HTTP/HTTPS download test using Silicon Labs
*        implementation of BSD socket API
*
*   \par MODULE NAME:
*       rsi_sock_test.h - Silabs sockets test header
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

#ifndef RSI_SOCK_TEST
#define RSI_SOCK_TEST
#include <rsi_common_app.h>
#if (UNIFIED_PROTOCOL && (MULTITHREADED_HTTP_DOWNLOAD_TEST || MULTITHREADED_TCP_TX_TEST))
#include <rsi_socket.h>
/*DEFINES*/
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif
#define SOCKTEST_INSTANCES_MAX (1)
#define NO_OF_ITERATIONS       (10)
void init_sock_test(void);

void perform_sock_test(void *instance_ctx);

#define http_buffer_sz    (1500)
#define ERRORTRACK_SIZE   (17)
#define BYTES_TO_RECEIVE  1048576
#define BYTES_TO_TRANSMIT 1048576
struct socktest_ctx_struct {
  int threadid;
  rsi_semaphore_handle_t http_soc_wait_sem;
  int num_successful_test;
  int num_failed_test;
};
typedef struct socktest_ctx_struct socktest_ctx_t;

#endif
#endif /* RSI_SOCK_TEST */
