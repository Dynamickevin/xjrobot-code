/****************************************************************************
	注意：
	(1) 中断程序未对发送缓冲区满做处理.也就是说调用comSendBuf()函数时，如果
		开辟的发送缓冲区不足，将导致最先放入FIFO的数据被冲掉。
		不过对以一般的应用程序，将发送缓冲区开的足够大以保证一个最长的通信帧能够容纳下即可。*/
/* Includes ------------------------------------------------------------------*/
#include <includes.h>
#include "stm32f10x.h"
#include <stdio.h>
#include "uart_api.h"

/* 定义串口结构体变量 */
static UART_T g_tUart1;

static void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen);
static uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte);

OS_EVENT	*Usart1Sem;
AtCmdInfo AtCmdFromDebug;
ZT_INFO_TYPE g_zt_msg1;

OS_EVENT	*Usart2Sem;
AtCmdInfo AtCmdFromLinux;
ZT_INFO_TYPE g_zt_msg2;

OS_EVENT	*Usart3Sem;
AtCmdInfo AtCmdFromRF433;
ZT_INFO_TYPE g_zt_msg3;

OS_EVENT	*Uart4Sem;
AtCmdInfo AtCmdFromGPS;
ZT_INFO_TYPE g_zt_msg4;

OS_EVENT	*Uart5Sem;
AtCmdInfo AtCmdFromPTZ;
ZT_INFO_TYPE g_zt_msg5;

//jy_901_data_type gJy901Data;
//static u8 gBufFor_SendJY_901[4];
//static u8 gBufFor_RecvJY_901[12];
//static u8 gnJY_901_RecvCnt = 0;

//CmdParaInfo gCmdParaInfos;//,gCmdParaInfos_RF433,gCmdParaInfos_DEBUG,gCmdParaInfos_LINUX;
extern RbtState gRbtState;

/****************************************************************************
* 函数名: comSendBuf
* 功  能: 向串口发送一组数据，非阻塞。
* 输  入: 	_ucPort: 端口号(COM1)
*			_ucaBuf: 待发送的数据缓冲区
*			_usLen : 数据长度
* 输  出: 无
* 返  回: 无
*/
void comSendBuf(uint8_t _ucPort, uint8_t *_ucaBuf, uint16_t _usLen)
{
	UART_T *pUart;
		
	if (_ucPort == COM1)
	{
		pUart = &g_tUart1;
		
	}
	else
	{
		/* 不做任何处理 */
		return;
	}

	UartSend(pUart, _ucaBuf, _usLen);	
}

/****************************************************************************
* 函数名: comSendChar
* 功  能: 向串口发送1个字节，非阻塞。
* 输  入: 	_ucPort: 端口号(COM1 COM2 COM3)
*			_ucByte: 待发送的数据
* 输  出: 无
* 返  回: 无
*/
void comSendChar(uint8_t _ucPort, uint8_t _ucByte)
{
	comSendBuf(_ucPort, &_ucByte, 1);
}

/****************************************************************************
* 函数名: comGetChar
* 功  能: 从串口缓冲区读取1字节，非阻塞。
* 输  入: _pByte 结果存放地址
* 输  出: 无
* 返  回: 0 表示无数据, 1 表示读取到有效字节
*/
uint8_t comGetChar(uint8_t _ucPort, uint8_t *_pByte)
{
	UART_T *pUart;
	
	if (_ucPort == COM1)
	{
		pUart = &g_tUart1;
	}
	else
	{
		return 0;
	}
	
	return UartGetChar(pUart, _pByte);
}


/* this is for DEBUG module */
void USART1_Config(void)
{
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_TypeDef *uart;
  
  /* 第1步：打开GPIOA和USART1部件的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  
 /* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 第3步：将USART Rx的GPIO配置为浮空输入模式
		由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
		但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*  第3步已经做了，因此这步可以不做
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/

  /* Enable the USART OverSampling by 8 */
  USART_OverSampling8Cmd(USART1, ENABLE);  

  /* USARTx configuration ----------------------------------------------------*/
  /* USARTx configured as follow:
        - BaudRate = 5250000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 8
		     is: (USART APB Clock / 8) 
			 Example: 
			    - (USART3 APB1 Clock / 8) = (42 MHz / 8) = 5250000 baud
			    - (USART1 APB2 Clock / 8) = (84 MHz / 8) = 10500000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 16
		     is: (USART APB Clock / 16) 
			 Example: (USART3 APB1 Clock / 16) = (42 MHz / 16) = 2625000 baud
			 Example: (USART1 APB2 Clock / 16) = (84 MHz / 16) = 5250000 baud
        - Word Length = 8 Bits
        - one Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */ 
  /* 第4步：设置串口硬件参数 */
	uart = USART1;	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(uart, &USART_InitStructure);
	/* 
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		注意: 不要在此处打开发送中断
		发送中断使能在SendUart()函数打开
	*/
	

	/* CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
		如下语句解决第1个字节无法正确发送出去的问题 */
	USART_ClearFlag(USART1, USART_FLAG_TC);     /* 清发送外城标志，Transmission Complete flag */
  
	#if 1
  /* NVIC configuration */
  /* Configure the Priority Group to 2 bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  #endif

  
  /* Enable USART */
	AtCmdFromDebug.counter = 0;
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    Usart1Sem = OSSemCreate(1);

	USART_Cmd(uart, ENABLE);		/* 使能串口 */ 

}

