//#include "box_os_user.h"
#include <includes.h> 
//////////////////////////////////////////////////////////////////////////
//电机速度与脉冲速度   设置电机速度为10 脉冲速度=0.5S->50=100/S  脉冲速度=电机速度*10
//按照上面的比例关系，在某个加速度，速度下，若目标速度为0，到速度为0时，脉冲数为：
//??????
//////////////////////////////////////////////////////////////////////////

MotorDriverCtrlType gMstMt;

/************************************************* 
*Function:		zt_motor_master_driver_init
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  主动轮电机初始化
*************************************************/
void zt_motor_master_driver_init(void)
{
    SET_MASTER_MOTOR_PWM(0);  //电机当前速度为 0

    //编码器值 进行初始化
    TimerCode_DefaultFunction_Init(2);  //编码器 数据采集初始化 主动轮 TIM2
    memset( &gMstMt , 0 , sizeof(gMstMt) );
    gMstMt.limit_speed  = 101*100 ;
    SET_MASTER_MOTOR_CLOSE();
}

/************************************************* 
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  设置 主动轮电机的速度
*************************************************/
s16 zt_motor_master_driver_set_speed(s16 speed,u16 code_run)
{
    u8 bFanxiang = 0;
    if ( speed < 0 )
    {
        speed = -speed;
        bFanxiang = 1;
    }
    else{
        ;
    }
    if ( speed >= 85 )
    {
        speed = 85;
    }
    else if ( speed<12 )
    {
        speed = 0;
    }
    gMstMt.left_code = code_run ;

    gMstMt.set_dir   = bFanxiang ;
    gMstMt.set_speed = speed*100 ;  //速度需要乘以 100
	
    //SetPwm_Tim8_CH3( speed );  //电机当前速度为 0
    //SET_MASTER_MOTOR_PWM(speed);
    return (bFanxiang)?(-speed):(speed);
}

#if 0
/************************************************* 
*Return:	void
*DESCRIPTION:  主动轮速度实际控制速度分析及更新过程
*************************************************/
void zt_motor_master_driver_update(void)
{
    static u16 real_mst_mt_set_speed = 0;
    static u16 real_mst_mt_set_accl  = 0;
    static u16 real_mst_mt_set_del_accl  = 0;

    //根据设置的速度 和 限速，分析实际需要的设置速度 和 加速度，减速度值
    //然后更新电机状态
    if ( gMstMt.set_speed >= gMstMt.limit_speed )
    {
        real_mst_mt_set_speed       = gMstMt.limit_speed;
        real_mst_mt_set_accl        = 1 ;      //加速度减慢
        real_mst_mt_set_del_accl    = 220 ;    //减速度加快
    }
    else
    {
        real_mst_mt_set_speed       = gMstMt.set_speed;
        real_mst_mt_set_accl        = gMstMt.n_accl ;    
        real_mst_mt_set_del_accl    = gMstMt.n_del_accl ;
    }

    //////主动轮运行过程中 需要判断是否卡死异常，如果是，需要暂停一段时间后再继续
    if (  1300 < gMstMt.cur_speed  ) 
    {
        gMstMt.ys_running_time++;
        if( (gSpeedAnaly_Mst.speed<5) && (gSpeedAnaly_Mst.speed>-5) )
        {
            gMstMt.ys_stop_in_running_time++;
            if ( gMstMt.ys_stop_in_running_time > 800 ) //卡死一段时间了 需要先停止下，在启动
            {
                gMstMt.ys_stop_in_running_time = 0;
                gMstMt.ys_need_stop_time_err = 100;
                real_mst_mt_set_speed = 0;
                gMstMt.cur_speed = 0;
                gMstMt.cur_dir = gMstMt.set_dir;
                SET_MASTER_MOTOR_CLOSE();
                SET_MT_BREAK_CLOSE;
            }
        }
        else{
            if(gMstMt.ys_stop_in_running_time){
                gMstMt.ys_stop_in_running_time--;
            }
        }
    }
    else if ( gMstMt.ys_need_stop_time_err )
    {
        gMstMt.ys_need_stop_time_err--;
        real_mst_mt_set_speed = 0;
        gMstMt.cur_speed = 0;
        gMstMt.cur_dir = gMstMt.set_dir;
    }
    else  //主动轮已经停止
    {
        gMstMt.ys_running_time = 0;
        gMstMt.ys_stop_in_running_time = 0;
    }


    if ( (real_mst_mt_set_speed!=gMstMt.cur_speed) || (gMstMt.set_dir!=gMstMt.cur_dir) )
    {
        if ( gMstMt.set_dir!=gMstMt.cur_dir )
        {
            if ( gMstMt.cur_speed > real_mst_mt_set_del_accl )
            {
                gMstMt.cur_speed -= real_mst_mt_set_del_accl ;
            }
            else
            {
                gMstMt.cur_speed = 0 ;
                gMstMt.cur_dir = gMstMt.set_dir ;
            }
        }
        else if ( real_mst_mt_set_speed > gMstMt.cur_speed )  //需要加速
        {
            gMstMt.cur_speed += real_mst_mt_set_accl ;
            if( gMstMt.cur_speed > real_mst_mt_set_speed )
            {
                gMstMt.cur_speed = real_mst_mt_set_speed ;
            }
        }
        else  //real_mst_mt_set_speed < gMstMt.cur_speed  需要减速 最多减速到设置的值
        {
            if ( (real_mst_mt_set_speed+real_mst_mt_set_del_accl) >= gMstMt.cur_speed )
            {
                gMstMt.cur_speed = real_mst_mt_set_speed ;
            }
            else{
                gMstMt.cur_speed -= real_mst_mt_set_del_accl ;
            }
        }
        
#if 1 //ifdef SET_MASTER_MOTOR_CLOSE
        if ( gMstMt.cur_speed <= 10 )
        {
            SET_MASTER_MOTOR_CLOSE();
            SET_MT_BREAK_CLOSE;  //控制抱闸关闭
        }
        if( gMstMt.cur_speed < 1100 )
        {
            SET_MASTER_MOTOR_CLOSE();
            //SET_MT_BREAK_OPEN;
        }
        else if ( gMstMt.cur_dir )
        {
            SET_MASTER_MOTOR_FZ() ;
            SET_MT_BREAK_OPEN;
        } 
        else
        {
            SET_MASTER_MOTOR_ZZ() ;
            SET_MT_BREAK_OPEN;
        }
#endif
        SET_MASTER_MOTOR_PWM( gMstMt.cur_speed/100 ); 
    }
}

