//#include "box_os_user.h"
#include <includes.h>

extern ZT_INFO_TYPE g_zt_msg;
SpeedAnalyByCode gSpeedAnaly_Mst;    //MST  master 缩写
SpeedAnalyByCode gSpeedAnaly_Slv;    //从动轮控制电机 编码器
SpeedAnalyByCode gSpeedAnaly_Whl;    //从动轮 码盘wheel,  Whl -- 缩写
SlvMtCfgType     gSlvMtCfg;  		 //配置参数
SlaveMotorAnaly  gSlaveMtAnaly;
MtDisControlCheck gMtDisControlCheck;
BatAutoCtrl		 gBatAutoCtrl;
RbtState 		 gRbtState;

#define OUT_LOG_SLAVEMT  USART_DEBUG_OUT
#define OUT_LOG_SLAVE_BYTE(byte) // uart_send_byte( DEBUG_PORT , (byte) )

static __inline void real_set_slv_mt_close(void)
{
    SET_SLAVE_MOTOR_CLOSE();
    gSlaveMtAnaly.real_out_speed = 0;
    if ( gSlaveMtAnaly.ys_cur_real_dir != SLV_DIR_TO_PRESS_CLOSE )
    {
        gSlaveMtAnaly.ys_last_ctrl_dir    = gSlaveMtAnaly.ys_cur_real_dir;
        gSlaveMtAnaly.ys_cur_real_dir     = SLV_DIR_TO_PRESS_CLOSE;
        gSlaveMtAnaly.ys_need_close_delay = SLV_DIR_TO_DIF_DELAY_DEFAULT;
    }
    gSlaveMtAnaly.ys_to_set_dir   = SLV_DIR_TO_PRESS_CLOSE ;
    gSlaveMtAnaly.ys_to_set_speed = 0 ;
    gSlaveMtAnaly.ys_cur_real_speed = 0;
}


/************************************************* 
*Function:		zt_motor_slave_driver_cfg_init
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  从动轮电机 配置参数初始化
*************************************************/
void zt_motor_slave_driver_cfg_init(void)
{
    float tmp;
    const UserCfgType* pcfg = (const UserCfgType*)( MCU_FLASH_ADDR_BEGIN+0x68000 );
    if ( (pcfg->begin_cfg==0XAAAAAAAA) && (pcfg->end_cfg==0X55555555) )
    {
        memcpy( &gSlvMtCfg , &(pcfg->SlvMtCfg) , sizeof(gSlvMtCfg) );
		//CopyBuffer(&(pcfg->SlvMtCfg), &gSlvMtCfg, sizeof(gSlvMtCfg));
    }
    else
    {
        gSlvMtCfg.bat_no_move     = 220;			//机器人不能移动时的电压
        gSlvMtCfg.bat_auto_charge = 225;			//机器人需要充电的电压
        gSlvMtCfg.bat_charge_full = 245;			//机器人充满时的电压
        gSlvMtCfg.onBridgeTime = 6; 				//桥上行走判断时间为6秒
        gSlvMtCfg.mstAddAccl = 10;
        gSlvMtCfg.mstDelAccl = 40;
        //AT+SlvCFG=6 pmin=40 pmax=110 pmid=75 StopMin=20 StopMax=150 SpeedBrg=10
        gSlvMtCfg.press_ok_min         = 40;  //30;  //40;  //   正常运行过程中，压力的最小值
        gSlvMtCfg.press_ok_max         = 70;  //70;  //100; //   正常运动过程中，压力的最大值
        gSlvMtCfg.press_mst_stop_min   = 10;  //10;  //20;  //   压力太小，需要停止主动轮 停止主动轮压力最小值
        gSlvMtCfg.press_mst_stop_max   = 120; //120; //150; //   压力太大，需要停止主动轮 停止主动轮压力最大值
        gSlvMtCfg.mst_limit_on_bridge  = 16;  //10;  //10;  //   在桥上的限速 //50;  //70;  //   需要进行调整时，目标压力值
    }
    
    //gSlvMtCfg.press_ok_to_adjust   = (gSlvMtCfg.press_ok_min+gSlvMtCfg.press_ok_max)>>1; 
    gSlvMtCfg.press_ok_to_adjust   = gSlvMtCfg.press_ok_max - 
        ((gSlvMtCfg.press_ok_max-gSlvMtCfg.press_ok_min)/3) ; 

    //从动轮 上行时 限速的系数       		k*(press_ok_to_adjust-press_mst_stop_min)^2 = 100
    gSlvMtCfg.fk_mst_limit_up =  gSlvMtCfg.press_ok_to_adjust - gSlvMtCfg.press_mst_stop_min; 
    gSlvMtCfg.fk_mst_limit_up = gSlvMtCfg.fk_mst_limit_up*gSlvMtCfg.fk_mst_limit_up ;
    gSlvMtCfg.fk_mst_limit_up = 100.0f / gSlvMtCfg.fk_mst_limit_up ;

	//从动轮 下行时 限速的系数		        k*(press_mst_stop_max-press_ok_to_adjust)^2 = 100
    gSlvMtCfg.fk_mst_limit_down = gSlvMtCfg.press_mst_stop_max - gSlvMtCfg.press_ok_to_adjust  ;   
    gSlvMtCfg.fk_mst_limit_down = gSlvMtCfg.fk_mst_limit_down*gSlvMtCfg.fk_mst_limit_down ;
    gSlvMtCfg.fk_mst_limit_down = 100.0f / gSlvMtCfg.fk_mst_limit_down ;
    
    
    //线性关系的系数
    //gSlvMtCfg.fk_slv_speed_by_press_up  =(80.0f-20.0f)/(gSlvMtCfg.press_ok_to_adjust-gSlvMtCfg.press_mst_stop_min); //上升过程压力与速度系数
    //gSlvMtCfg.fk_slv_speed_by_press_down=(70.0f-20.0f)/(gSlvMtCfg.press_ok_to_adjust-gSlvMtCfg.press_mst_stop_max); //下降过程压力与速度系数
    //二次关系 系数 在目标压力处，速度为0
    tmp = (gSlvMtCfg.press_ok_to_adjust - gSlvMtCfg.press_mst_stop_min) ;
    gSlvMtCfg.fk_slv_speed_by_press_up  = 60.0f/(tmp*tmp);
    tmp = (gSlvMtCfg.press_ok_to_adjust - gSlvMtCfg.press_mst_stop_max) ;
    gSlvMtCfg.fk_slv_speed_by_press_down = 50.0f/(tmp*tmp);

}

