/*
 * File:   init_md48.h
 * Author: Administrator
 *
 * Created on 2014��5��26��, ����23:27
 */
 
 /* ================================����ͷ�ļ�=============================== */
#ifndef INIT_MODBUS_H
#include "init_modbus.h"
#endif

#ifndef INIT_MD24_H
#define	INIT_MD24_H

#ifdef	__cplusplus
extern "C" {
#endif

/* ==================================�궨��================================== */

/* ==================================��������================================ */
void MD24_SETDO(void);  //��ȡEEPROM���������
void MD24_GETDI(void);  //��ȡ���룬����EEPROM
//extern void memcpyee(eeprom unsigned char *to, const unsigned char *from, unsigned char size);
/*===================================ȫ�ֱ���==================================*/

#ifdef	__cplusplus
}
#endif

#endif	/* INIT_MD48_H */