#if !defined(AFX_TOWERPROPERTYDLG_H__86244D2C_2EC7_46C7_A82D_832BDBEC408A__INCLUDED_)
#define AFX_TOWERPROPERTYDLG_H__86244D2C_2EC7_46C7_A82D_832BDBEC408A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TowerPropertyDlg.h : header file
//
#include "PropertyList.h"
#include "HashTable.h"

/////////////////////////////////////////////////////////////////////////////
// CTowerPropertyDlg dialog

class CTowerPropertyDlg : public CDialog
{
// Construction
	BOOL m_bShowTopCMB;
	CStatic m_propHelpWnd;
	CBitmap m_xCurHelpBmp;
	int m_nOldBtmHeight;
public:
	void *m_pAtom;
	RECT m_rcClient;
	int m_nOldHorzY;
	HICON m_hCursorArrow;
	HICON m_hCursorSize;
	BOOL m_bTracking;
	int m_nSplitterWidth;
	CStringArray m_arrPropGroupLabel;
	int m_curClsTypeId;	//当前显示的构件类型 wht 10-11-12
public:
	void RefreshTabCtrl(int iCurSel);
	BOOL IsHasFocus();
	CTowerPropertyDlg(CWnd* pParent = NULL);   // standard constructor
	CPropertyList *GetPropertyList(){return &m_propList;}
	DECLARE_DYNCREATE(CTowerPropertyDlg)
	void SetCurSelPropGroup(int iCurSel);
	//更新构件过滤器 wht 10-11-12
	typedef CHashSet<void*> HASHOBJSET;
	void SetPropHelpBmpID(UINT nID);
	void SetPropHelpToTextMode();
// Dialog Data
	//{{AFX_DATA(CTowerPropertyDlg)
	enum { IDD = IDD_TOWER_PROPERTY_DLG };
	CComboBox	m_cmbSelectObjInfo;
	CTabCtrl	m_ctrlPropGroup;
	CPropertyList	m_propList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTowerPropertyDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CToolBar    m_wndHeadBar;
	void InvertLine(CDC* pDC,CPoint ptFrom,CPoint ptTo);

	// Generated message map functions
	//{{AFX_MSG(CTowerPropertyDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSelchangeTabGroup(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOWERPROPERTYDLG_H__86244D2C_2EC7_46C7_A82D_832BDBEC408A__INCLUDED_)
