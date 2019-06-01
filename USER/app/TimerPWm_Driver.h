#ifndef __TIMER_PWM_DRIVER_H
#define __TIMER_PWM_DRIVER_H

#define MAX_HARD_TIMER_CNT    8
extern TIM_TypeDef* const gHardTimerList[MAX_HARD_TIMER_CNT+1];
static __inline TIM_TypeDef* GetTimerByID(u8 id)
{
    return gHardTimerList[id];
}

//////////////////////////////////////////////////////////////////////////
//��ʱ�� �������ɼ���������  TIM1  ~  TIM8   1 to 8
void TimerCode_DefaultFunction_Init(u32 timer_ID);

//CNT����궨�� �������Ϊ TIM2 / TIM3 / TIM4 ...
#define GetTimerCodeVal(timer_P)  ( (timer_P)->CNT )


//////////////////////////////////////////////////////////////////////////
//��ʱ��  PWM������Ƴ�ʼ�� �ⲿ�ֹ�����Ҫ�û�����ʵ������Զ������
void PWM_Init_Tim8_CH3_CH4(void);

//ʹ������� �������������Ӵ���ִ��Ч�ʣ�����Ҫִ�в����Ƿ���ȷ���ж�
//���� ������ �ٶ�
static __inline void SetPwm_Tim8_CH3(uint16_t Compare3)
{
    //TIM_SetCompare3( TIM8 , Compare3 );
    TIM8->CCR3 = Compare3;
}
//���� �Ӷ��� �ٶ�
static __inline void SetPwm_Tim8_CH4(uint16_t Compare4)
{
    //TIM_SetCompare4( TIM8 , Compare4 );
    TIM8->CCR4 = Compare4;
}


//��ĳ����ʱ�� ĳ��ͨ�������ú���ָ���������Ͷ���
//stm32�ṩ�˼��� tim_ocX_init ���ͺ�����
//    �ֱ�Ϊ��  TIM_OC1Init,TIM_OC2Init,TIM_OC3Init,TIM_OC4Init
//  ��Ӧ�Ĵ���: TIMx->CCR1   TIMx->CCR2  TIMx->CCR3  TIMx->CCR4
typedef void (*tim_ocX_init)(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct);

//TIM_OC1PreloadConfig  TIM_OC2PreloadConfig TIM_OC3PreloadConfig TIM_OC4PreloadConfig
typedef void (*tim_ocX_preload_init)(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload);

    //ch2   TIMx->CCR2 = TIMx->ARR/2 ;
    //ch3   TIMx->CCR3 = TIMx->ARR/2 ;
    //TIMx->PSC  TIMx->ARR  ����ֵ �ܺ��趨Ƶ��   Cpu/(TIMx->PSC+1)/(TIMx->ARR+1) = 72M /(TIMx->PSC+1)/(TIMx->ARR+1)


void timer2_3_4_5_pwm_init(TIM_TypeDef* TIMx,uint16_t Prescaler,uint16_t Period);
void timer1_8_pwm_init(TIM_TypeDef* TIMx,uint16_t Prescaler,uint16_t Period);

//ĳ����ʱ����ĳ��ͨ�� ��������
void timer2_3_4_5_pwm_channel_init(TIM_TypeDef* TIMx,tim_ocX_init ocX_init);
void timer1_8_pwm_channel_init(
    TIM_TypeDef*            TIMx,
    tim_ocX_init            ocX_init,        //TIM_OC1Init ...
    tim_ocX_preload_init    ocX_preload_init //TIM_OC1PreloadConfig ...
);


static __inline
void timer_set_Prescaler(TIM_TypeDef* TIMx,uint16_t Prescaler)
{
    TIMx->PSC = Prescaler;
}

//timer_set_fb_Period ���÷����������
//�Զ�����ֵ����4��ͨ���ıȽ�ֵ
static __inline
void timer_set_fb_Period(TIM_TypeDef* TIMx,uint16_t Period)
{
    TIMx->ARR = Period;
    Period >>= 1;
    TIMx->CCR1 = Period;
    TIMx->CCR2 = Period;
    TIMx->CCR3 = Period;
    TIMx->CCR4 = Period;
}


#endif
