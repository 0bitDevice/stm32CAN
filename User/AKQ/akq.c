/***************************
*文件名：akq.c
*创建者：zhangzengwei
*创建时间：2016-7-20
*文件描述：
***************************/

#include "akq.h"
#include "usart3.h"
#include "string.h"
#include "math.h"


u8 numbers1=0,numbers2=0,numcnt=0;

extern MSG_SEND akq;
u8 navstate_flag = 0;

u8 bdtxr = 0;   // 指令安控类型
u8 itxr = 0;    // 指令安控计数

u8 CANpackMsNum = 0;		//CAN每秒发送的包序号0~9

//从buf里面得到第cx个逗号所在的位置
//返回值:0~0XFE,代表逗号所在位置的偏移.
//       0XFF,代表不存在第cx个逗号							  
u8 NMEA_Comma_Pos(u8 *buf,u8 cx)
{	 		    
	u8 *p=buf;
	while(cx)
	{		 
		if(*buf<' '||*buf>'z')return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;	 
}

//m^n函数
//返回值:m^n次方.
u32 NMEA_Pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}

///接收到短报文，进行提取短报文消息
int TQ(u8 *buf)
{
	char tqbuf[128];
	char tqok[128];
	int i=0,j=0,k=0,ok=0;
	u8 *p=buf;
	while(1)
	{
		if((*p!=',') && (*p!='*'))  ///提取字符串
		{
			tqbuf[i] = *p;
			p++;
			i++;
		}
		else
			break;
	}
	for(j=0;j<i;j++)    ///提取报文数字
	{
		if(tqbuf[j]=='3')
		{
			tqok[k]=tqbuf[j+1];
			k++;
			j++;
		}	
	}
	for(j=0;j<k;j++)	
	{
		if(tqok[j]=='0')	///判断是不是坠毁消息
			if(tqok[j+1]=='4')
				if(tqok[j+2]=='2')
					if(tqok[j+3]=='3')
					{
						ok++;
						if(ok>=2)
							return 0;
					}
		if(tqok[j]=='0')   ///判断是不是自检消息
			if(tqok[j+1]=='1')
				if(tqok[j+2]=='1')
					if(tqok[j+3]=='1')
					{
						ok++;
						if(ok>=2)
							return 1;
					}					
	}
	return 2;	
}

//str转换为数字,以','或者'*'结束
//buf:数字存储区
//dx:小数点位数,返回给调用函数
//返回值:转换后的数值

