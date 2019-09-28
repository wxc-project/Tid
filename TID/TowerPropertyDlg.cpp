//<LOCALE_TRANSLATE Confirm by hxr/>
// TowerPropertyDlg.cpp : implementation file
//
#include "stdafx.h"
#include "TID.h"
#include "TowerPropertyDlg.h"
#include "TIDView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTowerPropertyDlg dialog
IMPLEMENT_DYNCREATE(CTowerPropertyDlg, CDialog)

CTowerPropertyDlg::CTowerPropertyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTowerPropertyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTowerPropertyDlg)
	//}}AFX_DATA_INIT
	m_nSplitterWidth = 5;
	m_pAtom = NULL;
	m_bShowTopCMB=FALSE;
	m_nOldBtmHeight = 0;
	m_curClsTypeId=0;	//默认显示选中的所有构件的属性
}


void CTowerPropertyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTowerPropertyDlg)
	DDX_Control(pDX, IDC_CMB_SELECT_OBJECT_INFO, m_cmbSelectObjInfo);
	DDX_Control(pDX, IDC_TAB_GROUP, m_ctrlPropGroup);
	DDX_Control(pDX, IDC_LIST_BOX, m_propList);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_S_PROP_HELP_PIC, m_propHelpWnd);
}


BEGIN_MESSAGE_MAP(CTowerPropertyDlg, CDialog)
	//{{AFX_MSG_MAP(CTowerPropertyDlg)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KILLFOCUS()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_GROUP, OnSelchangeTabGroup)
	ON_WM_CREATE()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTowerPropertyDlg message handlers
BOOL CTowerPropertyDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	CPropTreeItem* pPropItem=m_propList.GetSelectedItem();
	
	return TRUE;	
}

BOOL CTowerPropertyDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
#ifdef __INTERNAL_VERSION
	m_bShowTopCMB=TRUE;
	m_cmbSelectObjInfo.SetCurSel(0);
