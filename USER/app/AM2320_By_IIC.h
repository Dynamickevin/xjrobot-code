#ifndef _AM2320_BY_IIC_H_
#define _AM2320_BY_IIC_H_

/************************************************* 
*Function:		AM2320_and_iic_init
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  AM2320 оƬ״̬��ʼ�� �Լ�ϵͳ��ʼ��
*************************************************/
void AM2320_and_iic_init(void);

//��ȡ�¶�ֵ
short AM2320_get_temper(void);

//��ȡʪ��ֵ
short AM2320_get_humidi(void);

#endif
