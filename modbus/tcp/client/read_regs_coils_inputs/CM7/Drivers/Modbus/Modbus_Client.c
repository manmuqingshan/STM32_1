/*
 * Modbus_Client.c
 *
 *  Created on: 27-Jul-2026
 *      Author: arunrawat
 */

#include "lwip/tcp.h"
#include "modbus.h"
#include "Modbus_Client.h"
#include "Modbus_Client_Parser.h"

struct tcp_pcb *ClientPCB = 0;
static uint8_t ClientConnected = 0;

static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_client_error(void *arg, err_t err);
static void  tcp_client_connection_close(struct tcp_pcb *tpcb);

/*-----------------------------------------------------------*/

err_t Modbus_Client_Init(char *serverIP, uint16_t port)
{
	ip_addr_t destIPADDR;

	ClientPCB = tcp_new();

    if(ClientPCB == NULL)
    {
        printf("ERROR: tcp_new() failed!\r\n");
        return ERR_MEM;
    }

    ipaddr_aton(serverIP, &destIPADDR);

    printf("Connecting to the Server...\r\n");
    printf("IP : %s\r\n", serverIP);
    printf("Port   : %d\r\n", port);

    return tcp_connect(ClientPCB, &destIPADDR, port, tcp_client_connected);
}



void Modbus_Client_Process(char *serverIP, uint16_t port)
{
#define TCP_CONNECT_TIMEOUT_MS    5000
#define TCP_MAX_RETRIES           5

	extern struct netif gnetif;
	static uint8_t isServerOK = 1;
	static uint8_t tcp_started = 0;
	static uint8_t tcp_retries = 0;
	static uint32_t tcp_connect_start_tick = 0;

	if (isServerOK)
	{
		/* Wait until the Ethernet interface is ready */
		if (!netif_is_up(&gnetif) || !netif_is_link_up(&gnetif))
		{
			tcp_started = 0;
			return;
		}

		/* Start TCP Client */
		if (!tcp_started)
		{
			printf("\r\natStarting Modbus Client...\r\n");

			if (Modbus_Client_Init(serverIP, port) == ERR_OK)
			{
				tcp_started = 1;
				tcp_connect_start_tick = HAL_GetTick();
			}

			return;
		}

		/* Connection Timeout */
		if (!ClientConnected)
		{
			if ((HAL_GetTick() - tcp_connect_start_tick) > TCP_CONNECT_TIMEOUT_MS)
			{
				printf("Connection Timeout\r\n");

				tcp_started = 0;

				tcp_retries++;

				if (tcp_retries >= TCP_MAX_RETRIES)
				{
					printf("Maximum retries reached\r\n");
					isServerOK = 0;
				}
			}
		}
	}
}


static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	printf("Connected to Modbus Server\r\n");
	ClientConnected = 1;

	// Register Callbacks
    tcp_recv(ClientPCB, tcp_client_recv);
    tcp_err(ClientPCB, tcp_client_error);

	return ERR_OK;
}


static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    uint8_t rxBuf[260];

    if((err != ERR_OK) || (p == NULL))
    {
        if(p != NULL) pbuf_free(p);

        tcp_client_connection_close(tpcb);

        return ERR_OK;
    }

    tcp_recved(tpcb, p->tot_len);

    pbuf_copy_partial(p, rxBuf, p->tot_len, 0);

    MB_Client_Parser(tpcb, rxBuf, p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

static void tcp_client_connection_close(struct tcp_pcb *tpcb)
{
    ClientConnected = 0;

    tcp_recv(tpcb, NULL);
    tcp_err(tpcb, NULL);

    tcp_close(tpcb);

    ClientPCB = NULL;
}

static void tcp_client_error(void *arg, err_t err)
{
    printf("TCP ERROR = %d\r\n", err);
}

err_t TCP_SendRequest(uint8_t *txBuf, uint16_t length)
{
    if(ClientPCB == NULL)
        return ERR_CONN;

    err_t err;

    err = tcp_write(ClientPCB, txBuf, length, TCP_WRITE_FLAG_COPY);
    if(err != ERR_OK)
        return err;

    return tcp_output(ClientPCB);
}

