// TIDCore.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "TidApi.h"
#include "TidCplus.h"
#include "TIDModel.h"

/*#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif*/

//////////////////////////////////////////////////////////////////////////
//----材质库
UINT TIDCORE_API SteelMaterialLib_GetCount(WPARAM hISteelMaterialLib)
{
	ISteelMaterialLibrary* pSteelMaterialLib=(ISteelMaterialLibrary*)hISteelMaterialLib;
	return pSteelMaterialLib->GetCount();
}

void TIDCORE_API SteelMaterialLib_GetSteelMatAt(WPARAM hISteelMaterialLib,UINT i,char *tid_steelmat)
{
	ISteelMaterialLibrary* pSteelMaterialLib=(ISteelMaterialLibrary*)hISteelMaterialLib;
	TID_STEELMAT steelmat = pSteelMaterialLib->GetSteelMatAt(i);
	memcpy(tid_steelmat,&steelmat,9);
}
char TIDCORE_API SteelMaterialLib_QuerySteelBriefSymbol(WPARAM hISteelMaterialLib,const char* steelmark)
{
	ISteelMaterialLibrary* pSteelMaterialLib=(ISteelMaterialLibrary*)hISteelMaterialLib;
	return pSteelMaterialLib->QuerySteelBriefSymbol(steelmark);
}
bool TIDCORE_API SteelMaterialLib_QuerySteelNameCode(WPARAM hISteelMaterialLib,char cBriefSymbol,char* nameCode)
{
	ISteelMaterialLibrary* pSteelMaterialLib=(ISteelMaterialLibrary*)hISteelMaterialLib;
	return pSteelMaterialLib->QuerySteelNameCode(cBriefSymbol,nameCode);
}
/* 此类为工具函数不对Ｃ＃开放，如有需要开发人员可以自己写Ｃ＃版本的代码 wjh-2016.1.23
//<class TidArcline begin>
int TIDCORE_API TidArcline_ToByteArr(WPARAM hTidArcline,char* buf)
{
	TidArcline* pTidArcline = (TidArcline*)hTidArcline;
	return pTidArcline->ToByteArr(buf);
}

void TIDCORE_API TidArcline_FromByteArr(WPARAM hTidArcline,char* buf,DWORD buf_size)
{
	TidArcline* pTidArcline = (TidArcline*)hTidArcline;
	pTidArcline->FromByteArr(buf,buf_size);
}

void TIDCORE_API TidArcline_StartPosition(WPARAM hTidArcline,double* TID_COORD3D)
{
	TidArcline* pTidArcline = (TidArcline*)hTidArcline;
	TID_COORD3D* pCoord3d = pTidArcline->StartPosition();
	memcpy(tid_coord3d,&pCoord3d，24);
}
void TIDCORE_API  TidArcline_EndPosition(WPARAM hTidArcline,double* TID_COORD3D)
{
	TidArcline* pTidArcline = (TidArcline*)hTidArcline;
	TID_COORD3D* pCoord3d = pTidArcline->EndPosition();
	memcpy(tid_coord3d,&pCoord3d,24);
}
void TIDCORE_API TidArcline_ColumnNorm(WPARAM hTidArcline,double* TID_COORD3D)
{
	TidArcline* pTidArcline = (TidArcline*)hTidArcline;
	TID_COORD3D* pCoord3d = pTidArcline->ColumnNorm();
	memcpy(tid_coord3d,pCoord3d,24);
}
void TIDCORE_API TidArcline_WorkNorm(WPARAM hTidArcline,double* TID_COORD3D)
{
	TidArcline* pTidArcline = (TidArcline*)hTidArcline;
	TID_COORD3D* pCoord3d = pTidArcline->WorkNorm();
	memcpy(tid_coord3d,&pCoord3d,24);
}
void TIDCORE_API TidArcline_Center(WPARAM hTidArcline,double* TID_COORD3D)
{
	TidArcline* pTidArcline = (TidArcline*)hTidArcline;
	TID_COORD3D* pCoord3d = pTidArcline->Center();
	memcpy(tid_coord3d,  &pCoord3d,24);
}
double TIDCORE_API TidArcline_Radius(WPARAM hTidArcline)
{
	TidArcline* pTidArcline = (TidArcline*)hTidArcline;
	return pTidArcline->Radius();
}
double TIDCORE_API TidArcline_SectorAngle(WPARAM hTidArcline)
{
	TidArcline* pTidArcline = (TidArcline*)hTidArcline;
	return pTidArcline->SectorAngle();
}
double TIDCORE_API TidArcline_Length(WPARAM hTidArcline)
{
	TidArcline* pTidArcline = (TidArcline*)hTidArcline;
	return pTidArcline->Length();
}
void  TIDCORE_API TidArcline_PositionInAngle(WPARAM hTidArcline,double posAngle,double* TID_COORD3D)
{
	TidArcline* pTidArcline = (TidArcline*)hTidArcline;
	TID_COORD3D* pCoord3d = pTidArcline->PositionInAngle(posAngle);
	memcpy(tid_coord3d,&pCoord3d,24);
}
void TIDCORE_API TidArcline_TangentVecInAngle(WPARAM hTidArcline,double posAngle,double* TID_COORD3D )
{
	TidArcline* pTidArcline = (TidArcline*)hTidArcline;
	TID_COORD3D* pCoord3d = pTidArcline->PositionInAngle(posAngle);
	memcpy(tid_coord3d,&pCoord3d,24);
}
//<class TidArcline end>
*/
//<class TidRawSolidEdge begin>
BYTE TIDCORE_API TidRawSolidEdge_EdgeType(WPARAM hTidRawSolidEdge)
{
	ITidRawSolidEdge* pTidRawSolidEdge = (ITidRawSolidEdge*)hTidRawSolidEdge;
	return pTidRawSolidEdge->EdgeType();
}
BYTE TIDCORE_API TidRawSolidEdge_SolidDrawWidth(WPARAM hTidRawSolidEdge)
{
	ITidRawSolidEdge* pTidRawSolidEdge = (ITidRawSolidEdge*)hTidRawSolidEdge;
	return pTidRawSolidEdge->SolidDrawWidth();
}
DWORD TIDCORE_API TidRawSolidEdge_LineStartId(WPARAM hTidRawSolidEdge)
{
	ITidRawSolidEdge* pTidRawSolidEdge = (ITidRawSolidEdge*)hTidRawSolidEdge;
	return pTidRawSolidEdge->LineStartId();
}
DWORD TIDCORE_API TidRawSolidEdge_LineEndId(WPARAM hTidRawSolidEdge)
{
	ITidRawSolidEdge* pTidRawSolidEdge = (ITidRawSolidEdge*)hTidRawSolidEdge;
	return pTidRawSolidEdge->LineEndId();
}
void  TIDCORE_API TidRawSolidEdge_Center(WPARAM hTidRawSolidEdge,double* tid_coord3d)
{
	ITidRawSolidEdge* pTidRawSolidEdge = (ITidRawSolidEdge*)hTidRawSolidEdge;
	TID_COORD3D center = pTidRawSolidEdge->Center();
	memcpy(tid_coord3d,&center,24);
}
void TIDCORE_API TidRawSolidEdge_WorkNorm(WPARAM hTidRawSolidEdge,double* tid_coord3d)
{
	ITidRawSolidEdge* pTidRawSolidEdge = (ITidRawSolidEdge*)hTidRawSolidEdge;
	TID_COORD3D worknorm = pTidRawSolidEdge->WorkNorm();
	memcpy(tid_coord3d,&worknorm,24);
}
void TIDCORE_API TidRawSolidEdge_ColumnNorm(WPARAM hTidRawSolidEdge,double* tid_coord3d)
{
	ITidRawSolidEdge* pTidRawSolidEdge = (ITidRawSolidEdge*)hTidRawSolidEdge;
	TID_COORD3D columnnorm = pTidRawSolidEdge->ColumnNorm();
	memcpy(tid_coord3d,&columnnorm,24);
}
//<class TidRawSolidEdge end>
//------------------------
//<class IFaceLoop begin>
WORD TIDCORE_API FaceLoop_EdgeLineNum(WPARAM hIFaceLoop)
{
	IFaceLoop *pFaceLoop=(IFaceLoop*)hIFaceLoop;
	return pFaceLoop->EdgeLineNum();
}
bool TIDCORE_API FaceLoop_EdgeLineDataAt(WPARAM hIFaceLoop,int index,double* pArcline)
{
	IFaceLoop *pFaceLoop=(IFaceLoop*)hIFaceLoop;
	TidArcline line;
	if(!pFaceLoop->EdgeLineAt(index,&line))
		return false;
	memcpy(pArcline,&line,sizeof(double)*17);
	return true;
}
WPARAM TIDCORE_API FaceLoop_EdgeLineAt(WPARAM hIFaceLoop,int index)
{
	IFaceLoop *pFaceLoop=(IFaceLoop*)hIFaceLoop;
	return (WPARAM)pFaceLoop->EdgeLineAt(index);
}
//<class IFaceLoop end>

