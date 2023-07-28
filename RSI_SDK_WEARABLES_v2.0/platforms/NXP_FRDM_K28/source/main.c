/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "rsi_common_app.h"
#include "board.h"
#include "pin_mux.h"
#include "fsl_sysmpu.h"
#include "fsl_dspi.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern int32_t rsi_app_main(void);
extern void lptmr_init(void);
extern int8_t sd_filesystem_init(void);
extern dspi_master_config_t spi_init(void);
extern dspi_master_config_t spi_dma_init(void);
extern int sdram_init(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
	BOARD_InitPins();
	//BOARD_BootClockRUN();
	BOARD_BootClockHSRUN();
	BOARD_InitDebugConsole();
	SYSMPU_Enable(SYSMPU, false);

#ifndef RSI_WITH_OS
	//Initialize systick
#if TICK_100_USEC
	//! Set systick reload value to generate 100us interrupt
	if(SysTick_Config(SystemCoreClock / 10000U))
#elif TICK_1_MSEC
	//! Set systick reload value to generate 1ms interrupt
	if(SysTick_Config(SystemCoreClock / 1000U))
#endif
	{
		while(1)
		{
		}
	}
#endif

	//! SDRAM Init
	sdram_init();

	//! Initialize SD FILESYSTEM
	sd_filesystem_init();

	//! Initialize SPI
#ifndef SPI_DMA
	spi_init();
#else
	spi_dma_init();
#endif

	//! APPLICATION MAIN
	rsi_app_main();
}

