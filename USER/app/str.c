#include <includes.h>


/*
************************************************************************************************************************
*                                                   GetCheckAddVal
*
* Description: 这个功能是获得数据包的校验和
*
* Arguments  : pData	一个指向数据包首地址的指针
*
*		     len        数据包的长度
*
*Returns    : added    所有数据的和 
************************************************************************************************************************
*/

unsigned char GetCheckAddVal(unsigned char* pData,unsigned short len)
{
    unsigned char added = 0;
    while(len--)
    {
        added += *pData++;
    }
    return added;
}

/*
************************************************************************************************************************
*                                                   box_str_len
*
* Description: 这个功能是计算字符串的长度
*
* Arguments  : psc	指向要计算的数组的首地址的指针
*
*
*Returns    : (psc - psc_in)    返回地址差 即为长度 
************************************************************************************************************************
*/
unsigned short box_str_len(const char* psc)
{
    const char* psc_in = psc;
    while(*psc)
    {
        psc++;
    }
    return (psc - psc_in); //返回地址差 即为长度
}

//获取字符串长度，有长度限制
unsigned short box_str_len_limit(const char* psc,unsigned short nLimint)
{
    const char* psc_in = psc;
    unsigned short il = 0;
    while( (*psc) && (il++<nLimint) )
    {
        psc++;
    }
    return (psc - psc_in); //返回地址差 即为长度
}

