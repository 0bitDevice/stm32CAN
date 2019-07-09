/***************************
*�ļ�����akq.c
*�����ߣ�zhangzengwei
*����ʱ�䣺2016-7-20
*�ļ�������
***************************/

#include "akq.h"
#include "usart3.h"
#include "string.h"
#include "math.h"


u8 numbers1=0,numbers2=0,numcnt=0;

extern MSG_SEND akq;
u8 navstate_flag = 0;

u8 bdtxr = 0;   // ָ�������
u8 itxr = 0;    // ָ��ؼ���

u8 CANpackMsNum = 0;		//CANÿ�뷢�͵İ����0~9

//��buf����õ���cx���������ڵ�λ��
//����ֵ:0~0XFE,����������λ�õ�ƫ��.
//       0XFF,�������ڵ�cx������							  
u8 NMEA_Comma_Pos(u8 *buf,u8 cx)
{	 		    
	u8 *p=buf;
	while(cx)
	{		 
		if(*buf<' '||*buf>'z')return 0XFF;//����'*'���߷Ƿ��ַ�,�򲻴��ڵ�cx������
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;	 
}

//m^n����
//����ֵ:m^n�η�.
u32 NMEA_Pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}

///���յ��̱��ģ�������ȡ�̱�����Ϣ
int TQ(u8 *buf)
{
	char tqbuf[128];
	char tqok[128];
	int i=0,j=0,k=0,ok=0;
	u8 *p=buf;
	while(1)
	{
		if((*p!=',') && (*p!='*'))  ///��ȡ�ַ���
		{
			tqbuf[i] = *p;
			p++;
			i++;
		}
		else
			break;
	}
	for(j=0;j<i;j++)    ///��ȡ��������
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
		if(tqok[j]=='0')	///�ж��ǲ���׹����Ϣ
			if(tqok[j+1]=='4')
				if(tqok[j+2]=='2')
					if(tqok[j+3]=='3')
					{
						ok++;
						if(ok>=2)
							return 0;
					}
		if(tqok[j]=='0')   ///�ж��ǲ����Լ���Ϣ
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

//strת��Ϊ����,��','����'*'����
//buf:���ִ洢��
//dx:С����λ��,���ظ����ú���
//����ֵ:ת�������ֵ

u8 floatval;
int NMEA_Str2num(u8 *buf,u8*dx)
{
	u8 *p=buf;
	u32 ires=0,fres=0;
	u8 ilen=0,flen=0,i;
	u8 mask=0;
	int res;
	while(1) //�õ�������С���ĳ���
	{
		if(*p=='-'){mask|=0X02;p++;floatval=1;}//�Ǹ���
		if(*p==','||(*p=='*'))break;//����������
		if(*p=='.'){mask|=0X01;p++;}//����С������
		else if(*p>'9'||(*p<'0'))	//�зǷ��ַ�
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//ȥ������
	for(i=0;i<ilen;i++)	//�õ�������������
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//���ȡ5λС��
	*dx=flen;	 		//С����λ��
	for(i=0;i<flen;i++)	//�õ�С����������
	{  
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	} 
	res=ires*NMEA_Pow(10,flen)+fres;
	//if(mask&0X02)res=-res;		   
	return res;
}

//����GNRMC��Ϣ
void NMEA_GNRMC_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{
	u32 du1,du2;
	double fen1,fen2;
	u8 *p1,dx;			 
	u8 posx;     
	u32 temp;	   
	char navstate;
	u8 ewhemi;											/*������������*/
	u8 nshemi;											/*��γ���Ǳ�γ*/  
	u8 zero = 0;
	
	p1=(u8*)strstr((const char *)buf,"GNRMC");
	if(p1)
	{	
		posx=NMEA_Comma_Pos(p1,1);								//�õ�UTCʱ��
		if(posx!=0XFF)
		{
			temp=NMEA_Str2num(p1+posx,&dx);
			CANpackMsNum = temp % 1000 / 100;	//�����ֽ�1�洢����ת����CAN���͵İ�˳��
			temp /= 1000;
			bd2gpsx->msg_to_akq.ux450_data.hour = temp/10000;
			bd2gpsx->msg_to_akq.ux450_data.minute = (temp/100)%100;
			bd2gpsx->msg_to_akq.ux450_data.second = temp%100;		
		}
			
		posx=NMEA_Comma_Pos(p1,3);								//�õ�γ��
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
		posx=NMEA_Comma_Pos(p1,4);								//��γ���Ǳ�γ ///ͨ�����жϷ��ŵ�����
		if(posx != 0xFF)
		{
			nshemi = *(p1+posx);
			if(nshemi == 'S')
				bd2gpsx->msg_to_akq.ux450_data.latitude = ~(bd2gpsx->msg_to_akq.ux450_data.latitude)+1;
		}		
		posx=NMEA_Comma_Pos(p1,5);								//�õ�����
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
		posx=NMEA_Comma_Pos(p1,6);				//������������   ///ͨ�������жϷ��ŵ�����
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
			posx=NMEA_Comma_Pos(p1,2);								//�õ���λ״̬
			if(posx!=0XFF)
			{
				navstate = *(p1+posx);
				if(navstate == 'A')				//��λ
				{
					if(navstate_flag>0)
						navstate_flag--;				
					else
						bd2gpsx->msg_to_akq.ux450_data.navstate = 0x01;									
				}
				else if(navstate == 'V')				//����λ
				{	
					navstate_flag = 20;
					bd2gpsx->msg_to_akq.ux450_data.navstate = 0x00;
				}
			}
		}		
	}	
}

//����GNZDA��Ϣ
void NMEA_GNZDA_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx;     
	u32 temp;
	
	p1=(u8*)strstr((const char *)buf,"GNZDA");
	if(p1)
	{
		posx=NMEA_Comma_Pos(p1,2);								//�õ�UTCʱ��
		if(posx!=0XFF)
		{
			temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//�õ�UTCʱ��,ȥ��ms
			//����ʱ����ȫΪ0���쳣���
			bd2gpsx->msg_to_akq.ux450_data.hour = temp/10000;
			bd2gpsx->msg_to_akq.ux450_data.minute = (temp/100)%100;
			bd2gpsx->msg_to_akq.ux450_data.second = temp%100;		
		}		
	}	
}


