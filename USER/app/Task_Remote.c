#define IN_TASK_REMOTE
#include "includes.h"

/*********************************
������������
**********************************/
uint16 AllUpdataNumber = 0;
uint16 UnCompUpNumber = 0;
uint16 UnUpCounter = 0;
uint16 CompUpNumCounter = 0;
uint32 	UpDataDataLength; 
uint8 	LastPocketLength = 0;
uint8  *PlishUpPockBuf;
uint8  *CompUpNumBuf;
/********************************/

OS_EVENT				*RemoteQ; 

//RESPONSE_INF_T response_flag;
//static const uint32 GPRS_Bode[] = {9600,38400,19200,9600,19200,9600,4800,2400,1200,600,300};
//int16 dataLen[] = {0, WEATHER_SIZE, LINE_HEAT_SIZE, WIND_ANGLE_SIZE, TANGLE_SIZE, ICE_SIZE, VIB_WAVE_SIZE};
//int32 dataMaxNum[]= {0, WEATHER_NUMBER_LIMIT, LINE_HEAT_NUMBER_LIMIT, WIND_ANGLE_NUMBER_LIMIT, TANGLE_NUMBER_LIMIT, ICE_NUMBER_LIMIT, VIB_WAVE_NUMBER_LIMIT};
uint8 sendData_Buf[512];



