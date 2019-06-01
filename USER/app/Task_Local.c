#define IN_TASK_LOCAL
#include "includes.h"

ZT_INFO_TYPE g_zt_msg;
extern ZT_INFO_TYPE g_zt_msg2;
int  gCmdParaVal_Ints[8];
float gCmdParaVal_floats[3];
extern RbtState gRbtState;
CmdParaInfo gCmdParaInfos;
CamSet GTZMHD_Set;
CamGet GTZMHD_Get;
extern AtCmdInfo AtCmdFromGPS;
char Frame_No_Debug[4];
char Frame_No_Linux[4];
char Frame_No_Rf433[4];

/************************************************* 
*Return:     
*DESCRIPTION: 发送消息到发送命令的串口和调试串口
*************************************************/
void ack_with_debug(uint8 com,uint8 *buf, uint16 len)
{
	if( com== ID_DEBUG)
	{
        uart1_send(buf,len);
	}
	else if( com== ID_LINUX)
	{
        uart2_send(buf,len);
	}
	else if( com== ID_RF433)
	{
        uart3_send(buf,len);
	}
}

static void at_build_mtSetBk_string(uint8 com,uint8 rst,uint8 kind)
{
    char* str;
	//const char* const str_temp_unit = "℃";
    str = (char*)(g_zt_msg.sendbuf) ;
    //str = cmd_para_build_para_string_str( str , "AT+MstMt" , "1" );
    if(kind==1)
    {
		//stprintf(com,"AT+MstMt true ");
		str = cmd_para_build_para_string_str( str , "AT+MstMt " , "1" );
    }
	else
	{
		//stprintf(com,"AT+SlvMt true ");
		str = cmd_para_build_para_string_str( str , "AT+SlvMt " , "1" );
    }	

	str--;//去掉'=';
	str--;//去掉'1';
	str--;//去掉'  ';
	if(rst==true)
	{
	    str = cmd_para_build_para_string_str( str , "code" , "0" );
    }
	else
	{
		str = cmd_para_build_para_string_int( str , "code" , rst );
	}
	
    str = cmd_para_build_para_string_int( str , "MstMt" , gMstMt.set_speed );
    if ( gSlaveMtAnaly.s_SlvMtState == SLAVE_MT_CTRL_AUTO )
    {
        str = cmd_para_build_para_string_str( str , "SlvMt" , "auto" );
    } 
    else
    {
        str = cmd_para_build_para_string_int( str , "SlvMt" , gSlaveMtAnaly.real_out_speed );
    }

    if(com==ID_DEBUG)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Debug);
    }
	else if(com==ID_LINUX)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Linux);
    }
	else if(com==ID_RF433)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Rf433);
    }
	
	str = cmd_para_build_para_string_str(str , "Name" , gRbtState.RobotName);
	
    *str++ = '\r';
    *str++ = '\n';
    *str   = ' ';//0;
    
    g_zt_msg.icmd_len = str - (char*)(g_zt_msg.sendbuf);
    ack_with_debug(com, g_zt_msg.sendbuf , g_zt_msg.icmd_len );
		
}

static void at_build_PowerCtl_string(uint8 com,uint8 kind)
{
    char* str = (char*)(g_zt_msg.sendbuf);

    if(kind==1)//开设备
    {
	    if(box_str_cmp( gCmdParaInfos.items[0].value , "All" )== 0)
		{
			str = cmd_para_build_para_string_str( str , "AT+OpenDev" , "All" );
		}
		else
		{
			str = cmd_para_build_para_string_str( str , "AT+OpenDev" , gCmdParaInfos.items[0].value);
		}

		if( OpenOrCloseDevOperate(gCmdParaInfos.items[0].value,true) )
	    { 
	        str = cmd_para_build_para_string_str( str , "code" , "0" );
	    }
	    else
	    {
	    	str = cmd_para_build_para_string_str( str , "code" , "7" );
	    }
    }
	else//关设备
    {
	    if(box_str_cmp( gCmdParaInfos.items[0].value , "All" )== 0)
		{
			str = cmd_para_build_para_string_str( str , "AT+CloseDev" , "All" );
		}
		else
		{
			str = cmd_para_build_para_string_str( str , "AT+CloseDev" , gCmdParaInfos.items[0].value);
		}

		if( OpenOrCloseDevOperate(gCmdParaInfos.items[0].value,false) )
	    { 
	        str = cmd_para_build_para_string_str( str , "code" , "0" );
	    }
	    else
	    {
	    	str = cmd_para_build_para_string_str( str , "code" , "7" );
	    }
    }
		
    if(com==ID_DEBUG)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Debug);
    }
	else if(com==ID_LINUX)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Linux);
    }
	else if(com==ID_RF433)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Rf433);
    }
	str = cmd_para_build_para_string_str(str , "Name" , gRbtState.RobotName);
	
    *str++ = '\r';
    *str++ = '\n';
    *str   = 0;
    g_zt_msg.icmd_len = str - (char*)(g_zt_msg.sendbuf) ;

    ack_with_debug(com, g_zt_msg.sendbuf , g_zt_msg.icmd_len );
		
}




uint8 CheckIsCmdOk(uint8 com, __IN__ const uint8* cmd, __OUT_ CmdParaInfo* pParaInfo)
{
	uint16 hex_u16,cal_u16,cal_u16_znsh;			//cal_u16_znsh为兼容蒲洋编写的智能手环的软件
	//uint8 i;
	
	//stprintf(com,"\r\npParaInfo->item_cnt=");
	//nprintf(com,pParaInfo->item_cnt,0,DEC);
	//stprintf(com,"\r\n");
	
	if ( pParaInfo->item_cnt < 2 )
    {
		//GpioSetL(GPIO_LED_SHOW1);
		return false;
    }
    //判断有无CRC校验值

    if ( box_str_cmp( pParaInfo->items[pParaInfo->item_cnt-1].name , "CRC" ) )
    {
		//GpioSetL(GPIO_LED_SHOW1);
		return false;
    }
    else
    {  
		/*stprintf(1,"解析后的各值");
		for(i=0;i<pParaInfo->item_cnt;i++)
    	{
			stprintf(1, pParaInfo->items[i].name);
			nprintf(1,(uint16)box_str_hex_to_int(pParaInfo->items[i].value),0,HEX);
			stprintf(1," ");
    	}
		stprintf(1,"\r\n")*/
		 
		hex_u16 = box_str_hex_to_int( pParaInfo->items[pParaInfo->item_cnt-1].value );
        cal_u16 = GetCRC16( (unsigned char*)(cmd)+3 , pParaInfo->items[pParaInfo->item_cnt-1].item_begin_pos-1 );
		cal_u16_znsh = GetCRC16( (unsigned char*)(cmd) , pParaInfo->items[pParaInfo->item_cnt-1].item_begin_pos );


		if (( hex_u16 == cal_u16 )||(hex_u16 == cal_u16_znsh))
        {
        
        }
		else
        {
            char crc_strs2[8];
            char crc_strs1[8];
            box_str_u16_to_hex( crc_strs1 , hex_u16 );
            box_str_u16_to_hex( crc_strs2 , cal_u16 );
			
            stprintf(com,"\r\nCRC Err:rev=");
            nprintf(com,hex_u16,0,HEX);
			stprintf(com," CRC true=");
            nprintf(com,cal_u16,0,HEX);
			stprintf(com,"\r\n");
			
			//stprintf(ID_DEBUG,"\r\nCRC Err:接收值=");
            //nprintf(ID_DEBUG,hex_u16,0,HEX);
			//stprintf(ID_DEBUG,"  计算值=");
            //nprintf(ID_DEBUG,cal_u16,0,HEX);
			//stprintf(ID_DEBUG,"\r\n");
			
            return false;
        }
    }
    return true;
}

