/*******************************************************************************
* @file  rsi_spi_frame_rd_wr.c
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
 * @file     rsi_spi_frame_rd_wr.c
 * @version  0.1
 * @date     15 Aug 2015
 *
 *
 *
 * @brief SPI Control: Functions used to control SPI frame read/write functions
 *
 * Description
 * SPI function to read/write management descriptors frames to/from the Wi-Fi module
 *
 * Improvements
 * Function header is modified for frameDscRd() function
 */

/*
 * Includes
 */
#include "rsi_driver.h"

#if ((defined DEBUG_PACKET_EXCHANGE) && (defined RSI_DEBUG_PRINTS))
#include <stdio.h>
#define MAX_PRINT_PAYLOAD_LEN 8
#define DEBUG_OUTPUT_SZ       (80 + MAX_PRINT_PAYLOAD_LEN * 2)
char debug_output[DEBUG_OUTPUT_SZ];
#endif

#ifdef RSI_SPI_INTERFACE
/*
  Global Variables
 */

#ifdef RSI_CHIP_MFG_EN
uint8_t spi_read_pkt[1600];
/** @addtogroup DRIVER2
* @{
*/
/*===========================================================================*/
/**
 * @fn          int16_t rsi_nlink_pkt_rd(uint8_t *buf, uint16_t total_len)
 * @brief       Perform frame decriptor and payload read.
 * @param[in]   buf       -  pointer to the buffer into which  decriptor and payload has to be read
 * @param[in]   total_len  -  number of bytes to be read
 * @return      0  - SUCCESS \n
 *              -1 - SPI busy / Timeout \n
 *              -2 - SPI Failure
 *               
 */
int16_t rsi_nlink_pkt_rd(uint8_t *buf, uint16_t total_len)
{

  int16_t retval;
  uint8_t c1;
  uint8_t c2;
  uint8_t c3;
  uint8_t c4;
  uint32_t aligned_len = 0;
  uint16_t frame_len;
  uint16_t frame_offset;
  uint16_t pkt_len;

  aligned_len = ((total_len) + 3) & ~3;

  c1 = RSI_C1FRMRD16BIT1BYTE;
#ifdef RSI_BIT_32_SUPPORT
  c2 = RSI_C2SPIADDR1BYTE;
#else
  c2 = RSI_C2MEMRDWRNOCARE;
#endif
  // command frame response descriptor
  c3 = aligned_len & 0xff;

  // upper bye of transfer length
  c4 = (aligned_len & 0xff00) >> 8;

  // Send C1/C2
  retval = rsi_send_c1c2(c1, c2);
  if (retval != 0) {
    // exit with error if we timed out waiting for the SPI to get ready
    return retval;
  }

  // Send C3/C4
  retval = rsi_send_c3c4(c3, c4);

  // Wait for start token
  retval = rsi_spi_wait_start_token(RSI_START_TOKEN_TIMEOUT, RSI_MODE_32BIT);
  if (retval != 0) {
    // exit with error if we timed out waiting for the SPI to get ready
    return retval;
  }

  retval = rsi_spi_transfer(NULL, spi_read_pkt, aligned_len, RSI_MODE_8BIT);
  if (retval != 0) {
    // exit with error if we timed out waiting for the SPI to get ready
    return retval;
  }

  pkt_len      = *(uint16_t *)&spi_read_pkt[0];
  frame_offset = *(uint16_t *)&spi_read_pkt[2];
  frame_len    = pkt_len - frame_offset;
  // Actual spi read for descriptor and payload
  if (buf) {
    memcpy(buf, &spi_read_pkt[frame_offset], frame_len);
  }

  return retval;
}
#endif
/** @} */
/** @addtogroup DRIVER2
* @{
*/

/*====================================================*/
/**
 * @fn          int16_t rsi_frame_read(uint8_t *pkt_buffer)
 */
