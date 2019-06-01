//#include "box_os_user.h"
#include <includes.h> 
//////////////////////////////////////////////////////////////////////////
//����ٶ��������ٶ�   ���õ���ٶ�Ϊ10 �����ٶ�=0.5S->50=100/S  �����ٶ�=����ٶ�*10
//��������ı�����ϵ����ĳ�����ٶȣ��ٶ��£���Ŀ���ٶ�Ϊ0�����ٶ�Ϊ0ʱ��������Ϊ��
//??????
//////////////////////////////////////////////////////////////////////////

MotorDriverCtrlType gMstMt;

/************************************************* 
*Function:		zt_motor_master_driver_init
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  �����ֵ����ʼ��
*************************************************/
void zt_motor_master_driver_init(void)
{
    SET_MASTER_MOTOR_PWM(0);  //�����ǰ�ٶ�Ϊ 0

    //������ֵ ���г�ʼ��
    TimerCode_DefaultFunction_Init(2);  //������ ���ݲɼ���ʼ�� ������ TIM2
    memset( &gMstMt , 0 , sizeof(gMstMt) );
    gMstMt.limit_speed  = 101*100 ;
    SET_MASTER_MOTOR_CLOSE();
}

/************************************************* 
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  ���� �����ֵ�����ٶ�
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
    gMstMt.set_speed = speed*100 ;  //�ٶ���Ҫ���� 100
	
    //SetPwm_Tim8_CH3( speed );  //�����ǰ�ٶ�Ϊ 0
    //SET_MASTER_MOTOR_PWM(speed);
    return (bFanxiang)?(-speed):(speed);
}

#if 0
/************************************************* 
*Return:	void
*DESCRIPTION:  �������ٶ�ʵ�ʿ����ٶȷ��������¹���
*************************************************/
void zt_motor_master_driver_update(void)
{
    static u16 real_mst_mt_set_speed = 0;
    static u16 real_mst_mt_set_accl  = 0;
    static u16 real_mst_mt_set_del_accl  = 0;

    //�������õ��ٶ� �� ���٣�����ʵ����Ҫ�������ٶ� �� ���ٶȣ����ٶ�ֵ
    //Ȼ����µ��״̬
    if ( gMstMt.set_speed >= gMstMt.limit_speed )
    {
        real_mst_mt_set_speed       = gMstMt.limit_speed;
        real_mst_mt_set_accl        = 1 ;      //���ٶȼ���
        real_mst_mt_set_del_accl    = 220 ;    //���ٶȼӿ�
    }
    else
    {
        real_mst_mt_set_speed       = gMstMt.set_speed;
        real_mst_mt_set_accl        = gMstMt.n_accl ;    
        real_mst_mt_set_del_accl    = gMstMt.n_del_accl ;
    }

    //////���������й����� ��Ҫ�ж��Ƿ����쳣������ǣ���Ҫ��ͣһ��ʱ����ټ���
    if (  1300 < gMstMt.cur_speed  ) 
    {
        gMstMt.ys_running_time++;
        if( (gSpeedAnaly_Mst.speed<5) && (gSpeedAnaly_Mst.speed>-5) )
        {
            gMstMt.ys_stop_in_running_time++;
            if ( gMstMt.ys_stop_in_running_time > 800 ) //����һ��ʱ���� ��Ҫ��ֹͣ�£�������
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
    else  //�������Ѿ�ֹͣ
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
        else if ( real_mst_mt_set_speed > gMstMt.cur_speed )  //��Ҫ����
        {
            gMstMt.cur_speed += real_mst_mt_set_accl ;
            if( gMstMt.cur_speed > real_mst_mt_set_speed )
            {
                gMstMt.cur_speed = real_mst_mt_set_speed ;
            }
        }
        else  //real_mst_mt_set_speed < gMstMt.cur_speed  ��Ҫ���� �����ٵ����õ�ֵ
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
            SET_MT_BREAK_CLOSE;  //���Ʊ�բ�ر�
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
*DESCRIPTION:  ������ �ٶ�����ʱ����� �ٶ�����ֻ��Ҫ����һ��ʱ�䣬
*       ʱ�䵽�ˣ����ٽ����ٶ�����
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
*DESCRIPTION:  �������ٶ�ʵ�ʿ����ٶȷ��������¹���
*************************************************/
void zt_motor_master_driver_update(void)
{
    static u16 real_mst_mt_set_speed = 0;
    static u16 real_mst_mt_set_accl  = 0;
    static u16 real_mst_mt_set_del_accl  = 0;
    static bool bMtCheckStop = false; 

    //����ɼ���״ֵ̬���жϵ�ǰ���ʵ���Ƿ��Ѿ�ֹͣ
    if( ( ( gMstMt.cur_dir) && (gMtDisControlCheck.mstMtBackSpeed.lvVal>-4) )||
        ( (!gMstMt.cur_dir) && (gMtDisControlCheck.mstMtBackSpeed.lvVal< 4) ) )
    { //ʵ���ٶ�Ϊ0
        bMtCheckStop = true;
    }
    else{
        bMtCheckStop = false;
    }
    
    //�������õ��ٶ� �� ���٣�����ʵ����Ҫ�������ٶ� �� ���ٶȣ����ٶ�ֵ
    //Ȼ����µ��״̬
    DoLimitMstMtSpeed_Stop();
    if ( gMstMt.set_speed >= gMstMt.limit_speed )
    {   
        real_mst_mt_set_speed       = gMstMt.limit_speed;
        real_mst_mt_set_accl        = gSlvMtCfg.mstAddAccl/2 ;      //���ٶȼ���
        real_mst_mt_set_del_accl    = gSlvMtCfg.mstDelAccl*6 ;    //���ٶȼӿ�
        
        //�����ٶ�Ϊ0
        if( gMstMt.limit_speed == 0 ){
            bMtCheckStop = true;  //ǿ��ʶ��Ϊֹͣ״̬
        }
    }
    else
    {  
        real_mst_mt_set_speed       = gMstMt.set_speed;
        real_mst_mt_set_accl        = gSlvMtCfg.mstAddAccl ;    
        real_mst_mt_set_del_accl    = gSlvMtCfg.mstDelAccl ;
    }
    
    //�����ǰ��⵽�����ʧ�أ���Ҫǿ�ƹر������һ��ʱ��
    if( gMtDisControlCheck.mstNeedStopCnt > 0 ){
        gMtDisControlCheck.mstNeedStopCnt--;
        real_mst_mt_set_speed    = 0 ;
        real_mst_mt_set_del_accl = gSlvMtCfg.mstDelAccl*100 ;    //���ٶȼӿ�
    }
    
    

    //�趨�ٶȷ��� �뵱ǰʵ����� �ٶȺͷ���ͬ����Ҫ�����л�����
    //���жϿ��Ʒ��� �� ��ǰ����˶������Ƿ���ͬ
    if( gMstMt.set_dir != gMstMt.cur_dir )  //��Ҫ�л�����Ĵ������
    {   
        if( bMtCheckStop ){ //ʵ���ٶ�Ϊ0
            gMstMt.cur_speed = 11*100;
            gMstMt.cur_dir = gMstMt.set_dir;
        }
        else{ //�����ǰ��һ���趨�ٶȣ����ռ��ٶ�ֵ���м���
            if( gMstMt.cur_speed > real_mst_mt_set_del_accl ){
                gMstMt.cur_speed -= real_mst_mt_set_del_accl;
            }
            else{
                gMstMt.cur_speed = 0;
            }
        }
    }
    else if( real_mst_mt_set_speed != gMstMt.cur_speed ){ //�ٶ��б仯
		if ( real_mst_mt_set_speed > gMstMt.cur_speed )   //��Ҫ����
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
        else  //real_mst_mt_set_speed < gMstMt.cur_speed  ��Ҫ���� �����ٵ����õ�ֵ
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
        if ( gMstMt.limit_speed_time )  //�����ϣ�����ֹͣ
        {
			SET_MT_BREAK_CLOSE ;
        }
        else{ //�������ϣ��ȴ�ֹͣ
            SET_MT_BREAK_OPEN ;
        }
    }
    else{
        SET_MT_BREAK_CLOSE ;
    }
    
    real_mst_mt_set_speed = gMstMt.cur_speed/100;
    SET_MASTER_MOTOR_PWM( real_mst_mt_set_speed );  //�ٶ�ֵ
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
*Return:		ʵ�����õ� ���ٶ�
*DESCRIPTION:  ���� �����ֵ���ļ��ٶ�  ���ٶ�ֵΪ���Ƶ���ٶȱ仯ʱ��
*           ��������θ���ʱ���仯���ٶ��� �ٶ�ֵΪ 0.00��100.00 
*           Ϊ�˿��� �ͼ��㾫�� ���ٶ�ֵ ��λΪ0.01
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
