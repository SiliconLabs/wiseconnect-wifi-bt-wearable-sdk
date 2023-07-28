/*******************************************************************************
* @file  rsi_wlan_apis.c
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
#include "rsi_wlan_config.h"
#define GAIN_TABLE_MAX_PAYLOAD_LEN 128
#ifdef RSI_CHIP_MFG_EN
#include "rsi_qspi_defines.h"
#endif

extern struct wpa_scan_results_arr *scan_results_array;

#ifdef PROCESS_SCAN_RESULTS_AT_HOST
/** This function will sort the scan-results array based on rssi
 *  * @param[in] scan_results_array - is the scan results
 *   */
void rsi_sort_scan_results_array_based_on_rssi(struct wpa_scan_results_arr *scan_results_array)
{
  uint16_t i = 0, j = 0;
  struct wpa_scan_res temp;
  memset(&temp, 0, sizeof(struct wpa_scan_res));
  if (scan_results_array != NULL) {
    for (i = 0; i < scan_results_array->num; i++) {
      for (j = 0; j < ((scan_results_array->num) - i - 1); j++) {
        if (scan_results_array->res[j].level < scan_results_array->res[j + 1].level) {
          memcpy(&temp, &(scan_results_array->res[j]), sizeof(struct wpa_scan_res));
          memcpy(&(scan_results_array->res[j]), &(scan_results_array->res[j + 1]), sizeof(struct wpa_scan_res));
          memcpy(&(scan_results_array->res[j + 1]), &temp, sizeof(struct wpa_scan_res));
        }
      }
    }
  }
}
#endif
/** @addtogroup WLAN
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_config_timeout(uint32_t timeout_bitmap,uint16_t timeout_value)
 * @brief      This API is used to set timeouts. \n This is a blocking API.
 * @pre        \ref rsi_wireless_init() API needs to be called before this API.
 * @param[in]  timeout_type        - It is used to identify which timeout to be set.\n 
 *																		 if timeout_type is
 *																		 1 - set for association and authentication timeout request in ms.
 *																		 2 - set for the each channel active scan time in ms
 *																		 3 - is set for  the WLAN keep alive time in seconds
 * @param[in]  timeout_value         - timeout value to be set.
 * @note			 Packet is sent as Wlan Keep alive before \ref rsi_config_ipaddress() \n
 *						 After \ref rsi_config_ipaddress() it is sent as gratuitous ARP.
 * @return     0              -    Success \n
 *             Non-Zero Value -    Failure
 *
 */

int32_t rsi_config_timeout(uint32_t timeout_type, uint16_t timeout_value)
{
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb  = rsi_driver_cb->wlan_cb;
  uint32_t timeout_bitmap = 0;

  // check whether valid timeout_type is given or not
  if (timeout_type != AUTH_ASSOC_TIMEOUT && timeout_type != CHANNEL_ACTIVE_SCAN_TIMEOUT
      && timeout_type != KEEP_ALIVE_TIMEOUT) {
    return RSI_ERROR_INVALID_PARAM;
  }

  // check whether module is in valid state range or not
  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE || wlan_cb->state >= RSI_WLAN_STATE_INIT_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);

  if (status == RSI_SUCCESS) {

    timeout_bitmap = BIT(timeout_type - 1);

    status = send_timeout(timeout_bitmap, timeout_value);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status
  return status;
}

/*==============================================*/
/**
 * @fn         int32_t send_timeout(uint32_t timeout_bitmap,uint16_t timeout_value)
 * @brief      This API is used to set timeouts. \n This is a blocking API.
 * @pre        \ref rsi_wireless_init() API needs to be called before this API.
 * @param[in]  timeout_type        - It is used to identify which timeout to be set.\n 
 *																		 if timeout_type is
 *																		 1 - set for association and authentication timeout request in ms.
 *																		 2 - set for the each channel active scan time in ms
 *																		 3 - is set for  the WLAN keep alive time in seconds
 * @param[in]  timeout_value         - timeout value to be set.
 * @note			 Packet is sent as Wlan Keep alive before \ref rsi_config_ipaddress() \n
 *						 After \ref rsi_config_ipaddress() it is sent as gratuitous ARP.
 * @return     0              -    Success \n
 *             Non-Zero Value -    Failure
 *
 */
int32_t send_timeout(uint32_t timeout_bitmap, uint16_t timeout_value)
{
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  rsi_pkt_t *pkt = NULL;

  rsi_req_timeout_t *rsi_timeout = NULL;

  // allocate command buffer  from wlan pool
  pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

  // If allocation of packet fails
  if (pkt == NULL) {
    // return packet allocation failure error
    return RSI_ERROR_PKT_ALLOCATION_FAILURE;
  }

  // Memset data
  memset(&pkt->data, 0, sizeof(rsi_req_timeout_t));

  rsi_timeout = (rsi_req_timeout_t *)pkt->data;

  // Timeout Bitmap
  rsi_uint32_to_4bytes(rsi_timeout->timeout_bitmap, timeout_bitmap);

  // Timeout value
  rsi_uint16_to_2bytes(rsi_timeout->timeout_value, timeout_value);

#ifndef RSI_WLAN_SEM_BITMAP
  rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif

  // send configure timeout command
  status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_TIMEOUT, pkt);
  // wait on wlan semaphore
  rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_TIMEOUT_RESPONSE_WAIT_TIME);
  // get wlan/network command response status
  status = rsi_wlan_get_status();
  // Return the status

  return status;
}
/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_scan_with_bitmap_options(int8_t *ssid,
 *														uint8_t chno, 
 *														rsi_rsp_scan_t *result, 
 *														uint32_t length,
 *														uint32_t scan_bitmap)
 * @brief	   Scan the access points available and posts the scan response to application.
 *             Application should call this api to get the scan results.
 * @pre		   \ref rsi_wireless_init() API needs to be called before this API.
 * @param[in]  ssid        - This is the SSID of an access point to connect. \n The SSID should be less than or equal to 32 bytes. \n
 *					         Note:
 *					         SSID is a null terminated string.
 * @param[in]  chno        - channel number of the access point 
 * @param[in]  result      - buffer address provided by the application to fill the scan response.				
 * @param[in]  length      - This is the length of the resulted buffer measured in bytes to hold scan results. \n
 *							 Size of structure \ref rsi_rsp_scan_t is 54 bytes. \n
 * 						     In which length for storing one scan result is sizeof( \ref rsi_scan_info_t)
 *							 which is 44 bytes. \n
 *						     Maximum of 11 scan results will be returned by the
 *							 module, \n in this case length should be configured
 *							 as  8 + 11*sizeof( \ref rsi_scan_info_t).	 
 * @param[in]  scan_bitmap - scan bitmap options
 * @return     0   		      -    Success \n
 *             Non-Zero Value -    Failure 
 *
 */
int32_t rsi_wlan_scan_with_bitmap_options(int8_t *ssid,
                                          uint8_t chno,
                                          rsi_rsp_scan_t *result,
                                          uint32_t length,
                                          uint32_t scan_bitmap)
{
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE || wlan_cb->state >= RSI_WLAN_STATE_CONNECTED) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // attach the buffer given by user
    wlan_cb->app_buffer = (uint8_t *)result;

    // length of the buffer provided by user
    wlan_cb->app_buffer_length = length;

    // call async scan to perform actual scan
    status = rsi_wlan_scan_async_with_bitmap_options(ssid, chno, scan_bitmap, NULL);

    if (status != RSI_SUCCESS) {
      // Return the status if error in sending command occurs
      return status;
    }

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_SCAN_WITH_BITMAP_RESPONSE_WAIT_TIME);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
  } else {
    // return wlan command error
    return status;
  }

  // Return status
  return status;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_scan_async_with_bitmap_options(int8_t *ssid, 
															   uint8_t chno,
															   uint32_t bitmap, 
															   void (*callback)(uint16_t status, 
																				const uint8_t *buffer, 
																				const uint16_t length))
 * @brief	   Scan the access points available and posts the scan response to application.
 * 			   Application should call this api to get the scan results.
 * @pre		   \ref rsi_wireless_init() API needs to be called before this API. 	
 * @param[in]  ssid        - This is the SSID of an access point to connect. \n
							 The SSID should be less than or equal to 32 bytes.  
 * @param[in]  chno        - channel number of the access point 
 * @param[in]  bitmap      - scan feature bitmap  
 * @param[in]  result      - buffer address provided by the application to fill
							 the scan response. 
 * @param[in]  length      - This is the length of the resulted buffer measured 
							 in bytes to hold scan results. \n Size of structure 
							 \ref rsi_rsp_scan_t is 54 bytes.
 *						     In which length for storing one scan result is 
                             sizeof( \ref rsi_scan_info_t) which is 46 bytes. \n
 *						     Maximum of 11 scan results will be returned by the
							 module, \n in this case length should be configured 
							 as 8 + 11*sizeof( \ref rsi_scan_info_t).
 * @param[in]  scan_bitmap - scan feature bitmap
 * @return     0		      -   Success \n
 *             Non-Zero Value -   Failure 
 *
 */

int32_t rsi_wlan_scan_async_with_bitmap_options(int8_t *ssid,
                                                uint8_t chno,
                                                uint32_t bitmap,
                                                void (*scan_response_handler)(uint16_t status,
                                                                              const uint8_t *buffer,
                                                                              const uint16_t length))

{
  rsi_pkt_t *pkt = NULL;
  rsi_req_scan_t *scan;
  int32_t status = RSI_SUCCESS;
  int32_t allow  = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

#ifdef PROCESS_SCAN_RESULTS_AT_HOST
  if (bitmap & RSI_SCAN_RESULTS_TO_HOST) {
    memset(scan_results_array, 0, sizeof(struct wpa_scan_results_arr));
    rsi_wlan_cb_non_rom->scan_results_to_host = 1;
  }
#endif

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE || wlan_cb->state >= RSI_WLAN_STATE_CONNECTED) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  if (scan_response_handler != NULL) {
    allow = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  }
  if (allow == RSI_SUCCESS) {
    switch (wlan_cb->state) {
      case RSI_WLAN_STATE_OPERMODE_DONE: {
        if (wlan_cb->field_valid_bit_map & RSI_SET_MAC_BIT) {
          // allocate command buffer  from wlan pool
          pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);
          // If allocation of packet fails
          if (pkt == NULL) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
            // return packet allocation failure error
            return RSI_ERROR_PKT_ALLOCATION_FAILURE;
          }

          // Memset data
          memset(&pkt->data, 0, sizeof(rsi_req_mac_address_t));

          // Memcpy data
          memcpy(&pkt->data, wlan_cb->mac_address, sizeof(rsi_req_mac_address_t));

#ifndef RSI_WLAN_SEM_BITMAP
          rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
          // send set mac command
          status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_MAC_ADDRESS, pkt);

          // wait on wlan semaphore
          rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_MAC_RESPONSE_WAIT_TIME);

          // get wlan/network command response status
          status = rsi_wlan_get_status();

          if (status != RSI_SUCCESS) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

            // Return the status if error in sending command occurs
            return status;
          }
        }

        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // Memset data
        memset(&pkt->data, 0, sizeof(rsi_req_band_t));

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send band command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_BAND, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_BAND_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
      }
        //no break
      // fall through
      case RSI_WLAN_STATE_BAND_DONE: {

#if RSI_TIMEOUT_SUPPORT

        status = send_timeout(RSI_TIMEOUT_BIT_MAP, RSI_TIMEOUT_VALUE);
        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
#endif
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);
        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif

        // send init command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_INIT, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_INIT_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // Return the status if error in sending command occurs
          return status;
        }
#if RSI_SET_REGION_SUPPORT
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // Memset data
        memset(&pkt->data, 0, sizeof(rsi_req_set_region_t));

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif

        // send set region command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_REGION, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_REGION_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
#endif
      }
        //no break
      // fall through
      case RSI_WLAN_STATE_SCAN_DONE:
      case RSI_WLAN_STATE_INIT_DONE:
      case RSI_WLAN_STATE_CONNECTED: {
#if RSI_WLAN_CONFIG_ENABLE
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif

        // send Configuration request command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_CONFIG, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_WLAN_CONFIG_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
#endif
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // FIll SCAN parameters
        scan = (rsi_req_scan_t *)pkt->data;

        // memset the pkt
        memset(&pkt->data, 0, sizeof(rsi_req_scan_t));

        // copy the channel number
        rsi_uint32_to_4bytes(scan->channel, chno);

        // copy SSID given by the application
        if (ssid != NULL) {
          rsi_strcpy(scan->ssid, ssid);
        }

        scan->scan_feature_bitmap = bitmap;

        if (scan_response_handler != NULL) {
          // Register scan response handler
          rsi_wlan_cb_non_rom->callback_list.wlan_scan_response_handler = scan_response_handler;
        } else {
#ifndef RSI_WLAN_SEM_BITMAP
          rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        }

        // send scan command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SCAN, pkt);
      } break;
      case RSI_WLAN_STATE_NONE:
      case RSI_WLAN_STATE_IP_CONFIG_DONE:
      case RSI_WLAN_STATE_IPV6_CONFIG_DONE:
      case RSI_WLAN_STATE_AUTO_CONFIG_GOING_ON:
      case RSI_WLAN_STATE_AUTO_CONFIG_DONE:
      case RSI_WLAN_STATE_AUTO_CONFIG_FAILED:
        break;

      default: {
        // Return the status if command given in driver invalid state
        status = RSI_ERROR_COMMAND_NOT_SUPPORTED;
        if (scan_response_handler == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
        }
      } break;
    }
  } else {
    // return wlan command error
    return allow;
  }
  return status;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_scan(int8_t *ssid, uint8_t chno, rsi_rsp_scan_t *result, uint32_t length)
 * @brief      Scan the surrounding Wi-Fi networks
 * @pre		   \ref rsi_wireless_init() API needs to be called before this API.
 * @param[in]  ssid    - The SSID size should be less than or equal to 32 bytes.Note: SSID is a null terminated string. 
 * @param[in]  chno    - This is the channel number to perform scan. If 0, then the module will scan all the channels. 
 * @param[in]  result  - This is the scanned Wi-Fi networks information. \n
 *						 This is an output parameter. This output contains a maximum of 11 scan results \n
 *						 The structure \ref rsi_rsp_scan_t contains members scan_count which specifies the number of scan \n
 *						 results followed by array of structure type \ref rsi_scan_info_t, where each array element contains \n
 *						 information about each network scanned
 * @param[in]  length  - This is the size that should be allocated to buffer which will store scan results. \n
 * 						 where scan result buffer is of Scan Response structure format. \n
 *                       Note: 
 *						 Size of structure rsi_rsp_scan_t is 54 bytes.
 *						 In which length for storing one scan result is sizeof() which is 44 bytes. \n
 *						 Maximum of 11 scan results will be returned by the module, in this case length should be configured as 8 + 11*sizeof(rsi_scan_info_t) \n
 * @note       To set various timeouts, user should change the following macros in rsi_wlan_config.h \n
 *			   #define RSI_TIMEOUT_SUPPORT RSI_ENABLE \n
 *			   #define RSI_TIMEOUT_BIT_MAP 4 \n
 *			   #define RSI_TIMEOUT_VALUE 300
 * @return     0      	      -  Success \n
 *             Non-Zero	Value - Failure \n
 *			                   If return value is less than 0 \n
 *                           -2 - Invalid parameters \n
 *			                 -3 - Command given in wrong state \n
 *			                 -4 - Buffer not available to serve the command \n
 *                           If return value is greater than 0 \n
 * 			                0x0002, 0x0003, 0x0005, 0x000A, 0x0014, 0x0015, 0x001A, 0x0021,0x0024,0x0025,0x0026,0x002C,0x003c
 * @note       Refer Error Codes section for above error codes description  \ref SP16.
 *
 *
 */
int32_t rsi_wlan_scan(int8_t *ssid, uint8_t chno, rsi_rsp_scan_t *result, uint32_t length)
{
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE || wlan_cb->state >= RSI_WLAN_STATE_CONNECTED) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // attach the buffer given by user
    wlan_cb->app_buffer = (uint8_t *)result;

    // length of the buffer provided by user
    wlan_cb->app_buffer_length = length;

    // call async scan to perform actual scan
    status = rsi_wlan_scan_async(ssid, chno, NULL);

    if (status != RSI_SUCCESS) {

      // Return the status if error in sending command occurs
      return status;
    }

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_SCAN_RESPONSE_WAIT_TIME);
    // get wlan/network command response status
    status = rsi_wlan_get_status();

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
  } else {
    // return wlan command error
    return status;
  }

  // Return status
  return status;
}
/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_scan_async(int8_t *ssid,  uint8_t chno, 
 *										   void (*scan_response_handler)(uint16_t status, 
										   const uint8_t *buffer, const uint16_t length))
 * @note	   If status is Success , the argument buffer passed to scan_response_handler \n 
 * 			   holds scan results in \ref rsi_rsp_scan_t structure format.
 * @brief      Scan the surrounding Wi-Fi networks. A scan response handler is registered with it to get the response for scan. 
 * @pre 	   \ref rsi_wireless_init() API needs to be called before this API. 
 * @param[in]  ssid					 -  The SSID size should be less than or equal to 32 bytes 
 *					   Note: SSID is a null terminated string.
 * @param[in]  chno 				 - Channel number to perform scan, if 0 then module will scan in all channels. 
 * @param[in]  Scan_response_handler - This callback is called when the response for scan has been received from the module. \n
 *									   The parameters involved are status, buffer & length. \n
 *									   Status: This is the response status. If status is zero, the scan response is stated as Success  \n
 *									   Buffer: This is the response buffer \n
 *									   Length: This is the length of the resulted buffer measured in bytes to hold scan results. \n Size of structure \ref rsi_rsp_scan_t is 54 bytes. \n
 *									   In which length for storing one scan result is sizeof(rsi_scan_info_t) which is 44 bytes. \n
 *									   Maximum of 11 scan results will be returned by the module, \n in this case length should be configured as 8 + 11*sizeof(rsi_scan_info_t).
 * @note       To set various timeouts, user should change the following macros in rsi_wlan_config.h \n
 *			   #define RSI_TIMEOUT_SUPPORT RSI_ENABLE \n
 *			   #define RSI_TIMEOUT_BIT_MAP 4 \n
 *			   #define RSI_TIMEOUT_VALUE 300
 * @return      0 			-  Success \n
 *              Non-Zero Value	- Failure \n
 *			   -2 - Invalid parameters \n
 *			   -3 - Command given in wrong state \n
 *             -4 - Buffer not available to serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 */

int32_t rsi_wlan_scan_async(int8_t *ssid,
                            uint8_t chno,
                            void (*scan_response_handler)(uint16_t status,
                                                          const uint8_t *buffer,
                                                          const uint16_t length))

{
  rsi_pkt_t *pkt;
  rsi_req_scan_t *scan;
  int32_t status = RSI_SUCCESS;
  int32_t allow  = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE || wlan_cb->state >= RSI_WLAN_STATE_CONNECTED) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  if (scan_response_handler != NULL) {
    allow = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  }
  if (allow == RSI_SUCCESS) {

    switch (wlan_cb->state) {
      case RSI_WLAN_STATE_OPERMODE_DONE: {
        if (wlan_cb->field_valid_bit_map & RSI_SET_MAC_BIT) {
          // allocate command buffer  from wlan pool
          pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

          // If allocation of packet fails
          if (pkt == NULL) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
            // return packet allocation failure error
            return RSI_ERROR_PKT_ALLOCATION_FAILURE;
          }

          // Memset data
          memset(&pkt->data, 0, sizeof(rsi_req_mac_address_t));

          // Memcpy data
          memcpy(&pkt->data, wlan_cb->mac_address, sizeof(rsi_req_mac_address_t));

#ifndef RSI_WLAN_SEM_BITMAP
          rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
          // send set mac command
          status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_MAC_ADDRESS, pkt);

          // wait on wlan semaphore
          rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_MAC_RESPONSE_WAIT_TIME);

          // get wlan/network command response status
          status = rsi_wlan_get_status();

          if (status != RSI_SUCCESS) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

            // Return the status if error in sending command occurs
            return status;
          }
        }
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // Memset data
        memset(&pkt->data, 0, sizeof(rsi_req_band_t));

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send band command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_BAND, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_BAND_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
      }
        //no break
      // fall through
      case RSI_WLAN_STATE_BAND_DONE: {
#if RSI_TIMEOUT_SUPPORT
        status = send_timeout(RSI_TIMEOUT_BIT_MAP, RSI_TIMEOUT_VALUE);
        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
#endif
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);
        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }
#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send init command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_INIT, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_INIT_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
#if RSI_SET_REGION_SUPPORT
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // Memset data
        memset(&pkt->data, 0, sizeof(rsi_req_set_region_t));

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif

        // send set region command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_REGION, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_REGION_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
#endif
      }
        //no break
      // fall through
      case RSI_WLAN_STATE_SCAN_DONE:
      case RSI_WLAN_STATE_INIT_DONE:
      case RSI_WLAN_STATE_CONNECTED: {
#if RSI_WLAN_CONFIG_ENABLE
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }
#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send Configuration request command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_CONFIG, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_WLAN_CONFIG_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
#endif
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // FIll SCAN parameters
        scan = (rsi_req_scan_t *)pkt->data;

        // memset the pkt
        memset(&pkt->data, 0, sizeof(rsi_req_scan_t));

        // copy the channel number
        rsi_uint32_to_4bytes(scan->channel, chno);

        // copy SSID given by the application
        if (ssid != NULL) {
          rsi_strcpy(scan->ssid, ssid);
        }

        if (scan_response_handler != NULL) {
          // Register scan response handler
          rsi_wlan_cb_non_rom->callback_list.wlan_scan_response_handler = scan_response_handler;
        } else {
#ifndef RSI_WLAN_SEM_BITMAP
          rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        }
        // send scan command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SCAN, pkt);
      } break;
      case RSI_WLAN_STATE_NONE:
      case RSI_WLAN_STATE_IP_CONFIG_DONE:
      case RSI_WLAN_STATE_IPV6_CONFIG_DONE:
      case RSI_WLAN_STATE_AUTO_CONFIG_GOING_ON:
      case RSI_WLAN_STATE_AUTO_CONFIG_DONE:
      case RSI_WLAN_STATE_AUTO_CONFIG_FAILED:
        break;
      default: {
        // Return the status if command given in driver invalid state
        status = RSI_ERROR_COMMAND_NOT_SUPPORTED;
        if (scan_response_handler == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
        }
      } break;
    }
  } else {
    // return wlan command error
    return allow;
  }
  return status;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_wlan_connect(int8_t *ssid, rsi_security_mode_t sec_type, void *secret_key)
 * @brief       Connect to the specified Wi-Fi network. 
 * @pre		    \ref rsi_wireless_init() API needs to be called before this API. 
 * @param[in]   ssid		- This is the SSID of an access point to connect. The SSID should be less than or equal to 32 bytes.   
 * @param[in]   sec_type 	- This is the security type of the access point to connect. 
 * @param[in]   secret_key  - This is the pointer to a buffer that contains security information based on sec_type. 
 * @note       To set various timeouts, user should change the following macros in rsi_wlan_config.h \n
 *			   #define RSI_TIMEOUT_SUPPORT RSI_ENABLE \n
 *			   #define RSI_TIMEOUT_BIT_MAP 4 \n
 *			   #define RSI_TIMEOUT_VALUE 300
 * @return      0 		        -  Success \n
 *              Non-Zero Value  - Failure \n
 *				-2 - Invalid parameters \n
 *				-3 - Command given in wrong state \n
 *				-4 - Buffer not available to serve the command
 * @note		Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 */

