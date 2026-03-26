/*
  ***************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************

  File:		  	   tcpClientRAW.c
  Modified By:     ControllersTech.com
  Updated:    	   29-Jul-2021

  ***************************************************************************************************************
  Copyright (C) 2017 ControllersTech.com

  This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
  of the GNU General Public License version 3 as published by the Free Software Foundation.
  This software library is shared with public for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
  or indirectly by this software, read more about this on the GNU General Public License.

  ***************************************************************************************************************
*/


/**
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of and a contribution to the lwIP TCP/IP stack.
 *
 * Credits go to Adam Dunkels (and the current maintainers) of this software.
 *
 * Christiaan Simons rewrote this file to get a more stable  application.
 *
 **/

 /* This file was modified by ST */

#include "tcpClientRAW.h"

#include "lwip/tcp.h"

#include "string.h"


/*  protocol states */
enum tcp_client_states
{
  ES_NONE = 0,
  ES_CONNECTED,
  ES_RECEIVING,
  ES_CLOSING
};

/* structure for maintaining connection infos to be passed as argument
   to LwIP callbacks*/
struct tcp_client_struct
{
  u8_t state;             /* current connection state */
  u8_t retries;
  struct tcp_pcb *pcb;    /* pointer on the current tcp_pcb */
  struct pbuf *p;         /* pointer on the received/to be transmitted pbuf */
};



/* This callback will be called, when the client is connected to the server */
static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);

/* This callback will be called, when the client receive data from the server */
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

/* This callback will be called, when the server Polls for the Client */
static err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb);

/* This callback will be called, when the server acknowledges the data sent by the client */
static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

/* A Function to send the data to the server */
static void tcp_client_send(struct tcp_pcb *tpcb, struct tcp_client_struct *es);

/* Function to close the connection */
static void tcp_client_connection_close(struct tcp_pcb *tpcb, struct tcp_client_struct *es);

/* This is the part where we are going to handle the incoming data from the server */
static void tcp_client_handle (struct tcp_pcb *tpcb, struct tcp_client_struct *es, struct pbuf *p);


int counter = 0;
uint8_t data[100];
static char rx_buffer[257];

extern TIM_HandleTypeDef htim1;

struct tcp_client_struct *esTx = 0;
struct tcp_pcb *pcbTx = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    char buf[100];

    /* Format the message with the current counter value */
    int len = sprintf(buf, "Sending TCPclient Message %d\n", counter);

    if (esTx == NULL || pcbTx == NULL)
        return;

    /* Only send if the server has responded at least once */
    if (counter != 0)
    {
    	if (esTx->p != NULL) return;

        /* Allocate pbuf */
        esTx->p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);

        if (esTx->p == NULL) return;

        /* Copy data into pbuf */
        pbuf_take(esTx->p, (char *)buf, len);

        /* Send to server */
        tcp_client_send(pcbTx, esTx);
    }
}






/* IMPLEMENTATION FOR TCP CLIENT

1. Create TCP block.
2. connect to the server
3. start communicating
*/

void tcp_client_init(void)
{
    /* 1. Create new TCP PCB */
    struct tcp_pcb *tpcb;
    tpcb = tcp_new();

    /* 2. Connect to the server */
    if (tpcb != NULL)
    {
        ip_addr_t destIPADDR;
        IP_ADDR4(&destIPADDR, 192, 168, 0, 100);
        tcp_connect(tpcb, &destIPADDR, 31, tcp_client_connected);
    }
}

/** This callback is called, when the client is connected to the server
 * Here we will initialise few other callbacks
 * and in the end, call the client handle function
  */
static err_t tcp_client_connected(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    err_t ret_err;
    struct tcp_client_struct *es;

    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);

    /* Allocate structure to maintain connection info */
    es = (struct tcp_client_struct *)mem_malloc(sizeof(struct tcp_client_struct));

    if (es != NULL)
    {
        es->state = ES_CONNECTED;
        es->pcb = newpcb;
        es->retries = 0;
        es->p = NULL;

        /* Pass es structure to all subsequent callbacks */
        tcp_arg(newpcb, es);

        /* Register receive callback */
        tcp_recv(newpcb, tcp_client_recv);

        /* Register poll callback */
        tcp_poll(newpcb, tcp_client_poll, 0);

        /* Register sent callback */
        tcp_sent(newpcb, tcp_client_sent);

        /* Send the first message to the server */
        tcp_client_handle(newpcb, es, NULL);

        ret_err = ERR_OK;
    }
    else
    {
        /* Close connection on memory failure */
        tcp_client_connection_close(newpcb, es);
        ret_err = ERR_MEM;
    }

    return ret_err;
}


