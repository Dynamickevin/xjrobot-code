#include <includes.h>
#include "iic_driver.h"

#ifndef BX_CALL
#define BX_CALL(FUN_NAME)   FUN_NAME
#endif

IIC_DriverCtrl DriverIIC1 = { I2C1 , I2C1_EV_IRQn  };

void iic_start_one_operation_from_list(IIC_DriverCtrl* piic);

/************************************************* 
*Function:		IicDriverDelayTimerCb
*Input:			SoftTimer*  tmr ��ʱ������
*               
*OUTPUT:		void
*Return:		void
*DESCRIPTION:  ��ʱ���ص�������������IICһ��ͨ����Ϻ�
*           ����ʱ������̣�ͨ����ʱ�����������ʱ������
*       1.IIC���߷����� STOP λ�󣬵�һ��ʱ����Ҫ�ر�IICһ�Σ�����쳣��־λ
*       2.IIC����һ�δ�����ɺ���Ҫ�ȴ�һ��ʱ����������һ��IIC��ͨ�Ų���
*************************************************/
void IicDriverDelayTimerCb(SoftTimer* timer)
{
    IIC_DriverCtrl* piic = (IIC_DriverCtrl*)(timer->CallbackArg) ;
    
    if( piic->gCurIICState < 10 ) //�ڶ�ȡ����ʱ������Ϊ���ͼĴ������ٴη�����ʼ��Ϣ����Ҫ�ȴ�һ��ʱ�䷢��
    {
        if( 0 == DriverIIC1.int_oper_fun_execute_pos ) //��δ���չ��жϣ���Ҫ��������IIC
        {
            if( I2C1 == piic->pRegs )
            {
                //to do    iic Ӳ����ʼ������  piic->pRegs 
                piic->pRegs->CR1 |= 0X8000; //SWRST ��λ
                RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 ,DISABLE);
                IIC1_Hard_Init();
            }
        }
        else
        {
            piic->redo_times++;
            if( piic->redo_times < 2 )
            {
                piic->pRegs->CR2 |= I2C_IT_EVT;  /* Enable EVT IT*/
                piic->pRegs->SR1 |= 0X0001 ;
                piic->pRegs->SR2 |= 0X0003 ;
                BX_CALL(soft_timer_start)( timer );
                return;
            }
        }
        piic->pRegs->CR2 &= ~I2C_IT_EVT;
        piic->pRegs->CR2 &= ~I2C_IT_BUF;
        piic->pRegs->CR2 &= ~I2C_IT_ERR;
        
        piic->gCurIICState = 88 ;  //iic err
        if( 0 == (piic->pCurOper->IicOperFlag&IIC_OPER_FLAG_NO_STOP) )
        {
            I2C_GenerateSTOP(piic->pRegs,ENABLE); 
        }
        piic->pCurOper->oq.sOperState = OPER_QUEUE_STATE_ERR3;
        piic->int_oper_fun_execute_pos = 0;
    }
    if( piic->pCurOper )
    {
        if( OPER_QUEUE_STATE_OPER_END != piic->pCurOper->oq.queueState )
        {
            OS_CPU_SR  cpu_sr = 0;
            OS_ENTER_CRITICAL();
            piic->pCurOper->oq.queueState = OPER_QUEUE_STATE_OPER_END ;  //����״̬Ϊ�������
            OS_EXIT_CRITICAL();
            if( 0 == (piic->pCurOper->IicOperFlag&IIC_OPER_FLAG_NO_STOP) )
            {
            }
            else{
                I2C_Cmd(piic->pRegs , DISABLE);
            }
            if ( piic->pCurOper->oq.pFinishCb )
            {
                ( *(piic->pCurOper->oq.pFinishCb) )( piic->pCurOper->oq.pFinishCbPara ) ;  //���� �ص�����
            }
            BX_CALL(soft_timer_start)( timer );
            return;
        }
    }
    
    {
        OS_CPU_SR  cpu_sr = 0;
        OS_ENTER_CRITICAL();
        piic->pCurOper->oq.queueState = OPER_QUEUE_STATE_NULL ;  //
        DriverIIC1.pCurOper      = NULL ;
        OS_EXIT_CRITICAL();

        iic_start_one_operation_from_list(piic);  //����������һ���µĲ���
        return;
    }    
}

