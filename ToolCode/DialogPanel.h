#pragma once

//<DEVELOP_PROCESS_MARK nameId="CDialogPanel">
class CDialogPanel :public CDockablePane
{
	UINT m_idDialog;
	CDialogEx *m_pDialog;
	CRuntimeClass *m_pDlgRunClass;
public:
	CDialogPanel();
	virtual ~CDialogPanel(void);
	void Init(CRuntimeClass *pClass, UINT idDlg);
	CDialogEx *GetDlgPtr() {return m_pDialog;}
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();

	virtual BOOL OnShowControlBarMenu(CPoint point);
};
//</DEVELOP_PROCESS_MARK>

