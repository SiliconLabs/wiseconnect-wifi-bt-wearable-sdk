/*******************************************************************************
* @file  rsi_hal_mcu_ioports.c
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
 * @file       rsi_hal_mcu_ioports.c
 * @version    0.1
 * @date       18 sept 2015
 *
 *
 *
 * @brief Functions to control IO pins of the microcontroller
 *  
 * @section Description
 * This file contains API to control different pins of the microcontroller 
 * which interface with the module and other components related to the module. 
 *
 */


/**
 * Includes
 */
#include "rsi_driver.h"


/**
 * Global Variables
 */

#include "pin_mux.h"
#include "board.h"

/*===========================================================*/
/**
 * @fn            void rsi_hal_config_gpio(uint8_t gpio_number,uint8_t mode,uint8_t value)
 * @brief         Configures gpio pin in output mode,with a value
 * @param[in]     uint8_t gpio_number, gpio pin number to be configured
 * @param[in]     uint8_t mode , input/output mode of the gpio pin to configure
 *                0 - input mode
 *                1 - output mode
 * @param[in]     uint8_t value, default value to be driven if gpio is configured in output mode
 *                0 - low
 *                1 - high
 * @param[out]    none
 * @return        none
 * @description This API is used to configure host gpio pin in output mode. 
 */
void rsi_hal_config_gpio(uint8_t gpio_number,uint8_t mode,uint8_t value)
{
	UNUSED_PARAMETER(value);
	UNUSED_PARAMETER(mode);
	UNUSED_PARAMETER(gpio_number);
	// all configuration is in pin_mux.c
  //! Initialize the gpio pins in input/output mode

  //! Drive a default value on gpio if gpio is configured in output mode
#ifdef FRDM_K28F
	//! RESET and SPI INTR pins are initialized in BOARD_InitPins() in pin_mux.c
	/*gpio_pin_config_t RESET_config = {
			.pinDirection = kGPIO_DigitalOutput,
			.outputLogic = 0U
	};
	 Initialize GPIO functionality on pin PTA8 (pin K9)
	GPIO_PinInit(BOARD_INITPINS_RESET_GPIO, BOARD_INITPINS_RESET_PIN, &RESET_config);

	gpio_pin_config_t INTERRUPT_config = {
			.pinDirection = kGPIO_DigitalInput,
			.outputLogic = 0U
	};
	 Initialize GPIO functionality on pin PTA9 (pin K10)
	GPIO_PinInit(BOARD_INITPINS_INTERRUPT_GPIO, BOARD_INITPINS_INTERRUPT_PIN, &INTERRUPT_config);*/

	//! RESET PIN

	 /* Initialize GPIO functionality on pin PTD14  */
/*	    GPIO_PinInit(BOARD_INITPINS_RESET_GPIO, BOARD_INITPINS_RESET_PIN, &RESET_config);

	    PORTD14 is configured as PTD14
	    PORT_SetPinMux(BOARD_INITPINS_RESET_PORT, BOARD_INITPINS_RESET_PIN, kPORT_MuxAsGpio);

	    gpio_pin_config_t INTERRUPT_config = {
	        .pinDirection = kGPIO_DigitalInput,
	        .outputLogic = 0U
	    };*/
#endif
#ifdef WAKEUP_GPIO_INTERRUPT_METHOD
	  NVIC_SetPriority(PORTA_IRQn, 5);
#endif
   return;
}



/*===========================================================*/
/**
 * @fn            void rsi_hal_set_gpio(uint8_t gpio_number)
 * @brief         Makes/drives the gpio  value high
 * @param[in]     uint8_t gpio_number, gpio pin number
 * @param[out]    none
 * @return        none 
 * @description   This API is used to drives or makes the host gpio value high. 
 */
