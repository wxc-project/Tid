#pragma once
#include <stdlib.h>
#include "XhCharString.h"
//复合段号类，段号组成允许含两个前缀字符，段号数字部分允许最大为65536
union SEGI
{
	long iSeg;
	struct{
		WORD number;
		char prefix[2];
	}key;
	SEGI(long seg_i=0){iSeg=seg_i;}
	SEGI(const char* segStr)
	{
		iSeg=0;
		if(segStr==NULL||strlen(segStr)==0)
			return;
		int prefix_n=0,suffix_n=0,serial_n=0;
		//段号序号超过5位时会死机(由于件号最多为16位,serial_str长度由5改为16) wht 17-07-11
		char prefix[2]="",suffix[2]="",serial_str[16]="";
		for(int i=0;i<(int)strlen(segStr);i++)
		{
			if(segStr[i]<0)
				continue;
			if(isdigit(segStr[i]))
			{	//数字部分
				if(serial_n<16)
				{
					serial_str[serial_n]=segStr[i];
					serial_n++;
				}
				else
					return;
			}
			else if(serial_n==0)
			{	//前缀
				if(prefix_n<2)
					prefix[prefix_n++]=segStr[i];
				else //最多允许2个前缀字符
					return;
			}
			else if(serial_n>0)
			{	//后缀
				if(suffix_n<2)
					suffix[suffix_n++]=segStr[i];
				else //最多允许2个后缀字符
					return;
			}
		}
		//number前两位值表示前后缀类型，00：表示prefix[0,1]均为前缀；01:表示prefix[0]为前缀，prefix[1]为后缀；11表示prefix[0,1]为后缀
		WORD serial=0;
		if(serial_n>0)
			serial=(WORD)atoi(serial_str);
		if(prefix_n==0&&suffix_n==0)		//无前后缀
			key.number=serial;
		else if(prefix_n==1&&suffix_n==1)	//一个前缀一个后缀
		{
			if(prefix_n==1)
				key.prefix[0]=prefix[0];
			if(suffix_n==1)
				key.prefix[1]=suffix[0];
			key.number=0X3FFF&serial;
			key.number|=0x4000;
		}
		else if(prefix_n==0&&(suffix_n==2||suffix_n==1))	//两个后缀或一个后缀
		{
			key.prefix[0]=suffix[0];
			key.prefix[1]=suffix[1];
			key.number=0X3FFF&serial;
			key.number|=0xC000;
		}
		else //if(suffix_n==0&&(prefix_n==2||prefix_n==1))//两个前缀或一个前缀
		{
			key.prefix[0]=prefix[0];
			key.prefix[1]=prefix[1];
			key.number=0X3FFF&serial;
		}
		/*
		int prefix_n=0;
		for(int i=0;i<2&&i<(int)strlen(segStr);i++)
		{
			if(segStr[i]<0||segStr[i]>255||isdigit(segStr[i])!=0)	//字符
				continue;
			key.prefix[i]=segStr[i];
			if(i==1)
				key.prefix[0]=segStr[0];
			prefix_n=i+1;
		}
		WORD serial=(WORD)atoi(&segStr[prefix_n]);
		//number前两位值表示前后缀类型，00：表示prefix[0,1]均为前缀；01:表示prefix[0]为前缀，prefix[1]为后缀；11表示prefix[0,1]为后缀
		key.number=0X3FFF&serial;*/
	}
	SEGI(char* prefix,WORD seg_i)
	{
		key.number=seg_i;
		key.prefix[0]=key.prefix[1]=0;
		if(prefix)
		{
			key.prefix[0]=prefix[0];
			if(strlen(prefix)>=2)
				key.prefix[1]=prefix[1];
		}
	}
	operator long()const{return iSeg;}
	CXhChar16 Prefix()const
	{
		char buf[3]="";
		BYTE flag=(key.number&0XC000)>>14;
		if(flag==0)		//key.prefix[0,1]为前缀
			memcpy(buf,key.prefix,2);
		else if(flag==1)//key.prefix[0]为前缀
			buf[0]=key.prefix[0];
		return CXhChar16(buf);
	}
	CXhChar16 Suffix()const
	{
		char buf[3]="";
		BYTE flag=(key.number&0XC000)>>14;
		if(flag==3)		//key.prefix[0,1]为后缀
			memcpy(buf,key.prefix,2);
		else if(flag==1)	//key.prefix[1]为后缀
			buf[0]=key.prefix[1];
		return CXhChar16(buf);
	}
	WORD Digital ()const{return key.number&0X3FFF;}
	CXhChar16 ToString()const
	{
		char prefix[3]="",suffix[3]="";
		BYTE flag=(key.number&0XC000)>>14;
		if(flag==0)		//key.prefix[0,1]为前缀
			memcpy(prefix,key.prefix,2);
		else if(flag==3)//key.prefix[0,1]为后缀
			memcpy(suffix,key.prefix,2);
		else //if(flag==1)//key.prefix[0]为前缀, key.prefix[1]为后缀
		{
			prefix[0]=key.prefix[0];
			suffix[0]=key.prefix[1];
		}
		if(key.number>=0)
			return CXhChar16("%s%d%s",prefix,Digital(),suffix);
		else
			return CXhChar16("%s%s",prefix,suffix);
	}
};

//根据件号提取段号及构件序列号
bool ParsePartNo(const char* sPartNo,SEGI* pSegI,char* sSerialNo,char* materialPrefix=NULL,char *sSeparator=NULL);