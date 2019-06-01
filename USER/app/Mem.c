/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               Mem.c
** Descriptions:            �ڴ�Ĺ�����
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
MEMTcb MemTcb[MEM_MAX];						/* �ڴ����������壬�������MEM_MAX���ڴ��� */
uint8_t USARTMemQ[MEM_USART_MAX];  		    /* �հ��ڴ��,MEM_USART_MAX��ʾ�ڴ�����е�������� */


/*******************************************************************************
* Function Name  : MemInit
* Description    : �հ��ڴ��ĳ�ʼ������
* Input          : - ptr: �հ������ڴ�����ָ��
*				   - count: �հ������ڴ����ڵ�����
* Output         : None
* Return         : �ڴ����׵�ַ
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
* Description    : ����һ���ڴ��������������ο�UCOS���ڴ����ṹ��
* Input          : - ptr: �հ������ڴ�����ָ��
*				   - blksize: ÿ���ڴ���ж��ٸ��ֽ�
*                  - nblks: ���ٸ��ڴ��
* Output         : None
* Return         : �ڴ����׵�ַ
* Attention		 : None
*******************************************************************************/	
MEMTcb *MemCreate(uint8_t *ptr,uint8_t blksize,uint8_t nblks)
{
	uint8_t *link = (ptr+blksize);
	void **plink = (void **)ptr;
	uint16_t i = 0;
	uint16_t j = 0;
	/* ���Ȳ��ҿհ׵��ڴ������ */
	while( (MemTcb[i].MemFreeList != (uint8_t*)0) && (i <= MEM_MAX) )		
	{														
		i++;
	}
	if(i >= MEM_MAX)								    /* �ڴ����������䲻�������ؿ�ָ�� */
	{
	  return (MEMTcb *)0;
	}								
	MemTcb[i].MemFreeList = ptr;					    /* �ڴ����������ڴ��ָ�� */
	MemTcb[i].MemBlkSize = blksize;						/* �ڴ���������ÿ���ڴ����ֽ� */
	MemTcb[i].MemNBlks = nblks;							/* �ڴ����������ڴ���������	*/
	MemTcb[i].MemFreeNBlks = nblks;						/* �ڴ��������Ŀհ��ڴ������� */
	j = i;
	MemInit(ptr,blksize*nblks);							/* ���ڴ���ڵ����ݳ�ʼ�� */
	for( i=0; i<nblks-1; i++ )								
	{
		*plink = (void *)(link);						/* ���ڴ��ĵ�ַ��ŵ��ǵڶ�Ƭ�ڴ������׵�ַ */
		plink = (void **)(link);						/* ����άָ�붨λ�������λ */		
		link += blksize;								/* һάָ�����¶�λ */
	}
//	*plink = (void *)0;									/* ���һ���ڴ�������һ����ַ�������Ϊ0 */
	return (MEMTcb *)(MemTcb+j); 						/* �����ڴ����������׵�ַ */
}	

/*******************************************************************************
* Function Name  : MemGet
* Description    : ��һ���ڴ��������ȡһ���ڴ��
* Input          : - ptr: �ڴ��������ָ��
*                  - err: ���ش�����Ϣ
* Output         : None
* Return         : ��ȡ�Ŀհ��ڴ����׵�ַ
* Attention		 : None
*******************************************************************************/	
uint8_t *MemGet(MEMTcb *ptr,uint8_t *err)
{
	void *tcb;
	uint8_t *index;
	tcb = (*ptr).MemFreeList;
	if( (*ptr).MemFreeNBlks == 0 )          /* ����հ��ڴ�������Ϊ����,����ȷ���أ��յ�������Ӧ����0 */
	{
	  *err = 0xff;
	  return (void *)0;
	}       
	(*ptr).MemFreeNBlks--;				    /* �հ��ڴ���������һ */
	if( (*ptr).MemFreeNBlks != 0 )
	{
	  (*ptr).MemFreeList = *(void **)tcb;	/* ǿ������ת�� */
	}
	index = (u8 *)tcb;
	index += 4;
	*err = 0;
	return index;	 						/* ���ػ�ȡ�Ŀհ��ڴ����׵�ַ */
}

/*******************************************************************************
* Function Name  : MemDelete
* Description    : ��һ���ڴ������ɾ��һ���ڴ��
* Input          : - ptr: �ڴ��������ָ��
* Output         : None
* Return         : ���Ҫɾ�����ڴ����һ����ָ�룬�򷵻�0xff�����ܹ�ɾ��������0
* Attention		 : None
*******************************************************************************/	
uint8_t MemDelete(MEMTcb *ptr,uint8_t *index)
{
	void *tcb;
	if( index == (void*)0 )  return 0xff;				/* ���Ҫɾ�����ڴ����һ����ָ�룬�򷵻�0xff */
	index -= 4;
	tcb = (void *)index;	
	*(void **)tcb = (*ptr).MemFreeList;				    /* ��OSMemFreeList����ָ������Ѿ���ɿհ��˵�ָ�� */
	(*ptr).MemFreeList = tcb;								/* ������հ׵�ָ�����һ��ָ��ָ��һ��ԭ�ȵ�OSMemFreeList */												//
	(*ptr).MemFreeNBlks++;								/* �հ��ڴ��������1 */
	return 0;	 		
}


/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
