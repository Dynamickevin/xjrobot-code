//////////////////////////////////////////////////////////////////////////
//zt_motor_slave_driver.h 
//�Ӷ��� �������
//�ײ��������� �ṩ�����ʼ�����ٶȿ��ƣ�λ�ÿ��ƽӿ�
//�����ڲ���һ���Զ����ٶȿ��Ƹ��º������ú�����ÿ��tickʱ������һ��
//////////////////////////////////////////////////////////////////////////
#ifndef _ZT_MOTOR_SLAVE_DRIVER_H_
#define _ZT_MOTOR_SLAVE_DRIVER_H_

#define SLV_DIR_TO_PRESS_UP    0XFF   //�ٶ�Ϊ- Ϊ�� +Ϊ��
#define SLV_DIR_TO_PRESS_CLOSE  0
#define SLV_DIR_TO_PRESS_DOWN   1
#define SLV_DIR_TO_DIF_DELAY_DEFAULT  100
//#define SLV_AUTO_SPEED_UP    80
//#define SLV_AUTO_SPEED_DOWN  -80

enum
{
    SLAVE_MT_STATE_HOLD = 0 ,       //�ӻ�����
    SLAVE_MT_STATE_MST_MT_BLOCK  ,  //������ת
    SLAVE_MT_STATE_SLV_MT_BLOCK  ,  //�ӻ���ת
    SLAVE_MT_STATE_DOWN_BRIDGE  ,   // ����
    SLAVE_MT_STATE_UP_BRIDGE  ,     // ����
    SLAVE_MT_STATE_PRESS_TOO_LOW  ,  //ѹ��̫С
    SLAVE_MT_STATE_PRESS_TOO_BIG  ,  //ѹ��̫��

    SLAVE_MT_CTRL_AUTO = 0 ,  //�Զ�����
    SLAVE_MT_CTRL_HANDLE ,    //�ֶ�����
};


/************************************************* 
*Function:		zt_motor_slave_driver_init
*Input:			
*OUTPUT:		void
*Return:		
*DESCRIPTION:  �Ӷ��ֵ����ʼ��
*************************************************/
void zt_motor_slave_driver_init(void);

/*************************************************
*Input:			
*OUTPUT:		void
*Return:		ʵ�����õ��ٶ�
*DESCRIPTION:  ���� �Ӷ��ֵ�����ٶ�
*************************************************/
s16 zt_motor_slave_driver_set_speed(s16 speed,u16 code_run);


#define GET_MASTER_MOTOR_CODE()  GetTimerCodeVal(TIM3)
#define GET_SLAVE_MOTOR_CODE()  GetTimerCodeVal(TIM2)
#define GET_SLAVE_WHEEL_CODE()  GetTimerCodeVal(TIM4)

//��բ ʹ�ܹܽ� PD1
#define SET_MT_BREAK_OPEN      GpioSetL(GPIO_BREAK_MEN)
#define SET_MT_BREAK_CLOSE     GpioSetH(GPIO_BREAK_MEN)

//����������Ϊ �͵�ƽʹ�� �ߵ�ƽ�ر�
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
//������ �Ӷ��� �Ӷ��ֵ�� �ı��������ٶȵķ���
//�ٶȷ�����Ϊ�ܶ൵�� ���ǰ����ʱ�ٶȣ��и���ʱ��ĳ����ٶȣ�
//��Ҫ�Ļ������Խ��ж��ʱ����ٶȷ���
//һ����0.1S���ٶ� -- ��ʱ�ٶ�
//һ����5S  ���ٶ� -- ƽ���ٶ�  �������Ҫ������ж���ٶȻ�ȡ�ͼ���
#define SPEED_ANALY_CODE_DIVIDER  50   //ÿ������� ����һ�����ݼ�¼ ��ͨ�����ݼ�¼�����ٶ�
#define SPEED_ANALY_CODE_MAX_CNT  10   //��¼���� �����ٶ� ʱ��������
typedef struct
{
    u16   original_codes[SPEED_ANALY_CODE_MAX_CNT];
    u16   original_oldest_code;
    s16   speed;       //���õ����ٶ�  ��ʱ�ٶ�
    s16   speed_hold;  //��ʱ����ٶ� ƽ���ٶȣ������ٶ� 
}SpeedAnalyByCode;

