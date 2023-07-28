/*******************************************************************************
* @file  rsi_ble_master2_DEMO_54.c
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
 * @file    rsi_ble_master2_DEMO_54.c
 * @version 0.2
 * @date    26 Sep 2019
 *
 *
 *
 *  @brief : This file contains example application for BLE DUAL role.
 *
 *  @section Description  This application connects as a Central/Master with l2cap connection.
 *
 */

/*=======================================================================*/
//   ! INCLUDES
/*=======================================================================*/

#include <rsi_common_app.h>
#if (UNIFIED_PROTOCOL && RSI_ENABLE_BLE_TEST)
#include "rsi_driver.h"
#include "rsi_bt_common_apis.h"
#include <rsi_ble.h>
#include "rsi_ble_apis.h"
#include <rsi_wlan_non_rom.h>
#include "rsi_ble_device_info_DEMO_54.h"
#include "rsi_ble_config_DEMO_54.h"
#if RSI_BLE_MULTICONN_TEST

uint8_t master_2_event_tracker, master2_smp_done, master2_mtu_done;
#if SMP_ENABLE_M2
static uint8_t first_connect = 0;
#endif
static volatile uint64_t master2_state;
static rsi_ble_profile_list_by_conn_t rsi_ble_profile_list_by_conn[RSI_MAX_PROFILE_CNT] = { 0 };

extern volatile uint8_t num_of_conn_masters, num_of_conn_slaves;
#if CHECK_NOTIFICATIONS
extern volatile uint16_t rsi_ble_att1_val_hndl;
#endif
extern volatile uint32_t ble_app_event_task_map[];
extern volatile uint32_t ble_app_event_task_map1[];

extern uint32_t rsi_m2_discnt_reason;
extern rsi_ble_conn_info_t rsi_ble_conn_info[];
extern rsi_ble_req_adv_t change_adv_param;
extern rsi_semaphore_handle_t ble_main_task_sem, ble_master2_sem;
extern int32_t rsi_ble_start_advertising_with_values(void *rsi_ble_adv);
/*==============================================*/
/**
 * @fn         rsi_ble_master2_app_set_event
 * @brief      set the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to set/raise the specific event.
 */
void rsi_ble_master2_app_set_event(uint32_t event_num)
{
  if (event_num < 32) {
    ble_app_event_task_map[RSI_BLE_MAX_NBR_SLAVES + 1] |= BIT(event_num);
  } else {
    ble_app_event_task_map1[RSI_BLE_MAX_NBR_SLAVES + 1] |= BIT((event_num - 32));
  }
  rsi_semaphore_post(&ble_main_task_sem);
  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_master2_app_clear_event
 * @brief      clears the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to clear the specific event.
 */
static void rsi_ble_master2_app_clear_event(uint32_t event_num)
{
  if (event_num < 32) {
    ble_app_event_task_map[RSI_BLE_MAX_NBR_SLAVES + 1] &= ~BIT(event_num);
  } else {
    ble_app_event_task_map1[RSI_BLE_MAX_NBR_SLAVES + 1] &= ~BIT((event_num - 32));
  }
  return;
}

int32_t rsi_ble_master2_app_get_task_event(void)
{

  uint32_t ix;

  for (ix = 0; ix < 64; ix++) {
    if (ix < 32) {
      if (ble_app_event_task_map[RSI_BLE_MAX_NBR_SLAVES + 1] & (1 << ix)) {
        return ix;
      }
    } else {
      if (ble_app_event_task_map1[RSI_BLE_MAX_NBR_SLAVES + 1] & (1 << (ix - 32))) {
        return ix;
      }
    }
  }

  return (-1);
}

void rsi_ble_master2_app_task(void)
{
  bool done_profiles_query         = false;
  static bool prof_resp_recvd      = false;
  static bool char_resp_recvd      = false;
  static bool char_desc_resp_recvd = false;
#if CHECK_WRITE_PROPERTY
  bool write_handle_found = false;
  int16_t write_handle    = 0;
  uint16_t write_cnt      = 0;
#elif CHECK_WRITE_WITHOUT_RESP
  bool write_wwr_handle_found = false;
  int16_t write_wwr_handle    = 0;
  uint16_t wwr_count          = 0;
#elif CHECK_INDICATIONS
  bool indication_handle_found = false;
  uint16_t indication_handle   = 0;
  uint8_t indicate_data[2]     = { 2, 0 };
#elif CHECK_NOTIFICATIONS
  uint16_t notfy_cnt = 0;
#endif

  uint8_t no_of_profiles = 0, total_remote_profiles = 0;
  uint8_t l_num_of_services = 0, l_char_property = 0;
  uint8_t profs_evt_cnt = 0, prof_evt_cnt = 0, char_for_serv_cnt = 0, char_desc_cnt = 0;
  uint8_t i = 0, profile_index_for_char_query = 0, temp1 = 0, temp2 = 0;
  int32_t status                                   = 0;
  int32_t event_id                                 = 0;
  uint16_t profiles_endhandle                      = 0;
  uuid_t search_serv                               = { 0 };
  uint8_t rsi_connected_dev_addr[RSI_DEV_ADDR_LEN] = { 0 };
  uint16_t offset                                  = 0;
  uint16_t handle                                  = 0;
  uint8_t type                                     = 0;
  uint8_t read_data1[230]                          = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 72, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86,
    87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 72,
    74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99
  };
  rsi_bt_event_encryption_enabled_t l_rsi_encryption_enabled = { 0 };

  while (1) {
    //! checking for events list
    event_id = rsi_ble_master2_app_get_task_event();
    if (event_id == -1) {
      //! wait on events
      rsi_semaphore_wait(&ble_master2_sem, 0);
      continue;
    }

    //! Check if the event id is requested when not in connected state and clear it
    if (!((event_id == RSI_BLE_CONN_EVENT) || (event_id == RSI_BLE_ENHC_CONN_EVENT))) {
      if (rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].conn_status != 1) {
        //! Check if connection is done or not;
        rsi_ble_master2_app_clear_event(event_id);
      }
    }

    switch (event_id) {
      case RSI_BLE_CONN_EVENT: {
        //! event invokes when connection was completed
#if RSI_DEBUG_EN
        LOG_PRINT_D("\r\n In on conn evt - m2\r\n");
#endif
        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_CONN_EVENT);
        rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].conn_status = 1;

#if !CONNECTIVITY_TEST
        rsi_ble_master2_app_set_event(RSI_BLE_REQ_GATT_PROFILE);
#endif
        num_of_conn_masters++;

        LOG_PRINT("\r\n Number of master devices connected:%d -m2\n", num_of_conn_masters);

        memcpy(rsi_connected_dev_addr,
               rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].conn_event_to_app.dev_addr,
               RSI_DEV_ADDR_LEN);

        if (num_of_conn_masters < RSI_BLE_MAX_NBR_MASTERS) {
          status = rsi_ble_start_advertising();
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n advertising failed with status = 0x%x -m2 \n", status);
            //continue;
          }
        } else if (num_of_conn_masters == RSI_BLE_MAX_NBR_MASTERS) {
          status = rsi_ble_start_advertising_with_values(&change_adv_param);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n advertising with values failed with status = 0x%x -m2 \n", status);
            //continue ;
          }
        }
        LOG_PRINT("\n advertising device -m2\n");
