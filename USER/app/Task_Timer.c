#define IN_TASK_TIMER

#include "includes.h"


/*************************************************/
OS_EVENT *timerQueue;
extern RbtState gRbtState;
extern ZT_INFO_TYPE g_zt_msg;//robot��Ϣ�ṹ��

void Boot_HWDT_Feed(void)
{
    uint8 i;
	
	GpioSetH(GPIO_CTL_FED_WTDG);	      
	for ( i=0;i<70;i++ )
	{
		;
	}
	GpioSetL(GPIO_CTL_FED_WTDG);
}

void HWDT_Feed(void)
{
	GpioSetH(GPIO_CTL_FED_WTDG);	      
	OSTimeDly(10);
	GpioSetL(GPIO_CTL_FED_WTDG);
}

/*==================================

	������	Clear_Buffer
	˵����	����һ�λ�����
	��ڣ�	*pointer--Դָ��
			Length----���㳤��
	���ڣ�	��
	
===================================*/
void Clear_Buffer(void *pointer,uint16 length)
{
	uint16 i;
	uint8 *dp; 

	dp = (uint8 *)pointer;
	for (i=0;i<length;i++)
	{
		*(dp++) = 0x00;
	}
}

/*==================================

	������	CopyBuffer
	˵����	��Դ�ڴ�����Ŀ���ڴ���˳�򿽱�һ�����ȵ��ڴ��
	��ڣ�	*SouPointer--Դָ��
			*DesPointer--Ŀ��ָ��
			Length-------��������
	���ڣ�	����Ŀ������ָ��
	
===================================*/
void CopyBuffer(void *SouPointer,void *DesPointer,uint16 Length)
{
		
	uint16 i;
	uint8 *D_Pointer;
	uint8 *S_Pointer; 

	/*if ( (SouPointer==NULL)||(DesPointer==NULL) ) 
	{
		return;
	}*/

	if(DesPointer < SouPointer) 
	{
		D_Pointer = (uint8 *)DesPointer;
		S_Pointer = (uint8 *)SouPointer;
		for (i = 0; i < Length; i++ )
		{
			*D_Pointer++ = *S_Pointer++ ;
		}
	}
	else 
	{
		D_Pointer = ((uint8 *)DesPointer)+(Length - 1);
		S_Pointer = ((uint8 *)SouPointer)+(Length - 1);
		for(i = 0; i < Length; i++ )
		{
			*D_Pointer-- = *S_Pointer--;
		}
	}

	//return DesPointer;
}

void weather_timer()
{
	
}


void TimerProg(void)
{
	//uint8 err;
	//DATA_CONTROL_BLOCK  *dp;

	#if WEATHER_EN
	weather_timer();
	#endif
}

//��ص�ѹ����ص���  ADC�ɼ����˲��������
void DoBatUsingGetFilter(void)
{
    static u8  iVolId = 0;
    static u16 nVolValues[5]={0,0,0,0,0} ; 
    static u16 nVolAll=0; //
    //static u8  iCurId = 0;
    static u16 nCurValues[5]={0,0,0,0,0} ; 
    static u16 nCurAll=0; //
    
    if(iVolId>=5){ iVolId = 0; }
    nVolAll -= nVolValues[iVolId];
    nCurAll -= nCurValues[iVolId];
    nVolValues[iVolId] = get_adc_val(ADC_TYPE_ID_BAT_VOT);
    nCurValues[iVolId] = get_adc_val(ADC_TYPE_ID_BAT_CUR);
    nVolAll += nVolValues[iVolId];
    nCurAll += nCurValues[iVolId];
    gRbtState.nBatVol = nVolAll /5 ;
    gRbtState.nBatCur = nCurAll /5 ;
    iVolId++;
}

