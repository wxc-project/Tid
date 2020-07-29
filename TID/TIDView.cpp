// TIDView.cpp : CTIDView 类的实现
//

#include "stdafx.h"
#include "TID.h"
#include "TIDDoc.h"
#include "TIDView.h"
#include "TIDview.h"
#include "CreateFace.h"
#ifdef NEW_VERSION_TIDCORE
#include "TidCplus.h"
#else
#include ".\SolidTowerModel.h"
#endif
#include "SolidBodyBuffer.h"
#include "ArrayList.h"
#include "HashTable.h"
#include "ProcBarDlg.h"
#include "UserDefMsg.h"
#include "CommandDefOper.h"
#include "TowerPropertyDlg.h"
#include "MainFrm.h"
#include "f_alg_fun.h"
#include "direct.h"
#include "btc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CTIDView

IMPLEMENT_DYNCREATE(CTIDView, CView)

BEGIN_MESSAGE_MAP(CTIDView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_MESSAGE(WM_OBJECT_SNAPED, &CTIDView::ObjectSnappedProcess)
	ON_MESSAGE(WM_OBJECT_SELECTED, &CTIDView::ObjectSelectProcess)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_SIZE()
	ON_COMMAND(ID_GL_ALL_ZOOM, OnGlAllZoom)
	ON_COMMAND(ID_GL_OPEN_WINDOW, OnGlOpenWindow)
	ON_UPDATE_COMMAND_UI(ID_GL_OPEN_WINDOW, OnUpdateGlOpenWindow)
	ON_COMMAND(ID_GL_PAN, OnGlPan)
	ON_UPDATE_COMMAND_UI(ID_GL_PAN, OnUpdateGlPan)
	ON_COMMAND(ID_GL_ZOOM, OnGlZoom)
	ON_UPDATE_COMMAND_UI(ID_GL_ZOOM, OnUpdateGlZoom)
	ON_COMMAND(ID_GL_ROTATED, OnGlRotated)
	ON_UPDATE_COMMAND_UI(ID_GL_ROTATED, OnUpdateGlRotated)
	ON_COMMAND(ID_RESET_VIEW, OnResetView)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_SOLID_MODE_DISPLAY, OnSolidModeDisplay)
	ON_UPDATE_COMMAND_UI(ID_SOLID_MODE_DISPLAY, OnUpdateSolidModeDisplay)

END_MESSAGE_MAP()

// CTIDView 构造/析构

CTIDView::CTIDView()
{
	// TODO: 在此处添加构造代码
	m_cCurTask=0;
	m_curTask = TASK_OTHER;
}

CTIDView::~CTIDView()
{
	m_pSolidSet->ExitComponent();
	CDrawSolidFactory::Destroy(m_pDrawSolid->GetSerial());
	m_pSolidDraw=NULL;
	m_pSolidOper=NULL;
	m_pSolidSet=NULL;
	m_pSolidSnap=NULL;
	//
	if(m_pPipeTaskThread)
	{
		DWORD dwExitCode=0;
		GetExitCodeThread(m_pPipeTaskThread,&dwExitCode);
		ExitThread(dwExitCode);
	}
	m_pPipeTaskThread=NULL;
}

