// CreateFace.cpp: implementation of the CCreateFace class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CreateFace.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//#if !defined(__TSA_)&&!defined(__TSA_FILE_)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSliceVertexIndex::CSliceVertexIndex()
{	current=next=vice_current=vice_next=0;}
void CSliceVertexIndex::Init(int i,UINT slices,int index_offset)
{
	current=i*4;
	if(i==slices-1)
		next=0;
	else
		next=current+4;	
	vice_current=current+index_offset;
	vice_next=next+index_offset;
}
UINT GetCirclePolyVertex(double radius, GEPOINT* parrVertexes, UINT uiMaxCount,bool blIntelliCalSlices=true)
{
	UINT slices=uiMaxCount;
	if(blIntelliCalSlices)
	{
		const double SAMPLE_LEN=5.0;
		double length=(Pi+Pi)*radius;
		slices = max(6,(int)(length/SAMPLE_LEN+0.5));
		slices=min(slices,uiMaxCount);//int max_n=144;//ftoi(72*sector_angle/Pi);
	}
	double sina,cosa;
	if(slices==6)
	{
		sina=0.86602540378443864676372317075294;
		cosa=0.5;
	}
	else if(slices==12)
	{
		sina=0.5;
		cosa=0.86602540378443864676372317075294;
	}
	else if(slices==16)
	{
		sina=0.3826834323650897717284599840304;
		cosa=0.92387953251128675612818318939679;
	}
	else if(slices==24)
	{
		sina=0.25881904510252076234889883762405;
		cosa=0.9659258262890682867497431997289;
	}
	else if(slices==36)
	{
		sina=0.17364817766693034885171662676931;
		cosa=0.98480775301220805936674302458952;
	}
	else
	{
		double dfSliceAngle= 2*Pi/slices;
		sina = sin(dfSliceAngle);	//扇片角正弦
		cosa = cos(dfSliceAngle);	//扇片角余弦
	}
	parrVertexes[0].Set(radius);
	for(UINT i=1;i<slices;i++)
	{
		parrVertexes[i].x=parrVertexes[i-1].x*cosa-parrVertexes[i-1].y*sina;
		parrVertexes[i].y=parrVertexes[i-1].y*cosa+parrVertexes[i-1].x*sina;
		parrVertexes[i].z=0;
	}
	return slices;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCreateFace::CCreateFace()
{
	vertex_list.Attach(xarrPointers,0,POINTER_STACKPOOL_SIZE);
}

//根据pBody的vertex_list生成一个类型为*f3dPoint的ArrayList链表
void CCreateFace::InitVertexList(fBody *pBody)
{
	vertex_list.SetSize(pBody->vertex.GetNodeNum());
	int i=0;
	for(f3dPoint *pVertex=pBody->vertex.GetFirst();pVertex;pVertex=pBody->vertex.GetNext(),i++)
		vertex_list[i] = pVertex;	
}

//在末尾添加一个添加一个顶点
void CCreateFace::AddVertex(f3dPoint *pVertex)
{
	vertex_list.append(pVertex);
}

//根据一组轮廓点生成一个面
void CCreateFace::NewOutsideFace(f3dPolyFace *pFace, int* ptIndexArr, int nSize)
{
	for (int i = 0; i < nSize; i++)
		NewOutterEdgeLine(pFace, ptIndexArr[(i + 1) % nSize], ptIndexArr[i]);
}

//填充外环链表
f3dAtomLine* CCreateFace::NewOutterEdgeLine(f3dPolyFace *pFace, int e_vertex_i, int s_vertex_i/* =-1 */)
{
	if(s_vertex_i>=0)
		start_face_vertex_i=s_vertex_i;
	f3dAtomLine* pLine=pFace->outer_edge.append(vertex_list[start_face_vertex_i],vertex_list[e_vertex_i]);
	start_face_vertex_i = e_vertex_i;
	return pLine;
}

//填充内环链表
f3dAtomLine* CCreateFace::NewInnerLoopEdgeLine(fLoop *pInnerLoop, int e_vertex_i, int s_vertex_i/* =-1 */)
{
	if(s_vertex_i>=0)
		start_loop_vertex_i=s_vertex_i;
	f3dAtomLine* pLine=pInnerLoop->append(vertex_list[start_loop_vertex_i],vertex_list[e_vertex_i]);
	start_loop_vertex_i = e_vertex_i;
	return pLine;
}

CCreateFace::~CCreateFace()
{
	vertex_list.Empty();
}
//#endif