/************************************************* 
*Function:	zt_build_send_state_string 	
*Return:		
*DESCRIPTION: 发送机器人的状态字符串
*************************************************/
int zt_build_send_state_string(u32 stateFlag,uint8 id,uint8 kind)
{
    int vals[6];
	//static long code_ab[2];
	  uint8 com=id;
	//const char* const str_temp_unit = "℃";
    char* str = (char*)(g_zt_msg.sendbuf) ;
	if(kind==8)//AT+Hello命令
	{
		str = cmd_para_build_para_string_int( str , "AT+RunTime" , (OSTimeGet())/OS_TICKS_PER_SEC );
	}
	else//AT+GetState命令
	{
		//stprintf(com,"AT+GetState ");
		str = cmd_para_build_para_string_str( str , "AT+GetState " , "1" );
		str--;//去掉'=';
		str--;//去掉'1';
		str--;//去掉'  ';
		str = cmd_para_build_para_string_int( str , "RunTime" , (OSTimeGet())/OS_TICKS_PER_SEC );
    }	
	//str = (char*)(g_zt_msg.sendbuf) ;

    //str = cmd_para_build_para_string_str( str , "Name" , gRbtState.RobotName );
    
    //str = cmd_para_build_para_string_int( str , "STATE" , gBoxOs_CpuIdleTime/1000 );

    if ( g_zt_msg.nTimeForNoLinuxHeartIn > 30 )
    {
        str = cmd_para_build_para_string_str( str , "linux" , "ERR" );
    }

    if( stateFlag & BUILD_STATE_FLAG_BAT )
	{
        //str = cmd_para_build_para_string_int( str , "bat" , get_adc_val(ADC_TYPE_ID_BAT_VOT)*2.5f/4096 );
        str = cmd_para_build_para_string_float( str , "bat" , gBatAutoCtrl.curBatVol*0.1f , 1 );//BAT_VOL_FLOAT
        str--; 
		//*str++ = 'V';    
		*str++ = ' ';//'\t';添加一个字符 空格 
        str = cmd_para_build_para_string_float( str , "cur" , BAT_CUR_FLOAT , 2 );
        str--; 
		//*str++ = 'A';    
		*str++ = ' ';//'\t';添加一个字符 空格
    }
    if( stateFlag & BUILD_STATE_FLAG_PRESS )
	{
        str = cmd_para_build_para_string_int( str , "press" , (int)(gPressFilter.val*2.34375+0.5) );
    }
    if( stateFlag & BUILD_STATE_FLAG_AM2320 )
	{
        //zs//str = cmd_para_build_para_string_float( str , "temp" , AM2320_get_temper()*0.1f , 1 );
        str = cmd_para_build_para_string_float( str , "temp" ,250*0.1f , 1 );
        str--; 
		//*str++ = '^';  *str++ = 'C';  
		*str++ = ' ';//'\t';添加一个字符 空格
        //zs//str = cmd_para_build_para_string_float( str , "humi" , AM2320_get_humidi()*0.1f , 1 );
        str = cmd_para_build_para_string_float( str , "humi" , 600*0.1f , 1 );
        str--; 
		//*str++ = '%';    
		*str++ = ' ';//'\t'; //添加一个字符 空格 
    }
    //if( stateFlag & BUILD_STATE_FLAG_JY901 ){
        //CMD_PARA_VAL_TRANS_3 ( g_zt_msg.trans_vals , gJy901Data.accl );
        //str = cmd_para_build_para_string_ints( str , "accl" , g_zt_msg.trans_vals , 3 );
        //CMD_PARA_VAL_TRANS_3 ( g_zt_msg.trans_vals , gJy901Data.angle );
        //str = cmd_para_build_para_string_ints( str , "angle" , g_zt_msg.trans_vals , 3 );
    //}
    if( stateFlag & BUILD_STATE_FLAG_MSTMT )
	{
        //主动轮电机相关信息
        g_zt_msg.trans_vals[0] = gMstMt.set_speed ;  //设置速度
        g_zt_msg.trans_vals[1] = gMstMt.real_out_speed ; //实时速度
        //g_zt_msg.trans_vals[2] = gMstMt.left_code ;          //剩余运动编码器值
        str = cmd_para_build_para_string_ints( str , "MstSet" , g_zt_msg.trans_vals , 2 );
        //g_zt_msg.trans_vals[0] = GET_MASTER_MOTOR_CODE() ;   //主动轮电机编码器值
        g_zt_msg.trans_vals[0] = get_adc_val(ADC_TYPE_ID_MW_SP) ;   //速度电压值
        g_zt_msg.trans_vals[1] = gSpeedAnaly_Mst.speed ;   //编码器速度
        str = cmd_para_build_para_string_ints( str , "MstState" , g_zt_msg.trans_vals , 1 );
    }
    if( stateFlag & BUILD_STATE_FLAG_SLVMT )
	{
        //从动轮电机 相关信息
        str = cmd_para_build_para_string_str( str , "SlvSet" , (gSlaveMtAnaly.s_SlvMtState == SLAVE_MT_CTRL_AUTO)? "AUTO" : "HANDLE" ); //从 电机 设置速度
        g_zt_msg.trans_vals[0] = gSlaveMtAnaly.real_out_speed ; //从机当前速度
        //g_zt_msg.trans_vals[2] = GET_SLAVE_MOTOR_CODE() ;      //从机编码器值
        g_zt_msg.trans_vals[1] = get_adc_val(ADC_TYPE_ID_SW_SP) ;   //速度电压值
        g_zt_msg.trans_vals[2] = gSpeedAnaly_Slv.speed ;        //从机速度
        str = cmd_para_build_para_string_ints( str , "SlvState" , g_zt_msg.trans_vals , 3 );
    }
    if( stateFlag & BUILD_STATE_FLAG_SLVWH )
	{
        //从动轮轮子 相关信息
        g_zt_msg.trans_vals[0] = (signed short)(GET_SLAVE_WHEEL_CODE());
        g_zt_msg.trans_vals[1] = gBatAutoCtrl.nCxCheckMove; //gSpeedAnaly_Whl.speed ;
        //g_zt_msg.trans_vals[1] = gCodeZ+1;
        str = cmd_para_build_para_string_ints( str , "SlvWhl" , g_zt_msg.trans_vals , 2 );
        
        //code_ab[0] = (GET_SLAVE_WHEEL_CODE());
		//code_ab[1] =gBatAutoCtrl.nCxCheckMove;
		//str = cmd_para_build_para_string_longs( str , "SlvWhl" , &code_ab[0] , 2 );
    }
    if( stateFlag & BUILD_STATE_FLAG_XWSW )
	{
        //限位开关
        g_zt_msg.trans_vals[0] = gSlaveMtAnaly.b_XianWei_Down ;
        g_zt_msg.trans_vals[1] = gSlaveMtAnaly.b_XianWei_Up ;
        str = cmd_para_build_para_string_ints( str , "XianWei" , g_zt_msg.trans_vals , 2 );
    }
    if( stateFlag & BUILD_STATE_FLAG_CHARGE )
	{
        //if( !gBatAutoCtrl.bCharging ){
        //    g_zt_msg.trans_vals[0] = gRbtState.bCX_SwCheck1||gRbtState.bCX_SwCheck2 ;
        //    g_zt_msg.trans_vals[1] = gBatAutoCtrl.bChargingClosed ;
        //    g_zt_msg.trans_vals[2] = gRbtState.bChargeVolIn ;
        //    g_zt_msg.trans_vals[3] = gRbtState.bChargeShort ;
        //    str = cmd_para_build_para_string_ints( str , "Charge" , g_zt_msg.trans_vals , 4 );
        //}
        //else{
        //    g_zt_msg.trans_vals[0] = gRbtState.bChargeVolIn ;
        //    g_zt_msg.trans_vals[1] = gRbtState.bChargeShort ;
        //    g_zt_msg.trans_vals[2] = !GpioGet(GPIO_BAT_CHARGE_EN) ;
        //    str = cmd_para_build_para_string_ints( str , "Charging" , g_zt_msg.trans_vals , 3 );
        //}
        str = box_str_cpy_rt_pos(str,"Charge=");
        *str++ = (gRbtState.bCX_SwCheck1||gRbtState.bCX_SwCheck2)? 'B' : '_' ; //巢穴检测
        *str++ = ',';
        *str++ = (gRbtState.bChargeVolIn)? 'I' : '_' ; //充电输入检测
        *str++ = ',';
        *str++ = (gRbtState.bChargeShort)? 'D' : '_' ; //充电短路检测
        *str++ = ',';
        *str++ = (gBatAutoCtrl.bOnBridge)? 'Q' : '_' ; //软件判断，正在桥上
        *str++ = ',';
        *str++ = (gBatAutoCtrl.bCharging)? 'C' : '_' ; //软件判断，充电中
        *str++ = ',';
        *str++ = (gBatAutoCtrl.bChargingClosed)? 'X' : '_' ; //软件判断，手动关闭充电中
        *str++ = ',';
        *str++ = (!GpioGet(GPIO_BAT_CHARGE_EN))? 'O' : '_' ; //实时输出
        *str++ = ' ';//'\t' ;
        *str = 0 ;
    }
    if( stateFlag & BUILD_STATE_FLAG_GPS )
	{
        if( gRbtState.GpsPosState[0] == 0 )
		{
            gRbtState.GpsPosState[0] = '?' ; 
			gRbtState.GpsPosState[1] = 0 ;
        }
        if( gRbtState.GpsTimeState[0] == 0 )
		{
            gRbtState.GpsTimeState[0] = '?' ; 
			gRbtState.GpsTimeState[1] = 0 ;
        }
        
    }
    
#if 0
    //测试过程中
    g_zt_msg.trans_vals[0] = gSlaveMtAnaly.ys_to_set_speed*((SLV_DIR_TO_PRESS_UP==gSlaveMtAnaly.ys_to_set_dir)?1:-1) ;
    g_zt_msg.trans_vals[1] = gSlaveMtAnaly.ys_cur_real_speed*((SLV_DIR_TO_PRESS_UP==gSlaveMtAnaly.ys_cur_real_speed)?1:-1) ;
    g_zt_msg.trans_vals[2] = gSlaveMtAnaly.ys_need_close_delay ;
    //g_zt_msg.trans_vals[3] = gRbtState.bChargeShort ;
    str = cmd_para_build_para_string_ints( str , "Test" , g_zt_msg.trans_vals , 3 );
#endif

    //GpsPos=116.1479415,33.7518119,61.4	GpsTime=17,11,20,14,0,26
    //str = cmd_para_build_para_string_str( str , "GpsPos" , gRbtState.GpsPosState );
    //str = cmd_para_build_para_string_str( str , "GpsTime" , gRbtState.GpsTimeState );
    str = box_str_cpy_rt_pos(str,"GpsPos");
    *str++ = '=' ;

    //供妙算解析用
    /*gGPS.bCurGetPos=1;
	gGPS.f_lng =117.12345678;
	gGPS.f_lat=39.87654321;
	gGPS.f_height=60.0;
    gGPS.bCurGetTime=1;
    gGPS.gpsTime_year=118;
	gGPS.gpsTime_mon=11;
	gGPS.gpsTime_mday=21;
	gGPS.gpsTime_hour=14;
	gGPS.gpsTime_min=40;
	gGPS.gpsTime_sec=38;
	*/
	
    if ( gGPS.bCurGetPos )
    {
        str = float_to_str( str ,  gGPS.f_lng , 7  );
        *str++ = ',' ;
        str = float_to_str( str ,  gGPS.f_lat , 7  );
        *str++ = ',' ;
        str = float_to_str( str ,  gGPS.f_height , 1  );
    } 
    else
    {
        str = box_str_cpy_rt_pos(str,"NoPos");  //未定位
    }
    *str++ = ' ';//'\t' ;

    if ( gGPS.bCurGetTime )
    {
        vals[0] = gGPS.gpsTime_year - 100 ;
        vals[1] = gGPS.gpsTime_mon + 1 ;
        vals[2] = gGPS.gpsTime_mday ;
        vals[3] = gGPS.gpsTime_hour ;
        vals[4] = gGPS.gpsTime_min ;
        vals[5] = gGPS.gpsTime_sec ;
        str = cmd_para_build_para_string_ints( str , "GpsTime" , vals , 6 );
    }
    else
    {
        str = cmd_para_build_para_string_str( str , "GpsTime" , "NoTime" );
    }

    vals[0] = GTZMHD_Get.directAngle/100;
    vals[1] = GTZMHD_Get.pitchAngle/100 ;
    vals[2] = 0;
    vals[3] = GTZMHD_Get.nVisibleFocalDis/5;
	str = cmd_para_build_para_string_ints( str , "PTZ" , vals , 4 );
  
    str = cmd_para_build_para_string_str( str , "type" , "Robot" );
	
	if(kind!=8)				//Hello指令不发FN
	{
	    if(com==ID_DEBUG)
	    {
	    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Debug);
	    }
		else if(com==ID_LINUX)
	    {
	    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Linux);
	    }
		else if(com==ID_RF433)
	    {
	    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Rf433);
	    }
	}
	str = cmd_para_build_para_string_str(str , "Name" , gRbtState.RobotName);
	//*str++ =  ' ';
	str = cmd_para_build_para_string_str(str , "StateSw" , gRbtState.StateSwitchFlag);
    
    *str++ = '\r';
    *str++ = '\n';
    *str   = 0;
	
    g_zt_msg.icmd_len = str - (char*)(g_zt_msg.sendbuf) ;
    //nprintf(1, g_zt_msg.icmd_len, 0, DEC);
    //DEBUG_OUT_BYTES ( g_zt_msg.sendbuf , g_zt_msg.icmd_len );  
    return g_zt_msg.icmd_len;
}

