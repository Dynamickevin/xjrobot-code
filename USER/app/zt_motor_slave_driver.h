//////////////////////////////////////////////////////////////////////////
//zt_motor_slave_driver.h 
//从动轮 电机控制
//底层驱动级别 提供电机初始化，速度控制，位置控制接口
//驱动内部有一个自动的速度控制更新函数，该函数在每次tick时被调用一次
//////////////////////////////////////////////////////////////////////////
#ifndef _ZT_MOTOR_SLAVE_DRIVER_H_
#define _ZT_MOTOR_SLAVE_DRIVER_H_

#define SLV_DIR_TO_PRESS_UP    0XFF   //速度为- 为松 +为紧
#define SLV_DIR_TO_PRESS_CLOSE  0
#define SLV_DIR_TO_PRESS_DOWN   1
#define SLV_DIR_TO_DIF_DELAY_DEFAULT  100
//#define SLV_AUTO_SPEED_UP    80
//#define SLV_AUTO_SPEED_DOWN  -80

enum
{
    SLAVE_MT_STATE_HOLD = 0 ,       //从机不动
    SLAVE_MT_STATE_MST_MT_BLOCK  ,  //主机堵转
    SLAVE_MT_STATE_SLV_MT_BLOCK  ,  //从机堵转
    SLAVE_MT_STATE_DOWN_BRIDGE  ,   // 下桥
    SLAVE_MT_STATE_UP_BRIDGE  ,     // 上桥
    SLAVE_MT_STATE_PRESS_TOO_LOW  ,  //压力太小
    SLAVE_MT_STATE_PRESS_TOO_BIG  ,  //压力太大

    SLAVE_MT_CTRL_AUTO = 0 ,  //自动控制
    SLAVE_MT_CTRL_HANDLE ,    //手动控制
};


/************************************************* 
*Function:		zt_motor_slave_driver_init
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  从动轮电机初始化
*************************************************/
void zt_motor_slave_driver_init(void);

/*************************************************
*Input:			
*OUTPUT:		void
*Return:		实际设置的速度
*DESCRIPTION:  设置 从动轮电机的速度
*************************************************/
s16 zt_motor_slave_driver_set_speed(s16 speed,u16 code_run);


#define GET_MASTER_MOTOR_CODE()  GetTimerCodeVal(TIM3)
#define GET_SLAVE_MOTOR_CODE()  GetTimerCodeVal(TIM2)
#define GET_SLAVE_WHEEL_CODE()  GetTimerCodeVal(TIM4)

//抱闸 使能管脚 PD1
#define SET_MT_BREAK_OPEN      GpioSetL(GPIO_BREAK_MEN)
#define SET_MT_BREAK_CLOSE     GpioSetH(GPIO_BREAK_MEN)

//驱动器设置为 低电平使能 高电平关闭
//->BSRR to set io=1  ->BRR to set io=0
#define SET_MASTER_MOTOR_CLOSE() GpioSetH(GPIO_CTL_DIR_M1) ; GpioSetH(GPIO_CTL_DIR_M2)
#define SET_SLAVE_MOTOR_CLOSE()  GpioSetH(GPIO_CTL_DIR_S1) ; GpioSetH(GPIO_CTL_DIR_S2) ; SET_SLAVE_MOTOR_PWM(0)

#define SET_MASTER_MOTOR_ZZ()   GpioSetH(GPIO_CTL_DIR_M1) ; GpioSetL(GPIO_CTL_DIR_M2)
#define SET_MASTER_MOTOR_FZ()   GpioSetL(GPIO_CTL_DIR_M1) ; GpioSetH(GPIO_CTL_DIR_M2)
#define SET_SLAVE_MOTOR_ZZ()    GpioSetH(GPIO_CTL_DIR_S2) ; GpioSetL(GPIO_CTL_DIR_S1)
#define SET_SLAVE_MOTOR_FZ()    GpioSetL(GPIO_CTL_DIR_S2) ; GpioSetH(GPIO_CTL_DIR_S1)

#define SET_MASTER_MOTOR_PWM  SetPwm_Tim8_CH4
#define SET_SLAVE_MOTOR_PWM   SetPwm_Tim8_CH3
//#define SET_SLAVE_MOTOR_PWM(...)


//////////////////////////////////////////////////////////////////////////
//主动轮 从动轮 从动轮电机 的编码器到速度的分析
//速度分析分为很多档次 有最当前的临时速度，有更多时间的长期速度；
//需要的话，可以进行多个时间点速度分析
//一个是0.1S的速度 -- 临时速度
//一个是5S  的速度 -- 平均速度  如果有需要，会进行多个速度获取和计算
#define SPEED_ANALY_CODE_DIVIDER  50   //每多次数据 进行一次数据记录 并通过数据记录分析速度
#define SPEED_ANALY_CODE_MAX_CNT  10   //记录数据 计算速度 时间最大次数
typedef struct
{
    u16   original_codes[SPEED_ANALY_CODE_MAX_CNT];
    u16   original_oldest_code;
    s16   speed;       //最后得到的速度  临时速度
    s16   speed_hold;  //长时间的速度 平均速度，保持速度 
}SpeedAnalyByCode;