void rsi_hal_set_gpio(uint8_t gpio_number)
{

  //! drives a high value on GPIO 
  //! module reset pin
  if(gpio_number == RSI_HAL_RESET_PIN)
  {
#ifdef FRDM_K28F  
	  GPIO_WritePinOutput(BOARD_INITPINS_RESET_GPIO, BOARD_INITPINS_RESET_PIN, 1);
#endif	  
  }
  else if( gpio_number == RSI_HAL_SLEEP_CONFIRM_PIN)
	{
#ifdef FRDM_K28F
		GPIO_WritePinOutput(BOARD_INITPINS_PS_OUT_GPIO, BOARD_INITPINS_PS_OUT_PIN, 1);
#endif
	}

	//! Setting power save input GPIO to 1
	else if( gpio_number == RSI_HAL_WAKEUP_INDICATION_PIN)
	{
#ifdef FRDM_K28F
		GPIO_WritePinOutput(BOARD_INITPINS_PS_IN_GPIO, BOARD_INITPINS_PS_IN_PIN, 1);
#endif
	}
  return;
}




/*===========================================================*/
/**
 * @fn          uint8_t rsi_hal_get_gpio(void)
 * @brief       get the gpio pin value
 * @param[in]   uint8_t gpio_number, gpio pin number
 * @param[out]  none  
 * @return      gpio pin value 
 * @description This API is used to configure get the gpio pin value. 
 */
uint8_t rsi_hal_get_gpio(uint8_t gpio_number)
{
  uint8_t gpio_value = 0;
 
  //! Get the gpio value
  if(gpio_number == RSI_HAL_RESET_PIN)
  {
#ifdef FRDM_K28F
	  gpio_value = GPIO_ReadPinInput(BOARD_INITPINS_RESET_GPIO, BOARD_INITPINS_RESET_PIN);
#endif	  
  }

  //! SPI interrupt pin
  if(gpio_number == RSI_HAL_MODULE_INTERRUPT_PIN)
  {
#ifdef FRDM_K28F
	  gpio_value = GPIO_ReadPinInput(BOARD_INITPINS_INTERRUPT_GPIO, BOARD_INITPINS_INTERRUPT_PIN);
#endif
  }
//! Get PS sleep gpio value
	else if( gpio_number == RSI_HAL_SLEEP_CONFIRM_PIN)
	{
#ifdef FRDM_K28F
		gpio_value = GPIO_ReadPinInput(BOARD_INITPINS_PS_OUT_GPIO, BOARD_INITPINS_PS_OUT_PIN);
#endif
	}
	//! Get PS wakeup gpio value
	else if ( gpio_number == RSI_HAL_WAKEUP_INDICATION_PIN )
	{
#ifdef FRDM_K28F
		gpio_value = GPIO_ReadPinInput(BOARD_INITPINS_PS_IN_GPIO, BOARD_INITPINS_PS_IN_PIN);
#endif
	}
  return gpio_value;
}




/*===========================================================*/
/**
 * @fn            void rsi_hal_set_gpio(uint8_t gpio_number)
 * @brief         Makes/drives the gpio value to low
 * @param[in]     uint8_t gpio_number, gpio pin number
 * @param[out]    none
 * @return        none 
 * @description   This API is used to drives or makes the host gpio value low. 
 */
void rsi_hal_clear_gpio(uint8_t gpio_number)
{

  //! drives a low value on GPIO 
  if(gpio_number == RSI_HAL_RESET_PIN)
  {
#ifdef FRDM_K28F  
	  GPIO_WritePinOutput(BOARD_INITPINS_RESET_GPIO, BOARD_INITPINS_RESET_PIN, 0);
#endif	  
  }

	//! Setting power save output GPIO to 1
	else if( gpio_number == RSI_HAL_SLEEP_CONFIRM_PIN)
	{
#ifdef FRDM_K28F
		GPIO_WritePinOutput(BOARD_INITPINS_PS_OUT_GPIO, BOARD_INITPINS_PS_OUT_PIN, 0);
#endif
	}

	//! Setting power save input GPIO to 1
	else if( gpio_number == RSI_HAL_WAKEUP_INDICATION_PIN)
	{
#ifdef FRDM_K28F
		GPIO_WritePinOutput(BOARD_INITPINS_PS_IN_GPIO, BOARD_INITPINS_PS_IN_PIN, 0);
#endif
	}
  return;
}