BOOL CTIDView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或
	// 样式

	return CView::PreCreateWindow(cs);
}
CProcBarDlg *pProcDlg=NULL;
void DisplayProcess(int percent,const char *sTitle=NULL)
{
	if(percent>=100)
	{
		if(pProcDlg!=NULL)
		{
			pProcDlg->DestroyWindow();
			delete pProcDlg;
			pProcDlg=NULL;
		}
		return;
	}
	else if(pProcDlg==NULL)
		pProcDlg=new CProcBarDlg(NULL);
	if(pProcDlg->GetSafeHwnd()==NULL)
		pProcDlg->Create();
	static int prevPercent;
	if(percent!=0&&percent==prevPercent)
		return;	//跟上次进度一致不需要更新
	else
		prevPercent=percent;
	if(sTitle)
		pProcDlg->SetTitle(CString(sTitle));
	else
#ifdef AFX_TARG_ENU_ENGLISH
		pProcDlg->SetTitle("Process");
#else 
		pProcDlg->SetTitle("进度");
#endif
	pProcDlg->Refresh(percent);
}
CHashList<CSolidBody> hashListBody;
#ifdef NEW_VERSION_TIDCORE
UCS_STRU CastToUcs( TID_CS& tcs)
{
	UCS_STRU cs;
	cs.origin=f3dPoint(tcs.origin);
	cs.axis_x=f3dPoint(tcs.axisX);
	cs.axis_y=f3dPoint(tcs.axisY);
	cs.axis_z=f3dPoint(tcs.axisZ);
	return cs;
}
UCS_STRU cs;
// CTIDView 绘制
static bool BuildSolidBodyFromTidSolidBody(CSolidBody* pSolid,ITidSolidBody* pTidSolid,bool bSplit=false)
{
	if(!bSplit)	
	{	//直接拷贝实体buffer
		pSolid->CopyBuffer(pTidSolid->SolidBufferPtr(),pTidSolid->SolidBufferLength());
		return true;
	}
	//解析实体buffer,重新生成面
	fBody solid;
	TID_COORD3D vertice;
	if(pTidSolid->BasicFaceNum()==0)	//传统模式即原始B-rep模型定义实体
		pTidSolid->SplitToBasicFacets();
	if(pTidSolid->BasicFaceNum()==0)	//传统模式即原始B-rep模型定义实体
	{
		int i,j,vertex_n=pTidSolid->KeyPointNum();
		for(i=0;i<vertex_n;i++)
		{
			vertice=pTidSolid->GetKeyPointAt(i);
			solid.vertex.append(f3dPoint(vertice));
		}
		CCreateFace nf;
		nf.InitVertexList(&solid);
		int face_n=pTidSolid->PolyFaceNum();
		for(i=0;i<face_n;i++)
		{
			f3dPolyFace* pFace=solid.faceList.append();
			IRawSolidFace* pRawTidFace=pTidSolid->GetPolyFaceAt(i);
			pFace->material=pRawTidFace->MatColor();
			pFace->poly_norm=(double*)pRawTidFace->WorkNorm();
			IFaceLoop* pOuterLoop=pRawTidFace->OutterLoop();
			for(j=0;j<pOuterLoop->EdgeLineNum();j++)
			{
				TidArcline arcline;
				pOuterLoop->EdgeLineAt(j,&arcline);
				ITidRawSolidEdge* pEdge=pOuterLoop->EdgeLineAt(j);
				f3dAtomLine* pLine=nf.NewOutterEdgeLine(pFace,pEdge->LineEndId()-1,pEdge->LineStartId()-1);
				if(arcline.SectorAngle()>0)	//圆弧
				{
					if(arcline.ColumnNorm().IsZero())	//普通圆弧
						pLine->CreateArcLine(f3dPoint(arcline.WorkNorm()),arcline.SectorAngle());
					else	//椭圆弧
						pLine->CreateEllipseArcLine(f3dPoint(arcline.Center()),f3dPoint(arcline.ColumnNorm()),f3dPoint(arcline.WorkNorm()),arcline.Radius());
				}
			}
		}
	}
	else
	{
		int i,j,face_n=pTidSolid->BasicFaceNum();
		for(i=0;i<face_n;i++)
		{
			ITidBasicFace* pBasicFace=pTidSolid->GetBasicFaceAt(i);
			int cluster_n=pBasicFace->FacetClusterNumber();
			for(j=0;j<cluster_n;j++)
			{
				IFacetCluster* pFacetCluster=pBasicFace->FacetClusterAt(j);
				int k,facet_n=pFacetCluster->FacetNumber();
				DYN_ARRAY<f3dPoint*> vptrs(pFacetCluster->VertexNumber());
				for(int jj=0;jj<pFacetCluster->VertexNumber();jj++)
				{
					TID_COORD3D v=pFacetCluster->VertexAt(jj);
					vptrs[jj]=solid.vertex.append(v.x,v.y,v.z);
				}
				for(k=0;k<facet_n;k++)
				{
					int vi[3];
					TID_COORD3D vv[3];
					if(pFacetCluster->Mode()==IFacetCluster::TRIANGLES)
					{
						for(int ii=0;ii<3;ii++)
							vi[ii]=k*3+ii;
					}
					else if(pFacetCluster->Mode()==IFacetCluster::TRIANGLE_STRIP)
					{
						if(k%2==0)
						{
							for(int ii=0;ii<3;ii++)
								vi[ii]=k+ii;
						}
						else
						{
							vi[0]=k;
							vi[1]=k+2;
							vi[2]=k+1;
						}
					}
					else if(pFacetCluster->Mode()==IFacetCluster::TRIANGLE_FAN)
					{
						vi[0]=0;
						vi[1]=k+1;
						vi[2]=k+2;
					}
					else
						continue;
					f3dPolyFace* pFace=solid.faceList.append();
					pFace->material=pBasicFace->Color();
					pFace->poly_norm=(double*)pFacetCluster->Normal();
					pFace->outer_edge.append(vptrs[vi[0]],vptrs[vi[1]]);
					pFace->outer_edge.append(vptrs[vi[1]],vptrs[vi[2]]);
					pFace->outer_edge.append(vptrs[vi[2]],vptrs[vi[0]]);
				}
			}
		}
	}
	pSolid->ConvertFrom(&solid);
	solid.Empty();
	return true;
}
#endif
static CTIDView* _pLocalCurrView=NULL;
bool FireEntSnapPosition(long ID,const double* front_pos,const double* back_pos,double* nearest_pos)
{
	if(theApp.m_ciFileType==1)
		return false;
	ITidTowerInstance* pTowerInstance=(ITidTowerInstance*)theApp.GetActiveTowerInstance();
	if(pTowerInstance==NULL)
		return false;
	bool bFindMatchPart=false;
	UINT serial=1;
	f3dPoint front(front_pos),back(back_pos);
	f3dPoint depth_vec=back-front;
	normalize(depth_vec);
	GEPOINT inters;
	DISPLAY_TYPE display_type;
	for(ITidAssemblePart* pAssmPart=pTowerInstance->EnumAssemblePartFirst();!bFindMatchPart&&pAssmPart;pAssmPart=pTowerInstance->EnumAssemblePartNext(),serial++)
	{
		if(ID!=pAssmPart->GetId())
			continue;
		bFindMatchPart=true;
		ITidPart* pTidPart=pAssmPart->GetPart();
		if( pTidPart->GetPartType()==ITidPart::TYPE_ANGLE||pTidPart->GetPartType()==ITidPart::TYPE_TUBE||
			pTidPart->GetPartType()==ITidPart::TYPE_FLAT||pTidPart->GetPartType()==ITidPart::TYPE_SLOT)
		{
			GECS cs;
			f3dLine rodline;
			TID_CS tidcs=pAssmPart->GetAcs();
			_pLocalCurrView->SolidSet()->GetDisplayType(&display_type);
			if(display_type==DISP_LINE&&pAssmPart->IsHasBriefRodLine())
			{
				rodline.startPt=f3dPoint(pAssmPart->BriefLineStart());
				rodline.endPt  =f3dPoint(pAssmPart->BriefLineEnd());
			}
			else
			{
				rodline.startPt=f3dPoint(tidcs.origin);
				rodline.endPt  =rodline.startPt+f3dPoint(GEPOINT(tidcs.axisZ)*pTidPart->GetLength());
			}
			cs.origin=rodline.startPt;
			cs.axis_x=rodline.endPt-rodline.startPt;
			cs.axis_z=cs.axis_x^depth_vec;
			normalize(cs.axis_x);
			normalize(cs.axis_z);
			cs.axis_y=cs.axis_z^cs.axis_x;
			front=cs.TransPToCS(front);
			back=cs.TransPToCS(back);
			front.z=back.z=0;
			double length=DISTANCE(rodline.startPt,rodline.endPt);
			int rslt=Int2dll(f2dLine(f2dPoint(0,0),f2dPoint(length,0)),f2dLine(front,back),inters.x,inters.y);
			if(rslt==0||rslt==-1)
				return false;
			inters=cs.TransPFromCS(inters);
		}
		else if(pAssmPart->GetPart()->GetPartType()==ITidPart::TYPE_PLATE)
		{
			TID_CS tidcs=pAssmPart->GetAcs();
			GEPOINT axis_z=GEPOINT(tidcs.axisZ),origin=GEPOINT(tidcs.origin);
			if(fabs(axis_z*depth_vec)>0.717)
			{
				if(Int3dlf(inters,front,depth_vec,origin+axis_z*(pAssmPart->GetPart()->GetThick()),axis_z)!=1)
					inters=origin;
			}
			else
				inters=origin;
		}
		else
			inters=GEPOINT(pAssmPart->GetAcs().origin);
		break;
	}
	for(ITidAssembleBolt* pAssmBolt=pTowerInstance->EnumAssembleBoltFirst();!bFindMatchPart&&pAssmBolt;pAssmBolt=pTowerInstance->EnumAssembleBoltNext(),serial++)
	{
		if(ID!=serial)
			continue;
		bFindMatchPart=true;
		inters=GEPOINT(pAssmBolt->GetAcs().origin);
		break;
	}
	nearest_pos[0]=inters.x;
	nearest_pos[1]=inters.y;
	nearest_pos[2]=inters.z;
	return true;
}
void DrawSolidTower(void* pContext)
{
	CTIDView* pView=(CTIDView*)pContext;
	if(pView==NULL)
		pView=theApp.GetTIDView();
	if(pView==NULL)
		return;
	pView->SolidDraw()->EmptyDisplayBuffer();
	DISPLAY_TYPE disp_type;
	pView->SolidSet()->GetDisplayType(&disp_type);
	if(theApp.m_ciFileType==1)
	{
		IModTowerInstance* pTowerInstance=(IModTowerInstance*)theApp.GetActiveTowerInstance();
		if(pTowerInstance==NULL)
			return;
		UCS_STRU ucs;
		ucs.axis_x.Set(1,0,0);
		ucs.axis_y.Set(0,0,1);
		ucs.axis_z.Set(0,-1,0);
		pView->SolidSet()->SetObjectUcs(ucs);
		DWORD serial=1;
		int nRod=pTowerInstance->GetModRodNum();
		if(disp_type==DISP_SOLID)
		{
			DisplayProcess(0,"生成实体模型....");
			for(IModRod* pRod=pTowerInstance->EnumModRodFir();pRod;pRod=pTowerInstance->EnumModRodNext(),serial++)
			{
				DisplayProcess(ftoi(serial*100/nRod),"生成实体模型...");
				//
				CSolidPart xSolidPart;
				xSolidPart.m_hPart=pRod->GetId();
				xSolidPart.pBody=hashListBody.Add(serial);
				pRod->Create3dSolidModel(xSolidPart.pBody);
				pView->SolidDraw()->NewSolidPart(xSolidPart);
			}
			DisplayProcess(100,"生成实体模型....");
		}
		else
		{
			DisplayProcess(0,"生成单线模型...");
			for(IModRod* pRod=pTowerInstance->EnumModRodFir();pRod;pRod=pTowerInstance->EnumModRodNext(),serial++)
			{
				DisplayProcess(ftoi(serial*100/nRod),"生成单线模型...");
				//
				MOD_LINE mod_line=pRod->GetBaseLineToLdsModel();
				f3dLine line;
				line.startPt.Set(mod_line.startPt.x,mod_line.startPt.y,mod_line.startPt.z);
				line.endPt.Set(mod_line.endPt.x,mod_line.endPt.y,mod_line.endPt.z);
				line.pen.crColor=RGB(220,220,220);
				line.ID=pRod->GetId();
				pView->SolidDraw()->NewLine(line);
			}
			DisplayProcess(100,"生成单线模型...");
		}
		//绘制挂点信息
		MOD_CS cs = gpModModel->BuildUcsByModCS();
		UCS_STRU lds_ucs;
		lds_ucs.origin.Set(cs.origin.x,cs.origin.y,cs.origin.z);
		lds_ucs.axis_x.Set(cs.axisX.x, cs.axisX.y, cs.axisX.z);
		lds_ucs.axis_y.Set(cs.axisY.x, cs.axisY.y, cs.axisY.z);
		lds_ucs.axis_z.Set(cs.axisZ.x, cs.axisZ.y, cs.axisZ.z);
		for (int i = 0; i < gpModModel->GetHangNodeNum(); i++)
		{
			MOD_HANG_NODE* pHangPt = gpModModel->GetHangNodeById(i);
			if (pHangPt == NULL)
				return;
			GEPOINT org(pHangPt->m_xHangPos), vec;
			coord_trans(org, lds_ucs, TRUE);
			if (pHangPt->GetHangPosSymbol() == 0)
				vec.Set(0, 0, 1);	//悬垂挂线默认铅垂向下
			else if (pHangPt->GetHangPosSymbol() == 'Q')
			{
				if (pHangPt->m_ciWireDirect == 'X')
					vec.Set(-1, 0, 0);
				else
					vec.Set(0, -1, 0);
			}
			else //if(pHangPt->GetHangPosSymbol() == 'H')
			{
				if (pHangPt->m_ciWireDirect == 'X')
					vec.Set(1, 0, 0);
				else
					vec.Set(0, 1, 0);
			}
			//绘制箭头
			pView->SolidSet()->SetPen(RGB(0, 255, 255), PS_SOLID, 2);
			double dfArrowLen = 2000;
			CSolidPart solidpart;
			solidpart.m_bDisplaySingle = true;
			solidpart.m_iLineType = 3;
			solidpart.dfSphereRadius = 0;
			solidpart.line.startPt = org;
			solidpart.line.endPt = org + vec *dfArrowLen;
			pView->SolidDraw()->NewSolidPart(solidpart);
			//绘制文字
			pView->SolidSet()->SetPen(RGB(255, 255, 0), PS_SOLID, 2);
			f3dPoint dimpos = org + vec *dfArrowLen*0.5, align_vec(vec);
			pView->SolidDraw()->NewText(pHangPt->m_sHangName, dimpos, 160, align_vec, f3dPoint(0, 0, 0), ISolidDraw::TEXT_ALIGN_BTMCENTER, 2);
		}
		pView->SolidOper()->ZoomAll(0.95);
		return;
	}
#ifdef NEW_VERSION_TIDCORE
	if(gpTidModel==NULL || gpTidModel->HeightGroupCount()==0)
		return;
	UCS_STRU cs=CastToUcs(gpTidModel->ModelCoordSystem());
	pView->SolidSet()->SetObjectUcs(cs);
	//根据配基信息获取一桩塔例
	ITidTowerInstance* pTowerInstance=(ITidTowerInstance*)theApp.GetActiveTowerInstance();
	if(pTowerInstance==NULL)
		return;
	int nPart=pTowerInstance->GetAssemblePartNum();
	int nBolt=pTowerInstance->GetAssembleBoltNum();
	int nAnchorBolt=pTowerInstance->GetAssembleAnchorBoltNum();
	int nSum=nPart+nBolt+nAnchorBolt;
	if(disp_type==DISP_SOLID)
	{	//绘制实体
		DisplayProcess(0,"生成实体模型....");
		DWORD serial=1;
		for(ITidAssemblePart* pAssmPart=pTowerInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pTowerInstance->EnumAssemblePartNext(),serial++)
		{
			CSolidPart solidpart;
			solidpart.m_hPart=pAssmPart->GetId();
			solidpart.pBody=hashListBody.Add(serial);
			if(pAssmPart->IsHasBriefRodLine())
			{
				TID_COORD3D ptS=pAssmPart->BriefLineStart();
				TID_COORD3D ptE=pAssmPart->BriefLineEnd();
				solidpart.line.startPt.Set(ptS.x,ptS.y,ptS.z);
				solidpart.line.endPt.Set(ptE.x,ptE.y,ptE.z);
				solidpart.m_iLineType=1;
				solidpart.m_bDisplaySingle=true;
			}
			ITidSolidBody* pSolidBody=pAssmPart->GetSolidPart();
			if(BuildSolidBodyFromTidSolidBody(solidpart.pBody,pSolidBody))
				pView->SolidDraw()->NewSolidPart(solidpart);
			pSolidBody->ReleaseTemporaryBuffer();
			DisplayProcess(ftoi(100*serial/nSum),"生成杆件实体模型....");
		}
		for(ITidAssembleBolt* pAssmBolt=pTowerInstance->EnumAssembleBoltFirst();pAssmBolt;pAssmBolt=pTowerInstance->EnumAssembleBoltNext(),serial++)
		{
			DisplayProcess(ftoi(100*serial/nSum),"生成螺栓实体模型....");
			ITidSolidBody* pSolidBody=pAssmBolt->GetBoltSolid();
			ITidSolidBody* pNutSolid=pAssmBolt->GetNutSolid();
			if(pSolidBody==NULL||pNutSolid==NULL)
				continue;
			CSolidPart solidpart;
			solidpart.m_hPart=pAssmBolt->GetId();
			solidpart.pBody=hashListBody.Add(serial);
			if(!BuildSolidBodyFromTidSolidBody(solidpart.pBody,pSolidBody))
				continue;
			CSolidBody nutsolid;
			if(BuildSolidBodyFromTidSolidBody(&nutsolid,pNutSolid))
				solidpart.pBody->MergeBodyBuffer(nutsolid.BufferPtr(),nutsolid.BufferLength());
			pView->SolidDraw()->NewSolidPart(solidpart);
			//
			pSolidBody->ReleaseTemporaryBuffer();
			pNutSolid->ReleaseTemporaryBuffer();
		}
		for(ITidAssembleAnchorBolt* pAssmAnchorBolt=pTowerInstance->EnumFirstAnchorBolt();pAssmAnchorBolt;pAssmAnchorBolt=pTowerInstance->EnumNextAnchorBolt(),serial++)
		{
			DisplayProcess(ftoi(100*serial/nSum),"生成地脚螺栓实体模型....");
			ITidSolidBody* pSolidBody=pAssmAnchorBolt->GetBoltSolid();
			ITidSolidBody* pNutSolid=pAssmAnchorBolt->GetNutSolid();
			if(pSolidBody==NULL||pNutSolid==NULL)
				continue;
			CSolidPart solidpart;
			solidpart.m_hPart=pAssmAnchorBolt->GetId();
			solidpart.pBody=hashListBody.Add(serial);
			if(!BuildSolidBodyFromTidSolidBody(solidpart.pBody,pSolidBody))
				continue;
			CSolidBody nutsolid;
			if(BuildSolidBodyFromTidSolidBody(&nutsolid,pNutSolid))
				solidpart.pBody->MergeBodyBuffer(nutsolid.BufferPtr(),nutsolid.BufferLength());
			pView->SolidDraw()->NewSolidPart(solidpart);
			//
			pSolidBody->ReleaseTemporaryBuffer();
			pNutSolid->ReleaseTemporaryBuffer();
		}
		DisplayProcess(100,"生成实体模型....");
	}
	else if(disp_type==DISP_LINE)
	{	//绘制单线
		f3dLine line;
		line.pen.crColor=RGB(150,150,255);
		line.pen.width=1;
		line.pen.style=PS_SOLID;
		DisplayProcess(0,"生成单线实体...");
		int index=0;
		for(ITidAssemblePart* pAssmPart=pTowerInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pTowerInstance->EnumAssemblePartNext(),index++)
		{
			DisplayProcess(ftoi(index*100/nPart),"生成单线实体...");
			if(!pAssmPart->IsHasBriefRodLine())
				continue;
			ITidNode* pNodeS=pTowerInstance->FindNode(pAssmPart->GetStartNodeId());
			ITidNode* pNodeE=pTowerInstance->FindNode(pAssmPart->GetEndNodeId());
			if(pNodeS==NULL||pNodeE==NULL)
				continue;	//短角钢
			TID_COORD3D ptS=pNodeS->GetPos();//pAssmPart->BriefLineStart();
			TID_COORD3D ptE=pNodeE->GetPos();//pAssmPart->BriefLineEnd();
			line.startPt.Set(ptS.x,ptS.y,ptS.z);
			line.endPt.Set(ptE.x,ptE.y,ptE.z);
			line.ID=pAssmPart->GetId();
			pView->SolidDraw()->NewLine(line);
		}
		DisplayProcess(100,"生成单线实体...");
	}
#ifdef __ALFA_TEST_
	COLORREF clr = RGB(152, 152, 152);
	double fZeroZ = gpTidModel->GetNamedHeightZeroZ();
	double fHeight = pTowerInstance->GetInstanceHeight();
	double fTransZ = pTowerInstance->BelongHeightGroup()->GetBody2LegTransitZ();
	//实际呼高
	btc::SKETCH_PLANE sketch1, sketch2;
	sketch1.CreateStdPlane(GEPOINT(10000,0, fZeroZ), GEPOINT(0, 0, 1), GEPOINT(1, 0, 0), 20000);
	pView->SolidDraw()->NewWorkPlaneSketch(1, clr, sketch1.pVertexArr, sketch1.VertexCount, sketch1.normal, "呼高");
	sketch2.CreateStdPlane(GEPOINT(10000,0, fHeight), GEPOINT(0, 0, -1), GEPOINT(1, 0, 0), 20000);
	pView->SolidDraw()->NewWorkPlaneSketch(2, clr, sketch2.pVertexArr, sketch2.VertexCount, sketch2.normal,CXhChar16("%g",fHeight-fZeroZ));
	//接身高
	btc::SKETCH_PLANE sketch3, sketch4;
	sketch3.CreateStdPlane(GEPOINT(-10000, 0, fZeroZ), GEPOINT(0, 0, 1), GEPOINT(1, 0, 0), 20000);
	pView->SolidDraw()->NewWorkPlaneSketch(3, clr, sketch3.pVertexArr, sketch3.VertexCount, sketch3.normal, "接身高");
	sketch4.CreateStdPlane(GEPOINT(-10000, 0, fTransZ), GEPOINT(0, 0, -1), GEPOINT(1, 0, 0), 20000);
	pView->SolidDraw()->NewWorkPlaneSketch(4, clr, sketch4.pVertexArr, sketch4.VertexCount, sketch4.normal,CXhChar16("%g", fTransZ - fZeroZ));
	//腿高
	btc::SKETCH_PLANE sketch5, sketch6;
	sketch5.CreateStdPlane(GEPOINT(0, 0, fTransZ), GEPOINT(0, 0, 1), GEPOINT(1, 0, 0), 20000);
	pView->SolidDraw()->NewWorkPlaneSketch(5, clr, sketch5.pVertexArr, sketch5.VertexCount, sketch5.normal, "腿长");
	sketch6.CreateStdPlane(GEPOINT(0, 0, fHeight), GEPOINT(0, 0, -1), GEPOINT(1, 0, 0), 20000);
	pView->SolidDraw()->NewWorkPlaneSketch(6, clr, sketch6.pVertexArr, sketch6.VertexCount, sketch6.normal, CXhChar16("%g", fHeight - fTransZ));
#endif
	//绘制挂点信息
	for (DWORD i = 0; i < gpTidModel->HangPointCount(); i++)
	{
		ITidHangPoint* pHangPt = gpTidModel->GetHangPointAt(i);
		if (pHangPt == NULL)
			return;
		GECS acs;
		acs.origin = GEPOINT(pHangPt->GetPos());
		if(pHangPt->GetPosSymbol()==0)
			acs.axis_z.Set(0, 0, 1);	//悬垂挂线默认铅垂向下
		else if(pHangPt->GetPosSymbol() == 'Q')
		{
			if (pHangPt->GetWireDirection() == 'X')
				acs.axis_z.Set(-1, 0, 0);
			else
				acs.axis_z.Set(0, -1, 0);
		}
		else //if(pHangPt->GetPosSymbol() == 'H')
		{
			if (pHangPt->GetWireDirection() == 'X')
				acs.axis_z.Set(1, 0, 0);
			else
				acs.axis_z.Set(0, 1, 0);
		}
		//绘制箭头
		pView->SolidSet()->SetPen(RGB(0, 255, 255), PS_SOLID, 2);
		double dfArrowLen= 2000;
		CSolidPart solidpart;
		solidpart.acs = acs;
		solidpart.m_bDisplaySingle = true;
		solidpart.m_iLineType = 3;
		solidpart.dfSphereRadius = 0;
		solidpart.line.startPt = acs.origin;
		solidpart.line.endPt = acs.origin + acs.axis_z*dfArrowLen;
		pView->SolidDraw()->NewSolidPart(solidpart);
		//绘制文字
		pView->SolidSet()->SetPen(RGB(255, 255,0), PS_SOLID, 2);
		CXhChar50 szWireName;
		pHangPt->GetWireDescription(szWireName);
		f3dPoint dimpos = acs.origin + acs.axis_z*dfArrowLen*0.5, align_vec(acs.axis_z);
		pView->SolidDraw()->NewText(szWireName, dimpos, 160, align_vec, f3dPoint(0,0,0), ISolidDraw::TEXT_ALIGN_BTMCENTER, 2);
	}
#else
	pView->SolidSet()->SetObjectUcs(model.ModelCoordSystem());
	if(model.GetLength()==0)
		return;	//模型缓存未初始化
	CModuleSection modulesect=model.ModuleSection();
	TOWER_MODULE module=modulesect.GetModuleAt(0);
	CBoltSection boltsect=model.BoltSection();
	CPartSection partsect=model.PartSection();
	CBlockSection blksect=model.BlockSection();
	CAssembleSection assemblysect=model.AssembleSection();
	CPartAssemblySection assemble_parts=assemblysect.PartSection();
	CBoltAssemblySection assemble_bolts=assemblysect.BoltSection();
	DWORD i,baseId=0;
	DWORD assemble_parts_n=assemble_parts.AssemblyCount();
	DWORD assemble_bolts_n=assemble_bolts.AssemblyCount();
	CSolidPart solidpart;
	for(i=1;i<=assemble_parts_n;i++)
	{
		PART_ASSEMBLY assembly=assemble_parts.GetAssemblyAt(i);
		PART_INFO part=partsect.GetPartInfoAt(assembly.dwIndexId);
		solidpart.m_hPart=i;
		solidpart.pBody=hashListBody.Add(i);
		solidpart.pBody->CopyBuffer(part.solid.BufferPtr(),part.solid.BufferLength());
		solidpart.pBody->TransToACS(assembly.acs);
		pView->SolidDraw()->NewSolidPart(solidpart);
	}
	baseId=assemble_parts_n;
	for(i=1;i<=assemble_bolts_n;i++)
	{
		BOLT_ASSEMBLY assembly=assemble_bolts.GetAssemblyAt(i);
		CBoltSeries series=boltsect.GetBoltSeriesAt(assembly.cSeriesId-1);
		BOLT_INFO bolt;
		if(assembly.wIndexId==0)
			bolt=series.GetBoltSizeAt(0);
		else
			bolt=series.GetBoltSizeAt(assembly.wIndexId-1);
		bolt.solidOfBolt.ToInternalBuffer();
		bolt.solidOfCap.ToInternalBuffer();
		UCS_STRU acs;
		acs.origin=assembly.origin;
		acs.axis_z=assembly.work_norm;
		acs.axis_x=inters_vec(f3dPoint(assembly.work_norm));
		acs.axis_y=acs.axis_z^acs.axis_x;
		normalize(acs.axis_y);
		acs.axis_x=acs.axis_y^acs.axis_z;
		normalize(acs.axis_x);
		bolt.solidOfBolt.TransToACS(acs);
		acs.origin+=acs.axis_z*assembly.wL0;
		bolt.solidOfCap.TransToACS(acs);
		CSolidBodyBuffer solidbuf;
		solidbuf.MergeBodyBuffer(bolt.solidOfBolt.BufferPtr(),bolt.solidOfBolt.BufferLength());
		solidbuf.MergeBodyBuffer(bolt.solidOfCap.BufferPtr(),bolt.solidOfCap.BufferLength());
		solidpart.pBody=hashListBody.Add(baseId+i);
		solidpart.pBody->CopyBuffer(solidbuf.GetBufferPtr(),solidbuf.GetLength());
		solidpart.m_hPart=baseId+i;
		pView->SolidDraw()->NewSolidPart(solidpart);
	}
	baseId+=assemble_bolts_n;
	//初始化部件块的整合实体
	DYN_ARRAY<TOWER_BLOCK> blockArr(blksect.GetBlockCount());
	for(i=0;i<blksect.GetBlockCount();i++)
		blockArr[i]=blksect.GetBlockAt(i);
	DWORD block_assemble_parts_n=assemble_parts.AssemblyCount(false);
	DWORD block_assemble_bolts_n=assemble_bolts.AssemblyCount(false);
	for(i=1;i<=block_assemble_parts_n;i++)
	{
		PART_ASSEMBLY assembly=assemble_parts.GetAssemblyAt(i,false);
		PART_INFO part=partsect.GetPartInfoAt(assembly.dwIndexId);
		CSolidBody body(part.solid.BufferPtr(),part.solid.BufferLength());
		body.TransToACS(assembly.acs);
		blockArr[assembly.wBlockIndexId-1].solid.MergeBodyBuffer(body.BufferPtr(),body.BufferLength());
	}
	for(i=1;i<=block_assemble_bolts_n;i++)
	{
		BOLT_ASSEMBLY assembly=assemble_bolts.GetAssemblyAt(i,false);
		CBoltSeries series=boltsect.GetBoltSeriesAt(assembly.cSeriesId-1);
		BOLT_INFO bolt;
		if(assembly.wIndexId==0)
			bolt=series.GetBoltSizeAt(0);
		else
			bolt=series.GetBoltSizeAt(assembly.wIndexId-1);
		bolt.solidOfBolt.ToInternalBuffer();
		bolt.solidOfCap.ToInternalBuffer();
		UCS_STRU acs;
		acs.origin=assembly.origin;
		acs.axis_z=assembly.work_norm;
		acs.axis_x=inters_vec(f3dPoint(assembly.work_norm));
		acs.axis_y=acs.axis_z^acs.axis_x;
		normalize(acs.axis_y);
		acs.axis_x=acs.axis_y^acs.axis_z;
		normalize(acs.axis_x);
		bolt.solidOfBolt.TransToACS(acs);
		acs.origin+=acs.axis_z*assembly.wL0;
		bolt.solidOfCap.TransToACS(acs);
		blockArr[assembly.wBlockIndexId-1].solid.MergeBodyBuffer(bolt.solidOfBolt.BufferPtr(),bolt.solidOfBolt.BufferLength());
		blockArr[assembly.wBlockIndexId-1].solid.MergeBodyBuffer(bolt.solidOfCap.BufferPtr(),bolt.solidOfCap.BufferLength());
	}

	DWORD assemble_block_n=assemblysect.BlockAssemblyCount();
	for(i=1;i<=assemble_block_n;i++)
	{
		BLOCK_ASSEMBLY assembly=assemblysect.GetAssemblyAt(i);
		solidpart.m_hPart=i+baseId;
		solidpart.pBody=hashListBody.Add(i+baseId);
		solidpart.pBody->CopyBuffer(blockArr[assembly.wIndexId-1].solid.BufferPtr(),blockArr[assembly.wIndexId-1].solid.BufferLength());
		solidpart.pBody->TransACS(blockArr[assembly.wIndexId-1].lcs,assembly.acs);
		pView->SolidDraw()->NewSolidPart(solidpart);
	}
#endif
}
void CTIDView::OnDraw(CDC* /*pDC*/)
{
	CTIDDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	m_pSolidDraw->Draw();
}