extern SpeedAnalyByCode gSpeedAnaly_Mst;    //MST  master 缩写
extern SpeedAnalyByCode gSpeedAnaly_Slv;    //从动轮控制电机 编码器
extern SpeedAnalyByCode gSpeedAnaly_Whl;    //从动轮 编码器  Whl -- 缩写

//(1000/OS_TICKS_PER_SEC)  系统tick的时间长度 ms
#define TIME_MS_TO_CODE_SAVE_TICK(ms_time)   ( (ms_time)/( (1000/OS_TICKS_PER_SEC)*SPEED_ANALY_CODE_DIVIDER ) )
s16 GetCode_DiffFromSaveList(SpeedAnalyByCode* code_list,u16 save_tick);

//////////////////////////////////////////////////////////////////////////
//从动轮 上桥 或者 下桥 过云绞丝
//从动轮 电机编码器分析
typedef struct
{
    int CodeScale;             //码盘换算比例系数
    int curBridgeCodeHalf;     //桥宽度码盘值的一半
	//u16   hand_ctrl_last_code;
    //u16   hand_ctrl_left_code; //手动控制 还剩下的编码器数据量
    
    //从电机控制，需要状态有： 手动控制还是自动控制；手动控制设置速度，自动控制计算速度
    //根据不同状态的一个目标速度；已经当前控制速度；和当前电机反馈速度
    //分析一个电机实际 控制速度
    s16   hand_set_speed;  //手动控制，设置速度
    s16   auto_cal_speed;  //自动控制，计算速度
    s16   need_out_speed;  //分析需要输出速度
    
    s16   real_out_speed;  //根据 need_out_speed 及电机速度分析实际输出速度    
    s16   last_out_speed;  //之前输出的速度
    
    //压力控制相关
    s16   press_to_adj;    //调整压力的目标值
    s16   press_cal_stop;  //计算出当前状态停止时，压力变化值
    
    //s16   cur_set_speed; //当前设置的速度
    u8    last_set_dir;    //上一次设置的方向
    u8    curr_set_dir;    //当前设置的方向
    u16   move_times;      //连续运动的时间
    s16   real_time_speed;  //实时速度，最近阶段速度
    u16   in_blocking;      //当前处于堵转状态---进行堵转的处理
    u16   s_SlvMtState;     //从机控制状态
    u16   s_SlvMtState_last;  //从机控制状态
    u16   n_SlvMtStateTimeHold; //同一个状态保持时间
    u16   n_HoldTime_PressLow ; //压力很小的 持续时间
    u16   n_HoldTime_PressBig ; //压力很大的 持续时间
    u16   n_HoldTime_CtrlSlvDown ; //压力很大的 已经控制电机运行的时间 持续时间
    u8    b_XianWei_Up;
    u8    b_XianWei_Down;
    //在到达底部限位开关位置了，上桥过程的处理过程 
    //需要在不得已的时候控制位置，或者就不能控制从动轮下降了
    //在限位开关启动后， 在桥上行走过程，需要单独控制压力参数
    s16   n_more_slv_code_running; //惯性原因 从电机还会运行的量
    s16   n_more_press_running;    //惯性原因 还会运行的量
    float mst_speed_limit;
    float mst_speed_limit_det_pr;
        
    u16   real_slv_up_press_to_adg;  //上升过程中，目标压力值；如果当前是出于未压紧，打滑状态，目标压力值增加些
    u16   test_out_debug;
    u16   real_slv_down_press_to_adg; 

    //2017-03-27 Add By BG.ltd to let slave motor turn direction need some delate
    u8  ys_to_set_dir;
    u8  ys_to_set_speed;
    u8  ys_cur_real_dir;
    u8  ys_cur_real_speed;
    u8  ys_last_ctrl_dir;
    u16  ys_need_close_delay;  		//when to close slave motor,need delay some ticks
    u16  ys_running_time; 			//有刷电机 已经启动电机时间 用于时候异常不运动的判断
}SlaveMotorAnaly;
extern SlaveMotorAnaly gSlaveMtAnaly;



//////////////////////////////////////////////////////////////////////////
//从电机控制的 配置参数结构体
typedef struct
{
    u16 press_ok_min         ;    //正常运行过程中，压力的最小值
    u16 press_ok_max         ;    //正常运动过程中，压力的最大值
    u16 press_ok_to_adjust   ;    //需要进行调整时，目标压力值

    //限速相关
    u16 press_mst_stop_min   ;    //压力太小，需要停止主动轮 停止主动轮压力最小值
    u16 press_mst_stop_max   ;    //压力太大，需要停止主动轮 停止主动轮压力最大值 
    
    //主动轮过桥过程的参数
    u16 mstAddAccl;               //主电机加速度值
    u16 mstDelAccl;               //主电机减速度值
    u16 onBridgeTime ;            //在桥上行走的时间 单位秒
    u16 mst_limit_on_bridge ;     //在桥上的限速

    
    //电池允许充电以及充电满电压值 *10倍
    u16 bat_auto_charge;		 //电池需要自动充电电量
    u16 bat_charge_full;		 //电池充满电量
    u16 bat_no_move;             //不能移动的电量值
    
    float fk_mst_limit_up   ;   //从动轮 上行时 限速的系数
    float fk_mst_limit_down ;   //从动轮 下行时 限速的系数
    
    //下面的参数需要在 计算压力与从电机速度控制关系的参数，压力与目标压力越大，速度越大
    //从电机速度 = k*(压力-press_ok_to_adjust) + 最低速度
    float fk_slv_speed_by_press_up; //上升过程压力与速度系数
    float fk_slv_speed_by_press_down; //下降过程压力与速度系数
}SlvMtCfgType;
extern SlvMtCfgType gSlvMtCfg;

