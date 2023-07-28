/*******************************************************************************
* @file  rsi_prop_protocol.c
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

/*
  Include files
  */
#ifdef RSI_PROP_PROTOCOL_ENABLE
#include "rsi_driver.h"
#include "rsi_bt.h"
#include "rsi_hid.h"
#include "rsi_bt_common.h"
#include "rsi_ble.h"
#include "rsi_bt_config.h"
#include "rsi_ble_config.h"
#include "stdio.h"
#include "rsi_prop_protocol.h"

uint32_t rsi_get_bt_state(rsi_bt_cb_t *bt_cb);
void rsi_bt_set_status(rsi_bt_cb_t *bt_cb, int32_t status);
#define BT_CMD_SEM 0x2

/**
 * @fn          uint16_t rsi_bt_prepare_prop_protocol_pkt(uint16_t cmd_type, void *cmd_struct, rsi_pkt_t *pkt)
 * @brief       Form the payload of the PROP_PROTOCOL command packet
 * @param[in]   cmd_type     -  type of the command
 * @param[in]   cmd_struct    -  pointer of the command structure
 * @param[out]  pkt          -  pointer of the packet to fill the contents of the payload
 * @return      0              - Success \n 
 *              Non-Zero Value - Failure
 */

uint16_t rsi_bt_prepare_prop_protocol_pkt(uint16_t cmd_type, void *cmd_struct, rsi_pkt_t *pkt)
{
  uint16_t payload_size = 0;
#ifdef PROP_PROTOCOL_TX_HOST_FLOW_CTRL
  //These statements are added only to resolve compilation warning, value is unchanged
  // Get pool availability info prop_protocol cb struct pointer
  rsi_bt_cb_t *prop_protocol_cb = rsi_driver_cb->prop_protocol_cb;
  rsi_pkt_pool_t *pool_cb       = &prop_protocol_cb->bt_tx_pool;
#endif
  switch (cmd_type) {
    case RSI_PROP_PROTOCOL_CMD: {
      pkt->data[0] = ((uint8_t *)cmd_struct)[0];
      payload_size = ((uint8_t *)cmd_struct)[1] + 2;
      memcpy(pkt->data, cmd_struct, payload_size);
      break;
    }
    case RSI_PROP_PROTOCOL_CMD_PER: {
      pkt->data[0] = (*(uint8_t *)cmd_struct);
      printf("cmd type :%d\n", pkt->data[0]);
      switch (pkt->data[0]) {
        case PROP_PROTOCOL_DATA_CONFIG_REQ:
          payload_size = sizeof(rsi_prop_protocol_tx_data_t);
          memcpy(pkt->data, cmd_struct, payload_size);
          break;
        case PROP_PROTOCOL_ENCRYPTION_REQ:
          payload_size = sizeof(rsi_prop_protocol_encryption_data_t);
          memcpy(pkt->data, cmd_struct, payload_size);
          break;
        case PROP_PROTOCOL_PER_MODE_REQ:
          payload_size = sizeof(rsi_prop_protocol_per_stats_t);
          memcpy(pkt->data, cmd_struct, payload_size);
          break;
        default:
          break;
      }
      break;
    }
    default:
      break;
  }

#ifdef PROP_PROTOCOL_TX_HOST_FLOW_CTRL
  if (pool_cb->avail == 0) {
    prop_protocol_cb->buf_status = RSI_TRUE;
#ifdef DBG_PROP_PROTOCOL_HOST_FLOW_CTRL_TEST
    printf("\n<######## PROP_PROTOCOL Buffer full status set --> available buffers :%d ########>\n", pool_cb->avail);
    rsi_unmask_event(RSI_TX_EVENT);
#endif
  }
#endif

  // return payload_size
  return payload_size;
}

