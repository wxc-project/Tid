#if !defined(AFX_SETACTIVEMODULEDLG_H__3E236B08_BE12_4057_A980_EAC4E2416C56__INCLUDED_)
#define AFX_SETACTIVEMODULEDLG_H__3E236B08_BE12_4057_A980_EAC4E2416C56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetActiveModuleDlg.h : header file
//
#include "resource.h"
#include "TID.h"
/////////////////////////////////////////////////////////////////////////////
// CSetActiveHeightLegsDlg dialog

class CSetActiveHeightLegsDlg : public CDialog
{
// Construction
public:
	CSetActiveHeightLegsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetActiveHeightLegsDlg)
	enum { IDD = IDD_SET_ACTIVE_MODULE_DLG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetActiveHeightLegsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetActiveHeightLegsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBnOk();
	afx_msg void OnCbnSelchangeCmbModuleNo();
	afx_msg void OnCbnSelchangeCmbLegQuadA();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	HEIGHT_GROUP* m_pHeight;
public:
	UINT m_idBodyHeight;
	BYTE xarrActiveLegSerials[4];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETACTIVEMODULEDLG_H__3E236B08_BE12_4057_A980_EAC4E2416C56__INCLUDED_)
