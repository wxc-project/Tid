// SetActiveModuleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SetActiveHeightLegsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SetActiveHeightLegsDlg dialog


CSetActiveHeightLegsDlg::CSetActiveHeightLegsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetActiveHeightLegsDlg::IDD, pParent)
{
	m_idBodyHeight=1;
	xarrActiveLegSerials[0]=xarrActiveLegSerials[1]=xarrActiveLegSerials[2]=xarrActiveLegSerials[3]=1;
	//{{AFX_DATA_INIT(SetActiveHeightLegsDlg)
	//}}AFX_DATA_INIT
}


void CSetActiveHeightLegsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetActiveHeightLegsDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetActiveHeightLegsDlg, CDialog)
	//{{AFX_MSG_MAP(CSetActiveHeightLegsDlg)
	ON_CBN_SELCHANGE(IDC_CMB_MODULE_NO, OnCbnSelchangeCmbModuleNo)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnOk)
	ON_CBN_SELCHANGE(IDC_CMB_LEG_QUAD_A, OnCbnSelchangeCmbLegQuadA)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetActiveHeightLegsDlg message handlers

BOOL CSetActiveHeightLegsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd* pNameCtrl=GetDlgItem(IDC_S_HEIGHT_NAME);
	ITidHeightGroup* pHeight=gpTidModel->GetHeightGroup(this->m_idBodyHeight);
	if (pNameCtrl&&pHeight)
	{
		char name[50]="";
		pHeight->GetName(name,50);
		pNameCtrl->SetWindowText(name);
	}
	CComboBox *pQuadCmb=NULL;
	pQuadCmb=(CComboBox*)GetDlgItem(IDC_CMB_LEG_QUAD_A);
	pQuadCmb->ResetContent();
	pQuadCmb=(CComboBox*)GetDlgItem(IDC_CMB_LEG_QUAD_B);
	pQuadCmb->ResetContent();
	pQuadCmb=(CComboBox*)GetDlgItem(IDC_CMB_LEG_QUAD_C);
	pQuadCmb->ResetContent();
	pQuadCmb=(CComboBox*)GetDlgItem(IDC_CMB_LEG_QUAD_D);
	pQuadCmb->ResetContent();
	ITidHeightGroup* pTidHeight=gpTidModel->GetHeightGroup(this->m_idBodyHeight);
	BYTE xarrLegCfgBytes[24]={ 0 };
	BYTE xarrConstBytes[8]={ 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80 };
	pTidHeight->GetConfigBytes(xarrLegCfgBytes);
	CString sTemp;
	int i,j,niLegSerial=0;
	for(i=1;i<=192;i++)
	{
		int niByteIndex=(i-1)/8;
		int niBitIndex=(i-1)%8;
		if(xarrLegCfgBytes[niByteIndex]&xarrConstBytes[niBitIndex])
		{
			sTemp.Format("%C",niLegSerial+'A');
			for(j=0;j<4;j++)
			{
				if(j==0)
					pQuadCmb=(CComboBox*)GetDlgItem(IDC_CMB_LEG_QUAD_A);
				else if(j==1)
					pQuadCmb=(CComboBox*)GetDlgItem(IDC_CMB_LEG_QUAD_B);
				else if(j==2)
					pQuadCmb=(CComboBox*)GetDlgItem(IDC_CMB_LEG_QUAD_C);
				else if(j==3)
					pQuadCmb=(CComboBox*)GetDlgItem(IDC_CMB_LEG_QUAD_D);
				int index=pQuadCmb->AddString(sTemp);
				if (i==xarrActiveLegSerials[j])
					pQuadCmb->SetCurSel(index);
			}
			niLegSerial++;
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSetActiveHeightLegsDlg::OnCbnSelchangeCmbModuleNo()
{
	UpdateData();
	//m_pHeight=theApp.hashModelHeights.Add(this->m_idBodyHeight);
}

void CSetActiveHeightLegsDlg::OnBnOk()
{
	UpdateData();
	UINT xarrLegCmbId[4]={ IDC_CMB_LEG_QUAD_A,IDC_CMB_LEG_QUAD_B,IDC_CMB_LEG_QUAD_C,IDC_CMB_LEG_QUAD_D };
	ITidHeightGroup* pTidHeight=gpTidModel->GetHeightGroup(this->m_idBodyHeight);
	BYTE xarrLegCfgBytes[24]={ 0 };
	BYTE xarrConstBytes[8]={ 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80 };
	pTidHeight->GetConfigBytes(xarrLegCfgBytes);
	int i,j,niLegSerial=0;
	memset(xarrActiveLegSerials,0,4);
	for (i=1;i<=192;i++)
	{
		int niByteIndex=(i-1)/8;
		int niBitIndex=(i-1)%8;
		if (xarrLegCfgBytes[niByteIndex]&xarrConstBytes[niBitIndex])
		{
			char ciQuadLegSymbol=niLegSerial+'A';
			bool hasNonSet=false;
			for (j=0;j<4;j++)
			{
				if (xarrActiveLegSerials[j]>0)
					continue;
				CComboBox* pQuadCmb=(CComboBox*)GetDlgItem(xarrLegCmbId[j]);
				char szActiveSymbol[2]={ 0 };
				int iSel=pQuadCmb->GetCurSel();
				pQuadCmb->GetLBText(iSel,szActiveSymbol);
				if (szActiveSymbol[0]==ciQuadLegSymbol)
					xarrActiveLegSerials[j]=i;
				else
					hasNonSet=true;
			}
			niLegSerial++;
			if (!hasNonSet)
				break;
		}
	}
	OnOK();
}

void CSetActiveHeightLegsDlg::OnCbnSelchangeCmbLegQuadA()
{
	CComboBox *pCmbQuadA=(CComboBox*)GetDlgItem(IDC_CMB_LEG_QUAD_A);
	CComboBox *pCmbQuadB=(CComboBox*)GetDlgItem(IDC_CMB_LEG_QUAD_B);
	CComboBox *pCmbQuadC=(CComboBox*)GetDlgItem(IDC_CMB_LEG_QUAD_C);
	CComboBox *pCmbQuadD=(CComboBox*)GetDlgItem(IDC_CMB_LEG_QUAD_D);
	int iCurSel=pCmbQuadA->GetCurSel();
	pCmbQuadB->SetCurSel(iCurSel);
	pCmbQuadC->SetCurSel(iCurSel);
	pCmbQuadD->SetCurSel(iCurSel);
}
