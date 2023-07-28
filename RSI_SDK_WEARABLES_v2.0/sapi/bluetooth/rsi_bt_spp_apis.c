/*******************************************************************************
* @file  rsi_bt_spp_apis.c
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

/** @addtogroup BT-CLASSIC4
* @{
*/
/*==============================================*/
/**
 * @fn		   int32_t rsi_bt_spp_init(void)
 * @brief	   Set the SPP profile mode.
 * @pre		   rsi_wireless_init() API needs to be called before this API. 
 * @param[in]  void
 * @return	   0		-	Success \n
 *			   Non-Zero Value	-	Failure 
 *             
 */

int32_t rsi_bt_spp_init(void)
{
  rsi_bt_req_profile_mode_t bt_req_spp_init = { 0 };
  bt_req_spp_init.profile_mode              = RSI_SPP_PROFILE_BIT;
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_PROFILE_MODE, &bt_req_spp_init, NULL);
}
/*==============================================*/
/**
 * @fn		   int32_t rsi_bt_spp_connect(uint8_t *remote_dev_addr)
 * @brief      Initiate the SPP profile level connection
 * @pre		   \ref rsi_bt_spp_init() API needs to be called before this API
 * @param[in]  remote_dev_addr - Remote device address 
 * @return	   0		- 	Success \n
 *			   Non-Zero Value	-	Failure           
 *
 */

int32_t rsi_bt_spp_connect(uint8_t *remote_dev_addr)
{
  rsi_bt_req_connect_t bt_req_spp_connect = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_spp_connect.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_spp_connect.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SPP_CONNECT, &bt_req_spp_connect, NULL);
}

/*==============================================*/
/**
 * @fn		   int32_t rsi_bt_spp_disconnect(uint8_t *remote_dev_addr)
 * @brief	   Initiate the SPP service level disconnection.
 * @pre		   \ref rsi_bt_spp_connect() API need to be called before this API 
 * @param[in]  remote_dev_addr - This is the remote device address   
 * @return	   0		-	Success \n
 *			   Non-Zero Value	-	Failure 
 *             
 */

int32_t rsi_bt_spp_disconnect(uint8_t *remote_dev_addr)
{
  //This statement is added only to resolve compilation warning, value is unchanged
  USED_PARAMETER(remote_dev_addr);
  rsi_bt_req_disconnect_t bt_req_spp_disconnect = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_spp_disconnect.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_spp_disconnect.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SPP_DISCONNECT, &bt_req_spp_disconnect, NULL);
}

/*==============================================*/
/**
 * @fn		   int32_t rsi_bt_spp_transfer(uint8_t *remote_dev_addr, uint8_t *data, uint16_t length)
 * @brief	   Transfer the data through SPP profile.
 * @pre		   \ref rsi_bt_spp_connect() API needs to be called before this API. 
 * @param[in]  remote_dev_addr - This is the remote device address  
 * @param[in]  data - This is the data for transmission 
 * @param[in]  length - This is the data length for transfer, Max length supported upto 1000 bytes   
 * @return	   0		-	Success  \n
 *			   Non Zero Value	-	Failure 
 *             
 *
 */

int32_t rsi_bt_spp_transfer(uint8_t *remote_dev_addr, uint8_t *data, uint16_t length)
{
  //This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(remote_dev_addr);
  uint16_t xfer_len = 0;

  rsi_bt_req_spp_transfer_t bt_req_spp_transfer = { 0 };
  xfer_len                                      = RSI_MIN(length, RSI_BT_MAX_PAYLOAD_SIZE);
  bt_req_spp_transfer.data_length               = xfer_len;

  memcpy(bt_req_spp_transfer.data, data, xfer_len);

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SPP_TRANSFER, &bt_req_spp_transfer, NULL);
}
#endif

/** @} */
