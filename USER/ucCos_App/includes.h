/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               includes.h
** Descriptions:            includes.h for ucos
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-9
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             kevin
** Modified date:          2019-04-04 
** Version:                 
** Descriptions:           Inspection Robot 
**
*********************************************************************************************************/


#ifndef  __INCLUDES_H__
#define  __INCLUDES_H__

/* Includes ------------------------------------------------------------------*/
#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <stdarg.h>

#include  <uCOS-II\Source\ucos_ii.h>
#include  <uCOS-II\Ports\ARM-Cortex-M3\RealView\os_cpu.h>
#include  <uC-CPU\ARM-Cortex-M3\RealView\cpu.h>
#include  <lib_def.h>
#include  <lib_mem.h>
#include  <lib_str.h>

#if (OS_VIEW_MODULE == DEF_ENABLED)
#include    <uCOS-VIEW\Ports\ARM-Cortex-M3\Realview\os_viewc.h>
#include    <uCOS-VIEW\Source\os_view.h>
#endif

#include    <stm32f10x.h>

#define Q_MAX_SIZE		2
extern void Delay(u32 nCount);


typedef unsigned char  uint8;                   /* defined for unsigned 8-bits integer variable 	�޷���8λ���ͱ���  */
typedef signed   char  int8;                    /* defined for signed 8-bits integer variable		�з���8λ���ͱ���  */
typedef unsigned short uint16;                  /* defined for unsigned 16-bits integer variable 	�޷���16λ���ͱ��� */
typedef signed   short int16;                   /* defined for signed 16-bits integer variable 		�з���16λ���ͱ��� */
typedef unsigned int   uint32;                  /* defined for unsigned 32-bits integer variable 	�޷���32λ���ͱ��� */
typedef signed   int   int32;                   /* defined for signed 32-bits integer variable 		�з���32λ���ͱ��� */
typedef float          fp32;                    /* single precision floating point variable (32bits) �����ȸ�������32λ���ȣ� */
typedef double         fp64;                    /* double precision floating point variable (64bits) ˫���ȸ�������64λ���ȣ� */

typedef struct {
    uint8  type;
	//uint8  id;
    uint16 count;
    uint8 *point;
}DATA_CONTROL_BLOCK;

#define BATTERY_MSG          1
#define SENSOR_MSG           2
#define MOTOR_MSG            3
#define WHL_MSG              8

#define WEATHER_DEBUG_OUT_MSG_DEBUG    0xf1
#define WEATHER_DEBUG_OUT_MSG_LINUX    0xf2
#define WEATHER_DEBUG_OUT_MSG_RF433    0xf3
#define LOCAL_DEBUG_IN_MSG_DEBUG       0x81
#define LOCAL_DEBUG_IN_MSG_LINUX       0x82
#define LOCAL_DEBUG_IN_MSG_RF433       0x83
//#define LOCAL_DEBUG_IN_MSG_GPS        0x84
#define LOCAL_DEBUG_IN_MSG_PTZ         0x85
#define LOCAL_DEBUG_IN_MSG_GPS_GNRMC   0x86
#define LOCAL_DEBUG_IN_MSG_GPS_GNGGA   0x87

#define ID_DEBUG                1               
#define ID_LINUX                2
#define ID_RF433                3

//�������߹رյ�����ܲ���
//����������ܲ��Ժ���������Ӷ��ֿ��Ʒ�ʽ�޸�Ϊ���ⲿ�������
//�����ٶȣ����ֱ����������ٶ�
#define MOTER_PERFORM_TEST_EN   1

#define YES 0x55
#define NOT 0xaa

#define OS_NO_ERR                 0

#define true  1
#define false 0


#ifndef bool
#define bool unsigned char
#endif

#define LsiFreq 32768

//-----------------����ϵͳ�ڴ�鶨��----------------------

#define		MEM64_NUM 			12		//1K
#define		MEM160_NUM 			30		//1K          
#define		MEM256_NUM 			5		//4K
#define		MEM512_NUM 			7		//3K 
#define		MEM1024_NUM			4		//3K