/* this is for LINUX module */
void USART2_Config(void)
{
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_TypeDef *uart;
  
  /* 第1步：打开GPIO和USART部件的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);////////////////////////////////////20180926 add
  
 /* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* 第3步：将USART Rx的GPIO配置为浮空输入模式
		由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
		但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	/*  第3步已经做了，因此这步可以不做
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/

  /* Enable the USART OverSampling by 8 */
  USART_OverSampling8Cmd(USART2, ENABLE);  

  /* USARTx configuration ----------------------------------------------------*/
  /* USARTx configured as follow:
        - BaudRate = 5250000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 8
		     is: (USART APB Clock / 8) 
			 Example: 
			    - (USART3 APB1 Clock / 8) = (42 MHz / 8) = 5250000 baud
			    - (USART1 APB2 Clock / 8) = (84 MHz / 8) = 10500000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 16
		     is: (USART APB Clock / 16) 
			 Example: (USART3 APB1 Clock / 16) = (42 MHz / 16) = 2625000 baud
			 Example: (USART1 APB2 Clock / 16) = (84 MHz / 16) = 5250000 baud
        - Word Length = 8 Bits
        - one Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */ 
  /* 第4步：设置串口硬件参数 */
	uart = USART2;	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(uart, &USART_InitStructure);
	/* 
		USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
		注意: 不要在此处打开发送中断
		发送中断使能在SendUart()函数打开
	*/


	/* CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
		如下语句解决第1个字节无法正确发送出去的问题 */
	USART_ClearFlag(USART2, USART_FLAG_TC);     /* 清发送外城标志，Transmission Complete flag */
  
	#if 1
  /* NVIC configuration */
  /* Configure the Priority Group to 2 bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  #endif

  
  /* Enable USART */
	AtCmdFromLinux.counter = 0;
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
    Usart2Sem = OSSemCreate(1);
   	USART_Cmd(uart, ENABLE);		/* 使能串口 */ 

}


/* this is for RF433 module */
void USART3_Config(void)
{
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_TypeDef *uart;
  
  /* 第1步：打开GPIO和USART部件的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD| RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);////////////////////////////////////20180926 add
    
 /* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* 第3步：将USART Rx的GPIO配置为浮空输入模式
		由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
		但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	/*  第3步已经做了，因此这步可以不做
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/

  /* Enable the USART OverSampling by 8 */
  USART_OverSampling8Cmd(USART3, ENABLE);  

  /* USARTx configuration ----------------------------------------------------*/
  /* USARTx configured as follow:
        - BaudRate = 5250000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 8
		     is: (USART APB Clock / 8) 
			 Example: 
			    - (USART3 APB1 Clock / 8) = (42 MHz / 8) = 5250000 baud
			    - (USART1 APB2 Clock / 8) = (84 MHz / 8) = 10500000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 16
		     is: (USART APB Clock / 16) 
			 Example: (USART3 APB1 Clock / 16) = (42 MHz / 16) = 2625000 baud
			 Example: (USART1 APB2 Clock / 16) = (84 MHz / 16) = 5250000 baud
        - Word Length = 8 Bits
        - one Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */ 
  /* 第4步：设置串口硬件参数 */
	uart = USART3;	
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(uart, &USART_InitStructure);
	/* 
		USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
		注意: 不要在此处打开发送中断
		发送中断使能在SendUart()函数打开
	*/


	/* CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
		如下语句解决第1个字节无法正确发送出去的问题 */
	USART_ClearFlag(USART3, USART_FLAG_TC);     /* 清发送外城标志，Transmission Complete flag */
  
	#if 1
  /* NVIC configuration */
  /* Configure the Priority Group to 2 bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  #endif

  
  /* Enable USART */
	AtCmdFromRF433.counter = 0;
	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
	
   Usart3Sem = OSSemCreate(1);
   	USART_Cmd(uart, ENABLE);		/* 使能串口 */ 

}

/* this is for GPS module */
void UART4_Config(void)
{
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_TypeDef *uart;
  
  /* 第1步：打开GPIO和USART部件的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
  
 /* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* 第3步：将USART Rx的GPIO配置为浮空输入模式
		由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
		但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	/*  第3步已经做了，因此这步可以不做
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/

  /* Enable the USART OverSampling by 8 */
  USART_OverSampling8Cmd(UART4, ENABLE);  

  /* USARTx configuration ----------------------------------------------------*/
  /* USARTx configured as follow:
        - BaudRate = 5250000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 8
		     is: (USART APB Clock / 8) 
			 Example: 
			    - (USART3 APB1 Clock / 8) = (42 MHz / 8) = 5250000 baud
			    - (USART1 APB2 Clock / 8) = (84 MHz / 8) = 10500000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 16
		     is: (USART APB Clock / 16) 
			 Example: (USART3 APB1 Clock / 16) = (42 MHz / 16) = 2625000 baud
			 Example: (USART1 APB2 Clock / 16) = (84 MHz / 16) = 5250000 baud
        - Word Length = 8 Bits
        - one Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */ 
  /* 第4步：设置串口硬件参数 */
	uart = UART4;	
	USART_InitStructure.USART_BaudRate = 9600;//4800;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(uart, &USART_InitStructure);
	/* 
		USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
		注意: 不要在此处打开发送中断
		发送中断使能在SendUart()函数打开
	*/


	/* CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
		如下语句解决第1个字节无法正确发送出去的问题 */
	USART_ClearFlag(UART4, USART_FLAG_TC);     /* 清发送外城标志，Transmission Complete flag */
  
	#if 1
  /* NVIC configuration */
  /* Configure the Priority Group to 2 bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  #endif

  
  /* Enable USART */
	AtCmdFromGPS.counter = 0;
    AtCmdFromGPS.CurRecStatus = COMM_REC_WAITPREFIX;
    //USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);
	//Uart4Sem = OSSemCreate(1);
	USART_Cmd(uart, ENABLE);		/* 使能串口 */ 
}