/*
************************************************************************************************************************
*                                                   box_str_cpy_rt_pos
*
* Description: 这个功能是拷贝字符串，并返回结束位置的 指针
*
* Arguments  : pOut	一个指向原字符串的尾的指针
*
*		    pIn        一个指向新字符串头的指针
*
*Returns    : pOut    源字符串的尾，也就是新拷贝的字符串的头
************************************************************************************************************************
*/
//拷贝字符串，并返回结束位置的 指针
///return: 源字符串的尾，也就是新拷贝的字符串的头
char* box_str_cpy_rt_pos(char* pOut,const char* pIn)
{
    while( *pIn )
    {
       *pOut = *pIn ;
        pOut++;
        pIn++;
    }
    *pOut = 0 ;
    return pOut;
}
/*
************************************************************************************************************************
*                                                  box_str_cpy
*
* Description: 这个功能是拷贝字符串，并返回数据长度
*
* Arguments  : pOut	一个指向原字符串的尾的指针
*
*		    pIn        一个指向新字符串头的指针
*
*Returns    : str_len    数据长度
*************************************************************************************************************************/
unsigned short box_str_cpy(char* pOut,const char* pIn)
{
    unsigned short str_len = 0;
    while( *pIn )
    {
        *pOut = *pIn ;
        pOut++;
        pIn++;
        str_len++;
    }
    *pOut = 0 ;
    return str_len;
}
/*************************************************************************************************************************
*                                                   box_str_cpy_zhuanyi
*
* Description: 这个功能是拷贝字符串，并且 增加并替换成转义字符 \  ，返回结束位置
*
* Arguments  : pOut	一个指向原字符串的尾的指针
*
*		    pIn        一个指向新字符串头的指针
*
*Returns    : pOut    结束位置
************************************************************************************************************************/
char* box_str_cpy_zhuanyi(char* pOut,const char* pIn)
{
    while( *pIn )
    {
        if ( (*pIn==',') || (*pIn=='\\') || (*pIn=='\"') )
        {
            *pOut++ = '\\';
        }
        *pOut++ = *pIn++ ;
    }
    *pOut = 0 ;
    return pOut;
}
/*************************************************************************************************************************
*                                                   box_str_find_last
*
* Description: 从后到钱比较两个字符的数据，不同则返回pFind数组不相同的位置
*
* Arguments  : pSource	一个指向原数据尾地址的指针
*
*		    pFind        一个指向要查找的数据尾地址的指针
*           nFindLen      要查找数据的长度
*Returns    : 0
*************************************************************************************************************************/
unsigned char  box_str_find_last(const char* pSource,const char* pFind,unsigned char nFindLen)
{
    while( nFindLen )
    {
        nFindLen--;
        if ( (*pSource) != pFind[nFindLen] )
        {
            return nFindLen+1;  //返回不同的长度
        }
        pSource--;
    }
    return 0;
}
/*************************************************************************************************************************
*                                                   wifi_str_find_char
*
* Description: 查找字符串中 某个范围内的数据
*
* Arguments  : strd	一个指向原数据首地址的指针
*
*		    valMin        该范围的最小值
*           valMax      该范围的最大值
*Returns    : strd     在该范围内的数据的地址
*************************************************************************************************************************/
char* box_str_find_char(char* strd,char valMin,char valMax)
{
    while( (*strd<valMin) || (*strd>valMax) ){
        strd++;
    }
    return strd;
}
/*************************************************************************************************************************
*                                                   box_str_find_string_begin
*
* Description: 查找字符串中是否有strFind 字符串存在。 并且返回对应的开始位置 如果没有找到，返回 0
*
* Arguments  : pOutBig	一个指向原数据首地址的指针
*
*		    pInToFind        一个指向要查找的数据首地址的指针
*           
*Returns    : 如果存在，返回对应的开始位置 如果没有找到，返回 0
*************************************************************************************************************************/
char* box_str_find_string_begin(char* pOutBig,const char* pInToFind)
{
    const char* pPosCmpFnd ;                                //对比使用的指针
    const char* pPosCmpSrc;                                 //对比使用的指针
    if ( (*pInToFind) == 0 )                                //需要找的字符串 为 空
    {
        return 0;
    }
    while( * pOutBig )
    {
        if ( (*pOutBig) == (*pInToFind) )
        {
            pPosCmpFnd = pInToFind; //小字符串
            pPosCmpSrc = pOutBig;
            while( ((*pPosCmpFnd)) && ( (*pPosCmpFnd)==(*pPosCmpSrc) ) )
            {
                pPosCmpFnd++; 
                pPosCmpSrc++;
            }
            if (  (*pPosCmpFnd == 0) ) //如果小字符串值为0 表示找到了
            {
                return pOutBig;
            }
            if ( *pPosCmpSrc == 0 )
            {
                return 0;
            }
        }
        pOutBig ++;  //整体 寻找的 指针加1
    }
    return 0;
}
/*************************************************************************************************************************
*                                                   box_str_find_string_end
*
* Description: 查找字符串，并且返回末尾的位置 如果没有找到，返回 NULL
*
* Arguments  : pOutBig	一个指向原数据首地址的指针
*
*		    pInToFind        一个指向要查找的数据首地址的指针
*           
*Returns    : 如果存在，返回末尾的位置 如果没有找到，返回 NULL
*************************************************************************************************************************/
char* box_str_find_string_end(char* pOutBig,const char* pInToFind)
{
    const char* pPosCmpFnd ;                                //对比使用的指针
    char* pPosCmpSrc;                                 //对比使用的指针
    if ( (*pInToFind) == 0 )                                //需要找的字符串 为 空
    {
        return 0;
    }
    while( * pOutBig )
    {
        if ( (*pOutBig) == (*pInToFind) )
        {
            pPosCmpFnd = pInToFind; //小字符串
            pPosCmpSrc = pOutBig;
            while( ((*pPosCmpFnd)) && ( (*pPosCmpFnd)==(*pPosCmpSrc) ) )
            {
                pPosCmpFnd++; 
                pPosCmpSrc++;
            }
            if (  (*pPosCmpFnd == 0) ) //如果小字符串值为0 表示找到了
            {
                //e.g.  "we1are2here" ->"are"  and return pos->'2' 
                return pPosCmpSrc;
            }
            if ( *pPosCmpSrc == 0 )
            {
                return 0;
            }
        }
        pOutBig ++;  //整体 寻找的 指针加1
    }
    return 0;
}
/*************************************************************************************************************************
*                                                 box_str_cmp_left
*
* Description: 对比 某个字符串是否是 另外一个的左边
*
* Arguments  : p1	一个指向原数据首地址的指针
*
*		    p2        一个指向要对比的数据首地址的指针
*           
*Returns    : (*p1) - (*p2)         当(*p1)不等于(*p2)时，返回他俩的差
*************************************************************************************************************************/
char box_str_cmp_left(const char* p1,const char* p2)
{
    while( ((*p1)) && (*p1) == (*p2) ){
        p1++;
        p2++;
    }
    if ( ((*p1) == 0) || ((*p2) == 0) )
    {
        return 0;
    }
    return (*p1) - (*p2);
}