/************************************************* 
*Function:		zt_motor_slave_driver_init
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  从动轮电机  		   初始化 
*************************************************/
void zt_motor_slave_driver_init(void)
{
    //PA12 从动轮 电机方向控制 
    //GPIO_InitTypeDef            GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA , ENABLE);
    
    GPIO_INIT_OUT_PP(GPIO_CTL_DIR_S1);
    GPIO_INIT_OUT_PP(GPIO_CTL_DIR_S2);
    GPIO_INIT_OUT_PP(GPIO_CTL_DIR_M1);
    GPIO_INIT_OUT_PP(GPIO_CTL_DIR_M2);
    GPIO_INIT_OUT_PP(GPIO_BREAK_MEN );
    SET_MASTER_MOTOR_CLOSE() ;
    SET_SLAVE_MOTOR_CLOSE()  ;    
    SET_MT_BREAK_CLOSE       ;
    
    GPIO_INIT_IN_FLOAT(GPIO_CHK_LIMIT1);
    GPIO_INIT_IN_FLOAT(GPIO_CHK_LIMIT1 );
    
    
    //编码器值 进行初始化
    TimerCode_DefaultFunction_Init(3);  //编码器 数据采集初始化 从动轮 TIM3

    memset( &gBatAutoCtrl    , 0 , sizeof(gBatAutoCtrl    ) );
    memset( &gSpeedAnaly_Mst , 0 , sizeof(SpeedAnalyByCode) );
    memset( &gSpeedAnaly_Slv , 0 , sizeof(SpeedAnalyByCode) );
    memset( &gSpeedAnaly_Whl , 0 , sizeof(SpeedAnalyByCode) );
    memset( &gSlaveMtAnaly   , 0 , sizeof(gSlaveMtAnaly   ) );
    memset( &gMtDisControlCheck , 0 , sizeof(gMtDisControlCheck) );
    gSlaveMtAnaly.test_out_debug = 0;

    //各种 配置信息的初始化
    zt_motor_slave_driver_cfg_init();
    
    SET_MASTER_MOTOR_CLOSE();
    SET_SLAVE_MOTOR_CLOSE() ;

	//从动轮码盘有关参数
	gSlaveMtAnaly.CodeScale = 100;
    gSlaveMtAnaly.curBridgeCodeHalf  = 50;
    /*BX_CALL(BoxOsTaskCreate)(
        "MotorUpdate" ,
        &gZtMotorUpdate_Task ,
        TaskFun_MotorUpdate  ,
        0 ,
        &stack_MotorCtrlTask[DATA_ARRAY_CNT(stack_MotorCtrlTask) - 1] ,
        &stack_MotorCtrlTask[0]
    );*/
}

/************************************************* 
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  设置 从动轮电机的速度
*************************************************/
s16 zt_motor_slave_driver_set_speed(s16 speed,u16 code_run)
{
    s16 speed_abs = (speed>0)? speed : (-speed);
        
    if ( speed_abs >= 100 )
    {
        if( gSlaveMtAnaly.s_SlvMtState != SLAVE_MT_CTRL_AUTO )
		{
            gSlaveMtAnaly.s_SlvMtState = SLAVE_MT_CTRL_AUTO;
            gSlaveMtAnaly.auto_cal_speed = 0;
            gSlaveMtAnaly.hand_set_speed = 0;
            real_set_slv_mt_close();
        }
    }
#ifndef MOTER_PERFORM_TEST_EN
    else if ( speed_abs<2 ) 
#else
    else if ( speed_abs<12 ) 
#endif

    {
        gSlaveMtAnaly.s_SlvMtState = SLAVE_MT_CTRL_HANDLE;
        gSlaveMtAnaly.hand_set_speed = 0;
        real_set_slv_mt_close();
    }
    else
    {
        if( speed_abs > 85 ){ speed_abs = 85; }
        if(speed<0){ speed_abs = -speed_abs; }
        gSlaveMtAnaly.s_SlvMtState = SLAVE_MT_CTRL_HANDLE;
        gSlaveMtAnaly.hand_set_speed = speed_abs;
        //real_set_slv_mt_speed(bFanxiang,speed); //  SetPwm_Tim8_CH4(  );  //电机当前速度为  speed
    }

    return gSlaveMtAnaly.hand_set_speed;
}