/************************************************* 
*Function:	Ptz_SetRst_string 	
*Return:		
*DESCRIPTION: 设置云台的状态字符串
*************************************************/  
void Ptz_SetRst_string(uint8 id,uint8 rst)
{
    //int vals[6];
	
	uint8 com=id;
	
    char* str = (char*)(g_zt_msg.sendbuf); 

    str = cmd_para_build_para_string_str( str , "AT+PtzSet" , "OK" );
	if(rst==1)					//设置云台姿态和焦距成功
	{
		str = cmd_para_build_para_string_str( str , "code" , "0" );
		GTZMHDCmdSetOk=0;
    }  
    else
	{
		str = cmd_para_build_para_string_str( str , "code" , "9" );
	}
   
    if(com==ID_DEBUG)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Debug);
    }
	else if(com==ID_LINUX)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Linux);
    }
	else if(com==ID_RF433)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Rf433);
    }
	str = cmd_para_build_para_string_str(str , "Name" , gRbtState.RobotName);
    
    *str++ = '\r';
    *str++ = '\n';
    *str   = 0;
    g_zt_msg.icmd_len = str - (char*)(g_zt_msg.sendbuf) ;
	
	ack_with_debug(CurrentPtzSetCom,g_zt_msg.sendbuf,g_zt_msg.icmd_len);
	CurrentPtzSetCom=0;
    //DEBUG_OUT_BYTES ( g_zt_msg.sendbuf , g_zt_msg.icmd_len );  
    //return g_zt_msg.icmd_len;
}

/************************************************* 
*Function:	Ptz_GetRst_string 	
*Return:		
*DESCRIPTION: 获取云台的状态字符串
*************************************************/  
//void Ptz_GetRst_string(uint8 id,uint8 rst)
void Ptz_GetRst_string(uint8 rst)
{
    int vals[6];
	
	//uint8 com=id;
	
    char* str = (char*)(g_zt_msg.sendbuf); 

    if(rst==1)//获取云台姿态和焦距成功
	{
	    vals[0] = GTZMHD_Get.directAngle/100;
	    vals[1] = GTZMHD_Get.pitchAngle/100 ;
	    vals[2] = 0;
	    vals[3] = GTZMHD_Get.nVisibleFocalDis/5;
		str = cmd_para_build_para_string_ints( str , "AT+PtzGet" , vals , 4 );
		str = cmd_para_build_para_string_str( str , "code" , "0" );
		GTZMHDCmdGetOk=0;
    }  
    else
	{
		vals[0] = 0;
	    vals[1] = 0 ;
	    vals[2] = 0;
	    vals[3] = 0;
		str = cmd_para_build_para_string_ints( str , "AT+PtzGet" , vals , 4 );
		str = cmd_para_build_para_string_str( str , "code" , "9" );
	}
    
    if(CurrentPtzSetCom==ID_DEBUG)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Debug);
    }
	else if(CurrentPtzSetCom==ID_LINUX)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Linux);
    }
	else if(CurrentPtzSetCom==ID_RF433)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Rf433);
    }
	str = cmd_para_build_para_string_str(str , "Name" , gRbtState.RobotName);
    
    *str++ = '\r';
    *str++ = '\n';
    *str   = 0;
    g_zt_msg.icmd_len = str - (char*)(g_zt_msg.sendbuf) ;
	
    ack_with_debug(CurrentPtzSetCom,g_zt_msg.sendbuf,g_zt_msg.icmd_len);
	CurrentPtzSetCom=0;
    //DEBUG_OUT_BYTES ( g_zt_msg.sendbuf , g_zt_msg.icmd_len );  
    //return g_zt_msg.icmd_len;
}

void rt_get_SlvCFG(uint8 com)
{
    char* str = (char*)(g_zt_msg.sendbuf) ;
    str = cmd_para_build_para_string_str( str , "AT+GetSlvCFG" , "OK" );
    //str = cmd_para_build_para_string_int( str , "pmin" , gSlvMtCfg.press_ok_min         );
    str = cmd_para_build_para_string_int( str , "pmin" , (int)(gSlvMtCfg.press_ok_min*2.34375+0.5)    );
	
    //str = cmd_para_build_para_string_int( str , "pmax" , gSlvMtCfg.press_ok_max         );
    str = cmd_para_build_para_string_int( str , "pmax" , (int)(gSlvMtCfg.press_ok_max*2.34375+0.5)    );
	
    //str = cmd_para_build_para_string_int( str , "pmid" , gSlvMtCfg.press_ok_to_adjust   );
    str = cmd_para_build_para_string_int( str , "pmid" , (int)(gSlvMtCfg.press_ok_to_adjust*2.34375+0.5)    );
	
    //str = cmd_para_build_para_string_int( str , "StopMin" , gSlvMtCfg.press_mst_stop_min   );
    str = cmd_para_build_para_string_int( str , "StopMin" , (int)(gSlvMtCfg.press_mst_stop_min*2.34375+0.5)    );
	
    //str = cmd_para_build_para_string_int( str , "StopMax" , gSlvMtCfg.press_mst_stop_max   );
    str = cmd_para_build_para_string_int( str , "StopMax" , (int)(gSlvMtCfg.press_mst_stop_max*2.34375+0.5)    );
	
    str = cmd_para_build_para_string_int( str , "SpeedBrg" , gSlvMtCfg.mst_limit_on_bridge  );
    str = cmd_para_build_para_string_int( str , "AddAccl" , gSlvMtCfg.mstAddAccl  );
    str = cmd_para_build_para_string_int( str , "DelAccl" , gSlvMtCfg.mstDelAccl  );
    str = cmd_para_build_para_string_int( str , "BrgTime" , gSlvMtCfg.onBridgeTime  );

    *str++ = '\r';
    *str++ = '\n';
    *str   = 0;
    g_zt_msg.icmd_len = str - (char*)(g_zt_msg.sendbuf) ;

    ack_with_debug(com, g_zt_msg.sendbuf , g_zt_msg.icmd_len );
}

UserCfgType gUserAllCfg;
//AT+SlvCFG=6 pmin=40 pmax=110 pmid=75 StopMin=20 StopMax=150 SpeedBrg=10
/************************************************* 
*Return:		
*DESCRIPTION:参数配置过程，配置参数初始化函数
*************************************************/
void cfg_oper_init(void)
{
    //const UserCfgType* pcfg = (const UserCfgType*)( BOX_OS_USER_SLV_CFG_ADDR );
   // memcpy( &gUserAllCfg , pcfg , sizeof(gUserAllCfg) );  //不是加载默认，而是从以及使用的进行加载
    //memcpy( &gUserAllCfg.SlvMtCfg , &gSlvMtCfg , sizeof(gSlvMtCfg) );
	CopyBuffer(&gSlvMtCfg, &gUserAllCfg.SlvMtCfg, sizeof(gSlvMtCfg));
    gUserAllCfg.begin_cfg = 0XAAAAAAAA;
    gUserAllCfg.end_cfg   = 0X55555555;
    //gUserAllCfg.SlvMtCfg.fk_mst_limit_down = 0;
    //gUserAllCfg.SlvMtCfg.fk_mst_limit_up   = 0;
}

void flash_write_u32_datas(u32 addr,const void* pds, int u32_cnt)
{
    const u32* pDatas = (const u32*)(pds);
    while( u32_cnt )
    {
        FLASH_ProgramWord( addr , *pDatas );
        addr += sizeof(u32);
        pDatas++ ;
        u32_cnt--;
    }
    FLASH_WaitForLastOperation(0x00002000);
}

/************************************************* 
*Return:		
*DESCRIPTION:参数配置过程，判断参数是否有变化，如果有，进行一次配置过程
*************************************************/
void cfg_oper_save_to_flash(void)
{
    const UserCfgType* pcfg = (const UserCfgType*)( BOX_OS_USER_SLV_CFG_ADDR );
    if ( memcmp( &gUserAllCfg , pcfg , sizeof(gUserAllCfg) ) )
    {
        OS_CPU_SR  cpu_sr = 0;
        OS_ENTER_CRITICAL();
        //不相同 需要擦除该页，并且写入对应的数据
        FLASH_Unlock();
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
        FLASH_ErasePage( BOX_OS_USER_SLV_CFG_ADDR );
        flash_write_u32_datas(BOX_OS_USER_SLV_CFG_ADDR , &gUserAllCfg , (sizeof(gUserAllCfg)+3)/4 );
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
        FLASH_Lock();
        zt_motor_slave_driver_cfg_init();
        OS_EXIT_CRITICAL();
    }
}

/************************************************* 
*Function:		rt_get_BatCFG
*Input:			com
*Return:		
*DESCRIPTION:  电池参数设置回复函数
*************************************************/

void rt_get_BatCFG(uint8 com)
{
    char* str = (char*)(g_zt_msg.sendbuf) ;
    str = cmd_para_build_para_string_str( str , "AT+RtBatCFG" , "OK" );
    str = cmd_para_build_para_string_float( str , "BatNoMove" , gSlvMtCfg.bat_no_move*0.1f , 1 );
    str = cmd_para_build_para_string_float( str , "ChgNeed" , gSlvMtCfg.bat_auto_charge*0.1f , 1 );
    str = cmd_para_build_para_string_float( str , "ChgFull" , gSlvMtCfg.bat_charge_full*0.1f , 1 );

    *str++ = '\r';
    *str++ = '\n';
    *str   = 0;
    g_zt_msg.icmd_len = str - (char*)(g_zt_msg.sendbuf) ;

    ack_with_debug( com,g_zt_msg.sendbuf , g_zt_msg.icmd_len );
}

