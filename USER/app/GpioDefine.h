#ifndef _GPIO_DEFINE_H__
#define _GPIO_DEFINE_H__

#define GPIO_MOTOR_PWR_EN        GPIOC,GPIO_Pin_2       //电机电源使能  
#define GPIO_BAT_CHARGE_EN       GPIOA,GPIO_Pin_0       //主电池充电使能
#define GPIO_SLV_CHG_EN          GPIOA,GPIO_Pin_11      //备用电池充电使能
#define GPIO_BREAK_MEN           GPIOC,GPIO_Pin_1       //抱闸开关；低电平使能

//#define GPIO_4G_EN        	     GPIOA,GPIO_Pin_8     //4G开关；低电平使能
#define GPIO_GPS_EN        		   GPIOD,GPIO_Pin_1       //GPS开关；低电平使能
#define GPIO_12V_EN        		   GPIOC,GPIO_Pin_3       //12V开关；低电平使能

#define GPIO_CAM_AV_EN           GPIOD,GPIO_Pin_0      //图传开关；低电平使能
#define GPIO_CAM_EN        		   GPIOD,GPIO_Pin_3      //摄像头开关；低电平使能
#define GPIO_CTR_CAM_AV_SW 	 	   GPIOA,GPIO_Pin_12     //摄像头图传信号切换

#define GPIO_CHK_PRS_VOL1        GPIOA,GPIO_Pin_2     //压力值ADC2
#define GPIO_CHK_PRS_VOL2        GPIOA,GPIO_Pin_3     //压力值ADC3
#define GPIO_CHK_MT_SPD_M        GPIOA,GPIO_Pin_6     //主动轮速度ADC6
#define GPIO_CHK_MT_SPD_S     	 GPIOA,GPIO_Pin_7     //从动轮速度ADC7
#define GPIO_CHK_BAT_CUR         GPIOC,GPIO_Pin_4     //电池电流 ADC14
#define GPIO_CHK_BAT_VOL	       GPIOC,GPIO_Pin_5     //电池电压 ADC15

#define GPIO_CHK_GPS_DATA	       GPIOC,GPIO_Pin_11     //GPS数据接收
#define GPIO_CHK_GPS_TX	         GPIOC,GPIO_Pin_10     //GPS数据发送

//#define GPIO_CHK_HOARE1	 	 	     GPIOE,GPIO_Pin_7	  //霍尔检测1
//#define GPIO_CHK_HOARE2 	 	     GPIOE,GPIO_Pin_8	  //霍尔检测2

#define GPIO_CHK_CHAG_SHORT	 	 GPIOE,GPIO_Pin_4     //充电短路检测
#define GPIO_CHK_CHAG_JOIN	 	 GPIOE,GPIO_Pin_5	    //充电接入检测
#define GPIO_CHK_CHAOXUE1	 	   GPIOB,GPIO_Pin_8	    //进入巢穴开关检测1
#define GPIO_CHK_CHAOXUE2	 	   GPIOE,GPIO_Pin_9 	  //进入巢穴开关检测2  预留
#define GPIO_CHK_LIMIT1   		 GPIOD,GPIO_Pin_10    //从动轮运动上下限位开关检测1
#define GPIO_CHK_LIMIT2   		 GPIOD,GPIO_Pin_11    //从动轮运动上下限位开关检测2

#define GPIO_CTL_DIR_S1 	 	   GPIOE,GPIO_Pin_2	    //从动轮方向1
#define GPIO_CTL_DIR_S2 	 	   GPIOE,GPIO_Pin_1	    //从动轮方向2
#define GPIO_CTL_DIR_M1 	 	   GPIOE,GPIO_Pin_0	    //主动轮方向1
#define GPIO_CTL_DIR_M2 	 	   GPIOE,GPIO_Pin_3	    //主动轮方向2
#define GPIO_CTL_FED_WTDG 	 	 GPIOC,GPIO_Pin_0	    //看门狗喂狗
//#define GPIO_CTL_ARM_RST 	 	   GPIOC,GPIO_Pin_1	  //ARM复位控制脚
#define GPIO_CTR_RF433_M0 	 	 GPIOE,GPIO_Pin_6	    //无线串口模式选择0
#define GPIO_CTR_RF433_M1 	 	 GPIOC,GPIO_Pin_13	  //无线串口模式选择1
#define GPIO_CTR_RF433_AUX 	 	 GPIOA,GPIO_Pin_5     //无线串口检测
//#define GPIO_CTR_FLASH_CS 	 	 GPIOD,GPIO_Pin_8	  //FLASH片选

