/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               app_cfg.h
** Descriptions:            ucosii configuration
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-9
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Version:                 
** Descriptions:            
**
*********************************************************************************************************/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__
					  
/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/ 
#define  OS_VIEW_TASK_PRIO               				2


#define  APP_TASK_START_PRIO                               	0
#define  APP_TASK_OSVIEW_TERMINAL_PRIO	 (OS_LOWEST_PRIO - 6)
#define  OS_TASK_TMR_PRIO                (OS_LOWEST_PRIO - 2)

//-----------------各个任务的优先级定义----------------------

#define		WEATHER_TASK_PRIO	4
#define		WEATHER_TASK_ID		4

#define		REMOTE_REC_TASK_PRIO	5
#define		REMOTE_REC_TASK_ID		5

#define		TIMER_TASK_PRIO	    6
#define		TIMER_TASK_ID		6

#define		LOCAL_TASK_PRIO		7
#define		LOCAL_TASK_ID		7



/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  APP_TASK_START_STK_SIZE                          128u
#define  APP_TASK_OSVIEW_TERMINAL_STK_SIZE   			  256u
#define  OS_VIEW_TASK_STK_SIZE                            256u


/*
*********************************************************************************************************
*                                                  LIB
*********************************************************************************************************
*/

#define  uC_CFG_OPTIMIZE_ASM_EN                 DEF_ENABLED
#define  LIB_STR_CFG_FP_EN                      DEF_DISABLED






#endif

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

