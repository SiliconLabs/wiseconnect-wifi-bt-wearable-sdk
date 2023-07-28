/*******************************************************************************
* @file  rsi_hal_mcu_interrupt.c
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
 * @file       rsi_hal_mcu_interrupt.c
 * @version    0.1
 * @date       18 sept 2015
 *
 *
 *
 * @brief HAL INTERRUPT: Functions related to HAL Interrupts
 * 
 * @section Description
 * This file contains the list of functions for configuring the microcontroller interrupts. 
 * Following are list of API's which need to be defined in this file.
 *
 */


/**
 * Includes
 */
#include "rsi_driver.h"
#ifdef RSI_WITH_OS
#include <projdefs.h>
#include "portmacro.h"
#include "FreeRTOSConfig.h"
#endif
#include "board.h"
#ifdef FRDM_K28F   
#include "pin_mux.h"
#include "MK28F15.h"
#include "fsl_common.h"

extern void GPIO_ClearPinsInterruptFlags(GPIO_Type *base, uint32_t mask);
void (* g_rsi_interrupt_handler)(void);
void PORTD_IRQHandler(void)
{
	rsi_hal_intr_clear();

	if(g_rsi_interrupt_handler)
	{
		g_rsi_interrupt_handler();
	}
#ifdef RSI_WITH_OS
	portYIELD_FROM_ISR(pdTRUE);
#endif
}
#ifdef WAKEUP_GPIO_INTERRUPT_METHOD
void PORTA_IRQHandler(void)
{
  rsi_give_wakeup_indication();
#ifdef RSI_WITH_OS
	portYIELD_FROM_ISR(pdTRUE);
#endif
}
#endif
#endif


/*===================================================*/
/**
 * @fn           void rsi_give_wakeup_indication(void)
 * @brief        isr to wakeup indication
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @description  This HAL API should post the semaphore .
 */
#ifdef WAKEUP_GPIO_INTERRUPT_METHOD
void rsi_give_wakeup_indication(void)
{
	if(rsi_hal_get_gpio(RSI_HAL_WAKEUP_INDICATION_PIN))
	{
		rsi_hal_gpio_clear();
		rsi_hal_gpio_mask();
#ifndef RSI_COMMON_SEM_BITMAP
    rsi_driver_cb_non_rom->common_wait_bitmap &= ~BIT(4);
#endif
		rsi_semaphore_post_from_isr(&rsi_driver_cb->common_cb->wakeup_gpio_sem);
	}
}
#endif

/*===================================================*/
/**
 * @fn           void rsi_hal_enable_uart_irq(void)
 * @brief        Enables the UART interrupt
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @description  This HAL API enables UART interrupts.
 */
void rsi_hal_enable_uart_irq(void)
{
  //! Enable uart interrupt
  EnableIRQ(BOARD_UART_IRQ);
  return;
}

/*===================================================*/
/**
 * @fn           void rsi_hal_intr_config(void (* rsi_interrupt_handler)())
 * @brief        Starts and enables the SPI interrupt
 * @param[in]    rsi_interrupt_handler() ,call back function to handle interrupt
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to initialize the register/pins
 *               related to interrupts and enable the interrupts.
 */
void rsi_hal_intr_config(void (* rsi_interrupt_handler)(void))
{
  //! Configure interrupt pin/register in input mode and register the interrupt handler

  // Pin config is handled in pin_mux.c
  // Interrupt is fixed to PORTE_IRQhandler
#ifdef FRDM_K28F  
  g_rsi_interrupt_handler = rsi_interrupt_handler;
  NVIC_SetPriority(PORTD_IRQn, 5);
  EnableIRQ(PORTD_IRQn);
#endif
  return;

}


/*===================================================*/
/** 
 * @fn           void rsi_hal_intr_mask(void)
 * @brief        Disables the SPI Interrupt
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to mask/disable interrupts.
 */
void rsi_hal_intr_mask(void)
{
  //! Mask/Disable the interrupt 
#ifdef FRDM_K28F     
  DisableIRQ(PORTD_IRQn);
#endif  
  return;

}
#ifdef WAKEUP_GPIO_INTERRUPT_METHOD
/*===================================================*/
/**
 * @fn           void rsi_hal_gpio_mask(void)
 * @brief        Disables the SPI Interrupt
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to mask/disable interrupts.
 */
void rsi_hal_gpio_mask(void)
{
  //! Mask/Disable the interrupt
#ifdef FRDM_K28F
  DisableIRQ(PORTA_IRQn);
#endif
  return;

}
/*===================================================*/
/**
 * @fn           void rsi_hal_gpio_unmask(void)
 * @brief        Enables the SPI interrupt
 * @param[in]    none  
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to enable interrupts.
 */
 
void rsi_hal_gpio_unmask(void)
{
  //! Unmask/Enable the interrupt
#ifdef FRDM_K28F
  EnableIRQ(PORTA_IRQn);
#endif
  return;

}
/*===================================================*/
/**
 * @fn           void rsi_hal_gpio_clear(void)
 * @brief        Clears the pending interrupt
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to clear the handled interrupts.
 */
void rsi_hal_gpio_clear(void)
{
   //! Clear the interrupt
#ifdef FRDM_K28F
   GPIO_ClearPinsInterruptFlags(BOARD_INITPINS_PS_IN_GPIO, (1U << BOARD_INITPINS_PS_IN_PIN));
#endif
   return;
}
#endif
/*===================================================*/
/**
 * @fn           void rsi_hal_intr_unmask(void)
 * @brief        Enables the SPI interrupt
 * @param[in]    none  
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to enable interrupts.
 */
void rsi_hal_intr_unmask(void)
{
  //! Unmask/Enable the interrupt
#ifdef FRDM_K28F     
  EnableIRQ(PORTD_IRQn);
#endif  
  return;

}



/*===================================================*/
/**
 * @fn           void rsi_hal_intr_clear(void)
 * @brief        Clears the pending interrupt
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to clear the handled interrupts.
 */
void rsi_hal_intr_clear(void)
{
   //! Clear the interrupt
#ifdef FRDM_K28F
   GPIO_ClearPinsInterruptFlags(BOARD_INITPINS_INTERRUPT_GPIO, (1U << BOARD_INITPINS_INTERRUPT_PIN));
#endif   
   return;
}


/*===================================================*/
/**
 * @fn          void rsi_hal_intr_pin_status(void)
 * @brief       Checks the SPI interrupt at pin level
 * @param[in]   none  
 * @param[out]  uint8_t, interrupt status 
 * @return      none
 * @description This API is used to check interrupt pin status(pin level whether it is high/low).
 */	
uint8_t rsi_hal_intr_pin_status(void)
{

  volatile uint8_t status = 0;

  //! Return interrupt pin  status(high(1) /low (0))
  status = rsi_hal_get_gpio(RSI_HAL_MODULE_INTERRUPT_PIN);
  
  return status;
}

