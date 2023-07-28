/*******************************************************************************
* @file  rsi_hal_mcu_timer.c
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
 * @file       rsi_hal_mcu_timer.c
 * @version    0.1
 * @date       15 Aug 2015
 *
 *
 *
 * @brief HAL TIMER: Functions related to HAL timers
 *
 * @section Description
 * This file contains the list of functions for configuring the microcontroller timers.
 * Following are list of API's which need to be defined in this file.
 *
 */


/**
 * Includes
 */


#include "rsi_driver.h"
#if (defined(RSI_WITH_OS) && defined(FRDM_K28F))
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "stack_macros.h"

extern void xPortSysTickHandler( void );
#endif

#ifdef RSI_M4_INTERFACE
static volatile uint32_t 		_dwTickCount  ;		//systick cout variable
#endif



#ifdef FRDM_K28F
/*! @brief Time variable as sys ticks. */
volatile uint32_t g_systicks;

void SysTick_Handler(void)
{
	g_systicks++;
#ifdef RSI_WITH_OS
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
		xPortSysTickHandler();
	}
#endif

}
#endif
/*===================================================*/
/**
 * @fn           int32_t rsi_timer_start(uint8_t timer_no, uint8_t mode,uint8_t type,uint32_t duration,void (* rsi_timer_expiry_handler)())
 * @brief        Starts and configures timer
 * @param[in]    timer_node, timer node to be configured.
 * @param[in]    mode , mode of the timer
 *               0 - Micro seconds mode
 *               1 - Milli seconds mode
 * @param[in]    type, type of  the timer
 *               0 - single shot type
 *               1 - periodic type
 * @param[in]    duration, timer duration
 * @param[in]    rsi_timer_expiry_handler() ,call back function to handle timer interrupt
 * @param[out]   none
 * @return       0 - success
 *               !0 - Failure
 * @description  This HAL API should contain the code to initialize the timer and start the timer
 *
 */

int32_t rsi_timer_start(uint8_t timer_node, uint8_t mode, uint8_t type, uint32_t duration, void (* rsi_timer_expiry_handler)(void))
{
	UNUSED_PARAMETER(timer_node);
	UNUSED_PARAMETER(mode);
	UNUSED_PARAMETER(type);
	UNUSED_PARAMETER(duration);
	UNUSED_PARAMETER(rsi_timer_expiry_handler);
  //! Initialise the timer


  //! register the call back


  //! Start timer

  return RSI_SUCCESS;
}


/*===================================================*/
/**
 * @fn           int32_t rsi_timer_stop(uint8_t timer_no)
 * @brief        Stops timer
 * @param[in]    timer_node, timer node to stop
 * @param[out]   none
 * @return       0 - success
 *               !0 - Failure
 * @description  This HAL API should contain the code to stop the timer
 *
 */

int32_t rsi_timer_stop(uint8_t timer_node)
{
	UNUSED_PARAMETER(timer_node);
	//! Stop the timer

	return RSI_SUCCESS;


}




/*===================================================*/
/**
 * @fn           uint32_t rsi_timer_read(uint8_t timer_node)
 * @brief        read timer
 * @param[in]    timer_node, timer node to read
 * @param[out]   none
 * @return       timer value
 * @description  This HAL API should contain API to  read the timer
 *
 */

uint32_t rsi_timer_read(uint8_t timer_node)
{
	UNUSED_PARAMETER(timer_node);
	volatile uint32_t timer_val = 0;

	//! read the timer and return timer value

	return timer_val;


}






/*===================================================*/
/**
 * @fn           void rsi_delay_us(uint32_t delay)
 * @brief        create delay in micro seconds
 * @param[in]    delay_us, timer delay in micro seconds
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to create delay in micro seconds
 *
 */
void rsi_delay_us(uint32_t delay_us)
{
    //! call the API for delay in micro seconds
#ifdef FRDM_K28F
	uint32_t start;

	if (delay_us == 0)
		return;
#if TICK_100_USEC
	delay_us = delay_us/100;
#endif

	start = rsi_hal_gettickcount();

	do {
	} while ((rsi_hal_gettickcount() - start) < delay_us);

	return;
#endif
}


#ifdef RSI_M4_INTERFACE

 extern void SysTick_Handler(void);

void SysTick_Handler(void)
{
	_dwTickCount++;
}
uint32_t GetTickCount( void )
{
	return _dwTickCount ;						// gets the tick count from systic ISR
}
#endif
/*===================================================*/
/**
 * @fn           void rsi_delay_ms(float delay)
 * @brief        create delay in micro seconds
 * @param[in]    delay, timer delay in micro seconds
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to create delay in micro seconds
 *
 */
void rsi_delay_ms(uint32_t delay_ms)
{
#ifdef FRDM_K28F
#if TICK_1_MSEC
	uint32_t start;

	if (delay_ms == 0)
		return;

	start = rsi_hal_gettickcount();

	do {
	} while ((rsi_hal_gettickcount() - start) < delay_ms);

#elif TICK_100_USEC
	rsi_delay_us((uint32_t)(delay_ms * 1000U));
#endif
	return;
#endif	
}

/*===================================================*/
/**
 * @fn           uint32_t rsi_hal_gettickcount()
 * @brief        provides a tick value in milliseconds
 * @return       tick value
 * @description  This HAL API should contain the code to read the timer tick count value in milliseconds 
 *
 */

uint32_t rsi_hal_gettickcount(void)
{
  //! Define your API to get the tick count delay in milli seconds from systic ISR and return the resultant value
#if defined(FRDM_K28F)
	return g_systicks /*(xTaskGetTickCount() * portTICK_PERIOD_MS)*/;
#else
	return RSI_SUCCESS;

#endif
}
