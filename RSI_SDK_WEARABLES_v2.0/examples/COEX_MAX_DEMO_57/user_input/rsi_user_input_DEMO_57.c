/*******************************************************************************
* @file  rsi_user_input_DEMO_57.c
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
 * @file         rsi_user_input_DEMO_57.h
 * @version      0.1
 * @date         01 Feb 2020 *
 *
 *
 *  @brief : This file stores the serial data in temporary buffer, parses the buffer and saves in local buffer 'rsi_parsed_conf'
 *
 *  @section Description  This file stores the serial data in temporary buffer, parses the buffer and saves in local buffer 'rsi_parsed_conf'
 */
/*=======================================================================*/
//   ! INCLUDES
/*=======================================================================*/
#include <rsi_common_app.h>
#if (COEX_MAX_APP && RUN_TIME_CONFIG_ENABLE)
#include <stdio.h>
#include <string.h>
#include <rsi_driver.h>
#include "rsi_ble_apis.h"
#include "rsi_ble_device_info_DEMO_57.h"
#include "rsi_ble_config_DEMO_57.h"
#include "rsi_os.h"
#include "rsi_user_input_DEMO_57.h"
#include "rsi_common_app_DEMO_57.h"

/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/

/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/
uint8_t Test_Case                 = 0;
rsi_parsed_conf_t rsi_parsed_conf = { 0 };

/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/
extern rsi_semaphore_handle_t ui_task_sem;
extern rsi_task_handle_t ui_task_handle;
extern uint8_t demoRingBuffer[];
extern rsi_semaphore_handle_t common_task_sem;

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
 * @fn         rsi_parse_conn_conf
 * @brief      This function parses complete line in ble configuration and  copies in buffer 'rsi_parsed_conf'
 * @param[in]		recv_buffer - temporary buffer which extracted from uart handle
 * 					parse_line - line to parse
 * 					temp_len - lenght of line
 * @param[out] none
 * @return     none.
 * @section description
 * This function parses complete line in ble configuration and copies in buffer 'rsi_parsed_conf'
 */
