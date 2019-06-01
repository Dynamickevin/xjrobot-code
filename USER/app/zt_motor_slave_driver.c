//#include "box_os_user.h"
#include <includes.h>

extern ZT_INFO_TYPE g_zt_msg;
SpeedAnalyByCode gSpeedAnaly_Mst;    //MST  master ��д
SpeedAnalyByCode gSpeedAnaly_Slv;    //�Ӷ��ֿ��Ƶ�� ������
SpeedAnalyByCode gSpeedAnaly_Whl;    //�Ӷ��� ����wheel,  Whl -- ��д
SlvMtCfgType     gSlvMtCfg;  		 //���ò���
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
*DESCRIPTION:  �Ӷ��ֵ�� ���ò�����ʼ��
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
        gSlvMtCfg.bat_no_move     = 220;			//�����˲����ƶ�ʱ�ĵ�ѹ
        gSlvMtCfg.bat_auto_charge = 225;			//��������Ҫ���ĵ�ѹ
        gSlvMtCfg.bat_charge_full = 245;			//�����˳���ʱ�ĵ�ѹ
        gSlvMtCfg.onBridgeTime = 6; 				//���������ж�ʱ��Ϊ6��
        gSlvMtCfg.mstAddAccl = 10;
        gSlvMtCfg.mstDelAccl = 40;
        //AT+SlvCFG=6 pmin=40 pmax=110 pmid=75 StopMin=20 StopMax=150 SpeedBrg=10
        gSlvMtCfg.press_ok_min         = 40;  //30;  //40;  //   �������й����У�ѹ������Сֵ
        gSlvMtCfg.press_ok_max         = 70;  //70;  //100; //   �����˶������У�ѹ�������ֵ
        gSlvMtCfg.press_mst_stop_min   = 10;  //10;  //20;  //   ѹ��̫С����Ҫֹͣ������ ֹͣ������ѹ����Сֵ
        gSlvMtCfg.press_mst_stop_max   = 120; //120; //150; //   ѹ��̫����Ҫֹͣ������ ֹͣ������ѹ�����ֵ
        gSlvMtCfg.mst_limit_on_bridge  = 16;  //10;  //10;  //   �����ϵ����� //50;  //70;  //   ��Ҫ���е���ʱ��Ŀ��ѹ��ֵ
    }
    
    //gSlvMtCfg.press_ok_to_adjust   = (gSlvMtCfg.press_ok_min+gSlvMtCfg.press_ok_max)>>1; 
    gSlvMtCfg.press_ok_to_adjust   = gSlvMtCfg.press_ok_max - 
        ((gSlvMtCfg.press_ok_max-gSlvMtCfg.press_ok_min)/3) ; 

    //�Ӷ��� ����ʱ ���ٵ�ϵ��       		k*(press_ok_to_adjust-press_mst_stop_min)^2 = 100
    gSlvMtCfg.fk_mst_limit_up =  gSlvMtCfg.press_ok_to_adjust - gSlvMtCfg.press_mst_stop_min; 
    gSlvMtCfg.fk_mst_limit_up = gSlvMtCfg.fk_mst_limit_up*gSlvMtCfg.fk_mst_limit_up ;
    gSlvMtCfg.fk_mst_limit_up = 100.0f / gSlvMtCfg.fk_mst_limit_up ;

	//�Ӷ��� ����ʱ ���ٵ�ϵ��		        k*(press_mst_stop_max-press_ok_to_adjust)^2 = 100
    gSlvMtCfg.fk_mst_limit_down = gSlvMtCfg.press_mst_stop_max - gSlvMtCfg.press_ok_to_adjust  ;   
    gSlvMtCfg.fk_mst_limit_down = gSlvMtCfg.fk_mst_limit_down*gSlvMtCfg.fk_mst_limit_down ;
    gSlvMtCfg.fk_mst_limit_down = 100.0f / gSlvMtCfg.fk_mst_limit_down ;
    
    
    //���Թ�ϵ��ϵ��
    //gSlvMtCfg.fk_slv_speed_by_press_up  =(80.0f-20.0f)/(gSlvMtCfg.press_ok_to_adjust-gSlvMtCfg.press_mst_stop_min); //��������ѹ�����ٶ�ϵ��
    //gSlvMtCfg.fk_slv_speed_by_press_down=(70.0f-20.0f)/(gSlvMtCfg.press_ok_to_adjust-gSlvMtCfg.press_mst_stop_max); //�½�����ѹ�����ٶ�ϵ��
    //���ι�ϵ ϵ�� ��Ŀ��ѹ�������ٶ�Ϊ0
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
*DESCRIPTION:  �Ӷ��ֵ��  		   ��ʼ�� 
*************************************************/
void zt_motor_slave_driver_init(void)
{
    //PA12 �Ӷ��� ���������� 
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
    
    
    //������ֵ ���г�ʼ��
    TimerCode_DefaultFunction_Init(3);  //������ ���ݲɼ���ʼ�� �Ӷ��� TIM3

    memset( &gBatAutoCtrl    , 0 , sizeof(gBatAutoCtrl    ) );
    memset( &gSpeedAnaly_Mst , 0 , sizeof(SpeedAnalyByCode) );
    memset( &gSpeedAnaly_Slv , 0 , sizeof(SpeedAnalyByCode) );
    memset( &gSpeedAnaly_Whl , 0 , sizeof(SpeedAnalyByCode) );
    memset( &gSlaveMtAnaly   , 0 , sizeof(gSlaveMtAnaly   ) );
    memset( &gMtDisControlCheck , 0 , sizeof(gMtDisControlCheck) );
    gSlaveMtAnaly.test_out_debug = 0;

    //���� ������Ϣ�ĳ�ʼ��
    zt_motor_slave_driver_cfg_init();
    
    SET_MASTER_MOTOR_CLOSE();
    SET_SLAVE_MOTOR_CLOSE() ;

	//�Ӷ��������йز���
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
*DESCRIPTION:  ���� �Ӷ��ֵ�����ٶ�
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
        //real_set_slv_mt_speed(bFanxiang,speed); //  SetPwm_Tim8_CH4(  );  //�����ǰ�ٶ�Ϊ  speed
    }

    return gSlaveMtAnaly.hand_set_speed;
}