int32_t rsi_wlan_connect(int8_t *ssid, rsi_security_mode_t sec_type, void *secret_key)
{

  int32_t status = RSI_SUCCESS;

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // call async connect to call actual connect
    status = rsi_wlan_connect_async(ssid, sec_type, secret_key, NULL);

    if (status != RSI_SUCCESS) {
      // Return the status if error in sending command occurs
      return status;
    }

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_CONNECT_RESPONSE_WAIT_TIME);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

    // get wlan/network command response status
    status = rsi_wlan_get_status();

    if (status != RSI_SUCCESS) {
      // Return the status if error in sending command occurs
      return status;
    }

  } else {
    // return wlan command error
    return status;
  }
  // execute post connect commands
  status = rsi_wlan_execute_post_connect_cmds();

  status = rsi_wlan_get_status();
  return status;
}
/*==============================================*/
/**
 * @fn          int32_t rsi_wlan_connect_async(int8_t *ssid,
 *                              rsi_security_mode_t sec_type,
 *                              void *secret_key,
 *                              void (*join_response_handler)(uint16_t status,
 *                                                            const uint8_t *buffer,
 *                                                            const uint16_t length))
 * @brief       Connect to the specified Wi-Fi network.A join response handler is registered to get the response for join. 
 * @pre 	   \ref rsi_wireless_init() API needs to be called before this API. 
 * @param[in]   ssid - This is the SSID of an access point to connect. The SSID should be less than or equal to 32 bytes.   
 * @param[in]   sec_type			  - This is the security type of the Access point to connect. 
 * @param[in]   secret_key			  - This is the pointer to a buffer that contains security information based on sec_type. 
 * @param[in]   join_response_handler - This callback is called when the response for join has been received from the module \n
 *										The parameters involved are status, buffer & length \n
 *										Status: This is the response status. If status is zero, then the join response is stated as Success  \n
 *										Buffer: This is the response buffer. On Success ful execution of the command.  \n GO_Status (1 byte, hex): 0x47 (ASCII "G") If the module becomes a Group Owner (GO) after the GO 
 *										negotiation stage or becomes an Access Point. \n 0x43 (ASCII "C") If the module does not become a GO after the GO negotiation stage or becomes a client (or station).
 *										Length: This is the response buffer length.
 * @note         The module gets a default IP of 192.168.100.76 if it becomes a Group Owner or Access Point in case of IPv4. \n and gets a default IP of 2001:db8:0:1:0:0:0:120 in case of IPv6.
 * @return      0 - Success \n 
 *              Non-Zero - Failure \n
 *              -2 - Invalid parameters \n
 *				-3 - Command given in wrong state \n
 *				-4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 */

int32_t rsi_wlan_connect_async(int8_t *ssid,
                               rsi_security_mode_t sec_type,
                               void *secret_key,
                               void (*join_response_handler)(uint16_t status,
                                                             const uint8_t *buffer,
                                                             const uint16_t length))
{
  rsi_eap_credentials_t *credentials;
  rsi_req_wps_method_t *wps_method;
  rsi_req_psk_t *psk;
  rsi_wep_keys_t *wep_keys;
  rsi_pkt_t *pkt;
  rsi_req_scan_t *scan;
  rsi_req_join_t *join;
  rsi_req_eap_config_t *eap_config;
  int32_t status = RSI_SUCCESS;
  int32_t allow  = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;
  if (join_response_handler != NULL) {
    allow = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  }

  if (allow == RSI_SUCCESS) {
    // Check whether PSK is provided or not for security modes
    if ((sec_type != 0) && (secret_key == NULL) && (sec_type != RSI_WPS_PUSH_BUTTON)
        && (sec_type != RSI_USE_GENERATED_WPSPIN)) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // Throw error in case of invalid parameters
      return RSI_ERROR_INVALID_PARAM;
    }

    // check whether module is in valid state range or not
    if (wlan_cb->state >= RSI_WLAN_STATE_CONNECTED) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // Command given in wrong state
      return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
    }

    if ((wlan_cb->opermode == RSI_WLAN_ENTERPRISE_CLIENT_MODE)
        && !((sec_type == RSI_WPA_EAP) || (sec_type == RSI_WPA2_EAP))) {
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // Throw error in case of invalid parameters
      return RSI_ERROR_INVALID_PARAM;
    }

    switch (wlan_cb->state) {
      case RSI_WLAN_STATE_OPERMODE_DONE: {
        if (wlan_cb->field_valid_bit_map & RSI_SET_MAC_BIT) {
          // allocate command buffer  from wlan pool
          pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);
          // If allocation of packet fails
          if (pkt == NULL) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
            // return packet allocation failure error
            return RSI_ERROR_PKT_ALLOCATION_FAILURE;
          }
          // Memset data
          memset(&pkt->data, 0, sizeof(rsi_req_mac_address_t));
          // Memcpy data
          memcpy(&pkt->data, wlan_cb->mac_address, sizeof(rsi_req_mac_address_t));
#ifndef RSI_WLAN_SEM_BITMAP
          rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
          // send set mac command
          status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_MAC_ADDRESS, pkt);
          // wait on wlan semaphore
          rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_MAC_RESPONSE_WAIT_TIME);
          // get wlan/network command response status
          status = rsi_wlan_get_status();
          if (status != RSI_SUCCESS) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
            // Return the status if error in sending command occurs
            return status;
          }
        }
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);
        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }
#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send band command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_BAND, pkt);
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_BAND_RESPONSE_WAIT_TIME);
        // get wlan/network command response status
        status = rsi_wlan_get_status();
        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // Return the status if error in sending command occurs
          return status;
        }
      }
        //no break
      // fall through
      case RSI_WLAN_STATE_BAND_DONE: {
#if RSI_TIMEOUT_SUPPORT
        status = send_timeout(RSI_TIMEOUT_BIT_MAP, RSI_TIMEOUT_VALUE);
        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // Return the status if error in sending command occurs
          return status;
        }
#endif
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);
        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }
#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send init command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_INIT, pkt);
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_INIT_RESPONSE_WAIT_TIME);
        // get wlan/network command response status
        status = rsi_wlan_get_status();
        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          return status;
        }
#if RSI_SET_REGION_SUPPORT
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);
        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }
        // Memset data
        memset(&pkt->data, 0, sizeof(rsi_req_set_region_t));

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send set region command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_REGION, pkt);
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_REGION_RESPONSE_WAIT_TIME);
        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
#endif
      }
        //no break
      // fall through
      case RSI_WLAN_STATE_INIT_DONE: {
#if RSI_WMM_PS_ENABLE
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // memset the packet data
        memset(&pkt->data, 0, sizeof(rsi_wmm_ps_parms_t));

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send wmm parameters
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_WMM_PS, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_WMM_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }

#endif
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // give buffer as NULL, need not give scan results to host
        wlan_cb->app_buffer = NULL;

        // length of the buffer is zero
        wlan_cb->app_buffer_length = 0;

        // Send SCAN command
        scan = (rsi_req_scan_t *)pkt->data;

        // memset the packet data
        memset(&pkt->data, 0, sizeof(rsi_req_scan_t));

        // copy ssid given by the application
        if (ssid != NULL) {
          rsi_strcpy(scan->ssid, ssid);
        }

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send scan command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SCAN, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_SCAN_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
      }
        //no break
      // fall through
      case RSI_WLAN_STATE_SCAN_DONE: {

        // Send EAP command for EAP security
        if ((wlan_cb->opermode == RSI_WLAN_ENTERPRISE_CLIENT_MODE)) {
          // allocate command buffer  from wlan pool
          pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

          // If allocation of packet fails
          if (pkt == NULL) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
            // return packet allocation failure error
            return RSI_ERROR_PKT_ALLOCATION_FAILURE;
          }

          credentials = (rsi_eap_credentials_t *)secret_key;

          // memset the packet data
          memset(&pkt->data, 0, sizeof(rsi_req_eap_config_t));

          eap_config = (rsi_req_eap_config_t *)pkt->data;

          // Copy user name
          memcpy(eap_config->user_identity, credentials->username, RSI_EAP_USER_NAME_LENGTH);

          // Copy password
          memcpy(eap_config->password, credentials->password, RSI_EAP_PASSWORD_LENGTH);

#ifndef RSI_WLAN_SEM_BITMAP
          rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
          // send eap config command
          status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_EAP_CONFIG, pkt);

          // wait on wlan semaphore
          rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_EAP_RESPONSE_WAIT_TIME);

          // get wlan/network command response status
          status = rsi_wlan_get_status();

          if (status != RSI_SUCCESS) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

            // Return the status if error in sending command occurs
            return status;
          }
        }

#if RSI_WMM_PS_ENABLE
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // memset the packet data
        memset(&pkt->data, 0, sizeof(rsi_wmm_ps_parms_t));

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send wmm parameters
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_WMM_PS, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_WMM_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }

#endif
        if ((sec_type == RSI_WPS_PIN) && (wlan_cb->opermode != RSI_WLAN_ENTERPRISE_CLIENT_MODE)) {
          // allocate command buffer  from wlan pool
          pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

          // If allocation of packet fails
          if (pkt == NULL) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
            // return packet allocation failure error
            return RSI_ERROR_PKT_ALLOCATION_FAILURE;
          }

          wps_method = (rsi_req_wps_method_t *)pkt->data;

          // Memset buffer
          memset(&pkt->data, 0, sizeof(rsi_req_wps_method_t));

          //set configuration to validate wps pin
          rsi_uint16_to_2bytes(wps_method->wps_method, 1);

          //set configuration to validate wps pin
          rsi_uint16_to_2bytes(wps_method->generate_pin, 0);

          // Copy WPS Pin
          memcpy(wps_method->wps_pin, secret_key, RSI_WPS_PIN_LEN);

#ifndef RSI_WLAN_SEM_BITMAP
          rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
          // send wps method command
          status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_WPS_METHOD, pkt);

          // wait on wlan semaphore
          rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_WPS_RESPONSE_WAIT_TIME);

          // get wlan/network command response status
          status = rsi_wlan_get_status();

          if (status != RSI_SUCCESS) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
            // Return the status if error in sending command occurs
            return status;
          }
        } else if (sec_type == RSI_WEP) {
          // allocate command buffer  from wlan pool
          pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

          // If allocation of packet fails
          if (pkt == NULL) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
            // return packet allocation failure error
            return RSI_ERROR_PKT_ALLOCATION_FAILURE;
          }

          // Memset buffer
          memset(&pkt->data, 0, sizeof(rsi_wep_keys_t));

          wep_keys = (rsi_wep_keys_t *)pkt->data;

          // Copy wep keys
          memcpy(wep_keys, secret_key, sizeof(rsi_wep_keys_t));

#ifndef RSI_WLAN_SEM_BITMAP
          rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
          // send psk command
          status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_WEP_KEYS, pkt);

          // wait on wlan semaphore
          rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_WEP_RESPONSE_WAIT_TIME);

          // get wlan/network command response status
          status = rsi_wlan_get_status();

          if (status != RSI_SUCCESS) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

            // Return the status if error in sending command occurs
            return status;
          }

        }
        // Send PSK command for PSK security
        else if (((sec_type == RSI_WPA) || (sec_type == RSI_WPA2) || (sec_type == RSI_WPA_WPA2_MIXED)
                  || (sec_type == RSI_WPA_WPA2_MIXED_PMK) || (sec_type == RSI_WPA_PMK) || (sec_type == RSI_WPA2_PMK))
                 && (wlan_cb->opermode != RSI_WLAN_ENTERPRISE_CLIENT_MODE)) {
          // allocate command buffer  from wlan pool
          pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

          // If allocation of packet fails
          if (pkt == NULL) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
            // return packet allocation failure error
            return RSI_ERROR_PKT_ALLOCATION_FAILURE;
          }
          psk = (rsi_req_psk_t *)pkt->data;

          // Memset buffer
          memset(&pkt->data, 0, sizeof(rsi_req_psk_t));

          if ((sec_type == RSI_WPA2_PMK) || (sec_type == RSI_WPA_PMK) || (sec_type == RSI_WPA_WPA2_MIXED_PMK)) {
            if (sec_type == RSI_WPA_WPA2_MIXED_PMK) {
              sec_type == RSI_WPA_WPA2_MIXED;
            }
            // Set type as PMK
            psk->type = 2;

            // Copy PMK
            memcpy(psk->psk_or_pmk, secret_key, RSI_PMK_LEN);

          } else {
            // Set type as PSK
            psk->type = 1;

            // Copy PSK
            memcpy(psk->psk_or_pmk, secret_key, RSI_PSK_LEN);
          }

#ifndef RSI_WLAN_SEM_BITMAP
          rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
          // send psk command
          status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_HOST_PSK, pkt);
          // wait on wlan semaphore
          rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_WPS_RESPONSE_WAIT_TIME);

          // get wlan/network command response status
          status = rsi_wlan_get_status();

          if (status != RSI_SUCCESS) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

            // Return the status if error in sending command occurs
            return status;
          }
        }

#if RSI_REJOIN_PARAMS_SUPPORT
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // Memset data
        memset(&pkt->data, 0, sizeof(rsi_req_rejoin_params_t));

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send rejoin params command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_REJOIN_PARAMS, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_REJOIN_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
#endif
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // memset the pkt
        memset(&pkt->data, 0, sizeof(rsi_req_join_t));

        join = (rsi_req_join_t *)pkt->data;

        // Fill security type
        if (sec_type == RSI_WPA2_PMK) {
          join->security_type = RSI_WPA2;
        } else if (sec_type == RSI_WPA_PMK) {
          join->security_type = RSI_WPA;
        } else if ((sec_type == RSI_WPS_PIN) || (sec_type == RSI_USE_GENERATED_WPSPIN)
                   || (sec_type == RSI_WPS_PUSH_BUTTON)) {
          join->security_type = RSI_OPEN;
        } else {
          join->security_type = sec_type;
        }

        if (ssid != NULL) {
          // Copy ssid and ssid len
          join->ssid_len = rsi_strlen(ssid);

          // Copy Join SSID
          rsi_strcpy(join->ssid, ssid);
        }

        if (join_response_handler != NULL) {
          // Register scan response handler
          rsi_wlan_cb_non_rom->callback_list.wlan_join_response_handler = join_response_handler;
        } else {
#ifndef RSI_WLAN_SEM_BITMAP
          rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        }
        // send join command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_JOIN, pkt);
      } break;
      case RSI_WLAN_STATE_NONE:
      case RSI_WLAN_STATE_CONNECTED:
      case RSI_WLAN_STATE_IP_CONFIG_DONE:
      case RSI_WLAN_STATE_IPV6_CONFIG_DONE:
      case RSI_WLAN_STATE_AUTO_CONFIG_GOING_ON:
      case RSI_WLAN_STATE_AUTO_CONFIG_DONE:
      case RSI_WLAN_STATE_AUTO_CONFIG_FAILED:
        break;
      default: {
        // Return the status if command given in driver invalid state
        status = RSI_ERROR_COMMAND_NOT_SUPPORTED;
        if (join_response_handler == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
        }
      } break;
    }
  } else {
    // return wlan command error
    return allow;
  }
  // Return the status
  return status;
}

/*==============================================*/
/**
 * @fn           int32_t rsi_wlan_bgscan_profile(uint8_t cmd,rsi_rsp_scan_t *result,uint32_t length )
 * @brief        Get background scan results or stop bgscan.
 * @pre		     \ref rsi_wlan_connect() API needs to be called before this API. 
 * @param[in]    result - Result
 * @param[out]   length - Length
 * @return       0  	  -  Success \n
 *               Non-Zero Value - Failure \n
 *				 -3 - Command given in wrong state \n
 *				 -4 - Buffer not availableto serve the command
 * @note 		 Please refer to Error Codes section for the description of the above error codes  \ref SP16. 
 *
 */

int32_t rsi_wlan_bgscan_profile(uint8_t cmd, rsi_rsp_scan_t *result, uint32_t length)
{
  rsi_pkt_t *pkt;
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  // check whether module is in valid state range or not
  if (wlan_cb->state < RSI_WLAN_STATE_CONNECTED) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // attach the buffer given by user
    wlan_cb->app_buffer = (uint8_t *)result;

    // length of the buffer provided by user
    wlan_cb->app_buffer_length = length;

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Memset data
    memset(&pkt->data, 0, sizeof(rsi_req_bg_scan_t));

    // Magic word
    pkt->data[0] = 0xAB;

    // cmd
    pkt->data[1] = cmd;
#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif

    // send bg scan command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_BG_SCAN, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_BGSCAN_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}
/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_execute_post_connect_cmds(void)
 * @brief      Enable scan and roaming after connecting to the Access Point. 
 * @pre        \ref rsi_wlan_connect() API needs to be called before this API.  
 * @param[in]  void
 * @return      0  		 -  Success \n 
 *              Non-Zero  Value - Failure \n
 *				-3 - Command given in wrong state \n
 *				-4 - Buffer not available to serve the command
 * @note		Please refer to Error Codes section for the description of the above error codes  \ref SP16. 
 *
 */

// execute post connect commands
int32_t rsi_wlan_execute_post_connect_cmds(void)
{
#if (RSI_BG_SCAN_SUPPORT || RSI_ROAMING_SUPPORT)
  rsi_pkt_t *pkt;
#endif
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  // check whether module is in valid state range or not
  if (wlan_cb->state < RSI_WLAN_STATE_CONNECTED) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
#if RSI_BG_SCAN_SUPPORT
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Memset data
    memset(&pkt->data, 0, sizeof(rsi_req_bg_scan_t));

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send bg scan command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_BG_SCAN, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_BGSCAN_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();

    if (status != RSI_SUCCESS) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

      // Return the status if error in sending command occurs
      return status;
    }

#endif
#if RSI_ROAMING_SUPPORT
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }
    // Memset data
    memset(&pkt->data, 0, sizeof(rsi_req_roam_params_t));

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send roaming parameters command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_ROAM_PARAMS, pkt);

    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_ROAMING_RESPONSE_WAIT_TIME);
    // get wlan/network command response status
    status = rsi_wlan_get_status();
#endif
    // wait on wlan semaphore

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return nwk command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_wlan_wps_push_button_event(int8_t *ssid)
 * @brief       Start the WPS Push button in AP mode.It should be called after rsi_wlan_ap_start API once it has returned Success .
 * @pre         \ref rsi_wlan_ap_start() API needs to be called before this API.
 * @param[in]   ssid - This is the SSID of the Access Point. The SSID should be same as that of given in AP start API.  
 * @return      0		 -  Success \n 
 *              Non-Zero Value - Failure \n
 *				-4 - Buffer not available to serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 */

int32_t rsi_wlan_wps_push_button_event(int8_t *ssid)
{
  rsi_pkt_t *pkt;
  rsi_req_join_t *join;
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_req_join_t));

    join = (rsi_req_join_t *)pkt->data;

    if (ssid != NULL) {
      // Copy ssid and ssid len
      join->ssid_len = rsi_strlen(ssid);

      // Copy Join SSID
      rsi_strcpy(join->ssid, ssid);
    }

    // Deregister join response handler
    rsi_wlan_cb_non_rom->callback_list.wlan_join_response_handler = NULL;

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send join command to start wps
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_JOIN, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_JOIN_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn           int32_t rsi_send_freq_offset (int32_t freq_offset_in_khz)
 * @brief        Application to provide feedback of frequency error in KHz feedback of frequency error in KHz.
 * @param[in]    freq_offset_in_khz - Frequency deviation observed in KHz
 * @return       0 		 - Success \n
 *				 Non zero Value - Failure
 */
int32_t rsi_send_freq_offset(int32_t freq_offset_in_khz)
{
  rsi_pkt_t *pkt;
  rsi_freq_offset_t *freq_offset = NULL;
  int32_t status                 = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      // Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }
    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_freq_offset_t));

    freq_offset = (rsi_freq_offset_t *)pkt->data;

    freq_offset->freq_offset_in_khz = freq_offset_in_khz;

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send  wps request command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_FREQ_OFFSET, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_FREQ_OFFSET_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    // Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn                 int32_t rsi_calib_write(uint8_t target, uint32_t flags,
											  int8_t gain_offset, int32_t freq_offset)
 * @brief             Write flash/EFuse with xoctune and gain_offset in dBm
 * @param[in]         Target BURN_INTO_EFUSE or BURN_INTO_FLASH
 * @param[in]         Flags - valids information 
 *                       BURN_GAIN_OFFSET   
 *                       BURN_FREQ_OFFSET
 *                       SW_XO_CTUNE_VALID 
 * @param[in]         gain_offset calculated as (observed_power_level + cable_or_channel_loss - configured_power_level) in dBm
 * @param[in]         Frequency offset
 * @return            0 		- Success \n
 *                    Non-Zero Value  - Failure
 */

int32_t rsi_calib_write(uint8_t target, uint32_t flags, int8_t gain_offset, int32_t xo_ctune)
{
  rsi_pkt_t *pkt;
  rsi_calib_write_t *calib_wr = NULL;
  int32_t status              = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      // Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    calib_wr = (rsi_calib_write_t *)pkt->data;

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_calib_write_t));

    // Target flash or efuse
    calib_wr->target = target;

    // flags
    calib_wr->flags = flags;

    // gain offset
    calib_wr->gain_offset = gain_offset;

    // freq offset
    calib_wr->xo_ctune = xo_ctune;

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send  wps request command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_CALIB_WRITE, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_CALIB_WRITE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    // Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}
/*==============================================*/
/**
 * @fn            int16_t rsi_parse(void *address, uint16_t length, uint8_t *value)
 * @brief         Parse and convert the given value in ASCII to datatype of given length
 * @param[in]     address - Address 
 * @param[in]     length - Length 
 * @param[in]     value - Value 
 * @return        0 		- Success \n 
 *                Non-Zero Value  - Failure
 */
int16_t rsi_parse(void *address, uint16_t length, uint8_t *value)
{
  uint8_t temp_buff[15];
  uint16_t temp_offset = 0;
  uint16_t offset;

  for (offset = 0; offset < length; offset++) {
    uint16_t index = 0;
    /* Copy the bytes untill '.' ',' '\0' reached */
    while ((value[temp_offset] != '.') && (value[temp_offset] != ',') && (value[temp_offset] != '\0')) {
      if (index >= (sizeof(temp_buff) - 1))
        break;
      temp_buff[index++] = value[temp_offset++];
    }
    temp_buff[index] = '\0';
    temp_offset++;
    if (length == RSI_PARSE_2_BYTES) {
      *((uint16_t *)address) = rsi_atoi((int8_t *)temp_buff);
      break;
    } else if (length == RSI_PARSE_4_BYTES) {
      *((uint32_t *)address) = rsi_atoi((int8_t *)temp_buff);
      break;
    } else
      *((uint8_t *)address + offset) = rsi_atoi((int8_t *)temp_buff);
  }

  return temp_offset;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_wps_generate_pin(uint8_t *wps_pin, uint16_t length)
 * @brief      Generate WPS pin 
 * @pre        \ref rsi_wireless_init() API needs to be called before this API.
 * @param[in]  wps_pin - This is the 8-byte WPS pin generated by the device. This is the output parameter. 
 * @param[in]  length  -  This is the length of the resulted buffer measured in bytes to hold WPS pin.  
 * @param[out] 8 bytes wps pin 
 * @return	   0  		- Success \n
 *             Non-Zero Value - Failure \n
 *			   -4 - Buffer not availableto serve the command
 * @note       Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 */

int32_t rsi_wlan_wps_generate_pin(uint8_t *wps_pin, uint16_t length)
{
  rsi_pkt_t *pkt;
  rsi_req_wps_method_t *wps_method;
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    wps_method = (rsi_req_wps_method_t *)pkt->data;

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_req_wps_method_t));

    //set configuration to generate wps pin
    rsi_uint16_to_2bytes(wps_method->wps_method, 1);

    //set configuration to generate wps pin
    rsi_uint16_to_2bytes(wps_method->generate_pin, 1);

    // attach the buffer given by user
    wlan_cb->app_buffer = (uint8_t *)wps_pin;

    // length of the buffer provided by user
    wlan_cb->app_buffer_length = length;

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send  wps request command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_WPS_METHOD, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_WPS_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn			int32_t rsi_wlan_wps_enter_pin(int8_t *wps_pin)
 * @brief		Validate WPS pin entered. 
 * @pre   		\ref rsi_wlan_init() API needs to be called before this API. 
 * @param[in]   wps_pin - 8 byte valid wps pin  
 * @param[out]  same wps pin if command Success  
 * @return		0 		- Success \n
 *				Non-Zero Value  - Failure
 */