/************************************************* 
*Function:		OpenOrCloseDevOperate
*Input:			char* cmd   命令字符串
*Return:		
*DESCRIPTION:  接收到给各种外部设备使能和关闭的命令
*************************************************/
bool OpenOrCloseDevOperate(char* devName,bool bOpen)
{
    bool bGpioVal = !bOpen ; //一般是低电平使能
    
    //if ( box_str_cmp( devName , "4G" ) == 0 ){ 
        //GpioSetBit(GPIO_4G_EN,bGpioVal);
    //}
    if(box_str_cmp( devName , "MT" )== 0){
        GpioSetBit(GPIO_MOTOR_PWR_EN,bGpioVal);
    }
    else if(box_str_cmp( devName , "Brk" )== 0){
        GpioSetBit(GPIO_BREAK_MEN,bGpioVal);
    }
    else if(box_str_cmp( devName , "Charge" )== 0){
        if( bOpen ){
            if(!gRbtState.bChargeShort){
                BAT_CHARGE_OPEN();
            }
            else{
                BAT_CHARGE_CLOSE();
                return false;
            }
        }
        else{
            BAT_CHARGE_CLOSE();
        }
    }
    else if(box_str_cmp( devName , "CamAv" )== 0){ //图传
        GpioSetBit(GPIO_CAM_AV_EN,bGpioVal);
    }
    else if(box_str_cmp( devName , "Cam" )== 0){//摄像头
        GpioSetBit(GPIO_CAM_EN,bGpioVal);
    }
    else if(box_str_cmp( devName , "GPS" )== 0){
        GpioSetBit(GPIO_GPS_EN,bGpioVal);
    }
    else if(box_str_cmp( devName , "12V" )== 0){
        GpioSetBit(GPIO_12V_EN,bGpioVal);
    }
    else if(box_str_cmp( devName , "CamSW" )== 0){
        GpioSetBit(GPIO_CTR_CAM_AV_SW,bGpioVal);
    }
	else  if(box_str_cmp( devName , "ALL" )== 0)//处理所有的外设电源
	{
        GpioSetBit(GPIO_MOTOR_PWR_EN,bGpioVal);//电机驱动板
        GpioSetBit(GPIO_CAM_AV_EN,bGpioVal);//图传
        GpioSetBit(GPIO_CTR_CAM_AV_SW,bGpioVal);//视频编码器
        GpioSetBit(GPIO_CAM_EN,bGpioVal);//主摄像头
	}
    else
    {
        return false;
    }
    return true;
}

u8 char_to_byte_val(u8 ch)
{
    if( (ch>='0') && (ch<='9') ){ return ch-'0'; }
    if( (ch>='a') && (ch<='f') ){ return ch-'a'+10; }
    if( (ch>='A') && (ch<='F') ){ return ch-'A'+10; }
    return 100;
}

int box_str_get_bytes_by_hex(const char* str,u8* pDataOut,int dataCntMax)
{
    u8 ch = *str;
    bool bFirst = true;
    int  nGet = 0;
    while( ch && (dataCntMax>0) ){
        ch = char_to_byte_val(ch);
        if( ch < 16 ){
            if( bFirst ){
                bFirst = false;
                *pDataOut = ch<<4;
            }
            else{
                *pDataOut |= ch;
                nGet++;
                pDataOut++;
                dataCntMax--;
                bFirst = true;
            }
        }
        str++;ch = *str;
    }
    return nGet;
}

//AT指令列表
#define		ORDER_NUM			21
const  char *CommandList[] = 
{
	"AT+list",				//  0
	"AT+system",			//	1
	"AT+MemStates",     	//  2
	"AT+reboot",			//  3
	"AT+SlvMt",				//	4
	"AT+MstMt",				//  5
	"AT+Hello",				//  6
    "AT+GetState",	    	//  7
    "AT+CFG1",				//  8
    "AT+GetSlvCFG",			//  9
    "AT+CFG3",				//  10   电池充电设置参数BATCFG
	"AT+Lcmd",	       		//  11
	"AT+OpenDev",			//  12
	"AT+CloseDev",			//  13
	"AT+LinuxRst",			//  14
	"AT+Rf433Cfg",			//  15
	"AT+Rf433Set",			//  16
	"AT+Lack",		   		//  17
	"AT+PtzSet",	    	//  18
	"AT+PtzGet",	   		//  19
	"AT+CFG2",				//  20
	"AT+StateSw"			//  21  自动巡检、手动指令切换
};

//SYSTEM_SET		SystemInfo;
OS_EVENT *LocalQueue;		//串口0接收数据邮箱

//AT+list		0
void ShowDebugList(void)							//0
{
	uint8 i;
	
	OSVersion();
	nprintf(ID_DEBUG,OS_VERSION,0,DEC);
	
	stprintf(ID_DEBUG,"本驻塔机器人系统的调试命令如下：\r\n");
	for( i=0;i<=ORDER_NUM;i++ )
	{
		stprintf(ID_DEBUG,(char*)CommandList[i]);
		stprintf(ID_DEBUG,"\r\n");
		OSTimeDly(OS_TICKS_PER_SEC/50);
	}
}

//AT+system			1
const char *TaskName[] = {"Idel","Weather","Com_Rec","Timer","Local",};
void ShowSystemInformation(void)					//1
{
	//OS_SEM_DATA		mem_sem;
	OS_TCB			task_data;
	OS_STK_DATA		task_stk; 
	uint8 err;
	uint8 i,j;
	
	//#if COMM_DEBUG_EN
	j = 0;
	stprintf(ID_DEBUG,"\r\n----------------------------------------------------------");
    stprintf(ID_DEBUG,"\r\n任务		总堆栈		空闲栈		运行态");
	stprintf(ID_DEBUG,"\r\n----		------		------		------\r\n");
     	           
	for( i=0;i<8;i++ )
	{
		OSTimeDly(OS_TICKS_PER_SEC/50);
		if ( i==1 )	continue;
		if ( i==2 )	continue;
		if ( i==3 )	continue;

		err = OSTaskQuery(i,&task_data);
		if(err != OS_ERR_NONE)
	   		continue;
		stprintf(ID_DEBUG,(char *)TaskName[j++]);
		stprintf(ID_DEBUG,"\t\t");
		err = OSTaskStkChk(i,&task_stk);
		if(err != OS_ERR_NONE)
			continue;
		nprintf(ID_DEBUG,task_stk.OSFree+task_stk.OSUsed,0,DEC);
		stprintf(ID_DEBUG,"\t\t");
		nprintf(ID_DEBUG,task_stk.OSFree,0,DEC);
		stprintf(ID_DEBUG,"\t\t");

		OSTimeDly(OS_TICKS_PER_SEC/50);
		
		err = task_data.OSTCBStat;
		if(err == OS_STAT_RDY)				
			stprintf(ID_DEBUG,"任务就绪");      
		else if(err == OS_STAT_SEM)	
			stprintf(ID_DEBUG,"等信号量");		      
		else if(err == OS_STAT_MBOX)		
			stprintf(ID_DEBUG,"等消息邮件");    
		else if(err == OS_STAT_Q)			
			stprintf(ID_DEBUG,"等消息队列");     
		else if(err == OS_STAT_SUSPEND)		
			stprintf(ID_DEBUG,"任务挂起");       
		else if(err == OS_STAT_MUTEX)		
			stprintf(ID_DEBUG,"等互斥量");      
		else if(err == OS_STAT_FLAG)		
			stprintf(ID_DEBUG,"等标志量"); 
		stprintf(ID_DEBUG,"\r\n");
	}	
	stprintf(ID_DEBUG,"---^_^---\r\n");
	//#endif
}

//AT+MemStates			2
void ShowMemStates(void)					//2
{
	uint8 err;
	OS_MEM_DATA		mem_mem;
	OS_SEM_DATA		mem_sem;
	
	err = err;
	stprintf(ID_DEBUG,"\r\n消息队列内存剩余: ");
	err = OSMemQuery(p_msgBuffer,&mem_mem);
	nprintf(ID_DEBUG,mem_mem.OSNFree,0,DEC);
					
	
	stprintf(ID_DEBUG,"\r\n160B的内存块剩余: ");
	err = OSMemQuery(mem160ptr,&mem_mem);
	nprintf(ID_DEBUG,mem_mem.OSNFree,0,DEC);				
	
	stprintf(ID_DEBUG,"\r\n512B的内存块剩余: ");
	err = OSMemQuery(mem512ptr,&mem_mem);
	nprintf(ID_DEBUG,mem_mem.OSNFree,0,DEC);								

	stprintf(ID_DEBUG,"\r\n512B的信号量剩余: ");
	err = OSSemQuery(mem512_sem,&mem_sem);
	nprintf(ID_DEBUG,mem_sem.OSCnt,0,DEC);

	stprintf(ID_DEBUG,"\r\n");
}

//AT+reboot			3
void set_SystemReset(void)  					//3
{		
	stprintf(ID_DEBUG,"\r\n-- 系统复位 --\r\n"); 
	OSTimeDly(OS_TICKS_PER_SEC*2);
	
	__disable_irq();	// 关闭全局中断
	(*((void(*)())0))();
	while(1);
}

//设置从动轮 电机速度 			4
//AT+SlvMt=15,1000  CRC=9c2d
void SetSlaveMotor(uint8 com,uint8* cmd)       //4
{	
    uint8 ret;

	//对命令进行解析
    if( CheckIsCmdOk(com,cmd , &gCmdParaInfos) == false )
	{
        stprintf(ID_DEBUG,"\r\n SlvMt Cmd CRC Err ");
		ret = 0x08;//命令的CRC校验错误
		at_build_mtSetBk_string(com,ret,2);
        return;
    }
    else
	{
		int ValCnt = cmd_para_get_int_by_val_str( gCmdParaInfos.items[0].value , gCmdParaVal_Ints , 5 );
		
        if ( (ValCnt <= 0) || (ValCnt>2) )
        {
			stprintf(ID_DEBUG,"\r\n SlvMt Err Val ");
			ret = 0x01;//参数错误
			at_build_mtSetBk_string(com,ret,2);
            return;
        }
		//nprintf(1,gCmdParaVal_Ints[0],0,DEC);
		//stprintf(1," ");
		//nprintf(1,gCmdParaVal_Ints[1],0,DEC);
		
		if(gMstMt.cur_speed !=0)		//主动轮先停止，从动轮才能下降
        {
			zt_motor_master_driver_set_speed( 0 , 50000 );
			
			if ( ValCnt == 1 )
		    {
				zt_motor_slave_driver_set_speed( gCmdParaVal_Ints[0] , 10000 );
		    }
		    else if ( ValCnt == 2 )
		    {
				zt_motor_slave_driver_set_speed( gCmdParaVal_Ints[0] , gCmdParaVal_Ints[1] );
		    }
		}
		else
		{
			if ( ValCnt == 1 )
		    {
				zt_motor_slave_driver_set_speed( gCmdParaVal_Ints[0] , 10000 );
		    }
		    else if ( ValCnt == 2 )
		    {
				zt_motor_slave_driver_set_speed( gCmdParaVal_Ints[0] , gCmdParaVal_Ints[1] );
		    }
    	}
    }
	ret = 0x00;//命令执行成功
    at_build_mtSetBk_string(com,ret,2);
}