/* this is for LINUX module */
void UART5_Config(void)
{
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_TypeDef *uart;
  
  /* 第1步：打开GPIO和USART部件的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
  
 /* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* 第3步：将USART Rx的GPIO配置为浮空输入模式
		由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
		但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	/*  第3步已经做了，因此这步可以不做
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/

  /* Enable the USART OverSampling by 8 */
  USART_OverSampling8Cmd(UART5, ENABLE);  

  /* USARTx configuration ----------------------------------------------------*/
  /* USARTx configured as follow:
        - BaudRate = 5250000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 8
		     is: (USART APB Clock / 8) 
			 Example: 
			    - (USART3 APB1 Clock / 8) = (42 MHz / 8) = 5250000 baud
			    - (USART1 APB2 Clock / 8) = (84 MHz / 8) = 10500000 baud
		   - Maximum BaudRate that can be achieved when using the Oversampling by 16
		     is: (USART APB Clock / 16) 
			 Example: (USART3 APB1 Clock / 16) = (42 MHz / 16) = 2625000 baud
			 Example: (USART1 APB2 Clock / 16) = (84 MHz / 16) = 5250000 baud
        - Word Length = 8 Bits
        - one Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */ 
  /* 第4步：设置串口硬件参数 */
	uart = UART5;	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(uart, &USART_InitStructure);
	/* 
		USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
		注意: 不要在此处打开发送中断
		发送中断使能在SendUart()函数打开
	*/


	/* CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
		如下语句解决第1个字节无法正确发送出去的问题 */
	USART_ClearFlag(UART5, USART_FLAG_TC);     /* 清发送外城标志，Transmission Complete flag */
  
	#if 1
  /* NVIC configuration */
  /* Configure the Priority Group to 2 bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  #endif

  
  /* Enable USART */
	AtCmdFromPTZ.counter = 0;
	USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	USART_ITConfig(UART5, USART_IT_IDLE, ENABLE);
    Uart5Sem = OSSemCreate(1);
   	USART_Cmd(uart, ENABLE);		/* 使能串口 */ 

}

/****************************************************************************
* 函数名: UartSend
* 功  能: 填写数据到UART发送缓冲区,并启动发送中断。中断处理函数发送完毕后，自动
*			关闭发送中断。
* 输  入: _pUart: 串口结构体指针
* 输  出: 无
* 返  回: 无
*/
static void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen)
{
	uint16_t i;
	uint8_t ucNew = 0;
    
	CPU_IntDis();
	if (_pUart->usTxWrite == _pUart->usTxRead)
	{
		ucNew = 1;
	}
	CPU_IntEn();
	
	for (i = 0; i < _usLen; i++)
	{	
		_pUart->pTxBuf[_pUart->usTxWrite] = _ucaBuf[i];
		
		CPU_IntDis();	
		if (++_pUart->usTxWrite >= _pUart->usTxBufSize)
		{
			_pUart->usTxWrite = 0;
		}
		CPU_IntEn();
	}

	if (ucNew == 1)
	{
		USART_ITConfig(_pUart->uart, USART_IT_TXE, ENABLE);
	}
}

/****************************************************************************
* 函数名: UartIRQ
* 功  能: 供中断服务程序调用，通用串口中断处理函数
* 输  入: _pUart: 串口结构体指针
* 输  出: 无
* 返  回: 无
*/
void UartIRQ(UART_T *_pUart)
{
	if (USART_GetITStatus(_pUart->uart, USART_IT_RXNE) != RESET)
	{ 
		/* 回调函数,通知应用程序发送完毕,一般是发送1个消息或者设置一个标记 */
		if (_pUart->usRxWrite == _pUart->usRxRead)
		{
			if (_pUart->ReciveNew)
			{
				_pUart->ReciveNew();
			}
		}
			
		/* Read one byte from the receive data register */
		_pUart->pRxBuf[_pUart->usRxWrite] = USART_ReceiveData(_pUart->uart);		
		if (++_pUart->usRxWrite >= _pUart->usRxBufSize)
		{
			_pUart->usRxWrite = 0;
		}		
	}

	/* 发送缓冲区空中断 */
	if (USART_GetITStatus(_pUart->uart, USART_IT_TXE) != RESET)
	{   
		if (_pUart->usTxRead == _pUart->usTxWrite)
		{
			/* 禁止发送缓冲区空中断，使能发送完毕中断 */
			USART_ITConfig(_pUart->uart, USART_IT_TXE, DISABLE);
			USART_ITConfig(_pUart->uart, USART_IT_TC, ENABLE);
		} 
		else
		{		
			/* Write one byte to the transmit data register */
			USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);                    
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
		}
   
	}
	/* 1个字节发送完毕的中断 */
	else if (USART_GetITStatus(_pUart->uart, USART_IT_TC) != RESET)
	{
		if (_pUart->usTxRead == _pUart->usTxWrite)
		{
			/* 禁止发送缓冲区空中断，使能发送完毕中断 */
			USART_ITConfig(_pUart->uart, USART_IT_TC, DISABLE);
			
			/* 回调函数,通知应用程序发送完毕,一般是发送1个消息或者设置一个标记 */
			if (_pUart->SendOver)
			{
				_pUart->SendOver();
			}
		} 
		else
		{		
			/* Write one byte to the transmit data register */
			USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);                    
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
		}
	}
}