int32_t rsi_wlan_wps_enter_pin(int8_t *wps_pin)
{
  rsi_pkt_t *pkt;
  rsi_req_wps_method_t *wps_method;
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    wps_method = (rsi_req_wps_method_t *)pkt->data;

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_req_wps_method_t));

    //set configuration to validate wps pin
    rsi_uint16_to_2bytes(wps_method->wps_method, 1);

    //set configuration to validate wps pin
    rsi_uint16_to_2bytes(wps_method->generate_pin, 0);

    // Copy WPS Pin
    memcpy(wps_method->wps_pin, wps_pin, RSI_WPS_PIN_LEN);

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send wps request command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_WPS_METHOD, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_WPS_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_get_random_bytes(uint8_t *result,uint32_t length)
 * @brief       Get random bytes from the device.
 * @param[in]   result - pointer to the buffer in which random data which is to be copied
 * @param[in]   length - the length of the random data required
 * @return      0 		- Success \n
 *              Non-Zero Value  - Failure
 */

int32_t rsi_get_random_bytes(uint8_t *result, uint32_t length)
{
  rsi_pkt_t *pkt;
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;
  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // attach the buffer given by user
    wlan_cb->app_buffer = (uint8_t *)result;

    // length of the buffer provided by user
    wlan_cb->app_buffer_length = length;

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_req_disassoc_t));

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send get random number command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_GET_RANDOM, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_WLAN_GET_RANDOM_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_wlan_disconnect(void)
 * @brief       Disconnect the module from the connected Access point.
 * @pre    		\ref rsi_wlan_connect() API needs to be called before this API.
 * @return      Non-Zero Value  - Failure \n
 *                    0 		- Success \n 
 *				-3 - Command given in wrong state \n
 *				-4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 */

int32_t rsi_wlan_disconnect(void)
{
  rsi_pkt_t *pkt;
  int32_t status = RSI_SUCCESS;

#ifndef RSI_CHIP_MFG_EN
  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_CONNECTED) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_req_disassoc_t));

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send disconnect command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_DISCONNECT, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_DISCONNECT_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

#endif
  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_disconnect_stations(uint8_t *mac_address)
 * @brief      Disconnect the connected stations in AP mode.
 * @pre   	   \ref rsi_wlan_ap_start() API needs to be called before this API.
 * @param[in]  mac_address - Mac address (6 bytes) of the station to be disconnected.
 * @return     0 		- Success \n
 *             Non-Zero Value  - Failure \n
 *			   -4	- Buffer not availableto serve the command
 * @note       Please refer to Error Codes section for the description of the above error codes  \ref SP16. 
 *
 */

int32_t rsi_wlan_disconnect_stations(uint8_t *mac_address)
{
  rsi_pkt_t *pkt;
  rsi_req_disassoc_t *disassoc_info;
  int32_t status = RSI_SUCCESS;

#ifndef RSI_CHIP_MFG_EN
  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    disassoc_info = (rsi_req_disassoc_t *)pkt->data;

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_req_disassoc_t));

    //set mode flag to represent ap mode
    rsi_uint16_to_2bytes(disassoc_info->mode_flag, 1);

    //copy mac address of station to be disconnected
    memcpy(disassoc_info->client_mac_address, mac_address, 6);

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send disconnect command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_DISCONNECT, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_DISCONNECT_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

#endif
  // Return the status if error in sending command occurs
  return status;
}
/** @} */
/** @addtogroup NETWORK5
* @{
*/

/*==============================================*/
/**
 * @fn        	 int32_t rsi_config_ipaddress(rsi_ip_version_t version, uint8_t mode, uint8_t *ip_addr, 
 *										   uint8_t *mask, uint8_t *gw, uint8_t *ipconfig_rsp, 
 *											uint16_t length, uint8_t vap_id);
 * @brief      	 Configure the IP address to the module.
 * @pre   		 \ref rsi_wlan_connect() API needs to be called before this API. 
 * @param[in]    version 	  - ip version to be used 
 * @param[in]    mode    	  - 1 - dhcp mode ; 0 - static mode  
 * @param[in]    ip_addr 	  - This is the pointer to IP address
 * @param[in]    mask    	  - This is the pointer to network mask 
 * @param[in]    gw      	  - This is the pointer to gateway address   
 * @param[in]    ipconfig_rsp -  To hold the IP configuration received using DHCP. 
 * @param[in]    length  	  - This is the length of ipconfig_rsp buffer 
 * @param[in]    vap_id       - This is the vap id to differentiate between AP and station in concurrent mode.
 * @return       Non-Zero Value  - Failure \n
 *                    0 		- Success \n
 *               -2  - Invalid parameters \n
 *				 -3  - Command given in wrong state \n
 *				 -4  - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes  \ref SP16. 
 *
 */
int32_t rsi_config_ipaddress(rsi_ip_version_t version,
                             uint8_t mode,
                             uint8_t *ip_addr,
                             uint8_t *mask,
                             uint8_t *gw,
                             uint8_t *ipconfig_rsp,
                             uint16_t length,
                             uint8_t vap_id)
{
  rsi_pkt_t *pkt;
  rsi_req_ipv4_parmas_t *ip_params;
  rsi_req_ipv6_parmas_t *ipv6_params;
  int32_t status                 = RSI_SUCCESS;
  int32_t rsi_response_wait_time = 0;
  // Get WLAN cb struct pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  // Wlan radio init
  status = rsi_wlan_radio_init();

  if (status != RSI_SUCCESS) {
    // Return the status if error in sending command occurs
    return status;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // attach the buffer given by user
    wlan_cb->app_buffer = (uint8_t *)ipconfig_rsp;

    // length of the buffer provided by user
    wlan_cb->app_buffer_length = length;

    // If ipversion is IPv4
    if (version == RSI_IP_VERSION_4) {
      // fill all the ipv4 parameters
      ip_params = (rsi_req_ipv4_parmas_t *)pkt->data;

      // Set DHCP mode
      ip_params->dhcp_mode = mode;

      // fill ip address only if static mode is selected
      if (mode == RSI_STATIC) {
        // Fill IP address
        memcpy(ip_params->ipaddress, ip_addr, RSI_IPV4_ADDRESS_LENGTH);

        // Fill network mask
        memcpy(ip_params->netmask, mask, RSI_IPV4_ADDRESS_LENGTH);

        // Fill gateway
        memcpy(ip_params->gateway, gw, RSI_IPV4_ADDRESS_LENGTH);
      }

      // Check for DHCP HOSTNAME feature
      if ((mode & RSI_DHCP_HOSTNAME) == RSI_DHCP_HOSTNAME) {
        // Copy DHCP client hostname
        rsi_strcpy(ip_params->hostname, RSI_DHCP_HOST_NAME);
      }

      // Set vap_id
      ip_params->vap_id = vap_id;
#ifndef RSI_WLAN_SEM_BITMAP
      rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif

      // send ip config command
      status                 = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_IPCONFV4, pkt);
      rsi_response_wait_time = RSI_IPCONFV4_RESPONSE_WAIT_TIME;

    } else if (version == RSI_IP_VERSION_6) {
      // fill all the ipv6 params
      ipv6_params = (rsi_req_ipv6_parmas_t *)pkt->data;

      // Fill prefix length
      rsi_uint16_to_2bytes(ipv6_params->prefixLength, 64);

      // Fill DHCP mode
      rsi_uint16_to_2bytes(ipv6_params->mode, mode);

      // fill ip address only if static mode is selected
      if (mode == RSI_STATIC) {
        // Fill IPv6 address
        memcpy(ipv6_params->ipaddr6, ip_addr, RSI_IPV6_ADDRESS_LENGTH);

        // Fill gateway
        memcpy(ipv6_params->gateway6, gw, RSI_IPV6_ADDRESS_LENGTH);
      }

#ifndef RSI_WLAN_SEM_BITMAP
      rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
      // send ip config command
      status                 = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_IPCONFV6, pkt);
      rsi_response_wait_time = RSI_IPCONFV6_RESPONSE_WAIT_TIME;
    } else {

      // deattach buffer
      wlan_cb->app_buffer = NULL;

      //make length of the buffer to 0
      wlan_cb->app_buffer_length = 0;

      // update Throw error in case of invalid parameters
      status = RSI_ERROR_INVALID_PARAM;
      // free the packet
      rsi_pkt_free(&wlan_cb->wlan_tx_pool, pkt);
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

      return status;
    }

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status
  return status;
}
/** @} */
/** @addtogroup WLAN
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t  rsi_wlan_set_certificate_index(uint8_t certificate_type, uint8_t cert_inx, 
 * 													   uint8_t *buffer, uint32_t certificate_length)
 * @brief      Write or erase certificate into module.
 * @pre   	   \ref rsi_wireless_init() API must be called before this API.
 * @param[in]  certificate_type   - type of certificate
 * @param[in]  cert_inx 		  - Index of certificate 
 * @param[in]  buffer 			  - certificate content
 * @param[in]  certificate_length - This is the certificate length 
 * @return      Non-Zero Value  - Failure \n
 *                    0 		- Success \n
 *              -2 - Invalid parameters \n
 *				-3 - Command given in wrong state \n
 *				-4 - Buffer not availableto serve the command
 * @note 		 Index based certificate loading is valid only for storing certificates on to ram or flash but not both at a time. \n
 * 				 Enable BIT(27) in tcp_ip_feature_bit_map to load SSl certificate into RAM. \n
 *				 Enable BIT(31) in tcp_ip_feature_bit_map and BIT (29) in ext_tcp_ip_feature_bit_map to open 3 SSL client sockets. \n
 *				 Three SSL client socket feature supported only in WLAN mode.
 *
 *
 */
int32_t rsi_wlan_set_certificate_index(uint8_t certificate_type,
                                       uint8_t cert_inx,
                                       uint8_t *buffer,
                                       uint32_t certificate_length)
{
  static uint32_t rem_len;
  uint16_t chunk_size = 0;
  static uint32_t offset;
  rsi_pkt_t *pkt;
  int32_t status = RSI_SUCCESS;
  rsi_req_set_certificate_t *rsi_chunk_ptr;
  int32_t rsi_response_wait_time = 0;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  // check if command is in correct state or not
  if ((wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE)) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }

  if ((buffer == NULL) && (certificate_length)) {
    // Throw error in case of invalid parameters
    return RSI_ERROR_INVALID_PARAM;
  }

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    //Get the certificate chunk size
    chunk_size = (RSI_MAX_CERT_SEND_SIZE - sizeof(struct rsi_cert_info_s));

    // get certificate length
    rem_len = certificate_length;

    // if certificate length is 0,then erase that certifate
    if (rem_len == 0) {
      // allocate command buffer  from wlan pool
      pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

      // If allocation of packet fails
      if (pkt == NULL) {
        //Changing the wlan cmd state to allow
        rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
        // return packet allocation failure error
        return RSI_ERROR_PKT_ALLOCATION_FAILURE;
      }

      rsi_chunk_ptr = (rsi_req_set_certificate_t *)pkt->data;

      // memset the pkt
      memset(&pkt->data, 0, RSI_MAX_CERT_SEND_SIZE);

      //Set the total length of certificate
      rsi_uint16_to_2bytes(rsi_chunk_ptr->cert_info.total_len, certificate_length);

      // Set the certificate type
      rsi_chunk_ptr->cert_info.certificate_type = certificate_type;

      // Set the certificate index
      rsi_chunk_ptr->cert_info.certificate_inx = cert_inx;

      // more chunks to send
      rsi_chunk_ptr->cert_info.more_chunks = 0;

      // Set the length of the certificate chunk
      rsi_uint16_to_2bytes(rsi_chunk_ptr->cert_info.certificate_length, 0);

      rsi_response_wait_time = RSI_CERTIFICATE_RESPONSE_WAIT_TIME;
#ifndef RSI_WLAN_SEM_BITMAP
      rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
      // send the driver command
      status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_CERTIFICATE, pkt);
      // wait on wlan semaphore
      rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);
      // get wlan/network command response status
      status = rsi_wlan_get_status();
      if (status != RSI_SUCCESS) {
        //Changing the wlan cmd state to allow
        rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

        // Return the status if error in sending command occurs
        return status;
      }
    }
    while (rem_len) {
      // allocate command buffer  from wlan pool
      pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

      // If allocation of packet fails
      if (pkt == NULL) {
        //Changing the wlan cmd state to allow
        rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
        // return packet allocation failure error
        return RSI_ERROR_PKT_ALLOCATION_FAILURE;
      }

      rsi_chunk_ptr = (rsi_req_set_certificate_t *)pkt->data;
      // memset the pkt
      memset(&pkt->data, 0, RSI_MAX_CERT_SEND_SIZE);

      if (rem_len >= chunk_size) {
        // Copy the certificate chunk
        memcpy(rsi_chunk_ptr->certificate, buffer + offset, chunk_size);

        // Move the offset by chunk size
        offset += chunk_size;

        // Subtract the rem_len by the chunk size
        rem_len -= chunk_size;

        //Set the total length of certificate
        rsi_uint16_to_2bytes(rsi_chunk_ptr->cert_info.total_len, certificate_length);

        // Set the certificate type
        rsi_chunk_ptr->cert_info.certificate_type = certificate_type;

        // Set the certificate index
        rsi_chunk_ptr->cert_info.certificate_inx = cert_inx;

        // more chunks to send
        rsi_chunk_ptr->cert_info.more_chunks = 1;

        // Set the length of the certificate chunk
        rsi_uint16_to_2bytes(rsi_chunk_ptr->cert_info.certificate_length, chunk_size);

        rsi_response_wait_time = RSI_CERTIFICATE_RESPONSE_WAIT_TIME;
#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send the driver command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_CERTIFICATE, pkt);
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);
        // get wlan/network command response status
        status = rsi_wlan_get_status();
        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }

      } else {

        // Copy the certificate chunk
        memcpy(rsi_chunk_ptr->certificate, buffer + offset, rem_len);

        // Move the offset by chunk size
        offset += rem_len;

        //Set the total length of certificate
        rsi_uint16_to_2bytes(rsi_chunk_ptr->cert_info.total_len, certificate_length);

        // Set the certificate type
        rsi_chunk_ptr->cert_info.certificate_type = certificate_type;

        // Set the certificate index
        rsi_chunk_ptr->cert_info.certificate_inx = cert_inx;

        //This is the last chunks to send
        rsi_chunk_ptr->cert_info.more_chunks = 0;

        // Set the length of the certificate chunk
        rsi_uint16_to_2bytes(rsi_chunk_ptr->cert_info.certificate_length, rem_len);

        rsi_response_wait_time = RSI_CERTIFICATE_RESPONSE_WAIT_TIME;
#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send the driver command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_CERTIFICATE, pkt);
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);
        // get wlan/network command response status
        status = rsi_wlan_get_status();
        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }

        // Reset rem_len and offset
        rem_len = 0;
        offset  = 0;
      }
    }

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // get wlan/network command response status
  status = rsi_wlan_get_status();

  // Return the status
  return status;
}

/*==============================================*/
/**
 * @fn         int32_t  rsi_wlan_set_certificate(uint8_t certificate_type, uint8_t *buffer, uint32_t certificate_length)
 * @brief      Load SSL / EAP certificate on WiSeConnect module. 
 * @pre        \ref rsi_wireless_init() API must be called before this API. 
 * @param[in]  certificate_type   - type of certificate
 * @param[in]  buffer 			  - This is the pointer to a buffer which contain certificate 
 * @param[in]  certificate_length - This is the certificate length 
 * @return      Non-Zero Value  - Failure \n
 *                    0 		- Success \n
 *              -2 - Invalid parameters \n
 *				-3 - Command given in wrong state \n
 *				-4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 */
int32_t rsi_wlan_set_certificate(uint8_t certificate_type, uint8_t *buffer, uint32_t certificate_length)
{
  return rsi_wlan_set_certificate_index(certificate_type, 0, buffer, certificate_length);
}

/*==============================================*/
/**
 * @fn          int32_t rsi_wlan_get_status(void)
 * @brief       Checks the status (specific error code) of the errors encountered during a call to a WLAN API or BSD sockets
 * 				functions. User can call this API to check the error code (refer error code table for description of the errors). 
 * @param[in]   void
 * @return      wlan status
 */
int32_t rsi_wlan_get_status(void)
{
  return rsi_driver_cb->wlan_cb->status;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_wlan_get(rsi_wlan_query_cmd_t cmd_type, uint8_t *response, uint16_t length)
 * @brief       Get the required information based on the type of command.
 * @pre 		\ref rsi_wireless_init () API needs to be called before this API. 
 * 				This function returns wlan status
 * @param[in] cmd_type  -  This is the Query command type: \n
 *              		1 : RSI_FW_VERSION \n
 *              		2 : RSI_MAC_ADDRESS \n
 *              		3 : RSI_RSSI \n
 *              		4 : RSI_WLAN_INFO \n
 *              		5 : RSI_CONNECTION_STATUS \n
 *              		6 : RSI_STATIONS_INFO \n
 *              		7 : RSI_SOCKETS_INFO \n 
 *              		8 : RSI_CFG_GET \n
 *                      9 : RSI_GET_WLAN_STAT 
 * @param[in]   response - This is the the response of the requested command. This is an output parameter.
 * @param[in]   length  - This is the length of the response buffer in bytes to hold result. 
 * @note	  	RSI_WLAN_INFO is relevant in both station and AP mode. \n
 *				RSI_SOCKETS_INFO is relevant in both station mode and AP mode. \n
 *				RSI_STATIONS_INFO is relevant in AP mode \n
 *				RSI_GET_WLAN_STATS is relevant in AP and Station mode
 * @return  	Non-Zero Value  - Failure \n
 *                    0 		- Success \n
 *				-3 - Command given in wrong state \n
 *				-4 - Buffer not availableto serve the command \n
 *				-6 - Insufficient input buffer given
 * @note        Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 *
 */
int32_t rsi_wlan_get(rsi_wlan_query_cmd_t cmd_type, uint8_t *response, uint16_t length)
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  int32_t rsi_response_wait_time = 0;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  // Get common control block structure pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  // if state is not in card ready received state
  if (common_cb->state == RSI_COMMON_STATE_NONE) {
    if (cmd_type == RSI_FW_VERSION) {
      while (common_cb->state != RSI_COMMON_CARDREADY) {
#ifndef RSI_WITH_OS
        rsi_scheduler(&rsi_driver_cb->scheduler_cb);
#endif
      }
    } else
      // Command given in wrong state
      return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }

  if (cmd_type == RSI_FW_VERSION) {
    // Send firmware version query request
    status = rsi_get_fw_version(response, length);
    return status;
  }

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // copy query cmd type in driver control block
    wlan_cb->query_cmd = cmd_type;

    if (response != NULL) {
      // attach the buffer given by user
      wlan_cb->app_buffer = response;

      // length of the buffer provided by user
      wlan_cb->app_buffer_length = length;
    } else {
      // Assign NULL to the app_buffer to avoid junk
      wlan_cb->app_buffer = NULL;

      // length of the buffer to 0
      wlan_cb->app_buffer_length = 0;
    }
    switch (cmd_type) {
      case RSI_RSSI: {
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }
        rsi_response_wait_time = RSI_WLAN_RSSI_RESPONSE_WAIT_TIME;
#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // Send RSSI query request
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_RSSI, pkt);
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);

      } break;
      case RSI_MAC_ADDRESS: {
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // Send MAC address query request
        status                 = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_MAC_ADDRESS, pkt);
        rsi_response_wait_time = RSI_MAC_RESPONSE_WAIT_TIME;
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);

      } break;
      case RSI_CONNECTION_STATUS: {
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // Send MAC address query request
        status                 = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_CONNECTION_STATUS, pkt);
        rsi_response_wait_time = RSI_CONNECT_RESPONSE_WAIT_TIME;
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);

      } break;
      case RSI_WLAN_INFO: {
        if (length < sizeof(rsi_rsp_wireless_info_t)) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          return RSI_ERROR_INSUFFICIENT_BUFFER;
        }
        if (wlan_cb->opermode == RSI_WLAN_ACCESS_POINT_MODE) {
          // allocate command buffer  from wlan pool
          pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

          // If allocation of packet fails
          if (pkt == NULL) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
            // return packet allocation failure error
            return RSI_ERROR_PKT_ALLOCATION_FAILURE;
          }
#ifndef RSI_WLAN_SEM_BITMAP
          rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
          // Send cmd for wlan info in AP mode
          status                 = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_QUERY_GO_PARAMS, pkt);
          rsi_response_wait_time = RSI_WLAN_QUERY_NETWORK_PARAMS_WAIT_TIME;
          // wait on wlan semaphore
          rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);
        } else if ((wlan_cb->opermode == RSI_WLAN_CLIENT_MODE) || (wlan_cb->opermode == RSI_WLAN_ENTERPRISE_CLIENT_MODE)
                   || (wlan_cb->opermode == RSI_WLAN_CONCURRENT_MODE)) {
          // allocate command buffer  from wlan pool
          pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

          // If allocation of packet fails
          if (pkt == NULL) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
            // return packet allocation failure error
            return RSI_ERROR_PKT_ALLOCATION_FAILURE;
          }
#ifndef RSI_WLAN_SEM_BITMAP
          rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
          // Send cmd for wlan info in client mode
          status                 = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_QUERY_NETWORK_PARAMS, pkt);
          rsi_response_wait_time = RSI_WLAN_QUERY_NETWORK_PARAMS_WAIT_TIME;
          // wait on wlan semaphore
          rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);
        } else if (wlan_cb->opermode == RSI_WLAN_WIFI_DIRECT_MODE) {
          //no code here
        }
      } break;
      case RSI_STATIONS_INFO: {
        if (length < sizeof(rsi_rsp_stations_info_t)) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          return RSI_ERROR_INSUFFICIENT_BUFFER;
        }
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // Send MAC address query request
        status                 = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_QUERY_GO_PARAMS, pkt);
        rsi_response_wait_time = RSI_WLAN_REQ_QUERY_GO_PARAMS;
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);

      } break;
      case RSI_SOCKETS_INFO: {
        if (length < sizeof(rsi_rsp_sockets_info_t)) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          return RSI_ERROR_INSUFFICIENT_BUFFER;
        }

        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }
#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // Send MAC address query request
        status                 = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_QUERY_NETWORK_PARAMS, pkt);
        rsi_response_wait_time = RSI_WLAN_QUERY_NETWORK_PARAMS_WAIT_TIME;
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);

      } break;
      case RSI_CFG_GET: {
        if (length < sizeof(rsi_cfgGetFrameRcv_t)) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          return RSI_ERROR_INSUFFICIENT_BUFFER;
        }
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // Send CFG_GET command for getting user store configuration.
        status                 = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_GET_CFG, pkt);
        rsi_response_wait_time = RSI_WLAN_REQ_GET_CFG_WAIT_TIME;
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);

      } break;
      case RSI_GET_WLAN_STATS: {

        if (length < sizeof(rsi_rsp_wlan_stats_t)) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          return RSI_ERROR_INSUFFICIENT_BUFFER;
        }
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }
#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // Send Wifi STATS query request
        status                 = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_GET_STATS, pkt);
        rsi_response_wait_time = RSI_WLAN_REQ_GET_STATS_WAIT_TIME;
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);

      } break;
      case RSI_FW_VERSION:
        break;
      default:
        // Return the status if command given in driver invalid state
        status = RSI_ERROR_COMMAND_NOT_SUPPORTED;
        break;
    }
    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status
  return status;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_wlan_set(rsi_wlan_set_cmd_t cmd_type, 
 *									 uint8_t *request, uint16_t length)
 * @brief       Requested configuration based on the command type. 
 * @pre   		\ref rsi_wireless_init() API needs to be called before this API. \n 
 *        		For setting MAC address, call this API immediately after \n
 *		  		\ref rsi_wireless_init() and before calling any other API. 
 * @param[in]  cmd_type - 	Set command type:  : \n
 *                       	1 : RSI_SET_MAC_ADDRESS \n
 *                       	2 : RSI_MULTICAST_FILTER \n
 *                      	3 : RSI_JOIN_BSSID 
 * @param[in]  request  -   Request buffer 
 * @param[in]  length   -   Length of the request buffer in bytes  
 * @return      Non-Zero Value  - Failure \n
 *                    0 		- Success \n
 *              -2 - Invalid parameters \n
 *				-3 - Command given in wrong state \n
 *				-4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 *
 */
