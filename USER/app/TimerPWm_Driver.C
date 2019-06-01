//#include "box_os_user.h"
#include <includes.h>

TIM_TypeDef* const gHardTimerList[MAX_HARD_TIMER_CNT+1] = {
    TIM1 , TIM1 , TIM2 , TIM3 , TIM4 , TIM5 , TIM6 , TIM7 , TIM8
};

//////////////////////////////////////////////////////////////////////////
//多个定时器的 编码器功能 IO口配置函数  需要配置每个定时器的 1 2 通道为定时器功能复用
void TimerCode_And_Gpio_Init_TIM1(void)
{
    //BOX_ERROR_HANDLER( 777 );  //未实现 错误报警
}

//	从电机编码器采集	PA15 TIM2_CH1   PB3->TIM2_CH2 
//01：部分映像(CH1/ETR/PA15，CH2/PB3，CH3/PA2，CH4/PA3)
void TimerCode_And_Gpio_Init_TIM2(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 ,ENABLE);    /*使能TIM2 时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB , ENABLE); /*使能GPIOA/GPIOB 时钟*/

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable ,ENABLE);  //先将PA15作为普通IO口
    GPIO_PinRemapConfig( GPIO_PartialRemap1_TIM2 ,ENABLE);  //将定时器管脚映射到pd12 13 14 15引脚

    GPIO_InitStructure.GPIO_Speed  =GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 ;
    GPIO_Init(GPIOA,&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Speed  =GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 ;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
}

//	主电机编码器采集
//10：部分映像(CH1/PB4 ，CH2/PB5，CH3/PB0，CH4/PB1)
//PB4 – 90 – CK_CODEA_M2 – TIM3_CH1 （重映射）– 主动轮编码器A通道信号采集
//PB5 – 91 – CK_CODEB_M2 – TIM3_CH2 （重映射）– 主动轮编码器B通道信号采集
void TimerCode_And_Gpio_Init_TIM3(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);    /*使能TIM3时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);   /*使能GPIOB时钟*/
    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3,ENABLE);   //将定时器管脚映射到pd12 13 14 15引脚

    GPIO_InitStructure.GPIO_Speed  =GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5 ; /*timer3 重映射的通道1 2*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
}

//1：完全映像(TIM4_CH1/PD12 ，TIM4_CH2/PD13，TIM4_CH3/PD14，TIM4_CH4/PD15) 
// PD12 – 59 – CK_CODEB_CD – TIM4_CH1（重映射）– 从动轮轮子编码器B通道信号采集
// PD13 – 60 – CK_CODEA_CD – TIM4_CH2（重映射）– 从动轮轮子编码器A通道信号采集
// PD14 – 61 – CK_CODEZ_CD – TIM4_CH3（重映射）– 从动轮轮子编码器Z通道信号采集
void TimerCode_And_Gpio_Init_TIM4(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);    /*使能TIM4时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);   /*使能GPIOD时钟*/
    GPIO_PinRemapConfig(GPIO_Remap_TIM4,ENABLE);  		  //将定时器管脚映射到pd12 13 14 15引脚

    GPIO_InitStructure.GPIO_Speed  =GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13; /*timer4 重映射的通道1 2*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOD,&GPIO_InitStructure);
}


typedef void (*VoidFuncPtr) (void); 
const VoidFuncPtr TimerCode_And_Gpio_Inits[MAX_HARD_TIMER_CNT+1] = {
    TimerCode_And_Gpio_Init_TIM1 ,
    TimerCode_And_Gpio_Init_TIM1 ,
    TimerCode_And_Gpio_Init_TIM2 ,
    TimerCode_And_Gpio_Init_TIM3 ,
    TimerCode_And_Gpio_Init_TIM4 ,
    //TimerCode_And_Gpio_Init_TIM5 ,
    //TimerCode_And_Gpio_Init_TIM6 ,
    //TimerCode_And_Gpio_Init_TIM7 ,
    //TimerCode_And_Gpio_Init_TIM8 ,
};

//定时器 编码器采集功能驱动  TIM1  ~  TIM8
void TimerCode_DefaultFunction_Init(u32 timer_ID)
{
    TIM_TimeBaseInitTypeDef     TIM_TimeBaseStructure;
    TIM_ICInitTypeDef           TIM4_ICInitStructure;
    
    (*TimerCode_And_Gpio_Inits[timer_ID])();

    TIM_DeInit( gHardTimerList[timer_ID] );
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period        = 0xFFFF;				//设定计数器重装值
    TIM_TimeBaseStructure.TIM_Prescaler     = 0;					//TIM4时钟预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;		//设置时钟分割 T_dts = T_ck_int	
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up; //TIM向上计数 
    TIM_TimeBaseInit( gHardTimerList[timer_ID] , &TIM_TimeBaseStructure);

    TIM_EncoderInterfaceConfig( gHardTimerList[timer_ID] , TIM_EncoderMode_TI12, TIM_ICPolarity_Rising ,TIM_ICPolarity_Rising);//使用编码器模式3，上升计数
    TIM_ICStructInit(&TIM4_ICInitStructure);						//将结构体中的内容缺省输入   通道1输入
    TIM4_ICInitStructure.TIM_ICFilter = 0;							//选择输入比较滤波器 
    TIM_ICInit( gHardTimerList[timer_ID] , &TIM4_ICInitStructure);						//将TIM_ICInitStructure中的指定参数初始化TIM4

    TIM_ITConfig( gHardTimerList[timer_ID] , TIM_IT_Update, DISABLE );						//运行更新中断 禁用

    gHardTimerList[timer_ID]->CNT = 0; //初始化时清空计数
    TIM_Cmd( gHardTimerList[timer_ID] , ENABLE);											//启动TIM?定时器
}



/************************************************* 
*Function:	 PWM_Init_Tim8_CH3_CH4	
*Input:			
*OUTPUT:		    
*Return:		
*DESCRIPTION:电机PWM波初始化配置函数
*           PC8 – 65 – MT_CTL_PWM1 – TIM8_CH3 – 主动轮PWM控制
*           PC9 – 66 – MT_CTL_PWM2 – TIM8_CH4 – 从动轮PWM控制
*************************************************/
void PWM_Init_Tim8_CH3_CH4(void)
{
    GPIO_InitTypeDef            GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef     TIM_TimeBaseStructure;
    TIM_OCInitTypeDef           TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE); //打开C组GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);  // 打开定时器8时钟

    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8|GPIO_Pin_9;//
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;  
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;   
    GPIO_Init(GPIOC,&GPIO_InitStructure);


    //   TIM_TimeBaseStructure.TIM_RepetitionCounter=1;  . 
    TIM_TimeBaseStructure.TIM_ClockDivision=0;
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数
    TIM_TimeBaseStructure.TIM_Period=100;
    TIM_TimeBaseStructure.TIM_Prescaler=360;
    TIM_TimeBaseInit(TIM8,&TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM2        ;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_Low     ;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low    ;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable ;
    TIM_OCInitStructure.TIM_Pulse       = 0                      ;

    TIM_OCInitStructure.TIM_OutputNState    = TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_OCNIdleState    = TIM_OCNIdleState_Reset;
    TIM_OCInitStructure.TIM_OCIdleState     = TIM_OCIdleState_Reset ;

    TIM_OC3Init(TIM8,&TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM8,TIM_OCPreload_Enable); //TIM_8---->通道3[PC8] 

    TIM_OC4Init(TIM8,&TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM8,TIM_OCPreload_Enable); 

    TIM_CtrlPWMOutputs(TIM8,ENABLE);
    TIM_Cmd(TIM8,ENABLE);
}