#if 1
        if (RSI_BLE_MTU_EXCHANGE_FROM_HOST) {
          rsi_ascii_dev_address_to_6bytes_rev(rsi_connected_dev_addr,
                                              (int8_t *)rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES].remote_dev_addr);

          status = rsi_ble_mtu_exchange_event(rsi_connected_dev_addr, 240);
          if (status != 0) {
            LOG_PRINT("\r\n MTU Exchange request failed -m2\n");
          }
        }
#else
        master2_mtu_done = 1;
#endif
#if ENABLE_POWER_SAVE
        if ((num_of_conn_masters == RSI_BLE_MAX_NBR_MASTERS) && (num_of_conn_slaves == RSI_BLE_MAX_NBR_SLAVES)) {
          //! enable wlan radio
          status = rsi_wlan_radio_init();
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n radio init failed \n");
          }
          //! initiating power save in BLE mode
          status = rsi_bt_power_save_profile(PSP_MODE, PSP_TYPE);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n Failed in initiating power save -m2 \r\n");
            //continue;
          }
          //! initiating power save in wlan mode
          status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n Failed in initiating power save -m2 \r\n");
            //continue;
          }
        }
#endif
      } break;
      case RSI_BLE_ENHC_CONN_EVENT: {
        LOG_PRINT("\r\n In on_enhance_conn evt - m2\r\n");
        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_ENHC_CONN_EVENT);
        rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].conn_status = 1;
#if !CONNECTIVITY_TEST
        rsi_ble_master2_app_set_event(RSI_BLE_REQ_GATT_PROFILE);
#endif
        num_of_conn_masters++;

        LOG_PRINT("\r\n Number of master devices connected:%d -m2\r\n", num_of_conn_masters);

        memcpy(rsi_connected_dev_addr,
               rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_enhc_conn_status.dev_addr,
               RSI_DEV_ADDR_LEN);

        if (num_of_conn_masters < RSI_BLE_MAX_NBR_MASTERS) {
          status = rsi_ble_start_advertising();
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n advertising failed with status = 0x%x -m2 \n", status);
            //continue;
          }
        } else if (num_of_conn_masters == RSI_BLE_MAX_NBR_MASTERS) {
          status = rsi_ble_start_advertising_with_values(&change_adv_param);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n advertising with values failed with status = 0x%x -m2 \n", status);
            //continue ;
          }
        }
        LOG_PRINT("\n advertising device -m2\n");

#if 1
        if (RSI_BLE_MTU_EXCHANGE_FROM_HOST) {
          rsi_ascii_dev_address_to_6bytes_rev(rsi_connected_dev_addr,
                                              (int8_t *)rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].remote_dev_addr);
          status = rsi_ble_mtu_exchange_event(rsi_connected_dev_addr, 240);
          if (status != 0) {
            LOG_PRINT("\r\n MTU Exchange request failed -m2\n");
          }
        }
#else
        master2_mtu_done = 1;
#endif

#if ENABLE_POWER_SAVE
        if ((num_of_conn_masters == RSI_BLE_MAX_NBR_MASTERS) && (num_of_conn_slaves == RSI_BLE_MAX_NBR_SLAVES)) {
          //! enable wlan radio
          status = rsi_wlan_radio_init();
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n radio init failed \n");
          }
          //! initiating power save in BLE mode
          status = rsi_bt_power_save_profile(PSP_MODE, PSP_TYPE);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n Failed in initiating power save -m2 \r\n");
            //continue;
          }
          //! initiating power save in wlan mode
          status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n Failed in initiating power save -m2 \r\n");
            //continue;
          }
        }
