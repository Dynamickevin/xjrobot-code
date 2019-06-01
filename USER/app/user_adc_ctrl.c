//#include "box_os_user.h"
#include <includes.h>

const uint8_t gUserAdcChannelUsing[] = { ADC_CHANNEL_LIST_TO_ID };

//下面的宏定义用于判断 列表中数量 和 宏定义的数量相同
//ACI_ASSERT_VAL_SAME( AdcChannelCnt , sizeof(gUserAdcChannelUsing) , ADC_TYPE_ID_USING_CNT );

u16 gUserAdcTransDatas[ADC_TYPE_ID_USING_CNT];

/************************************************* 
*Function:		adc_dma_init_gpio
*Input:			
*OUTPUT:		void
*Return:		void
*DESCRIPTION:  初始化 ADC 的IO口，需要用户自己修改初始化代码
*************************************************/
void adc_dma_init_gpio(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC,ENABLE); 
    
    GPIO_INIT_AIN( GPIO_CHK_PRS_VOL1 );
    GPIO_INIT_AIN( GPIO_CHK_PRS_VOL2 );
    GPIO_INIT_AIN( GPIO_CHK_MT_SPD_M );
    GPIO_INIT_AIN( GPIO_CHK_MT_SPD_S );
    GPIO_INIT_AIN( GPIO_CHK_BAT_CUR  );
    GPIO_INIT_AIN( GPIO_CHK_BAT_VOL	 );
}


/************************************************* 
*Function:		user_adc_dma_init
*Input:			
*OUTPUT:		void
*Return:		void
*DESCRIPTION:  用户使用ADC 的初始化
*************************************************/
void user_adc_dma_init(void)
{
    adc_dma_init_gpio();
    adc_dma_init_and_start( gUserAdcTransDatas , gUserAdcChannelUsing , ADC_TYPE_ID_USING_CNT , ADC_SampleTime_239Cycles5 );
}

u16 get_adc_val(u8 AdcTypeID)
{
    //BOX_OS_ASSERT(AdcTypeID>=ADC_TYPE_ID_USING_CNT);
    return gUserAdcTransDatas[AdcTypeID];
}