#ifdef DBG_PROP_PROTOCOL_HOST_FLOW_CTRL_TEST
uint16_t start_prop_protocol_protocol = 0;
#endif
/** @} */
/**
 * @fn          int32_t rsi_prop_protocol_driver_send_cmd(uint16_t cmd, void *cmd_struct, void *resp)
 * @brief       Fill commands and places into prop_protocol TX queue
 * @param[in]   cmd       - type of the command to send 
 * @param[in]   cmd_stuct - pointer of the packet structure to send
 * @return      0              - Success \n 
 *              Non-Zero Value - Failure
 */
int32_t rsi_prop_protocol_driver_send_cmd(uint16_t cmd, void *cmd_struct, void *resp)
{
  //This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(resp);
  uint16_t payload_size         = 0;
  uint16_t protocol_type        = 0;
  int32_t status                = RSI_SUCCESS;
  rsi_pkt_t *pkt                = NULL;
  uint8_t *host_desc            = NULL;
  rsi_bt_cb_t *prop_protocol_cb = NULL;
  rsi_common_cb_t *common_cb    = rsi_driver_cb->common_cb;

  if ((common_cb->state < RSI_COMMON_OPERMODE_DONE)) {
    //command given in wrong state
    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }

  protocol_type = rsi_bt_get_proto_type(cmd, &prop_protocol_cb);

  if (protocol_type == 0xFF) {
    // return packet allocation failure error
    return RSI_ERROR_PKT_ALLOCATION_FAILURE;
  }
#ifndef PROP_PROTOCOL_TX_HOST_FLOW_CTRL
  rsi_bt_set_wait_bitmap(RSI_PROTO_PROP_PROTOCOL, BT_CMD_SEM);
  if (rsi_semaphore_wait(&prop_protocol_cb->bt_cmd_sem, RSI_PROP_PROTOCOL_CMD_TX_PKT_SEND_WAIT_TIME)
      != RSI_ERROR_NONE) {
#ifndef RSI_WAIT_TIMEOUT_EVENT_HANDLE_TIMER_DISABLE
    if (rsi_driver_cb_non_rom->rsi_wait_timeout_handler_error_cb != NULL) {
      rsi_driver_cb_non_rom->rsi_wait_timeout_handler_error_cb(RSI_ERROR_RESPONSE_TIMEOUT, PROP_PROTOCOL_CMD);
    }
#endif
    return RSI_ERROR_PROP_PROTOCOL_CMD_IN_PROGRESS;
  }
  // allocate command buffer from prop_protocol pool
  pkt = rsi_pkt_alloc(&prop_protocol_cb->bt_tx_pool);
#else
  pkt = ROM_WL_rsi_pkt_alloc_non_blocking(global_cb_p, &prop_protocol_cb->bt_tx_pool);
#endif

  // Get host descriptor pointer
  host_desc = (pkt->desc);

  // If allocation of packet fails
  if (pkt == NULL) {
#ifndef PROP_PROTOCOL_TX_HOST_FLOW_CTRL
    rsi_bt_clear_wait_bitmap(RSI_PROTO_PROP_PROTOCOL, BT_CMD_SEM);
    rsi_semaphore_post(&prop_protocol_cb->bt_cmd_sem);
#endif
    // return packet allocation failure error
    return RSI_ERROR_PKT_ALLOCATION_FAILURE;
  }

  // Memset host descriptor
  memset(host_desc, 0, RSI_HOST_DESC_LENGTH);

  prop_protocol_cb->sync_rsp = 0;

  payload_size = 0;

#ifdef RSI_PROP_PROTOCOL_ENABLE
  if (protocol_type == RSI_PROTO_PROP_PROTOCOL) {
    // Memset data
    memset(pkt->data, 0, (RSI_PROP_PROTOCOL_CMD_LEN - sizeof(rsi_pkt_t)));
    payload_size = rsi_bt_prepare_prop_protocol_pkt(cmd, cmd_struct, pkt);
  }
#endif
#if 0
  if (prop_protocol_cb->buf_status)
  {
    rsi_pkt_free(&prop_protocol_cb->bt_tx_pool, pkt);
    prop_protocol_cb->buf_status = 0;

    rsi_semaphore_post(&prop_protocol_cb->bt_cmd_sem);
    return RSI_ERROR_PROP_PROTOCOL_DEV_BUF_FULL;
  }
#endif
  // Fill payload length
  rsi_uint16_to_2bytes(host_desc, (payload_size & 0xFFF));

  // Fill frame type
  host_desc[1] |= (RSI_BT_Q << 4);

  // Fill frame type
  rsi_uint16_to_2bytes(&host_desc[2], cmd);

  // Save expected response type
  prop_protocol_cb->expected_response_type = 0; //cmd;

  // Save expected response type
  prop_protocol_cb->expected_response_buffer = 0; //resp;

#ifdef SAPIS_BT_STACK_ON_HOST
  send_cmd_from_app_to_bt_stack(host_desc, pkt->data, payload_size);
#else
    // Enqueue packet to PROP_PROTOCOL TX queue
  rsi_enqueue_pkt(&rsi_driver_cb->prop_protocol_tx_q, pkt);

  // Set TX packet pending event
  rsi_set_event(RSI_TX_EVENT);
#ifdef DBG_PROP_PROTOCOL_HOST_FLOW_CTRL_TEST
  start_prop_protocol_protocol++;
  if (start_prop_protocol_protocol >= 10 && prop_protocol_cb->bt_tx_pool.avail != 0) {
    rsi_mask_event(RSI_TX_EVENT);
  }
#endif
#endif

  // Return status
  return status;
}

