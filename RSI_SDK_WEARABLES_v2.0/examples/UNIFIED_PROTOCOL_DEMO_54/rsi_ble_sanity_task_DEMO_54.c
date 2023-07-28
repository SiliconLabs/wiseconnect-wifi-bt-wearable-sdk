/*******************************************************************************
* @file  rsi_ble_sanity_task_DEMO_54.c
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
 * @file    rsi_ble_sanity_task_DEMO_54.c
 * @version 0.1
 * @date    xx Oct 2015
 *
 *
 *
 *  @brief : This file contains example application for BT Classic SPP Slave Role
 *
 *  @section Description  This application serves as a BT SPP Slave.
 *
 */

/*=======================================================================*/
//   ! INCLUDES
/*=======================================================================*/

#include <rsi_common_app.h>
#if (UNIFIED_PROTOCOL && RSI_ENABLE_BLE_TEST)
//! BT include file to refer BT APIs
#include <rsi_driver.h>
#include <rsi_bt_common_apis.h>
#include <rsi_ble_config.h>
#include <rsi_ble_apis.h>
#include <rsi_bt.h>
#include <stdio.h>
#include <rsi_ble.h>
#include <rsi_wlan_non_rom.h>
#include <rsi_ble_config_DEMO_54.h>
#include <rsi_ble_device_info_DEMO_54.h>
#if (RSI_BLE_SANITY_TEST || RSI_BLE_POWER_NUM_TEST || RSI_BLE_ADVERTISING_TEST)
/*=======================================================================*/
//   ! MACROS
/*=======================================================================*/
#define RSI_BT_LE_SC_JUST_WORKS 0x01
#define RSI_BT_LE_SC_PASSKEY    0x02

/*=======================================================================*/
//   ! GLOBAL VARIABLES
/*=======================================================================*/

uint32_t start_handle;
uint32_t end_handle;
uint32_t number_of_profiles;
uint8_t profiles_list_done;
uint8_t conn_dev_addr[18] = { 0 };
uint8_t rsi_conn_update   = 0;
uint8_t req_type, offset;
uint16_t handle;
uint16_t mtu_size;
uint8_t temp_prepare_write_value[500];
uint16_t temp_prepare_write_value_len;
volatile uint64_t master1_state; //! Manufacturing data
static uint8_t first_connect = 0;
static uint32_t passkey;
static uint32_t rsi_app_async_event_map                                                 = 0;
static uint32_t rsi_app_async_event_map1                                                = 0;
static rsi_ble_profile_list_by_conn_t rsi_ble_profile_list_by_conn[RSI_MAX_PROFILE_CNT] = { 0 };
static uint32_t rsi_ble_async_event_mask                                                = 0;
static uint16_t rsi_ble_att1_val_hndl;
static uint16_t rsi_ble_att2_val_hndl;

//! Structure Variables
rsi_ble_t att_list;
static rsi_bt_event_smp_req_t rsi_ble_event_smp_req;
static rsi_bt_event_smp_resp_t rsi_ble_event_smp_resp;
static rsi_bt_event_smp_passkey_display_t rsi_ble_smp_passkey_display;
static rsi_ble_read_req_t app_ble_read_event;
static rsi_ble_event_remote_features_t remote_dev_feature;
static rsi_bt_event_le_ltk_request_t rsi_le_ltk_resp;
static rsi_bt_event_encryption_enabled_t rsi_encryption_enabled;
static rsi_ble_event_write_t app_ble_write_event;
static rsi_ble_event_prepare_write_t app_ble_prepared_write_event;
static rsi_ble_execute_write_t app_ble_execute_write_event;
static rsi_ble_event_mtu_exchange_information_t mtu_exchange_info;
#if RSI_BLE_SANITY_TEST
static rsi_ble_event_mtu_t app_ble_mtu_event;
static rsi_ble_event_phy_update_t rsi_app_phy_update_complete;
static rsi_ble_event_att_value_t rsi_char_descriptors;
#endif
#if SMP_ENABLE
static rsi_bt_event_le_security_keys_t temp_le_sec_keys;
#endif
static rsi_ble_event_error_resp_t rsi_ble_gatt_err_resp;
static rsi_ble_event_profiles_list_t get_allprofiles;
static rsi_ble_event_profile_by_uuid_t get_profile;
static rsi_ble_event_read_by_type1_t get_char_services;
static rsi_ble_event_remote_conn_param_req_t rsi_app_remote_device_conn_params;
static rsi_ble_event_conn_update_t rsi_event_conn_update;
static uint8_t master1_smp_done, master1_mtu_done, smp_pairing_initated = 0;
static uint8_t remote_dev_address[RSI_DEV_ADDR_LEN] = { 0 };
static uint8_t str_remote_address[18];
#if RESOLVE_ENABLE
static rsi_ble_resolve_key_t resolve_key;
static rsi_ble_resolvlist_group_t resolvlist[5];
rsi_ble_dev_ltk_list_t ble_dev_ltk_list[RSI_MAX_LIST_SIZE];
static uint8_t rem_dev_address_type;
static uint8_t rsi_app_resp_resolvlist_size = 0;
#endif
#if WRITE_TO_READONLY_CHAR_TEST
static uint8_t device_found = 0;
uint16_t conn_req_pending   = 0;
uint8_t conn_done;
uint16_t num_of_conn_slaves = 0;
static rsi_ble_event_adv_report_t rsi_app_adv_reports_to_app[NO_OF_ADV_REPORTS];
static uint16_t rsi_app_no_of_adv_reports_rcvd = 0;
static uint8_t remote_addr_type                = 0;
static uint8_t remote_dev_addr[18]             = { 0 };
static uint8_t remote_name[31];
uint8_t remote_dev_bd_addr[6] = { 0 };
static uint8_t remote_name_conn[31];
uint8_t remote_dev_addr_conn[18] = { 0 };
#else
static uint8_t num_of_conn_masters = 0;
#endif

#if SET_SMP_CONFIGURATION
rsi_ble_set_smp_pairing_capabilty_data_t no_signing_keys_supported_capabilities;
#endif
/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/
extern rsi_semaphore_handle_t ble_sanity_task_sem;
/*=======================================================================*/
//   ! EXTERN FUNCTIONS
/*=======================================================================*/
extern int32_t rsi_ble_gatt_write_response(uint8_t *dev_addr, uint8_t type);
extern int32_t rsi_ble_gatt_prepare_write_response(uint8_t *dev_addr,
                                                   uint16_t handle,
                                                   uint16_t offset,
                                                   uint16_t length,
                                                   uint8_t *data);
#if RESOLVE_ENABLE
extern int32_t rsi_ble_start_advertising_with_values(void *rsi_ble_adv);
#endif
/*========================================================================*/
//!  CALLBACK FUNCTIONS
/*=======================================================================*/

/*=======================================================================*/
//   ! PROCEDURES
/*=======================================================================*/

/*==============================================*/
/**
 * @fn         rsi_ble_add_char_serv_att
 * @brief      this function is used to add characteristic service attribute..
 * @param[in]  serv_handler, service handler.
 * @param[in]  handle, characteristic service attribute handle.
 * @param[in]  val_prop, characteristic value property.
 * @param[in]  att_val_handle, characteristic value handle
 * @param[in]  att_val_uuid, characteristic value uuid
 * @return     none.
 * @section description
 * This function is used at application to add characteristic attribute
 */
static void rsi_ble_add_char_serv_att(void *serv_handler,
                                      uint16_t handle,
                                      uint8_t val_prop,
                                      uint16_t att_val_handle,
                                      uuid_t att_val_uuid)
{
  rsi_ble_req_add_att_t new_att = { 0 };

  //! preparing the attribute service structure
  new_att.serv_handler       = serv_handler;
  new_att.handle             = handle;
  new_att.att_uuid.size      = 2;
  new_att.att_uuid.val.val16 = RSI_BLE_CHAR_SERV_UUID;
  new_att.property           = RSI_BLE_ATT_PROPERTY_READ;

  //! preparing the characteristic attribute value
  new_att.data_len = att_val_uuid.size + 4;
  new_att.data[0]  = val_prop;
  rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
  if (new_att.data_len == 6) {
    rsi_uint16_to_2bytes(&new_att.data[4], att_val_uuid.val.val16);
  } else if (new_att.data_len == 8) {
    rsi_uint32_to_4bytes(&new_att.data[4], att_val_uuid.val.val32);
  } else if (new_att.data_len == 20) {
    memcpy(&new_att.data[4], &att_val_uuid.val.val128, att_val_uuid.size);
  }
  //! Add attribute to the service
  rsi_ble_add_attribute(&new_att);

  return;
}

/*==============================================*/
/**
 * @fn         rsi_gatt_add_attribute_to_list
 * @brief      This function is used to store characteristic service attribute.
 * @param[in]  p_val, pointer to homekit structure
 * @param[in]  handle, characteristic service attribute handle.
 * @param[in]  data_len, characteristic value length
 * @param[in]  data, characteristic value pointer
 * @param[in]  uuid, characteristic value uuid
 * @return     none.
 * @section description
 * This function is used to store all attribute records
 */
void rsi_gatt_add_attribute_to_list(rsi_ble_t *p_val,
                                    uint16_t handle,
                                    uint16_t data_len,
                                    uint8_t *data,
                                    uuid_t uuid,
                                    uint8_t char_prop)
{
  if ((p_val->DATA_ix + data_len) >= BLE_ATT_REC_SIZE) { //! Check for max data length for the characteristic value
    LOG_PRINT("\n no data memory for att rec values \n");
    return;
  }

  p_val->att_rec_list[p_val->att_rec_list_count].char_uuid     = uuid;
  p_val->att_rec_list[p_val->att_rec_list_count].handle        = handle;
  p_val->att_rec_list[p_val->att_rec_list_count].value_len     = data_len;
  p_val->att_rec_list[p_val->att_rec_list_count].max_value_len = data_len;
  p_val->att_rec_list[p_val->att_rec_list_count].char_val_prop = char_prop;
  memcpy(p_val->DATA + p_val->DATA_ix, data, data_len);
  p_val->att_rec_list[p_val->att_rec_list_count].value = p_val->DATA + p_val->DATA_ix;
  p_val->att_rec_list_count++;
  p_val->DATA_ix += p_val->att_rec_list[p_val->att_rec_list_count].max_value_len;

  return;
}

/*==============================================*/
/**
 * @fn         rsi_gatt_get_attribute_from_list
 * @brief      This function is used to retrieve attribute from list based on handle.
 * @param[in]  p_val, pointer to characteristic structure
 * @param[in]  handle, characteristic service attribute handle.
 * @return     pointer to the attribute
 * @section description
 * This function is used to store all attribute records
 */
rsi_ble_att_list_t *rsi_gatt_get_attribute_from_list(rsi_ble_t *p_val, uint16_t handle)
{
  uint16_t i;
  for (i = 0; i < p_val->att_rec_list_count; i++) {
    if (p_val->att_rec_list[i].handle == handle) {
      //*val_prop = p_val.att_rec_list[i].char_val_prop;
      //*length = p_val.att_rec_list[i].value_len;
      //*max_data_len = p_val.att_rec_list[i].max_value_len;
      return &(p_val->att_rec_list[i]);
    }
  }
  return 0;
}

static void rsi_ble_add_char_val_att(void *serv_handler,
                                     uint16_t handle,
                                     uuid_t att_type_uuid,
                                     uint8_t val_prop,
                                     uint8_t *data,
                                     uint8_t data_len,
                                     uint8_t auth_read)
{
  rsi_ble_req_add_att_t new_att = { 0 };

  //! preparing the attributes
  new_att.serv_handler  = serv_handler;
  new_att.handle        = handle;
  new_att.config_bitmap = auth_read;
  memcpy(&new_att.att_uuid, &att_type_uuid, sizeof(uuid_t));
  new_att.property = val_prop;

  if (data != NULL)
    memcpy(new_att.data, data, RSI_MIN(sizeof(new_att.data), data_len));

  //! preparing the attribute value
  new_att.data_len = data_len;

  //! add attribute to the service
  rsi_ble_add_attribute(&new_att);

  if ((auth_read == ATT_REC_MAINTAIN_IN_HOST) || (data_len > 20)) {
    if (data != NULL) {
      rsi_gatt_add_attribute_to_list(&att_list, handle, data_len, data, att_type_uuid, val_prop);
    }
  }

  //! check the attribute property with notification
  if (val_prop & RSI_BLE_ATT_PROPERTY_NOTIFY) {
    //! if notification property supports then we need to add client characteristic service.

    //! preparing the client characteristic attribute & values
    memset(&new_att, 0, sizeof(rsi_ble_req_add_att_t));
    new_att.serv_handler       = serv_handler;
    new_att.handle             = handle + 1;
    new_att.att_uuid.size      = 2;
    new_att.att_uuid.val.val16 = RSI_BLE_CLIENT_CHAR_UUID;
    new_att.property           = RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE;
    new_att.data_len           = 2;

    //! add attribute to the service
    rsi_ble_add_attribute(&new_att);
  }

  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_add_simple_chat_serv
 * @brief      this function is used to add new servcie i.e.., simple chat service.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */

static uint32_t rsi_ble_add_simple_chat_serv(void)
{
  uuid_t new_uuid                       = { 0 };
  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
  uint8_t data[230]                     = { 1, 0 };

  //! adding new service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_NEW_SERVICE_UUID;
  rsi_ble_add_service(new_uuid, &new_serv_resp);

  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY | RSI_BLE_ATT_PROPERTY_WRITE,
                            new_serv_resp.start_handle + 2,
                            new_uuid);

  //! adding characteristic value attribute to the service
  rsi_ble_att1_val_hndl = new_serv_resp.start_handle + 2;
  new_uuid.size         = 2;
  new_uuid.val.val16    = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           new_uuid,
                           RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY | RSI_BLE_ATT_PROPERTY_WRITE,
                           data,
                           sizeof(data),
                           1);

  return 0;
}

