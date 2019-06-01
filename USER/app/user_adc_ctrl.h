
#ifndef __USER_ADC_CTRL_H
#define __USER_ADC_CTRL_H
#include "adc_dma.h"

/************************************************* 
*Function:		adc_dma_init_gpio
*Input:			
*OUTPUT:		void
*Return:		void
*DESCRIPTION:  ��ʼ�� ADC ��IO�ڣ���Ҫ�û��Լ��޸ĳ�ʼ������
*************************************************/
void adc_dma_init_gpio(void);

/************************************************* 
*Function:		user_adc_dma_init
*Input:			
*OUTPUT:		void
*Return:		void
*DESCRIPTION:  �û�ʹ��ADC �ĳ�ʼ��
*************************************************/
void user_adc_dma_init(void);

enum
{
    //PA2~PA3 PA6~PA7   
    ADC_TYPE_ID_PRESS1  = 0  ,//ѹ��ֵADC2
    ADC_TYPE_ID_PRESS2       ,//ѹ��ֵADC3
    ADC_TYPE_ID_MW_SP        ,//�������ٶ�ADC6
    ADC_TYPE_ID_SW_SP        ,//�Ӷ����ٶ�ADC7
        
    //PC4 ; PC5 ADC1_14 
    ADC_TYPE_ID_BAT_CUR       ,//��ص��� ADC14
    ADC_TYPE_ID_BAT_VOT       ,//��ص�ѹ ADC15
    
    ADC_TYPE_ID_USING_CNT ,  //ʹ�õ�ADCͨ��������
};

#define ADC_CHANNEL_LIST_TO_ID   ADC_Channel_2 , \
                                 ADC_Channel_3 , \
                                 ADC_Channel_6 , \
                                 ADC_Channel_7 , \
                                 ADC_Channel_14 , \
                                 ADC_Channel_15 , \
                                 
                                 
u16 get_adc_val(u8 AdcTypeID);

#endif
