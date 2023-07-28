/*******************************************************************************
* @file  rsi_hal_mcu_usb_cdc.c
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
 * Includes
 */
#ifdef RSI_CHIP_MFG_EN

#include "rsi_driver.h"


#ifdef FRDM_K28F
#include "fsl_device_registers.h"
#include <fsl_debug_console.h>
#include "virtual_com.h"
/**
 * Global Variables
 */
 
/*==================================================================*/
/**
 * @fn         int16_t rsi_usb_cdc_transfer(uint8_t *ptrBuf,uint16_t bufLen,uint8_t *valBuf,uint8_t mode)
 * @param[in]  uint8_t *tx_buff, pointer to the buffer with the data to be transfered
 * @param[in]  uint8_t *rx_buff, pointer to the buffer to store the data received
 * @param[in]  uint16_t transfer_length, Number of bytes to send and receive
 * @param[in]  uint8_t mode, To indicate mode 8 BIT/32 BIT mode transfers.
 * @param[out] None
 * @return     0, 0=success
 * @section description  
 * This API is used to tranfer/receive data to the Wi-Fi module through the SPI interface.
 */
void rsi_host_send(uint8_t *tx_buff, uint16_t transfer_length)
{
	USB_Vcom_host_send(tx_buff, transfer_length);
}

int32_t rsi_host_req_nxt_cmd(rsi_host_packet_recv_notify_t callback)
{
	return USB_Vcom_host_req_recv(callback);
}
#endif

#endif
