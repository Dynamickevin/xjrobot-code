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


typedef unsigned char  uint8;                   /* defined for unsigned 8-bits integer variable 	无符号8位整型变量  */
typedef signed   char  int8;                    /* defined for signed 8-bits integer variable		有符号8位整型变量  */
typedef unsigned short uint16;                  /* defined for unsigned 16-bits integer variable 	无符号16位整型变量 */
typedef signed   short int16;                   /* defined for signed 16-bits integer variable 		有符号16位整型变量 */
typedef unsigned int   uint32;                  /* defined for unsigned 32-bits integer variable 	无符号32位整型变量 */
typedef signed   int   int32;                   /* defined for signed 32-bits integer variable 		有符号32位整型变量 */
typedef float          fp32;                    /* single precision floating point variable (32bits) 单精度浮点数（32位长度） */
typedef double         fp64;                    /* double precision floating point variable (64bits) 双精度浮点数（64位长度） */

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

//开启或者关闭电机性能测试
//开启电机性能测试后，主动轮与从动轮控制方式修改为，外部命令控制
//多少速度，则会直接输出多少速度
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

//-----------------操作系统内存块定义----------------------

#define		MEM64_NUM 			12		//1K
#define		MEM160_NUM 			30		//1K          
#define		MEM256_NUM 			5		//4K
#define		MEM512_NUM 			7		//3K 
#define		MEM1024_NUM			4		//3K


//-----------------各个任务的任务堆栈长度----------------------

#define		START_LENGH			100		//空闲任务
#define		WEATHER_LENGH		100		//气象任务	
#define		TIMER_LENGH			100		//时间任务
#define		LOCAL_LENGH			200		//本地任务
#define		REMOTE_REC_LENGH	200		//远程接收任务

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


//-----------------向外部声明的变量----------------------
extern OS_EVENT  	*log_sem;

extern OS_MEM		*p_msgBuffer;

extern OS_MEM		*mem160ptr;
extern INT8U		 mem_160[MEM160_NUM][220];

extern OS_EVENT 	*mem512_sem;
extern OS_MEM		*mem512ptr;
extern INT8U		 mem_512[MEM512_NUM][512];

#define MCU_RAM_ADDR_BEGIN         0x20000000
#define MCU_RAM_ADDR_USER_BEGIN   (0x20000000+0x1000) //32K  用户RAM允许的最小值
#define MCU_RAM_ADDR_END          (0x20000000+0x10000) //64K
#define MCU_FLASH_ADDR_BEGIN       0x8000000 
#define MCU_FLASH_ADDR_END        (0x8000000+0x80000) //512K

#define MCU_FLASH_PAGE_SIZE        0x800 //2K
#define MCU_FLASH_PAGE_CNT         250 //页码删除 页码数量必须小于 250页

#define BOX_OS_USER_SLV_CFG_ADDR (MCU_FLASH_ADDR_BEGIN+0x68000)


typedef struct{//所有关于机器人的数据都存放在该结构体
    u8		sendbuf[1000];		//发送缓冲区
    u16     counter;
    u16		icmd_len;			//命令长度
    int     trans_vals[6];      //用于辅助命令数据的生成
    //机器人相关状态
    u8 nTimeForNoLinuxHeartIn ; //没有心跳包接收
}ZT_INFO_TYPE;



//////////////////////////////////////////////////////////////////////////
//管理不同串口接收的AT指令数据，下面结构体记录命令接收及处理状态
typedef struct{
    unsigned char Data;
	unsigned char CurSendStatus;
	unsigned char CurRecStatus;
    //UartDriverCtrl* uart;  //AT命令使用的串口
    //const char*     name;  //AT指令来源名称
    u8 recv_buf[254];
    u8 head[2];
	u8 counter;
    u8 recv_len;
    //bool bRecvdOneCmd;  //如果该值为true，表示已经接收成功一包数据包了
}AtCmdInfo;

typedef struct{
    bool bCX_SwCheck1 ;  //巢穴限位开关1
    bool bCX_SwCheck2 ;  //巢穴限位开关2
    bool bChargeVolIn ;  //充电接口有电压接入
    bool bChargeShort ;  //充电接口外部短路
    u16  nBatVol  ;      //电池电压滤波后值  需要乘以系数  21.0f*2.5f/4096 作为真正电池电压
    u16  nBatCur  ;      //电池电流滤波后值 需要乘以系数 (2.5f/4096/0.3f) 
    char GpsPosState[36];  //GPS 位置字符串 116.1479415,33.7518119,61.2
    char GpsTimeState[24]; //17,11,20,14,0,24
    char RobotName[24];
    char RobotState[28]; //空闲状态（Idle）手动控制状态（Handle）自动巡检状态（AutoRun）返回充电状态（BackChg）
    bool bRf433Mode3;
	char StateSwitchFlag[8];  //自动巡检时，接收到手动指令，状态切换
}RbtState;

#define CMD_PARA_STRING_LEN_NAME  20
#define CMD_PARA_STRING_LEN_VALUE 40

#pragma pack(1)
typedef struct{//__packed 
	char name [CMD_PARA_STRING_LEN_NAME];					//数据名称
	char value[CMD_PARA_STRING_LEN_VALUE];					//数据值内容
    //int item_begin_pos; //当前 参数的名字 在整个命令中的开始位置
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
    s16 accl[3];        //加速度值
    s16 angle_vel[3];   //角速度
    s16 angle[3];       //角度
    s16 magnetic[3];    //磁场方向
    s16 temperature;    //温度
}jy_901_data_type;
extern jy_901_data_type gJy901Data;*/

typedef struct{
    bool bHongWai;  //0-可见视频  1-红外视频
    u8   lastCmd;
    u16  nInfraredFocalDis;  //红外焦距
    u16  nVisibleFocalDis;   //可见光焦距
    s16  directAngle;  //方位角
    s16  pitchAngle;   //俯仰角
    s16  rollAngle;    //横滚角   
}CamSet; 
extern CamSet GTZMHD_Set;

typedef struct{
    bool bHongWai;  //0-可见视频  1-红外视频
    u8   lastCmd;
    u16  nInfraredFocalDis;  //红外焦距
    u16  nVisibleFocalDis;   //可见光焦距
    s16  directAngle;  //方位角
    s16  pitchAngle;   //俯仰角
    s16  rollAngle;    //横滚角   
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
#define  __IN__      //参数作为输入
#endif
#ifndef  __OUT_
#define  __OUT_      //参数作为输出
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