int16_t rsi_frame_read(uint8_t *pkt_buffer)
{

  int16_t retval;
  uint8_t local_buffer[8];
#ifndef RSI_CHIP_MFG_EN
  // Read first 4 bytes
  retval = rsi_pre_dsc_rd(&local_buffer[0]);
  if (retval != RSI_SUCCESS) {
    return retval;
  }
  // Read complete RX packet
  retval = rsi_pkt_rd(pkt_buffer,
                      ((rsi_bytes2R_to_uint16(&local_buffer[2])) - 4),
                      ((rsi_bytes2R_to_uint16(&local_buffer[0])) - 4));
  if (retval != RSI_SUCCESS) {
    return retval;
  }
#else
  // Read first 4 bytes
  retval = rsi_spi_pkt_len(&local_buffer[0]);
  if (retval != 0x00) {
    return retval;
  }
  retval = rsi_nlink_pkt_rd(pkt_buffer, rsi_bytes2R_to_uint16(&local_buffer[0]) & 0xFFF);
  if (retval != 0x00) {
    return retval;
  }
#endif

#ifdef DEBUG_PACKET_EXCHANGE
  uint16_t size = rsi_bytes2R_to_uint16(pkt_buffer) & 0x0FFF;
  memset(debug_output, '\0', sizeof(debug_output));
  sprintf(debug_output, "[RD(%d)]", rsi_hal_gettickcount());
  for (uint32_t i = 0; i < 16; i++) {
    sprintf(debug_output, "%s%.2x", debug_output, pkt_buffer[i]);
  }
  if (size) {
    sprintf(debug_output, "%s | ", debug_output);
    // print max of 8 bytes of payload
    for (uint32_t i = 0; i < size && i < MAX_PRINT_PAYLOAD_LEN; i++) {
      sprintf(debug_output, "%s%.2x", debug_output, pkt_buffer[16 + i]);
    }
  }
  LOG_PRINT("%s\r\n", debug_output);
#endif
  return retval;
}

/*====================================================*/
/**
 * @fn          int16_t rsi_frame_write(rsi_frame_desc_t *uFrameDscFrame, uint8_t *payloadparam, uint16_t size_param)
 * @brief       Process a command to the wlan module. 
 * @param[in]   uFrameDscFrame  -  frame descriptor
 * @param[in]   payloadparam     -  pointer to the command payload parameter structure
 * @param[in]   size_param       -  size of the payload for the command
 * @return      0  - SUCCESS \n
 *              -1 - SPI busy / Timeout \n
 *              -2 - SPI Failure
 *              
 *              
 */
int16_t rsi_frame_write(rsi_frame_desc_t *uFrameDscFrame, uint8_t *payloadparam, uint16_t size_param)
{
  int16_t retval;

#ifdef DEBUG_PACKET_EXCHANGE
  memset(debug_output, '\0', sizeof(debug_output));
  sprintf(debug_output, "[WR(%d)]", rsi_hal_gettickcount());
  for (uint32_t i = 0; i < 16; i++) {
    sprintf(debug_output, "%s%.2x", debug_output, ((char *)(uFrameDscFrame))[i]);
  }
  if (size_param) {
    sprintf(debug_output, "%s | ", debug_output);
    // print max of 8 bytes of payload
    for (uint32_t i = 0; i < size_param && i < MAX_PRINT_PAYLOAD_LEN; i++) {
      sprintf(debug_output, "%s%.2x", debug_output, payloadparam[i]);
    }
  }
  LOG_PRINT("%s\r\n", debug_output);
#endif
#ifndef RSI_CHIP_MFG_EN
  // Write host descriptor
  retval = rsi_spi_frame_dsc_wr(uFrameDscFrame);
  if (retval != RSI_SUCCESS) {
    return retval;
  }

  // Write payload if present
  if (size_param)
#endif
  {
    // 4 byte align for payload size
    size_param = (size_param + 3) & ~3;
#ifdef RSI_CHIP_MFG_EN
    size_param += RSI_HOST_DESC_LENGTH;
    retval = rsi_spi_frame_data_wr(size_param, uFrameDscFrame, 0, NULL);
#else
    retval = rsi_spi_frame_data_wr(size_param, payloadparam, 0, NULL);
#endif
    if (retval != RSI_SUCCESS) {
      return retval;
    }
  }
  return retval;
}

/*===========================================================================*/
/**
 * @fn          int16_t rsi_pre_dsc_rd(uint8_t *dbuf)
 * @brief       Perform a pre frame decriptor read. 
 * @param[in]   dbuf - pointer to the buffer into which pre decriptor has to be read
 * @return       0 - SUCCESS \n
 *              -1 - SPI busy / Timeout \n
 *              -2 - SPI Failure
 *              
 */