/**
 * @fn          void rsi_prop_protocol_tx_done(rsi_bt_cb_t *prop_protocol_cb, rsi_pkt_t *pkt) 
 * @brief       Handle the protocol specific PROP_PROTOCOL data transfer completion
 * @param[in]   prop_protocol_cb - PROP_PROTOCOL control block
 * @return      void
 */

void rsi_prop_protocol_tx_done(rsi_bt_cb_t *prop_protocol_cb, rsi_pkt_t *pkt)
{
#ifdef PROP_PROTOCOL_TX_HOST_FLOW_CTRL
  // Get prop_protocol cb struct pointer
  rsi_prop_protocol_cb_t *prop_protocol_specific_cb = prop_protocol_cb->bt_global_cb->prop_protocol_specific_cb;
  // Get pool availability info prop_protocol cb struct pointer
  rsi_pkt_pool_t *pool_cb = &prop_protocol_cb->bt_tx_pool;
#endif
  // If the command is not a synchronous / blocking one
  if (!prop_protocol_cb->sync_rsp) {
    // Set PROP_PROTOCOL command status as success
    rsi_bt_set_status(prop_protocol_cb, RSI_SUCCESS);

#ifdef PROP_PROTOCOL_TX_HOST_FLOW_CTRL
    //rsi_pkt_free_non_blocking(&prop_protocol_cb->bt_tx_pool, pkt);
    ROM_WL_rsi_pkt_free_non_blocking(global_cb_p, &prop_protocol_cb->bt_tx_pool, pkt);
#else
    rsi_pkt_free(&prop_protocol_cb->bt_tx_pool, pkt);
    // Post the semaphore which is waiting on driver_send API
    rsi_bt_clear_wait_bitmap(RSI_PROTO_PROP_PROTOCOL, BT_CMD_SEM);
    rsi_semaphore_post(&prop_protocol_cb->bt_cmd_sem);
#endif
  }

#ifdef PROP_PROTOCOL_TX_HOST_FLOW_CTRL
#ifdef DBG_PROP_PROTOCOL_HOST_FLOW_CTRL_TEST
  printf("\n<===== PROP_PROTOCOL pkt tx done :available buffers =%d\t Buffer full status = %d======>\n",
         pool_cb->avail,
         prop_protocol_cb->buf_status);
#endif

  if ((pool_cb->avail >= PROP_PROTOCOL_MORE_DATA_REQUEST_THRESHOLD) && (prop_protocol_cb->buf_status == RSI_TRUE)) {
    prop_protocol_cb->buf_status = 0;
#ifdef DBG_PROP_PROTOCOL_HOST_FLOW_CTRL_TEST
    printf("\n<===== RAISE an event to PROP_PROTOCOL application to allow more data ### available buffers= %d======>\n",
           pool_cb->avail);
#endif
    if (prop_protocol_specific_cb->rsi_prop_protocol_data_request_callback != NULL) {
      prop_protocol_specific_cb->rsi_prop_protocol_data_request_callback();
    }
  }
#endif
}

