/*******************************************************************************
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
/*=======================================================================*/
//   ! HEADER FILES 
/*=======================================================================*/
#include "lwip/pbuf.h"
#include "rsi_common_apis.h"
#include "rsi_common_app.h"
/*=======================================================================*/
//   ! EXTERN VARIABLES
/*=======================================================================*/
extern  struct netif sta_netif;

/***************************************************************************//**
 * Transmits packet(s).
 *
 * @param netif the lwip network interface structure
 * @param p the packet to send
 * @returns ERR_OK if successful
 ******************************************************************************/
err_t low_level_output(struct netif *netif, struct pbuf *p)
{

  int32_t status = RSI_SUCCESS;

    status = rsi_send_raw_data((uint8_t *)p->payload, p->len);
    if (status != ERR_OK)
    {
        return status;
    }
    return ERR_OK;
}


/***************************************************************************//**
 * Receive  packet(s).
 *
 * @param length of the received packet
 * @param buffer the packet received from wifi chip
 * @returns status 0 if successful
 ******************************************************************************/

void wlan_data_receive_handler (uint16_t status, uint8_t *buffer, const uint32_t length)
{
  struct pbuf *recv_pkt = NULL;
   recv_pkt = pbuf_alloc (PBUF_RAW, sizeof(struct pbuf), PBUF_POOL);
   if (recv_pkt != NULL)
   {
  recv_pkt->next = NULL;
  memcpy (recv_pkt->payload, buffer, length);
  recv_pkt->len = length;
  recv_pkt->tot_len = length;
  tcpip_input(recv_pkt, &sta_netif);
   }
}