#endif
      } break;
      case RSI_BLE_MTU_EXCHANGE_INFORMATION: {
        rsi_ble_master2_app_clear_event(RSI_BLE_MTU_EXCHANGE_INFORMATION);
        LOG_PRINT("\r\n MTU EXCHANGE INFORMATION - in subtask -m2 \r\n");
        if ((rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].mtu_exchange_info.initiated_role
             == PEER_DEVICE_INITATED_MTU_EXCHANGE)
            && (RSI_BLE_MTU_EXCHANGE_FROM_HOST)) {
          status = rsi_ble_mtu_exchange_resp(rsi_connected_dev_addr, LOCAL_MTU_SIZE);
          //! check for procedure already in progress error
          if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
            master2_state |= BIT64(RSI_BLE_MTU_EXCHANGE_INFORMATION);
            LOG_PRINT("\r\n rsi_ble_mtu_exchange_resp procedure is already in progress -m2 \r\n");
            break;
          }
          if (status != RSI_SUCCESS) {
            LOG_PRINT("MTU EXCHANGE RESP Failed status : 0x%x \n", status);
          } else {
            LOG_PRINT("MTU EXCHANGE RESP SUCCESS status : 0x%x \n", status);
          }
        }
      } break;
      case RSI_BLE_MORE_DATA_REQ_EVT: {

        rsi_ble_master2_app_clear_event(RSI_BLE_MORE_DATA_REQ_EVT);
#if RSI_DEBUG_EN
        LOG_PRINT_D("\r\n more data request -m2 \r\n");
#endif
        if (master2_state & BIT64(RSI_DATA_TRANSFER_EVENT)) {
          master2_state &= ~BIT64(RSI_DATA_TRANSFER_EVENT);
          rsi_ble_master2_app_set_event(RSI_DATA_TRANSFER_EVENT);
        }
        if (master2_state & BIT64(RSI_BLE_REQ_GATT_PROFILE)) {
          master2_state &= ~BIT64(RSI_BLE_REQ_GATT_PROFILE);
          rsi_ble_master2_app_set_event(RSI_BLE_REQ_GATT_PROFILE);
        }
        if (master2_state & BIT64(RSI_BLE_GATT_PROFILES)) {
          master2_state &= ~BIT64(RSI_BLE_GATT_PROFILES);
          rsi_ble_master2_app_set_event(RSI_BLE_GATT_PROFILES);
        }
        if (master2_state & BIT64(RSI_BLE_GATT_PROFILE)) {
          master2_state &= ~BIT64(RSI_BLE_GATT_PROFILE);
          rsi_ble_master2_app_set_event(RSI_BLE_GATT_PROFILE);
        }
        if (master2_state & BIT64(RSI_BLE_GATT_CHAR_SERVICES)) {
          master2_state &= ~BIT64(RSI_BLE_GATT_CHAR_SERVICES);
          rsi_ble_master2_app_set_event(RSI_BLE_GATT_CHAR_SERVICES);
        }
        if (master2_state & BIT64(RSI_BLE_READ_REQ_EVENT)) {
          master2_state &= ~BIT64(RSI_BLE_READ_REQ_EVENT);
          rsi_ble_master2_app_set_event(RSI_BLE_READ_REQ_EVENT);
        }
        if (master2_state & BIT64(RSI_BLE_BUFF_CONF_EVENT)) {
          master2_state &= ~BIT64(RSI_BLE_BUFF_CONF_EVENT);
          rsi_ble_master2_app_set_event(RSI_BLE_BUFF_CONF_EVENT);
        }
      } break;
      case RSI_BLE_REQ_GATT_PROFILE: {
        if (master2_mtu_done
#if SMP_ENABLE_M2
            && master2_smp_done
#endif
        ) {
          //
          rsi_ble_master2_app_clear_event(RSI_BLE_REQ_GATT_PROFILE);
          //! get remote device profiles
          LOG_PRINT("\r\n remote device profile discovery started -m2 \r\n");
          status = rsi_ble_get_profiles_async(rsi_connected_dev_addr, 1, 0xffff, NULL);
          if (status != RSI_SUCCESS) {
            //! check for procedure already in progress error
            if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
              master2_state |= BIT64(RSI_BLE_REQ_GATT_PROFILE);
              LOG_PRINT("\r\n rsi_ble_get_profiles_async procedure is already in progress -m2 \r\n");
              break;
            }
            //! check for buffer full error, which is not expected for this procedure
            else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
              LOG_PRINT("\r\n rsi_ble_get_profiles_async failed with buffer full error -m2 \r\n");
              break;

            } else {
              LOG_PRINT("\r\n get profile async call failed with error code :%x -m2 \r\n", status);
            }
          }
        }
      } break;
      case RSI_BLE_GATT_PROFILES: {
        //! prof_resp_recvd is set to false for every profile query response
#if RSI_DEBUG_EN
        LOG_PRINT_D("\n in gatt profiles -m2\n");
#endif
        if (!prof_resp_recvd) {
          //! check until completion of first level query
          if (!done_profiles_query) {
            rsi_ble_master2_app_clear_event(RSI_BLE_GATT_PROFILES);
            no_of_profiles = rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].get_allprofiles.number_of_profiles;
            //! copy the end of handle of last searched profile
            profiles_endhandle = *(uint16_t *)(rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1]
                                                 .get_allprofiles.profile_desc[no_of_profiles - 1]
                                                 .end_handle);

            //! copy retrieved profiles in local master buffer
            for (i = 0; i < no_of_profiles; i++) {
              memcpy(&rsi_ble_profile_list_by_conn[total_remote_profiles].profile_desc,
                     &rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].get_allprofiles.profile_desc[i],
                     sizeof(profile_descriptors_t));
              total_remote_profiles++;
              if (total_remote_profiles == RSI_MAX_PROFILE_CNT) {
                total_remote_profiles = RSI_MAX_PROFILE_CNT - 1;
                profiles_endhandle    = 0xffff;
                break;
              }
            }
            //! check for end of profiles
            if (profiles_endhandle != 0xffff) {
              status = rsi_ble_get_profiles_async(rsi_connected_dev_addr, profiles_endhandle + 1, 0xffff, NULL);
              if (status != RSI_SUCCESS) {
                //! check for procedure already in progress error
                if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
                  master2_state |= BIT64(RSI_BLE_GATT_PROFILES);
#if RSI_DEBUG_EN
                  LOG_PRINT_D("\r\n rsi_ble_get_profiles_async procedure is already in progress -m2 \r\n");
#endif
                  break;
                }
                //! check for buffer full error, which is not expected for this procedure
                else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
                  LOG_PRINT("\r\n rsi_ble_get_profiles_async failed with buffer full error -m2 \r\n");
                  break;
                } else {
                  LOG_PRINT("\r\n get profile async call failed with error code :%x -m2 \r\n", status);
                }
              }
            } else {
              //! first level profile query completed
              done_profiles_query = true;
              //set event to start second level profile query
              rsi_ble_master2_app_set_event(RSI_BLE_GATT_PROFILES);
            }
          } else {
            prof_resp_recvd = true;
            //! check until completion of second level profiles query
            if (profs_evt_cnt < total_remote_profiles) {
              //! search handles for all retrieved profiles
              search_serv.size = rsi_ble_profile_list_by_conn[profs_evt_cnt].profile_desc.profile_uuid.size;
              if (search_serv.size == 2) //! check for 16 bit(2 bytes) UUID value
              {
                search_serv.val.val16 = rsi_ble_profile_list_by_conn[profs_evt_cnt].profile_desc.profile_uuid.val.val16;
#if RSI_DEBUG_EN
                LOG_PRINT_D("\r\n search for profile :0x%x -m2 \r\n", search_serv.val.val16);
#endif
              } else if (search_serv.size == 4) {
                search_serv.val.val32 = rsi_ble_profile_list_by_conn[profs_evt_cnt].profile_desc.profile_uuid.val.val32;
#if RSI_DEBUG_EN
                LOG_PRINT_D("\r\n search for profile :0x%x -m2 \r\n", search_serv.val.val32);
#endif
              } else if (search_serv.size == 16) //! 128 bit(16 byte) UUID value
              {
                search_serv.val.val128 =
                  rsi_ble_profile_list_by_conn[profs_evt_cnt].profile_desc.profile_uuid.val.val128;
#if RSI_DEBUG_EN
                LOG_PRINT_D("\r\n search for profile :0x%x -m2 \r\n", search_serv.val.val128);
#endif
              }
              status = rsi_ble_get_profile_async(rsi_connected_dev_addr, search_serv, NULL);
              if (status != RSI_SUCCESS) {
                //! check for procedure already in progress error
                if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
                  prof_resp_recvd = false;
                  master2_state |= BIT64(RSI_BLE_GATT_PROFILES);
#if RSI_DEBUG_EN
                  LOG_PRINT_D("\r\n rsi_ble_get_profile_async procedure is already in progress -m2 \r\n");
#endif
                  break;
                }
                //! check for buffer full error, which is not expected for this procedure
                else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
                  LOG_PRINT("\r\n rsi_ble_get_profiles_async failed with buffer full error -m2 \r\n");
                  break;
                } else {
                  LOG_PRINT("\r\n get profile async call failed with error code :%x -m2 \r\n", status);
                }
              } else {
                profs_evt_cnt++;
              }
            } else {
              //! second level of profile query completed
              rsi_ble_master2_app_clear_event(RSI_BLE_GATT_PROFILES);
              rsi_ble_master2_app_set_event(RSI_BLE_GATT_PROFILE);
            }
          }
        }

      } break;
      case RSI_BLE_GATT_PROFILE: {
#if RSI_DEBUG_EN
        LOG_PRINT_D("\n in gatt profile -m2\n");
#endif
        if (prof_evt_cnt < total_remote_profiles) {
          //! clear the served event
          rsi_ble_master2_app_clear_event(RSI_BLE_GATT_PROFILE);
          //! copy to master buffer
          memcpy(&rsi_ble_profile_list_by_conn[prof_evt_cnt].profile_info_uuid,
                 &rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].get_profile,
                 sizeof(rsi_ble_event_profile_by_uuid_t));
          prof_resp_recvd = false;
#if RSI_DEBUG_EN
          LOG_PRINT_D("\r\n Gatt profile:\nStart handle: 0x%x  \nEnd handle:0x%x -m2 \r\n",
                      *(uint16_t *)rsi_ble_profile_list_by_conn[prof_evt_cnt].profile_info_uuid.start_handle,
                      *(uint16_t *)rsi_ble_profile_list_by_conn[prof_evt_cnt].profile_info_uuid.end_handle);
#endif
          prof_evt_cnt++;
        } else {
          if (!char_resp_recvd) {
            if (profile_index_for_char_query < total_remote_profiles) {
              char_resp_recvd = true;
#if RSI_DEBUG_EN
              LOG_PRINT_D(
                "\r\n search for profile characteristics :0x%x -m2 \r\n",
                rsi_ble_profile_list_by_conn[profile_index_for_char_query].profile_desc.profile_uuid.val.val16);
#endif
              //! Get characteristic services of searched profile
              status = rsi_ble_get_char_services_async(
                rsi_connected_dev_addr,
                *(uint16_t *)rsi_ble_profile_list_by_conn[profile_index_for_char_query].profile_info_uuid.start_handle,
                *(uint16_t *)rsi_ble_profile_list_by_conn[profile_index_for_char_query].profile_info_uuid.end_handle,
                NULL);
              if (status != RSI_SUCCESS) {
                //! check for procedure already in progress error
                if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
                  char_resp_recvd = false;
                  master2_state |= BIT64(RSI_BLE_GATT_PROFILE);
                  LOG_PRINT("\r\n rsi_ble_get_char_services_async procedure is already in progress -m2 \r\n");
                  break;
                }
                //! check for buffer full error, which is not expected for this procedure
                else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
                  LOG_PRINT("\r\n rsi_ble_get_char_services_async failed with buffer full error -m2 \r\n");
                  break;
                } else {
                  LOG_PRINT(
                    "\r\n failed to get service characteristics of the remote GATT server with error:0x%x -m2 \r\n",
                    status);
                }
              }
              profile_index_for_char_query++;
            } else {
              //! discovery of complete characteristics in each profile is completed
              rsi_ble_master2_app_clear_event(RSI_BLE_GATT_PROFILE);
              rsi_ble_master2_app_set_event(RSI_BLE_GATT_CHAR_SERVICES);
            }
          }
        }
      } break;
      case RSI_BLE_GATT_CHAR_SERVICES: {
#if RSI_DEBUG_EN
        LOG_PRINT_D("\r\n in gatt test:RSI_BLE_GATT_CHAR_SERVICES -m2 \r\n");
#endif
        if (char_for_serv_cnt < total_remote_profiles) {
          rsi_ble_master2_app_clear_event(RSI_BLE_GATT_CHAR_SERVICES);

          //! copy total no of characteristic services for each profile
          rsi_ble_profile_list_by_conn[char_for_serv_cnt].no_of_char_services =
            rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].get_char_services.num_of_services;
          if (rsi_ble_profile_list_by_conn[char_for_serv_cnt].no_of_char_services > RSI_BLE_MAX_RESP_LIST) {
            rsi_ble_profile_list_by_conn[char_for_serv_cnt].no_of_char_services = RSI_BLE_MAX_RESP_LIST;
          }
          for (uint8_t ix = 0; ix < rsi_ble_profile_list_by_conn[char_for_serv_cnt].no_of_char_services; ix++) {
            memcpy(&rsi_ble_profile_list_by_conn[char_for_serv_cnt].profile_char_info[ix],
                   &rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].get_char_services,
                   sizeof(rsi_ble_event_read_by_type1_t));
          }
          char_for_serv_cnt++;
          char_resp_recvd = false;
        } else {
          if (!char_desc_resp_recvd) {
            char_desc_resp_recvd = true;
            //! search for all characteristics descriptor in all profiles
            if (temp1 < total_remote_profiles) {
              l_num_of_services = rsi_ble_profile_list_by_conn[temp1].no_of_char_services;
              //! search for all characteristics descriptor in each profile
              if (temp2 < l_num_of_services) {
                l_char_property = rsi_ble_profile_list_by_conn[temp1]
                                    .profile_char_info[temp2]
                                    .char_services[temp2]
                                    .char_data.char_property;
                if ((l_char_property == RSI_BLE_ATT_PROPERTY_INDICATE)
                    || (l_char_property == RSI_BLE_ATT_PROPERTY_NOTIFY)) {
                  //LOG_PRINT("\n query for profile service1 %d -m2 \n",temp1);
                  status = rsi_ble_get_att_value_async(
                    rsi_connected_dev_addr,
                    rsi_ble_profile_list_by_conn[temp1].profile_char_info[temp2].char_services[temp2].handle,
                    NULL);

                  if (status != RSI_SUCCESS) {
                    //! check for procedure already in progress error
                    if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
                      char_desc_resp_recvd = false;
                      master2_state |= BIT64(RSI_BLE_GATT_CHAR_SERVICES);
#if RSI_DEBUG_EN
                      LOG_PRINT_D("\r\n rsi_ble_get_att_value_async procedure is already in progress -m2 \r\n");
#endif
                      break;
                    }
                    //! check for buffer full error, which is not expected for this procedure
                    else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
                      LOG_PRINT("\r\n rsi_ble_get_att_value_async failed with buffer full error -m2 \r\n");
                      break;
                    } else {
                      LOG_PRINT("\r\n failed to get characteristics descriptor of the remote GATT server with "
                                "error:0x%x -m2 \r\n",
                                status);
                    }
                  }
                } else {
                  temp2++;
                  char_desc_resp_recvd = false;
                }
              }
              //! completed characteristic descriptor discovery in required profile
              else if (temp2 == l_num_of_services) {
                temp2                = 0; //!  to start searching from starting of next profile
                char_desc_resp_recvd = false;
                temp1++; //! look for next profile, after completion of searching all characteristic descriptors in one profile
#if RSI_DEBUG_EN
                LOG_PRINT_D("\n query for profile service  %d \n -m2", temp1);
#endif
              }
            }
          }
          //! discovering completed for all profiles
          else if (temp1 == total_remote_profiles) {
            rsi_ble_master2_app_clear_event(RSI_BLE_GATT_CHAR_SERVICES);
            rsi_ble_master2_app_set_event(RSI_BLE_GATT_DESC_SERVICES);
          }
        }
      } break;

      case RSI_BLE_GATT_DESC_SERVICES: {

#if RSI_DEBUG_EN
        LOG_PRINT_D("\r\n in gatt test:RSI_BLE_GATT_DESC_SERVICES : temp1 %d -m2 \r\n", temp1);
#endif
        if (temp1 < total_remote_profiles) {
          temp2++;
          rsi_ble_master2_app_clear_event(RSI_BLE_GATT_DESC_SERVICES);
          char_desc_cnt++;
          char_desc_resp_recvd = false;
        } else {

          rsi_ble_master2_app_clear_event(RSI_BLE_GATT_DESC_SERVICES);
          LOG_PRINT("\n remote device profiles discovery completed -m2 \n");
#if CHECK_INDICATIONS
          //! check for Health Thermometer profile
          for (i = 0; i < total_remote_profiles; i++) {
            if (rsi_ble_profile_list_by_conn[i].profile_desc.profile_uuid.val.val16 == 0x1809) {
              for (uint8_t ix = 0; ix < rsi_ble_profile_list_by_conn[i].no_of_char_services; ix++) {
                //! check in Health Thermometer profile
                if ((!indication_handle_found)
                    && ((rsi_ble_profile_list_by_conn[i]
                           .profile_char_info[ix]
                           .char_services[ix]
                           .char_data.char_property)
                        & RSI_BLE_ATT_PROPERTY_INDICATE)) {
                  LOG_PRINT("\n indicate handle found -m2 \n");
                  indication_handle_found = true;
                  indication_handle =
                    rsi_ble_profile_list_by_conn[i].profile_char_info[ix].char_services[ix].char_data.char_handle;
                  //! configure the buffer configuration mode
                  rsi_ble_master2_app_set_event(RSI_BLE_BUFF_CONF_EVENT);
                  break;
                }
              }
            }
          }
          if (!indication_handle_found) {
            LOG_PRINT("\n Health Thermometer profile not found -m2 \n");
          }

#elif CHECK_WRITE_PROPERTY
          //! check for heart rate profile
          for (i = 0; i < total_remote_profiles; i++) {
            if (rsi_ble_profile_list_by_conn[i].profile_desc.profile_uuid.val.val16 == 0x180D) {
              for (uint8_t ix = 0; ix < rsi_ble_profile_list_by_conn[i].no_of_char_services; ix++) {
                //! check in Health Thermometer profile
                if ((!write_handle_found)
                    && ((rsi_ble_profile_list_by_conn[i]
                           .profile_char_info[ix]
                           .char_services[ix]
                           .char_data.char_property)
                        & RSI_BLE_ATT_PROPERTY_WRITE)) {
                  LOG_PRINT("\n write handle found -m2 \n");
                  write_handle_found = true;
                  write_handle =
                    rsi_ble_profile_list_by_conn[i].profile_char_info[ix].char_services[ix].char_data.char_handle;
                  //! configure the buffer configuration mode
                  rsi_ble_master2_app_set_event(RSI_BLE_BUFF_CONF_EVENT);
                  break;
                }
              }
            }
          }
          if (!write_handle_found) {
            LOG_PRINT("\n Heart Rate profile not found -m2 \n");
          }
#elif CHECK_WRITE_WITHOUT_RESP
          // check for Immediate Alert profile
          for (i = 0; i < total_remote_profiles; i++) {
            if (rsi_ble_profile_list_by_conn[i].profile_desc.profile_uuid.val.val16 == 0x1802) {
              for (uint8_t ix = 0; ix < rsi_ble_profile_list_by_conn[i].no_of_char_services; ix++) {
                if ((!write_wwr_handle_found)
                    && (rsi_ble_profile_list_by_conn[i].profile_char_info[ix].char_services[ix].char_data.char_property
                        & RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE)) {
#if RSI_DEBUG_EN
                  LOG_PRINT_D("\n write without response handle found -m2 \n");
#endif
                  write_wwr_handle_found = true; //! write handle found
                  write_wwr_handle =
                    rsi_ble_profile_list_by_conn[i].profile_char_info[ix].char_services[ix].char_data.char_handle;

                  rsi_ble_master2_app_set_event(RSI_BLE_BUFF_CONF_EVENT);
                  master_2_event_tracker |= BIT(2); //! Trigger Write without response
                  break;
                }
              }
            }
          }
          if (!write_wwr_handle_found) {
            LOG_PRINT("\n Immediate Alert profile not found -m2 \n");
          }
#endif
        }
      } break;
      case RSI_BLE_BUFF_CONF_EVENT: {
#if RSI_DEBUG_EN
        LOG_PRINT_D("\r\n in gatt RSI_BLE_BUFF_CONF_EVENT -m2 \r\n");
#endif
        status = rsi_ble_set_wo_resp_notify_buf_info(rsi_connected_dev_addr, DLE_BUFFER_MODE, DLE_BUFFER_COUNT);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n failed to set the buffer configuration mode, error:0x%x -m2 \r\n", status);
        } else {
          rsi_ble_master2_app_clear_event(RSI_BLE_BUFF_CONF_EVENT);
          rsi_ble_master2_app_set_event(RSI_CONN_UPDATE_REQ_EVENT);
        }

      } break;
      case RSI_CONN_UPDATE_REQ_EVENT: {
#if RSI_DEBUG_EN
        LOG_PRINT_D("\r\n in connection update req event -m2 \r\n");
#endif
        //! update connection interval to 300ms, latency 0, supervision timeout 4s
        status = rsi_ble_conn_params_update(rsi_connected_dev_addr, 240, 240, 0, 400);
        if (status != RSI_SUCCESS) {
          //! check for procedure already in progress error
          if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
            LOG_PRINT("\r\n rsi_ble_conn_params_update procedure is already in progress -m2 \r\n");
            break;
          } else {
            LOG_PRINT("\r\n failed to update connection paramaters error:0x%x -m2 \r\n", status);
          }
        } else {
          rsi_ble_master2_app_clear_event(RSI_CONN_UPDATE_REQ_EVENT);
          rsi_ble_master2_app_set_event(RSI_DATA_TRANSFER_EVENT);
          LOG_PRINT("\n connection params request was successfull -m2 \n");
        }
      } break;
      case RSI_BLE_RECEIVE_REMOTE_FEATURES: {

        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_RECEIVE_REMOTE_FEATURES);
