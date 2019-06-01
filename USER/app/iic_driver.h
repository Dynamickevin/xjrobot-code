//stm32f103 iic ��BoxOs�µ�����
//һ��IIC�ӿڿ��ԽӶ���豸
//����豸����ͬһ��IIC�ӿڣ�ĳһ��ģ����Ҫʹ��IIC�ӿ�ģ��
//������IIC�������У�ÿ�����������һ����������
//���ĳ��ʹ��IIC��ģ��ͬʱ�����ж���������У����ģ���ڲ�������е�ʹ��
/**************************IIC ʹ��ʾ������***************************
IIC_Operation IicOperSend;  //���Ͷ���Ԫ��
void IicSendFinCb(void* pPara)
{
    //todo ������Ϻ�ĸ��ֲ���
    //������ݶ��� IicOperSend �ṹ����
}

void ____module_init()
{
iic_init_operation(
        &IicOperSend ,  // pOperation,    //������Ҫ���еĲ����������Ƚ�������
        0X33 ,          // devAddr,        //�豸��ַ
        0 ,             // IicOperFlag ,   //��������ر�־
        IicSendFinCb ,  // pFinishCb,      //�������ʱ�Ļص�����;��������ж��б�����
        NULL            // pFinishCbPara   //��ɻص���������
        );
}

//��Ҫ���ͺ���λ�õ���
u8 send_datas[20];
{
    
    iic_send_data(
        IIC_PORT_1      ,// piic,           //���� IIC_PORT_1 ����
        &IicOperSend    ,// pOperation,     //������Ҫ���еĲ������ڸú����б�����
        0X44            ,// regAddr,        //�Ĵ�����ַ��Ҳ������ֵ�������豸û��ʹ�üĴ�����ַ����������
        send_datas      ,// *toSend,        //��Ҫ���͵�����
        5                // nToSend         //��Ҫ���͵����ݳ���
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
    u8*             pOperBuf;  //��ǰ������Ҫ��Ӧ������
    int             nOperBuf;  //��Ҫ���������ݳ���
    u16             IicOperFlag; //��������ر�־  
    u8              devAddr;   //�豸��ַ
    u8              regAddr;   //�Ĵ�����ַ
}IIC_Operation;

typedef struct
{
    I2C_TypeDef*    pRegs   ;  //�Ĵ�����ʼ��ַ
    uint8_t         u8_NVIC_IRQChannel;
    ListCtrl        iic_list;  //��Ҫִ�е�iic����
    
    //IIC���β�����Ҫ�ĸ���״̬������
    IIC_Operation*  pCurOper;  //��ǰ����ִ�е�iic����
    u32         int_oper_fun_execute_pos;  //�жϴ������ǰλ��
    SoftTimer   delay_timer;
    int         iCurOperDataCnt; //��ǰ���ڴ���Ĳ�����������
    u16         gCurIICState   ; //��ǰ���ڴ���Ĳ�����״̬�����͵�ַ���ȴ�ACK1�����ͼĴ������ȴ�ACK2���շ����ݣ���״̬
    u16         redo_times     ; //ĳ�����������Դ��������̫����Ҫ �����쳣����
}IIC_DriverCtrl;
extern IIC_DriverCtrl DriverIIC1;
#define IIC_PORT_1    (&DriverIIC1)

enum{
    IIC_OPER_FLAG_NULL  = 0 ,       //�޲���
    IIC_OPER_FLAG_READ  = 0X0001 ,  //��ȡ����
    IIC_OPER_FLAG_WRITE = 0X0002 ,  //д�����ݵ�IIC�豸
    IIC_OPER_FLAG_REG   = 0X0004 ,  //�趨��ǰ�Ĵ���
    IIC_OPER_FLAG_NO_ACK  = 0X0008 ,  //����Ҫ��� ACK �ź�
    IIC_OPER_FLAG_NO_STOP   = 0X0010 ,  //����Ҫ����STOP�ź�
};


void iic1_init(void);

//ÿ��IIC_Operation�������һ�� iic_init_operation ����������ֻ�ܵ���һ��
void iic_init_operation(
    IIC_Operation*  pOperation,     //������Ҫ���еĲ����������Ƚ�������
    u8              devAddr,        //�豸��ַ
    VoidParaFuncPtr pFinishCb,      //�������ʱ�Ļص�����;��������ж��б�����
    void*           pFinishCbPara   //��ɻص���������
    );

void iic_add_operation(
    IIC_DriverCtrl* piic,           //���� IIC_PORT_1 ����
    IIC_Operation* pOperation ,     //������Ҫ���еĲ����������Ƚ�������
    ListAddType     addType
    );
void iic_send_data(
    IIC_DriverCtrl* piic,           //���� IIC_PORT_1 ����
    IIC_Operation*  pOperation,     //������Ҫ���еĲ������ڸú����б�����
    u8              regAddr,        //�Ĵ�����ַ��Ҳ������ֵ�������豸û��ʹ�üĴ�����ַ����������
    u8              *toSend,        //��Ҫ���͵�����
    int             nToSend         //��Ҫ���͵����ݳ���
    );
void iic_send_data_ex(
    IIC_DriverCtrl* piic,           //���� IIC_PORT_1 ����
    IIC_Operation* pOperation,      //������Ҫ���еĲ������ڸú����б�����
    u8              devAddr,        //�豸��ַ
    u8              regAddr,        //�Ĵ�����ַ��Ҳ������ֵ�������豸û��ʹ�üĴ�����ַ����������
    u8              *toSend,        //��Ҫ���͵�����
    int             nToSend,        //��Ҫ���͵����ݳ���
    BX_UINT         IicOperFlag ,   //��������ر�־  IIC_OPER_FLAG_WRITE|IIC_OPER_FLAG_REG
    VoidParaFuncPtr pFinishCb,      //�������ʱ�Ļص�����;��������ж��б�����
    void*           pFinishCbPara , //��ɻص���������
    ListAddType     addType
    );

void iic_read_data(
    IIC_DriverCtrl* piic,           //���� IIC_PORT_1 ����
    IIC_Operation* pOperation,      //������Ҫ���еĲ������ڸú����б�����
    u8              regAddr,        //�Ĵ�����ַ��Ҳ������ֵ�������豸û��ʹ�üĴ�����ַ����������
    u8              *toRead,        //��Ҫ���յ�����
    int             nToRead         //��Ҫ���յ����ݳ���
    );
void iic_read_data_ex(
    IIC_DriverCtrl* piic,           //���� IIC_PORT_1 ����
    IIC_Operation* pOperation,      //������Ҫ���еĲ������ڸú����б�����
    u8              devAddr,        //�豸��ַ
    u8              regAddr,        //�Ĵ�����ַ��Ҳ������ֵ�������豸û��ʹ�üĴ�����ַ����������
    u8              *toRead,        //��Ҫ���յ�����
    int             nToRead,        //��Ҫ���յ����ݳ���
    BX_UINT         IicOperFlag ,   //��������ر�־  IIC_OPER_FLAG_READ|IIC_OPER_FLAG_REG
    VoidParaFuncPtr pFinishCb,      //�������ʱ�Ļص�����;��������ж��б�����
    void*           pFinishCbPara , //��ɻص���������
    ListAddType     addType
    );

//��Ӳ����صĺ���
void IIC1_Hard_Init(void) ;  //IO�ڳ�ʼ�����Ĵ�����ʼ�����жϳ�ʼ��
void IIC_StartIntr(I2C_TypeDef*  pRegs,IIC_Operation* pOper);  //����һ������
    
void IIC_SendStop(IIC_DriverCtrl* piic);  //����ǿ�Ʒ���һ��ֹͣ�ź�

//IIC ״̬�Ⱥ궨��

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