char box_str_cmp(const char* p1,const char* p2)
{
    while( ((*p1)) &&  ( (*p1) == (*p2) )  ){
        p1++;
        p2++;
    }
    if ( ((*p1) == 0) && ((*p2) == 0) )
    {
        return 0;
    }
    return (*p1) - (*p2);
}

/*************************************************************************************************************************
*                                                 box_str_find_num_end
*
* Description: 找数字的尾
*
* Arguments  : pInToFind	一个指向要被查找的数据的首地址的指针
*
*		    
*           
*Returns    : nFindCounts         数字的长度
************************************************************************************************************************
*/
///找数字的尾
///pInToFind: [in]起始是数字的字符串;[out]指向数字尾的下一个字符
///ret: 数字的长度
unsigned short box_str_find_num_end(const char* pInToFind)
{
    unsigned short nFindCounts = 0;
    while( ( ( (*pInToFind)>='0' ) && ( (*pInToFind)<='9' ) )  )
    {
        nFindCounts++;
        pInToFind++;
    }
    return nFindCounts;
}
/*
************************************************************************************************************************
*                                                 box_str_left_not_tdc
*
* Description: 拷贝字符串，并且字符串拷贝过程需要判断 不能为特定的字符
*
* Arguments  : pSource	一个指向要被拷贝数据的首地址的指针
*              pDes     一个指向放数据的地址的指针
*		       len      要被拷贝数据的长度
*              endCh    字符结束符
*Returns    : len1         数字的长度
************************************************************************************************************************
*/
//拷贝字符串，并且字符串拷贝过程需要判断 不能为特定的字符
unsigned short box_str_left_not_tdc(const char* pSource ,char* pDes , unsigned short len , char endCh)
{
    unsigned short len1=0;
    while( (len1<len) && (*pSource) && (*pSource!=endCh) ){
        *pDes = *pSource;
        pDes++ ; pSource++;
        len1++;
    }
    *pDes = 0;
    return len1;
}
/*
************************************************************************************************************************
*                                                 box_str_find_num
*
* Description: 查找 字符串中数字部分的长度
*
* Arguments  : pInToFind	一个指向待查找数据首地址的指针
*              
*		       
*              
*Returns    : 字符串中数字部分的长度， 若没有数字则返回-1
************************************************************************************************************************
*/
//查找 字符串中数字部分的长度
///pInToFind: [in] 待查找的字符串; 若没有数字则指向字符串尾
///ret: 字符串中数字部分的长度， 若没有数字则返回-1.
signed short box_str_find_num(const char* pInToFind)
{
    unsigned short nFindCounts = 0;
    while( ( (*pInToFind) ) && (nFindCounts<1000) )
    {
        if ( ( (*pInToFind)>='0' ) && ( (*pInToFind)<='9' ) )
        {
            return nFindCounts;
        }
        nFindCounts++;
        pInToFind++;
    }
    return -1;
}
/*
************************************************************************************************************************
*                                                 box_str_find_char_pos
*
* Description: 寻找 某个字符串 中某位字符位置
*
* Arguments  : pstr	一个指向待查找数据地址的指针
*              fval     要查找的字符
*		       
*              
*Returns    : 0
************************************************************************************************************************
*/
//寻找 某个字符串 中某位字符位置
char* box_str_find_char_pos(char* pstr,char fval)
{
    unsigned short val_find_i = 0;
    while( (val_find_i<500) && (*pstr))
    {
        if ( *pstr == fval )
        {
            return pstr;
        }
        pstr++;
        val_find_i++;
    }
    return 0;
}
/*
************************************************************************************************************************
*                                                 box_str_get_int
*
* Description: 字符串转数字
*
* Arguments  : pstr	一个指向字符串首地址的指针
*              fval     一个指向放数字的首地址的指针
*		       
*              
*Returns    : (pstr-pstr_begin)字符串的长度
************************************************************************************************************************
*/
//字符串转数字，并且返回字符串 数字长度
unsigned char box_str_get_int(const char* pstr,signed short* pVal)
{
    const char* pstr_begin = pstr;
    unsigned char bIs_FuShu = 0;
    *pVal = 0;
    if ( *pstr == '-' )
    {
        bIs_FuShu = 1;
        pstr++;
    }
    while( (*pstr>='0') && (*pstr<='9')  )
    {
        *pVal *= 10;
        *pVal += *pstr - '0';
        pstr++;
    }
    if ( bIs_FuShu )
    {
        *pVal = -(*pVal);
    }
    return (pstr-pstr_begin);
}
/*
************************************************************************************************************************
*                                                 box_str_to_long
*
* Description: 字符串转数字
*
* Arguments  : pstr	一个指向字符串首地址的指针
*              
*		       
*              
*Returns    : pVal
************************************************************************************************************************
*/
//字符串 转数字
signed long  box_str_to_long(const char* pstr)
{
    signed long  pVal = 0;
    unsigned char bIs_FuShu = 0;
    if ( *pstr == '-' )
    {
        bIs_FuShu = 1;
        pstr++;
    }
    while( (*pstr>='0') && (*pstr<='9')  )
    {
        pVal *= 10;
        pVal += *pstr - '0';
        pstr++;
    }
    if ( bIs_FuShu )
    {
        pVal = -(pVal);
    }
    return pVal;
}
/*
************************************************************************************************************************
*                                                 box_str_s16_to_str
*
* Description: 数字转字符串
*
* Arguments  : pstr	一个指向数字首地址的指针
*              
*		       val
*              
*Returns    : pstr   返回结束位置
************************************************************************************************************************
*/
//数字转字符串，返回结束位置
char* box_str_s16_to_str(char* pstr,signed short val)
{
    char d = 0;
    char flag = 0;
    unsigned short i = 0;
    if ( val < 0 )
    {
        *pstr++ = '-';
        val = -val;
    }
    else if ( val == 0 )
    {
        *pstr++ = '0';
    }
    for (i = 10000; i > 0; i /= 10)
    {
        d = val / i;

        if (d || flag)
        {
            *pstr++ = (char)(d + 0x30);
            val -= (d * i);
            flag = 1;
        }
    }
    *pstr = 0;
    return pstr;
}

