#ifndef IN_TASK_REMOTE

extern const uint8 UpdataHeader[];
extern uint8	*CompUpNumBuf;
extern uint8	*PlishUpPockBuf;				
extern uint16	CompUpNumCounter;
extern uint16	AllUpdataNumber;
extern uint16	UnCompUpNumber;
extern uint16	UnUpCounter;
//extern uint8	UpdataFlagNOT;
//extern uint8	HeatBeatFalgNOT;
extern uint8 	LastPocketLength;
extern uint32 	UpDataDataLength;
//extern uint8 	GPRS_ErrorFlag;
extern uint8	NetStatus;
extern uint8	NetMode;
//extern uint8 	GPRS_LogErrorFlag;
extern OS_EVENT	*RemoteQ;
extern void Task_Remote(void *pdata);
extern void Task_Remote_Rec(void *pdata);
extern void NormalReceive(uint8 *sp,uint16 length);
extern uint8 base_sent_ack(void *msg);
extern void UpdataProgDeal(uint8 *sp,uint16 length);
extern uint8 LoginGPRS(void);



#endif