/**
 * @fn          void rsi_prop_protocol_common_tx_done(rsi_pkt_t *pkt)
 * @brief       Handle PROP_PROTOCOL data transfer completion
 * @param[in]   pkt - pointer to packet 
 * @return      void
 *
 */

void rsi_prop_protocol_common_tx_done(rsi_pkt_t *pkt)
{
  uint8_t *host_desc            = NULL;
  uint8_t protocol_type         = 0;
  uint16_t rsp_type             = 0;
  rsi_bt_cb_t *prop_protocol_cb = NULL;

  // Get Host Descriptor
  host_desc = pkt->desc;

  // Get Command response Type
  rsp_type = rsi_bytes2R_to_uint16(host_desc + RSI_BT_RSP_TYPE_OFFSET);

  // Get the protocol Type
  protocol_type = rsi_bt_get_proto_type(rsp_type, &prop_protocol_cb);

  if (protocol_type == 0xFF) {
    return;
  }
  // Call PROP_PROTOCOL transfer done
  rsi_prop_protocol_tx_done(prop_protocol_cb, pkt);
}

/**
 * @fn          void rsi_prop_protocol_callbacks_handler(rsi_bt_cb_t *prop_protocol_cb, uint16_t rsp_type, uint8_t *payload, uint16_t payload_length)
 * @brief       Handle PROP_PROTOCOL callbacks
 * @param[in]   prop_protocol_cb - prop_protocol callbacks
 * @param[in]   rsp_type - Type 
 * @param[in]   payload - Payload 
 * @param[in]   payload_length -Payload length 
 * @return      void
 *
 */
void rsi_prop_protocol_callbacks_handler(rsi_bt_cb_t *prop_protocol_cb,
                                         uint16_t rsp_type,
                                         uint8_t *payload,
                                         uint16_t payload_length)
{
  //This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(payload_length);
  //! Get prop_protocol cb struct pointer
  rsi_prop_protocol_cb_t *prop_protocol_specific_cb = prop_protocol_cb->bt_global_cb->prop_protocol_specific_cb;

  // updating the response status;
  // status  = rsi_bt_get_status (prop_protocol_cb);
  // Check each cmd_type like decode_resp_handler and call the respective callback
  switch (rsp_type) {
    case RSI_PROP_PROTOCOL_CMD: {
      if (prop_protocol_specific_cb->prop_protocol_async_resp_handler != NULL) {
        prop_protocol_specific_cb->prop_protocol_async_resp_handler(payload);
      }
    } break;
    default:
      break;
  }
}

/**
 * @fn          void rsi_prop_protocol_register_callbacks(rsi_prop_protocol_resp_handler_t prop_protocol_async_resp_handler,
                                rsi_prop_protocol_data_request_callback_t rsi_prop_protocol_data_request_callback)
 * @brief       Register callbacks
 * @param[in]   prop_protocol_async_resp_handler - prop_protocol asynchronization handler 
 * @param[in]   rsi_prop_protocol_data_request_callback - Data request callback
 * @return      void
 *
 */
