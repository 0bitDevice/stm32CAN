#include "usart4.h"

	  
void Uart4_Init(u32 bound)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	// GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE); //串口4时钟使能

 	USART_DeInit(UART4);  //复位串口3
	
	//USART4_TX   PC10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PC10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化PC10
   
    //USART4_RX	  PC11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOC, &GPIO_InitStructure);  //初始化PC11
	
	USART_InitStructure.USART_BaudRate = bound;//波特率一般设置为115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Tx;	//收发模式  
	USART_Init(UART4, &USART_InitStructure); //初始化串口	3
	
	USART_DMACmd(UART4, USART_DMAReq_Tx, ENABLE);
	
	USART_Cmd(UART4, ENABLE);                    //使能串口 
}


 



















