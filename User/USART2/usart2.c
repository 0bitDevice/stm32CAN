#include "string.h"	 
#include "usart2.h"


//���ջ�����  ��

extern u32 buf_cnt;

u8 USART2_RX_BUF[USART2_NUM]; 
u32 u2_cnt = 0;
u8 u2_flag = 0;


//��ʼ��IO ����2
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������	  
void Usart2_Init(u32 bound)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //����2ʱ��ʹ��

	USART_DeInit(USART2);  //��λ����2

	//USART2_TX   PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA2  

	//USART2_RX	  PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA3

	USART_InitStructure.USART_BaudRate = bound;//������һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx;	//����ģʽ  
	USART_Init(USART2, &USART_InitStructure); //��ʼ������	2  
	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 

	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//�����ж� 
	USART_ITConfig(USART2,USART_IT_IDLE,ENABLE); 	
}


void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//���յ�����
	{		
		USART2_RX_BUF[u2_cnt++] = USART_ReceiveData(USART2);
		if(u2_cnt>=USART2_NUM)
		{
			u2_cnt = 0;
		}
	}  
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET) // �����ж�
	{ 		
		u2_flag = USART2->SR;
		u2_flag = USART2->DR;

		buf_cnt = u2_cnt;
		u2_cnt = 0;
		u2_flag = 1;

		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  			//���TIMx�����жϱ�־  			  	
		TIM_Cmd(TIM3, DISABLE);			
	}
		
	if(USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET) 
	{ 
		USART_ClearFlag(USART2, USART_FLAG_ORE);
	} 				 											 
}   





