/*==============================================*/
/**
 * @fn         rsi_ble_add_simple_chat_serv3
 * @brief      this function is used to add new servcie i.e.., simple chat service.
 * @param[in]  none.
 * @return     int32_t
 *             0  =  success
 *             !0 = failure
 * @section description
 * This function is used at application to create new service.
 */

static uint32_t rsi_ble_add_simple_chat_serv3(void)
{
  //! adding the custom service
  // 0x6A4E3300-667B-11E3-949A-0800200C9A66
  uint8_t data1[231]                 = { 1, 0 };
  static const uuid_t custom_service = { .size             = 16,
                                         .reserved         = { 0x00, 0x00, 0x00 },
                                         .val.val128.data1 = 0x6A4E3300,
                                         .val.val128.data2 = 0x667B,
                                         .val.val128.data3 = 0x11E3,
                                         .val.val128.data4 = { 0x9A, 0x94, 0x00, 0x08, 0x66, 0x9A, 0x0C, 0x20 } };

  // 0x6A4E3304-667B-11E3-949A-0800200C9A66
  static const uuid_t custom_characteristic = {
    .size             = 16,
    .reserved         = { 0x00, 0x00, 0x00 },
    .val.val128.data1 = 0x6A4E3304,
    .val.val128.data2 = 0x667B,
    .val.val128.data3 = 0x11E3,
    .val.val128.data4 = { 0x9A, 0x94, 0x00, 0x08, 0x66, 0x9A, 0x0C, 0x20 }
  };

  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
  rsi_ble_add_service(custom_service, &new_serv_resp);

  //! adding custom characteristic declaration to the custom service
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE, //Set read, write, write without response
                            new_serv_resp.start_handle + 2,
                            custom_characteristic);

  //! adding characteristic value attribute to the service
  rsi_ble_att2_val_hndl = new_serv_resp.start_handle + 2;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           custom_characteristic,
                           RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE, //Set read, write, write without response
                           data1,
                           sizeof(data1),
                           1);
  return 0;
}
/*==============================================*/
/**
 * @fn         rsi_ble_app_init_events
 * @brief      initializes the event parameter.
 * @param[in]  none.
 * @return     none.
 * @section description
 * This function is used during BLE initialization.
 */
