#ifndef TASK_WEATHER

#define		AD_CHANNEL_NUM		6
#define		AD_CHANNEL_BUF		10
#define		AD_BUF_LENGTH		AD_CHANNEL_NUM*AD_CHANNEL_BUF

extern uint16 BoardTemperatureValue;

extern OS_EVENT *GetWeatherQueue;		       						//接收AD数据邮箱

extern void Task_Weather(void *pdata);
extern uint16 WeatherCalculate(uint16 AD_Value);
extern uint16 BatteryAD(void);
extern uint16 BoardTemperature(void); 

#endif