#if SMP_ENABLE_M2
        if (master2_mtu_done) {
          //! initiating the SMP pairing process
          status = rsi_ble_smp_pair_request(rsi_connected_dev_addr, RSI_BLE_SMP_IO_CAPABILITY, MITM_ENABLE);

          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n start of SMP pairing process failed with error code %x -m2 \r\n", status);
          }
        }
#endif
      } break;
      case RSI_BLE_CONN_UPDATE_COMPLETE_EVENT: {

        rsi_ble_master2_app_clear_event(RSI_BLE_CONN_UPDATE_COMPLETE_EVENT);
        LOG_PRINT("\n conn updated device address : %s\n conn_interval:%d\n supervision timeout:%d -m2",
                  rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].remote_dev_addr,
                  rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].conn_update_resp.conn_interval,
                  rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].conn_update_resp.timeout);
#if UPDATE_CONN_PARAMETERS
        status = rsi_conn_update_request();
#endif
      } break;
      case RSI_BLE_DISCONN_EVENT: {

        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_DISCONN_EVENT);

        rsi_ble_master2_app_clear_event(RSI_DATA_TRANSFER_EVENT);

        ble_app_event_task_map[RSI_BLE_MAX_NBR_SLAVES + 1]  = 0;
        ble_app_event_task_map1[RSI_BLE_MAX_NBR_SLAVES + 1] = 0;
