#ifndef _TASK_LOCAL_H_

//extern uint16 OSVersion(void);

enum{
	DEC = 0,		// ʮ����
	HEX,			//ʮ������
	STR
};

/************************************************* 
*Function:	zt_build_send_state_string 	
*Return:      ����״̬�ַ����ĳ���
*DESCRIPTION: ���ɻ����˵�״̬�ַ���
*u32 stateFlag ��Ҫ�������ݵı�־
*************************************************/
int zt_build_send_state_string(u32 stateFlag,uint8 id,uint8 kind);
enum{
    BUILD_STATE_FLAG_BAT      = 0X01 , // B �����Ϣ
    BUILD_STATE_FLAG_AM2320   = 0X02 , // A ��ʪ����Ϣ
    BUILD_STATE_FLAG_JY901    = 0X04 , // J ���ٶȺͷ�λ��Ϣ
    BUILD_STATE_FLAG_PRESS    = 0X08 , // P ѹ��ֵ��Ϣ
    BUILD_STATE_FLAG_MSTMT    = 0X10 , // M �����ֵ����Ϣ
    BUILD_STATE_FLAG_SLVMT    = 0X20 , // S �Ӷ��ֵ����Ϣ
    BUILD_STATE_FLAG_SLVWH    = 0X40 , // W �Ӷ��� �����ٶ���Ϣ
    BUILD_STATE_FLAG_XWSW     = 0X80 , // X ��λ������Ϣ
    BUILD_STATE_FLAG_CHARGE   = 0X100 ,// C ���ӿ������Ϣ
    BUILD_STATE_FLAG_GPS      = 0X200 ,// G GPS �����Ϣ
    BUILD_STATE_FLAG_ALL      = 0XFFFF , // L  ����״̬
};


extern void Task_Local(void *pdata);
extern void DisplayTime(uint8 type);
extern void StoreWithProtect(uint8 *sp,uint16 CRC_result,uint16 length,uint32 StoreAddr,uint32 StoreBackAddr);
extern OS_EVENT *LocalQueue;		//����0������������
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