//<class IRawSolidFace begin>
COLORREF TIDCORE_API RawSolidFace_MatColor(WPARAM hIRawSolidFace )	//TODO
{ 
	IRawSolidFace* pRawSolidFace=(IRawSolidFace*)hIRawSolidFace;
	return pRawSolidFace->MatColor(); 
}
DWORD TIDCORE_API RawSolidFace_FaceId(WPARAM hIRawSolidFace)
{
	IRawSolidFace* pRawSolidFace=(IRawSolidFace*)hIRawSolidFace;
	return pRawSolidFace->FaceId();
}
WORD TIDCORE_API RawSolidFace_InnerLoopNum(WPARAM hIRawSolidFace)
{
	IRawSolidFace* pRawSolidFace=(IRawSolidFace*)hIRawSolidFace;
	return pRawSolidFace->InnerLoopNum();
}

void TIDCORE_API RawSolidFace_WorkNorm(WPARAM hIRawSolidFace,double* tid_coord3d)
{
	IRawSolidFace* pRawSolidFace=(IRawSolidFace*)hIRawSolidFace;
	TID_COORD3D worknorm = pRawSolidFace->WorkNorm();
	memcpy(tid_coord3d,&worknorm,24);
}
WPARAM TIDCORE_API RawSolidFace_InnerLoopAt(WPARAM hIRawSolidFace,int i)
{
	IRawSolidFace* pRawSolidFace=(IRawSolidFace*)hIRawSolidFace;
	return (WPARAM) pRawSolidFace->InnerLoopAt(i);
}
WPARAM TIDCORE_API RawSolidFace_OutterLoop(WPARAM hIRawSolidFace)
{
	IRawSolidFace* pRawSolidFace=(IRawSolidFace*)hIRawSolidFace;
	return (WPARAM) pRawSolidFace->OutterLoop();
}
//<class IRawSolidFace end>

