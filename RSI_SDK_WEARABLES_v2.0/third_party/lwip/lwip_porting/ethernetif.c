/***************************************************************************//**
 * @file
 * @brief Ethernet interface implementation for LwIP and WFX
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include "lwip/timeouts.h"
#include "netif/etharp.h"
#include "ethernetif.h"
#include <string.h>
#include "lwip/tcpip.h"
   
#define STATION_NETIF0 's'
#define STATION_NETIF1 't'
#define SOFTAP_NETIF0  'a'
#define SOFTAP_NETIF1  'p'
err_t low_level_output(struct netif *netif, struct pbuf *p);
extern uint8_t mac_address[6];
extern struct netif sta_netif;
void low_level_init(struct netif *netif);
/***************************************************************************//**
 *  LWIP_OS Initializes  
 * @param None 
 ******************************************************************************/
//! 
 void lwip_init_os()
 {
  tcpip_init(NULL, 0);
  netif_add (&sta_netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY, NULL,
             &sta_ethernetif_init, &tcpip_input);
  sta_netif.name[0] = 'S';
  sta_netif.name[1] = 'A';
  sta_netif.output = etharp_output;
  sta_netif.linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init (&sta_netif);
  netif_set_default (&sta_netif);
  netif_set_up (&sta_netif);
 }

/***************************************************************************//**
 * Initializes the hardware parameters. Called from ethernetif_init().
 *
 * @param netif The already initialized lwip network interface structure
 ******************************************************************************/
 void low_level_init(struct netif *netif)
{
  // set netif MAC hardware address length
  netif->hwaddr_len = ETH_HWADDR_LEN;

  // set netif MAC hardware address

    netif->hwaddr[0] =  mac_address[0];
    netif->hwaddr[1] =  mac_address[1];
    netif->hwaddr[2] =  mac_address[2];
    netif->hwaddr[3] =  mac_address[3];
    netif->hwaddr[4] =  mac_address[4];
    netif->hwaddr[5] =  mac_address[5];


  // set netif maximum transfer unit
  netif->mtu = 1500;

  // Accept broadcast address and ARP traffic
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

  // Set netif link flag
  netif->flags |= NETIF_FLAG_LINK_UP;
}






/***************************************************************************//**
 * Sets up the station network interface.
 *
 * @param netif the lwip network interface structure
 * @returns ERR_OK if successful
 ******************************************************************************/
err_t sta_ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));
#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip_sta";
#endif /* LWIP_NETIF_HOSTNAME */
  /* Set the netif name to identify the interface */
  netif->name[0] = STATION_NETIF0;
  netif->name[1] = STATION_NETIF1;
  
  netif->input = tcpip_input;
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);
  sta_netif = *netif;

  return ERR_OK;
}