static void rsi_ble_app_init_events()
{
  rsi_app_async_event_map  = 0;
  rsi_app_async_event_map1 = 0;
  rsi_ble_async_event_mask = 0xFFFFFFFF;
  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_set_event
 * @brief      sets the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to set/raise the specific event.
 */
static void rsi_ble_app_set_event(uint32_t event_num)
{

  if (event_num < 32) {
    rsi_app_async_event_map |= BIT(event_num);
  } else {
    rsi_app_async_event_map1 |= BIT((event_num - 32));
  }
  rsi_semaphore_post(&ble_sanity_task_sem);
  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_clear_event
 * @brief      clears the specific event.
 * @param[in]  event_num, specific event number.
 * @return     none.
 * @section description
 * This function is used to clear the specific event.
 */
static void rsi_ble_app_clear_event(uint32_t event_num)
{
  if (event_num < 32) {
    rsi_app_async_event_map &= ~BIT(event_num);
  } else {
    rsi_app_async_event_map1 &= ~BIT((event_num - 32));
  }
  return;
}

/*==============================================*/
/**
 * @fn         rsi_ble_app_get_event
 * @brief      returns the first set event based on priority
 * @param[in]  none.
 * @return     int32_t
 *             > 0  = event number
 *             -1   = not received any event
 * @section description
 * This function returns the highest priority event among all the set events
 */
static int32_t rsi_ble_app_get_event(void)
{
  uint32_t ix;

  for (ix = 0; ix < 64; ix++) {
    if (ix < 32) {
      if (rsi_app_async_event_map & (1 << ix)) {
        return ix;
      }
    } else {
      if (rsi_app_async_event_map1 & (1 << (ix - 32))) {
        return ix;
      }
    }
  }
  return (RSI_FAILURE);
}

#if WRITE_TO_READONLY_CHAR_TEST
void rsi_ble_on_adv_report_event(rsi_ble_event_adv_report_t *adv_report)
{
  //uint8_t status = 0;

  if (device_found == 1) {
    return;
  }

  memcpy(&rsi_app_adv_reports_to_app[(rsi_app_no_of_adv_reports_rcvd) % (NO_OF_ADV_REPORTS)],
         adv_report,
         sizeof(rsi_ble_event_adv_report_t));
  rsi_app_no_of_adv_reports_rcvd++;

  if (rsi_app_no_of_adv_reports_rcvd == NO_OF_ADV_REPORTS) {
    rsi_app_no_of_adv_reports_rcvd = 0;
    //LOG_PRINT("\nrestart scan\r\n");
    rsi_ble_app_set_event(RSI_BLE_SCAN_RESTART_EVENT);
  }

  BT_LE_ADPacketExtract(remote_name, adv_report->adv_data, adv_report->adv_data_len);

  remote_addr_type = adv_report->dev_addr_type;
  rsi_6byte_dev_address_to_ascii(remote_dev_addr, (uint8_t *)adv_report->dev_addr);
  memcpy((int8_t *)remote_dev_bd_addr, (uint8_t *)adv_report->dev_addr, 6);

#if (CONNECT_OPTION == CONN_BY_NAME)
  if ((device_found == 0) && ((strcmp((const char *)remote_name, RSI_REMOTE_DEVICE_NAME1)) == 0)) {
    memcpy(remote_name_conn, remote_name, 31);
    memcpy(remote_dev_addr_conn, remote_dev_addr, 18);
    if (conn_done == 0) {
      device_found = 1;
      conn_done    = 1;
      rsi_ble_app_set_event(RSI_APP_EVENT_ADV_REPORT);
      return;
    }
  }
#else
  if ((!strcmp(RSI_BLE_DEV_1_ADDR, (char *)remote_dev_addr))) {
    memcpy(conn_dev_addr, remote_dev_addr, sizeof(remote_dev_addr));
    memcpy(remote_dev_addr_conn, remote_dev_addr, 18);
    device_found = 1;
    rsi_ble_app_set_event(RSI_APP_EVENT_ADV_REPORT);
  }
#endif

  return;
}
#endif
/*==============================================*/
/**
 * @fn         rsi_ble_on_enhance_conn_status_event
 * @brief      invoked when enhanced connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
static void rsi_ble_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t *resp_enh_conn)
{
  memcpy((int8_t *)remote_dev_address, resp_enh_conn->dev_addr, 6);
  LOG_PRINT("\nconnect - str_remote_address : %s\r\n",
            rsi_6byte_dev_address_to_ascii(str_remote_address, resp_enh_conn->dev_addr));
  rsi_ble_app_set_event(RSI_BLE_ENHC_CONN_EVENT);
#if RESOLVE_ENABLE
  rem_dev_address_type = resp_enh_conn->dev_addr_type;
#endif
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_connect_event
 * @brief      invoked when connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
static void rsi_ble_on_connect_event(rsi_ble_event_conn_status_t *resp_conn)
{
  memcpy((int8_t *)remote_dev_address, resp_conn->dev_addr, 6);
  LOG_PRINT("\nconnect - str_remote_address : %s\r\n",
            rsi_6byte_dev_address_to_ascii(str_remote_address, resp_conn->dev_addr));
  rsi_ble_app_set_event(RSI_BLE_CONN_EVENT);
#if RESOLVE_ENABLE
  rem_dev_address_type = resp_conn->dev_addr_type;
#endif
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_disconnect_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This Callback function indicates disconnected device information and status
 */
static void rsi_ble_on_disconnect_event(rsi_ble_event_disconnect_t *resp_disconnect, uint16_t reason)
{
  //uint8_t str_disconn_device[18] = { 0 };

  //memcpy ((int8_t *) str_disconn_device, (uint8_t *) resp_disconnect->dev_addr);
  rsi_ble_app_set_event(RSI_BLE_DISCONN_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_simple_peripheral_on_remote_features_event
 * @brief      invoked when LE remote features event is received.
 * @param[in] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
static void rsi_ble_simple_peripheral_on_remote_features_event(
  rsi_ble_event_remote_features_t *rsi_ble_event_remote_features)
{
  LOG_PRINT("\nFeature received is %d\n", rsi_ble_event_remote_features->remote_features);
  memcpy(&remote_dev_feature, rsi_ble_event_remote_features, sizeof(rsi_ble_event_remote_features_t));
  rsi_ble_app_set_event(RSI_BLE_RECEIVE_REMOTE_FEATURES);
}

/*
static void rsi_ble_on_mtu_event(rsi_ble_event_mtu_t *rsi_ble_mtu) {
	rsi_ble_app_set_event(RSI_BLE_MTU_EVENT);
}
 */

#if SMP_ENABLE
/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_request
 * @brief      its invoked when smp request event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP request events is received(we are in Master mode)
 * Note: slave requested to start SMP request, we have to send SMP request command
 */
static void rsi_ble_on_smp_request(rsi_bt_event_smp_req_t *remote_smp)
{
  //! copy to conn specific buffer
  memcpy(&rsi_ble_event_smp_req, remote_smp, sizeof(rsi_bt_event_smp_req_t));
  //memcpy ((int8_t *)remote_dev_address, remote_smp->dev_addr, 6);
  LOG_PRINT("\nsmp_req - str_remote_address : %s\r\n",
            rsi_6byte_dev_address_to_ascii(str_remote_address, remote_smp->dev_addr));
  rsi_ble_app_set_event(RSI_BLE_SMP_REQ_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_response
 * @brief      its invoked when smp response event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP response events is received(we are in slave mode)
 * Note: Master initiated SMP protocol, we have to send SMP response command
 */
static void rsi_ble_on_smp_response(rsi_bt_event_smp_resp_t *remote_smp)
{
  //memcpy ((int8_t *)remote_dev_address, remote_smp->dev_addr, 6);
  memcpy(&rsi_ble_event_smp_resp, remote_smp, sizeof(rsi_bt_event_smp_resp_t));
  LOG_PRINT("\nsmp_resp - str_remote_address : %s\r\n",
            rsi_6byte_dev_address_to_ascii(str_remote_address, remote_smp->dev_addr));
  rsi_ble_app_set_event(RSI_BLE_SMP_RESP_EVENT);
}

#if 0
/*==============================================*/
/**
 * @fn         rsi_ble_on_cli_smp_response
 * @brief      its invoked when smp response event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP response events is received(we are in slave mode)
 * Note: Master initiated SMP protocol, we have to send SMP response command
 */
void rsi_ble_on_cli_smp_response(rsi_bt_event_smp_resp_t *remote_smp) {
	memcpy ((int8_t *)remote_dev_address, remote_smp->dev_addr, 6);
	remote_smp_response.io_cap = remote_smp->io_cap;
	remote_smp_response.oob_data = remote_smp->oob_data;
	remote_smp_response.auth_req = remote_smp->auth_req;
	remote_smp_response.min_req_key_size = remote_smp->min_req_key_size;
	LOG_PRINT("\nsmp_rmeote_cli_resp - str_remote_address : %s\r\n", rsi_6byte_dev_address_to_ascii(str_remote_address, remote_smp->dev_addr));
	rsi_ble_app_set_event(RSI_BLE_CLI_SMP_RESP_EVENT);
}
#endif
/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_passkey
 * @brief      its invoked when smp passkey event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP passkey events is received
 * Note: We have to send SMP passkey command
 */
static void rsi_ble_on_smp_passkey(rsi_bt_event_smp_passkey_t *smp_passkey_event)
{
  memcpy((int8_t *)remote_dev_address, smp_passkey_event->dev_addr, 6);
  LOG_PRINT("\nsmp_passkey - str_remote_address : %s\r\n",
            rsi_6byte_dev_address_to_ascii(str_remote_address, smp_passkey_event->dev_addr));
  rsi_ble_app_set_event(RSI_BLE_SMP_PASSKEY_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_passkey_display
 * @brief      its invoked when smp passkey event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP passkey events is received
 * Note: We have to send SMP passkey command
 */
static void rsi_ble_on_smp_passkey_display(rsi_bt_event_smp_passkey_display_t *smp_passkey_display)
{
  memcpy(&rsi_ble_smp_passkey_display, smp_passkey_display, sizeof(rsi_bt_event_smp_passkey_display_t));
  LOG_PRINT("\nremote addr: %s, passkey: %s \r\n",
            rsi_6byte_dev_address_to_ascii(str_remote_address, smp_passkey_display->dev_addr),
            smp_passkey_display->passkey);
  rsi_ble_app_set_event(RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT);
  //(RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT);
}
/*==============================================*/
/**
 * @fn         rsi_ble_on_sc_passkey
 * @brief      its invoked when smp passkey event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP passkey events is received
 * Note: We have to send SMP passkey command
 */
static void rsi_ble_on_sc_passkey(rsi_bt_event_sc_passkey_t *sc_passkey)
{
  memcpy((int8_t *)remote_dev_address, sc_passkey->dev_addr, 6);
  LOG_PRINT("\nremote addr: %s, passkey: %06d \r\n",
            rsi_6byte_dev_address_to_ascii(str_remote_address, sc_passkey->dev_addr),
            sc_passkey->passkey);
  passkey = sc_passkey->passkey;
  rsi_ble_app_set_event(RSI_BLE_SC_PASSKEY_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_smp_failed
 * @brief      its invoked when smp failed event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when SMP failed events is received
 */
static void rsi_ble_on_smp_failed(uint16_t status, rsi_bt_event_smp_failed_t *remote_dev_address)
{
  memcpy((int8_t *)remote_dev_address, remote_dev_address->dev_addr, 6);
  LOG_PRINT("\nsmp_failed status: 0x%x, str_remote_address: %s\r\n",
            status,
            rsi_6byte_dev_address_to_ascii(str_remote_address, remote_dev_address->dev_addr));
  rsi_ble_app_set_event(RSI_BLE_SMP_FAILED_EVENT);
}

#if SIG_VUL
/*==============================================*/
/**
 * @fn         rsi_ble_on_sc_method
 * @brief      its invoked when security method event is received.
 * @param[in]  scmethod, 1 means Justworks and 2 means Passkey.
 * @return     none.
 * @section description
 * This callback function is invoked when SC Method events is received
 */
void rsi_ble_on_sc_method(rsi_bt_event_sc_method_t *scmethod)
{
  if (scmethod->sc_method == RSI_BT_LE_SC_JUST_WORKS) {
    LOG_PRINT("\nOur Security method is Justworks, hence compare the 6 digit numeric value on both devices\r\n");
  } else if (scmethod->sc_method == RSI_BT_LE_SC_PASSKEY) {
    LOG_PRINT(
      "\nOur Security method is Passkey_Entry, hence same 6 digit Numeric value to be entered on both devices using "
      "keyboard, do not enter the numeric value displayed on one device into another device using keyboard \r\n");
  } else {
  }
}
#endif

/*==============================================*/
/**
 * @fn         rsi_ble_on_encrypt_started
 * @brief      its invoked when encryption started event is received.
 * @param[in]  remote_dev_address, it indicates remote bd address.
 * @return     none.
 * @section description
 * This callback function is invoked when encryption started events is received
 */
static void rsi_ble_on_encrypt_started(uint16_t status, rsi_bt_event_encryption_enabled_t *enc_enabled)
{
  LOG_PRINT("\nstart encrypt status: %d \r\n", status);
  memcpy(&rsi_encryption_enabled, enc_enabled, sizeof(rsi_bt_event_encryption_enabled_t));
  rsi_ble_app_set_event(RSI_BLE_ENCRYPT_STARTED_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_le_ltk_req_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates disconnected device information and status
 */
static void rsi_ble_on_le_ltk_req_event(rsi_bt_event_le_ltk_request_t *le_ltk_req)
{
  memcpy(&rsi_le_ltk_resp, le_ltk_req, sizeof(rsi_bt_event_le_ltk_request_t));
  rsi_ble_app_set_event(RSI_BLE_LTK_REQ_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_le_security_keys_event
 * @brief      invoked when security keys event is received
 * @param[in]  rsi_bt_event_le_security_keys_t, security keys information
 * @return     none.
 * @section description
 * This callback function indicates security keys information
 */
static void rsi_ble_on_le_security_keys_event(rsi_bt_event_le_security_keys_t *le_sec_keys)
{
  LOG_PRINT(" \r\n rsi_ble_on_le_security_keys_event \r\n");
  memcpy(&temp_le_sec_keys, le_sec_keys, sizeof(rsi_bt_event_le_security_keys_t));
  rsi_ble_app_set_event(RSI_BLE_SECURITY_KEYS_EVENT);
}
#endif

static void rsi_ble_on_conn_update_complete_event(rsi_ble_event_conn_update_t *rsi_ble_event_conn_update_complete,
                                                  uint16_t resp_status)
{
  memcpy(&rsi_event_conn_update, rsi_ble_event_conn_update_complete, sizeof(rsi_ble_event_conn_update_t));
  rsi_ble_app_set_event(RSI_BLE_CONN_UPDATE_COMPLETE_EVENT);
}

#if RSI_BLE_SANITY_TEST

static void rsi_ble_on_read_req_event(uint16_t event_id, rsi_ble_read_req_t *rsi_ble_read_req)
{
  offset   = 0;
  req_type = 0;
  memcpy(&app_ble_read_event, rsi_ble_read_req, sizeof(rsi_ble_read_req_t));
  if (rsi_ble_read_req->type == 1)
    req_type = 1;
  offset = rsi_ble_read_req->offset;
  handle = rsi_ble_read_req->handle;
  rsi_6byte_dev_address_to_ascii(str_remote_address, rsi_ble_read_req->dev_addr);
  memcpy(remote_dev_address, rsi_ble_read_req->dev_addr, 6);
  LOG_PRINT("\nOffset value is %d\n", offset);
  rsi_ble_app_set_event(RSI_BLE_READ_REQ_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_char_services_event
 * @brief      invoked when response is received for characteristic services details
 * @param[out] rsi_ble_event_char_services, list of characteristic services.
 * @return     none
 * @section description
 */
static void rsi_ble_char_services_event(uint16_t resp_status,
                                        rsi_ble_event_read_by_type1_t *rsi_ble_event_char_services)
{
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(str_remote_address, rsi_ble_event_char_services->dev_addr);

  //! copy to conn specific buffer
  memcpy(&get_char_services, rsi_ble_event_char_services, sizeof(rsi_ble_event_read_by_type1_t));

  rsi_ble_app_set_event(RSI_BLE_GATT_CHAR_SERVICES);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_read_resp_event
 * @brief      invoked when response is received for the characteristic descriptors details
 * @param[out] rsi_ble_event_att_value_t, list of descriptors response
 * @return     none
 * @section description
 */
static void rsi_ble_on_read_resp_event(uint16_t event_status, rsi_ble_event_att_value_t *rsi_ble_event_att_val)
{

  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(str_remote_address, rsi_ble_event_att_val->dev_addr);

  //! copy to conn specific buffer
  memcpy(&rsi_char_descriptors, rsi_ble_event_att_val, sizeof(rsi_ble_event_att_value_t));

  rsi_ble_app_set_event(RSI_BLE_GATT_DESC_SERVICES);
}

/*==============================================*/
/**
 * @fn         rsi_ble_data_length_change_event
 * @brief      invoked when data length is set
 * @section description
 * This Callback function indicates data length is set
 */
static void rsi_ble_data_length_change_event(rsi_ble_event_data_length_update_t *rsi_ble_data_length_update)
{
  LOG_PRINT("\nMax_tx_octets: %d \r\n", rsi_ble_data_length_update->MaxTxOctets);
  LOG_PRINT("Max_tx_time: %d \r\n", rsi_ble_data_length_update->MaxTxTime);
  LOG_PRINT("Max_rx_octets: %d \r\n", rsi_ble_data_length_update->MaxRxOctets);
  LOG_PRINT("Max_rx_time: %d \r\n", rsi_ble_data_length_update->MaxRxTime);
  rsi_ble_app_set_event(RSI_APP_EVENT_DATA_LENGTH_CHANGE);
}

/*==============================================*/
/**
 * @fn         rsi_ble_phy_update_complete_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This Callback function indicates disconnected device information and status
 */
static void rsi_ble_phy_update_complete_event(rsi_ble_event_phy_update_t *rsi_ble_event_phy_update_complete)
{
  memcpy(&rsi_app_phy_update_complete, rsi_ble_event_phy_update_complete, sizeof(rsi_ble_event_phy_update_t));

  rsi_ble_app_set_event(RSI_APP_EVENT_PHY_UPDATE_COMPLETE);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_remote_conn_params_request_event
 * @brief      invoked when remote conn params request is received
 * @param[out] remote_conn_param, emote conn params request information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
static void rsi_ble_on_remote_conn_params_request_event(rsi_ble_event_remote_conn_param_req_t *remote_conn_param,
                                                        uint16_t status)
{
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(str_remote_address, remote_conn_param->dev_addr);

  //! copy to conn specific buffer
  memcpy(&rsi_app_remote_device_conn_params, remote_conn_param, sizeof(rsi_ble_event_remote_conn_param_req_t));

  rsi_ble_app_set_event(RSI_APP_EVENT_REMOTE_CONN_PARAM_REQ);
}

static void rsi_ble_more_data_req_event(rsi_ble_event_le_dev_buf_ind_t *rsi_ble_more_data_evt)
{

  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(str_remote_address, rsi_ble_more_data_evt->remote_dev_bd_addr);

  rsi_ble_app_set_event(RSI_BLE_MORE_DATA_REQ_EVT);

  return;
}
static void rsi_ble_on_event_mtu_exchange_info(
  rsi_ble_event_mtu_exchange_information_t *rsi_ble_event_mtu_exchange_info)
{
  LOG_PRINT("\r\n Received MTU EXCHANGE Information Event in main task\r\n");
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(str_remote_address, rsi_ble_event_mtu_exchange_info->dev_addr);

  memcpy(&mtu_exchange_info, rsi_ble_event_mtu_exchange_info, sizeof(rsi_ble_event_mtu_exchange_information_t));

  LOG_PRINT("Remote Device Address : %s\n", str_remote_address);
  LOG_PRINT("RemoteMTU : %d \r\n", mtu_exchange_info.remote_mtu_size);
  LOG_PRINT("LocalMTU : %d\r\n", mtu_exchange_info.local_mtu_size);
  LOG_PRINT("Initated Role : 0x%x \r\n", mtu_exchange_info.initiated_role);
  //! set conn specific event
  rsi_ble_app_set_event(RSI_BLE_MTU_EXCHANGE_INFORMATION);
}
/*==============================================*/
/**
 * @fn         rsi_ble_on_gatt_write_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void rsi_ble_on_gatt_write_event(uint16_t event_id, rsi_ble_event_write_t *rsi_ble_write)
{
  memcpy(&app_ble_write_event, rsi_ble_write, sizeof(rsi_ble_event_write_t));
  rsi_ble_app_set_event(RSI_BLE_GATT_WRITE_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_gatt_prepare_write_event
 * @brief      its invoked when prepared write events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void rsi_ble_on_gatt_prepare_write_event(uint16_t event_id,
                                                rsi_ble_event_prepare_write_t *rsi_app_ble_prepared_write_event)
{
  memcpy(&app_ble_prepared_write_event, rsi_app_ble_prepared_write_event, sizeof(rsi_ble_event_prepare_write_t));
  LOG_PRINT("\nPGW Offset value is %d\n", (*(uint16_t *)app_ble_prepared_write_event.offset));
  rsi_ble_app_set_event(RSI_BLE_GATT_PREPARE_WRITE_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_gatt_execute_write_event
 * @brief      its invoked when prepared write events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void rsi_ble_on_execute_write_event(uint16_t event_id, rsi_ble_execute_write_t *rsi_app_ble_execute_write_event)
{
  memcpy(&app_ble_execute_write_event, rsi_app_ble_execute_write_event, sizeof(rsi_ble_event_prepare_write_t));
  rsi_ble_app_set_event(RSI_BLE_GATT_EXECUTE_WRITE_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_on_mtu_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void rsi_ble_on_mtu_event(rsi_ble_event_mtu_t *rsi_ble_mtu)
{
  memcpy(&app_ble_mtu_event, rsi_ble_mtu, sizeof(rsi_ble_event_mtu_t));
  mtu_size = (uint16_t)app_ble_mtu_event.mtu_size;
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(str_remote_address, app_ble_mtu_event.dev_addr);
  LOG_PRINT("\n MTU size from remote device(%s), %d\n", str_remote_address, app_ble_mtu_event.mtu_size);
  rsi_ble_app_set_event(RSI_BLE_MTU_EVENT);
}

/*==============================================*/
/**
 * @fn         rsi_ble_gatt_error_event
 * @brief      this function will invoke when set the attribute value complete
 * @param[out] rsi_ble_gatt_error, event structure
 * @param[out] status, status of the response
 * @return     none
 * @section description
 */
static void rsi_ble_gatt_error_event(uint16_t resp_status, rsi_ble_event_error_resp_t *rsi_ble_gatt_error)
{
  //! copy to conn specific buffer
  memcpy(&rsi_ble_gatt_err_resp, rsi_ble_gatt_error, sizeof(rsi_ble_event_error_resp_t));

  rsi_ble_app_set_event(RSI_BLE_GATT_ERROR);
}

/*==============================================*/
/**
 * @fn         rsi_ble_profiles_list_event
 * @brief      invoked when response is received for get list of services.
 * @param[out] p_ble_resp_profiles, profiles list details
 * @return     none
 * @section description
 */
static void rsi_ble_profiles_list_event(uint16_t resp_status, rsi_ble_event_profiles_list_t *rsi_ble_event_profiles)
{

  if (resp_status == 0x4A0A) {
    return;
  }
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(str_remote_address, rsi_ble_event_profiles->dev_addr);

  //! copy to conn specific buffer
  memcpy(&get_allprofiles, rsi_ble_event_profiles, sizeof(rsi_ble_event_profiles_list_t));

  rsi_ble_app_set_event(RSI_BLE_GATT_PROFILES);
}

/*==============================================*/
/**
 * @fn         rsi_ble_profile_event
 * @brief      invoked when the specific service details are received for
 *             supported specific services.
 * @param[out] rsi_ble_event_profile, specific profile details
 * @return     none
 * @section description
 * This is a callback function
 */
static void rsi_ble_profile_event(uint16_t resp_status, rsi_ble_event_profile_by_uuid_t *rsi_ble_event_profile)
{

  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(str_remote_address, rsi_ble_event_profile->dev_addr);

  //! copy to conn specific buffer
  memcpy(&get_profile, rsi_ble_event_profile, sizeof(rsi_ble_event_profile_by_uuid_t));

  rsi_ble_app_set_event(RSI_BLE_GATT_PROFILE);
}
#endif

#if RESOLVE_ENABLE

/*==============================================*/
/**
 * @fn         add_device_to_ltk_key_list
 * @brief      this function will add device to the ltk key list
 * @param[in]  ltk device list pointer
 * @param[in]  encrypt start event pointer
 * @param[out] status
 * @section description
 * add device to ltk key list
 * */
int8_t add_device_to_ltk_key_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list,
                                  rsi_bt_event_encryption_enabled_t enc_enabled)
{
  uint8_t status = RSI_SUCCESS;
  uint8_t ix;
  uint8_t index_found = 0;

  for (ix = 0; ix < RSI_MAX_LIST_SIZE; ix++) {

    if (ble_dev_ltk_list[ix].used == 1) {
      if ((enc_enabled.dev_addr_type > 1) && (!memcmp(enc_enabled.dev_addr, ble_dev_ltk_list[ix].Identity_addr, 6))) {
        index_found = 1;
        break;
      }

      if ((enc_enabled.dev_addr_type <= 1)
          && (!memcmp(enc_enabled.dev_addr, ble_dev_ltk_list[ix].remote_dev_addr, 6))) {
        index_found = 1;
        break;
      }
    }

    if (ble_dev_ltk_list[ix].used == 0) {
      ble_dev_ltk_list[ix].used       = 1;
      ble_dev_ltk_list[ix].enc_enable = enc_enabled.enabled;
      ble_dev_ltk_list[ix].sc_enable  = enc_enabled.sc_enable;
      memcpy(ble_dev_ltk_list[ix].remote_dev_addr, enc_enabled.dev_addr, 6);
      memcpy(ble_dev_ltk_list[ix].localltk, enc_enabled.localltk, 16);
      memcpy(ble_dev_ltk_list[ix].localrand, enc_enabled.localrand, 8);
      ble_dev_ltk_list[ix].local_ediv = enc_enabled.localediv;
      break;
    }
  }

  if (index_found == 1) {
    //LOG_PRINT("\n Add device to LTK list and its index = %d \n", status);
    ble_dev_ltk_list[ix].enc_enable = enc_enabled.enabled;
    ble_dev_ltk_list[ix].sc_enable  = enc_enabled.sc_enable;
    memcpy(ble_dev_ltk_list[ix].remote_dev_addr, enc_enabled.dev_addr, 6);
    memcpy(ble_dev_ltk_list[ix].localltk, enc_enabled.localltk, 16);
    memcpy(ble_dev_ltk_list[ix].localrand, enc_enabled.localrand, 8);
    ble_dev_ltk_list[ix].local_ediv = enc_enabled.localediv;
    index_found                     = 0;
  }
  if (ix >= RSI_MAX_LIST_SIZE) {
    return -1;
  }
  return status;
}

/*==============================================*/
/**
 * @fn         add_security_keys_to_device_list
 * @brief      this function will add device to resolvlistwith updated irks 
 * @param[out] ix, index of resolvlist
 * @return     none.
 * @section description
 * add device to resolvlistwith updated irks 
 * */
int8_t add_security_keys_to_device_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list,
                                        rsi_bt_event_le_security_keys_t le_sec_keys)
{
  uint8_t status = RSI_SUCCESS;
  uint8_t ix;
  for (ix = 0; ix < RSI_MAX_LIST_SIZE; ix++) {

    if ((ble_dev_ltk_list[ix].used == 1) && (!memcmp(ble_dev_ltk_list[ix].remote_dev_addr, le_sec_keys.dev_addr, 6))
        && (ble_dev_ltk_list[ix].remote_dev_addr_type == le_sec_keys.dev_addr_type)) {
      memcpy(ble_dev_ltk_list[ix].local_irk, le_sec_keys.local_irk, 16);
      memcpy(ble_dev_ltk_list[ix].peer_irk, le_sec_keys.remote_irk, 16);
      memcpy(ble_dev_ltk_list[ix].remote_rand, le_sec_keys.remote_rand, 8);
      memcpy(ble_dev_ltk_list[ix].remote_ltk, le_sec_keys.remote_ltk, 16);
      memcpy(ble_dev_ltk_list[ix].Identity_addr, le_sec_keys.Identity_addr, 6);
      ble_dev_ltk_list[ix].remote_ediv        = le_sec_keys.remote_ediv;
      ble_dev_ltk_list[ix].Identity_addr_type = le_sec_keys.Identity_addr_type;
      break;
    }
  }
  if (ix >= RSI_MAX_LIST_SIZE) {
    return -1;
  }

  return status;
}

/*==============================================*/
/**
 * @fn         add_device_to_resolvlist
 * @brief      this function will add device to resolvlistwith updated irks 
 * @param[out] ix, index of resolvlist
 * @return     none.
 * @section description
 * add device to resolvlistwith updated irks 
 * */
uint32_t add_device_to_resolvlist(rsi_ble_resolvlist_group_t *resolvlist_p, rsi_ble_resolve_key_t *resolve_key_p)
{
  uint8_t status;
  uint8_t ix;

  for (ix = 0; ix < RSI_BLE_RESOLVING_LIST_SIZE; ix++) {
    if (resolvlist_p[ix].used == 0) {
      //LOG_PRINT ("\n Values are Update and index is = %d \n", ix);
      resolvlist_p[ix].used               = 1;
      resolvlist_p[ix].Identity_addr_type = resolve_key_p->Identity_addr_type;
      memcpy(resolvlist_p[ix].Identity_addr, resolve_key_p->Identity_addr, sizeof(resolve_key_p->Identity_addr));
      memcpy(resolvlist_p[ix].peer_irk, resolve_key_p->peer_irk, sizeof(resolve_key_p->peer_irk));
      memcpy(resolvlist_p[ix].local_irk, resolve_key_p->local_irk, sizeof(resolve_key_p->local_irk));
      break;
    }
  }
  if (ix >= RSI_BLE_RESOLVING_LIST_SIZE) {
    return -1;
  }

  //add device to resolvlist or remove from resolvlist or clear the resolvlist
  status = rsi_ble_resolvlist(RSI_BLE_ADD_TO_RESOLVE_LIST,
                              resolve_key_p->Identity_addr_type,
                              temp_le_sec_keys.Identity_addr,
                              resolve_key_p->peer_irk,
                              resolve_key_p->local_irk);
  if (status != RSI_SUCCESS) {
    return status;
  }
  return status;
}

int8_t rsi_get_ltk_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list, rsi_bt_event_le_ltk_request_t le_ltk_req)
{
  uint8_t ix;

  for (ix = 0; ix < RSI_MAX_LIST_SIZE; ix++) {

    if (ble_dev_ltk_list[ix].used == 1) {
      if ((le_ltk_req.dev_addr_type > 1) && (!(memcmp(le_ltk_req.dev_addr, ble_dev_ltk_list[ix].Identity_addr, 6)))) {
        return ix;
      }

      if ((le_ltk_req.dev_addr_type <= 1)
          && (!(memcmp(le_ltk_req.dev_addr, ble_dev_ltk_list[ix].remote_dev_addr, 6)))) {
        return ix;
      }
    }
  }
  return -1;
}

/*==============================================*/
/**
 * @fn         update_resolvlist
 * @brief      this function will update the resolvlist if different remote address or different irks for same address received
 * @param[out] none.
 * @return     none.
 * @section description
 * update the resolvlist if different remote address or different irks for same address received
 * */
void update_resolvlist(rsi_ble_resolvlist_group_t *resolvlist_p, rsi_ble_resolve_key_t *resolve_key_p)
{
  uint8_t ix = 0;
  while (ix < RSI_BLE_RESOLVING_LIST_SIZE) {
    if (!strcmp((const char *)resolvlist_p[ix].Identity_addr, (const char *)resolve_key_p->Identity_addr)) {
      if (memcmp(resolvlist_p[ix].peer_irk, resolve_key_p->peer_irk, sizeof(resolve_key_p->peer_irk))) {
        //LOG_PRINT ("\n Peer irk is modified and its index = %d \n", ix);
        add_device_to_resolvlist(resolvlist_p, resolve_key_p);
        break;
      } else {
        //LOG_PRINT ("\n Peer IRK is not Modified \n");
        break;
      }
    } else {
      strcpy((char *)resolvlist_p[ix].Identity_addr, (const char *)resolve_key_p->Identity_addr);
      //LOG_PRINT ("\n Add New device to resolving list and its index = %d \n", ix);
      add_device_to_resolvlist(resolvlist_p, resolve_key_p);
      break;
    }
    ix++;
  }
}
#endif

/*==============================================*/
/**
 * @fn         rsi_ble_dual_role
 * @brief      Tests the BLE GAP peripheral role.
 * @param[in]  none
 * @return    none.
 * @section description
 * This function is used to test the BLE peripheral role and simple GAP API's.
 */
static int32_t rsi_ble_dual_role(void)
{
  int32_t status  = 0;
  uint8_t adv[31] = { 2, 1, 6 };

  //! This should be vary from one device to other, Present RSI dont have a support of FIXED IRK on every Reset
  uint8_t local_irk[16] = { 0x4e, 0xd7, 0xbd, 0x3e, 0xec, 0x10, 0xda, 0xab,
                            0x1f, 0x85, 0x56, 0xee, 0xa5, 0xc8, 0xe6, 0x93 };

  rsi_ble_add_simple_chat_serv();
  rsi_ble_add_simple_chat_serv3();

#if RSI_BLE_SANITY_TEST
  //! registering the GAP callback functions
  rsi_ble_gap_register_callbacks(
#if WRITE_TO_READONLY_CHAR_TEST
    rsi_ble_on_adv_report_event,
#else
    NULL,
#endif
    rsi_ble_on_connect_event,
    rsi_ble_on_disconnect_event,
    NULL,
    rsi_ble_phy_update_complete_event,
    rsi_ble_data_length_change_event,
    rsi_ble_on_enhance_conn_status_event,
    NULL,
    rsi_ble_on_conn_update_complete_event,
    rsi_ble_on_remote_conn_params_request_event);

  rsi_ble_gap_extended_register_callbacks(rsi_ble_simple_peripheral_on_remote_features_event,
                                          rsi_ble_more_data_req_event);

  rsi_ble_gatt_register_callbacks(NULL,
                                  NULL, /*rsi_ble_profile*/
                                  NULL, /*rsi_ble_char_services*/
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  rsi_ble_on_gatt_write_event,
                                  rsi_ble_on_gatt_prepare_write_event,
                                  rsi_ble_on_execute_write_event,
                                  rsi_ble_on_read_req_event,
                                  rsi_ble_on_mtu_event,
                                  rsi_ble_gatt_error_event,
                                  NULL,
                                  rsi_ble_profiles_list_event,
                                  rsi_ble_profile_event,
                                  rsi_ble_char_services_event,
                                  NULL,
                                  NULL,
                                  rsi_ble_on_read_resp_event,
                                  NULL,
                                  NULL,
                                  NULL);
  rsi_ble_gatt_extended_register_callbacks(rsi_ble_on_event_mtu_exchange_info);
#if SMP_ENABLE
  //! registering the SMP callback functions
  rsi_ble_smp_register_callbacks(rsi_ble_on_smp_request,
                                 rsi_ble_on_smp_response,
                                 rsi_ble_on_smp_passkey,
                                 rsi_ble_on_smp_failed,
                                 rsi_ble_on_encrypt_started,
                                 rsi_ble_on_smp_passkey_display,
                                 rsi_ble_on_sc_passkey,
                                 rsi_ble_on_le_ltk_req_event,
                                 rsi_ble_on_le_security_keys_event,
                                 NULL,
#if SIG_VUL
                                 rsi_ble_on_sc_method
#else
                                 NULL
#endif
  );
#endif
#elif (RSI_BLE_POWER_NUM_TEST || RSI_BLE_ADVERTISING_TEST)
  //! registering the GAP call back functions
  rsi_ble_gap_register_callbacks(NULL,
                                 rsi_ble_on_connect_event,
                                 rsi_ble_on_disconnect_event,
                                 NULL,
                                 NULL, /*rsi_ble_phy_update_complete_event*/
                                 NULL, /*rsi_ble_data_length_change_event*/
                                 rsi_ble_on_enhance_conn_status_event,
                                 NULL,
                                 rsi_ble_on_conn_update_complete_event,
                                 NULL);
  rsi_ble_gap_extended_register_callbacks(rsi_ble_simple_peripheral_on_remote_features_event, NULL);
#endif
  //! initializing the application events map
  rsi_ble_app_init_events();

  //! Set local name
  rsi_bt_set_local_name(RSI_BLE_APP_GATT_TEST);

  //! Set local IRK Value
  //! This value should be fixed on every reset
  LOG_PRINT("\n Setting the Local IRK Value \n");
  status = rsi_ble_set_local_irk_value(local_irk);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n Setting the Local IRK Value Failed = %x \n", status);
    return status;
  }

  //! prepare advertise data //local/device name
  adv[3] = strlen(RSI_BLE_APP_GATT_TEST) + 1;
  adv[4] = 9;
  strcpy((char *)&adv[5], RSI_BLE_APP_GATT_TEST);

  //! set advertise data
  rsi_ble_set_advertise_data(adv, strlen(RSI_BLE_APP_GATT_TEST) + 5);
  uint8_t dummy_remote_dev_addr[6] = { 0x00, 0x00, 0x00, 0x11, 0x22, 0x33 };

#ifndef RSI_BLE_POWER_NUM_TEST
  status = rsi_ble_set_ble_tx_power(ADV_ROLE, dummy_remote_dev_addr, 22);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\nSET BLE TX POWER FOR ADV_ROLE FAILED : 0x%x\n", status);
  } else {
    LOG_PRINT("\nSET BLE TX POWER FOR ADV_ROLE SUCCESS : 0x%x\n", status);
  }
  status = rsi_ble_set_ble_tx_power(SCAN_AND_CENTRAL_ROLE, dummy_remote_dev_addr, 15);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\nSET BLE TX POWER FOR SCAN_AND_CENTRAL_ROLE FAILED : 0x%x\n", status);
  } else {
    LOG_PRINT("\nSET BLE TX POWER FOR SCAN_AND_CENTRAL_ROLE SUCCESS : 0x%x\n", status);
  }
  status = rsi_ble_set_ble_tx_power(PERIPHERAL_ROLE, dummy_remote_dev_addr, 10);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\nSET BLE TX POWER FOR PERIPHERAL_ROLE FAILED : 0x%x\n", status);
  } else {
    LOG_PRINT("\nSET BLE TX POWER FOR PERIPHERAL_ROLE SUCCESS : 0x%x\n", status);
  }
#endif
#if WRITE_TO_READONLY_CHAR_TEST
  //! start scanning
  status = rsi_ble_start_scanning();
  if (status != RSI_SUCCESS) {
    return status;
  }
  LOG_PRINT("\r\n scanning started \r\n");
#else
  //! set device in advertising mode.
  status = rsi_ble_start_advertising();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n Advertising failed \n");
    return status;
  }

  LOG_PRINT("\n Advertising started \n");
#endif
#if ENABLE_POWER_SAVE
  //! enable wlan radio
  status = rsi_wlan_radio_init();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n radio init failed \n");
    return status;
  }
  //! initiating power save in BLE mode
  status = rsi_bt_power_save_profile(PSP_MODE, PSP_TYPE);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Failed in initiating power save \r\n");
    return status;
  }
  //! initiating power save in wlan mode
  status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, PSP_TYPE);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Failed in initiating power save \r\n");
    return status;
  }
#endif
#if RSI_BLE_SANITY_TEST
#if SET_SMP_CONFIGURATION
  no_signing_keys_supported_capabilities.io_capability        = RSI_BLE_SMP_IO_CAPABILITY;
  no_signing_keys_supported_capabilities.oob_data_flag        = LOCAL_OOB_DATA_FLAG_NOT_PRESENT;
  no_signing_keys_supported_capabilities.auth_req             = AUTH_REQ_BITS;
  no_signing_keys_supported_capabilities.enc_key_size         = MAXIMUM_ENC_KEY_SIZE_16;
  no_signing_keys_supported_capabilities.ini_key_distribution = INITIATOR_KEYS_TO_DIST;
  no_signing_keys_supported_capabilities.rsp_key_distribution = RESPONDER_KEYS_TO_DIST;
  status = rsi_ble_set_smp_pairing_cap_data(&no_signing_keys_supported_capabilities);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("smp_pairing_cap_data cmd failed = %x", status);
    return status;
  }
#endif
#endif
  return 0;
}

int32_t rsi_ble_sanity_app_task()
{
  int32_t status                = 0;
  int32_t temp_event_map        = 0;
  static uint8_t prep_write_err = 0;
  //uuid_t service_uuid = { 0 };
  bool done_profiles_query         = false;
  static bool prof_resp_recvd      = false;
  static bool char_resp_recvd      = false;
  static bool char_desc_resp_recvd = false;
  uint8_t no_of_profiles = 0, total_remote_profiles = 0;
  uint8_t l_num_of_services = 0, l_char_property = 0;
  uint8_t profs_evt_cnt = 0, prof_evt_cnt = 0, char_for_serv_cnt = 0, char_desc_cnt = 0;
  uint8_t dev_addr[RSI_DEV_ADDR_LEN] = { 0 };
  uint8_t i = 0, profile_index_for_char_query = 0, temp1 = 0, temp2 = 0;
  uint16_t profiles_endhandle = 0;
  uuid_t search_serv          = { 0 };
  uint8_t read_data1[230]     = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 72, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86,
    87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 72,
    74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99
  };

#if CHECK_NOTIFICATIONS
  uint16_t notfy_cnt                       = 0;
  bool remote_dev_enabled_rsi_notfications = 0;
#endif
#if CHECK_NOTIFICATIONS_RX
  bool notify_handle_found              = 0;
  uint8_t notify_handle                 = 0;
  uint8_t data1[2]                      = { 1, 0 };
  bool rsi_enabled_remote_notifications = 0;
#endif
#if CHECK_INDICATIONS
  bool indication_handle_found = 0;
  uint8_t indication_handle    = 0;
  uint8_t indicate_data[2]     = { 2, 0 };
#elif (WRITE_TO_READONLY_CHAR_TEST)
  bool write_handle_found = 0;
  uint8_t write_handle    = 0;
#endif
#if RESOLVE_ENABLE
  rsi_ble_dev_ltk_list_t *ble_dev_ltk = NULL;
#endif

  //! disable the wlan radio
  status = rsi_wlan_radio_deinit();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n radio deinit failed \n");
  }

  //! BLE dual role Initialization
  status = rsi_ble_dual_role();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("BLE DUAL role init failed \n");
  }

  while (1) {

    //! checking for received events
    temp_event_map = rsi_ble_app_get_event();
    if (temp_event_map == RSI_FAILURE) {
      //! if events are not received loop will be continued.
      //! wait on events
      rsi_semaphore_wait(&ble_sanity_task_sem, 0);
      continue;
    }

    //! if any event is received, it will be served.
    switch (temp_event_map) {

#if WRITE_TO_READONLY_CHAR_TEST
      case RSI_APP_EVENT_ADV_REPORT: {
        LOG_PRINT("\n Advertise report received \n");
        if (conn_req_pending == 0) {
          if (num_of_conn_slaves < 1) {
#if (CONNECT_OPTION == CONN_BY_NAME)
            if (((strcmp((const char *)remote_name, (void *)RSI_REMOTE_DEVICE_NAME1)) == 0))
#else
            if ((!strcmp((char *)conn_dev_addr, RSI_BLE_DEV_1_ADDR)))
#endif
            {
              LOG_PRINT("\n Device found. Stop scanning \n");
              status = rsi_ble_stop_scanning();
              if (status != RSI_SUCCESS) {
                LOG_PRINT("\n stop scanning failed = %x \n", status);
                //return status;
              }
              LOG_PRINT("\n Connect command \n");
              status = rsi_ble_connect(remote_addr_type, (int8_t *)remote_dev_bd_addr);
#if 0
							status = rsi_ble_connect_with_params(remote_addr_type,
									(int8_t *) remote_dev_bd_addr,
									LE_SCAN_INTERVAL_CONN,
									LE_SCAN_WINDOW_CONN,
									M2S12_CONNECTION_INTERVAL_MAX,
									M2S12_CONNECTION_INTERVAL_MIN,
									M2S12_CONNECTION_LATENCY,
									M2S12_SUPERVISION_TIMEOUT);
							LOG_PRINT("\n connecting to device :  %s\n",(int8_t *) remote_dev_bd_addr);

#endif
              if (status != RSI_SUCCESS) {
                LOG_PRINT("\n Connecting failed with status : 0x%x\n", status);
                //return status;
                rsi_ble_app_set_event(RSI_BLE_SCAN_RESTART_EVENT);
              }
              memset(remote_name, 0, sizeof(remote_name));
              conn_req_pending = 1;
            }
          }
        }
        //! clear the advertise report event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_ADV_REPORT);
      } break;
#endif

      case RSI_BLE_CONN_EVENT: {
        //! event invokes when connection was completed
        LOG_PRINT("\r\n In on conn evt - m1\r\n");
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_CONN_EVENT);
#if WRITE_TO_READONLY_CHAR_TEST
        num_of_conn_slaves++;
        LOG_PRINT("\r\n Number of devices connected:%d\n", num_of_conn_slaves);
#else
        num_of_conn_masters++;
        LOG_PRINT("\r\n Number of devices connected:%d\n", num_of_conn_masters);
#endif
#if RSI_BLE_SANITY_TEST
        rsi_ble_app_set_event(RSI_BLE_REQ_GATT_PROFILE);
        if (RSI_BLE_MTU_EXCHANGE_FROM_HOST) {
          rsi_ascii_dev_address_to_6bytes_rev(dev_addr, (int8_t *)str_remote_address);

          status = rsi_ble_mtu_exchange_event(dev_addr, MAX_MTU_SIZE);
          if (status != 0) {
            LOG_PRINT("\r\n MTU Exchange request failed -m1\n");
          }
        }
#endif
      } break;
      case RSI_BLE_ENHC_CONN_EVENT: {

        LOG_PRINT("\r\n In on_enhance_conn evt - m1\r\n");
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_ENHC_CONN_EVENT);

#if WRITE_TO_READONLY_CHAR_TEST
        num_of_conn_slaves++;
        LOG_PRINT("\r\n Number of devices connected:%d\n", num_of_conn_slaves);
#else
        num_of_conn_masters++;
        LOG_PRINT("\r\n Number of devices connected:%d\n", num_of_conn_masters);
#endif
#if RSI_BLE_SANITY_TEST
        rsi_ble_app_set_event(RSI_BLE_REQ_GATT_PROFILE);

        if (RSI_BLE_MTU_EXCHANGE_FROM_HOST) {
          rsi_ascii_dev_address_to_6bytes_rev(dev_addr, (int8_t *)str_remote_address);

          status = rsi_ble_mtu_exchange_event(dev_addr, MAX_MTU_SIZE);
          if (status != 0) {
            LOG_PRINT("\r\n MTU Exchange request failed -m1\n");
          }
        }
#endif
      } break;
      case RSI_BLE_MTU_EXCHANGE_INFORMATION: {
        rsi_ble_app_clear_event(RSI_BLE_MTU_EXCHANGE_INFORMATION);
        LOG_PRINT("\r\n MTU EXCHANGE INFORMATION - in subtask -m1 \r\n");
        if ((mtu_exchange_info.initiated_role == PEER_DEVICE_INITATED_MTU_EXCHANGE)
            && (RSI_BLE_MTU_EXCHANGE_FROM_HOST)) {
          status = rsi_ble_mtu_exchange_resp(remote_dev_address, LOCAL_MTU_SIZE);
          //! check for procedure already in progress error
          if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
            master1_state |= BIT64(RSI_BLE_MTU_EXCHANGE_INFORMATION);
            LOG_PRINT("\r\n rsi_ble_mtu_exchange_resp procedure is already in progress -m1 \r\n");
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

        rsi_ble_app_clear_event(RSI_BLE_MORE_DATA_REQ_EVT);

        if (master1_state & BIT64(RSI_DATA_TRANSFER_EVENT)) {
          master1_state &= ~BIT64(RSI_DATA_TRANSFER_EVENT);
          rsi_ble_app_set_event(RSI_DATA_TRANSFER_EVENT);
        }
        if (master1_state & BIT64(RSI_BLE_REQ_GATT_PROFILE)) {
          master1_state &= ~BIT64(RSI_BLE_REQ_GATT_PROFILE);
          rsi_ble_app_set_event(RSI_BLE_REQ_GATT_PROFILE);
        }
        if (master1_state & BIT64(RSI_BLE_GATT_PROFILES)) {
          master1_state &= ~BIT64(RSI_BLE_GATT_PROFILES);
          rsi_ble_app_set_event(RSI_BLE_GATT_PROFILES);
        }
        if (master1_state & BIT64(RSI_BLE_GATT_PROFILE)) {
          master1_state &= ~BIT64(RSI_BLE_GATT_PROFILE);
          rsi_ble_app_set_event(RSI_BLE_GATT_PROFILE);
        }
        if (master1_state & BIT64(RSI_BLE_GATT_CHAR_SERVICES)) {
          master1_state &= ~BIT64(RSI_BLE_GATT_CHAR_SERVICES);
          rsi_ble_app_set_event(RSI_BLE_GATT_CHAR_SERVICES);
        }
        if (master1_state & BIT64(RSI_BLE_READ_REQ_EVENT)) {
          master1_state &= ~BIT64(RSI_BLE_READ_REQ_EVENT);
          rsi_ble_app_set_event(RSI_BLE_READ_REQ_EVENT);
        }
        if (master1_state & BIT64(RSI_BLE_BUFF_CONF_EVENT)) {
          master1_state &= ~BIT64(RSI_BLE_BUFF_CONF_EVENT);
          rsi_ble_app_set_event(RSI_BLE_BUFF_CONF_EVENT);
        }
      } break;
      case RSI_BLE_REQ_GATT_PROFILE: {
        if (master1_mtu_done
#if SMP_ENABLE
            && master1_smp_done
#endif
        ) {
          //
          rsi_ble_app_clear_event(RSI_BLE_REQ_GATT_PROFILE);
          //! get remote device profiles
          LOG_PRINT("\r\n remote device profile discovery started -m1 \r\n");
          status = rsi_ble_get_profiles_async(remote_dev_address, 1, 0xffff, NULL);
          if (status != RSI_SUCCESS) {
            //! check for procedure already in progress error
            if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
              master1_state |= BIT64(RSI_BLE_REQ_GATT_PROFILE);
              break;
            }
            //! check for buffer full error, which is not expected for this procedure
            else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
              LOG_PRINT("\r\n rsi_ble_get_profiles_async failed with buffer full error -m1 \r\n");
              break;
            } else {
              LOG_PRINT("\r\n get profile async call failed with error code :%x -m1 \r\n", status);
            }
          }
        }
      } break;
      case RSI_BLE_GATT_PROFILES: {
        //! prof_resp_recvd is set to false for every profile query response
        if (!prof_resp_recvd) {
          //! check until completion of first level query
          if (!done_profiles_query) {
            rsi_ble_app_clear_event(RSI_BLE_GATT_PROFILES);
            no_of_profiles = get_allprofiles.number_of_profiles;
            //! copy the end of handle of last searched profile
            profiles_endhandle = *(uint16_t *)get_allprofiles.profile_desc[no_of_profiles - 1].end_handle;

            //! copy retrieved profiles in local master buffer
            for (i = 0; i < no_of_profiles; i++) {
              memcpy(&rsi_ble_profile_list_by_conn[total_remote_profiles].profile_desc,
                     &get_allprofiles.profile_desc[i],
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
              status = rsi_ble_get_profiles_async(remote_dev_address, profiles_endhandle + 1, 0xffff, NULL);
              if (status != RSI_SUCCESS) {
                //! check for procedure already in progress error
                if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
                  master1_state |= BIT64(RSI_BLE_GATT_PROFILES);
                  break;
                }
                //! check for buffer full error, which is not expected for this procedure
                else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
                  LOG_PRINT("\r\n rsi_ble_get_profiles_async failed with buffer full error -m1 \r\n");
                  break;
                } else {
                  LOG_PRINT("\r\n get profile async call failed with error code :%x -m1 \r\n", status);
                }
              }
            } else {
              //! first level profile query completed
              done_profiles_query = true;
              //set event to start second level profile query
              rsi_ble_app_set_event(RSI_BLE_GATT_PROFILES);
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
              } else if (search_serv.size == 4) {
                search_serv.val.val32 = rsi_ble_profile_list_by_conn[profs_evt_cnt].profile_desc.profile_uuid.val.val32;
              } else if (search_serv.size == 16) //! 128 bit(16 byte) UUID value
              {
                search_serv.val.val128 =
                  rsi_ble_profile_list_by_conn[profs_evt_cnt].profile_desc.profile_uuid.val.val128;
              }
              status = rsi_ble_get_profile_async(remote_dev_address, search_serv, NULL);
              if (status != RSI_SUCCESS) {
                //! check for procedure already in progress error
                if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
                  prof_resp_recvd = false;
                  master1_state |= BIT64(RSI_BLE_GATT_PROFILES);
                  break;
                }
                //! check for buffer full error, which is not expected for this procedure
                else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
                  LOG_PRINT("\r\n rsi_ble_get_profiles_async failed with buffer full error -m1 \r\n");
                  break;
                } else {
                  LOG_PRINT("\r\n get profile async call failed with error code :%x -m1 \r\n", status);
                }
              } else {
                profs_evt_cnt++;
              }
            } else {
              //! second level of profile query completed
              rsi_ble_app_clear_event(RSI_BLE_GATT_PROFILES);
              rsi_ble_app_set_event(RSI_BLE_GATT_PROFILE);
            }
          }
        }

      } break;
      case RSI_BLE_GATT_PROFILE: {
        //copy total searched profiles in local buffer
        if (prof_evt_cnt < total_remote_profiles) {
          //! clear the served event
          rsi_ble_app_clear_event(RSI_BLE_GATT_PROFILE);
          //! copy to master buffer
          memcpy(&rsi_ble_profile_list_by_conn[prof_evt_cnt].profile_info_uuid,
                 &get_profile,
                 sizeof(rsi_ble_event_profile_by_uuid_t));
          prof_resp_recvd = false;
          prof_evt_cnt++;
        } else {
          if (!char_resp_recvd) {
            if (profile_index_for_char_query < total_remote_profiles) {
              char_resp_recvd = true;
              //! Get characteristic services of searched profile
              status = rsi_ble_get_char_services_async(
                remote_dev_address,
                *(uint16_t *)rsi_ble_profile_list_by_conn[profile_index_for_char_query].profile_info_uuid.start_handle,
                *(uint16_t *)rsi_ble_profile_list_by_conn[profile_index_for_char_query].profile_info_uuid.end_handle,
                NULL);
              if (status != RSI_SUCCESS) {
                //! check for procedure already in progress error
                if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
                  char_resp_recvd = false;
                  master1_state |= BIT64(RSI_BLE_GATT_PROFILE);
                  break;
                }
                //! check for buffer full error, which is not expected for this procedure
                else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
                  LOG_PRINT("\r\n rsi_ble_get_char_services_async failed with buffer full error -m1 \r\n");
                  break;
                } else {
                  LOG_PRINT(
                    "\r\n failed to get service characteristics of the remote GATT server with error:0x%x -m1 \r\n",
                    status);
                }
              }
              profile_index_for_char_query++;
            } else {
              //! discovery of complete characteristics in each profile is completed
              rsi_ble_app_clear_event(RSI_BLE_GATT_PROFILE);
              rsi_ble_app_set_event(RSI_BLE_GATT_CHAR_SERVICES);
            }
          }
        }
      } break;
      case RSI_BLE_GATT_CHAR_SERVICES: {
        if (char_for_serv_cnt < total_remote_profiles) {
          rsi_ble_app_clear_event(RSI_BLE_GATT_CHAR_SERVICES);

          //! copy total no of characteristic services for each profile
          rsi_ble_profile_list_by_conn[char_for_serv_cnt].no_of_char_services = get_char_services.num_of_services;
          if (rsi_ble_profile_list_by_conn[char_for_serv_cnt].no_of_char_services > RSI_BLE_MAX_RESP_LIST) {
            rsi_ble_profile_list_by_conn[char_for_serv_cnt].no_of_char_services = RSI_BLE_MAX_RESP_LIST;
          }
          for (uint8_t ix = 0; ix < rsi_ble_profile_list_by_conn[char_for_serv_cnt].no_of_char_services; ix++) {
            memcpy(&rsi_ble_profile_list_by_conn[char_for_serv_cnt].profile_char_info[ix],
                   &get_char_services,
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
              ;
              //! search for all characteristics descriptor in each profile
              if (temp2 < l_num_of_services) {
                l_char_property = rsi_ble_profile_list_by_conn[temp1]
                                    .profile_char_info[temp2]
                                    .char_services[temp2]
                                    .char_data.char_property;
                if ((l_char_property == RSI_BLE_ATT_PROPERTY_INDICATE)
                    || (l_char_property == RSI_BLE_ATT_PROPERTY_NOTIFY)) {
                  //LOG_PRINT("\n query for profile service1 %d -m1 \n",temp1);
                  status = rsi_ble_get_att_value_async(
                    remote_dev_address,
                    rsi_ble_profile_list_by_conn[temp1].profile_char_info[temp2].char_services[temp2].handle,
                    NULL);

                  if (status != RSI_SUCCESS) {
                    //! check for procedure already in progress error
                    if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
                      char_desc_resp_recvd = false;
                      master1_state |= BIT64(RSI_BLE_GATT_CHAR_SERVICES);
                      break;
                    }
                    //! check for buffer full error, which is not expected for this procedure
                    else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
                      LOG_PRINT("\r\n rsi_ble_get_att_value_async failed with buffer full error -m1 \r\n");
                      break;
                    } else {
                      LOG_PRINT("\r\n failed to get characteristics descriptor of the remote GATT server with "
                                "error:0x%x -m1 \r\n",
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
              }
            }
          }
          //! discovering completed for all profiles
          else if (temp1 == total_remote_profiles) {
            rsi_ble_app_clear_event(RSI_BLE_GATT_CHAR_SERVICES);
            rsi_ble_app_set_event(RSI_BLE_GATT_DESC_SERVICES);
          }
        }
      } break;

      case RSI_BLE_GATT_DESC_SERVICES: {

        if (temp1 < total_remote_profiles) {
          temp2++;
          rsi_ble_app_clear_event(RSI_BLE_GATT_DESC_SERVICES);
          char_desc_cnt++;
          char_desc_resp_recvd = false;
        } else {

          rsi_ble_app_clear_event(RSI_BLE_GATT_DESC_SERVICES);
          LOG_PRINT("\n remote device profiles discovery completed -m1 \n");
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
                  LOG_PRINT("\n indicate handle found -m1 \n");
                  indication_handle_found = true;
                  indication_handle =
                    rsi_ble_profile_list_by_conn[i].profile_char_info[ix].char_services[ix].char_data.char_handle;
                  //! configure the buffer configuration mode
                  rsi_ble_app_set_event(RSI_BLE_BUFF_CONF_EVENT);
                  break;
                }
              }
            }
          }
          if (!indication_handle_found) {
            LOG_PRINT("\n Health Thermometer profile not found -m1 \n");
          }