//<class IFacetCluster begin>
BYTE TIDCORE_API FacetCluster_Mode(WPARAM hIFacetCluster)//绘制启动函数glBegin()中需要的绘制模式
{
	IFacetCluster* pFacetCluster=(IFacetCluster*)hIFacetCluster;
	return pFacetCluster->Mode();
}
void  TIDCORE_API FacetCluster_Normal(WPARAM hIFacetCluster,double *tid_coord3d)
{
	IFacetCluster* pFacetCluster=(IFacetCluster*)hIFacetCluster;
	TID_COORD3D normal = pFacetCluster->Normal();
	memcpy(tid_coord3d,&normal,24);
}
WORD TIDCORE_API FacetCluster_FacetNumber(WPARAM hIFacetCluster)
{
	IFacetCluster* pFacetCluster=(IFacetCluster*)hIFacetCluster;
	return pFacetCluster->FacetNumber();
}
WORD TIDCORE_API FacetCluster_VertexNumber(WPARAM hIFacetCluster)
{
	IFacetCluster* pFacetCluster=(IFacetCluster*)hIFacetCluster;
	return pFacetCluster->VertexNumber();
}
void TIDCORE_API FacetCluster_VertexAt(WPARAM hIFacetCluster,int index,double* tid_coord3d)
{
	IFacetCluster* pFacetCluster=(IFacetCluster*)hIFacetCluster;
	TID_COORD3D vertice = pFacetCluster->VertexAt(index);
	memcpy(tid_coord3d,&vertice,24);
}
//<class IFacetCluster end>

//<class ITidBasicFace begin>
COLORREF TIDCORE_API TidBasicFace_Color(WPARAM hITidBasicFace)
{
	ITidBasicFace* pTidBasicFace=(ITidBasicFace*)hITidBasicFace;
	return pTidBasicFace->Color();
}
WORD  TIDCORE_API TidBasicFace_FacetClusterNumber(WPARAM hITidBasicFace)
{
	ITidBasicFace* pTidBasicFace=(ITidBasicFace*)hITidBasicFace;
	return pTidBasicFace->FacetClusterNumber();
}
WPARAM TIDCORE_API TidBasicFace_FacetClusterAt(WPARAM hITidBasicFace,int index)
{
	ITidBasicFace* pTidBasicFace=(ITidBasicFace*)hITidBasicFace;
	return (WPARAM)pTidBasicFace->FacetClusterAt(index);
}
//<class ITidBasicFace end>

//<class ITidSolidBody begin>
int TIDCORE_API ITidSolidBody_KeyPointNum(WPARAM hITidSolidBody)
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	return pTidSolidBody->KeyPointNum();
}
/*
char* TIDCORE_API ITidSolidBody_SolidBufferPtr(WPARAM hITidSolidBody)
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	return pTidSolidBody->SolidBufferPtr();
}
*/
DWORD TIDCORE_API ITidSolidBody_SolidBufferLength(WPARAM hITidSolidBody)
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	return pTidSolidBody->SolidBufferLength();
}
void TIDCORE_API ITidSolidBody_ReleaseTemporaryBuffer(WPARAM hITidSolidBody)
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	pTidSolidBody->ReleaseTemporaryBuffer();
}
void TIDCORE_API ITidSolidBody_GetKeyPointAt(WPARAM hITidSolidBody,int i,double* tid_coords)
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	TID_COORD3D vertice = pTidSolidBody->GetKeyPointAt(i);
	memcpy( tid_coords,&vertice,24);
}
int TIDCORE_API ITidSolidBody_KeyEdgeLineNum(WPARAM hITidSolidBody)
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	return pTidSolidBody->KeyEdgeLineNum();
}
bool TIDCORE_API ITidSolidBody_GetKeyEdgeLineDataAt(WPARAM hITidSolidBody,int i,double* line_data)
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	TidArcline line;
	if(!pTidSolidBody->GetKeyEdgeLineAt(i,line))
		return false;
	memcpy(line_data,&line,sizeof(double)*17);
	return true;
}
WPARAM TIDCORE_API ITidSolidBody_GetKeyEdgeLineAt(WPARAM hITidSolidBody,int i) //return ITidRawSolidEdge*
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	return(WPARAM) pTidSolidBody->GetKeyEdgeLineAt(i);
}