void iic1_init( void   )
{
    DriverIIC1.pRegs    = I2C1;
    DriverIIC1.pCurOper = NULL;
    DriverIIC1.gCurIICState    = 0;
    DriverIIC1.iCurOperDataCnt = 0;
    BoxOsMemSet( &(DriverIIC1.iic_list) , 0 , sizeof(DriverIIC1.iic_list) );

    soft_timer_init_timer( 
        &DriverIIC1.delay_timer ,
        3,
        (VoidParaFuncPtr)(IicDriverDelayTimerCb) ,
        &DriverIIC1 ,
        false 
    );
        
    //to do    iic Ӳ����ʼ������  piic->pRegs 
    IIC1_Hard_Init();
}

//ÿ��IIC_Operation�������һ�� iic_init_operation ����������ֻ�ܵ���һ��
void iic_init_operation(
    IIC_Operation*  pOperation,     //������Ҫ���еĲ����������Ƚ�������
    u8              devAddr,        //�豸��ַ
    VoidParaFuncPtr pFinishCb,      //�������ʱ�Ļص�����;��������ж��б�����
    void*           pFinishCbPara   //��ɻص���������
    )
{
    BoxOsMemSet( pOperation , 0 , sizeof(IIC_Operation) );
    pOperation->devAddr         = devAddr ;
    pOperation->oq.pFinishCb    = pFinishCb ;
    pOperation->oq.pFinishCbPara = pFinishCbPara ;
}

void iic_start_one_operation_from_list(IIC_DriverCtrl* piic)
{
    OS_CPU_SR  cpu_sr = 0;
    OS_ENTER_CRITICAL();
    
    if ( NULL == piic->pCurOper ) //ֻ�е�ǰû���κβ���ʱ���ܽ������洦��
    {
        piic->pCurOper = (IIC_Operation*)( OperationQueuePick(&(piic->iic_list)) );
        if ( piic->pCurOper )  //��ʾ��ǰ��������Ҫ���д���
        {
            BX_CALL(soft_timer_start)( &(piic->delay_timer) );  //��������Ƿ��쳣�Ķ�ʱ��
            piic->int_oper_fun_execute_pos = 0;
            piic->iCurOperDataCnt = 0;
            piic->gCurIICState    = 0;
            piic->redo_times      = 0;
            //to do  ����IICӲ���ж�
            IIC_StartIntr( piic->pRegs , piic->pCurOper );
        }
        else{
            BX_CALL(soft_timer_stop)( &(piic->delay_timer) );
        }
    }

    OS_EXIT_CRITICAL();
}

void iic_add_operation(
    IIC_DriverCtrl* piic,           //���� IIC_PORT_1 ����
    IIC_Operation* pOperation ,     //������Ҫ���еĲ����������Ƚ�������
    ListAddType     addType
    )
{
    //�Ƚ�������ӵ�������
    OperationQueueAddEx( &(piic->iic_list) , &(pOperation->oq) , addType );

    iic_start_one_operation_from_list(piic);
}


void iic_send_data(
    IIC_DriverCtrl* piic,           //���� IIC_PORT_1 ����
    IIC_Operation*  pOperation,     //������Ҫ���еĲ������ڸú����б�����
    u8              regAddr,        //�Ĵ�����ַ��Ҳ������ֵ�������豸û��ʹ�üĴ�����ַ����������
    u8              *toSend,        //��Ҫ���͵�����
    int             nToSend         //��Ҫ���͵����ݳ���
    )
{
    if ( pOperation->oq.queueState != OPER_QUEUE_STATE_NULL )
    {
        return;   //�Ѿ����������У����ܶ�����
    }
    pOperation->regAddr      = regAddr ;
    pOperation->pOperBuf     = toSend ;
    pOperation->nOperBuf     = nToSend ;
    pOperation->IicOperFlag  = IIC_OPER_FLAG_WRITE|IIC_OPER_FLAG_REG;

    iic_add_operation(piic,pOperation,LIST_ADD_ITEM_TO_TAIL);
    
}