//AT+MstMt=15,1000  CRC=bb6f
//AT+MstMt=15,1000  SlvMt=hold CRC=da3c
//AT+MstMt=15,1000  SlvMt=0 CRC=8b08
void SetMasterMotor(uint8 com,uint8* cmd)   		    //5
{
    uint8 ret;
	//对命令进行解析
    if( CheckIsCmdOk(com,cmd , &gCmdParaInfos) == false )
	{
        stprintf(ID_DEBUG,"MstMt Cmd Err\r\n");
		ret = 0x08;//命令的CRC校验错误
		at_build_mtSetBk_string(com,ret,1);
        return;
    }
    else
    {
        u16 bNeed_SetSlvMtAuto = 1; 			//是否设置从动轮自动
        int ValCnt = cmd_para_get_int_by_val_str( gCmdParaInfos.items[0].value , gCmdParaVal_Ints , 5 );
        if ( (ValCnt <= 0) || (ValCnt>2) )
        {
            stprintf(ID_DEBUG,"MstMt Err Val\r\n");
			ret = 0x01;//参数错误
			at_build_mtSetBk_string(com,ret,1);
            return;
        }
		/*
		if(gPressFilter.val < gSlvMtCfg.press_mst_stop_min){
				
			zt_motor_master_driver_set_speed( 0 , 50000 );
			stprintf(ID_DEBUG,"MstMt  press is less than min Val\r\n");
			return;	
			}
			
		if(gPressFilter.val > gSlvMtCfg.press_mst_stop_max){
				
			zt_motor_master_driver_set_speed( 0 , 50000 );
			stprintf(ID_DEBUG,"MstMt  press is more than max Val\r\n");
			
			return;	
			}
			
			*/
        
        //如果处于桥上，且电池电量较低，控制主电机速度为0
        if( gBatAutoCtrl.bOnBridge && (gCmdParaVal_Ints[0]!=0) )
		{
            if( (!gRbtState.bCX_SwCheck1) && (!gRbtState.bCX_SwCheck2) )
			{
                gBatAutoCtrl.bOnBridge = false; //如果当前没有遮挡传感器，认为离开桥了
            }
            else if(gBatAutoCtrl.curBatVol<=gSlvMtCfg.bat_no_move)
			{
                zt_motor_master_driver_set_speed( 0 , 50000 );
                //uart_send_string( g_pCurAtCmdFrom->uart ,"AT+MtSetBk=BatLow MstMt=0\r\n" );
				stprintf(ID_DEBUG,"AT+MtSetBk=BatLow MstMt=0\r\n");
				ret = 0x02;//电池电量低
				at_build_mtSetBk_string(com,ret,1);
                return;
            }
            
            //处于桥上的处理，以及充电状态更新处理
            if( gBatAutoCtrl.bCharging )
			{ //如果正在充电中，需要关闭充电
                gBatAutoCtrl.bChargingClosed = true;
                gBatAutoCtrl.bCharging = false; 
				BAT_CHARGE_CLOSE();
            }
            //BAT_CHARGE_CLOSE();  //如果运动了，需要关闭充电
            BAT_CHARGE_OPEN();  //如果运动了，需要关闭充电，但继电器需要断开到常开触点。
            //因为是在桥上，需要现在速度在桥上速度
            gMstMt.limit_speed      = gSlvMtCfg.mst_limit_on_bridge*100;
            gMstMt.limit_speed_time = (OS_TICKS_PER_SEC/4)*gSlvMtCfg.onBridgeTime ;
        }
        
		if(gPressFilter.val<gSlvMtCfg.press_ok_min)//从动轮没有加紧时，从动轮先加紧，主动轮再运动
		{
			gSlaveMtAnaly.press_to_adj = gSlvMtCfg.press_ok_to_adjust ;
		    if ( gPressFilter.val < gSlvMtCfg.press_ok_min )
		    {
		        //gSlaveMtAnaly.real_slv_up_press_to_adg = gSlvMtCfg.press_ok_to_adjust + 10 ;
		        gSlaveMtAnaly.real_slv_up_press_to_adg = gSlvMtCfg.press_ok_max - 4 ;
		        SLV_AUTO_SPEED_UP();
		    }
		    else if ( gPressFilter.val > gSlvMtCfg.press_ok_max )
		    {
		        //需要松开从动轮
		        gSlaveMtAnaly.real_slv_down_press_to_adg = gSlvMtCfg.press_ok_to_adjust ;
		        SLV_AUTO_SPEED_DOWN();
		    }
		    else  //当前压力在正常范围内
		    {
		        gSlaveMtAnaly.auto_cal_speed = 0;
		    }
			if ( ValCnt == 1 )
	        {
	            zt_motor_master_driver_set_speed( gCmdParaVal_Ints[0] , 50000 );
	        }
	        else if ( ValCnt == 2 )
	        {
	            zt_motor_master_driver_set_speed( gCmdParaVal_Ints[0] , gCmdParaVal_Ints[1] );
	        }
		}
		else
		{
			if ( ValCnt == 1 )
	        {
	            zt_motor_master_driver_set_speed( gCmdParaVal_Ints[0] , 50000 );
	        }
	        else if ( ValCnt == 2 )
	        {
	            zt_motor_master_driver_set_speed( gCmdParaVal_Ints[0] , gCmdParaVal_Ints[1] );
	        }
		}
        
        //USART_DEBUG_OUT("M_Mt=%d\n", gMstMt.set_speed );
        if ( gCmdParaInfos.item_cnt >= 2 )
        {
            if ( box_str_cmp(gCmdParaInfos.items[1].name , "SlvMt" ) == 0 )
            {
                if( box_str_cmp(gCmdParaInfos.items[1].value , "hold" ) == 0 )
				{
                    bNeed_SetSlvMtAuto = 0;
                }
                else if( box_str_cmp(gCmdParaInfos.items[1].value , "0" ) == 0 )
				{
                    bNeed_SetSlvMtAuto = 0;
                    zt_motor_slave_driver_set_speed( 0 , 0 );
                }
            }
        }
        
        if ( bNeed_SetSlvMtAuto )
        {
            zt_motor_slave_driver_set_speed( 200 , 0 );
            //USART_DEBUG_OUT("slv mt auto\n" );
        }
    }
	ret = 0x00;//命令执行成功
    at_build_mtSetBk_string(com,ret,1);
	
}

//AT+Hello=OK GpsPos=116.1479415,33.7518119,61.2 GpsTime=17,11,20,14,0,24 name=Rbt9999
void Hello(uint8 com,uint8* cmd)   			 //6
{
	int vid;
	//CmdParaInfo* pParaInfo;
	//uint8 i;
    //USART_DEBUG_OUT("receave Hello\r\n");
    //Hello=OK	GpsPos=116.1479415,33.7518119,61.2	GpsTime=17,11,20,14,0,24

	 /*stprintf(1,"解析后的各值");
	 for(i=0;i<gCmdParaInfos.item_cnt;i++)
     {
		 stprintf(1, gCmdParaInfos.items[i].name);
		 nprintf(1,(uint16)box_str_hex_to_int(gCmdParaInfos.items[i].value),0,HEX);
		 stprintf(1," ");
     }
	 stprintf(1,"\r\n");*/
	 
    if( com == ID_LINUX )
	{
        g_zt_msg.nTimeForNoLinuxHeartIn = 0;  //清空错误标识

		//GpsPos,GpsTime主动发送给妙算
        /*vid = cmd_para_get_id_by_name(&gCmdParaInfos,"GpsPos");
        if ( vid >= 0 ) 
		{
            if( gCmdParaInfos.items[vid].value_str_len < sizeof(gRbtState.GpsPosState) )
			{
                CopyBuffer(gCmdParaInfos.items[vid].value,gRbtState.GpsPosState , 
                    gCmdParaInfos.items[vid].value_str_len+1);
            }
        }
		
        vid = cmd_para_get_id_by_name(&gCmdParaInfos,"GpsTime");
        if ( vid >= 0 ) 
		{
            if( gCmdParaInfos.items[vid].value_str_len < sizeof(gRbtState.GpsTimeState) )
			{
                CopyBuffer(gCmdParaInfos.items[vid].value,gRbtState.GpsTimeState , 
                    gCmdParaInfos.items[vid].value_str_len+1);
            }
        }*/
		
        vid = cmd_para_get_id_by_name(&gCmdParaInfos,"Lname");
        if ( vid >= 0 )
		{
            if( gCmdParaInfos.items[vid].value_str_len < sizeof(gRbtState.RobotName) )
			{
               CopyBuffer( gCmdParaInfos.items[vid].value,gRbtState.RobotName ,
                    gCmdParaInfos.items[vid].value_str_len+1);
            }
            //USART_DEBUG_OUT("receave name=%s\r\n",gRbtState.RobotName);
        }

        vid = cmd_para_get_id_by_name(&gCmdParaInfos,"Lst");
        if ( vid >= 0 )
		{
            if( gCmdParaInfos.items[vid].value_str_len < sizeof(gRbtState.RobotState) )
			{
                CopyBuffer( gCmdParaInfos.items[vid].value,gRbtState.RobotState ,
                    gCmdParaInfos.items[vid].value_str_len+1);
                //"Idle" , "Handle" ,  "AutoRun" ,  "BackChg"
                //各种状态下的自动处理流程；巡检状态，各种外设需要自动打开
                if( 'A' == gRbtState.RobotState[0] )			//自动巡检状态
				{ 
                    //需要默认开启各种功能
                    //GpioSetBit(GPIO_MOTOR_PWR_EN,0);
                    GpioSetBit(GPIO_CAM_AV_EN,0);
                    GpioSetBit(GPIO_CAM_EN,0);
                }
                else if( 'B' == gRbtState.RobotState[0] )		//返回充电
				{ 
                    //需要默认关闭摄像头等各种功能
                    GpioSetBit(GPIO_CAM_AV_EN,1);
                    GpioSetBit(GPIO_CAM_EN,1);
                }
            }
        }

		//codeCfg=比例系数,目标桥编码器长度一半
        gCmdParaVal_Ints[0] = 100;
        gCmdParaVal_Ints[1] = 50;
        vid = cmd_para_get_id_by_name(&gCmdParaInfos,"codeCfg"); 
        if ( vid >= 0 ) 
		{
            int ValCnt = cmd_para_get_int_by_val_str(
                gCmdParaInfos.items[vid].value , gCmdParaVal_Ints , 2 );
            if(ValCnt<2)
			{ //设置回到默认值
                gCmdParaVal_Ints[0] = 100;
                gCmdParaVal_Ints[1] = 50;
            }
        }
        gSlaveMtAnaly.CodeScale          = gCmdParaVal_Ints[0];
        gSlaveMtAnaly.curBridgeCodeHalf  = gCmdParaVal_Ints[1];
		
		//自动与手动状态切换标志
		vid = cmd_para_get_id_by_name(&gCmdParaInfos,"StateSw");
		if ( vid >= 0 )
		{
			if( gCmdParaInfos.items[vid].value_str_len < sizeof(gRbtState.StateSwitchFlag) )
			{
				CopyBuffer( gCmdParaInfos.items[vid].value,gRbtState.StateSwitchFlag,
                gCmdParaInfos.items[vid].value_str_len+1);
			}
		}
		
			
		zt_build_send_state_string(BUILD_STATE_FLAG_ALL,ID_LINUX,8);//zs 1116  add
        uart2_send(g_zt_msg.sendbuf , g_zt_msg.icmd_len );//zs 1116 add
    }
}

