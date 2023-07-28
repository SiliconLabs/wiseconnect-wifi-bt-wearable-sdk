
#include <rsi_common_app.h>
#if BT_A2DP_SOURCE_WIFI_HTTP_S_RX_DYN_COEX
#include <stdio.h>
#include <string.h>

#include <rsi_driver.h>
#include <rsi_bt_common_apis.h>
#include <rsi_bt_apis.h>
#include <rsi_bt.h>
#include "rsi_os.h"

#ifdef RSI_WITH_OS

extern rsi_semaphore_handle_t ui_task_sem;
#endif
//! Enumeration for states in application
typedef enum rsi_wlan_app_state_e {
  RSI_WLAN_APP_DISABLE            = 0,
  RSI_WLAN_APP_ENABLE             = 1,
  RSI_WLAN_INITIAL_STATE          = 2,
  RSI_WLAN_UNCONNECTED_STATE      = 3,
  RSI_WLAN_CONNECTED_STATE        = 4,
  RSI_WLAN_IPCONFIG_DONE_STATE    = 5,
  RSI_WLAN_SOCKET_CONNECTED_STATE = 6,
  RSI_WLAN_DATA_RECEIVE_STATE     = 7,
  RSI_SD_WRITE_STATE              = 8,
  RSI_WLAN_DEMO_COMPLETE_STATE    = 9
} rsi_wlan_app_state_t;

//! wlan application control block
typedef struct rsi_wlan_app_cb_s {
  //! wlan application state
  rsi_wlan_app_state_t state;
  //! length of buffer to copy
  uint32_t length;
  //! application buffer
  uint8_t buffer[RSI_APP_BUF_SIZE];
  //! to check application buffer availability
  uint8_t buf_in_use;
  //! application events bit map
  uint32_t event_map;

} rsi_wlan_app_cb_t;
//! application control block
extern rsi_wlan_app_cb_t rsi_wlan_app_cb;

extern uint8_t demoRingBuffer[100];

extern void rsi_bt_app_set_event(uint32_t event_num);

void rsi_ui_app_task(void)
{
  int8_t buffer[100];

  while (1) {
#ifdef RSI_WITH_OS
    //! Wait for interrupt reception
    rsi_semaphore_wait(&ui_task_sem, 0);
#endif
    //! Extract input
    memset(buffer, 0, 100);
    memcpy(buffer, demoRingBuffer, 100);

    memset(demoRingBuffer, 0, 100);

    /* Parse the input
		 * "bt_enable" - Initialize BT task and execute its functionality
		 * "bt_disable" - Close the connections
		 * "wifi_enable" - Initialize WLAN task and execute its functionality
		 * "wifi_disable" - Close the connections and suspend the task
		 */
    if (!(strcmp((const char *)buffer, "bt_enable\r")))
    //if(buffer[0] == 1)
    {
      //! Set event and post
      rsi_bt_app_set_event(RSI_APP_EVENT_BT_ENABLE);
    } else if (!(strcmp((const char *)buffer, "bt_disable\r")))
    //else if(buffer[0] == 0)
    {
      //! Set event
      rsi_bt_app_set_event(RSI_APP_EVENT_BT_DISABLE);
    }
    if (!(strcmp((const char *)buffer, "wlan_enable"))) {
      //! release "wifi_task_sem" semaphore
      //rsi_semaphore_post(&wifi_task_sem);

    } else if (!(strcmp((const char *)buffer, "wlan_disable"))) {
      //! Set event
      //! update wlan application state
      rsi_wlan_app_cb.state = RSI_WLAN_APP_DISABLE;
    }
  }
}
#endif