int TIDCORE_API ITidSolidBody_PolyFaceNum(WPARAM hITidSolidBody)
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	return(WPARAM) pTidSolidBody->PolyFaceNum();
}
WPARAM TIDCORE_API ITidSolidBody_GetPolyFaceAt(WPARAM hITidSolidBody,int i)
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	return(WPARAM) pTidSolidBody->GetPolyFaceAt(i);
}
int TIDCORE_API ITidSolidBody_BasicFaceNum(WPARAM hITidSolidBody)
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	return(WPARAM) pTidSolidBody->BasicFaceNum();
}
WPARAM TIDCORE_API ITidSolidBody_GetBasicFaceAt(WPARAM hITidSolidBody,int i)
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	return(WPARAM) pTidSolidBody->GetBasicFaceAt(i);
}
bool TIDCORE_API ITidSolidBody_SplitToBasicFacets(WPARAM hITidSolidBody)
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	return pTidSolidBody->SplitToBasicFacets();
}
void  TIDCORE_API ITidSolidBody_TransACS(WPARAM hITidSolidBody,const double* fromACS,const double* toACS)
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	TID_CS fromcs,tocs;
	fromcs.FromCoordsSet(fromACS);
	tocs.FromCoordsSet(toACS);
	pTidSolidBody->TransACS( fromcs,tocs);
}
void  TIDCORE_API ITidSolidBody_TransFromACS(WPARAM hITidSolidBody,const double* fromACS) // TODO
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	TID_CS fromcs;
	fromcs.FromCoordsSet(fromACS);
	pTidSolidBody->TransFromACS(fromcs);
}
void  TIDCORE_API ITidSolidBody_TransToACS(WPARAM hITidSolidBody,const double* toACS) // TODO
{
	ITidSolidBody* pTidSolidBody=(ITidSolidBody*)hITidSolidBody;
	TID_CS tocs;
	tocs.FromCoordsSet(toACS);
	pTidSolidBody->TransFromACS(tocs);
}
//<class ITidSolidBody end>

//<class ITidBoltNut begin>
short  TIDCORE_API TidBoltNut_GetDiameter(WPARAM hITidBoltNut) // TODO
{
	ITidBoltNut* pTidBoltNut = (ITidBoltNut*) hITidBoltNut;
	return pTidBoltNut->GetDiameter();
}
WPARAM TIDCORE_API TidBoltNut_GetSolidBody(WPARAM hITidBoltNut)
{
	ITidBoltNut* pTidBoltNut = (ITidBoltNut*) hITidBoltNut;
	return (WPARAM)pTidBoltNut->GetSolidBody();
}
//<class ITidBoltNut end>

//<class IBoltSizeSpec begin>
UINT  TIDCORE_API BoltSizeSpec_GetSeriesId(WPARAM hIBoltSizeSpec)
{
	IBoltSizeSpec* pIBoltSizeSpec = (IBoltSizeSpec*) hIBoltSizeSpec;
	return pIBoltSizeSpec->GetSeriesId();
}
short  TIDCORE_API BoltSizeSpec_GetDiameter(WPARAM hIBoltSizeSpec)
{
	IBoltSizeSpec* pIBoltSizeSpec = (IBoltSizeSpec*) hIBoltSizeSpec;
	return pIBoltSizeSpec->GetDiameter();
}
short  TIDCORE_API BoltSizeSpec_GetLenValid(WPARAM hIBoltSizeSpec)
{
	IBoltSizeSpec* pIBoltSizeSpec = (IBoltSizeSpec*) hIBoltSizeSpec;
	return pIBoltSizeSpec->GetLenValid();
}
short  TIDCORE_API BoltSizeSpec_GetLenNoneThread(WPARAM hIBoltSizeSpec)
{
	IBoltSizeSpec* pIBoltSizeSpec = (IBoltSizeSpec*) hIBoltSizeSpec;
	return pIBoltSizeSpec->GetLenNoneThread();
}
double  TIDCORE_API BoltSizeSpec_GetTheoryWeight(WPARAM hIBoltSizeSpec)
{
	IBoltSizeSpec* pIBoltSizeSpec = (IBoltSizeSpec*) hIBoltSizeSpec;
	return pIBoltSizeSpec->GetTheoryWeight();
}
short  TIDCORE_API BoltSizeSpec_GetSpec(WPARAM hIBoltSizeSpec,char* spec)
{
	IBoltSizeSpec* pIBoltSizeSpec = (IBoltSizeSpec*) hIBoltSizeSpec;
	return pIBoltSizeSpec->GetSpec(spec);
}
WPARAM  TIDCORE_API BoltSizeSpec_GetBoltSolid(WPARAM hIBoltSizeSpec)
{
	IBoltSizeSpec* pIBoltSizeSpec = (IBoltSizeSpec*) hIBoltSizeSpec;
	return (WPARAM)pIBoltSizeSpec->GetBoltSolid();
}
WPARAM  TIDCORE_API BoltSizeSpec_GetNutSolid(WPARAM hIBoltSizeSpec)// retturn ITidSolidBody*
{

	IBoltSizeSpec* pIBoltSizeSpec = (IBoltSizeSpec*) hIBoltSizeSpec;
	return (WPARAM)pIBoltSizeSpec->GetNutSolid();
}
//<class IBoltSizeSpec end>