/****************************************************************************
* 函数名: USART1_ISR   USART1 = DEBUG_PORT
* 功  能: USART1中断服务程序.
* 输  入: 无
* 输  出: 无
* 返  回: 无
*/
void USART1_ISR(void)
{
	uint8 err,err1;
	DATA_CONTROL_BLOCK  *dp;
	uint8 *dp1;
	//char* str;
	
	/* USART in mode Tramitter -------------------------------------------------*/
  if (USART_GetITStatus(USART1, USART_IT_TXE) == SET)
  { /* When Joystick Pressed send the command then send the data */
  		
		g_zt_msg1.counter = g_zt_msg1.counter+1;
	  
      if (g_zt_msg1.counter >= g_zt_msg1.icmd_len)
      {				
          /* Disable the USARTx transmit data register empty interrupt */
          USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
          g_zt_msg1.counter = 0;
		  OSSemPost(Usart1Sem);
      }	
	  else
	  {
	  	USART_SendData(USART1, g_zt_msg1.sendbuf[g_zt_msg1.counter]);
		
	  }
  }

  if (USART_GetITStatus(USART1, USART_IT_IDLE) == SET)
	{	
		err = USART_ReceiveData(USART1);//空读清中断
		
		dp = OSMemGet(p_msgBuffer,&err);
		dp1 = OSMemGet(mem160ptr,&err1);
		if((err == OS_ERR_NONE)&&(err1 == OS_ERR_NONE))
		{
		   dp->type = LOCAL_DEBUG_IN_MSG_DEBUG;
		   dp->count = AtCmdFromDebug.counter;
		   CopyBuffer(&AtCmdFromDebug.recv_buf[0],dp1,AtCmdFromDebug.counter);
		   dp->point = (uint8 *)(dp1); 
		   //str = (char*)(dp->point+3);
		   //cmd_para_get_all_items( (char*)(str) , &gCmdParaInfos);
		  
		   if(OS_ERR_NONE != OSQPost(RemoteRecQueue,(void*)dp))
		   {
			   OSMemPut(mem160ptr,dp1);
			   OSMemPut(p_msgBuffer,dp);
		   }
		}
		else
		{
			if(err== OS_ERR_NONE)
			{
				OSMemPut(p_msgBuffer,dp);
			}	
			if(err1== OS_ERR_NONE)
			{
				OSMemPut(mem160ptr,dp1);
			}
		}
		AtCmdFromDebug.counter = 0;
	}
	
  /* USART in mode Receiver --------------------------------------------------*/
  if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
  {
	  AtCmdFromDebug.recv_buf[AtCmdFromDebug.counter++] = USART_ReceiveData(USART1);
	  AtCmdFromDebug.counter %= RX_LENGTH; 	
  }   
}

/****************************************************************************
* 函数名: USART2_ISR   USART2 = LINUX_PORT
* 功  能: USART2中断服务程序.
* 输  入: 无
* 输  出: 无
* 返  回: 无
*/
void USART2_ISR(void)
{
	uint8 err,err1;
	DATA_CONTROL_BLOCK  *dp;
	uint8 *dp1;
    //char* str;
	
	/* USART in mode Tramitter -------------------------------------------------*/
  if (USART_GetITStatus(USART2, USART_IT_TXE) == SET)
  { /* When Joystick Pressed send the command then send the data */
  		
	  g_zt_msg2.counter = g_zt_msg2.counter+1;
	  
      if (g_zt_msg2.counter >= g_zt_msg2.icmd_len)
      {			
          //GpioSetL(GPIO_LED_SHOW1);
		  /* Disable the USARTx transmit data register empty interrupt */
          USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
          g_zt_msg2.counter = 0;
		  OSSemPost(Usart2Sem);
      }	
	  else
	  {
	  	USART_SendData(USART2, g_zt_msg2.sendbuf[g_zt_msg2.counter]);
	  }
  }

  if (USART_GetITStatus(USART2, USART_IT_IDLE) == SET)
	{	
		err = USART_ReceiveData(USART2);//空读清中断

		dp = OSMemGet(p_msgBuffer,&err);
		dp1 = OSMemGet(mem160ptr,&err1);
		if((err == OS_ERR_NONE)&&(err1 == OS_ERR_NONE))
		{
		   dp->type = LOCAL_DEBUG_IN_MSG_LINUX;
		   dp->count = AtCmdFromLinux.counter;
		   CopyBuffer(&AtCmdFromLinux.recv_buf[0],dp1,AtCmdFromLinux.counter);
		   dp->point = (uint8 *)dp1; 
		   //str = (char*)(dp->point+3);
		   //cmd_para_get_all_items( (char*)(str) , &gCmdParaInfos); 
		   
		   if(OS_ERR_NONE != OSQPost(RemoteRecQueue,(void*)dp))
		   {
			   OSMemPut(mem160ptr,dp1);
			   OSMemPut(p_msgBuffer,dp);
		   }	
		}
		else
		{
			if(err== OS_ERR_NONE)
			{
				OSMemPut(p_msgBuffer,dp);
			}	
			if(err1== OS_ERR_NONE)
			{
				OSMemPut(mem160ptr,dp1);
			}
		}
		AtCmdFromLinux.counter = 0;
	}
	
  /* USART in mode Receiver --------------------------------------------------*/
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
	    AtCmdFromLinux.recv_buf[AtCmdFromLinux.counter++] = USART_ReceiveData(USART2);
	    AtCmdFromLinux.counter %= RX_LENGTH; 			
    }   
}


