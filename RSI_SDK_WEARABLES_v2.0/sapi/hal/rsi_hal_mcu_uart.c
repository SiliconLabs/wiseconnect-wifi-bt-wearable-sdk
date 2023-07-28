/*******************************************************************************
* @file  rsi_hal_mcu_uart.c
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

#include "rsi_driver.h"

/** @addtogroup HAL
* @{
*/
/*==================================================================*/
/**
 * @fn         int16_t rsi_uart_send(uint8_t *ptrBuf,uint16_t bufLen)
 * @brief      Send data to the module through the UART interface.
 * @param[in]  uint8 *ptrBuf - pointer to the buffer with the data to be sent/received
 * @param[in]  uint16 bufLen - number of bytes to send
 * @return     0 - Success
 */

int16_t rsi_uart_send(uint8_t *ptrBuf, uint16_t bufLen)
{
  return 0;
}
/*==================================================================*/
/**
 * @fn         int16_t rsi_uart_recv(uint8_t *ptrBuf,uint16_t bufLen)
 * @brief      Receive data from module through the UART interface.
 * @param[in]  uint8_t *ptrBuf - pointer to the buffer with the data to be sent/received
 * @param[in]  uint16_t bufLen - number of bytes to send
 * @param[out] None
 * @return     0 - Succes
 */

int16_t rsi_uart_recv(uint8_t *ptrBuf, uint16_t bufLen)
{
  return 0;
}
/** @} */