//<class IBoltSeries begin>
UINT TIDCORE_API BoltSeries_GetSeriesId(WPARAM hIBoltSeries)
{
	IBoltSeries *pIBoltSeries = (IBoltSeries*)hIBoltSeries;
	return pIBoltSeries->GetSeriesId();
}
short TIDCORE_API BoltSeries_GetName(WPARAM hIBoltSeries,char* name)
{
	IBoltSeries *pIBoltSeries = (IBoltSeries*)hIBoltSeries;
	return pIBoltSeries->GetName(name);
}
int TIDCORE_API BoltSeries_GetCount(WPARAM hIBoltSeries)
{
	IBoltSeries *pIBoltSeries = (IBoltSeries*)hIBoltSeries;
	return pIBoltSeries->GetCount();
}
short TIDCORE_API BoltSeries_GetBoltNutCount(WPARAM hIBoltSeries )
{
	IBoltSeries *pIBoltSeries = (IBoltSeries*)hIBoltSeries;
	return pIBoltSeries->GetBoltNutCount();
}
short TIDCORE_API BoltSeries_GetWasherCount(WPARAM hIBoltSeries )
{
	IBoltSeries *pIBoltSeries = (IBoltSeries*)hIBoltSeries;
	return pIBoltSeries->GetWasherCount();
}
/*
bool TIDCORE_API BoltSeries_IsFootNail(WPARAM hIBoltSeries)
{
	IBoltSeries *pIBoltSeries = (IBoltSeries*)hIBoltSeries;
	return pIBoltSeries->IsFootNail();
}
bool TIDCORE_API BoltSeries_IsBurglarproof(WPARAM hIBoltSeries)
{
	IBoltSeries *pIBoltSeries = (IBoltSeries*)hIBoltSeries;
	return pIBoltSeries->IsBurglarproof();
}
*/
WPARAM TIDCORE_API BoltSeries_EnumBoltFirst(WPARAM hIBoltSeries)
{
	IBoltSeries *pIBoltSeries = (IBoltSeries*)hIBoltSeries;
	return (WPARAM)pIBoltSeries->EnumFirst();
}
WPARAM TIDCORE_API BoltSeries_EnumBoltNext(WPARAM hIBoltSeries)//IBoltSizeSpec*
{
	IBoltSeries *pIBoltSeries = (IBoltSeries*)hIBoltSeries;
	return (WPARAM)pIBoltSeries->EnumNext();
}
WPARAM TIDCORE_API BoltSeries_GetBoltSizeSpecById(WPARAM hIBoltSeries,int id)//IBoltSizeSpec*
{
	IBoltSeries *pIBoltSeries = (IBoltSeries*)hIBoltSeries;
	return(WPARAM) pIBoltSeries->GetBoltSizeSpecById(id);
}
//<class IBoltSeries end>

//<class IBoltSeriesLib begin>
int  TIDCORE_API BoltSeriesLib_GetCount(WPARAM hIBoltSeriesLib)
{
	IBoltSeriesLib* pBoltSeriesLib =(IBoltSeriesLib*)hIBoltSeriesLib;
	return pBoltSeriesLib->GetCount();
}
WPARAM  TIDCORE_API BoltSeriesLib_GetBoltSeriesAt(WPARAM hIBoltSeriesLib,int i)
{
	IBoltSeriesLib* pBoltSeriesLib =(IBoltSeriesLib*)hIBoltSeriesLib;
	return (WPARAM)pBoltSeriesLib->GetBoltSeriesAt(i);
}
WPARAM TIDCORE_API BoltSeriesLib_GetBoltSizeSpec(WPARAM hIBoltSeriesLib,int seriesId,int sizeSpecId)
{
	IBoltSeriesLib* pBoltSeriesLib =(IBoltSeriesLib*)hIBoltSeriesLib;
	return (WPARAM)pBoltSeriesLib->GetBoltSizeSpec(seriesId,sizeSpecId);
}
//<class IBoltSeriesLib end>


