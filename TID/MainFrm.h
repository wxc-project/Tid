// MainFrm.h : CMainFrame 类的接口
//

#include <afxframewndex.h>


#include "DialogPanel.h"
#include "TowerTreeDlg.h"
#include "TowerPropertyDlg.h"

#pragma once
class CMainFrame : public CFrameWndEx
{
	
protected: // 仅从序列化创建
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// 属性
public:

// 操作
public:

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	CTowerTreeDlg* GetTowerTreePage() { return (CTowerTreeDlg*)m_towerTreeView.GetDlgPtr(); }
	CTowerPropertyDlg* GetTowerPropertyPage() { return (CTowerPropertyDlg*)m_towerPropertyView.GetDlgPtr(); }

protected:  // 控件条嵌入成员
	//CMouseStatusBar m_wndStatusBar;
	CMFCToolBar     m_wndToolBar;
	CDialogPanel	m_towerPropertyView;
	CDialogPanel	m_towerTreeView;

// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnApplicationLook(UINT id);
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};


