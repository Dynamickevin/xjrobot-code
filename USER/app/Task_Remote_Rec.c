#define IN_TASK_REMOTE_REC
#include "includes.h"

//uint8 CAP_NUM = 0;

OS_EVENT *RemoteRecQueue;

extern OS_MEM		*p_msgBuffer;

void Task_Remote_Rec(void *pdata)
{
	uint8 err;
	DATA_CONTROL_BLOCK *msg;

	//OSTimeDly(OS_TICKS_PER_SEC);
	
	while( 1 )
	{
		msg = (DATA_CONTROL_BLOCK *)OSQPend(RemoteRecQueue,0,&err);
		//debug_nprintf(ID_DEBUG,err,0,0);
		if(err == OS_ERR_NONE)
		{
			switch ( msg->type )
			{
				case LOCAL_DEBUG_IN_MSG_RF433:
				{   //GpioSetL(GPIO_LED_SHOW1);
				    CommDebug(ID_RF433,msg->point,msg->count);
				}
				break;	
				case LOCAL_DEBUG_IN_MSG_DEBUG:
				{
					//GpioSetL(GPIO_LED_SHOW1);
					CommDebug(ID_DEBUG,msg->point,msg->count);
				}
				break;	
				case LOCAL_DEBUG_IN_MSG_LINUX:	
				{
					CommDebug(ID_LINUX,msg->point,msg->count);
				}
				break;
				case LOCAL_DEBUG_IN_MSG_PTZ:	
				{
					GTZMHDDebug(msg->point,msg->count);
				}
				break;
				case LOCAL_DEBUG_IN_MSG_GPS_GNRMC:	
				{
					GPSGnrmcDebug(msg->point,msg->count);
				}
				break;
				case LOCAL_DEBUG_IN_MSG_GPS_GNGGA:	
				{
					GPSGnggaDebug(msg->point,msg->count);
				}
				break;
				
				default:
				break;
			}
			OSMemPut(mem160ptr,(void*)(msg->point));
			OSMemPut(p_msgBuffer,(void*)msg);
		}
	}
}