void iic_send_data_ex(
    IIC_DriverCtrl* piic,           //���� IIC_PORT_1 ����
    IIC_Operation* pOperation,      //������Ҫ���еĲ������ڸú����б�����
    u8              devAddr,        //�豸��ַ
    u8              regAddr,        //�Ĵ�����ַ��Ҳ������ֵ�������豸û��ʹ�üĴ�����ַ����������
    u8              *toSend,        //��Ҫ���͵�����
    int             nToSend,        //��Ҫ���͵����ݳ���
    BX_UINT         IicOperFlag ,   //��������ر�־ ,һ�㴫�� 0 ����
    VoidParaFuncPtr pFinishCb,      //�������ʱ�Ļص�����;��������ж��б�����
    void*           pFinishCbPara ,  //��ɻص���������
    ListAddType     addType
    )
{
    if ( pOperation->oq.queueState != OPER_QUEUE_STATE_NULL )
    {
        return;   //�Ѿ����������У����ܶ�����
    }

    pOperation->oq.pFinishCb        = pFinishCb ;
    pOperation->oq.pFinishCbPara    = pFinishCbPara ;
    pOperation->IicOperFlag         = IicOperFlag ;
    pOperation->devAddr             = devAddr ;
    pOperation->regAddr             = regAddr ;
    pOperation->pOperBuf            = toSend ;
    pOperation->nOperBuf            = nToSend ;
    pOperation->IicOperFlag         = IicOperFlag; //IIC_OPER_FLAG_WRITE|IIC_OPER_FLAG_REG;

    iic_add_operation(piic,pOperation,addType);
}

void iic_read_data(
    IIC_DriverCtrl* piic,           //���� IIC_PORT_1 ����
    IIC_Operation* pOperation,      //������Ҫ���еĲ������ڸú����б�����
    u8              regAddr,        //�Ĵ�����ַ��Ҳ������ֵ�������豸û��ʹ�üĴ�����ַ����������
    u8              *toRead,        //��Ҫ���յ�����
    int             nToRead         //��Ҫ���յ����ݳ���
    )
{
    if ( pOperation->oq.queueState != OPER_QUEUE_STATE_NULL )
    {
        return;   //�Ѿ����������У����ܶ�����
    }
    pOperation->regAddr             = regAddr ;
    pOperation->pOperBuf            = toRead ;
    pOperation->nOperBuf            = nToRead ;
    pOperation->IicOperFlag         = IIC_OPER_FLAG_READ|IIC_OPER_FLAG_REG;

    iic_add_operation(piic,pOperation,LIST_ADD_ITEM_TO_TAIL);
}

void iic_read_data_ex(
    IIC_DriverCtrl* piic,           //���� IIC_PORT_1 ����
    IIC_Operation*  pOperation,     //������Ҫ���еĲ������ڸú����б�����
    u8              devAddr,        //�豸��ַ
    u8              regAddr,        //�Ĵ�����ַ��Ҳ������ֵ�������豸û��ʹ�üĴ�����ַ����������
    u8              *toRead,        //��Ҫ���յ�����
    int             nToRead,        //��Ҫ���յ����ݳ���
    BX_UINT         IicOperFlag ,   //��������ر�־ ,һ�㴫�� 0 ����
    VoidParaFuncPtr pFinishCb,      //�������ʱ�Ļص�����;��������ж��б�����
    void*           pFinishCbPara , //��ɻص���������
    ListAddType     addType
    )
{
    if ( pOperation->oq.queueState != OPER_QUEUE_STATE_NULL )
    {
        return;   //�Ѿ����������У����ܶ�����
    }

    pOperation->oq.pFinishCb        = pFinishCb ;
    pOperation->oq.pFinishCbPara    = pFinishCbPara ;
    pOperation->devAddr             = devAddr ;
    pOperation->regAddr             = regAddr ;
    pOperation->pOperBuf            = toRead ;
    pOperation->nOperBuf            = nToRead ;
    pOperation->IicOperFlag         = IicOperFlag;

    iic_add_operation(piic,pOperation,addType);
}