//AT+GetState=L  		7
void GetState(uint8 com,uint8* cmd)					//7
{
	u32 getFlag = 0;

    if( box_str_find_char_pos(gCmdParaInfos.items[0].value,'B') ){ getFlag |= BUILD_STATE_FLAG_BAT    ; }
    if( box_str_find_char_pos(gCmdParaInfos.items[0].value,'A') ){ getFlag |= BUILD_STATE_FLAG_AM2320 ; }
    if( box_str_find_char_pos(gCmdParaInfos.items[0].value,'J') ){ getFlag |= BUILD_STATE_FLAG_JY901  ; }
    if( box_str_find_char_pos(gCmdParaInfos.items[0].value,'P') ){ getFlag |= BUILD_STATE_FLAG_PRESS  ; }
    if( box_str_find_char_pos(gCmdParaInfos.items[0].value,'M') ){ getFlag |= BUILD_STATE_FLAG_MSTMT  ; }
    if( box_str_find_char_pos(gCmdParaInfos.items[0].value,'S') ){ getFlag |= BUILD_STATE_FLAG_SLVMT  ; }
    if( box_str_find_char_pos(gCmdParaInfos.items[0].value,'W') ){ getFlag |= BUILD_STATE_FLAG_SLVWH  ; }
    if( box_str_find_char_pos(gCmdParaInfos.items[0].value,'X') ){ getFlag |= BUILD_STATE_FLAG_XWSW   ; }
    if( box_str_find_char_pos(gCmdParaInfos.items[0].value,'C') ){ getFlag |= BUILD_STATE_FLAG_CHARGE ; }
    if( box_str_find_char_pos(gCmdParaInfos.items[0].value,'G') ){ getFlag |= BUILD_STATE_FLAG_GPS    ; }
    if( box_str_find_char_pos(gCmdParaInfos.items[0].value,'L') ){ getFlag |= BUILD_STATE_FLAG_ALL    ; }
    //生成状态数据，并发送状态数据
    zt_build_send_state_string( getFlag,com ,0);
    ack_with_debug (com, g_zt_msg.sendbuf , g_zt_msg.icmd_len );
}

//AT+SlvCFG1=8 pmin=70 pmax=90 StopMin=20 StopMax=125 
void SlvCFG1(uint8 com,uint8* cmd)   				 //8
{
    int ValCnt;
  
	if( CheckIsCmdOk(com,cmd , &gCmdParaInfos) == false )
	{
		stprintf(ID_DEBUG,"SlvCFG Cmd Err\r\n");
        return;
    }
    else if ( gCmdParaInfos.item_cnt < 3 )
	{
        stprintf(ID_DEBUG,"SlvCFG Cmd No Data\r\n");
        rt_get_SlvCFG(com);
        return;
    }
    else
    {
        //int vid;        
        cfg_oper_init();

        ValCnt = cmd_para_get_int_by_val_str( gCmdParaInfos.items[0].value , gCmdParaVal_Ints , 4 );
        if (ValCnt !=4) 
        {
            stprintf(ID_DEBUG,"MstMt Err Val\r\n");
            return;
        }
		else
		{
        	//vid = cmd_para_get_id_by_name(&gCmdParaInfos,"pmin");
        	//if ( vid > 0 ) { gUserAllCfg.SlvMtCfg.press_ok_min = atoi( gCmdParaInfos.items[vid].value ); }
       		//if ( vid > 0 ) 
			 gUserAllCfg.SlvMtCfg.press_ok_min = (u16)(((float)gCmdParaVal_Ints[0])/2.34375+0.5); //单位为牛顿
        	//vid = cmd_para_get_id_by_name(&gCmdParaInfos,"pmax");
       		//if ( vid > 0 ) { gUserAllCfg.SlvMtCfg.press_ok_max = atoi( gCmdParaInfos.items[vid].value ); }
       		//if ( vid > 0 ) 
       		gUserAllCfg.SlvMtCfg.press_ok_max = (u16)(((float)gCmdParaVal_Ints[1])/2.34375+0.5); //单位为牛顿
        	//vid = cmd_para_get_id_by_name(&gCmdParaInfos,"StopMin");
       		//if ( vid > 0 ) { gUserAllCfg.SlvMtCfg.press_mst_stop_min = atoi( gCmdParaInfos.items[vid].value ); }
        	//if ( vid > 0 )
        	gUserAllCfg.SlvMtCfg.press_mst_stop_min = (u16)(((float)gCmdParaVal_Ints[2])/2.34375+0.5); //单位为牛顿
        	//vid = cmd_para_get_id_by_name(&gCmdParaInfos,"StopMax");
        	//if ( vid > 0 ) { gUserAllCfg.SlvMtCfg.press_mst_stop_max = atoi( gCmdParaInfos.items[vid].value ); }
        	//if ( vid > 0 ) 
         	gUserAllCfg.SlvMtCfg.press_mst_stop_max = (u16)(((float)gCmdParaVal_Ints[3])/2.34375+0.5); //单位为牛顿
    	}
        
        //vid = cmd_para_get_id_by_name(&gCmdParaInfos,"pmid");
        //if ( vid >= 0 ) { gUserAllCfg.SlvMtCfg.press_ok_to_adjust = atoi( gCmdParaInfos.items[vid].value ); }
        gUserAllCfg.SlvMtCfg.press_ok_to_adjust = 
            (gUserAllCfg.SlvMtCfg.press_ok_min>>1) + 
            (gUserAllCfg.SlvMtCfg.press_ok_max>>1) ;
        
        cfg_oper_save_to_flash();  //进行状态返回
        rt_get_SlvCFG(com);
    }
}

//AT+GetSlvCFG
void GetSlvCFG(uint8 com)							//9
{
	rt_get_SlvCFG(com);	
}

//AT+BatCFG=0 BatNoMove=21.2 ChgNeed=22.2 ChgFull=25.2 CRC=2982
//对应AT+CFG3
void BatCFG(uint8 com,uint8* cmd)					//10
{
    int ValCnt;
	
	if( CheckIsCmdOk(com,cmd , &gCmdParaInfos) == false ){
		stprintf(ID_DEBUG,"BatCFG Cmd Err\r\n");
        return;
    }
    else if ( gCmdParaInfos.item_cnt < 3 ){
		stprintf(ID_DEBUG,"BatCFG VAL Err\r\n");
        rt_get_BatCFG(com);
        return;
    }
    else
    {
        //int vid;        
        cfg_oper_init();

		ValCnt = cmd_para_get_float_by_val_str( gCmdParaInfos.items[0].value , gCmdParaVal_floats , 3 );
        if (ValCnt !=3) 
        {
            stprintf(ID_DEBUG,"BatCFG Err Val\r\n");
            return;
        }
        //vid = cmd_para_get_id_by_name(&gCmdParaInfos,"BatNoMove");
        //if ( vid > 0 ) 
        { gUserAllCfg.SlvMtCfg.bat_no_move     = (int16)((gCmdParaVal_floats[0])*10+0.5f); }
        //vid = cmd_para_get_id_by_name(&gCmdParaInfos,"ChgNeed");
        //if ( vid > 0 ) 
        { gUserAllCfg.SlvMtCfg.bat_auto_charge = (int16)((gCmdParaVal_floats[1])*10+0.5f); }
        //vid = cmd_para_get_id_by_name(&gCmdParaInfos,"ChgFull");
        //if ( vid > 0 ) 
        { gUserAllCfg.SlvMtCfg.bat_charge_full = (int16)((gCmdParaVal_floats[2])*10+0.5f); }

        cfg_oper_save_to_flash();
        rt_get_BatCFG(com);
    }
}

//AT+Lcmd 		11
void Lcmd(uint8 com,uint8* cmd)					//11
{
	//stprintf(1,(char*)cmd);
	if( com != ID_LINUX )
	{
        //发送数据给 linux  不需要 L  cmd=...s
        stprintf(ID_LINUX,"AT+");
        stprintf(ID_LINUX,(char*)cmd);
    }
}

//AT+OpenDev=12V
void OpenDev(uint8 com,uint8* cmd)				//12
{
    at_build_PowerCtl_string(com,1);
}

//AT+CloseDev=CamAv
void CloseDev(uint8 com,uint8* cmd)				//13
{
    at_build_PowerCtl_string(com,0);
}

