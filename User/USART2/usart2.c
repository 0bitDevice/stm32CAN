#include "string.h"	 
#include "usart2.h"


//接收缓存区  新

extern u32 buf_cnt;

u8 USART2_RX_BUF[USART2_NUM]; 
u32 u2_cnt = 0;
u8 u2_flag = 0;


//初始化IO 串口2
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率	  
void Usart2_Init(u32 bound)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //串口2时钟使能

	USART_DeInit(USART2);  //复位串口2

	//USART2_TX   PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA2  

	//USART2_RX	  PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA3

	USART_InitStructure.USART_BaudRate = bound;//波特率一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx;	//接收模式  
	USART_Init(USART2, &USART_InitStructure); //初始化串口	2  
	USART_Cmd(USART2, ENABLE);                    //使能串口 

	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//开启中断 
	USART_ITConfig(USART2,USART_IT_IDLE,ENABLE); 	
}


void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//接收到数据
	{		
		USART2_RX_BUF[u2_cnt++] = USART_ReceiveData(USART2);
		if(u2_cnt>=USART2_NUM)
		{
			u2_cnt = 0;
		}
	}  
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET) // 空闲中断
	{ 		
		u2_flag = USART2->SR;
		u2_flag = USART2->DR;

		buf_cnt = u2_cnt;
		u2_cnt = 0;
		u2_flag = 1;

		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  			//清除TIMx更新中断标志  			  	
		TIM_Cmd(TIM3, DISABLE);			
	}
		
	if(USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET) 
	{ 
		USART_ClearFlag(USART2, USART_FLAG_ORE);
	} 				 											 
}   






















