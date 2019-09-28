#if !defined(AFX_SETACTIVEITEMDLG_H__3E236B08_BE12_4057_A980_EAC4E2416C56__INCLUDED_)
#define AFX_SETACTIVEITEMDLG_H__3E236B08_BE12_4057_A980_EAC4E2416C56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetActiveItemDlg.h : header file
//
#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CSetActiveItemDlg dialog

class CSetActiveItemDlg : public CDialog
{
// Construction
public:
	CSetActiveItemDlg(CWnd* pParent = NULL);   // standard constructor
	CStringArray m_arrStrList;

// Dialog Data
	//{{AFX_DATA(CSetActiveItemDlg)
	enum { IDD = IDD_SET_ACTIVE_ITEM_DLG };
	CString	m_sPrompt;
	CString	m_sActiveItem;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetActiveItemDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetActiveItemDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETACTIVEITEMDLG_H__3E236B08_BE12_4057_A980_EAC4E2416C56__INCLUDED_)
