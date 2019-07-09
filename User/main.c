#include "stm32f10x.h"
#include "usart2.h"
#include "usart3.h"	
#include "usart4.h"
#include "timer.h"
#include "dma.h"
#include "can.h"
#include "nvic.h"
#include "akq.h"
#include <string.h>

extern void GPIO_Configuration(void);
extern u8 GPS_check(u8 buf);

MSG_SEND akq;   ///
u8 sendmsg[30];

u32 i = 0;
u32 j = 0;

u8 gps_cksum = 0;
u8 gps_cksum1 = 0;
u8 gps_flag = 0;
u32 gps_cnt = 0;
u32 buf_cnt = 0;
u8 GPS_BUF[200];

int main(void)
{	
	akq.msg_to_akq.ux450_hdr = 0x90EB;					///文件头
	for(i=0;i<5;i++)     								///备份字节
		akq.msg_to_akq.ux450_data.backup[i] = 0;
	akq.msg_to_akq.ux450_data.cmd = 0x00;
 
	Nvic_Init(); // 中断初始化
	
	Usart2_Init(115200); 		///初始化串口2为115200  接收
	
	////////////////////////////////////////////
	Usart3_Init(38400);		//初始化串口3波特率为  发送
	DMA_Tx_Config(DMA1_Channel2,(u32)&USART3->DR,(u32)sendmsg,30);//DMA1通道4,外设为串口1,存储器为SendBuff,长度SEND_BUF_SIZE.
	////////////////////////////////////////////
	
	CAN_Config();				//初始化CAN
	Tim2_Init(399,7199); 
	
	while(1) 
	{			
		 if(u2_flag)
		 {			 	 
		     u2_flag = 0;
			 for(i=0;i<buf_cnt;i++)
			 {
				 if(USART2_RX_BUF[i] == '$')
				 {
					gps_flag = 1;
					gps_cnt = 0;
				 }
				 if(gps_flag)
				 {
					GPS_BUF[gps_cnt++] = USART2_RX_BUF[i];
					if(USART2_RX_BUF[i] == 0xA)
					{
						gps_flag = 0;
						gps_cksum = 0;
						gps_cksum1 = 0;
						for(j=1;j<(gps_cnt-5);j++)
						{
						   gps_cksum = gps_cksum ^ GPS_BUF[j];		 
						}		 
						gps_cksum1 = (GPS_check(GPS_BUF[gps_cnt-4]) << 4)|GPS_check(GPS_BUF[gps_cnt-3]);
						if(gps_cksum == gps_cksum1)
						{
						  BD2GPS_Analysis(&akq,(u8*)GPS_BUF);
						  memset((u8*)GPS_BUF,0,200);									
						}
					}
				 }
			}												
            TIM_Cmd(TIM2, ENABLE);		
		 }	 
	}
}

u8 GPS_check(u8 buf)
{
	u8 p=buf;
	
	if(buf<='9')
		p -= 48;
	if(buf>='A')
		p -= 55;
	return p;

}
 
void GPIO_Configuration(void)
{
	//B12.B14
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_14;						  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	GPIO_SetBits(GPIOB,GPIO_Pin_14);
}