// CTIDView 打印

BOOL CTIDView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CTIDView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 打印前添加额外的初始化
}

void CTIDView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 打印后添加清除过程
}


// CTIDView 诊断

#ifdef _DEBUG
void CTIDView::AssertValid() const
{
	CView::AssertValid();
}

void CTIDView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTIDDoc* CTIDView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTIDDoc)));
	return (CTIDDoc*)m_pDocument;
}
#endif //_DEBUG

void GetSysPath(char* startPath)
{
	char drive[4];
	char dir[MAX_PATH];
	char fname[MAX_PATH];
	char ext[MAX_PATH];

	_splitpath(__argv[0], drive, dir, fname, ext);
	strcpy(startPath, drive);
	strcat(startPath, dir);
	_chdir(startPath);
}
// CTIDView 消息处理程序
int CTIDView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	_pLocalCurrView=this;
	m_pDrawSolid=CDrawSolidFactory::CreateDrawEngine();
	m_pSolidDraw=m_pDrawSolid->SolidDraw();
	m_pSolidSet=m_pDrawSolid->SolidSet();
	m_pSolidSnap=m_pDrawSolid->SolidSnap();
	m_pSolidOper=m_pDrawSolid->SolidOper();
	m_pSolidSet->Init(m_hWnd);
	m_pSolidSet->SetBkColor(RGB(0,64,160));
	m_pSolidSet->SetEntSnapPosFunc(FireEntSnapPosition);
	char sFontFile[MAX_PATH], sBigFontFile[MAX_PATH], sAppPath[MAX_PATH];
	GetSysPath(sAppPath);
	sprintf(sFontFile, "%s\\sys\\simplex.shx", sAppPath);
	bool bRetCode = m_pSolidSet->SetShxFontFile(sFontFile);
	sprintf(sBigFontFile, "%s\\sys\\GBHZFS.shx", sAppPath);
	bRetCode = m_pSolidSet->SetBigFontFile(sBigFontFile);
	return 0;
}

