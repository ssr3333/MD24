/* 
 * File:   init_modbus.h
 * Author: Administrator
 *
 * Created on 2014年5月11日, 上午10:52
 */

#ifndef INIT_MODBUS_H
#define	INIT_MODBUS_H

#ifdef	__cplusplus
extern "C" {
#endif

/*===================================宏定义====================================*/
//#define ADD 1           // 从机485地址
//#define FUN_CODE 4      /* 功能码，0x04表示在一个或多个输入寄存器中取得当前的值
//                         * MODBUS协议中65-72为用户可自定义代码，其余均有定义*/
#define T1_15H  0xF4    // 9600波特率每字符时间为1146us,
#define T1_15L  0xCF    // 1.5字符时间为2865us,T1_15常数设为0xF4CF
#define T1_35H  0xEB    // 3.5字符时间为5156us,
#define T1_35L  0xDC    // T1_35常数设为0xEBDC
#define RTE     RB6     // 串口接收发送使能，置高发送，同时TX_LED亮 置低接收

#define RX_LED_ON()  RB4 = 0  //串口接收数据指示灯，置低时RX_LED点亮
#define RX_LED_OFF() RB4 = 1  //串口接收数据指示灯，置高时RX_LED熄灭

/*===================================函数声明==================================*/
void crc16(unsigned char* recvByte, unsigned char count);   //crc16校验，结果存入 CRC16_H, CRC16_L
void send_one(unsigned char);    //串口发送1字符
void modbus_send(char* ptr,char len);   //MODBUS发送数组指定长度的数据
void modbus_recv(char recv_data);   //MODBUS接收数据存入目标数组
void set_timer1(char mode); //设置TIMER1延时模式，字符间隔or帧间隔

/*===================================全局变量==================================*/
unsigned char FLAG_T1;  //0表示TMR1延时1.5个字符时间，1表示TMR1延时3.5个字符时间
unsigned char FLAG_15;  //1.5字符时间到标志
unsigned char FLAG_35;  //3.5字符时间到标志
unsigned char FLAG_RCOK;//表示收到正确站号，功能码及相应长度的数据，但还未CRC校验
unsigned char RC_N;     //接收数据计数
unsigned char CRC_L,CRC_H;  //CRC16校验结果低位、高位
unsigned char ADD;      //Modbus设备地址
extern unsigned char RC_DATA[60];   //接收的到数据缓冲区，定义在其它文件中

#ifdef	__cplusplus
}
#endif

#endif	/* INIT_MODBUS_H */