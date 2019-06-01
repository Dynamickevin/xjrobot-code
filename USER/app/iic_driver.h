//stm32f103 iic 在BoxOs下的驱动
//一个IIC接口可以接多个设备
//多个设备共用同一个IIC接口，某一个模块需要使用IIC接口模块
//定义多个IIC操作队列，每个操作必须绑定一个操作队列
//如果某个使用IIC的模块同时可能有多个操作队列，则该模块内部管理队列的使用
/**************************IIC 使用示例代码***************************
IIC_Operation IicOperSend;  //发送队列元素
void IicSendFinCb(void* pPara)
{
    //todo 处理完毕后的各种操作
    //相关数据都在 IicOperSend 结构体中
}

void ____module_init()
{
iic_init_operation(
        &IicOperSend ,  // pOperation,    //传入需要进行的操作，必须先进行配置
        0X33 ,          // devAddr,        //设备地址
        0 ,             // IicOperFlag ,   //操作的相关标志
        IicSendFinCb ,  // pFinishCb,      //操作完成时的回调函数;会在软件中断中被调用
        NULL            // pFinishCbPara   //完成回调函数参数
        );
}

//需要发送函数位置调用
u8 send_datas[20];
{
    
    iic_send_data(
        IIC_PORT_1      ,// piic,           //传入 IIC_PORT_1 即可
        &IicOperSend    ,// pOperation,     //传入需要进行的操作，在该函数中被配置
        0X44            ,// regAddr,        //寄存器地址，也是命令值（部分设备没有使用寄存器地址概念，而是命令）
        send_datas      ,// *toSend,        //需要发送的数据
        5                // nToSend         //需要发送的数据长度
    );
}

**********************************************************************/

#ifndef  __IIC_DRIVER__H
#define  __IIC_DRIVER__H

//#include "BoxOs.h"
#include "OperationList.h"

typedef struct
{
    OperationQueue  oq;
    u8*             pOperBuf;  //当前操作需要对应缓冲区
    int             nOperBuf;  //需要操作的数据长度
    u16             IicOperFlag; //操作的相关标志  
    u8              devAddr;   //设备地址
    u8              regAddr;   //寄存器地址
}IIC_Operation;

typedef struct
{
    I2C_TypeDef*    pRegs   ;  //寄存器开始地址
    uint8_t         u8_NVIC_IRQChannel;
    ListCtrl        iic_list;  //需要执行的iic操作
    
    //IIC单次操作需要的各种状态及变量
    IIC_Operation*  pCurOper;  //当前正在执行的iic操作
    u32         int_oper_fun_execute_pos;  //中断处理程序当前位置
    SoftTimer   delay_timer;
    int         iCurOperDataCnt; //当前正在处理的操作的数据量
    u16         gCurIICState   ; //当前正在处理的操作的状态：发送地址，等待ACK1，发送寄存器，等待ACK2，收发数据，等状态
    u16         redo_times     ; //某个操作的重试次数；如果太多需要 进行异常处理
}IIC_DriverCtrl;
extern IIC_DriverCtrl DriverIIC1;
#define IIC_PORT_1    (&DriverIIC1)

enum{
    IIC_OPER_FLAG_NULL  = 0 ,       //无操作
    IIC_OPER_FLAG_READ  = 0X0001 ,  //读取数据
    IIC_OPER_FLAG_WRITE = 0X0002 ,  //写入数据到IIC设备
    IIC_OPER_FLAG_REG   = 0X0004 ,  //设定当前寄存器
    IIC_OPER_FLAG_NO_ACK  = 0X0008 ,  //不需要检查 ACK 信号
    IIC_OPER_FLAG_NO_STOP   = 0X0010 ,  //不需要发送STOP信号
};


void iic1_init(void);

//每个IIC_Operation必须调用一次 iic_init_operation 函数；而且只能调用一次
void iic_init_operation(
    IIC_Operation*  pOperation,     //传入需要进行的操作，必须先进行配置
    u8              devAddr,        //设备地址
    VoidParaFuncPtr pFinishCb,      //操作完成时的回调函数;会在软件中断中被调用
    void*           pFinishCbPara   //完成回调函数参数
    );

void iic_add_operation(
    IIC_DriverCtrl* piic,           //传入 IIC_PORT_1 即可
    IIC_Operation* pOperation ,     //传入需要进行的操作，必须先进行配置
    ListAddType     addType
    );
void iic_send_data(
    IIC_DriverCtrl* piic,           //传入 IIC_PORT_1 即可
    IIC_Operation*  pOperation,     //传入需要进行的操作，在该函数中被配置
    u8              regAddr,        //寄存器地址，也是命令值（部分设备没有使用寄存器地址概念，而是命令）
    u8              *toSend,        //需要发送的数据
    int             nToSend         //需要发送的数据长度
    );
void iic_send_data_ex(
    IIC_DriverCtrl* piic,           //传入 IIC_PORT_1 即可
    IIC_Operation* pOperation,      //传入需要进行的操作，在该函数中被配置
    u8              devAddr,        //设备地址
    u8              regAddr,        //寄存器地址，也是命令值（部分设备没有使用寄存器地址概念，而是命令）
    u8              *toSend,        //需要发送的数据
    int             nToSend,        //需要发送的数据长度
    BX_UINT         IicOperFlag ,   //操作的相关标志  IIC_OPER_FLAG_WRITE|IIC_OPER_FLAG_REG
    VoidParaFuncPtr pFinishCb,      //操作完成时的回调函数;会在软件中断中被调用
    void*           pFinishCbPara , //完成回调函数参数
    ListAddType     addType
    );