/************************************************* 
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  �������������ٶȷ�������
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

    //�ӵ�� �Ŀ��� ��Ҫ���п�����Ӧ�Ŀ��� ��ʱ��Ҫ ��ȡ��ǰ 20ms �ĵ���ٶ�
    //����20ms �ĵ���ٶ� ����������ֹͣ���������ߵľ��� �Լ�ѹ���仯ֵ
    //��һ�����������  �������Ҫ��ȡ 20ms�ٶ�
    // gSpeedAnaly_Slv.speed > 0; is up running
    gSpeedAnaly_Slv.speed = ( 1614 - get_adc_val(ADC_TYPE_ID_SW_SP) ) / (13) ;
    gSpeedAnaly_Mst.speed = ( 1614 - get_adc_val(ADC_TYPE_ID_MW_SP) ) / (-10) ;

    //����Ҫʵʱ�ɼ��Ӷ��ֵı�����ֵ ÿ��һ��ʱ��ɼ�һ�� ��βɼ�������ƽ���ٶ�
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


//��ȡ ���µı�����ֵ��ĳһ��ʱ����ڵı������仯��
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

//��ѹ������ ���� ѹ����Сʱ �����������ٶ�
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
    //gSlvMtCfg.fk_slv_speed_by_press_up  =(80-20)/(gSlvMtCfg.press_ok_to_adjust-gSlvMtCfg.press_mst_stop_min); //��������ѹ�����ٶ�ϵ��
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
//�Ӷ���û���˶����жϴӶ����Ƿ���Ҫ����
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
        //��Ҫ�ɿ��Ӷ���
        gSlaveMtAnaly.real_slv_down_press_to_adg = gSlvMtCfg.press_ok_to_adjust ;
        SLV_AUTO_SPEED_DOWN();
    }
    else  //��ǰѹ����������Χ��
    {
        gSlaveMtAnaly.auto_cal_speed = 0;
    }
}


static float slv_mt_stop_time = 0;
#define MIN_PRESS_CAL_DET_TO_STOP   25
/************************************************* 
*Return:	void
*DESCRIPTION:  �����������
*       ���ݵ�ǰѹ��ֵ���ӵ���ٶ�ֵ���ж��������ֹͣ
*       ѹ���仯Ŀ��ֵ
*************************************************/
static __inline void slv_mt_cal_stop_press_when_up(void)
{
    //��ͬѹ���£�ֹͣʱ�䲻ͬ gPressFilter.val
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
    gSlaveMtAnaly.press_cal_stop = slv_mt_stop_time * 1.5f ;//ԭʼֵΪ 5
    gSlaveMtAnaly.press_cal_stop += gPressFilter.val;
}
/************************************************* 
*Return:	void
*DESCRIPTION:  ����½�����
*       ���ݵ�ǰѹ��ֵ���ӵ���ٶ�ֵ���ж��������ֹͣ
*       ѹ���仯Ŀ��ֵ
*************************************************/
static __inline void slv_mt_cal_stop_press_when_down(void)
{
    //��ͬѹ���£�ֹͣʱ�䲻ͬ gPressFilter.val
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
    gSlaveMtAnaly.press_cal_stop = slv_mt_stop_time * -1.5f ; //ϵ��ԭʼΪ-2.5f
    gSlaveMtAnaly.press_cal_stop += gPressFilter.val;
    if( gSlaveMtAnaly.press_cal_stop < 0 )
	{
        gSlaveMtAnaly.press_cal_stop = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
//�Ӷ��� ѹ�� �ж��Ƿ���Ҫ�رմӶ���
static __inline void zt_slv_mt_determine_stop_when_up(void)
{
    //�����ǰѹ���Ѿ�������Ŀ��ѹ���ˣ�ֹͣ���
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
    //    DoLimitMstMtSpeed( gSlvMtCfg.mst_limit_on_bridge );  //�ٶ�
    //}
    //else
    //{
    //    DoLimitMstMtSpeed( 0 );
    //}

    //Ԥ��ر�ʱ ѹ��ֵ ���ѹ��������Ҫ���ڵ�ѹ��
    slv_mt_cal_stop_press_when_up();
    if (( gPressFilter.val>gSlvMtCfg.press_mst_stop_min ) && 
        ( gSlaveMtAnaly.press_cal_stop >= gSlaveMtAnaly.real_slv_up_press_to_adg ) )
        //( gSlaveMtAnaly.press_cal_stop >= gSlaveMtAnaly.press_to_adj ) ) //gSlvMtCfg.press_ok_max
    {
        OUT_LOG_SLAVE_BYTE('@');
        gSlaveMtAnaly.auto_cal_speed = 0;
        return;
    }
    
    
    //ѹ�����̣����������ٵļ��� ����Ӷ���ֹͣ������Ҫ����������ٶ���
    if( gPressFilter.val < gSlvMtCfg.press_mst_stop_min )            //��Ҫֹͣ
	{ 
        //if( gMstMt.limit_speed > gSlvMtCfg.mst_limit_on_bridge )
        DoLimitMstMtSpeed( 0 , (OS_TICKS_PER_SEC/4)*gSlvMtCfg.onBridgeTime/2 );
    }
    else
	{
        DoLimitMstMtSpeed( gSlvMtCfg.mst_limit_on_bridge-2 , (OS_TICKS_PER_SEC/4)*gSlvMtCfg.onBridgeTime/2 );
    }
    
    //�����ǰ����Ҫ����ѹ��������ѹ����ʵʱ�ٶ�
    SLV_AUTO_SPEED_UP();
}

//////////////////////////////////////////////////////////////////////////
//�Ӷ��� �ɿ� �ж��Ƿ���Ҫ �����ɿ�
static __inline void zt_slv_mt_determine_stop_when_down(void)
{
    if ( gPressFilter.val <= gSlaveMtAnaly.press_to_adj )
    {
        gSlaveMtAnaly.auto_cal_speed = 0;
        return;
    }

    //���������ٵļ���  ����ѹ����������
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

    //Ԥ��ر�ʱ ѹ��ֵ ���ѹ������
    slv_mt_cal_stop_press_when_down();
    if (( gPressFilter.val<gSlvMtCfg.press_mst_stop_max ) && 
        ( gSlaveMtAnaly.press_cal_stop <= gSlaveMtAnaly.press_to_adj ) )
    {
        OUT_LOG_SLAVE_BYTE('#');
        gSlaveMtAnaly.auto_cal_speed = 0;
        return;
    }    
    
    //�ɿ����̣����������ٵļ��� ����Ӷ���ֹͣ������Ҫ����������ٶ���
    if( gPressFilter.val > gSlvMtCfg.press_mst_stop_max )				//��Ҫֹͣ
	{ 
        DoLimitMstMtSpeed( 0 , (OS_TICKS_PER_SEC/4)*gSlvMtCfg.onBridgeTime );
    }
    else
	{
        DoLimitMstMtSpeed( gSlvMtCfg.mst_limit_on_bridge , (OS_TICKS_PER_SEC/4)*gSlvMtCfg.onBridgeTime );
    }   
    //�����ǰ����Ҫ�����ɿ��������ɿ���ʵʱ�ٶ�
    SLV_AUTO_SPEED_DOWN();
}

/************************************************* 
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  �������й����У��ӻ����¹���
*   ����ѹ��ֵ���жϵ�ǰ��̫�ɣ����̫��  --> ���  �ٶȿ�
*   �������ٶȺʹӶ����ٶȶԱȣ������ͬ  --> ���  �ٶ���
*   ѹ��ֵ�Ƿ������ֵ��  --> ����
*   ѹ��ֵ���ڱ仯�У������ǹ���ʱ�ı仯״̬�� --> ���ݱ仯��ʽ���ñ��ɣ����߱��
*************************************************/
void zt_motor_slave_driver_update(void)
{
    //if( gSlaveMtAnaly.auto_cal_speed == 0 ) { //��ǰ ���û�ж� ��Ҫ���������ֵ
    //    //gSlaveMtAnaly.auto_cal_speed = 0;
    //    zt_slv_mt_determine_need_move();
    //}
    
    //ѹ�� ѹ�������� ������ֵҲ���
    if( (gSlaveMtAnaly.auto_cal_speed > 0) || (gSpeedAnaly_Slv.speed>20) )  
    {
        zt_slv_mt_determine_stop_when_up();
    }
    else if( (gSlaveMtAnaly.auto_cal_speed < 0) || (gSpeedAnaly_Slv.speed<-20) )
	{ //�ɿ� ѹ�����ٹ���
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
*DESCRIPTION:  ѹ���˲�����
*   ����ÿ���ѹ��������ADC�ɼ�ֵ��ƽ��ֵ gPressFilter.val

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

//������� new_cnt ��ѹ������ƽ��ֵ
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
*DESCRIPTION: �������ֵ�����˲�����
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
*DESCRIPTION:  ��������ٶȣ��Ͳɼ��ٶ��˲�������
*       ���ڼ�����Ƿ�ʧ�ء�������ʧ�أ�����Ҫǿ�Ƶ���ر�0.4�롣
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
*DESCRIPTION:  �ӵ����һ�����������Ŀ��Ʒ�����ٶ�
*       ������Ҫ���ݵ�ǰʵ�ʷ������ٶȺͷ����趨ʵʱ��
*       �ٶȺͷ���
*       �趨���ʵʱ������ٶȺ���Ҫ�����������Ƿ�ʧ�أ�
*       �Ƿ�ʧ���жϷ�ʽΪ�������źŵ�һ����ʱƽ��ֵ�����ٶ�ֵ���бȽ�
*       ����ٶ�ֵ̫��
*************************************************/
void DoSetSlaveMtSpeedByCurSpeed(void)
{
    //static s16  last_out_speed = 0;
    //gSpeedAnaly_Slv.speed
    //ÿ�ζ�����һ���ٶ����ã��ķ�CPU�ڽ��ܷ�Χ�� ����Ҫ��֮ǰ�ٶȽ��жԱ�
    //ֻ��Ҫ�Ե�ǰ������ʵ���ٶȽ��бȽϷ���
    
    #define FAST_STOP_FX_SPEED_CUR_DOWN    0
    #define FAST_STOP_FX_SPEED_CUR_UP      0
    
    //�����Ҫ���ֹͣ�������ǰ����ٶȴ���ĳ��ֵ����Ҫ���ƽ��з�ת����ת�ٶȲ���Ҫ̫��
    //�ܹ��ӿ���ֹͣ
    if( gSlaveMtAnaly.need_out_speed > 10 )
	{
        //�����Ҫ�ٶ�Ϊ �������ǵ�ǰ�ٶ�Ϊ������Ҫ�ȼ��٣���һ���̶ȣ����ܷ�����ٶȿ���
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
    
    //�ӵ��ʧ���жϣ�ʧ����Ҫ�ȴ�һ��ʱ���ٿ���
    if( gMtDisControlCheck.slvNeedStopCnt > 0 )
	{
        gMtDisControlCheck.slvNeedStopCnt--;
        gSlaveMtAnaly.real_out_speed = 0;
        OUT_LOG_SLAVE_BYTE('\\');
    }
    
    //�ӵ��ʵ�ʿ������
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
	//���״̬���£�����ҪƵ��̫�ߣ�����ЩƵ��
	DoPressFilter();       			//ѹ���������ɼ�ֵ�˲�
    DoSpeedAnalyByCode();  			//������Ҫ���е�״̬���£��ٶȷ�������
    UpdateFastBatChargingState();	//��ع������״̬����
}

/************************************************* 
*Return:	void
*DESCRIPTION:  ���״̬���º���
*       

*************************************************/

void TaskFun_MotorUpdate(void)
{       
#ifndef MOTER_PERFORM_TEST_EN    //���Ϊ������������ ����Ҫ������ط�������

		//�����ָ��¹���
        zt_motor_master_driver_update();
#else     
        
		CheckMtDisControl(); //����Ƿ�ʧ�ؼ��
        
        if ( gSlaveMtAnaly.s_SlvMtState == SLAVE_MT_CTRL_AUTO )
        {   
            zt_motor_slave_driver_update();   //�Ӷ����Զ����ƹ���
            gSlaveMtAnaly.need_out_speed = gSlaveMtAnaly.auto_cal_speed;
            //gSlaveMtAnaly.need_out_speed = 0; //���Թ���
        }
        else  //�Ӷ����ֶ����ƣ������ٶ�
        {   
            gSlaveMtAnaly.need_out_speed = gSlaveMtAnaly.hand_set_speed;
            if( gSlaveMtAnaly.need_out_speed > 0 )
			{ 
                if( (gPressFilter.val >= 50) )		//�����˶�����һ��ѹ��ֵ�ж�
				{
                    gSlaveMtAnaly.need_out_speed = 0;
                }
            }
        }
        
        if( gSlaveMtAnaly.need_out_speed < 0 )
		{ 
            if( gSlaveMtAnaly.b_XianWei_Down )		 //�����˶�����Ҫ�ж���λ����
			{ 
                gSlaveMtAnaly.need_out_speed = 0;	 //��������������λ���أ�ֱ�ӹرյ��
                //OUT_LOG_SLAVE_BYTE('&');
            }
        }
        
        DoSetSlaveMtSpeedByCurSpeed(); 				 //�ӵ������
        
        zt_motor_master_driver_update();
#endif

}

/************************************************* 
*Return:	void
*DESCRIPTION:  ��ع������״̬����
*************************************************/
void UpdateFastBatChargingState(void)
{
    int curDisByCheck;
	
	gRbtState.bCX_SwCheck1 = !GpioGet(GPIO_CHK_CHAOXUE1) ;
    gRbtState.bCX_SwCheck2 = !GpioGet(GPIO_CHK_CHAOXUE2) ;
    gRbtState.bChargeVolIn = !GpioGet(GPIO_CHK_CHAG_JOIN ) ;
    //gRbtState.bChargeShort = !GpioGet(GPIO_CHK_CHAG_SHORT) ;
    
    ////1.���뵽�����ڵ�λ�ã��������Ĵ����Զ�����һ�Σ�
    ////2.���뵽�����ڵ�λ�ã��˶��ı�������������ž���һ�룬�Զ�ֹͣ������һ�Σ�
    ////3.�Ӻ����ڵ����������ڵ�λ�ã��Զ�ֹͣһ�Ρ����ڹ�����Ա��¼���ڵ����
    ////4.�ڵ���Χ�ڣ������ž��������ֵһֱ����Ϊ0��
    ////5.ֻ���ڵ���Χ�ڣ���⵽������룬�����г����Ч������������״̬����
    ////6.���������1�������ڵ���⵽��2����ѹ�����⵽��3��û�ж�·
    
    if( (gRbtState.bCX_SwCheck1) || (gRbtState.bCX_SwCheck2) ) 		//��ǰ���������ڵ���
	{
        if( !gBatAutoCtrl.bOnBridge )				//֮ǰ���������ϣ�������״̬�л�
		{ 
            gBatAutoCtrl.bOnBridge = true;
			
			//���������ڵ�ʱ������ǰ����һ��������Ϣ��UART3��RF433 caigz 0421 add
			zt_build_send_state_string(BUILD_STATE_FLAG_SLVWH,ID_RF433,3);
			uart3_send(g_zt_msg.sendbuf , g_zt_msg.icmd_len );
			
            GET_SLAVE_WHEEL_CODE() = 0; //��ǰ������ֵ����
            gBatAutoCtrl.nCxCheckMove = 0;
            gCodeZ=0;
			
			//���������ڵ�ʱ����һ����Ϣ������UART2	zs 0306 add
            zt_build_send_state_string(BUILD_STATE_FLAG_ALL,ID_LINUX,8);
            uart2_send(g_zt_msg.sendbuf , g_zt_msg.icmd_len );
           
			//zt_motor_master_driver_set_speed( 0 , 50000 ); //��Ҫֹͣ�����///����20181221
        }
        
        if( 0 == gMstMt.set_speed )				//������ֹͣ������
		{ 
            //GpioSetL(GPIO_LED_OUT_SHOW1);
			//OSTimeDly(500);
			//GpioSetH(GPIO_LED_OUT_SHOW1);
			if( gRbtState.bChargeVolIn )		//��ǰ�е�Դ���룬�������
			{ 
                gBatAutoCtrl.bCharging   = true;
            }
        }
        else									//�˶�������
		{
        	//GpioSetH(GPIO_LED_OUT_SHOW1);
            signed short curDis = (signed short)(GET_SLAVE_WHEEL_CODE());
            curDisByCheck = (curDis>=0)? curDis : -curDis ;
            if( gBatAutoCtrl.bChargingClosed )	//��ǰ�������������ó��رգ������κδ���
			{ 
				gBatAutoCtrl.bCharging = false;
                //return;
            }
			else
			{
	            if( gBatAutoCtrl.bCharging )	//��ǰ�����ڳ���У��������κδ���
				{ 
				
	            }
	            else
				{ //��ǰ��δ���г�磻��Ҫ�û�������ǰ��һ��ʱ�䣬Ȼ��ͣ���������ҿ������
	                //if( gBatAutoCtrl.nCxCheckMove >= 17800 ) //��ǰ�ӽ��������źŵ����λ�þ���
	                if( curDisByCheck >= gSlaveMtAnaly.curBridgeCodeHalf ) //100  20181221,Ӧ�øĻش�    
					//if( curDisByCheck >= 4000)//gSlaveMtAnaly.curBridgeCodeHalf
	                {   
	                	gBatAutoCtrl.bCharging       = true;
	                    gBatAutoCtrl.iChargingCircle = 0 ;
	                    zt_motor_master_driver_set_speed( 0 , 50000 ); //��Ҫֹͣ�����
	                    //GpioSetL(GPIO_LED_OUT_SHOW1);
	                }
	            }
			}
        }
    }
    else								//��ǰ�뿪�����ڵ���
	{ 
        signed short curCode = (signed short)(GET_SLAVE_WHEEL_CODE());
        signed short curDis  ;
        int nCurDisScale ;
        if( gBatAutoCtrl.bOnBridge )
		{
            gBatAutoCtrl.bChargingClosed = false;
            gBatAutoCtrl.bOnBridge       = false;
			
			//�뿪��ʱ�������������ݸ�UART3,RF433	caigz 0423 add
			zt_build_send_state_string(BUILD_STATE_FLAG_SLVWH,ID_RF433,3);
			uart3_send(g_zt_msg.sendbuf , g_zt_msg.icmd_len );
			
			zt_motor_master_driver_set_speed( 0 , 50000 ); //��Ҫֹͣ�����һ��
            BAT_CHARGE_OPEN();          //caigz 0421 add �뿪��ʱ��ʹ���̵����պϵ���������
            gBatAutoCtrl.nCxCheckMove = 0; //�뿪�ŵı�����ֵ/����ϵ��
            //gCodeZ=0;
            gBatAutoCtrl.nLastSlvWhlCode = (signed short)(GET_SLAVE_WHEEL_CODE());
        }
        //��Ҫ���� ��ǰ�����뿪�ڵ����ľ��� / ����ϵ��
        //��ǰ������ֵ���˶�����ֵ�෴
        curDis  = curCode - gBatAutoCtrl.nLastSlvWhlCode ;
        
        curDis  = gBatAutoCtrl.nLastSlvWhlCode - curCode ;
        if( curDis>0 )
		{
            if( curDis > gSlaveMtAnaly.CodeScale )	//���ڱ���ϵ��
			{ 
                nCurDisScale = curDis/gSlaveMtAnaly.CodeScale;
                gBatAutoCtrl.nCxCheckMove += nCurDisScale; 
                gBatAutoCtrl.nLastSlvWhlCode -= nCurDisScale*gSlaveMtAnaly.CodeScale;
            }
        }
        else
		{
            curDis = -curDis;
            if( curDis > gSlaveMtAnaly.CodeScale )	 //���ڱ���ϵ��
			{
                nCurDisScale = curDis/gSlaveMtAnaly.CodeScale;
                gBatAutoCtrl.nCxCheckMove -= nCurDisScale; 
                gBatAutoCtrl.nLastSlvWhlCode += nCurDisScale*gSlaveMtAnaly.CodeScale;
            }
        }
    }
}