/** This callback is called, when the client receives some data from the server
 * if the data received is valid, we will handle the data in the client handle function
  */
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    struct tcp_client_struct *es;
    err_t ret_err;

    LWIP_ASSERT("arg != NULL", arg != NULL);
    es = (struct tcp_client_struct *)arg;

    /* Server closed the connection */
    if (p == NULL)
    {
        es->state = ES_CLOSING;
        if (es->p == NULL)
        {
            tcp_client_connection_close(tpcb, es);
        }
        else
        {
            tcp_sent(tpcb, tcp_client_sent);
            tcp_client_send(tpcb, es);
        }
        ret_err = ERR_OK;
    }
    /* Error on receive */
    else if (err != ERR_OK)
    {
        if (p != NULL)
        {
            es->p = NULL;
            pbuf_free(p);
        }
        ret_err = err;
    }
    /* Data received while connected */
    else if (es->state == ES_CONNECTED)
    {

        /* Acknowledge the received data */
        tcp_recved(tpcb, p->tot_len);

        /* Handle the received data */
        tcp_client_handle(tpcb, es, p);

        pbuf_free(p);

        ret_err = ERR_OK;
    }
    /* Any other state - discard data */
    else
    {
        tcp_recved(tpcb, p->tot_len);
        es->p = NULL;
        pbuf_free(p);
        ret_err = ERR_OK;
    }

    return ret_err;
}


static err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
  err_t ret_err;
  struct tcp_client_struct *es;

  es = (struct tcp_client_struct *)arg;
  if (es != NULL)
  {
    if (es->p != NULL)
    {
    	// do nothing
    }
    else
    {
      /* no remaining pbuf (chain)  */
      if(es->state == ES_CLOSING)
      {
        /*  close tcp connection */
        tcp_client_connection_close(tpcb, es);
      }
    }
    ret_err = ERR_OK;
  }
  else
  {
    /* nothing to be done */
    tcp_abort(tpcb);
    ret_err = ERR_ABRT;
  }
  return ret_err;
}

/** This callback is called, when the server acknowledges the data sent by the client
 * If there is no more data left to sent, we will simply close the connection
  */
static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  struct tcp_client_struct *es;

  LWIP_UNUSED_ARG(len);

  es = (struct tcp_client_struct *)arg;
  es->retries = 0;

  if(es->p != NULL)
  {
	  // do nothing
  }
  else
  {
    /* if no more data to send and client closed connection*/
    if(es->state == ES_CLOSING)
      tcp_client_connection_close(tpcb, es);
  }
  return ERR_OK;
}


/** A function to send the data to the server
  */
static void tcp_client_send(struct tcp_pcb *tpcb, struct tcp_client_struct *es)
{
    struct pbuf *ptr;
    err_t wr_err = ERR_OK;

    while ((wr_err == ERR_OK) &&
           (es->p != NULL) &&
           (es->p->len <= tcp_sndbuf(tpcb)))
    {
        /* Get current pbuf */
        ptr = es->p;

        /* Write data to TCP */
        wr_err = tcp_write(tpcb, ptr->payload, ptr->len, TCP_WRITE_FLAG_COPY);

        if (wr_err == ERR_OK)
        {
            /* Move to next pbuf */
            es->p = ptr->next;

            if (es->p != NULL)
            {
                /* Increase ref count for remaining chain */
                pbuf_ref(es->p);
            }

            /* Free the sent pbuf */
            pbuf_free(ptr);
        }
        else if (wr_err == ERR_MEM)
        {
            /* Not enough memory, retry later (poll/sent will handle) */
            es->p = ptr;
        }
        else
        {
            /* Other errors → stop sending */
            break;
        }
    }

    /* Force output */
    tcp_output(tpcb);
}


static void tcp_client_connection_close(struct tcp_pcb *tpcb, struct tcp_client_struct *es)
{
    err_t err;

    /* remove all callbacks */
    tcp_arg(tpcb, NULL);
    tcp_sent(tpcb, NULL);
    tcp_recv(tpcb, NULL);
    tcp_err(tpcb, NULL);
    tcp_poll(tpcb, NULL, 0);

    /* try to close connection */
    err = tcp_close(tpcb);

    if (err != ERR_OK)
    {
        /* if close fails, abort connection */
        tcp_abort(tpcb);
    }

    /* free es structure */
    if (es != NULL)
    {
        mem_free(es);
    }
}

/* Handle the incoming TCP Data */

static void tcp_client_handle(struct tcp_pcb *tpcb, struct tcp_client_struct *es, struct pbuf *p)
{
    /* Get the remote IP and port */
    ip4_addr_t inIP = tpcb->remote_ip;
    uint16_t inPort = tpcb->remote_port;
    char *remIP = ipaddr_ntoa(&inIP);

    /* store the received data for later use */
    if (p != NULL)
    {
        memcpy(rx_buffer, p->payload, p->len);
        rx_buffer[p->len] = '\0';
    }

    /* Store globally for use in the timer callback */
    esTx = es;
    pcbTx = tpcb;

    /* Increment counter each time server sends data */
    if (p != NULL)
    {
        counter++;
    }
}