int32_t rsi_wlan_set(rsi_wlan_set_cmd_t cmd_type, uint8_t *request, uint16_t length)
{
  //This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(length);
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  rsi_user_store_config_t *user_store_ptr = NULL;
  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb         = rsi_driver_cb->wlan_cb;
  int32_t rsi_response_wait_time = 0;
  // Get common control block structure pointer
  rsi_common_cb_t *common_cb = rsi_driver_cb->common_cb;

  // if state is not in card ready received state
  if (common_cb->state == RSI_COMMON_STATE_NONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  if (!(cmd_type == 4)) {
    if (request == NULL) {
      // Command given in wrong state
      return RSI_ERROR_INVALID_PARAM;
    }
  }

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    switch (cmd_type) {
      case RSI_SET_MAC_ADDRESS: {
        // enable valid bit to ensure setting mac id before every initilization
        wlan_cb->field_valid_bit_map |= RSI_SET_MAC_BIT;

        // copy mac address in to wlan control block
        memcpy(wlan_cb->mac_address, request, 6);

      } break;

      case RSI_MULTICAST_FILTER: {
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);
        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // Memset data
        memset(&pkt->data, 0, sizeof(rsi_req_multicast_filter_info_t));

        // copy mac address
        memcpy((uint8_t *)&pkt->data, request, sizeof(rsi_req_multicast_filter_info_t));

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // Set  Multi cast filter configuration
        status                 = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_MULTICAST_FILTER, pkt);
        rsi_response_wait_time = RSI_MULTICAST_FIL_RESPONSE_WAIT_TIME;
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);
      } break;

      case RSI_JOIN_BSSID: {

        // copy mac address in to wlan control block
        memcpy(rsi_wlan_cb_non_rom->join_bssid_non_rom, request, 6);

      } break;

      case RSI_CFG_SAVE: {
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }
#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // Send CFG_SAVE command for Saving user store configuration.
        status                 = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_CFG_SAVE, pkt);
        rsi_response_wait_time = RSI_WLAN_CFG_SAVE_WAIT_TIME;
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);

      } break;
      case RSI_CFG_STORE: {
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }
        user_store_ptr = (rsi_user_store_config_t *)pkt->data;
        // Memset buffer
        memset(&pkt->data, 0, sizeof(rsi_user_store_config_t));
        // copy user store configuration parameter to the packet .
        memcpy(user_store_ptr, (rsi_user_store_config_t *)request, sizeof(rsi_user_store_config_t));
#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send psk command
        status                 = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_USER_STORE_CONFIG, pkt);
        rsi_response_wait_time = RSI_USER_SC_RESPONSE_WAIT_TIME;
        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, rsi_response_wait_time);

      } break;

      default:
        // Return the status if command given in driver invalid state
        status = RSI_ERROR_COMMAND_NOT_SUPPORTED;
        break;
    }
    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status
  return status;
}

/*==============================================*/
/**
 * @fn       int32_t rsi_wlan_buffer_config(void)
 * @brief    Configure the tx ,rx global buffers ratio. 
 * @pre    	\ref rsi_wlan_buffer_config() API needs to be called after opermode command only. 
 * @return       Non-Zero Value  - Failure \n
 *                    0 		- Success \n 
 *               -2 - Invalid parameters \n
 *				 -3 - Command given in wrong state \n
 *				 -4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 *
 */
int32_t rsi_wlan_buffer_config(void)
{

  rsi_pkt_t *pkt;
  rsi_udynamic *dyn_buf;
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Take the user provided data and fill it in antenna select structure
    dyn_buf = (rsi_udynamic *)pkt->data;

    // pll mode value
    dyn_buf->dynamic_tx_pool = TX_POOL_RATIO;

    // rf type
    dyn_buf->dynamic_rx_pool = RX_POOL_RATIO;

    // wireless mode
    dyn_buf->dynamic_global_pool = GLOBAL_POOL_RATIO;

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send antenna select command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_DYNAMIC_POOL, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_DYNAMIC_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status
  return status;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_ap_start(int8_t *ssid, uint8_t channel, 
 *										rsi_security_mode_t security_type,rsi_encryption_mode_t encryption_mode, 
 *										uint8_t *password, uint16_t beacon_interval, uint8_t dtim_period)
 * @brief      Start the module in Access point mode with the given configuration.
 * @pre    	   \ref rsi_wireless_init() API needs to be called before this API.
 * @param[in]  ssid, This is the SSID of the Access Point. The length of the SSID should be less than or equal to 32 bytes. 
 * @param[in]  channel, This is the channel number. Refer the following channels for the valid channel numbers supported. \n
 *                      2.4GHz Band Channel Mapping, 5GHz Band Channel Mappin \n
 *                      channel number 0 is to enable ACS feature  
 * @param[in]  security_type - This is the type of the security modes on which an access point needs to be operated. \n
 *                         1 : RSI_OPEN \n
 *                         2 : RSI_WPA \n
 *                         3 : RSI_WPA2 \n
 *                         4 : RSI_WPA_WPA2_MIXED \n
 *                         5 : RSI_WPS_PUSH_BUTTON 
 * @param[in]  encryption_mode - This is the type of the encryption mode \n
 *                         0 : RSI_NONE \n
 *                         1 : RSI_TKIP \n
 *                         2 : RSI_CCMP
 * @param[in]  password - This is the PSK to be used in security mode. \n
 *                      Minimum and maximum length of PSK is 8 bytes and 63 bytes respectively
 * @param[in]  beacon_interval - This is the beacon interval   
 * @param[in]  dtim_period - This is the DTIM period 
 *
 * @return     Non-Zero Value  - Failure \n
 *                    0 		- Success\n
 *              -2 - Invalid parameters \n
 *				-3 - Command given in wrong state \n
 *				-4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 *
 */
int32_t rsi_wlan_ap_start(int8_t *ssid,
                          uint8_t channel,
                          rsi_security_mode_t security_type,
                          rsi_encryption_mode_t encryption_mode,
                          uint8_t *password,
                          uint16_t beacon_interval,
                          uint8_t dtim_period)
{

  rsi_pkt_t *pkt;
  rsi_req_join_t *join;
  int32_t status = RSI_SUCCESS;

  rsi_req_ap_config_t *ap_config;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    switch (wlan_cb->state) {
      case RSI_WLAN_STATE_OPERMODE_DONE: {
        if (wlan_cb->field_valid_bit_map & RSI_SET_MAC_BIT) {
          // allocate command buffer  from wlan pool
          pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

          // If allocation of packet fails
          if (pkt == NULL) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
            // return packet allocation failure error
            return RSI_ERROR_PKT_ALLOCATION_FAILURE;
          }
          // Memset data
          memset(&pkt->data, 0, sizeof(rsi_req_mac_address_t));

          // Memcpy data
          memcpy(&pkt->data, wlan_cb->mac_address, sizeof(rsi_req_mac_address_t));

#ifndef RSI_WLAN_SEM_BITMAP
          rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
          // send set mac command
          status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_MAC_ADDRESS, pkt);

          // wait on wlan semaphore
          rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_MAC_RESPONSE_WAIT_TIME);

          // get wlan/network command response status
          status = rsi_wlan_get_status();

          if (status != RSI_SUCCESS) {
            //Changing the wlan cmd state to allow
            rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

            // Return the status if error in sending command occurs
            return status;
          }
        }
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // Memset data
        memset(&pkt->data, 0, sizeof(rsi_req_band_t));

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send band command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_BAND, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_BAND_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          // If BAND command fails
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
      }
        //no break
      // fall through
      case RSI_WLAN_STATE_BAND_DONE: {
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }
#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send init command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_INIT, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_INIT_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
#if RSI_SET_REGION_AP_SUPPORT
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // Memset data
        memset(&pkt->data, 0, sizeof(rsi_req_set_region_ap_t));

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send set region AP command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_REGION_AP, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_REGION_AP_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
#endif
      }
        //no break
      // fall through
      case RSI_WLAN_STATE_IP_CONFIG_DONE:
      case RSI_WLAN_STATE_IPV6_CONFIG_DONE:
      case RSI_WLAN_STATE_INIT_DONE: {
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }
        // Memset packet
        memset(&pkt->data, 0, sizeof(rsi_req_ap_config_t));

        ap_config = (rsi_req_ap_config_t *)pkt->data;

        // copy channel by uint16 to 2 bytes conversion
        rsi_uint16_to_2bytes(ap_config->channel, channel);

        if (ssid != NULL) {
          // Copy Join SSID
          rsi_strcpy(ap_config->ssid, ssid);
        }

        // security type
        ap_config->security_type = security_type;

        // encryption mode
        ap_config->encryption_mode = encryption_mode;

        // If security is enabled
        if (security_type) {
          // copy psk
          rsi_strcpy(ap_config->psk, password);
        }

        // copy beacon interval by uint16 to 2 bytes conversion
        rsi_uint16_to_2bytes(ap_config->beacon_interval, beacon_interval);

        // copy dtim period by uint16 to 2 bytes conversion
        rsi_uint16_to_2bytes(ap_config->dtim_period, dtim_period);

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send AP config command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_AP_CONFIGURATION, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_AP_CONFIG_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        // check status for AP config command
        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }

#if RSI_MODE_11N_ENABLE
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }

        // Memset packet
        memset(&pkt->data, 0, sizeof(rsi_req_ap_ht_caps_t));

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send HT capabilities command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_HT_CAPABILITIES, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_HT_CAPS_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }
#endif
        // allocate command buffer  from wlan pool
        pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

        // If allocation of packet fails
        if (pkt == NULL) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
          // return packet allocation failure error
          return RSI_ERROR_PKT_ALLOCATION_FAILURE;
        }
        // memset the pkt
        memset(&pkt->data, 0, sizeof(rsi_req_join_t));

        join = (rsi_req_join_t *)pkt->data;

        if (ssid != NULL) {
          // Copy ssid and ssid len
          join->ssid_len = rsi_strlen(ssid);

          // Copy Join SSID
          rsi_strcpy(join->ssid, ssid);
        }

        // vap id of the current mode 0 - station mode, 1 - AP1 mode
        //  Applicable in cocurrent mode only
#if CONCURRENT_MODE
        join->vap_id = 1;
#else
        join->vap_id = 0;
#endif

        // Deregister join response handler
        rsi_wlan_cb_non_rom->callback_list.wlan_join_response_handler = NULL;

#ifndef RSI_WLAN_SEM_BITMAP
        rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
        // send join command
        status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_JOIN, pkt);

        // wait on wlan semaphore
        rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_JOIN_RESPONSE_WAIT_TIME);

        // get wlan/network command response status
        status = rsi_wlan_get_status();

        if (status != RSI_SUCCESS) {
          //Changing the wlan cmd state to allow
          rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

          // Return the status if error in sending command occurs
          return status;
        }

      } break;

      case RSI_WLAN_STATE_NONE:
      case RSI_WLAN_STATE_SCAN_DONE:
      case RSI_WLAN_STATE_CONNECTED:
      case RSI_WLAN_STATE_AUTO_CONFIG_GOING_ON:
      case RSI_WLAN_STATE_AUTO_CONFIG_DONE:
      case RSI_WLAN_STATE_AUTO_CONFIG_FAILED:

        break;
      default: {
        // Return the status if command given in driver invalid state
        status = RSI_ERROR_COMMAND_NOT_SUPPORTED;
      } break;
    }
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return nwk command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_wlan_power_save_with_listen_interval(uint8_t psp_mode, uint8_t psp_type,uint16_t listen_interval)
 * @brief       Set the power save profile in wlan mode with listen interval-based wakeup.
 * @pre         \ref rsi_wireless_init() API needs to be called before this API. 
 * @param[in]   psp_mode - The options are : \n
 *              0 - Active \n
 *              1 - Sleep without SoC turn off when connected else disconnected deep sleep \n
 *              2 - Sleep with SoC turn off (connected/disconnected) \n
 *              8 - Deep sleep
 * @param[in]   psp_type - The options are : \n
 *              0 - Max power save \n
 *              1 - Fast PSP \n
 *              2 - UAPSD 
 * @param[in]   listen interval - This parameter is used to configure maximum sleep duration in power save. \n
 *								  Note:
 *								  This is valid only if BIT (7) in join_feature_bit_map is set. This value is given in time units (1024 microsecond). \n 
 *								  This parameter is used to configure. \n
 *								  maximum sleep duration in power save. and should be less than the listen interval configured in join command.
 * @note 		1. psp_type is only valid in psp_mode 1 and 2. \n
 *				2. psp_type UAPSD is applicable only if WMM_PS is enabled in \ref rsi_wlan_config.h file. \n
 *				3. In RSI_MAX_PSP mode, Few Access points won't aggregate the packets, when power save is enabled from STA. This may cause the drop in throughputs.
 * @note		For the power mode 3/9 select the RSI_HAND_SHAKE_TYPE as MSG_BASED.
 * @return      Non-Zero Value  - Failure \n
 *                    0 		- Success \n
 *              -2 - Invalid parameters \n
 *				-3 - Command given in wrong state \n
 *				-4 - Buffer not availableto serve the command
 *
 *
 */

int32_t rsi_wlan_power_save_with_listen_interval(uint8_t psp_mode, uint8_t psp_type, uint16_t listen_interval)
{
  int32_t status = RSI_SUCCESS;

  //Get commmon cb pointer
  rsi_common_cb_t *rsi_common_cb = rsi_driver_cb->common_cb;

  //keep backup of power save profile mode and type
  rsi_common_cb->power_save.wlan_psp_type = psp_type;

  //keep backup of power save profile mode and type
  rsi_common_cb->power_save.wlan_psp_mode = psp_mode;

  rsi_wlan_cb_non_rom->ps_listen_interval = listen_interval;

  status = rsi_sleep_mode_decision(rsi_common_cb);

  return status;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_wlan_power_save_profile(uint8_t psp_mode, uint8_t psp_type)
 * @brief       Set the power save profile in wlan mode.
 * @pre         \ref rsi_wireless_init() API needs to be called before this API. 
 * @param[in]   psp_mode - The options are : \n
 *              		0 - Active \n
 *              		1 - Sleep without SoC turn off when connected else disconnected deep sleep \n
 *              		2 - Sleep with SoC turn off (connected/disconnected) \n
 *              		8 - Deep sleep
 * @param[in]   psp_type - Follwing psp_type is defined.
 *             
 * @note		1. psp_type is only valid in psp_mode 1 and 2. \n
 *				2. psp_type UAPSD is applicable only if WMM_PS is enabled in rsi_wlan_config.h file. \n
 *				3. In RSI_MAX_PSP mode, Few Access points won't aggregate the packets, when power save is enabled \n
 *				   from STA. This may cause the drop in throughputs.
 * @note		For the power mode 3/9 select the RSI_HAND_SHAKE_TYPE as MSG_BASED.
 * @note		Powersave handshake option: \n
 *					- When sleep clock source is configured to '32KHz bypass clock on UULP_VBAT_GPIO_3', \n
 *						 use UULP_VBAT_GPIO_0 for SLEEP_IND_FROM_DEV \n
 *						 set RS9116_SILICON_CHIP_VER in 'RS9116.NB0.WC.GENR.OSI.X.X.X\host\sapis\include\rsi_user.h' to 'CHIP_VER_1P4_AND_ABOVE' \n
 *					- If not using external clock on UULP_VBAT_GPIO_3' as sleep clock source, \n
 *						 use UULP_VBAT_GPIO_3 for SLEEP_IND_FROM_DEV \n
 *						 set RS9116_SILICON_CHIP_VER in 'RS9116.NB0.WC.GENR.OSI.X.X.X\host\sapis\include\rsi_user.h' to 'CHIP_VER_1P3'
 * @return      Non-Zero Value  - Failure \n
 *                    0 		- Success \n
 *              -2 - Invalid parameters \n
 *				-3 - Command given in wrong state \n
 *				-4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 *
 */

int32_t rsi_wlan_power_save_profile(uint8_t psp_mode, uint8_t psp_type)
{
  int32_t status = RSI_SUCCESS;
  status         = rsi_wlan_power_save_with_listen_interval(psp_mode, psp_type, 0);
  return status;
}
/*==============================================*/
/**
 * @fn        int32_t rsi_wlan_power_save_disable_and_enable(uint8_t psp_mode, uint8_t psp_type)
 * @brief 	  Stop the transmit test.
 * @param[in]   psp_mode - power save mode  
 * @param[in]   psp_tye -  power save type
 * @return      Non-Zero Value  - Failure \n
 *                    0 		- Success \n
 *
 *
 *
 */
int32_t rsi_wlan_power_save_disable_and_enable(uint8_t psp_mode, uint8_t psp_type)
{
  int32_t status = RSI_SUCCESS;
  status         = rsi_wlan_power_save_profile(RSI_ACTIVE, psp_type);
  if (status != RSI_SUCCESS) {
    // Return the status if error in sending command occurs
    return status;
  }
  status = rsi_wlan_power_save_profile(psp_mode, psp_type);
  if (status != RSI_SUCCESS) {
    // Return the status if error in sending command occurs
    return status;
  }
  return status;
}

/*==============================================*/
/**
 * @fn       int32_t rsi_transmit_test_start(uint16_t power, uint32_t rate, uint16_t length, uint16_t mode, uint16_t channel)
 * @brief    Start the transmit test.
 * @note	 This API is relevant in PER mode.
 * @pre      \ref rsi_wlan_radio_init() API needs to be called before this API. 
 * @param[in]  power  - To set TX power in dbm. The valid values are from 2dbm to 18dbm for WiSeConnect module. 
 * @param[in]  rate   - To set transmit data rate
 * @param[in]  length - To configure length of the TX packet. \n
 *             The valid values are in the range of 24 to 1500 bytes in the burst mode and range of 24 to 260 bytes in the continuous mode. 
 * @param[in]  mode   - 0- Burst Mode \n
 *				        1- Continuous Mode \n
 *						2- Continuous wave Mode (non modulation) in DC mode \n
 *						3- Continuous wave Mode (non modulation) in single tone mode (center frequency -2.5MHz) \n
 *						4- Continuous wave Mode (non modulation) in single tone mode (center frequency +5MHz)  
 * @param[in] channel - For setting the channel number in 2.4 GHz / 5GHz.  
 * @note	  Before starting Continuous Wave mode, it is required to start Burst mode with power and channel values which \n
 *			  is intended to be used in Continuous Wave mode i.e. \n
 *				1. Start Burst mode with intended power value and channel values \n
 *				2. Pass any valid values for rate and length \n
 *				3. Stop Burst mode \n
 *				4. Start Continuous wave mode
 * @return      0 		- Success \n
 *              Non-Zero Value  - Failure \n
 *				-4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 */

int32_t rsi_transmit_test_start(uint16_t power, uint32_t rate, uint16_t length, uint16_t mode, uint16_t channel)
{
  rsi_pkt_t *pkt;
  rsi_req_tx_test_info_t *tx_test_info;
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  // Check whether transmit test mode is selescted
  if (wlan_cb->opermode != RSI_WLAN_TRANSMIT_TEST_MODE) {
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }

  // Wlan radio init
  status = rsi_wlan_radio_init();

  if (status != RSI_SUCCESS) {
    // Return the status if error in sending command occurs
    return status;
  }

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Fill tx test info parameters
    tx_test_info = (rsi_req_tx_test_info_t *)pkt->data;

    // memset the pkt
    memset(&pkt->data, 0, sizeof(rsi_req_tx_test_info_t));

    // enable transmit test
    rsi_uint16_to_2bytes(tx_test_info->enable, 1);

    // configure transmit power of tx test
    rsi_uint16_to_2bytes(tx_test_info->power, power);

    // configure transmit rate of tx test
    rsi_uint32_to_4bytes(tx_test_info->rate, rate);

    // configure packet length of tx test
    rsi_uint16_to_2bytes(tx_test_info->length, length);

    // configure transmit mode of tx test
    rsi_uint16_to_2bytes(tx_test_info->mode, mode);

    // configure channel of tx test
    rsi_uint16_to_2bytes(tx_test_info->channel, channel);

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send  command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_TX_TEST_MODE, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_TRANSMIT_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn        int32_t rsi_transmit_test_stop(void)
 * @brief 	  Stop the transmit test.
 * @note	  This API is relevant in PER mode. 
 * @pre       \ref rsi_wlan_radio_init() API needs to be called before this API. 
 * @param[in]   void    
 * @return      Non-Zero Value  - Failure \n
 *                    0 		- Success \n
 *				-4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes  \ref SP16.
 *
 *
 *
 */

int32_t rsi_transmit_test_stop(void)
{
  rsi_pkt_t *pkt;
  rsi_req_tx_test_info_t *tx_test_info;
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Fill tx test info parameters
    tx_test_info = (rsi_req_tx_test_info_t *)pkt->data;

    // memset the pkt
    memset(&pkt->data, 0, sizeof(rsi_req_tx_test_info_t));

    // configure to stop tx test
    rsi_uint16_to_2bytes(tx_test_info->enable, 0);

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send  command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_TX_TEST_MODE, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_TRANSMIT_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn        int32_t rsi_wlan_receive_stats_start(uint16_t channel)
 * @brief     Get the Transmit (TX) & Receive (RX) packets statistics.When this API is called by the host with 
 *            valid channel number, the module gives the statistics to the host for every 1 second asynchronously. 
 * @pre       \ref rsi_wireless_init() API needs to be called before this API. 
 * @param[in] channel - Valid channel number 2.4GHz or 5GHz          
 * @return    Non-Zero Value  - Failure \n
 *                    0 		- Success 
 * 
 */

int32_t rsi_wlan_receive_stats_start(uint16_t channel)
{
  rsi_pkt_t *pkt;
  rsi_req_rx_stats_t *rx_stats;
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Fill rx stats  parameters
    rx_stats = (rsi_req_rx_stats_t *)pkt->data;

    // memset the pkt
    memset(&pkt->data, 0, sizeof(rsi_req_rx_stats_t));

    // configure to start rx stats
    rsi_uint16_to_2bytes(rx_stats->start, 0);

    // copy the channel number
    rsi_uint16_to_2bytes(rx_stats->channel, channel);

    if (rsi_wlan_cb_non_rom->callback_list.wlan_receive_stats_response_handler == NULL) {
#ifndef RSI_WLAN_SEM_BITMAP
      rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    }
    // send  command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_RX_STATS, pkt);

    if (rsi_wlan_cb_non_rom->callback_list.wlan_receive_stats_response_handler == NULL) {
      // wait on wlan semaphore
      rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_RX_STATS_RESPONSE_WAIT_TIME);
      // get wlan/network command response status
      status = rsi_wlan_get_status();
    }
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn      int32_t rsi_wlan_receive_stats_stop(void)
 * @brief   Stop the Transmit (TX) & Receive(RX) packets statistics. 
 * @pre     \ref rsi_wireless_init() API needs to be called before this API. 
 * @param[in]         void 
 * @return      Non-Zero Value  - Failure \n
 *                    0 		- Success \n
 *				-4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16.
 */

int32_t rsi_wlan_receive_stats_stop(void)
{
  rsi_pkt_t *pkt;
  rsi_req_rx_stats_t *rx_stats;
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Fill rx stats  parameters
    rx_stats = (rsi_req_rx_stats_t *)pkt->data;

    // memset the pkt
    memset(&pkt->data, 0, sizeof(rsi_req_rx_stats_t));

    // configure to stop rx stats
    rsi_uint16_to_2bytes(rx_stats->start, 1);

    if (rsi_wlan_cb_non_rom->callback_list.wlan_receive_stats_response_handler == NULL) {
#ifndef RSI_WLAN_SEM_BITMAP
      rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    }
    // send  command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_RX_STATS, pkt);

    if (rsi_wlan_cb_non_rom->callback_list.wlan_receive_stats_response_handler == NULL) {
      // wait on wlan semaphore
      rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_RX_STATS_RESPONSE_WAIT_TIME);

      // get wlan/network command response status
      status = rsi_wlan_get_status();
    }

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn     int32_t rsi_wlan_wfd_start_discovery(uint16_t go_intent, int8_t *device_name, uint16_t channel, int8_t *ssid_post_fix, 
 *												uint8_t *psk, void (*wlan_wfd_discovery_notify_handler)(uint16_t status, 
 *												uint8_t *buffer, const uint32_t length),
 *                                              void (*wlan_wfd_connection_request_notify_handler)(uint16_t status, uint8_t *buffer, const uint32_t length))												const uint32_t length))
 * @brief      Start discovery in wifi direct mode.
 * @param[in]  go_intent     - group owner intent ,integer value 0 <= go_intent <= 16       
 * @param[in]  device_name   - device name during discovery phase for other wfd devices 
 * @param[in]  channel       - operating channel number 
 * @param[in]  ssid_post_fix - post fix SSID of the device when turned up as GO 
 * @param[in]  psk           - Preshared key 
 * @param[in]  void(*wlan_wfd_discovery_notify_handler)(uint16_t status,const uint8_t *buffer, const uint16_t length), \n
 *             callback to notify discoverd wfd devices
 * @param[in]  void(*wlan_wfd_connection_request_notify_handler)(uint16_t status,const uint8_t *buffer, const uint16_t length)) \n
               callback to notify connection request received from remote devices
 * @return      Non-Zero Value  - Failure \n
 *                    0 		- Success
 *
 */