bool rsi_parse_conn_conf(uint8_t *recv_buffer, uint8_t line_no, uint8_t temp_len)
{
  uint8_t i           = 0;
  uint8_t field_cnt   = 0;
  uint8_t cnt         = 0;
  bool status         = true;
  uint8_t temp_var[4] = { '\0' };
  uint8_t conn_id     = 0xff;
  i                   = temp_len;
  switch (line_no) {
    case LINE5: {
      conn_id = 0;
    } break;
    case LINE6: {
      conn_id = 1;
    } break;
    case LINE7: {
      conn_id = 2;
    } break;
    case LINE8: {
      conn_id = RSI_BLE_MAX_NBR_SLAVES;
    } break;
    case LINE9: {
      conn_id = RSI_BLE_MAX_NBR_SLAVES + 1;
    } break;
  }

  while (recv_buffer[i] != '\r') {
    if (recv_buffer[i] != ' ') {
      //! search and fill the fields in local buffer
      switch (field_cnt) {
        //! copy configurations in local structure
        case 0:
          rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].smp_enable = (recv_buffer[i] - 48);
          break;
        case 1:
          rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].add_to_whitelist = (recv_buffer[i] - 48);
          break;
        case 2:
          rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].profile_discovery = (recv_buffer[i] - 48);
          break;
        case 3:
          rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].data_transfer = (recv_buffer[i] - 48);
          break;
        case 4:
          rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].rx_notifications = (recv_buffer[i] - 48);
          break;
        case 5:
          rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].rx_indications = (recv_buffer[i] - 48);
          break;
        case 6:
          rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].tx_notifications = (recv_buffer[i] - 48);
          break;
        case 7:
          rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].tx_write = (recv_buffer[i] - 48);
          break;
        case 8:
          rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].tx_write_no_response = (recv_buffer[i] - 48);
          break;
        case 9:
          rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].tx_indications = (recv_buffer[i] - 48);
          break;
        case 10:
          rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].buff_mode_sel.buffer_mode = (recv_buffer[i] - 48);
          break;
        case 11: {
          //! read untill ' '(space) character received
          while (recv_buffer[i] != ' ') {
            cnt++;
            i++;
          }
          if (cnt == 1) {
            rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].buff_mode_sel.buffer_cnt =
              (recv_buffer[i - cnt] - 48);
          } else {
            //! copy the command
            memcpy(&temp_var, &recv_buffer[i - cnt], cnt);
            rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].buff_mode_sel.buffer_cnt =
              rsi_atoi((int8_t *)temp_var);
            memset(&temp_var, '\0', strlen((char *)temp_var));
          }
          cnt = 0;
        } break;
        case 12: {
          //! read untill ' '(space) character received
          while (recv_buffer[i] != ' ') {
            cnt++;
            i++;
          }
          if (cnt == 1) {
            rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].buff_mode_sel.max_data_length =
              (recv_buffer[i - cnt] - 48);
          } else {
            //! copy the command
            memcpy(&temp_var, &recv_buffer[i - cnt], cnt);
            rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].buff_mode_sel.max_data_length =
              rsi_atoi((int8_t *)temp_var);
            memset(&temp_var, '\0', strlen((char *)temp_var));
          }
          cnt = 0;
        } break;
        case 13: {
          //! read untill ' '(space) character received
          while (recv_buffer[i] != ' ') {
            cnt++;
            i++;
          }
          if (cnt == 1) {
            rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].conn_param_update.conn_int =
              (recv_buffer[i - cnt] - 48);
          } else {
            //! copy the command
            memcpy(&temp_var, &recv_buffer[i - cnt], cnt);
            rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].conn_param_update.conn_int =
              rsi_atoi((int8_t *)temp_var);
            memset(&temp_var, '\0', strlen((char *)temp_var));
          }
          cnt = 0;
        } break;
        case 14: {
          //! read untill ' '(space) character received
          while (recv_buffer[i] != ' ') {
            cnt++;
            i++;
          }
          if (cnt == 1) {
            rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].conn_param_update.conn_latncy =
              (recv_buffer[i - cnt] - 48);
          } else {
            memcpy(&temp_var, &recv_buffer[i - cnt], cnt);
            rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].conn_param_update.conn_latncy =
              rsi_atoi((int8_t *)temp_var);
            memset(&temp_var, '\0', strlen((char *)temp_var));
          }
          cnt = 0;
        } break;
        case 15: {
          //! read untill ' '(space) character received
          while (recv_buffer[i] != ' ') {
            if (recv_buffer[i] == '>') {
              break;
            }
            cnt++;
            i++;
          }
          if (cnt == 1) {
            rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].conn_param_update.supervision_to =
              (recv_buffer[i - cnt] - 48);
          } else {
            memcpy(&temp_var, &recv_buffer[i - cnt], cnt);
            rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].conn_param_update.supervision_to =
              rsi_atoi((int8_t *)temp_var);
            memset(&temp_var, '\0', strlen((char *)temp_var));
          }
          cnt = 0;
        } break;
#if 0
			case 16:
				rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].rx_notif_client_service_uuid = (recv_buffer[i] - 48);
				break;
			case 17:
				rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].rx_notif_client_char_uuid = (recv_buffer[i] - 48);
				break;
			case 18:
				rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].tx_write_clientservice_uuid = (recv_buffer[i] - 48);
				break;
			case 19:
				rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].tx_write_client_char_uuid = (recv_buffer[i] - 48);
				break;
			case 20:
				rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].tx_wnr_client_service_uuid = (recv_buffer[i] - 48);
				break;
			case 21:
				rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].tx_wnr_client_char_uuid = (recv_buffer[i] - 48);
				break;
			case 22:
				rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].rx_indi_client_service_uuid = (recv_buffer[i] - 48);
				break;
			case 23:
				rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[conn_id].rx_indi_client_char_uuid = (recv_buffer[i] - 48);
				break;
#endif
      }
      field_cnt++;
    }
    i++;
  }
  return status;
}