#elif (WRITE_TO_READONLY_CHAR_TEST)
          //! check for heart rate profile
          for (i = 0; i < total_remote_profiles; i++) {
            if (rsi_ble_profile_list_by_conn[i].profile_desc.profile_uuid.val.val16 == 0x180D) {
              for (uint8_t ix = 0; ix < rsi_ble_profile_list_by_conn[i].no_of_char_services; ix++) {
                if ((!write_handle_found)
                    && ((rsi_ble_profile_list_by_conn[i]
                           .profile_char_info[ix]
                           .char_services[ix]
                           .char_data.char_property)
                        == RSI_BLE_ATT_PROPERTY_READ)) {
                  LOG_PRINT("\n read handle found -m1 \n");
                  write_handle_found = true;
                  write_handle =
                    rsi_ble_profile_list_by_conn[i].profile_char_info[ix].char_services[ix].char_data.char_handle;
                  //! configure the buffer configuration mode
                  rsi_ble_app_set_event(RSI_BLE_BUFF_CONF_EVENT);
                  break;
                }
              }
            }
          }
          if (!write_handle_found) {
            LOG_PRINT("\n Heart rate profile not found -m1 \n");
          }
#elif CHECK_NOTIFICATIONS_RX
          //! check for required service
          for (i = 0; i < total_remote_profiles; i++) {
            if (rsi_ble_profile_list_by_conn[i].profile_desc.profile_uuid.val.val16 == ENABLE_NOTIFICATION_SERVICE) {
              for (uint8_t ix = 0; ix < rsi_ble_profile_list_by_conn[i].no_of_char_services; ix++) {
                //! check in ANCS profile
                if ((!notify_handle_found)
                    && (rsi_ble_profile_list_by_conn[i]
                          .profile_char_info[ix]
                          .char_services[ix]
                          .char_data.char_uuid.val.val32
                        == ENABLE_NOTIFICATION_UUID)
                    && ((rsi_ble_profile_list_by_conn[i]
                           .profile_char_info[ix]
                           .char_services[ix]
                           .char_data.char_property)
                        & RSI_BLE_ATT_PROPERTY_NOTIFY)) {
                  LOG_PRINT("\n Notify handle found -m1 \n");
                  notify_handle_found = true;
                  notify_handle =
                    rsi_ble_profile_list_by_conn[i].profile_char_info[ix].char_services[ix].char_data.char_handle;
                  break;
                }
              }
            }
          }
          if (!notify_handle_found) {
            LOG_PRINT("\n Profile not found to enable notifications on remote device \n");
          } else {
            LOG_PRINT("\n Profile found to enable notifications on remote device \n");
            rsi_enabled_remote_notifications = 1; //! Enabling notifications to send based on the value
            rsi_ble_app_set_event(RSI_BLE_BUFF_CONF_EVENT);
          }

