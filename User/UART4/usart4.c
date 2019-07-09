#include "usart4.h"

	  
void Uart4_Init(u32 bound)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	// GPIOBʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE); //����4ʱ��ʹ��

 	USART_DeInit(UART4);  //��λ����3
	
	//USART4_TX   PC10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PC10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOC, &GPIO_InitStructure); //��ʼ��PC10
   
    //USART4_RX	  PC11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOC, &GPIO_InitStructure);  //��ʼ��PC11
	
	USART_InitStructure.USART_BaudRate = bound;//������һ������Ϊ115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Tx;	//�շ�ģʽ  
	USART_Init(UART4, &USART_InitStructure); //��ʼ������	3
	
	USART_DMACmd(UART4, USART_DMAReq_Tx, ENABLE);
	
	USART_Cmd(UART4, ENABLE);                    //ʹ�ܴ��� 
}


 



















