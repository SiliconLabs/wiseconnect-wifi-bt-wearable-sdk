/*******************************************************************************
* @file  rsi_hal_mcu_spi.c
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
 * @fn         void cs_enable(void) 
 * @brief      Enable the spi chip select pin in SPI interface.
 * @param[in] void 
 * @return     void 
 */

void cs_enable(void)
{
  // enables the spi chip select pin on SPI interface
}

/*==================================================================*/
/**
 * @fn         void cs_disable(void) 
 * @brief      Disable the spi chip select pin in SPI interface.
 * @param[in] void 
 * @return      void 
 */

void cs_disable(void)
{

  // disables the spi chip select pin on SPI interface
}
/*==================================================================*/
/**
 * @fn         int16_t rsi_spi_transfer(uint8_t *tx_buff, uint8_t *rx_buff, uint16_t transfer_length, uint8_t mode) 
 * @brief	   Tranfer/receive data to the module through the SPI interface.
 * @param[in]   tx_buff         - pointer to the buffer with the data to be transfered
 * @param[in]   rx_buff         - pointer to the buffer to store the data received
 * @param[in]   transfer_length - Number of bytes to send and receive
 * @param[in]   mode            - To indicate mode 8 BIT/32 BIT mode transfers.
 * @return      0 - Success 
 */
int16_t rsi_spi_transfer(uint8_t *tx_buff, uint8_t *rx_buff, uint16_t transfer_length, uint8_t mode)
{
  return 0;
}
/** @} */
