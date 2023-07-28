/*******************************************************************************
* @file  rsi_hal_mcu_sdio.c
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
/*==============================================*/
/**
 * @fn          int16_t rsi_sdio_write_multiple(uint8_t *tx_data, uint32_t Addr, uint16_t no_of_blocks)
 * @brief       Write the packet on to the SDIO interface in block mode.
 * @param[in]   tx_data      - tx_data is the buffer to be written to sdio.
 * @param[in]   Addr         - Addr of the mem to which the data has to be written.
 * @param[in]   no_of_blocks - no_of_blocks is the blocks present to be transfered.
 * @return      0              - Success \n
 *			    Negative Value - Failure
 */

int16_t rsi_sdio_write_multiple(uint8_t *tx_data, uint32_t Addr, uint16_t no_of_blocks)
{
}

/*==============================================*/
/**
 * @fn          int8_t rsi_sdio_read_multiple(uint8_t *read_buff, uint32_t Addr)
 * @brief       Read no of bytes in blocked mode from device.This function gets the packet coming from the module and copies to the buffer pointed 
 * @param[in]   read_buff - buffer to be stored with the data read from device.
 * @param[in]   Addr      - Addr of the mem to be read.
 * @return      0              - Success \n
 *			    Negative Value - Failure
 */

int8_t rsi_sdio_read_multiple(uint8_t *read_buff, uint32_t Addr)
{
}

/*==============================================*/
/**
 * @fn          int8_t sdio_reg_writeb(uint32_t Addr, uint8_t *dBuf)
 * @brief       Write 1 byte of data to sdio slave register space.
 * @param[in]   Addr - Addr of the reg to be written.
 * @param[in]   dBuf - Buffer of data to be written to sdio slave reg.
 * @return      0    - Success \n
 *              Negative Value - Failure
 */

int8_t sdio_reg_writeb(uint32_t Addr, uint8_t *dBuf)
{
}

/*==============================================*/
/**
 * @fn          int8_t sdio_reg_readb(uint32_t Addr, uint8_t *dBuf)
 * @brief       Read 1 byte of data from sdio slave register space.
 * @param[in]   Addr - Addr of the reg to be read.
 * @param[in]   dBuf - Buffer of data to be read from sdio slave reg.
 * @return      0    - Success \n
 *              Negative Value - Failure
 */

int8_t sdio_reg_readb(uint32_t Addr, uint8_t *dBuf)
{
}

/*==============================================*/
/**
 * @fn          int16_t rsi_sdio_readb(uint32_t addr, uint16_t len, uint8_t *dBuf)
 * @brief       Read n bytes of data from device space in byte mode.
 * @param[in]   addr - Addr of the data to be read.
 * @param[in]   dBuf - Buffer of data to be read from sdio device.
 * @return      0              - Success \n 
 *			    Negative Value - Failure
 */

int16_t rsi_sdio_readb(uint32_t addr, uint16_t len, uint8_t *dBuf)
{
}

/*==============================================*/
/**
 * @fn          int16_t rsi_sdio_writeb(uint32_t addr, uint16_t len, uint8_t *dBuf)
 * @brief       Write n bytes of data to device space in byte mode.
 * @param[in]   addr - Addr of the data to be written.
 * @param[in]   dBuf - Buffer of data to be written to sdio device.
 * @return      0              - Success \n
 *			    Negative Value - Failure
 */

int16_t rsi_sdio_writeb(uint32_t addr, uint16_t len, uint8_t *dBuf)
{
}
/*=============================================*/
/**
 * @fn                  int32_t rsi_mcu_sdio_init(void)
 * @brief               Initialize the modules Slave SDIO interface.
 * @param[in]           void 
 * @return              void 
 */

int32_t rsi_mcu_sdio_init(void)
{
}
/** @} */
