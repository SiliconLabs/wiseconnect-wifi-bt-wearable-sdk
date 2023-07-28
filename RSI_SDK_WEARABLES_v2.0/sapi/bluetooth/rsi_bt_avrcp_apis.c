/*******************************************************************************
* @file  rsi_bt_avrcp_apis.c
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
/** @addtogroup BT-CLASSIC2
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_init(uint8_t *avrcp_feature)
 * @brief      Initialize avrcp.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 **/
int32_t rsi_bt_avrcp_init(uint8_t *avrcp_feature)
{
  rsi_bt_req_profile_mode_t bt_req_avrcp_init = { 0 };
  bt_req_avrcp_init.profile_mode              = RSI_AVRCP_PROFILE_BIT;
  if (avrcp_feature != NULL) {
    bt_req_avrcp_init.data_len = 1;
    bt_req_avrcp_init.data[0]  = *avrcp_feature;
  }
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_PROFILE_MODE, &bt_req_avrcp_init, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_conn(uint8_t *remote_dev_addr)
 * @brief      Initiate avrcp connection.
 * @param[in]  remote_dev_addr - remote device address
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_conn(uint8_t *remote_dev_addr)
{
  rsi_bt_req_avrcp_conn_t bt_req_avrcp_connect = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_avrcp_connect.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_avrcp_connect.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_CONNECT, &bt_req_avrcp_connect, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_disconn(uint8_t *remote_dev_addr)
 * @brief      Initiate avrcp disconnection.
 * @param[in]  remote_dev_addr - remote device address 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure  
 */
int32_t rsi_bt_avrcp_disconn(uint8_t *remote_dev_addr)
{
  rsi_bt_req_avrcp_disconnect_t bt_req_avrcp_disconnect = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_avrcp_disconnect.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_avrcp_disconnect.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_DISCONNECT, &bt_req_avrcp_disconnect, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_play(uint8_t *remote_dev_addr)
 * @brief      AVRCP play used after AVRCP profile connection.
 * @param[in]  remote_dev_addr - remote device address
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_play(uint8_t *remote_dev_addr)
{
  rsi_bt_req_avrcp_play_t bt_req_avrcp_play = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_avrcp_play.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_avrcp_play.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_PLAY, &bt_req_avrcp_play, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_pause(uint8_t *remote_dev_addr)
 * @brief      AVRCP pause used after AVRCP profile connection.
 * @param[in]  remote_dev_addr - remote device address
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure  
 */
int32_t rsi_bt_avrcp_pause(uint8_t *remote_dev_addr)
{
  rsi_bt_req_avrcp_pause_t bt_req_avrcp_pause = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_avrcp_pause.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_avrcp_pause.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_PAUSE, &bt_req_avrcp_pause, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_stop(uint8_t *remote_dev_addr)
 * @brief      AVRCP stop used after AVRCP profile connection.
 * @param[in]  remote_dev_addr - remote device address
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_stop(uint8_t *remote_dev_addr)
{
  rsi_bt_req_avrcp_stop_t bt_req_avrcp_stop = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_avrcp_stop.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_avrcp_stop.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_STOP, &bt_req_avrcp_stop, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_next(uint8_t *remote_dev_addr)
 * @brief      AVRCP next used after AVRCP profile connection.
 * @param[in]  remote_dev_addr - remote device address
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_next(uint8_t *remote_dev_addr)
{
  rsi_bt_req_avrcp_next_t bt_req_avrcp_next = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_avrcp_next.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_avrcp_next.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_NEXT, &bt_req_avrcp_next, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_previous(uint8_t *remote_dev_addr)
 * @brief      AVRCP previous used after AVRCP profile connection.
 * @param[in]  remote_dev_addr - remote device address
 * @return     0	 	-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_previous(uint8_t *remote_dev_addr)
{
  rsi_bt_req_avrcp_previous_t bt_req_avrcp_previous = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_avrcp_previous.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_avrcp_previous.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_PREVIOUS, &bt_req_avrcp_previous, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_vol_up(uint8_t *remote_dev_addr)
 * @brief      AVRCP vol-up used after AVRCP profile connection.
 * @param[in]  remote_dev_addr - remote device address  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure  
 */
int32_t rsi_bt_avrcp_vol_up(uint8_t *remote_dev_addr)
{
  rsi_bt_req_avrcp_vol_up_t bt_req_avrcp_vol_up = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_avrcp_vol_up.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_avrcp_vol_up.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_VOL_UP, &bt_req_avrcp_vol_up, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_vol_down(uint8_t *remote_dev_addr)
 * @brief      AVRCP vol-down used after AVRCP profile connection.
 * @param[in]  remote_dev_addr - remote device address  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_vol_down(uint8_t *remote_dev_addr)
{
  rsi_bt_req_avrcp_vol_down_t bt_req_avrcp_vol_down = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_avrcp_vol_down.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_avrcp_vol_down.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_VOL_DOWN, &bt_req_avrcp_vol_down, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_get_capabilities(uint8_t *remote_dev_addr,
 *                                                   uint8_t capability_type,
 *                                                   rsi_bt_rsp_avrcp_get_capabilities_t *cap_list)
 * @brief      AVRCP get capabilities used after AVRCP profile connection.
 * @param[in]  remote_dev_addr - remote device address  
 * @param[in]  capability_type - capability type  
 * @param[in]  cap_list - capability list 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_get_capabilities(uint8_t *remote_dev_addr,
                                      uint8_t capability_type,
                                      rsi_bt_rsp_avrcp_get_capabilities_t *cap_list)
{
  rsi_bt_req_avrcp_get_capabilities_t bt_req_avrcp_cap;
  memset(&bt_req_avrcp_cap, 0, sizeof(bt_req_avrcp_cap));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_avrcp_cap.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_avrcp_cap.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  bt_req_avrcp_cap.type = capability_type;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_CAPABILITES, &bt_req_avrcp_cap, cap_list);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_get_att_list(uint8_t *remote_dev_addr, rsi_bt_rsp_avrcp_get_atts_list_t *att_list)
 * @brief      Get the media player support attribute list from remote device.
 * @param[in]  remote_dev_addr - remote device address 
 * @param[in]  att_list - attribute list
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_get_att_list(uint8_t *remote_dev_addr, rsi_bt_rsp_avrcp_get_atts_list_t *att_list)
{
  rsi_bt_req_avrcp_get_att_list_t bt_req_avrcp_att = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_avrcp_att.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(bt_req_avrcp_att.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_ATTS_LIST, &bt_req_avrcp_att, att_list);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_get_att_vals_list(uint8_t *remote_dev_addr,
 *                                                    uint8_t att_id,
 *                                                    rsi_bt_rsp_avrcp_get_att_vals_list_t *att_vals_list)
 * @brief      Get the media player support attribute values list from remote device.
 * @param[in]  remote_dev_addr - remote device address   
 * @param[in]  att_id - attribute ID 
 * @param[in]  att_vals_list - attribute value list  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_get_att_vals_list(uint8_t *remote_dev_addr,
                                       uint8_t att_id,
                                       rsi_bt_rsp_avrcp_get_att_vals_list_t *att_vals_list)
{
  rsi_bt_req_avrcp_get_att_vals_list_t avrcp_att_vals;
  memset(&avrcp_att_vals, 0, sizeof(avrcp_att_vals));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(avrcp_att_vals.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(avrcp_att_vals.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  avrcp_att_vals.att_id = att_id;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_ATT_VALS_LIST, &avrcp_att_vals, att_vals_list);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_get_cur_att_val(uint8_t *remote_dev_addr,
                                     uint8_t *att_list,
                                     uint8_t nbr_atts,
                                     rsi_bt_rsp_avrcp_get_cur_att_val_t *att_vals_list)
 * @brief      Get the media player attribute value from remote device.
 * @param[in]  remote_dev_addr - remote device address  
 * @param[in]  att_list - attribute ID 
 * @param[in]  nbr_atts - number of attributes 
 * @param[in]  att_vals_list - attribute value list 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_get_cur_att_val(uint8_t *remote_dev_addr,
                                     uint8_t *att_list,
                                     uint8_t nbr_atts,
                                     rsi_bt_rsp_avrcp_get_cur_att_val_t *att_vals_list)
{
  rsi_bt_req_avrcp_get_cur_att_val_t avrcp_atts;
  memset(&avrcp_atts, 0, sizeof(avrcp_atts));
  uint8_t ix;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(avrcp_atts.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(avrcp_atts.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  avrcp_atts.nbr_atts = nbr_atts;
  for (ix = 0; (ix < nbr_atts) && (ix < RSI_MAX_ATT); ix++) {
    avrcp_atts.att_list[ix] = att_list[ix];
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_CUR_ATT_VAL, &avrcp_atts, att_vals_list);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_set_cur_att_val(uint8_t *remote_dev_addr, 
 *                                                  att_val_t *val_list, uint8_t nbr_atts)
 * @brief      Set the media player attribute value at remote device.
 * @param[in]  remote_dev_addr - remote device address 
 * @param[in]  val_list - value list 
 * @param[in]  nbr_atts - number of attributes  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_set_cur_att_val(uint8_t *remote_dev_addr, att_val_t *val_list, uint8_t nbr_atts)
{
  rsi_bt_req_avrcp_set_cur_att_val_t att_val_list;
  memset(&att_val_list, 0, sizeof(att_val_list));
  uint8_t ix;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(att_val_list.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(att_val_list.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  att_val_list.nbr_atts = nbr_atts;
  for (ix = 0; (ix < nbr_atts) && (ix < RSI_MAX_ATT); ix++) {
    att_val_list.att_val_list[ix].att_id = val_list[ix].att_id;
    att_val_list.att_val_list[ix].att_id = val_list[ix].att_val;
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_SET_CUR_ATT_VAL, &att_val_list, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_get_element_att(uint8_t *remote_dev_addr,
 *                                                  uint8_t *att_ids,
 *                                                  uint8_t nbr_atts,
 *                                                  rsi_bt_rsp_avrcp_get_ele_att_t *att_vals)
 * @brief      Get the media player element attribute from remote device.
 * @param[in]  remote_dev_addr - Address of remote device    
 * @param[in]  att_vals - This is the attribute value entered in this structure
 * @param[in]  att_ids - attributes ids
 * @param[in]  nbr_atts - number of attributes  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_get_element_att(uint8_t *remote_dev_addr,
                                     uint8_t *att_ids,
                                     uint8_t nbr_atts,
                                     rsi_bt_rsp_avrcp_get_ele_att_t *att_vals)
{
  rsi_bt_req_avrcp_get_ele_att_t att_list;
  memset(&att_list, 0, sizeof(att_list));
  uint8_t ix;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(att_list.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(att_list.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  att_list.nbr_atts = nbr_atts;
  for (ix = 0; (ix < nbr_atts) && (ix < RSI_MAX_ATT); ix++) {
    att_list.att_list[ix] = att_ids[ix];
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_ELEMENT_ATT, &att_list, att_vals);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_get_play_status(uint8_t *remote_dev_addr, rsi_bt_rsp_avrcp_get_player_status_t *play_status)
 * @brief      Get the media player status from remote device.
 * @param[in]  remote_dev_addr - remote device address 
 * @param[out] play_status - to capture the player status. 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure  
 */
int32_t rsi_bt_avrcp_get_play_status(uint8_t *remote_dev_addr, rsi_bt_rsp_avrcp_get_player_status_t *play_status)
{
  rsi_bt_req_avrcp_get_player_status_t play_status_req = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(play_status_req.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(play_status_req.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_PLAY_STATUS, &play_status_req, play_status);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_reg_notification(uint8_t *remote_dev_addr, uint8_t event_id, uint8_t *p_resp_val)
 * @brief      Register the media player notification events at remote device.
 * @param[in]  remote_dev_addr - remote device address.
 * @param[in]  event_id - event ID.
 * @param[out] p_resp_val - used to capture response from this API.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_reg_notification(uint8_t *remote_dev_addr, uint8_t event_id, uint8_t *p_resp_val)
{
#ifdef BD_ADDR_IN_ASCII
  //This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(p_resp_val);
#endif
  rsi_bt_req_avrcp_reg_notification_t reg_notify_req;
  memset(&reg_notify_req, 0, sizeof(reg_notify_req));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(reg_notify_req.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(reg_notify_req.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  reg_notify_req.event_id = event_id;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_REG_NOTIFICATION, &reg_notify_req, p_resp_val);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_get_remote_version(uint8_t *remote_dev_addr,
 *                                                     rsi_bt_rsp_avrcp_remote_version_t *version)
 * @brief      Get the AVRCP profile version from remote device.
 * @param[in]  remote_dev_addr - remote device address. 
 * @param[in]  version - version info. 
 * @return     0	 	-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_get_remote_version(uint8_t *remote_dev_addr, rsi_bt_rsp_avrcp_remote_version_t *version)
{
  rsi_bt_req_avrcp_remote_version_t profile_version = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(profile_version.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(profile_version.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_REMOTE_VERSION, &profile_version, version);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_get_att_text(uint8_t *remote_dev_addr,
 *                                 uint8_t nbr_atts,
 *                                 uint8_t *p_atts,
 *                                 player_att_text_t *p_att_text_resp)
 * @brief      Get the AVRCP profile player attribute text from remote device.
 * @param[in]  remote_dev_addr - remote device address.  
 * @param[in]  nbr_atts - number of attributes.
 * @param[in]  p_atts - pointer to attributes. 
 * @param[out] p_att_text_resp - to capture response from this API.  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_get_att_text(uint8_t *remote_dev_addr,
                                  uint8_t nbr_atts,
                                  uint8_t *p_atts,
                                  player_att_text_t *p_att_text_resp)
{
  rsi_bt_req_avrcp_get_cur_att_val_t att_text;
  memset(&att_text, 0, sizeof(att_text));
  uint8_t ix;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(att_text.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(att_text.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  att_text.nbr_atts = nbr_atts;
  for (ix = 0; (ix < nbr_atts) && (ix < RSI_MAX_ATT); ix++) {
    att_text.att_list[ix] = p_atts[ix];
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_ATT_TEXT, &att_text, p_att_text_resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_get_att_val_text(uint8_t *remote_dev_addr,
                                      uint8_t att_id,
                                      uint8_t nbr_vals,
                                      uint8_t *p_vals,
                                      player_att_text_t *p_att_text_resp)
 * @brief      Get the AVRCP profile player attribute 
 * values text from remote device.
 * @param[in]  remote_dev_addr - remote device address.  
 * @param[in]  att_id - attribute ID. 
 * @param[in]  nbr_vals - number of attribute values.
 * @param[in]  p_vals - pointer to attribute values. 
 * @param[out] p_att_text_resp - to capture response from this API. 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_get_att_val_text(uint8_t *remote_dev_addr,
                                      uint8_t att_id,
                                      uint8_t nbr_vals,
                                      uint8_t *p_vals,
                                      player_att_text_t *p_att_text_resp)
{
  rsi_bt_req_avrcp_get_att_val_text_t att_val_text;
  memset(&att_val_text, 0, sizeof(att_val_text));
  uint8_t ix;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(att_val_text.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(att_val_text.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  att_val_text.att_id   = att_id;
  att_val_text.nbr_vals = nbr_vals;
  for (ix = 0; (ix < nbr_vals) && (ix < RSI_MAX_ATT); ix++) {
    att_val_text.vals[ix] = p_vals[ix];
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_ATT_VALS_TEXT, &att_val_text, p_att_text_resp);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_batt_status(uint8_t *remote_dev_addr, uint8_t batt_level)
 * @brief      Send the device battery status to remote device.
 * @param[in]  remote_dev_addr - remote device address.  
 * @param[in]  batt_level - to update battery level. 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_batt_status(uint8_t *remote_dev_addr, uint8_t batt_level)
{
  rsi_bt_req_avrcp_batt_status_t batt_status;
  memset(&batt_status, 0, sizeof(batt_status));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(batt_status.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(batt_status.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  batt_status.batt_status = batt_level;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_BATTERY_STATUS, &batt_status, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_supp_char_sets(uint8_t *remote_dev_addr, uint8_t nbr_sets, uint16_t *p_sets)
 * @brief      Send the device battery character 
 * @todo       to be updated
 * @param[in]  remote_dev_addr - remote device address.   
 * @param[in]  nbr_sets - number of sets. 
 * @param[in]  p_sets - pointer to sets.  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_supp_char_sets(uint8_t *remote_dev_addr, uint8_t nbr_sets, uint16_t *p_sets)
{
  rsi_bt_req_avrcp_char_sets_t char_sets;
  memset(&char_sets, 0, sizeof(char_sets));
  uint8_t ix;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(char_sets.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(char_sets.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  char_sets.char_sets.nbr_sets = nbr_sets;
  for (ix = 0; (ix < nbr_sets) && (ix < MAX_SUPP_VALS); ix++) {
    char_sets.char_sets.supp_vals[ix] = p_sets[ix];
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_CHAR_SETS, &char_sets, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_set_abs_vol(uint8_t *remote_dev_addr, uint8_t abs_vol, uint8_t *p_resp_abs_vol)
 * @brief      Send absolute volume response to remote device.
 * @param[in]  remote_dev_addr - remote device address.  
 * @param[in]  abs_vol - absolute volume. 
 * @param[out] p_resp_abs_vol - to capture the response from this API. TBD  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure  
 */
int32_t rsi_bt_avrcp_set_abs_vol(uint8_t *remote_dev_addr, uint8_t abs_vol, uint8_t *p_resp_abs_vol)
{
  rsi_bt_avrcp_set_abs_vol_t abs_vol_req;
  memset(&abs_vol_req, 0, sizeof(abs_vol_req));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(abs_vol_req.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(abs_vol_req.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  abs_vol_req.abs_vol = abs_vol;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_SET_ABS_VOL, &abs_vol_req, p_resp_abs_vol);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_cap_resp(uint8_t *remote_dev_addr, uint8_t cap_type, uint8_t nbr_caps, uint32_t *p_caps)
 * @brief      Send the device capcabilites to remote device.
 * @param[in]  remote_dev_addr - remote device address. 
 * @param[in]  cap_type - capability type. 
 * @param[in]  nbr_caps - number of capabilities   
 * @param[in/out] p_caps - TBD  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_cap_resp(uint8_t *remote_dev_addr, uint8_t cap_type, uint8_t nbr_caps, uint32_t *p_caps)
{
  rsi_bt_avrcp_cap_resp_t cap_resp;
  memset(&cap_resp, 0, sizeof(cap_resp));
  uint8_t ix;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(cap_resp.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(cap_resp.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  cap_resp.cap_type = cap_type;
  cap_resp.nbr_caps = nbr_caps;
  for (ix = 0; (ix < nbr_caps) && (ix < MAX_CAPS); ix++) {
    cap_resp.caps[ix] = p_caps[ix];
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_CAPABILITES_RESP, &cap_resp, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_att_list_resp(uint8_t *remote_dev_addr, 
 *                                                uint8_t nbr_atts, uint8_t *p_atts)
 * @brief      Send the support attributes to remote device.
 * @param[in]  remote_dev_addr - remote device address. 
 * @param[in]  nbr_atts - number of attributes. 
 * @param[in]  p_atts - pointer to attributes list. TBD  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_att_list_resp(uint8_t *remote_dev_addr, uint8_t nbr_atts, uint8_t *p_atts)
{
  rsi_bt_avrcp_att_list_resp_t att_resp;
  memset(&att_resp, 0, sizeof(att_resp));
  uint8_t ix;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(att_resp.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(att_resp.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  att_resp.nbr_atts = nbr_atts;
  for (ix = 0; (ix < nbr_atts) && (ix < MAX_CAPS); ix++) {
    att_resp.atts[ix] = p_atts[ix];
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_ATTS_LIST_RESP, &att_resp, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_att_val_list_resp(uint8_t *remote_dev_addr, 
 *                                                    uint8_t nbr_vals, uint8_t *p_vals)
 * @brief      Get the attributes value list response from the remote device.
 * @param[in]  remote_dev_addr - remote device address.  
 * @param[in]  nbr_vals - number of values. 
 * @param[in]  p_vals - pointer to values list.  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure  
 */
int32_t rsi_bt_avrcp_att_val_list_resp(uint8_t *remote_dev_addr, uint8_t nbr_vals, uint8_t *p_vals)
{
  rsi_bt_avrcp_att_vals_list_resp_t att_vals_resp;
  memset(&att_vals_resp, 0, sizeof(att_vals_resp));
  uint8_t ix;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(att_vals_resp.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(att_vals_resp.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  att_vals_resp.nbr_vals = nbr_vals;
  for (ix = 0; (ix < nbr_vals) && (ix < MAX_CAPS); ix++) {
    att_vals_resp.vals[ix] = p_vals[ix];
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_ATT_VALS_LIST_RESP, &att_vals_resp, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_cur_att_val_resp(uint8_t *remote_dev_addr,
 *                                                   uint8_t nbr_atts, att_val_t *p_att_vals)
 * @brief      Get the current attributes value response from the remote device.
 * @param[in]  remote_dev_addr - remote device address.  
 * @param[in]  nbr_atts - number of attributes. 
 * @param[in]  p_att_vals - pointer to attributes values list. TBD 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_cur_att_val_resp(uint8_t *remote_dev_addr, uint8_t nbr_atts, att_val_t *p_att_vals)
{
  rsi_bt_avrcp_cur_att_vals_resp_t att_vals_resp;
  memset(&att_vals_resp, 0, sizeof(att_vals_resp));
  uint8_t ix;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(att_vals_resp.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(att_vals_resp.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  att_vals_resp.att_list.nbr_atts = nbr_atts;
  for (ix = 0; (ix < nbr_atts) && (ix < MAX_ATT_VALS); ix++) {
    att_vals_resp.att_list.att_vals[ix].att_id  = p_att_vals[ix].att_id;
    att_vals_resp.att_list.att_vals[ix].att_val = p_att_vals[ix].att_val;
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_CUR_ATT_VAL_RESP, &att_vals_resp, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_set_cur_att_val_resp(uint8_t *remote_dev_addr, uint8_t status)
 * @brief      Set the current attributes value to remote device.
 * @param[in]  remote_dev_addr - remote device address.  
 * @param[in]  status - status to be sent. 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_set_cur_att_val_resp(uint8_t *remote_dev_addr, uint8_t status)
{
  rsi_bt_avrcp_set_att_vals_resp_t set_att_vals_resp;
  memset(&set_att_vals_resp, 0, sizeof(set_att_vals_resp));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(set_att_vals_resp.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(set_att_vals_resp.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  set_att_vals_resp.status = status;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_SET_CUR_ATT_VAL_RESP, &set_att_vals_resp, NULL);
}

#define BT_AVRCP_UTF_8_CHAR_SET 0x006A

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_ele_att_resp(uint8_t *remote_dev_addr,
 *                                               uint8_t num_attrs, attr_list_t *p_attr_list)
 * @brief      Send the song attributes to remote device.
 * @param[in]  remote_dev_addr - remote device address. 
 * @param[in]  num_attrs - number of attributes. 
 * @param[in]  p_attr_list - pointer to attributes list. 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_ele_att_resp(uint8_t *remote_dev_addr, uint8_t num_attrs, attr_list_t *p_attr_list)
{
  uint8_t ix;
  rsi_bt_avrcp_elem_attr_resp_t elem_attr;
  memset(&elem_attr, 0, sizeof(elem_attr));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(elem_attr.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(elem_attr.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  elem_attr.elem_attr_list.num_attrs = num_attrs;
  for (ix = 0; (ix < num_attrs) && (ix < MAX_ATT_LIST); ix++) {
    elem_attr.elem_attr_list.attr_list[ix].id          = p_attr_list[ix].id;
    elem_attr.elem_attr_list.attr_list[ix].char_set_id = BT_AVRCP_UTF_8_CHAR_SET;
    elem_attr.elem_attr_list.attr_list[ix].attr_len    = p_attr_list[ix].attr_len;
    memcpy(elem_attr.elem_attr_list.attr_list[ix].attr_val, p_attr_list[ix].attr_val, p_attr_list[ix].attr_len);
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_ELEMENT_ATT_RESP, &elem_attr, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_play_status_resp(uint8_t *remote_dev_addr,
 *                                                   uint8_t play_status,
 *                                                   uint32_t song_len,
 *                                                   uint32_t song_pos)
 * @brief      Send the player status to remote device.
 * @param[in]  remote_dev_addr - remote device address. 
 * @param[in]  play_status - player status. 
 * @param[in]  song_len - song length. 
 * @param[in]  song_pos - song position. 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_play_status_resp(uint8_t *remote_dev_addr,
                                      uint8_t play_status,
                                      uint32_t song_len,
                                      uint32_t song_pos)
{
  rsi_bt_avrcp_play_status_resp_t player_status;
  memset(&player_status, 0, sizeof(player_status));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(player_status.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(player_status.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  player_status.play_status = play_status;
  player_status.song_len    = song_len;
  player_status.song_pos    = song_pos;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_PLAY_STATUS_RESP, &player_status, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_reg_notify_resp(uint8_t *remote_dev_addr,
 *                                                  uint8_t event_id,
 *                                                  uint8_t event_data_len,
 *                                                  uint8_t *event_data)
 * @brief      Register notify response to remote device.
 * @param[in]  remote_dev_addr - remote device address.  
 * @param[in]  event_id - event ID. 
 * @param[in]  event_data_len - length of event_data buffer. 
 * @param[in]  event_data - event data. 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_reg_notify_resp(uint8_t *remote_dev_addr,
                                     uint8_t event_id,
                                     uint8_t event_data_len,
                                     uint8_t *event_data)
{
  rsi_bt_avrcp_reg_notify_interim_resp_t reg_event;
  memset(&reg_event, 0, sizeof(reg_event));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(reg_event.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(reg_event.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  reg_event.event_id = event_id;

  if (event_id == 0x02 /* EVENT_TRACK_CHANGED */)
    memcpy(reg_event.reg_notify_val.curr_track_idx, event_data, event_data_len);
  else if (event_id == 0x0d /* EVENT_VOL_CHANGED */)
    memcpy(&reg_event.reg_notify_val.abs_vol, event_data, event_data_len);
  else if (event_id == 0x01 /* AVRCP_EVENT_PLAYBACK_STATUS_CHANGED */)
    memcpy(&reg_event.reg_notify_val.play_status, event_data, event_data_len);
  else if (event_id == 0x0b) {
    reg_event.reg_notify_val.playerid = event_data[0] << 8;
    reg_event.reg_notify_val.playerid += event_data[1];
    reg_event.reg_notify_val.uidcounter = event_data[2] << 8;
    reg_event.reg_notify_val.uidcounter += event_data[3];
  } else {
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_REG_NOTIFICATION_RESP, &reg_event, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_att_text_resp(uint8_t *remote_dev_addr,
 *                                                uint8_t nbr_atts, att_text_t *p_att_text)
 * @brief      Send supporting attribute text response to remote device.
 * @param[in]  remote_dev_addr - remote device address.   
 * @param[in]  nbr_atts - number of attributes. 
 * @param[out] p_att_text - pointer to attribute text to hold response of thei API.TBD 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_att_text_resp(uint8_t *remote_dev_addr, uint8_t nbr_atts, att_text_t *p_att_text)
{
  rsi_bt_avrcp_att_text_resp_t att_text;
  memset(&att_text, 0, sizeof(att_text));
  uint8_t ix;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(att_text.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(att_text.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  att_text.atts_text.nbr_atts = nbr_atts;
  for (ix = 0; (ix < nbr_atts) && (ix < MAX_TEXT_LIST); ix++) {
    att_text.atts_text.list[ix].id = p_att_text[ix].id;
    memcpy(att_text.atts_text.list[ix].att_text, p_att_text[ix].att_text, strlen((char *)p_att_text[ix].att_text));
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_ATT_TEXT_RESP, &att_text, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_att_vals_text_resp(uint8_t *remote_dev_addr,
 *                                                     uint8_t nbr_vals, att_text_t *p_vals_text)
 * @brief      Send supporting attribute values text response to remote device.
 * @param[in]  remote_dev_addr - remote device address.  
 * @param[in]  nbr_vals - number of values. 
 * @param[in]  p_vals_text - pointer to values list. TBD  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_att_vals_text_resp(uint8_t *remote_dev_addr, uint8_t nbr_vals, att_text_t *p_vals_text)
{
  rsi_bt_avrcp_att_text_resp_t vals_text;
  memset(&vals_text, 0, sizeof(vals_text));
  uint8_t ix;
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(vals_text.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(vals_text.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  vals_text.atts_text.nbr_atts = nbr_vals;
  for (ix = 0; (ix < nbr_vals) && (ix < MAX_TEXT_LIST); ix++) {
    vals_text.atts_text.list[ix].id = p_vals_text[ix].id;
    memcpy(vals_text.atts_text.list[ix].att_text, p_vals_text[ix].att_text, strlen((char *)p_vals_text[ix].att_text));
  }

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_ATT_VALS_TEXT_RESP, &vals_text, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_batt_status_resp(uint8_t *remote_dev_addr, uint8_t status)
 * @brief      Send battery status response to remote device.
 * @param[in]  remote_dev_addr - remote device address.
 * @param[in]  status - status to be sent.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_batt_status_resp(uint8_t *remote_dev_addr, uint8_t status)
{
  rsi_bt_avrcp_reg_notify_resp_t batt_status;
  memset(&batt_status, 0, sizeof(batt_status));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(batt_status.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(batt_status.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  batt_status.status = status;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_BATTERY_STATUS_RESP, &batt_status, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_char_set_status_resp(uint8_t *remote_dev_addr, uint8_t status)
 * @brief      Send character set repsonse to remote device.
 * @param[in]  remote_dev_addr - remote device address.
 * @param[in]  status - status to be sent.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure  
 *             
 */
int32_t rsi_bt_avrcp_char_set_status_resp(uint8_t *remote_dev_addr, uint8_t status)
{
  rsi_bt_avrcp_reg_notify_resp_t char_set;
  memset(&char_set, 0, sizeof(char_set));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(char_set.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(char_set.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  char_set.status = status;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_CHAR_SETS_RESP, &char_set, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_notify(uint8_t *remote_dev_addr, 
 *                                         uint8_t event_id, notify_val_t *p_notify_val)
 * @brief      Send player notification to remote device.
 * @param[in]  remote_dev_addr - remote device address.  
 * @param[in]  event_id - event ID.
 * @param[in]  p_notify_val - pointer to notofication values.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_notify(uint8_t *remote_dev_addr, uint8_t event_id, notify_val_t *p_notify_val)
{
  rsi_bt_avrcp_notify_t notify;
  memset(&notify, 0, sizeof(notify));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(notify.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(notify.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  notify.event_id = event_id;
  memcpy(&notify.notify_val, p_notify_val, sizeof(notify_val_t));

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_NOTIFICATION, &notify, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_cmd_reject(uint8_t *remote_dev_addr, uint8_t pdu_id, uint8_t status)
 * @brief      Reject the received request from remote device.
 * @param[in]  remote_dev_addr - remote device address.  
 * @param[in]  pdu_id - PDU ID. 
 * @param[in]  status - status to be sent.  
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_cmd_reject(uint8_t *remote_dev_addr, uint8_t pdu_id, uint8_t status)
{
  rsi_bt_avrcp_cmd_reject_t cmd_reject;
  memset(&cmd_reject, 0, sizeof(cmd_reject));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(cmd_reject.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(cmd_reject.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  cmd_reject.pdu_id = pdu_id;
  cmd_reject.status = status;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_CMD_REJECT, &cmd_reject, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_set_abs_vol_resp(uint8_t *remote_dev_addr, uint8_t abs_vol)
 * @brief      Send absolute volume response to remote device.
 * @param[in]  remote_dev_addr - remote device address. 
 * @param[in]  abs_vol - absolute volume. 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_set_abs_vol_resp(uint8_t *remote_dev_addr, uint8_t abs_vol)
{
  rsi_bt_avrcp_set_abs_vol_resp_t abs_vol_resp;
  memset(&abs_vol_resp, 0, sizeof(abs_vol_resp));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(abs_vol_resp.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(abs_vol_resp.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  abs_vol_resp.abs_vol = abs_vol;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_SET_ABS_VOL_RESP, &abs_vol_resp, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_set_addr_player_resp(uint8_t *remote_dev_addr, uint8_t status)
 * @brief      Send address player response to remote device.
 * @param[in]  remote_dev_addr - Remote device Address 
 * @param[in]  status - Status 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_set_addr_player_resp(uint8_t *remote_dev_addr, uint8_t status)
{
  rsi_bt_avrcp_set_addr_player_resp_t set_addr_player_resp = { 0 };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(set_addr_player_resp.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(set_addr_player_resp.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  set_addr_player_resp.status = status;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_SET_ADDR_PLAYER_RESP, &set_addr_player_resp, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_get_folder_items_resp(uint8_t *remote_dev_addr,
 *                                                        uint8_t status,
 *                                                        folder_items_resp_t folder_items_resp)
 * @brief      Send folder items response to remote device.
 * @param[in]  remote_dev_addr - Remote device Address 
 * @param[in]  status - Status 
 * @param[in]  folder_items_resp - Folder items 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_get_folder_items_resp(uint8_t *remote_dev_addr,
                                           uint8_t status,
                                           folder_items_resp_t folder_items_resp)
{
  rsi_bt_avrcp_get_folder_items_resp_t get_folder_items_resp = { 0 };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(get_folder_items_resp.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(get_folder_items_resp.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  get_folder_items_resp.status = status;
  memcpy(&get_folder_items_resp.fldr_items, &folder_items_resp, sizeof(folder_items_resp_t));
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_FOLDER_ITEMS_RESP, &get_folder_items_resp, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_avrcp_get_tot_num_items_resp(uint8_t *remote_dev_addr,
 *                                                         uint8_t status,
 *                                                         uint16_t uidcntr,
 *                                                         uint32_t numofitems)
 * @brief      Send total num items response to remote device.
 * @param[in]  remote_dev_addr - Remote device Address 
 * @param[in]  status - Status 
 * @param[in]  uidcntr - uid counter 
 * @param[in]  numofitems - Number of items 
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure 
 */
int32_t rsi_bt_avrcp_get_tot_num_items_resp(uint8_t *remote_dev_addr,
                                            uint8_t status,
                                            uint16_t uidcntr,
                                            uint32_t numofitems)
{
  rsi_bt_avrcp_get_tot_num_items_resp_t get_tot_num_items_resp = { 0 };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(get_tot_num_items_resp.dev_addr, (int8_t *)remote_dev_addr);
#else
  memcpy(get_tot_num_items_resp.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  get_tot_num_items_resp.status     = status;
  get_tot_num_items_resp.uidcntr    = uidcntr;
  get_tot_num_items_resp.numofitems = numofitems;

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_AVRCP_GET_TOT_NUM_ITEMS_RESP, &get_tot_num_items_resp, NULL);
}

#endif
/** @} */