/*
************************************************************************************************************************
*                                                 box_str_int_to_str_len
*
* Description: 数字转字符串，固定长度，首位补0
*
* Arguments  : pStr	一个指向数字首地址的指针
*              
*		       val  
*              str_cnt   
*Returns    : 4
************************************************************************************************************************
*/
//数字转字符串，固定长度，首位补0
void box_str_int_to_str_len(char* pstr,unsigned short val,unsigned char str_cnt)
{
    pstr += str_cnt-1;
    while( str_cnt-- ){
        *pstr-- = (val%10) + '0';
        val /= 10;
    }
}
/*
************************************************************************************************************************
*                                                 box_str_ip_str_to_ips
*
* Description: 通过IP字符串 得到IP值 "192.168.3.4"
*
* Arguments  : pStr	一个指向字符串首地址的指针
*              
*		       pIpVal  一个指向存放IP地址的指针
*              
*Returns    : 4
************************************************************************************************************************
*/
//通过IP字符串 得到IP值 "192.168.3.4"
char box_str_ip_str_to_ips(const char* pstr,unsigned char* pIpVal)
{
    signed short getVal = 0;
    char i=0;
    for ( ; i<3 ; i++ )
    {
        pstr += box_str_get_int(pstr,&getVal);
        if( *pstr != '.' ){ return i; }
        pIpVal[i] = getVal;
        pstr++;
    }
    box_str_get_int(pstr,&getVal);
    pIpVal[3] = getVal;
    return 4; //返回 4
}
/*
************************************************************************************************************************
*                                                 box_str_find_ip_val
*
* Description: 字符串中查找IP地址类似的数据。并且返回找到的位置
*
* Arguments  : pStr	一个指向字符串首地址的指针
*              
*		       pIpVal  一个指向存放IP地址的指针
*              
*Returns    : 如果没有找到，则返回结束位置。   找到了，会直接进行一次解析地址动作
************************************************************************************************************************
*/
char* box_str_find_ip_val(char* pStr,unsigned char* pIpVal)
{
    signed short getVal = 0;
    char i=0;
    while( *pStr ){
        //先寻找 数字开始位置
        while( (*pStr<'0') || (*pStr>'9') ){ pStr++; if( *pStr == 0) return 0; }
        for ( i=0; i<3 ; i++ )
        {
            pStr += box_str_get_int(pStr,&getVal);
            if( *pStr != '.' ){ break; }
            pIpVal[i] = getVal;
            pStr++;
        }
        if ( i >= 3 )
        {
            pStr += box_str_get_int(pStr,&getVal);
            pIpVal[3] = getVal;
            return pStr; // 返回 结束 位置
        }
    }
    return 0; //返回 4
}
/*
************************************************************************************************************************
*                                                 box_str_ips_to_str
*
* Description: ip地址转字符串
*
* Arguments  : pIpVal	一个指向数字首地址的指针
*              
*		       pStrOut  一个指向存放IP地址的指针
*              
*Returns    : pStrOut   返回结束位置
************************************************************************************************************************
*/
//ip地址转字符串 至少需要 4*3=12字节数据