#else
          //! do nothing
#endif
        }
      } break;
      case RSI_BLE_BUFF_CONF_EVENT: {
        status = rsi_ble_set_wo_resp_notify_buf_info(remote_dev_address, DLE_BUFFER_MODE, DLE_BUFFER_COUNT);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n failed to set the buffer configuration mode, error:0x%x -m1 \r\n", status);
        } else {
          rsi_ble_app_clear_event(RSI_BLE_BUFF_CONF_EVENT);
          rsi_ble_app_set_event(RSI_DATA_TRANSFER_EVENT);
        }
      } break;
      case RSI_BLE_RECEIVE_REMOTE_FEATURES: {

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_RECEIVE_REMOTE_FEATURES);
#if !RSI_BLE_POWER_NUM_TEST
#if SMP_ENABLE
        if (master1_mtu_done) {

          //! initiating the SMP pairing process
          if ((!smp_pairing_initated) && (!master1_smp_done)) {
            status = rsi_ble_smp_pair_request(remote_dev_address, RSI_BLE_SMP_IO_CAPABILITY, MITM_ENABLE);

            if (status != RSI_SUCCESS) {
              LOG_PRINT("\r\n start of SMP pairing process failed with error code %x -m1 \r\n", status);
            }
            smp_pairing_initated = 1;
          }
        }
