/*******************************************************************************
* @file  rsi_prop_protocol_app_DEMO_69.c
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
/**
 * @file    rsi_prop_protocol_app_DEMO_69.c
 * @version 0.1
 * @date    01 Feb 2020
 *
 *
 *
 *  @brief : This file contains example application for PROP_PROTOCOL test mode.
 *
 *  @section Description  This application for PROP_PROTOCOL API's tests.
 *
 */

/*=======================================================================*/
//   ! INCLUDES
/*=======================================================================*/

#include <rsi_common_app.h>
#if COEX_MAX_APP_BLE_2MAS_8SLAV
//! BLE include file to refer BLE APIs
#include <rsi_ble_apis.h>
#include <rsi_ble_config.h>
#include <rsi_bt_common_apis.h>
#include <rsi_bt_common.h>
#include <rsi_ble.h>
#include <rsi_prop_protocol.h>
#include <rsi_bt_common.h>
#include <string.h>
#ifdef FRDM_K28F
#include "fsl_lptmr.h"
#elif defined(MXRT_595s)
#include "fsl_mrt.h"
#endif
#include "rsi_wlan_apis.h"
#include "rsi_driver.h"
#include "rsi_common_app_DEMO_69.h"

/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/
//! Common include file
#define RSI_BLE_LOCAL_NAME  "PROP_PROTOCOL+RSI_COEX_MAX_APP_BLE_2MAS_8SLAV"
#define MAX_CHANNELS        8
#define OUTGOING_BUFFER_LEN 256
#define IDLE_PROP_PROTOCOL  0x00
#define START_PROP_PROTOCOL 0x01
#define STOP_PROP_PROTOCOL  0x02
#define PRINT_STATS         1
#define GET_PROP_PROTOCOL_STATS
#define START_PROP_PROTOCOL_DELAY (10000) // Only applied at powerup
#ifdef GET_PROP_PROTOCOL_STATS
#define STAT_REQUEST_PERIOD (1000) // 1Hz
#endif                             // GET_PROP_PROTOCOL_STATS
#ifdef CYCLE_PROP_PROTOCOL_START_STOP
#define CYCLE_PROP_PROTOCOL_PERIOD (10000)
#endif
#ifdef TRIGGER_PROP_PROTOCOL_STOP
#define STOP_PROP_PROTOCOL_DELAY (10000)
#endif

/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/
//! application defines
static uint32_t prop_protocol_app_event_map;
static uint8_t next_state        = 0;
static volatile bool bStatsTick  = 0;
volatile bool bPROP_PROTOCOLTick = 0;

