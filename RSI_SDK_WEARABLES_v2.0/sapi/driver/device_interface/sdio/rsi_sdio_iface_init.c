/*******************************************************************************
* @file  rsi_sdio_iface_init.c
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
 * @file     rsi_sdio_iface_init.c
 * @version  0.1
 * @date     15 Aug 2015
 *
 *
 *
 * @brief SPI INIT: Functions which initiaize the SPI hardware interface in the module
 * This file contains the SPI Initialization function.
 * This function enables the SPI interface on the chip.
 * This function is only run once during startup, after power-on, or reset
 *
 *
 */

/*
  Includes
 */
#include "rsi_driver.h"
#ifdef RSI_SDIO_INTERFACE
/*
  Global Variables
 */
/** @addtogroup DRIVER1
* @{
*/
/*=============================================*/
/**
 * @fn           int16_t rsi_sdio_iface_init(void)       
 * @brief        Initialize the wlan module�s Slave SDIO interface.
 * @param[in]    void 
 * @return       0              - Success \n 
 *               Non-Zero Value - Failure 
 */

int16_t rsi_sdio_iface_init(void)
{
  int16_t retval = RSI_SUCCESS;
  uint8_t data;

#ifdef MXRT_595s
  // Do enumeration
  retval = rsi_mcu_sdcard_init();

  if (retval != RSI_SUCCESS) {
    return retval;
  }
#endif
  // Do enumeration
  retval = rsi_mcu_sdio_init();

  if (retval != RSI_SUCCESS) {
    return retval;
  }

  // SDIO_READ_START_LVL
  data   = 0x24;
  retval = rsi_reg_wr(0xfc, &data);
  if (retval != RSI_SUCCESS) {
    return retval;
  }

  // SDIO_READ_FIFO_CTL
  data   = 0x60;
  retval = rsi_reg_wr(0xfd, &data);
  if (retval != RSI_SUCCESS) {
    return retval;
  }

  // SDIO_WRITE_FIFO_CTL
  data   = 0x20;
  retval = rsi_reg_wr(0xfe, &data);
  if (retval != RSI_SUCCESS) {
    return retval;
  }
  return retval;
}
#endif
/** @} */
