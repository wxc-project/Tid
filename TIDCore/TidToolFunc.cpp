#include "stdafx.h"
#include "f_ent.h"
#include "SegI.h"
#include "list.h"
#include "TidToolFunc.h"

static DWORD CFG_NO[32]={ 0X00000001,0X00000002,0X00000004,0X00000008,0X00000010,0X00000020,0X00000040,0X00000080,
	0X00000100,0X00000200,0X00000400,0X00000800,0X00001000,0X00002000,0X00004000,0X00008000,
	0X00010000,0X00020000,0X00040000,0X00080000,0X00100000,0X00200000,0X00400000,0X00800000,
	0X01000000,0X02000000,0X04000000,0X08000000,0X10000000,0X20000000,0X40000000,0X80000000};

static DWORD _LocalGetSingleWord(long iNo)
{
	if(iNo<=0)
		return 0;
	else if(iNo<=32)
		return CFG_NO[iNo-1];
	else
		return 0;
}
static BYTE ValidateSchema(BYTE schema)
{
	if(schema==0)
		schema=CFGWORD::MULTILEG_SCHEMA;
	if(schema>=1&&schema<=3)
		return schema;
	else
		return 1;
}
BYTE CFGWORD::MULTILEG_SCHEMA = 1;	//CFGLEG::MULTILEG_MAX08=1
UDF_MULTILEG_SCHEMA CFGWORD::xUdfSchema;
CFGWORD::CFGWORD(int iBodyNo,int iLegNo,BYTE schema/*=0*/)
{
	iBodyNo=min(MaxBodys(schema),iBodyNo);
	iLegNo=min(MaxLegs(schema),iLegNo);
	iBodyNo=max(iBodyNo,1);
	iLegNo=max(iLegNo,1);
	int offset=(iBodyNo-1)*MaxLegs(schema)+iLegNo-1;
	int indexOfBytes=offset/8;
	int odd=offset%8;
	memset(flag.bytes,0,24);
	flag.bytes[indexOfBytes]=0x01;
	flag.bytes[indexOfBytes]<<=odd;
}

BOOL CFGWORD::IsHasNo(int iNo)
{
	iNo-=1;
	if(iNo<0||iNo>=192)
		return FALSE;
	int iWord=iNo/32;
	int iBit=iNo%32;
	if((flag.word[iWord]&CFG_NO[iBit]) > 0)
		return TRUE;
	else
		return FALSE;
}

BOOL CFGWORD::And(CFGWORD wcfg) const
{
	return (flag.word[0]&wcfg.flag.word[0])||(flag.word[1]&wcfg.flag.word[1])||(flag.word[2]&wcfg.flag.word[2])||
		(flag.word[3]&wcfg.flag.word[3])||(flag.word[4]&wcfg.flag.word[4])||(flag.word[5]&wcfg.flag.word[5]);
}

BYTE CFGWORD::SetSchema(BYTE cbMultiLegSchema)
{
	return MULTILEG_SCHEMA = ValidateSchema(cbMultiLegSchema);
}
BYTE CFGWORD::MaxLegs(BYTE schema/*=0*/)	//指定模式支持最多呼高接腿数MULTILEG_DEFAULT=0
{
	if (schema==MULTILEG_UDF)
		return xUdfSchema.MaxLegs();
	switch(schema)
	{
	case 1://MULTILEG_MAX08:
		return 8;
	case 2://MULTILEG_MAX16:
		return 16;
	case 3://ZMULTILEG_MAX24:
		return 24;
	default:
		return 8;
	}
}
BYTE CFGWORD::MaxBodys(BYTE schema/*=0*/)	//指定模式支持最多呼高本体数MULTILEG_DEFAULT=0
{
	if (schema==MULTILEG_UDF)
		return xUdfSchema.cnHeightCount;
	switch(schema)
	{
	case 1://MULTILEG_MAX08:
		return 24;
	case 2://MULTILEG_MAX16:
		return 12;
	case 3://MULTILEG_MAX24:
		return 8;
	default:
		return 24;
	}
}
bool UDF_MULTILEG_SCHEMA::AllocHeightSchema(BYTE* xarrHeightLegCount,int niHeightCount)
{
	if (niHeightCount<1||niHeightCount>24)
		return false;
	cnHeightCount=(BYTE)niHeightCount;
	UDF_MULTILEG_SCHEMA copy=*this;
	int i,niSummLegCount=0;
	for (i=0;i<niHeightCount&&niSummLegCount<192;i++)
	{
		xarrHeights[i].ciStartAddr=niSummLegCount;
		xarrHeights[i].cnBodyLegs=xarrHeightLegCount[i];
		niSummLegCount+=xarrHeightLegCount[i];
	}
	return i==niHeightCount;
}
int UDF_MULTILEG_SCHEMA::MaxLegs() const
{
	int nMaxLegs=1;
	for (BYTE i=0;i<cnHeightCount;i++)
		nMaxLegs=max(nMaxLegs,xarrHeights[i].cnBodyLegs);
	return nMaxLegs;
}
BYTE CFGWORD::MaxLegOfBody(WORD wiBodySerial)
{	//指定呼高序号最多容纳的接腿数
	if (MULTILEG_SCHEMA==MULTILEG_UDF)
	{
		if (wiBodySerial==0)
			return 0;
		if (wiBodySerial<=xUdfSchema.cnHeightCount)
			return xUdfSchema.xarrHeights[wiBodySerial-1].cnBodyLegs;
		else
			return 0;
	}
	else if (MULTILEG_SCHEMA>=MULTILEG_MAX08&&MULTILEG_SCHEMA<=MULTILEG_MAX24)
	{
		switch (MULTILEG_SCHEMA)
		{
		case MULTILEG_MAX08:
			return 8;
		case MULTILEG_MAX16:
			return 16;
		case MULTILEG_MAX24:
			return 24;
		default:
			return 8;
		}
	}
	else
		return 0;
}

