#include "StdAfx.h"
#include "DialogPanel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDialogPanel::CDialogPanel()
{
	m_idDialog=0;
	m_pDialog=NULL;
	m_pDlgRunClass=NULL;
}

CDialogPanel::~CDialogPanel(void)
{
}

BEGIN_MESSAGE_MAP(CDialogPanel, CDockablePane)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CDialogPanel::Init(CRuntimeClass *pRunClass, UINT idDlg)
{
	m_pDlgRunClass=pRunClass;
	m_idDialog=idDlg;
}

void CDialogPanel::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	if(m_pDialog)
	{
       CRect rect;
       GetClientRect(rect);
       m_pDialog->MoveWindow(rect);
	}
}

int CDialogPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;
	ASSERT(m_idDialog>0&&m_pDlgRunClass);
	m_pDialog = (CDialogEx*)m_pDlgRunClass->CreateObject();
	if(m_pDialog!=NULL)
	{
		m_pDialog->Create(m_idDialog,this);
		m_pDialog->ShowWindow(SW_SHOW);
	}
	return 0;
}


void CDialogPanel::OnDestroy()
{
	CDockablePane::OnDestroy();
	if(m_pDialog)
	{
		m_pDialog->DestroyWindow();
		delete m_pDialog;
		m_pDialog=NULL;
	}
}

//重载OnShowControlBarMenu()禁用Panel上的右键菜单
//返回FALSE会导致显示控制工具栏的右键菜单在Panel上显示 wht 14-02-27
BOOL CDialogPanel::OnShowControlBarMenu(CPoint point)
{
	return TRUE;
}