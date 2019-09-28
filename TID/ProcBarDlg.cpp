#include "stdafx.h"
#include "ProcBarDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CProcBarDlg dialog


CProcBarDlg::CProcBarDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcBarDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProcBarDlg)
	m_sProgress = _T("0%");
	//}}AFX_DATA_INIT
	m_pWnd = pParent;
}
BOOL CProcBarDlg::Create()
{
	return CDialog::Create(CProcBarDlg::IDD);
}


void CProcBarDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcBarDlg)
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_ctrlProcBar);
	DDX_Text(pDX, IDC_NUMERIC_PROC, m_sProgress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcBarDlg, CDialog)
	//{{AFX_MSG_MAP(CProcBarDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcBarDlg message handlers
BOOL CProcBarDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ctrlProcBar.SetRange(0,100);
	return TRUE;
}

static void DoWindowEvent(HWND m_hWnd=NULL)
{
	MSG msg; //定义一个MSG类型的变量，不多解释
    while(PeekMessage(&msg,m_hWnd,0,0,PM_REMOVE)) //获取消息并把该消息从消息队列中移除（防止重复响应）。
    {
        TranslateMessage(&msg);//翻译消息 在合适的机会产生char消息
        DispatchMessage(&msg); //将消息移交给过程函数
    }
}
//BOOL CALLBACK EnumWindows1(HWND hwnd, LPARAM lParam);
//BOOL CALLBACK EnumWindows2(HWND hwnd, LPARAM lParam);
static BOOL CALLBACK EnumWindows2(HWND hwnd,LPARAM lParam){
    DoWindowEvent(hwnd);
    return true;
}
static BOOL CALLBACK EnumWindows1(HWND hwnd,LPARAM lParam){
    
    DoWindowEvent(hwnd);
    EnumChildWindows(hwnd,EnumWindows2,0);
    return true;
}
static void SafeDoevents(){
    EnumThreadWindows(GetCurrentThreadId(),EnumWindows1,0);
};
void CProcBarDlg::SetTitle(CString ss)
{
	SetWindowText(ss);
}

void CProcBarDlg::Refresh(int proc)
{
	m_ctrlProcBar.SetPos(proc);
	m_sProgress.Format("%d%%",proc);
	UpdateData(FALSE);
	DoWindowEvent();
}