uint32_t prop_protocol_succ[MAX_CHANNELS] = { 0 };
uint32_t prop_protocol_col[MAX_CHANNELS]  = { 0 };
uint32_t prop_protocol_blk[MAX_CHANNELS]  = { 0 };
uint32_t evt_counter[MAX_CHANNELS]        = { 0 };
uint8_t update_bcst_buffer[] = { 0x0E, 0x0B, 0x09, 0x4E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t outgoing_buffer[OUTGOING_BUFFER_LEN] = { 0 };
uint8_t outgoing_buffer_wr                   = 0;
uint8_t outgoing_buffer_rd                   = 0;
uint8_t outgoing_msg[256]                    = { 0 };
enum {
  PROP_PROTOCOL_APP_EVT_SEND_NEW_BUFFER = 0,
};

uint32_t prop_protocol_protocol_recv_buf_full = 0;
/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/
extern bool rsi_ble_running, rsi_bt_running, powersave_cmd_given;
extern rsi_mutex_handle_t power_cmd_mutex;
#if WLAN_SYNC_REQ
extern rsi_semaphore_handle_t sync_coex_prop_protocol_sem;
#endif
extern bool rsi_wlan_running;
extern rsi_semaphore_handle_t bt_inquiry_sem, ble_scan_sem;
/*========================================================================*/
//!  CALLBACK FUNCTIONS
/*=======================================================================*/

/*=======================================================================*/
//   ! EXTERN FUNCTIONS
/*=======================================================================*/

/*=======================================================================*/
//   ! PROCEDURES
/*=======================================================================*/

/*==============================================*/
/**
 * @fn         rsi_app_init_events
 * @brief      initializes the event parameter.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used during BLE initialization.
 */
static void rsi_prop_protocol_app_init_events()
{
  prop_protocol_app_event_map = 0;
  //! prop_protocol_app_event_mask = 0xFFFFFFFF;
  return;
}

/*==============================================*/
/**
 * @fn         rsi_app_get_event
 * @brief      returns the first set event based on priority
 * @param[in]  none.
 * @return     int32_t
 *             > 0  = event number
 *             -1   = not received any event
 * @section description
 * This function returns the highest priority event among all the set events
 */
static int32_t rsi_prop_protocol_app_get_event(void)
{
  uint32_t ix;

  for (ix = 0; ix < 32; ix++) {
    if (prop_protocol_app_event_map & (1 << ix)) {
      return ix;
    }
  }

  return (-1);
}

/*==============================================*/
/**
 * @fn         rsi_prop_protocol_app_set_event
 * @brief      sets the event
 * @param[in]  uint32_t ix - event number to set.
 * @return     none
 * @section description
 * This function sets the event in variable prop_protocol_app_event_map
 */
static void rsi_prop_protocol_app_set_event(uint32_t ix)
{
  prop_protocol_app_event_map |= (1 << ix);
}

/*==============================================*/
/**
 * @fn         rsi_prop_protocol_app_clear_event
 * @brief      clears the event
 * @param[in]  uint32_t ix - event number to be cleared.
 * @return     none
 * @section description
 * This function clears the event in variable prop_protocol_app_event_map
 */
static void rsi_prop_protocol_app_clear_event(uint32_t ix)
{
  prop_protocol_app_event_map &= ~ix;
}

void rsi_prop_protocol_send_outgoing_msg(void)
{
//This statement is added to resolve the warning: [-unused-but-set-variable],so declared the variable in a macro in which the variable is used.
#ifdef DBG_PROP_PROTOCOL_HOST_FLOW_CTRL_TEST
  int32_t status = 0;
#endif

  if (outgoing_buffer_rd == outgoing_buffer_wr) {
    // Buffer is empty
    return;
  }

  // Copy next message
  uint8_t len = outgoing_buffer[(outgoing_buffer_rd + 1) & (OUTGOING_BUFFER_LEN - 1)] + 2;
  for (uint8_t i = 0; i < len; i++) {
    outgoing_msg[i]                     = outgoing_buffer[outgoing_buffer_rd];
    outgoing_buffer[outgoing_buffer_rd] = 0;
    outgoing_buffer_rd                  = (outgoing_buffer_rd + 1) & (OUTGOING_BUFFER_LEN - 1);
  }

#ifdef DBG_PROP_PROTOCOL_HOST_FLOW_CTRL_TEST
  if (!prop_protocol_protocol_recv_buf_full)
#endif
  {
//This statement is added to resolve the warning: [-unused-but-set-variable],so declared the variable in a macro in which the variable is used.
#ifdef DBG_PROP_PROTOCOL_HOST_FLOW_CTRL_TEST
    // Send asynchronous message out
    status = rsi_prop_protocol_send_cmd(outgoing_msg, 0);
    if (status != 0) {
      prop_protocol_protocol_recv_buf_full = 1;
    }
#else
    rsi_prop_protocol_send_cmd(outgoing_msg, 0);
#endif
  }

  if (outgoing_buffer_rd != outgoing_buffer_wr) {
    rsi_prop_protocol_app_set_event(PROP_PROTOCOL_APP_EVT_SEND_NEW_BUFFER);
  }
}
#ifdef FRDM_K28F
void LPTMR0_LPTMR1_IRQHandler(void)
{
  if ((LPTMR0->CSR & LPTMR_CSR_TEN_MASK) && // Timer enabled
      (LPTMR0->CSR & LPTMR_CSR_TIE_MASK) && // Interrupts enabled
      (LPTMR0->CSR & LPTMR_CSR_TCF_MASK))   // Compare flag is set
  {
    bPROP_PROTOCOLTick = 1;
    LPTMR_StopTimer(LPTMR0);
  }
#ifdef GET_PROP_PROTOCOL_STATS
  if ((LPTMR1->CSR & LPTMR_CSR_TEN_MASK) && // Timer enabled
      (LPTMR1->CSR & LPTMR_CSR_TIE_MASK) && // Interrupts enabled
      (LPTMR1->CSR & LPTMR_CSR_TCF_MASK))   // Compare flag is set
  {
    bStatsTick = 1;
    LPTMR_StopTimer(LPTMR1);
  }
#endif // GET_PROP_PROTOCOL_STATS
}
#elif defined(MXRT_595s)
static volatile bool mrtEnableCount0 = false;
static volatile uint32_t mrtCountValue0 = 0;
static volatile uint32_t mrtDividerValue0 = 0;
static volatile bool mrtEnableCount1 = false;
static volatile uint32_t mrtCountValue1 = 0;
static volatile uint32_t mrtDividerValue1 = 0;
uint32_t mrt_clock = 0;

void MRT0_IRQHandler(void)
{
  if (MRT_GetStatusFlags_1(MRT0, kMRT_Channel_0) == MRT_CHANNEL_STAT_INTFLAG_MASK) {
    MRT_ClearStatusFlags(MRT0, kMRT_Channel_0, kMRT_TimerInterruptFlag);
    if (mrtEnableCount0 == true) {
      mrtCountValue0++;
      if (mrtCountValue0 == (1 << mrtDividerValue0)) {
        bPROP_PROTOCOLTick = 1;
        MRT_StopTimer(MRT0, kMRT_Channel_0);
      }
    } else {
      bPROP_PROTOCOLTick = 1;
      MRT_StopTimer(MRT0, kMRT_Channel_0);
    }
  }
  if (MRT_GetStatusFlags_1(MRT0, kMRT_Channel_1) == MRT_CHANNEL_STAT_INTFLAG_MASK) {
    MRT_ClearStatusFlags(MRT0, kMRT_Channel_1, kMRT_TimerInterruptFlag);
    if (mrtEnableCount1 == true) {
      mrtCountValue1++;
      if (mrtCountValue1 == (1 << mrtDividerValue1)) {
        bStatsTick = 1;
        MRT_StopTimer(MRT0, kMRT_Channel_1);
      }
    } else {
      bStatsTick = 1;
      MRT_StopTimer(MRT0, kMRT_Channel_1);
    }
  }
}
#endif
/*==============================================*/
/**
* @fn         rsi_prop_protocol_data_request_callback
 * @brief      invoked when asyncronous prop_protocol more data event packets are received.
 * @return     none.
 * @section description
 */
void rsi_prop_protocol_data_request_callback(void)
{
  printf("PROP_PROTOCOL DATA REQUEST Received from SAPI driver\n");
#ifdef DBG_PROP_PROTOCOL_HOST_FLOW_CTRL_TEST
  prop_protocol_protocol_recv_buf_full = 0;
  if (outgoing_buffer_rd != outgoing_buffer_wr) {
    rsi_prop_protocol_app_set_event(PROP_PROTOCOL_APP_EVT_SEND_NEW_BUFFER);
  }
#endif
}
/*==============================================*/
/**
 * @fn         rsi_prop_protocol_async_resp_handler
 * @brief      invoked when asyncronous prop_protocol stack packets are received.
 * @return     none.
 * @section description
 */
void rsi_prop_protocol_async_resp_handler(uint8_t *payload)
{
  uint8_t free_bytes_in_buffer = 0;

  if ((payload[0] == 0x07) && (payload[1] == 0x0C) && (payload[2] == 0x05) && (payload[3] == 0x03)
      && (payload[4] == 0x40) && (payload[6] == 0x01)) {
    // This is a command we get when PROP_PROTOCOL successfully transmit a message
    // We use this to update the broadcast buffer
    uint8_t ch = payload[5];
    if (ch < MAX_CHANNELS) {
      //LOG_PRINT("Event Tx from ch %d\n", ch);
      evt_counter[ch]++;

      // Update the buffer
      update_bcst_buffer[4] = ch;
      update_bcst_buffer[5] = (evt_counter[ch] >> 24) & 0xFF;
      update_bcst_buffer[6] = (evt_counter[ch] >> 16) & 0xFF;
      update_bcst_buffer[7] = (evt_counter[ch] >> 8) & 0xFF;
      update_bcst_buffer[8] = (evt_counter[ch] >> 0) & 0xFF;

      // Don't overwrite the buffer if there is no room left, otherwise we might corrupt it and create invalid packets
      // outgoing_buffer_rd should always lag behind wr
      if (outgoing_buffer_rd <= outgoing_buffer_wr) {
        // wr hasn't rolled over yet
        free_bytes_in_buffer = (OUTGOING_BUFFER_LEN - (outgoing_buffer_wr - outgoing_buffer_rd));
      } else {
        // wr rolled over
        free_bytes_in_buffer = (outgoing_buffer_rd - outgoing_buffer_wr);
      }
      free_bytes_in_buffer -= 1; // prevent full buffer from looking empty

      if (free_bytes_in_buffer <= sizeof(update_bcst_buffer)) {
        // No room!
        return;
      }

      for (uint8_t j = 0; j < sizeof(update_bcst_buffer); j++) {
        // Add new message to the buffer
        outgoing_buffer[outgoing_buffer_wr] = update_bcst_buffer[j];
        outgoing_buffer_wr                  = (outgoing_buffer_wr + 1) & (OUTGOING_BUFFER_LEN - 1);
      }
      rsi_prop_protocol_app_set_event(PROP_PROTOCOL_APP_EVT_SEND_NEW_BUFFER);
    }
  } else if ((payload[0] == 0x05) && (payload[1] == 0x0C) && (payload[2] == 0x03) && (payload[3] == 0x01)
             && (payload[4] == 0x6F)) {
    //LOG_PRINT("PROP_PROTOCOL started\n");
  } else if ((payload[0] == 0x05) && (payload[1] == 0x0C) && (payload[2] == 0x03) && (payload[3] == 0x01)
             && (payload[4] == 0xC9)) {
    //LOG_PRINT("PROP_PROTOCOL Ack\n");
  } else {
#ifdef PROP_PROTOCOL_PKT_TYPE2_RX
    //else if(payload[1] == 0xAA) {
    char mesg[100]       = { 0 };
    uint16_t offset      = 0;
    uint16_t written     = 0;
    uint8_t ucBufferSize = payload[0];

    for (int i = 0; i < ucBufferSize; i++) {
      written = snprintf(mesg + offset, sizeof(mesg) - offset, "%02X ", payload[i + 1]);
      offset += written;
    }
    LOG_PRINT(mesg);
    LOG_PRINT("\n");
    /*   }
	else
	{ */
#endif
#ifdef GET_PROP_PROTOCOL_STATS
    // Stats packets
    // Print out the contents
    switch (payload[1]) {
      case 0x0A:
        //LOG_PRINT("[Blocked]: ");
        break;
      case 0x0B:
        LOG_PRINT("[Success]: ");
        break;
      case 0x08:
        //LOG_PRINT("[Catchup]: ");
        break;
      case 0x09:
        //LOG_PRINT("[Collisions]: ");
        break;
      default:
        return;
    }
    uint8_t num_channels = payload[2] / 2;
    uint8_t stats_val    = payload[1];

    for (uint8_t i = 0; i < num_channels; i++) {
      uint16_t val = payload[3 + (i * 2)] | (payload[3 + (i * 2) + 1] << 8);

      uint32_t *loc;
      if (stats_val == 0x0A) {
        loc = &prop_protocol_blk[i];
      } else if (stats_val == 0x0B) {
        loc = &prop_protocol_succ[i];
      } else if (stats_val == 0x09) {
        break;
        loc = &prop_protocol_col[i];
      } else {
        break;
      }

      // Do the increment
      uint32_t loc_val = *loc;
      if (val < loc_val) {
        // Counter rolled over
        *loc += (65536 - loc_val) + val;
      } else {
        *loc += (val - loc_val);
      }

      if (stats_val == 0x0B) {
        LOG_PRINT(" {%d} ", ((prop_protocol_succ[i] * 100) / (prop_protocol_succ[i] + prop_protocol_blk[i])));
      }
    }

    if (stats_val == 0x0B) {
      LOG_PRINT("\r\n");
    }
#endif
  }
}

/*==============================================*/
/**
 * @fn         rsi_app_task
 * @brief      this function will execute when BLE events are raised.
 * @param[in]  none.
 * @return     none.
 * @section description
 */
void rsi_prop_protocol_task(void)
{
  static bool prop_protocol_stopped = false;
  int32_t event_id;
  //This statement is added to resolve the warning: [-unused-but-set-variable],so declared the variable in a macro in which the variable is used.
#ifdef DBG_PROP_PROTOCOL_HOST_FLOW_CTRL_TEST
  int32_t status = 0;
#endif
  //! checking for events list
#ifdef GET_PROP_PROTOCOL_STATS
  if (bStatsTick) {
    bStatsTick = 0;
#ifndef GET_PROP_PROTOCOL_STATS_WHEN_OFF
    if (!prop_protocol_stopped)
#endif
    {
      //LOG_PRINT("Request stats\n");
      rsi_prop_protocol_stack_cmd_t stat_cmd;
      stat_cmd.data[0] = 0x0C;
      stat_cmd.data[1] = 0x04;
      stat_cmd.data[2] = 0x02;
      stat_cmd.data[3] = 0x4D;
      stat_cmd.data[4] = 0x00;
      stat_cmd.data[5] = 0x52;
      //This statement is added to resolve the warning: [-unused-but-set-variable],so these statements are added inside the macro.
#ifdef DBG_PROP_PROTOCOL_HOST_FLOW_CTRL_TEST
      status = rsi_prop_protocol_send_cmd(&stat_cmd.data[0], 0);
      if (status != 0) {
        prop_protocol_protocol_recv_buf_full = 1;
      }
#else
      rsi_prop_protocol_send_cmd(&stat_cmd.data[0], 0);
#endif
#ifdef FRDM_K28F
      // Trigger timer again
      LPTMR_SetTimerPeriod(LPTMR1, STAT_REQUEST_PERIOD);
      LPTMR_StartTimer(LPTMR1);
#elif defined(MXRT_595s)
      if (MSEC_TO_COUNT(STAT_REQUEST_PERIOD, mrt_clock) > MRT_CHANNEL_INTVAL_IVALUE_MASK) {
        mrtDividerValue1 = 0;
        mrtCountValue1   = 0;
        mrtEnableCount1  = true;
        while (MSEC_TO_COUNT((STAT_REQUEST_PERIOD >> (++mrtDividerValue1)), mrt_clock) > MRT_CHANNEL_INTVAL_IVALUE_MASK)
          ;
        MRT_StartTimer(MRT0, kMRT_Channel_1, MSEC_TO_COUNT((STAT_REQUEST_PERIOD >> mrtDividerValue1), mrt_clock));
      } else {
        MRT_StartTimer(MRT0, kMRT_Channel_1, MSEC_TO_COUNT(STAT_REQUEST_PERIOD, mrt_clock));
      }
#endif
    }
  }
#endif // GET_PROP_PROTOCOL_STATS

  if (bPROP_PROTOCOLTick) {
    bPROP_PROTOCOLTick = 0;
    switch (next_state) {
      case START_PROP_PROTOCOL: {
#ifdef GET_PROP_PROTOCOL_STATS
        for (uint8_t i = 0; i < MAX_CHANNELS; i++) {
          prop_protocol_blk[i]  = 0;
          prop_protocol_col[i]  = 0;
          prop_protocol_succ[i] = 0;
        }
#endif // GET_PROP_PROTOCOL_STATS
        LOG_PRINT("Send PROP_PROTOCOL START CMD\n");
        rsi_prop_protocol_stack_cmd_t start_cmd;
        start_cmd.data[0] = 0x0C;
        start_cmd.data[1] = 0x03;
        start_cmd.data[2] = 0x01;
        start_cmd.data[3] = 0x4B;
        start_cmd.data[4] = 0x00;
        rsi_prop_protocol_send_cmd(&start_cmd.data[0], 0);
        prop_protocol_stopped = false;
#ifdef GET_PROP_PROTOCOL_STATS
#ifdef FRDM_K28F
        // Trigger timer again
        LPTMR_SetTimerPeriod(LPTMR1, STAT_REQUEST_PERIOD);
        LPTMR_StartTimer(LPTMR1);
#elif defined(MXRT_595s)
        if (MSEC_TO_COUNT(STAT_REQUEST_PERIOD, mrt_clock) > MRT_CHANNEL_INTVAL_IVALUE_MASK) {
          mrtDividerValue1 = 0;
          mrtCountValue1   = 0;
          mrtEnableCount1  = true;
          while (MSEC_TO_COUNT((STAT_REQUEST_PERIOD >> (++mrtDividerValue1)), mrt_clock)
                 > MRT_CHANNEL_INTVAL_IVALUE_MASK)
            ;
          MRT_StartTimer(MRT0, kMRT_Channel_1, MSEC_TO_COUNT((STAT_REQUEST_PERIOD >> mrtDividerValue1), mrt_clock));
        } else {
          MRT_StartTimer(MRT0, kMRT_Channel_1, MSEC_TO_COUNT(STAT_REQUEST_PERIOD, mrt_clock));
        }
#endif
#endif
#ifdef TRIGGER_PROP_PROTOCOL_STOP
#ifdef FRDM_K28F
        LPTMR_SetTimerPeriod(LPTMR0, START_PROP_PROTOCOL_DELAY);
        LPTMR_StartTimer(LPTMR0);
#elif defined(MXRT_595s)
        if (MSEC_TO_COUNT(STOP_PROP_PROTOCOL_DELAY, mrt_clock) > MRT_CHANNEL_INTVAL_IVALUE_MASK) {
          mrtDividerValue0 = 0;
          mrtCountValue0   = 0;
          mrtEnableCount0  = true;
          while (MSEC_TO_COUNT((STOP_PROP_PROTOCOL_DELAY >> (++mrtDividerValue0)), mrt_clock)
                 > MRT_CHANNEL_INTVAL_IVALUE_MASK)
            ;
          MRT_StartTimer(MRT0,
                         kMRT_Channel_0,
                         MSEC_TO_COUNT((STOP_PROP_PROTOCOL_DELAY >> mrtDividerValue0), mrt_clock));
        } else {
          MRT_StartTimer(MRT0, kMRT_Channel_0, MSEC_TO_COUNT(STOP_PROP_PROTOCOL_DELAY, mrt_clock));
        }
#endif
        next_state = STOP_PROP_PROTOCOL;
#elif defined(CYCLE_PROP_PROTOCOL_START_STOP)
#ifdef FRDM_K28F
        LPTMR_SetTimerPeriod(LPTMR0, CYCLE_PROP_PROTOCOL_PERIOD);
        LPTMR_StartTimer(LPTMR0);
#elif defined(MXRT_595s)
        if (MSEC_TO_COUNT(CYCLE_PROP_PROTOCOL_PERIOD, mrt_clock) > MRT_CHANNEL_INTVAL_IVALUE_MASK) {
          mrtDividerValue0 = 0;
          mrtCountValue0   = 0;
          mrtEnableCount0  = true;
          while (MSEC_TO_COUNT((CYCLE_PROP_PROTOCOL_PERIOD >> (++mrtDividerValue0)), mrt_clock)
                 > MRT_CHANNEL_INTVAL_IVALUE_MASK)
            ;
          MRT_StartTimer(MRT0,
                         kMRT_Channel_0,
                         MSEC_TO_COUNT((CYCLE_PROP_PROTOCOL_PERIOD >> mrtDividerValue0), mrt_clock));
        } else {
          MRT_StartTimer(MRT0, kMRT_Channel_0, MSEC_TO_COUNT(CYCLE_PROP_PROTOCOL_PERIOD, mrt_clock));
        }
#endif
        next_state = STOP_PROP_PROTOCOL;
#endif
      } break;
      case STOP_PROP_PROTOCOL: {
        LOG_PRINT("Send PROP_PROTOCOL STOP CMD\n");
        rsi_prop_protocol_stack_cmd_t stop_cmd;
        stop_cmd.data[0] = 0x0C;
        stop_cmd.data[1] = 0x03;
        stop_cmd.data[2] = 0x01;
        stop_cmd.data[3] = 0x4C;
        stop_cmd.data[4] = 0x00;
        rsi_prop_protocol_send_cmd(&stop_cmd.data[0], 0);
#ifdef CYCLE_PROP_PROTOCOL_START_STOP
#ifdef FRDM_K28F
        LPTMR_SetTimerPeriod(LPTMR0, CYCLE_PROP_PROTOCOL_PERIOD);
        LPTMR_StartTimer(LPTMR0);
#elif defined(MXRT_595s)
        if (MSEC_TO_COUNT(CYCLE_PROP_PROTOCOL_PERIOD, mrt_clock) > MRT_CHANNEL_INTVAL_IVALUE_MASK) {
          mrtDividerValue0 = 0;
          mrtCountValue0   = 0;
          mrtEnableCount0  = true;
          while (MSEC_TO_COUNT((CYCLE_PROP_PROTOCOL_PERIOD >> (++mrtDividerValue0)), mrt_clock)
                 > MRT_CHANNEL_INTVAL_IVALUE_MASK)
            ;
          MRT_StartTimer(MRT0,
                         kMRT_Channel_0,
                         MSEC_TO_COUNT((CYCLE_PROP_PROTOCOL_PERIOD >> mrtDividerValue0), mrt_clock));
        } else {
          MRT_StartTimer(MRT0, kMRT_Channel_0, MSEC_TO_COUNT(CYCLE_PROP_PROTOCOL_PERIOD, mrt_clock));
        }
#endif
        next_state = START_PROP_PROTOCOL;
#else
        next_state = IDLE_PROP_PROTOCOL;
#endif // CYCLE_PROP_PROTOCOL_START_STOP
        prop_protocol_stopped = true;
        // Reset buffer, don't send any outstanding messages:
        outgoing_buffer_rd = outgoing_buffer_wr;
      } break;
      default:
        break;
    }
  }

  event_id = rsi_prop_protocol_app_get_event();

  switch (event_id) {
    case PROP_PROTOCOL_APP_EVT_SEND_NEW_BUFFER:

      if (!prop_protocol_stopped) {
        rsi_prop_protocol_app_clear_event(PROP_PROTOCOL_APP_EVT_SEND_NEW_BUFFER);
        rsi_prop_protocol_send_outgoing_msg();
      }
      break;
    default:
      break;
  }
  //GPIOC->PCOR |= (1UL << 6);
  return;
}

/*==============================================*/
/**
 * @fn         rsi_prop_protocol_app_task
 * @brief      This function will get invoked once the PROP_PROTOCOL OS thread created.
 * @param[in]  none.
 * @return     error status.
 * @section description
 */
void rsi_prop_protocol_app_task(void)
{
  uint32_t status = RSI_SUCCESS;
  rsi_prop_protocol_app_init_events();

  //! PROP_PROTOCOL register callbacks
  rsi_prop_protocol_register_callbacks(rsi_prop_protocol_async_resp_handler, rsi_prop_protocol_data_request_callback);

#ifdef FRDM_K28F
  lptmr_config_t lptmrConfig;
  LPTMR_GetDefaultConfig(&lptmrConfig);
  LPTMR_Init(LPTMR0, &lptmrConfig);
  bPROP_PROTOCOLTick = 0;

#ifdef GET_PROP_PROTOCOL_STATS
  lptmr_config_t lptmrConfig_stats;
  LPTMR_GetDefaultConfig(&lptmrConfig_stats);
  LPTMR_Init(LPTMR1, &lptmrConfig_stats);
  LPTMR_SetTimerPeriod(LPTMR1, STAT_REQUEST_PERIOD);
  bStatsTick = 0;
  LPTMR_EnableInterrupts(LPTMR1, kLPTMR_TimerInterruptEnable);
#endif // GET_PROP_PROTOCOL_STATS
  LPTMR_EnableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);
  EnableIRQ(LPTMR0_LPTMR1_IRQn);
#elif defined(MXRT_595s)
  mrt_config_t mrtConfig; //multirate timer
  mrt_clock = CLOCK_GetFreq(kCLOCK_BusClk);

  MRT_GetDefaultConfig(&mrtConfig);
  MRT_Init(MRT0, &mrtConfig);
  bPROP_PROTOCOLTick = 0;
  MRT_SetupChannelMode(MRT0, kMRT_Channel_0, kMRT_RepeatMode);
  MRT_EnableInterrupts(MRT0, kMRT_Channel_0, kMRT_TimerInterruptEnable);
  bStatsTick = 0;
  MRT_SetupChannelMode(MRT0, kMRT_Channel_1, kMRT_RepeatMode);
  MRT_EnableInterrupts(MRT0, kMRT_Channel_1, kMRT_TimerInterruptEnable);
  EnableIRQ(MRT0_IRQn);
#endif

#if 0 // Check FW version. If this is done in powersave, ensure WLAN enter powersave
	uint8_t fw[20];
	rsi_wlan_get(RSI_FW_VERSION,fw,sizeof(fw));
#endif

#if TEST_WLAN_INIT_CURRENT // To test WLAN-related current draw
  rsi_wlan_radio_int();
#endif // TEST_WLAN_INIT_CURRENT
#if WLAN_SYNC_REQ
  //! FIXME: Workaround
  if (rsi_wlan_running) {
    LOG_PRINT("Waiting PROP_PROTOCOL Thread,Wlan to unblock\n");
    rsi_semaphore_wait(&sync_coex_prop_protocol_sem, 0);
  }
#endif
  // Initialize PROP_PROTOCOL (won't start PROP_PROTOCOL channels, PROP_PROTOCOL will enter deep sleep after this command)
  rsi_prop_protocol_stack_cmd_t startup_cmd;
  startup_cmd.data[0] = 0x0C;
  startup_cmd.data[1] = 0x03;
  startup_cmd.data[2] = 0x01;
  startup_cmd.data[3] = 0x4A;
  startup_cmd.data[4] = 0x00;
  rsi_prop_protocol_send_cmd(&startup_cmd.data[0], 0);

  LOG_PRINT("Initialize PROP_PROTOCOL...\n");

  next_state = START_PROP_PROTOCOL;
#ifdef FRDM_K28F
  LPTMR_SetTimerPeriod(LPTMR0, START_PROP_PROTOCOL_DELAY);
  LPTMR_StartTimer(LPTMR0);
#elif defined(MXRT_595s)
  if (MSEC_TO_COUNT(START_PROP_PROTOCOL_DELAY, mrt_clock) > MRT_CHANNEL_INTVAL_IVALUE_MASK) {
    mrtDividerValue0 = 0;
    mrtCountValue0 = 0;
    mrtEnableCount0 = true;
    while (MSEC_TO_COUNT((START_PROP_PROTOCOL_DELAY >> (++mrtDividerValue0)), mrt_clock)
           > MRT_CHANNEL_INTVAL_IVALUE_MASK)
      ;
    MRT_StartTimer(MRT0, kMRT_Channel_0, MSEC_TO_COUNT((START_PROP_PROTOCOL_DELAY >> mrtDividerValue0), mrt_clock));
  } else {
    MRT_StartTimer(MRT0, kMRT_Channel_0, MSEC_TO_COUNT(START_PROP_PROTOCOL_DELAY, mrt_clock));
  }
#endif

#if ENABLE_POWER_SAVE
  rsi_mutex_lock(&power_cmd_mutex);
  if (!powersave_cmd_given) {
    status = rsi_initiate_power_save();
    if (status != RSI_SUCCESS) {
      LOG_PRINT("failed to keep module in power save \r\n");
      return;
    }
    powersave_cmd_given = true;
  }
  rsi_mutex_unlock(&power_cmd_mutex);
#endif // ENABLE_POWER_SAVE

  //! start bt inquiry after prop_protocol activities
  if (rsi_bt_running) {
    if (rsi_ble_running) {
      //! start ble scan after prop_protocol start
      rsi_semaphore_post(&ble_scan_sem);
    } else {
      //! start bt inquiry after prop_protocol start
      rsi_semaphore_post(&bt_inquiry_sem);
    }
  }
  while (1) {
    rsi_prop_protocol_task();
  }
}
#endif