#endif
#else

        status =
          rsi_ble_conn_params_update(remote_dev_address, CONNECTION_INTERVAL_MIN, CONNECTION_INTERVAL_MAX, 0, 400);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n connection update failed \n");
          return status;
        }
#endif
      } break;
      case RSI_BLE_CONN_UPDATE_COMPLETE_EVENT: {

        rsi_ble_app_clear_event(RSI_BLE_CONN_UPDATE_COMPLETE_EVENT);
        //! convert to ascii
        rsi_6byte_dev_address_to_ascii(dev_addr, rsi_event_conn_update.dev_addr);
        LOG_PRINT("\n conn updated device address : %s\n conn_interval:%d\n supervision timeout:%d -m1",
                  dev_addr,
                  rsi_event_conn_update.conn_interval,
                  rsi_event_conn_update.timeout);
#if UPDATE_CONN_PARAMETERS
        status = rsi_conn_update_request();
#endif
      } break;
      case RSI_BLE_DISCONN_EVENT: {

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_DISCONN_EVENT);
#if RSI_BLE_POWER_NUM_TEST
        status = rsi_ble_start_advertising();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n advertising failed in disconnect event \n");
          //return status;
        }
#else
        rsi_ble_app_clear_event(RSI_DATA_TRANSFER_EVENT);

        rsi_app_async_event_map             = 0;
#if CHECK_NOTIFICATIONS_RX
        rsi_enabled_remote_notifications    = 0;
#endif
#if CHECK_NOTIFICATIONS
        remote_dev_enabled_rsi_notfications = 0;
#endif
#if CHECK_INDICATIONS
        indication_handle                   = 0;
        indication_handle_found             = false;
#elif (WRITE_TO_READONLY_CHAR_TEST)
        write_handle       = 0;
        write_handle_found = false;
#endif
        master1_state                       = 0;
        master1_mtu_done                    = 0;
        prof_resp_recvd                     = false;
        done_profiles_query                 = false;
        char_resp_recvd                     = false;
        char_desc_resp_recvd                = false;
        profile_index_for_char_query        = 0;
        prof_evt_cnt                        = 0;
        profs_evt_cnt                       = 0;
        char_for_serv_cnt                   = 0;
        master1_state                       = 0;
        //! clear the profile data
        for (uint8_t i = 0; i < total_remote_profiles; i++) {
          memset(&rsi_ble_profile_list_by_conn[i], 0, sizeof(rsi_ble_profile_list_by_conn_t));
        }
        total_remote_profiles = 0;
        memset(remote_dev_address, 0, RSI_DEV_ADDR_LEN);
#if WRITE_TO_READONLY_CHAR_TEST
        LOG_PRINT("\r\n slave is disconnected -m1 \r\n");

        device_found     = 0;
        conn_done        = 0;
        conn_req_pending = 0;

        if (num_of_conn_slaves) {
          num_of_conn_slaves--;
        }
        LOG_PRINT("\n Device disconnected - m1\n ");
        LOG_PRINT("\n Number of connected devices:%d\n", num_of_conn_slaves);
        rsi_ble_app_set_event(RSI_BLE_SCAN_RESTART_EVENT);