//<class ITidPart begin>
int  TIDCORE_API ITidPart_GetPartType(WPARAM hITidPart)
{
	ITidPart* pITidPart = (ITidPart*)hITidPart;
	return pITidPart->GetPartType();
}
UINT TIDCORE_API ITidPart_GetSerialId(WPARAM hITidPart)
{
	ITidPart* pITidPart = (ITidPart*)hITidPart;
	return pITidPart->GetSerialId();
}
char  TIDCORE_API ITidPart_GetBriefMark(WPARAM hITidPart)
{
	ITidPart* pITidPart = (ITidPart*)hITidPart;
	return pITidPart->GetBriefMark();
}
WORD  TIDCORE_API ITidPart_GetLength(WPARAM hITidPart)
{
	ITidPart* pITidPart = (ITidPart*)hITidPart;
	return pITidPart->GetLength();
}
double TIDCORE_API ITidPart_GetWidth(WPARAM hITidPart)
{
	ITidPart* pITidPart = (ITidPart*)hITidPart;
	return pITidPart->GetWidth();
}
double TIDCORE_API ITidPart_GetThick(WPARAM hITidPart)
{
	ITidPart* pITidPart = (ITidPart*)hITidPart;
	return pITidPart->GetThick();
}
double TIDCORE_API ITidPart_GetHeight(WPARAM hITidPart)
{
	ITidPart* pITidPart = (ITidPart*)hITidPart;
	return pITidPart->GetHeight();
}
double TIDCORE_API ITidPart_GetWeight(WPARAM hITidPart)
{
	ITidPart* pITidPart = (ITidPart*)hITidPart;
	return pITidPart->GetWeight();
}
WORD TIDCORE_API ITidPart_GetStateFlag(WPARAM hITidPart)
{
	ITidPart* pITidPart = (ITidPart*)hITidPart;
	return pITidPart->StateFlag();
}
short TIDCORE_API ITidPart_GetSegStr(WPARAM hITidPart,char* segstr)
{
	ITidPart* pITidPart = (ITidPart*)hITidPart;
	return pITidPart->GetSegStr(segstr);
}

short TIDCORE_API ITidPart_GetSpec(WPARAM hITidPart,char* sizeSpec)
{
	ITidPart* pITidPart = (ITidPart*)hITidPart;
	return pITidPart->GetSpec(sizeSpec);
}
short TIDCORE_API ITidPart_GetLabel(WPARAM hITidPart,char*label)
{
	ITidPart* pITidPart = (ITidPart*)hITidPart;
	return pITidPart->GetLabel(label);
}
short TIDCORE_API ITidPart_GetFuncType(WPARAM hITidPart)
{
	ITidPart* pITidPart = (ITidPart*)hITidPart;
	return pITidPart->GetFuncType();
}
WPARAM TIDCORE_API ITidPart_GetSolidPart(WPARAM hITidPart)
{
	ITidPart* pTidPart = (ITidPart*)hITidPart;
	return (WPARAM)pTidPart->GetSolidPart();
}
UINT TIDCORE_API ITidPart_GetProcessBuffBytes(WPARAM hITidPart,char* processbytes,UINT maxBufLength/*=0*/)
{
	ITidPart* pTidPart = (ITidPart*)hITidPart;
	return (WPARAM)pTidPart->GetSolidPart();
}
//<class ITidPart end>

//<class ITidPartsLib begin>
int  TIDCORE_API TidPartsLib_GetCount(WPARAM hITidPartsLib)
{
	ITidPartsLib* pPartLib=(ITidPartsLib*)hITidPartsLib;
	return pPartLib->GetCount();
}
WPARAM TIDCORE_API TidPartsLib_GetTidPartBySerialId(WPARAM hITidPartsLib,int serialid)// return ITidPart*
{
	ITidPartsLib* pPartLib=(ITidPartsLib*)hITidPartsLib;
	return (WPARAM)pPartLib->GetTidPartBySerialId(serialid);
}
//<class ITidPartsLib end>

//<class ITidAssemblePart begin>
WPARAM TIDCORE_API TidAssemblePart_GetPart(WPARAM hITidAssemblePart)
{
	ITidAssemblePart* pTidAssemblePart = (ITidAssemblePart*)hITidAssemblePart;
	return (WPARAM )pTidAssemblePart->GetPart();
}
bool TIDCORE_API TidAssemblePart_IsHasBriefRodLine(WPARAM hITidAssemblePart)
{
	ITidAssemblePart* pTidAssemblePart=(ITidAssemblePart*)hITidAssemblePart;
	return pTidAssemblePart->IsHasBriefRodLine();
}
void TIDCORE_API TidAssemblePart_BriefLineStart(WPARAM hITidAssemblePart, double* tid_coord3d) //TODO
{
	ITidAssemblePart* pTidAssemblePart=(ITidAssemblePart*)hITidAssemblePart;
	TID_COORD3D start = pTidAssemblePart->BriefLineStart();
	memcpy(tid_coord3d,&start,24);
}
void TIDCORE_API TidAssemblePart_BriefLineEnd(WPARAM hITidAssemblePart, double* tid_coord3d)
{
	ITidAssemblePart*pTidAssemblePart=(ITidAssemblePart*)hITidAssemblePart;
	TID_COORD3D end = pTidAssemblePart->BriefLineEnd();
	memcpy(tid_coord3d,&end,24);
}
int TIDCORE_API TidAssemblePart_GetStartNodeId(WPARAM hITidAssemblePart)
{
	ITidAssemblePart*pTidAssemblePart=(ITidAssemblePart*)hITidAssemblePart;
	return pTidAssemblePart->GetStartNodeId();
}
int TIDCORE_API TidAssemblePart_GetEndNodeId(WPARAM hITidAssemblePart)
{
	ITidAssemblePart*pTidAssemblePart=(ITidAssemblePart*)hITidAssemblePart;
	return pTidAssemblePart->GetEndNodeId();
}
void TIDCORE_API TidAssemblePart_GetAcs(WPARAM hITidAssemblePart, double* tid_cs)  //TODO
{
	ITidAssemblePart*pTidAssemblePart=(ITidAssemblePart*)hITidAssemblePart;
	TID_CS cs = pTidAssemblePart->GetAcs();
	memcpy(tid_cs,&cs,24*4);
}
WPARAM TIDCORE_API TidAssemblePart_GetSolidPart(WPARAM hITidAssemblePart)
{
	ITidAssemblePart*pTidAssemblePart=(ITidAssemblePart*)hITidAssemblePart;
	return (WPARAM)pTidAssemblePart->GetSolidPart();
}
//<class ITidAssemblePart end>

