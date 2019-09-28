#include "StdAfx.h"
#include "SegI.h"

//根据件号提取段号及构件序列号
static bool IsCharInStr(char ch,char* prefix)
{
	if(prefix==NULL)
		return false;
	while(*prefix!='\0'&& ch!=*prefix)
	{
		prefix++;
		continue;
	}
	return (ch==*prefix);
}
bool ParsePartNo(const char* sPartNo,SEGI* pSegI,char* sSerialNo,char* materialPrefix/*=NULL*/,char *sSeparator/*=NULL*/)
{
	int i;
	int str_len = strlen(sPartNo);
	if(str_len<=0)
		return false;
	int separator = -1;
	SEGI segi;
	if(pSegI==NULL)
		pSegI=&segi;
	pSegI->key.number=0;
	pSegI->key.prefix[0]=pSegI->key.prefix[1]=0;
	int iDigitStart=0;	//构件编号中首个数字的出现位置索引
	int iDigitFinal=0;
	if(sSerialNo)
		strcpy(sSerialNo,"");
	for(i=0;i<str_len;i++)
	{
		BOOL bDigit=isdigit((BYTE)sPartNo[i]);	//使用isdigit时传入的数据需转换为BYTE，为负数时会抛出异常 wht 14-09-05
		if(bDigit)
			iDigitFinal=i;
		if(i==0&&!bDigit)
		{
			pSegI->key.prefix[0]=sPartNo[0];
			iDigitStart=1;
		}
		else if(iDigitStart==1&&i==1&&!bDigit&&sPartNo[i]!='-'&&sPartNo[i]!='_')
		{	//两个前缀
			memcpy(pSegI->key.prefix,sPartNo,2);
			iDigitStart=2;
		}
		else if(separator<0&&(sPartNo[i]=='_' || sPartNo[i]=='-'))
		{
			separator=i;
			if(sSeparator)
				sSeparator[0]=sPartNo[i];
		}
	}
	if(IsCharInStr(pSegI->key.prefix[0],materialPrefix))
		pSegI->key.prefix[0]=pSegI->key.prefix[1];
	if(separator>0)
	{	//有段号流水号间分隔符
		char segStr[16]="";
		//分隔符前的字符串作为段号，考虑首字母为材质字符的情况 wht 17-07-10
		if(IsCharInStr(sPartNo[0],materialPrefix))
			memcpy(segStr,&sPartNo[1],separator-1);
		else
			memcpy(segStr,sPartNo,separator);
		*pSegI=SEGI(segStr);
		if(sSerialNo)
		{
			if(separator+1<str_len)
			{
				memcpy(sSerialNo,&sPartNo[separator+1],str_len-separator);
				sSerialNo[str_len-separator]=0;
				int len=strlen(sSerialNo);
				if(len>1&&IsCharInStr(sSerialNo[len-1],materialPrefix))
					sSerialNo[len-1]=0;
			}
			else
				return false;
		}
	}
	else
	{	//无段号流水号间分隔符，默认取后两位连续数字及后缀作为流水号
		char segStr[8]="";
		int len=min(8,iDigitFinal-1-iDigitStart);
		if(len>0)
		{	//分隔符前的字符串作为段号，考虑首字母为材质字符的情况 wht 17-07-10
			if(IsCharInStr(sPartNo[0],materialPrefix))
				memcpy(segStr,&sPartNo[1],min(7,len+iDigitStart-1));
			else
				memcpy(segStr,sPartNo,min(7,len+iDigitStart));
			segStr[7]=0;
			*pSegI=SEGI(segStr);
		}
		else if(len<0)	
		{	//针对FL7A这样的编号(流水号中数字少于2)提取
			if(sSerialNo)
			{
				memcpy(sSerialNo,&sPartNo[iDigitStart],str_len-iDigitStart);
				sSerialNo[str_len-iDigitStart]=0;
				len=strlen(sSerialNo);
				if(len>1&&IsCharInStr(sSerialNo[len-1],materialPrefix))
					sSerialNo[len-1]=0;
			}
			return true;
		}
		if(sSerialNo)
		{
			memcpy(sSerialNo,&sPartNo[iDigitFinal-1],str_len-iDigitFinal+1);
			sSerialNo[str_len-iDigitFinal+1]=0;
			len=strlen(sSerialNo);
			if(len>1&&IsCharInStr(sSerialNo[len-1],materialPrefix))
				sSerialNo[len-1]=0;
		}
	}
	return TRUE;
}