//////////////////////////////////////////////////////////////////////////
//Ӳ���������
//OPER_QUEUE_STATE_OK
static __inline void IIC_Oper_End_Deal_In_Intr(IIC_DriverCtrl* piic,int c_state)
{
    piic->pCurOper->oq.sOperState = c_state;
    
    //BoxOs_AddSoftIsr( (VoidParaFuncPtr)(iic_deal_finish_one_oper) , piic ) ;
    //BX_CALL(soft_timer_start)( &(piic->delay_timer) );
}

void IIC1_Hard_Init(void)  //IO�ڳ�ʼ�����Ĵ�����ʼ�����жϳ�ʼ��
{
    //�ȶ���ṹ��
    GPIO_InitTypeDef	 GPIO_InitStructure;
    I2C_InitTypeDef      I2C_InitStructure;

    //�ж�ʹ��
    NVIC_SetPriority(I2C1_EV_IRQn, 0x00); 
    NVIC_EnableIRQ(I2C1_EV_IRQn);

    NVIC_SetPriority(I2C1_ER_IRQn, 0x01); 
    NVIC_EnableIRQ(I2C1_ER_IRQn);

    //Ҫ���˶�Ӧ��gpio��ʱ�ӻ��������������ʱ�ӣ�Ȼ�������üĴ����ſ��ԣ�����������濪ʱ���Ⱥ�û��Ӱ�죬����ʵ�����棬Ҫ�ȿ�ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	//ʹ��APB2�����GPIOB��ʱ��,I2C2��PB_10 SCL��PB_11 SDA����ĸ���	 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 ,ENABLE);    //����ʱ��

    //Ҫ�����涨��������ṹ����ж��壬����PBҪ����Ϊ���õĲ��У������ǿ�©����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;    //ѡ��PB_6��SCL����
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //�ܽ�Ƶ��Ϊ50MHZ
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;	     //���ģʽΪ���ÿ�©���
    GPIO_Init(GPIOB,&GPIO_InitStructure);				 //��ʼ��GPIOB�Ĵ���

    //����I2C
    I2C_InitStructure.I2C_ClockSpeed    = 20000;          /*I2C��ʱ��Ƶ��,����С��400kHz ������I2C����ʱ�ӵ�Ƶ�ʷ�Ƶֵ*/
    I2C_InitStructure.I2C_Mode          = I2C_Mode_I2C;        
    I2C_InitStructure.I2C_DutyCycle     = I2C_DutyCycle_2;  /*����CCR�Ĵ����ģ�ռ�ձ�  ����ģʽ��0:Tlow/Thigh = 2��1:Tlow/Thigh = 16/9 */
    I2C_InitStructure.I2C_OwnAddress1   = 0xB0 ;         /*��仰˵����stm32��Ϊ�ӻ���ʱ�����ĵ�ַ�����û�����ӻ�����������ֵ*/
    I2C_InitStructure.I2C_Ack           = I2C_Ack_Enable;        /*Ӧ��ʹ�� */
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; /* */
    I2C_Init(I2C1,&I2C_InitStructure);

    //ʹ��I2C
    I2C_Cmd(I2C1, ENABLE);
}

