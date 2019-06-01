#ifndef TASK_TIMER_H
#define TASK_TIMER_H

extern uint16 SensorRecCounter;
extern uint16 VibCounter[];
extern void StatisticsRecord(void);
extern uint8 RecordExistFlag;
extern uint16 HeartBeatCounter;				//心跳
extern uint16 ReportCounter;				//主动上报
extern uint16 GPRS_LogDelayCounter;
//extern uint8 StatisticsInterval;
extern uint8 GPRS_writelogin_Interval;
extern OS_EVENT *timerQueue;				//定时器秒脉冲邮箱
extern void Task_Timer(void *pdata);

extern uint8 GPRS_writelogin_Interval;
extern uint8 GPRS_noack_Interval;

extern void CopyBuffer(void *SouPointer,void *DesPointer,uint16 Length);
void Clear_Buffer(void *pointer,uint16 length);
void DoBatUsingGetFilter(void);
void Battery(void);
void zt_msg_send_real_time_pkg(void);
extern void Boot_HWDT_Feed(void);
extern void HWDT_Feed(void);


#endif