/************************************************* 
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  三个编码器的速度分析过程
*************************************************/
static u16 nLastCodeRecordTime = 0;
static u16 nSpeedAnaly_ChangeId = 0;
//static u16 nSpeedAnaly_SlvMtChangeId = 0;
static __inline void DoSpeedAnalyByCode(void)
{
    //gSlaveMtAnaly.b_XianWei_Up   = !GpioGet(GPIO_CHK_LIMIT1) ;
    //gSlaveMtAnaly.b_XianWei_Down = !GpioGet(GPIO_CHK_LIMIT2) ;
    gSlaveMtAnaly.b_XianWei_Up   = (!GpioGet(GPIO_CHK_LIMIT1)) || (!GpioGet(GPIO_CHK_LIMIT2));
    gSlaveMtAnaly.b_XianWei_Down = gSlaveMtAnaly.b_XianWei_Up;

    //从电机 的控制 需要进行快速响应的控制 这时需要 获取当前 20ms 的电机速度
    //根据20ms 的电机速度 计算电机立即停止，会再行走的距离 以及压力变化值
    //做一个大体的评估  因此先需要获取 20ms速度
    // gSpeedAnaly_Slv.speed > 0; is up running
    gSpeedAnaly_Slv.speed = ( 1614 - get_adc_val(ADC_TYPE_ID_SW_SP) ) / (13) ;
    gSpeedAnaly_Mst.speed = ( 1614 - get_adc_val(ADC_TYPE_ID_MW_SP) ) / (-10) ;

    //不需要实时采集从动轮的编码器值 每隔一段时间采集一次 多次采集，计算平均速度
    nLastCodeRecordTime++;
    if ( nLastCodeRecordTime < SPEED_ANALY_CODE_DIVIDER )
    {
        return;
    }
    nLastCodeRecordTime = 0;

    nSpeedAnaly_ChangeId++;
    if ( nSpeedAnaly_ChangeId >= SPEED_ANALY_CODE_MAX_CNT )
    {
        nSpeedAnaly_ChangeId = 0;
    }
    gSpeedAnaly_Whl.original_oldest_code = gSpeedAnaly_Whl.original_codes[nSpeedAnaly_ChangeId];
    gSpeedAnaly_Whl.original_codes[nSpeedAnaly_ChangeId] = GET_SLAVE_WHEEL_CODE();
    gSpeedAnaly_Whl.speed = gSpeedAnaly_Whl.original_codes[nSpeedAnaly_ChangeId] - gSpeedAnaly_Whl.original_oldest_code;
}


//获取 最新的编码器值与某一个时间段内的编码器变化量
s16 GetCode_DiffFromSaveList(SpeedAnalyByCode* code_list,u16 save_tick)
{
    s16 get_Id = nSpeedAnaly_ChangeId - save_tick;
    if ( get_Id < 0 )
    {
        get_Id += SPEED_ANALY_CODE_MAX_CNT;
    }
    if ( (get_Id<0) && (get_Id>=SPEED_ANALY_CODE_MAX_CNT) )
    {
        return 0;
    }
    return (code_list->original_codes[nSpeedAnaly_ChangeId] - code_list->original_codes[get_Id]);
}

//在压力过大 或者 压力过小时 限制主动轮速度
static __inline void DoLimitMstMtSpeed(u16 limit_speed,u16 limitTime) //,u16 time_hold
{
    gMstMt.limit_speed      = limit_speed*100;
    gMstMt.limit_speed_time = limitTime ;
}

