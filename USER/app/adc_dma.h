
#ifndef __ADC_DMA_H
#define __ADC_DMA_H

/************************************************* 
*Function:		adc_dma_init_and_start
*Input:			
*OUTPUT:		void
*Return:		void
*DESCRIPTION:  使用DMA函数 初始化ADC转换功能
*           当前使用的逻辑为，开启转换后，ADC转换会一直运行。转换的数据
*           依次存入 传入的 数据地址中 需要最新的值，直接从内存中取即可
*************************************************/
void adc_dma_init_and_start(
    u16*        pAdcTansDesData ,   //ADC转换后的数据存储位置
    const uint8_t*    pAdcChannleList ,   //需要转换的 通道列表
    u8          nAdcChannleCnt  ,   //需要转换的 通道数量
    u8          nAdcSampleTime      //ADC_SampleTime_239Cycles5
    );


#endif
