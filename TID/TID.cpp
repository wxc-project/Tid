// TID.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "TID.h"
#include "MainFrm.h"

#include "TIDDoc.h"
#include "TIDView.h"
#include "AssemblePart.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CTIDApp

BEGIN_MESSAGE_MAP(CTIDApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// �����ļ��ı�׼�ĵ�����
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// ��׼��ӡ��������
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


// CTIDApp ����

CTIDApp::CTIDApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
	m_ciFileType=0;
	m_uiActiveHeightSerial=1;
	for(int i=0;i<4;i++)
		m_uiActiveLegSerial[i]=1;
	m_bChildProcess=FALSE;
}


// Ψһ��һ�� CTIDApp ����

CTIDApp theApp;
IModModel* gpModModel;
#ifdef NEW_VERSION_TIDCORE
ITidModel* gpTidModel;
#endif
// CTIDApp ��ʼ��
BOOL GetLicFilePathFromReg(char* licfile_path)
{
	const char* szRegItemKey="lic_file2";
	char sPath[MAX_PATH]="",sSubKey[MAX_PATH]="";
	strcpy(sSubKey,"Software\\Xerofox\\LDS\\Settings");
	//
	HKEY hKey;
	RegOpenKeyEx(HKEY_CURRENT_USER,sSubKey,0,KEY_READ,&hKey);
	DWORD dwDataType,dwLength=MAX_PATH;
	if(RegQueryValueEx(hKey,_T(szRegItemKey),NULL,&dwDataType,(BYTE*)&sPath[0],&dwLength)== ERROR_SUCCESS)
		strcpy(licfile_path,sPath);
	RegCloseKey(hKey);
	if(strlen(licfile_path)>1)
		return TRUE;
	return FALSE;
}
BOOL CTIDApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControls()�����򣬽��޷��������ڡ�
	InitCommonControls();

	CWinApp::InitInstance();

	// ��ʼ�� OLE ��
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	//���м��ܴ���
	/*DWORD version[2]={0,20170615};
	BYTE* pByteVer=(BYTE*)version;
	pByteVer[0]=1;
	pByteVer[1]=2;
	pByteVer[2]=0;
	pByteVer[3]=0;
	char lic_file[MAX_PATH];
	GetLicFilePathFromReg(lic_file);
	ULONG retCode=ImportLicFile(lic_file,PRODUCT_LDS,version);
	if(retCode!=0)
	{
		if(retCode==-2)
			AfxMessageBox("�״�ʹ�ã���δָ����֤���ļ���");
		else if(retCode==-1)
			AfxMessageBox("��������ʼ��ʧ��");
		else if(retCode==1)
			AfxMessageBox("1#�޷���֤���ļ�");
		else if(retCode==2)
			AfxMessageBox("2#֤���ļ��⵽�ƻ�");
		else if(retCode==3)
			AfxMessageBox("3#֤������ܹ���ƥ��");
		else if(retCode==4)
			AfxMessageBox("4#��Ȩ֤��ļ��ܰ汾����");
		else if(retCode==5)
			AfxMessageBox("5#֤������ܹ���Ʒ��Ȩ�汾��ƥ��");
		else if(retCode==6)
			AfxMessageBox("6#�����汾ʹ����Ȩ��Χ");
		else if(retCode==7)
			AfxMessageBox("7#������Ѱ汾������Ȩ��Χ");
		else if(retCode==8)
			AfxMessageBox("8#֤������뵱ǰ���ܹ���һ��");
		else if(retCode==9)
			AfxMessageBox("9#��Ȩ���ڣ���������Ȩ");
		else if(retCode==10)
			AfxMessageBox("10#����ȱ����Ӧִ��Ȩ�ޣ����Թ���ԱȨ�����г���");
		exit(0);
	}*/
	SetRegistryKey(_T("Xerofox"));
	LoadStdProfileSettings(4); 
	//�ж�TID�Ƿ����ӽ��̷�ʽ���й���
	if(__argc>1)
	{
		char system_path[MAX_PATH];
		strcpy(system_path,__argv[1]);
		char *key=strtok(system_path,"-/");
		if(key&&_stricmp(key,"child")==0)
			m_bChildProcess=TRUE;
		else
			m_bChildProcess=FALSE;
	}
	// ע��Ӧ�ó�����ĵ�ģ�塣�ĵ�ģ��
	// �������ĵ�����ܴ��ں���ͼ֮�������
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CTIDDoc),
		RUNTIME_CLASS(CMainFrame),       // �� SDI ��ܴ���
		RUNTIME_CLASS(CTIDView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	// ���á�DDE ִ�С�
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);
	// ������׼������DDE�����ļ�������������
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// ��������������ָ����������
	// �� /RegServer��/Register��/Unregserver �� /Unregister ����Ӧ�ó����򷵻� FALSE��
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// Ψһ��һ�������ѳ�ʼ���������ʾ����������и���
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();
	// �������ں�׺ʱ�ŵ��� DragAcceptFiles��
	//  �� SDI Ӧ�ó����У���Ӧ�� ProcessShellCommand  ֮����
	// ������/��
	m_pMainWnd->DragAcceptFiles();
	CAssemblePart::InitPropHashtable();
	CAssembleBolt::InitPropHashtable();
	CAssembleAnchorBolt::InitPropHashtable();
	return TRUE;
}
CTIDDoc* CTIDApp::GetTIDDoc()
{
	POSITION pos,docpos;
	pos=AfxGetApp()->m_pDocManager->GetFirstDocTemplatePosition();
	for(CDocTemplate *pDocTemp=AfxGetApp()->m_pDocManager->GetNextDocTemplate(pos);
		pDocTemp;pDocTemp=AfxGetApp()->m_pDocManager->GetNextDocTemplate(pos))
	{
		docpos=pDocTemp->GetFirstDocPosition();
		for(CDocument *pDoc=pDocTemp->GetNextDoc(docpos);pDoc;pDoc=pDocTemp->GetNextDoc(docpos))
		{
			if(pDoc->IsKindOf(RUNTIME_CLASS(CTIDDoc)))
				return (CTIDDoc*)pDoc;
		}
	}
	return NULL;
}
BOOL CTIDApp::SaveState(LPCTSTR lpszSectionName/*=NULL*/, CFrameImpl* pFrameImpl/*=NULL*/)
{
	BOOL bRetCode = CWinAppEx::SaveState(lpszSectionName,pFrameImpl);
	return bRetCode;
}
CTIDView* CTIDApp::GetTIDView()
{
	CTIDDoc *pDoc=GetTIDDoc();
	if(pDoc==NULL)
		return NULL;
	CTIDView *pView = (CTIDView*)pDoc->GetView(RUNTIME_CLASS(CTIDView));
	return pView;
}
bool TestBitState(BYTE xarrLegCfgBytes[24],int niBitSerial)
{
	BYTE xarrConstBytes[8]={ 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80 };
	int niByteIndex=(niBitSerial-1)/8;
	int niBitIndex=(niBitSerial-1)%8;
	if (xarrLegCfgBytes[niByteIndex]&xarrConstBytes[niBitIndex])
		return true;
	else
		return false;
}
void* CTIDApp::GetActiveTowerInstance()
{
	if(m_ciFileType==0)
	{
		ITidTowerInstance* pTowerInstance=NULL;
#ifdef NEW_VERSION_TIDCORE
		ITidHeightGroup* pHeightGroup=gpTidModel->GetHeightGroupAt(m_uiActiveHeightSerial-1);
		if(pHeightGroup==NULL)
			return NULL;
		BYTE xarrLegCfgBytes[24]={ 0 };
		BYTE xarrLegQuadSerialInHeight[4]={ 1 };
		pHeightGroup->GetConfigBytes(xarrLegCfgBytes);
		int i,liCurrLegSerialInHeight=0;
		for (i=1;i<=192;i++)
		{
			if (TestBitState(xarrLegCfgBytes,i))
				liCurrLegSerialInHeight++;
			for (int j=0;j<4;j++)
			{
				if (i==m_uiActiveLegSerial[j])
					xarrLegQuadSerialInHeight[j]=liCurrLegSerialInHeight;
			}
		}
		pTowerInstance=pHeightGroup->GetTowerInstance(xarrLegQuadSerialInHeight[0],xarrLegQuadSerialInHeight[1],xarrLegQuadSerialInHeight[2],xarrLegQuadSerialInHeight[3]);
#endif
		return pTowerInstance;
	}
	else if(m_ciFileType==1)
	{
		IModHeightGroup* pHeightGroup=gpModModel->GetHeightGroup(m_uiActiveHeightSerial);
		if(pHeightGroup==NULL)
			return NULL;
		IModTowerInstance* pInstance=NULL;
		BYTE xarrLegCfgBytes[24]={ 0 };
		BYTE xarrLegQuadSerialInHeight[4]={ 1 };
		/*pHeightGroup->GetConfigBytes(xarrLegCfgBytes);
		int i,liCurrLegSerialInHeight=0;
		for (i=1;i<=192;i++)
		{
			if (TestBitState(xarrLegCfgBytes,i))
				liCurrLegSerialInHeight++;
			for (int j=0;j<4;j++)
			{
				if (i==m_uiActiveLegSerial[j])
					xarrLegQuadSerialInHeight[j]=liCurrLegSerialInHeight;
			}
		}*/
		pInstance=pHeightGroup->GetTowerInstance(xarrLegQuadSerialInHeight[0],xarrLegQuadSerialInHeight[1],xarrLegQuadSerialInHeight[2],xarrLegQuadSerialInHeight[3]);
		return pInstance;
	}
	return NULL;
}
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// �������жԻ����Ӧ�ó�������
#include "LogFile.h"
#include "ArrayList.h"
void Test1();
void Test2();
void CTIDApp::OnAppAbout()
{
	return Test2();
	//
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

//���Ժ����еļ���������������
void Test1()
{
	CLogErrorLife logErrLife;
	ITidHeightGroup* pHeightGroup=gpTidModel->GetHeightGroupAt(1);
	if(pHeightGroup==NULL)
		return;
	int nn=pHeightGroup->GetLegSerialArr(NULL);
	ARRAY_LIST<int> xLegSerialArr;
	xLegSerialArr.SetSize(nn);
	pHeightGroup->GetLegSerialArr(xLegSerialArr.m_pData);
	for(int i=0;i<nn;i++)
	{
		double fDiffDist=pHeightGroup->GetLegHeightDifference(xLegSerialArr[i]);
		logerr.Log("�������:%d\t��������:%f",xLegSerialArr[i],fDiffDist);
		//
		int iLegSerial=pHeightGroup->GetLegSerial(fDiffDist);
		logerr.Log("��������:%f\t�������:%d",fDiffDist,iLegSerial);
	}
	if(logerr.IsHasContents())
		logerr.ShowToScreen();
}
//���Թҵ���Ϣ
void Test2()
{
	int nHangPt=gpTidModel->HangPointCount();
	for(int i=0;i<nHangPt;i++)
	{
		ITidHangPoint* pHangPt=gpTidModel->GetHangPointAt(i);
		char cType=pHangPt->GetWireType();
		CXhChar50 sName;
		pHangPt->GetWireDescription(sName);
		logerr.Log("�ҵ㣺%c\t���ƣ�%s",cType,sName);
	}
	if(logerr.IsHasContents())
		logerr.ShowToScreen();
}
// CTIDApp ��Ϣ�������

