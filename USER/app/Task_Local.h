#ifndef _TASK_LOCAL_H_

//extern uint16 OSVersion(void);

enum{
	DEC = 0,		// 十进制
	HEX,			//十六进制
	STR
};

/************************************************* 
*Function:	zt_build_send_state_string 	
*Return:      返回状态字符串的长度
*DESCRIPTION: 生成机器人的状态字符串
*u32 stateFlag 需要发送数据的标志
*************************************************/
int zt_build_send_state_string(u32 stateFlag,uint8 id,uint8 kind);
enum{
    BUILD_STATE_FLAG_BAT      = 0X01 , // B 电池信息
    BUILD_STATE_FLAG_AM2320   = 0X02 , // A 温湿度信息
    BUILD_STATE_FLAG_JY901    = 0X04 , // J 加速度和方位信息
    BUILD_STATE_FLAG_PRESS    = 0X08 , // P 压力值信息
    BUILD_STATE_FLAG_MSTMT    = 0X10 , // M 主动轮电机信息
    BUILD_STATE_FLAG_SLVMT    = 0X20 , // S 从动轮电机信息
    BUILD_STATE_FLAG_SLVWH    = 0X40 , // W 从动轮 轮子速度信息
    BUILD_STATE_FLAG_XWSW     = 0X80 , // X 限位开关信息
    BUILD_STATE_FLAG_CHARGE   = 0X100 ,// C 充电接口相关信息
    BUILD_STATE_FLAG_GPS      = 0X200 ,// G GPS 相关信息
    BUILD_STATE_FLAG_ALL      = 0XFFFF , // L  所有状态
};


extern void Task_Local(void *pdata);
extern void DisplayTime(uint8 type);
extern void StoreWithProtect(uint8 *sp,uint16 CRC_result,uint16 length,uint32 StoreAddr,uint32 StoreBackAddr);
extern OS_EVENT *LocalQueue;		//串口0接收数据邮箱
extern void CommDebug(uint8 id,uint8 *sp,uint16 length);
extern void GTZMHDDebug(uint8 *sp,uint16 length);
extern void GPSGnrmcDebug(uint8 *sp,uint16 length);
extern void GPSGnggaDebug(uint8 *sp,uint16 length);
extern void ShowSystemInformation(void);
extern void write_log(INT16U taskID,uint16 err_code);
extern void ShowLogs(void);
extern uint8 vib_err_counter;
extern void BaseShutUp(void);

extern void debug_nprintf(uint8 com,uint32 num,uint8 mul,uint8 typ);
#define nprintf(com,p,m,n) debug_nprintf(com,p, m, n)
extern void debug_sprintf(uint8 com,char *str);
#define stprintf(com,p) debug_sprintf(com,p)

extern uint8 CheckIsCmdOkCheckIsCmdOk( __IN__ const uint8* cmd, __OUT_ CmdParaInfo* pParaInfo);
int zt_build_send_state_string(u32 stateFlag,uint8 id,uint8 kind);
//void Ptz_GetRst_string(uint8 id,uint8 rst);
void Ptz_GetRst_string(uint8 rst);
void Ptz_SetRst_string(uint8 id,uint8 rst);
void SetSlaveMotor(uint8 com,uint8* cmd);
void SetMasterMotor(uint8 com,uint8* cmd);//3           
void Hello(uint8 com,uint8* cmd);//3           
void GetState(uint8 com,uint8* cmd);
void SlvCFG(uint8 com,uint8* cmd);
void GetSlvCFG(uint8 com);
void BatCFG(uint8 com,uint8* cmd);
void Lcmd(uint8 com,uint8* cmd);
void OpenDev(uint8 com,uint8* cmd);
void CloseDev(uint8 com,uint8* cmd);
void LinuxRst(uint8 com);
void Rf433Cfg(uint8 com,uint8* cmd);
void Rf433Set(uint8 com,uint8* cmd);
void PtzSet(uint8 cmd);
void PtzGet(uint8 cmd);
void StateSw(uint8 com,uint8* cmd);

void ack_with_debug(uint8 com,uint8 *buf, uint16 len);
static void at_build_mtSetBk_string(uint8 com,uint8 rst,uint8 kind);
static void at_build_PowerCtl_string(uint8 com,uint8 kind);
void rt_get_SlvCFG(uint8 com);
void cfg_oper_init(void);
void flash_write_u32_datas(u32 addr,const void* pds, int u32_cnt);
void cfg_oper_save_to_flash(void);
void rt_get_BatCFG(uint8 com);
bool OpenOrCloseDevOperate(char* devName,bool bOpen);
u8 char_to_byte_val(u8 ch);
int box_str_get_bytes_by_hex(const char* str,u8* pDataOut,int dataCntMax);

void zt_slv_mt_determine_need_move(void);

void SLV_AUTO_SPEED_UP(void);
void SLV_AUTO_SPEED_DOWN(void);





#endif
