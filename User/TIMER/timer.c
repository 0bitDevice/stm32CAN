#include "timer.h"
#include "akq.h"
#include "dma.h"
#include <string.h>

extern MSG_SEND akq;
extern u8 sendmsg[30];
extern CanTxMsg TxMessage01;
extern CanTxMsg TxMessage02;

extern u8 CANpackMsNum;		//CAN包计数0-9
u8 CAN1FrameNum = 0;			//CAN1包每秒计数0-255
u8 CAN2FrameNum = 0;			//CAN2包每秒计数0-255

void Tim2_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //时钟使能
	
	//定时器TIM2初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE );	
}

void Tim3_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断	
    TIM_Cmd(TIM3, ENABLE);	
}

static void zeroTxMsgData(CanTxMsg *RxMessage)
{
	uint8_t i = 0;
	for (i = 0; i < 8; i++)
	{
		RxMessage->Data[i] = 0x00;
	}
}

static void sendCANMsg(CAN_TypeDef* CANx, CanTxMsg* TxMessage,u8* CANFrameNum)
{
	u8 arrIndex = 0, frameNum = 0;	
	
	while(frameNum != 4)
	{
		switch(frameNum)
		{
			case 0:
				arrIndex = 0;
				TxMessage->Data[arrIndex++] = *CANFrameNum;
				TxMessage->Data[arrIndex++] = CANpackMsNum;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.hour;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.minute;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.second;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.navstate;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.cmd;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.backup[0];	
				break;
			case 1:
				arrIndex = 0;
				TxMessage->Data[arrIndex++] = *CANFrameNum;
				TxMessage->Data[arrIndex++] = CANpackMsNum;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.longitude >> 24;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.longitude >> 16;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.longitude >> 8;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.longitude;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.speedground >> 8;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.speedground;
				break;
			case 2:
				arrIndex = 0;
				TxMessage->Data[arrIndex++] = *CANFrameNum;
				TxMessage->Data[arrIndex++] = CANpackMsNum;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.latitude >> 24;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.latitude >> 16;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.latitude >> 8;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.latitude;	
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.speedsky >> 8;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.speedsky;	
				break;
			case 3:
				arrIndex = 0;
				TxMessage->Data[arrIndex++] = *CANFrameNum;
				TxMessage->Data[arrIndex++] = CANpackMsNum;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.flightangle >> 8;	
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.flightangle;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.height >> 8;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.height;
				TxMessage->Data[arrIndex++] = akq.msg_to_akq.ux450_data.numbers;
				TxMessage->Data[arrIndex++] = 0xFF;
				break;
			default:
				break;
		}

		if(CAN_Transmit(CANx, TxMessage) != CAN_NO_MB)
		{
//			++*CANFrameNum;
			++frameNum;
		}
		zeroTxMsgData(TxMessage);
	}
	++*CANFrameNum;
}
//定时器2中断服务程序
void TIM2_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  			//清除TIMx更新中断标志 		
		TIM_Cmd(TIM2, DISABLE);  	
//////////////////////////////////////////////////////////////////////////////////////
        if((bdtxr == 1) || (bdtxr == 2) )//坠毁,自检
		{			
			if(itxr > 0)
			{
				itxr --;					
			}
			if(itxr == 0)  //发送完10次
			{	
				bdtxr = 0;
				akq.msg_to_akq.ux450_data.cmd = 0x00;	
			}		
		}				
		memcpy((u8*)sendmsg,(u8*)akq.msg_send,30);
		BD2GPS_CheckSum((u8*)sendmsg,sizeof(MSG_TO_AKQ_TAG)-1,&sendmsg[29]);	
		MyDMA_Enable(DMA1_Channel2,30);	
		
		sendCANMsg(CAN1, &TxMessage01, &CAN1FrameNum);
		sendCANMsg(CAN2, &TxMessage02, &CAN2FrameNum);

/////////////////////////////////////////////////////////////////////////////////////		
		Tim3_Init(999,7199); 	
	}
}

//定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  			//清除TIMx更新中断标志 		
//////////////////////////////////////////////////////////////////////////////////////
        if((bdtxr == 1) || (bdtxr == 2) )//坠毁,自检
		{			
			if(itxr > 0)
			{
				itxr --;					
			}
			if(itxr == 0)  //发送完10次
			{	
				bdtxr = 0;
				akq.msg_to_akq.ux450_data.cmd = 0x00;	
			}		
		}				
		memcpy((u8*)sendmsg,(u8*)akq.msg_send,30);
		BD2GPS_CheckSum((u8*)sendmsg,sizeof(MSG_TO_AKQ_TAG)-1,&sendmsg[29]);
		MyDMA_Enable(DMA1_Channel2,30);	
		
		sendCANMsg(CAN1, &TxMessage01, &CAN1FrameNum);
		sendCANMsg(CAN2, &TxMessage02, &CAN2FrameNum);

/////////////////////////////////////////////////////////////////////////////////////		
	}
}

	 
