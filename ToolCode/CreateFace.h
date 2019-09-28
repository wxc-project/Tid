// CreateFace.h: interface for the CCreateFace class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CREATEFACE_H__EF0A4EA0_84A6_43FC_9922_98B9782A1152__INCLUDED_)
#define AFX_CREATEFACE_H__EF0A4EA0_84A6_43FC_9922_98B9782A1152__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ArrayList.h"
#include "f_ent_list.h"

//#if !defined(__TSA_)&&!defined(__TSA_FILE_)
class CSliceVertexIndex
{
public:
	int current,next,vice_current,vice_next;
	CSliceVertexIndex();
	void Init(int slice_index,UINT slices,int index_offset);
};

class CCreateFace  
{
	int start_face_vertex_i;
	int start_loop_vertex_i;
	typedef f3dPoint* f3dPointPtr;
	static const int POINTER_STACKPOOL_SIZE = 100;
	f3dPointPtr xarrPointers[POINTER_STACKPOOL_SIZE];
	ARRAY_LIST<f3dPoint*> vertex_list;
public:
	CCreateFace();
	virtual ~CCreateFace();
	//
	void InitVertexList(fBody *pBody);
	void AddVertex(f3dPoint* pVertex);
	f3dPoint VertexAt(int i){return *vertex_list[i];}
	f3dPoint* VertexPtrAt(int i){return vertex_list[i];}
	f3dAtomLine* NewOutterEdgeLine(f3dPolyFace *pFace, int e_vertex_i, int s_vertex_i=-1);
	f3dAtomLine* NewInnerLoopEdgeLine(fLoop *pInnerLoop, int e_vertex_i, int s_vertex_i=-1);
	void NewOutsideFace(f3dPolyFace *pFace, int* ptIndexArr,int nSize);
};
//#endif
#endif // !defined(AFX_CREATEFACE_H__EF0A4EA0_84A6_43FC_9922_98B9782A1152__INCLUDED_)