#if CHECK_INDICATIONS
        indication_handle       = 0;
        indication_handle_found = false;
#elif CHECK_WRITE_PROPERTY
        write_handle       = 0;
        write_handle_found = false;
        write_cnt          = 0;
#elif CHECK_WRITE_WITHOUT_RESP
        write_wwr_handle_found = false;
        write_wwr_handle       = 0;
        wwr_count              = 0;
#endif
        master2_state                = 0;
        master2_smp_done             = 0;
        master2_mtu_done             = 0;
        prof_resp_recvd              = false;
        done_profiles_query          = false;
        char_resp_recvd              = false;
        char_desc_resp_recvd         = false;
        profile_index_for_char_query = 0;
        prof_evt_cnt                 = 0;
        profs_evt_cnt                = 0;
        char_for_serv_cnt            = 0;
        master2_state                = 0;
        //! clear the profile data
        for (uint8_t i = 0; i < total_remote_profiles; i++) {
          memset(&rsi_ble_profile_list_by_conn[i], 0, sizeof(rsi_ble_profile_list_by_conn_t));
        }
        total_remote_profiles = 0;
        memset(rsi_connected_dev_addr, 0, RSI_DEV_ADDR_LEN);

        //! clear flag for notification sending
        master_2_event_tracker = 0; //! Clearing all bits

        LOG_PRINT("\r\n master2 is disconnected, reason : 0x%x -m2 \r\n", rsi_m2_discnt_reason);

        if (num_of_conn_masters) {
          num_of_conn_masters--;
        }

        LOG_PRINT("\n Device disconnected - m2\n ");
        LOG_PRINT("\n Number of connected master devices:%d\n", num_of_conn_masters);
        LOG_PRINT("\n Number of connected slave devices:%d\n", num_of_conn_slaves);
        memset(&rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1], 0, sizeof(rsi_ble_conn_info_t));

        if (num_of_conn_masters < RSI_BLE_MAX_NBR_MASTERS) {
          status = rsi_ble_stop_advertising();
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n advertising failed to stop = 0x%x -m2 \n", status);
            //continue;
          }
          LOG_PRINT("\r\n In dis-conn evt, Start Adv -m2 \r\n");
          //! Advertising interval set to ~211.25ms with full advertising data payload
          status = rsi_ble_start_advertising();
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n advertising failed to start = 0x%x -m2 \n", status);
            //continue;
          }
          LOG_PRINT("\n advertising started -m2 \n");
        }
      } break;
      case RSI_BLE_GATT_WRITE_EVENT: {
        //! event invokes when write/notification events received

        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_GATT_WRITE_EVENT);