//AT+LinuxRst
void LinuxRst(uint8 com)						//14
{
	//GpioSetL(GPIO_CTL_ARM_RST);
	OSTimeDly(2);
	//GpioSetH(GPIO_CTL_ARM_RST);
	stprintf(com,"Reset Linux");		
}

//AT+Rf433Cfg=1 FN=255 dName=Rbt9999 CRC=4321
//AT+Rf433Cfg=0 FN=255 dName=Rbt9999 CRC=4321
void Rf433Cfg(uint8 com,uint8* cmd)				//15
{
    if( atoi( gCmdParaInfos.items[0].value ) == 1 ){
        gRbtState.bRf433Mode3 = true;
        GpioSetH(GPIO_CTR_RF433_M0);
        GpioSetH(GPIO_CTR_RF433_M1);
        stprintf(ID_DEBUG,"\r\nRf433Cfg set 1\r\n");
    }
    else{
        gRbtState.bRf433Mode3 = false;
        GpioSetL(GPIO_CTR_RF433_M0);
        GpioSetL(GPIO_CTR_RF433_M1);
        stprintf(ID_DEBUG,"\r\nRf433Cfg set 0\r\n");
    }
}

//AT+Rf433Set=c0,00,00,1d,17,46 FN=255 dName=Rbt9999 CRC=4321
//串口波特率=9600、校验位=8N1、模块地址=0、信道CH=23、接收响应时间=250ms
//空中传输速率=19200、输出功率=24dBm、发射时透明传输、功率输出模式=推挽/上拉
void Rf433Set(uint8 com,uint8* cmd)					//16
{
	u8 vals[10];
    int bytes = 0;
	
    bytes = box_str_get_bytes_by_hex(gCmdParaInfos.items[0].value,vals,10);
	nprintf(ID_DEBUG, bytes, 0, HEX);
    if( bytes>0 )
	{
		uart3_send(vals, bytes);		//发送设置指令至RF433模块
        OSTimeDly(OS_TICKS_PER_SEC/2);
    }
	//发送参数查询指令 0xc1 0xc1 0xc1
	vals[0]=0xc1;
	vals[1]=0xc1;
	vals[2]=0xc1;
	uart3_send(vals,3);
}

//AT+Lack  		 17
void Lack(uint8* cmd)					//17
{
	uint8 i;
	
	stprintf(ID_RF433,"本驻塔机器人系统的调试命令如下：\r\n");
	for( i=0;i<=ORDER_NUM;i++ )
	{
		stprintf(ID_RF433,(char*)CommandList[i]);
		stprintf(ID_RF433,"\r\n");
		OSTimeDly(OS_TICKS_PER_SEC/50);
	}
}

//AT+PtzSet=90,-20,0,10
//AT+PtzSet=90,-20,0,10 
void PtzSet(uint8 id)				//18
{
    int ValCnt;

	CurrentPtzSetCom=id;
    ValCnt = cmd_para_get_int_by_val_str( gCmdParaInfos.items[0].value , gCmdParaVal_Ints , 5 );
	
	//nprintf(1, CurrentPtzSetCom, 0, DEC);
	
    if(ValCnt!=4) 
    {
        stprintf(ID_DEBUG,"Ptz Cmd paras err\r\n");
        return;
    }
    else
    {   
        //stprintf(ID_DEBUG,"Ptz Cmd Set OK\r\n");
		if(gCmdParaVal_Ints[0]>270)			//方位角限定设置范围
		{
			gCmdParaVal_Ints[0]=270;
		}
		else if(gCmdParaVal_Ints[0]<-270)
		{
			gCmdParaVal_Ints[0]=-270;
		}
		else
		{

		}
		GTZMHD_Set.directAngle=gCmdParaVal_Ints[0]*10;
		
		if(gCmdParaVal_Ints[1]>30)			//俯仰角限定设置范围
		{
			gCmdParaVal_Ints[1]=30;
		}
		else if(gCmdParaVal_Ints[1]<-120)
		{
			gCmdParaVal_Ints[1]=-120;
		}
		else
		{

		}
		
		GTZMHD_Set.pitchAngle=gCmdParaVal_Ints[1]*10;
		//gCmdParaVal_Ints[2]为横滚角
		if(gCmdParaVal_Ints[3]>30)			//焦距限定设置范围
		{
			gCmdParaVal_Ints[3]=30;
		}
		else if(gCmdParaVal_Ints[3]<0)
		{
			gCmdParaVal_Ints[3]=1;
		}
		else
		{

		}
		GTZMHD_Set.nVisibleFocalDis=gCmdParaVal_Ints[3]*5;
        
    }
	GTZMHDCmdSetStep1=1;
}


//AT+PtzGet=1
//AT+PtzGet=1 name=Rbt9999
void PtzGet(uint8 id)						//19
{
    CurrentPtzSetCom=id;
	GTZMHDCmdGetStep1=1;
}

//AT+CFG2=16,10,40,6 a=1 dName=Rbt9999 CRC=9507
//从动轮参数 过桥速度，加速度，减速度，过桥时间
void SlvCFG2(uint8 com,uint8* cmd)			//20
{
    int ValCnt;
  
	if( CheckIsCmdOk(com,cmd , &gCmdParaInfos) == false )
	{
		stprintf(ID_DEBUG,"SlvCFG Cmd Err\r\n");
        return;
    }
    else if ( gCmdParaInfos.item_cnt < 3 )
	{
        stprintf(ID_DEBUG,"SlvCFG Cmd No Data\r\n");
        rt_get_SlvCFG(com);
        return;
    }
    else
    {
        //int vid;        
        cfg_oper_init();

        ValCnt = cmd_para_get_int_by_val_str( gCmdParaInfos.items[0].value , gCmdParaVal_Ints , 4 );
        if (ValCnt !=4) 
        {
            stprintf(ID_DEBUG,"MstMt Err Val\r\n");
            return;
        }
       
        //vid = cmd_para_get_id_by_name(&gCmdParaInfos,"SpeedBrg");
        //if ( vid > 0 ) 
        { gUserAllCfg.SlvMtCfg.mst_limit_on_bridge = gCmdParaVal_Ints[0]; }
        //vid = cmd_para_get_id_by_name(&gCmdParaInfos,"AddAccl");
        //if ( vid > 0 )
        { gUserAllCfg.SlvMtCfg.mstAddAccl = gCmdParaVal_Ints[1]; }
        //vid = cmd_para_get_id_by_name(&gCmdParaInfos,"DelAccl");
        //if ( vid > 0 ) 
        { gUserAllCfg.SlvMtCfg.mstDelAccl = gCmdParaVal_Ints[2]; }
        //vid = cmd_para_get_id_by_name(&gCmdParaInfos,"BrgTime");
        //if ( vid > 0 )
        { gUserAllCfg.SlvMtCfg.onBridgeTime = gCmdParaVal_Ints[3]; }
        
        cfg_oper_save_to_flash();  //进行状态返回
        rt_get_SlvCFG(com);
    }
}

//AT+StateSw=Hand FN=255 dName=Rbt9999 CRC=06d1
//AT+StateSw=Auto FN=255 dName=Rbt9999 CRC=06d1
//自动与手动状态切换指令 					21
void StateSw(uint8 com,uint8* cmd)				//21
{
	char* str = (char*)(g_zt_msg.sendbuf);
	
	//GpioSetL(GPIO_LED_SHOW1);
	
	if ( box_str_cmp(gCmdParaInfos.items[0].name , "StateSw" ) == 0 )
     {
         if( box_str_cmp(gCmdParaInfos.items[0].value , "Hand" ) == 0 )
		{
           gRbtState.StateSwitchFlag[0] = 'H' ;
		   str = cmd_para_build_para_string_str( str , "AT+StateSw" , "Hand" );
		   //str = cmd_para_build_para_string_str( str , "StateSw" , gRbtState.StateSwitchFlag );
		   //zt_build_send_state_string(BUILD_STATE_FLAG_ALL,ID_LINUX,8);
           //uart2_send(g_zt_msg.sendbuf , g_zt_msg.icmd_len );
         }
         else if( box_str_cmp(gCmdParaInfos.items[0].value , "Auto" ) == 0 )
		 {
            gRbtState.StateSwitchFlag[0] =  'A' ;
			str = cmd_para_build_para_string_str( str , "AT+StateSw" , "Auto" );
			//str = cmd_para_build_para_string_str( str , "StateSw" , gRbtState.StateSwitchFlag );
		    //zt_build_send_state_string(BUILD_STATE_FLAG_ALL,ID_LINUX,8);
        	//uart2_send(g_zt_msg.sendbuf , g_zt_msg.icmd_len );
         }
		 else
		 {
		 	 stprintf(ID_DEBUG,"AT+StateSw Cmd Err \r\n");

		 }
      }
	
	if(com==ID_DEBUG)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Debug);
    }
	else if(com==ID_LINUX)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Linux);
    }
	else if(com==ID_RF433)
    {
    	str = cmd_para_build_para_string_str( str , "FN" , Frame_No_Rf433);
    }
	str = cmd_para_build_para_string_str(str , "dName" , gRbtState.RobotName);
	
    *str++ = '\r';
    *str++ = '\n';
    *str   = 0;
    g_zt_msg.icmd_len = str - (char*)(g_zt_msg.sendbuf) ;

    ack_with_debug(com, g_zt_msg.sendbuf , g_zt_msg.icmd_len );
	
}


/*==================================

	函数：	StringCopmare
	说明：	比较两个字符串是否相同
	入口：	*pointer--源指针
			*DPointer----目标
	出口：	相同返回YES，不同返回NOT
	
===================================*/
uint8 StringCopmare(const char *SPointer,uint8 *DPointer)
{	
	while ( (*SPointer) != '\0' )
	{
		if ( *(SPointer++) != *(DPointer++) )
		{
			return NOT;
		}
	}
	return YES;
}
/*==================================

	函数：	FindingOrder
	说明：	从缓冲区里找命令字段
	入口：	*DPointer----目标
	出口：	有则返回找到的命令号，无则返回NOT
	
===================================*/
uint8 FindingOrder(uint8 *DPointer)
{
	uint8 i;
	uint8 temp=0xff;

	for ( i=0;i<=ORDER_NUM;i++ )
	{
		temp = StringCopmare(CommandList[i],DPointer);
		
		if ( temp==YES )
		{
			//nprintf(ID_DEBUG,i,0,DEC);
			//stprintf(1, "\r\n");
			return i;
		}
	}
	
	return NOT;
}