u8 floatval;
int NMEA_Str2num(u8 *buf,u8*dx)
{
	u8 *p=buf;
	u32 ires=0,fres=0;
	u8 ilen=0,flen=0,i;
	u8 mask=0;
	int res;
	while(1) //得到整数和小数的长度
	{
		if(*p=='-'){mask|=0X02;p++;floatval=1;}//是负数
		if(*p==','||(*p=='*'))break;//遇到结束了
		if(*p=='.'){mask|=0X01;p++;}//遇到小数点了
		else if(*p>'9'||(*p<'0'))	//有非法字符
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//去掉负号
	for(i=0;i<ilen;i++)	//得到整数部分数据
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//最多取5位小数
	*dx=flen;	 		//小数点位数
	for(i=0;i<flen;i++)	//得到小数部分数据
	{  
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	} 
	res=ires*NMEA_Pow(10,flen)+fres;
	//if(mask&0X02)res=-res;		   
	return res;
}

//分析GNRMC信息
void NMEA_GNRMC_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{
	u32 du1,du2;
	double fen1,fen2;
	u8 *p1,dx;			 
	u8 posx;     
	u32 temp;	   
	char navstate;
	u8 ewhemi;											/*东经还是西经*/
	u8 nshemi;											/*南纬还是北纬*/  
	u8 zero = 0;
	
	p1=(u8*)strstr((const char *)buf,"GNRMC");
	if(p1)
	{	
		posx=NMEA_Comma_Pos(p1,1);								//得到UTC时间
		if(posx!=0XFF)
		{
			temp=NMEA_Str2num(p1+posx,&dx);
			CANpackMsNum = temp % 1000 / 100;	//备份字节1存储毫秒转化的CAN发送的包顺序
			temp /= 1000;
			bd2gpsx->msg_to_akq.ux450_data.hour = temp/10000;
			bd2gpsx->msg_to_akq.ux450_data.minute = (temp/100)%100;
			bd2gpsx->msg_to_akq.ux450_data.second = temp%100;		
		}
			
		posx=NMEA_Comma_Pos(p1,3);								//得到纬度
		if(posx!=0XFF)
		{
			if(*(p1+posx)==',')
				zero = 1;
			else
			{
				temp=NMEA_Str2num(p1+posx,&dx);			
				du1 = temp/10000000;
				fen1 = (double)(temp%10000000)/6000000;
				bd2gpsx->msg_to_akq.ux450_data.latitude = (du1+fen1)*10000000;
			}
		}			
		posx=NMEA_Comma_Pos(p1,4);								//南纬还是北纬 ///通过此判断符号的正负
		if(posx != 0xFF)
		{
			nshemi = *(p1+posx);
			if(nshemi == 'S')
				bd2gpsx->msg_to_akq.ux450_data.latitude = ~(bd2gpsx->msg_to_akq.ux450_data.latitude)+1;
		}		
		posx=NMEA_Comma_Pos(p1,5);								//得到经度
		if(posx!=0XFF)
		{												  
			if(*(p1+posx)==',')
				zero = 1;
			else
			{
				temp=NMEA_Str2num(p1+posx,&dx);		 	 
				du2 = temp/10000000;
				fen2 = (double)(temp%10000000)/6000000;
				bd2gpsx->msg_to_akq.ux450_data.longitude = (du2+fen2)*10000000;
			}
		}
		posx=NMEA_Comma_Pos(p1,6);				//东经还是西经   ///通过此来判断符号的正负
		if(posx!=0XFF)
		{
			ewhemi=*(p1+posx);
			if(ewhemi=='W')
				bd2gpsx->msg_to_akq.ux450_data.longitude = ~(bd2gpsx->msg_to_akq.ux450_data.longitude)+1;
		}
		if(zero==1)
			bd2gpsx->msg_to_akq.ux450_data.navstate = 0x00;
		else
		{
			posx=NMEA_Comma_Pos(p1,2);								//得到定位状态
			if(posx!=0XFF)
			{
				navstate = *(p1+posx);
				if(navstate == 'A')				//定位
				{
					if(navstate_flag>0)
						navstate_flag--;				
					else
						bd2gpsx->msg_to_akq.ux450_data.navstate = 0x01;									
				}
				else if(navstate == 'V')				//不定位
				{	
					navstate_flag = 20;
					bd2gpsx->msg_to_akq.ux450_data.navstate = 0x00;
				}
			}
		}		
	}	
}

//分析GNZDA信息
void NMEA_GNZDA_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx;     
	u32 temp;
	
	p1=(u8*)strstr((const char *)buf,"GNZDA");
	if(p1)
	{
		posx=NMEA_Comma_Pos(p1,2);								//得到UTC时间
		if(posx!=0XFF)
		{
			temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//得到UTC时间,去掉ms
			//测试时分秒全为0的异常情况
			bd2gpsx->msg_to_akq.ux450_data.hour = temp/10000;
			bd2gpsx->msg_to_akq.ux450_data.minute = (temp/100)%100;
			bd2gpsx->msg_to_akq.ux450_data.second = temp%100;		
		}		
	}	
}


//分析GNVTG信息
void NMEA_GNVTG_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx;
	u32 flightangle,speedground;
	p1=(u8*)strstr((const char *)buf,"$GNVTG");
	if(p1)
	{		
		posx=NMEA_Comma_Pos(p1,1);                   //航迹角
		if(posx!=0XFF)
		{
			flightangle = NMEA_Str2num(p1+posx,&dx);
			bd2gpsx->msg_to_akq.ux450_data.flightangle = flightangle/NMEA_Pow(10,dx-1);
		} 	
		posx=NMEA_Comma_Pos(p1,7);								//地速
		if(posx!=0XFF)
		{
			speedground = NMEA_Str2num(p1+posx,&dx);
			speedground /= NMEA_Pow(10,dx-3);
			speedground /= 360;
			if(floatval==1) 
			{
				speedground = ~speedground+1;
				floatval = 0;
			}
			bd2gpsx->msg_to_akq.ux450_data.speedground = speedground;
		}
	}	
}

//分析GNTXR信息
void NMEA_GNTXR_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{
	u8 *p1,pd;			 
	u8 posx; 
	p1=(u8*)strstr((const char *)buf,"$BDTXR");
	if(p1)
	{
		posx=NMEA_Comma_Pos(p1,5);								///得到短报文内容
		if(posx!=0XFF)
		{
			pd=TQ(p1+posx+2);
			if(pd==0)	///坠毁   
			{
				bd2gpsx->msg_to_akq.ux450_data.cmd = 0x03;	
				bdtxr=1;
				itxr = 10;
			}
			else if(pd==1)    ///自检 
			{
				bd2gpsx->msg_to_akq.ux450_data.cmd = 0x02;
				bdtxr=2;
				itxr = 10;
			}
		}	
	}	
}