void iic_read_data(
    IIC_DriverCtrl* piic,           //传入 IIC_PORT_1 即可
    IIC_Operation* pOperation,      //传入需要进行的操作，在该函数中被配置
    u8              regAddr,        //寄存器地址，也是命令值（部分设备没有使用寄存器地址概念，而是命令）
    u8              *toRead,        //需要接收的数据
    int             nToRead         //需要接收的数据长度
    );
void iic_read_data_ex(
    IIC_DriverCtrl* piic,           //传入 IIC_PORT_1 即可
    IIC_Operation* pOperation,      //传入需要进行的操作，在该函数中被配置
    u8              devAddr,        //设备地址
    u8              regAddr,        //寄存器地址，也是命令值（部分设备没有使用寄存器地址概念，而是命令）
    u8              *toRead,        //需要接收的数据
    int             nToRead,        //需要接收的数据长度
    BX_UINT         IicOperFlag ,   //操作的相关标志  IIC_OPER_FLAG_READ|IIC_OPER_FLAG_REG
    VoidParaFuncPtr pFinishCb,      //操作完成时的回调函数;会在软件中断中被调用
    void*           pFinishCbPara , //完成回调函数参数
    ListAddType     addType
    );

//与硬件相关的函数
void IIC1_Hard_Init(void) ;  //IO口初始化，寄存器初始化，中断初始化
void IIC_StartIntr(I2C_TypeDef*  pRegs,IIC_Operation* pOper);  //启动一个操作
    
void IIC_SendStop(IIC_DriverCtrl* piic);  //总线强制发送一个停止信号

//IIC 状态等宏定义

/* I2C SPE mask */
#define CR1_PE_Set              ((uint16_t)0x0001)
#define CR1_PE_Reset            ((uint16_t)0xFFFE)

/* I2C START mask */
#define CR1_START_Set           ((uint16_t)0x0100)
#define CR1_START_Reset         ((uint16_t)0xFEFF)

#define CR1_POS_Set           ((uint16_t)0x0800)
#define CR1_POS_Reset         ((uint16_t)0xF7FF)

/* I2C STOP mask */
#define CR1_STOP_Set            ((uint16_t)0x0200)
#define CR1_STOP_Reset          ((uint16_t)0xFDFF)

/* I2C ACK mask */
#define CR1_ACK_Set             ((uint16_t)0x0400)
#define CR1_ACK_Reset           ((uint16_t)0xFBFF)

/* I2C ENARP mask */
#define CR1_ENARP_Set           ((uint16_t)0x0010)
#define CR1_ENARP_Reset         ((uint16_t)0xFFEF)

/* I2C NOSTRETCH mask */
#define CR1_NOSTRETCH_Set       ((uint16_t)0x0080)
#define CR1_NOSTRETCH_Reset     ((uint16_t)0xFF7F)

/* I2C registers Masks */
#define CR1_CLEAR_Mask          ((uint16_t)0xFBF5)

/* I2C DMAEN mask */
#define CR2_DMAEN_Set           ((uint16_t)0x0800)
#define CR2_DMAEN_Reset         ((uint16_t)0xF7FF)

/* I2C LAST mask */
#define CR2_LAST_Set            ((uint16_t)0x1000)
#define CR2_LAST_Reset          ((uint16_t)0xEFFF)

/* I2C FREQ mask */
#define CR2_FREQ_Reset          ((uint16_t)0xFFC0)

/* I2C ADD0 mask */
#define OAR1_ADD0_Set           ((uint16_t)0x0001)
#define OAR1_ADD0_Reset         ((uint16_t)0xFFFE)

/* I2C ENDUAL mask */
#define OAR2_ENDUAL_Set         ((uint16_t)0x0001)
#define OAR2_ENDUAL_Reset       ((uint16_t)0xFFFE)

/* I2C ADD2 mask */
#define OAR2_ADD2_Reset         ((uint16_t)0xFF01)

/* I2C F/S mask */
#define CCR_FS_Set              ((uint16_t)0x8000)

/* I2C CCR mask */
#define CCR_CCR_Set             ((uint16_t)0x0FFF)

/* I2C FLAG mask */
#define FLAG_Mask               ((uint32_t)0x00FFFFFF)

/* I2C Interrupt Enable mask */
#define ITEN_Mask               ((uint32_t)0x07000000)


#define I2C_IT_BUF                      ((uint16_t)0x0400)
#define I2C_IT_EVT                      ((uint16_t)0x0200)
#define I2C_IT_ERR                      ((uint16_t)0x0100)


#define I2C_DIRECTION_TX 0
#define I2C_DIRECTION_RX 1

#define OwnAddress1 0x28
#define OwnAddress2 0x30


#define I2C1_DMA_CHANNEL_TX           DMA1_Channel6
#define I2C1_DMA_CHANNEL_RX           DMA1_Channel7

#define I2C2_DMA_CHANNEL_TX           DMA1_Channel4
#define I2C2_DMA_CHANNEL_RX           DMA1_Channel5

#define I2C1_DR_Address              0x40005410
#define I2C2_DR_Address              0x40005810
    


#endif //__IIC_DRIVER__H

