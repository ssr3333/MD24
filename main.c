/*******************************************************************************
 * MD88通讯程序（也适用于MD44，MD66，MD48等，通过预定义模块类型设置） 
 * 时间：2014-08-14  15:00
 * 作者：宋松如
 * MCU: PIC16F690     OSC:片内4MHz
 * 功能描述：通过MODBUS-RTU协议与枪柜控制器进行通讯  异步串行通讯波特率9600
*******************************************************************************/

/* ================================引用头文件================================ */
#include <xc.h>
#include <stdint.h>
#include "init_config.h"
#include "init_16f690.h"
#include "init_modbus.h"
//#include "init_md48.h"

/*===================================宏定义====================================*/
//定义模块类型（例如MD48- 4路DI，8路DO） 
#define MD24                    //模块2入4出

#ifdef MD24 
//输入输出掩码定义（由输入输出路数定义） 
#define DI_MASK	0xFC		//DI有x路，则输入掩码的高（8-x)位为1
#define DO_MASK 0x33		//DO有y路，则输出掩码的高（8-y)位为0
#endif
#ifdef MD48 
//输入输出掩码定义（由输入输出路数定义） 
#define DI_MASK	0xF0		//DI有x路，则输入掩码的高（8-x)位为1
#define DO_MASK 0xFF		//DO有y路，则输出掩码的高（8-y)位为0
#endif
#ifdef MD68
//输入输出掩码定义（由输入输出路数定义） 
#define DI_MASK	0xC0		//DI有x路，则输入掩码的高（8-x)位为1
#define DO_MASK 0xFF		//DO有y路，则输出掩码的高（8-y)位为0
#endif 



/*===================================函数声明==================================*/
void process_cmd();            //处理接收到的数据指令

/*===================================全局变量==================================*/
uint8_t RC_DATA[60];            //接收数据缓冲数组
uint8_t TX_DATA[60];            //发送数据缓冲数组（指针共用RC缓冲，节省空间）
//uint8_t IO_RAM[54];   		//IO状态缓存
//#define DI_STATUS IO_RAM[53]	//IO_RAM[53]代表DI输入的总状态值
//#define DO_STATUS IO_RAM[49] 	//IO_RAM[49]代表DO输出的总状态值

uint8_t IO_RAM[27];             //IO状态缓存
#define DI_STATUS IO_RAM[26]	//IO_RAM[0x1A]代表DI输入的总状态值
#define DO_STATUS IO_RAM[24] 	//IO_RAM[0x18]代表DO输出的总状态值

uint8_t FLAG_IOCHANGE = 1;    	//端口电平变化标志,初始置1，程序中强制更新一次
uint8_t tmr2DelayCnt  = 0;      //TMR2溢出计数

/* =============================================================================
 * 名  称：主函数
 * 输  入：
 * 输  出：
 * 功  能：
 * ===========================================================================*/