#else  //2017-11-29

/************************************************* 
*Return:	void
*DESCRIPTION:  主动轮 速度限制时间更新 速度限制只需要持续一定时间，
*       时间到了，不再进行速度限制
*************************************************/
static void __inline DoLimitMstMtSpeed_Stop(void) //,u16 time_hold
{
    if ( gMstMt.limit_speed_time )
    {
        gMstMt.limit_speed_time--;
    }
    else{
        gMstMt.limit_speed = 101*100;
    }
}


/************************************************* 
*Return:	void
*DESCRIPTION:  主动轮速度实际控制速度分析及更新过程
*************************************************/
void zt_motor_master_driver_update(void)
{
    static u16 real_mst_mt_set_speed = 0;
    static u16 real_mst_mt_set_accl  = 0;
    static u16 real_mst_mt_set_del_accl  = 0;
    static bool bMtCheckStop = false; 

    //电机采集的状态值，判断当前电机实际是否已经停止
    if( ( ( gMstMt.cur_dir) && (gMtDisControlCheck.mstMtBackSpeed.lvVal>-4) )||
        ( (!gMstMt.cur_dir) && (gMtDisControlCheck.mstMtBackSpeed.lvVal< 4) ) )
    { //实际速度为0
        bMtCheckStop = true;
    }
    else{
        bMtCheckStop = false;
    }
    
    //根据设置的速度 和 限速，分析实际需要的设置速度 和 加速度，减速度值
    //然后更新电机状态
    DoLimitMstMtSpeed_Stop();
    if ( gMstMt.set_speed >= gMstMt.limit_speed )
    {   
        real_mst_mt_set_speed       = gMstMt.limit_speed;
        real_mst_mt_set_accl        = gSlvMtCfg.mstAddAccl/2 ;      //加速度减慢
        real_mst_mt_set_del_accl    = gSlvMtCfg.mstDelAccl*6 ;    //减速度加快
        
        //限制速度为0
        if( gMstMt.limit_speed == 0 ){
            bMtCheckStop = true;  //强制识别为停止状态
        }
    }
    else
    {  
        real_mst_mt_set_speed       = gMstMt.set_speed;
        real_mst_mt_set_accl        = gSlvMtCfg.mstAddAccl ;    
        real_mst_mt_set_del_accl    = gSlvMtCfg.mstDelAccl ;
    }
    
    //如果当前检测到主电机失控，需要强制关闭主电机一段时间
    if( gMtDisControlCheck.mstNeedStopCnt > 0 ){
        gMtDisControlCheck.mstNeedStopCnt--;
        real_mst_mt_set_speed    = 0 ;
        real_mst_mt_set_del_accl = gSlvMtCfg.mstDelAccl*100 ;    //减速度加快
    }
    
    

    //设定速度方向 与当前实际输出 速度和方向不同；需要进行切换处理
    //先判断控制方向 与 当前电机运动方向是否相同
    if( gMstMt.set_dir != gMstMt.cur_dir )  //需要切换方向的处理过程
    {   
        if( bMtCheckStop ){ //实际速度为0
            gMstMt.cur_speed = 11*100;
            gMstMt.cur_dir = gMstMt.set_dir;
        }
        else{ //如果当前有一个设定速度，按照加速度值进行减速
            if( gMstMt.cur_speed > real_mst_mt_set_del_accl ){
                gMstMt.cur_speed -= real_mst_mt_set_del_accl;
            }
            else{
                gMstMt.cur_speed = 0;
            }
        }
    }
    else if( real_mst_mt_set_speed != gMstMt.cur_speed ){ //速度有变化
		if ( real_mst_mt_set_speed > gMstMt.cur_speed )   //需要加速
        {
            gMstMt.cur_speed += real_mst_mt_set_accl ;
            if( gMstMt.cur_speed > real_mst_mt_set_speed )
            {
                gMstMt.cur_speed = real_mst_mt_set_speed ;
            }
            if( gMstMt.cur_speed < 11*100 ){
                gMstMt.cur_speed = 11*100 ;
            }
        }
        else  //real_mst_mt_set_speed < gMstMt.cur_speed  需要减速 最多减速到设置的值
        {
            if ( (real_mst_mt_set_speed+real_mst_mt_set_del_accl) >= gMstMt.cur_speed )
            {
                gMstMt.cur_speed = real_mst_mt_set_speed ;
            }
            else{
                gMstMt.cur_speed -= real_mst_mt_set_del_accl ;
            }
        }
    }
    
    if( (gMstMt.cur_speed > (10*100-1)) ){
        SET_MT_BREAK_OPEN ;
    }
    else if( (!bMtCheckStop)  ){
        if ( gMstMt.limit_speed_time )  //在桥上，立即停止
        {
			SET_MT_BREAK_CLOSE ;
        }
        else{ //不在桥上，等待停止
            SET_MT_BREAK_OPEN ;
        }
    }
    else{
        SET_MT_BREAK_CLOSE ;
    }
    
    real_mst_mt_set_speed = gMstMt.cur_speed/100;
    SET_MASTER_MOTOR_PWM( real_mst_mt_set_speed );  //速度值
    gMstMt.real_out_speed = (gMstMt.cur_dir)? (-real_mst_mt_set_speed):(real_mst_mt_set_speed) ;
    
    if( gMstMt.cur_speed < 9*100){
        SET_MASTER_MOTOR_CLOSE();
    }
    else if ( gMstMt.cur_dir )
    {
        SET_MASTER_MOTOR_FZ() ;
    } 
    else
    {
        SET_MASTER_MOTOR_ZZ() ;
    }
}
#endif

/************************************************* 
*Function:		zt_motor_master_driver_set_accl
*Input:			
*OUTPUT:		void
*Return:		实际设置的 加速度
*DESCRIPTION:  设置 主动轮电机的加速度  加速度值为控制电机速度变化时，
*           电机在两次更新时，变化的速度量 速度值为 0.00～100.00 
*           为了控制 和计算精度 加速度值 单位为0.01
*************************************************/
u16 zt_motor_master_driver_set_accl(u16 accl)
{
    if ( accl < 1 )
    {
        accl = 1;
    }
    else if ( accl > 12000 )
    {
        accl = 12000 ;
    }
    gMstMt.n_accl = accl;
    return accl;
}