void IIC_StartIntr(I2C_TypeDef*  I2Cx,IIC_Operation* pOper)  //����һ������
{
    I2Cx->CR1 &= CR1_PE_Reset;
    if( NULL == pOper )
    {
        I2Cx->CR2 &= ~I2C_IT_EVT;  //stop the iic operater
        return;
    }
    
    I2C_Cmd(I2Cx, ENABLE);
    
    if( pOper->IicOperFlag & IIC_OPER_FLAG_NO_ACK )
    {
        I2Cx->CR1 &= ~CR1_ACK_Set;
        I2Cx->CR1 &= ~CR1_ACK_Set;
    }
    else
    {
        I2Cx->CR1 |= CR1_ACK_Set; /* Enable Acknowledgement to be ready for another reception */
    }
    I2Cx->CR1 &= ~CR1_STOP_Set ;
    I2Cx->CR1 &= ~CR1_START_Set ;
    
    I2Cx->CR2 |= I2C_IT_EVT;  /* Enable EVT IT*/
    I2Cx->CR2 |= I2C_IT_ERR;  /* Enable EVT IT*/
    
    /* Send START condition */
    I2Cx->CR1 |= CR1_START_Set;
    //I2Cx->SR1 |= 0X0001 ;
    //I2Cx->SR2 |= 0X0003 ;
}

ErrorStatus MyI2C_CheckEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT,u32 sid)
{
    uint32_t lastevent = 0;
    uint32_t flag1 = 0, flag2 = 0;
    ErrorStatus status = ERROR;

    /* Read the I2Cx status register */
    flag1 = I2Cx->SR1;
    flag2 = I2Cx->SR2;
    flag2 = flag2 << 16;

    /* Get the last event value from I2C status register */
    lastevent = (flag1 | flag2) & FLAG_Mask;

    /* Check whether the last event contains the I2C_EVENT */
    if ((lastevent & I2C_EVENT) == I2C_EVENT)
    {
    /* SUCCESS: last event is equal to I2C_EVENT */
    status = SUCCESS;
    }
    else
    {
    /* ERROR: last event is different from I2C_EVENT */
    status = ERROR;
    }
    /* Return status */
    return status;
}

