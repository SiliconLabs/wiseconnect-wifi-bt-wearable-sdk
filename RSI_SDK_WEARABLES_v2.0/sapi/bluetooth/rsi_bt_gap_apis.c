/*******************************************************************************
* @file  rsi_bt_gap_apis.c
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

#ifdef RSI_BT_ENABLE
#include "rsi_driver.h"
#include "rsi_bt.h"
#include "rsi_bt_apis.h"
#include "rsi_bt_config.h"

/** @addtogroup BT-CLASSIC3
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_bt_set_local_class_of_device(uint32_t class_of_device)
 * @brief      Set the given name to the local device COD
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  class_of_device - name to be set to the local device COD \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_set_local_class_of_device(uint32_t class_of_device)
{
  rsi_bt_req_set_local_cod_t bt_req_set_local_cod = { 0 };
  bt_req_set_local_cod.class_of_device            = class_of_device;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_LOCAL_COD, &bt_req_set_local_cod, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_local_class_of_device(uint8_t *resp)
 * @brief      Know the local device COD.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[out] resp - response buffer to hold the response of this API \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_get_local_class_of_device(uint8_t *resp)
{
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_QUERY_LOCAL_COD, NULL, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_start_discoverable(void)
 * @brief      Set the BT Module in discovery mode.
 * @pre        \ref rsi_wireless_init API needs to be called before this API
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_start_discoverable(void)
{
  rsi_bt_req_set_discv_mode_t bt_req_set_discv_mode = { 0 };
  bt_req_set_discv_mode.mode                        = START_DISCOVERY;
  bt_req_set_discv_mode.time_out                    = 0;
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_DISCV_MODE, &bt_req_set_discv_mode, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_start_limited_discoverable(int32_t time_out_ms)
 * @brief      Request the local device to enter limited discovery mode. 
 * 			   The device comes out of discovery mode after the time out. 
 *       	   If the 'time_out_ms' is set to '0', the device will be in continuous discoverable mode.
 * @pre        \ref rsi_wireless_init API needs to be called before this API. 
 * @param[in]  time_out_ms - Limited discovery mode time_out in ms. \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_start_limited_discoverable(int32_t time_out_ms)
{
  rsi_bt_req_set_discv_mode_t bt_req_set_discv_mode = { 0 };
  bt_req_set_discv_mode.mode                        = START_LIMITED_DISCOVERY;
  bt_req_set_discv_mode.time_out                    = time_out_ms;
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_DISCV_MODE, &bt_req_set_discv_mode, NULL);
}
/*==============================================*/
/**
 * @fn         int32_t rsi_bt_write_current_iac_lap(uint8_t no_of_iaps, uint8_t *iap_lap_list)
 * @brief      Write the current iac lap to the controller.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  no_of_iaps - number of iap lap count \n 
 * @param[in]  iap_lap_list - pointer to the iap laps list \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
#define IAP_LAP_SIZE 3
int32_t rsi_bt_write_current_iac_lap(uint8_t no_of_iaps, uint8_t *iap_lap_list)
{
  rsi_bt_req_set_current_iap_lap_t bt_req_iap_lap = { 0 };
  uint8_t ix                                      = 0;
  uint8_t len                                     = 0;

  bt_req_iap_lap.cnt = no_of_iaps;

  for (ix = 0; ix < no_of_iaps; ix++) {
    len = ix * IAP_LAP_SIZE;
    memcpy(&bt_req_iap_lap.list[len], (uint8_t *)&iap_lap_list[len], IAP_LAP_SIZE);
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_CURRENT_IAC_LAP, &bt_req_iap_lap, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_stop_discoverable(void)
 * @brief      Exit the BT Module from discovery mode.
 * @pre        \ref rsi_wireless_init API needs to be called before this API. 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_stop_discoverable(void)
{
  rsi_bt_req_set_discv_mode_t bt_req_set_discv_mode = { 0 };
  bt_req_set_discv_mode.mode                        = STOP_DISCOVERY;
  bt_req_set_discv_mode.time_out                    = 0;
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_DISCV_MODE, &bt_req_set_discv_mode, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_discoverable_status(uint8_t *resp)
 * @brief      Get BT Module discovery mode status.
 * @pre        \ref rsi_bt_start_discoverable() API need to be called before this API.
 * @param[out] resp - response buffer to hold the response of this API \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
// Change this
int32_t rsi_bt_get_discoverable_status(uint8_t *resp)
{
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_QUERY_DISCV_MODE, NULL, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_set_connectable(void)
 * @brief      Set the BT Module in connectable mode.
 * @pre        \ref rsi_wireless_init API needs to be called before this API.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_set_connectable(void)
{
  rsi_bt_req_set_connectability_mode_t bt_req_set_connectability_mode = { 0 };
  bt_req_set_connectability_mode.connectability_mode                  = CONNECTABLE;
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_CONNECTABILITY_MODE, &bt_req_set_connectability_mode, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_set_afh_host_channel_classification(uint8_t enable, uint8_t *channel_map)
 * @brief      Set the No.of channels to be used
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  enable - This parameter is used to enable or disable afh host channel classification. \n
 *                      This parameter supports the following values. \n 
 *			            0 - Disable \n
 *			            1 - Enable \n
 * @param[in]  channel_map - Array of channel mapping values. \n
 *                           Channel map range: This parameter contains 80 1-bit fields. \n
 *                           The nth such field (in the range 0 to 78 bits) contains the value for channel n: \n
 *			                 0: channel n is bad \n
 *			                 1: channel n is unknown \n
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_set_afh_host_channel_classification(uint8_t enable, uint8_t *channel_map)
{
  rsi_bt_req_set_afh_host_channel_classification_t afh_host_channel_classification = { 0 };

  afh_host_channel_classification.channel_assessment_mode = RSI_WRITE_ASSESSMENT_MODE;
  afh_host_channel_classification.enable                  = enable;
  if (channel_map != NULL) {
    memcpy(afh_host_channel_classification.channel_classification, channel_map, CHANNEL_MAP_LEN);
  }
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_AFH_HOST_CHANNEL_CLASSIFICATION, &afh_host_channel_classification, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_afh_host_channel_classification(uint8_t *status)
 * @brief      Get the afh channel assessmnet mode
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[out] status - This parameter is to hold the response of this API. \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_get_afh_host_channel_classification(uint8_t *status)
{
  rsi_bt_req_set_afh_host_channel_classification_t afh_host_channel_classification = { 0 };

  afh_host_channel_classification.channel_assessment_mode = RSI_READ_ASSESSMENT_MODE;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_AFH_HOST_CHANNEL_CLASSIFICATION,
                                &afh_host_channel_classification,
                                status);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_set_non_connectable(void)
 * @brief      Set the BT Module in non-connectable mode.
 * @pre        \ref rsi_wireless_init API need to be called before this API.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 */
