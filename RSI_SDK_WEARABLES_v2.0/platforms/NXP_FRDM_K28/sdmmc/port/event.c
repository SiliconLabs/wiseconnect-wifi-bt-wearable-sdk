/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
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

#include "rsi_driver.h"
#include "event.h"
#ifdef RSI_WITH_OS
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "stack_macros.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief Get event instance.
 * @param eventType The event type
 * @return The event instance's pointer.
 */
static volatile uint32_t *EVENT_GetInstance(event_t eventType);
extern rsi_driver_cb_t *rsi_driver_cb;
/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief Card detect event. */
static volatile uint32_t g_eventCardDetect;

/*! @brief transfer complete event. */
static volatile uint32_t g_eventTransferComplete;

/*! @brief Time variable unites as milliseconds. */
//volatile uint32_t g_timeMilliseconds;

/*! @brief Time variable as sys ticks. */
#ifndef RSI_WITH_OS
extern volatile uint32_t g_systicks;
#else
extern volatile TickType_t xTickCount;
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/
void EVENT_InitTimer(void)
{
    //! Set systick reload value to generate 1ms interrupt
    //SysTick_Config(CLOCK_GetFreq(kCLOCK_CoreSysClk) / 1000U);
}


static volatile uint32_t *EVENT_GetInstance(event_t eventType)
{
    volatile uint32_t *event;

    switch (eventType)
    {
        case kEVENT_TransferComplete:
            event = &g_eventTransferComplete;
            break;
        case kEVENT_CardDetect:
            event = &g_eventCardDetect;
            break;
        default:
            event = NULL;
            break;
    }

    return event;
}

bool EVENT_Create(event_t eventType)
{
    volatile uint32_t *event = EVENT_GetInstance(eventType);

    if (event)
    {
        *event = 0;
        return true;
    }
    else
    {
        return false;
    }
}

bool EVENT_Wait(event_t eventType, uint32_t timeoutMilliseconds)
{
    uint32_t startTime;
    uint32_t elapsedTime = 0;

    volatile uint32_t *event = EVENT_GetInstance(eventType);

#if TICK_100_USEC
    timeoutMilliseconds *= 10U;
#endif
    if (timeoutMilliseconds && event)
    {
#ifndef RSI_WITH_OS
        startTime = g_systicks;
#else
        startTime = xTickCount;
#endif
        do
        {
#if 0
        	//! mask event1
        	rsi_mask_event(RSI_APP_EVENT1);

        	//! call rsi schedular
        	rsi_scheduler(&rsi_driver_cb->scheduler_cb);

        	//! unmask event1
        	rsi_unmask_event(RSI_APP_EVENT1);
#endif
#ifndef RSI_WITH_OS
            elapsedTime = (g_systicks - startTime);
#else
            elapsedTime = (xTickCount - startTime);
#endif
        } while ((*event == 0U) && (elapsedTime < timeoutMilliseconds));

        *event = 0U;

        return ((elapsedTime < timeoutMilliseconds) ? true : false);
    }
    else
    {
        return false;
    }
}

bool EVENT_Notify(event_t eventType)
{
    volatile uint32_t *event = EVENT_GetInstance(eventType);

    if (event)
    {
        *event = 1U;
        return true;
    }
    else
    {
        return false;
    }
}

void EVENT_Delete(event_t eventType)
{
    volatile uint32_t *event = EVENT_GetInstance(eventType);

    if (event)
    {
        *event = 0U;
    }
}