#endif
	RECT rc,rcHeader,rcTop,rcBottom,rcCMB;
	GetClientRect(&rc);
	m_rcClient = rc;
	CWnd *pTopWnd = GetDlgItem(IDC_LIST_BOX);
	CWnd *pBtmWnd = GetDlgItem(IDC_S_PROP_HELP_PIC);	//IDC_E_PROP_HELP_STR
	CWnd *pCmbWnd = GetDlgItem(IDC_CMB_SELECT_OBJECT_INFO);
	if(pTopWnd)
		pTopWnd->GetWindowRect(&rcTop);
	if(pBtmWnd)
		pBtmWnd->GetWindowRect(&rcBottom);
	if(pCmbWnd)
		pCmbWnd->GetWindowRect(&rcCMB);
	if(!m_bShowTopCMB)
	{	//不显示顶部下拉框
		if(pCmbWnd)
			pCmbWnd->ShowWindow(SW_HIDE);
		rcCMB.top=rcCMB.bottom=rcCMB.left=rcCMB.right=0;
	}
	else 
	{
		if(pCmbWnd)
			pCmbWnd->ShowWindow(SW_SHOW);
		/*int cmbHeight = rcCMB.bottom - rcCMB.top;
		int cmbWidth  = rcCMB.right - rcCMB.left;
		rcCMB.left=rcCMB.top=0;
		rcCMB.right=cmbWidth;
		rcCMB.bottom=cmbHeight;
		rcCMB.bottom+=4;
		if(pCmbWnd)
		pCmbWnd->MoveWindow(&rcCMB);*/
		rcCMB.bottom+=4;
	}
	m_propList.m_hPromptWnd = pBtmWnd->GetSafeHwnd();
	ScreenToClient(&rcTop);
	ScreenToClient(&rcBottom);
	if(m_bShowTopCMB)
		ScreenToClient(&rcCMB);
	int btmHeight = rcBottom.bottom - rcBottom.top;
	rcHeader.left = rcTop.left = rcBottom.left = 0;
	rcHeader.right = rcTop.right = rcBottom.right = rc.right;
	rcHeader.top=rcCMB.bottom;
	rcBottom.bottom = rc.bottom;
	//根据分组数调整窗口位置
	if(m_arrPropGroupLabel.GetSize()<=0)
		rcHeader.bottom=rcTop.top=rcCMB.bottom;
	else
		rcHeader.bottom = rcTop.top = rcCMB.bottom+21;

	rcTop.bottom=rc.bottom-btmHeight-m_nSplitterWidth-1;
	rcBottom.top=rcTop.bottom+m_nSplitterWidth+1;
	m_nOldHorzY = rcBottom.top-m_nSplitterWidth/2;

	RefreshTabCtrl(0);
	m_ctrlPropGroup.MoveWindow(&rcHeader);
	if(pTopWnd)
		pTopWnd->MoveWindow(&rcTop);
	if(pBtmWnd)
		pBtmWnd->MoveWindow(&rcBottom);
	
	m_bTracking = FALSE;
	m_hCursorSize = AfxGetApp()->LoadStandardCursor(IDC_SIZENS);
	m_hCursorArrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTowerPropertyDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	m_rcClient.bottom = cy;
	m_rcClient.right = cx;
	RECT rcHeader,rcTop,rcBottom,rcCMB;
	CWnd *pTopWnd = GetDlgItem(IDC_LIST_BOX);
	CWnd *pBtmWnd = GetDlgItem(IDC_S_PROP_HELP_PIC);	//IDC_E_PROP_HELP_STR
	CWnd *pCmbWnd = GetDlgItem(IDC_CMB_SELECT_OBJECT_INFO);
	if(pTopWnd)
		pTopWnd->GetWindowRect(&rcTop);
	if(pBtmWnd)
		pBtmWnd->GetWindowRect(&rcBottom);
	if(pCmbWnd)
		pCmbWnd->GetWindowRect(&rcCMB);
	if(!m_bShowTopCMB)
	{	//不显示顶部下拉框
		if(pCmbWnd)
			pCmbWnd->ShowWindow(SW_HIDE);
		rcCMB.top=rcCMB.bottom=rcCMB.left=rcCMB.right=0;
	}
	else 
	{
		if(pCmbWnd)
			pCmbWnd->ShowWindow(SW_SHOW);
		/*int cmbHeight = rcCMB.bottom - rcCMB.top;
		int cmbWidth  = rcCMB.right - rcCMB.left;
		rcCMB.left=rcCMB.top=0;
		rcCMB.right=cmbWidth;
		rcCMB.bottom=cmbHeight;
		rcCMB.bottom+=4;
		if(pCmbWnd)
			pCmbWnd->MoveWindow(&rcCMB);*/
		rcCMB.bottom+=4;
	}
	ScreenToClient(&rcTop);
	ScreenToClient(&rcBottom);
	if(m_bShowTopCMB)
		ScreenToClient(&rcCMB);
	int btmHeight = rcBottom.bottom - rcBottom.top;
	rcHeader.left = rcTop.left = rcBottom.left = 0;
	rcHeader.right = rcTop.right = rcBottom.right = cx;
	rcHeader.top=rcCMB.bottom;
	rcBottom.bottom = cy;
	//根据分组数调整窗口位置
	if(m_arrPropGroupLabel.GetSize()<=0)
		rcHeader.bottom=rcTop.top=rcCMB.bottom;
	else
		rcHeader.bottom = rcTop.top = rcCMB.bottom+21;

	rcTop.bottom=cy-btmHeight-m_nSplitterWidth-1;
	rcBottom.top=rcTop.bottom+m_nSplitterWidth+1;
	m_nOldHorzY = rcBottom.top-m_nSplitterWidth/2;
	if(m_ctrlPropGroup.GetSafeHwnd())
		m_ctrlPropGroup.MoveWindow(&rcHeader);
	if(pTopWnd)
		pTopWnd->MoveWindow(&rcTop);
	if(pBtmWnd)
		pBtmWnd->MoveWindow(&rcBottom);
}

void CTowerPropertyDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	RECT rcFull,rcHelpStr,rcSplitter;
	GetClientRect(&rcFull);
	CWnd *pWnd = GetDlgItem(IDC_S_PROP_HELP_PIC);	//IDC_E_PROP_HELP_STR
	pWnd->GetWindowRect(&rcHelpStr);
	ScreenToClient(&rcHelpStr);
	rcSplitter.left = rcFull.left;
	int width = 4;
	rcSplitter.top = rcHelpStr.top-width;
	rcSplitter.right = rcFull.right;
	rcSplitter.bottom = rcHelpStr.top-1;
	CPen psPen(PS_SOLID, 1, RGB(120,120,120));
	CPen* pOldPen = dc.SelectObject(&psPen);
	dc.MoveTo(rcSplitter.left,rcSplitter.top);
	dc.LineTo(rcSplitter.right,rcSplitter.top);
	dc.MoveTo(rcSplitter.left,rcSplitter.bottom);
	dc.LineTo(rcSplitter.right,rcSplitter.bottom);
	dc.SelectObject(pOldPen);
	psPen.DeleteObject();
	// TODO: Add your message handler code here
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CTowerPropertyDlg::InvertLine(CDC* pDC,CPoint ptFrom,CPoint ptTo)
{
	int nOldMode = pDC->SetROP2(R2_NOT);
	
	pDC->MoveTo(ptFrom);
	pDC->LineTo(ptTo);

	pDC->SetROP2(nOldMode);
}

void CTowerPropertyDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if ((point.y>=m_nOldHorzY-5) && (point.y<=m_nOldHorzY+5))
	{
		//if mouse clicked on divider line, then start resizing

		::SetCursor(m_hCursorSize);

		CRect windowRect;
		GetWindowRect(windowRect);
		windowRect.left += 10; windowRect.right -= 10;
		//do not let mouse leave the dialog boundary
		::ClipCursor(windowRect);
		m_nOldHorzY = point.y;

		CClientDC dc(this);
		InvertLine(&dc,CPoint(m_rcClient.left,m_nOldHorzY),CPoint(m_rcClient.right,m_nOldHorzY));

		//capture the mouse
		SetCapture();
		m_bTracking = TRUE;
	}
	else
	{
		m_bTracking = FALSE;
		CDialog::OnLButtonDown(nFlags, point);
	}
}

void CTowerPropertyDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bTracking)
	{
		m_bTracking = FALSE;
		//if mouse was captured then release it
		if (GetCapture()==this)
			::ReleaseCapture();

		::ClipCursor(NULL);

		CClientDC dc(this);
		InvertLine(&dc,CPoint(m_rcClient.left,point.y),CPoint(m_rcClient.right,point.y));
		//set the divider position to the new value
		m_nOldHorzY = point.y;

		RECT rcTop,rcBottom;
		CWnd *pTopWnd = GetDlgItem(IDC_LIST_BOX);
		CWnd *pBtmWnd = GetDlgItem(IDC_S_PROP_HELP_PIC);	//IDC_E_PROP_HELP_STR
		if(pTopWnd)
			pTopWnd->GetWindowRect(&rcTop);
		if(pBtmWnd)
			pBtmWnd->GetWindowRect(&rcBottom);
		ScreenToClient(&rcTop);
		ScreenToClient(&rcBottom);
		rcBottom.top = m_nOldHorzY+m_nSplitterWidth/2;
		rcTop.bottom = rcBottom.top-m_nSplitterWidth-1;
		pTopWnd->MoveWindow(&rcTop);
		pBtmWnd->MoveWindow(&rcBottom);
		//redraw
		Invalidate();
	}
	else
		CDialog::OnLButtonUp(nFlags, point);
}

void CTowerPropertyDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bTracking)
	{
		//move divider line to the mouse pos. if columns are
		//currently being resized
		CClientDC dc(this);
		//remove old divider line
		InvertLine(&dc,CPoint(m_rcClient.left,m_nOldHorzY),CPoint(m_rcClient.right,m_nOldHorzY));
		//draw new divider line
		InvertLine(&dc,CPoint(m_rcClient.left,point.y),CPoint(m_rcClient.right,point.y));
		m_nOldHorzY = point.y;
	}
	else if ((point.y >= m_nOldHorzY-5) && (point.y <= m_nOldHorzY+5))
		//set the cursor to a sizing cursor if the cursor is over the row divider
		::SetCursor(m_hCursorSize);
	else
		CDialog::OnMouseMove(nFlags, point);
}

