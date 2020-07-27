// TID.h : TID Ӧ�ó������ͷ�ļ�
//
#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h�� 
#endif
#include "resource.h"       // ������
#include "TIDView.h"
#include "ModCore.h"
#include "HashTable.h"// CTIDApp:
// �йش����ʵ�֣������ TID.cpp
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
	BOOL m_bChildProcess;	//�ӽ���ģʽ
	UINT m_uiActiveHeightSerial;
	UINT m_uiActiveLegSerial[4];
	CHashListEx<HEIGHT_GROUP> hashModelHeights;
public:
	CTIDApp();
// ��д
public:
	virtual BOOL InitInstance();
	virtual BOOL SaveState(LPCTSTR lpszSectionName = NULL, CFrameImpl* pFrameImpl = NULL);
	CTIDDoc* GetTIDDoc();
	CTIDView* GetTIDView();
	void* GetActiveTowerInstance();
// ʵ��
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