//分析GNGSA信息
void NMEA_GNGSA_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx; 
	u8 i;
	u8 possl;

	p1=(u8*)strstr((const char *)buf,"$GNGSA");
	if(p1)
	{
		for(i=0;i<12;i++)										//得到定位卫星编号通过编号得到的bd和gps数目
		{
			posx=NMEA_Comma_Pos(p1,3+i);					 
			if(posx!=0XFF) 
			{
				possl = NMEA_Str2num(p1+posx,&dx);				
				if((possl>=1) && (possl<=60))				
					numbers1 += 1;//numbers += (1<<4);
				else if((possl>=161))
					numbers2 += 1;	
			}
			else break; 
		}
		
		numcnt++;
		
//		akq.msg_to_akq.ux450_data.backup[1] = numcnt;      //测试用
//		akq.msg_to_akq.ux450_data.backup[2] = numbers1;    //测试用
//		akq.msg_to_akq.ux450_data.backup[3] = numbers2;    //测试用
		
		if(numcnt>=2)
		{
			bd2gpsx->msg_to_akq.ux450_data.numbers = (numbers1<<4)+numbers2;
			
		   numcnt = 0;
			 numbers1 = 0;
       numbers2 = 0;				
		}							
	}
}


//分析GNGGA信息
void NMEA_GNGGA_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{ 
	u8 *p1,dx;			 
	u8 posx;
  u32 height,val;	
	p1=(u8*)strstr((const char *)buf,"$GNGGA");
	if(p1)
	{			
		posx=NMEA_Comma_Pos(p1,9);								//得到高度
		if(posx!=0XFF)
		{
			height = NMEA_Str2num(p1+posx,&dx);
			height /= NMEA_Pow(10,dx-1);
			val = height%10;
			
			if(val >=5 )
				height = (height/10)*2+1;
			else
				height = (height/10)*2;
			
			if(floatval==1)
			{
			  height = ~height+1;
				floatval = 0;
			}
					
			bd2gpsx->msg_to_akq.ux450_data.height = height;			
		}
	}	
}

//分析GPDHV信息
void NMEA_GNDHV_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{ 
	u8 *p1,dx;			 
	u8 posx; 
	s32 speedsky;
  s32 Vx,Vy,Vz;
  double lon,lat;

	p1=(u8*)strstr((const char *)buf,"$GNDHV");
	
	if(p1)
	{		
		posx=NMEA_Comma_Pos(p1,3);	//得到X轴速度（km/h）
		if(posx!=0XFF) 
		{			
			Vx = NMEA_Str2num(p1+posx,&dx);
		
			if(floatval==1) 
			{
				Vx = ~Vx+1;
				floatval = 0;
			}			
		}	
		
		posx=NMEA_Comma_Pos(p1,4);	//得到Y轴速度（km/h）
		if(posx!=0XFF) 
		{			
			Vy = NMEA_Str2num(p1+posx,&dx);
			
			if(floatval==1) 
			{
				Vy = ~Vy+1;
				floatval = 0;
			}
		}	
		
		posx=NMEA_Comma_Pos(p1,5);	//得到Z轴速度（km/h）
		if(posx!=0XFF) 
		{			
			Vz = NMEA_Str2num(p1+posx,&dx);
			
			if(floatval==1) 
			{
				Vz = ~Vz+1;
				floatval = 0;
			}
		}
		
		lon = (double)bd2gpsx->msg_to_akq.ux450_data.longitude/10000000;
		lon = lon*pi/180;   // 经度（弧度）
		lat = (double)bd2gpsx->msg_to_akq.ux450_data.latitude/10000000;  
		lat = lat*pi/180;  // 纬度（弧度）
				
		speedsky = cos(lat)*cos(lon)*Vx/360+cos(lat)*sin(lon)*Vy/360+sin(lat)*Vz/360;  // 实际天速乘以10倍（m/s）
				
		bd2gpsx->msg_to_akq.ux450_data.speedsky = speedsky;  // 天速		
	}	
}

//提取协议信息
void BD2GPS_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{	
	NMEA_GNGSA_Analysis(bd2gpsx,buf); //GNGSA解析
	NMEA_GNRMC_Analysis(bd2gpsx,buf); //GNRMC解析
	NMEA_GNVTG_Analysis(bd2gpsx,buf); //GNVTG解析
	NMEA_GNTXR_Analysis(bd2gpsx,buf); //GNTXR解析
	NMEA_GNGGA_Analysis(bd2gpsx,buf); //GNGGA解析
	NMEA_GNDHV_Analysis(bd2gpsx,buf); //GNDHV解析		
}

//BD2GPS校验和计算
//buf:数据缓存区首地址
//len:数据长度
//cka,ckb:两个校验结果.
void BD2GPS_CheckSum(u8 *buf,u16 len,u8 *cksum)
{
	u16 i;
	
	*cksum=0;
	for(i=0;i<len;i++)
		*cksum=*cksum+buf[i];
	*cksum = 0xFF-*cksum;
}





