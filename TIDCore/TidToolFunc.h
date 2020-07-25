#pragma once
#include "SegI.h"
#include "list.h"
#include "HashTable.h"

struct UDF_HEIGHTLEGS { 
	BYTE ciStartAddr;	//���߽��Ⱥ���ʼ����λ��ַ���(0Ϊ������ַ��)
	BYTE cnBodyLegs;	//�ú��߶�Ӧ�Ľ�����
};
struct UDF_MULTILEG_SCHEMA {
	BYTE cnHeightCount;
	UDF_HEIGHTLEGS xarrHeights[24];	//���δ洢1�ź���;2�ź���......
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
	BOOL And(CFGWORD wcfg) const;	//�൱���á�&�����ж��Ƿ��н���
	BOOL IsHasNo(int iNo);			//��������Ƿ���ָ����iNo��λ
	BOOL IsHasBodyNoOnly(int iBodyNo,BYTE schema=0);	//��������Ƿ��ָ���ֽ�����ֵ������Ϊ0��iByte��1Ϊ����(������ֵ+1)
	BOOL IsHasBodyNo(int iBodyNo,BYTE schema=0);		//�������ָ���ֽ��Ƿ���ֵ������Ϊ0��iByte��1Ϊ����(������ֵ+1)
	BOOL AddBodyLegs(int iBodyNo,DWORD legword=0xffffff,BYTE schema=0);
	BOOL SetBodyLegs(int iBodyNo,DWORD legword=0xffffff,BYTE schema=0);
	CFGWORD SetWordByNo(int iNo);			//����ָ����iNo��λָ�������
	CFGWORD AddSpecWord(CFGWORD cfgword);	//�൱����������ֽ��л����,���������ֵ����ǰ�����
	DWORD SubDword(UINT uiStartAddr,UINT nBitCount);	//uiStartAddr��0Ϊ��ʼ��ַ����
	bool AddBits(DWORD dwBits,UINT uiBitStartAddr,UINT nBitCount);	//uiStartAddr��0Ϊ��ʼ��ַ����
	bool SetBits(DWORD dwBits,UINT uiBitStartAddr,UINT nBitCount);	//uiStartAddr��0Ϊ��ʼ��ַ����
public:
	static BYTE MULTILEG_SCHEMA;	//��ǰĬ�ϵĺ��߽���ռλ����ģʽ
	static UDF_MULTILEG_SCHEMA xUdfSchema;
	static const BYTE MULTILEG_DEFAULT	= 0;
	static const BYTE MULTILEG_MAX08	= 1;
	static const BYTE MULTILEG_MAX16	= 2;
	static const BYTE MULTILEG_MAX24	= 3;
	static const BYTE MULTILEG_UDF		= 4;	//�Զ���(��������߽�������ͬ�� wjh-2020.1.4
	static BYTE SetSchema(BYTE cbMultiLegSchema);
	static BYTE MaxLegs(BYTE schema=0);	//ָ��ģʽ֧�������߽�����MULTILEG_DEFAULT=0
	static BYTE MaxBodys(BYTE schema=0);//ָ��ģʽ֧�������߱�����MULTILEG_DEFAULT=0
	static BYTE MaxLegOfBody(WORD wiBodySerial);		//ָ���������������ɵĽ�����
};

long FromStringVersion(const char* version);
int compareVersion(const char* version1,const char* version2);
int GetSortedSegNoList(const char* sSegStr,CXhSimpleList<SEGI> &listSegI);
BOOL vector_trans( double* vcoord, UCS_STRU ucs, BOOL fromUcs,BOOL bSkipStandardize=TRUE);
BOOL coord_trans(double* ptcoord,UCS_STRU ucs,BOOL fromUcs,BOOL bSkipStandardize=TRUE);