BOOL CFGWORD::IsHasBodyNoOnly(int iBodyNo,BYTE schema/*=0*/)	//配材字中是否仅指定字节是有值，即不为0，iByte以1为基数(即索引值+1)
{
	if(iBodyNo<=0||iBodyNo>MaxBodys(schema))
		return FALSE;
	else
	{
		int i,nMaxBodies=MaxBodys(schema),nMaxLegs=MaxLegs(schema);
		schema=ValidateSchema(schema);
		for(i=1;i<=nMaxBodies;i++)
		{
			DWORD legword=0;
			if (schema==MULTILEG_UDF)
			{
				BYTE ciStartAddr=xUdfSchema.xarrHeights[i-1].ciStartAddr;
				BYTE ciBodyLegs=xUdfSchema.xarrHeights[i-1].cnBodyLegs;
				legword=SubDword(ciStartAddr,ciBodyLegs);
			}
			else
				memcpy(&legword,&flag.bytes[(i-1)*schema],schema);
			if(i==iBodyNo&&legword==0)
				return FALSE;
			else if(i!=iBodyNo&&legword>0)
				return FALSE;
		}
		return TRUE;
	}
}
BOOL CFGWORD::IsHasBodyNo(int iBodyNo,BYTE schema/*=0*/)		//配材字中指定字节是否有值，即不为0，iByte以1为基数(即索引值+1)
{
	if(iBodyNo<=0||iBodyNo>MaxBodys(schema))
		return FALSE;
	schema=ValidateSchema(schema);
	DWORD legword=0;
	if (schema==MULTILEG_UDF)
	{
		BYTE ciStartAddr=xUdfSchema.xarrHeights[iBodyNo-1].ciStartAddr;
		BYTE ciBodyLegs=xUdfSchema.xarrHeights[iBodyNo-1].cnBodyLegs;
		legword=SubDword(ciStartAddr,ciBodyLegs);
	}
	else
		memcpy(&legword,&flag.bytes[(iBodyNo-1)*schema],schema);
	if(legword>0)
		return TRUE;
	else
		return FALSE;
}
BOOL CFGWORD::AddBodyLegs(int iBodyNo,DWORD legword/*=0xffffff*/,BYTE schema/*=0*/)
{
	if(iBodyNo<=0||iBodyNo>MaxBodys(schema))
		return FALSE;
	schema=ValidateSchema(schema);
	if(schema==MULTILEG_MAX08)
		legword&=0xff;
	else if(schema==MULTILEG_MAX16)
		legword&=0xffff;
	else if(schema==MULTILEG_MAX24)
		legword&=0xffffff;
	else if (schema==MULTILEG_UDF)
	{
		BYTE cnMaxLegs=MaxLegOfBody(iBodyNo);
		DWORD dwFullFlag=0;
		for(BYTE i=0;i<cnMaxLegs;i++)
		{
			dwFullFlag<<=1;
			dwFullFlag|=0x00000001;
		}
		legword&=dwFullFlag;
		AddBits(legword,xUdfSchema.xarrHeights[iBodyNo-1].ciStartAddr,cnMaxLegs);
		return TRUE;
	}
	BYTE* bytes=(BYTE*)&legword;
	for(int j=0;j<schema;j++)
		flag.bytes[(iBodyNo-1)*schema+j]|=bytes[j];
	if(legword>0)
		return TRUE;
	else
		return FALSE;
}
BOOL CFGWORD::SetBodyLegs(int iBodyNo,DWORD legword/*=0xffffff*/,BYTE schema/*=0*/)
{
	if(iBodyNo<=0||iBodyNo>MaxBodys(schema))
		return FALSE;
	schema=ValidateSchema(schema);
	if(schema==MULTILEG_MAX08)
		legword&=0xff;
	else if(schema==MULTILEG_MAX16)
		legword&=0xffff;
	else if(schema==MULTILEG_MAX24)
		legword&=0xffffff;
	else if (schema==MULTILEG_UDF)
	{
		BYTE cnMaxLegs=MaxLegOfBody(iBodyNo);
		DWORD dwFullFlag=0;
		for(BYTE i=0;i<cnMaxLegs;i++)
		{
			dwFullFlag<<=1;
			dwFullFlag|=0x00000001;
		}
		legword&=dwFullFlag;
		SetBits(legword,xUdfSchema.xarrHeights[iBodyNo-1].ciStartAddr,cnMaxLegs);
		return TRUE;
	}
	memcpy(&flag.bytes[(iBodyNo-1)*schema],&legword,schema);
	if(legword>0)
		return TRUE;
	else
		return FALSE;
}
DWORD CFGWORD::SubDword(UINT uiStartAddr,UINT nBitCount)
{
	DWORD dwSubWord=0;
	UINT index=uiStartAddr;
	long liLeastBitCount=nBitCount;
	BYTE xarrConstMaskBytes[8]={ 0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff };
	int indicator=0;
	while(liLeastBitCount>0)
	{
		UINT uiByteIndex=index/8;
		UINT uiBitIndex =index%8;
		DWORD dwBits=flag.bytes[uiByteIndex];
		if(uiBitIndex>0)
			dwBits>>=uiBitIndex;
		UINT uiCopyBits=min(8-(int)uiBitIndex,liLeastBitCount);	//当前循环拷贝的bit数
		dwBits&=xarrConstMaskBytes[uiCopyBits-1];
		dwBits<<=indicator;
		dwSubWord|=dwBits;
		indicator+=uiCopyBits;
		index+=uiCopyBits;
		liLeastBitCount-=uiCopyBits;
	}
	return dwSubWord;
}
bool CFGWORD::AddBits(DWORD dwBits,UINT uiStartBitAddr,UINT nBitCount)	//uiStartAddr以0为起始地址索引
{
	__int64 ui64Bits=dwBits;
	UINT uiAddrByteIndex=uiStartBitAddr/8;
	UINT uiAddrBitOffset=uiStartBitAddr%8;
	int niBytesCount=uiAddrBitOffset+nBitCount<=32?4:5;
	ui64Bits<<=uiAddrBitOffset;
	BYTE* pcbByte=(BYTE*)&ui64Bits;
	for (int i=0;i<niBytesCount&&i+uiAddrByteIndex<24;i++,pcbByte++)
		flag.bytes[i+uiAddrByteIndex]|=*pcbByte;
	return true;
}
bool CFGWORD::SetBits(DWORD dwBits,UINT uiStartBitAddr,UINT nBitCount)	//uiStartAddr以0为起始地址索引
{
	__int64 ui64Bits=dwBits;
	UINT uiAddrByteIndex=uiStartBitAddr/8;
	UINT uiAddrBitOffset=uiStartBitAddr%8;
	int niBytesCount=uiAddrBitOffset+nBitCount<=32?4:5;
	ui64Bits<<=uiAddrBitOffset;
	BYTE* pcbByte=(BYTE*)&ui64Bits;
	BYTE cbNowHeadByte=flag.bytes[uiAddrByteIndex];
	BYTE cbNowTailByte=flag.bytes[uiAddrByteIndex+niBytesCount-1];
	for (int i=0;i<niBytesCount&&i+uiAddrByteIndex<24;i++,pcbByte++)
		flag.bytes[i+uiAddrByteIndex]=*pcbByte;
	if (uiAddrBitOffset>0)
	{
		BYTE xarrLowBitsMask[8]={ 0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff };
		cbNowHeadByte&=xarrLowBitsMask[uiAddrBitOffset];
	}
	UINT uiAddrBitOfTailByteLeast=(uiAddrBitOffset+nBitCount)%8;
	if(uiAddrBitOfTailByteLeast>0)
	{
		BYTE xarrHighBitsMask[8]={ 0xff,0xf7,0xf3,0xf1,0xf0,0x70,0x30,0x10 };
		cbNowTailByte&=xarrHighBitsMask[uiAddrBitOfTailByteLeast];
	}
	flag.bytes[uiAddrByteIndex]|=cbNowHeadByte;
	flag.bytes[uiAddrByteIndex+niBytesCount-1]|=cbNowTailByte;
	return false;
}
CFGWORD CFGWORD::SetWordByNo(int iNo)			//根据指定的iNo号位指定配材字
{
	iNo--;
	Clear();
	if(iNo>=0&&iNo<192)
	{
		int iWord=iNo/32;
		int iBit=iNo%32;
		flag.word[iWord]=_LocalGetSingleWord(iBit+1);//CFG_NO[iBit];
	}
	return *this;
}
CFGWORD CFGWORD::AddSpecWord(CFGWORD cfgword)	//相当于两个配材字进行或操作,并将结果赋值给当前配材字
{
	for(int i=0;i<6;i++)
		flag.word[i]=flag.word[i] | cfgword.flag.word[i];
	return *this;
}
//////////////////////////////////////////////////////////////////////////
//将字符串型的版本号转换为一个长整数，如"1.01.3.21"-->1 01 03 21
long FromStringVersion(const char* version)
{
	char local_version[20]="";
	strncpy(local_version,version,20);
	char *key=strtok(local_version," .,-");
	UINT version_item[4]={0};
	int n,i=0;
	while(key!=NULL&&i<4)
	{
		n=(int)strlen(key);
		if(n==1)
			version_item[i]=key[0]-'0';
		else if(n>1)
			version_item[i]=(key[0]-'0')*10+key[1]-'0';
		key=strtok(NULL,".,-");
		i++;
	}
	long nVersion=version_item[0]*1000000+version_item[1]*10000+version_item[2]*100+version_item[3];
	//long nVersion=version_item[0]*lic.ConstE6()+version_item[1]*lic.ConstE4()+version_item[2]*lic.ConstE2()+version_item[3];
	return nVersion;
}
//大于零表示版本号1高,等于零表示同版本，否则表示版本号1低
int compareVersion(const char* version1,const char* version2)
{
	if(version1==NULL)
		return 1;
	else if(version2==NULL)
		return -1;
	long nVersion1=FromStringVersion(version1);
	long nVersion2=FromStringVersion(version2);
	if(nVersion1>nVersion2)
		return 1;
	else if(nVersion1<nVersion2)
		return -1;
	else
		return 0;
}
//////////////////////////////////////////////////////////////////////////
static CXhSimpleList<SEGI> SegIArr;
SEGI FindAdaptNoForSegI(const char *limit_str,const char *delimiter1/*=NULL*/,const char *delimiter2/*=NULL*/)
{
	char *limstr=NULL;
	if(limit_str)
	{
		limstr=new char[strlen(limit_str)+1];
		strcpy(limstr,limit_str);
	}
	SEGI iNo;
	if(limit_str)
	{
		SegIArr.DeleteList();
		char *sKey;
		if(delimiter1)
			sKey=strtok(limstr,delimiter1);
		else
			sKey=strtok(limstr,",\n");
		while(sKey)
		{
			char *delimiter;
			if(delimiter2)
				delimiter=strchr(sKey,*delimiter2);
			else
				delimiter=strchr(sKey,'-');
			if(delimiter)
			{		
				*delimiter=' ';
				SEGI start_i,end_i;
				char start_str[16]="",end_str[16]="";
				sscanf(sKey,"%s%s",&start_str,&end_str);
				start_i=SEGI(start_str);
				end_i=SEGI(end_str);
				if(end_i.iSeg<start_i.iSeg)
					end_i=start_i;
				for(iNo.iSeg=start_i.iSeg;iNo.iSeg<=end_i.iSeg;iNo.iSeg++)
					SegIArr.AttachObject(iNo);
			}
			else
			{
				iNo=SEGI(sKey);
				SegIArr.AttachObject(iNo);
			}
			sKey=strtok(NULL,",\n");
		}
		SEGI *pNo=SegIArr.EnumObjectFirst();
		if(pNo)
			iNo=*pNo;
		else
			iNo=SEGI();
	}
	else
	{
		SEGI *pNo=SegIArr.EnumObjectNext();
		if(pNo)
			iNo=*pNo;
		else
			iNo=SEGI();
	}
	if(limstr)
		delete []limstr;
	return iNo;
}
//通过段号字符串得到段号哈希表
DWORD GetSegNoHashTblBySegStr(const char* sSegStr,CHashList<SEGI> &segNoHashTbl)
{
	char seg_str[200]="";
	_snprintf(seg_str,199,"%s",sSegStr);
	if(seg_str[0]=='*')
		segNoHashTbl.Empty();
	else
	{
		for(SEGI seg_i=FindAdaptNoForSegI(seg_str,",","-");SegIArr.Current!=NULL;seg_i=FindAdaptNoForSegI(NULL,",","-"))
			segNoHashTbl.SetValue(seg_i.iSeg,seg_i);
	}
	return segNoHashTbl.GetNodeNum();
}
//根据段号哈希表获取有序的段号链表
int GetSortedSegNoList(const char* sSegStr,CXhSimpleList<SEGI> &listSegI)
{
	CHashList<SEGI> segNoHashList;
	GetSegNoHashTblBySegStr(sSegStr,segNoHashList);
	//选择排序分段号
	int n=segNoHashList.GetNodeNum();
	SEGI *seg_i_arr = new SEGI[n];
	int i=0;
	for(SEGI *pSegI=segNoHashList.GetFirst();pSegI;pSegI=segNoHashList.GetNext())
	{
		if(i>0)
		{
			int j;
			for(j=0;j<i;j++)
			{
				if(*pSegI<seg_i_arr[j])
				{
					memmove(&seg_i_arr[j+1],&seg_i_arr[j],(n-j-1)*sizeof(int));
					seg_i_arr[j]=*pSegI;
					break;
				}
			}
			if(j==i)
				seg_i_arr[i]=*pSegI;
		}
		else
			seg_i_arr[i]=*pSegI;
		i++;
	}
	listSegI.DeleteList();
	for(int i=0;i<n;i++)
		listSegI.AttachObject(seg_i_arr[i]);
	delete []seg_i_arr;
	return n;
}
//////////////////////////////////////////////////////////////////////////
//矢量方向的坐标系转换(与坐标原点无关)
//TRUE：UCS-->WCS; FALSE：WCS-->UCS
#ifndef APP_EMBEDDED_MODULE
BOOL vector_trans( double* vcoord, UCS_STRU ucs, BOOL fromUcs,BOOL bSkipStandardize/*=TRUE*/)
{
    f3dPoint pnt;
    if(!fromUcs)//----WCS-->UCS对齐坐标原点
    {
	    pnt=f3dPoint(vcoord);
    	vcoord[0] = pnt*ucs.axis_x;
    	vcoord[1] = pnt*ucs.axis_y;
    	vcoord[2] = pnt*ucs.axis_z;
    }
    else
    {
	    pnt=f3dPoint(vcoord);
    	vcoord[0] = pnt.x*ucs.axis_x.x+pnt.y*ucs.axis_y.x+pnt.z*ucs.axis_z.x;
    	vcoord[1] = pnt.x*ucs.axis_x.y+pnt.y*ucs.axis_y.y+pnt.z*ucs.axis_z.y;
    	vcoord[2] = pnt.x*ucs.axis_x.z+pnt.y*ucs.axis_y.z+pnt.z*ucs.axis_z.z;
    }
    return TRUE;
}
//坐标系转换 TRUE：UCS-->WCS; FALSE：WCS-->UCS
BOOL coord_trans(double* ptcoord,UCS_STRU ucs,BOOL fromUcs,BOOL bSkipStandardize/*=TRUE*/)
{
    f3dPoint pnt;
    if(!fromUcs)//----WCS-->UCS对齐坐标原点
    {
	    Sub_Pnt(pnt, f3dPoint(ptcoord), ucs.origin);
    	ptcoord[0] = pnt*ucs.axis_x;
    	ptcoord[1] = pnt*ucs.axis_y;
    	ptcoord[2] = pnt*ucs.axis_z;
    }
    else
    {
	    Cpy_Pnt(pnt,f3dPoint(ptcoord));
    	ptcoord[0] = pnt.x*ucs.axis_x.x+pnt.y*ucs.axis_y.x+pnt.z*ucs.axis_z.x;
    	ptcoord[1] = pnt.x*ucs.axis_x.y+pnt.y*ucs.axis_y.y+pnt.z*ucs.axis_z.y;
    	ptcoord[2] = pnt.x*ucs.axis_x.z+pnt.y*ucs.axis_y.z+pnt.z*ucs.axis_z.z;
		ptcoord[0]+=ucs.origin.x;
		ptcoord[1]+=ucs.origin.y;
		ptcoord[2]+=ucs.origin.z;
    }
    return TRUE;
}
#endif
BOOL normalize(double v[3])
{
    double len = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    if (len < EPS)
        return FALSE;

    v[0] /= len;
    v[1] /= len;
    v[2] /= len;

    return TRUE;
}
