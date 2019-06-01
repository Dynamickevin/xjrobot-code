#define IN_TASK_WEATHER
#include "includes.h"

//---------本任务涉及到的消息-----------
OS_EVENT *GetWeatherQueue;			

/******************************************************/

extern OS_MEM		*p_msgBuffer;

uint16 BoardTemperature(void); 

void Task_Weather(void *pdata)
{
	uint8 err;
	DATA_CONTROL_BLOCK *bp;
	
	while ( 1 )
	{	
		bp = (DATA_CONTROL_BLOCK*)OSQPend(GetWeatherQueue,0,&err);
		if ( err == OS_ERR_NONE )
		{
			switch(bp->type)
			{
				case WEATHER_DEBUG_OUT_MSG_DEBUG:
				{
					uart1_send(bp->point, bp->count);
					OSMemPut(mem160ptr,bp->point);
					break;
				}
					
				case WEATHER_DEBUG_OUT_MSG_LINUX:
				{
					uart2_send(bp->point, bp->count);
					OSMemPut(mem160ptr,bp->point);
					break;
				}				
				case WEATHER_DEBUG_OUT_MSG_RF433:
				{
					uart3_send(bp->point, bp->count);
					OSMemPut(mem160ptr,bp->point);
					break;
				}
				case SENSOR_MSG:
				{
					//GpioSetL(GPIO_LED_SHOW1);
					//GpioSetL(GPIO_LED_OUT_SHOW1);
					Sensor_Collect();
					break;
				}
					
				default:
					break;				
			}
	 	}
	OSMemPut(p_msgBuffer,(void *)bp);
	}
}