int16_t rsi_pre_dsc_rd(uint8_t *dbuf)
{
  int16_t retval;
  uint8_t c1;
  uint8_t c2;
  uint8_t c3;
  uint8_t c4;

  c1 = RSI_C1FRMRD16BIT4BYTE;
#ifdef RSI_BIT_32_SUPPORT
  c2 = RSI_C2SPIADDR4BYTE;
#else
  c2 = RSI_C2MEMRDWRNOCARE;
#endif
  // command frame response descriptor
  c3 = 0x04;

  // upper bye of transfer length
  c4 = 0x00;

  // Send C1/C2
  retval = rsi_send_c1c2(c1, c2);
  if (retval != RSI_SUCCESS) {
    // exit with error if we timed out waiting for the SPI to get ready
    return retval;
  }

  // Send C3/C4
  retval = rsi_send_c3c4(c3, c4);
  if (retval != RSI_SUCCESS) {
    return retval;
  }

  // Wait for start token
  retval = rsi_spi_wait_start_token(RSI_START_TOKEN_TIMEOUT, RSI_MODE_32BIT);
  if (retval != RSI_SUCCESS) {
    // exit with error if we timed out waiting for the SPI to get ready
    return retval;
  }

  // SPI read after start token
  retval = rsi_spi_transfer(NULL, (uint8_t *)dbuf, 0x4, RSI_MODE_32BIT);
  if (retval != RSI_SUCCESS) {
    return retval;
  }

  return retval;
}

/*===========================================================================*/
/**
 * @fn          int16_t rsi_pkt_rd(uint8_t *buf, uint16_t dummy_len, uint16_t total_len)
 * @brief       Perform frame decriptor and payload read.
 * @param[in]   buf       - pointer to the buffer into which  decriptor and payload has to be read
 * @param[in]   dummy_len - number of dummy bytes which can be discarded
 * @param[in]   total_len - number of bytes to be read
 * @return      0 - SUCCESS \n
 *              -1 - SPI busy / Timeout \nrsi_spi_frame_data_wr
 *              -2 - SPI Failure
 *               
 */

int16_t rsi_pkt_rd(uint8_t *buf, uint16_t dummy_len, uint16_t total_len)
{
  int16_t retval;
  uint8_t c1;
  uint8_t c2;
  uint8_t c3;
  uint8_t c4;
#ifdef SAPIS_BT_STACK_ON_HOST
  uint8_t dummy_buf[150];
#else
  uint8_t dummy_buf[8];
#endif
  uint32_t aligned_len = 0;

  aligned_len = ((total_len) + 3) & ~3;

  c1 = RSI_C1FRMRD16BIT1BYTE;
#ifdef RSI_BIT_32_SUPPORT
  c2 = RSI_C2SPIADDR1BYTE;
#else
  c2 = RSI_C2MEMRDWRNOCARE;
#endif
  // command frame response descriptor
  c3 = aligned_len & 0xff;

  // upper bye of transfer length
  c4 = (aligned_len & 0xff00) >> 8;

  // Send C1/C2
  retval = rsi_send_c1c2(c1, c2);
  if (retval != RSI_SUCCESS) {
    // exit with error if we timed out waiting for the SPI to get ready
    return retval;
  }

  // Send C3/C4
  retval = rsi_send_c3c4(c3, c4);
  if (retval != RSI_SUCCESS) {
    return retval;
  }

  // Wait for start token
  retval = rsi_spi_wait_start_token(RSI_START_TOKEN_TIMEOUT, RSI_MODE_32BIT);
  if (retval != RSI_SUCCESS) {
    // exit with error if we timed out waiting for the SPI to get ready
    return retval;
  }

  if (dummy_len) {
    retval = rsi_spi_transfer(NULL, (uint8_t *)&dummy_buf[0], dummy_len, RSI_MODE_8BIT);
    if (retval != RSI_SUCCESS) {
      return retval;
    }
  }

  // Actual spi read for descriptor and payload
  if (buf) {
    retval = rsi_spi_transfer(NULL, buf, (aligned_len - dummy_len), RSI_MODE_32BIT);
    if (retval != RSI_SUCCESS) {
      return retval;
    }
  }

  return retval;
}

