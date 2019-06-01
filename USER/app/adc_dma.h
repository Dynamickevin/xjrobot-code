
#ifndef __ADC_DMA_H
#define __ADC_DMA_H

/************************************************* 
*Function:		adc_dma_init_and_start
*Input:			
*OUTPUT:		void
*Return:		void
*DESCRIPTION:  ʹ��DMA���� ��ʼ��ADCת������
*           ��ǰʹ�õ��߼�Ϊ������ת����ADCת����һֱ���С�ת��������
*           ���δ��� ����� ���ݵ�ַ�� ��Ҫ���µ�ֵ��ֱ�Ӵ��ڴ���ȡ����
*************************************************/
void adc_dma_init_and_start(
    u16*        pAdcTansDesData ,   //ADCת��������ݴ洢λ��
    const uint8_t*    pAdcChannleList ,   //��Ҫת���� ͨ���б�
    u8          nAdcChannleCnt  ,   //��Ҫת���� ͨ������
    u8          nAdcSampleTime      //ADC_SampleTime_239Cycles5
    );


#endif
