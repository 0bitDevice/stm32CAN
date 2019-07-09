/***************************
*文件名：akq.h
*创建者：zhangzengwei
*创建时间：2016-7-20
*文件描述：
***************************/

#ifndef __AKQ_H
#define __AKQ_H
#include "stm32f10x.h"

#define pi 3.1415926

extern u8 bdtxr;
extern u8 itxr;

/*向安控器发送的数据*/
#pragma  pack(push,1)
typedef struct msgdata_to_akq
{
	u8 navstate;										/*定位状态	*/
	u8 numbers;											/*星数			*/
	s32 longitude;									/*经度			*/
	s32 latitude;										/*纬度			*/
	u16 height;											/*高度			*/
	u16 speedground;								/*地速			*/
	s16 speedsky;										/*天速			*/
	u16 flightangle;								/*航迹角		*/
	u8 hour;												/*时				*/
	u8 minute;											/*分				*/
	u8 second;											/*秒				*/
	u8 cmd;													/*安控指令	*/
	u8 backup[5];										/*备份1			*/
	
}MSGDATA_TO_AKQ;

/*向安控器发送的数据帧格式*/
typedef struct msg_TO_AKQ					/*数据帧		*/
{
	u16 ux450_hdr;									/*帧头			*/
	MSGDATA_TO_AKQ ux450_data;			/*数据			*/
	u8 ux450_checksum;										/*校验			*/
	
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
