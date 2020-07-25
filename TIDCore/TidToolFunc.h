#pragma once
#include "SegI.h"
#include "list.h"
#include "HashTable.h"

struct UDF_HEIGHTLEGS { 
	BYTE ciStartAddr;	//呼高接腿号起始索引位地址序号(0为索引地址号)
	BYTE cnBodyLegs;	//该呼高对应的接腿数
};
struct UDF_MULTILEG_SCHEMA {
	BYTE cnHeightCount;
	UDF_HEIGHTLEGS xarrHeights[24];	//依次存储1号呼高;2号呼高......
	bool AllocHeightSchema(BYTE* xarrHeightLegCount,int niHeightCount);
	int MaxLegs() const;
	UDF_MULTILEG_SCHEMA() { cnHeightCount=0;memset(xarrHeights,0,sizeof(UDF_HEIGHTLEGS)*24); }
};
struct CFGWORD{
public:
	union{
		DWORD word[6];
		BYTE bytes[24];
	}flag;
	CFGWORD(){memset(&flag,0,24);}
	CFGWORD(int iNo){SetWordByNo(iNo);}
	CFGWORD(int iBodyNo,int iLegNo,BYTE schema=0);
	void Clear(){memset(&flag,0,24);}
	BOOL And(CFGWORD wcfg) const;	//相当于用‘&’号判断是否有交集
	BOOL IsHasNo(int iNo);			//配材字中是否含有指定的iNo号位
	BOOL IsHasBodyNoOnly(int iBodyNo,BYTE schema=0);	//配材字中是否仅指定字节是有值，即不为0，iByte以1为基数(即索引值+1)
	BOOL IsHasBodyNo(int iBodyNo,BYTE schema=0);		//配材字中指定字节是否有值，即不为0，iByte以1为基数(即索引值+1)
	BOOL AddBodyLegs(int iBodyNo,DWORD legword=0xffffff,BYTE schema=0);
	BOOL SetBodyLegs(int iBodyNo,DWORD legword=0xffffff,BYTE schema=0);
	CFGWORD SetWordByNo(int iNo);			//根据指定的iNo号位指定配材字
	CFGWORD AddSpecWord(CFGWORD cfgword);	//相当于两个配材字进行或操作,并将结果赋值给当前配材字
	DWORD SubDword(UINT uiStartAddr,UINT nBitCount);	//uiStartAddr以0为起始地址索引
	bool AddBits(DWORD dwBits,UINT uiBitStartAddr,UINT nBitCount);	//uiStartAddr以0为起始地址索引
	bool SetBits(DWORD dwBits,UINT uiBitStartAddr,UINT nBitCount);	//uiStartAddr以0为起始地址索引
public:
	static BYTE MULTILEG_SCHEMA;	//当前默认的呼高接腿占位分配模式
	static UDF_MULTILEG_SCHEMA xUdfSchema;
	static const BYTE MULTILEG_DEFAULT	= 0;
	static const BYTE MULTILEG_MAX08	= 1;
	static const BYTE MULTILEG_MAX16	= 2;
	static const BYTE MULTILEG_MAX24	= 3;
	static const BYTE MULTILEG_UDF		= 4;	//自定义(允许各呼高接腿数不同） wjh-2020.1.4
	static BYTE SetSchema(BYTE cbMultiLegSchema);
	static BYTE MaxLegs(BYTE schema=0);	//指定模式支持最多呼高接腿数MULTILEG_DEFAULT=0
	static BYTE MaxBodys(BYTE schema=0);//指定模式支持最多呼高本体数MULTILEG_DEFAULT=0
	static BYTE MaxLegOfBody(WORD wiBodySerial);		//指定呼高序号最多容纳的接腿数
};

long FromStringVersion(const char* version);
int compareVersion(const char* version1,const char* version2);
int GetSortedSegNoList(const char* sSegStr,CXhSimpleList<SEGI> &listSegI);
BOOL vector_trans( double* vcoord, UCS_STRU ucs, BOOL fromUcs,BOOL bSkipStandardize=TRUE);
BOOL coord_trans(double* ptcoord,UCS_STRU ucs,BOOL fromUcs,BOOL bSkipStandardize=TRUE);