/****************************************************************************
* 函数名: USART3_ISR   USART3 = RF433
* 功  能: USART3中断服务程序
* 输  入: 无
* 输  出: 无
* 返  回: 无
*/
void USART3_ISR(void)
{
	uint8 err,err1;
	DATA_CONTROL_BLOCK  *dp;
	uint8 *dp1;
   // char* str;
	
	/* USART in mode Tramitter -------------------------------------------------*/
  if (USART_GetITStatus(USART3, USART_IT_TXE) == SET)
  { /* When Joystick Pressed send the command then send the data */
  		
		g_zt_msg3.counter = g_zt_msg3.counter+1;
	  
      if (g_zt_msg3.counter >= g_zt_msg3.icmd_len)
      {				
          /* Disable the USARTx transmit data register empty interrupt */
          USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
          g_zt_msg3.counter = 0;
		  OSSemPost(Usart3Sem);
      }	
	  else
	  {
	  	USART_SendData(USART3, g_zt_msg3.sendbuf[g_zt_msg3.counter]);
	  }
  }

  if (USART_GetITStatus(USART3, USART_IT_IDLE) == SET)
	{	
		err = USART_ReceiveData(USART3);//空读清中断

		dp = OSMemGet(p_msgBuffer,&err);
		dp1 = OSMemGet(mem160ptr,&err1);
		if((err == OS_ERR_NONE)&&(err1 == OS_ERR_NONE))
		{  
           //GpioSetL(GPIO_LED_SHOW1);
		   dp->type = LOCAL_DEBUG_IN_MSG_RF433;
		   dp->count = AtCmdFromRF433.counter;
		   CopyBuffer(&AtCmdFromRF433.recv_buf[0],dp1,AtCmdFromRF433.counter);
		   dp->point = (uint8 *)(dp1); 
		   //str = (char*)(dp->point+3);
		   //cmd_para_get_all_items( (char*)(str) , &gCmdParaInfos);
		   /*if( (gCmdParaInfos.item_cnt>1) && (box_str_cmp("name",gCmdParaInfos.items[gCmdParaInfos.item_cnt-1].name)==0) )
		   {
				 //字符串的结束位置进行更改
		       str[gCmdParaInfos.items[gCmdParaInfos.item_cnt-1].item_begin_pos] = 0;
		       gCmdParaInfos.item_cnt--; //参数数量进行修改
			   if( box_str_cmp(gRbtState.RobotName,gCmdParaInfos.items[gCmdParaInfos.item_cnt-1].value)==0) 
			   {
					if(OS_ERR_NONE != OSQPost(RemoteRecQueue,(void*)dp))
				   {
					   OSMemPut(mem160ptr,dp1);
					   OSMemPut(p_msgBuffer,dp);
				   }
			   }
		   	}*/
		   	if(OS_ERR_NONE != OSQPost(RemoteRecQueue,(void*)dp))
		    {
			    OSMemPut(mem160ptr,dp1);
			    OSMemPut(p_msgBuffer,dp);
		    }	
		   	
		}
		else
		{
			if(err== OS_ERR_NONE)
			{
				OSMemPut(p_msgBuffer,dp);
			}	
			if(err1== OS_ERR_NONE)
			{
				OSMemPut(mem160ptr,dp1);
			}
		}
		AtCmdFromRF433.counter = 0;
	}
	
  /* USART in mode Receiver --------------------------------------------------*/
  if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
  {	  
	  AtCmdFromRF433.recv_buf[AtCmdFromRF433.counter++] = USART_ReceiveData(USART3);
	  AtCmdFromRF433.counter %= RX_LENGTH; 			
  }   
}

