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

CCreateFace::CCreateFace()
{

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