/*
 * The Clear BSD License
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
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
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
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
#include "fsl_device_registers.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_sdram.h"
#include "pin_mux.h"

#include "fsl_common.h"
#include "clock_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SDRAMC SDRAM
#define SDRAM_START_ADDRESS (0x70000000U)
#define BUS_CLK_FREQ CLOCK_GetFreq(kCLOCK_FlexBusClk)

#define SDRAM_EXAMPLE_DATALEN (0x1000U)
#define SDRAM_EXAMPLE_WRITETIMES (1000U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
//uint32_t *sdram = (uint32_t *)SDRAM_START_ADDRESS; /* SDRAM start address. */
//uint8_t *sdram = (uint8_t *)SDRAM_START_ADDRESS; /* SDRAM start address. */
uint8_t *sdram = (uint8_t *)SDRAM_START_ADDRESS; /* SDRAM start address. */

uint32_t sdram_writeBuffer[SDRAM_EXAMPLE_DATALEN];
uint32_t sdram_readBuffer[SDRAM_EXAMPLE_DATALEN];

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int sdram_init(void)
{
    uint32_t clockSrc;

    /* Hardware initialize. */
    uint32_t soptReg;
    uint32_t fbReg;
   /* BOARD_InitPins();
    BOARD_InitPinsForUart();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();*/

    /* Set clock out to flexbus CLKOUT. */
    CLOCK_SetClkOutClock(0);

    /* Sets the Flexbus security level*/
    soptReg = SIM->SOPT2 & ~SIM_SOPT2_FBSL_MASK;
    SIM->SOPT2 = soptReg | SIM_SOPT2_FBSL(3);

    /* Enable the FB_BE_xx_yy signal in Flexbus */
    CLOCK_EnableClock(kCLOCK_Flexbus0);

    fbReg = FB->CSPMCR & ~FB_CSPMCR_GROUP2_MASK;
    FB->CSPMCR = fbReg | FB_CSPMCR_GROUP2(2);
    fbReg = FB->CSPMCR & ~FB_CSPMCR_GROUP3_MASK;
    FB->CSPMCR = fbReg | FB_CSPMCR_GROUP3(2);
    fbReg = FB->CSPMCR & ~FB_CSPMCR_GROUP4_MASK;
    FB->CSPMCR = fbReg | FB_CSPMCR_GROUP4(2);
    fbReg = FB->CSPMCR & ~FB_CSPMCR_GROUP5_MASK;
    FB->CSPMCR = fbReg | FB_CSPMCR_GROUP5(2);
    /* SDRAM initialize. */
    clockSrc = BUS_CLK_FREQ;

    if (SDRAM_Init(EXAMPLE_SDRAMC, (uint32_t)SDRAM_START_ADDRESS, clockSrc) != kStatus_Success)
    {
        PRINTF("\r\n SDRAM Init Failed\r\n");
        return -1;
    }

    memset(sdram, 0, (8192*512*2));

#if 0
    PRINTF("\r\n SDRAM Memory Write Start, Start Address 0x%x, Data Length %d !\r\n", sdram, datalen);
    /* Prepare data and write to SDRAM. */
    for (index = 0; index < datalen; index++)
    {
        sdram_writeBuffer[index] = index;
        *(uint32_t *)(sdram + index) = sdram_writeBuffer[index];
    }
    PRINTF("\r\n SDRAM Write finished!\r\n");

    PRINTF("\r\n SDRAM Read Start, Start Address 0x%x, Data Length %d !\r\n", sdram, datalen);
    /* Read data from the SDRAM. */
    for (index = 0; index < datalen; index++)
    {
        sdram_readBuffer[index] = *(uint32_t *)(sdram + index);
    }
    PRINTF("\r\n SDRAM Read finished!\r\n");

    PRINTF("\r\n SDRAM Write Data and Read Data Compare Start!\r\n");
    /* Compare the two buffers. */
    while (datalen--)
    {
        if (sdram_writeBuffer[datalen] != sdram_readBuffer[datalen])
        {
            result = false;
            PRINTF("\r\n SDRAM Write Data and Read Data Check Error!\r\n");
            break;
        }
    }

#if (defined TWR_K81F150M) || (defined TWR_K80F150M)
    /* For K80 serial board debug console, the sdramc pin is conflict with the uart pin
     so configure uart pin to enable the log print at the end of the example. */
    BOARD_InitPinsForUart();
#endif
    if (result)
    {
        PRINTF("\r\n SDRAM Write Data and Read Data Succeed.\r\n");
    }
    else
    {
        PRINTF("\r\n SDRAM Write Data and Read Data Failed.\r\n");
    }
    PRINTF("\r\n SDRAM Example End.\r\n");
    while (1)
    {
    }
#endif
    return 0;
}
