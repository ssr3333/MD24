/*******************************************************************************
 * MD88ͨѶ����Ҳ������MD44��MD66��MD48�ȣ�ͨ��Ԥ����ģ���������ã� 
 * ʱ�䣺2014-08-14  15:00
 * ���ߣ�������
 * MCU: PIC16F690     OSC:Ƭ��4MHz
 * ����������ͨ��MODBUS-RTUЭ����ǹ�����������ͨѶ  �첽����ͨѶ������9600
*******************************************************************************/

/* ================================����ͷ�ļ�================================ */
#include <xc.h>
#include <stdint.h>
#include "init_config.h"
#include "init_16f690.h"
#include "init_modbus.h"
//#include "init_md48.h"

/*===================================�궨��====================================*/
//����ģ�����ͣ�����MD48- 4·DI��8·DO�� 
#define MD24                    //ģ��2��4��

#ifdef MD24 
//����������붨�壨���������·�����壩 
#define DI_MASK	0xFC		//DI��x·������������ĸߣ�8-x)λΪ1
#define DO_MASK 0x33		//DO��y·�����������ĸߣ�8-y)λΪ0
#endif
#ifdef MD48 
//����������붨�壨���������·�����壩 
#define DI_MASK	0xF0		//DI��x·������������ĸߣ�8-x)λΪ1
#define DO_MASK 0xFF		//DO��y·�����������ĸߣ�8-y)λΪ0
#endif
#ifdef MD68
//����������붨�壨���������·�����壩 
#define DI_MASK	0xC0		//DI��x·������������ĸߣ�8-x)λΪ1
#define DO_MASK 0xFF		//DO��y·�����������ĸߣ�8-y)λΪ0
#endif 



/*===================================��������==================================*/
void process_cmd();            //������յ�������ָ��

/*===================================ȫ�ֱ���==================================*/
uint8_t RC_DATA[60];            //�������ݻ�������
uint8_t TX_DATA[60];            //�������ݻ������飨ָ�빲��RC���壬��ʡ�ռ䣩
//uint8_t IO_RAM[54];   		//IO״̬����
//#define DI_STATUS IO_RAM[53]	//IO_RAM[53]����DI�������״ֵ̬
//#define DO_STATUS IO_RAM[49] 	//IO_RAM[49]����DO�������״ֵ̬

uint8_t IO_RAM[27];             //IO״̬����
#define DI_STATUS IO_RAM[26]	//IO_RAM[0x1A]����DI�������״ֵ̬
#define DO_STATUS IO_RAM[24] 	//IO_RAM[0x18]����DO�������״ֵ̬

uint8_t FLAG_IOCHANGE = 1;    	//�˿ڵ�ƽ�仯��־,��ʼ��1��������ǿ�Ƹ���һ��
uint8_t tmr2DelayCnt  = 0;      //TMR2�������

/* =============================================================================
 * ��  �ƣ�������
 * ��  �룺
 * ��  ����
 * ��  �ܣ�
 * ===========================================================================*/
