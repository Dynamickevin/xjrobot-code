/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    10/15/2010
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include <includes.h>


#include "platform_config.h"

#if ( OS_VIEW_MODULE == DEF_ENABLED )
extern	void OSView_RxTxISRHandler(void);
#endif


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}



/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None




  
  */
void SysTick_Handler(void)
{
    static uint16 counter;
	INT8U  err;
	DATA_CONTROL_BLOCK  *dp0,*dp1,*dp2,*dp8;

	CPU_SR         cpu_sr;
	OS_ENTER_CRITICAL();                         /* Tell uC/OS-II that we are starting an ISR          */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    OSTimeTick();                                /* Call uC/OS-II's OSTimeTick()                       */
	
	counter++;

	/*if(counter==127)//254ms
	{
		dp8= OSMemGet(p_msgBuffer,&err);
		if(OS_NO_ERR == err)
		{
			((DATA_CONTROL_BLOCK *)dp8)->type = WHL_MSG; 
			if(OS_NO_ERR != OSQPost(timerQueue,(void*)dp8))      
			{
				OSMemPut(p_msgBuffer,dp8);
			}
		}	
	}*/

	if ( (counter&0x000F)==0 )//32ms
	{
		dp8= OSMemGet(p_msgBuffer,&err);
		if(OS_NO_ERR == err)
		{
			((DATA_CONTROL_BLOCK *)dp8)->type = WHL_MSG; 
			if(OS_NO_ERR != OSQPost(timerQueue,(void*)dp8))      
			{
				OSMemPut(p_msgBuffer,dp8);
			}
		}	
	}
	
	if(counter>=128)//256ms
	{
		counter = 0;

        dp0 = OSMemGet(p_msgBuffer,&err);
		if(OS_NO_ERR == err)
		{
			((DATA_CONTROL_BLOCK *)dp0)->type = BATTERY_MSG; 
			if(OS_NO_ERR != OSQPost(timerQueue,(void*)dp0))      
			{
				OSMemPut(p_msgBuffer,dp0);
			}
		}	
	}
	
	if ( (counter&0x01)==0 )//4ms
	//if ( (counter&0x0000)==0x0003 )//8ms
	{
		dp1 = OSMemGet(p_msgBuffer,&err);
		if(OS_NO_ERR == err)
		{
			((DATA_CONTROL_BLOCK *)dp1)->type = SENSOR_MSG; 
			if(OS_NO_ERR != OSQPost(GetWeatherQueue,(void*)dp1))     
			{
				OSMemPut(p_msgBuffer,dp1);
			}
		}
	}

	if ( (counter&0x07)==0 )//16ms
	//if ( (counter&0x000F)==0 )//32ms
	{
		dp2 = OSMemGet(p_msgBuffer,&err);
		if(OS_NO_ERR == err)
		{
			((DATA_CONTROL_BLOCK *)dp2)->type = MOTOR_MSG; 
			if(OS_NO_ERR != OSQPost(LocalQueue,(void*)dp2))     
			{
				OSMemPut(p_msgBuffer,dp2);
			}
		}
	}
	
    OSIntExit();                                 /* Tell uC/OS-II that we are leaving the ISR          */
}


/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
extern void USART1_ISR(void);
void USART1_IRQHandler(void)
{       
		CPU_SR         cpu_sr;

	OS_ENTER_CRITICAL();                         
       OSIntNesting++;
       OS_EXIT_CRITICAL();
	
	#if ( OS_VIEW_MODULE == DEF_ENABLED )
		OSView_RxTxISRHandler();
	#endif
	
	USART1_ISR();
	OSIntExit();
}

extern void USART2_ISR(void);
void USART2_IRQHandler(void)
{       
		CPU_SR         cpu_sr;

	OS_ENTER_CRITICAL();                         
       OSIntNesting++;
       OS_EXIT_CRITICAL();
	
	#if ( OS_VIEW_MODULE == DEF_ENABLED )
		OSView_RxTxISRHandler();
	#endif
	
	USART2_ISR();
	OSIntExit();
}

extern void USART3_ISR(void);
void USART3_IRQHandler(void)
{       
		CPU_SR         cpu_sr;

	OS_ENTER_CRITICAL();                         
       OSIntNesting++;
       OS_EXIT_CRITICAL();
	
	#if ( OS_VIEW_MODULE == DEF_ENABLED )
		OSView_RxTxISRHandler();
	#endif
	
	USART3_ISR();
	OSIntExit();
}

extern void UART4_ISR(void);
void UART4_IRQHandler(void)
{       
		CPU_SR         cpu_sr;

	OS_ENTER_CRITICAL();                         
       OSIntNesting++;
       OS_EXIT_CRITICAL();
	
	#if ( OS_VIEW_MODULE == DEF_ENABLED )
		OSView_RxTxISRHandler();
	#endif
	
	UART4_ISR();
	OSIntExit();
}

extern void UART5_ISR(void);
void UART5_IRQHandler(void)
{       
		CPU_SR         cpu_sr;

	OS_ENTER_CRITICAL();                         
       OSIntNesting++;
       OS_EXIT_CRITICAL();
	
	#if ( OS_VIEW_MODULE == DEF_ENABLED )
		OSView_RxTxISRHandler();
	#endif
	
	UART5_ISR();
	OSIntExit();
}

extern void EXTI15_10_IRQ_ISR(void);
void EXTI15_10_IRQHandler(void)
{
	CPU_SR         cpu_sr;
	
	OS_ENTER_CRITICAL();                         
	OSIntNesting++;
	OS_EXIT_CRITICAL();

    #if ( OS_VIEW_MODULE == DEF_ENABLED )
		OSView_RxTxISRHandler();
	#endif
	
	EXTI15_10_IRQ_ISR();
	OSIntExit();	
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/*******************************************************************************
* Function Name  : USB_HP_CAN1_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts requests
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*void USB_HP_CAN1_TX_IRQHandler(void)
{	CPU_SR         cpu_sr;

	       OS_ENTER_CRITICAL();                         
       OSIntNesting++;
       OS_EXIT_CRITICAL();
  CTR_HP();
  OSIntExit();
}*/

/*******************************************************************************
* Function Name  : USB_LP_CAN1_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*void USB_LP_CAN1_RX0_IRQHandler(void)
{	CPU_SR         cpu_sr;

       OS_ENTER_CRITICAL();                         
       OSIntNesting++;
       OS_EXIT_CRITICAL();
  USB_Istr();
  OSIntExit();
}*/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