void main(void)
{
    init_16f690();  //芯片初始化
    RTE = 0;     	//初始设为接收状态
    RCIE = 1;    	//开接收中断
    eecpymem(IO_RAM, 0x20, 27);   //将EEPROM数据缓存到IO状态数组
    

    RX_LED_OFF(); 	//收发指示灯初始化为熄灭状态
    RC_N =0;    	//初始化接收字符计数
    FLAG_15 = 0;
    FLAG_35 = 0;
    FLAG_RCOK = 0;  //各标志清零
    TMR1H =T1_35H;  
    TMR1L =T1_35L;  //TMR1设为3.5字符间隔检测状态
    FLAG_T1 =1; 	//设置3.5字符间隔检测标志
    DI_STATUS = ~PORTA | DI_MASK;   //存储DI端口状态,掩码位不要
    
    TMR1IE = 1;  	//开TMR1中断
    TMR2IE = 1;     //开TMR2中断
    TMR2ON = 1;     //启动Timer2
    PEIE = 1;    	//开外设中断
    GIE =1;     	//全局中断开启
    CLRWDT();       //喂狗
    
    ADD = EEPROM_READ(0x19);    //读取本机485地址
    
    /* =============主循环============ */
    while(1)
    {
        TX_DATA[0] = ADD;           //发送数据第1位是本机地址
        /*串口接收指令处理*/
        if( RC_N && FLAG_RCOK )     //如果有数据且一帧接收结束
        {
            crc16(RC_DATA,RC_N);    //进行CRC16校验
            if( CRC_L == 0 && CRC_H == 0)
            {
                uint8_t count = 0;  	//数据计数
                count = RC_DATA[5] * 2; //数据个数
                if( RC_DATA[1] == 0x03 )//如果是查询指令功能码
                {
                    if(RC_DATA[2] == 0x04)  //如果是读IO数据
                    {
                        for(char i = 0; i < count; i += 2)
                        {
                            TX_DATA[i+3] = 0x00;    //读取缓存中数据（前置0,后数据）
                            TX_DATA[i+4] = IO_RAM[ i/2 + RC_DATA[3]];
                        }
                    }
                    else if( RC_DATA[2] ==0x00 )    //如果是读配置数据
                    {
                        eecpymem(TX_DATA+3, RC_DATA[3] * 2, count); //读出EEPROM数据
                    }
                    TX_DATA[1] = RC_DATA[1];        //应答功能码
                    TX_DATA[2] = count;              //数据个数
                    modbus_send(TX_DATA, count+3);   //发送应答报文
                }
                else if( RC_DATA[1] ==0x10 )    //如果是写指令功能码
                {
                    if(RC_DATA[2] ==0x04 )  	//如果是写IO数据
                    {
                        for(char i=0;i<count;i+=2)
                        {
                            IO_RAM[i/2+RC_DATA[3]] = RC_DATA[i+8];  //将数据写入缓存
                        }
                        process_cmd();  	//处理输出状态
                    }
                    else if( RC_DATA[2] ==0x00 )//如果是写配置数据
                    {
                        memcpyee(RC_DATA[3]*2, RC_DATA + 7, count); //数据写入EEPROM
                    }
                    modbus_send(RC_DATA,6); 	//将收到的指令部分返回以示成功
                }
            }
            RC_N =0;                            //帧计数清零
            FLAG_RCOK =0;                       //接收完成标志清零准备下一帧接收
        }

        /*DI端口变化处理，存入缓冲数组*/
        if(FLAG_IOCHANGE)
        {
            if(tmr2DelayCnt > 250)              //
            {
                tmr2DelayCnt = 0;
                FLAG_IOCHANGE = 0;
                for( char i = 0; i < 8 ; i++ ) 	//轮询设置值，由高位开始
                {
                    IO_RAM[i+0x10] = ( DI_STATUS>>i ) & 0x01 ;
                }
            }
        }
        
        CLRWDT();
    }
}

/* =============================================================================
 * 名  称：中断处理函数
 * 输  入：
 * 输  出：
 * 功  能：
 * ===========================================================================*/
void interrupt ISR(void)
{
    /*Timer1中断处理*/
    if( TMR1IE && TMR1IF )
    {
        if(FLAG_T1 ==1) 			//如果是帧间隔检测
        {
            FLAG_15 =0;  FLAG_35 =1;
        }
        else    //如果是字符间隔检测
        {
            FLAG_35 =0;  FLAG_15 =1;
            FLAG_RCOK =1;   		//字符间隔超时，一帧接收结束
            RX_LED_OFF();   //一帧接收结束，灭灯
            set_timer1(35);
        }
        TMR1IF =0;      			//清零Timer1中断标志位
    }

    /*Timer2中断处理*/
    else if (TMR2IE && TMR2IF)
    {
        if(FLAG_IOCHANGE)
        {
            tmr2DelayCnt++;
        }
        TMR2IF = 0;
    }

    /*串口接收中断处理*/
    else if( RCIF && RCIE )
    {
        RX_LED_ON();  				//接收指示灯点亮
        modbus_recv( RCREG );    	//读取接收区数据后RCIF由硬件自动清零
        //RCIF = 0;
    }

    /*端口电平变化中断处理*/
    else if( RABIE && RABIF )
    {
        tmr2DelayCnt = 0;
        DI_STATUS = ~PORTA | DI_MASK;   //读取端口状态,掩码位不要
        RABIF = 0;  				//清中断标志位(清端口中断前必读端口值）
        FLAG_IOCHANGE = 1;
    }
}

/* =============================================================================
 * 名  称：接收到指令处理函数
 * 输  入：
 * 输  出：
 * 功  能：执行接收到的指令或做出响应
 * ===========================================================================*/
void process_cmd(void)
{
    unsigned char portc = 0;			//DO输出值临时缓存
	unsigned char temp;					// 
    for(char i=0;i<8;i++)
    {
        portc <<= 1;
        portc |= IO_RAM[ 7-i ];  		//当前DO分项状态存放在缓存0x00-0x0F
    }
    DO_STATUS = (portc & DO_MASK);		//当前DO总状态(掩码运算后)结果存入缓存
    temp  = (PORTC & ~DO_MASK);			//获取当前PORTC中非DO位的状态 
    PORTC = (portc & DO_MASK)|temp;     //将结果输出
}