#if CHECK_INDICATIONS
        //! Check whether received write is indication and acknowledge it
        if (*(uint16_t *)rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].app_ble_write_event.handle
            == (indication_handle)) {
#if RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
          //! Send indication acknowledgement to remote device
          status = rsi_ble_indicate_confirm(rsi_connected_dev_addr);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n indication confirm failed \t reason = %x -m2\n", status);
          } else {
            LOG_PRINT("\n indication confirm response sent -m2\n");
          }
#endif
        }
#elif CHECK_NOTIFICATIONS
        //check for valid notifications
        if ((rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].app_ble_write_event.att_value[0] == NOTIFY_ENABLE)
            && ((*(uint16_t *)(rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].app_ble_write_event.handle) - 1)
                == rsi_ble_att1_val_hndl)) {
          LOG_PRINT("\n Remote device enabled the notification -m2\n");
          //! Set flag for notification sending
          master_2_event_tracker |=
            BIT(0); //! BIT 0 for notifications enabled from remote device and sending notifications to it

          //! configure the buffer configuration mode
          rsi_ble_master2_app_set_event(RSI_BLE_BUFF_CONF_EVENT);
        } else if ((rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].app_ble_write_event.att_value[0] == NOTIFY_DISABLE)
                   && ((*(uint16_t *)(rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].app_ble_write_event.handle) - 1)
                       == rsi_ble_att1_val_hndl /*rsi_ble_att1_val_hndl*/)) {
          LOG_PRINT("\n Remote device disabled the notification -m2\n");
          //! clear flag for notification sending
          master_2_event_tracker &=
            ~BIT(0); //! BIT 0 for notifications disabled from remote device and stop sending notifications to it
          rsi_ble_master2_app_clear_event(RSI_DATA_TRANSFER_EVENT);
        }
#endif
      } break;

      case RSI_DATA_TRANSFER_EVENT: {

#if CHECK_INDICATIONS
        LOG_PRINT("\n in indication event -m2 \n");
        rsi_ble_master2_app_clear_event(RSI_DATA_TRANSFER_EVENT);
        status = rsi_ble_set_att_value_async(rsi_connected_dev_addr, //enable the indications
                                             indication_handle + 1,
                                             2,
                                             indicate_data);
        if (status != RSI_SUCCESS) {
          if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
            LOG_PRINT("\r\n indication failed with buffer error -m2 \r\n");
            master2_state |= BIT64(RSI_DATA_TRANSFER_EVENT);
            break;
          } else {
            LOG_PRINT("\r\n indication failed with status = %x -m2 \r\n", status);
          }
        }
#elif CHECK_WRITE_PROPERTY
        rsi_ble_master2_app_clear_event(RSI_DATA_TRANSFER_EVENT);
#if RSI_DEBUG_EN
        LOG_PRINT_D("\n in write with response event -m2 \n");
#endif
        read_data1[0] = write_cnt;
        status = rsi_ble_set_att_value_async(rsi_connected_dev_addr, write_handle, RSI_BLE_MAX_DATA_LEN, read_data1);
        if (status != RSI_SUCCESS) {
          //! check for procedure already in progress error
          if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
            master2_state |= BIT64(RSI_DATA_TRANSFER_EVENT);
#if RSI_DEBUG_EN
            LOG_PRINT_D("\r\n rsi_ble_set_att_value_async procedure is already in progress -m2 \r\n");
#endif
            break;
          } else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
            LOG_PRINT("\r\n write with response failed with buffer error -m2 \r\n");
            master2_state |= BIT64(RSI_DATA_TRANSFER_EVENT);
            break;
          } else {
            LOG_PRINT("\r\n write with response failed with status = %x -m2 \r\n", status);
          }
        } else {
          write_cnt++;
        }
