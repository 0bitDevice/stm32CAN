#ifndef __DMA_H
#define	__DMA_H	   

#include "stm32f10x.h"

void DMA_Tx_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);
void MyDMA_Enable(DMA_Channel_TypeDef* DMA_CHx,u8 num);
		   
#endif