void SLV_AUTO_SPEED_UP(void)
{
    s16 detPress = (gSlvMtCfg.press_ok_to_adjust-gPressFilter.val) ;
    if( gPressFilter.val < gSlvMtCfg.press_mst_stop_min )
	{
        gSlaveMtAnaly.auto_cal_speed = 60;
    }
    else
	{
        //gSlaveMtAnaly.auto_cal_speed = gSlvMtCfg.fk_slv_speed_by_press_up*detPress + 20;
        gSlaveMtAnaly.auto_cal_speed = gSlvMtCfg.fk_slv_speed_by_press_up*detPress*detPress;
        if( gSlaveMtAnaly.auto_cal_speed < 20 )
		{
            gSlaveMtAnaly.auto_cal_speed = 20;
        }
    }
    //gSlvMtCfg.fk_slv_speed_by_press_up  =(80-20)/(gSlvMtCfg.press_ok_to_adjust-gSlvMtCfg.press_mst_stop_min); //上升过程压力与速度系数
}
void SLV_AUTO_SPEED_DOWN(void)
{
    s16 detPress = (gPressFilter.val-gSlvMtCfg.press_ok_to_adjust) ;
    //gSlvMtCfg.fk_slv_speed_by_press_down=(70-20)/(gSlvMtCfg.press_ok_to_adjust-gSlvMtCfg.press_mst_stop_max);
    if( gPressFilter.val > gSlvMtCfg.press_mst_stop_max )
	{
        gSlaveMtAnaly.auto_cal_speed = -50;
    }
    else
	{
        //gSlaveMtAnaly.auto_cal_speed = gSlvMtCfg.fk_slv_speed_by_press_down*detPress - 20;
        gSlaveMtAnaly.auto_cal_speed = -gSlvMtCfg.fk_slv_speed_by_press_down*detPress*detPress;
        if( gSlaveMtAnaly.auto_cal_speed > -20 )
		{
            gSlaveMtAnaly.auto_cal_speed = -20;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//从动轮没有运动，判断从动轮是否需要启动
static __inline void zt_slv_mt_determine_need_move(void)
{
    gSlaveMtAnaly.press_to_adj = gSlvMtCfg.press_ok_to_adjust ;
    if ( gPressFilter.val < gSlvMtCfg.press_ok_min )
    {
        //gSlaveMtAnaly.real_slv_up_press_to_adg = gSlvMtCfg.press_ok_to_adjust + 10 ;
        gSlaveMtAnaly.real_slv_up_press_to_adg = gSlvMtCfg.press_ok_max - 4 ;
        SLV_AUTO_SPEED_UP();
    }
    else if ( gPressFilter.val > gSlvMtCfg.press_ok_max )
    {
        //需要松开从动轮
        gSlaveMtAnaly.real_slv_down_press_to_adg = gSlvMtCfg.press_ok_to_adjust ;
        SLV_AUTO_SPEED_DOWN();
    }
    else  //当前压力在正常范围内
    {
        gSlaveMtAnaly.auto_cal_speed = 0;
    }
}


static float slv_mt_stop_time = 0;
#define MIN_PRESS_CAL_DET_TO_STOP   25
/************************************************* 
*Return:	void
*DESCRIPTION:  电机上升过程
*       根据当前压力值，从电机速度值，判断如果立即停止
*       压力变化目标值
*************************************************/
static __inline void slv_mt_cal_stop_press_when_up(void)
{
    //不同压力下，停止时间不同 gPressFilter.val
    if( gSpeedAnaly_Slv.speed>3 )
	{
        slv_mt_stop_time = gSpeedAnaly_Slv.speed ;
        slv_mt_stop_time *= slv_mt_stop_time ;
    }
    else
	{
        slv_mt_stop_time = 0;
    }
    if( gPressFilter.val < MIN_PRESS_CAL_DET_TO_STOP )
	{
        slv_mt_stop_time /= MIN_PRESS_CAL_DET_TO_STOP;
    }
    else
	{
        slv_mt_stop_time /= gPressFilter.val;
    }
    gSlaveMtAnaly.press_cal_stop = slv_mt_stop_time * 1.5f ;//原始值为 5
    gSlaveMtAnaly.press_cal_stop += gPressFilter.val;
}
/************************************************* 
*Return:	void
*DESCRIPTION:  电机下降过程
*       根据当前压力值，从电机速度值，判断如果立即停止
*       压力变化目标值
*************************************************/
static __inline void slv_mt_cal_stop_press_when_down(void)
{
    //不同压力下，停止时间不同 gPressFilter.val
    if( gSpeedAnaly_Slv.speed<-3 )
	{
        slv_mt_stop_time = gSpeedAnaly_Slv.speed ;
        slv_mt_stop_time *= slv_mt_stop_time ;
    }
    else
	{
        slv_mt_stop_time = 0;
    }
    if( gPressFilter.val < MIN_PRESS_CAL_DET_TO_STOP )
	{
        slv_mt_stop_time /= MIN_PRESS_CAL_DET_TO_STOP;
    }
    else
	{
        slv_mt_stop_time /= gPressFilter.val;
    }
    gSlaveMtAnaly.press_cal_stop = slv_mt_stop_time * -1.5f ; //系数原始为-2.5f
    gSlaveMtAnaly.press_cal_stop += gPressFilter.val;
    if( gSlaveMtAnaly.press_cal_stop < 0 )
	{
        gSlaveMtAnaly.press_cal_stop = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
//从动轮 压紧 判断是否需要关闭从动轮
static __inline void zt_slv_mt_determine_stop_when_up(void)
{
    //如果当前压力已经调整到目标压力了，停止电机
    if ( gPressFilter.val >= gSlaveMtAnaly.press_to_adj )
    {
        OUT_LOG_SLAVE_BYTE('!');
        gSlaveMtAnaly.auto_cal_speed = 0;
        return;
    }

    //gSlaveMtAnaly.mst_speed_limit_det_pr = gSlaveMtAnaly.press_to_adj - gPressFilter.val ;
    //gSlaveMtAnaly.mst_speed_limit = gSlvMtCfg.fk_mst_limit_up * gSlaveMtAnaly.mst_speed_limit_det_pr * gSlaveMtAnaly.mst_speed_limit_det_pr ;
    //if ( (gSlaveMtAnaly.mst_speed_limit>0) && (gSlaveMtAnaly.mst_speed_limit<100) )
    //{
    //    //DoLimitMstMtSpeed(100 - ((u16)(gSlaveMtAnaly.mst_speed_limit)));
    //    DoLimitMstMtSpeed( gSlvMtCfg.mst_limit_on_bridge );  //速度
    //}
    //else
    //{
    //    DoLimitMstMtSpeed( 0 );
    //}

    //预测关闭时 压力值 如果压力大于需要调节的压力
    slv_mt_cal_stop_press_when_up();
    if (( gPressFilter.val>gSlvMtCfg.press_mst_stop_min ) && 
        ( gSlaveMtAnaly.press_cal_stop >= gSlaveMtAnaly.real_slv_up_press_to_adg ) )
        //( gSlaveMtAnaly.press_cal_stop >= gSlaveMtAnaly.press_to_adj ) ) //gSlvMtCfg.press_ok_max
    {
        OUT_LOG_SLAVE_BYTE('@');
        gSlaveMtAnaly.auto_cal_speed = 0;
        return;
    }
    
    
    //压紧过程，主动轮限速的计算 如果从动轮停止，不需要限制主电机速度了
    if( gPressFilter.val < gSlvMtCfg.press_mst_stop_min )            //需要停止
	{ 
        //if( gMstMt.limit_speed > gSlvMtCfg.mst_limit_on_bridge )
        DoLimitMstMtSpeed( 0 , (OS_TICKS_PER_SEC/4)*gSlvMtCfg.onBridgeTime/2 );
    }
    else
	{
        DoLimitMstMtSpeed( gSlvMtCfg.mst_limit_on_bridge-2 , (OS_TICKS_PER_SEC/4)*gSlvMtCfg.onBridgeTime/2 );
    }
    
    //如果当前还需要继续压紧，计算压紧的实时速度
    SLV_AUTO_SPEED_UP();
}

//////////////////////////////////////////////////////////////////////////
//从动轮 松开 判断是否需要 进行松开
static __inline void zt_slv_mt_determine_stop_when_down(void)
{
    if ( gPressFilter.val <= gSlaveMtAnaly.press_to_adj )
    {
        gSlaveMtAnaly.auto_cal_speed = 0;
        return;
    }

    //主动轮限速的计算  根据压力计算限速
    //gSlaveMtAnaly.mst_speed_limit_det_pr =  gPressFilter.val - gSlaveMtAnaly.press_to_adj;
    //gSlaveMtAnaly.mst_speed_limit = gSlvMtCfg.fk_mst_limit_down * gSlaveMtAnaly.mst_speed_limit_det_pr * gSlaveMtAnaly.mst_speed_limit_det_pr ;
    //if ( (gSlaveMtAnaly.mst_speed_limit>0) && (gSlaveMtAnaly.mst_speed_limit<100) )
    //{
    //    DoLimitMstMtSpeed( gSlvMtCfg.mst_limit_on_bridge ); //(100 - ((u16)(gSlaveMtAnaly.mst_speed_limit)));
    //}
    //else
    //{
    //    DoLimitMstMtSpeed( 0 );
    //}

    //预测关闭时 压力值 如果压力大于
    slv_mt_cal_stop_press_when_down();
    if (( gPressFilter.val<gSlvMtCfg.press_mst_stop_max ) && 
        ( gSlaveMtAnaly.press_cal_stop <= gSlaveMtAnaly.press_to_adj ) )
    {
        OUT_LOG_SLAVE_BYTE('#');
        gSlaveMtAnaly.auto_cal_speed = 0;
        return;
    }    
    
    //松开过程，主动轮限速的计算 如果从动轮停止，不需要限制主电机速度了
    if( gPressFilter.val > gSlvMtCfg.press_mst_stop_max )				//需要停止
	{ 
        DoLimitMstMtSpeed( 0 , (OS_TICKS_PER_SEC/4)*gSlvMtCfg.onBridgeTime );
    }
    else
	{
        DoLimitMstMtSpeed( gSlvMtCfg.mst_limit_on_bridge , (OS_TICKS_PER_SEC/4)*gSlvMtCfg.onBridgeTime );
    }   
    //如果当前还需要继续松开，计算松开的实时速度
    SLV_AUTO_SPEED_DOWN();
}

/************************************************* 
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  正常运行过程中，从机更新过程
*   根据压力值，判断当前是太松，如果太松  --> 变紧  速度快
*   主动轮速度和从动轮速度对比，如果不同  --> 变紧  速度慢
*   压力值是否大于阈值，  --> 变松
*   压力值处于变化中，而且是过桥时的变化状态， --> 根据变化方式设置变松，或者变紧
*************************************************/
void zt_motor_slave_driver_update(void)
{
    //if( gSlaveMtAnaly.auto_cal_speed == 0 ) { //当前 电机没有动 需要检查两个限值
    //    //gSlaveMtAnaly.auto_cal_speed = 0;
    //    zt_slv_mt_determine_need_move();
    //}
    
    //压紧 压力变大过程 编码器值也变大
    if( (gSlaveMtAnaly.auto_cal_speed > 0) || (gSpeedAnaly_Slv.speed>20) )  
    {
        zt_slv_mt_determine_stop_when_up();
    }
    else if( (gSlaveMtAnaly.auto_cal_speed < 0) || (gSpeedAnaly_Slv.speed<-20) )
	{ //松开 压力减少过程
        zt_slv_mt_determine_stop_when_down();
    }
    else
	{ 
        zt_slv_mt_determine_need_move();
    }
}

/************************************************* 
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  压力滤波函数
*   计算每五次压力传感器ADC采集值的平均值 gPressFilter.val

*************************************************/

PressFilterCtrl gPressFilter;
static __inline void DoPressFilter(void)
{
    static u16 i;
    gPressFilter.cur_press_change++;
    if( gPressFilter.cur_press_change >= PRESS_FILTER_CNT_ONE )
	{ 
		gPressFilter.cur_press_change = 0;
	}

    //gPressFilter.press_sum_all -= gPressFilter.original_vals[gPressFilter.cur_press_change] ;
    //gPressFilter.original_vals[gPressFilter.cur_press_change] = get_adc_val(ADC_TYPE_ID_PRESS) >> 4 ;
    //gPressFilter.press_sum_all += gPressFilter.original_vals[gPressFilter.cur_press_change] ;

    gPressFilter.original_vals[gPressFilter.cur_press_change] = get_adc_val(ADC_TYPE_ID_PRESS1) >> 5 ;
    gPressFilter.press_sum_all = 0;
    for ( i=0 ; i<PRESS_FILTER_CNT_ONE ; i++ )
    {
        gPressFilter.press_sum_all += gPressFilter.original_vals[i];
    }

    gPressFilter.val = (gPressFilter.press_sum_all+(PRESS_FILTER_CNT_ONE/2)) / PRESS_FILTER_CNT_ONE;
}

//或者最近 new_cnt 个压力数据平均值
u8 GetPressNewFilter(u16 new_cnt)
{
    u16 i = gPressFilter.cur_press_change ;
    u16 sum_val = gPressFilter.original_vals[gPressFilter.cur_press_change];
    u16 sum_cnt = 1;
    while( sum_cnt < new_cnt )
    {
        i--;
        if( i >= PRESS_FILTER_CNT_ONE ) { i = 0; }
        sum_val += gPressFilter.original_vals[i];
        sum_cnt++;
    }
    return ( sum_val+(new_cnt/2)) / new_cnt ;
}


/************************************************* 
*Return:	void
*DESCRIPTION: 对输入的值进行滤波处理
*************************************************/
static __inline void DoPjzLvboAnaly(PjzLvboAnaly* plv,s16 newVal)
{
    plv->ValSumAll -= plv->ysVals[gMtDisControlCheck.curDealValID];
    plv->ysVals[gMtDisControlCheck.curDealValID] = newVal;
    plv->ValSumAll += newVal;
    plv->lvVal = plv->ValSumAll / PJZ_ANALY_MAX_CNT ;
}
/************************************************* 
*Return:	void
*DESCRIPTION:  电机控制速度，和采集速度滤波处理函数
*       用于检测电机是否失控。如果电机失控，则需要强制电机关闭0.4秒。
*************************************************/
void CheckMtDisControl(void)
{
    gMtDisControlCheck.curDealValID++;
    if( gMtDisControlCheck.curDealValID>=PJZ_ANALY_MAX_CNT )
	{
        gMtDisControlCheck.curDealValID = 0;
    }
    DoPjzLvboAnaly(&gMtDisControlCheck.mstMtBackSpeed,gSpeedAnaly_Mst.speed);
    DoPjzLvboAnaly(&gMtDisControlCheck.slvMtBackSpeed,gSpeedAnaly_Slv.speed);
    
    if( gSlaveMtAnaly.real_out_speed > 13 )
	{
        gMtDisControlCheck.slvOutCnt++;
        if( gMtDisControlCheck.slvOutCnt >= MT_DIS_CONTROL_CHECK_TICK )
		{
            gMtDisControlCheck.slvOutCnt = MT_DIS_CONTROL_CHECK_TICK-1 ;
            if( gMtDisControlCheck.slvMtBackSpeed.lvVal < 10 )
			{
                //uart_send_byte( DEBUG_PORT , 'S' );
                gMtDisControlCheck.slvNeedStopCnt = MT_DIS_CONTROL_CHECK_TICK/2;
          	}
        }
    }
    else if( gSlaveMtAnaly.real_out_speed < -13 )
	{
        gMtDisControlCheck.slvOutCnt--;
        if( gMtDisControlCheck.slvOutCnt <= -MT_DIS_CONTROL_CHECK_TICK )
		{
            gMtDisControlCheck.slvOutCnt = -(MT_DIS_CONTROL_CHECK_TICK-1) ;
            if( gMtDisControlCheck.slvMtBackSpeed.lvVal > -10 )
			{
                //uart_send_byte( DEBUG_PORT , 'V' );
                gMtDisControlCheck.slvNeedStopCnt = MT_DIS_CONTROL_CHECK_TICK/2;
            }
        }
    }
    else
	{
        gMtDisControlCheck.slvOutCnt = 0;
    }
    
    
    if( gMstMt.real_out_speed > 12 )
	{
        gMtDisControlCheck.mstOutCnt++;
        if( gMtDisControlCheck.mstOutCnt >= MT_DIS_CONTROL_CHECK_TICK )
		{
            gMtDisControlCheck.mstOutCnt = MT_DIS_CONTROL_CHECK_TICK-1 ;
            if( gMtDisControlCheck.mstMtBackSpeed.lvVal < 3 )
			{
                gMtDisControlCheck.mstNotMoveCnt++;
                if( gMtDisControlCheck.mstNotMoveCnt > 20 )
				{
                    //uart_send_byte( DEBUG_PORT , 'M' );
                    gMtDisControlCheck.mstNeedStopCnt = MT_DIS_CONTROL_CHECK_TICK/2;
                }
            }
            else
			{
                gMtDisControlCheck.mstNotMoveCnt=0;
            }
        }
    }
    else if( gMstMt.real_out_speed < -12 )
	{
        gMtDisControlCheck.mstOutCnt--;
        if( gMtDisControlCheck.mstOutCnt <= -MT_DIS_CONTROL_CHECK_TICK )
		{
            gMtDisControlCheck.mstOutCnt = -(MT_DIS_CONTROL_CHECK_TICK-1) ;
            if( gMtDisControlCheck.mstMtBackSpeed.lvVal > -3 )
			{
                gMtDisControlCheck.mstNotMoveCnt++;
                if( gMtDisControlCheck.mstNotMoveCnt > 20 )
				{
                    //uart_send_byte( DEBUG_PORT , 'W' );
                    gMtDisControlCheck.mstNeedStopCnt = MT_DIS_CONTROL_CHECK_TICK/2;
                }
            }
            else
			{
                gMtDisControlCheck.mstNotMoveCnt=0;
            }
        }
    }
    else
	{
        gMtDisControlCheck.mstOutCnt = 0;
        gMtDisControlCheck.mstNotMoveCnt = 0;
    }
}



/************************************************* 
*Return:	void
*DESCRIPTION:  从电机有一个分析出来的控制方向和速度
*       程序需要根据当前实际反馈的速度和方向，设定实时的
*       速度和方向
*       设定电机实时方向和速度后，需要处理分析电机是否失控；
*       是否失控判断方式为，控制信号的一个延时平均值；与速度值进行比较
*       如果速度值太大
*************************************************/
void DoSetSlaveMtSpeedByCurSpeed(void)
{
    //static s16  last_out_speed = 0;
    //gSpeedAnaly_Slv.speed
    //每次都进行一次速度设置，耗费CPU在接受范围内 不需要和之前速度进行对比
    //只需要对当前驱动器实际速度进行比较分析
    
    #define FAST_STOP_FX_SPEED_CUR_DOWN    0
    #define FAST_STOP_FX_SPEED_CUR_UP      0
    
    //如果需要电机停止，如果当前电机速度大于某个值，需要控制进行反转，反转速度不需要太大
    //能够加快电机停止
    if( gSlaveMtAnaly.need_out_speed > 10 )
	{
        //如果需要速度为 正，但是当前速度为负，需要先减少，到一定程度，才能反向大速度控制
        if( gSpeedAnaly_Slv.speed < -15 )
		{
            gSlaveMtAnaly.real_out_speed = FAST_STOP_FX_SPEED_CUR_DOWN;
            OUT_LOG_SLAVE_BYTE('%');
        }
        else
		{
            gSlaveMtAnaly.real_out_speed = gSlaveMtAnaly.need_out_speed;
        }
    }
    else if( gSlaveMtAnaly.need_out_speed < -10 )
	{
        if( gSpeedAnaly_Slv.speed > 15 )
		{
            gSlaveMtAnaly.real_out_speed = FAST_STOP_FX_SPEED_CUR_UP;
            OUT_LOG_SLAVE_BYTE('^');
        }
        else
		{
            gSlaveMtAnaly.real_out_speed = gSlaveMtAnaly.need_out_speed;
        }
    }
    else
	{
        if( gSpeedAnaly_Slv.speed > 15 )
		{
            gSlaveMtAnaly.real_out_speed = FAST_STOP_FX_SPEED_CUR_UP;
        }
        else if( gSpeedAnaly_Slv.speed < -15 )
		{
            gSlaveMtAnaly.real_out_speed = FAST_STOP_FX_SPEED_CUR_DOWN;
        }
        else
		{
            gSlaveMtAnaly.real_out_speed = 0;
        }
    }
    
    //从电机失控判断，失控需要等待一段时间再控制
    if( gMtDisControlCheck.slvNeedStopCnt > 0 )
	{
        gMtDisControlCheck.slvNeedStopCnt--;
        gSlaveMtAnaly.real_out_speed = 0;
        OUT_LOG_SLAVE_BYTE('\\');
    }
    
    //从电机实际控制输出
    if( gSlaveMtAnaly.real_out_speed > 5)
	{
        SET_SLAVE_MOTOR_ZZ() ;
        SET_SLAVE_MOTOR_PWM( gSlaveMtAnaly.real_out_speed );
    }
    else if( gSlaveMtAnaly.real_out_speed < -5)
	{
        SET_SLAVE_MOTOR_FZ() ;
        SET_SLAVE_MOTOR_PWM( -gSlaveMtAnaly.real_out_speed );
    }
    else
	{
        SET_SLAVE_MOTOR_CLOSE();
    }
}

void Sensor_Collect(void)
{
	//电机状态更新，不需要频率太高，降低些频率
	DoPressFilter();       			//压力传感器采集值滤波
    DoSpeedAnalyByCode();  			//各种需要进行的状态更新；速度分析更新
    UpdateFastBatChargingState();	//电池管理，充电状态更新
}

/************************************************* 
*Return:	void
*DESCRIPTION:  电机状态更新函数
*       

*************************************************/

void TaskFun_MotorUpdate(void)
{       
#ifndef MOTER_PERFORM_TEST_EN    //如果为正常控制流程 不需要进行相关分析处理

		//主动轮更新过程
        zt_motor_master_driver_update();
#else     
        
		CheckMtDisControl(); //电机是否失控检测
        
        if ( gSlaveMtAnaly.s_SlvMtState == SLAVE_MT_CTRL_AUTO )
        {   
            zt_motor_slave_driver_update();   //从动轮自动控制过程
            gSlaveMtAnaly.need_out_speed = gSlaveMtAnaly.auto_cal_speed;
            //gSlaveMtAnaly.need_out_speed = 0; //测试过程
        }
        else  //从动轮手动控制，设置速度
        {   
            gSlaveMtAnaly.need_out_speed = gSlaveMtAnaly.hand_set_speed;
            if( gSlaveMtAnaly.need_out_speed > 0 )
			{ 
                if( (gPressFilter.val >= 50) )		//向上运动，有一个压力值判断
				{
                    gSlaveMtAnaly.need_out_speed = 0;
                }
            }
        }
        
        if( gSlaveMtAnaly.need_out_speed < 0 )
		{ 
            if( gSlaveMtAnaly.b_XianWei_Down )		 //向下运动，需要判断限位开关
			{ 
                gSlaveMtAnaly.need_out_speed = 0;	 //如果碰到下面的限位开关，直接关闭电机
                //OUT_LOG_SLAVE_BYTE('&');
            }
        }
        
        DoSetSlaveMtSpeedByCurSpeed(); 				 //从电机控制
        
        zt_motor_master_driver_update();
#endif

}

/************************************************* 
*Return:	void
*DESCRIPTION:  电池管理，充电状态更新
*************************************************/
void UpdateFastBatChargingState(void)
{
    int curDisByCheck;
	
	gRbtState.bCX_SwCheck1 = !GpioGet(GPIO_CHK_CHAOXUE1) ;
    gRbtState.bCX_SwCheck2 = !GpioGet(GPIO_CHK_CHAOXUE2) ;
    gRbtState.bChargeVolIn = !GpioGet(GPIO_CHK_CHAG_JOIN ) ;
    //gRbtState.bChargeShort = !GpioGet(GPIO_CHK_CHAG_SHORT) ;
    
    ////1.进入到红外遮挡位置，编码器寄存器自动清零一次；
    ////2.进入到红外遮挡位置，运动的编码器距离大于桥距离一半，自动停止机器人一次；
    ////3.从红外遮挡处出到不遮挡位置，自动停止一次。用于工作人员记录桥遮挡间距
    ////4.遮挡范围内，距离桥距离编码器值一直保持为0；
    ////5.只有遮挡范围内，检测到充电输入，并进行充电有效，其它情况充电状态忽略
    ////6.充电条件：1）红外遮挡检测到；2）电压输入检测到；3）没有短路
    
    if( (gRbtState.bCX_SwCheck1) || (gRbtState.bCX_SwCheck2) ) 		//当前处于桥上遮挡处
	{
        if( !gBatAutoCtrl.bOnBridge )				//之前不处于桥上；进行了状态切换
		{ 
            gBatAutoCtrl.bOnBridge = true;
			
			//遇到红外遮挡时，清零前，发一条码盘信息给UART3，RF433 caigz 0421 add
			zt_build_send_state_string(BUILD_STATE_FLAG_SLVWH,ID_RF433,3);
			uart3_send(g_zt_msg.sendbuf , g_zt_msg.icmd_len );
			
            GET_SLAVE_WHEEL_CODE() = 0; //当前编码器值清零
            gBatAutoCtrl.nCxCheckMove = 0;
            gCodeZ=0;
			
			//遇到红外遮挡时，发一条信息给妙算UART2	zs 0306 add
            zt_build_send_state_string(BUILD_STATE_FLAG_ALL,ID_LINUX,8);
            uart2_send(g_zt_msg.sendbuf , g_zt_msg.icmd_len );
           
			//zt_motor_master_driver_set_speed( 0 , 50000 ); //需要停止主电机///新增20181221
        }
        
        if( 0 == gMstMt.set_speed )				//如果电机停止过程中
		{ 
            //GpioSetL(GPIO_LED_OUT_SHOW1);
			//OSTimeDly(500);
			//GpioSetH(GPIO_LED_OUT_SHOW1);
			if( gRbtState.bChargeVolIn )		//当前有电源输入，启动充电
			{ 
                gBatAutoCtrl.bCharging   = true;
            }
        }
        else									//运动过程中
		{
        	//GpioSetH(GPIO_LED_OUT_SHOW1);
            signed short curDis = (signed short)(GET_SLAVE_WHEEL_CODE());
            curDisByCheck = (curDis>=0)? curDis : -curDis ;
            if( gBatAutoCtrl.bChargingClosed )	//当前处于其它命令让充电关闭；则不做任何处理
			{ 
				gBatAutoCtrl.bCharging = false;
                //return;
            }
			else
			{
	            if( gBatAutoCtrl.bCharging )	//当前正处于充电中；不进行任何处理
				{ 
				
	            }
	            else
				{ //当前还未进行充电；需要让机器人再前进一点时间，然后停下来；并且开启充电
	                //if( gBatAutoCtrl.nCxCheckMove >= 17800 ) //当前接近开关有信号到充电位置距离
	                if( curDisByCheck >= gSlaveMtAnaly.curBridgeCodeHalf ) //100  20181221,应该改回此    
					//if( curDisByCheck >= 4000)//gSlaveMtAnaly.curBridgeCodeHalf
	                {   
	                	gBatAutoCtrl.bCharging       = true;
	                    gBatAutoCtrl.iChargingCircle = 0 ;
	                    zt_motor_master_driver_set_speed( 0 , 50000 ); //需要停止主电机
	                    //GpioSetL(GPIO_LED_OUT_SHOW1);
	                }
	            }
			}
        }
    }
    else								//当前离开桥上遮挡处
	{ 
        signed short curCode = (signed short)(GET_SLAVE_WHEEL_CODE());
        signed short curDis  ;
        int nCurDisScale ;
        if( gBatAutoCtrl.bOnBridge )
		{
            gBatAutoCtrl.bChargingClosed = false;
            gBatAutoCtrl.bOnBridge       = false;
			
			//离开桥时，发送码盘数据给UART3,RF433	caigz 0423 add
			zt_build_send_state_string(BUILD_STATE_FLAG_SLVWH,ID_RF433,3);
			uart3_send(g_zt_msg.sendbuf , g_zt_msg.icmd_len );
			
			zt_motor_master_driver_set_speed( 0 , 50000 ); //需要停止主电机一次
            BAT_CHARGE_OPEN();          //caigz 0421 add 离开桥时，使充电继电器闭合到常闭引脚
            gBatAutoCtrl.nCxCheckMove = 0; //离开桥的编码器值/比例系数
            //gCodeZ=0;
            gBatAutoCtrl.nLastSlvWhlCode = (signed short)(GET_SLAVE_WHEEL_CODE());
        }
        //需要计算 当前距离离开遮挡处的距离 / 比例系数
        //当前编码器值与运动方向值相反
        curDis  = curCode - gBatAutoCtrl.nLastSlvWhlCode ;
        
        curDis  = gBatAutoCtrl.nLastSlvWhlCode - curCode ;
        if( curDis>0 )
		{
            if( curDis > gSlaveMtAnaly.CodeScale )	//大于比例系数
			{ 
                nCurDisScale = curDis/gSlaveMtAnaly.CodeScale;
                gBatAutoCtrl.nCxCheckMove += nCurDisScale; 
                gBatAutoCtrl.nLastSlvWhlCode -= nCurDisScale*gSlaveMtAnaly.CodeScale;
            }
        }
        else
		{
            curDis = -curDis;
            if( curDis > gSlaveMtAnaly.CodeScale )	 //大于比例系数
			{
                nCurDisScale = curDis/gSlaveMtAnaly.CodeScale;
                gBatAutoCtrl.nCxCheckMove -= nCurDisScale; 
                gBatAutoCtrl.nLastSlvWhlCode += nCurDisScale*gSlaveMtAnaly.CodeScale;
            }
        }
    }
}
