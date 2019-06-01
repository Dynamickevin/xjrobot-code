/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               Mem.h
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
#ifndef __MEM_H
#define __MEM_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Private define ------------------------------------------------------------*/
#define 	MEM_MAX 			 8	   			/* 最多允许的内存块管理区 */
#define 	MEM_USART_MAX 		 1024			/* 发送缓冲区的内存大小 */
#define 	MEM_USART_BLK 		 32				/* 每一个块的长度 */

/* Private typedef -----------------------------------------------------------*/
typedef struct MEMTCB
{
   void 	*MemFreeList;
   uint8_t 	MemBlkSize;
   uint8_t 	MemNBlks;
   uint8_t 	MemFreeNBlks;
}
MEMTcb;

/* Private function prototypes -----------------------------------------------*/
void MemInit(uint8_t *ptr,uint16_t count);
MEMTcb *MemCreate(uint8_t *ptr,uint8_t blksize,uint8_t nblks);
uint8_t *MemGet(MEMTcb *ptr,uint8_t *err);
uint8_t MemDelete(MEMTcb *ptr,uint8_t *index);

#endif

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