void CTowerPropertyDlg::OnOK() 
{
	//确认输入
}

void CTowerPropertyDlg::OnCancel() 
{
}

BOOL CTowerPropertyDlg::PreTranslateMessage(MSG* pMsg) 
{
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CTowerPropertyDlg::IsHasFocus()
{
	CWnd *pWnd = GetFocus();
	CWnd *pParentWnd = NULL;
	HWND hParent = NULL;
	if(pWnd)
	{
		pParentWnd = pWnd->GetParent();
		if(pParentWnd)
			hParent = pParentWnd->GetSafeHwnd();
	}
	if(pWnd->GetSafeHwnd()==GetParent()->GetSafeHwnd()||pWnd->GetSafeHwnd()==GetSafeHwnd()||hParent==GetSafeHwnd())
		return TRUE;
	else
		return FALSE;
}


void CTowerPropertyDlg::OnKillFocus(CWnd* pNewWnd) 
{
	CDialog::OnKillFocus(pNewWnd);
/*	CPropertyList *pList = (CPropertyList*)GetDlgItem(IDC_LIST_BOX);
	pList->OnKillfocusCmbBox();
	pList->OnKillfocusEditBox();
	*/
}

void CTowerPropertyDlg::SetCurSelPropGroup(int iCurSel)
{
	/*switch(m_propList.m_nObjClassTypeId)
	{
	default:
		CLDSApp::m_iCurDisplayPropGroup=iCurSel;
		break;
	}*/
}

void CTowerPropertyDlg::OnSelchangeTabGroup(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_propList.m_iPropGroup = m_ctrlPropGroup.GetCurSel();
	SetCurSelPropGroup(m_propList.m_iPropGroup);
	m_propList.Redraw();
	*pResult = 0;
}

void CTowerPropertyDlg::RefreshTabCtrl(int iCurSel)
{
	m_ctrlPropGroup.DeleteAllItems();
	for(int i=0;i<m_arrPropGroupLabel.GetSize();i++)
		m_ctrlPropGroup.InsertItem(i,m_arrPropGroupLabel[i]);
	if(m_arrPropGroupLabel.GetSize()>0)
	{
		m_ctrlPropGroup.SetCurSel(iCurSel);
		m_propList.m_iPropGroup=iCurSel;
	}
	else //不需要分组显示
		m_propList.m_iPropGroup=0;
	
	//根据分组数调整窗口位置
	RECT rcPropWnd,rcHeader,rcTop,rcBottom,rcCMB;
	GetClientRect(&rcPropWnd);
	CWnd *pTopWnd = GetDlgItem(IDC_LIST_BOX);
	CWnd *pBtmWnd = GetDlgItem(IDC_S_PROP_HELP_PIC);	//IDC_E_PROP_HELP_STR
	CWnd *pCmbWnd = GetDlgItem(IDC_CMB_SELECT_OBJECT_INFO);
	if(pTopWnd)
		pTopWnd->GetWindowRect(&rcTop);
	if(pBtmWnd)
		pBtmWnd->GetWindowRect(&rcBottom);
	if(pCmbWnd)
		pCmbWnd->GetWindowRect(&rcCMB);
	if(!m_bShowTopCMB)
	{	//不显示顶部下拉框
		if(pCmbWnd)
			pCmbWnd->ShowWindow(SW_HIDE);
		rcCMB.top=rcCMB.bottom=rcCMB.left=rcCMB.right=0;
	}
	else 
	{
		if(pCmbWnd)
			pCmbWnd->ShowWindow(SW_SHOW);
		/*int cmbHeight = rcCMB.bottom - rcCMB.top;
		int cmbWidth  = rcCMB.right - rcCMB.left;
		rcCMB.left=rcCMB.top=0;
		rcCMB.right=cmbWidth;
		rcCMB.bottom=cmbHeight;
		rcCMB.bottom+=4;
		if(pCmbWnd)
			pCmbWnd->MoveWindow(&rcCMB);*/
		rcCMB.bottom+=4;
	}
	ScreenToClient(&rcTop);
	ScreenToClient(&rcBottom);
	if(m_bShowTopCMB)
		ScreenToClient(&rcCMB);
	int btmHeight = rcBottom.bottom - rcBottom.top;
	rcHeader.left = rcTop.left = rcBottom.left = 0;
	rcHeader.right = rcTop.right = rcBottom.right = rcPropWnd.right;
	rcHeader.top=rcCMB.bottom;
	rcBottom.bottom = rcPropWnd.bottom;
	if(m_arrPropGroupLabel.GetSize()<=0)
		rcHeader.bottom=rcTop.top=rcCMB.bottom;
	else
		rcHeader.bottom = rcTop.top = rcCMB.bottom+20;
	
	rcTop.bottom=rcPropWnd.bottom-btmHeight-m_nSplitterWidth-1;
	rcBottom.top=rcTop.bottom+m_nSplitterWidth+1;
	m_nOldHorzY = rcBottom.top-m_nSplitterWidth/2;
	if(m_ctrlPropGroup.GetSafeHwnd())
		m_ctrlPropGroup.MoveWindow(&rcHeader);
	if(pTopWnd)
		pTopWnd->MoveWindow(&rcTop);
	if(pBtmWnd)
		pBtmWnd->MoveWindow(&rcBottom);
}

void CTowerPropertyDlg::SetPropHelpToTextMode()
{
	if(m_propHelpWnd.GetStyle()&SS_BITMAP)
		SetPropHelpBmpID(NULL);	//设置为文本帮助模式
}
void CTowerPropertyDlg::SetPropHelpBmpID(UINT nID)
{	
	CRect rcBottom;
	m_propHelpWnd.GetWindowRect(&rcBottom);
	ScreenToClient(&rcBottom);
	//
	m_xCurHelpBmp.Detach();
	BOOL bRetCode=FALSE;
	if(nID!=0)
		bRetCode=m_xCurHelpBmp.LoadBitmap(nID);
	int nBtmHeight=0;
	if(bRetCode)
	{	//显示帮助图片
		m_propHelpWnd.ModifyStyle(0,SS_BITMAP|SS_CENTERIMAGE|SS_REALSIZEIMAGE);
		m_propHelpWnd.SetBitmap(m_xCurHelpBmp);
		//根据图片高度调整m_propHelpWnd高度
		nBtmHeight=rcBottom.bottom-rcBottom.top;
		if(m_nOldBtmHeight==0)
			m_nOldBtmHeight=nBtmHeight;
		BITMAP bmp;
		m_xCurHelpBmp.GetBitmap(&bmp);
		if(nBtmHeight<bmp.bmHeight)	//确保m_propHelpWnd高度大于图片高度
			nBtmHeight=bmp.bmHeight;
	}
	else
	{
		m_propHelpWnd.ModifyStyle(SS_BITMAP,0);
		m_propHelpWnd.ModifyStyle(SS_CENTERIMAGE,0);
		m_propHelpWnd.ModifyStyle(SS_REALSIZEIMAGE,0);
		m_propHelpWnd.SetFont(m_propList.GetFont());
		//恢复m_propHelpWnd原高度
		nBtmHeight=m_nOldBtmHeight;
	}
	if(nBtmHeight>0&&nBtmHeight!=(rcBottom.bottom-rcBottom.top))
	{	//调整帮助字符串窗口尺寸
		CRect rcTop;
		m_propList.GetWindowRect(&rcTop);
		ScreenToClient(&rcTop);
		rcTop.bottom=m_rcClient.bottom-nBtmHeight-m_nSplitterWidth-1;
		rcBottom.top=rcTop.bottom+m_nSplitterWidth+1;
		m_nOldHorzY = rcBottom.top-m_nSplitterWidth/2;
		m_propList.MoveWindow(&rcTop);
		m_propHelpWnd.MoveWindow(&rcBottom);
		m_propHelpWnd.Invalidate();
	}
}