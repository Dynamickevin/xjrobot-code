#ifndef _AM2320_BY_IIC_H_
#define _AM2320_BY_IIC_H_

/************************************************* 
*Function:		AM2320_and_iic_init
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  AM2320 芯片状态初始化 以及系统初始化
*************************************************/
void AM2320_and_iic_init(void);

//获取温度值
short AM2320_get_temper(void);

//获取湿度值
short AM2320_get_humidi(void);

#endif