int32_t rsi_wlan_wfd_start_discovery(
  uint16_t go_intent,
  int8_t *device_name,
  uint16_t channel,
  int8_t *ssid_post_fix,
  uint8_t *psk,
  void (*wlan_wfd_discovery_notify_handler)(uint16_t status, uint8_t *buffer, const uint32_t length),
  void (*wlan_wfd_connection_request_notify_handler)(uint16_t status, uint8_t *buffer, const uint32_t length))
{
  rsi_pkt_t *pkt;
  int32_t status = RSI_SUCCESS;
  rsi_req_configure_p2p_t *config_p2p;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  // Check whether WiFi-Direct mode is selescted
  if (wlan_cb->opermode != RSI_WLAN_WIFI_DIRECT_MODE) {
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }

  // Wlan radio init
  status = rsi_wlan_radio_init();

  if (status != RSI_SUCCESS) {
    // Return the status if error in sending command occurs
    return status;
  }

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    config_p2p = (rsi_req_configure_p2p_t *)pkt->data;

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_req_configure_p2p_t));

    // configure go intent
    rsi_uint16_to_2bytes(config_p2p->go_intent, go_intent);

    if (device_name != NULL) {
      // Copy device name
      rsi_strcpy(config_p2p->device_name, device_name);
    }

    // configure operating channel
    rsi_uint16_to_2bytes(config_p2p->operating_channel, channel);

    if (ssid_post_fix != NULL) {
      // Copy ssid post fix
      rsi_strcpy(config_p2p->ssid_post_fix, ssid_post_fix);
    }

    if (psk != NULL) {
      // Copy psk
      rsi_strcpy(config_p2p->psk, psk);
    }

    if (wlan_wfd_discovery_notify_handler != NULL) {
      rsi_wlan_cb_non_rom->callback_list.wlan_wfd_discovery_notify_handler = wlan_wfd_discovery_notify_handler;
    }

    if (wlan_wfd_connection_request_notify_handler != NULL) {
      rsi_wlan_cb_non_rom->callback_list.wlan_wfd_connection_request_notify_handler =
        wlan_wfd_connection_request_notify_handler;
    }
    if ((wlan_wfd_discovery_notify_handler == NULL) && (wlan_wfd_connection_request_notify_handler == NULL)) {
#ifndef RSI_WLAN_SEM_BITMAP
      rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    }
    // send wifi direct discovery command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_CONFIGURE_P2P, pkt);

    if ((wlan_wfd_discovery_notify_handler == NULL) && (wlan_wfd_connection_request_notify_handler == NULL)) {
      // wait on wlan semaphore
      rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_P2P_RESPONSE_WAIT_TIME);
      // get wlan/network command response status
      status = rsi_wlan_get_status();
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
    }

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn          int32_t rsi_wlan_wfd_connect(int8_t *device_name, 
 *                 void (*join_response_handler)(uint16_t status, const uint8_t *buffer, const uint16_t length))
 * @brief       Connect to wfd device with given device name
 * @param[in]   device_name           - remote device name if GO intent in configuration is < 16, \n
 *                                      rsi module device_name to create as auto GO.
 * @param[in]   join_response_handler - asynchronous response to the connect api          
 * @param[out]  <nodetype> \n
 *              'G' - GO \n
 *              'C' - client 
 * @return      0  -  Success \n
 *              Non-Zero Value - Failure 
 *              
 *
 */

int32_t rsi_wlan_wfd_connect(int8_t *device_name,
                             void (*join_response_handler)(uint16_t status,
                                                           const uint8_t *buffer,
                                                           const uint16_t length))
{
  rsi_pkt_t *pkt;
  rsi_req_join_t *join;
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_req_join_t));

    join = (rsi_req_join_t *)pkt->data;

    if (device_name != NULL) {
      // Copy ssid and ssid len
      join->ssid_len = rsi_strlen(device_name);

      // Copy Join SSID
      rsi_strcpy(join->ssid, device_name);
    }

    if (join_response_handler != NULL) {
      // Register scan response handler
      rsi_wlan_cb_non_rom->callback_list.wlan_join_response_handler = join_response_handler;
    }

    if (join_response_handler == NULL) {
#ifndef RSI_WLAN_SEM_BITMAP
      rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    }
    // send join command to start wps
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_JOIN, pkt);

    if (join_response_handler == NULL) {
      // wait on wlan semaphore
      rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_JOIN_RESPONSE_WAIT_TIME);
      // get wlan/network command response status
      status = rsi_wlan_get_status();
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
    }

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_send_data(uint8_t* buffer, uint32_t length)
 * @brief      Send the raw data in TCP / IP bypass mode.
 * @param[in]  buffer - This is the pointer to the buffer to send 
 * @param[in]  length - This is the length of the buffer to send 
 * @return      0  -  Success \n
 *              Non-Zero Value - Failure \n
 *              -2 - Invalid parameters \n
 *				-4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16.
 *
 *
 */

int32_t rsi_wlan_send_data(uint8_t *buffer, uint32_t length)
{
  int32_t status = RSI_SUCCESS;
  uint8_t *host_desc;
  rsi_pkt_t *pkt;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_CONNECTED) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  // If buffer is not valid
  if ((buffer == NULL) || (length == 0)) {
    // return packet allocation failure error
    return RSI_ERROR_INVALID_PARAM;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // Allocate packet to send data
    pkt = rsi_pkt_alloc(&rsi_driver_cb->wlan_cb->wlan_tx_pool);

    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Get host descriptor pointer
    host_desc = pkt->desc;

    // Memset host descriptor
    memset(host_desc, 0, RSI_HOST_DESC_LENGTH);

    // Fill host descriptor
    rsi_uint16_to_2bytes(host_desc, (length & 0xFFF));

    // Fill packet type
    host_desc[1] |= (RSI_WLAN_DATA_Q << 4);
    host_desc[2] |= 0x01;

    // Copy data to be sent
    memcpy(pkt->data, buffer, length);

    // Enqueue packet to WLAN TX queue
    rsi_enqueue_pkt(&rsi_driver_cb->wlan_tx_q, pkt);

#ifndef RSI_SEND_SEM_BITMAP
    rsi_driver_cb_non_rom->send_wait_bitmap |= BIT(0);
#endif
    // Set TX packet pending event
    rsi_set_event(RSI_TX_EVENT);

    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->send_data_sem, RSI_SEND_DATA_RESPONSE_WAIT_TIME);

    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
  }
  // Return status
  return status;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_ping_async(uint8_t flags, uint8_t *ip_address, uint16_t size, 
 *                                         void (*wlan_ping_response_handler)(uint16_t status,
 *                                                                            const uint8_t *buffer, const uint16_t length))
 * @brief      Send the ping request to the target IP address. 
 * @pre    	   \ref rsi_config_ipaddress() API needs to be called before this API. 
 * @param[in]  flags                      - BIT(0) RSI_IPV6 Set this bit to enable IPv6; by default it is configured to IPv4 
 * @param[in]  ip_address                 - target IP address \n
 *						                    IPv4 address  4 Bytes hexa-decimal, \n
 *   					                    IPv6 address  16 Bytes hexa-decimal  
 * @param[in]  size                       - ping data size to send. Maximum supported is 300 bytes. 
 * @param[in]  wlan_ping_response_handler - This callback is called when ping response has been received from the module. \n
 *                                          The parameters involved are status, buffer & length. 
 * @param[in]  buffer                     - This is the response buffer. 
 * @param[in]  length                     - This is the response buffer length. 
 * @return      0  -  Success \n
 *              Non-Zero Value - Failure \n
 *              -2 - Invalid parameters \n
 *				-4 - Buffer not availableto serve the command
 * @note        Please refer to Error Codes section for the description of the above error codes \ref SP16.
 *
 *
 */

int32_t rsi_wlan_ping_async(uint8_t flags,
                            uint8_t *ip_address,
                            uint16_t size,
                            void (*wlan_ping_response_handler)(uint16_t status,
                                                               const uint8_t *buffer,
                                                               const uint16_t length))
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  rsi_req_ping_t *ping;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;
  if (wlan_cb->state < RSI_WLAN_STATE_IP_CONFIG_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  // If ipaddress is not valid
  if ((ip_address == NULL) || (size == 0)) {
    // return packet allocation failure error
    return RSI_ERROR_INVALID_PARAM;
  }

  status = rsi_check_and_update_cmd_state(NWK_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);

      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_req_ping_t));

    ping = (rsi_req_ping_t *)pkt->data;

    if (flags & RSI_IPV6) {
      // copy ip version
      rsi_uint16_to_2bytes(ping->ip_version, (uint16_t)RSI_IP_VERSION_6);

      // copy ipv6 address
      memcpy(ping->ping_address.ipv6_address, ip_address, RSI_IPV6_ADDRESS_LENGTH);
    } else {
      // copy ip version
      rsi_uint16_to_2bytes(ping->ip_version, (uint16_t)RSI_IP_VERSION_4);

      // copy ipv4 address
      memcpy(ping->ping_address.ipv4_address, ip_address, RSI_IPV4_ADDRESS_LENGTH);
    }

    // copy ping size
    rsi_uint16_to_2bytes(ping->ping_size, size);

    ping->timeout = RSI_PING_REQ_TIMEOUT;

    // register ping response handler
    if (wlan_ping_response_handler != NULL) {
      rsi_wlan_cb_non_rom->callback_list.wlan_ping_response_handler = wlan_ping_response_handler;
      rsi_wlan_cb_non_rom->nwk_cmd_rsp_pending |= PING_RESPONSE_PENDING;
    }

    if (wlan_ping_response_handler == NULL) {
#ifndef RSI_WLAN_SEM_BITMAP
      rsi_driver_cb_non_rom->nwk_wait_bitmap |= BIT(0);
#endif
    }
    // send ping command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_PING_PACKET, pkt);

    if (wlan_ping_response_handler == NULL) {
      // wait on wlan semaphore
      rsi_wait_on_nwk_semaphore(&rsi_driver_cb_non_rom->nwk_sem, RSI_PING_RESPONSE_WAIT_TIME);
      // get wlan/network command response status
      status = rsi_wlan_get_nwk_status();
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(NWK_CMD, ALLOW);
    }
  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn         void rsi_register_auto_config_rsp_handler(void (*rsi_auto_config_rsp_handler )(uint16_t status, uint8_t state))
 * @brief      Register Auto configuration response handler
 * @param[in]  rsi_auto_config_rsp_handler - pointer to rsi_auto_config_rsp_handler 
 * @return     void 
 */

void rsi_register_auto_config_rsp_handler(void (*rsi_auto_config_rsp_handler)(uint16_t status, uint8_t state))
{

  // Register callback handler
  rsi_wlan_cb_non_rom->callback_list.auto_config_rsp_handler = rsi_auto_config_rsp_handler;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_add_profile(uint32_t type, uint8_t *profile)
 * @brief      Add profile for auto configuration 
 * @pre        \ref rsi_wireless_init() API needs to be called before this API. 
 * @param[in]  type - profile type. Supported profile types are \n
 *  							1.RSI_WLAN_PROFILE_AP, \n
 *								2. RSI_WLAN_PROFILE_CLIENT, \n
 *								3.RSI_WLAN_PROFILE_EAP, \n
 *								4. RSI_WLAN_PROFILE_P2P, \n
 *								5.RSI_WLAN_PROFILE_ALL 
 * @param[in]  profile - pointer to config profile Config profile structure \n
 *						ap_profile, eap_client_profile_t, client_profile_t, p2p_profile_t, rsi_config_profile_t 
 * @return      0  -  Success \n
 *              Non-Zero Value - Failure\n
 *				-4 - Buffer not availableto serve the command
 *
 */

int32_t rsi_wlan_add_profile(uint32_t type, uint8_t *profile)
{
  int32_t status = RSI_SUCCESS;
  rsi_pkt_t *pkt;
  uint16_t send_size = 0;
  uint8_t *host_desc = NULL;
  rsi_config_profile_t *config_profile;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_config_profile_t));

    config_profile = (rsi_config_profile_t *)pkt->data;

    // Fill profile type
    rsi_uint32_to_4bytes(config_profile->profile_type, type);

    // Fill wlan profile
    if (type == RSI_WLAN_PROFILE_AP) {
      memcpy(&config_profile->wlan_profile_struct.ap_profile, (uint8_t *)profile, sizeof(ap_profile_t));
    } else if (type == RSI_WLAN_PROFILE_CLIENT) {
      memcpy(&config_profile->wlan_profile_struct.client_profile, (uint8_t *)profile, sizeof(client_profile_t));
    } else if (type == RSI_WLAN_PROFILE_EAP) {
      memcpy(&config_profile->wlan_profile_struct.eap_client_profile, (uint8_t *)profile, sizeof(eap_client_profile_t));
    } else if (type == RSI_WLAN_PROFILE_P2P) {
      memcpy(&config_profile->wlan_profile_struct.p2p_profile, (uint8_t *)profile, sizeof(p2p_profile_t));
    } else {
      // no code here
    }

    // Using host descriptor to set payload length
    send_size = sizeof(rsi_config_profile_t);

    // get the host descriptor
    host_desc = (pkt->desc);

    // Fill data length in the packet host descriptor
    rsi_uint16_to_2bytes(host_desc, (send_size & 0xFFF));

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send set profile command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_PROFILE, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_SET_PROFILE_RESPONSE_WAIT_TIME);
    // get wlan/network command response status
    status = rsi_wlan_get_status();

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn         uint8_t rsi_wlan_get_state(void)
 * @brief      Get the current wlan state. 
 * @param[in]  void
 * @return     state
 */

uint8_t rsi_wlan_get_state(void)
{
  // return wlan state
  return rsi_driver_cb->wlan_cb->state;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_get_profile(uint32_t type, rsi_config_profile_t *profile_rsp, uint16_t length)
 * @brief      Get the stored config profile. 
 * @pre  	   \ref rsi_wireless_init() API needs to be called before this API. 
 * @param[in]  type - config profile type. Supported profile types are \n
 *  				RSI_WLAN_PROFILE_AP, \n
 *					RSI_WLAN_PROFILE_CLIENT, \n
 *					RSI_WLAN_PROFILE_EAP, \n 
 *					RSI_WLAN_PROFILE_P2P, \n 
 *					RSI_WLAN_PROFILE_ALL 
 * @param[in]  profile_rsp - Config profile response in the form of below structure \n
 *              ap_profile - eap_client_profile_t, client_profile_t, p2p_profile_t, rsi_config_profile_t   
 * @param[in]  length      - Length of the config profile response 
 * @return    0  -  Success \n
 *              Non-Zero Value - Failure \n
 *				-4 - Buffer not availableto serve the command
 *
 *
 *
 */

int32_t rsi_wlan_get_profile(uint32_t type, rsi_config_profile_t *profile_rsp, uint16_t length)
{
  int32_t status                 = RSI_SUCCESS;
  rsi_pkt_t *pkt                 = NULL;
  rsi_profile_req_t *profile_req = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_profile_req_t));

    profile_req = (rsi_profile_req_t *)pkt->data;

    // Fill profile type
    rsi_uint32_to_4bytes(profile_req->profile_type, type);

    // attach the buffer given by user
    wlan_cb->app_buffer = (uint8_t *)profile_rsp;

    // length of the buffer provided by user
    wlan_cb->app_buffer_length = length;

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send get profile command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_GET_PROFILE, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_GET_PROFILE_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn         uint8_t* rsi_fill_config_profile(uint32_t type, uint8_t *profile_buffer)
 * @brief      Fill the config profile based on the profile type.
 * @pre   	   \ref rsi_wireless_init() API needs to be called before this API.    
 * @param[in]  Type           - Profile type
 * @param[in]  Profile_buffer - pointer to profile buffer
 * @return      profile_buffer 
 *
 */