void SendWHLToLinux(void)
{
	u8 buf[14];
    signed long temp0;
	static unsigned long fn;
	
    fn++;

	//temp0=(signed long)(GET_SLAVE_WHEEL_CODE());
	temp0=gBatAutoCtrl.nCxCheckMove;
	//temp0=9876;///////////////////////////////////////test
	buf[0]=0xa5;
	buf[1]=0x5a;
	buf[2]=0x04;
	buf[3]=0x00;
	buf[4]=0xb1;
	buf[5]=(temp0&0x000000ff);
	buf[6]=(temp0&0x0000ff00)>>8;
	buf[7]=(temp0&0x00ff0000)>>16;
    buf[8]=(temp0&0xff000000)>>24;
	buf[9]=(fn&0x000000ff);
	buf[10]=(fn&0x0000ff00)>>8;
	buf[11]=(fn&0x00ff0000)>>16;
	buf[12]=(fn&0xff000000)>>24;
	buf[13]=0x96;

	ack_with_debug(ID_LINUX,buf,14);	
}

void Battery(void)
{
    //��������ڳ���У�gBatAutoCtrl.bCharging
    DoBatUsingGetFilter(); 
    if( gBatAutoCtrl.bCharging )
	{
        gBatAutoCtrl.iChargingCircle++;
        if( gBatAutoCtrl.iChargingCircle <= (4*60*5) )        //��������
		{
            if(!gRbtState.bChargeShort)
			{ 
				BAT_CHARGE_OPEN(); 
			}
        }
        else if( gBatAutoCtrl.iChargingCircle <= (4*60+10) )	//�������ӣ��Ͽ�2S��������ѹ
		{
            BAT_CHARGE_CLOSE();
        }
        else //if(gBatAutoCtrl.iChargingCircle >= 0)
		{
            gBatAutoCtrl.curBatVol = BAT_VOL_FLOAT*10;
            if( gBatAutoCtrl.curBatVol > gSlvMtCfg.bat_charge_full )	//���ڳ�����ѹ���Ͽ��̵�����ֹͣ���
			{
                gBatAutoCtrl.bCharging = false; 
				BAT_CHARGE_CLOSE();
            }
            gBatAutoCtrl.iChargingCircle = 0;			
        }
        
        if( !gRbtState.bChargeVolIn )					//�޵�ѹ���룬�Ͽ��̵�����ֹͣ���
		{
            gBatAutoCtrl.bCharging = false; 
			BAT_CHARGE_CLOSE();  						//�ⲿ����ر�
        }
    }
    else
	{
        gBatAutoCtrl.curBatVol = BAT_VOL_FLOAT*10;
    }

    g_zt_msg.nTimeForNoLinuxHeartIn++;
	
    //zt_build_send_state_string(BUILD_STATE_FLAG_ALL);//zs 1116 del
    //uart2_send(g_zt_msg.sendbuf , g_zt_msg.icmd_len );//zs 1116 del
	
    //uart1_send(g_zt_msg.sendbuf , g_zt_msg.icmd_len );
	
    //stprintf(ID_DEBUG,"send to linux=ok\r\n");

}

enum
{
    LINUX_CMD50_HEAD            = 0X00,						//֡ͷλ
    LINUX_CMD50_PKG_LEN         ,
};