//<class ITidAssembleBolt begin>

WPARAM TIDCORE_API TidAssembleBolt_GetTidBolt(WPARAM hITidAssembleBolt)
{
	ITidAssembleBolt* pTidAssembleBolt=(ITidAssembleBolt*)hITidAssembleBolt;
	return (WPARAM)pTidAssembleBolt->GetTidBolt();
}
void  TIDCORE_API TidAssembleBolt_GetAcs(WPARAM hITidAssembleBolt,double* tid_cs)
{
	ITidAssembleBolt* pTidAssembleBolt=(ITidAssembleBolt*)hITidAssembleBolt;
	TID_CS cs = pTidAssembleBolt->GetAcs();
	memcpy(tid_cs,&cs,4*24);
}
WPARAM TIDCORE_API TidAssembleBolt_GetBoltSolid(WPARAM hITidAssembleBolt)
{
	ITidAssembleBolt* pTidAssembleBolt=(ITidAssembleBolt*)hITidAssembleBolt;
	return (WPARAM)pTidAssembleBolt->GetBoltSolid();
}
WPARAM TIDCORE_API TidAssembleBolt_GetNutSolid(WPARAM hITidAssembleBolt)//return ITidSolidBody*
{
	ITidAssembleBolt* pTidAssembleBolt=(ITidAssembleBolt*)hITidAssembleBolt;
	return (WPARAM)pTidAssembleBolt->GetNutSolid();	
}
//<class ITidAssembleBolt end>

//<class ITidHeightGroup begin>
int TIDCORE_API TidHeightGroup_GetSerialId(WPARAM hITidHeightGroup)
{
	ITidHeightGroup* pTidHeightGroup= (ITidHeightGroup*)hITidHeightGroup;
	return pTidHeightGroup->GetSerialId();
}
int TIDCORE_API TidHeightGroup_GetName(WPARAM hITidHeightGroup,char *name,UINT maxBufLength/*=0*/)
{
	ITidHeightGroup* pTidHeightGroup = (ITidHeightGroup*)hITidHeightGroup;
	return pTidHeightGroup->GetName(name,maxBufLength);
}
int TIDCORE_API TidHeightGroup_GetLegSerialArr(WPARAM hITidHeightGroup,int* legSerialArr)
{
	ITidHeightGroup* pTidHeightGroup = (ITidHeightGroup*)hITidHeightGroup;
	return pTidHeightGroup->GetLegSerialArr(legSerialArr);
}
int TIDCORE_API TidHeightGroup_GetLegSerial(WPARAM hITidHeightGroup,double heightDifference)
{
	ITidHeightGroup* pTidHeightGroup = (ITidHeightGroup*)hITidHeightGroup;
	return pTidHeightGroup->GetLegSerial(heightDifference);
}
double TIDCORE_API TidHeightGroup_GetLegHeightDifference(WPARAM hITidHeightGroup,int legSerial)
{
	ITidHeightGroup* pTidHeightGroup = (ITidHeightGroup*)hITidHeightGroup;
	return pTidHeightGroup->GetLegHeightDifference(legSerial);
}
WPARAM TIDCORE_API TidHeightGroup_GetTowerInstance(WPARAM hITidHeightGroup,int legSerialQuad1, int legSerialQuad2, 
	                                              int legSerialQuad3, int legSerialQuad4)
{
	ITidHeightGroup* pTidHeightGroup = (ITidHeightGroup*)hITidHeightGroup;
	return (WPARAM) pTidHeightGroup->GetTowerInstance( legSerialQuad1,legSerialQuad2, legSerialQuad3, legSerialQuad4);
}
//<class ITidHeightGroup end>

