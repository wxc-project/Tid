// TIDDoc.cpp :  CTIDDoc 类的实现
//

#include "stdafx.h"
#include "TID.h"
#include "TIDDoc.h"
#ifdef NEW_VERSION_TIDCORE
#include "TidCplus.h"
#else
#include ".\SolidTowerModel.h"
#endif
#include "XhCharString.h"
#include "ArrayList.h"
#include "LogFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

char* SearchChar(char* srcStr,char ch,bool reverseOrder=false)
{
	if(!reverseOrder)
		return strchr(srcStr,ch);
	else
	{
		int len=strlen(srcStr);
		for(int i=len-1;i>=0;i--)
		{
			if(srcStr[i]==ch)
				return &srcStr[i];
		}
	}
	return NULL;
}
// CTIDDoc

IMPLEMENT_DYNCREATE(CTIDDoc, CDocument)

BEGIN_MESSAGE_MAP(CTIDDoc, CDocument)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_EXPORT_SWAP_INFO_FILE, OnExportSwapInfoFile)
END_MESSAGE_MAP()


// CTIDDoc 构造/析构

CTIDDoc::CTIDDoc()
{
	// TODO: 在此添加一次性构造代码
#ifdef NEW_VERSION_TIDCORE
	gpTidModel=NULL;
#endif
	gpModModel=NULL;
}

CTIDDoc::~CTIDDoc()
{
}

BOOL CTIDDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	return TRUE;
}

// CTIDDoc 序列化

void CTIDDoc::Serialize(CArchive& ar)
{
	try{
		if (ar.IsLoading())
		{
			ar.GetFile()->SeekToEnd();
			DWORD file_len=(DWORD)ar.GetFile()->GetPosition();
			ar.GetFile()->SeekToBegin();
			DYN_ARRAY<char> pBuf(file_len);
			ar.Read((char*)pBuf,file_len);
#ifdef NEW_VERSION_TIDCORE
			if(gpTidModel==NULL)
				gpTidModel=CTidModelFactory::CreateTidModel();
			gpTidModel->InitTidBuffer(pBuf,file_len);
#else
			model.InitBuffer(pBuf,file_len);
#endif
		}
	}
	catch(char* sError)
	{
		AfxMessageBox(sError);
		SetTitle("无标题");//throw CArchiveException(CArchiveException::badClass);
	}
}


// CTIDDoc 诊断

#ifdef _DEBUG
void CTIDDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTIDDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CTIDDoc 命令


void CTIDDoc::DeleteContents()
{
#ifdef NEW_VERSION_TIDCORE
	if(gpTidModel!=NULL)
		CTidModelFactory::Destroy(gpTidModel->GetSerialId());
	gpTidModel=NULL;
#endif
	if(gpModModel!=NULL)
		CModModelFactory::Destroy(gpModModel->GetSerialId());
	gpModModel=NULL;
	//
	CTIDView *pTidView=theApp.GetTIDView();
	if(pTidView)
	{
		ISolidDraw* pSolidDraw=pTidView->SolidDraw();
		if(pSolidDraw)
			pSolidDraw->EmptyDisplayBuffer();
	}
	theApp.hashModelHeights.Empty();
	CDocument::DeleteContents();
}

CView* CTIDDoc::GetView(const CRuntimeClass *pClass)
{
	CView *pView;
	POSITION position;
	position = GetFirstViewPosition();
	for(;;)
	{
		if(position==NULL)
		{
			pView = NULL;
			break;
		}
		pView = GetNextView(position);
		if(pView->IsKindOf(pClass))
			break;
	}
	return pView;
}
void CTIDDoc::OnFileOpen() 
{
	CXhChar500 filter("铁塔三维数据模型文件(*.tid)|*.tid");
	filter.Append("|国网移交几何模型文件(*.mod)|*.mod");
	filter.Append("|所有文件(*.*)|*.*||");
	CFileDialog dlg(TRUE,"lds","LDS",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,filter);
	if(dlg.DoModal()!=IDOK)
		return;
	theApp.m_uiActiveHeightSerial=1;
	OnOpenDocument(dlg.GetPathName());
	UpdateAllViews(NULL);
}
BOOL CTIDDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	char drive[4];
	char dir[MAX_PATH];
	CXhChar50 file_name;
	CXhChar16 extension;
	_splitpath(lpszPathName,drive,dir,file_name,extension);
	SetPathName(lpszPathName);
	SetModifiedFlag();  // dirty during de-serialize
	DeleteContents();
	if(extension.EqualNoCase(".mod"))
	{
		theApp.m_ciFileType=1;	//MOD_FILE
		if(gpModModel==NULL)
			gpModModel=CModModelFactory::CreateModModel();
		gpModModel->ImportModFile(lpszPathName);
		return TRUE;
	}
	if(!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	return TRUE;
}
void CTIDDoc::OnExportSwapInfoFile()
{
	if(gpTidModel==NULL)
		return;
	CXhChar200 fn("%s",GetPathName());
	if(fn.GetLength()==0)
	{
		AfxMessageBox("未打开任何文件，无法导出任何数据交换信息！");
		return;
	}
	char* separator=SearchChar(fn,'.',true);
	if(separator!=NULL)
		*separator=0;
	CFileDialog dlg(FALSE,"mod",fn,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"国网移交几何模型文件(*.mod)|*.mod|所有文件(*.*)|*.*||");
	if(dlg.DoModal()!=IDOK)
		return;
	CString ext=dlg.GetFileExt();
	if(ext.CompareNoCase("mod")==0)
	{
		gpTidModel->ExportModFile(dlg.GetPathName());
		//生成单基塔例的MOD文件
		/*ITidTowerInstance* pInstance=(ITidTowerInstance*)theApp.GetActiveTowerInstance();
		if(pInstance==NULL)
			return;
		pInstance->ExportModFile(dlg.GetPathName());*/
	}
}