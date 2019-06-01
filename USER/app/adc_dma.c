//#include "box_os_user.h"
#include <includes.h>

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
    )
{
    DMA_InitTypeDef DMA_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    u8 i;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);    /* Enable DMA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE);    /* Enable ADC1 and GPIOC clock */

    ADC_DeInit(ADC1); //������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ

    ADC_InitStructure.ADC_Mode                  = ADC_Mode_Independent; //ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
    ADC_InitStructure.ADC_ScanConvMode          = ENABLE; //ģ��ת��������ɨ��ģʽ
    ADC_InitStructure.ADC_ContinuousConvMode    = ENABLE; //ģ��ת������������ת��ģʽ
    ADC_InitStructure.ADC_ExternalTrigConv      = ADC_ExternalTrigConv_None; //�ⲿ����ת���ر�
    ADC_InitStructure.ADC_DataAlign             = ADC_DataAlign_Right; //ADC�����Ҷ���
    ADC_InitStructure.ADC_NbrOfChannel          = nAdcChannleCnt ; //˳����й���ת����ADCͨ������Ŀ
    ADC_Init(ADC1, &ADC_InitStructure); //����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���


    //����ָ��ADC�Ĺ�����ͨ�����������ǵ�ת��˳��Ͳ���ʱ��
    //ADC1,ADCͨ��x,�������˳��ֵΪy,����ʱ��Ϊ nAdcSampleTime ����  ADC_SampleTime_239Cycles5
    for( i=0 ; i<nAdcChannleCnt ; i++ )
    {
        ADC_RegularChannelConfig(ADC1, pAdcChannleList[i] , 1+i , nAdcSampleTime );
    }

    // ����ADC��DMA֧�֣�Ҫʵ��DMA���ܣ������������DMAͨ���Ȳ�����
    ADC_DMACmd(ADC1, ENABLE);

    ADC_Cmd(ADC1, ENABLE); //ʹ��ָ����ADC1

    ADC_ResetCalibration(ADC1); //��λָ����ADC1��У׼�Ĵ���
    while(ADC_GetResetCalibrationStatus(ADC1)); //��ȡADC1��λУ׼�Ĵ�����״̬,����״̬��ȴ�
    ADC_StartCalibration(ADC1); //��ʼָ��ADC1��У׼״̬
    while(ADC_GetCalibrationStatus(ADC1)); //��ȡָ��ADC1��У׼����,����״̬��ȴ�

    //���ת��ʹ��
    //ADC_SoftwareStartConvCmd( ADC1 ,ENABLE);

    //////////////////////////////////////////////////////
    //ADC  DMA1 ����
    DMA_DeInit(DMA1_Channel1); //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
    DMA_InitStructure.DMA_PeripheralBaseAddr    = (u32)&ADC1->DR       ; //DMA����ADC����ַ
    DMA_InitStructure.DMA_MemoryBaseAddr        = (u32)pAdcTansDesData ; //DMA�ڴ����ַ
    DMA_InitStructure.DMA_DIR                   = DMA_DIR_PeripheralSRC; //�ڴ���Ϊ���ݴ����Ŀ�ĵ�
    DMA_InitStructure.DMA_BufferSize            = nAdcChannleCnt ; //DMAͨ����DMA����Ĵ�С
    DMA_InitStructure.DMA_PeripheralInc         = DMA_PeripheralInc_Disable; //�����ַ�Ĵ�������
    DMA_InitStructure.DMA_MemoryInc             = DMA_MemoryInc_Enable; //�ڴ��ַ�Ĵ�������
    DMA_InitStructure.DMA_PeripheralDataSize    = DMA_PeripheralDataSize_HalfWord; //���ݿ��Ϊ16λ
    DMA_InitStructure.DMA_MemoryDataSize        = DMA_MemoryDataSize_HalfWord   ; //���ݿ��Ϊ16λ
    DMA_InitStructure.DMA_Mode                  = DMA_Mode_Circular; //������ѭ������ģʽ
    DMA_InitStructure.DMA_Priority              = DMA_Priority_High; //DMAͨ�� xӵ�и����ȼ�
    DMA_InitStructure.DMA_M2M                   = DMA_M2M_Disable; //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
    DMA_Init(DMA1_Channel1, &DMA_InitStructure); //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��

    /* ����DMA1ͨ��1��������ж� */
    //DMA_ITConfig(DMA1_Channel1,DMA_IT_TC, ENABLE);   //��ǰ��ʹ��DMA1���жϣ���˲�ʹ���ж�

    //ʹ��DMAͨ��1
    DMA_Cmd(DMA1_Channel1, ENABLE); 
    //ADC1ת������
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);	 
}