void timer1_8_pwm_init(TIM_TypeDef* TIMx,uint16_t Prescaler,uint16_t Period)
{
    TIM_TimeBaseInitTypeDef     TIM_TimeBaseStructure;
    TIM_DeInit(TIMx);
    //   TIM_TimeBaseStructure.TIM_RepetitionCounter=1;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up; //向上计数
    TIM_TimeBaseStructure.TIM_Period   = Period;
    TIM_TimeBaseStructure.TIM_Prescaler= Prescaler ;
    TIM_TimeBaseInit(TIMx,&TIM_TimeBaseStructure);
    TIM_CtrlPWMOutputs(TIMx,ENABLE);
}

void timer2_3_4_5_pwm_init(TIM_TypeDef* TIMx,uint16_t Prescaler,uint16_t Period)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_DeInit(TIMx);
    ////TIM_InternalClockConfig(TIMx);
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_ClockDivision=0;
    TIM_TimeBaseStructure.TIM_Period   = Period ;//;//ARR的值周期10K
    TIM_TimeBaseStructure.TIM_Prescaler= Prescaler ;
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
}


void timer2_3_4_5_pwm_channel_init(TIM_TypeDef* TIMx,tim_ocX_init ocX_init)
{
    TIM_OCInitTypeDef TIMOCInitStructure;
    TIMOCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //PWM模式1输出
    TIMOCInitStructure.TIM_Pulse = TIMx->ARR/2 ; //  0;//占空比=(CCRx/ARR)*100%
    TIMOCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;//TIM输出比较极性高
    TIMOCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//使能输出状态
    ocX_init(TIMx, &TIMOCInitStructure);//TIM3的CH2输出 TIM_OC2Init
}
void timer1_8_pwm_channel_init(
    TIM_TypeDef*            TIMx,
    tim_ocX_init            ocX_init,
    tim_ocX_preload_init    ocX_preload_init
)
{
    TIM_OCInitTypeDef TIMOCInitStructure;
        
    //TIMOCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //PWM模式1输出
    TIMOCInitStructure.TIM_Pulse       = TIMx->ARR/2            ; //  0;//占空比=(CCRx/ARR)*100%
    TIMOCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1        ;
    TIMOCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_Low     ;
    TIMOCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low    ;
    TIMOCInitStructure.TIM_OutputState = TIM_OutputState_Enable ;

    TIMOCInitStructure.TIM_OutputNState    = TIM_OutputNState_Disable;
    TIMOCInitStructure.TIM_OCNIdleState    = TIM_OCNIdleState_Reset;
    TIMOCInitStructure.TIM_OCIdleState     = TIM_OCIdleState_Reset ;

    ocX_init( TIMx ,&TIMOCInitStructure);
    ocX_preload_init( TIMx , TIM_OCPreload_Enable ); //TIM_8---->通道3[PC8] 
}