/************************************************* 
*Function:	zt_msg_send_real_time_pkg 	
*Input:			
*OUTPUT:		    
*Return:		
*DESCRIPTION:��פ����Ҫ����Ϣ��װ ʵʱ���� ���
*************************************************/
void zt_msg_send_real_time_pkg()
{
    u8 oId = 2;
    g_zt_msg.sendbuf[ LINUX_CMD50_HEAD           ] = 0xE0;
    g_zt_msg.sendbuf[ oId++ ] = gSlaveMtAnaly.real_out_speed ;
    g_zt_msg.sendbuf[ oId++ ] = gMstMt.real_out_speed ;
    g_zt_msg.sendbuf[ oId++ ] = gMtDisControlCheck.mstMtBackSpeed.lvVal >>8; //gSpeedAnaly_Mst.speed>>8 ;
    g_zt_msg.sendbuf[ oId++ ] = gMtDisControlCheck.mstMtBackSpeed.lvVal ;//gSpeedAnaly_Mst.speed&0X0FF ;
    g_zt_msg.sendbuf[ oId++ ] = get_adc_val(ADC_TYPE_ID_BAT_CUR)>>4; //gSpeedAnaly_Slv.speed>>8  ;
    g_zt_msg.sendbuf[ oId++ ] = gSpeedAnaly_Slv.speed&0X0FF ;
    g_zt_msg.sendbuf[ oId++ ] = get_adc_val(ADC_TYPE_ID_PRESS1) >>5 ;  //ѹ���ɼ�ԭʼֵ
    g_zt_msg.sendbuf[ oId++ ] = gPressFilter.val ;   //ѹ���˲�ֵ
    
    g_zt_msg.sendbuf[ oId++ ] = 0xFA;
    g_zt_msg.sendbuf[ LINUX_CMD50_PKG_LEN        ] = oId ;

	//uart1_send(g_zt_msg.sendbuf , oId);  
}

uint8 StringLen(char * Pointer)
{
	uint8 i=0;
	
	//CopyBuffer(Pointer,&me.buffer[0],5);
	while((*Pointer++) != '\0')
	{
		i++;
	}
	//me.len= i;
	return i;
}

void debug_sprintf(uint8 com,char *str)
{
	 uint8 err;
	 char *dp;
	 DATA_CONTROL_BLOCK *dp1;

	 
	 dp = (char *)OSMemGet(mem160ptr,&err);
	 if(err != OS_ERR_NONE)
	 {	
		 return;
	 }
	 dp1 = OSMemGet(p_msgBuffer,&err);
	 if(err != OS_ERR_NONE)
	 {
	 	OSMemPut(mem160ptr,dp);
		 return;
	 }
	 
	 CopyBuffer(str, dp, StringLen(str));
	 if(com==ID_DEBUG)
 	 {
		 dp1->type = WEATHER_DEBUG_OUT_MSG_DEBUG;
 	 }
	 else if(com==ID_LINUX)
 	 {
		 dp1->type = WEATHER_DEBUG_OUT_MSG_LINUX;
 	 }
     else if(com==ID_RF433)
 	 {
		 dp1->type = WEATHER_DEBUG_OUT_MSG_RF433;
 	 }
	 dp1->point =  (uint8 *)dp;
	 dp1->count =  StringLen(str);
	 if(OS_ERR_NONE != OSQPost(GetWeatherQueue,(void*)dp1))
	 {
         //GpioSetL(GPIO_LED_OUT_SHOW1);
		 OSMemPut(mem160ptr,dp);
		 OSMemPut(p_msgBuffer,dp1);
	 }
     
	 
}

