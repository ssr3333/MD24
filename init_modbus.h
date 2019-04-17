/* 
 * File:   init_modbus.h
 * Author: Administrator
 *
 * Created on 2014��5��11��, ����10:52
 */

#ifndef INIT_MODBUS_H
#define	INIT_MODBUS_H

#ifdef	__cplusplus
extern "C" {
#endif

/*===================================�궨��====================================*/
//#define ADD 1           // �ӻ�485��ַ
//#define FUN_CODE 4      /* �����룬0x04��ʾ��һ����������Ĵ�����ȡ�õ�ǰ��ֵ
//                         * MODBUSЭ����65-72Ϊ�û����Զ�����룬������ж���*/
#define T1_15H  0xF4    // 9600������ÿ�ַ�ʱ��Ϊ1146us,
#define T1_15L  0xCF    // 1.5�ַ�ʱ��Ϊ2865us,T1_15������Ϊ0xF4CF
#define T1_35H  0xEB    // 3.5�ַ�ʱ��Ϊ5156us,
#define T1_35L  0xDC    // T1_35������Ϊ0xEBDC
#define RTE     RB6     // ���ڽ��շ���ʹ�ܣ��ø߷��ͣ�ͬʱTX_LED�� �õͽ���

#define RX_LED_ON()  RB4 = 0  //���ڽ�������ָʾ�ƣ��õ�ʱRX_LED����
#define RX_LED_OFF() RB4 = 1  //���ڽ�������ָʾ�ƣ��ø�ʱRX_LEDϨ��

/*===================================��������==================================*/
void crc16(unsigned char* recvByte, unsigned char count);   //crc16У�飬������� CRC16_H, CRC16_L
void send_one(unsigned char);    //���ڷ���1�ַ�
void modbus_send(char* ptr,char len);   //MODBUS��������ָ�����ȵ�����
void modbus_recv(char recv_data);   //MODBUS�������ݴ���Ŀ������
void set_timer1(char mode); //����TIMER1��ʱģʽ���ַ����or֡���

/*===================================ȫ�ֱ���==================================*/
unsigned char FLAG_T1;  //0��ʾTMR1��ʱ1.5���ַ�ʱ�䣬1��ʾTMR1��ʱ3.5���ַ�ʱ��
unsigned char FLAG_15;  //1.5�ַ�ʱ�䵽��־
unsigned char FLAG_35;  //3.5�ַ�ʱ�䵽��־
unsigned char FLAG_RCOK;//��ʾ�յ���ȷվ�ţ������뼰��Ӧ���ȵ����ݣ�����δCRCУ��
unsigned char RC_N;     //�������ݼ���
unsigned char CRC_L,CRC_H;  //CRC16У������λ����λ
unsigned char ADD;      //Modbus�豸��ַ
extern unsigned char RC_DATA[60];   //���յĵ����ݻ������������������ļ���

#ifdef	__cplusplus
}
#endif

#endif	/* INIT_MODBUS_H */