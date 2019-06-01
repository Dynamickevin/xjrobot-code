//////////////////////////////////////////////////////////////////////////
//zt_motor_master_driver.h 
//主动轮 电机控制
//底层驱动级别 提供电机初始化，速度控制，位置控制接口
//驱动内部有一个自动的速度控制更新函数，该函数在每次tick时被调用一次
//////////////////////////////////////////////////////////////////////////
#ifndef _ZT_MOTOR_MASTER_DRIVER_H_
#define _ZT_MOTOR_MASTER_DRIVER_H_

/************************************************* 
*Function:		zt_motor_master_driver_init
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  主动轮电机初始化
*************************************************/
void zt_motor_master_driver_init(void);

/*************************************************
*Input:			
*OUTPUT:		void
*Return:		实际设置的速度
*DESCRIPTION:  设置 主动轮电机的速度
*************************************************/
s16 zt_motor_master_driver_set_speed(s16 speed,u16 code_run);

/************************************************* 
*Function:		zt_motor_master_driver_set_accl
*Input:			
*OUTPUT:		void
*Return:		实际设置的 加速度
*DESCRIPTION:  设置 主动轮电机的加速度  加速度值为控制电机速度变化时，
*           电机在两次更新时，变化的速度量 速度值为 0.00～100.00 
*           为了控制 和计算精度 加速度值 单位为0.01
*************************************************/
u16 zt_motor_master_driver_set_accl(u16 accl);


//////////////////////////////////////////////////////////////////////////
//单位换算 需要将各种控制参数 从 float 转换为 int 类型
//电机速度，编码器值 等内部控制都是 int类型

typedef struct
{
    u8      cur_dir;    //当前的方向  电机的方向0 表示正传  其他都是反转
    u8      set_dir;    //设置的方向
    u16     n_accl;     //当前的加速度
    u16     n_del_accl;     //当前的加速度
    u16     set_speed;    //设置的速度 目标速度
    u16     cur_speed;    //当前的速度 其值为 速度的100倍 并增加些尾数
    u16     left_code;    //控制电机运动 剩下的脉冲数 当达到这些脉冲数时，需要电机停止
    
    s16   real_out_speed; 

    //limit_speed 不同的情况下 限速不同 但压力太小 或者 太大时 限速要多些
    //如果不需要进行限速，需要设置限速值为 101*100
    u16     limit_speed ;  
    u16     limit_speed_time ; //限速持续时间

    u16     ys_running_time;  //2017-04-06 有刷电机，用于检测主动轮是否异常卡死；卡死需要停止电机一段时间
    u16     ys_stop_in_running_time; //运行过程中 停止的时间长度
    u16     ys_need_stop_time_err; //有刷电机异常时，需要休息的时间
}MotorDriverCtrlType;

extern MotorDriverCtrlType gMstMt;

/************************************************* 
*Return:	void
*DESCRIPTION:  主动轮速度实际控制速度分析及更新过程
*************************************************/
void zt_motor_master_driver_update(void);

#endif
