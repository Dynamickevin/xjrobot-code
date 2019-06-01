/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               Mem.c
** Descriptions:            内存的管理函数
**
**--------------------------------------------------------------------------------------------------------
** Created by:              Ya Dan
** Created date:            2011-1-18
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Version:                 
** Descriptions:            
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "Mem.h"	


/* Private variables ---------------------------------------------------------*/			
MEMTcb MemTcb[MEM_MAX];						/* 内存块管理区定义，最多允许MEM_MAX个内存区 */
uint8_t USARTMemQ[MEM_USART_MAX];  		    /* 空白内存块,MEM_USART_MAX表示内存块区中的最大数量 */


/*******************************************************************************
* Function Name  : MemInit
* Description    : 空白内存块的初始化函数
* Input          : - ptr: 空白连续内存区的指针
*				   - count: 空白连续内存区内的数量
* Output         : None
* Return         : 内存块的首地址
* Attention		 : None
*******************************************************************************/
void MemInit(uint8_t *ptr,uint16_t count)
{
	uint16_t i;
	for( i=0; i<count; i++ )
	{
		ptr[i] = 0;
	}	
}

/*******************************************************************************
* Function Name  : MemCreate
* Description    : 建立一个内存管理区（具体请参考UCOS的内存管理结构）
* Input          : - ptr: 空白连续内存区的指针
*				   - blksize: 每个内存块有多少个字节
*                  - nblks: 多少个内存块
* Output         : None
* Return         : 内存块的首地址
* Attention		 : None
*******************************************************************************/	
MEMTcb *MemCreate(uint8_t *ptr,uint8_t blksize,uint8_t nblks)
{
	uint8_t *link = (ptr+blksize);
	void **plink = (void **)ptr;
	uint16_t i = 0;
	uint16_t j = 0;
	/* 首先查找空白的内存管理区 */
	while( (MemTcb[i].MemFreeList != (uint8_t*)0) && (i <= MEM_MAX) )		
	{														
		i++;
	}
	if(i >= MEM_MAX)								    /* 内存块管理区分配不到，返回空指针 */
	{
	  return (MEMTcb *)0;
	}								
	MemTcb[i].MemFreeList = ptr;					    /* 内存块管理区的内存的指针 */
	MemTcb[i].MemBlkSize = blksize;						/* 内存块管理区的每个内存块的字节 */
	MemTcb[i].MemNBlks = nblks;							/* 内存块管理区的内存块的总数量	*/
	MemTcb[i].MemFreeNBlks = nblks;						/* 内存块管理区的空白内存块的数量 */
	j = i;
	MemInit(ptr,blksize*nblks);							/* 将内存块内的数据初始化 */
	for( i=0; i<nblks-1; i++ )								
	{
		*plink = (void *)(link);						/* 该内存块的地址存放的是第二片内存区的首地址 */
		plink = (void **)(link);						/* 将二维指针定位到框的首位 */		
		link += blksize;								/* 一维指针重新定位 */
	}
//	*plink = (void *)0;									/* 最后一块内存区的下一个地址块的数据为0 */
	return (MEMTcb *)(MemTcb+j); 						/* 返回内存块管理区的首地址 */
}	

/*******************************************************************************
* Function Name  : MemGet
* Description    : 从一个内存管理区获取一个内存块
* Input          : - ptr: 内存管理区的指针
*                  - err: 返回错误信息
* Output         : None
* Return         : 获取的空白内存块的首地址
* Attention		 : None
*******************************************************************************/	
uint8_t *MemGet(MEMTcb *ptr,uint8_t *err)
{
	void *tcb;
	uint8_t *index;
	tcb = (*ptr).MemFreeList;
	if( (*ptr).MemFreeNBlks == 0 )          /* 如果空白内存块的数量为返回,若正确返回，收到的数据应该是0 */
	{
	  *err = 0xff;
	  return (void *)0;
	}       
	(*ptr).MemFreeNBlks--;				    /* 空白内存块块数量减一 */
	if( (*ptr).MemFreeNBlks != 0 )
	{
	  (*ptr).MemFreeList = *(void **)tcb;	/* 强制类型转换 */
	}
	index = (u8 *)tcb;
	index += 4;
	*err = 0;
	return index;	 						/* 返回获取的空白内存块的首地址 */
}

/*******************************************************************************
* Function Name  : MemDelete
* Description    : 从一个内存管理区删除一个内存块
* Input          : - ptr: 内存管理区的指针
* Output         : None
* Return         : 如果要删除的内存块是一个空指针，则返回0xff，若能够删除，返回0
* Attention		 : None
*******************************************************************************/	
uint8_t MemDelete(MEMTcb *ptr,uint8_t *index)
{
	void *tcb;
	if( index == (void*)0 )  return 0xff;				/* 如果要删除的内存块是一个空指针，则返回0xff */
	index -= 4;
	tcb = (void *)index;	
	*(void **)tcb = (*ptr).MemFreeList;				    /* 将OSMemFreeList重新指向这个已经变成空白了的指针 */
	(*ptr).MemFreeList = tcb;								/* 将这个空白的指针的下一个指针指向一个原先的OSMemFreeList */												//
	(*ptr).MemFreeNBlks++;								/* 空白内存块数量加1 */
	return 0;	 		
}


/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