//����GNVTG��Ϣ
void NMEA_GNVTG_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx;
	u32 flightangle,speedground;
	p1=(u8*)strstr((const char *)buf,"$GNVTG");
	if(p1)
	{		
		posx=NMEA_Comma_Pos(p1,1);                   //������
		if(posx!=0XFF)
		{
			flightangle = NMEA_Str2num(p1+posx,&dx);
			bd2gpsx->msg_to_akq.ux450_data.flightangle = flightangle/NMEA_Pow(10,dx-1);
		} 	
		posx=NMEA_Comma_Pos(p1,7);								//����
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

//����GNTXR��Ϣ
void NMEA_GNTXR_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{
	u8 *p1,pd;			 
	u8 posx; 
	p1=(u8*)strstr((const char *)buf,"$BDTXR");
	if(p1)
	{
		posx=NMEA_Comma_Pos(p1,5);								///�õ��̱�������
		if(posx!=0XFF)
		{
			pd=TQ(p1+posx+2);
			if(pd==0)	///׹��   
			{
				bd2gpsx->msg_to_akq.ux450_data.cmd = 0x03;	
				bdtxr=1;
				itxr = 10;
			}
			else if(pd==1)    ///�Լ� 
			{
				bd2gpsx->msg_to_akq.ux450_data.cmd = 0x02;
				bdtxr=2;
				itxr = 10;
			}
		}	
	}	
}

//����GNGSA��Ϣ
void NMEA_GNGSA_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx; 
	u8 i;
	u8 possl;

	p1=(u8*)strstr((const char *)buf,"$GNGSA");
	if(p1)
	{
		for(i=0;i<12;i++)										//�õ���λ���Ǳ��ͨ����ŵõ���bd��gps��Ŀ
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
		
//		akq.msg_to_akq.ux450_data.backup[1] = numcnt;      //������
//		akq.msg_to_akq.ux450_data.backup[2] = numbers1;    //������
//		akq.msg_to_akq.ux450_data.backup[3] = numbers2;    //������
		
		if(numcnt>=2)
		{
			bd2gpsx->msg_to_akq.ux450_data.numbers = (numbers1<<4)+numbers2;
			
		   numcnt = 0;
			 numbers1 = 0;
       numbers2 = 0;				
		}							
	}
}


//����GNGGA��Ϣ
void NMEA_GNGGA_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{ 
	u8 *p1,dx;			 
	u8 posx;
  u32 height,val;	
	p1=(u8*)strstr((const char *)buf,"$GNGGA");
	if(p1)
	{			
		posx=NMEA_Comma_Pos(p1,9);								//�õ��߶�
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

//����GPDHV��Ϣ
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
		posx=NMEA_Comma_Pos(p1,3);	//�õ�X���ٶȣ�km/h��
		if(posx!=0XFF) 
		{			
			Vx = NMEA_Str2num(p1+posx,&dx);
		
			if(floatval==1) 
			{
				Vx = ~Vx+1;
				floatval = 0;
			}			
		}	
		
		posx=NMEA_Comma_Pos(p1,4);	//�õ�Y���ٶȣ�km/h��
		if(posx!=0XFF) 
		{			
			Vy = NMEA_Str2num(p1+posx,&dx);
			
			if(floatval==1) 
			{
				Vy = ~Vy+1;
				floatval = 0;
			}
		}	
		
		posx=NMEA_Comma_Pos(p1,5);	//�õ�Z���ٶȣ�km/h��
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
		lon = lon*pi/180;   // ���ȣ����ȣ�
		lat = (double)bd2gpsx->msg_to_akq.ux450_data.latitude/10000000;  
		lat = lat*pi/180;  // γ�ȣ����ȣ�
				
		speedsky = cos(lat)*cos(lon)*Vx/360+cos(lat)*sin(lon)*Vy/360+sin(lat)*Vz/360;  // ʵ�����ٳ���10����m/s��
				
		bd2gpsx->msg_to_akq.ux450_data.speedsky = speedsky;  // ����		
	}	
}

//��ȡЭ����Ϣ
void BD2GPS_Analysis(MSG_SEND *bd2gpsx,u8 *buf)
{	
	NMEA_GNGSA_Analysis(bd2gpsx,buf); //GNGSA����
	NMEA_GNRMC_Analysis(bd2gpsx,buf); //GNRMC����
	NMEA_GNVTG_Analysis(bd2gpsx,buf); //GNVTG����
	NMEA_GNTXR_Analysis(bd2gpsx,buf); //GNTXR����
	NMEA_GNGGA_Analysis(bd2gpsx,buf); //GNGGA����
	NMEA_GNDHV_Analysis(bd2gpsx,buf); //GNDHV����		
}

//BD2GPSУ��ͼ���
//buf:���ݻ������׵�ַ
//len:���ݳ���
//cka,ckb:����У����.
void BD2GPS_CheckSum(u8 *buf,u16 len,u8 *cksum)
{
	u16 i;
	
	*cksum=0;
	for(i=0;i<len;i++)
		*cksum=*cksum+buf[i];
	*cksum = 0xFF-*cksum;
}