#else
        LOG_PRINT("\r\n master is disconnected -m1 \r\n");

        if (num_of_conn_masters) {
          num_of_conn_masters--;
        }
        LOG_PRINT("\n Device disconnected - m1\n ");
        LOG_PRINT("\n Number of connected devices:%d\n", num_of_conn_masters);

        LOG_PRINT("\r\n In dis-conn evt, Start Adv -m1 \r\n");
#if RESOLVE_ENABLE
        if (master1_smp_done) {
          rsi_ble_req_adv_t adv_params = { 0 };
          adv_params.status            = RSI_BLE_START_ADV;
          adv_params.adv_type          = UNDIR_CONN;

          adv_params.filter_type      = RSI_BLE_ADV_FILTER_TYPE;
          adv_params.direct_addr_type = resolve_key.Identity_addr_type;

          rsi_ascii_dev_address_to_6bytes_rev(adv_params.direct_addr, resolve_key.Identity_addr);
          adv_params.adv_int_min = RSI_BLE_ADV_INT_MIN;
          adv_params.adv_int_max = RSI_BLE_ADV_INT_MAX;

          adv_params.own_addr_type =
            LE_RESOLVABLE_PUBLIC_ADDRESS; //LE_RESOLVABLE_PUBLIC_ADDRESS - working  //LE_RESOLVABLE_RANDOM_ADDRESS - first pairing success, subsequent conn fail

          adv_params.adv_channel_map = RSI_BLE_ADV_CHANNEL_MAP;

          status = rsi_ble_start_advertising_with_values(&adv_params);
          LOG_PRINT("\n Advertising with resolve addr \n");
        } else {
          status = rsi_ble_start_advertising();
        }
#else
        status = rsi_ble_start_advertising();
#endif
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n advertising with values failed in disconn event = 0x%x -m1 \n", status);
        }
#endif
#endif
        master1_smp_done     = 0;
        smp_pairing_initated = 0;
      } break;
      case RSI_BLE_GATT_WRITE_EVENT: {
        //! event invokes when write/notification events received

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_GATT_WRITE_EVENT);

        if (*(uint16_t *)app_ble_write_event.handle == rsi_ble_att1_val_hndl) {
          rsi_ble_att_list_t *attribute = NULL;
          uint8_t opcode = 0x12, err = 0x00;
          attribute = rsi_gatt_get_attribute_from_list(&att_list, *(uint16_t *)app_ble_write_event.handle);

          //! Check if value has write properties
          if ((attribute != NULL) && (attribute->value != NULL)) {
            if (!(attribute->char_val_prop & 0x08)) //! If no write property, send error response
            {
              err = 0x03; //! Error - Write not permitted
            }
          } else {
            //!Error = No such handle exists
            err = 0x01;
          }

          //! Update the value based6 on the offset and length of the value
          if ((err == 0) && ((app_ble_write_event.length) <= attribute->max_value_len)) {
            memset(attribute->value, 0, attribute->max_value_len);

            //! Check if value exists for the handle. If so, maximum length of the value.
            memcpy(attribute->value, app_ble_write_event.att_value, app_ble_write_event.length);

            //! Update value length
            attribute->value_len = app_ble_write_event.length;

            //! Send gatt write response
            rsi_ble_gatt_write_response(app_ble_write_event.dev_addr, 0);
          } else {
            //! Error : 0x07 - Invalid request,  0x0D - Invalid attribute value length
            err = 0x07;
          }

          if (err) {
            //! Send error response
            rsi_ble_att_error_response(app_ble_write_event.dev_addr,
                                       *(uint16_t *)app_ble_write_event.handle,
                                       opcode,
                                       err);
          }
        }

#if CHECK_INDICATIONS
        //! Check whether received write is indication and acknowledge it
        if (*(uint16_t *)app_ble_write_event.handle == (indication_handle)) {
#if RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
          //! Send indication acknowledgement to remote device
          status = rsi_ble_indicate_confirm(remote_dev_address);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\n indication confirm failed \t reason = %x -m1\n", status);
          } else {
            LOG_PRINT("\n indication confirm response sent -m1\n");
          }
#endif
        }
#elif (WRITE_TO_READONLY_CHAR_TEST)
        //! Check whether received write is indication and acknowledge it
        if (*(uint16_t *)app_ble_write_event.handle == (write_handle)) {
          LOG_PRINT("\n write confirm response sent -m1\n");
        }
#elif CHECK_NOTIFICATIONS
        //check for valid notifications
        if ((app_ble_write_event.att_value[0] == NOTIFY_ENABLE)
            && ((*(uint16_t *)(app_ble_write_event.handle) - 1) == rsi_ble_att1_val_hndl)) {
          remote_dev_enabled_rsi_notfications = 1;
          LOG_PRINT("\n Remote device enabled the notification -m1\n");
          //! configure the buffer configuration mode
          rsi_ble_app_set_event(RSI_BLE_BUFF_CONF_EVENT);
        } else if ((app_ble_write_event.att_value[0] == NOTIFY_DISABLE)
                   && ((*(uint16_t *)(app_ble_write_event.handle) - 1)
                       == rsi_ble_att1_val_hndl /*rsi_ble_att1_val_hndl*/)) {
          remote_dev_enabled_rsi_notfications = 0;
          LOG_PRINT("\n Remote device disabled the notification -m1\n");
          rsi_ble_app_clear_event(RSI_DATA_TRANSFER_EVENT);
        }
#endif
#if CHECK_NOTIFICATIONS_RX
        if ((*(uint16_t *)app_ble_write_event.handle) == notify_handle) {
          //! print received notifications
          LOG_PRINT("\n received notification data from remote device\n");
        }
#endif
      } break;

      case RSI_BLE_GATT_PREPARE_WRITE_EVENT: {
        LOG_PRINT("\nPWE\n");
        uint8_t err = 0;
        rsi_ble_app_clear_event(RSI_BLE_GATT_PREPARE_WRITE_EVENT);
        if (*(uint16_t *)app_ble_prepared_write_event.handle == rsi_ble_att1_val_hndl) {
          rsi_ble_att_list_t *attribute = NULL;
          uint8_t opcode                = 0x16;
          attribute = rsi_gatt_get_attribute_from_list(&att_list, *(uint16_t *)app_ble_prepared_write_event.handle);

          //! Check if value has write properties
          if ((attribute != NULL) && (attribute->value != NULL)) {
            if (!(attribute->char_val_prop & 0x08)) //! If no write property, send error response
            {
              err = 0x03; //! Error - Write not permitted
            }
          } else {
            //!Error = No such handle exists
            err = 0x01;
          }

          if (err) {
            //! Send error response
            rsi_ble_att_error_response(app_ble_prepared_write_event.dev_addr,
                                       *(uint16_t *)app_ble_prepared_write_event.handle,
                                       opcode,
                                       err);
            break;
          }

          //! Update the value based6 on the offset and length of the value
          if ((err == 0) && ((*(uint16_t *)app_ble_prepared_write_event.offset) <= attribute->max_value_len)) {
            //LOG_PRINT("PWE - offset : %d\n",(*(uint16_t *)app_ble_prepared_write_event.offset));
            //! Hold the value to update it
            memcpy(&temp_prepare_write_value[temp_prepare_write_value_len],
                   app_ble_prepared_write_event.att_value,
                   app_ble_prepared_write_event.length);
            temp_prepare_write_value_len += app_ble_prepared_write_event.length;
          } else {
            //! Error : 0x07 - Invalid offset,  0x0D - Invalid attribute value length
            prep_write_err = 0x07;
          }
          //! Send gatt write response
          rsi_ble_gatt_prepare_write_response(app_ble_prepared_write_event.dev_addr,
                                              *(uint16_t *)app_ble_prepared_write_event.handle,
                                              (*(uint16_t *)app_ble_prepared_write_event.offset),
                                              app_ble_prepared_write_event.length,
                                              app_ble_prepared_write_event.att_value);
        }
      } break;
      case RSI_BLE_GATT_EXECUTE_WRITE_EVENT: {
        LOG_PRINT("\nEWE\n");
        rsi_ble_app_clear_event(RSI_BLE_GATT_EXECUTE_WRITE_EVENT);
        if (*(uint16_t *)app_ble_prepared_write_event.handle == rsi_ble_att1_val_hndl) {
          rsi_ble_att_list_t *attribute = NULL;
          uint8_t opcode = 0x18, err = 0x00;
          attribute = rsi_gatt_get_attribute_from_list(&att_list, *(uint16_t *)app_ble_prepared_write_event.handle);

          //! Check if value has write properties
          if ((attribute != NULL) && (attribute->value != NULL)) {
            if (!(attribute->char_val_prop & 0x08)) //! If no write property, send error response
            {
              err = 0x03; //! Error - Write not permitted
            }
          } else {
            //!Error = No such handle exists
            err = 0x01;
          }

          //! Update the value based on the offset and length of the value
          if ((!err) && (app_ble_execute_write_event.exeflag == 0x1)
              && (temp_prepare_write_value_len <= attribute->max_value_len) && !prep_write_err) {
            //! Hold the value to update it
            memcpy((uint8_t *)attribute->value, temp_prepare_write_value, temp_prepare_write_value_len);
            attribute->value_len = temp_prepare_write_value_len;

            //! Send gatt write response
            rsi_ble_gatt_write_response(app_ble_execute_write_event.dev_addr, 1);
          } else {
            err = 0x0D; //Invalid attribute value length
          }
          if (prep_write_err) {
            //! Send error response
            rsi_ble_att_error_response(app_ble_execute_write_event.dev_addr,
                                       *(uint16_t *)app_ble_prepared_write_event.handle,
                                       opcode,
                                       prep_write_err);
          } else if (err) {
            //! Send error response
            rsi_ble_att_error_response(app_ble_execute_write_event.dev_addr,
                                       *(uint16_t *)app_ble_prepared_write_event.handle,
                                       opcode,
                                       err);
          }
          prep_write_err = 0;
          err            = 0;
          memset(temp_prepare_write_value, 0, temp_prepare_write_value_len);
          temp_prepare_write_value_len = 0;
        }
      } break;

      case RSI_DATA_TRANSFER_EVENT: {

#if CHECK_INDICATIONS
        LOG_PRINT("\n in indication event -m1 \n");
        rsi_ble_app_clear_event(RSI_DATA_TRANSFER_EVENT);
        status = rsi_ble_set_att_value_async(remote_dev_address, //enable the indications
                                             indication_handle + 1,
                                             2,
                                             indicate_data);
        if (status != RSI_SUCCESS) {
          if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
            LOG_PRINT("\r\n indication failed with buffer error -m1 \r\n");
            master1_state |= BIT64(RSI_DATA_TRANSFER_EVENT);
            break;
          } else {
            LOG_PRINT("\r\n indication failed with status = %x -m1 \r\n", status);
          }
        }
#elif (WRITE_TO_READONLY_CHAR_TEST)
        LOG_PRINT("\n in write with response event -m1 \n");
        rsi_ble_app_clear_event(RSI_DATA_TRANSFER_EVENT);
        LOG_PRINT("\n writing to read only characteristic \n");
        status = rsi_ble_set_att_value_async(remote_dev_address, write_handle, 2, read_data1);
        if (status != RSI_SUCCESS) {
          if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
            LOG_PRINT("\r\n write with response failed with buffer error -m1 \r\n");
            master1_state |= BIT64(RSI_DATA_TRANSFER_EVENT);
            break;
          } else {
            LOG_PRINT("\r\n write with response failed with status = %x -m1 \r\n", status);
          }
        }
#elif CHECK_NOTIFICATIONS
        if (remote_dev_enabled_rsi_notfications) {
          //! prepare the data to set as local attribute value.
          read_data1[0] = notfy_cnt;
          read_data1[1] = notfy_cnt >> 8;

          //! set the local attribute value.
          status = rsi_ble_notify_value(remote_dev_address,
                                        rsi_ble_att1_val_hndl,
                                        RSI_BLE_MAX_DATA_LEN,
                                        (uint8_t *)read_data1);
          if (status != RSI_SUCCESS) {
            if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
              rsi_ble_app_clear_event(RSI_DATA_TRANSFER_EVENT);
              master1_state |= BIT64(RSI_DATA_TRANSFER_EVENT);
              break;
            } else if (status == RSI_ERROR_IN_BUFFER_ALLOCATION) //! TO-DO, add proper error code
            {
              LOG_PRINT("\n cannot transmit %d bytes in small buffer configuration mode -m1\n", RSI_BLE_MAX_DATA_LEN);
              rsi_ble_disconnect(remote_dev_address);
              break;
            } else {
              LOG_PRINT("\n notify %d failed with error code %x  -m1\n", notfy_cnt, status);
            }
          } else {
            notfy_cnt++;
          }

          if (notfy_cnt == 3) //! Updating connection parameters after sending 3 notifications
          {
#if RSI_CONNECTION_UPDATE_REQ
            //! connection paramater request for 45ms
            status = rsi_ble_conn_params_update(remote_dev_address, 36, 36, 0, 400);
            if (status != RSI_SUCCESS) {
              //! check for procedure already in progress error
              if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
                master1_state |= BIT64(RSI_DATA_TRANSFER_EVENT);
                LOG_PRINT("\r\n rsi_ble_conn_params_update procedure is already in progress -m1 \r\n");
                break;
              }
              //! check for buffer full error, which is not expected for this procedure
              else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
#if RSI_DEBUG_EN
                LOG_PRINT_D("\r\n once_connection_update_done failed with buffer full error -m1 \r\n");
#endif
              } else {
                LOG_PRINT("\r\n failed to update connection paramaters error:0x%x -m1 \r\n", status);
              }
            } else {
              LOG_PRINT("\n connection params request was successfull -m1 \n");
            }
#endif //! end of RSI_CONNECTION_UPDATE_REQ

#if RSI_PROFILE_QUERY_AGAIN
            LOG_PRINT("\n start remote device profiles discovery again -m1 \n");
            //! start profile query procedure for every 256 notifications
            rsi_ble_app_set_event(RSI_BLE_REQ_GATT_PROFILE);