#elif CHECK_WRITE_WITHOUT_RESP
        read_data1[0]          = wwr_count;

#if RSI_DEBUG_EN
        LOG_PRINT_D("\n in write without response event -m2 \n");
#endif
        status = rsi_ble_set_att_cmd(rsi_connected_dev_addr, write_wwr_handle, 1, (uint8_t *)read_data1);
        if (status != RSI_SUCCESS) {
          if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
#if RSI_DEBUG_EN
            LOG_PRINT_D("\r\n write without response failed with buffer error -m2 \r\n");
#endif
            master2_state |= BIT64(RSI_DATA_TRANSFER_EVENT);
            break;
          } else if (status == RSI_ERROR_IN_BUFFER_ALLOCATION) //! TO-DO, add proper error code
          {
            LOG_PRINT("\n cannot transmit %d bytes in small buffer configuration mode -m2\n", RSI_BLE_MAX_DATA_LEN);
            rsi_ble_disconnect(rsi_connected_dev_addr);
            break;
          } else {
            LOG_PRINT("\r\n write without response failed with status = 0x%x -m2 \r\n", status);
          }

        } else {
          wwr_count++;
          //! update connection interval after transmitting for every 256 times
#if RSI_PROFILE_QUERY_AGAIN
          if (wwr_count == 3) {
            LOG_PRINT("\n start remote device profiles discovery again -m2 \n");
            //! start profile query procedure for every 256 notifications
            write_wwr_handle_found = false; //! search for new handle
            rsi_ble_master2_app_set_event(RSI_BLE_REQ_GATT_PROFILE);
          }
#endif
        }
#elif CHECK_NOTIFICATIONS
        //! prepare the data to set as local attribute value.

        read_data1[0] = notfy_cnt;
        read_data1[1] = notfy_cnt >> 8;
#if RSI_DEBUG_EN
        LOG_PRINT_D("\n sending notify :%d\n", notfy_cnt);
#endif
        //! set the local attribute value.
        status = rsi_ble_notify_value(rsi_connected_dev_addr,
                                      rsi_ble_att1_val_hndl,
                                      RSI_BLE_MAX_DATA_LEN,
                                      (uint8_t *)read_data1);
        if (status != RSI_SUCCESS) {
          if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
#if RSI_DEBUG_EN
            LOG_PRINT_D("\r\n notify %d failed with buffer full error -m2 \r\n", notfy_cnt);
#endif
            rsi_ble_master2_app_clear_event(RSI_DATA_TRANSFER_EVENT);
            master2_state |= BIT64(RSI_DATA_TRANSFER_EVENT);
            break;
          } else if (status == RSI_ERROR_IN_BUFFER_ALLOCATION) //! TO-DO, add proper error code
          {
            LOG_PRINT("\n cannot transmit %d bytes in small buffer configuration mode -m2\n", RSI_BLE_MAX_DATA_LEN);
            rsi_ble_disconnect((int8_t *)rsi_connected_dev_addr);
            break;
          } else {
            LOG_PRINT("\n notify %d failed with error code %x  -m2\n", notfy_cnt, status);
          }
        } else {
#if RSI_DEBUG_EN
          LOG_PRINT_D("\r\n notify %d success -m2 \r\n", notfy_cnt);
#endif
          notfy_cnt++;
        }

#if RSI_PROFILE_QUERY_AGAIN
        if (notfy_cnt == 3) //! Updating connection parameters after sending 3 notifications
        {

          LOG_PRINT("\n start remote device profiles discovery again -m2 \n");
          //! start profile query procedure for every 256 notifications
          rsi_ble_master2_app_set_event(RSI_BLE_REQ_GATT_PROFILE);
        }
#endif
#endif
      } break;
      case RSI_BLE_WRITE_EVENT_RESP: {
        //! event invokes when write response received
        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_WRITE_EVENT_RESP);
#if CHECK_WRITE_PROPERTY
        rsi_ble_master2_app_set_event(RSI_DATA_TRANSFER_EVENT);
        LOG_PRINT("\n write response received -m2\n");
#endif
      } break;
      case RSI_BLE_READ_REQ_EVENT: {
        //! event invokes when write/notification events received

        LOG_PRINT("\n Read request initiated by remote device -m2 \n");
        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_READ_REQ_EVENT);

        type   = rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].app_ble_read_event.type;
        handle = rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].app_ble_read_event.handle;
        offset = rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].app_ble_read_event.offset;

        if (type == 1) {
          status = rsi_ble_gatt_read_response(rsi_connected_dev_addr,
                                              1,
                                              handle,
                                              offset,
                                              (sizeof(read_data1) - offset),
                                              &(read_data1[offset]));
          offset = 0;
        } else {

          status = rsi_ble_gatt_read_response(rsi_connected_dev_addr, 0, handle, 0, (sizeof(read_data1)), read_data1);
        }

        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n read response failed, error:0x%x -m2 \r\n", status);
        } else {
          LOG_PRINT("\n response to read request initiated by remote device was successfull -m2 \n");
        }

      } break;
      case RSI_BLE_MTU_EVENT: {
        //! event invokes when write/notification events received

        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_MTU_EVENT);
        master2_mtu_done = 1;
#if SMP_ENABLE_M2
        rsi_ble_master2_app_set_event(RSI_BLE_RECEIVE_REMOTE_FEATURES);
#endif
      } break;
      case RSI_BLE_SCAN_RESTART_EVENT: {
        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_SCAN_RESTART_EVENT);
      } break;
      case RSI_APP_EVENT_REMOTE_CONN_PARAM_REQ: {
        //!default ACCEPT the remote conn params request (0-ACCEPT, 1-REJECT)
        status = rsi_ble_conn_param_resp(
          rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_app_remote_device_conn_params.dev_addr,
          1);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("conn param resp status: 0x%X\r\n", status);
        }
        //! remote device conn params request
        //! clear the conn params request event.
        rsi_ble_master2_app_clear_event(RSI_APP_EVENT_REMOTE_CONN_PARAM_REQ);

      } break;
      case RSI_APP_EVENT_DATA_LENGTH_CHANGE: {
        //! clear the disconnected event.
        rsi_ble_master2_app_clear_event(RSI_APP_EVENT_DATA_LENGTH_CHANGE);

        if (rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].remote_dev_feature.remote_features[1] & 0x01) {
          status = rsi_ble_setphy((int8_t *)rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].remote_dev_feature.dev_addr,
                                  TX_PHY_RATE,
                                  RX_PHY_RATE,
                                  CODDED_PHY_RATE);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n failed to cancel the connection request: 0x%x \r\n -m2", status);
          }
        }
      } break;

      case RSI_APP_EVENT_PHY_UPDATE_COMPLETE: {
        //! phy update complete event

        //! clear the phy updare complete event.
        rsi_ble_master2_app_clear_event(RSI_APP_EVENT_PHY_UPDATE_COMPLETE);
      } break;
      case RSI_BLE_SMP_REQ_EVENT: {
        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_SMP_REQ_EVENT);
        LOG_PRINT("\r\n in smp request \r\n -m2 \r\n");
