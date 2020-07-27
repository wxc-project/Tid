// TID.h : TID 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error 在包含用于 PCH 的此文件之前包含“stdafx.h” 
#endif
#include "resource.h"       // 主符号
#include "TIDView.h"
#include "ModCore.h"
#include "HashTable.h"// CTIDApp:
// 有关此类的实现，请参阅 TID.cpp
//

class CTIDDoc; 
class CMaxDouble{
	bool inited;
public:
	double number;
	void* m_pRelaObj;
	CMaxDouble(){inited=false;m_pRelaObj=NULL;}
	CMaxDouble(double init_val,void* pRelaObj=NULL){inited=true;number=init_val;m_pRelaObj=pRelaObj;}
	operator double(){return number;}
	bool IsInited() const{return inited;}
	double Update(double val,void* pRelaObj=NULL,double tolerance=0)
	{
		if(!inited)
		{
			number=val;
			inited=true;
			m_pRelaObj=pRelaObj;
		}
		else if(number<val-tolerance)
		{
			number=val;
			m_pRelaObj=pRelaObj;
		}
		return number;
	}
};
struct HEIGHT_GROUP {
	ITidHeightGroup* m_pModule;
	UINT m_uiActiveHeightSerial;
	UINT m_uiActiveLegSerial[4];
	CMaxDouble lowestZ;
	HEIGHT_GROUP(){
		m_pModule=NULL;
		m_uiActiveHeightSerial=1;
		m_uiActiveLegSerial[0]= m_uiActiveLegSerial[1]= m_uiActiveLegSerial[2]= m_uiActiveLegSerial[3]= 1;
	}
	void SetKey(DWORD key) { m_uiActiveHeightSerial=key; }
};

class CTIDApp : public CWinAppEx
{
public:
	BYTE m_ciFileType;		//0.TID_FILE|1.MOD_FILE
	BOOL m_bChildProcess;	//子进程模式
	UINT m_uiActiveHeightSerial;
	UINT m_uiActiveLegSerial[4];
	CHashListEx<HEIGHT_GROUP> hashModelHeights;
public:
	CTIDApp();
// 重写
public:
	virtual BOOL InitInstance();
	virtual BOOL SaveState(LPCTSTR lpszSectionName = NULL, CFrameImpl* pFrameImpl = NULL);
	CTIDDoc* GetTIDDoc();
	CTIDView* GetTIDView();
	void* GetActiveTowerInstance();
// 实现
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

bool TestBitState(BYTE xarrLegCfgBytes[24],int niBitSerial);

extern CTIDApp theApp;
extern IModModel* gpModModel;
#ifdef NEW_VERSION_TIDCORE
#include "TidCplus.h"
extern ITidModel* gpTidModel;
#endif