/****************************************************************************
* 函数名: UART4_ISR   UART4 = GPS
* 功  能: UART4中断服务程序.
* 输  入: 无
* 输  出: 无
* 返  回: 无
*/
void UART4_ISR(void)
{
	uint8 err,err1;
	DATA_CONTROL_BLOCK  *dp;
	uint8 *dp1;
   // char* str;
	//GpioSetL(GPIO_LED_SHOW1);
	/* USART in mode Tramitter -------------------------------------------------*/
  if (USART_GetITStatus(UART4, USART_IT_TXE) == SET)
  { 
	  g_zt_msg4.counter = g_zt_msg4.counter+1;
	  
      if (g_zt_msg4.counter >= g_zt_msg4.icmd_len)
      {				
          /* Disable the USARTx transmit data register empty interrupt */
          USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
          g_zt_msg4.counter = 0;
		  OSSemPost(Uart4Sem);
      }	
	  else
	  {
	  	USART_SendData(UART4, g_zt_msg4.sendbuf[g_zt_msg4.counter]);
	  }
  }
  /*
  if (USART_GetITStatus(UART4, USART_IT_IDLE) == SET)
	{	
		err = USART_ReceiveData(UART4);//空读清中断

		dp = OSMemGet(p_msgBuffer,&err);
		dp1 = OSMemGet(mem160ptr,&err1);
		if((err == OS_ERR_NONE)&&(err1 == OS_ERR_NONE))
		{
		   dp->type = LOCAL_DEBUG_IN_MSG_GPS;/////////////////////////////////////////////
		   dp->count = AtCmdFromGPS.counter;
		   CopyBuffer(&AtCmdFromGPS.recv_buf[0],dp1,AtCmdFromGPS.counter);
		   dp->point = (uint8 *)(dp1); 

		   	if(OS_ERR_NONE != OSQPost(RemoteRecQueue,(void*)dp))
		    {
			    OSMemPut(mem160ptr,dp1);
			    OSMemPut(p_msgBuffer,dp);
		    }	
		   	
		}
		else
		{
			if(err== OS_ERR_NONE)
			{
				OSMemPut(p_msgBuffer,dp);
			}	
			if(err1== OS_ERR_NONE)
			{
				OSMemPut(mem160ptr,dp1);
			}
		}
		AtCmdFromGPS.counter = 0;
	}
	*/
  /* USART in mode Receiver --------------------------------------------------*/
  if (USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
  {	
    AtCmdFromGPS.Data= USART_ReceiveData(UART4);
    
    switch (AtCmdFromGPS.CurRecStatus)
    {
        case COMM_REC_IDLE:
            break;
        case COMM_REC_WAITPREFIX:
            if (AtCmdFromGPS.Data == '$')
            {   
                AtCmdFromGPS.recv_buf[0] = '$';
				AtCmdFromGPS.CurRecStatus = COMM_REC_RECEIVING;
                AtCmdFromGPS.counter = 1;
            }
            break;
        case COMM_REC_RECEIVING:
			AtCmdFromGPS.recv_buf[AtCmdFromGPS.counter] = AtCmdFromGPS.Data;
            AtCmdFromGPS.counter++;
			if(AtCmdFromGPS.counter==6)
			{
			    if((AtCmdFromGPS.recv_buf[4]=='M')&&(AtCmdFromGPS.recv_buf[5]=='C'))
				{
					AtCmdFromGPS.CurRecStatus = COMM_REC_GNRMC;
				}
				else if((AtCmdFromGPS.recv_buf[4]=='G')&&(AtCmdFromGPS.recv_buf[5]=='A'))
				{
	      			AtCmdFromGPS.CurRecStatus = COMM_REC_GNGGA;
				}
				else
				{
					AtCmdFromGPS.CurRecStatus = COMM_REC_WAITPREFIX;
					AtCmdFromGPS.counter = 0;
				}
			}
			break;
		case COMM_REC_GNRMC:
		case COMM_REC_GNGGA:
            AtCmdFromGPS.recv_buf[AtCmdFromGPS.counter] = AtCmdFromGPS.Data;
            AtCmdFromGPS.counter++;
			if((AtCmdFromGPS.recv_buf[AtCmdFromGPS.counter-1]==0x0a)&&(AtCmdFromGPS.recv_buf[AtCmdFromGPS.counter-2]==0x0d))
			{
                AtCmdFromGPS.CurRecStatus = COMM_REC_OVER;
				dp = OSMemGet(p_msgBuffer,&err);
				dp1 = OSMemGet(mem160ptr,&err1);
				if((err == OS_ERR_NONE)&&(err1 == OS_ERR_NONE))
				{
				    if((AtCmdFromGPS.recv_buf[4]=='M')&&(AtCmdFromGPS.recv_buf[5]=='C'))
				    {
					 	dp->type = LOCAL_DEBUG_IN_MSG_GPS_GNRMC;
					}
					else if((AtCmdFromGPS.recv_buf[4]=='G')&&(AtCmdFromGPS.recv_buf[5]=='A'))
					{
						dp->type = LOCAL_DEBUG_IN_MSG_GPS_GNGGA;
					}
					else
					{

					}
				   dp->count = AtCmdFromGPS.counter;
				   CopyBuffer(&AtCmdFromGPS.recv_buf[0],dp1,AtCmdFromGPS.counter);
				   dp->point = (uint8 *)(dp1); 

				   	if(OS_ERR_NONE != OSQPost(RemoteRecQueue,(void*)dp))
				    {
					    OSMemPut(mem160ptr,dp1);
					    OSMemPut(p_msgBuffer,dp);
				    }	
				   	
				}
				else
				{
					if(err== OS_ERR_NONE)
					{
						OSMemPut(p_msgBuffer,dp);
					}	
					if(err1== OS_ERR_NONE)
					{
						OSMemPut(mem160ptr,dp1);
					}
				}
				//AtCmdFromGPS.counter = 0;
			}
			break;   
        case COMM_REC_OVER:
			err = USART_ReceiveData(UART4);//空读清中断
            break;
        default:
            break;
    }	  
  }   
}

/****************************************************************************
* 函数名: UART5_ISR   UART5 = PTZ
* 功  能: UART5中断服务程序
* 输  入: 无
* 输  出: 无
* 返  回: 无
*/
void UART5_ISR(void)
{
	uint8 err,err1;
	DATA_CONTROL_BLOCK  *dp;
	uint8 *dp1;
   // char* str;
	
	/* USART in mode Tramitter -------------------------------------------------*/
  if (USART_GetITStatus(UART5, USART_IT_TXE) == SET)
  { /* When Joystick Pressed send the command then send the data */
  		//GpioSetL(GPIO_LED_SHOW1);
		g_zt_msg5.counter = g_zt_msg5.counter+1;
	  
      if (g_zt_msg5.counter >= g_zt_msg5.icmd_len)
      {				
          /* Disable the USARTx transmit data register empty interrupt */
          USART_ITConfig(UART5, USART_IT_TXE, DISABLE);
          g_zt_msg5.counter = 0;
		  OSSemPost(Uart5Sem);
      }	
	  else
	  {
	  	USART_SendData(UART5, g_zt_msg5.sendbuf[g_zt_msg5.counter]);
	  }
  }

  if (USART_GetITStatus(UART5, USART_IT_IDLE) == SET)
	{	//GpioSetL(GPIO_LED_SHOW1);
		err = USART_ReceiveData(UART5);//空读清中断

		dp = OSMemGet(p_msgBuffer,&err);
		dp1 = OSMemGet(mem160ptr,&err1);
		if((err == OS_ERR_NONE)&&(err1 == OS_ERR_NONE))
		{
		   dp->type = LOCAL_DEBUG_IN_MSG_PTZ;
		   dp->count = AtCmdFromPTZ.counter;
		   CopyBuffer(&AtCmdFromPTZ.recv_buf[0],dp1,AtCmdFromPTZ.counter);
		   dp->point = (uint8 *)(dp1); 
		   //str = (char*)(dp->point+3);
		   //cmd_para_get_all_items( (char*)(str) , &gCmdParaInfos);
		   /*if( (gCmdParaInfos.item_cnt>1) && (box_str_cmp("name",gCmdParaInfos.items[gCmdParaInfos.item_cnt-1].name)==0) )
		   {
				 //字符串的结束位置进行更改
		       str[gCmdParaInfos.items[gCmdParaInfos.item_cnt-1].item_begin_pos] = 0;
		       gCmdParaInfos.item_cnt--; //参数数量进行修改
			   if( box_str_cmp(gRbtState.RobotName,gCmdParaInfos.items[gCmdParaInfos.item_cnt-1].value)==0) 
			   {
					if(OS_ERR_NONE != OSQPost(RemoteRecQueue,(void*)dp))
				   {
					   OSMemPut(mem160ptr,dp1);
					   OSMemPut(p_msgBuffer,dp);
				   }
			   }
		   	}*/
		   	if(OS_ERR_NONE != OSQPost(RemoteRecQueue,(void*)dp))
		    {
			    OSMemPut(mem160ptr,dp1);
			    OSMemPut(p_msgBuffer,dp);
		    }	
		   	
		}
		else
		{
			if(err== OS_ERR_NONE)
			{
				OSMemPut(p_msgBuffer,dp);
			}	
			if(err1== OS_ERR_NONE)
			{
				OSMemPut(mem160ptr,dp1);
			}
		}
		AtCmdFromPTZ.counter = 0;
	}
	
  /* USART in mode Receiver --------------------------------------------------*/
  if (USART_GetITStatus(UART5, USART_IT_RXNE) == SET)
  {	
	  AtCmdFromPTZ.recv_buf[AtCmdFromPTZ.counter++] = USART_ReceiveData(UART5);
	  AtCmdFromPTZ.counter %= RX_LENGTH; 			
  }   
}

void uart1_send(uint8 *sp, uint16 len)
{
    uint8 err; 
 
    OSSemPend(Usart1Sem,0,&err);
    
	if(OS_ERR_NONE != err)
	{
		return;
	}
	CopyBuffer(sp,&g_zt_msg1.sendbuf[0],len);

	g_zt_msg1.icmd_len = len;
	g_zt_msg1.counter = 0;
	USART_SendData(USART1, g_zt_msg1.sendbuf[0]);
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

void uart2_send(uint8 *sp, uint16 len)
{
    uint8 err; 

    OSSemPend(Usart2Sem,0,&err);
	
	if(OS_ERR_NONE != err)
	{
		return;
	}
    //OSTimeDly(OS_TICKS_PER_SEC/5);
	//OSTimeDly(OS_TICKS_PER_SEC/10);
	
	CopyBuffer(sp,&g_zt_msg2.sendbuf[0],len);

	g_zt_msg2.icmd_len = len;
	g_zt_msg2.counter = 0;
	USART_SendData(USART2, g_zt_msg2.sendbuf[0]);
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

void uart3_send(uint8 *sp, uint16 len)
{
    uint8 err; 

    OSSemPend(Usart3Sem,0,&err);
	if(OS_ERR_NONE != err)
	{
		return;
	}

	CopyBuffer(sp,&g_zt_msg3.sendbuf[0],len);

	g_zt_msg3.icmd_len = len;
	g_zt_msg3.counter = 0;
	USART_SendData(USART3, g_zt_msg3.sendbuf[0]);
	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
}
/*
void uart5_send(uint8 *sp, uint8 len)
{
    uint8 err; 

    OSSemPend(Uart5Sem,0,&err);
	if(OS_ERR_NONE != err)
	{
		return;
	}

	CopyBuffer(sp,&g_zt_msg5.sendbuf[0],len);

	g_zt_msg5.icmd_len = len;
	g_zt_msg5.counter = 0;
	USART_SendData(UART5, g_zt_msg5.sendbuf[0]);
	USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
}*/

uint8 GetGTZMHDCmdXOR(void)
{
    uint8 ret,i;
	
	ret=g_zt_msg5.sendbuf[2];
	for(i=3;i<7;i++)
	{
		ret ^= g_zt_msg5.sendbuf[i];
	}
	return ret;
}


uint8 CheckGTZMHDRecXOR(void)
{
    /*uint8 ret,i;
	
	ret=g_zt_msg5.sendbuf[3];
	for(i=4;i<7;i++)
	{
		ret ^= g_zt_msg5.sendbuf[i];
	}*/
	return 0;
}

void SetState1GTZMHD(void)
{
    uint8 err;

    //USART_ITConfig(UART5, USART_IT_RXNE, DISABLE);
	//USART_ITConfig(UART5, USART_IT_IDLE, DISABLE);
	
	OSSemPend(Uart5Sem,0,&err);
	if(OS_ERR_NONE != err)
	{
		return;
	}
    
    g_zt_msg5.sendbuf[0] = 0xaa;
	g_zt_msg5.sendbuf[1] = 0x05;
	g_zt_msg5.sendbuf[2] = 0x0a;
	g_zt_msg5.sendbuf[3] = (uint8)(GTZMHD_Set.directAngle>>8);
	g_zt_msg5.sendbuf[4] = (uint8)(GTZMHD_Set.directAngle);
	g_zt_msg5.sendbuf[5] = (uint8)(GTZMHD_Set.pitchAngle>>8);
	g_zt_msg5.sendbuf[6] = (uint8)(GTZMHD_Set.pitchAngle);
	g_zt_msg5.sendbuf[7] = GetGTZMHDCmdXOR();
	//nprintf(1, g_zt_msg5.sendbuf[7], 0, DEC);
	g_zt_msg5.sendbuf[8] = 0x55;
	
    g_zt_msg5.icmd_len = 9;
	g_zt_msg5.counter = 0;
	USART_SendData(UART5, g_zt_msg5.sendbuf[0]);
	USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
	
}

void SetState2GTZMHD(void)
{
    uint8 err;

	//USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(UART5, USART_IT_IDLE, ENABLE);

	OSSemPend(Uart5Sem,0,&err);
	if(OS_ERR_NONE != err)
	{
		return;
	}

    g_zt_msg5.sendbuf[0] = 0xaa;
	g_zt_msg5.sendbuf[1] = 0x05;
	g_zt_msg5.sendbuf[2] = 0x0c;
	g_zt_msg5.sendbuf[3] = (uint8)(GTZMHD_Set.nVisibleFocalDis>>8);
	g_zt_msg5.sendbuf[4] = (uint8)(GTZMHD_Set.nVisibleFocalDis);
	g_zt_msg5.sendbuf[5] = 0x00;
	g_zt_msg5.sendbuf[6] = 0x00;
	g_zt_msg5.sendbuf[7] = GetGTZMHDCmdXOR();
	g_zt_msg5.sendbuf[8] = 0x55;
	
    g_zt_msg5.icmd_len = 9;
	g_zt_msg5.counter = 0;
	USART_SendData(UART5, g_zt_msg5.sendbuf[0]);
	USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
}

void SendGTZMHDCmdSetRet(u8 ret)
{
    Ptz_SetRst_string(CurrentPtzSetCom,ret);
    /*
	u8 buf[11];

	buf[0]='A';
	buf[1]='T';
	buf[2]='+';
    buf[3]='P';
	buf[4]='t';
	buf[5]='z';
    buf[6]='S';
	buf[7]='e';
	buf[8]='t';
	buf[9]='=';
	
	if(ret==1)//设置云台姿态和焦距成功
	{
		buf[10]='0';
	}
	else//设置云台姿态和焦距失败
	{
	    buf[10]='1';
	}
	
	if( CurrentPtzSetCom == ID_LINUX)
	{
        uart2_send(buf,11);
	}
	else if( CurrentPtzSetCom== ID_RF433)
	{
        uart3_send(buf,11);
	}
	else if( CurrentPtzSetCom== ID_DEBUG)
	{
        uart1_send(buf,11);
	}*/
}

void GetState1GTZMHD(void)
{
    uint8 err;

	//USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(UART5, USART_IT_IDLE, ENABLE);

	OSSemPend(Uart5Sem,0,&err);
	if(OS_ERR_NONE != err)
	{
		return;
	}

    g_zt_msg5.sendbuf[0] = 0xaa;
	g_zt_msg5.sendbuf[1] = 0x05;
	g_zt_msg5.sendbuf[2] = 0x0b;
	g_zt_msg5.sendbuf[3] = 0x00;
	g_zt_msg5.sendbuf[4] = 0x00;
	g_zt_msg5.sendbuf[5] = 0x00;
	g_zt_msg5.sendbuf[6] = 0x00;
	g_zt_msg5.sendbuf[7] = 0x0b;
	g_zt_msg5.sendbuf[8] = 0x55;
	
    g_zt_msg5.icmd_len = 9;
	g_zt_msg5.counter = 0;
	USART_SendData(UART5, g_zt_msg5.sendbuf[0]);
	USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
}

void GetState2GTZMHD(void)
{
    uint8 err;

	//USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(UART5, USART_IT_IDLE, ENABLE);

	OSSemPend(Uart5Sem,0,&err);
	if(OS_ERR_NONE != err)
	{
		return;
	}

    g_zt_msg5.sendbuf[0] = 0xaa;
	g_zt_msg5.sendbuf[1] = 0x05;
	g_zt_msg5.sendbuf[2] = 0x0d;
	g_zt_msg5.sendbuf[3] = 0x00;
	g_zt_msg5.sendbuf[4] = 0x00;
	g_zt_msg5.sendbuf[5] = 0x00;
	g_zt_msg5.sendbuf[6] = 0x00;
	g_zt_msg5.sendbuf[7] = 0x0d;
	g_zt_msg5.sendbuf[8] = 0x55;
	
    g_zt_msg5.icmd_len = 9;
	g_zt_msg5.counter = 0;
	USART_SendData(UART5, g_zt_msg5.sendbuf[0]);
	USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
}

void SendGTZMHDCmdGetRet(u8 ret)
{
    //u8 buf[30],len;
	//s16 temp;

    //Ptz_GetRst_string(CurrentPtzSetCom,ret);
    Ptz_GetRst_string(ret);
	
	/*
    len=0;
	buf[len++]='A';
	buf[len++]='T';
	buf[len++]='+';
    buf[len++]='P';
	buf[len++]='t';
	buf[len++]='z';
    buf[len++]='G';
	buf[len++]='e';
	buf[len++]='t';
	buf[len++]='=';

	if(ret==1)//设置云台姿态和焦距成功
	{
        

		temp=GTZMHD_Get.directAngle;
		if(temp<0)
    	{
			temp=-temp;
			buf[len++]='-'; 
    	}
		temp=temp/100;//武汉高德的吊仓协议
		if(temp>=100)
		{
			buf[len++]=temp/100+0x30;
            temp%=100;
			buf[len++]=temp/10+0x30;
            temp%=10;
			buf[len++]=temp+0x30;
		}
        else if(temp>=10)
    	{
			buf[len++]=temp/10+0x30;
            temp%=10;
			buf[len++]=temp+0x30;
    	}
		else
		{
			buf[len++]=temp+0x30;
		}
		buf[len++]=',';

        temp=GTZMHD_Get.pitchAngle;
		if(temp<0)
    	{
			temp=-temp;
			buf[len++]='-'; 
    	}
        temp=temp/100;//武汉高德的吊仓协议
		if(temp>=100)
		{
			buf[len++]=temp/100+0x30;
            temp%=100;
			buf[len++]=temp/10+0x30;
            temp%=10;
			buf[len++]=temp+0x30;
		}
        else if(temp>=10)
    	{
			buf[len++]=temp/10+0x30;
            temp%=10;
			buf[len++]=temp+0x30;
    	}
		else
		{
			buf[len++]=temp+0x30;
		}
		buf[len++]=',';

		buf[len++]='0';
		buf[len++]=',';

        temp=GTZMHD_Get.nVisibleFocalDis;
        temp=temp/5;//武汉高德的吊仓协议
        if(temp>=100)
		{
			buf[len++]=temp/100+0x30;
            temp%=100;
			buf[len++]=temp/10+0x30;
            temp%=10;
			buf[len++]=temp+0x30;
		}
        else if(temp>=10)
    	{
			buf[len++]=temp/10+0x30;
            temp%=10;
			buf[len++]=temp+0x30;
    	}
		else
		{
			buf[len++]=temp+0x30;
		}
	}
	else//设置云台姿态和焦距失败
	{
	    buf[len++]='1';
	}
	
	if( CurrentPtzSetCom == ID_LINUX)
	{
        uart2_send(buf,len);
	}
	else if( CurrentPtzSetCom== ID_RF433)
	{
        uart3_send(buf,len);
	}
	else if( CurrentPtzSetCom== ID_DEBUG)
	{
        uart1_send(buf,len);
	}*/
}

/****************************************************************************
* 函数名: UartGetChar
* 功  能: 从串口接收缓冲区读取n字节数据
* 输  入: _pUart: 串口结构体指针
* 输  出: 无
* 返  回: 1表示正确，0表示无数据
*/
static uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte)
{
	uint16_t usRxWrite;
	
	CPU_IntDis();	
	usRxWrite = _pUart->usRxWrite;
	CPU_IntEn();

	if (_pUart->usRxRead == usRxWrite)
	{
		return 0;
	}
	else
	{
		*_pByte = _pUart->pRxBuf[_pUart->usRxRead];
		
		CPU_IntDis();
		if (++_pUart->usRxRead >= _pUart->usRxBufSize)
		{
			_pUart->usRxRead = 0;
		}		
		CPU_IntEn();
		return 1;
	}
}


