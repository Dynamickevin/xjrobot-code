
#ifndef __USER_ADC_CTRL_H
#define __USER_ADC_CTRL_H
#include "adc_dma.h"

/************************************************* 
*Function:		adc_dma_init_gpio
*Input:			
*OUTPUT:		void
*Return:		void
*DESCRIPTION:  初始化 ADC 的IO口，需要用户自己修改初始化代码
*************************************************/
void adc_dma_init_gpio(void);

/************************************************* 
*Function:		user_adc_dma_init
*Input:			
*OUTPUT:		void
*Return:		void
*DESCRIPTION:  用户使用ADC 的初始化
*************************************************/
void user_adc_dma_init(void);

enum
{
    //PA2~PA3 PA6~PA7   
    ADC_TYPE_ID_PRESS1  = 0  ,//压力值ADC2
    ADC_TYPE_ID_PRESS2       ,//压力值ADC3
    ADC_TYPE_ID_MW_SP        ,//主动轮速度ADC6
    ADC_TYPE_ID_SW_SP        ,//从动轮速度ADC7
        
    //PC4 ; PC5 ADC1_14 
    ADC_TYPE_ID_BAT_CUR       ,//电池电流 ADC14
    ADC_TYPE_ID_BAT_VOT       ,//电池电压 ADC15
    
    ADC_TYPE_ID_USING_CNT ,  //使用的ADC通道的数量
};

#define ADC_CHANNEL_LIST_TO_ID   ADC_Channel_2 , \
                                 ADC_Channel_3 , \
                                 ADC_Channel_6 , \
                                 ADC_Channel_7 , \
                                 ADC_Channel_14 , \
                                 ADC_Channel_15 , \
                                 
                                 
u16 get_adc_val(u8 AdcTypeID);

#endif