int32_t rsi_bt_set_non_connectable(void)
{
  rsi_bt_req_set_connectability_mode_t bt_req_set_connectability_mode = { 0 };
  bt_req_set_connectability_mode.connectability_mode                  = NON_CONNECTABLE;
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_CONNECTABILITY_MODE, &bt_req_set_connectability_mode, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_connectable_status(uint8_t *resp)
 * @brief      Get BT Module connectablity status.
 * @pre        \ref rsi_bt_set_connectable() API needs to be called before this API
 * @param[out] resp - response buffer to hold the response of this API \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_get_connectable_status(uint8_t *resp)
{
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_QUERY_CONNECTABILITY_MODE, NULL, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_enable_authentication(void)
 * @brief      Request the local device to enable authentication.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_enable_authentication(void)
{
  rsi_bt_req_set_pair_mode_t bt_req_set_pair_mode = { 0 };
  bt_req_set_pair_mode.pair_mode                  = RSI_ENABLE;
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_PAIR_MODE, &bt_req_set_pair_mode, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_disable_authentication(void)
 * @brief      Request the local device to disable authentication.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_disable_authentication(void)
{
  rsi_bt_req_set_pair_mode_t bt_req_set_pair_mode = { 0 };
  bt_req_set_pair_mode.pair_mode                  = RSI_DISABLE;
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_PAIR_MODE, &bt_req_set_pair_mode, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_authentication(void)
 * @brief      Request authentication.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_get_authentication(void)
{
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_QUERY_PAIR_MODE, NULL, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_remote_name_request_async(uint8_t *remote_dev_addr,
 *                                        rsi_bt_event_remote_device_name_t *bt_event_remote_device_name)
 * @brief      Know the remote device name.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  remote_dev_addr - remote device address 
 * @param[out] bt_event_remote_device_name - response buffer to hold the name of remote device  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_remote_name_request_async(uint8_t *remote_dev_addr,
                                         rsi_bt_event_remote_device_name_t *bt_event_remote_device_name)
{
  rsi_bt_req_remote_name_t bt_req_remote_name = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_remote_name.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_remote_name.dev_addr, remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_REMOTE_NAME_REQUEST,
                                &bt_req_remote_name,
                                (void *)bt_event_remote_device_name);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_remote_name_request_cancel(uint8_t *remote_dev_addr)
 * @brief      Cancel the remote device name request.
 * @pre        \ref rsi_bt_remote_name_request_async() API needs to be called before this API.
 * @param[in]  remote_dev_addr - remote device address 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_remote_name_request_cancel(uint8_t *remote_dev_addr)
{
  rsi_bt_req_remote_name_cancel_t bt_req_remote_name_cancel = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_remote_name_cancel.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_remote_name_cancel.dev_addr, remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_REMOTE_NAME_REQUEST_CANCEL, &bt_req_remote_name_cancel, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_inquiry(uint8_t inquiry_type, uint32_t inquiry_duration, uint8_t max_devices)
 * @brief      Start inquiry.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  inquiry_type - This is the Inquiry type. \n 
 *			   0 - Standard Inquiry \n
 *			   1 - Inquiry with RSSI \n
 *			   2 - Extended Inquiry \n
 * @param[in]  inquiry_duration - duration of inquiry. \n 
 * @param[in]  max_devices - This is the maximum number of devices allowed to inquiry from 1 to 10  \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_inquiry(uint8_t inquiry_type, uint32_t inquiry_duration, uint8_t max_devices)
{
  rsi_bt_req_inquiry_t bt_req_inquiry = { 0 };
  if (inquiry_type & BIT(7)) {
    bt_req_inquiry.lap[0] = 0x00;
    bt_req_inquiry.lap[1] = 0x8B;
    bt_req_inquiry.lap[2] = 0x9E;
  } else {
    bt_req_inquiry.lap[0] = 0x33;
    bt_req_inquiry.lap[1] = 0x8B;
    bt_req_inquiry.lap[2] = 0x9E;
  }

  bt_req_inquiry.inquiry_type            = (inquiry_type & ~(BIT(7)));
  bt_req_inquiry.duration                = inquiry_duration;
  bt_req_inquiry.maximum_devices_to_find = max_devices;
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_INQUIRY, &bt_req_inquiry, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_cancel_inquiry(void)
 * @brief      Cancel inquiry.
 * @pre        rsi_wireless_init() API need to be called before this API
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_cancel_inquiry(void)
{
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_INQUIRY_CANCEL, NULL, NULL);
}
/** @} */
/** @addtogroup BT-CLASSIC5
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_enable_device_under_testmode(void)
 * @brief      Keep the device in the LMP_TEST_MODE.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_enable_device_under_testmode(void)
{
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_ENABLE_DEVICE_UNDER_TESTMODE, NULL, NULL);
}
/** @} */

/** @addtogroup BT-CLASSIC3
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_connect(uint8_t *remote_dev_addr)
 * @brief      Initiate the connection request
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  remote_dev_addr - remote device address  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_connect(uint8_t *remote_dev_addr)
{
  rsi_bt_req_connect_t bt_req_connect = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_connect.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_connect.dev_addr, remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_BOND, &bt_req_connect, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_gatt_connect(uint8_t *remote_dev_addr)
 * @brief      IAP_LAP_SIZEnitiate the gatt connection request
 * @param[in]  remote_dev_addr - remote device address 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */

int32_t rsi_bt_gatt_connect(uint8_t *remote_dev_addr)
{
  rsi_bt_req_gatt_connect_t bt_req_gatt_connect = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_gatt_connect.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_gatt_connect.dev_addr, remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_GATT_CONNECT, &bt_req_gatt_connect, NULL);
}
/** @} */
/** @addtogroup BT-CLASSIC3
* @{
*/

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_cancel_connect(uint8_t *remote_dev_address)
 * @brief      Cancel the connection request
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  remote_dev_address - remote device address  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_cancel_connect(uint8_t *remote_dev_address)
{
  rsi_bt_req_connect_cancel_t bt_req_connect_cancel = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_connect_cancel.dev_addr, remote_dev_address);
#else
  memcpy(bt_req_connect_cancel.dev_addr, remote_dev_address, 6);
#endif
  // Send connect cancel command
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_BOND_CANCEL, &bt_req_connect_cancel, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_disconnect(uint8_t *remote_dev_address)
 * @brief      Disconnect the physical connection
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  remote_dev_address - remote device address \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_disconnect(uint8_t *remote_dev_address)
{
  rsi_bt_req_disconnect_t bt_req_disconnect = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_disconnect.dev_addr, remote_dev_address);
#else
  memcpy(bt_req_disconnect.dev_addr, remote_dev_address, 6);
#endif
  // Send disconnect command
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_UNBOND, &bt_req_disconnect, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_accept_ssp_confirm(uint8_t *remote_dev_address)
 * @brief      Confirmation for the passkey sent by local BT device at the time of pairing. 
 * @pre        \ref rsi_bt_set_ssp_mode() API need to be called before this API 
 * @param[in]  remote_dev_address - remote device address \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_accept_ssp_confirm(uint8_t *remote_dev_address)
{
  rsi_bt_req_user_confirmation_t bt_req_user_confirmation;
  memset(&bt_req_user_confirmation, 0, sizeof(bt_req_user_confirmation));

  bt_req_user_confirmation.confirmation = ACCEPT;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_user_confirmation.dev_addr, remote_dev_address);
#else
  memcpy(bt_req_user_confirmation.dev_addr, remote_dev_address, 6);
#endif

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_USER_CONFIRMATION, &bt_req_user_confirmation, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_reject_ssp_confirm(uint8_t *remote_dev_address)
 * @brief      Reject the confirmation for the passkey sent by the local 
 *             BT device at the time of pairing.
 * @pre        \ref rsi_bt_set_ssp_mode() API need to be called bef ore this API 
 * @param[in]  remote_dev_addr - remote device address  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_reject_ssp_confirm(uint8_t *remote_dev_address)
{
  rsi_bt_req_user_confirmation_t bt_req_user_confirmation;
  memset(&bt_req_user_confirmation, 0, sizeof(bt_req_user_confirmation));

  bt_req_user_confirmation.confirmation = REJECT;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_user_confirmation.dev_addr, remote_dev_address);
#else
  memcpy(bt_req_user_confirmation.dev_addr, remote_dev_address, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_USER_CONFIRMATION, &bt_req_user_confirmation, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_passkey(uint8_t *remote_dev_addr, uint32_t passkey, uint8_t reply_type)
 * @brief      Send the passkey or rejects the incoming pass key request.
 * @pre        \ref rsi_bt_spp_connect() and \ref rsi_bt_on_passkey_request() APIs 
 *             need to be called before this API.
 * @param[in]  remote_dev_addr - remote device address  
 * @param[in]  passkey - This is the passkey input 
 * @param[in]  reply_type -This is the positive or negative reply \n 
 *                         0 - negative reply \n
 *			               1 - positive reply 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_passkey(uint8_t *remote_dev_addr, uint32_t passkey, uint8_t reply_type)
{
  rsi_bt_req_passkey_reply_t bt_req_passkey_reply;
  memset(&bt_req_passkey_reply, 0, sizeof(bt_req_passkey_reply));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_passkey_reply.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_passkey_reply.dev_addr, remote_dev_addr, 6);
#endif
  bt_req_passkey_reply.pass_key   = passkey;
  bt_req_passkey_reply.reply_type = reply_type;
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_PASSKEY_REPLY, &bt_req_passkey_reply, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_pincode_request_reply(uint8_t *remote_dev_addr, 
 *                                                  uint8_t *pin_code, uint8_t reply_type)
 * @brief      Send the pincode or rejects the incoming pincode request.
 * @pre        \ref rsi_bt_ssp_mode() and \ref rsi_bt_app_on_pincode_req() APIs 
 *             need to be called before this API. 
 * @param[in]  remote_dev_addr - remote device address  
 * @param[in]  pin_code - pin code input  
 * @param[in]  reply_type - This is the positive or negative reply \n
 *                          0 - negative reply \n
 *        			        1 - positive reply 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_pincode_request_reply(uint8_t *remote_dev_addr, uint8_t *pin_code, uint8_t reply_type)
{
  rsi_bt_req_pincode_reply_t bt_req_pincode_reply;
  memset(&bt_req_pincode_reply, 0, sizeof(bt_req_pincode_reply));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_pincode_reply.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_pincode_reply.dev_addr, remote_dev_addr, 6);
#endif
  bt_req_pincode_reply.reply_type = reply_type;
  memcpy(bt_req_pincode_reply.pincode, pin_code, RSI_MIN(strlen((const char *)pin_code), RSI_MAX_PINCODE_REPLY_SIZE));

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_PINCODE_REPLY, &bt_req_pincode_reply, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_linkkey_request_reply(uint8_t *remote_dev_addr, 
 *                                                  uint8_t *linkkey, 
 *													uint8_t reply_type)
 * @brief      Sends either positive (along with the link key) or
 * negative reply to the incoming link key request.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  remote_dev_addr - remote device address
 * @param[in]  linkkey - Linkkey input
 * @param[in]  reply_type - Positive or negative reply \n
 *                          0 - negative reply \n
 *			                1 - positive reply 
* @return      0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_linkkey_request_reply(uint8_t *remote_dev_addr, uint8_t *linkkey, uint8_t reply_type)
{
  rsi_bt_req_linkkey_reply_t bt_linkkey_req_reply;
  memset(&bt_linkkey_req_reply, 0, sizeof(bt_linkkey_req_reply));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_linkkey_req_reply.dev_addr, remote_dev_addr);
#else
  memcpy(bt_linkkey_req_reply.dev_addr, remote_dev_addr, 6);
#endif
  bt_linkkey_req_reply.reply_type = reply_type;
  if (linkkey != NULL) {
    memcpy(bt_linkkey_req_reply.linkkey, linkkey, 16);
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_LINKKEY_REPLY, &bt_linkkey_req_reply, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_local_device_role(uint8_t *remote_dev_addr, uint8_t *resp)
 * @brief      Request the role of local device.
 * @pre        \ref rsi_bt_set_ssp_mode() API needs to be called before this API. 
 * @param[in]  remote_dev_addr - remote device address  
 * @param[out] resp - response buffer to hold the response of this API  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_get_local_device_role(uint8_t *remote_dev_addr, uint8_t *resp)
{
  rsi_bt_req_query_role_t bt_req_query_role = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_query_role.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_query_role.dev_addr, remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_QUERY_ROLE, &bt_req_query_role, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_set_local_device_role(uint8_t *remote_dev_addr, 
 *                                                  uint8_t set_role, uint8_t *resp)
 * @brief      Set the role of local device.
 * @pre        \ref rsi_bt_set_ssp_mode() API needs to be called before this API. 
 * @param[in]  remote_dev_addr - remote device address 
 * @param[in]  set_role - This paramets sets either Master/Slave Role \n
               0  Master Role \n
               1  Slave Role 
 * @param[out] resp - response buffer to hold the response of this API 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_set_local_device_role(uint8_t *remote_dev_addr, uint8_t set_role, uint8_t *resp)
{
  rsi_bt_req_set_role_t bt_req_set_role;
  memset(&bt_req_set_role, 0, sizeof(bt_req_set_role));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_set_role.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_set_role.dev_addr, remote_dev_addr, 6);
#endif
  bt_req_set_role.role = set_role;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_ROLE, &bt_req_set_role, resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_get_services_async(uint8_t *remote_dev_addr, 
 *                                               rsi_bt_resp_query_services_t *bt_resp_query_services)
 * @brief      Query the services of local device.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  remote_dev_addr - Remote device address 
 * @param[out] bt_resp_query_services - Response struct to hold the response of this API  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure  
 */
//int32_t rsi_bt_get_services(uint8_t *remote_dev_addr, uint32_t *resp, uint16_t num_of_responses)
int32_t rsi_bt_get_services_async(uint8_t *remote_dev_addr, rsi_bt_resp_query_services_t *bt_resp_query_services)
{
  rsi_bt_req_query_services_t bt_req_query_services = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_query_services.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_query_services.dev_addr, remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_QUERY_SERVICES, &bt_req_query_services, bt_resp_query_services);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_search_service_async(uint8_t *remote_dev_addr, uint32_t service_uuid)
 * @brief      Search service of the given uuid.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  remote_dev_addr - remote device address 
 * @param[in]  service_uuid - uuid of the service for search  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_search_service_async(uint8_t *remote_dev_addr, uint32_t service_uuid)
{
  rsi_bt_req_search_service_t bt_req_search_service;
  memset(&bt_req_search_service, 0, sizeof(bt_req_search_service));
  bt_req_search_service.service_uuid = service_uuid;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_search_service.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_search_service.dev_addr, remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SEARCH_SERVICE, &bt_req_search_service, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_sniff_mode(uint8_t *remote_dev_addr,
 *                                       uint16_t sniff_max_intv,
 *                                       uint16_t sniff_min_intv,
 *                                       uint16_t sniff_attempt,
 *                                       uint16_t sniff_tout)
 * @brief      Request the local device to enter into sniff mode.
 * @pre        \ref rsi_bt_spp_connect() API needs to be called before this API.
 * @param[in]  remote_dev_addr - remote device address  
 * @param[in]  sniff_max_intv -  Sniff maximum Interval  
 * @param[in]  sniff_min_intv - Sniff Minimum Interval  
 * @param[in]  sniff_attempt - Sniff Attempt 
 * @param[in]  sniff_tout - Sniff timeout 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_sniff_mode(uint8_t *remote_dev_addr,
                          uint16_t sniff_max_intv,
                          uint16_t sniff_min_intv,
                          uint16_t sniff_attempt,
                          uint16_t sniff_tout)
{
  rsi_bt_req_sniff_mode_t bt_req_sniff_mode;
  memset(&bt_req_sniff_mode, 0, sizeof(bt_req_sniff_mode));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_sniff_mode.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_sniff_mode.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  bt_req_sniff_mode.sniff_max_intv = sniff_max_intv;
  bt_req_sniff_mode.sniff_min_intv = sniff_min_intv;
  bt_req_sniff_mode.sniff_attempt  = sniff_attempt;
  bt_req_sniff_mode.sniff_tout     = sniff_tout;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SNIFF_MODE, &bt_req_sniff_mode, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_sniff_exit_mode(uint8_t *remote_dev_addr)
 * @brief      Request the local device to exit from sniff/sniff subrating mode.
 * @pre        \ref rsi_bt_sniff_mode() API needs to be called before this API.
 * @param[in]  remote_dev_addr - remote device address \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_sniff_exit_mode(uint8_t *remote_dev_addr)
{
  rsi_bt_req_sniff_exit_mode_t bt_req_sniff_exit_mode = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_sniff_exit_mode.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_sniff_exit_mode.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SNIFF_EXIT_MODE, &bt_req_sniff_exit_mode, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_sniff_subrating_mode(uint8_t *remote_dev_addr,
 *                                                 uint16_t max_latency,
 *                                                 uint16_t min_remote_tout,
 *                                                 uint16_t min_local_tout)
 * @brief      Requests the device entered into the sniff sub rating mode.
 * @pre        \ref rsi_bt_sniff_mode() API needs to be called before this API.
 * @param[in]  remote_dev_addr - remote device address 
 * @param[in]  max_latency - Maximum Latency 
 * @param[in]  min_remote_tout - Minimum remote timeout 
 * @param[in]  min_local_tout - Minimum local timeout 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_sniff_subrating_mode(uint8_t *remote_dev_addr,
                                    uint16_t max_latency,
                                    uint16_t min_remote_tout,
                                    uint16_t min_local_tout)
{
  rsi_bt_req_sniff_subrating_mode_t bt_req_sniff_subrating_mode;
  memset(&bt_req_sniff_subrating_mode, 0, sizeof(bt_req_sniff_subrating_mode));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_sniff_subrating_mode.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_sniff_subrating_mode.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  bt_req_sniff_subrating_mode.max_latency     = max_latency;
  bt_req_sniff_subrating_mode.min_remote_tout = min_remote_tout;
  bt_req_sniff_subrating_mode.min_local_tout  = min_local_tout;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SNIFF_SUBRATING_MODE, &bt_req_sniff_subrating_mode, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_set_ssp_mode(uint8_t pair_mode, uint8_t IOcapability)
 * @brief      Enable or disables Simple Secure Profile (SSP) mode.
 * @pre        \ref rsi_bt_set_connectable() API needs to be called before this API.
 * @param[in]  pair_mode - This parameter is used to enable or disable SSP mode. \n
 *              This parameters supports the following values. \n
 *			   0 - Disable \n
 *			   1 - Enable 
 * @param[in]  IOcapability - This is the IO capability request for SSP mode. \n
 *             This parameter supports the following values. \n
 *			   0 - DisplayOnly \n
 *			   1 - DisplayYesNo \n
 *			   2 - KeyboardOnly \n
 *			   3 - NoInputNoOutput 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_set_ssp_mode(uint8_t pair_mode, uint8_t IOcapability)
{
  rsi_bt_req_set_ssp_mode_t bt_req_set_ssp_mode = { 0 };

  bt_req_set_ssp_mode.pair_mode     = pair_mode;
  bt_req_set_ssp_mode.IO_capability = IOcapability;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_SSP_MODE, &bt_req_set_ssp_mode, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_set_eir_data(uint8_t *data, uint16_t data_len)
 * @brief      Set the extended Inquiry Response data.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  data - Pointer to the EIR data buffer which is an array that can store data upto 200 Bytes 
 * @param[in]  data_len - length of the eir data \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 * @note       Currently EIR data supports upto 200 bytes. \n 
 */
int32_t rsi_bt_set_eir_data(uint8_t *data, uint16_t data_len)
{
  rsi_bt_set_eir_data_t eirdata = { 0 };

  eirdata.fec_required = 0;
  eirdata.data_len     = RSI_MIN((data_len + 1), sizeof(eirdata.eir_data)); // adding 1byte(for fec_required variable)

  memcpy(eirdata.eir_data, data, eirdata.data_len);

  // Send stop advertise command
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_EIR, &eirdata, NULL);
}

/** @} */

/** @addtogroup BT-CLASSIC5
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_bt_per_rx(uint32_t *bt_perrx)
 * @brief      Configure the per receive parameters in the controller and start/stop the PER.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  bt_rx_per - This parameter is the buffer to hold the structure values \n 
 *             This is a structure variable of rsi_bt_rx_per_params_t. \n 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure  
 */
int32_t rsi_bt_per_rx(uint32_t *bt_perrx)
{
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_PER_CMD, bt_perrx, NULL);
}
/*==============================================*/
/**
 * @fn         int32_t rsi_bt_per_tx(uint32_t *bt_pertx)
 * @brief      Configure the per transmit parameters in the controller and start/stop the PER.
 * @pre        Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  bt_tx_per - This parameter is the buffer to hold the structure values. \n 
 *             This is a structure variable of rsi_bt_tx_per_params_t.  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure \n 
 *             
 */
int32_t rsi_bt_per_tx(uint32_t *bt_pertx)
{
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_PER_CMD, bt_pertx, NULL);
}
/** @} */
/** @addtogroup BT-CLASSIC3 
* @{
*/
/*==============================================*/
/**
 * @fn         	int32_t rsi_bt_add_device_id(uint16_t spec_id,
 *                                           uint16_t vendor_id,
 *                                           uint16_t product_id,
 *                                           uint16_t version,
 *                                           int primary_rec,
 *                                           uint16_t vendor_id_source)
 * @brief      	Add device Identification in SDP protocol.
 * @pre        	Call \ref rsi_wireless_init() before calling this API.
 * @param[in]  	spec_id - Version number of the Bluetooth Device ID Profile 
 *                        specification supported by the device. \n 
 * @param[in]  	vendor_id - Uniquely identify the vendor of the device. 
 * @param[in]  	product_id - To distinguish between different products made by the vendor  
 * @param[in]  	version - A numeric expression identifying the device release number in Binary-Coded Decimal 
 * @param[in]  	primary_rec - Set to TRUE in the case single Device ID Service \n 
 *							  Record exists in the device. If multiple Device ID Service Records exist, \n 
 *                            and no primary record has been defined, set to FALSE.  
 * @param[in]  vendor_id_source - This attribute designates which organization assigned the \n
 *                                VendorID attribute, 0x201. 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure \n   
 *             
 * 
 */
int32_t rsi_bt_add_device_id(uint16_t spec_id,
                             uint16_t vendor_id,
                             uint16_t product_id,
                             uint16_t version,
                             int primary_rec,
                             uint16_t vendor_id_source)
{
  rsi_bt_req_add_device_id_t add_device_id = { 0 };

  add_device_id.spec_id          = spec_id;
  add_device_id.vendor_id        = vendor_id;
  add_device_id.product_id       = product_id;
  add_device_id.version          = version;
  add_device_id.primary_rec      = primary_rec;
  add_device_id.vendor_id_source = vendor_id_source;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_ADD_DEVICE_ID, &add_device_id, NULL);
}

/*==============================================*/
/**
 * @fn         	int32_t rsi_bt_change_pkt_type(uint8_t *remote_dev_addr, uint16_t pkt_type)
 * @brief      	Change the packet types in the controller after connection only.
 * @param[in]  	remote_dev_addr - Remote device address
 * @param[in] 	pkt_type - change the packet types in the controller  
 * @return      0			-	Success \n
 *             	Non-Zero Value	-	Failure 
 *              
 */
int32_t rsi_bt_change_pkt_type(uint8_t *remote_dev_addr, uint16_t pkt_type)
{
  rsi_bt_ptt_pkt_type_t bt_pkttypechange = { 0 };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_pkttypechange.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_pkttypechange.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  bt_pkttypechange.pkt_type = pkt_type;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_CHANGE_CONNECTION_PKT_TYPE, &bt_pkttypechange, NULL);
}
/** @} */

/** @addtogroup BT-CLASSIC3
* @{
*/
/*==============================================*/
/**
 * @fn         	int32_t rsi_bt_ptt_req(uint8_t mode)
 * @brief      	Configure the LP HP transitions in the controller.
 * @pre        	Call \ref rsi_wireless_init() before calling this API.
 * @param[in]	mode - BR/EDR mode  
 * @return    	0			-	Success \n
 *            	Non-Zero Value	-	Failure 
 *            
 */
int32_t rsi_bt_ptt_req(uint8_t mode)
{
  rsi_bt_ptt_mode_t bt_lphpchange;
  bt_lphpchange.mode = mode;
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_BR_EDR_LP_HP_TRANSISTION, &bt_lphpchange, NULL);
}

/*=================================================*/
/**
 *   @fn        int32_t rsi_bt_request_authentication(int8_t *remote_dev_addr)
 *   @brief     Start authentication
 *   @param     remote_dev_addr - Remote device address
 *   @return    0  			- Success \n
 *              Non-Zero Value 	- Failure
 */

int32_t rsi_bt_request_authentication(int8_t *remote_dev_addr)
{

  rsi_bt_cmd_conn_auth_t bt_conn_auth = { { 0 } };

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_conn_auth.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_conn_auth.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_ENABLE_AUTH, &bt_conn_auth, NULL);
}

/*=================================================*/
/**
 *   @fn        int32_t rsi_bt_encryption_enable_or_disable(int8_t *remote_dev_addr, uint8_t enable_or_disable)
 *   @brief     Enable or disable the encryption
 *   @param     remote_dev_addr - Remote device address
 *   @param     enable or disable
 *   @return    0  			- Success \n
 *              Non-Zero Value 	- Failure
 *   */

int32_t rsi_bt_encryption_enable_or_disable(int8_t *remote_dev_addr, uint8_t enable_or_disable)
{

  rsi_bt_cmd_conn_enc_t bt_conn_enc = { 0 };

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_conn_enc.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_conn_enc.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif

  bt_conn_enc.enable_or_disable = enable_or_disable;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_ENABLE_ENC, &bt_conn_enc, NULL);
}

/*=================================================*/
/**
 *  @fn        int32_t rsi_bt_delete_linkkeys(int8_t *remote_dev_addr, uint8_t delete_linkkeys)
 *  @brief     Delete the existing linkkeys
 *  @param     remote_dev_addr - Remote device address
 *  @param     delete_linkkeys
 *  @return    0  		- Success \n
 *             Non-Zero Value - Failure
 */

int32_t rsi_bt_delete_linkkeys(int8_t *remote_dev_addr, uint8_t delete_linkkeys)
{

  rsi_bt_cmd_delete_linkkeys_t bt_delete_linkkeys = { 0 };

#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_delete_linkkeys.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_delete_linkkeys.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif

  bt_delete_linkkeys.delete_linkkeys = delete_linkkeys;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_DEL_LINKKEYS, &bt_delete_linkkeys, NULL);
}

#endif
/** @} */
