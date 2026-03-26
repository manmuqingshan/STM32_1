# STM32 Ethernet – TCP Client (Raw API)

This project demonstrates how to implement a **TCP Client** on STM32 using the lwIP Raw API. The STM32 connects to a remote server, receives data, and sends periodic messages back every second using a hardware timer. We use Hercules on a PC as the TCP Server for testing.

This is **Part 5** of the STM32 Ethernet (lwIP) series.

---

## Features Covered

- Creating a TCP PCB with `tcp_new()`
- Connecting to a remote server with `tcp_connect()`
- Handling the connection setup inside `tcp_client_connected()`
- Registering receive, poll and sent callbacks
- Receiving server data via `tcp_client_recv()`
- Processing incoming data inside `tcp_client_handle()`
- Sending periodic data to the server using a hardware timer
- Proper memory management — allocating and freeing pbufs correctly

---

### TCP Server vs TCP Client

| Role | Who Initiates | STM32 Use Case |
|------|--------------|----------------|
| TCP Server | Waits for client to connect | STM32 receives commands from a PC |
| TCP Client | STM32 reaches out to server | STM32 sends data to a remote server |

---

## Tested On

| Board | Series | Interface |
|-------|--------|-----------|
| Discovery H745 | H7 | MII |

The CubeMX and lwIP configuration is identical to Part 1 of this series. Refer to the `hardware-ping` README for full hardware and CubeMX setup details.

---

## IP Address Configuration

| Device | IP Address | Port |
|--------|-----------|------|
| STM32 (Client) | `192.168.0.123` | Any |
| PC / Hercules (Server) | `192.168.0.100` | `31` |

---

## How It Works
```
1. tcp_new()                  -> Create TCP control block (PCB)
2. tcp_connect()              -> Connect to server IP and port 31
3. tcp_client_connected()     -> Connection established; set up recv, poll, sent callbacks
4. tcp_client_recv()          -> Server sends data; store pbuf, call handle
5. tcp_client_handle()        -> Process incoming data, increment counter
6. HAL_TIM_PeriodElapsedCallback() -> Send message to server every second
```

---

## Key Code

### Initialize the TCP Client
```c
void tcp_client_init(void)
{
    struct tcp_pcb *tpcb;
    tpcb = tcp_new();

    if (tpcb != NULL)
    {
        ip_addr_t destIPADDR;
        IP_ADDR4(&destIPADDR, 192, 168, 0, 100);
        tcp_connect(tpcb, &destIPADDR, 31, tcp_client_connected);
    }
}
```

### Connected Callback
```c
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
```

### Client Handle — Process Incoming Data
```c
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
    counter++;
}
```

### Timer Callback — Send Data Every Second
```c
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
```

### main.c
```c
#include "tcpClientRAW.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_LWIP_Init();

    tcp_client_init();
    HAL_TIM_Base_Start_IT(&htim1);

    while (1)
    {
        MX_LWIP_Process();
    }
}
```

---

## Result

The image below shows Hercules acting as the TCP Server, receiving periodic messages from the STM32 client. The counter value updates every time Hercules sends data back to the STM32.

![TCP Client Result](images/tcp_client_result.avif)

---

## Full Tutorial and Explanation

Step-by-step explanation, CubeMX screenshots, and video walkthrough available at:

👉 https://controllerstech.com/stm32-ethernet-5-tcp-client/

---

## Related Tutorials

| Part | Topic | Link |
|------|-------|------|
| Part 1 | Hardware Setup, lwIP & Ping Test | https://controllerstech.com/stm32-ethernet-hardware-cubemx-lwip-ping/ |
| Part 2 | UDP Server | https://controllerstech.com/stm32-ethenret-2-udp-server/ |
| Part 3 | UDP Client | https://controllerstech.com/stm32-ethernet-3-udp-client/ |
| Part 4 | TCP Server | https://controllerstech.com/stm32-ethernet-4-tcp-server/ |
| Part 6 | HTTP Web Server (Simple) | https://controllerstech.com/stm32-ethernet-6-http-webserver-simple/ |
| Part 7 | UDP Server using NETCONN (RTOS) | https://controllerstech.com/udp-server-using-netconn-with-rtos-in-stm32/ |

---

## License

This example is provided for educational purposes under Controllerstech Guidelines.
