#include <xc.h>
#include "init_modbus.h"

/* =============================================================================
 * 名  称：发送一个字节
 * 输  入：char字符
 * 输  出：
 * 功  能：串口发送一个字节数据
 * ===========================================================================*/
void send_one(char a)
{
    TXREG =a;   //数据存入发送寄存器
    while(!TRMT);   //等待发送完成，TRMT置1
    //delay_us(19);    //延时202us  等待2bit停止位数据时长
}

/* =============================================================================
 * 名  称：MODBUS-RTU 发送函数
 * 输  入：ptr指令数组， len要发送的数据长度
 * 输  出：
 * 功  能：发送指定个数的字节数据，并自动加上CRC低、高两位字节
 * ===========================================================================*/
void modbus_send(char* ptr,char len)
{
    crc16(ptr,len);     //生成校验结果
    RTE = 1; //485通讯为半双工，置发送状态
    TMR1IE = 0; //暂停定时器中断
    for(char i = 0; i < len; i++)
    {
        send_one(ptr[i]); //逐位发送
    }
    send_one(CRC_L);    //发送CRC校验低位
    send_one(CRC_H);    //发送CRC校验高位
    RTE = 0; //控制485通讯芯片转入接收状态
    TMR1IE = 1; //恢复定时器中断
}

/* =============================================================================
 * 名  称：MODBUS-RTU 接收函数
 * 输  入：
 * 输  出：
 * 功  能：接收到的数据存入目标数组
 * ===========================================================================*/
void modbus_recv(char recv_data)
{
    unsigned char rd;
    rd = recv_data;
    if(RC_N == 0) //如果是第1个字节，判断3.5倍间隔时间是否足够
    {
        if( 1 == FLAG_35 && recv_data == ADD )   //如果间隔大于3.5倍字符时间且地址匹配
        {
            RC_DATA[0] = rd; //数据有效，存入接收数组
            RC_N++;             //字符计数加1
            set_timer1(15);
        }
        else    //否则，间隔时间不足
        {
            set_timer1(35); //数据丢弃，等待一下个字符
        }
    }
    else if(RC_N < 60)   //其它位置数据
    {
        RC_DATA[RC_N] = rd; //数据有效，存入接收数组
        RC_N++;
        set_timer1(15);     //转为帧间检测
    }
    //接收超过60字节的不再处理了,RC_N也不再增加,此时RC_N=60
//    RX_LED_OFF(); //处理完毕，熄灭接收指示灯
}

/* =============================================================================
 * 名  称：CRC16校验函数
 * 输  入：char字符数组， len 数据长度
 * 输  出：
 * 功  能：校验计算后的数据存入CRC_L,CRC_H
 * ===========================================================================*/
void crc16(unsigned char* recvByte, unsigned char count) //对数组指定长度数据进行校验
{
    unsigned char saveLo, saveHi;
    CRC_L = 0xFF;   //初始化
    CRC_H = 0xFF;

    for (unsigned char n = 0; n < count; n++) //对接收数据CRC校验
    {
        CRC_L ^= recvByte[n];   //数据与CRC低位进行异或
        for (char i = 0; i < 8; i++)
        {
            saveHi = CRC_H;
            saveLo = CRC_L;
            CRC_H >>= 1;     //CRC高段 右移1位
            CRC_L >>= 1;     //CRC低段 右移1位
            if ((saveHi & 0x01) == 0x01)     //如果CRC高位字节最后一位是1
            {
                CRC_L |= 0x80;    //则CRC低位字节前面补1，相当于将CRC高位移出的1移过来
            }
            if ((saveLo & 0x01) == 0x01)    //如果CRC_L最低位是1
            {
                CRC_H ^= 0xA0;   //则CRC与多项式码进行异或运算  多项式X15+X13+1
                CRC_L ^= 0x01;
            }
        }
    }
}

/* =============================================================================
 * 名  称：等待从设备返回数据函数
 * 输  入：
 * 输  出：接收结果，0接收到并校验正确，1校验失败，2超时未收到数据
 * 功  能：等待从设备返回数据
 * ===========================================================================
unsigned char wait_data(void)
{
    char over=0;    //记录TMR0溢出次数
    TMR1H =T1_35H; TMR1L =T1_35L;   TMR1IF =0;//发送完成后，TMR1转为帧间3.5字符间隔检测设置
    FLAG_T1 =1;     //标志表明转为进行帧间3.5字符间隔检测
    TMR0 = 60;  //当分频比为1:256时，一次溢出为（256-60）*256=50176us≈50ms
    T0IF = 0;   //清零Timer0中断标志
    while(FLAG_RCOK ==0)    //如果未收到或未收完数据则等待
    {
        if(T0IF ==1)    //如果等到50ms超时
        {
            TMR0 =60;
            T0IF =0;
            over++;
        }
        if(over ==2)    //累计2次溢出，即100ms(足够收到32位数据的时间）
            break;      //跳出等待
    }
    if(FLAG_RCOK ==1)   //如果接收到1组“有效”数据（未经CRC16校验）
    {
        FLAG_RCOK =0;   //数据接收成功标志清零，准备下一次接收
        CRC_H =0xFF;    CRC_L =0xFF;    //CRC16初始化
        for(char j=0; j<RC_N; j++) //对接收数据CRC校验
        {
            crc16(RC_DATA[j]);
        }
        if(CRC_H ==0 && CRC_L ==0)  //如果校验正确
        {
            GIE =0; //暂停中断，等待数据处理
            return 0;
        }
        else    //如果校验失败
        {
            return 1;
        }
    }
    else  //否则，超时未收到有效数据
    {
        return 2;
    }
}
*/
/* =============================================================================
 * 名  称：Timer1延时设置
 * 输  入：只能为15或35，表示1.5倍间隔检测或3.5倍间隔检测
 * 输  出：
 * 功  能：设置Timer1初始值，以改变其溢出中断时间，做为串口数据间隔检测之用
 * ===========================================================================*/
void set_timer1(char mode)
{
    switch(mode)
    {
        case 15:    FLAG_15 = FLAG_35 = 0;  //1.5、3.5倍间隔超时标志清零
                    TMR1H = T1_15H;
                    TMR1L = T1_15L;  //TMR1转为帧内1.5字符间隔检测设置
                    TMR1IF = 0;
                    FLAG_T1 = 0;     //标志表明进行帧内1.5字符间隔检测
                    break;
        case 35:    FLAG_15 = FLAG_35 = 0;  //1.5、3.5倍间隔超时标志清零
                    TMR1H = T1_35H;
                    TMR1L = T1_35L;  //TMR1转为帧间3.5字符间隔检测设置
                    TMR1IF = 0;
                    FLAG_T1 = 1;     //标志表明转为进行帧间3.5字符间隔检测
                    break;
        default:    break;
    }
}