void CTIDView::OnInitialUpdate()
{
	//m_pSolidSet->SetDelObjectFunc(ObjectDeleteFunc);
	m_pSolidSet->SetDisplayType(DISP_SOLID);
	m_pSolidSet->SetDisplayFunc(DrawSolidTower);
	m_pSolidSet->SetSolidAndLineStatus(TRUE);
	m_pSolidSet->SetArcSamplingLength(5);
	m_pSolidSet->SetSmoothness(36);
	m_pSolidDraw->InitialUpdate();
	//g_pSolidDraw->AddCS(2,work_ucs);
	if(theApp.m_bChildProcess)
		m_pPipeTaskThread=AfxBeginThread(StartPipeTaskListen,this,THREAD_PRIORITY_NORMAL);
	CView::OnInitialUpdate();
}
void CTIDView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	CTowerTreeDlg *pTreePage=((CMainFrame*)AfxGetMainWnd())->GetTowerTreePage();
	if(pTreePage)
		pTreePage->RefreshTreeCtrl();
	m_pSolidDraw->BuildDisplayList(this);
	m_pSolidOper->ZoomAll(0.95);
	m_pSolidOper->ReSetView();
}

void CTIDView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	m_pSolidOper->ReSize();
}

void CTIDView::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();	//锁定光标在当前窗口
	if(nFlags&MK_SHIFT)
	{
		m_pSolidDraw->ReleaseSnapStatus();
		m_pSolidDraw->BatchClearCS(4);
	}
	m_pSolidOper->LMouseDown(point);

	CView::OnLButtonDown(nFlags, point);
}