char* box_str_ips_to_str(unsigned char* pIpVal,char* pStrOut)
{
    unsigned char i=0;
    for( i=0 ; i<4 ; i++ )
    {
        pStrOut = box_str_s16_to_str( pStrOut , pIpVal[i] );
        *pStrOut++ = '.';
    }
    pStrOut--; //最后一个 '.' 位置
    *pStrOut = 0;
    return pStrOut;
}

char *int_to_str(signed long value, char *string)
{
    char    buf_fan[13];
    char    bit_Cnt=0;

    if ( value < 0 )
    {
        *string++ = '-';
        value = -value;
    }

    while( value )
    {
        buf_fan[bit_Cnt++] = value%10 + '0';
        value /= 10;
    }

    if ( bit_Cnt < 1 )
    {
        buf_fan[bit_Cnt++] = '0';
    }

    while ( bit_Cnt )
    {
        bit_Cnt--;
        *string++ = buf_fan[bit_Cnt];
    }
    *string = 0;
   return string;
}

char* box_str_float_to_str(char* pstr,float val,unsigned char XiaoShuWei)
{
    char xiaos;
    unsigned long nzs = 0;
    if ( val < 0.0f )
    {
        *pstr++ = '-';
        val = -val;
    }
    nzs = (unsigned long) val;
    if ( nzs > 30000 )
    {
        pstr = box_str_s16_to_str(pstr, nzs/10000 );
        box_str_int_to_str_len(pstr, nzs%10000 , 4 );
        pstr += 4;
    }
    else{
        pstr = box_str_s16_to_str(pstr, nzs );
    }
    *pstr++ = '.';
    val -= nzs;
    while( XiaoShuWei-- )
    {
        val *= 10.0f;
        xiaos =  ((char)val) ;
        val -= xiaos;
        *pstr++ = '0' + xiaos;
    }
    *pstr = 0;
    return pstr;
}

char* float_to_str(char *string,float val,int point_cnt)
{
    signed int ZsVal;
    if ( val < 0 )
    {
        *string++ = '-';
        val = -val;
    }

    ZsVal = (signed int)(val);
    val -= ZsVal;
    string = int_to_str(ZsVal,string);

    *string++ = '.';
    while( point_cnt-- )
    {
        val *= 10;
        ZsVal = (signed int)(val);
        val -= ZsVal;
        *string++ = ZsVal+'0';
    }
    *string = 0;
    return string;
}


#if 0

unsigned short GetCrc16_add(unsigned short crc,unsigned char *buf, unsigned int length)
{
    unsigned int i;
    unsigned int j;
    unsigned short c;
    for (i=0; i<length; i++)
    {
        c = *(buf+i) & 0x00FF;
        crc^=c;
        for (j=0; j<8; j++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            { 
                crc >>= 1;
            }
        }
    }
    return(crc);
}
unsigned short GetCRC16(unsigned char *buf, unsigned int length)
{
    unsigned int i;
    unsigned int j;
    unsigned short c;
    unsigned short crc = 0xFFFF;   //设置crc寄存器为0xffff
    for (i=0; i<length; i++)
    {
        c = *(buf+i) & 0x00FF;
        crc^=c;
        for (j=0; j<8; j++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            { 
                crc >>= 1;
            }
        }
    }
    return(crc);
}