u32 IIC_DealEventInt(IIC_DriverCtrl* piic,I2C_TypeDef* I2Cx,IIC_Operation* pOper,u32 run_pos)
{
    BOXOS_TASKFUN_START(run_pos);
    
    piic->iCurOperDataCnt = 0;
    piic->gCurIICState    = 0;
    piic->redo_times      = 0;
    
    if( pOper->IicOperFlag & IIC_OPER_FLAG_READ )  //��IIC����
    {
        if( pOper->IicOperFlag & IIC_OPER_FLAG_REG )  //��Ҫ���мĴ�������
        {
            //EV5�¼�
            if( MyI2C_CheckEvent( I2Cx, I2C_EVENT_MASTER_MODE_SELECT, piic->gCurIICState ) )
            {
                I2C_Send7bitAddress( I2Cx, pOper->devAddr, I2C_Direction_Transmitter ); 
                piic->gCurIICState++;
            }
            else
            {
                goto IIC_ERR_DEAL;
            }
            BOXOS_TASKFUN_END_ONCE(run_pos);
            
            
            //EV6 EV8_1�¼������¼��ж�ͬʱ�ж���EV8_1�¼���
            if( MyI2C_CheckEvent( I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, piic->gCurIICState ) )
            {
                I2C_SendData( I2Cx,pOper->regAddr ); 
                piic->gCurIICState++;
            }
            else
            {
                goto IIC_ERR_DEAL;
            }
            BOXOS_TASKFUN_END_ONCE(run_pos);
            
            //����EV8_2���ж�������Ӧ��ʱ�Ѿ��յ�ACK��Word_Address������ɡ�
            if( MyI2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED,piic->gCurIICState) )
            {
                /*��Start�����������豸��ַ���������ҵ��豸Ӧ���ˣ������ݣ�������Ӧ����ֹ*/
                I2C_GenerateSTART(I2Cx, ENABLE);
                piic->gCurIICState++;
                I2Cx->CR2 &= ~I2C_IT_EVT;  //�����ж�
                I2Cx->CR2 &= ~I2C_IT_BUF;
                BX_CALL(soft_timer_start)( &(piic->delay_timer) );
            }
            else
            {
                goto IIC_ERR_DEAL;
            }
            BOXOS_TASKFUN_END_ONCE(run_pos); 
        }       
        
        //EV5�¼�
        piic->redo_times = 0;
        while( !MyI2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_MODE_SELECT,piic->gCurIICState) )
        {
            piic->redo_times++;
            if( piic->redo_times > 1500 )
            {
                goto IIC_ERR_DEAL;
            }
            BOXOS_TASKFUN_END_ONCE(run_pos);
        }               
        piic->gCurIICState++;
        I2C_Send7bitAddress(I2Cx, pOper->devAddr, I2C_Direction_Receiver ); 
        
        BOXOS_TASKFUN_END_ONCE(run_pos);
    
        //EV6�¼�
        piic->redo_times = 0;
        while( !MyI2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED,piic->gCurIICState))
        {
            piic->redo_times++;
            if( piic->redo_times > 500 )
            {
                goto IIC_ERR_DEAL;
            }
            BOXOS_TASKFUN_END_ONCE(run_pos);
        }
        piic->gCurIICState++;
        
        
        while( piic->iCurOperDataCnt < pOper->nOperBuf )
        {
            if( (piic->iCurOperDataCnt+1) == pOper->nOperBuf )
            {
                //EV6_1�¼���û�б�־λ��Ҫ����ACKʧ�ܺ�ֹͣλ����
                I2Cx->CR1 &= ~I2C_Ack_Enable ; //I2C_NACKPosition_Current ; //0xFBFF ;   //ʧ��ACK
                I2Cx->CR1 |= CR1_STOP_Set ;   //ʹ��Stop
            }
            else{
                I2Cx->CR1 |= I2C_Ack_Enable ;
            }
            I2Cx->CR2 |= I2C_IT_BUF;
            
            BOXOS_TASKFUN_END_ONCE(run_pos);
            
            //EV7�¼�����DR�Ĵ���
            piic->redo_times = 0;
            while( !MyI2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_RECEIVED,piic->gCurIICState) )
            {
                piic->redo_times++;
                if( piic->redo_times > 500 )
                {
                    goto IIC_ERR_DEAL;
                }
                BOXOS_TASKFUN_END_ONCE(run_pos);
            }
            
            pOper->pOperBuf[piic->iCurOperDataCnt] = I2Cx->DR;
            piic->iCurOperDataCnt++;
        }
        piic->gCurIICState++;
    }
    else
    {
        //EV5�¼�
        piic->redo_times = 0;
        while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_MODE_SELECT)) /*�ж�EV5,���º���������Է��֣����¼���SB=1��MSL=1��BUSY=1 
                                                                ��˼����ʼ�����Ѿ������ˣ�Ȼ������ģʽ��������ͨѶ*/
        {
            piic->redo_times++;
            if( piic->redo_times > 5 )
            {
                goto IIC_ERR_DEAL;
            }
            BOXOS_TASKFUN_END_ONCE(run_pos);
        }
        piic->gCurIICState++;
        I2C_Send7bitAddress(I2Cx, pOper->devAddr, I2C_Direction_Transmitter);  /*����������ַ�����һ��������ʾ��ַbit0Ϊ0����˼
                                                                            ��д������ͬʱ����д��DR�Ĵ������ʻ����SBλ���0*/
        BOXOS_TASKFUN_END_ONCE(run_pos);
        
        //EV6 EV8_1�����¼��ж�ͬʱ�ж���EV8_1�¼���
        piic->redo_times = 0;
        while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))/*�ж�EV6��EV8_1����������ʱSB=0��ADDR=1��ַ���ͽ�����
                                                                         TXE=1���ݼĴ���DRΪ�գ�BUSY=1�ܲ�����,MSL=1��ģʽ,TRA=1
                                                                            �����Ѿ������ˣ���Ϊ��д��������ʵ�ǵ�ַ�����Ѿ������ˣ�
                                                                            ���������ģʽ�������EV6��TRA=1��ʾ���ݻ�û�յ���0��ʾ
                                                                            �յ����ݣ�ע������TRA=1��ʾ�Ѿ������ˣ�ADDR=1���Ƿ������
                                                                            ��,������¼���ADDR=0����*/
        {
            piic->redo_times++;
            if( piic->redo_times > 5 )
            {
                goto IIC_ERR_DEAL;
            }
            BOXOS_TASKFUN_END_ONCE(run_pos);
        }
        piic->gCurIICState++;
        
        
        if( pOper->IicOperFlag & IIC_OPER_FLAG_REG )  //��Ҫ���мĴ�������
        {
            I2C_SendData(I2Cx,pOper->regAddr);      /*EV8_1�¼�����Ϊ��һ��ʱ��DR�Ѿ�Ϊ�գ����¼���д��data1������EEPROM��˵���data1��Ҫд��
                                          �ֽڵĵĵ�ַ��data2��Ҫд������ݣ�data1Ϊ8λ1k����Ч�ֽ��ǵ�7λ��2kbit����Ч�ֽ���8λ��32ҳ��
                                          ÿҳ8���ֽڣ�һ��2kλ��16kbit��Ҫ11λ����ô�ͣ���Ӳ����A1��2��3��GPIO��ѡ��洢��ҳ*/
            BOXOS_TASKFUN_END_ONCE(run_pos);
        }
        else  //�����ͼĴ��� ��ֱ�ӷ���һ�������ֽ�
        {
            /*���ݼĴ���DRΪ�գ�������д��data2���ò���������EV8�¼�*/
            if( pOper->nOperBuf > 0 )
            {
                I2C_SendData(I2Cx, pOper->pOperBuf[piic->iCurOperDataCnt] );
                piic->iCurOperDataCnt++;
            }
        }
        
        while( piic->iCurOperDataCnt < pOper->nOperBuf )
        {
            //EV8�¼�
            piic->redo_times = 0;
            while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING)) /*Ϊ��һ��ִ��EV8�¼����ж�TXE=1 DR�Ĵ���Ϊ�գ�׼��д��DR�Ĵ���
                                                                         data2��ע���ʱҲ�ж�TRA=1��˼��data1�Ѿ������˲���ʾ������ɣ�
                                                                         ��λ�Ĵ����ǿձ�ʾ�����ڷ�������BTF=0˵��data1�ֽڷ���û��ɣ�
                                                                         NOSTRETCHΪ0ʱ��BTF=1����ʾ������Ҫ�������ˣ���ζ���ֽڷ��ͣ�
                                                                         ����˵��������ݻ�û��д�뵽DR���棬�����EV8�¼����ڵ�һ��һֱ
                                                                         �������ݷ��͵ģ�������BTF=1�����*/ 
            {
                piic->redo_times++;
                if( piic->redo_times > 5 )
                {
                    goto IIC_ERR_DEAL;
                }
                BOXOS_TASKFUN_END_ONCE(run_pos);
            }
            I2C_SendData(I2Cx, pOper->pOperBuf[piic->iCurOperDataCnt] );                                    /*���ݼĴ���DRΪ�գ�������д��data2���ò���������EV8�¼�*/
            piic->iCurOperDataCnt++;
            
            BOXOS_TASKFUN_END_ONCE(run_pos); 
        } 
        
        piic->gCurIICState++;

        /*EV8_2�¼�������ͷ�������data�����Ծ�ֻ��һ��EV8�¼���
        EV8_1��data1��EV8_1��data2��EV8_2��EV8�������Ǽ���
        ���BTF,��EV8_2��дDR���ǳ���Ҫ��ֹͣ��*/
        piic->redo_times = 0;
        if( (pOper->IicOperFlag & IIC_OPER_FLAG_REG) || (pOper->nOperBuf>0) )
        {
            while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED)) /*EV8_2���¼��жϺ�EV8�¼���һ����TRA=1��ʾdata2�Ѿ������ˣ�
                                                                          BTF=1�ֽڷ��ͽ�����˵����һʱ��û���ֽ��ڷ��ͣ���ʵ��ʾ��
                                                                          data2���ڷ��͵�ʱ��û��data3д�뵽DR���棬 Ȼ�����ڸ��ֵ�Ҫ
                                                                          ����data3�ˣ�����DR�����ǿյģ���ʵ����data2��ʱ���м�ĳʱ
                                                                          ��DR���Ѿ����ˣ�*/
            {
                piic->redo_times++;
                if( piic->redo_times > 5 )
                {
                    goto IIC_ERR_DEAL;
                }
                BOXOS_TASKFUN_END_ONCE(run_pos);
            }
        }
        piic->gCurIICState++;
    }
    
    if( 0 == (pOper->IicOperFlag&IIC_OPER_FLAG_NO_STOP) )
    {
        I2C_GenerateSTOP(I2Cx,ENABLE);                                  /*EV8_2�¼��еĳ���дֹͣ*/
    }
    piic->gCurIICState = 100;
    