const uint8 bin2asc[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',};
#define	 DATALENGTH_PER_PACK	 40
/*
void Clear_UpdataPara(void)
{
	AllUpdataNumber = 0;
	UnCompUpNumber = 0;
	UnUpCounter = 0;
	CompUpNumCounter = 0;
	LastPocketLength = 0;	
}*/
/*
uint8 BaseSentData(uint8 *sp,uint16 Length)
{
	uint8 temp,err;
	uint8 *mp;
	uint8 i,j;
		
	if ( Length<1024 )
	{
		if ( (Length%DATALENGTH_PER_PACK)==0 )
		{
			temp = Length/DATALENGTH_PER_PACK;
		}
		else
		{
			temp = Length/DATALENGTH_PER_PACK + 1;
		}
		
		for ( j=0;j<temp;j++ )
		{
			OSTimeDly(OS_TICKS_PER_SEC/5);
			OSSemPend(mem256_sem,0,&err);  
			mp = OSMemGet(mem256ptr,&err);
			if(OS_NO_ERR != err)
			{
				OSSemPost(mem256_sem);
				return FALSE;
			}

			CopyBuffer( "at+mipsend=1,\"",mp,14 ); 
			for(i = 0; i<DATALENGTH_PER_PACK; i++)
			{
			    mp[i*2+14]  = bin2asc[sp[i+j*DATALENGTH_PER_PACK]>>4];
			    mp[i*2+15]  = bin2asc[sp[i+j*DATALENGTH_PER_PACK]&0x0f]; 
				
				if ( Length==0 )
				{
					break;
				}
				Length -= 1;
			}
	
			mp[i*2+14] = '"';
			mp[i*2+15] = '\r';
			mp[i*2+16] = '\n';
			mp[i*2+17] = '\0';
				
			Uart1SendSTR((char *)mp);
			OSTimeDly(OS_TICKS_PER_SEC/5);

			OSMemPut(mem256ptr,mp);             //�ͷ��ڴ��  
			OSSemPost(mem256_sem);	
		}
		OSTimeDly(OS_TICKS_PER_SEC/10);
		Uart1SendSTR("at+mippush=1\r\n"); 
	}

	OSTimeDly(1);//Uart0SendSTR("\r\n�ϴ�����\r\n");
	//stprintf("\r\nDataUp");
	return TRUE;
}*/
/*
void break_net_power(void)
{
	GPRS_IGN_L();
	GPRS_EN_L();
	Uart1Flush();
	OSTimeDly(OS_TICKS_PER_SEC*2);
	GPRS_EN_H();
	OSTimeDly(OS_TICKS_PER_SEC*1);  
	GPRS_IGN_H();  
	OSTimeDly(OS_TICKS_PER_SEC*1); 
}*/
/*
void NetSentHeartBeat(void)
{
	uint32 temp32;
	HEART_T	HeartBeat;
	
	HeartBeat.Sync = 0x5aa5;										// ��ͷ
	HeartBeat.Netbasestation_ID = SystemInfo.BasestationID;			// ���ݼ��������
	HeartBeat.Frame_Type = 0x05;									// ֡����
	HeartBeat.Packet_Type = 0xf6;									// ��������
	HeartBeat.Packet_Length = 6;									// ���ĳ���
	HeartBeat.Sensor_ID = 0;										// ��ⵥԪ���
	temp32 = RTC_toTimeStamp();										// �ɼ�ʱ��--���ݼ�����ʱ��
	HeartBeat.Timestamp_L = (uint16)temp32;
	HeartBeat.Timestamp_H = temp32>>16;
	HeartBeat.CRC16 = CRCW16((uint8 *)&HeartBeat,HeartBeat.Packet_Length+8);				
	BaseSentData((uint8 *)&HeartBeat,HeartBeat.Packet_Length+10);
#if REMOTE_DEBUG_EN
	stprintf("\r\n������������\r\n");
#endif
}

void NetSentUpHeartBeat(void)
{
	heartshake	HeartBeatUpdata;
	
	HeartBeatUpdata.Sync = 0x5aa5;									// ��ͷ
	HeartBeatUpdata.Netbasestation_ID = SystemInfo.BasestationID;		// ���ݼ��������
	HeartBeatUpdata.Frame_Type = 0xa0;								// ֡����
	HeartBeatUpdata.Packet_Type = 0x08;								// ��������
	HeartBeatUpdata.Packet_Length = 18;		       					// ���ĳ���
	HeartBeatUpdata.CRC16 = CRCW16((uint8 *)&HeartBeatUpdata,HeartBeatUpdata.Packet_Length+8);				
	BaseSentData((uint8 *)&HeartBeatUpdata,HeartBeatUpdata.Packet_Length+10);
	#if REMOTE_DEBUG_EN
	stprintf("\r\n���³�������\r\n");
	#endif
}

uint8 gprs_init()
{
	int i,j;
	//break_net_power();
	OSTimeDly(OS_TICKS_PER_SEC/10);

	for( i=0;i<3;i++)
	{
		for( j=0;j<4;j++)
		{
			memset(&response_flag,0,sizeof(RESPONSE_INF_T));
			SetUART1Bps(GPRS_Bode[i] );
			OSTimeDly(OS_TICKS_PER_SEC/4);
			Uart1SendSTR("AT\r\n");  
			if(response_flag.login_ok_flag == 1)
			{
				response_flag.login_ok_flag=0;
				goto next;
			}
		}
	}	
	if( LogFlag.G24_logsFlag==NOT )
	{
		LogFlag.G24_logsFlag = YES;
		write_log(REMOTE_TASK_ID,G24_CONNECT_ERR);	
	}
	return FALSE;
	
next:
	stprintf("\r\nGPRS��ʼ��0 �ɹ�");
	for ( i=0;i<3;i++ )
	{
  		memset(&response_flag,0,sizeof(RESPONSE_INF_T));
		Uart1SendSTR("at+ipr=9\r\n");
		OSTimeDly(OS_TICKS_PER_SEC/2);

		if(response_flag.login_ok_at_flag==1)
		{		
			response_flag.login_ok_at_flag=0;
			goto next1;
		}

	}
	if( LogFlag.G24_logsFlag==NOT )
	{
		LogFlag.G24_logsFlag = YES;
		write_log(REMOTE_TASK_ID,G24_CONNECT_ERR);	
	}
	return FALSE;
	
next1:
	stprintf("\r\nGPRS��ʼ��1 �ɹ�");

	return TRUE;
}*/

/*
uint8 LoginGPRS(void)
{
	uint16 len;
	uint8 i,j,k;//,m;
	uint8 dp[64];
	DATA_CONTROL_BLOCK  *bp;

	if(FALSE==gprs_init())
	{
		return FALSE;
	}
	//break_net_power();   
	SetUART1Bps(38400);
		
	for ( i=0;i<3;i++ )
	{	
		memset(&response_flag,0,sizeof(RESPONSE_INF_T));
		Uart1SendSTR("ate1\r\n");  																		//ATE0-�ػ��ԣ�ATE1-������
		OSTimeDly(OS_TICKS_PER_SEC);
		
		if(response_flag.login_ok_flag == 1)
		{
			response_flag.login_ok_flag=0;
			goto next;
		}		
	}
	
	return FALSE;
	
next:	
		stprintf("\r\nGPRS��ʼ��2 �ɹ�");	
	for ( i=0;i<3;i++ ) //���Կ�  
	{
		memset(&response_flag,0,sizeof(RESPONSE_INF_T));
		Uart1SendSTR("at+cpin?\r\n");
		OSTimeDly(OS_TICKS_PER_SEC*2);	
		
		if(response_flag.sim_ready_flag == 1)
		{
			response_flag.sim_ready_flag=0;
			goto next1;
		}
		
		if(response_flag.sim_no_card == 1)
		{
			//moto_g20.state = NO_CARD;//�޿�д��־
			response_flag.sim_no_card = 0;
		}
		
		if(response_flag.sim_card_lock== 1)
		{
			response_flag.sim_card_lock=0;
			//moto_g20.state = CARD_LOCK; //������   д��־
		}	
	}
	
	return FALSE;
	
next1:
	#if GPRS_SMS
	stprintf("\r\nGPRS��ʼ��3 �ɹ�");	
	for ( i=0;i<3;i++ )
	{	
		memset(&response_flag,0,sizeof(RESPONSE_INF_T));
		Uart1SendSTR("at+csms=128\r\n");  
		OSTimeDly(OS_TICKS_PER_SEC/2);
		if(response_flag.login_ok_flag == 1)
		{
			response_flag.login_ok_flag=0;
			goto next2;
		}
	}
	return FALSE;
	
next2:	
	#endif
	stprintf("\r\nGPRS��ʼ��4 �ɹ�");	
	for ( i=0;i<3;i++ )
	{	
		memset(&response_flag,0,sizeof(RESPONSE_INF_T));
		Uart1SendSTR("at+creg=1\r\n");  
		OSTimeDly(OS_TICKS_PER_SEC/2);
		
		if(response_flag.login_ok_flag == 1)
		{
			response_flag.login_ok_flag=0;
			goto next3;
		}
	}
	return FALSE;
		
next3:	
	#if GPRS_SMS
	stprintf("\r\nGPRS��ʼ��5 �ɹ�");	

	for ( i=0;i<3;i++ )
	{
		memset(&response_flag,0,sizeof(RESPONSE_INF_T));
		Uart1SendSTR("at+cmgf=1\r\n"); 
		OSTimeDly(OS_TICKS_PER_SEC/2);
		
		if(response_flag.login_ok_flag == 1)
		{
			response_flag.login_ok_flag=0;
			goto next4;
		}

	}
	return FALSE;
		
next4:
	stprintf("\r\nGPRS��ʼ��6 �ɹ�");	

	for ( i=0;i<3;i++ )
	{	
		memset(&response_flag,0,sizeof(RESPONSE_INF_T));
		Uart1SendSTR("at+csca=\"+8613800100500\"\r\n"); 
		OSTimeDly(OS_TICKS_PER_SEC/2);
		if(response_flag.login_ok_flag == 1)
		{
		  response_flag.login_ok_flag=0;
		  goto next5;
		}

	}
	return FALSE;
		
next5:	
	stprintf("\r\nGPRS��ʼ��7 �ɹ�");	
	//ѡ�����Ϣ��ʾ
	for ( i=0;i<3;i++ )
	{	
						
		Uart1SendSTR("at+cnmi=0,1,0,0,0\r\n"); //��֧��
		OSTimeDly(OS_TICKS_PER_SEC);
		if(response_flag.login_ok_flag == 1)
		{
		  response_flag.login_ok_flag=0;
		  goto next6;
		}

	}
	return FALSE;
	
next6:	
	#endif
	stprintf("\r\nGPRS��ʼ��8 �ɹ�");	
	for ( i=0;i<3;i++ )		//���縲��
	{
		memset(&response_flag,0,sizeof(RESPONSE_INF_T));
		Uart1SendSTR("at+cgprs?\r\n");
		OSTimeDly(OS_TICKS_PER_SEC*1);	
		
		if(response_flag.sim_net_normal== 1)
		{
		  response_flag.sim_net_normal=0;
		  goto next7;
		}

		if(response_flag.sim_no_net== 1)
		{
			response_flag.sim_no_net=0;
			//������
			//moto_g20.state = NO_NET;
		}

	}
	return FALSE;
	
next7:				
	stprintf("\r\nGPRS��ʼ��9 �ɹ�");	

	for ( i=0;i<3;i++ )		//���ӵ��������д��������
	{
		memset(&response_flag,0,sizeof(RESPONSE_INF_T));
		datcpy(dp,(uint8*)"at+mipcall=1,",13);		
		datcpy(dp+13,GPRS_Cfg.AccessPoint,strlen((char*)GPRS_Cfg.AccessPoint));
		Uart1SendSTR((char *)dp);
		OSTimeDly(OS_TICKS_PER_SEC*2);
		for ( j=0;j<3;j++ )	
		{
			if(response_flag.net_mipcall0== 1)
			{
			  response_flag.net_mipcall0=0;
			}


			if(response_flag.net_mipcall== 1)
			{
			  response_flag.net_mipcall0=0;
			  goto next8;
			}

			
			//if(found_str("ERROR",sp,len) != NULL)
			//{
				//moto_g20.state = LINK_OK;
				//break;
			//}

		}
	}
	return FALSE;
	
next8:	
	stprintf("\r\nGPRS��ʼ��10 �ɹ�");	

	for ( i=0;i<5;i++ )		//���ӵ�����
	{
		OSTimeDly(OS_TICKS_PER_SEC*2);
		datcpy(dp,(uint8*)"at+mipopen=1,4000,\"211.160.019.092\",01234,1\r\n",45);
		if ( machine_state.LoginMode == NET_NORMAL_MODE)
		{
			ip_addr2str(&GPRS_Cfg.Remote_IP_1[0],(char*)dp+18);
		}
		if ( machine_state.LoginMode == NET_UPDATA_PROG_MODE)
		{
			ip_addr2str(&GPRS_Cfg.Updata_IP_1[0],(char*)dp+18);
		}
		if((machine_state.comnetMode == NET_LOGIN_MODE)&&(machine_state.LoginMode == NET_LOGIN_MODE))
		{
			ip_addr2str(&GPRS_Cfg.Remote_IP_1[0],(char*)dp+18);
		}
		  
		*(dp+16) = SEC % 10 +'0'; //add 06-6-7
		Uart1SendSTR((char*)dp);	

		for ( j=0;j<3;j++ )
		{	

			len = bp->count;
			len %=256;
			
			OSTimeDly(OS_TICKS_PER_SEC*2);
			if(response_flag.net_mipopen== 1)
			{
				response_flag.net_mipopen = 0;
				for ( k=0;k<3;k++ )
				{	
					memset(&response_flag,0,sizeof(RESPONSE_INF_T));
					Uart1SendSTR("ate0\r\n");  	
					OSTimeDly(OS_TICKS_PER_SEC*1);//ATE0-�ػ��ԣ�ATE1-������

					if(response_flag.login_ok_flag == 1)
					{
						response_flag.login_ok_flag =0;
						goto next9;
					}
						
				}

			}

		}

	}

	return FALSE;
	
next9:
	stprintf("\r\nGPRS��ʼ��11 �ɹ�");	
	stprintf("��ʼ���ɹ�");

	if((machine_state.comnetMode == NET_LOGIN_MODE)&&(machine_state.LoginMode == NET_LOGIN_MODE))
	{
		machine_state.comnetMode = NET_NORMAL_MODE;
		machine_state.LoginMode = NET_NORMAL_MODE;
		stprintf("\r\nͨ�����翪��ע��ɹ�!\r\n");
	}
	else
	{	
	
		 machine_state.comnetMode = machine_state.LoginMode;
		 stprintf("\r\nͨ������ע��ɹ�!\r\n");
		 
	}
	GPRS_writelogin_Interval = 1;
	LogFlag.G24_logsFlag = NOT;

	return TRUE;

}*/
/*
uint8 base_sent_ack(void *msg)
{

	BaseSentData((uint8 *)(((DATA_CONTROL_BLOCK *)msg)->point),((DATA_CONTROL_BLOCK *)msg)->count);	
	OSMemPut(mem512ptr,(void *)(((DATA_CONTROL_BLOCK *)msg)->point));
	OSSemPost(mem512_sem);
	
	return 0;
}*/

/*
uint8 SentRecord(uint8 type)
{	

	uint32 temp32;

	switch (type)
	{
		case weather: 		//����
		{
			temp32 = DataRecord.SentPoint[type]*WEATHER_SIZE + WEATHER_START_ADDR;

			flash_read( temp32,WEATHER_SIZE,sendData_Buf);
			BaseSentData(sendData_Buf,WEATHER_SIZE);
			
			#if REMOTE_DEBUG_EN
			stprintf("\r\n�ϱ�һ����������!");
			#endif

			
		}break;
			
		case windAngle: 		//��ƫ
		#if WIND_ANGLE_EN
		{

			temp32 = (DataRecord.SentPoint[type])*WIND_ANGLE_SIZE + WIND_ANGLE_START_ADDR;

			flash_read( temp32,WIND_ANGLE_SIZE,sendData_Buf);
			BaseSentData(sendData_Buf,WIND_ANGLE_SIZE);

			#if REMOTE_DEBUG_EN
			stprintf("\r\n�ϱ�һ����ƫ����!");
			#endif
		
		}break;
		#endif
		
		case lineTemp: 		//�¶�
		{
				
			temp32 = (DataRecord.SentPoint[type])*LINE_HEAT_SIZE + LINE_HEAT_START_ADDR;


			flash_read( temp32,LINE_HEAT_SIZE,sendData_Buf);
			BaseSentData(sendData_Buf,LINE_HEAT_SIZE);

			#if REMOTE_DEBUG_EN
			stprintf("\r\n�ϱ�һ���¶�����!");
			#endif

		}break;

		case towerAngle: 		//��б
		#if TANGLE_START_EN
		{
			temp32 = (DataRecord.SentPoint[type])*TANGLE_SIZE + TANGLE_START_ADDR;
			flash_read( temp32,TANGLE_SIZE,sendData_Buf);
			BaseSentData(sendData_Buf,TANGLE_SIZE);

			#if REMOTE_DEBUG_EN
			stprintf("\r\n�ϱ�һ����б����!");
			#endif

			//machine_state.reportdatatype = 4;
		}break;
		#endif

		case ice: 		//����
		#if ICE_EN
		{
			temp32 = (DataRecord.SentPoint[type])*ICE_SIZE + ICE_START_ADDR;

			flash_read( temp32,ICE_SIZE,sendData_Buf);
			BaseSentData(sendData_Buf,ICE_SIZE);
			#if REMOTE_DEBUG_EN
			stprintf("\r\n�ϱ�һ����������!");
			#endif


		}break;
		#endif

		case vibration: 		//��
		{
			temp32 = (DataRecord.SentPoint[type])*VIB_WAVE_SIZE + VIB_WAVE_START_ADDR;

			flash_read(temp32,VIB_WAVE_SIZE,sendData_Buf);
			BaseSentData(sendData_Buf,VIB_WAVE_SIZE);				

			#if REMOTE_DEBUG_EN
			stprintf("\r\n�ϱ�һ��������!");
			#endif

		}break;
		
		default:
		{
			if(machine_state.send_heart_times++ > 10)
			{
				machine_state.send_sucss_flag[0] = 0;
				NetSentHeartBeat();
				machine_state.send_heart_times = 0;
			}
		}break;
	}

return 0;

	
}*/
/*
int8 getSendType(uint8 *type)
{
	uint8 i;
	for(i = *type+1; i < RECORD_TYPE_NUM+1; i++)
	{
		if(DataRecord.SentPoint[i] != DataRecord.StorePoint[i])
		{
			*type = i;
			return 0;
		}
	}
	if(i == RECORD_TYPE_NUM+1)
	{
		for(i = 1; i < *type+1; i++)
		{
			if(DataRecord.SentPoint[i] != DataRecord.StorePoint[i])
			{
				*type = i;
				return 0;
			}
		}
	}	
	*type = 0;
	return -1;
}*/

/*
uint8 checkDataCRC(uint8 type)
{
	uint16 len;
	
	len = dataLen[type] - 10;

	if((sendData_Buf[len+8] + sendData_Buf[len+9]*256)==CRCW16(sendData_Buf,len+8) )
		return TRUE;

	return FALSE;
}*/

/*
uint8 ReportRecords(uint8 *start)
{
	uint8 err;
	DATA_CONTROL_BLOCK	*dp1;

	if(*start != 0)
	{
		machine_state.send_heart_times = 0;
		
		if(machine_state.send_sucss_flag[*start] == YES)
		{
			machine_state.send_data_times = 0;
			memset(machine_state.send_sucss_flag, 0, sizeof(machine_state.send_sucss_flag));
			DataRecord.SentPoint[*start] = (DataRecord.SentPoint[*start]+1)%dataMaxNum[*start];
			SaveRecordPointer();
			getSendType(start);
			GPRS_LED2_OFF();
			LogFlag.noAck_Flag = NOT;
		}
		else
		{
			if(checkDataCRC(*start) == FALSE)
			{
				DataRecord.SentPoint[*start] = (DataRecord.SentPoint[*start]+1)%dataMaxNum[*start];
				SaveRecordPointer();
				getSendType(start);
				write_log(REMOTE_TASK_ID,BASE_SEND_CRC_ERR);
				stprintf("\r\n crc error");
			}
			else
			{
				if(machine_state.send_data_times++% 8 == 0)
				{
					BaseSentData(sendData_Buf,dataLen[*start]);
				}
				if((LogFlag.noAck_Flag == NOT)&&(machine_state.send_data_times %60 == 0))
				{
					write_log(REMOTE_TASK_ID,CENTER_NO_ACK);
					LogFlag.noAck_Flag = YES;
				}
				if(machine_state.send_data_times%1200 == 0)
				{					
					machine_state.comnetMode = NET_LOGIN_MODE;
					machine_state.LoginMode = NET_NORMAL_MODE;	
					dp1 = OSMemGet(p_msgBuffer,&err);
					if(err == OS_NO_ERR)
					{
						if(OS_NO_ERR != OSQPost(RemoteQ,(void*)dp1))
						{
							OSMemPut(p_msgBuffer,dp1);
						}
					}
				}

				if((machine_state.send_data_times%2 == 0)&&(machine_state.send_data_times > 10))
				{
				
					GPRS_LED2_ON();
					OSTimeDly(OS_TICKS_PER_SEC/3);
					GPRS_LED2_OFF();
				}

				return 0;
			}
		}
	}
	else
	{
		getSendType(start);
	}
	
	SentRecord(*start);

	return 0;
}
*/

/*
void get_UpdataCommand(uint8 *dp)
{
	uint8 *dp1;
	uint8 err_p,err_n,err, err_q;
	DATA_CONTROL_BLOCK *np;

#if REMOTE_DEBUG_EN
	stprintf("\r\n�յ�Զ�̸��³�������\r\n");
#endif

	#if 0
	UpdataRequest.Sync = 0x5aa5;					
	UpdataRequest.Netbasestation_ID = SystemInfo.BasestationID; 
	UpdataRequest.Frame_Type = 0xA0;			
	UpdataRequest.Packet_Type = 0x02;			
	UpdataRequest.Packet_Length = 12;		
	UpdataRequest.AllPackets_No = dp[8] + dp[9]*256;		
	Clear_Buffer(UpdataRequest.Reserve,10); 
	UpdataRequest.CRC16 = CRCW16((uint8 *)&UpdataRequest,UpdataRequest.Packet_Length+8);
	OSTimeDly(OS_TICKS_PER_SEC/5);	
	
	OSSemPend(mem512_sem,0,&err);
	np = OSMemGet(p_msgBuffer,&err_p);
	dp1 = OSMemGet(mem512ptr,&err_n);
	if((err_p == OS_NO_ERR)&&(err_n == OS_NO_ERR))
	{
		np->type = NORMAL_ACK_MSG;
		np->count = UpdataRequest.Packet_Length + 10;
		
	
		CopyBuffer( &UpdataRequest,dp1,np->count);
		np->point = (uint8 *)dp1;
		if(OS_NO_ERR != OSQPost(RemoteQ,(void*)np)) 
		{
			OSMemPut(mem512ptr,dp1);
			OSSemPost(mem512_sem);
			OSMemPut(p_msgBuffer,np);
		}
	}
	else
	{
		if ( err_p == OS_NO_ERR )
		{
			OSMemPut(p_msgBuffer,np);
		}
		if ( err_n == OS_NO_ERR )
		{
			OSMemPut(mem512ptr,dp1);
		}
		OSSemPost(mem512_sem);
		return ;
	}	
	#endif
	
	OSSemPend(mem512_sem,0,&err);									
	PlishUpPockBuf = OSMemGet(mem512ptr,&err_n);
	CompUpNumBuf = OSMemGet(mem160ptr,&err_p);
	np = OSMemGet(p_msgBuffer,&err_q);
	if((err_p == OS_NO_ERR)&&(err_n == OS_NO_ERR)&&(err_q == OS_NO_ERR))
	{
		//Save Updata IP
		GPRS_Cfg.Updata_IP_1[0] = dp[8];					//������IP���˿�  
		GPRS_Cfg.Updata_IP_2[1] = dp[9];
		GPRS_Cfg.Updata_IP_3[2] = dp[10];
		GPRS_Cfg.Updata_IP_4[3] = dp[11];
		GPRS_Cfg.Updata_Port_1 = dp[12]+dp[13]*256;
		SaveGPRS_Information();
				
		Clear_Buffer(CompUpNumBuf,200);
		machine_state.comnetMode = NET_LOGIN_MODE;
		machine_state.LoginMode = NET_UPDATA_PROG_MODE;
		if(OS_NO_ERR != OSQPost(RemoteQ,(void*)dp1))
		{
		#if 0
			OSMemPut(mem512ptr,PlishUpPockBuf);
			OSSemPost(mem512_sem);
			OSMemPut(mem160ptr,CompUpNumBuf);
		#endif
			OSMemPut(p_msgBuffer,np);
		}
	}
	else
	{
		if(err_n == OS_NO_ERR)
			OSMemPut(mem512ptr,PlishUpPockBuf);
		if(err_p== OS_NO_ERR)
			OSMemPut(mem160ptr,CompUpNumBuf);
		if(err_q== OS_NO_ERR)
		{
			OSMemPut(p_msgBuffer,np);
		}
		OSSemPost(mem512_sem);
	}
	write_log(REMOTE_REC_TASK_ID, PRO_UPDATE_CMD);

}
*/
/*
void get_TimerCommand(void)
{
	uint8 err;
	uint8 *dp1;
	uint8 err_p,err_n;
	DATA_CONTROL_BLOCK *np;

		
	SetBaseTime(0);
	OSSemPend(mem512_sem,0,&err);
	dp1 = OSMemGet(mem512ptr,&err_n);
	np = OSMemGet(p_msgBuffer,&err_p);	
	if((err_p == OS_NO_ERR)&&(err_n == OS_NO_ERR))
	{
		np->type = NORMAL_ACK_MSG;
		np->count = RTRes.Packet_Length + 10;
							
		CopyBuffer(&RTRes,dp1,np->count);
		np->point = (uint8 *)dp1;
		if(OS_NO_ERR != OSQPost(RemoteQ,(void*)np)) 
		{
			OSMemPut(mem512ptr,dp1);
			OSSemPost(mem512_sem);
			OSMemPut(p_msgBuffer,np);
		}

	}
	else
	{
		if ( err_p == OS_NO_ERR )
		{
			OSMemPut(p_msgBuffer,np);
		}
		if ( err_n == OS_NO_ERR )
		{
			OSMemPut(mem512ptr,dp1);
		}
		OSSemPost(mem512_sem);
	}

}
*/
/*
void set_TimerCommand(uint8 *dp)
{
	uint8 err;
	uint8 *dp1;
	uint8 err_p,err_n;
	DATA_CONTROL_BLOCK *np;	

	CopyBuffer(&dp[8],TempUnion.u8.U8,4);   
	SetBaseTime(TempUnion.U32);
	OSTimeDly(OS_TICKS_PER_SEC/5);
	
	OSSemPend(mem512_sem,0,&err);
	dp1 = OSMemGet(mem512ptr,&err_n);
	np = OSMemGet(p_msgBuffer,&err_p);
	if((err_p == OS_NO_ERR)&&(err_n == OS_NO_ERR))
	{
		np->type = NORMAL_ACK_MSG;
		np->count = RTRes.Packet_Length + 10;
		
	
		CopyBuffer( &RTRes,dp1,np->count);
		np->point = (uint8 *)dp1;
		if(OS_NO_ERR != OSQPost(RemoteQ,(void*)np)) 
		{
			OSMemPut(mem512ptr,dp1);
			OSSemPost(mem512_sem);
			OSMemPut(p_msgBuffer,np);
		}
	}
	else
	{
		if ( err_p == OS_NO_ERR )
		{
			OSMemPut(p_msgBuffer,np);
		}
		if ( err_n == OS_NO_ERR )
		{
			OSMemPut(mem512ptr,dp1);
		}
		OSSemPost(mem512_sem);
	}
	//write_log(REMOTE_REC_TASK_ID,RECEIVE_TIMESYN_CMD);

}*/
/*
void set_ObjectIP(uint8 *dp)
{
	IPChangeRes *IPRes;
	uint16 temp16;
	uint8 err;
	uint8 *dp1, *dp2;
	uint8 err_p,err_n;
	DATA_CONTROL_BLOCK *np;	
	
	if ( (dp[2]+dp[3]*256) == SystemInfo.BasestationID )
	{
		//Save IP

		DelayNmS(100);
		OSTimeDly(OS_TICKS_PER_SEC/5); 
		
		OSSemPend(mem512_sem,0,&err);		
		dp1 = OSMemGet(mem512ptr,&err_n);
		np = OSMemGet(p_msgBuffer,&err_p);
		if((err_p == OS_NO_ERR)&&(err_n == OS_NO_ERR))
		{
			np->type = NORMAL_ACK_MSG;
			
			GPRS_Cfg.Remote_IP_1[0] = dp[16];					//������IP���˿�  
			GPRS_Cfg.Remote_IP_1[1] = dp[17];
			GPRS_Cfg.Remote_IP_1[2] = dp[18];
			GPRS_Cfg.Remote_IP_1[3] = dp[19];
			GPRS_Cfg.Remote_Port_1 = dp[14]+dp[15]*256;
			SaveGPRS_Information();
			IPRes = (IPChangeRes *)dp1;

			//Response to Center
			IPRes->Sync = 0x5AA5;
			IPRes->Netbasestation_ID = SystemInfo.BasestationID;
			IPRes->Frame_Type = 0x04;
			IPRes->Packet_Type = 0xAD;
			IPRes->Packet_Length = 13;
			IPRes->Command_Status = 0xFF;
			CopyBuffer(&dp[8],&IPRes->SIM_ID[0],6);
			IPRes->Port = GPRS_Cfg.Remote_Port_1;
			CopyBuffer(&dp[16],&IPRes->IP[0],4);
			
			temp16 = CRCW16((uint8 *)&IPRes,IPRes->Packet_Length+8);
			IPRes->CRC16_L = temp16>>0;
			IPRes->CRC16_H = temp16>>8;		
			np->count = IPRes->Packet_Length + 10;
			np->point = (uint8 *)dp1;
			if(OS_NO_ERR != OSQPost(RemoteQ,(void*)np)) 
			{
				OSMemPut(mem512ptr,dp1);
				OSSemPost(mem512_sem);
				OSMemPut(p_msgBuffer,np);
			}
		}
		else
		{
			if ( err_p == OS_NO_ERR )
			{
				OSMemPut(p_msgBuffer,np);
			}
			if ( err_n == OS_NO_ERR )
			{
				OSMemPut(mem512ptr,dp1);
			}
			OSSemPost(mem512_sem);
		}
		
		OSTimeDly(50);
		machine_state.comnetMode = NET_LOGIN_MODE;
		machine_state.LoginMode = NET_NORMAL_MODE;		

		dp2 = OSMemGet(p_msgBuffer,&err);
		if(err == OS_NO_ERR)
		{
			if(OS_NO_ERR != OSQPost(RemoteQ,(void*)dp2))
			{
				OSMemPut(p_msgBuffer,dp2);
			}
		}

		write_log(REMOTE_REC_TASK_ID,MODIFY_PORT_IP_CMD);
	}

}*/
/*
void set_TerminalReset(void)
{
	TerminalResetRes *TRRes;
	uint16 temp16;
	uint8 err;
	uint8 *dp1;
	uint8 err_p,err_n;
	DATA_CONTROL_BLOCK *np;	
	
	OSSemPend(mem512_sem,0,&err);	
	dp1 = OSMemGet(mem512ptr,&err_n);
	np = OSMemGet(p_msgBuffer,&err_p);
	if((err_p == OS_NO_ERR)&&(err_n == OS_NO_ERR))
	{								
		TRRes = (TerminalResetRes *)dp1;
		TRRes->Sync = 0x5AA5;
		TRRes->Netbasestation_ID = SystemInfo.BasestationID;
		TRRes->Frame_Type = 0x04;
		TRRes->Packet_Type = 0xAF;
		TRRes->Packet_Length = 13;
		TRRes->Command_Status = 0xFF;
		TRRes->Reset_Para = 0x00;
		Clear_Buffer(TRRes->Reserve1,10);
		temp16 = CRCW16((uint8 *)TRRes,TRRes->Packet_Length+8);
		TRRes->CRC16_L = temp16>>0;
		TRRes->CRC16_H = temp16>>8;

		np->type = NORMAL_ACK_MSG;
		np->count = TRRes->Packet_Length + 10;
		np->point = (uint8 *)dp1;
		if(OS_NO_ERR != OSQPost(RemoteQ,(void*)np))
		{											
			OSMemPut(mem512ptr,dp1);
			OSSemPost(mem512_sem);
			OSMemPut(p_msgBuffer,np);
		}
	}
	else
	{
		if ( err_p == OS_NO_ERR )
		{
			OSMemPut(p_msgBuffer,np);
		}
		if ( err_n == OS_NO_ERR )
		{
			OSMemPut(mem512ptr,dp1);
		}
		OSSemPost(mem512_sem);
	}		
	#if 1					
		#if REMOTE_DEBUG_EN
		stprintf("\r\n-- ϵͳ��λ --\r\n"); 
		#endif
		OSTimeDly(OS_TICKS_PER_SEC*2);
		OS_ENTER_CRITICAL();
		(*((void(*)())0))();
		while(1);
	#endif
}*/
/*
void NormalReceive(uint8 *sp,uint16 length)
{
	uint8 err;
	uint16 i;
	uint8 *dp;
	uint16 temp16;
	
	machine_state.no_ack_flag = FALSE;
	//timer_evnt_inf.no_ack_timer = NO_ACK_INTERVAL;
	
	for ( i=0;i<length;i++ )
	{
		if ( (sp[i+0]==0xa5)&&(sp[i+1]==0x5a) )
		{
			break;
		}
	}
	
	if(i == length)
		goto errout;

	OSSemPend(mem512_sem,0,&err);
	dp = OSMemGet(mem512ptr,&err); 
	if(OS_NO_ERR != err)
	{
		OSSemPost(mem512_sem);
		goto errout;
	}
	temp16 = sp[i+6] + sp[i+7]*256;
	if(temp16>100)
	{
		OSMemPut(mem512ptr,dp);
		OSSemPost(mem512_sem);
		goto errout;
	}
	CopyBuffer(&sp[i],dp,temp16+10);

	if((dp[temp16+8] + dp[temp16+9]*256)!=CRCW16(dp,temp16+8))
	{

		OSMemPut(mem512ptr,dp);
		OSSemPost(mem512_sem);
		goto errout;

	}

	switch ( dp[4] )
	{
		#if 0
		case 0xa0:				//�յ��Զ���������				
		{		
			if ( (dp[2]+dp[3]*256) == SystemInfo.BasestationID )						//�ж��Ƿ��Ǳ���վ
			{
				switch ( dp[5] )
				{
					case 0x01:															//Request To Updata
					{
						get_UpdataCommand(dp);
					}break;             

					default:			//Unknown
					{	
						//������ 
					}break;
				}
			}
			else   
			{
				#if REMOTE_DEBUG_EN
				stprintf("\r\n���³����վ�Ų�ƥ��!!\r\n");  
				#endif
			}
		
		}break;
		#endif
		case 0x02:				//��̨Ӧ���౨��		
		{
			#if REMOTE_DEBUG_EN
			stprintf("\r\n---��̨Ӧ��---");
			#endif
			
			switch ( dp[5] )
			{
				case 0x08:
				case 0xf6:		//����
				{	
					//machine_state.send_heart_times = 0;
					#if REMOTE_DEBUG_EN
					stprintf("<- ���� ->");	
					if ( dp[8]==0xff )			//�����ϱ��ɹ�  
					{
						stprintf("- ��ȷ -\r\n"); 
						machine_state.send_sucss_flag[0] = YES;
					}
					else 
					{
						stprintf("- ���� -\r\n"); 
						machine_state.send_sucss_flag[0] = NOT;
					}
					#endif

				}break;
				case 0x01:		//����
				{
					#if WEATHER_EN
						#if REMOTE_DEBUG_EN
						stprintf("<- ���� ->");
						#endif
						if ( dp[8]==0xff )			//�����ϱ��ɹ�  
						{
							stprintf("- ��ȷ -\r\n"); 
							machine_state.send_sucss_flag[weather] = YES;
						}
						else 
						{		
							machine_state.send_sucss_flag[weather] = NOT;
							stprintf("- ���� -\r\n");   
						}
					#endif

				}break;

				case 0x21:		//�¶�
				{	
					#if LINE_HEAT_EN
						#if REMOTE_DEBUG_EN
						stprintf("<--- �¶� --->");	
						#endif
						if ( dp[8]==0xff )			//�����ϱ��ɹ�  
						{
							stprintf("- ��ȷ -\r\n"); 
							machine_state.send_sucss_flag[lineTemp] = YES;
						}
						else 
						{
							machine_state.send_sucss_flag[lineTemp] = NOT;
							stprintf("- ���� -\r\n");   
						}
					#endif
				}break;
				
				case 0x5b:		//��ƫ
				{	
					#if WIND_ANGLE_EN
						#if REMOTE_DEBUG_EN
						stprintf("<--- ��ƫ --->");
						#endif
					
						if ( dp[8]==0xff )			//�����ϱ��ɹ�  
						{
							stprintf("- ��ȷ -\r\n"); 
							machine_state.send_sucss_flag[windAngle] = YES;
						}
						else 
						{
							machine_state.send_sucss_flag[windAngle] = NOT;
							stprintf("- ���� -\r\n");   
						}
					#endif
				}break;

				case 0x1f:		//��
				{	
					#if REMOTE_DEBUG_EN
					stprintf("<--- �� --->");
					if ( dp[8]==0xff )			//�����ϱ��ɹ�  
					{
						stprintf("- ��ȷ -\r\n"); 
						machine_state.send_sucss_flag[vibration] = YES;
					}
					else 
					{
						machine_state.send_sucss_flag[vibration] = NOT;
						stprintf("- ���� -\r\n");   
					}
					#endif
				}break;
				case 0x22:		//����
				{	
					#if ICE_EN	
						#if REMOTE_DEBUG_EN
						stprintf("<--- ���� --->");
						#endif
					if ( dp[8]==0xff )			//�����ϱ��ɹ�  
					{
						stprintf("- ��ȷ -\r\n"); 
						machine_state.send_sucss_flag[ice] = YES;
					}
					else 
					{
						machine_state.send_sucss_flag[ice] = NOT;
						stprintf("- ���� -\r\n");   
					}
					#endif
				}break;

				case 0x0c:		//��б
				{	
					#if TANGLE_START_EN
						#if REMOTE_DEBUG_EN
						stprintf("<--- ��б --->");
						#endif
					if ( dp[8]==0xff )			//�����ϱ��ɹ�  
					{
						stprintf("- ��ȷ -\r\n"); 
						machine_state.send_sucss_flag[towerAngle] = YES;
					}
					else 
					{
						machine_state.send_sucss_flag[towerAngle] = NOT;
						stprintf("- ���� -\r\n");   
					}
					#endif
				}break;
				
				default:	
				{										
				}break;
			}

		}break;
		
		case 0x03:				//��̨�����౨��							     
		{
			switch ( dp[5] )
			{
				case 0xab:
				{					
					get_TimerCommand();
				}break;

				case 0xac://--ʱ������(ʱ��ͬ��)����-- 
				{					
					#if REMOTE_DEBUG_EN
					stprintf("\r\n-- ʱ��ͬ�� --\r\n"); 
					#endif
					set_TimerCommand(dp);

				}break;
				
				case 0xad://--Զ������IP ����--
				{
					#if REMOTE_DEBUG_EN
					stprintf("\r\n-- Զ������ IP ���� --\r\n"); 
					#endif
					set_ObjectIP(dp);
				}break;

				case 0xaf://--------�ն˸�λ---------	
				{	
					if ((dp[8]==0)&&(dp[9]==0))
					{
						set_TerminalReset();	
					}
				}break;
				case 0xbc:				//�յ��Զ���������				
				{		
					if ( (dp[2]+dp[3]*256) == SystemInfo.BasestationID )						//�ж��Ƿ��Ǳ���վ
					{
						get_UpdataCommand(dp);
					}
					else   
					{
						#if REMOTE_DEBUG_EN
						stprintf("\r\n���³����վ�Ų�ƥ��!!\r\n");  
						#endif
					}
				
				}break;

				default:
				{
				}break;
			}
		}break;

		default:
		{
		}
		break;
	}				

		
	OSMemPut(mem512ptr,dp);
	OSSemPost(mem512_sem);
	return ;
errout:

	return;
	
}*/
const uint8 UpdataHeader[] = {"UpData"};
/*
void UpdataProgDeal(uint8 *sp,uint16 length)
{
	uint8 err,err_n,err_p;
	uint8 temp;
	uint16 i;
	uint8 *dp,*dp1;
	uint16 temp16;
	uint32 temp32,temp32i; 
	DATA_CONTROL_BLOCK *np;

	for ( i=0;i<length;i++ )
	{
		if ( (sp[i+0]==0xa5)&&(sp[i+1]==0x5a) )
		{
			break;
		}
	}
	
	if(i == length)
		goto errout;

	OSSemPend(mem512_sem,0,&err);
	dp = OSMemGet(mem512ptr,&err); 
	if(OS_NO_ERR != err)
	{
		OSSemPost(mem512_sem);
		return;
	}

	temp16 = sp[i+6] + sp[i+7]*256;
	if(temp16>100)
	{
		OSMemPut(mem512ptr,dp);
		OSSemPost(mem512_sem);
		goto errout;
	}
	
	CopyBuffer(&sp[i],dp,temp16+10);
	if( (dp[temp16+8] + dp[temp16+9]*256)==CRCW16(dp,temp16+8) )		//CRCУ��
	{
		switch ( dp[4] )
		{
			case 0xa0:													//�Զ���������				
			{		
				if ( (dp[2]+dp[3]*256) == SystemInfo.BasestationID )		//�ж��Ƿ��Ǳ���վ
				{
					timer_evnt_inf.UpdataTimeoutTimer = UPDATA_LIMIT_INTERVAL;
					switch ( dp[5] )
					{
						case 0x01:															//Request To Updata
						{
							#if REMOTE_DEBUG_EN
							stprintf("\r\n�յ�Զ�̸��³�������\r\n");
							#endif
							Clear_UpdataPara();
							
							UpdataRequest.Sync = 0x5aa5;				    
							UpdataRequest.Netbasestation_ID = SystemInfo.BasestationID;	
							UpdataRequest.Frame_Type = 0xA0;			
							UpdataRequest.Packet_Type = 0x02;		    
							UpdataRequest.Packet_Length = 12;		
							UpdataRequest.AllPackets_No = dp[8] + dp[9]*256;        
							Clear_Buffer(UpdataRequest.Reserve,10);	
							UpdataRequest.CRC16 = CRCW16((uint8 *)&UpdataRequest,UpdataRequest.Packet_Length+8);

							OSTimeDly(OS_TICKS_PER_SEC/10);
							OSSemPend(mem512_sem,0,&err);										
							dp1 = OSMemGet(mem512ptr,&err_n);
							np = OSMemGet(p_msgBuffer,&err_p);
							if((err_p == OS_NO_ERR)&&(err_n == OS_NO_ERR))
							{
								np->type = UPDATA_ACK_MSG;
								np->count = UpdataRequest.Packet_Length + 10;
								
						
								CopyBuffer( &UpdataRequest,dp1,np->count);
								np->point = (uint8 *)dp1;
								if(OS_NO_ERR != OSQPost(RemoteQ,(void*)np))
								{											
									OSMemPut(mem512ptr,dp1);
									OSSemPost(mem512_sem);
									OSMemPut(p_msgBuffer,np);
								}
							}
							else
							{
								if ( err_p == OS_NO_ERR )
								{
									OSMemPut(p_msgBuffer,np);
								}
								if ( err_n == OS_NO_ERR )
								{
									OSMemPut(mem512ptr,dp1);
								}
								OSSemPost(mem512_sem);
							}
							//BaseSentData((uint8 *)&UpdataRequest,UpdataRequest.Packet_Length+10);
							//machine_state.comnetMode=NET_UPDATA_PROG_MODE;

						}break;   
						
						case 0x03:															//Completed Updata
						{
							OS_ENTER_CRITICAL();
							AllUpdataNumber = dp[8] + dp[9]*256;
							UnCompUpNumber = AllUpdataNumber - CompUpNumCounter;
							OS_EXIT_CRITICAL();
							
							#if REMOTE_DEBUG_EN
							stprintf("\r\n���Զ�̸��³������ݰ����ͽ���~\r\n");
							stprintf("\r\nԶ�̸��³������ݰ��ܰ���");
							nprintf(AllUpdataNumber,0,DEC);
							stprintf("��������ȷ�յ�");
							nprintf(CompUpNumCounter,0,DEC);
							stprintf("����δ��ȷ��ɽ���");
							nprintf(UnCompUpNumber,0,DEC);
							stprintf("��\r\n");
							#endif

							PolishPocket.Sync = 0x5aa5;					
							PolishPocket.Netbasestation_ID = SystemInfo.BasestationID;		
							PolishPocket.Frame_Type = 0xa0;				
							PolishPocket.Packet_Type = 0x04;
							
							if ( UnCompUpNumber<=128 )			//δ��ɵİ����Դ˴���  
							{
								if ( UnCompUpNumber!=0 )		//��δ��ɵİ�
								{
									PolishPocket.Packet_Length = 12 + UnCompUpNumber*2;		    
									PolishPocket.ComplementPack_Sum = UnCompUpNumber;
									UnUpCounter = 0;
									OS_ENTER_CRITICAL();
									for ( i=0;i<AllUpdataNumber;i++ )  
									{
										temp = CompUpNumBuf[i/8];
										if ( (temp&(1<<(i%8)))==0 )
										{
											PlishUpPockBuf[10+2*UnUpCounter] = (i+1);
											PlishUpPockBuf[11+2*UnUpCounter] = (i+1)>>8;
											UnUpCounter += 1;
										}
										else
										{
										}
									}
									OS_EXIT_CRITICAL();
									CopyBuffer(&PolishPocket,PlishUpPockBuf,10);
									Clear_Buffer(&PlishUpPockBuf[10+PolishPocket.ComplementPack_Sum*2],10);
									temp16 = CRCW16(PlishUpPockBuf,PolishPocket.Packet_Length+8);
									PlishUpPockBuf[PolishPocket.Packet_Length+8] = temp16;
									PlishUpPockBuf[PolishPocket.Packet_Length+9] = temp16>>8;

									OSTimeDly(OS_TICKS_PER_SEC/10);
									OSSemPend(mem512_sem,0,&err);												
									dp1 = OSMemGet(mem512ptr,&err_n);
									np = OSMemGet(p_msgBuffer,&err_p);
									if((err_p == OS_NO_ERR)&&(err_n == OS_NO_ERR))
									{
										np->type = UPDATA_ACK_MSG;
										np->count = PolishPocket.Packet_Length + 10;
										
								
										CopyBuffer(&PlishUpPockBuf[0],dp1,np->count);
										np->point = (uint8 *)dp1;
										if(OS_NO_ERR != OSQPost(RemoteQ,(void*)np))
										{											
											OSMemPut(mem512ptr,dp1);
											OSSemPost(mem512_sem);
											OSMemPut(p_msgBuffer,np);
										}
									}
									else
									{
										if ( err_p == OS_NO_ERR )
										{
											OSMemPut(p_msgBuffer,np);
										}
										if ( err_n == OS_NO_ERR )
										{
											OSMemPut(mem512ptr,dp1);
										}
										OSSemPost(mem512_sem);
									}														
									//BaseSentData(PlishUpPockBuf,PolishPocket.Packet_Length+10);
								}
								else   
								{

									//---------���еĳ�����°�ȫ������--------

									PolishPocket.Packet_Length = 12;		    
									PolishPocket.ComplementPack_Sum = 0;

									CopyBuffer(&PolishPocket,PlishUpPockBuf,10);
									Clear_Buffer(&PlishUpPockBuf[10+PolishPocket.ComplementPack_Sum*2],10);
									temp16 = CRCW16(PlishUpPockBuf,PolishPocket.Packet_Length+8);
									PlishUpPockBuf[PolishPocket.Packet_Length+8] = temp16;
									PlishUpPockBuf[PolishPocket.Packet_Length+9] = temp16>>8;
									//BaseSentData(PlishUpPockBuf,PolishPocket.Packet_Length+10);

									OSTimeDly(OS_TICKS_PER_SEC/10);
									OSSemPend(mem512_sem,0,&err);												
									dp1 = OSMemGet(mem512ptr,&err_n);
									np = OSMemGet(p_msgBuffer,&err_p);
									if((err_p == OS_NO_ERR)&&(err_n == OS_NO_ERR))
									{
										np->type = UPDATA_ACK_MSG;
										np->count = PolishPocket.Packet_Length + 10;
										
								
										CopyBuffer(&PlishUpPockBuf[0],dp1,np->count);
										np->point = (uint8 *)dp1;
										if(OS_NO_ERR != OSQPost(RemoteQ,(void*)np)) 
										{											
											OSMemPut(mem512ptr,dp1);
											OSSemPost(mem512_sem);
											OSMemPut(p_msgBuffer,np);
										}
									
									}
									else
									{
										if ( err_p == OS_NO_ERR )
										{
											OSMemPut(p_msgBuffer,np);
										}
										if ( err_n == OS_NO_ERR )
										{
											OSMemPut(mem512ptr,dp1);
										}
										OSSemPost(mem512_sem);
									}
									OSTimeDly(OS_TICKS_PER_SEC*3);										
									//---------Test to View CompUpNumBuf[]-------------
									#if 0
									Uart0SendSTR("\r\n");  
									for ( i=0;i<256;i++ )
									{
										Uart0SendNUM(CompUpNumBuf[i],0,1);
										Uart0SendSTR("--");
									}
									stprintf("\r\n");
									#endif
									//--------------------------------------------------

									//-----------׼����������---------------------   
									
									flash_write_loader( PROG_CODE_START*1056,6,(uint8 *)UpdataHeader );			//�����ʶ
									UpDataDataLength = (AllUpdataNumber-1)*PackageLength + LastPocketLength;
									flash_write_loader( PROG_CODE_START*1056+6,4,(uint8 *)&UpDataDataLength );	//���泤�� 
									
									//--------------------For Test-----------------------
									//#if REMOTE_DEBUG_EN
									stprintf("\r\n���볤�ȣ�  ");
									flash_read_loader( PROG_CODE_START*1056+6,4,(uint8 *)&temp32 );
									nprintf(temp32,0,DEC);
									stprintf("\r\nCRC�����    "); 
									temp32 = crc32_loader(temp32);
									nprintf(temp32,0,DEC);
									//#endif
									//--------------------Test End----------------------

									flash_write_loader( PROG_CODE_START*1056+10,8,&dp[10] );					//����汾�ţ�CRCֵ
									//--------------------For Test-----------------------
									stprintf("\r\n�汾����Ϣ�� ");  
									flash_read_loader( PROG_CODE_START*1056+14,4,(uint8 *)&temp32i ); 
									OSMemPut(mem512ptr,PlishUpPockBuf);
									OSMemPut(mem160ptr,CompUpNumBuf);
									OSSemPost(mem512_sem);
																					
									if ( temp32i == temp32  )
									{
										stprintf("\r\n������ȷ����������~\r\n"); 
										write_log(REMOTE_TASK_ID,PRO_UPDATE_SUCCESS);				//Զ�̸��³ɹ���־
										//OSTimeDly(OS_TICKS_PER_SEC*2);		
										OS_ENTER_CRITICAL();
										(*((void(*)())0))();

										while(1);							//�ӿ��Ź���������    
									}
									else
									{
										write_log(REMOTE_TASK_ID,PRO_UPDATE_ERROR);				//Զ�̸���ʧ����־
										stprintf("\r\n�����д���\r\n");
										 
										OSMemPut(mem512ptr,PlishUpPockBuf);													
										OSMemPut(mem160ptr,CompUpNumBuf);
										OSSemPost(mem512_sem);
										
										machine_state.comnetMode = NET_LOGIN_MODE;
										machine_state.LoginMode = NET_UPDATA_PROG_MODE;
										dp1 = OSMemGet(p_msgBuffer,&err);
										if(err == OS_NO_ERR)
										{ 
											if(OS_NO_ERR != OSQPost(RemoteQ,(void*)dp1))
											{
												OSMemPut(p_msgBuffer,dp1);
											}
										}  
									}
								}
							}
							else											//δ��ɵİ���ִδ���             
							{
								PolishPocket.Packet_Length = 268;							    
								PolishPocket.ComplementPack_Sum = 256;    
								UnUpCounter = 0;
								for ( i=0;i<AllUpdataNumber;i++ )
								{
									temp = CompUpNumBuf[i/8]; 
									if ( (temp&(1<<(i%8)))==0 )  
									{
										PlishUpPockBuf[10+2*UnUpCounter] = (i+1);  
										PlishUpPockBuf[11+2*UnUpCounter] = (i+1)>>8; 
										
										if ( UnUpCounter==128 ) 
										{
											break; 
										}
										else
										{
											UnUpCounter += 1;  
										}
									}
									else 
									{
									}  
								}  
								CopyBuffer(&PolishPocket,PlishUpPockBuf,10);
								Clear_Buffer(&PlishUpPockBuf[10+PolishPocket.ComplementPack_Sum*2],10);
								temp16 = CRCW16(PlishUpPockBuf,PolishPocket.Packet_Length+8);
								PlishUpPockBuf[PolishPocket.Packet_Length+8] = temp16;
								PlishUpPockBuf[PolishPocket.Packet_Length+9] = temp16>>8;
								
								OSTimeDly(OS_TICKS_PER_SEC/10);	
								OSSemPend(mem512_sem,0,&err);											
								dp1 = OSMemGet(mem512ptr,&err_n);
								np = OSMemGet(p_msgBuffer,&err_p);
								if((err_p == OS_NO_ERR)&&(err_n == OS_NO_ERR))
								{
									np->type = UPDATA_ACK_MSG;
									np->count = PolishPocket.Packet_Length + 10;
									
							
									CopyBuffer(&PlishUpPockBuf[0],dp1,np->count);
									np->point = (uint8 *)dp1;
									if(OS_NO_ERR != OSQPost(RemoteQ,(void*)np))
									{
										OSMemPut(mem512ptr,dp1);
										OSSemPost(mem512_sem);
										OSMemPut(p_msgBuffer,np);
									}
								}
								else
								{
									if ( err_p == OS_NO_ERR )
									{
										OSMemPut(p_msgBuffer,np);
									}
									if ( err_n == OS_NO_ERR )
									{
										OSMemPut(mem512ptr,dp1);
									}
									OSSemPost(mem512_sem);
								}													
								//BaseSentData(PlishUpPockBuf,PolishPocket.Packet_Length+10);
							}
							
						}break;  

						case 0x05:															//Prog Data  
						{

							OS_ENTER_CRITICAL();											//���ⱻ��� 
							temp16 = dp[12] + dp[13]*256 - 1;					//��ǰ����			
							if ( (dp[10]==dp[12])&&(dp[11]==dp[13]) )	//���һ��  
							{
								LastPocketLength = dp[6]+dp[7]*256-6;
								flash_write_loader( (temp16%8)*PackageLength + (PROG_CODE_START+1+temp16/8)*1056,LastPocketLength,&dp[14] );	//���ܲ���PackageLength�ֽ�
							}
							else
							{
								flash_write_loader( (temp16%8)*PackageLength + (PROG_CODE_START+1+temp16/8)*1056,PackageLength,&dp[14] );					//ÿ��PackageLength�ֽ�
							}

							if ( (CompUpNumBuf[temp16/8]&(1<<(temp16%8)))==0 )	//���б�Ҫ
							{
								 CompUpNumCounter += 1;								//����ɵİ���  
							}
							CompUpNumBuf[temp16/8] |= (1<<temp16%8);				//����ɵİ���λ��
							OS_EXIT_CRITICAL();

							//-------------For Test View-------------
							#if REMOTE_DEBUG_EN
							stprintf("\r\n--��"); 
							nprintf(temp16+1,0,DEC);
							stprintf("��--");
							#if 0
							stprintf("\r\n--��"); 
							nprintf(temp16+1,0,DEC);
							stprintf("��--");
							#endif
							#endif
							//-------------Test End-----------------

						}break;

						default:
						{
						}break;
					}
				}
			}
		}
	}
	OSMemPut(mem512ptr,dp);
	OSSemPost(mem512_sem);
errout:
	
	return;
}*/


/*
#define SIM_Capation	40
uint8 DelCounter;
void Delete_SMS(void)
{
	uint8 err;
	uint8 *mp;
	
	if ( DelCounter>SIM_Capation )
	{
		DelCounter = 1;
	}
	else
	{
		DelCounter++;
	}
	
	//DelCounter = 1;
	
	//Uart1SendSTR("AT+CMGD=1\r\n");  
	Uart0SendSTR("\r\nɾ����:");
	Uart0SendNUM(DelCounter,0,0);
	Uart0SendSTR(" ������Ϣ\r\n");  

	OSSemPend(mem64_sem,0,&err);
	mp = OSMemGet(mem64ptr,&err);  
	if(OS_NO_ERR != err)
	{
		OSSemPost(mem64_sem);
		return;
	}

	datcpy(mp,(uint8*)"AT+CMGD=",8);    
	
	if ( DelCounter>=100 )
	{
		mp[8] = DelCounter/100+0x30;
		mp[9] = DelCounter%100/10+0x30;
		mp[10] = DelCounter%100%10+0x30; 
		mp[11] = '\r';
		mp[12] = '\n';
		mp[13] = '\0';
		Uart1SendSTR((char*)mp);
	}
	else if ( (DelCounter>=10)&&(DelCounter<=99) ) 
	{
		mp[8] = DelCounter/10+0x30;
		mp[9] = DelCounter%10+0x30;  
		mp[10] = '\r';
		mp[11] = '\n';  
		mp[12] = '\0';
		Uart1SendSTR((char*)mp);
	}
	else
	{
		mp[8] = DelCounter+0x30; 
		//mp[9] = '\r';
		//mp[10] = '\n';
		//mp[11] = '\0';
		mp[9] = ',';
		mp[10] = '0';
		mp[11] = '\r';
		mp[12] = '\n';
		mp[13] = '\0';
		Uart1SendSTR((char*)mp); 
	}
	OSMemPut(mem64ptr,mp);            
	OSSemPost(mem64_sem);
}*/

void Task_Remote(void *pdata)
{

	uint8 err;	
	DATA_CONTROL_BLOCK  *msg;
	//uint8 i;
	OSTimeDly(OS_TICKS_PER_SEC*2);
#if 0
	stprintf("\r\nGPRS��ʼ��");
	for(i=0;i<2;i++)
	{
		OSTimeDly(OS_TICKS_PER_SEC/4);
		if(LoginGPRS() == TRUE)
		{
			GPRS_LED2_OFF();
			goto POWERON_LOGIN_SUCCESS;	
		}
	}
	GPRS_LED2_ON();
	if(LogFlag.LoginlogsFlag==NOT )
	{
		LogFlag.LoginlogsFlag = YES;
		write_log(REMOTE_TASK_ID,LOGIN_ERR);
	}
POWERON_LOGIN_SUCCESS:
	#endif
	//machine_state.LoginMode = NET_NORMAL_MODE;


	while ( 1 )
	{
		msg = (DATA_CONTROL_BLOCK *)OSQPend(RemoteQ,0,&err);
		if(err == OS_ERR_NONE)
		{
			//switch (  machine_state.comnetMode )
			{

				//case NET_NORMAL_MODE:		//����ģʽ
				{
					switch ( msg->type )
					{
						//case REPORT_DATA_MSG: {	      ReportRecords(&machine_state.reportdatatype);}break;
						//case NORMAL_HEART_BEAT_MSG:{  NetSentHeartBeat();                        }break;
						#if GPRS_SMS
						//case DEL_SMS_MSG:	{  		  Delete_SMS();                                 }break;
						#endif
						//case NORMAL_ACK_MSG:	{  		  base_sent_ack((void*)msg);                                 }break;
						default:{																	}break;
					}
				}break;

				//case NET_UPDATA_PROG_MODE:
				//{
					//switch ( msg->type )
					//{
						//case UPDATA_HEART_BEAT_MSG:	{	NetSentUpHeartBeat();						    }break;
						//case UPDATA_ACK_MSG:		{	base_sent_ack((void*)msg);						    }break;
						//default:					{		                                        }break;
					//}				
				//}break;
				//case NET_LOGIN_MODE:
				//{					
					//break_net_power();
					//for(i=0;i<3;i++)
					//{
						//OSTimeDly(OS_TICKS_PER_SEC);
						//if( LoginGPRS() == TRUE )
						//{
							//GPRS_LED2_OFF();
							//goto LOGIN_SUCCESS; 
						//}
					//}
					//GPRS_LED2_ON();
					//if ( LogFlag.LoginlogsFlag==NOT )
					{
						//LogFlag.LoginlogsFlag = YES;
						//write_log(REMOTE_TASK_ID,LOGIN_ERR);
					}
					//if( machine_state.LoginMode == NET_UPDATA_PROG_MODE )
					//{
						//Uart0SendSTR("\r\n ERR");
						//OSMemPut(mem512ptr,PlishUpPockBuf);
						//OSMemPut(mem160ptr,CompUpNumBuf);
						//OSSemPost(mem512_sem);
						//machine_state.LoginMode = NET_NORMAL_MODE;
					//}
		//LOGIN_SUCCESS:
					//break;
		 		
				//}break;				
				/*default:					//�ǳ�̬����Ϊ����
				{
				}break;*/
	
			}

		}
		
		OSMemPut(p_msgBuffer,(void*)msg);
	}

}