#else

//////////////////////////////////CRC 校验和蓝牙名称修改

const unsigned short crc_const_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a,                 //0  10
    0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, //11 23
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401, 0x64e6,//24 36
    0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d, 0x3653, 0x2672,//37
    0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d,//50
    0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af,//63
    0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd,//76
    0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03,//89
    0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5,//102
    0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,//115
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004,//128
    0x4025, 0x7046, 0x6067, 0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290,//141
    0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c,//154
    0xc50d, 0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,//167
    0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634, 0xd94c,//180
    0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1,//193
    0x3882, 0x28a3, 0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37,//206
    0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,//219
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b,//232
    0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0 //245
};


unsigned short GetCrc16_add(unsigned short crc,unsigned char * buffer, unsigned char l)
{
    //     unsigned short by;
    //     unsigned char i;
    //     for ( i = 0; i < l; i++)
    //     {
    //         by = (crc >> 8) & 0xff;
    //         crc = (crc ) << 8;
    //         crc = (crc ^ crc_const_table[(buffer[i] ^ by) & 0xff]) ;
    //     }
    //     return crc;
    unsigned short by;
    while(l--)
    {
        by = (crc >> 8) & 0xff;
        crc = (crc ) << 8;
        crc = (crc ^ crc_const_table[((*buffer++) ^ by) & 0xff]) ;
    }
    return crc;
}

unsigned short GetCRC16(unsigned char *buffer, unsigned int length)
{
    //     unsigned short crc = 0xFFFF;   //设置crc寄存器为0xffff
    //     unsigned short by;
    //     while(length--)
    //     {
    //         by = (crc >> 8) & 0xff;
    //         crc = (crc ) << 8;
    //         crc = (crc ^ crc_const_table[((*buffer++) ^ by) & 0xff]) ;
    //     }
    //     return crc;
    return GetCrc16_add(0xFFFF,buffer,length);
}

#endif

//////////////////////////////////////////////////////////////////////////
//unsigned long 与字符串数据之间的转换;
//FA34 -> 0XFA34==64052
#define HEX_CHAR_TO_INT_VAL(ch) ( (((ch)<='9') && ((ch)>='0'))? ((ch)-'0') : \
    ( (((ch)<='F')&&((ch)>='A'))? ((ch)-('A'-10)) : ( (((ch)<='f')&&((ch)>='a'))? ((ch)-('a'-10)) : 30) ) )
#define INT_VAL_TO_HEX_CHAR(iVal)  ( ((iVal)<10)? ('0'+(iVal)) : ('A'-10+(iVal)) )
unsigned long box_str_hex_to_int(const char* strHex)
{
    unsigned long rt=0;
    char char_hex = *strHex++;
    unsigned char bitHex = HEX_CHAR_TO_INT_VAL(char_hex);
    while( bitHex <= 0X0F )
    {
        rt <<= 4;
        rt |= bitHex;
        char_hex = *strHex++;
        bitHex = HEX_CHAR_TO_INT_VAL(char_hex);
    }
    return rt;
}

char* box_str_u16_to_hex(char* strHex,unsigned short vInt)
{
    unsigned char i;
    unsigned char one_bit ;
    for( i=0 ; i<4 ; i++ )
    {
        one_bit = (vInt>>12)&0X0F ;
        vInt <<= 4;
        *strHex++ = INT_VAL_TO_HEX_CHAR(one_bit);
    }
    *strHex = 0;
    return strHex;
}
char* box_str_u32_to_hex(char* strHex,unsigned long  vInt)
{
    unsigned char i;
    unsigned char one_bit ;
    for( i=0 ; i<8 ; i++ )
    {
        one_bit = (vInt>>28)&0X0F ;
        vInt <<= 4;
        *strHex++ = INT_VAL_TO_HEX_CHAR(one_bit);
    }
    *strHex = 0;
    return strHex;
}