extern AtCmdInfo AtCmdFromDebug;
void CommDebug(uint8 id,uint8 *sp,uint16 length)
{	
	uint8 err,i;
	uint8 temp;
	uint8 *dp;
    uint8 com;
	char* str;

	//u8 buf[14];
    //signed long temp0;
	//uint8 i,j;
	
	length %= 512;
	com = id;
 
    if(com==ID_LINUX)
	{
		//stprintf(1, "\r\n接收到的命令串\r\n");
		//stprintf(1, "\r\nreceived\r\n");
		uart1_send(sp,length);//打印收到的命令串
    }
	
	OSSemPend(mem512_sem,0,&err);
	dp = OSMemGet(mem512ptr,&err);
	if(OS_ERR_NONE != err)
	{
		OSSemPost(mem512_sem);
		return;
	}

    /*for(j=0;j<30;j++)
    {
		for(i=0;i<CMD_PARA_STRING_LEN_NAME;i++)
		{
			gCmdParaInfos.items[j].name[i]=0;
	    }
		for(i=0;i<CMD_PARA_STRING_LEN_VALUE;i++)
		{
			gCmdParaInfos.items[j].value[i]=0;
	    }
    }*/

	//memset(&gCmdParaInfos,0,sizeof(CmdParaInfo));
	//for(i=0;i<length;i++)
	//{
		//*(str+i)=*(sp+i);
	//}
	
	 // str = (char*)(sp+3);
    //cmd_para_get_all_items( (char*)(str) , &gCmdParaInfos);
	           
    Clear_Buffer(dp,512); 
	CopyBuffer(sp,dp,length);
    //CopyBuffer(sp,dp,length-3);//减去尾部的\r \n 0三个字节

    str = (char*)(dp+3);         //减去开头 AT+ 三个字节
    cmd_para_get_all_items( (char*)(str) , &gCmdParaInfos);
	
    //if(com == ID_RF433)
   // if((com == ID_RF433)||(com == ID_LINUX)||(com == ID_DEBUG))
    {   
		if( (gCmdParaInfos.item_cnt>1) && (box_str_cmp("dName",gCmdParaInfos.items[gCmdParaInfos.item_cnt-2].name)==0) )
	    {
		    for(i=0;i<3;i++)
			{
				if(com == ID_DEBUG)
				{
					Frame_No_Debug[i]= gCmdParaInfos.items[gCmdParaInfos.item_cnt-3].value[i];
				}
				else if((com == ID_LINUX)&&(box_str_cmp("Hello",gCmdParaInfos.items[0].name)!=0))
				{ 
					Frame_No_Linux[i]= gCmdParaInfos.items[gCmdParaInfos.item_cnt-3].value[i];
				}
				else if(com == ID_RF433)
				{
					Frame_No_Rf433[i]= gCmdParaInfos.items[gCmdParaInfos.item_cnt-3].value[i];
				}
			} 


			if( box_str_cmp(gRbtState.RobotName,gCmdParaInfos.items[gCmdParaInfos.item_cnt-2].value)==0) 
		    {
				//GpioSetL(GPIO_LED_SHOW1);
				//字符串的结束位置进行更改
				//str[gCmdParaInfos.items[gCmdParaInfos.item_cnt-1].item_begin_pos] = 0;
				
				//str[gCmdParaInfos.items[gCmdParaInfos.item_cnt-2].item_begin_pos] = 0;//zs
	        	//gCmdParaInfos.item_cnt--; //参数数量进行修改
	        	//gCmdParaInfos.item_cnt=gCmdParaInfos.item_cnt-2;//zs
		    }
			else
			{
                if((gCmdParaInfos.item_cnt>1) && (box_str_cmp("Hello",gCmdParaInfos.items[0].name)==0))
	        	{
					Hello(com,dp);
					OSMemPut(mem512ptr,dp);//释放内存块  
					OSSemPost(mem512_sem);
					return;
	        	}
				else
				{
					OSMemPut(mem512ptr,dp);//释放内存块  
					OSSemPost(mem512_sem);
					return;
				}
			}
		}
		else
		{  
            OSMemPut(mem512ptr,dp);//释放内存块  
			OSSemPost(mem512_sem);
			return;
		}
    }
	
	temp = FindingOrder(dp);
    if ( temp!=NOT )
	{
		switch ( temp )
		{
			case 0:		 				//List
				ShowDebugList();
				break;				
			case 1:    					//system
				ShowSystemInformation();
				break;
			case 2:		
				ShowMemStates();		//加看门狗触发重启-保护措施
				break;
			case 3:    
				set_SystemReset();
				break;
			case 4:
				SetSlaveMotor(com,dp);
				break;
			case 5:
				SetMasterMotor(com,dp);
				break;
			case 6:
				Hello(com,dp);
				break;
			case 7:
				GetState(com,dp);
				break;
			case 8:
				SlvCFG1(com,dp);
				break;	
			case 9:
				GetSlvCFG(com);
				break;	
			case 10:
				BatCFG(com,dp);
				break;	
			case 11:
				Lcmd(com,dp);
				break;	
			case 12:
				OpenDev(com,dp);
				break;
			case 13:
				CloseDev(com,dp);
				break;	
			case 14:
				LinuxRst(com);
				break;
			case 15:
				Rf433Cfg(com,dp);
				//GpioSetL(GPIO_LED_SHOW1);
				break;	
			case 16:
				Rf433Set(com,dp);
				//GpioSetH(GPIO_LED_SHOW1);
				break;	
			case 17:
				Lack(dp);
				break;	
			case 18:
				//OSSemPost(PtzSem);
				PtzSet(com);
				break;
			case 19:
				//OSSemPost(PtzSem);
				PtzGet(com);
			  break;
			case 20:
				SlvCFG2(com,dp);
				break;
			case 21:
				StateSw(com,dp);
				break;
			default:
				break;
		}
	}

	OSMemPut(mem512ptr,dp);             //释放内存块  
	OSSemPost(mem512_sem);
}

void GTZMHDDebug(uint8 *sp,uint16 length)
{
    uint8 err;
	uint8 *dp;

	OSSemPend(mem512_sem,0,&err);
	dp = OSMemGet(mem512ptr,&err);
	if(OS_ERR_NONE != err)
	{
		OSSemPost(mem512_sem);
		return;
	}
	           
    Clear_Buffer(dp,512); 
	CopyBuffer(sp,dp,length);
	
	if((*dp==0xbb)&&(*(dp+4)==0x1a)&&(*(dp+5)==0x44))//设置时返回指令
	{
		GTZMHDCmdSetOk=1;
	}

	if(GTZMHDCmdGetStep1==1)//读取姿态时返回指令 方位角、俯仰角
	{
		GTZMHD_Get.directAngle=((s16)(*(dp+1))<<8)|(s16)(*(dp+2));
		GTZMHD_Get.pitchAngle=((s16)(*(dp+3))<<8)|(s16)(*(dp+4));
    }

	if(GTZMHDCmdGetStep2==1)//读取焦距时返回指令 焦距
	{
		GTZMHD_Get.nVisibleFocalDis=((s16)(*(dp+1))<<8)|((s16)(*(dp+2)));
		if(GTZMHD_Get.nVisibleFocalDis!=0)
		{
			GTZMHDCmdGetOk=1;
		}
    }

	OSMemPut(mem512ptr,dp);  //释放内存块  
	OSSemPost(mem512_sem);
}

void GPSGnrmcDebug(uint8 *sp,uint16 length)
{
    uint8 err;
	uint8 *dp;

    //stprintf(1, "\r\n串口4接收到的数据\r\n");
	//uart1_send(sp,length);//打印收到的命令串
	
	OSSemPend(mem512_sem,0,&err);
	dp = OSMemGet(mem512ptr,&err);
	if(OS_ERR_NONE != err)
	{
		OSSemPost(mem512_sem);
		return;
	}
	           
    Clear_Buffer(dp,512); 
	CopyBuffer(sp,dp,length);
	
    gGPS.bCurGetTime=GpsParaTimeDateForGNRMC((char *)dp);
    AtCmdFromGPS.CurRecStatus = COMM_REC_WAITPREFIX;
	AtCmdFromGPS.recv_buf[4]=0;
	AtCmdFromGPS.recv_buf[5]=0;
	AtCmdFromGPS.counter = 0;
	
	
	OSMemPut(mem512ptr,dp);  //释放内存块  
	OSSemPost(mem512_sem);
}

void GPSGnggaDebug(uint8 * sp, uint16 length)
{
    uint8 err;
	uint8 *dp;

    //stprintf(1, "\r\n串口4接收到的数据\r\n");
	//uart1_send(sp,length);//打印收到的命令串
	
	OSSemPend(mem512_sem,0,&err);
	dp = OSMemGet(mem512ptr,&err);
	if(OS_ERR_NONE != err)
	{
		OSSemPost(mem512_sem);
		return;
	}
	           
    Clear_Buffer(dp,512); 
	CopyBuffer(sp,dp,length);
	
    gGPS.bCurGetPos=GpsParaPosDateFromGNGGA((char *)dp);
    AtCmdFromGPS.CurRecStatus = COMM_REC_WAITPREFIX;
	AtCmdFromGPS.recv_buf[4]=0;
	AtCmdFromGPS.recv_buf[5]=0;
	AtCmdFromGPS.counter = 0;
	
	
	OSMemPut(mem512ptr,dp);  //释放内存块  
	OSSemPost(mem512_sem);
}

void Task_Local(void *pdata)
{
	DATA_CONTROL_BLOCK *msg;
	uint8 err;
	
	//OSTimeDly(OS_TICKS_PER_SEC);
	//BaseShutUp();

	while ( 1 )
	{
		msg = (DATA_CONTROL_BLOCK*)OSQPend(LocalQueue,0,&err);
		if ( err == OS_ERR_NONE )
		{
			switch ( msg->type )
			{
				case MOTOR_MSG:
				{
					//GpioSetL(GPIO_LED_SHOW1);
					//GpioSetL(GPIO_LED_OUT_SHOW1);
					TaskFun_MotorUpdate();
				}
				    break;
				default:
				    break;
			}		
			OSMemPut(p_msgBuffer,(void*)msg);
		}	

	}

}
	