//-----------------��������������ջ����----------------------

#define		START_LENGH			100		//��������
#define		WEATHER_LENGH		100		//��������	
#define		TIMER_LENGH			100		//ʱ������
#define		LOCAL_LENGH			200		//��������
#define		REMOTE_REC_LENGH	200		//Զ�̽�������

/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/
#define  OS_VIEW_MODULE                  DEF_DISABLED	     	/* DEF_ENABLED = Present, DEF_DISABLED = Not Present        */


/*
*********************************************************************************************************
*                                              TASKS NAMES
*********************************************************************************************************
*/


//-----------------���ⲿ�����ı���----------------------
extern OS_EVENT  	*log_sem;

extern OS_MEM		*p_msgBuffer;

extern OS_MEM		*mem160ptr;
extern INT8U		 mem_160[MEM160_NUM][220];

extern OS_EVENT 	*mem512_sem;
extern OS_MEM		*mem512ptr;
extern INT8U		 mem_512[MEM512_NUM][512];

#define MCU_RAM_ADDR_BEGIN         0x20000000
#define MCU_RAM_ADDR_USER_BEGIN   (0x20000000+0x1000) //32K  �û�RAM�������Сֵ
#define MCU_RAM_ADDR_END          (0x20000000+0x10000) //64K
#define MCU_FLASH_ADDR_BEGIN       0x8000000 
#define MCU_FLASH_ADDR_END        (0x8000000+0x80000) //512K

#define MCU_FLASH_PAGE_SIZE        0x800 //2K
#define MCU_FLASH_PAGE_CNT         250 //ҳ��ɾ�� ҳ����������С�� 250ҳ

#define BOX_OS_USER_SLV_CFG_ADDR (MCU_FLASH_ADDR_BEGIN+0x68000)


typedef struct{//���й��ڻ����˵����ݶ�����ڸýṹ��
    u8		sendbuf[1000];		//���ͻ�����
    u16     counter;
    u16		icmd_len;			//�����
    int     trans_vals[6];      //���ڸ����������ݵ�����
    //���������״̬
    u8 nTimeForNoLinuxHeartIn ; //û������������
}ZT_INFO_TYPE;



//////////////////////////////////////////////////////////////////////////
//����ͬ���ڽ��յ�ATָ�����ݣ�����ṹ���¼������ռ�����״̬
typedef struct{
    unsigned char Data;
	unsigned char CurSendStatus;
	unsigned char CurRecStatus;
    //UartDriverCtrl* uart;  //AT����ʹ�õĴ���
    //const char*     name;  //ATָ����Դ����
    u8 recv_buf[254];
    u8 head[2];
	u8 counter;
    u8 recv_len;
    //bool bRecvdOneCmd;  //�����ֵΪtrue����ʾ�Ѿ����ճɹ�һ�����ݰ���
}AtCmdInfo;

typedef struct{
    bool bCX_SwCheck1 ;  //��Ѩ��λ����1
    bool bCX_SwCheck2 ;  //��Ѩ��λ����2
    bool bChargeVolIn ;  //���ӿ��е�ѹ����
    bool bChargeShort ;  //���ӿ��ⲿ��·
    u16  nBatVol  ;      //��ص�ѹ�˲���ֵ  ��Ҫ����ϵ��  21.0f*2.5f/4096 ��Ϊ������ص�ѹ
    u16  nBatCur  ;      //��ص����˲���ֵ ��Ҫ����ϵ�� (2.5f/4096/0.3f) 
    char GpsPosState[36];  //GPS λ���ַ��� 116.1479415,33.7518119,61.2
    char GpsTimeState[24]; //17,11,20,14,0,24
    char RobotName[24];
    char RobotState[28]; //����״̬��Idle���ֶ�����״̬��Handle���Զ�Ѳ��״̬��AutoRun�����س��״̬��BackChg��
    bool bRf433Mode3;
	char StateSwitchFlag[8];  //�Զ�Ѳ��ʱ�����յ��ֶ�ָ�״̬�л�
}RbtState;