uint8_t *rsi_fill_config_profile(uint32_t type, uint8_t *profile_buffer)
{
  int i, j = 0;

  uint32_t ip_addr                             = 0;
  uint32_t mask                                = 0;
  uint32_t gw                                  = 0;
  eap_client_profile_t *eap_client_profile_ptr = NULL;
  network_profile_t *nwk_profile_ptr           = NULL;
  feature_frame_params_t *feature_frame_ptr    = NULL;
  client_profile_t *client_profile_ptr         = NULL;
  ap_profile_t *ap_profile_ptr                 = NULL;
  p2p_profile_t *p2p_profile_ptr               = NULL;

  if (type == RSI_WLAN_PROFILE_AP) {
    ip_addr = RSI_CONFIG_AP_IP_ADDRESS;
    mask    = RSI_CONFIG_AP_SN_MASK_ADDRESS;
    gw      = RSI_CONFIG_AP_GATEWAY_ADDRESS;

    ap_profile_ptr = (ap_profile_t *)profile_buffer;

    nwk_profile_ptr = &ap_profile_ptr->network_profile;

    feature_frame_ptr = &ap_profile_ptr->feature_frame_params_s;

    memset(ap_profile_ptr, 0, sizeof(ap_profile_t));

    // Fill AP wlan feature bitmap
    rsi_uint32_to_4bytes(ap_profile_ptr->wlan_feature_bit_map, RSI_CONFIG_AP_WLAN_FEAT_BIT_MAP);

    // Fill AP TCP_IP feature bitmap
    rsi_uint32_to_4bytes(ap_profile_ptr->tcp_ip_feature_bit_map, RSI_CONFIG_AP_TCP_IP_FEAT_BIT_MAP);

    // Fill AP custom feature bitmap
    rsi_uint32_to_4bytes(ap_profile_ptr->custom_feature_bit_map, RSI_CONFIG_AP_CUSTOM_FEAT_BIT_MAP);

    // Fill data rate
    ap_profile_ptr->data_rate = RSI_CONFIG_AP_DATA_RATE;

    // Fill tx power
    ap_profile_ptr->tx_power = RSI_CONFIG_AP_TX_POWER;

    // Fill AP band
    ap_profile_ptr->band = RSI_CONFIG_AP_BAND;

    // Fill AP channel number
    rsi_uint16_to_2bytes(ap_profile_ptr->channel, RSI_CONFIG_AP_CHANNEL);

    // Fill AP SSID
    rsi_strcpy(ap_profile_ptr->ssid, RSI_CONFIG_AP_SSID);

    // Fill AP security type
    ap_profile_ptr->security_type = RSI_CONFIG_AP_SECURITY_TYPE;

    // Fill AP encryption type
    ap_profile_ptr->encryption_type = RSI_CONFIG_AP_ENCRYPTION_TYPE;

    // Fill AP PSK
    if (RSI_CONFIG_AP_PSK)
      rsi_strcpy(ap_profile_ptr->psk, RSI_CONFIG_AP_PSK);

    // Fill beacon interval
    rsi_uint16_to_2bytes(ap_profile_ptr->beacon_interval, RSI_CONFIG_AP_BEACON_INTERVAL);

    // Fill DTIM period
    rsi_uint16_to_2bytes(ap_profile_ptr->dtim_period, RSI_CONFIG_AP_DTIM);

    // Fill AP keep alive time
    ap_profile_ptr->keep_alive_type = RSI_CONFIG_AP_KEEP_ALIVE_TYPE;

    // Fill AP keep alive counter
    ap_profile_ptr->keep_alive_counter = RSI_CONFIG_AP_KEEP_ALIVE_COUNTER;

    // Fill AP max number of stations
    rsi_uint16_to_2bytes(ap_profile_ptr->max_no_sta, RSI_CONFIG_AP_MAX_STATIONS_COUNT);

    nwk_profile_ptr->tcp_stack_used = RSI_CONFIG_AP_TCP_STACK_USED;

    if (nwk_profile_ptr->tcp_stack_used == 1) {
      nwk_profile_ptr->dhcp_enable = 0;

      // Fill IP address
      memcpy(nwk_profile_ptr->ip_address, (uint8_t *)&ip_addr, RSI_IPV4_ADDRESS_LENGTH);

      // Fill network mask
      memcpy(nwk_profile_ptr->sn_mask, (uint8_t *)&mask, RSI_IPV4_ADDRESS_LENGTH);

      // Fill gateway
      memcpy(nwk_profile_ptr->default_gw, (uint8_t *)&gw, RSI_IPV4_ADDRESS_LENGTH);
    } else {
      // Not supported
    }
    // Fill feature frame params
    feature_frame_ptr->pll_mode      = PLL_MODE;
    feature_frame_ptr->rf_type       = RF_TYPE;
    feature_frame_ptr->wireless_mode = WIRELESS_MODE;
    feature_frame_ptr->enable_ppp    = ENABLE_PPP;
    feature_frame_ptr->afe_type      = AFE_TYPE;
    rsi_uint32_to_4bytes(feature_frame_ptr->feature_enables, FEATURE_ENABLES);

  } else if (type == RSI_WLAN_PROFILE_CLIENT) {
    ip_addr = RSI_CONFIG_CLIENT_IP_ADDRESS;
    mask    = RSI_CONFIG_CLIENT_SN_MASK_ADDRESS;
    gw      = RSI_CONFIG_CLIENT_GATEWAY_ADDRESS;

    client_profile_ptr = (client_profile_t *)profile_buffer;

    nwk_profile_ptr = &client_profile_ptr->network_profile;

    feature_frame_ptr = &client_profile_ptr->feature_frame_params_s;

    memset(client_profile_ptr, 0, sizeof(client_profile_t));

    // Fill wlan feature bitmap
    rsi_uint32_to_4bytes(client_profile_ptr->wlan_feature_bit_map, RSI_CONFIG_CLIENT_WLAN_FEAT_BIT_MAP);

    // Fill TCP_IP feature bitmap
    rsi_uint32_to_4bytes(client_profile_ptr->tcp_ip_feature_bit_map, RSI_CONFIG_CLIENT_TCP_IP_FEAT_BIT_MAP);

    // Fill custom feature bitmap
    rsi_uint32_to_4bytes(client_profile_ptr->custom_feature_bit_map, RSI_CONFIG_CLIENT_CUSTOM_FEAT_BIT_MAP);

    // Fill listen interval
    rsi_uint32_to_4bytes(client_profile_ptr->listen_interval, RSI_CONFIG_CLIENT_LISTEN_INTERVAL);

    // Fill data rate
    client_profile_ptr->data_rate = RSI_CONFIG_CLIENT_DATA_RATE;

    // Fill tx power
    client_profile_ptr->tx_power = RSI_CONFIG_CLIENT_TX_POWER;

    // Fill AP band
    client_profile_ptr->band = RSI_CONFIG_CLIENT_BAND;

    // Fill Client SSID
    rsi_strcpy(client_profile_ptr->ssid, RSI_CONFIG_CLIENT_SSID);

    // Fill AP SSID length
    client_profile_ptr->ssid_len = strlen(RSI_CONFIG_CLIENT_SSID);

    // Fill scan feature bitmap
    client_profile_ptr->scan_feature_bitmap = RSI_CONFIG_CLIENT_SCAN_FEAT_BITMAP;

    // Fill Client channel number
    rsi_uint16_to_2bytes(client_profile_ptr->channel, RSI_CONFIG_CLIENT_CHANNEL);

    // Scan channel bitmap magic code
    rsi_uint16_to_2bytes(client_profile_ptr->scan_chan_bitmap_magic_code, RSI_CONFIG_CLIENT_MAGIC_CODE);

    // Scan channel bitmap for 2_4 Ghz
    rsi_uint32_to_4bytes(client_profile_ptr->scan_chan_bitmap_2_4_ghz, RSI_CONFIG_CLIENT_SCAN_CHAN_BITMAP_2_4_GHZ);

    // Scan channel bitmap for 5_0 Ghz
    rsi_uint32_to_4bytes(client_profile_ptr->scan_chan_bitmap_5_0_ghz, RSI_CONFIG_CLIENT_SCAN_CHAN_BITMAP_5_0_GHZ);

    // Fill Client security type
    client_profile_ptr->security_type = RSI_CONFIG_CLIENT_SECURITY_TYPE;

    // Fill AP encryption type
    client_profile_ptr->encryption_type = RSI_CONFIG_CLIENT_ENCRYPTION_TYPE;

    // Fill Client PSK
    if (RSI_CONFIG_CLIENT_PSK)
      rsi_strcpy(client_profile_ptr->psk, RSI_CONFIG_CLIENT_PSK);

    // Fill Client PMK
    if (strlen(RSI_CONFIG_CLIENT_PMK)) {
      for (i = 0, j = 0; i < 64 && j < 32; i += 2, j++) {
        if (RSI_CONFIG_CLIENT_PMK[i] && RSI_CONFIG_CLIENT_PMK[i + 1]) {
          client_profile_ptr->pmk[j] = ((uint16_t)convert_lower_case_to_upper_case(RSI_CONFIG_CLIENT_PMK[i])) * 16;
          client_profile_ptr->pmk[j] += convert_lower_case_to_upper_case(RSI_CONFIG_CLIENT_PMK[i + 1]);
        } else {
          client_profile_ptr->pmk[j] = 0;
        }
      }
    }

    nwk_profile_ptr->tcp_stack_used = RSI_CONFIG_CLIENT_TCP_STACK_USED;

    if (nwk_profile_ptr->tcp_stack_used == 1) {
      nwk_profile_ptr->dhcp_enable = RSI_CONFIG_CLIENT_DHCP_MODE;

      // Fill IP address
      memcpy(nwk_profile_ptr->ip_address, (uint8_t *)&ip_addr, RSI_IPV4_ADDRESS_LENGTH);

      // Fill network mask
      memcpy(nwk_profile_ptr->sn_mask, (uint8_t *)&mask, RSI_IPV4_ADDRESS_LENGTH);

      // Fill gateway
      memcpy(nwk_profile_ptr->default_gw, (uint8_t *)&gw, RSI_IPV4_ADDRESS_LENGTH);
    } else {
      // Not supported
    }

    // Fill feature frame params
    feature_frame_ptr->pll_mode      = PLL_MODE;
    feature_frame_ptr->rf_type       = RF_TYPE;
    feature_frame_ptr->wireless_mode = WIRELESS_MODE;
    feature_frame_ptr->enable_ppp    = ENABLE_PPP;
    feature_frame_ptr->afe_type      = AFE_TYPE;
    rsi_uint32_to_4bytes(feature_frame_ptr->feature_enables, FEATURE_ENABLES);

  } else if (type == RSI_WLAN_PROFILE_EAP) {
    ip_addr = RSI_CONFIG_EAP_IP_ADDRESS;
    mask    = RSI_CONFIG_EAP_SN_MASK_ADDRESS;
    gw      = RSI_CONFIG_EAP_GATEWAY_ADDRESS;

    eap_client_profile_ptr = (eap_client_profile_t *)profile_buffer;

    nwk_profile_ptr = &eap_client_profile_ptr->network_profile;

    feature_frame_ptr = &eap_client_profile_ptr->feature_frame_params_s;

    memset(eap_client_profile_ptr, 0, sizeof(eap_client_profile_t));

    // Fill wlan feature bitmap
    rsi_uint32_to_4bytes(eap_client_profile_ptr->wlan_feature_bit_map, RSI_CONFIG_EAP_WLAN_FEAT_BIT_MAP);

    // Fill TCP_IP feature bitmap
    rsi_uint32_to_4bytes(eap_client_profile_ptr->tcp_ip_feature_bit_map, RSI_CONFIG_EAP_TCP_IP_FEAT_BIT_MAP);

    // Fill custom feature bitmap
    rsi_uint32_to_4bytes(eap_client_profile_ptr->custom_feature_bit_map, RSI_CONFIG_EAP_CUSTOM_FEAT_BIT_MAP);

    // Fill data rate
    eap_client_profile_ptr->data_rate = RSI_CONFIG_EAP_DATA_RATE;

    // Fill tx power
    eap_client_profile_ptr->tx_power = RSI_CONFIG_EAP_TX_POWER;

    // Fill band
    eap_client_profile_ptr->band = RSI_CONFIG_EAP_BAND;

    // Fill Client SSID
    rsi_strcpy(eap_client_profile_ptr->ssid, RSI_CONFIG_EAP_SSID);

    // Fill AP SSID length
    eap_client_profile_ptr->ssid_len = strlen(RSI_CONFIG_EAP_SSID);

    // Fill scan feature bitmap
    eap_client_profile_ptr->scan_feature_bitmap = RSI_CONFIG_EAP_SCAN_FEAT_BITMAP;

    // Fill security type
    eap_client_profile_ptr->security_type = RSI_CONFIG_EAP_SECURITY_TYPE;

    // Fill Client channel number
    rsi_uint16_to_2bytes(eap_client_profile_ptr->channel, RSI_CONFIG_EAP_CHANNEL);

    // Scan channel bitmap magic code
    rsi_uint16_to_2bytes(eap_client_profile_ptr->scan_chan_bitmap_magic_code, RSI_CONFIG_EAP_CHAN_MAGIC_CODE);

    // Scan channel bitmap for 2_4 Ghz
    rsi_uint32_to_4bytes(eap_client_profile_ptr->scan_chan_bitmap_2_4_ghz, RSI_CONFIG_EAP_SCAN_CHAN_BITMAP_2_4_GHZ);

    // Scan channel bitmap for 5_0 Ghz
    rsi_uint32_to_4bytes(eap_client_profile_ptr->scan_chan_bitmap_5_0_ghz, RSI_CONFIG_EAP_SCAN_CHAN_BITMAP_5_0_GHZ);

    // Fill EAP method
    rsi_strcpy(eap_client_profile_ptr->eap_method, RSI_CONFIG_EAP_METHOD);

    // Fill Inner method
    rsi_strcpy(eap_client_profile_ptr->inner_method, RSI_CONFIG_EAP_INNER_METHOD);

    // Fill User identity
    rsi_strcpy(eap_client_profile_ptr->user_identity, RSI_CONFIG_EAP_USER_IDENTITY);

    // Fill Password
    rsi_strcpy(eap_client_profile_ptr->passwd, RSI_CONFIG_EAP_PASSWORD);

    nwk_profile_ptr->tcp_stack_used = RSI_CONFIG_EAP_TCP_STACK_USED;

    if (nwk_profile_ptr->tcp_stack_used == 1) {
      nwk_profile_ptr->dhcp_enable = RSI_CONFIG_EAP_DHCP_MODE;

      // Fill IP address
      memcpy(nwk_profile_ptr->ip_address, (uint8_t *)&ip_addr, RSI_IPV4_ADDRESS_LENGTH);

      // Fill network mask
      memcpy(nwk_profile_ptr->sn_mask, (uint8_t *)&mask, RSI_IPV4_ADDRESS_LENGTH);

      // Fill gateway
      memcpy(nwk_profile_ptr->default_gw, (uint8_t *)&gw, RSI_IPV4_ADDRESS_LENGTH);
    } else {
      // Not supported
    }

    // Fill feature frame params
    feature_frame_ptr->pll_mode      = PLL_MODE;
    feature_frame_ptr->rf_type       = RF_TYPE;
    feature_frame_ptr->wireless_mode = WIRELESS_MODE;
    feature_frame_ptr->enable_ppp    = ENABLE_PPP;
    feature_frame_ptr->afe_type      = AFE_TYPE;
    rsi_uint32_to_4bytes(feature_frame_ptr->feature_enables, FEATURE_ENABLES);

  } else if (type == RSI_WLAN_PROFILE_P2P) {
    ip_addr = RSI_CONFIG_P2P_IP_ADDRESS;
    mask    = RSI_CONFIG_P2P_SN_MASK_ADDRESS;
    gw      = RSI_CONFIG_P2P_GATEWAY_ADDRESS;

    p2p_profile_ptr = (p2p_profile_t *)profile_buffer;

    nwk_profile_ptr = &p2p_profile_ptr->network_profile;

    //feature_frame_params_t *feature_frame_ptr = NULL;

    feature_frame_ptr = &p2p_profile_ptr->feature_frame_params_s;

    memset(p2p_profile_ptr, 0, sizeof(p2p_profile_t));

    // Fill wlan feature bitmap
    rsi_uint32_to_4bytes(p2p_profile_ptr->wlan_feature_bit_map, RSI_CONFIG_P2P_WLAN_FEAT_BIT_MAP);

    // Fill TCP_IP feature bitmap
    rsi_uint32_to_4bytes(p2p_profile_ptr->tcp_ip_feature_bit_map, RSI_CONFIG_P2P_TCP_IP_FEAT_BIT_MAP);

    // Fill custom feature bitmap
    rsi_uint32_to_4bytes(p2p_profile_ptr->custom_feature_bit_map, RSI_CONFIG_P2P_CUSTOM_FEAT_BIT_MAP);

    // Fill Join SSID
    rsi_strcpy(p2p_profile_ptr->join_ssid, RSI_CONFIG_P2P_JOIN_SSID);

    // Fill data rate
    p2p_profile_ptr->data_rate = RSI_CONFIG_CLIENT_DATA_RATE;

    // Fill tx power
    p2p_profile_ptr->tx_power = RSI_CONFIG_CLIENT_TX_POWER;

    // Fill AP band
    p2p_profile_ptr->band = RSI_CONFIG_CLIENT_BAND;

    // Fill GO Intnent
    rsi_uint16_to_2bytes(p2p_profile_ptr->go_intent, RSI_CONFIG_P2P_GO_INTNET);

    // Fill Device name
    rsi_strcpy(p2p_profile_ptr->device_name, RSI_CONFIG_P2P_DEVICE_NAME);

    // Fill operating channel
    rsi_uint16_to_2bytes(p2p_profile_ptr->operating_channel, RSI_CONFIG_P2P_OPERATING_CHANNEL);

    // Fill SSID postfix
    rsi_strcpy(p2p_profile_ptr->ssid_postfix, RSI_CONFIG_P2P_SSID_POSTFIX);

    // Fill PSK key
    rsi_strcpy(p2p_profile_ptr->psk_key, RSI_CONFIG_P2P_PSK_KEY);

    nwk_profile_ptr->tcp_stack_used = RSI_CONFIG_P2P_TCP_STACK_USED;

    if (nwk_profile_ptr->tcp_stack_used == 1) {
      nwk_profile_ptr->dhcp_enable = RSI_CONFIG_P2P_DHCP_MODE;

      // Fill IP address
      memcpy(nwk_profile_ptr->ip_address, (uint8_t *)&ip_addr, RSI_IPV4_ADDRESS_LENGTH);

      // Fill network mask
      memcpy(nwk_profile_ptr->sn_mask, (uint8_t *)&mask, RSI_IPV4_ADDRESS_LENGTH);

      // Fill gateway
      memcpy(nwk_profile_ptr->default_gw, (uint8_t *)&gw, RSI_IPV4_ADDRESS_LENGTH);
    } else {
      // Not supported
    }

    // Fill feature frame params
    feature_frame_ptr->pll_mode      = PLL_MODE;
    feature_frame_ptr->rf_type       = RF_TYPE;
    feature_frame_ptr->wireless_mode = WIRELESS_MODE;
    feature_frame_ptr->enable_ppp    = ENABLE_PPP;
    feature_frame_ptr->afe_type      = AFE_TYPE;
    rsi_uint32_to_4bytes(feature_frame_ptr->feature_enables, FEATURE_ENABLES);

  } else {
    profile_buffer = NULL;
  }

  return profile_buffer;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_delete_profile(uint32_t type)
 * @brief      Send raw data packet. 
 * @pre        \ref rsi_wireless_init() API needs to be called before this API. 
 * @param[in]  type - profile type
 * @return    	0  -  Success \n
 *              Non-Zero Value - Failure \n
 *				-4 - Buffer not availableto serve the command
 */

int32_t rsi_wlan_delete_profile(uint32_t type)
{
  int32_t status = RSI_SUCCESS;
#ifndef RSI_CHIP_MFG_EN
  rsi_pkt_t *pkt                 = NULL;
  rsi_profile_req_t *profile_req = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;
  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_profile_req_t));

    profile_req = (rsi_profile_req_t *)pkt->data;

    // Fill profile type
    rsi_uint32_to_4bytes(profile_req->profile_type, type);

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send delete profile command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_DELETE_PROFILE, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_DELETE_PROFILE_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

#endif
  // Return the status if error in sending command occurs
  return status;
}

/*==============================================*/
/**
 * @fn        int32_t rsi_wlan_enable_auto_config(uint8_t enable, uint32_t type)
 * @brief     Send raw data packet to module. 
 * @param[in]  enable - to enable/disable the profile configuration,first parameter value would be 1(CFG_ENABLE)- means This command is used to enable or disable the feature of auto-join or \n
 *                      auto-create on powerup and 2(enable_auto_config) -for profile based.
 * @param[in]  type   - profile type \n
 *						Note:
 *						Possible profile types. \n
 *						// Client profile \n
 *						#define RSI_WLAN_PROFILE_CLIENT 0 \n
 *						// P2P profile \n
 *						#define RSI_WLAN_PROFILE_P2P 1 \n
 *						// EAP profile \n
 *						#define RSI_WLAN_PROFILE_EAP 2 \n
 *						// AP profile \n
 *						#define RSI_WLAN_PROFILE_AP 6 \n
 *						// All profiles \n
 *						#define RSI_WLAN_PROFILE_ALL 0xF
 * @return   	0  -  Success \n
 *              Non-Zero Value - Failure \n
 *				-4 - Buffer not availableto serve the command
 *
 *
 *
 */

int32_t rsi_wlan_enable_auto_config(uint8_t enable, uint32_t type)
{
  int32_t status                              = RSI_SUCCESS;
  rsi_pkt_t *pkt                              = NULL;
  rsi_auto_config_enable_t *config_enable_req = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }
    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_auto_config_enable_t));

    config_enable_req = (rsi_auto_config_enable_t *)pkt->data;

    // Fill config profile enable/disable
    config_enable_req->config_enable = enable;

    // Fill profile type
    rsi_uint32_to_4bytes(config_enable_req->profile_type, type);

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send delete profile command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_AUTO_CONFIG_ENABLE, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_AUTO_CONFIG_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}
/*==============================================*/
/**
 * @fn      	int32_t rsi_wlan_pmk_generate(int8_t type,int8_t *psk,int8_t *ssid, uint8_t *pmk, uint16_t length)
 * @brief   	Generate PMK using PSK and SSID are provided.
 * @pre  		\ref rsi_wlan_connect() API needs to be called before this API. 
 * @param[in]   type - possible values of this field are 1, 2 and 3, but we only pass 3 for generation of PMK. 
 * @param[in]   psk  - In this field expected parameters are pre shared key(psk) of the access point to 
 * @param[in]   ssid - This field contains the SSID of the access point, this field will be valid only if TYPE value is 3.
 * @param[in]   pmk  - this is an array 
 * @param[in]   length - length of pmk array  
 * @param[out]  32 bytes pmk 
 * @return		0  -  Success \n
 *              Non-Zero Value - Failure
 *
 */

int32_t rsi_wlan_pmk_generate(int8_t type, int8_t *psk, int8_t *ssid, uint8_t *pmk, uint16_t length)
{
  int32_t status         = RSI_SUCCESS;
  rsi_pkt_t *pkt         = NULL;
  rsi_req_psk_t *psk_ptr = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    psk_ptr = (rsi_req_psk_t *)pkt->data;

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_req_psk_t));
    //
    // attach the buffer given by user
    wlan_cb->app_buffer = (uint8_t *)pmk;

    // length of the buffer provided by user
    wlan_cb->app_buffer_length = length;

    // assigning type into structure member
    psk_ptr->type = type;

    // copy PSK given by user into structure member
    rsi_strcpy(psk_ptr->psk_or_pmk, psk);

    // copy SSID given by user into structure member
    rsi_strcpy(psk_ptr->ap_ssid, ssid);

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send psk command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_HOST_PSK, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_PSK_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}
/*==============================================*/
/**
 * @fn      int16_t rsi_wlan_set_sleep_timer( uint16_t sleep_time )
 * @brief   Configure the sleep timer mode of the module to go into sleep during power save operation. 
 * @pre   	This command can be issued any time in case of power save mode 9 (MSG_BASED). 
 * @param[in]   sleep_time - Sleep Time value in seconds Minimum value is 1, and maximum value is 2100 
 * @return     0  -  Success \n
 *              Non-Zero Value - Failure      
 *
 */
int16_t rsi_wlan_set_sleep_timer(uint16_t sleep_time)
{
  int16_t status               = RSI_SUCCESS;
  rsi_pkt_t *pkt               = NULL;
  rsi_set_sleep_timer_t *timer = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    timer = (rsi_set_sleep_timer_t *)pkt->data;
    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_set_sleep_timer_t));

    // copy sleep_timer given by user into structure member
    rsi_uint16_to_2bytes(timer->timeval, sleep_time);

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send set sleep timer command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SET_SLEEP_TIMER, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_SET_SLEEP_TIMER_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/*====================================================*/
/**
 * @fn       uint16_t rsi_wlan_register_callbacks(uint32_t callback_id, void (*callback_handler_ptr)(uint16_t status, 
 *                                                uint8_t *buffer, const uint32_t length))
 * @brief    Register the WLAN call back functions. 
 * @param[in]  callback_id                       - This is the Id of the call back function Following ids are supported: 
 * @param[in]  void (*callback_handler_ptr)(void - This is the Call back handler 
 * @param[in]  status                            - status of the asynchronous response 
 * @param[in]  buffer                            - payload of the asynchronous response 
 * @param[in]  length                            - length of the payload
 * @return      0 - Success \n
 *              3 - Failure \n
 *				If call_back_id is greater than the maximum callbacks to register, returns 1.
 * @note        In callbacks, application should not initiate any TX operation to the module.
 */
uint16_t rsi_wlan_register_callbacks(uint32_t callback_id,
                                     void (*callback_handler_ptr)(uint16_t status,
                                                                  uint8_t *buffer,
                                                                  const uint32_t length))
{
  if (callback_id > RSI_MAX_NUM_CALLBACKS) {
    /*
     *Return , if the callback number exceeds the RSI_MAX_NUM_CALLBACKS ,or
     * the callback is already registered
     */
    return 1;
  }
  if (callback_id == RSI_JOIN_FAIL_CB) // check for NULL or not
  {
    // Register join fail call back handler
    rsi_wlan_cb_non_rom->callback_list.join_fail_handler = callback_handler_ptr;
  } else if (callback_id == RSI_IP_FAIL_CB) {
    // Register ip renewal fail call back handler
    rsi_wlan_cb_non_rom->callback_list.ip_renewal_fail_handler = callback_handler_ptr;
  } else if (callback_id == RSI_REMOTE_SOCKET_TERMINATE_CB) {
    // Register remote socket terminate call back handler
    rsi_wlan_cb_non_rom->callback_list.remote_socket_terminate_handler = callback_handler_ptr;
  } else if (callback_id == RSI_IP_CHANGE_NOTIFY_CB) {
    // Register ip change notify call back handler
    rsi_wlan_cb_non_rom->callback_list.ip_change_notify_handler = callback_handler_ptr;
  } else if (callback_id == RSI_STATIONS_CONNECT_NOTIFY_CB) {
    // Register station connect notify call back handler
    rsi_wlan_cb_non_rom->callback_list.stations_connect_notify_handler = callback_handler_ptr;
  } else if (callback_id == RSI_STATIONS_DISCONNECT_NOTIFY_CB) {
    // Register station disconnect notify call back handler
    rsi_wlan_cb_non_rom->callback_list.stations_disconnect_notify_handler = callback_handler_ptr;
  } else if (callback_id == RSI_WLAN_DATA_RECEIVE_NOTIFY_CB) {
    // Register wlan data packet receive notify call back handler
    rsi_wlan_cb_non_rom->callback_list.wlan_data_receive_handler = callback_handler_ptr;
  } else if (callback_id == RSI_WLAN_RECEIVE_STATS_RESPONSE_CB) {
    // Register wlan receive stats notify call back handler
    rsi_wlan_cb_non_rom->callback_list.wlan_receive_stats_response_handler = callback_handler_ptr;
  } else if (callback_id == RSI_WLAN_WFD_DISCOVERY_NOTIFY_CB) {
    // Register wifi direct device discovery notify call back handler
    rsi_wlan_cb_non_rom->callback_list.wlan_wfd_discovery_notify_handler = callback_handler_ptr;
  } else if (callback_id == RSI_WLAN_WFD_CONNECTION_REQUEST_NOTIFY_CB) {
    // Register wifi direct connection request notify call back handler
    rsi_wlan_cb_non_rom->callback_list.wlan_wfd_connection_request_notify_handler = callback_handler_ptr;
  } else if (callback_id == RSI_WLAN_RAW_DATA_RECEIVE_HANDLER) {
    // Register raw data receive notify call back handler
    rsi_wlan_cb_non_rom->callback_list.raw_data_receive_handler = callback_handler_ptr;

  }
#ifndef RSI_M4_INTERFACE
  else if (callback_id == RSI_WLAN_SOCKET_CONNECT_NOTIFY_CB) {
    // Register socket connection notify call back handler
    rsi_wlan_cb_non_rom->callback_list.socket_connect_response_handler = callback_handler_ptr;
  } else if (callback_id == RSI_WLAN_SERVER_CERT_RECEIVE_NOTIFY_CB) {
    // Register certificate response call back handler
    rsi_wlan_cb_non_rom->callback_list.certificate_response_handler = callback_handler_ptr;
  } else if (callback_id == RSI_WLAN_ASYNC_STATS) {
    rsi_wlan_cb_non_rom->callback_list.wlan_async_module_state = callback_handler_ptr;
  }
#endif
#ifdef RSI_CHIP_MFG_EN
  else if (callback_id == RSI_FLASH_WRITE_RESPONSE) {
    rsi_wlan_cb_non_rom->callback_list.flash_write_response_handler = callback_handler_ptr;
  }
#endif
  else if (callback_id == RSI_WLAN_ASSERT_NOTIFY_CB) {
    // Register assert notify call back handler
    rsi_wlan_cb_non_rom->callback_list.rsi_assertion_cb = callback_handler_ptr;
  } else if (callback_id == RSI_WLAN_MAX_TCP_WINDOW_NOTIFY_CB) {
    if ((RSI_TCP_IP_FEATURE_BIT_MAP & TCP_IP_FEAT_EXTENSION_VALID)
        && (RSI_EXT_TCPIP_FEATURE_BITMAP & EXT_TCP_DYNAMIC_WINDOW_UPDATE_FROM_HOST)) {
      // Register max available rx window call back handler
      rsi_wlan_cb_non_rom->callback_list.rsi_max_available_rx_window = callback_handler_ptr;
    }
  }
  return 0;
}
/** @} */
/** @addtogroup NETWORK16
* @{
*/
/*==============================================*/
/**
 * @fn        int32_t rsi_socket_config()    
 * @brief     Select the socket comfiguration
 * @return    0  -  Success \n
 *            Non-Zero Value - Failure 
 * @note      Please refer to Error Codes section for the description of the above error codes \ref SP16. 
 */