extern SpeedAnalyByCode gSpeedAnaly_Mst;    //MST  master ��д
extern SpeedAnalyByCode gSpeedAnaly_Slv;    //�Ӷ��ֿ��Ƶ�� ������
extern SpeedAnalyByCode gSpeedAnaly_Whl;    //�Ӷ��� ������  Whl -- ��д

//(1000/OS_TICKS_PER_SEC)  ϵͳtick��ʱ�䳤�� ms
#define TIME_MS_TO_CODE_SAVE_TICK(ms_time)   ( (ms_time)/( (1000/OS_TICKS_PER_SEC)*SPEED_ANALY_CODE_DIVIDER ) )
s16 GetCode_DiffFromSaveList(SpeedAnalyByCode* code_list,u16 save_tick);

//////////////////////////////////////////////////////////////////////////
//�Ӷ��� ���� ���� ���� ���ƽ�˿
//�Ӷ��� �������������
typedef struct
{
    int CodeScale;             //���̻������ϵ��
    int curBridgeCodeHalf;     //�ſ������ֵ��һ��
	//u16   hand_ctrl_last_code;
    //u16   hand_ctrl_left_code; //�ֶ����� ��ʣ�µı�����������
    
    //�ӵ�����ƣ���Ҫ״̬�У� �ֶ����ƻ����Զ����ƣ��ֶ����������ٶȣ��Զ����Ƽ����ٶ�
    //���ݲ�ͬ״̬��һ��Ŀ���ٶȣ��Ѿ���ǰ�����ٶȣ��͵�ǰ��������ٶ�
    //����һ�����ʵ�� �����ٶ�
    s16   hand_set_speed;  //�ֶ����ƣ������ٶ�
    s16   auto_cal_speed;  //�Զ����ƣ������ٶ�
    s16   need_out_speed;  //������Ҫ����ٶ�
    
    s16   real_out_speed;  //���� need_out_speed ������ٶȷ���ʵ������ٶ�    
    s16   last_out_speed;  //֮ǰ������ٶ�
    
    //ѹ���������
    s16   press_to_adj;    //����ѹ����Ŀ��ֵ
    s16   press_cal_stop;  //�������ǰ״ֹ̬ͣʱ��ѹ���仯ֵ
    
    //s16   cur_set_speed; //��ǰ���õ��ٶ�
    u8    last_set_dir;    //��һ�����õķ���
    u8    curr_set_dir;    //��ǰ���õķ���
    u16   move_times;      //�����˶���ʱ��
    s16   real_time_speed;  //ʵʱ�ٶȣ�����׶��ٶ�
    u16   in_blocking;      //��ǰ���ڶ�ת״̬---���ж�ת�Ĵ���
    u16   s_SlvMtState;     //�ӻ�����״̬
    u16   s_SlvMtState_last;  //�ӻ�����״̬
    u16   n_SlvMtStateTimeHold; //ͬһ��״̬����ʱ��
    u16   n_HoldTime_PressLow ; //ѹ����С�� ����ʱ��
    u16   n_HoldTime_PressBig ; //ѹ���ܴ�� ����ʱ��
    u16   n_HoldTime_CtrlSlvDown ; //ѹ���ܴ�� �Ѿ����Ƶ�����е�ʱ�� ����ʱ��
    u8    b_XianWei_Up;
    u8    b_XianWei_Down;
    //�ڵ���ײ���λ����λ���ˣ����Ź��̵Ĵ������ 
    //��Ҫ�ڲ����ѵ�ʱ�����λ�ã����߾Ͳ��ܿ��ƴӶ����½���
    //����λ���������� ���������߹��̣���Ҫ��������ѹ������
    s16   n_more_slv_code_running; //����ԭ�� �ӵ���������е���
    s16   n_more_press_running;    //����ԭ�� �������е���
    float mst_speed_limit;
    float mst_speed_limit_det_pr;
        
    u16   real_slv_up_press_to_adg;  //���������У�Ŀ��ѹ��ֵ�������ǰ�ǳ���δѹ������״̬��Ŀ��ѹ��ֵ����Щ
    u16   test_out_debug;
    u16   real_slv_down_press_to_adg; 

    //2017-03-27 Add By BG.ltd to let slave motor turn direction need some delate
    u8  ys_to_set_dir;
    u8  ys_to_set_speed;
    u8  ys_cur_real_dir;
    u8  ys_cur_real_speed;
    u8  ys_last_ctrl_dir;
    u16  ys_need_close_delay;  		//when to close slave motor,need delay some ticks
    u16  ys_running_time; 			//��ˢ��� �Ѿ��������ʱ�� ����ʱ���쳣���˶����ж�
}SlaveMotorAnaly;
extern SlaveMotorAnaly gSlaveMtAnaly;