//<class ITidTowerInstance begin>
short TIDCORE_API TidTowerInstance_GetLegSerialIdByQuad(WPARAM hTidTowerInstance,short siQuad)
{
	ITidTowerInstance* pTidTowerInstance =(ITidTowerInstance*) hTidTowerInstance;
	return pTidTowerInstance->GetLegSerialIdByQuad(siQuad);
}
WPARAM TIDCORE_API TidTowerInstance_BelongHeightGroup(WPARAM hTidTowerInstance)
{
	ITidTowerInstance* pTidTowerInstance =(ITidTowerInstance*) hTidTowerInstance;
	return (WPARAM)pTidTowerInstance->BelongHeightGroup();
}
WPARAM TIDCORE_API TidTowerInstance_EnumAssembleBoltFirst(WPARAM hTidTowerInstance)
{
	ITidTowerInstance* pTidTowerInstance =(ITidTowerInstance*) hTidTowerInstance;
	return (WPARAM)pTidTowerInstance->EnumAssembleBoltFirst();
}
WPARAM TIDCORE_API TidTowerInstance_EnumAssembleBoltNext(WPARAM hTidTowerInstance) //return ITidAssembleBolt*
{
	ITidTowerInstance* pTidTowerInstance =(ITidTowerInstance*) hTidTowerInstance;
	return (WPARAM)pTidTowerInstance->EnumAssembleBoltNext();
}
WPARAM TIDCORE_API TidTowerInstance_EnumAssemblePartFirst(WPARAM hTidTowerInstance)
{
	ITidTowerInstance* pTidTowerInstance =(ITidTowerInstance*) hTidTowerInstance;
	return (WPARAM)pTidTowerInstance->EnumAssemblePartFirst();
}
WPARAM TIDCORE_API TidTowerInstance_EnumAssemblePartNext(WPARAM hTidTowerInstance)
{
	ITidTowerInstance* pTidTowerInstance =(ITidTowerInstance*) hTidTowerInstance;
	return (WPARAM)pTidTowerInstance->EnumAssemblePartNext();
}
//<class ITidTowerInstance end>

// CTidModel
UINT TIDCORE_API TidModel_GetSerialId(WPARAM hITidModel)
{
	ITidModel* pTidModel=(ITidModel*)hITidModel;
	return pTidModel->GetSerialId();
}
int  TIDCORE_API TidModel_GetTowerTypeName(WPARAM hITidModel,char* towerTypeName,UINT maxBufLength/*=0*/)
{
	ITidModel* pTidModel=(ITidModel*)hITidModel;
	return pTidModel->GetTowerTypeName(towerTypeName,maxBufLength);
}
void TIDCORE_API TidModel_ModelCoordSystem(WPARAM hITidModel,double* tid_cs)	//TID_CS=24*4字节
{
	ITidModel* pTidModel=(ITidModel*)hITidModel;
	TID_CS cs=pTidModel->ModelCoordSystem();
	memcpy(tid_cs,&cs,96);
}
WPARAM TIDCORE_API TidModel_GetSteelMatLibrary(WPARAM hITidModel)			//return ISteelMaterialLibrary*
{
	ITidModel* pTidModel=(ITidModel*)hITidModel;
	return (WPARAM)pTidModel->GetSteelMatLib();
}
WPARAM TIDCORE_API TidModel_GetBoltLibrary(WPARAM hITidModel)				//return IBoltSeriesLib*
{
	ITidModel* pTidModel=(ITidModel*)hITidModel;
	return (WPARAM)pTidModel->GetBoltLib();
}
WPARAM TIDCORE_API TidModel_GetTidPartsLib(WPARAM hITidModel)				//return ITidPartsLib*
{
	ITidModel* pTidModel=(ITidModel*)hITidModel;
	return (WPARAM)pTidModel->GetTidPartsLib();
}
short TIDCORE_API TidModel_HeightGroupCount(WPARAM hITidModel)
{
	ITidModel* pTidModel=(ITidModel*)hITidModel;
	return pTidModel->HeightGroupCount();
}
WPARAM TIDCORE_API TidModel_GetHeightGroupAt(WPARAM hITidModel,short i)
{
	ITidModel* pTidModel=(ITidModel*)hITidModel;
	ITidHeightGroup* pHeightGroup=pTidModel->GetHeightGroupAt(i);
	return (WPARAM)pHeightGroup;
}
bool TIDCORE_API TidModel_InitTidBuffer(WPARAM hITidModel,const char* src_buf,UINT buf_len)
{
	ITidModel* pTidModel=(ITidModel*)hITidModel;
	return pTidModel->InitTidBuffer(src_buf,buf_len);
}
bool TIDCORE_API TidModel_ReadTidFile(WPARAM hITidModel,const char* file_path)
{
	ITidModel* pTidModel=(ITidModel*)hITidModel;
	return pTidModel->ReadTidFile(file_path);
}
//////////////////////////////////////////////////////////////////////////
// CTidModelFactory
WPARAM TIDCORE_API Factory_CreateTidModel()
{
	ITidModel* pTidModel=CTidModelFactory::CreateTidModel();
	return (WPARAM)pTidModel;
}
//WPARAM TIDCORE_API Factory_TidModelFromSerial();
bool TIDCORE_API Factory_DestroyTidModel(WPARAM hITidModel)
{
	ITidModel* pTidModel=(ITidModel*)hITidModel;
	return CTidModelFactory::Destroy(pTidModel->GetSerialId());
}

