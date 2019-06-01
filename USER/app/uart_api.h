#ifndef _USART_API_H_
#define _USART_API_H_

#define  CPU_IntDis()           { __set_PRIMASK(1); }	/* 关中断 */
#define  CPU_IntEn()            { __set_PRIMASK(0); }	/* 开中断 */

#define RX_LENGTH  254

#define COMM_REC_IDLE					0
#define COMM_REC_WAITPREFIX				1
#define COMM_REC_RECEIVING				2
#define COMM_REC_OVER					3
#define COMM_REC_GNRMC				    4
#define COMM_REC_GNGGA				    5

#define COMM_SEND_IDLE					0
#define COMM_SEND_DATA					1
#define COMM_SEND_POSTFIX				2
#define COMM_SEND_OVER					3

/* 全局参数 */
typedef struct
{
	USART_TypeDef *uart;	/*  */	
	uint8_t *pTxBuf;			/* 发送缓冲区 */
	uint8_t *pRxBuf;			/* 接收缓冲区 */
	uint16_t usTxBufSize;		/* 发送缓冲区大小 */
	uint16_t usRxBufSize;		/* 接收缓冲区大小 */
	uint16_t usTxWrite;		/* 发送缓冲区写指针 */
	uint16_t usTxRead;		/* 发送缓冲区读指针 */
	uint16_t usRxWrite;		/* 接收缓冲区写指针 */
	uint16_t usRxRead;		/* 接收缓冲区读指针 */
	void (*SendOver)(void); 	/* 发送完毕的回调函数指针 */
	void (*ReciveNew)(void);	/* 串口收到数据的回调函数指针 */
}UART_T;
 
#define UART1_TX_BUF_SIZE	32
#define UART1_RX_BUF_SIZE	32


typedef struct buf_t{
	uint16_t	len;
	uint8_t	data[UART1_RX_BUF_SIZE];
} BUF;


/* 定义端口号 */
typedef enum
{
	COM1 = 0,	/* COM1口, RS232 */
	COM2 = 1,	/* COM2口, RS232 */
	COM3 = 2	/* COM3口, RS485 */
}PORT_NO_E;


void UART_Configuration(unsigned int bauds);
void comSendBuf(uint8_t _ucPort, uint8_t *_ucaBuf, uint16_t _usLen);
void comSendChar(uint8_t _ucPort, uint8_t _ucByte);
uint8_t comGetChar(uint8_t _ucPort, uint8_t *_pByte);

void USART1_IRQHandler(void);
extern void USART1_Config(void);
extern void uart1_send(uint8 *sp,uint16 len) ;

void USART2_IRQHandler(void);
extern void USART2_Config(void);
extern void uart2_send(uint8 *sp,uint16 len) ;

void USART3_IRQHandler(void);
extern void USART3_Config(void);
extern void uart3_send(uint8 *sp,uint16 len) ;

void UART4_IRQHandler(void);
extern void UART4_Config(void);

void UART5_IRQHandler(void);
extern void UART5_Config(void);
extern void uart5_send(uint8 *sp,uint8 len) ;
uint8 GetGTZMHDCmdXOR(void);
uint8 CheckGTZMHDRecXOR(void);
void SetState1GTZMHD(void);
void SetState2GTZMHD(void);
void SendGTZMHDCmdSetRet(u8 ret);
void GetState1GTZMHD(void);
void GetState2GTZMHD(void);
void SendGTZMHDCmdGetRet(u8 ret);
#endif