#endif
          }
        }
#endif
#if CHECK_NOTIFICATIONS_RX
        if (rsi_enabled_remote_notifications) {
          status = rsi_ble_set_att_value(remote_dev_address, //enable the notifications
                                         notify_handle + 1,
                                         2,
                                         data1);
          if (status != RSI_SUCCESS) {
            if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
              LOG_PRINT("\r\n notify failed with buffer error -m1 \r\n");
              break;
            } else {
              LOG_PRINT("\r\n notify value failed with status = %x -m1 \r\n", status);
            }
          } else {
            LOG_PRINT("\n rsi enabled the remote device notifications \n");
            rsi_enabled_remote_notifications = 0;
          }
        }
#endif
      } break;

      case RSI_BLE_READ_REQ_EVENT: {
        //! event invokes when write/notification events received

        LOG_PRINT("\n Read request initiated by remote device -m1 \n");
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_READ_REQ_EVENT);

        if (app_ble_read_event.handle == rsi_ble_att1_val_hndl) {
          rsi_ble_att_list_t *attribute = NULL;
          uint8_t opcode = 0x12, err = 0x00;
          attribute = rsi_gatt_get_attribute_from_list(&att_list, app_ble_read_event.handle);

          //! Check if value has write properties
          if ((attribute != NULL) && (attribute->value != NULL)) {
            if (!(attribute->char_val_prop & 0x02)) //! If no read property, send error response
            {
              err = 0x02; //! Error - read not permitted
            }
          } else {
            //!Error = No such handle exists
            err = 0x01;
          }

          if ((!err) && (app_ble_read_event.type == 1)) {

            //! Check whether the offset is less than or equal to value length
            if (app_ble_read_event.offset <= attribute->value_len) {
              rsi_ble_gatt_read_response(app_ble_read_event.dev_addr,
                                         1,
                                         app_ble_read_event.handle,
                                         app_ble_read_event.offset,
                                         (attribute->value_len - app_ble_read_event.offset),
                                         (uint8_t *)attribute->value + app_ble_read_event.offset);
              app_ble_read_event.offset = 0;
            } else {
              err = 0x07; //! Invalid offset, invalid request
            }
          } else {

            rsi_ble_gatt_read_response(app_ble_read_event.dev_addr,
                                       0,
                                       app_ble_read_event.handle,
                                       0,
                                       attribute->value_len,
                                       attribute->value);
          }

          if (err) {
            //! Send error response
            rsi_ble_att_error_response(app_ble_read_event.dev_addr, app_ble_read_event.handle, opcode, err);
          }
        } else {

          if (app_ble_read_event.type == 1) {
            status                    = rsi_ble_gatt_read_response(remote_dev_address,
                                                1,
                                                app_ble_read_event.handle,
                                                app_ble_read_event.offset,
                                                (sizeof(read_data1) - app_ble_read_event.offset),
                                                &(read_data1[app_ble_read_event.offset]));
            app_ble_read_event.offset = 0;
          } else {

            status = rsi_ble_gatt_read_response(remote_dev_address,
                                                0,
                                                app_ble_read_event.handle,
                                                0,
                                                (sizeof(read_data1)),
                                                read_data1);
          }
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n read response failed, error:0x%x -m1 \r\n", status);
          } else {
            LOG_PRINT("\n response to read request initiated by remote device was successfull -m1 \n");
          }
        }

      } break;
      case RSI_BLE_MTU_EVENT: {
        //! event invokes when write/notification events received

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_MTU_EVENT);
        master1_mtu_done = 1;
#if !RSI_BLE_POWER_NUM_TEST
#if SMP_ENABLE
        rsi_ble_app_set_event(RSI_BLE_RECEIVE_REMOTE_FEATURES);
#endif
#endif

      } break;

#if WRITE_TO_READONLY_CHAR_TEST
      case RSI_BLE_SCAN_RESTART_EVENT: {
        //! clear the served event
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SCAN_RESTART_EVENT);

        status = rsi_ble_stop_scanning();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n stop scanning failed = %x \n", status);
          //return status;
        }

        LOG_PRINT("\n Start scanning \n");
        rsi_app_no_of_adv_reports_rcvd = 0;
        status                         = rsi_ble_start_scanning();
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n start scanning failed = %x \n", status);
          rsi_ble_app_set_event(RSI_BLE_SCAN_RESTART_EVENT);
          //return status;
        } else {
          LOG_PRINT("\n scanning started \n");
          rsi_ble_app_clear_event(RSI_BLE_SCAN_RESTART_EVENT);
        }
      } break;
#endif

      case RSI_APP_EVENT_REMOTE_CONN_PARAM_REQ: {

        //!default ACCEPT the remote conn params request (0-ACCEPT, 1-REJECT)
        status = rsi_ble_conn_param_resp(rsi_app_remote_device_conn_params.dev_addr, 0);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("conn param resp status: 0x%X\r\n", status);
        }
        //! remote device conn params request
        //! clear the conn params request event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_REMOTE_CONN_PARAM_REQ);

      } break;
      case RSI_APP_EVENT_DATA_LENGTH_CHANGE: {
        //! clear the disconnected event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_DATA_LENGTH_CHANGE);

        if (remote_dev_feature.remote_features[1] & 0x01) {
          status = rsi_ble_setphy((int8_t *)remote_dev_feature.dev_addr, TX_PHY_RATE, RX_PHY_RATE, CODDED_PHY_RATE);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n failed to set phy length : 0x%x \r\n -m1", status);
          }
        }
      } break;

      case RSI_APP_EVENT_PHY_UPDATE_COMPLETE: {
        //! phy update complete event

        //! clear the phy updare complete event.
        rsi_ble_app_clear_event(RSI_APP_EVENT_PHY_UPDATE_COMPLETE);
      } break;

      case RSI_BLE_SMP_REQ_EVENT: {
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_REQ_EVENT);

        LOG_PRINT("\r\n in smp request \r\n -m1 \r\n");

        if (first_connect == 0) {
          //! initiating the SMP pairing process
          if ((!smp_pairing_initated) && (!master1_smp_done)) {
            status = rsi_ble_smp_pair_request(rsi_ble_event_smp_req.dev_addr, RSI_BLE_SMP_IO_CAPABILITY, MITM_ENABLE);
            if (status != RSI_SUCCESS) {
              LOG_PRINT("\r\n RSI_BLE_SMP_REQ_EVENT: failed to initiate the SMP pairing process: 0x%x \r\n -m1",
                        status);
            }
            smp_pairing_initated = 1;
          }

        } else {
          status = rsi_ble_start_encryption(rsi_ble_event_smp_req.dev_addr,
                                            rsi_encryption_enabled.localediv,
                                            rsi_encryption_enabled.localrand,
                                            rsi_encryption_enabled.localltk);
        }
      } break;

      case RSI_BLE_SMP_RESP_EVENT: {

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_RESP_EVENT);

        LOG_PRINT("\r\n in smp response -m1 \r\n");

        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_response(rsi_ble_event_smp_resp.dev_addr, RSI_BLE_SMP_IO_CAPABILITY, MITM_ENABLE);
      } break;

      case RSI_BLE_SMP_PASSKEY_EVENT: {

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_PASSKEY_EVENT);
        LOG_PRINT("\n in smp_passkey - str_remote_address : %s\r\n", remote_dev_address);

        //! initiating the SMP pairing process
        status = rsi_ble_smp_passkey(remote_dev_address, RSI_BLE_APP_SMP_PASSKEY);
      } break;
      case RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT: {
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_PASSKEY_DISPLAY_EVENT);

        LOG_PRINT("\nremote addr: %s, passkey: %s \r\n", remote_dev_address, rsi_ble_smp_passkey_display.passkey);
      } break;
      case RSI_BLE_LTK_REQ_EVENT: {
        //! event invokes when disconnection was completed

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_LTK_REQ_EVENT);

        LOG_PRINT("\r\n in LTK  request \r\n");
#if RESOLVE_ENABLE
        int8_t ix1 = rsi_get_ltk_list(ble_dev_ltk_list, rsi_le_ltk_resp);

        LOG_PRINT("\n LTK req index = %d \n", status);

        ble_dev_ltk = &ble_dev_ltk_list[ix1];
        if ((ix1 != -1) && (ble_dev_ltk != NULL)) {
          LOG_PRINT("\n positive reply\n");
          //! give le ltk req reply cmd with positive reply
          status = rsi_ble_ltk_req_reply(rsi_le_ltk_resp.dev_addr,
                                         (1 | (ble_dev_ltk->enc_enable) | (ble_dev_ltk->sc_enable << 7)),
                                         rsi_encryption_enabled.localltk);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n failed to restart smp pairing with status: 0x%x -m1\r\n", status);
          }
        } else
#endif
        {
          LOG_PRINT("\n negative reply\n");
          //! give le ltk req reply cmd with negative reply
          status = rsi_ble_ltk_req_reply(rsi_le_ltk_resp.dev_addr, 0, NULL);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n failed to restart smp pairing with status: 0x%x \r\n", status);
          }
        }
      } break;

      case RSI_BLE_SC_PASSKEY_EVENT: {
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SC_PASSKEY_EVENT);

        LOG_PRINT("\r\n in smp sc passkey event -m1 \r\n");

        rsi_ble_smp_passkey(remote_dev_address, passkey);
      } break;

      case RSI_BLE_SECURITY_KEYS_EVENT: {
        //! event invokes when security keys are received
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SECURITY_KEYS_EVENT);

        LOG_PRINT("\r\n in smp security keys event  -m1 \r\n");

#if RESOLVE_ENABLE

        status = add_security_keys_to_device_list(ble_dev_ltk_list, temp_le_sec_keys);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Failed to add Security keys to list \n");
        }

        resolve_key.remote_dev_addr_type = rem_dev_address_type;
        resolve_key.remote_ediv          = temp_le_sec_keys.remote_ediv;
        memcpy(resolve_key.remote_rand, temp_le_sec_keys.remote_rand, sizeof(temp_le_sec_keys.remote_rand));
        memcpy(resolve_key.remote_ltk, temp_le_sec_keys.remote_ltk, sizeof(temp_le_sec_keys.remote_ltk));

        rsi_6byte_dev_address_to_ascii(resolve_key.remote_dev_addr, temp_le_sec_keys.dev_addr);
        memcpy(resolve_key.peer_irk, temp_le_sec_keys.remote_irk, sizeof(temp_le_sec_keys.remote_irk));
        memcpy(resolve_key.local_irk, temp_le_sec_keys.local_irk, sizeof(temp_le_sec_keys.local_irk));

        resolve_key.Identity_addr_type = temp_le_sec_keys.Identity_addr_type;
        rsi_6byte_dev_address_to_ascii(resolve_key.Identity_addr, temp_le_sec_keys.Identity_addr);

        //LOG_PRINT ("\n Updating the Resolving list\n");
        update_resolvlist((rsi_ble_resolvlist_group_t *)&resolvlist, &resolve_key);

        //get resolvlist size
        status = rsi_ble_get_resolving_list_size(&rsi_app_resp_resolvlist_size);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n get resolving size cmd failed = %x \n", status);
          //return status;
        }

        //set address resolution enable and resolvable private address timeout
        status =
          rsi_ble_set_addr_resolution_enable(RSI_BLE_DEV_ADDR_RESOLUTION_ENABLE, RSI_BLE_SET_RESOLVABLE_PRIV_ADDR_TOUT);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n set reolution enable cmd failed = %x \n", status);
          //return status;
        }
#endif
#if PRIVACY_ENABLE
        //set privacy mode
        uint8_t remote_dev_bd_addr[6] = { 0 };

        rsi_ascii_dev_address_to_6bytes_rev(remote_dev_bd_addr, resolve_key.Identity_addr);
        status = rsi_ble_set_privacy_mode(resolve_key.Identity_addr_type, remote_dev_bd_addr, RSI_BLE_PRIVACY_MODE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Privacy command Failed = %x \n", status);
          return status;
        }
#endif
      } break;

      case RSI_BLE_SMP_FAILED_EVENT: {
        //! initiate SMP protocol as a Master

        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_SMP_FAILED_EVENT);
      }

      break;

      case RSI_BLE_ENCRYPT_STARTED_EVENT: {
        //! start the encrypt event
        master1_smp_done = 1;
        //! clear the served event
        rsi_ble_app_clear_event(RSI_BLE_ENCRYPT_STARTED_EVENT);
        LOG_PRINT("\r\n in smp encrypt event \r\n");
#if RESOLVE_ENABLE
        status = add_device_to_ltk_key_list(ble_dev_ltk_list, rsi_encryption_enabled);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\n Failed to add LTK to the device");
        }
#endif
      } break;
      case RSI_BLE_GATT_ERROR: {
        rsi_ble_app_clear_event(RSI_BLE_GATT_ERROR);
        if ((*(uint16_t *)rsi_ble_gatt_err_resp.error) == RSI_END_OF_PROFILE_QUERY) {
          if (total_remote_profiles != 0) //! If any profiles exists
          {
            if ((profile_index_for_char_query - 1) < total_remote_profiles) //! Error received for any profile
            {
              char_resp_recvd = false;
              char_for_serv_cnt++;
              //! set event
              rsi_ble_app_set_event(RSI_BLE_GATT_PROFILE);
            } else //! Error received for last profile
            {
              rsi_ble_app_set_event(RSI_BLE_GATT_CHAR_SERVICES);
            }
          } else //! Check for profiles pending, else done profile querying
          {
            //! first level profile query completed
            done_profiles_query = true;

            //set event to start second level profile query
            rsi_ble_app_set_event(RSI_BLE_GATT_PROFILES);
          }

        } else {
          LOG_PRINT("\nGATT ERROR REASON:0x%x -m1 \n", *(uint16_t *)rsi_ble_gatt_err_resp.error);
        }

      } break;
      default:
        break;
    }
  }
  return 0;
}
#endif
#endif