void debug_nprintf(uint8 com,uint32 num,uint8 mul,uint8 typ)
{
	 uint8 err;
	 uint32 i;
	 char *cp,*dp;
	 DATA_CONTROL_BLOCK *dp1;
	 dp = (char *)OSMemGet(mem160ptr,&err);
	 if(err != OS_ERR_NONE)
	 {		
		 return;
	 }
	  
	 dp1 = OSMemGet(p_msgBuffer,&err);
	 if(err != OS_ERR_NONE)
	 {
	 	 OSMemPut(mem160ptr,dp);
		 return;
	 }

	cp = dp;
	switch(typ)
	{		 
		case 0:						  //ʮ������ʾ
		for(i=1000000000;i>1;i/=10)	  //ת����ascll��
		{
		  *cp=num/i+'0';
		  num%=i;
		  cp++; 
		} 
		*cp=num+'0';
		cp++;
		*cp='\0';					  //���ַ���������־
		if(mul)
		{
		   for(i=0;i<mul+1;i++)
		   {
			   *(cp+1)=*cp;
			   cp--;
		   }
		   *(cp+1)='.'; 			 //���ݱ������С����
		}
		cp=dp;
		while(*cp!='\0') 			  //ȥ����ͷ�� 0
		{
		   if(*cp!='0') break;
		   cp++;
		}
		if( *cp=='.'|| *cp=='\0')  cp--;
	  CopyBuffer(cp,dp,StringLen(cp));
		
		if(com==ID_DEBUG)
	 	{
			dp1->type = WEATHER_DEBUG_OUT_MSG_DEBUG;
	 	}
		else if(com==ID_LINUX)
	 	{
			dp1->type = WEATHER_DEBUG_OUT_MSG_LINUX;
	 	}
	    else if(com==ID_RF433)
	 	{
			dp1->type = WEATHER_DEBUG_OUT_MSG_RF433;
	 	}
		dp1->point =  (uint8 *)dp;
		dp1->count =  StringLen(cp);
		
		if(OS_ERR_NONE != OSQPost(GetWeatherQueue,(void*)dp1))
		{
			 OSMemPut(mem160ptr,dp);
			 OSMemPut(p_msgBuffer,dp1);
		}
		break;
		case 1:
		i=0x10000000;
		if(num<65536) i=0x1000;
		if(num<256)	 i=0x10;
		for( ;i>1;i>>=4) 			 //ת����ascll��
		{
		  *cp=num/i;
		  if(*cp>9) *cp+='a'-10;
		  else		*cp+='0';
		  num%=i;
		  cp++; 
		}
		*cp=num;
		if(*cp>9) *cp+='a'-10;
		else 	 *cp+='0';
		cp++;
		*cp='\0';					  //���ַ���������־
		cp=dp;
		CopyBuffer(cp,dp,StringLen(cp));
		if(com==ID_DEBUG)
	 	{
			dp1->type = WEATHER_DEBUG_OUT_MSG_DEBUG;
	 	}
		else if(com==ID_LINUX)
	 	{
			dp1->type = WEATHER_DEBUG_OUT_MSG_LINUX;
	 	}
	    else if(com==ID_RF433)
	 	{
			dp1->type = WEATHER_DEBUG_OUT_MSG_RF433;
	 	}
		dp1->point =  (uint8 *)dp;
		dp1->count =  StringLen(cp);
		if(OS_ERR_NONE != OSQPost(GetWeatherQueue,(void*)dp1))
		{
			OSMemPut(mem160ptr,dp);
			OSMemPut(p_msgBuffer,dp1);		
		}
		break;
		default:
		break;
	 
	 }
}

void Task_Timer(void *pdata)
{
	uint8 err;
	DATA_CONTROL_BLOCK *msg;
	static s32 nCxCheckMove;
	//static u8 test;//������
	
	//OSTimeDly(OS_TICKS_PER_SEC);

	while( 1 )
	{
		msg = (DATA_CONTROL_BLOCK *)OSQPend(timerQueue,0,&err);
		
		if ( err == OS_ERR_NONE )
		{
			switch ( msg->type )
			{
				case BATTERY_MSG:
				{
					//GpioSetL(GPIO_LED_SHOW1);
					//GpioSetL(GPIO_LED_OUT_SHOW1);
					Battery();					
				}
				    break;
                case WHL_MSG:
				{
					//GpioSetL(GPIO_LED_SHOW1);
					//GpioSetL(GPIO_LED_OUT_SHOW1);
                    //test++;//������
					//if((test&0x07)==0)//������
					//if(((nCxCheckMove-gBatAutoCtrl.nCxCheckMove)>8)||((gBatAutoCtrl.nCxCheckMove-nCxCheckMove)>8))
					if(nCxCheckMove!=gBatAutoCtrl.nCxCheckMove)
                    {
                        nCxCheckMove=gBatAutoCtrl.nCxCheckMove;
						SendWHLToLinux();
					}					
				 }
				    break;
				default:
				    break;
			 }		
			OSMemPut(p_msgBuffer,(void*)msg);
		}	
	}

}

