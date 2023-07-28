/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

#define GPIO_INTERRUPT_PORT_WHEN_SPI 0U
#define GPIO_INTERRUPT_PIN_WHEN_SPI   14U 
#define APP_GPIO_INTA_IRQHandler GPIO_INTA_DriverIRQHandler
#define WAKEUP_GPIO_SW_IRQ  GPIO_INTA_IRQn
#define UART_INTERRUPT_SW_IRQ  FLEXCOMM0_IRQn

  
#if MXRT_595s_ADDONCARD         /*RT595 ADD ON CARD powersave and reset pins*/
#define BOARD_INITPINS_PS_OUT_PORT   3U 
#define BOARD_INITPINS_PS_OUT_PIN    20U

#define BOARD_INITPINS_PS_IN_PORT   3U
#define BOARD_INITPINS_PS_IN_PIN    17U

#define BOARD_INITPINS_RESET_PORT   0U 
#define BOARD_INITPINS_RESET_PIN    18U

#else /*RT595 EVK powersave and reset pins*/
#define BOARD_INITPINS_PS_OUT_PORT   0U 
#define  BOARD_INITPINS_PS_OUT_PIN   5U

#define BOARD_INITPINS_PS_IN_PORT   0U
#define BOARD_INITPINS_PS_IN_PIN    6U

#define BOARD_INITPINS_RESET_PORT   1U
#define BOARD_INITPINS_RESET_PIN    3U
#endif

#define  BOARD_INITPINS_HOST_GPIO_SEL_OUT_PORT      3U 
#define  BOARD_INITPINS_HOST_GPIO_SEL_OUT_PIN      14U    

/* SDIO interrupt (SD1_D1) PORT and PIN */
#define  SD1_D1_PORT 3U 
#define  SD1_D1_PIN 11U  
/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void); /* Function assigned for the Cortex-M33 */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void SPI5_InitPins(void); /* Function assigned for the Cortex-M33 */
 
#define IOPCTL_PIO_ANAMUX_DI 0x00u        /*!<@brief Analog mux is disabled */
#define IOPCTL_PIO_FULLDRIVE_DI 0x00u     /*!<@brief Normal drive */
#define IOPCTL_PIO_FUNC0 0x00u            /*!<@brief Selects pin function 0 */
#define IOPCTL_PIO_FUNC1 0x01u            /*!<@brief Selects pin function 1 */
#define IOPCTL_PIO_INBUF_DI 0x00u         /*!<@brief Disable input buffer function */
#define IOPCTL_PIO_INBUF_EN 0x40u         /*!<@brief Enables input buffer function */
#define IOPCTL_PIO_INV_DI 0x00u           /*!<@brief Input function is not inverted */
#define IOPCTL_PIO_PSEDRAIN_DI 0x00u      /*!<@brief Pseudo Output Drain is disabled */
#define IOPCTL_PIO_PSEDRAIN_EN 0x0400u    /*!<@brief Pseudo Output Drain is enabled */
#define IOPCTL_PIO_PULLDOWN_EN 0x00u      /*!<@brief Enable pull-down function */
#define IOPCTL_PIO_PULLUP_EN 0x20u        /*!<@brief Enable pull-up function */
#define IOPCTL_PIO_PUPD_DI 0x00u          /*!<@brief Disable pull-up / pull-down function */
#define IOPCTL_PIO_PUPD_EN 0x10u          /*!<@brief Enable pull-up / pull-down function */
#define IOPCTL_PIO_SLEW_RATE_NORMAL 0x00u /*!<@brief Normal mode */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void SPI5_DeinitPins(void); /* Function assigned for the Cortex-M33 */

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/