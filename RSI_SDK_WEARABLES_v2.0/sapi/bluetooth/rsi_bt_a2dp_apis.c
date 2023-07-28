/*******************************************************************************
* @file  rsi_bt_a2dp_apis.c
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
#include "rsi_bt_sbc_codec.h"

/** @addtogroup BT-CLASSIC1
* @{
*/
/*==============================================*/
/**
 * @fn         int32_t rsi_bt_a2dp_init(rsi_bt_a2dp_sbc_codec_cap_t *sbc_cap)
 * @brief      Initialize A2DP.
 * @param[in]  sbc_cap -
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 *
 */
int32_t rsi_bt_a2dp_init(rsi_bt_a2dp_sbc_codec_cap_t *sbc_cap)
{
  rsi_bt_req_profile_mode_t bt_req_a2dp_init = { 0 };
  bt_req_a2dp_init.profile_mode              = RSI_A2DP_PROFILE_BIT;
  if (sbc_cap != NULL) {
    bt_req_a2dp_init.data_len = sizeof(rsi_bt_a2dp_sbc_codec_cap_t);
    memcpy(&bt_req_a2dp_init.data, sbc_cap, sizeof(rsi_bt_a2dp_sbc_codec_cap_t));
  }
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_SET_PROFILE_MODE, &bt_req_a2dp_init, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_a2dp_connect(uint8_t *remote_dev_addr)
 * @brief      Initiate the A2DP connection.
 * @param[in]  remote_dev_addr - remote device address
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 *
 */
int32_t rsi_bt_a2dp_connect(uint8_t *remote_dev_addr)
{
  rsi_bt_req_a2dp_connect_t bt_req_a2dp_connect = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_a2dp_connect.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_a2dp_connect.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  return rsi_bt_driver_send_cmd(RSI_BT_REQ_A2DP_CONNECT, &bt_req_a2dp_connect, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_a2dp_disconnect(uint8_t *remote_dev_addr)
 * @brief      Use to disconnect A2DP.
 * @param[in]  remote_dev_addr - remote device address
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 */
int32_t rsi_bt_a2dp_disconnect(uint8_t *remote_dev_addr)
{
  rsi_bt_req_a2dp_disconnect_t bt_req_a2dp_disconnect = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_a2dp_disconnect.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_a2dp_disconnect.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_A2DP_DISCONNECT, &bt_req_a2dp_disconnect, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_a2dp_send_pcm_mp3_data(uint8_t *remote_dev_addr,
 *                                                   uint8_t *pcm_mp3_data,
 *                                                   uint16_t pcm_mp3_data_len,
 *                                                   uint8_t audio_type)
 * @brief      Send the PCM data to the BT stack.
 * @param[in]  remote_dev_addr - remote device address
 * @param[in]  pcm_mp3_data - PCM data buffer.
 * @param[in]  pcm_mp3_data_len - PCM data length.
 * @param[in]  audio_type - audio type.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 *
 */

int32_t rsi_bt_a2dp_send_pcm_mp3_data(uint8_t *remote_dev_addr,
                                      uint8_t *pcm_mp3_data,
                                      uint16_t pcm_mp3_data_len,
                                      uint8_t audio_type)
{
#if (!TA_BASED_ENCODER)
  /*These statement are added only to resolve compilation warning, value is unchanged*/
  UNUSED_PARAMETER(audio_type);
  USED_PARAMETER(remote_dev_addr);
  USED_PARAMETER(pcm_mp3_data);
  USED_PARAMETER(pcm_mp3_data_len);
#endif

#if (TA_BASED_ENCODER)
  rsi_bt_req_a2dp_pcm_mp3_data_t bt_req_a2dp_pcm_mp3_pkt_part1 = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_a2dp_pcm_mp3_pkt_part1.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_a2dp_pcm_mp3_pkt_part1.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  bt_req_a2dp_pcm_mp3_pkt_part1.pcm_mp3_data_len =
    RSI_MIN(sizeof(bt_req_a2dp_pcm_mp3_pkt_part1.pcm_mp3_data), pcm_mp3_data_len);
  bt_req_a2dp_pcm_mp3_pkt_part1.audio_type = audio_type;
  memcpy(bt_req_a2dp_pcm_mp3_pkt_part1.pcm_mp3_data, pcm_mp3_data, bt_req_a2dp_pcm_mp3_pkt_part1.pcm_mp3_data_len);

  rsi_bt_driver_send_cmd(RSI_BT_REQ_A2DP_PCM_MP3_DATA_PREFILL_1, &bt_req_a2dp_pcm_mp3_pkt_part1, NULL);

  rsi_bt_req_a2dp_pcm_mp3_data_t bt_req_a2dp_pcm_mp3_pkt_part2 = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_a2dp_pcm_mp3_pkt_part2.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_a2dp_pcm_mp3_pkt_part2.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  bt_req_a2dp_pcm_mp3_pkt_part2.pcm_mp3_data_len =
    RSI_MIN(sizeof(bt_req_a2dp_pcm_mp3_pkt_part2.pcm_mp3_data), pcm_mp3_data_len);
  bt_req_a2dp_pcm_mp3_pkt_part2.audio_type = audio_type;
  memcpy(bt_req_a2dp_pcm_mp3_pkt_part2.pcm_mp3_data,
         (pcm_mp3_data + sizeof(bt_req_a2dp_pcm_mp3_pkt_part1.pcm_mp3_data)),
         bt_req_a2dp_pcm_mp3_pkt_part2.pcm_mp3_data_len);

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_A2DP_PCM_MP3_DATA, &bt_req_a2dp_pcm_mp3_pkt_part2, NULL);
#endif
  return RSI_SUCCESS; // This is added to remove the compilation warning
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_a2dp_send_sbc_aac_data(uint8_t *remote_dev_addr,
 *                                                   uint8_t *sbc_aac_data,
 *                                                   uint16_t sbc_aac_data_len,
 *                                                   uint8_t audio_type)
 * @brief      Send the SBC/AAC data to the BT stack.
 * @param[in]  remote_dev_addr - remote device address
 * @param[in]  sbc_aac_data - SBC/AAC data buffer.
 * @param[in]  sbc_aac_data_len - SBC/AAC data length.
 * @param[in]  audio_type - audio type.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 *
 */
int32_t rsi_bt_a2dp_send_sbc_aac_data(uint8_t *remote_dev_addr,
                                      uint8_t *sbc_aac_data,
                                      uint16_t sbc_aac_data_len,
                                      uint8_t audio_type)
{
  rsi_bt_req_a2dp_sbc_aac_data_t bt_req_a2dp_sbc_aac_pkt;
  memset(&bt_req_a2dp_sbc_aac_pkt, 0, sizeof(bt_req_a2dp_sbc_aac_pkt));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_a2dp_sbc_aac_pkt.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_a2dp_sbc_aac_pkt.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  bt_req_a2dp_sbc_aac_pkt.sbc_aac_data_len = RSI_MIN(sizeof(bt_req_a2dp_sbc_aac_pkt.sbc_aac_data), sbc_aac_data_len);
  bt_req_a2dp_sbc_aac_pkt.audio_type       = audio_type;
  memcpy(bt_req_a2dp_sbc_aac_pkt.sbc_aac_data, sbc_aac_data, bt_req_a2dp_sbc_aac_pkt.sbc_aac_data_len);

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_A2DP_SBC_AAC_DATA, &bt_req_a2dp_sbc_aac_pkt, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_a2dp_start(uint8_t *remote_dev_addr)
 * @brief      Send the A2DP start to the BT stack.
 * @param[in]  remote_dev_addr - remote device address.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 *
 */
int32_t rsi_bt_a2dp_start(uint8_t *remote_dev_addr)
{
  rsi_bt_req_a2dp_start_t bt_req_a2dp_start = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_a2dp_start.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_a2dp_start.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_A2DP_START, &bt_req_a2dp_start, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_a2dp_suspend(uint8_t *remote_dev_addr)
 * @brief      Send the A2DP suspend to the BT stack.
 * @param[in]  remote_dev_addr - remote device address.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 *
 */
int32_t rsi_bt_a2dp_suspend(uint8_t *remote_dev_addr)
{
  rsi_bt_req_a2dp_suspend_t bt_req_a2dp_suspend = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_a2dp_suspend.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_a2dp_suspend.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_A2DP_SUSPEND, &bt_req_a2dp_suspend, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_a2dp_close(uint8_t *remote_dev_addr)
 * @brief      Send the A2DP close to the BT stack.
 * @param[in]  remote_dev_addr - remote device address.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 *
 */
int32_t rsi_bt_a2dp_close(uint8_t *remote_dev_addr)
{
  rsi_bt_req_a2dp_close_t bt_req_a2dp_close = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_a2dp_close.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_a2dp_close.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_A2DP_CLOSE, &bt_req_a2dp_close, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_a2dp_abort(uint8_t *remote_dev_addr)
 * @brief      Send the A2DP abort to the BT stack.
 * @param[in]  remote_dev_addr - remote device address
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 *
 */
int32_t rsi_bt_a2dp_abort(uint8_t *remote_dev_addr)
{
  rsi_bt_req_a2dp_abort_t bt_req_a2dp_abort = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_a2dp_abort.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_a2dp_abort.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_A2DP_ABORT, &bt_req_a2dp_abort, NULL);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_a2dp_get_config(uint8_t *remote_dev_addr,
 * 											  rsi_bt_resp_a2dp_get_config_t *sbc_resp_cap)
 * @brief      Get A2DP CODEC configuration.
 * @param[in]  remote_dev_addr - remote device address
 * @param[out] sbc_resp_cap - to capture response from this API.
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 *
 */
int32_t rsi_bt_a2dp_get_config(uint8_t *remote_dev_addr, rsi_bt_resp_a2dp_get_config_t *sbc_resp_cap)
{
  rsi_bt_req_a2dp_get_config_t bt_req_a2dp_get_config = { { 0 } };
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_req_a2dp_get_config.dev_addr, remote_dev_addr);
#else
  memcpy(bt_req_a2dp_get_config.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_A2DP_GET_CONFIG, &bt_req_a2dp_get_config, sbc_resp_cap);
}

/*==============================================*/
/**
 * @fn         int32_t rsi_bt_a2dp_set_config(uint8_t *remote_dev_addr,
 * 											  rsi_bt_a2dp_sbc_codec_cap_t *set_sbc_cap,
 * 											  int32_t *status)
 * @brief      Set A2DP CODEC configuration.
 * @param[in]  remote_dev_addr - remote device address
 * @param[in]  set_sbc_cap - Set SBC
 * @param[in]  status - This is the response status
 * @return     0		-	Success \n
 *             Non-Zero Value	-	Failure
 *
 */
int32_t rsi_bt_a2dp_set_config(uint8_t *remote_dev_addr, rsi_bt_a2dp_sbc_codec_cap_t *set_sbc_cap, int32_t *status)
{
  rsi_bt_req_a2dp_set_config_t bt_a2dp_set_config;
  memset(&bt_a2dp_set_config, 0, sizeof(bt_a2dp_set_config));
#ifdef BD_ADDR_IN_ASCII
  rsi_ascii_dev_address_to_6bytes_rev(bt_a2dp_set_config.dev_addr, remote_dev_addr);
#else
  memcpy(bt_a2dp_set_config.dev_addr, (int8_t *)remote_dev_addr, 6);
#endif
  memcpy(&bt_a2dp_set_config.sbc_cap, set_sbc_cap, sizeof(rsi_bt_a2dp_sbc_codec_cap_t));

  return rsi_bt_driver_send_cmd(RSI_BT_REQ_A2DP_SET_CONFIG, &bt_a2dp_set_config, status);
}

#endif
/** @} */
