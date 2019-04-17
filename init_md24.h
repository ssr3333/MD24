/*
 * File:   init_md48.h
 * Author: Administrator
 *
 * Created on 2014年5月26日, 下午23:27
 */
 
 /* ================================引用头文件=============================== */
#ifndef INIT_MODBUS_H
#include "init_modbus.h"
#endif

#ifndef INIT_MD24_H
#define	INIT_MD24_H

#ifdef	__cplusplus
extern "C" {
#endif

/* ==================================宏定义================================== */

/* ==================================函数声明================================ */
void MD24_SETDO(void);  //读取EEPROM，设置输出
void MD24_GETDI(void);  //读取输入，存入EEPROM
//extern void memcpyee(eeprom unsigned char *to, const unsigned char *from, unsigned char size);
/*===================================全局变量==================================*/

#ifdef	__cplusplus
}
#endif

#endif	/* INIT_MD48_H */