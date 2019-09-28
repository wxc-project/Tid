// MainFrm.cpp : CMainFrame 类的实现
//

#include "stdafx.h"
#include "TID.h"
#include "MainFrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 状态行指示器
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame 构造/析构

CMainFrame::CMainFrame()
{
	m_towerPropertyView.Init(RUNTIME_CLASS(CTowerPropertyDlg), IDD_TOWER_PROPERTY_DLG);
	m_towerTreeView.Init(RUNTIME_CLASS(CTowerTreeDlg), IDD_MODEL_TREE_DLG);
}

CMainFrame::~CMainFrame()
{
}

static BOOL CreateDockingWindow(CWnd *pParentWnd,UINT nDlgID,UINT nViewNameID,CDialogPanel &dlgPanel,DWORD dwPosStyle)
{
	CString sViewName="";
	BOOL bNameValid = sViewName.LoadString(nViewNameID);
	ASSERT(bNameValid);
	if (!dlgPanel.Create(sViewName, pParentWnd, CRect(0, 0, 200, 200), TRUE, nDlgID,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwPosStyle | CBRS_FLOAT_MULTI))
	{
#ifdef AFX_TARG_ENU_ENGLISH
		TRACE0("fail to creat“"+sViewName+"”window\n");
#else
		TRACE0("未能创建“"+sViewName+"”窗口\n");
#endif
		return FALSE;
	}
	return TRUE;
}

#include "afxvisualmanagerwindows7.h"
void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	/*theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:*/
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
	/*	break;

	default:
		switch (theApp.m_nAppLoo
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}*/

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	//theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	// 基于持久值设置视觉管理器和样式
	OnApplicationLook(0);//theApp.m_nAppLook);
	
	if (!m_wndToolBar.CreateEx (this, TBSTYLE_TRANSPARENT) ||!m_wndToolBar.LoadToolBar (IDR_MAINFRAME))
	{
		TRACE0("未能创建工具栏\n");
		return -1;      // 未能创建
	}
	m_wndToolBar.SetWindowText("标准");
	m_wndToolBar.SetBorders ();

	/*if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("未能创建状态栏\n");
		return -1;      // 未能创建
	}*/
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	DockPane(&m_wndToolBar);

	
	CreateDockingWindow(this,IDD_TOWER_PROPERTY_DLG,IDS_TOWER_PROP_VIEW,m_towerPropertyView,CBRS_RIGHT);
	m_towerPropertyView.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_towerPropertyView);
	if(!theApp.m_bChildProcess)
	{	//子进程启动程序时，不显示树形列表框
		CreateDockingWindow(this,IDD_MODEL_TREE_DLG,IDS_TOWER_CTRL_VIEW,m_towerTreeView,CBRS_LEFT);
		m_towerTreeView.EnableDocking(CBRS_ALIGN_ANY);
		DockPane(&m_towerTreeView);
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或
	// 样式

	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		 | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE | WS_SYSMENU;

	return TRUE;
}
/*BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// 基类将执行真正的工作

	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	// 为所有用户工具栏启用自定义按钮
	//BOOL bNameValid;
	//CString strCustomize;
	//bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	//ASSERT(bNameValid);
	//
	//for (int i = 0; i < iMaxUserToolbars; i ++)
	//{
	//	CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
	//	if (pUserToolbar != NULL)
	//	{
	//		pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	//	}
	//}
	return TRUE;
}*/


// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}

#endif //_DEBUG


// CMainFrame 消息处理程序


BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类

	try{
		return CFrameWndEx::OnCommand(wParam, lParam);
	}
	catch(char* sError)
	{
		AfxMessageBox(sError);
	}
	return FALSE;
}