//////////////////////////////////////////////////////////////////////////
//�ӵ�����Ƶ� ���ò����ṹ��
typedef struct
{
    u16 press_ok_min         ;    //�������й����У�ѹ������Сֵ
    u16 press_ok_max         ;    //�����˶������У�ѹ�������ֵ
    u16 press_ok_to_adjust   ;    //��Ҫ���е���ʱ��Ŀ��ѹ��ֵ

    //�������
    u16 press_mst_stop_min   ;    //ѹ��̫С����Ҫֹͣ������ ֹͣ������ѹ����Сֵ
    u16 press_mst_stop_max   ;    //ѹ��̫����Ҫֹͣ������ ֹͣ������ѹ�����ֵ 
    
    //�����ֹ��Ź��̵Ĳ���
    u16 mstAddAccl;               //��������ٶ�ֵ
    u16 mstDelAccl;               //��������ٶ�ֵ
    u16 onBridgeTime ;            //���������ߵ�ʱ�� ��λ��
    u16 mst_limit_on_bridge ;     //�����ϵ�����

    
    //����������Լ��������ѹֵ *10��
    u16 bat_auto_charge;		 //�����Ҫ�Զ�������
    u16 bat_charge_full;		 //��س�������
    u16 bat_no_move;             //�����ƶ��ĵ���ֵ
    
    float fk_mst_limit_up   ;   //�Ӷ��� ����ʱ ���ٵ�ϵ��
    float fk_mst_limit_down ;   //�Ӷ��� ����ʱ ���ٵ�ϵ��
    
    //����Ĳ�����Ҫ�� ����ѹ����ӵ���ٶȿ��ƹ�ϵ�Ĳ�����ѹ����Ŀ��ѹ��Խ���ٶ�Խ��
    //�ӵ���ٶ� = k*(ѹ��-press_ok_to_adjust) + ����ٶ�
    float fk_slv_speed_by_press_up; //��������ѹ�����ٶ�ϵ��
    float fk_slv_speed_by_press_down; //�½�����ѹ�����ٶ�ϵ��
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
*DESCRIPTION:  �Ӷ��ֵ�� ���ò�����ʼ��
*************************************************/
void zt_motor_slave_driver_cfg_init(void);


/************************************************* 
*Return:		
*DESCRIPTION:  ����˶������У�����⼰�������״̬����
*   δ���-->��ⶼ�������---��ѹ��-->ֹͣ�����г��--->���ʵ�ʵ�ѹ����Ҫ�رճ��
*                           -��ѹ��-->�����˶�
*   �����Ϊ���رյ����������Ҫ�ֶ�������������˶������ʾ�Զ��رճ��
*************************************************/

#define PRESS_FILTER_CNT_ONE  5
typedef struct 
{
    u8   original_vals[PRESS_FILTER_CNT_ONE];
    u8   cur_press_change;  //��ǰ��Ҫ�޸ĵ�����λ��
    u8   val;               //�˲����ֵ
    u16  press_sum_all;     //���������ݵ� �ܺ�
}PressFilterCtrl;
extern PressFilterCtrl gPressFilter;
u8 GetPressNewFilter(u16 new_cnt);  //������� new_cnt ��ѹ������ƽ��ֵ


/////////////////////////////////////////////////////////////////////////////////////////
/////�ӵ�� �ٶȿ�����غ���
/************************************************* 
*Return:	void
*DESCRIPTION:  �ӵ����һ�����������Ŀ��Ʒ�����ٶ�
*       ������Ҫ���ݵ�ǰʵ�ʷ������ٶȺͷ����趨ʵʱ��
*       �ٶȺͷ���
*************************************************/
void DoSetSlaveMtSpeedByCurSpeed(void);


#define PJZ_ANALY_MAX_CNT  14           //��¼���� �����ٶ� ʱ��������
#define MT_DIS_CONTROL_CHECK_TICK  60   //0.5��/8ms = 
typedef struct
{
    s16   ysVals[PJZ_ANALY_MAX_CNT];
    s16   lvVal;
    s32   ValSumAll; 					//�������ݺ�
}PjzLvboAnaly;
typedef struct
{
    //PjzLvboAnaly mstMtOut;
    PjzLvboAnaly mstMtBackSpeed;
    //PjzLvboAnaly slvMtOut;
    PjzLvboAnaly slvMtBackSpeed;
    s16 mstOutCnt;  				//��������������ʱ�����
    s16 slvOutCnt; 					//�ӵ�����������ʱ�����
    s16 mstNeedStopCnt;
    s16 slvNeedStopCnt;
    u16 mstNotMoveCnt;
    u16 curDealValID; 				//��ǰ���µ�����λ��
}MtDisControlCheck;
extern MtDisControlCheck gMtDisControlCheck;
/************************************************* 
*Return:	void
*DESCRIPTION:  ��������ٶȣ��Ͳɼ��ٶ��˲�������
*       ���ڼ�����Ƿ�ʧ�ء�������ʧ�أ�����Ҫǿ�Ƶ���ر�0.4�롣
*************************************************/
void CheckMtDisControl(void);

//�������еĵ�ص�ѹֵ������ ��ʵֵ*10
typedef struct{
    s16  nLastSlvWhlCode;
    s32  nCxCheckMove; 		//��⵽��Ѩ�����źţ��˶��������
    //iChargingCircle�����ڳ������еĵ�ѹ�ɼ�������Ͽ�������
    u16  iChargingCircle; 	//���ѭ��ֵ  ���5����--->�Ͽ����2��---->���ʵ�ʵ�ѹֵ--->���5����
    u16  iChargingInCnt;
    u16  curBatVol;  		//��ǰ��ѹֵ
    bool bChargingClosed;   //���رչ���
    bool bCharging; 		//��ǰ�Ƿ��ڳ����
    bool bOnBridge; 		//���������Ϸ�Χ�����뿪ʱ�����������ϡ�ֻ��������ʱ���˶����Ʋ��жϵ�ѹ�Ƿ�ͣ����˶�
}BatAutoCtrl;
extern BatAutoCtrl gBatAutoCtrl;
#define BAT_CHARGE_OPEN()    GpioSetBit(GPIO_BAT_CHARGE_EN,1)
#define BAT_CHARGE_CLOSE()   GpioSetBit(GPIO_BAT_CHARGE_EN,0)
/************************************************* 
*Return:	void
*DESCRIPTION:  ��ع������״̬����
*************************************************/
void UpdateFastBatChargingState(void);
void Sensor_Collect(void);
void TaskFun_MotorUpdate(void);
void  zt_motor_slave_driver_update(void);

#endif