int32_t rsi_socket_config(void)
{

  rsi_pkt_t *pkt;
  rsi_socket_config_t *rsi_socket_config;
  int32_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_IP_CONFIG_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Take the user provided data and fill it in antenna select structure
    rsi_socket_config = (rsi_socket_config_t *)pkt->data;

    // total sockets
    rsi_socket_config->total_sockets = TOTAL_SOCKETS;

    // total tcp sockets
    rsi_socket_config->total_tcp_sockets = TOTAL_TCP_SOCKETS;

    // total udp sockets
    rsi_socket_config->total_udp_sockets = TOTAL_UDP_SOCKETS;

    // total tcp tx sockets
    rsi_socket_config->tcp_tx_only_sockets = TCP_TX_ONLY_SOCKETS;

    // total tcp rx sockets
    rsi_socket_config->tcp_rx_only_sockets = TCP_RX_ONLY_SOCKETS;

    // total udp tx sockets
    rsi_socket_config->udp_tx_only_sockets = UDP_TX_ONLY_SOCKETS;

    // total udp tx sockets
    rsi_socket_config->udp_rx_only_sockets = UDP_RX_ONLY_SOCKETS;

    //tcp rx high performance sockets
    rsi_socket_config->tcp_rx_high_performance_sockets = TCP_RX_HIGH_PERFORMANCE_SOCKETS;

    //tcp rx windows size
    rsi_socket_config->tcp_rx_window_size_cap = TCP_RX_WINDOW_SIZE_CAP;

    //tcp rx windows division factor
    rsi_socket_config->tcp_rx_window_div_factor = TCP_RX_WINDOW_DIV_FACTOR;

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send socket configuration command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_SOCKET_CONFIG, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_SOCK_CONFIG_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status
  return status;
}
/** @} */
/** @addtogroup NETWORK17
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_req_radio(uint8_t enable)
 * @brief      Register and de-register wlan radio.
 * @param[in]    enable - To register ot de-register wlan radio  
 * @param[out]   RSI_SUCCESS 
 * @return		0  -  Success \n
 *              Non-Zero Value - Failure
 *
 */

int32_t rsi_wlan_req_radio(uint8_t enable)
{
  rsi_pkt_t *pkt                   = NULL;
  int32_t status                   = RSI_SUCCESS;
  rsi_wlan_req_radio_t *wlan_radio = NULL;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  if (wlan_cb->state < RSI_WLAN_STATE_OPERMODE_DONE) {
    // Command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }
    wlan_radio = (rsi_wlan_req_radio_t *)pkt->data;

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_wlan_req_radio_t));

    wlan_radio->radio_req = enable;
    if (wlan_radio->radio_req) {
      rsi_wlan_cb_non_rom->wlan_radio = RSI_WLAN_RADIO_REGISTER;
    } else {
      rsi_wlan_cb_non_rom->wlan_radio = RSI_WLAN_RADIO_DEREGISTER;
    }

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send  command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_RADIO, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_WLAN_RADIO_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}

/** @} */
/** @addtogroup WLAN
* @{
*/
/*==============================================*/
/**
 * @fn        int32_t rsi_wlan_radio_deinit()
 * @brief     De-register wlan radio. 
 * @pre 	  The Application should be in radio init and unconnected state before calling this function. 
 *			  \ref rsi_wlan_radio_deinit() API is called after \ref rsi_wlan_scan()
 * @param[in]   void       
 * @param[out]  RSI_SUCCESS      
 * @return		0  -  Success \n
 *              Non-Zero Value - Failure \n
 *              0x0021- Command given in wrong state \n
 *              0x0101-WLAN radio is already deregistered
 *
 */

int32_t rsi_wlan_radio_deinit(void)
{
  int32_t status = RSI_SUCCESS;
  // req for wlan radio
  status = rsi_wlan_req_radio(RSI_DISABLE);

  // Return the status if error in sending command occurs
  return status;
}
/** @} */
//#ifdef RSI_WAC_MFI_ENABLE

#ifdef RSI_WAC_MFI_ENABLE
/** @addtogroup NETWORK18
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t rsi_wlan_add_mfi_ie(int8_t *mfi_ie, uint32_t ie_len)
 * @brief      Add the Apple defined IE elements to the in Beacon command request structure. 
 * @param[in]  mfi_ie - pointer  to the IE element
 * @param[in]  ie_len - Length of the IE element
 * @return     0  -  Success \n
 *             Non-Zero Value - Failure \n
 *			   -4 - Buffer not availableto serve the command
 */
int32_t rsi_wlan_add_mfi_ie(int8_t *mfi_ie, uint32_t ie_len)
{
  int32_t status = RSI_SUCCESS;

  rsi_pkt_t *pkt;
  rsi_req_add_mfi_ie_t *mfi_ie_req = NULL;

  // Get WLAN cb struct pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    mfi_ie_req = (rsi_req_add_mfi_ie_t *)pkt->data;

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_req_add_mfi_ie_t));

    // Fill IE length
    mfi_ie_req->mfi_ie_len = ie_len;

    // Copy IE
    memcpy(mfi_ie_req->mfi_ie, mfi_ie, ie_len);

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send  wps request command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_ADD_MFI_IE, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_ADD_MFI_IE_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();
    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}
/** @} */
#endif
/** @addtogroup WLAN
* @{
*/

/*==============================================*/
/**
 * @fn       int32_t rsi_wlan_update_gain_table(uint8_t band, uint8_t bandwidth, uint8_t *payload, uint16_t payload_len)
 * @brief    Assign the user configurable channel gain values in different regions to the module from user.  
 * @pre   	 \ref rsi_radio_init() API needs to be called before this API
 * @param[in]  band        -  0 ?  2.4GHz \n   
 *					          1 ? 5GHz \n
 *					          2 ? Dual band (2.4 Ghz and 5 Ghz) 
 * @param[in]  bandwidth   -  0 ? 20 MHz \n
 *                            1 ? 40 MHz 
 * @param[in]  payload     - Pass channel gain values for different regions in an given array format. 
 * @param[in]  payload_len - Max payload length (table size) in 2.4GHz is 128 bytes \n
 *                           Max payload length (table size) in 5GHz is 64 bytes 
 * @note		1. Length of the payload should match with payload_len parameter value. \n
 *				2. In 2.4Ghz band, 40Mhz is not supported.
 * @return     0  -  Success \n
 *             Non-Zero Value - Failure  \n
 * 	           -2 - Invalid parameters \n
 *             -3 - Command given in wrong state 
 * @note        Refer to Error Codes section for above error codes \ref SP16.
 */
int32_t rsi_wlan_update_gain_table(uint8_t band, uint8_t bandwidth, uint8_t *payload, uint16_t payload_len)
{
  int32_t status = 0;

  rsi_pkt_t *pkt = NULL;

  rsi_gain_table_info_t *gain_table = NULL;

  // Get WLAN cb struct pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  // Check for gain table max supported payload length
  if (payload_len > GAIN_TABLE_MAX_PAYLOAD_LEN) {
    return RSI_ERROR_INVALID_PARAM;
  }

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {

    // allocate command buffer from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    gain_table = (rsi_gain_table_info_t *)pkt->data;

    // Memset buffer
    memset(&pkt->data, 0, sizeof(rsi_gain_table_info_t));

    // Fill band
    gain_table->band = band;

    // Fill bandwidth
    gain_table->bandwidth = bandwidth;

    // Fill payload length
    gain_table->size = payload_len;

    // Fill payload content
    memcpy(gain_table->gain_table, payload, payload_len);

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send  command
    status = rsi_driver_wlan_send_cmd(RSI_WLAN_REQ_GAIN_TABLE, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_GAIN_TABLE_RESPONSE_WAIT_TIME);

    // get wlan/network command response status
    status = rsi_wlan_get_status();

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status if error in sending command occurs
  return status;
}
/** @} */
#ifdef RSI_CHIP_MFG_EN
static bootup_params_t bootup_params = {
  .magic_number                 = 0x5aa5,
  .crystal_good_time            = 0x0,
  .valid                        = 0x10001e3,
  .reserved_for_valids          = 0x0,
  .bootup_mode_info             = 0x0,
  .digital_loop_back_params     = 0x0,
  .rtls_timestamp_en            = 0x0,
  .host_spi_intr_cfg            = 0x0,
  .device_clk_info              = { { .pll_config_g = { .modem_pll_info_g = { .pll_ctrl_set_reg         = 0xd518,
                                                                 .pll_ctrl_clr_reg         = 0x2ae7,
                                                                 .modem_config_reg         = 0x2000,
                                                                 .soc_clk_config_reg       = 0xc18,
                                                                 .adc_dac_strm1_config_reg = 0x1100,
                                                                 .adc_dac_strm2_config_reg = 0x6600 } },
                         .switch_clk_g = { .switch_tass_clk              = 0x1,
                                           .switch_qspi_clk              = 0x0,
                                           .switch_slp_clk_2_32          = 0x0,
                                           .switch_wlan_bbp_lmac_clk_reg = 0x1,
                                           .switch_zbbt_bbp_lmac_clk_reg = 0x0,
                                           .switch_bbp_lmac_clk_reg      = 0x1,
                                           .modem_clk_is_160mhz          = 0x0,
                                           .reserved                     = 0x0,
                                           .tass_clock_config_reg1       = 0x83c0503,
                                           .wlan_bbp_lmac_clk_reg_val    = 0x1042001,
                                           .zbbt_bbp_lmac_clk_reg_val    = 0x2010001,
                                           .bbp_lmac_clk_en_val          = 0x3b } } },
  .buckboost_wakeup_cnt         = 0x0,
  .pmu_wakeup_wait              = 0x0,
  .shutdown_wait_time           = 0x0,
  .pmu_slp_clkout_sel           = 0x0,
  .wdt_prog_value               = 0x0,
  .wdt_soc_rst_delay            = 0x0,
  .dcdc_operation_mode          = 0x0,
  .soc_reset_wait_cnt           = 0x0,
  .waiting_time_at_fresh_sleep  = 0x0,
  .max_threshold_to_avoid_sleep = 0x0,
  .beacon_resedue_algo_en       = 0x0
};
/** @addtogroup WLAN
* @{
*/

/*==============================================*/
/**
 * @fn        int32_t rsi_load_bootup_params()
 * @brief     Send Boot_up parameters to the Firmware.  
 * @param[in]  void  
 * @return     0  -  Success \n
 *             Non-Zero Value - Failure
 */

int32_t rsi_load_bootup_params()
{
  rsi_pkt_t *pkt;
  uint8_t status = 0;
  bootup_params_t *bootup_params_p;

  // Get WLAN cb struct pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    bootup_params_p = (bootup_params_t *)pkt->data;

    memcpy(bootup_params_p, &bootup_params, sizeof(bootup_params_t));

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send  command
    status = rsi_driver_wlan_send_cmd(RSI_BOOTUP_PARAMS, pkt);
    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_BOOTUP_PARAMS_RESPONSE_WAIT_TIME);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  return status;
}

/*==============================================*/
/**
 * @fn         void rsi_get_spi_config(uint32_t which_flash_type, uint32_t pinset, spi_config_t *spi_config)
 * @brief      Get SPI configuration
 * @param[in]  which_flash_type  - Flash type    
 * @param[in]  pinset - Pin set           
 * @param[in]  spi_config - SPI configuration
 * @return     0  -  Success \n
 *             Non-Zero Value - Failure
 */

void rsi_get_spi_config(uint32_t which_flash_type, uint32_t pinset, spi_config_t *spi_config)
{
  spi_config_1_t *spi_config1 = (spi_config_1_t *)&spi_config->spi_config_1;
  spi_config_2_t *spi_config2 = (spi_config_2_t *)&spi_config->spi_config_2;
  spi_config_3_t *spi_config3 = (spi_config_3_t *)&spi_config->spi_config_3;
  spi_config_4_t *spi_config4 = (spi_config_4_t *)&spi_config->spi_config_4;
  spi_config_5_t *spi_config5 = (spi_config_5_t *)&spi_config->spi_config_5;
  spi_config_6_t *spi_config6 = (spi_config_6_t *)&spi_config->spi_config_6;
  spi_config_7_t *spi_config7 = (spi_config_7_t *)&spi_config->spi_config_7;

  // Default SPI configuration
  spi_config1->inst_mode         = QUAD_MODE;
  spi_config1->addr_mode         = QUAD_MODE;
  spi_config1->data_mode         = QUAD_MODE;
  spi_config1->dummy_mode        = QUAD_MODE;
  spi_config1->extra_byte_mode   = QUAD_MODE;
  spi_config1->prefetch_en       = DIS_PREFETCH;
  spi_config1->dummy_W_or_R      = DUMMY_READS;
  spi_config1->extra_byte_en     = 0;
  spi_config1->d3d2_data         = 3;
  spi_config1->continuous        = CONTINUOUS;
  spi_config1->read_cmd          = FREAD_QUAD_O;
  spi_config1->flash_type        = MICRON_QUAD_FLASH;
  spi_config1->no_of_dummy_bytes = 1;

  spi_config2->auto_mode                   = EN_AUTO_MODE;
  spi_config2->cs_no                       = CHIP_ZERO;
  spi_config2->neg_edge_sampling           = NEG_EDGE_SAMPLING;
  spi_config2->qspi_clk_en                 = QSPI_FULL_TIME_CLK;
  spi_config2->protection                  = REM_WR_PROT;
  spi_config2->dma_mode                    = NO_DMA;
  spi_config2->swap_en                     = SWAP;
  spi_config2->full_duplex                 = IGNORE_FULL_DUPLEX;
  spi_config2->wrap_len_in_bytes           = NO_WRAP;
  spi_config2->addr_width_valid            = 0;
  spi_config2->addr_width                  = _24BIT_ADDR;
  spi_config2->dummy_cycles_for_controller = 0;
  spi_config2->pinset_valid                = 0; //(1) FIXME : This is not there in ta_mbr_efuse
  spi_config2->flash_pinset                = pinset;

  spi_config3->en_word_swap           = 0;
  spi_config3->_16bit_cmd_valid       = 0;
  spi_config3->_16bit_rd_cmd_msb      = 0;
  spi_config3->xip_mode               = 0;
  spi_config3->no_of_dummy_bytes_wrap = 0;
  spi_config3->ddr_mode_en            = 0;
  spi_config3->wr_cmd                 = 0x2;
  spi_config3->wr_inst_mode           = QUAD_MODE;
  spi_config3->wr_addr_mode           = QUAD_MODE;
  spi_config3->wr_data_mode           = QUAD_MODE;
  spi_config3->dummys_4_jump          = 1;

  spi_config4->_16bit_wr_cmd_msb      = 0;
  spi_config4->qspi_ddr_clk_en        = 1; //(0) FIXME : This is notthere in ta_mbr_efuse
  spi_config4->qspi_loop_back_mode_en = 0;
  spi_config4->qspi_manual_ddr_phasse = 0;
  spi_config4->ddr_data_mode          = 0;
  spi_config4->ddr_inst_mode          = 0;
  spi_config4->ddr_addr_mode          = 0;
  spi_config4->ddr_dummy_mode         = 0;
  spi_config4->ddr_extra_byte         = 0;
  spi_config4->dual_flash_mode        = 0;
  spi_config4->secondary_csn          = 0;
  spi_config4->polarity_mode          = 0;
  spi_config4->valid_prot_bits        = 4;
  spi_config4->no_of_ms_dummy_bytes   = 0;
  spi_config4->ddr_dll_en             = 0;
  spi_config4->continue_fetch_en      = 0;
  spi_config4->dma_write              = 0;
  spi_config4->prot_top_bottom        = 0;
  spi_config4->auto_csn_based_addr_en = 0;

  spi_config5->block_erase_cmd      = BLOCK_ERASE;
  spi_config5->busy_bit_pos         = 0;
  spi_config5->d7_d4_data           = 0xf;
  spi_config5->dummy_bytes_for_rdsr = 0x0;
  spi_config5->reset_type           = 0x0;

  spi_config6->chip_erase_cmd   = CHIP_ERASE;
  spi_config6->sector_erase_cmd = SECTOR_ERASE;

  spi_config7->status_reg_read_cmd  = 0x05;
  spi_config7->status_reg_write_cmd = 0x01;

  if (which_flash_type == GIGA_DEVICE_FLASH) {
    spi_config1->read_cmd          = 0x0B;
    spi_config1->flash_type        = GIGA_DEVICE_FLASH;
    spi_config1->no_of_dummy_bytes = 4;
    spi_config4->valid_prot_bits   = 5;
    spi_config5->reset_type        = 0x3;
  }

  if (which_flash_type == MX_QUAD_FLASH) {
    spi_config1->inst_mode = SINGLE_MODE;
#if 1
    spi_config1->addr_mode  = QUAD_MODE;
    spi_config1->dummy_mode = QUAD_MODE;
#else
    spi_config1->addr_mode         = SINGLE_MODE;
    spi_config1->dummy_mode        = SINGLE_MODE;
#endif
    spi_config3->wr_inst_mode  = SINGLE_MODE;
    spi_config3->wr_addr_mode  = SINGLE_MODE;
    spi_config3->wr_data_mode  = SINGLE_MODE;
    spi_config1->extra_byte_en = 1;
#if 1
    spi_config1->read_cmd          = 0xEB; // (0xB) FIXME :This is not there in ta_mbr_efuse
    spi_config1->no_of_dummy_bytes = 2;    // (2)FIXME : This is not there in ta_mbr_efuse
#else
    spi_config1->read_cmd          = 0x6B; // (0xB) FIXME :This is not there in ta_mbr_efuse
    spi_config1->no_of_dummy_bytes = 1;    // (2)FIXME : This is not there in ta_mbr_efuse
#endif
    spi_config1->flash_type = MX_QUAD_FLASH;

    spi_config4->prot_top_bottom = 1;
    spi_config5->reset_type      = 0x0; //(4) FIXME : This is not there in ta_mbr_efuse
  }

  if (which_flash_type == MICRON_QUAD_FLASH) {
    spi_config1->read_cmd          = FREAD_QUAD_O;
    spi_config1->flash_type        = MICRON_QUAD_FLASH;
    spi_config1->no_of_dummy_bytes = 1;
    spi_config5->reset_type        = 0x3;
  }

  if (which_flash_type == ADESTO_OCTA_FLASH) {
    spi_config1->inst_mode         = OCTA_MODE;
    spi_config1->addr_mode         = OCTA_MODE;
    spi_config1->data_mode         = OCTA_MODE;
    spi_config1->dummy_mode        = OCTA_MODE;
    spi_config1->extra_byte_mode   = OCTA_MODE;
    spi_config1->dummy_W_or_R      = DUMMY_WRITES;
    spi_config1->read_cmd          = 0x0B;
    spi_config1->flash_type        = ADESTO_OCTA_FLASH;
    spi_config1->no_of_dummy_bytes = 8;

    spi_config3->wr_inst_mode = OCTA_MODE;
    spi_config3->wr_addr_mode = OCTA_MODE;
    spi_config3->wr_data_mode = OCTA_MODE;

    spi_config5->dummy_bytes_for_rdsr = 4;
    spi_config5->reset_type           = 0x3;
    spi_config5->block_erase_cmd      = BLOCK_ERASE;

    //mbr_p->qspi_ctrl_flags |= OCTA_FLASH;
  }

  if (which_flash_type == MX_OCTA_FLASH) {
    spi_config1->inst_mode         = OCTA_MODE;
    spi_config1->addr_mode         = OCTA_MODE;
    spi_config1->data_mode         = OCTA_MODE;
    spi_config1->dummy_mode        = OCTA_MODE;
    spi_config1->extra_byte_mode   = OCTA_MODE;
    spi_config1->dummy_W_or_R      = DUMMY_WRITES;
    spi_config1->read_cmd          = 0xEC;
    spi_config1->flash_type        = MX_OCTA_FLASH;
    spi_config1->no_of_dummy_bytes = 4;

    spi_config2->addr_width_valid = 1;
    spi_config2->addr_width       = _32BIT_ADDR;

    spi_config3->_16bit_cmd_valid  = 1;
    spi_config3->_16bit_rd_cmd_msb = 0x13;
    spi_config3->wr_cmd            = 0x12;
    spi_config3->wr_inst_mode      = OCTA_MODE;
    spi_config3->wr_addr_mode      = OCTA_MODE;
    spi_config3->wr_data_mode      = OCTA_MODE;

    spi_config4->_16bit_wr_cmd_msb    = 0xED;
    spi_config4->no_of_ms_dummy_bytes = 1;

    spi_config5->dummy_bytes_for_rdsr = 4;
    spi_config5->reset_type           = 0x7;
    spi_config5->block_erase_cmd      = 0xDC23;

    spi_config6->chip_erase_cmd   = 0xC738;
    spi_config6->sector_erase_cmd = 0x21DE;

    spi_config7->status_reg_read_cmd  = 0x05FA;
    spi_config7->status_reg_write_cmd = 0x01FE;

#ifdef PHASE_2
    mbr_p->qspi_ctrl_flags |= OCTA_FLASH;
#endif
  }
}

/*==============================================*/
/**
 * @fn        int32_t rsi_flash_mem_wr(rsi_flash_write_t *write_req)
 * @brief     Write content to flash memory.  
 * @param[in] write_req - Content to be written to flash memory     
 * @return    0  -  Success \n
 *            Non-Zero Value - Failure
 */

int32_t rsi_flash_mem_wr(rsi_flash_write_t *write_req)
{
  rsi_pkt_t *pkt;
  spi_config_t spi_configs;
  uint8_t status = 0;

  // Get WLAN cb struct pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    rsi_get_spi_config(write_req->flash_type, write_req->flash_pinset, &spi_configs);

    memcpy(pkt->data, write_req, sizeof(rsi_flash_write_t));

    memcpy(pkt->data + sizeof(rsi_flash_write_t), &spi_configs, sizeof(spi_config_t));

    // send  command
    status = rsi_driver_wlan_send_cmd(RSI_FLASH_WRITE, pkt);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  return status;
}

/*==============================================*/
/**
 * @fn        int32_t rsi_reset_mac()
 * @brief     Reset mac.   
 * @param[in] void  
 * @return    0  -  Success \n
 *            Non-Zero Value - Failure
 */
int32_t rsi_reset_mac()
{
  rsi_pkt_t *pkt;
  uint8_t status         = 0;
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;
  status                 = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }
    memset(pkt->desc, 0, 256);

    // Fill frame type
    pkt->desc[1] |= (RSI_WLAN_MGMT_Q << 4);

    // Fill frame type
    pkt->desc[2] = RSI_RESET_MAC;

    // Bypass RX path
    pkt->desc[8] = 1;

    // MAX retries
    pkt->desc[9] = 0xf;

    // PER mode enable
    pkt->desc[10] = 1;

    // Enqueue packet to WLAN TX queue
    rsi_enqueue_pkt(&rsi_driver_cb->wlan_tx_q, pkt);

    rsi_driver_cb->wlan_cb->expected_response = RSI_RESET_MAC;

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // Set TX packet pending event
    rsi_set_event(RSI_TX_EVENT);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_RESET_MAC_RESPONSE_WAIT_TIME);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // get wlan/network command response status
  status = rsi_wlan_get_status();

  return status;
}

/*==============================================*/
/**
 * @fn        uint8_t translate_channel(uint8_t channelNum)
 * @brief     Channel translate.   
 * @param[in] channelNum  - Channel number
 * @return    0  -  Success \n
 *            Non-Zero Value - Failure
 */