/*===========================================================================*/
/**
 * @fn          int16_t rsi_spi_frame_dsc_wr(rsi_frame_desc_t *uFrameDscFrame)
 * @brief       Write a Frame descriptor. 
 * @param[in]   uFrameDscFrame - frame descriptor
 * @return      0  - SUCCESS \n
 *              -1  - SPI busy / Timeout \n
 *              -2  - SPI Failure
 *                 
 */
int16_t rsi_spi_frame_dsc_wr(rsi_frame_desc_t *uFrameDscFrame)
{
  int16_t retval;
  uint8_t localBuf[16];
  uint8_t c1;
  uint8_t c2;
  uint8_t c3;
  uint8_t c4;

  c1 = RSI_C1FRMWR16BIT4BYTE;
#ifdef RSI_BIT_32_SUPPORT
  c2 = RSI_C2RDWR4BYTE;
#else
  c2 = RSI_C2RDWR1BYTE;
#endif
  // frame descriptor is 16 bytes long
  c3 = RSI_FRAME_DESC_LEN;

  // upper bye of transfer length
  c4 = 0x00;

  // send C1/C2
  retval = rsi_send_c1c2(c1, c2);
  if (retval != RSI_SUCCESS) {
    // exit with error if we timed out waiting for the SPI to get ready
    return retval;
  }

  // send C3/C4
  retval = rsi_send_c3c4(c3, c4);
  if (retval != RSI_SUCCESS) {
    // exit with error if we timed out waiting for the SPI to get ready
    return retval;
  }
  // SPI send
  retval = rsi_spi_transfer(uFrameDscFrame->frame_len_queue_no, localBuf, RSI_FRAME_DESC_LEN, RSI_MODE_32BIT);
  if (retval != RSI_SUCCESS) {
    // exit with error if we timed out waiting for the SPI to get ready
    return retval;
  }

  return retval;
}

/*===========================================================================*/
/**
 * @fn          int16_t rsi_spi_frame_data_wr(uint16_t bufLen, uint8_t *dBuf,uint16_t tbufLen,uint8_t *tBuf)
 * @brief       Perform Frame Data Write. 
 * @param[in]   uint16_t buflen        -   length of the data buffer to write
 * @param[in]   dBuf         -   pointer to the buffer of data to write
 * @param[in]   tbuflen       -   length of the data fragment to write
 * @param[in]   tBuf         -   pointer to the buffer of data fragment to write
 * @return      0 - SUCCESS \n
 *              -1 - SPI busy / Timeout \n
 *              -2 - SPI Failure  
 *               
 */
int16_t rsi_spi_frame_data_wr(uint16_t bufLen, uint8_t *dBuf, uint16_t tbufLen, uint8_t *tBuf)
{
  int16_t retval;
  uint8_t c1;
  uint8_t c2;
  uint8_t c3;
  uint8_t c4;
  uint16_t tempbufLen;
  tempbufLen = bufLen + tbufLen;

  c1 = RSI_C1FRMWR16BIT4BYTE;
#ifdef RSI_BIT_32_SUPPORT
  c2 = RSI_C2RDWR4BYTE;
#else
  c2 = RSI_C2RDWR1BYTE;
#endif
  // lower byte of transfer length
  c3 = (uint8_t)(tempbufLen & 0x00ff);

  // upper byte of transfer length
  c4 = (uint8_t)((tempbufLen >> 8) & 0x00FF);

  // send C1/C2
  retval = rsi_send_c1c2(c1, c2);
  if (retval != RSI_SUCCESS) {
    // exit with error if we timed out waiting for the SPI to get ready
    return retval;
  }

  // send C3/C4
  retval = rsi_send_c3c4(c3, c4);
  if (retval != RSI_SUCCESS) {
    return retval;
  }

  // SPI send
  retval = rsi_spi_transfer(dBuf, NULL, bufLen, RSI_MODE_32BIT);
  if (retval != RSI_SUCCESS) {
    return retval;
  }
  if (tbufLen) {
    retval = rsi_spi_transfer(tBuf, NULL, tbufLen, RSI_MODE_32BIT);
    if (retval != RSI_SUCCESS) {
      return retval;
    }
  }

  return retval;
}

#endif
/** @} */
