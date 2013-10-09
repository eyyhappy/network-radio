/* RT-Thread config file */
#ifndef __RTTHREAD_CFG_H__
#define __RTTHREAD_CFG_H__

/* RT_NAME_MAX*/
#define RT_NAME_MAX	8

/* RT_ALIGN_SIZE*/
#define RT_ALIGN_SIZE	4

/* PRIORITY_MAX*/
#define RT_THREAD_PRIORITY_MAX	32

/* Tick per Second*/
#define RT_TICK_PER_SECOND	100

/* SECTION: RT_DEBUG */
/* Thread Debug*/
#define RT_DEBUG
/* #define RT_THREAD_DEBUG */

#define RT_USING_OVERFLOW_CHECK

/* Using Hook*/
#define RT_USING_HOOK

/* SECTION: IPC */
/* Using Semaphore*/
#define RT_USING_SEMAPHORE

/* Using Mutex*/
#define RT_USING_MUTEX

/* Using Event*/
#define RT_USING_EVENT

/* Using MailBox*/
#define RT_USING_MAILBOX

/* Using Message Queue*/
#define RT_USING_MESSAGEQUEUE

/* SECTION: Memory Management */
/* Using Memory Pool Management*/
#define RT_USING_MEMPOOL

/* Using Dynamic Heap Management*/
#define RT_USING_HEAP

/* Using Small MM*/
#define RT_USING_SMALL_MEM

/* Using SLAB Allocator*/
/* #define RT_USING_SLAB */

/* SECTION: Device System */
/* Using Device System*/
#define RT_USING_DEVICE
#define RT_USING_UART1
// #define RT_USING_UART2
// #define RT_USING_UART3

/* SECTION: Console options */
/* the buffer size of console*/
#define RT_CONSOLEBUF_SIZE	128

/* SECTION: FinSH shell options */
/* Using FinSH as Shell*/
#define RT_USING_FINSH
/* Using symbol table */
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_DEVICE_NAME   "uart1"

/* SECTION: C++ support */
/* Using C++ support*/
/* #define RT_USING_CPLUSPLUS */

#define RT_USING_DFS
/* #define RT_USING_DFS_EFSL */
/* byte alignment for EFSL */
#define BYTE_ALIGNMENT

#define RT_USING_DFS_ELMFAT
#define RT_DFS_ELM_WORD_ACCESS
#define RT_DFS_ELM_REENTRANT        1
#define RT_DFS_ELM_DRIVES			2
#define RT_DFS_ELM_USE_LFN			2
#define RT_DFS_ELM_MAX_LFN			255
#define RT_DFS_ELM_MAX_SECTOR_SIZE  4096

/* SECTION: DFS options */
/* the max number of mounted filesystem */
#define DFS_FILESYSTEMS_MAX			2
/* the max number of opened files 		*/
#define DFS_FD_MAX					8
/* the max number of cached sector 		*/
#define DFS_CACHE_MAX_NUM   		4

/* SECTION: lwip, a lighwight TCP/IP protocol stack */
/* Using lighweight TCP/IP protocol stack */
#define RT_USING_LWIP

/* Trace LwIP protocol */
/* #define RT_LWIP_DEBUG */

/* LwIP uses RT-Thread Memory Management */
#define RT_LWIP_USING_RT_MEM

/* Enable ICMP protocol */
#define RT_LWIP_ICMP

/* Enable IGMP protocol */
/* #define RT_LWIP_IGMP */

/* Enable UDP protocol */
#define RT_LWIP_UDP

/* Enable TCP protocol */
#define RT_LWIP_TCP

/* the number of simulatenously active TCP connections */
#define RT_LWIP_TCP_PCB_NUM	3

/* TCP sender buffer space */
#define RT_LWIP_TCP_SND_BUF	2048

/* TCP Window Size */
#define RT_LWIP_TCP_WND	(1024 * 20)

/* Enable SNMP protocol */
/* #define RT_LWIP_SNMP */

/* Using DHCP */
#define RT_LWIP_DHCP

/* Using DNS */
#define RT_LWIP_DNS

/* ip address of target */
#define RT_LWIP_IPADDR0	192
#define RT_LWIP_IPADDR1	168
#define RT_LWIP_IPADDR2	1
#define RT_LWIP_IPADDR3	30

/* gateway address of target */
#define RT_LWIP_GWADDR0	192
#define RT_LWIP_GWADDR1	168
#define RT_LWIP_GWADDR2	1
#define RT_LWIP_GWADDR3	1

/* mask address of target */
#define RT_LWIP_MSKADDR0	255
#define RT_LWIP_MSKADDR1	255
#define RT_LWIP_MSKADDR2	255
#define RT_LWIP_MSKADDR3	0

/* tcp thread options */
#define RT_LWIP_TCPTHREAD_PRIORITY		10
#define RT_LWIP_TCPTHREAD_MBOX_SIZE		4
#define RT_LWIP_TCPTHREAD_STACKSIZE		1024

/* ethernet if thread options */
#define RT_LWIP_ETHTHREAD_PRIORITY		12
#define RT_LWIP_ETHTHREAD_MBOX_SIZE		4
#define RT_LWIP_ETHTHREAD_STACKSIZE		512

/* SECTION: RT-Thread/GUI */
#define RT_USING_RTGUI

/* name length of RTGUI object */
#define RTGUI_NAME_MAX		12
/* support 16 weight font */
#define RTGUI_USING_FONT16
/* support Chinese font */
#define RTGUI_USING_FONTHZ
/* use DFS as file interface */
#define RTGUI_USING_DFS_FILERW
/* use font file as Chinese font */
#define RTGUI_USING_HZ_FILE
/* use small size in RTGUI */
#define RTGUI_USING_SMALL_SIZE
/* use mouse cursor */
/* #define RTGUI_USING_MOUSE_CURSOR */
/* default font size in RTGUI */
#define RTGUI_DEFAULT_FONT_SIZE	16

#endif