/*==============================================*/
/**
 * @fn         rsi_parse_input
 * @brief      This function parses each line in user input and copies in buffer 'rsi_parsed_conf'
 * @param[in]		recv_buffer - temporary buffer which extracted from uart handle
 * 					parse_line - line to parse
 * @param[out] none
 * @return     none.
 * @section description
 * This function parses each line in user input and copies in buffer 'rsi_parsed_conf'
 */
bool rsi_parse_input(uint8_t *recv_buffer, uint8_t parse_line)
{
  bool status               = true;
  uint8_t temp_len          = 0;
  uint8_t field_cnt         = 0;
  uint16_t i                = 0;
  uint8_t search_string[50] = { '\0' };

  switch (parse_line) {
    case LINE1: {
      //! parse line1
      temp_len = strlen(PROTOCOL_SEL);
      strncpy((char *)&search_string, (char *)&recv_buffer[i], temp_len);
      if (!strcmp(PROTOCOL_SEL, (char *)search_string)) {
        i = temp_len;
        while (recv_buffer[i] != '>') {
          if (recv_buffer[i] != ' ') {
            //! search and fill the fields in local buffer
            switch (field_cnt) {
              case 0:
                //! copy configurations in local structure
                rsi_parsed_conf.rsi_protocol_sel.is_ble_enabled = (recv_buffer[i] - 48);
                break;
              case 1:
                rsi_parsed_conf.rsi_protocol_sel.is_bt_enabled = (recv_buffer[i] - 48);
                break;
              case 2:
                rsi_parsed_conf.rsi_protocol_sel.is_prop_protocol_enabled = (recv_buffer[i] - 48);
                break;
              case 3:
                rsi_parsed_conf.rsi_protocol_sel.is_wifi_enabled = (recv_buffer[i] - 48);
                break;
            }
            field_cnt++;
          }
          i++;
        }
      } else {
        LOG_PRINT("\r\n wrong configuration provided in line %d \r\n", parse_line);
        return false;
      }
    } break;
#if 0
	case LINE2:
	{
		uint8_t total_conn =0;
		//! parse line2
		temp_len = strlen(BLE_CONF);
		strncpy((uint8_t *)&search_string, &recv_buffer[i], temp_len);
		if(!strcmp(BLE_CONF, search_string))
		{
			i = temp_len;
			while(recv_buffer[i] != '\r')
			{
				if(recv_buffer[i] != ' ')
				{
					//! search and fill the fields in local buffer
					switch(field_cnt)
					{
					case 0:
						//! copy configurations in local structure
						rsi_parsed_conf.rsi_ble_config.no_of_slaves = (recv_buffer[i] - 48);
						break;
					case 1:
						rsi_parsed_conf.rsi_ble_config.no_of_masters = (recv_buffer[i] - 48);
						break;
					case 2:
						rsi_parsed_conf.rsi_ble_config.conn_by_name = (recv_buffer[i] - 48);
						break;
					}
					field_cnt++;
				}
				i++;
			}
		}
		else
		{
			LOG_PRINT("\r\n wrong configuration provided in line %d \r\n",parse_line);
			return false;
		}

		//! allocate memory
		total_conn =rsi_parsed_conf.rsi_ble_config.no_of_slaves + rsi_parsed_conf.rsi_ble_config.no_of_masters;
		//rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config  = (rsi_ble_conn_config_t *)malloc(sizeof(uint8_t) * total_conn );
	}
	break;
	case LINE3:
	{
		//! TO DO
	}
	break;
	case LINE4:
	{

		//! TO DO
	}
	break;
#endif
    case LINE2: {
      //! parse line5 if no. of slaves > =1
      if (RSI_BLE_MAX_NBR_SLAVES >= 1) {
        //! parse line5
        temp_len = strlen(SLAVE1_CONF);
        strncpy((char *)&search_string, (char *)&recv_buffer[i], temp_len);
        if (!strcmp(SLAVE1_CONF, (char *)search_string)) {
          status = rsi_parse_conn_conf(recv_buffer, LINE5, temp_len);
          if (status != true) {
            LOG_PRINT("\r\n error while parsing line %d \r\n", parse_line);
          }
        } else {
          LOG_PRINT("\r\n wrong configuration provided in line %d \r\n", parse_line);
          return false;
        }
      }
    } break;
    case LINE3: {
      //! parse line6 if no. of slaves > =2
      if (RSI_BLE_MAX_NBR_SLAVES >= 2) {
        //! parse line5
        temp_len = strlen(SLAVE2_CONF);
        strncpy((char *)&search_string, (char *)&recv_buffer[i], temp_len);
        if (!strcmp(SLAVE2_CONF, (char *)search_string)) {
          status = rsi_parse_conn_conf(recv_buffer, LINE6, temp_len);
          if (status != true) {
            LOG_PRINT("\r\n error while parsing line %d \r\n", parse_line);
          }
        } else {
          LOG_PRINT("\r\n wrong configuration provided in line %d \r\n", parse_line);
          return false;
        }
      }
    } break;
    case LINE4: {
      //! parse line5 if no. of slaves >=3
      if (RSI_BLE_MAX_NBR_SLAVES >= 3) {
        //! parse line5
        temp_len = strlen(SLAVE3_CONF);
        strncpy((char *)&search_string, (char *)&recv_buffer[i], temp_len);
        if (!strcmp(SLAVE3_CONF, (char *)search_string)) {
          status = rsi_parse_conn_conf(recv_buffer, LINE7, temp_len);
          if (status != true) {
            LOG_PRINT("\r\n error while parsing line %d \r\n", parse_line);
          }
        } else {
          LOG_PRINT("\r\n wrong configuration provided in line %d \r\n", parse_line);
          return false;
        }
      }
    } break;
    case LINE5: {
      //! parse line8 if no. of masters > 1
      if (RSI_BLE_MAX_NBR_MASTERS >= 1) {
        //! parse line5
        temp_len = strlen(MASTER1_CONF);
        strncpy((char *)&search_string, (char *)&recv_buffer[i], temp_len);
        if (!strcmp(MASTER1_CONF, (char *)search_string)) {
          status = rsi_parse_conn_conf(recv_buffer, LINE8, temp_len);
          if (status != true) {
            LOG_PRINT("\r\n error while parsing line %d \r\n", parse_line);
          }
        } else {
          LOG_PRINT("\r\n wrong configuration provided in line %d \r\n", parse_line);
          return false;
        }
      }
    } break;
    case LINE6: {
      //! parse line5 if no. of masters > 2
      if (RSI_BLE_MAX_NBR_MASTERS >= 2) {
        //! parse line5
        temp_len = strlen(MASTER2_CONF);
        strncpy((char *)&search_string, (char *)&recv_buffer[i], temp_len);
        if (!strcmp(MASTER2_CONF, (char *)search_string)) {
          status = rsi_parse_conn_conf(recv_buffer, LINE9, temp_len);
          if (status != true) {
            LOG_PRINT("\r\n error while parsing line %d \r\n", parse_line);
          }
        } else {
          LOG_PRINT("\r\n wrong configuration provided in line %d \r\n", parse_line);
          return false;
        }
      }
    } break;
    case LINE7: {
      uint8_t temp_cnt = 0;
      field_cnt        = 0;
      temp_len         = strlen(BT_CONFIG);
      //strncpy((char *)&search_string, (char *)&recv_buffer[i], temp_len);
      if (!strncmp(BT_CONFIG, (char *)recv_buffer, temp_len)) {
        i = temp_len;
        while (recv_buffer[i] != '>') {
          if (recv_buffer[i] != ' ') {
            //! search and fill the fields in local buffer
            switch (field_cnt) {
              case 0:
                //! copy bd address in local structure
                while (recv_buffer[i] != ' ') {
                  search_string[temp_cnt] = recv_buffer[i];
                  temp_cnt++;
                  i++;
                }
                rsi_parsed_conf.rsi_bt_config.rsi_bd_addr = malloc(sizeof(uint8_t) * RSI_REM_DEV_ADDR_LEN);
                memcpy(rsi_parsed_conf.rsi_bt_config.rsi_bd_addr, &search_string, RSI_REM_DEV_ADDR_LEN);
                strncpy(rsi_parsed_conf.rsi_bt_config.rsi_bd_addr, (char *)&search_string, temp_cnt);
                //rsi_parsed_conf.rsi_bt_config.rsi_bd_addr = (recv_buffer[i] -48);
                break;
              case 1:
                rsi_parsed_conf.rsi_bt_config.rsi_app_avdtp_role = (recv_buffer[i] - 48);
                break;
              case 2:
                rsi_parsed_conf.rsi_bt_config.rsi_bt_avdtp_stats_enable = (recv_buffer[i] - 48);
                break;
              case 3:
                rsi_parsed_conf.rsi_bt_config.rsi_ta_based_encoder = (recv_buffer[i] - 48);
                break;
              case 4:
                rsi_parsed_conf.rsi_bt_config.rsi_bt_inquiry_enable = (recv_buffer[i] - 48);
                break;
              case 5:
                rsi_parsed_conf.rsi_bt_config.rsi_inq_rem_name_req = (recv_buffer[i] - 48);
                break;
              case 6:
                rsi_parsed_conf.rsi_bt_config.rsi_inq_conn_simultaneous = (recv_buffer[i] - 48);
                break;
            }
            field_cnt++;
          }
          i++;
        }
      } else {
        LOG_PRINT("\r\n wrong configuration provided in line %d \r\n", parse_line);
        return false;
      }

    } break;
  }

  return status;
}