typedef struct
{
    u32 begin_cfg;   //0XAAAAAAAA
    SlvMtCfgType SlvMtCfg;
    u32 end_cfg;   //0X55555555
}UserCfgType;

/************************************************* 
*Function:		zt_motor_slave_driver_init
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  从动轮电机 配置参数初始化
*************************************************/
void zt_motor_slave_driver_cfg_init(void);


/************************************************* 
*Return:		
*DESCRIPTION:  电机运动过程中，充电检测及处理；充电状态更新
*   未充电-->检测都充电输入---电压低-->停止并进行充电--->检测实际电压，需要关闭充电
*                           -电压高-->继续运动
*   如果因为充电关闭电机，启动需要手动发送命令，发送运动命令，表示自动关闭充电
*************************************************/

#define PRESS_FILTER_CNT_ONE  5
typedef struct 
{
    u8   original_vals[PRESS_FILTER_CNT_ONE];
    u8   cur_press_change;  //当前需要修改的数据位置
    u8   val;               //滤波后的值
    u16  press_sum_all;     //上面多个数据的 总和
}PressFilterCtrl;
extern PressFilterCtrl gPressFilter;
u8 GetPressNewFilter(u16 new_cnt);  //或者最近 new_cnt 个压力数据平均值


/////////////////////////////////////////////////////////////////////////////////////////
/////从电机 速度控制相关函数
/************************************************* 
*Return:	void
*DESCRIPTION:  从电机有一个分析出来的控制方向和速度
*       程序需要根据当前实际反馈的速度和方向，设定实时的
*       速度和方向
*************************************************/
void DoSetSlaveMtSpeedByCurSpeed(void);


#define PJZ_ANALY_MAX_CNT  14           //记录数据 计算速度 时间最大次数
#define MT_DIS_CONTROL_CHECK_TICK  60   //0.5秒/8ms = 
typedef struct
{
    s16   ysVals[PJZ_ANALY_MAX_CNT];
    s16   lvVal;
    s32   ValSumAll; 					//所有数据和
}PjzLvboAnaly;
typedef struct
{
    //PjzLvboAnaly mstMtOut;
    PjzLvboAnaly mstMtBackSpeed;
    //PjzLvboAnaly slvMtOut;
    PjzLvboAnaly slvMtBackSpeed;
    s16 mstOutCnt;  				//主电机有驱动输出时间次数
    s16 slvOutCnt; 					//从电机有驱动输出时间次数
    s16 mstNeedStopCnt;
    s16 slvNeedStopCnt;
    u16 mstNotMoveCnt;
    u16 curDealValID; 				//当前更新的数据位置
}MtDisControlCheck;
extern MtDisControlCheck gMtDisControlCheck;
/************************************************* 
*Return:	void
*DESCRIPTION:  电机控制速度，和采集速度滤波处理函数
*       用于检测电机是否失控。如果电机失控，则需要强制电机关闭0.4秒。
*************************************************/
void CheckMtDisControl(void);

//下面所有的电池电压值，都是 真实值*10
typedef struct{
    s16  nLastSlvWhlCode;
    s32  nCxCheckMove; 		//检测到巢穴开关信号，运动距离积分
    //iChargingCircle用于在充电过程中的电压采集，必须断开充电接入
    u16  iChargingCircle; 	//充电循环值  充电5分钟--->断开充电2秒---->检测实际电压值--->充电5分钟
    u16  iChargingInCnt;
    u16  curBatVol;  		//当前电压值
    bool bChargingClosed;   //充电关闭过程
    bool bCharging; 		//当前是否在充电中
    bool bOnBridge; 		//正处于桥上范围；当离开时，不处于桥上。只有在桥上时，运动控制才判断电压是否低，不运动
}BatAutoCtrl;
extern BatAutoCtrl gBatAutoCtrl;
#define BAT_CHARGE_OPEN()    GpioSetBit(GPIO_BAT_CHARGE_EN,1)
#define BAT_CHARGE_CLOSE()   GpioSetBit(GPIO_BAT_CHARGE_EN,0)
/************************************************* 
*Return:	void
*DESCRIPTION:  电池管理，充电状态更新
*************************************************/
void UpdateFastBatChargingState(void);
void Sensor_Collect(void);
void TaskFun_MotorUpdate(void);
void  zt_motor_slave_driver_update(void);

#endif