#if SMP_ENABLE_M2
        if (first_connect == 0) {
          //! initiating the SMP pairing process
          status =
            rsi_ble_smp_pair_request(rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_ble_event_smp_req.dev_addr,
                                     RSI_BLE_SMP_IO_CAPABILITY,
                                     MITM_ENABLE);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n RSI_BLE_SMP_REQ_EVENT: failed to initiate the SMP pairing process: 0x%x \r\n -m2", status);
          }
        } else {
          status =
            rsi_ble_start_encryption(rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_ble_event_smp_req.dev_addr,
                                     rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_encryption_enabled.localediv,
                                     rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_encryption_enabled.localrand,
                                     rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_encryption_enabled.localltk);
        }
#endif
      } break;
      case RSI_BLE_SMP_RESP_EVENT: {

        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_SMP_RESP_EVENT);
#if SMP_ENABLE_M2
        LOG_PRINT("\r\n in smp response -m2 \r\n");

        //! initiating the SMP pairing process
        status =
          rsi_ble_smp_pair_response(rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_ble_event_smp_resp.dev_addr,
                                    RSI_BLE_SMP_IO_CAPABILITY,
                                    MITM_ENABLE);
#endif
      } break;
      case RSI_BLE_SMP_PASSKEY_EVENT: {
        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_SMP_PASSKEY_EVENT);
#if RSI_DEBUG_EN
        LOG_PRINT_D("\r\n in smp passkey -m2 \r\n");
#endif
        LOG_PRINT("\n in smp_passkey - str_remote_address : %s -m2\r\n",
                  rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].remote_dev_addr);
        //! initiating the SMP pairing process
        status = rsi_ble_smp_passkey(rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_ble_event_smp_passkey.dev_addr,
                                     RSI_BLE_APP_SMP_PASSKEY);
      } break;
      case RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT: {
        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT);
#if RSI_DEBUG_EN
        LOG_PRINT_D("\r\n in smp pass key display -m2 \r\n");
#endif

        LOG_PRINT("\nremote addr: %s, passkey: %s \r\n",
                  rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].remote_dev_addr,
                  rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_ble_smp_passkey_display.passkey);
      } break;
      case RSI_BLE_LTK_REQ_EVENT: {
        //! event invokes when disconnection was completed

        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_LTK_REQ_EVENT);

        LOG_PRINT("\r\n in LTK  request -m2 \r\n");

        if (
          0) /*((rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES+1].rsi_le_ltk_resp.localediv == l_rsi_encryption_enabled.localediv)
					&& !((memcmp(rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES+1].rsi_le_ltk_resp.localrand,
							l_rsi_encryption_enabled.localrand, 8)))) */
        {
          LOG_PRINT("\n positive reply\n");
          //! give le ltk req reply cmd with positive reply
          status = rsi_ble_ltk_req_reply(rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_le_ltk_resp.dev_addr,
                                         1,
                                         l_rsi_encryption_enabled.localltk);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n failed to restart smp pairing with status: 0x%x -m2\r\n", status);
          }
        } else {
          LOG_PRINT("\n negative reply\n");
          //! give le ltk req reply cmd with negative reply
          status =
            rsi_ble_ltk_req_reply(rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_le_ltk_resp.dev_addr, 0, NULL);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n failed to restart smp pairing with status: 0x%x \r\n", status);
          }
        }
      } break;

      case RSI_BLE_SC_PASSKEY_EVENT: {
        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_SC_PASSKEY_EVENT);

        LOG_PRINT("\r\n in smp sc passkey event -m2 \r\n");

        LOG_PRINT("\n In passkey event, remote addr: %s, passkey: %06d -m2 \r\n",
                  rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].remote_dev_addr,
                  rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_event_sc_passkey.passkey);

        rsi_ble_smp_passkey(rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_event_sc_passkey.dev_addr,
                            rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_event_sc_passkey.passkey);
      } break;

      case RSI_BLE_SECURITY_KEYS_EVENT: {
        //! event invokes when security keys are received
        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_SECURITY_KEYS_EVENT);

        LOG_PRINT("\r\n in smp security keys event  -m2 \r\n");
      } break;

      case RSI_BLE_SMP_FAILED_EVENT: {
        //! initiate SMP protocol as a Master

        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_SMP_FAILED_EVENT);

        LOG_PRINT("\r\n in smp failed remote address: %s -m2 \r\n",
                  rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].remote_dev_addr);
      }

      break;

      case RSI_BLE_ENCRYPT_STARTED_EVENT: {
        //! start the encrypt event
        master2_smp_done = 1;
        //! clear the served event
        rsi_ble_master2_app_clear_event(RSI_BLE_ENCRYPT_STARTED_EVENT);
        //! copy to local buffer
        memcpy(&l_rsi_encryption_enabled,
               &rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_encryption_enabled,
               sizeof(rsi_bt_event_encryption_enabled_t));
        LOG_PRINT("\r\n in smp encrypt event -m2 \r\n");
      } break;
      case RSI_BLE_GATT_ERROR: {
        rsi_ble_master2_app_clear_event(RSI_BLE_GATT_ERROR);
        if ((*(uint16_t *)rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_ble_gatt_err_resp.error)
            == RSI_END_OF_PROFILE_QUERY) {
          if (total_remote_profiles != 0) //! If any profiles exists
          {
            if ((profile_index_for_char_query - 1) < total_remote_profiles) //! Error received for any profile
            {
              char_resp_recvd = false;
              char_for_serv_cnt++;
              //! set event
              rsi_ble_master2_app_set_event(RSI_BLE_GATT_PROFILE);
            } else //! Error received for last profile
            {
              rsi_ble_master2_app_set_event(RSI_BLE_GATT_CHAR_SERVICES);
            }
          } else //! Check for profiles pending, else done profile querying
          {
            //! first level profile query completed
            done_profiles_query = true;

            //set event to start second level profile query
            rsi_ble_master2_app_set_event(RSI_BLE_GATT_PROFILES);
          }
        } else {
          LOG_PRINT("\nGATT ERROR REASON:0x%x -m2 \n",
                    *(uint16_t *)rsi_ble_conn_info[RSI_BLE_MAX_NBR_SLAVES + 1].rsi_ble_gatt_err_resp.error);
        }

      } break;
      default:
        break;
    }
  }
}

#endif
#endif
