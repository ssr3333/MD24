#include <xc.h>
#include "init_16f690.h"

/*===EEPROM初始值===*/
/*EEPROM 初始值(只能顺序的写入，每次写入8Byte) 每2Byte为1个寄存器位*/
// 主/从设备、设备版本、模块名称（10）天津
__EEPROM_DATA (0x00,0x04,0x01,0x00,'M','D','2','4');    //0x00～0x07
//
__EEPROM_DATA (0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);    //0x08～0x0F
//
__EEPROM_DATA (0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);    //0x10～0x17
// 模块地址、空闲（3）
__EEPROM_DATA (0x00,0x02,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);    //0x18～0x1F
// 当前DO0～DO7状态
__EEPROM_DATA (0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);    //0x20～0x27
// 上电时DO0～DO7状态
__EEPROM_DATA (0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);    //0x28～0x2F
// DI0～DI7状态
__EEPROM_DATA (0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01);    //0x30～0x37
// DO的状态、上电时DO的状态、DI的状态
__EEPROM_DATA (0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00);    //0x38～0x3F

__EEPROM_DATA (0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);    //0x40～0x47
__EEPROM_DATA (0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);    //0x48～0x4F
__EEPROM_DATA (0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);    //0x50～0x57
__EEPROM_DATA (0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);    //0x58～0x5F

/* =============================================================================
 *                              初始化函数
 * ========================================================================== */
void init_16f690()
{
//振荡器控制
    OSCCON = 0x68;      //bit6-4: 内部振荡器频率4MHz（111为8MHz）
//WDT控制
    WDTCON = 0x15;      //bit<4-1>WDT自身预分频比，1：65536 溢出2.1秒
    SWDTEN = 0;         //WDTCON<bit0> 软控制WDT使能，仅在WDTE_OFF时有效
//选项寄存器配置
    OPTION_REG = 0x0F;
    /* bit<7>PORTA、PORTB弱上拉使能  &bit<6>RA2/INT引脚的下降沿触发中断
     * &bit<5>TIMER0时钟源选择内部时钟osc/4  &bit<4>timer0在RA2/TOCKI引
     * 脚低到高变化时递增计数
     * &bit<3>预分频器分配给TMR0   &bit<2:0>分频比为1:256
     */
//中断控制寄存器
    INTCON = 0x00;
    /* bit<7>全局中断GIE禁止 &bit<6>外设中断PEIE禁止 &bit<5>TMR0中断禁止
     * &bit<4>RA2/INT外部中断禁止
     * &bit<3>PORTA/PORTB电平变化中断禁止 &bit<2:0>清零T0IF、INTF、RABIF
     */
//外设中断控制寄存器1
    PIE1 = 0x00;
    /* 未实现位0 &bit<6>禁止ADC中断 &禁止EUSART接收中断 &禁止EUSART发送中断
     * &禁止SSP中断 &禁止CCP1中断 &禁止Timer2与PR2匹配中断 &bit<0>禁止TMR1IE
     * 中断
     */
//外设中断控制寄存器2
    PIE2 = 0x00;
    /*禁止振荡器故障中断 &禁止比较器C2中断 &禁止比较器C1中断 &禁止写操作
     * 中断 & bit3-0未实现，0
     */
//外设中断请求寄存器1，2
    PIR1 = 0x00;    //bit0:TMR1IF timer1溢出中断标志位
    PIR2 = 0x00;
//电源控制寄存器
    PCON = 0x13;
    /*bit5:禁止超低功耗唤醒 &使能BOR &bit1:未发生POR置1 &未发生BOR置1*/
//模拟引脚配置
    ANSEL = 0x00;	//模拟引脚选择（低位），全部置为数字引脚
    ANSELH = 0x00;	//模拟引脚选择（高位），全部置为数字引脚
//比较器配置
    CM1CON0 = 0x00; //关闭比较器
    CM2CON0 = 0x00;
//PORTA相关寄存器
    PORTA = 0x3F;
    TRISA = 0x3F;   //RA<5:0>配置为输入端口，（RA3-MCLR只能为输入）
    WPUA  = 0x3F;   //使能RA<5:0>弱上拉，若RA3为MCLR，自动开启弱上拉
    IOCA  = 0x3F;   //开启RA<5:0>电平变化中断
//PORTB相关寄存器（ 只有RB<7:4> )
    PORTB = 0x30;   //RB7串口发送, RB6发送使能, RB5串口接收, RB4接收LED
    TRISB = 0x20;   //RB<5>配置为输入端口 串口RX应用
    WPUB  = 0x20;   //使能RB<5>端口弱上拉
    IOCB  = 0x00;   //禁止PORTB端口电平变化中断
//PORTC相关寄存器
    TRISC = 0x00;   //RC<7:0>配置为输出端口
    PORTC = 0x00;   //PORTC输出低电平
//计时器1控制寄存器
    T1CON = 0x01;
    /* Timer1门控低电平有效 &Timer1始终计数 &bit5-4:Timer1与时钟预分频比1:1
     * &LP振荡器关闭 &同步外部时钟输入(使用内部时钟时无关) &Timer1使用内部
     * 时钟OSC/4 &Timer1使能
     */
//计时器2控制寄存器
    PR2   = 0xFA;   //Timer2溢出周期为250
    T2CON = 0x49;    /* bit<7> 未用位
                     * bit<6:3>Timer2后分频比为1:10
                     * bit<2>TMR2ON不开启
                     * bit<1:0>预分频比1:4 */
//USART串行口设置
    SYNC    = 0;    //异步模式

    BRGH    = 1;    //高速波特率使能
    BRG16   = 1;    //使用16位波特率发生器
    SPBRGH  = 0x00; //SPBRG高位
    SPBRG   = 0x67; //SPBRG低位，控制波特率为9600

    SPEN    = 1;    //使能异步串行口
    TXEN    = 1;    //TX发送使能
    CREN    = 1;    //RX接收使能
//配置完成后开启所需中断
    RABIE   =1;     //PORTA，PORTB端口电平变化中断使能
    PEIE    =1;     //外设中断使能
    T0IE    =0;     //Timer0中断使能（需先在）
    TMR1IE  =1;     //Timer1中断使能（需先在T1CON中使能TMR1ON）
    TMR2IE  =0;     //Timer2中断禁止（需先在T2CON中使能TMR2ON）
    RCIE    =1;     //RX接收中断使能（需先使能CREN）
    //GIE     =0;     //全局中断禁止
}