void rsi_prop_protocol_register_callbacks(
  rsi_prop_protocol_resp_handler_t prop_protocol_async_resp_handler,
  rsi_prop_protocol_data_request_callback_t rsi_prop_protocol_data_request_callback)
{
  // Get prop_protocol cb struct pointer
  rsi_prop_protocol_cb_t *prop_protocol_specific_cb =
    rsi_driver_cb->prop_protocol_cb->bt_global_cb->prop_protocol_specific_cb;

  // Assign the call backs to the respective call back
  prop_protocol_specific_cb->prop_protocol_async_resp_handler = prop_protocol_async_resp_handler;
  // Assign the call backs to the more data request respective call back
  prop_protocol_specific_cb->rsi_prop_protocol_data_request_callback = rsi_prop_protocol_data_request_callback;
}

/**
 * @fn          int32_t rsi_driver_process_prop_protocol_resp(
 *                                                  rsi_bt_cb_t *prop_protocol_cb,
 *                                                  rsi_pkt_t *pkt,
 *              void (*rsi_bt_async_callback_handler)(rsi_bt_cb_t *cb, uint16_t type, uint8_t *data, uint16_t length))  
 * @brief       Process PROP_PROTOCOL RX packets
 * @param[in ]  prop_protocol_cb - PROP_PROTOCOL control block
 * @param[in]   pkt   - pointer to received RX packet
 * @param[in]   type - Type 
 * @return      0              - Success \n 
 *              Non-Zero Value - Failure
 */

int32_t rsi_driver_process_prop_protocol_resp(
  rsi_bt_cb_t *prop_protocol_cb,
  rsi_pkt_t *pkt,
  void (*rsi_bt_async_callback_handler)(rsi_bt_cb_t *cb, uint16_t type, uint8_t *data, uint16_t length))
{
  uint16_t rsp_type  = 0;
  int16_t status     = RSI_SUCCESS;
  uint8_t *host_desc = NULL;
  uint8_t *payload;
  uint16_t payload_length;

  // Get Host Descriptor
  host_desc = pkt->desc;

  // Get Command response Type
  rsp_type = rsi_bytes2R_to_uint16(host_desc + RSI_BT_RSP_TYPE_OFFSET);

  // Get Payload start pointer
  payload = pkt->data;

  // Get Payload length
  payload_length = (rsi_bytes2R_to_uint16(host_desc) & 0xFFF);

  // Get Status
  status = rsi_bytes2R_to_uint16(host_desc + RSI_BT_STATUS_OFFSET);

  // Update the status in bt_cb
  rsi_bt_set_status(prop_protocol_cb, status);

  // Check bt_cb for any task is waiting for response
  if (prop_protocol_cb->expected_response_type == rsp_type) {
    // Clear expected response type
    prop_protocol_cb->expected_response_type = 0;

    // Copy the expected response to response structure/buffer, if any, passed in API
    if (prop_protocol_cb->expected_response_buffer != NULL) {
      // If (payload_length <= RSI_BLE_GET_MAX_PAYLOAD_LENGTH(expected_response_type)) //TODO Give the proper error code
      memcpy(prop_protocol_cb->expected_response_buffer, payload, payload_length);

      // Save expected_response pointer to a local variable, since it is being cleared below
      payload = prop_protocol_cb->expected_response_buffer;

      // Clear the expected response pointer
      prop_protocol_cb->expected_response_buffer = NULL;
    }

    // Check if it is sync response
    if (prop_protocol_cb->sync_rsp) {
      // Clear sync rsp variable
      prop_protocol_cb->sync_rsp = 0;

      rsi_bt_clear_wait_bitmap(RSI_PROTO_PROP_PROTOCOL, BT_CMD_SEM);
      // Signal the bt semaphore
      rsi_semaphore_post(&prop_protocol_cb->bt_cmd_sem);
    } else {
      if (rsi_bt_async_callback_handler != NULL) {
        // Call callbacks handler
        rsi_bt_async_callback_handler(prop_protocol_cb, rsp_type, payload, payload_length);
      }
    }
  } else {
    if (rsi_bt_async_callback_handler != NULL) {
      // Call callbacks handler
      rsi_bt_async_callback_handler(prop_protocol_cb, rsp_type, payload, payload_length);
    }
  }

  return status;
}
#endif