#include "can.h"

CanTxMsg TxMessage01;
CanTxMsg TxMessage02;
CanRxMsg RxMessage;
__IO uint32_t flag = 0xff;		 //用于标志是否接收到数据，在中断函数中赋值

void CAN_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	
	/* Configure CAN1 and CAN2 IOs **********************************************/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure CAN1 RX pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure CAN2 RX pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure CAN1 TX pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure CAN2 TX pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure CAN1 and CAN2 **************************************************/  
	/* CAN1 and CAN2 Periph clocks enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1 | RCC_APB1Periph_CAN2, ENABLE);  

	/* CAN1 and CAN2 register init */
	CAN_DeInit(CAN1);
	CAN_DeInit(CAN2);

	/* Struct init*/
	CAN_StructInit(&CAN_InitStructure);

	/* CAN1 and CAN2  cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = ENABLE;
	CAN_InitStructure.CAN_NART = ENABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = ENABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;  
	CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;

	#if CAN_BAUDRATE == 1000 /* 1MBps */
	CAN_InitStructure.CAN_Prescaler =6;
	#elif CAN_BAUDRATE == 500 /* 500KBps */
	CAN_InitStructure.CAN_Prescaler =12;
	#elif CAN_BAUDRATE == 250 /* 250KBps */
	CAN_InitStructure.CAN_Prescaler =24;
	#elif CAN_BAUDRATE == 125 /* 125KBps */
	CAN_InitStructure.CAN_Prescaler =48;
	#elif  CAN_BAUDRATE == 100 /* 100KBps */
	CAN_InitStructure.CAN_Prescaler =60;
	#elif  CAN_BAUDRATE == 50 /* 50KBps */
	CAN_InitStructure.CAN_Prescaler =120;
	#elif  CAN_BAUDRATE == 20 /* 20KBps */
	CAN_InitStructure.CAN_Prescaler =300;
	#elif  CAN_BAUDRATE == 10 /* 10KBps */
	CAN_InitStructure.CAN_Prescaler =600;
	#else
	#error "Please select first the CAN Baudrate in Private defines in main.c "
	#endif  /* CAN_BAUDRATE == 1000 */


	/*Initializes the CAN1  and CAN2 */
	CAN_Init(CAN1, &CAN_InitStructure);
	CAN_Init(CAN2, &CAN_InitStructure);

///////////////////////////////未使用/////////////////////////////////////////
	/* CAN1 filter init */
	CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh= (((u32)0x1314<<3)&0xFFFF0000)>>16;				//要过滤的ID高位 
    CAN_FilterInitStructure.CAN_FilterIdLow= (((u32)0x1314<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF; //要过滤的ID低位 
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0xFFFF;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0xFFFF;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);

	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
	
	CAN_SlaveStartBank(14);
	CAN_FilterInitStructure.CAN_FilterNumber = 14;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh= (((u32)0x1315<<3)&0xFFFF0000)>>16;				//要过滤的ID高位 
    CAN_FilterInitStructure.CAN_FilterIdLow= (((u32)0x1315<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF; //要过滤的ID低位 
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0xFFFF;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0xFFFF;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);

	CAN_ITConfig(CAN2, CAN_IT_FMP0, ENABLE);
//////////////////////////////////未使用///////////////////////////////////////

	/* Transmit */
	TxMessage01.StdId = 0x321;
	TxMessage01.ExtId = 0x01;
	TxMessage01.RTR = CAN_RTR_DATA;
	TxMessage01.IDE = CAN_ID_STD;
	TxMessage01.DLC = 8;  
	
	TxMessage02.StdId = 0x322;
	TxMessage02.ExtId = 0x01;
	TxMessage02.RTR = CAN_RTR_DATA;
	TxMessage02.IDE = CAN_ID_STD;
	TxMessage02.DLC = 8;  
}

void CAN1_RX0_IRQHandler(void)
{
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

//	if((RxMessage.ExtId==0x1314) && (RxMessage.IDE==CAN_ID_EXT)
//	&& (RxMessage.DLC==2) && ((RxMessage.Data[1]|RxMessage.Data[0]<<8)==0xDCBA))
//	{
//		flag = 0x00;
//	}
//	else
//	{
//		flag = 0xff;
//	}
}

void CAN2_RX0_IRQHandler(void)
{
	CAN_Receive(CAN2, CAN_FIFO1, &RxMessage);

//	if((RxMessage.ExtId==0x1314) && (RxMessage.IDE==CAN_ID_EXT)
//	&& (RxMessage.DLC==2) && ((RxMessage.Data[1]|RxMessage.Data[0]<<8)==0xDCBA))
//	{
//		flag = 0x00;
//	}
//	else
//	{
//		flag = 0xff;
//	}
}