/*#ifndef _TEST_SEGI_INC__
#define _TEST_SEGI_INC__
#include "LogFile.h"
void Test_SEGI()
{
	CXhChar16 segArr[21]={  CXhChar16("A1"),CXhChar16("A100"),CXhChar16("A1111"),
							CXhChar16("1B"),CXhChar16("100B"),CXhChar16("1111B"),
							CXhChar16("A1B"),CXhChar16("A100B"),CXhChar16("A1111B"),
							CXhChar16("AB1"),CXhChar16("AB100"),CXhChar16("AB1111"),
							CXhChar16("1AB"),CXhChar16("100AB"),CXhChar16("1111AB"),
							CXhChar16("AB1A"),CXhChar16("AB100A"),CXhChar16("AB11A"),
							CXhChar16("ABC1"),CXhChar16("1ABC"),CXhChar16("ABC1ABC")};
	for(int i=0;i<21;i++)
	{
		SEGI segI(segArr[i]);
		logerr.Log("%s,\tprefix:%s,\tserial:%d,\tsuffix:%s",
			(char*)segArr[i],(char*)segI.Prefix(),
			segI.Digital(),(char*)segI.Suffix());
	}
	logerr.ShowToScreen();
}
void Test_ParsePartNo()
{
	CXhChar16 partNoArr[43]={   CXhChar16("A101"),CXhChar16("A102A"),CXhChar16("A103AH"),CXhChar16("HA103A"),
								CXhChar16("AB101"),CXhChar16("AB102A"),CXhChar16("AB103AH"),CXhChar16("HAB103A"),
								CXhChar16("1B01"),CXhChar16("1B02B"),CXhChar16("1B03BH"),CXhChar16("H1B03B"),
								CXhChar16("1AB01"),CXhChar16("1AB02B"),CXhChar16("1AB03BH"),CXhChar16("H1AB03B"),
								CXhChar16("A1C01"),CXhChar16("A1C02C"),CXhChar16("A1C03CH"),CXhChar16("HA1C03C"),
								CXhChar16("7A-101"),CXhChar16("7A-101B"),CXhChar16("7A-101BH"),CXhChar16("H7A-101B"),
								CXhChar16("7AB-101"),CXhChar16("7AB-101B"),CXhChar16("7AB-101BH"),CXhChar16("H7AB-101B"),
								CXhChar16("A7-101"),CXhChar16("A7-101B"),CXhChar16("A7-101BH"),CXhChar16("HA7-101B"),
								CXhChar16("AB7-101"),CXhChar16("AB7-101B"),CXhChar16("AB7-101BH"),CXhChar16("HAB7-101B"),
								CXhChar16("A7B-101"),CXhChar16("A7B-101B"),CXhChar16("A7B-101BH"),CXhChar16("HA7B-101B"),
								CXhChar16("FL7A"),CXhChar16("FL7AH"),CXhChar16("7AFL")};
	CXhChar16 resultArr[43][4]={"A1","01","","",
								"A1","02A","","",
								"A1","03A","","",
								"A1","03A","","",
								"AB1","01","","",
								"AB1","02A","","",
								"AB1","03A","","",
								"AB1","03A","","",
								"1B","01","","",
								"1B","02B","","",
								"1B","03B","","",
								"1B","03B","","",
								"1AB","01","","",
								"1AB","02B","","",
								"1AB","03B","","",
								"1AB","03B","","",
								"A1C","01","","",
								"A1C","02C","","",
								"A1C","03C","","",
								"A1C","03C","","",
								"7A","101","","-",
								"7A","101B","","-",
								"7A","101B","","-",
								"7A","101B","","-",
								"7AB","101","","-",
								"7AB","101B","","-",
								"7AB","101B","","-",
								"7AB","101B","","-",
								"A7","101","","-",
								"A7","101B","","-",
								"A7","101B","","-",
								"A7","101B","","-",
								"AB7","101","","-",
								"AB7","101B","","-",
								"AB7","101B","","-",
								"AB7","101B","","-",
								"A7B","101","","-",
								"A7B","101B","","-",
								"A7B","101B","","-",
								"A7B","101B","","-",
								"FL","7A","","",
								"FL","7A","","",
								"7A","FL","","",};
	for(int i=0;i<43;i++)
	{
		SEGI segI;
		CXhChar16 sSerialNo,sMaterial="H",sSeparator;
		ParsePartNo(partNoArr[i],&segI,sSerialNo,sMaterial,sSeparator);
		if( resultArr[i][0].EqualNoCase(segI.ToString())&&
			resultArr[i][1].EqualNoCase(sSerialNo)&&
			resultArr[i][3].EqualNoCase(sSeparator))
			continue;
		logerr.Log("%s,\t\t段号:%s,\t序号:%s,\t分隔符:%s",
			(char*)partNoArr[i],(char*)segI.ToString(),(char*)sSerialNo,(char*)sSeparator);
	}
	logerr.ShowToScreen();
}
#endif*/