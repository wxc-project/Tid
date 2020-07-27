// TID.cpp : 定义应用程序的类行为。
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
	// 基于文件的标准文档命令
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// 标准打印设置命令
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


// CTIDApp 构造

CTIDApp::CTIDApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
	m_ciFileType=0;
	m_uiActiveHeightSerial=1;
	for(int i=0;i<4;i++)
		m_uiActiveLegSerial[i]=1;
	m_bChildProcess=FALSE;
}


// 唯一的一个 CTIDApp 对象

CTIDApp theApp;
IModModel* gpModModel;
#ifdef NEW_VERSION_TIDCORE
ITidModel* gpTidModel;
#endif
// CTIDApp 初始化
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
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControls()。否则，将无法创建窗口。
	InitCommonControls();

	CWinApp::InitInstance();

	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	//进行加密处理
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
			AfxMessageBox("首次使用，还未指定过证书文件！");
		else if(retCode==-1)
			AfxMessageBox("加密锁初始化失败");
		else if(retCode==1)
			AfxMessageBox("1#无法打开证书文件");
		else if(retCode==2)
			AfxMessageBox("2#证书文件遭到破坏");
		else if(retCode==3)
			AfxMessageBox("3#证书与加密狗不匹配");
		else if(retCode==4)
			AfxMessageBox("4#授权证书的加密版本不对");
		else if(retCode==5)
			AfxMessageBox("5#证书与加密狗产品授权版本不匹配");
		else if(retCode==6)
			AfxMessageBox("6#超出版本使用授权范围");
		else if(retCode==7)
			AfxMessageBox("7#超出免费版本升级授权范围");
		else if(retCode==8)
			AfxMessageBox("8#证书序号与当前加密狗不一致");
		else if(retCode==9)
			AfxMessageBox("9#授权过期，请续借授权");
		else if(retCode==10)
			AfxMessageBox("10#程序缺少相应执行权限，请以管理员权限运行程序");
		exit(0);
	}*/
	SetRegistryKey(_T("Xerofox"));
	LoadStdProfileSettings(4); 
	//判断TID是否以子进程方式进行工作
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
	// 注册应用程序的文档模板。文档模板
	// 将用作文档、框架窗口和视图之间的连接
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CTIDDoc),
		RUNTIME_CLASS(CMainFrame),       // 主 SDI 框架窗口
		RUNTIME_CLASS(CTIDView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	// 启用“DDE 执行”
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);
	// 分析标准外壳命令、DDE、打开文件操作的命令行
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// 调度在命令行中指定的命令。如果
	// 用 /RegServer、/Register、/Unregserver 或 /Unregister 启动应用程序，则返回 FALSE。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// 唯一的一个窗口已初始化，因此显示它并对其进行更新
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();
	// 仅当存在后缀时才调用 DragAcceptFiles，
	//  在 SDI 应用程序中，这应在 ProcessShellCommand  之后发生
	// 启用拖/放
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
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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

// 用于运行对话框的应用程序命令
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

//测试呼高中的减退序号与减退米数
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
		logerr.Log("接腿序号:%d\t减退米数:%f",xLegSerialArr[i],fDiffDist);
		//
		int iLegSerial=pHeightGroup->GetLegSerial(fDiffDist);
		logerr.Log("减退米数:%f\t接腿序号:%d",fDiffDist,iLegSerial);
	}
	if(logerr.IsHasContents())
		logerr.ShowToScreen();
}
//测试挂点信息
void Test2()
{
	int nHangPt=gpTidModel->HangPointCount();
	for(int i=0;i<nHangPt;i++)
	{
		ITidHangPoint* pHangPt=gpTidModel->GetHangPointAt(i);
		char cType=pHangPt->GetWireType();
		CXhChar50 sName;
		pHangPt->GetWireDescription(sName);
		logerr.Log("挂点：%c\t名称：%s",cType,sName);
	}
	if(logerr.IsHasContents())
		logerr.ShowToScreen();
}
// CTIDApp 消息处理程序