uint8_t translate_channel(uint8_t channelNum)
{
  if (channelNum) {
    if (channelNum < 9) {
      channelNum = (channelNum * 4 + 32);
    } else if (channelNum < 20) {
      channelNum = ((channelNum - 9) * 4 + 100);
    } else if (channelNum < 25) {
      channelNum = ((channelNum - 20) * 4) + 149;
    } /* End if <condition> */
  }
  return channelNum;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_radio_caps(uint8_t operating_channel)
 * @brief      Radio capabilities of the operating channel.   
 * @param[in]  operating_channel - Operating channel number 
 * @return     0  -  Success \n
 *             Non-Zero Value - Failure
 */

int32_t rsi_radio_caps(uint8_t operating_channel)
{
  rsi_pkt_t *pkt;
  uint8_t status = 0;
  uint32_t ii;
  uint8_t radio_id;
  radio_caps_t *radio_caps = NULL;

  uint16_t gc[20] = { 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                      0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0 };

  // Get WLAN cb struct pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    memset(pkt->desc, 0, 256);

    // Fill payload length
    rsi_uint16_to_2bytes(pkt->desc, (sizeof(radio_caps_t) & 0xFFF));

    // Fill frame type
    pkt->desc[1] |= (RSI_WLAN_MGMT_Q << 4);

    // Fill frame type
    pkt->desc[2] = RSI_RADIO_CAPS;

    if (operating_channel > 14) {
      rsi_uint16_to_2bytes(&pkt->desc[8], translate_channel(operating_channel));
    } else {
      rsi_uint16_to_2bytes(&pkt->desc[8], operating_channel);
    }

    // RF type
    pkt->desc[9] |= 1;

    pkt->desc[14] |= ONEBOX_LMAC_CLOCK_FREQ_40MHZ; //FIXME Radio config info

    radio_id = 0;

    pkt->desc[15] |= radio_id; //radio_id

    radio_caps = (radio_caps_t *)pkt->data;

    for (ii = 0; ii < MAX_HW_QUEUES; ii++) {
      radio_caps->qos_params[ii].cont_win_min_q = 3;
      radio_caps->qos_params[ii].cont_win_max_q = 0x3f;
      radio_caps->qos_params[ii].aifsn_val_q    = 2;
      radio_caps->qos_params[ii].txop_q         = 0;
    }

    // In PER mode, queue id have mix-7, max-0x3f, txop-0 and aifsn-2
    radio_caps->qos_params[1].cont_win_min_q = 7;
    radio_caps->qos_params[1].cont_win_max_q = 0x3f;
    radio_caps->qos_params[1].aifsn_val_q    = 2;
    radio_caps->qos_params[1].txop_q         = 0;

    ii = 0;
    //memcpy(radio_caps->gcpd_per_rate, &gc[0], 40);
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_1 & 0x00FF);    /*1*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_2 & 0x00FF);    /*2*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_5_5 & 0x00FF);  /*5.5*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_11 & 0x00FF);   /*11*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_48 & 0x00FF);   /*48*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_24 & 0x00FF);   /*24*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_12 & 0x00FF);   /*12*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_6 & 0x00FF);    /*6*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_54 & 0x00FF);   /*54*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_36 & 0x00FF);   /*36*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_18 & 0x00FF);   /*18*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_9 & 0x00FF);    /*9*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_MCS0 & 0x00FF); /*MCS0*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_MCS1 & 0x00FF); /*MCS1*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_MCS2 & 0x00FF); /*MCS2*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_MCS3 & 0x00FF); /*MCS3*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_MCS4 & 0x00FF); /*MCS4*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_MCS5 & 0x00FF); /*MCS5*/
    radio_caps->gcpd_per_rate[ii++] = (RSI_RATE_MCS6 & 0x00FF); /*MCS6*/
    radio_caps->gcpd_per_rate[ii]   = (RSI_RATE_MCS7 & 0x00FF); /*MCS7*/
    radio_caps->sifs_tx_11n         = 0x244;
    radio_caps->sifs_tx_11b         = 0x15a;
    radio_caps->slot_rx_11n         = 0x168;
    radio_caps->ofdm_ack_tout       = 0xaa0;
    radio_caps->cck_ack_tout_reg    = 0x24e0;
    radio_caps->preamble_type       = 0;

    // Enqueue packet to WLAN TX queue
    rsi_enqueue_pkt(&rsi_driver_cb->wlan_tx_q, pkt);

    rsi_driver_cb->wlan_cb->expected_response = RSI_RADIO_CAPS;

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // Set TX packet pending event
    rsi_set_event(RSI_TX_EVENT);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_RADIO_CAPS_RESPONSE_WAIT_TIME);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // get wlan/network command response status
  status = rsi_wlan_get_status();

  return status;
}

/*==============================================*/
/**
 * @fn        int32_t program_bb_rf()
 * @brief     Program radio frequency   
 * @param[in] void
 * @return    0  -  Success \n
 *            Non-Zero Value - Failure
 */

int32_t program_bb_rf()
{
  static uint32_t rf_reset = 1;
  rsi_pkt_t *pkt;
  uint8_t status   = 0;
  uint8_t endpoint = 0;

  // Get WLAN cb struct pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    memset(pkt->desc, 0, 256);

    // Fill frame type
    pkt->desc[1] |= (RSI_WLAN_MGMT_Q << 4);

    // Fill frame type
    pkt->desc[2] = RSI_BB_RF_INIT;

    pkt->desc[8] = endpoint;

    if (rf_reset) {
      pkt->desc[14] = (RF_RESET_ENABLE);
      rf_reset      = 0;
    }
    *(uint16_t *)&pkt->desc[14] |= (PUT_BBP_RESET | BB_RF_WRITE_PROG | (RSI_RF_TYPE << 4));

    // Enqueue packet to WLAN TX queue
    rsi_enqueue_pkt(&rsi_driver_cb->wlan_tx_q, pkt);

    rsi_driver_cb->wlan_cb->expected_response = RSI_BB_RF_INIT;

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // Set TX packet pending event
    rsi_set_event(RSI_TX_EVENT);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_BBRF_RESPONSE_WAIT_TIME);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // get wlan/network command response status
  status = rsi_wlan_get_status();

  return status;
}
/*==============================================*/
/**
 * @fn       int32_t set_channel(uint8_t channel_num, uint8_t power_level)
 * @brief    Set the operating channel.   
 * @param[in] channel_num - Operating channel number 
 * @param[in] power_level - power level
 * @return     0  -  Success \n
 *             Non-Zero Value - Failure
 */

int32_t set_channel(uint8_t channel_num, uint8_t power_level)
{
  rsi_pkt_t *pkt;
  uint8_t status   = 0;
  uint8_t endpoint = 0;

  // Get WLAN cb struct pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    memset(pkt->desc, 0, 256);

    // Fill frame type
    pkt->desc[1] |= (RSI_WLAN_MGMT_Q << 4);

    // Fill frame type
    pkt->desc[2] = RSI_SCAN_REQ;

    pkt->desc[8]  = channel_num;
    pkt->desc[9]  = 0; //RSI_ANTENNA_GAIN_2G;
    pkt->desc[10] = 0; //RSI_ANTENNA_GAIN_5G;

    pkt->desc[12] = power_level;

    pkt->desc[14] |= (PUT_BBP_RESET | BB_RF_WRITE_PROG | (RSI_RF_TYPE << 4));

    // Enqueue packet to WLAN TX queue
    rsi_enqueue_pkt(&rsi_driver_cb->wlan_tx_q, pkt);

    rsi_driver_cb->wlan_cb->expected_response = RSI_SCAN_REQ;

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // Set TX packet pending event
    rsi_set_event(RSI_TX_EVENT);
    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_SCAN_REQ_RESPONSE_WAIT_TIME);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }
  // get wlan/network command response status
  status = rsi_wlan_get_status();

  return 0;
}
/*==============================================*/
/**
 * @fn       int32_t rsi_feature_frame()
 * @brief    Select internal RF type or External RF type and clock frequency.   
 * @pre     \ref rsi_wireless_init() API needs to be called before this API 
 * @param[in] void 
 * @return     0  -  Success \n
 *             Non-Zero Value - Failure
 */

int32_t rsi_feature_frame()
{
  rsi_pkt_t *pkt;
  rsi_wlan_9116_features_t *features;
  uint8_t status = 0;

  // Get WLAN cb struct pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    features = (rsi_wlan_9116_features_t *)pkt->data;

    features->pll_mode            = 0x0;
    features->rf_type             = 0x1;
    features->wireless_mode       = 0x0;
    features->enable_ppp          = 0x0;
    features->afe_type            = 0x1;
    features->disable_programming = 0x0;

    // for PER mode
    features->feature_enables = (0x4 << LMAC_BEACON_DROP_FEATURES_THOLD_OFFET) | DUTY_CYCLING | END_OF_FRAME_DROP
                                | ASSOC_LP_CHAIN_ENABLE | (0x5 << LMAC_BEACON_DROP_ENABLES_OFFSET);

    // send  command
    status = rsi_driver_wlan_send_cmd(RSI_FEATURE_FRAME, pkt);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  return status;
}
/** @} */
/*40 MHz specific */
const uint8_t permanent_address[6] = { 0x88, 0xda, 0x1a, 0x11, 0x22, 0x33 };
const uint8_t per_packet[1500]     = { 0x0,  0x00, 0x00, 0x00, 0xc0, 0x08, 0x3a, 0x01, 0x00, 0x23,
                                   0xa7, 0x1b, 0x16, 0x38, 0x00, 0x23, 0xa7, 0x1b, 0x15, 0x1c,
                                   0x00, 0x23, 0xa7, 0x1b, 0x15, 0x1c, 0x10, 0x10, 0x02, 0x00 };

/** @addtogroup WLAN
* @{
*/

/*==============================================*/
/**
 * @fn        int32_t rsi_send_per_packet(tx_per_params_t *per_params, uint32_t pkt_size)
 * @brief     Sending packets   
 * @param[in] per_params -  This is the parameter 
 * @param[in] pkt_size   - Packet size
 * @return     0  -  Success \n
 *             Non-Zero Value - Failure
 */
int32_t rsi_send_per_packet(tx_per_params_t *per_params, uint32_t pkt_size)
{
  rsi_pkt_t *pkt;
  int32_t status = 0;

  uint8_t *data = NULL;
  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }
    memset(pkt->desc, 0, 256);

    data = pkt->data;

    rsi_uint16_to_2bytes(pkt->desc, (pkt_size & 0xFFF));
    // Fill frame type
    pkt->desc[1] |= (RSI_WLAN_DATA_Q << 4);

    // Add queue id for PER packets
    pkt->desc[14] = 1;

    // set the data rate
    pkt->desc[6] = 1 << 0; //setting rate info

    rsi_uint16_to_2bytes(&pkt->desc[8], per_params->rate);

#ifdef ENABLE_40MHz_BANDWIDTH
    if (operating_chwidth == BW_40Mhz) {
      pkt->desc[10] |= FULL_40M_ENABLE;
    }
#endif
    memcpy(&data[0], per_packet, pkt_size);
    // copy modules mac address into source address & bssid of mac address
    memcpy(&data[10], permanent_address, 6);
    memcpy(&data[16], permanent_address, 6);
    // Enqueue packet to WLAN TX queue
    rsi_enqueue_pkt(&rsi_driver_cb->wlan_tx_q, pkt);

    // Set TX packet pending event
    rsi_set_event(RSI_TX_EVENT);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // get wlan/network command response status
  status = rsi_wlan_get_status();

  return status;
}
/*==============================================*/
/**
 * @fn        int32_t rsi_send_per_frame(tx_per_params_t *tx_per_params)
 * @brief     Sending packets frame 
 * @param[in] tx_per_params -  This is the transmitte parameter 
 * @return     0  -  Success \n
 *             Non-Zero Value - Failure
 */

int32_t rsi_send_per_frame(tx_per_params_t *tx_per_params)
{
  rsi_pkt_t *pkt;
  tx_per_params_t *tx_per_params_p;
  uint8_t status = 0;

  // Get WLAN cb struct pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    memset(pkt->desc, 0, 256);

    tx_per_params_p = (tx_per_params_t *)pkt->data;

    // Fill payload length
    rsi_uint16_to_2bytes(pkt->desc, (sizeof(tx_per_params_t) & 0xFFF));

    // Fill frame type
    pkt->desc[1] |= (RSI_WLAN_MGMT_Q << 4);
    pkt->desc[2] = RSI_PER_CMD_PKT;
    pkt->desc[6] = tx_per_params->mode;

    memcpy(tx_per_params_p, tx_per_params, sizeof(tx_per_params_t));

    // Enqueue packet to WLAN TX queue
    rsi_enqueue_pkt(&rsi_driver_cb->wlan_tx_q, pkt);

    // Set TX packet pending event
    rsi_set_event(RSI_TX_EVENT);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }
  rsi_driver_cb->wlan_cb->expected_response = RSI_WLAN_RSP_CLEAR;

  // get wlan/network command response status
  status = rsi_wlan_get_status();

  return status;
}
/** @} */
uint16_t seq_num = 0;
/** @addtogroup WLAN
* @{
*/

/*==============================================*/
/**
 * @fn         uint32_t rsi_prepare_per_pkt(tx_per_params_t *per_params, uint16_t length)
 * @brief      Preparing per packet
 * @param[in]  per_params  - Parameters
 * @param[in]  length - Length 
 * @return     0  -  Success \n
 *             Non-Zero Value - Failure
 */
uint32_t rsi_prepare_per_pkt(tx_per_params_t *per_params, uint16_t length)
{
#define FRAME_DESC_SZ      16
#define MIN_802_11_HDR_LEN 24
#define BROADCAST_IND      BIT(9)
#define ENABLE_MAC_INFO    BIT(0)
#define CONTINUOUS_MODE    BIT(10)

  rsi_pkt_t *pkt;
  uint8_t status                = 0;
  ieee80211_macframe_t *tmp_hdr = NULL;
  uint8_t *data                 = NULL;
  uint8_t size_of_hdr; // frame_desc + MAC_HDR + Extended_desc   16 + 24 + 4
  /* For future use */
  uint8_t extended_desc = 4;

  uint8_t mac_addr[6] = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5 };
  uint8_t mac_hdr[24] = { 0x8,  0x1,  0,    0,    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0x0, 0x23,
                          0xa7, 0xa1, 0xa2, 0xa3, 0x0,  0x23, 0xa7, 0xa1, 0xa2, 0xa3, 0,   0 };
  uint8_t offset = 0, min_len = 0;
  uint32_t rate_flags, greenfield, ch_bw, i = 0;
  uint32_t *frame_desc, temp_word;
  // Get WLAN cb struct pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    memset(pkt->desc, 0, 256);

    size_of_hdr = FRAME_DESC_SZ + extended_desc + MIN_802_11_HDR_LEN;

    frame_desc = (uint32_t *)pkt->desc;
    data       = (uint8_t *)pkt->desc;

    *(uint16_t *)frame_desc = (length - FRAME_DESC_SZ) & 0xfff;

    if (length >= (MIN_802_11_HDR_LEN + FRAME_DESC_SZ + extended_desc)) {
      frame_desc[1] = (MIN_802_11_HDR_LEN << 8);
    }
    if (per_params->aggr_enable) {
      temp_word = ((ENABLE_MAC_INFO) << 16); //In mac_info set bit0 and bit9 for bcast pkt
      frame_desc[3] |= (QOS_EN);             // Indicating of QOS in mac_flags
    } else {
      temp_word = ((BROADCAST_IND | ENABLE_MAC_INFO) << 16); //In mac_info set bit0 and bit9 for bcast pkt
    }
    rate_flags = (per_params->rate_flags << 2);
    /*FIXME: Put macros for these mask values */
    ch_bw      = (rate_flags & 0x00F0);
    greenfield = (rate_flags & 0x0008);
    greenfield = (greenfield << 17);
    if (rate_flags & 0x0004) //checking short_GI
    {
      per_params->rate |= BIT(9);
    }
    if (per_params->mode) {
      rate_flags |= CONTINUOUS_MODE;
    }

    /* Rates are sent in word 5 and channel bandwidth in word 6
	 * */
    frame_desc[1] |= (temp_word | extended_desc);
    frame_desc[2] = ((per_params->rate & 0x3ff) | (ch_bw << 12) | greenfield);
    frame_desc[4] = (per_params->power);
    if (per_params->mode == PER_CONT_MODE) {
      frame_desc[4] |= (3 << 8);
      //txPkt->flags  |= CONT_TRANSFER;
    }
    offset += FRAME_DESC_SZ + extended_desc;
    /* Form the Mac header for the PER Packet to be transmitted */
    tmp_hdr = (ieee80211_macframe_t *)&data[offset];

    if (length >= size_of_hdr) {
      tmp_hdr->fc[0] = 0x8;
      tmp_hdr->fc[1] = 0x01;

      tmp_hdr->dur[0] = 0x00;
      tmp_hdr->dur[1] = 0x00;
      memcpy(tmp_hdr->addr1, mac_addr, MAC_ADDR_LEN);
      memcpy(tmp_hdr->addr2, permanent_address, MAC_ADDR_LEN);
      memcpy(tmp_hdr->addr3, permanent_address, MAC_ADDR_LEN);

      offset += MIN_802_11_HDR_LEN;

      /*Fill the PER packet with dummy data */
      for (i = offset; i <= (length); i += 4) {
        data[i]     = 0xff;
        data[i + 1] = 0x00;
        data[i + 2] = 0xbb;
        data[i + 3] = 0x55;
      }
      if (((length - offset) % 4))
        memset(&data[i - 4], 0xdd, ((length - offset) % 4));
    } else {

      memcpy(tmp_hdr, mac_hdr, min_len);
    }
    data[1] |= (RSI_WLAN_DATA_Q << 4);
    if (per_params->aggr_enable == 1)
      data[14] |= 1; // send pkts in queue no: 1
    else
      data[14] |= 1; // send pkts in queue no: 1
    if (length > (FRAME_DESC_SZ + MIN_802_11_HDR_LEN + extended_desc)) {
      data[FRAME_DESC_SZ + MIN_802_11_HDR_LEN + extended_desc - 2] = (seq_num << 4) & 0xff;
      data[FRAME_DESC_SZ + MIN_802_11_HDR_LEN + extended_desc - 1] = ((seq_num << 4) >> 8);
      seq_num                                                      = ((seq_num + 1) % 4096);
    }

    // Enqueue packet to WLAN TX queue
    rsi_enqueue_pkt(&rsi_driver_cb->wlan_tx_q, pkt);

    // Set TX packet pending event
    rsi_set_event(RSI_TX_EVENT);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // get wlan/network command response status
  status = rsi_wlan_get_status();

  return status;
}

/*==============================================*/
/**
 * @fn        int32_t rsi_process_transmit(tx_per_params_t *per_params)
 * @brief     Transmit packets frame 
 * @param[in] tx_per_params -  This is the transmitte parameter 
 * @return     0  -  Success \n
 *             Non-Zero Value - Failure
 */
int32_t rsi_process_transmit(tx_per_params_t *per_params)
{
  uint32_t length;
  int32_t status = 0;

  per_params->aggr_count = (per_params->length / PER_AGGR_LIMIT_PER_PKT);
  if (per_params->aggr_count == 0) {
    per_params->aggr_count = 1; // added this check to ensure aggr_count value is not 0,
  }
  if (per_params->enable) {
#ifdef CONFIG_IEEE_80211J
    if (sme_info_nonrom_g.enable_11j && ((Origregdmn == REGION_JAP_NUM))) {
      if (!valid_80211J_channel(per_params->channel)) {
        status = RSI_ERROR_INVALID_PARAM;
      }
      if (((per_params->rate == 0) || (per_params->rate == 2) || (per_params->rate == 4) || (per_params->rate == 6))) {
        status = RSI_ERROR_INVALID_PARAM;
      }

    } else
#endif
      if (per_params->channel <= 14) {

    } else if (((per_params->channel >= 36) && (per_params->channel <= 165))) {
      /*Check for b rates, Only a/n rates are valid*/
      if ((per_params->rate == 0) || (per_params->rate == 2) || (per_params->rate == 4) || (per_params->rate == 6)) {
        status = RSI_ERROR_INVALID_PARAM;
      }
    } else {
      status = RSI_ERROR_INVALID_PARAM;
    }

    // set channel number
#if 0
		if(operating_band == BAND_5GHZ
				&& (Origregdmn != REGION_JAP_NUM) )
		{
			cur_scan_channel = reverse_translate_channel(per_params->channel);
		}
		else if (Origregdmn == REGION_JAP_NUM)
		{
			cur_scan_channel = get_80211J_channel(adapter,per_params->channel);
		}
		else
		{
			cur_scan_channel = per_params->channel;
		}
#endif
    if (set_channel(per_params->channel, per_params->power)) {
      status = RSI_ERROR_INVALID_PARAM;
    }
#ifdef CW_MODE_ENABLED
    if ((per_params->mode >= 2) && (per_params->mode <= 6)) {
      cw_mode_start_stop(per_params, 0);
      break;
    }
#endif
    if (per_params->mode == PER_CONT_MODE) {
      if (per_params->aggr_enable == 1) {
        length = (per_params->length - ((per_params->aggr_count - 1) * PER_AGGR_LIMIT_PER_PKT));
      } else
        length =
          (per_params
             ->length); // > FIRST_SCATTER_AVALIABLE_LENGTH ) ? FIRST_SCATTER_AVALIABLE_LENGTH : per_params->length;
      if ((per_params->length < 24) || (per_params->length > 260)) {
        status = RSI_ERROR_INVALID_PARAM;
      }

      rsi_prepare_per_pkt(per_params, (length + FRAME_DESC_SZ));
    }

    if (per_params->mode == PER_BURST_MODE) {
      if (per_params->rate_flags & 0x01) //check for Short GI enable
        per_params->rate |= BIT(9);      //set Bit 9 to enable short GI in rate
      if ((per_params->length < 24) || (per_params->length > 1500)) {
        status = RSI_ERROR_INVALID_PARAM;
      }

      status = rsi_send_per_frame(per_params);
      if (per_params->aggr_enable == 1) {
      } else {
        length = (per_params->length > MAX_PER_PACKET_SIZE) ? MAX_PER_PACKET_SIZE : per_params->length;
        status = rsi_send_per_packet(per_params, length);
      }
    }
  } else {
    // disable per
#ifdef CW_MODE_ENABLED
    if (cw_iter > 0) {
      if (set_channel(per_params->channel, per_params->power_level)) {
        status = RSI_ERROR_INVALID_PARAM;
      }
      cw_mode_start_stop(per_params, 2);
    } else
#endif
      status = rsi_send_per_frame(per_params);
  }
  return status;
}

/*==============================================*/
/**
 * @fn         int32_t rsi_efuse_write(rsi_efuse_write_t *efuse_write_p)
 * @brief      Write content to efuse. 
 * @param[in]  efuse_write_p - write efuse content 
 * @return     0  -  Success \n
 *             Non-Zero Value - Failure
 * 
 */
int32_t rsi_efuse_write(rsi_efuse_write_t *efuse_write_p)
{

  rsi_pkt_t *pkt;
  int8_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Take the user provided data and fill it in antenna select structure
    memcpy(pkt->data, (uint8_t *)efuse_write_p, sizeof(rsi_efuse_write_t) + efuse_write_p->length);

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send antenna select command
    status = rsi_driver_wlan_send_cmd(RSI_EFUSE_WRITE_REQ, pkt);
    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_EFUSE_WRITE_RESPONSE_WAIT_TIME);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status
  return status;
}

/*==============================================*/
/**
 * @fn        int32_t rsi_efuse_read(rsi_efuse_read_t *efuse_read_p, uint8_t *buf, uint32_t length)
 * @brief     Get efuse content.
 * @param[in] efuse_read_p - Read efuse content 
 * @param[in] buf          - Pointer to buffer to store efuse content 
 * @param[in] length       - length of the efuse content 
 * @return    0  -  Success \n
 *            Non-Zero Value - Failure
 * 
 */
int32_t rsi_efuse_read(rsi_efuse_read_t *efuse_read_p, uint8_t *buf, uint32_t length)
{

  rsi_pkt_t *pkt;
  int8_t status = RSI_SUCCESS;

  // Get wlan cb structure pointer
  rsi_wlan_cb_t *wlan_cb = rsi_driver_cb->wlan_cb;

  status = rsi_check_and_update_cmd_state(WLAN_CMD, IN_USE);
  if (status == RSI_SUCCESS) {
    // allocate command buffer  from wlan pool
    pkt = rsi_pkt_alloc(&wlan_cb->wlan_tx_pool);

    // If allocation of packet fails
    if (pkt == NULL) {
      //Changing the wlan cmd state to allow
      rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);
      // return packet allocation failure error
      return RSI_ERROR_PKT_ALLOCATION_FAILURE;
    }

    // Take the user provided data and fill it in antenna select structure
    memcpy(pkt->data, (uint8_t *)efuse_read_p, sizeof(rsi_efuse_read_t));

    // attach the buffer given by user
    wlan_cb->app_buffer = (uint8_t *)buf;

    // length of the buffer provided by user
    wlan_cb->app_buffer_length = length;

#ifndef RSI_WLAN_SEM_BITMAP
    rsi_driver_cb_non_rom->wlan_wait_bitmap |= BIT(0);
#endif
    // send antenna select command
    status = rsi_driver_wlan_send_cmd(RSI_EFUSE_READ_REQ, pkt);

    // wait on wlan semaphore
    rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->wlan_cmd_sem, RSI_EFUSE_READ_RESPONSE_WAIT_TIME);

    //Changing the wlan cmd state to allow
    rsi_check_and_update_cmd_state(WLAN_CMD, ALLOW);

  } else {
    // return wlan command error
    return status;
  }

  // Return the status
  return status;
}
#endif
/** @} */
