#ifndef __USART2_H
#define __USART2_H	 
 
#include "stm32f10x.h"


#define USART2_NUM		1000

extern u8 USART2_RX_BUF[USART2_NUM];
extern u32 u2_cnt;
extern u8 u2_flag;

void Usart2_Init(u32 bound);				//´®¿Ú2³õÊ¼»¯ 


#endif













