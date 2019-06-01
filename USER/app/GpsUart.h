#ifndef __MYGPS_H__
#define __MYGPS_H__

#include <includes.h>
//#include "BoxLinux.h"
//#include "cmd_para.h"
//#include "uartDriver.h"

//#define PF(a)       ((a)*(a))
//#define POLE_NUM    50
//#define GPS_COM     "/dev/ttyS3"
//#define GPS_COM_BDR  9600


//    $GNRMC,015835.00,A,3945.11111,N,11608.87560,E,0.199,,181117,,,A*62
//    $GNVTG,,T,,M,0.199,N,0.368,K,A*31
//    $GNGGA,015835.00,3945.11111,N,11608.87560,E,1,08,1.03,65.2,M,-9.2,M,,*6D
//    $GNGSA,A,3,12,02,05,06,09,19,25,,,,,,1.82,1.03,1.50*15
//    $GNGSA,A,3,86,,,,,,,,,,,,1.82,1.03,1.50*1F
//    $GPGSV,4,1,15,02,74,000,27,05,57,251,33,06,49,083,21,07,00,093,*73
//    $GPGSV,4,2,15,09,29,052,24,12,22,246,23,13,09,183,,17,05,149,27*71
//    $GPGSV,4,3,15,19,25,151,13,20,08,245,,23,04,035,13,25,19,285,13*78
//    $GPGSV,4,4,15,29,21,318,18,42,35,139,,50,35,139,*45
//    $GLGSV,2,1,07,65,44,216,,71,28,034,,72,82,030,,79,02,328,*65
//    $GLGSV,2,2,07,86,36,128,28,87,75,009,,88,29,325,*53
//    $GNGLL,3945.11111,N,11608.87560,E,015835.00,A,A*75


typedef struct
{
    //UartDriverType  uart;          //linux 与 stm32串口通信文件描述符
    
    //通过 $GNRMC,015835.00,A,3945.11111,N,11608.87560,E,0.199,,181117,,,A*62 提取日期和时间
    //       时间         维度         经度       1-有效 0-无效  卫星数据        高度
    //$GNGGA,015835.00,3945.11111,N,11608.87560,E,1             ,08       ,1.03, 65.2 ,M,-9.2,M,,*6D
    char  strGNGGA[200];  //提取的GNGGA 值
    char  strGNRMC[200];  //提取的GNRMC 值

    //struct tm gpsTime;
	int gpsTime_year;
    int gpsTime_mon;
    int gpsTime_mday ;
    int gpsTime_hour ;
    int gpsTime_min ;
    int gpsTime_sec ;
    int gpsTimeSec;

    short  getDataState ; //0--无数据    1-GNRMC  2-GNGGA
    short  getDataCnt ;

    unsigned long  LastGetGpsTime;  //上一次定位时间，用于判断GPS是否异常

    char lng_str[16];  //经度
    char lat_str[16];  //纬度
    char height_str[8];//高度

    char  lat_n_s;  //	纬度：n:north;s:south
    char  lon_w_e;  //	经度：w:west e:east
    unsigned char lng_sign;      
    unsigned char lat_sign;
    bool  bCurGetPos;
    bool  bCurGetTime;

    float f_lng;
    float f_lat;
    float f_height;

}StateGPS;
extern StateGPS gGPS;

extern bool GpsParaTimeDateForGNRMC(char* strIn);
extern bool GpsParaPosDateFromGNGGA(char* strIn);
/************************************************* 
*Return:      int 0：初始化成功  -1：初始化失败
*DESCRIPTION: 初始化与GPS通信的串口
*************************************************/
//int gps_init(void);

/************************************************* 
*Return:		
*DESCRIPTION: GPS串口接收到数据后的处理流程
*************************************************/
//void GPS_recv_data_CB(UartDriverType* pUart);


#endif