IIC_ERR_DEAL:
    I2Cx->CR2 &= ~I2C_IT_EVT;
    I2Cx->CR2 &= ~I2C_IT_BUF;
    I2Cx->CR2 &= ~I2C_IT_ERR;
    
    if( piic->gCurIICState <= 90 ){ 
        piic->gCurIICState = 88 ;  //iic err
        if( 0 == (pOper->IicOperFlag&IIC_OPER_FLAG_NO_STOP) )
        {
            I2C_GenerateSTOP(I2Cx,ENABLE); 
        }
        IIC_Oper_End_Deal_In_Intr(piic ,OPER_QUEUE_STATE_ERR2);
    }
    else{
        IIC_Oper_End_Deal_In_Intr( piic ,OPER_QUEUE_STATE_OK);  //һ��ͨ�ųɹ��Ļص������������
    }
    BOXOS_TASKFUN_END_FUN(run_pos);
}


/*******************************************************************************
* Function Name  : I2C1_EV_IRQHandler
* Description    : This function handles I2C1 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_EV_IRQHandler(void)
{
    BoxOs_IntoInt() ;

    DriverIIC1.delay_timer.TmrDly = 0; //ǿ������Ѿ��ȴ�ʱ��
    //to do  iicͨ�Ź���
    DriverIIC1.int_oper_fun_execute_pos = IIC_DealEventInt(IIC_PORT_1,DriverIIC1.pRegs,DriverIIC1.pCurOper,DriverIIC1.int_oper_fun_execute_pos);

    BoxOs_ExitInt() ;
}

/*******************************************************************************
* Function Name  : I2C1_ER_IRQHandler
* Description    : This function handles I2C1 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_ER_IRQHandler(void)
{
    BoxOs_IntoInt() ;

    //to do iic �����ж� �������
    DriverIIC1.pRegs->CR2 &= ~I2C_IT_EVT;
    DriverIIC1.pRegs->CR2 &= ~I2C_IT_BUF;
    DriverIIC1.pRegs->CR2 &= ~I2C_IT_ERR;
    DriverIIC1.gCurIICState = 88 ;  //iic err
    DriverIIC1.int_oper_fun_execute_pos = 0;
    if( 0 == (DriverIIC1.pCurOper->IicOperFlag&IIC_OPER_FLAG_NO_STOP) )
    {
        I2C_GenerateSTOP(DriverIIC1.pRegs,ENABLE); 
    }
    
    IIC_Oper_End_Deal_In_Intr(IIC_PORT_1,OPER_QUEUE_STATE_ERR1);

    BoxOs_ExitInt() ;
}

//����ǿ�Ʒ���һ��ֹͣ�ź�
void IIC_SendStop(IIC_DriverCtrl* piic)
{
    I2C_Cmd(piic->pRegs , ENABLE);
    I2C_GenerateSTOP(piic->pRegs,ENABLE); 
    I2C_Cmd(piic->pRegs , DISABLE);
    //if( DriverIIC1.pCurOper )
    //{
    //}
}
