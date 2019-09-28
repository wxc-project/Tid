// TID.h : TID 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error 在包含用于 PCH 的此文件之前包含“stdafx.h” 
#endif
#include "resource.h"       // 主符号
#include "TIDView.h"
#include "ModCore.h"
// CTIDApp:
// 有关此类的实现，请参阅 TID.cpp
//

class CTIDDoc; 
class CTIDApp : public CWinAppEx
{
public:
	BYTE m_ciFileType;		//0.TID_FILE|1.MOD_FILE
	BOOL m_bChildProcess;	//子进程模式
	UINT m_uiActiveHeightSerial;
	UINT m_uiActiveLegSerial[4];
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

extern CTIDApp theApp;
extern IModModel* gpModModel;
#ifdef NEW_VERSION_TIDCORE
#include "TidCplus.h"
extern ITidModel* gpTidModel;
#endif