void CTIDView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_pSolidOper->LMouseUp(point);
	if(GetCapture()==this)
		ReleaseCapture();

	CView::OnLButtonUp(nFlags, point);
}

void CTIDView::OnMouseMove(UINT nFlags, CPoint point)
{
	m_pSolidOper->MouseMove(point,nFlags);

	CView::OnMouseMove(nFlags, point);
}

BOOL CTIDView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	ZOOM_CENTER_STYLE zoom_style;
	m_pSolidSet->GetZoomStyle(&zoom_style);
	m_pSolidSet->SetZoomStyle(MOUSE_CENTER);
	if(zDelta>0)
		m_pSolidOper->Scale(1.4);
	else
		m_pSolidOper->Scale(1/1.4);
	m_pSolidOper->RefreshScope();
	m_pSolidSet->SetZoomStyle(zoom_style);

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CTIDView::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_pSolidOper->RMouseDown(point);

	CView::OnRButtonDown(nFlags, point);
}

void CTIDView::OnGlAllZoom()
{
	m_pSolidOper->ZoomAll(0.95);
}

void CTIDView::OnGlOpenWindow()
{
	m_pSolidSet->SetOperType(OPER_ZOOMWND);
}

void CTIDView::OnUpdateGlOpenWindow(CCmdUI *pCmdUI)
{
	OPER_TYPE oper_type;
	oper_type = m_pSolidSet->GetOperType();
	if(oper_type==OPER_ZOOMWND)
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

void CTIDView::OnGlPan()
{
	m_pSolidSet->SetOperType(OPER_PAN);	
}

void CTIDView::OnUpdateGlPan(CCmdUI *pCmdUI)
{
	OPER_TYPE oper_type;
	oper_type = m_pSolidSet->GetOperType();
	if(oper_type==OPER_PAN)
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

void CTIDView::OnGlZoom()
{
	m_pSolidSet->SetOperType(OPER_ZOOM);	
}

void CTIDView::OnUpdateGlZoom(CCmdUI *pCmdUI)
{
	OPER_TYPE oper_type;
	oper_type = m_pSolidSet->GetOperType();
	if(oper_type==OPER_ZOOM)
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

void CTIDView::OnGlRotated()
{
	m_pSolidSet->SetOperType(OPER_ROTATE);	
}

void CTIDView::OnUpdateGlRotated(CCmdUI *pCmdUI)
{
	OPER_TYPE oper_type;
	oper_type = m_pSolidSet->GetOperType();
	if(oper_type==OPER_ROTATE)
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

void CTIDView::OnSolidModeDisplay() 
{
	DISPLAY_TYPE disp;
	m_pSolidSet->GetDisplayType(&disp);
	//切换视图显示类型
	if(disp==DISP_LINE)
		m_pSolidSet->SetDisplayType(DISP_SOLID);
	else if(disp==DISP_SOLID)
		m_pSolidSet->SetDisplayType(DISP_LINE);
	m_pSolidDraw->BuildDisplayList();
	m_pSolidDraw->Draw();
}

void CTIDView::OnUpdateSolidModeDisplay(CCmdUI* pCmdUI) 
{
	DISPLAY_TYPE type;
	if(m_pSolidSet)
		m_pSolidSet->GetDisplayType(&type);
	pCmdUI->SetCheck(type==DISP_SOLID);
}

void CTIDView::OnResetView()
{
	m_pSolidOper->ReSetView();
}

void CTIDView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	m_pSolidSet->SetOperType(OPER_ROTATE);	//为便捷操作，双击时激活旋转操作
	//BOOL bAltKeyDown=GetKeyState(VK_MENU)&0x8000;
	if((nFlags&MK_CONTROL)==0)	//只有未按下Control键时的双击才会激发双击更改旋转中心操作
		m_pSolidOper->LMouseDoubleClick(point);
	CView::OnLButtonDblClk(nFlags, point);
}

LRESULT CTIDView::ObjectSnappedProcess(WPARAM ID, LPARAM ent_type)
{
	if(ID==0&&ent_type==0)
		return 0;	//空点鼠标左键不做任何处理
	switch(m_cCurTask)
	{
	case TASK_OPEN_WND_SEL:
		UpdatePropertyPage();	//更新属性对话框
		OperOther();
		break;
	default:
		UpdatePropertyPage();	//更新属性对话框
		break;
	}
	return 0;
}

LRESULT CTIDView::ObjectSelectProcess(WPARAM nSelect, LPARAM other)
{
	switch(m_cCurTask)
	{
	case TASK_OPEN_WND_SEL:
		UpdatePropertyPage();	//更新属性对话框
		OperOther();
		break;
	default:
		UpdatePropertyPage();	//更新属性对话框
		break;
	}
	return 0;
}

void CTIDView::UpdatePropertyPage()
{
	CTowerPropertyDlg *pPropDlg=((CMainFrame*)AfxGetMainWnd())->GetTowerPropertyPage();
	if(pPropDlg==NULL||!pPropDlg->IsWindowVisible())
		return;
	CPropertyList *pPropList=pPropDlg->GetPropertyList();
	if(pPropList->IsLocked())
		return;
	long *id_arr,id_num = m_pSolidSnap->GetLastSelectEnts(id_arr);
	if(id_num<=0)
		return;
	if(theApp.m_ciFileType==1)
		return;
	ITidTowerInstance* pTowerInstance=(ITidTowerInstance*)theApp.GetActiveTowerInstance();
	if(pTowerInstance==NULL)
		return;
	ITidAssemblePart* pAssemblePart=pTowerInstance->FindAssemblePart(id_arr[0]);
	ITidAssembleBolt* pAssembleBolt=pTowerInstance->FindAssembleBolt(id_arr[0]);
	ITidAssembleAnchorBolt* pAssembleAnchor=pTowerInstance->FindAnchorBolt(id_arr[0]);
	if(pAssemblePart)
		DisplayPartProperty(pAssemblePart);
	if(pAssembleBolt)
		DisplayBoltProperty(pAssembleBolt);
	if(pAssembleAnchor)
		DisplayAnchorBoltProperty(pAssembleAnchor);
}

void CTIDView::OperOther()
{
	m_pSolidSnap->SetSnapType(SNAP_ALL);
	m_pSolidSet->SetOperType(OPER_OTHER);
	m_curTask = TASK_OTHER;
}

void CTIDView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar==VK_ESCAPE)
		OperOther();
	//在nChar==VK_DELETE时返回实际删除的构件数量,用以处理删除当前未显示构件的情况
	m_pSolidOper->KeyDown(nChar);
	//构件编辑器中的图元信息通过属性栏显示，按ESC需要更新属性栏
	if(nChar==VK_DELETE||nChar==VK_ESCAPE)
	{
		m_pSolidDraw->BatchClearCS(4);
		m_pSolidDraw->DelWorkPlaneSketch(1);
		m_pSolidDraw->DelWorkPlaneSketch(2);
	}
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
UINT CTIDView::StartPipeTaskListen(LPVOID lpPara)
{
	if(!theApp.m_bChildProcess)
		return -1;
	CTIDView* pView=(CTIDView*)lpPara;
	long nBufLen = 1024 * 1024 * 40;
	CBuffer pipe_buf(nBufLen);;
	while(1)
	{	//从匿名管道中读取数据
		HANDLE hPipeRead= GetStdHandle( STD_INPUT_HANDLE );
		if( hPipeRead == INVALID_HANDLE_VALUE)
			return -1;
		if(pipe_buf.ReadFromPipe(hPipeRead,1024))
		{
			BYTE ciDataFlag=0,ciSerialId=0,ciLegArr[4]={0};
			DWORD dwVersion=0,dwAttSize=0,file_size=0;
			pipe_buf.SeekToBegin();
			pipe_buf.ReadByte(&ciDataFlag);
			pipe_buf.ReadDword(&dwVersion);
			pipe_buf.ReadByte(&ciSerialId);
			pipe_buf.ReadByte(&ciLegArr[0]);
			pipe_buf.ReadByte(&ciLegArr[1]);
			pipe_buf.ReadByte(&ciLegArr[2]);
			pipe_buf.ReadByte(&ciLegArr[3]);
			pipe_buf.ReadDword(&dwAttSize);
			pipe_buf.ReadDword(&file_size);
			CBuffer tid_buf(file_size);
			tid_buf.Write(pipe_buf.GetCursorBuffer(),file_size);
			theApp.m_uiActiveHeightSerial=ciSerialId;
			for(int i=0;i<4;i++)
				theApp.m_uiActiveLegSerial[i]=ciLegArr[i];
			//
			pView->UpdatePipeData(tid_buf);
		}
	}
	return 0;
}
void CTIDView::UpdatePipeData(CBuffer& tid_buf)
{
	CTIDDoc* pDoc=theApp.GetTIDDoc();
	if(pDoc)
		pDoc->DeleteContents();
	//打开TID文件
	if(gpTidModel==NULL)
		gpTidModel=CTidModelFactory::CreateTidModel();
	tid_buf.SeekToBegin();
	gpTidModel->InitTidBuffer(tid_buf.GetBufferPtr(),tid_buf.GetLength());
	//刷新实体显示
	m_pSolidDraw->BuildDisplayList(this);
	Invalidate(FALSE);
}