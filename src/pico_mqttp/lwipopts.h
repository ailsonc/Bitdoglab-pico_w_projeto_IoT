#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// ---- FreeRTOS + lwIP ----
#define NO_SYS                         0
#define LWIP_SOCKET                    0
#define LWIP_NETCONN                   0

// ---- THREAD / MAILBOX (ESSENCIAL) ----
#define TCPIP_MBOX_SIZE                8
#define DEFAULT_TCP_RECVMBOX_SIZE      8
#define DEFAULT_UDP_RECVMBOX_SIZE      8
#define DEFAULT_RAW_RECVMBOX_SIZE      8
#define DEFAULT_ACCEPTMBOX_SIZE        8

#define TCPIP_THREAD_STACKSIZE         2048
#define TCPIP_THREAD_PRIO              2

// ---- MEMÓRIA ----
#define MEM_ALIGNMENT                  4
#define MEM_SIZE                       8192
#define PBUF_POOL_SIZE                 24
#define MEMP_NUM_TCP_SEG               64
#define MEMP_NUM_SYS_TIMEOUT           24

// ---- PROTOCOLOS ----
#define LWIP_ARP                       1
#define LWIP_ETHERNET                  1
#define LWIP_ICMP                      1
#define LWIP_RAW                       1
#define LWIP_DHCP                      1

#define LWIP_DNS                       1
#define DNS_TABLE_SIZE                 4
#define DNS_MAX_NAME_LENGTH            256
#define DNS_MAX_SERVERS                2

#define LWIP_NETIF_STATUS_CALLBACK     1
#define LWIP_NETIF_LINK_CALLBACK       1
#define LWIP_NETIF_HOSTNAME            1

#define TCP_MSS                        1460
#define TCP_WND                        (8 * TCP_MSS)
#define TCP_SND_BUF                    (8 * TCP_MSS)
#define TCP_SND_QUEUELEN               ((4 * TCP_SND_BUF) / TCP_MSS)

#define LWIP_CHKSUM_ALGORITHM          3

// ---- MQTT ----
#define MQTT_OUTPUT_RINGBUF_SIZE       256
#define MQTT_VAR_HEADER_BUFFER_LEN     128
#define MQTT_REQ_MAX_IN_FLIGHT         4

#endif