#define GPIO_LED_SHOW2          GPIOB,GPIO_Pin_14      //电路板上指示灯2
#define GPIO_LED_SHOW1          GPIOB,GPIO_Pin_15      //电路板上指示灯1
#define GPIO_LED_OUT_SHOW1      GPIOB,GPIO_Pin_9       //外接指示灯 1
#define GPIO_LED_OUT_SHOW2      GPIOD,GPIO_Pin_4       //外接指示灯 2

#define GPIO_MTS_CODEA1 	 	   GPIOA,GPIO_Pin_15    //从电机编码器 TIM2-CH1
#define GPIO_MTS_CODEB1 	 	   GPIOB,GPIO_Pin_3     //从电机编码器 TIM2-CH2
#define GPIO_MTS_CODEA2 	 	   GPIOC,GPIO_Pin_6     //从电机编码器 TIM8-CH1
#define GPIO_MTS_CODEB2 	   	 GPIOC,GPIO_Pin_7     //从电机编码器 
#define GPIO_MTM_CODEA 	 	  	 GPIOB,GPIO_Pin_4     //主电机编码器 TIM3-CH1
#define GPIO_MTM_CODEB 	 	  	 GPIOB,GPIO_Pin_5     //主电机编码器 TIM3-CH2
#define GPIO_MTM_PWM1 	 	  	 GPIOC,GPIO_Pin_8     //主电机PWM TIM8-CH3
#define GPIO_MTM_PWM2 	 	  	 GPIOC,GPIO_Pin_9     //主电机PWM TIM8-CH4
#define GPIO_CODEA_CD1 	 	  	 GPIOD,GPIO_Pin_12    //从轮子编码器 TIM4-CH1
#define GPIO_CODEA_CD2	 	  	 GPIOD,GPIO_Pin_13    //从轮子编码器 TIM4-CH2
#define GPIO_CODEA_CD3	 	  	 GPIOD,GPIO_Pin_14    //从轮子编码器 TIM4-CH3

#define GPIO_IIC_SCL 	 	  	   GPIOB,GPIO_Pin_6      //IIC 时钟
#define GPIO_IIC_SDA 	 	  	   GPIOB,GPIO_Pin_7      //IIC 数据
//串口扩展
#define GPIO_EXT_IRQ 	 	  	   GPIOB,GPIO_Pin_12     //扩展串口 外部中断
#define GPIO_EXT_RST 	 	  	   GPIOB,GPIO_Pin_13     //扩展串口 复位
#define GPIO_SCL2 	 	  	     GPIOB,GPIO_Pin_10     //扩展串口 时钟
#define GPIO_SDA2 	 	  	     GPIOB,GPIO_Pin_11     //扩展串口 数据


//#define GPIO_SPI_SCK 	 	  	   GPIOB,GPIO_Pin_13    //SPI2 时钟
//#define GPIO_SPI_MISO 	 	  	 GPIOB,GPIO_Pin_14    //SPI2 MISO
//#define GPIO_SPI_MOSI 	 	  	 GPIOB,GPIO_Pin_15    //SPI2 MOSI

#define GPIO_STM_SWIO 	 	     GPIOA,GPIO_Pin_13	   //SW IO
#define GPIO_STM_SWCK	 	       GPIOA,GPIO_Pin_14     //SW 时钟

#define GPIO_OUT_SPARE1 	 	   GPIOA,GPIO_Pin_4	    //备用输出1
#define GPIO_OUT_SPARE2 	 	   GPIOA,GPIO_Pin_1	    //备用输出2



#endif
