/***************************
*�ļ�����akq.h
*�����ߣ�zhangzengwei
*����ʱ�䣺2016-7-20
*�ļ�������
***************************/

#ifndef __AKQ_H
#define __AKQ_H
#include "stm32f10x.h"

#define pi 3.1415926

extern u8 bdtxr;
extern u8 itxr;

/*�򰲿������͵�����*/
#pragma  pack(push,1)
typedef struct msgdata_to_akq
{
	u8 navstate;										/*��λ״̬	*/
	u8 numbers;											/*����			*/
	s32 longitude;									/*����			*/
	s32 latitude;										/*γ��			*/
	u16 height;											/*�߶�			*/
	u16 speedground;								/*����			*/
	s16 speedsky;										/*����			*/
	u16 flightangle;								/*������		*/
	u8 hour;												/*ʱ				*/
	u8 minute;											/*��				*/
	u8 second;											/*��				*/
	u8 cmd;													/*����ָ��	*/
	u8 backup[5];										/*����1			*/
	
}MSGDATA_TO_AKQ;

/*�򰲿������͵�����֡��ʽ*/
typedef struct msg_TO_AKQ					/*����֡		*/
{
	u16 ux450_hdr;									/*֡ͷ			*/
	MSGDATA_TO_AKQ ux450_data;			/*����			*/
	u8 ux450_checksum;										/*У��			*/
	
}MSG_TO_AKQ_TAG;
 
typedef union
{
	u8 msg_send[30];
	MSG_TO_AKQ_TAG  msg_to_akq;
}MSG_SEND;
#pragma  pack(pop,1)   

void akq_us450_cfgsave(void);
void BD2GPS_Analysis(MSG_SEND *bd2gpsx,u8 *buf);
void BD2GPS_CheckSum(u8 *buf,u16 len,u8 *cksum);

#endif