void main(void)
{
    init_16f690();  //оƬ��ʼ��
    RTE = 0;     	//��ʼ��Ϊ����״̬
    RCIE = 1;    	//�������ж�
    eecpymem(IO_RAM, 0x20, 27);   //��EEPROM���ݻ��浽IO״̬����
    

    RX_LED_OFF(); 	//�շ�ָʾ�Ƴ�ʼ��ΪϨ��״̬
    RC_N =0;    	//��ʼ�������ַ�����
    FLAG_15 = 0;
    FLAG_35 = 0;
    FLAG_RCOK = 0;  //����־����
    TMR1H =T1_35H;  
    TMR1L =T1_35L;  //TMR1��Ϊ3.5�ַ�������״̬
    FLAG_T1 =1; 	//����3.5�ַ��������־
    DI_STATUS = ~PORTA | DI_MASK;   //�洢DI�˿�״̬,����λ��Ҫ
    
    TMR1IE = 1;  	//��TMR1�ж�
    TMR2IE = 1;     //��TMR2�ж�
    TMR2ON = 1;     //����Timer2
    PEIE = 1;    	//�������ж�
    GIE =1;     	//ȫ���жϿ���
    CLRWDT();       //ι��
    
    ADD = EEPROM_READ(0x19);    //��ȡ����485��ַ
    
    /* =============��ѭ��============ */
    while(1)
    {
        TX_DATA[0] = ADD;           //�������ݵ�1λ�Ǳ�����ַ
        /*���ڽ���ָ���*/
        if( RC_N && FLAG_RCOK )     //�����������һ֡���ս���
        {
            crc16(RC_DATA,RC_N);    //����CRC16У��
            if( CRC_L == 0 && CRC_H == 0)
            {
                uint8_t count = 0;  	//���ݼ���
                count = RC_DATA[5] * 2; //���ݸ���
                if( RC_DATA[1] == 0x03 )//����ǲ�ѯָ�����
                {
                    if(RC_DATA[2] == 0x04)  //����Ƕ�IO����
                    {
                        for(char i = 0; i < count; i += 2)
                        {
                            TX_DATA[i+3] = 0x00;    //��ȡ���������ݣ�ǰ��0,�����ݣ�
                            TX_DATA[i+4] = IO_RAM[ i/2 + RC_DATA[3]];
                        }
                    }
                    else if( RC_DATA[2] ==0x00 )    //����Ƕ���������
                    {
                        eecpymem(TX_DATA+3, RC_DATA[3] * 2, count); //����EEPROM����
                    }
                    TX_DATA[1] = RC_DATA[1];        //Ӧ������
                    TX_DATA[2] = count;              //���ݸ���
                    modbus_send(TX_DATA, count+3);   //����Ӧ����
                }
                else if( RC_DATA[1] ==0x10 )    //�����дָ�����
                {
                    if(RC_DATA[2] ==0x04 )  	//�����дIO����
                    {
                        for(char i=0;i<count;i+=2)
                        {
                            IO_RAM[i/2+RC_DATA[3]] = RC_DATA[i+8];  //������д�뻺��
                        }
                        process_cmd();  	//�������״̬
                    }
                    else if( RC_DATA[2] ==0x00 )//�����д��������
                    {
                        memcpyee(RC_DATA[3]*2, RC_DATA + 7, count); //����д��EEPROM
                    }
                    modbus_send(RC_DATA,6); 	//���յ���ָ��ַ�����ʾ�ɹ�
                }
            }
            RC_N =0;                            //֡��������
            FLAG_RCOK =0;                       //������ɱ�־����׼����һ֡����
        }

        /*DI�˿ڱ仯�������뻺������*/
        if(FLAG_IOCHANGE)
        {
            if(tmr2DelayCnt > 250)              //
            {
                tmr2DelayCnt = 0;
                FLAG_IOCHANGE = 0;
                for( char i = 0; i < 8 ; i++ ) 	//��ѯ����ֵ���ɸ�λ��ʼ
                {
                    IO_RAM[i+0x10] = ( DI_STATUS>>i ) & 0x01 ;
                }
            }
        }
        
        CLRWDT();
    }
}

/* =============================================================================
 * ��  �ƣ��жϴ�����
 * ��  �룺
 * ��  ����
 * ��  �ܣ�
 * ===========================================================================*/
void interrupt ISR(void)
{
    /*Timer1�жϴ���*/
    if( TMR1IE && TMR1IF )
    {
        if(FLAG_T1 ==1) 			//�����֡������
        {
            FLAG_15 =0;  FLAG_35 =1;
        }
        else    //������ַ�������
        {
            FLAG_35 =0;  FLAG_15 =1;
            FLAG_RCOK =1;   		//�ַ������ʱ��һ֡���ս���
            RX_LED_OFF();   //һ֡���ս��������
            set_timer1(35);
        }
        TMR1IF =0;      			//����Timer1�жϱ�־λ
    }

    /*Timer2�жϴ���*/
    else if (TMR2IE && TMR2IF)
    {
        if(FLAG_IOCHANGE)
        {
            tmr2DelayCnt++;
        }
        TMR2IF = 0;
    }

    /*���ڽ����жϴ���*/
    else if( RCIF && RCIE )
    {
        RX_LED_ON();  				//����ָʾ�Ƶ���
        modbus_recv( RCREG );    	//��ȡ���������ݺ�RCIF��Ӳ���Զ�����
        //RCIF = 0;
    }

    /*�˿ڵ�ƽ�仯�жϴ���*/
    else if( RABIE && RABIF )
    {
        tmr2DelayCnt = 0;
        DI_STATUS = ~PORTA | DI_MASK;   //��ȡ�˿�״̬,����λ��Ҫ
        RABIF = 0;  				//���жϱ�־λ(��˿��ж�ǰ�ض��˿�ֵ��
        FLAG_IOCHANGE = 1;
    }
}

/* =============================================================================
 * ��  �ƣ����յ�ָ�����
 * ��  �룺
 * ��  ����
 * ��  �ܣ�ִ�н��յ���ָ���������Ӧ
 * ===========================================================================*/
void process_cmd(void)
{
    unsigned char portc = 0;			//DO���ֵ��ʱ����
	unsigned char temp;					// 
    for(char i=0;i<8;i++)
    {
        portc <<= 1;
        portc |= IO_RAM[ 7-i ];  		//��ǰDO����״̬����ڻ���0x00-0x0F
    }
    DO_STATUS = (portc & DO_MASK);		//��ǰDO��״̬(���������)������뻺��
    temp  = (PORTC & ~DO_MASK);			//��ȡ��ǰPORTC�з�DOλ��״̬ 
    PORTC = (portc & DO_MASK)|temp;     //��������
}
