#include <includes.h>
#include "iic_driver.h"

#ifndef BX_CALL
#define BX_CALL(FUN_NAME)   FUN_NAME
#endif

IIC_DriverCtrl DriverIIC1 = { I2C1 , I2C1_EV_IRQn  };

void iic_start_one_operation_from_list(IIC_DriverCtrl* piic);

/************************************************* 
*Function:		IicDriverDelayTimerCb
*Input:			SoftTimer*  tmr 定时器对象
*               
*OUTPUT:		void
*Return:		void
*DESCRIPTION:  定时器回调函数，用于在IIC一次通信完毕后
*           的延时处理过程，通过定时器进行相关延时操作。
*       1.IIC总线发送完 STOP 位后，等一定时间需要关闭IIC一次，清除异常标志位
*       2.IIC总线一次处理完成后，需要等待一定时间再启动下一次IIC的通信操作
*************************************************/
void IicDriverDelayTimerCb(SoftTimer* timer)
{
    IIC_DriverCtrl* piic = (IIC_DriverCtrl*)(timer->CallbackArg) ;
    
    if( piic->gCurIICState < 10 ) //在读取数据时，会因为发送寄存器后再次发送起始信息，需要等待一定时间发送
    {
        if( 0 == DriverIIC1.int_oper_fun_execute_pos ) //从未接收过中断，需要重新配置IIC
        {
            if( I2C1 == piic->pRegs )
            {
                //to do    iic 硬件初始化工作  piic->pRegs 
                piic->pRegs->CR1 |= 0X8000; //SWRST 置位
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
            piic->pCurOper->oq.queueState = OPER_QUEUE_STATE_OPER_END ;  //处理状态为处理结束
            OS_EXIT_CRITICAL();
            if( 0 == (piic->pCurOper->IicOperFlag&IIC_OPER_FLAG_NO_STOP) )
            {
            }
            else{
                I2C_Cmd(piic->pRegs , DISABLE);
            }
            if ( piic->pCurOper->oq.pFinishCb )
            {
                ( *(piic->pCurOper->oq.pFinishCb) )( piic->pCurOper->oq.pFinishCbPara ) ;  //调用 回调函数
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

        iic_start_one_operation_from_list(piic);  //启动队列中一个新的操作
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
        
    //to do    iic 硬件初始化工作  piic->pRegs 
    IIC1_Hard_Init();
}

//每个IIC_Operation必须调用一次 iic_init_operation 函数；而且只能调用一次
void iic_init_operation(
    IIC_Operation*  pOperation,     //传入需要进行的操作，必须先进行配置
    u8              devAddr,        //设备地址
    VoidParaFuncPtr pFinishCb,      //操作完成时的回调函数;会在软件中断中被调用
    void*           pFinishCbPara   //完成回调函数参数
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
    
    if ( NULL == piic->pCurOper ) //只有当前没有任何操作时才能进行下面处理
    {
        piic->pCurOper = (IIC_Operation*)( OperationQueuePick(&(piic->iic_list)) );
        if ( piic->pCurOper )  //表示当前有任务需要进行处理
        {
            BX_CALL(soft_timer_start)( &(piic->delay_timer) );  //启动检测是否异常的定时器
            piic->int_oper_fun_execute_pos = 0;
            piic->iCurOperDataCnt = 0;
            piic->gCurIICState    = 0;
            piic->redo_times      = 0;
            //to do  启动IIC硬件中断
            IIC_StartIntr( piic->pRegs , piic->pCurOper );
        }
        else{
            BX_CALL(soft_timer_stop)( &(piic->delay_timer) );
        }
    }

    OS_EXIT_CRITICAL();
}

void iic_add_operation(
    IIC_DriverCtrl* piic,           //传入 IIC_PORT_1 即可
    IIC_Operation* pOperation ,     //传入需要进行的操作，必须先进行配置
    ListAddType     addType
    )
{
    //先将任务添加到队列中
    OperationQueueAddEx( &(piic->iic_list) , &(pOperation->oq) , addType );

    iic_start_one_operation_from_list(piic);
}


void iic_send_data(
    IIC_DriverCtrl* piic,           //传入 IIC_PORT_1 即可
    IIC_Operation*  pOperation,     //传入需要进行的操作，在该函数中被配置
    u8              regAddr,        //寄存器地址，也是命令值（部分设备没有使用寄存器地址概念，而是命令）
    u8              *toSend,        //需要发送的数据
    int             nToSend         //需要发送的数据长度
    )
{
    if ( pOperation->oq.queueState != OPER_QUEUE_STATE_NULL )
    {
        return;   //已经处于链表中，不能多次添加
    }
    pOperation->regAddr      = regAddr ;
    pOperation->pOperBuf     = toSend ;
    pOperation->nOperBuf     = nToSend ;
    pOperation->IicOperFlag  = IIC_OPER_FLAG_WRITE|IIC_OPER_FLAG_REG;

    iic_add_operation(piic,pOperation,LIST_ADD_ITEM_TO_TAIL);
    
}

void iic_send_data_ex(
    IIC_DriverCtrl* piic,           //传入 IIC_PORT_1 即可
    IIC_Operation* pOperation,      //传入需要进行的操作，在该函数中被配置
    u8              devAddr,        //设备地址
    u8              regAddr,        //寄存器地址，也是命令值（部分设备没有使用寄存器地址概念，而是命令）
    u8              *toSend,        //需要发送的数据
    int             nToSend,        //需要发送的数据长度
    BX_UINT         IicOperFlag ,   //操作的相关标志 ,一般传入 0 即可
    VoidParaFuncPtr pFinishCb,      //操作完成时的回调函数;会在软件中断中被调用
    void*           pFinishCbPara ,  //完成回调函数参数
    ListAddType     addType
    )
{
    if ( pOperation->oq.queueState != OPER_QUEUE_STATE_NULL )
    {
        return;   //已经处于链表中，不能多次添加
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
    IIC_DriverCtrl* piic,           //传入 IIC_PORT_1 即可
    IIC_Operation* pOperation,      //传入需要进行的操作，在该函数中被配置
    u8              regAddr,        //寄存器地址，也是命令值（部分设备没有使用寄存器地址概念，而是命令）
    u8              *toRead,        //需要接收的数据
    int             nToRead         //需要接收的数据长度
    )
{
    if ( pOperation->oq.queueState != OPER_QUEUE_STATE_NULL )
    {
        return;   //已经处于链表中，不能多次添加
    }
    pOperation->regAddr             = regAddr ;
    pOperation->pOperBuf            = toRead ;
    pOperation->nOperBuf            = nToRead ;
    pOperation->IicOperFlag         = IIC_OPER_FLAG_READ|IIC_OPER_FLAG_REG;

    iic_add_operation(piic,pOperation,LIST_ADD_ITEM_TO_TAIL);
}

void iic_read_data_ex(
    IIC_DriverCtrl* piic,           //传入 IIC_PORT_1 即可
    IIC_Operation*  pOperation,     //传入需要进行的操作，在该函数中被配置
    u8              devAddr,        //设备地址
    u8              regAddr,        //寄存器地址，也是命令值（部分设备没有使用寄存器地址概念，而是命令）
    u8              *toRead,        //需要接收的数据
    int             nToRead,        //需要接收的数据长度
    BX_UINT         IicOperFlag ,   //操作的相关标志 ,一般传入 0 即可
    VoidParaFuncPtr pFinishCb,      //操作完成时的回调函数;会在软件中断中被调用
    void*           pFinishCbPara , //完成回调函数参数
    ListAddType     addType
    )
{
    if ( pOperation->oq.queueState != OPER_QUEUE_STATE_NULL )
    {
        return;   //已经处于链表中，不能多次添加
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
//硬件处理过程
//OPER_QUEUE_STATE_OK
static __inline void IIC_Oper_End_Deal_In_Intr(IIC_DriverCtrl* piic,int c_state)
{
    piic->pCurOper->oq.sOperState = c_state;
    
    //BoxOs_AddSoftIsr( (VoidParaFuncPtr)(iic_deal_finish_one_oper) , piic ) ;
    //BX_CALL(soft_timer_start)( &(piic->delay_timer) );
}

void IIC1_Hard_Init(void)  //IO口初始化，寄存器初始化，中断初始化
{
    //先定义结构体
    GPIO_InitTypeDef	 GPIO_InitStructure;
    I2C_InitTypeDef      I2C_InitStructure;

    //中断使能
    NVIC_SetPriority(I2C1_EV_IRQn, 0x00); 
    NVIC_EnableIRQ(I2C1_EV_IRQn);

    NVIC_SetPriority(I2C1_ER_IRQn, 0x01); 
    NVIC_EnableIRQ(I2C1_ER_IRQn);

    //要开了对应的gpio的时钟还有其他的外设的时钟，然后你配置寄存器才可以，软件仿真里面开时钟先后没有影响，但是实物里面，要先开时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	//使能APB2外设的GPIOB的时钟,I2C2是PB_10 SCL，PB_11 SDA上面的复用	 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 ,ENABLE);    //开启时钟

    //要对上面定义的两个结构体进行定义，首先PB要定义为复用的才行，而且是开漏复用
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;    //选择PB_6是SCL引脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //管脚频率为50MHZ
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;	     //输出模式为复用开漏输出
    GPIO_Init(GPIOB,&GPIO_InitStructure);				 //初始化GPIOB寄存器

    //配置I2C
    I2C_InitStructure.I2C_ClockSpeed    = 20000;          /*I2C的时钟频率,必须小于400kHz ，不是I2C输入时钟的频率分频值*/
    I2C_InitStructure.I2C_Mode          = I2C_Mode_I2C;        
    I2C_InitStructure.I2C_DutyCycle     = I2C_DutyCycle_2;  /*设置CCR寄存器的，占空比  快速模式下0:Tlow/Thigh = 2；1:Tlow/Thigh = 16/9 */
    I2C_InitStructure.I2C_OwnAddress1   = 0xB0 ;         /*这句话说的是stm32作为从机的时候它的地址，如果没有做从机不用理会这个值*/
    I2C_InitStructure.I2C_Ack           = I2C_Ack_Enable;        /*应答使能 */
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; /* */
    I2C_Init(I2C1,&I2C_InitStructure);

    //使能I2C
    I2C_Cmd(I2C1, ENABLE);
}

void IIC_StartIntr(I2C_TypeDef*  I2Cx,IIC_Operation* pOper)  //启动一个操作
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
    
    if( pOper->IicOperFlag & IIC_OPER_FLAG_READ )  //读IIC数据
    {
        if( pOper->IicOperFlag & IIC_OPER_FLAG_REG )  //需要进行寄存器操作
        {
            //EV5事件
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
            
            
            //EV6 EV8_1事件（该事件判断同时判断了EV8_1事件）
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
            
            //采用EV8_2的判断条件反应此时已经收到ACK，Word_Address发送完成。
            if( MyI2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED,piic->gCurIICState) )
            {
                /*给Start条件，发送设备地址读操作，找到设备应答了，收数据，主机不应答，终止*/
                I2C_GenerateSTART(I2Cx, ENABLE);
                piic->gCurIICState++;
                I2Cx->CR2 &= ~I2C_IT_EVT;  //禁用中断
                I2Cx->CR2 &= ~I2C_IT_BUF;
                BX_CALL(soft_timer_start)( &(piic->delay_timer) );
            }
            else
            {
                goto IIC_ERR_DEAL;
            }
            BOXOS_TASKFUN_END_ONCE(run_pos); 
        }       
        
        //EV5事件
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
    
        //EV6事件
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
                //EV6_1事件，没有标志位，要设置ACK失能和停止位产生
                I2Cx->CR1 &= ~I2C_Ack_Enable ; //I2C_NACKPosition_Current ; //0xFBFF ;   //失能ACK
                I2Cx->CR1 |= CR1_STOP_Set ;   //使能Stop
            }
            else{
                I2Cx->CR1 |= I2C_Ack_Enable ;
            }
            I2Cx->CR2 |= I2C_IT_BUF;
            
            BOXOS_TASKFUN_END_ONCE(run_pos);
            
            //EV7事件，读DR寄存器
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
        //EV5事件
        piic->redo_times = 0;
        while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_MODE_SELECT)) /*判断EV5,看下函数定义可以发现，该事件是SB=1，MSL=1，BUSY=1 
                                                                意思是起始条件已经发送了，然后是主模式，总线在通讯*/
        {
            piic->redo_times++;
            if( piic->redo_times > 5 )
            {
                goto IIC_ERR_DEAL;
            }
            BOXOS_TASKFUN_END_ONCE(run_pos);
        }
        piic->gCurIICState++;
        I2C_Send7bitAddress(I2Cx, pOper->devAddr, I2C_Direction_Transmitter);  /*发送器件地址，最后一个参数表示地址bit0为0，意思
                                                                            是写操作，同时由于写了DR寄存器，故会清除SB位变成0*/
        BOXOS_TASKFUN_END_ONCE(run_pos);
        
        //EV6 EV8_1（该事件判断同时判断了EV8_1事件）
        piic->redo_times = 0;
        while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))/*判断EV6和EV8_1的条件，此时SB=0，ADDR=1地址发送结束，
                                                                         TXE=1数据寄存器DR为空，BUSY=1总不空闲,MSL=1主模式,TRA=1
                                                                            数据已经发送了（因为是写操作，其实是地址数据已经发送了）
                                                                            如果是主收模式，这里的EV6，TRA=1表示数据还没收到，0表示
                                                                            收到数据，注意这里TRA=1表示已经发送了，ADDR=1才是发送完成
                                                                            了,做完该事件，ADDR=0了又*/
        {
            piic->redo_times++;
            if( piic->redo_times > 5 )
            {
                goto IIC_ERR_DEAL;
            }
            BOXOS_TASKFUN_END_ONCE(run_pos);
        }
        piic->gCurIICState++;
        
        
        if( pOper->IicOperFlag & IIC_OPER_FLAG_REG )  //需要进行寄存器操作
        {
            I2C_SendData(I2Cx,pOper->regAddr);      /*EV8_1事件，因为这一步时候DR已经为空，该事件是写入data1，对于EEPROM来说这个data1是要写入
                                          字节的的地址，data2是要写入的内容，data1为8位1k的有效字节是低7位，2kbit的有效字节是8位，32页，
                                          每页8个字节，一共2k位，16kbit需要11位？怎么送？用硬件的A1，2，3接GPIO来选择存储的页*/
            BOXOS_TASKFUN_END_ONCE(run_pos);
        }
        else  //不发送寄存器 就直接发送一个数据字节
        {
            /*数据寄存器DR为空，这里是写入data2，该步骤隶属于EV8事件*/
            if( pOper->nOperBuf > 0 )
            {
                I2C_SendData(I2Cx, pOper->pOperBuf[piic->iCurOperDataCnt] );
                piic->iCurOperDataCnt++;
            }
        }
        
        while( piic->iCurOperDataCnt < pOper->nOperBuf )
        {
            //EV8事件
            piic->redo_times = 0;
            while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING)) /*为下一步执行EV8事件，判断TXE=1 DR寄存器为空，准备写入DR寄存器
                                                                         data2，注意此时也判定TRA=1意思是data1已经发送了不表示发送完成，
                                                                         移位寄存器非空表示数还在发，另外BTF=0说明data1字节发送没完成，
                                                                         NOSTRETCH为0时候，BTF=1还表示新数据要被发送了（意味着字节发送）
                                                                         完成了但是新数据还没有写入到DR里面，这里的EV8事件对于的一段一直
                                                                         是有数据发送的，不存在BTF=1的情况*/ 
            {
                piic->redo_times++;
                if( piic->redo_times > 5 )
                {
                    goto IIC_ERR_DEAL;
                }
                BOXOS_TASKFUN_END_ONCE(run_pos);
            }
            I2C_SendData(I2Cx, pOper->pOperBuf[piic->iCurOperDataCnt] );                                    /*数据寄存器DR为空，这里是写入data2，该步骤隶属于EV8事件*/
            piic->iCurOperDataCnt++;
            
            BOXOS_TASKFUN_END_ONCE(run_pos); 
        } 
        
        piic->gCurIICState++;

        /*EV8_2事件（这里就发送两个data，所以就只有一个EV8事件，
        EV8_1—data1，EV8_1—data2，EV8_2和EV8的区别是检测的
        差个BTF,且EV8_2不写DR而是程序要求停止）*/
        piic->redo_times = 0;
        if( (pOper->IicOperFlag & IIC_OPER_FLAG_REG) || (pOper->nOperBuf>0) )
        {
            while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED)) /*EV8_2的事件判断和EV8事件不一样，TRA=1表示data2已经发送了，
                                                                          BTF=1字节发送结束，说明这一时刻没有字节在发送，其实表示在
                                                                          data2正在发送的时候，没有data3写入到DR里面， 然后现在该轮到要
                                                                          发送data3了，但是DR里面是空的（其实发送data2的时候，中间某时
                                                                          刻DR就已经空了）*/
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
        I2C_GenerateSTOP(I2Cx,ENABLE);                                  /*EV8_2事件中的程序写停止*/
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
        IIC_Oper_End_Deal_In_Intr( piic ,OPER_QUEUE_STATE_OK);  //一次通信成功的回调函数处理过程
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

    DriverIIC1.delay_timer.TmrDly = 0; //强制清空已经等待时间
    //to do  iic通信过程
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

    //to do iic 错误中断 处理过程
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

//总线强制发送一个停止信号
void IIC_SendStop(IIC_DriverCtrl* piic)
{
    I2C_Cmd(piic->pRegs , ENABLE);
    I2C_GenerateSTOP(piic->pRegs,ENABLE); 
    I2C_Cmd(piic->pRegs , DISABLE);
    //if( DriverIIC1.pCurOper )
    //{
    //}
}