#define CMD_PARA_STRING_LEN_NAME  20
#define CMD_PARA_STRING_LEN_VALUE 40

#pragma pack(1)
typedef struct{//__packed 
	char name [CMD_PARA_STRING_LEN_NAME];					//��������
	char value[CMD_PARA_STRING_LEN_VALUE];					//����ֵ����
    //int item_begin_pos; //��ǰ ���������� �����������еĿ�ʼλ��
    char item_begin_pos;
	//int name_str_len;
	char name_str_len;
    //int value_str_len;
	char value_str_len;
}CmdParaOneItem;

typedef struct
{
    CmdParaOneItem items[30];
    int            item_cnt;
}CmdParaInfo;


/*typedef struct
{
    s16 accl[3];        //���ٶ�ֵ
    s16 angle_vel[3];   //���ٶ�
    s16 angle[3];       //�Ƕ�
    s16 magnetic[3];    //�ų�����
    s16 temperature;    //�¶�
}jy_901_data_type;
extern jy_901_data_type gJy901Data;*/

typedef struct{
    bool bHongWai;  //0-�ɼ���Ƶ  1-������Ƶ
    u8   lastCmd;
    u16  nInfraredFocalDis;  //���⽹��
    u16  nVisibleFocalDis;   //�ɼ��⽹��
    s16  directAngle;  //��λ��
    s16  pitchAngle;   //������
    s16  rollAngle;    //�����   
}CamSet; 
extern CamSet GTZMHD_Set;

typedef struct{
    bool bHongWai;  //0-�ɼ���Ƶ  1-������Ƶ
    u8   lastCmd;
    u16  nInfraredFocalDis;  //���⽹��
    u16  nVisibleFocalDis;   //�ɼ��⽹��
    s16  directAngle;  //��λ��
    s16  pitchAngle;   //������
    s16  rollAngle;    //�����   
}CamGet; 
extern CamGet GTZMHD_Get;

extern u8 CurrentPtzSetCom;
extern u8 GTZMHDCmdSetStep1;
extern u8 GTZMHDCmdSetStep2;
extern u8 GTZMHDCmdSetOk;
extern u8 DoGTZMHDCmdSetRet;
extern u8 GTZMHDCmdGetStep1;
extern u8 GTZMHDCmdGetStep2;
extern u8 GTZMHDCmdGetOk;
extern u8 DoGTZMHDCmdGetRet;
extern char Frame_No_Debug[4];
extern char Frame_No_Linux[4];
extern char Frame_No_Rf433[4];
extern signed short CodeAB_Start;
extern char CodeZ_Start;
extern OS_EVENT	*PtzSem;
extern signed long gCodeZ;
extern signed long CodeAB_Last;

#ifndef  __IN__
#define  __IN__      //������Ϊ����
#endif
#ifndef  __OUT_
#define  __OUT_      //������Ϊ���
#endif

#define BAT_VOL_FLOAT   (gRbtState.nBatVol*21.0f*2.5f/4096)
#define BAT_CUR_FLOAT   (gRbtState.nBatCur*(2.5f/4096/0.3f))

#include "Task_Timer.h"
#include "Task_Local.h"
//#include "Task_Remote.h"
#include "Task_Weather.h"
#include "Task_Remote_Rec.h"

#include "Uart_api.h"
#include "zt_motor_master_driver.h"
#include "zt_motor_slave_driver.h"
#include "TimerPWm_Driver.h"
#include "GpioDefine.h"
#include "GpioDriver.h"
#include "user_adc_ctrl.h"
#include "adc_dma.h"
#include "str.h"
#include "cmd_para.h"
#include "AM2320_By_IIC.h"
#include "App_cfg.h"
//#include "iic_driver.h"
//#include "OperationList.h"
#include "GpsUart.h"

#endif

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/


