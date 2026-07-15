/*
 * Modbus_Server.c
 *
 *  Created on: 29-Jun-2026
 *      Author: arunrawat
 */


#include "lwip/tcp.h"
#include "modbus.h"
#include "Modbus_Server.h"
#include "Modbus_Parser.h"

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void  tcp_server_error(void *arg, err_t err);
static void  tcp_server_connection_close(struct tcp_pcb *tpcb);

/*-----------------------------------------------------------*/

err_t Modbus_Server_Init(void)
{
    struct tcp_pcb *tpcb;

    printf("\r\n=====================================\r\n");
    printf("Initializing Modbus TCP Server...\r\n");
    printf("Port : %d\r\n", MB_TCP_PORT);

    /* Create a new TCP PCB */
    tpcb = tcp_new();

    if(tpcb == NULL)
    {
        printf("ERROR: tcp_new() failed!\r\n");
        return ERR_MEM;
    }

    printf("TCP PCB Created Successfully\r\n");

    /* Bind to Modbus TCP Port */
    if(tcp_bind(tpcb, IP_ADDR_ANY, MB_TCP_PORT) != ERR_OK)
    {
        printf("ERROR: Failed to bind to port %d\r\n", MB_TCP_PORT);

        tcp_close(tpcb);
        return ERR_ABRT;
    }

    printf("Successfully Bound to Port %d\r\n", MB_TCP_PORT);

    /* Put PCB into listening state */
    tpcb = tcp_listen(tpcb);
    /* Register Accept Callback */
    tcp_accept(tpcb, tcp_server_accept);

    printf("Server is Listening...\r\n");
    printf("=====================================\r\n\r\n");

    return ERR_OK;
}

/*-----------------------------------------------------------*/

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);

	tcp_setprio(newpcb, TCP_PRIO_MIN);

	tcp_recv(newpcb, tcp_server_recv);
	tcp_err(newpcb, tcp_server_error);

	return ERR_OK;
}

/*-----------------------------------------------------------*/

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	LWIP_UNUSED_ARG(arg);
	uint8_t rxBuf[260];

	if((err != ERR_OK) || (p == NULL))
	{
		if(p != NULL) pbuf_free(p);

		tcp_server_connection_close(tpcb);
		return ERR_OK;
	}

	tcp_recved(tpcb, p->tot_len);

	/* Parse request and transmit response */
	pbuf_copy_partial(p, rxBuf, p->tot_len, 0);

	MB_Parser(tpcb, rxBuf, p->tot_len);

	pbuf_free(p);

	return ERR_OK;
}

/*-----------------------------------------------------------*/

static void tcp_server_error(void *arg, err_t err)
{
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
}

/*-----------------------------------------------------------*/

static void tcp_server_connection_close(struct tcp_pcb *tpcb)
{
	tcp_arg(tpcb, NULL);
	tcp_recv(tpcb, NULL);
	tcp_sent(tpcb, NULL);
	tcp_err(tpcb, NULL);
	tcp_poll(tpcb, NULL, 0);

	tcp_close(tpcb);
}