/*==============================================*/
/**
 * @fn         rsi_ui_app_task
 * @brief      This function extracts the input from uart buffer, parses it and stores in local buffer 'rsi_parsed_conf'
 * @param[out] none
 * @return     none.
 * @section description
 * This function extracts the input from uart buffer, parses it and stores in local buffer 'rsi_parsed_conf'
 */
void rsi_ui_app_task(void)
{
  uint8_t parse_line_cnt = 0;
  uint8_t temp           = 0;
  uint16_t i             = 0;
  bool status            = true;
  uint8_t buffer[DEMO_RING_BUFFER_SIZE];
  uint8_t buffer_line[100] = { '\0' };

  //! display logs to user
  LOG_PRINT("\r\n provide configuration script to run the application \r\n");
  while (1) {
    //! Wait for interrupt reception
    rsi_semaphore_wait(&ui_task_sem, 0);

    //! Extract input
    memset(buffer, 0, DEMO_RING_BUFFER_SIZE);
#ifdef FRDM_K28F
    memcpy(buffer, demoRingBuffer, DEMO_RING_BUFFER_SIZE);
    memset(demoRingBuffer, 0, DEMO_RING_BUFFER_SIZE);
#endif

    //! loop through the buffer untill received characters is '#'
    while (buffer[i] != '#') {
      temp = 0;
      memset(&buffer_line, '\0', rsi_strlen(buffer_line));

      //! copy the received buffer up to  character '>'
      while (buffer[i] != '>') {
        buffer_line[temp] = buffer[i];
        temp++;
        i++;
      }
      //! copy the last character '>' as well
      buffer_line[temp] = buffer[i];
      //! increment the line no.
      parse_line_cnt++;

      status = rsi_parse_input((uint8_t *)&buffer_line, parse_line_cnt);

      if (status != true) {
        LOG_PRINT("\r\n line %d parsing failed \r\n", parse_line_cnt);
        while (1)
          ;
      }
      //! loop untill end of present line
      //! check for carriage return
      while (buffer[i] != '\r') {
        //! check for line feed '\n'
        if (buffer[i] == '\n') {
          break;
        }
        i++;
      }
      //! go to next line
      i++;
      //! check for line feed '\n'
      if (buffer[i] == '\n') {
        //! point to next line first character
        i++;
      }
    }

    LOG_PRINT("\r\n parsing completed succesfully \r\n");
    //! delete the task and unblock the common semaphore
    rsi_semaphore_post(&common_task_sem);
    //!enable uart interrupt
#ifdef FRDM_K28F
    EnableIRQ(LPUART0_IRQn);
#endif
    rsi_task_destroy(ui_task_handle);
  }
}
#endif
