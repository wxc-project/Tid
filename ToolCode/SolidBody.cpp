#include "stdafx.h"
#include "HashTable.h"
#include "ArrayList.h"
#include ".\solidbody.h"

#ifndef _DISABLE_DEBUG_NEW_
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif
#ifndef _DISABLE_ALG_FUNC_
#include "f_alg_fun.h"
#else
//defined TidToolFunc.cpp
BOOL vector_trans( double* vcoord, UCS_STRU ucs, BOOL fromUcs,BOOL bSkipStandardize=TRUE);
BOOL coord_trans(double* ptcoord,UCS_STRU ucs,BOOL fromUcs,BOOL bSkipStandardize=TRUE);
static bool _LocalNormalize(double* v/*[3]*/)
{
	double squre=v[0]*v[0]+v[1]*v[1]+v[2]*v[2];
	if(0.99999998<=squre&&squre<=1.00000001)	//此时tan(a)<0.0001
		return true;
	else if(squre<1e-16)
		return false;
	double root=sqrt(squre);
	if(v[0]!=0)
		v[0]/=root;
	if(v[1]!=0)
		v[1]/=root;
	if(v[2]!=0)
		v[2]/=root;
    return true;
}
bool is_ucs_valid(const UCS_STRU& ucs)
{
    double dd;
    f3dPoint norm;
	UCS_STRU cs=ucs;
    _LocalNormalize(cs.axis_x);
    _LocalNormalize(cs.axis_y);
    _LocalNormalize(cs.axis_z);
		//1.判断是坐标系是否正交
	dd = ucs.axis_x*ucs.axis_y;
    if( fabs(dd)>eps)
    	return false;
	dd = ucs.axis_x*ucs.axis_z;
    if( fabs(dd)>eps)
    	return false;
	dd = ucs.axis_y*ucs.axis_z;
    if( fabs(dd)>eps)
    	return false;

    return true;
}
#endif

const BYTE CRawSolidEdge::NURBS		=0x00;
const BYTE CRawSolidEdge::STRAIGHT	=0x01;
const BYTE CRawSolidEdge::ARCLINE	=0x02;
const BYTE CRawSolidEdge::ELLIPSE	=0x03;
//
const BYTE CRawSolidFace::MATERIAL_COLOR	= 0;
const BYTE CRawSolidFace::WORK_NORM			= 5;
const BYTE CRawSolidFace::BASICFACE_ID		= 29;
const BYTE CRawSolidFace::INNERLOOP_N		= 33;
const BYTE CRawSolidFace::INNERLOOP_INDEX_ADDR = 35;
////////////////////////////////////////////////////////////////////////////
WORD CFaceLoop::LoopEdgeLineNum()
{
	return *((WORD*)memoryAddr);	//读取环上边线总数
}
static DWORD BuffSize(BYTE* buf,int byte_num=4){
	if(byte_num==4)
		return *((DWORD*)buf);
	else if(byte_num==2)
		return *((WORD*)buf);
	else
		return *buf;
}
DWORD CFaceLoop::LoopEdgeLineIdAt(int i)
{
	if(i<0||i>=LoopEdgeLineNum())
		return false;	//地址越界
	DWORD* lat=(DWORD*)(memoryAddr+2);	//Line Allocation Table
	DWORD edge_id=(DWORD)*(lat+i);
	return edge_id;
}
bool CFaceLoop::GetLoopEdgeLineAt(CSolidBody* pBody,int i,f3dArcLine& line)
{
	if(i<0||i>=LoopEdgeLineNum())
		return false;	//地址越界
	DWORD* lat=(DWORD*)(memoryAddr+2);	//Line Allocation Table
	DWORD edge_id=(DWORD)*(lat+i);
	if(!pBody->GetKeyEdgeLineAt((edge_id&0x7FFFFFFF)-1,line))
		return false;
	line.ID=edge_id;
	return true;
}
DWORD CFaceLoop::GetLoopEdgeLineAt(CSolidBody* pBody,int i,CRawSolidEdge& edge)
{
	if(i<0||i>=LoopEdgeLineNum())
		return 0;	//地址越界
	DWORD* lat=(DWORD*)(memoryAddr+2);	//Line Allocation Table
	DWORD edge_id=(DWORD)*(lat+i);
	if(!pBody->GetKeyEdgeLineAt((edge_id&0x7FFFFFFF)-1,edge))
		return 0;
	return edge_id;
}

DWORD CRawSolidFace::FaceId()	//用于标识多边形面链中的某一特定面
{
	return 0;//*((DWORD*)(memoryAddr+4));	//读取面的Id属性
}
COLORREF CRawSolidFace::MatColor()	// 可用于记录此面的特征信息(如材质等)
{
	return *((COLORREF*)memoryAddr);
}
WORD CRawSolidFace::InnerLoopNum()
{
	return *((WORD*)(memoryAddr+33));	//读取面中内环总数
}
CFaceLoop CRawSolidFace::GetInnerLoopAt(int i)
{
	DWORD* lat=(DWORD*)(memoryAddr+35);	//Inner Loop Allocation Table
	char* loop_buf=memoryAddr+lat[i];
	return CFaceLoop(loop_buf);
}
CFaceLoop CRawSolidFace::GetOutterLoop()
{
	return CFaceLoop(memoryAddr+35+InnerLoopNum()*4);
};
void CRawSolidEdge::InitBuffer(char* address,DWORD size,bool externalBuf/*=false*/)
{
	if(!m_bExternalBuf&&m_dwBufSize>0&&memoryAddr!=NULL)
		delete []memoryAddr;
	m_dwBufSize=size;
	if(externalBuf)
		memoryAddr=address;
	else
	{
		memoryAddr=new char[size];
		memcpy(memoryAddr,address,size);
	}
	m_bExternalBuf=externalBuf;
}
CRawSolidEdge::~CRawSolidEdge()
{
	if(!m_bExternalBuf)
		delete []memoryAddr;
}
////////////////////////////////////////////////////////////////////////////
const BYTE CFacetCluster::TRIANGLES		= 0x04;
const BYTE CFacetCluster::TRIANGLE_STRIP= 0x05;
const BYTE CFacetCluster::TRIANGLE_FAN	= 0x06;
void CFacetCluster::InitBuffer(char* address,DWORD size,bool externalBuf/*=true*/)
{
	if(m_dwBufSize>0&&memoryAddr!=NULL&&!m_bExternalBuf)
		delete []memoryAddr;
	m_dwBufSize=size;
	if(externalBuf)
		memoryAddr=address;
	else
	{
		memoryAddr=new char[size];
		memcpy(memoryAddr,address,size);
	}
	m_bExternalBuf=externalBuf;
}
void CBasicGlFace::InitBuffer(char* address,DWORD size,bool externalBuf/*=true*/)
{
	if(!m_bExternalBuf&&m_dwBufSize>0&&memoryAddr!=NULL)
		delete []memoryAddr;
	m_dwBufSize=size;
	if(externalBuf)
		memoryAddr=address;
	else
	{
		memoryAddr=new char[size];
		memcpy(memoryAddr,address,size);
	}
	m_bExternalBuf=externalBuf;
}
bool CBasicGlFace::GetFacetClusterAt(int i,CFacetCluster& cluster)
{
	if(i<0||i>=FacetClusterNumber)
		return false;
	WORD wLen=0;
	char* facet_data=memoryAddr+FacetClusterDataAddr;
	CBuffer buf(facet_data,m_dwBufSize-6);
	for(int j=0;j<i;j++)
	{
		buf.ReadWord(&wLen);
		buf.Offset(wLen);
	}
	buf.ReadWord(&wLen);
	cluster.InitBuffer(buf.GetCursorBuffer(),wLen);
	return true;
}
////////////////////////////////////////////////////////////////////////////
CSolidBody::CSolidBody(char* buf,DWORD size, bool blBuffAsAttachMemory/* = false*/)
{
	m_bModified=TRUE;
	m_dwMaxBufSize=m_dwBufSize=size;
	if (m_bExternalBuffer = blBuffAsAttachMemory)//false;
		data = buf;
	else if(size>0)
	{
		data=new char[size];
		memcpy(data,buf,size);
	}
	else
		data=NULL;
}
CSolidBody::~CSolidBody()
{
	Empty();
}
void CSolidBody::Empty()
{
	if(IsExternalSolidBuffer())
		return;	//外挂实体对象不释放内存
	m_bModified=TRUE;
	if(m_dwBufSize>0&&data!=NULL)
		delete []data;
	m_dwMaxBufSize=m_dwBufSize=0;
	data=NULL;
}
void CSolidBody::Resize(DWORD size,bool bToInternalTrueOrUnchangeFalse/*=false*/)
{
	m_bModified=TRUE;
	if(size<m_dwMaxBufSize&&data!=NULL)
		m_dwBufSize=size;
	else
	{
		if(data!=NULL)
			delete []data;
		data = new char[size];
		m_dwMaxBufSize=m_dwBufSize=size;
	}
	if(bToInternalTrueOrUnchangeFalse)
		ToInternalBuffer();
}

struct EDGE_LIST_NODE{
	CRawSolidEdge* pEdge;
	BYTE cType;	//NURBS:0x00;STRAIGHT:0x01;ARCLINE:0x02;ELLIPSE:0x03
	DWORD id;
	f3dPoint *pStart,*pEnd;
	double sect_angle;
	GEPOINT work_norm,column_norm;
	EDGE_LIST_NODE* next;
	int IsEqualWith(f3dAtomLine* pLine)
	{
		double sign=0;
		if(pStart==pLine->pStart&&pEnd==pLine->pEnd)
			sign=1;
		else if(pStart==pLine->pEnd&&pEnd==pLine->pStart)
			sign=-1;
		else
			return 0;
		if(cType==0x01)	//STRAIGHT
		{
			if(pLine->SectorAngle()!=0)
				return 0;
			else
				return (int)sign;
		}
		else //if(cType==0x02||cType==0x03)	//ARCLINE|ELLIPSE
		{
			if(cType==0x02&&!pLine->ColumnNorm().IsZero())
				return 0;	//不是圆弧，而是椭圆弧
			else if(cType==0x03&&fabs(column_norm*pLine->ColumnNorm())<EPS_COS)
				return 0;	//柱面轴线方向不同
			sign*=work_norm*pLine->WorkNorm();
			if(sign>EPS_COS)
				sign=1;
			else if(sign<-EPS_COS)
				sign=-1;
			else
				return 0;	//方向不同
			if(fabs(sign*sect_angle-pLine->SectorAngle())>EPS)
				return 0;	//角度不同
			else
			{	//方向一致(相同或反向)角度相同
				if(pStart==pLine->pStart&&pEnd==pLine->pEnd)
					return 1;
				else
					return -1;
			}
		}
	}
	EDGE_LIST_NODE(){memset(this,0,sizeof(EDGE_LIST_NODE));}
	~EDGE_LIST_NODE()
	{
		if(next)
			delete next;
		if(pEdge!=NULL)
			delete pEdge;
	}
	static void ToBuffer(CBuffer& buffer,f3dAtomLine* pLine,CHashTable<DWORD>& hashVertexPtr)
	{
		BYTE cType;
		if(pLine->SectorAngle()==0)
			cType=0x01;	//STRAIGHT
		else if(pLine->ColumnNorm().IsZero())
			cType=0x02;	//ARCLINE
		else
			cType=0x03;	//ELLIPSE
		buffer.WriteByte(cType);
		buffer.WriteByte(0x00);	//实体模式下的显示宽度，暂默认为0
		DWORD index;
		if(hashVertexPtr.GetValueAt((DWORD)pLine->pStart,index))
			buffer.WriteDword(index);//起点
		else
			buffer.WriteDword(0);
		if(hashVertexPtr.GetValueAt((DWORD)pLine->pEnd,index))
			buffer.WriteDword(index);//终点
		else
			buffer.WriteDword(0);
		if(pLine->SectorAngle()>0)
		{
			buffer.WriteDouble(pLine->SectorAngle());
			buffer.WriteDouble(pLine->Radius());
			buffer.WritePoint(pLine->Center());
			buffer.WritePoint(pLine->WorkNorm());
			if(!pLine->ColumnNorm().IsZero())	//椭圆弧
				buffer.WritePoint(pLine->ColumnNorm());
		}
	}
	DWORD UpdateEdgeInfo(f3dAtomLine *pLine,CHashTable<DWORD>& hashVertexPtr,DWORD *ID_COUNTER_PTR)
	{
		if(pEdge==NULL)
		{
			pEdge=new CRawSolidEdge();
			char stack_memory[64];
			CBuffer buffer(64);
			buffer.AttachMemory(stack_memory,64);
			ToBuffer(buffer,pLine,hashVertexPtr);
			pEdge->InitBuffer(buffer.GetBufferPtr(),buffer.GetLength(),false);

			pStart=pLine->pStart;
			pEnd=pLine->pEnd;
			if(pLine->SectorAngle()==0)
				cType=0x01;	//STRAIGHT
			else if(pLine->ColumnNorm().IsZero())
				cType=0x02;	//ARCLINE
			else
				cType=0x03;	//ELLIPSE
			work_norm=pLine->WorkNorm();
			column_norm=pLine->ColumnNorm();
			id=*ID_COUNTER_PTR;
			*ID_COUNTER_PTR+=1;
			return id;
		}
		else
		{
			short sign=IsEqualWith(pLine);
			if(sign==1)
				return id;
			else if(sign==-1)
				return id|0x80000000;
			else if(next!=NULL)
				return next->UpdateEdgeInfo(pLine,hashVertexPtr,ID_COUNTER_PTR);
			else
			{
				next= new EDGE_LIST_NODE();
				return next->UpdateEdgeInfo(pLine,hashVertexPtr,ID_COUNTER_PTR);
			}
		}
	}
};
/*static void AppendToGeneralSolidBody(fBody* pBody)
{
	//if(CSolidBody::m_pHookBody==NULL)
		return;
	f3dPoint *pStart,*pEnd;
	CHashList<f3dPoint*> hashVertexPtr;
	for(f3dPoint *pVertex=pBody->vertex.GetFirst();pVertex;pVertex=pBody->vertex.GetNext())
	{
		f3dPoint *pNewVertex=CSolidBody::m_pHookBody->vertex.append(*pVertex);
		hashVertexPtr.SetValue((DWORD)pVertex,pNewVertex);
		f3dPoint vertex=*pNewVertex;
		//coord_trans(vertex,cs,TRUE);
		//pNewVertex->Set(vertex.x,vertex.y,vertex.z);
	}
	for(f3dPolyFace *pFace=pBody->faceList.GetFirst();pFace;pFace=pBody->faceList.GetNext())
	{
		f3dPolyFace *pNewFace=CSolidBody::m_pHookBody->faceList.append();
		pNewFace->material=pFace->material;
		pNewFace->poly_norm=pFace->poly_norm;
		//vector_trans(pNewFace->poly_norm,cs,TRUE);
		f3dAtomLine *pLine,*pNewLine;
		int head_len=sizeof(GEPOINT)*3+20;
		for(fLoop *pLoop=pFace->inner_loop.GetFirst();pLoop;pLoop=pFace->inner_loop.GetNext())
		{
			fLoop *pNewLoop = pNewFace->inner_loop.append();
			for(pLine=pLoop->loop->GetFirst();pLine;pLine=pLoop->loop->GetNext())
			{
				pStart=hashVertexPtr[(DWORD)pLine->pStart];
				pEnd=hashVertexPtr[(DWORD)pLine->pEnd];
				pNewLine=pNewLoop->loop->append(pStart,pEnd);
				memcpy(pNewLine,pLine,head_len);
				pNewLine->is_positive_line=pLine->is_positive_line;
				pNewLine->m_uDisplaySlices=pLine->m_uDisplaySlices;
				//pNewLine->CoordTrans(cs,TRUE);
			}
		}
		for(pLine=pFace->outer_edge.GetFirst();pLine;pLine=pFace->outer_edge.GetNext())
		{
			pStart=hashVertexPtr[(DWORD)pLine->pStart];
			pEnd=hashVertexPtr[(DWORD)pLine->pEnd];
			pNewLine=pNewFace->outer_edge.append(pStart,pEnd);
			memcpy(pNewLine,pLine,head_len);
			pNewLine->is_positive_line=pLine->is_positive_line;
			pNewLine->m_uDisplaySlices=pLine->m_uDisplaySlices;
			//pNewLine->CoordTrans(cs,TRUE);
		}
	}
}*/
bool CSolidBody::MergeBodyBuffer(char *pExterBuffer, DWORD size,CSolidBodyBuffer* pTurboBuf/*=NULL*/)
{
	CSolidBodyBuffer solidbuf;
	if(IsExternalSolidBuffer())
		return false;
	solidbuf.MergeBodyBuffer(BufferPtr(),BufferLength());
	solidbuf.MergeBodyBuffer(pExterBuffer,size,pTurboBuf);
	if(solidbuf.GetLength()<=m_dwMaxBufSize)
	{
		memcpy(data,solidbuf.GetBufferPtr(),solidbuf.GetLength());
		m_dwBufSize=solidbuf.GetLength();
	}
	else
	{
		if(data!=NULL&&m_dwBufSize>0)
			delete []data;
		data=solidbuf.GetBufferPtr();
		m_dwMaxBufSize=m_dwBufSize=solidbuf.GetLength();
		solidbuf.DetachMemory();
	}
	return true;
}
#if !defined(DISABLE_SOLID_BODY)&&!defined(_HANDOVER_TO_CLIENT_)
#include "BufferStack.h"
bool CSolidBody::MergeListBodyBuffer(CXhSimpleList<CSolidBody*>& listBodies)
{
	if(IsExternalSolidBuffer())
		return false;
	//合并实体集数据到一个实体内
	LIST_NODE<CSolidBody*>* pSolidNode;
	CSolidBody* pSolid;
	CSolidBodyBuffer selfsolid(BufferPtr(),BufferLength());
	DWORD bufsize=m_dwBufSize;
	DWORD dwVertexCount		=selfsolid.VertexNumber;
	DWORD dwRawEdgeCount	=selfsolid.EdgeNumber;
	DWORD dwRawFaceCount	=selfsolid.RawFaceNumber;
	DWORD dwBasicFaceCount	=selfsolid.BasicFaceNumber;
	for(pSolidNode=listBodies.EnumFirst();pSolidNode;pSolidNode=listBodies.EnumNext())
	{
		if((pSolid=pSolidNode->data)==NULL)
			continue;
		CSolidBodyBuffer tmpsolidbuf(pSolid->BufferPtr(),pSolid->BufferLength());
		bufsize+=pSolid->BufferLength();
		dwVertexCount	+=tmpsolidbuf.VertexNumber;
		dwRawEdgeCount	+=tmpsolidbuf.EdgeNumber;
		dwRawFaceCount	+=tmpsolidbuf.RawFaceNumber;
		dwBasicFaceCount+=tmpsolidbuf.BasicFaceNumber;
	}
	CBufferStack bufstack;
	CSolidBodyBuffer solidbuf(bufsize);
	solidbuf.AttachStack(&bufstack);
	solidbuf.VertexNumber	=dwVertexCount;
	solidbuf.EdgeNumber		=dwRawEdgeCount;
	solidbuf.RawFaceNumber	=dwRawFaceCount;
	solidbuf.BasicFaceNumber=dwBasicFaceCount;
	solidbuf.VertexRecordLength=25;
	//合并原始面索引数据
	solidbuf.RawFaceIndexStartAddr=45;			//更新原始定义面索引地址
	solidbuf.WriteAt(45,NULL,dwRawFaceCount*4);	//开辟原始定义面索引数据区
	//合并原始边索引数据
	UINT position=solidbuf.GetCursorPosition();
	solidbuf.Write(NULL,dwRawEdgeCount*4);		//开辟原始定义边索引数据区
	int push_pos=solidbuf.PushPositionStack();
	solidbuf.EdgeIndexStartAddr=position;		//更新原始定义边索引区起地址偏移量
	solidbuf.PopPositionStack(push_pos);
	//合并实体显示基本面片组索引数据
	position=solidbuf.GetCursorPosition();
	solidbuf.Write(NULL,dwBasicFaceCount*4);	//开辟实体显示基本面片组数据区
	solidbuf.PushPositionStack();
	solidbuf.BasicFaceIndexStartAddr=position;	//更新实体显示基本面片组索引
	solidbuf.PopPositionStack();
	//更新顶点数据区起始地址
	position=solidbuf.GetCursorPosition();
	solidbuf.VertexDataStartAddr=position;
	UINT i,n;
	//合并写入顶点数据
	pSolid=this;
	solidbuf.SeekPosition(position);
	do{
		CSolidBodyBuffer tmpsolidbuf(pSolid->BufferPtr(),pSolid->BufferLength());
		solidbuf.Write(pSolid->BufferPtr()+tmpsolidbuf.VertexDataStartAddr,25*tmpsolidbuf.VertexNumber);
		if(pSolid==this)
			pSolidNode=listBodies.EnumFirst();
		else
			pSolidNode=listBodies.EnumNext();
		pSolid=pSolidNode!=NULL ? pSolidNode->data : NULL;
	}while(pSolid);
	//合并写原始定义面数据
	position=solidbuf.GetCursorPosition();
	solidbuf.RawFaceDataStartAddr=position;//更新原始定义面数据区起始地址
	solidbuf.SeekPosition(position);
	pSolid=this;
	do{
		CSolidBodyBuffer tmpsolidbuf(pSolid->BufferPtr(),pSolid->BufferLength());
		i,n=tmpsolidbuf.RawFaceNumber;
		for(i=0;i<n;i++)
		{
			WORD facebuf_len=tmpsolidbuf.GetPolyFaceBuffLength(i);
			solidbuf.WriteWord(facebuf_len);
			solidbuf.Write(tmpsolidbuf.GetPolyFaceBuffer(i),facebuf_len);
		}
		if(pSolid==this)
			pSolidNode=listBodies.EnumFirst();
		else
			pSolidNode=listBodies.EnumNext();
		pSolid=pSolidNode!=NULL ? pSolidNode->data : NULL;
	}while(pSolid);
	//合并写原始定义边数据
	position=solidbuf.GetCursorPosition();
	solidbuf.EdgeDataStartAddr=position;//更新原始定义边数据区起始地址
	solidbuf.SeekPosition(position);
	pSolid=this;
	do{
		CSolidBodyBuffer tmpsolidbuf(pSolid->BufferPtr(),pSolid->BufferLength());
		i,n=tmpsolidbuf.EdgeNumber;
		for(i=0;i<n;i++)
		{
			WORD edgebuf_len=tmpsolidbuf.GetRawEdgeBuffLength(i);
			solidbuf.WriteWord(edgebuf_len);
			solidbuf.Write(tmpsolidbuf.GetRawEdgeBuffer(i),edgebuf_len);
		}
		if(pSolid==this)
			pSolidNode=listBodies.EnumFirst();
		else
			pSolidNode=listBodies.EnumNext();
		pSolid=pSolidNode!=NULL ? pSolidNode->data : NULL;
	}while(pSolid);
	//合并写实体显示基本面片组数据
	position=solidbuf.GetCursorPosition();
	solidbuf.BasicFaceDataStartAddr=position;//更新实体显示基本面片组数据区起始地址
	solidbuf.SeekPosition(position);
	pSolid=this;
	do{
		CSolidBodyBuffer tmpsolidbuf(pSolid->BufferPtr(),pSolid->BufferLength());
		i,n=tmpsolidbuf.BasicFaceNumber;
		for(i=0;i<n;i++)
		{
			WORD facebuf_len=tmpsolidbuf.GetBasicFaceBuffLength(i);
			solidbuf.WriteWord(facebuf_len);
			solidbuf.Write(tmpsolidbuf.GetBasicFaceBuffer(i),facebuf_len);
		}
		if(pSolid==this)
			pSolidNode=listBodies.EnumFirst();
		else
			pSolidNode=listBodies.EnumNext();
		pSolid=pSolidNode!=NULL ? pSolidNode->data : NULL;
	}while(pSolid);

	//更新各项索引表及新记录索引标识
	//更新面索引数据
	//更新边索引数据
	//更新实体显示基本面片组索引数据
	//更新实体显示基本面片组索引数据
	//更新原始定义边数据区
	struct{UINT uiVertex,uiRawEdge,uiRawFace,uiBasicFace;}init,currpos;
	init.uiVertex=init.uiRawEdge=init.uiRawFace=init.uiBasicFace=0;
	currpos.uiVertex=currpos.uiRawEdge=currpos.uiRawFace=currpos.uiBasicFace=0;
	pSolid=this;
	do{
		CSolidBodyBuffer tmpsolidbuf(pSolid->BufferPtr(),pSolid->BufferLength());
		if(currpos.uiRawEdge==0)
		{
			solidbuf.SeekPosition(solidbuf.EdgeDataStartAddr);
			position=solidbuf.GetCursorPosition();
		}
		else
			solidbuf.SeekPosition(currpos.uiRawEdge);
		i,n=tmpsolidbuf.EdgeNumber;	//dwRawEdgeCount
		for(i=0;i<n;i++)
		{	
			position=solidbuf.GetCursorPosition();
			//1.更新边索引地址
			memcpy(solidbuf.GetBufferPtr()+solidbuf.EdgeIndexStartAddr+(init.uiRawEdge+i)*4,&position,4);
			//2.更新边始终端顶点标识索引号
			WORD edge_len=0;
			solidbuf.ReadWord(&edge_len);
			if(init.uiVertex>0)
			{	//外部并入的边，始终端顶点号应增加原始实体顶点总数
				CRawSolidEdge edge;
				edge.InitBuffer(solidbuf.GetCursorBuffer(),edge_len,true);
				edge.LineStartId+=init.uiVertex;
				edge.LineEndId+=init.uiVertex;
			}
			solidbuf.Offset(edge_len);
		}
		currpos.uiRawEdge=solidbuf.GetCursorPosition();
		//更新原始定义面数据区
		if(currpos.uiRawFace==0)
		{
			solidbuf.SeekPosition(solidbuf.RawFaceDataStartAddr);
			position=solidbuf.GetCursorPosition();
		}
		else
			solidbuf.SeekPosition(currpos.uiRawFace);
		i,n=tmpsolidbuf.RawFaceNumber;
		for(i=0;i<n;i++)
		{
			position=solidbuf.GetCursorPosition();
			//1.更新面索引地址
			memcpy(solidbuf.GetBufferPtr()+solidbuf.RawFaceIndexStartAddr+(i+init.uiRawFace)*4,&position,4);
			//2.更新面的边标识索引号
			WORD face_len=0,iEdge=0;
			solidbuf.ReadWord(&face_len);
			CRawSolidFace face;
			face.InitBuffer(solidbuf.GetCursorBuffer(),face_len);
			if(init.uiRawEdge>0)
			{	//外部并入的面，更新各环内的组成边标识索引号（增加原始实体边总数）
				if(face.BasicFaceId>0)
					face.BasicFaceId+=init.uiBasicFace;
				CFaceLoop outerloop=face.GetOutterLoop();
				for(iEdge=0;iEdge<outerloop.LoopEdgeLineNum();iEdge++)
					*((DWORD*)(outerloop.GetBuffer()+2+iEdge*4))+=init.uiRawEdge;
				for(WORD iLoop=0;iLoop<face.InnerLoopNum();iLoop++)
				{
					CFaceLoop innerloop=face.GetInnerLoopAt(iLoop);
					for(iEdge=0;iEdge<innerloop.LoopEdgeLineNum();iEdge++)
						*((DWORD*)(innerloop.GetBuffer()+2+iEdge*4))+=init.uiRawEdge;
				}
			}
			solidbuf.Offset(face_len);
		}
		currpos.uiRawFace=solidbuf.GetCursorPosition();
		//更新实体显示基本面片组数据区
		i,n=tmpsolidbuf.BasicFaceNumber;
		if(currpos.uiBasicFace==0)
			solidbuf.SeekPosition(solidbuf.BasicFaceDataStartAddr);
		else
			solidbuf.SeekPosition(currpos.uiBasicFace);
		WORD face_len=0;
		for(i=0;i<n;i++)
		{	//1.更新实体显示基本面片组索引地址
			position=solidbuf.GetCursorPosition();
			memcpy(solidbuf.GetBufferPtr()+solidbuf.BasicFaceIndexStartAddr+(i+init.uiBasicFace)*4,&position,4);
			solidbuf.ReadWord(&face_len);
			solidbuf.Offset(face_len);
		}
		currpos.uiBasicFace=solidbuf.GetCursorPosition();
		init.uiVertex+=tmpsolidbuf.VertexNumber;
		init.uiRawFace+=tmpsolidbuf.RawFaceNumber;
		init.uiRawEdge+=tmpsolidbuf.EdgeNumber;
		init.uiBasicFace+=tmpsolidbuf.BasicFaceNumber;
		if(pSolid==this)
			pSolidNode=listBodies.EnumFirst();
		else
			pSolidNode=listBodies.EnumNext();
		pSolid=pSolidNode!=NULL ? pSolidNode->data : NULL;
	}while(pSolid);

	if(solidbuf.GetLength()<=m_dwMaxBufSize)
	{
		memcpy(data,solidbuf.GetBufferPtr(),solidbuf.GetLength());
		m_dwBufSize=solidbuf.GetLength();
	}
	else
	{
		if(data!=NULL&&m_dwBufSize>0)
			delete []data;
		data=solidbuf.GetBufferPtr();
		m_dwBufSize=solidbuf.GetLength();
		m_dwMaxBufSize=bufsize;
		solidbuf.DetachMemory();
	}
	return true;
}
bool CSolidBody::ConvertFrom(fBody* pBody)
{
	CHashTable<DWORD> hashVertexPtr;
	CHashList<DWORD> hashEdgePtr;
	CMapList<EDGE_LIST_NODE>hashEdges;
	f3dPoint *pVertex;
	hashVertexPtr.CreateHashTable(pBody->vertex.GetNodeNum());
	DWORD index=1;
	for(pVertex=pBody->vertex.GetFirst();pVertex;pVertex=pBody->vertex.GetNext(),index++)
		hashVertexPtr.SetValueAt((DWORD)pVertex,index);
	CSolidBodyBuffer solidbuf;
	//统计边数
	DWORD ID_COUNTER=1;
	f3dPolyFace *pFace=NULL;
	for(pFace=pBody->faceList.GetFirst();pFace;pFace=pBody->faceList.GetNext())
	{
		f3dAtomLine *pAtomLine;
		EDGE_LIST_NODE* pEdgeNode;
		for(pAtomLine=pFace->outer_edge.GetFirst();pAtomLine;pAtomLine=pFace->outer_edge.GetNext())
		{
			if(pAtomLine->pStart<pAtomLine->pEnd)
				pEdgeNode=hashEdges.Add((DWORD)pAtomLine->pStart,(DWORD)pAtomLine->pEnd);
			else
				pEdgeNode=hashEdges.Add((DWORD)pAtomLine->pEnd,(DWORD)pAtomLine->pStart);
			pAtomLine->id=pEdgeNode->UpdateEdgeInfo(pAtomLine,hashVertexPtr,&ID_COUNTER);
		}
		//内环数据区
		for(fLoop* pLoop=pFace->inner_loop.GetFirst();pLoop;pLoop=pFace->inner_loop.GetNext())
		{
			for(pAtomLine=pLoop->GetFirst();pAtomLine;pAtomLine=pLoop->GetNext())
			{
				if(pAtomLine->pStart<pAtomLine->pEnd)
					pEdgeNode=hashEdges.Add((DWORD)pAtomLine->pStart,(DWORD)pAtomLine->pEnd);
				else
					pEdgeNode=hashEdges.Add((DWORD)pAtomLine->pEnd,(DWORD)pAtomLine->pStart);
				pAtomLine->id=pEdgeNode->UpdateEdgeInfo(pAtomLine,hashVertexPtr,&ID_COUNTER);
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//顶点信息头
	solidbuf.VertexNumber=pBody->vertex.GetNodeNum();	//顶点数VN
	solidbuf.VertexRecordLength=25;						//每条顶点记录的数据存储长度VL（一般是25或32）
	solidbuf.VertexDataStartAddr=0;						//顶点数据区起始地址偏移量（B）
	//原始定义面信息头
	solidbuf.RawFaceNumber=pBody->faceList.GetNodeNum();	//原始定义面数FN
	solidbuf.RawFaceIndexStartAddr=0;						//原始定义面索引区起始地址偏移量（B）
	solidbuf.RawFaceDataStartAddr=0;						//原始定义面数据区起始地址偏移量（B）
	//边信息头
	solidbuf.EdgeNumber=ID_COUNTER-1;			//原始定义边数EN
	solidbuf.EdgeIndexStartAddr=0;	//原始定义边索引区起始地址偏移量（B）
	solidbuf.EdgeDataStartAddr=0;	//原始定义边数据区起始地址偏移量（B）
	//实体显示面信息头
	solidbuf.BasicFaceNumber=0;//pBody->faceList.GetNodeNum();	//实体显示面数DN
	solidbuf.BasicFaceIndexStartAddr=0;	//实体显示面索引区起始地址偏移量（B）
	solidbuf.BasicFaceDataStartAddr=0;	//实体显示面据区起始地址偏移量（B）

	//原始定义面索引区
	solidbuf.RawFaceIndexStartAddr=solidbuf.GetLength();	//原始定义面索引区起始地址偏移量（B）
	solidbuf.SeekToSection(CSolidBodyBuffer::RAWFACE_INDEXDATA);
	solidbuf.Write(NULL,solidbuf.RawFaceNumber*4);//写入索引数据的默认0值
	//原始定义边索引区
	solidbuf.EdgeIndexStartAddr=solidbuf.RawFaceIndexStartAddr+solidbuf.RawFaceNumber*4;
	solidbuf.WriteAt(solidbuf.EdgeIndexStartAddr,NULL,0);
	solidbuf.Write(NULL,solidbuf.EdgeNumber*4);//写入索引数据的默认0值
	//实体显示面索引区
	solidbuf.BasicFaceIndexStartAddr=solidbuf.EdgeIndexStartAddr+solidbuf.EdgeNumber*4;
	//solidbuf.Write(NULL,solidbuf.BasicFaceNumber*4);		//写入索引数据的默认0值
	
	//写入顶点数据
	solidbuf.VertexDataStartAddr=solidbuf.BasicFaceIndexStartAddr+0;
	solidbuf.WriteAt(solidbuf.VertexDataStartAddr,NULL,0);
	for(pVertex=pBody->vertex.GetFirst();pVertex;pVertex=pBody->vertex.GetNext())
	{
		DWORD uByteOffset=solidbuf.GetLength();
		solidbuf.WritePoint(*pVertex);
		solidbuf.WriteByte(0);	//0表示普通顶点，1表示同时为实体线框模型中的特征标记点
		//hashVertexPtr.SetValueAt((DWORD)pVertex,uByteOffset);
	}
	//写入原始定义面数据
	solidbuf.RawFaceDataStartAddr=solidbuf.GetLength();	//原始定义面索引区起始地址偏移量（B）
	solidbuf.SeekToEnd();
	long facei=0;
	for(pFace=pBody->faceList.GetFirst();pFace;pFace=pBody->faceList.GetNext(),facei++)
	{
		DWORD addr=solidbuf.GetLength();
		solidbuf.WriteAt(solidbuf.RawFaceIndexStartAddr+facei*4,&addr,4);
		solidbuf.SeekToEnd();
		solidbuf.LogPosition();
		solidbuf.WriteWord((WORD)0);			//当前数据行长度
		DWORD start_pos=solidbuf.GetCursorPosition();
		solidbuf.WriteDword(pFace->material);	//颜色RGBA
		solidbuf.WriteByte(0x84);				//多边形
		solidbuf.WritePoint(pFace->poly_norm);	//法线数量
		solidbuf.WriteDword(0);					//对应实体显示时的基本面片的标识索引编号
		solidbuf.WriteWord((WORD)pFace->inner_loop.GetNodeNum());	//内环数量
		//内环边地址偏移（自显示颜色存储地址开始）索引
		int loop_addr=solidbuf.GetCursorPosition();
		solidbuf.Write(NULL,pFace->inner_loop.GetNodeNum()*4);
		//外环边标识编号数据
		solidbuf.WriteWord((WORD)pFace->outer_edge.GetNodeNum());
		f3dAtomLine *pAtomLine;
		for(pAtomLine=pFace->outer_edge.GetFirst();pAtomLine;pAtomLine=pFace->outer_edge.GetNext())
			solidbuf.WriteDword(pAtomLine->id);
		//内环数据区
		int loopi=0;
		for(fLoop* pLoop=pFace->inner_loop.GetFirst();pLoop;pLoop=pFace->inner_loop.GetNext(),loopi++)
		{
			//写内环地址偏移索引
			DWORD addr=solidbuf.GetLength()-start_pos;
			solidbuf.WriteAt(loop_addr+loopi*4,&addr,4);
			solidbuf.SeekToEnd();
			solidbuf.WriteWord((WORD)pLoop->GetNodeNum());
			for(pAtomLine=pLoop->GetFirst();pAtomLine;pAtomLine=pLoop->GetNext())
				solidbuf.WriteDword(pAtomLine->id);
		}
		DWORD buf_len=solidbuf.GetCursorPosition()-start_pos;
		solidbuf.RecallPosition();
		solidbuf.WriteWord((WORD)buf_len);
		solidbuf.RecallPosition();
	}
	//写入原始定义边数据
	EDGE_LIST_NODE *pEdgeNode;
	DYN_ARRAY<CRawSolidEdge*> edgeArr(solidbuf.EdgeNumber);
	for(pEdgeNode=hashEdges.GetFirst();pEdgeNode;pEdgeNode=hashEdges.GetNext())
	{
		do
		{
			edgeArr[pEdgeNode->id-1]=pEdgeNode->pEdge;
			pEdgeNode=pEdgeNode->next;
		}while(pEdgeNode);
	}
	solidbuf.SeekToEnd();
	solidbuf.EdgeDataStartAddr=solidbuf.GetLength();	//原始定义面索引区起始地址偏移量（B）
	for(DWORD i=0;i<edgeArr.Size();i++)
	{
		//写原始定义边地址偏移索引
		DWORD edgeindex_addr=solidbuf.EdgeIndexStartAddr;
		DWORD addr=solidbuf.GetLength();
		solidbuf.WriteAt(edgeindex_addr+i*4,&addr,4);
		solidbuf.SeekToEnd();
		solidbuf.WriteWord((WORD)edgeArr[i]->Size());	//当前边数据记录存储长度(不含当前2B)
		solidbuf.Write(edgeArr[i]->BufferPtr(),edgeArr[i]->Size());
	}
	solidbuf.SeekToBegin();
	ReadFrom(&solidbuf,solidbuf.GetLength());
	//条件允许时添加到外挂的fBody整体集合中
	//AppendToGeneralSolidBody(pBody);
	m_bModified=TRUE;
	return true;
}
//double *pIntersPt		返回碰撞点 wht 15-09-15
//POLYGON *pIgnoreRgn	合理碰撞区域如果碰撞点位于该区域是为合理碰撞 wht 15-09-23
BOOL CSolidBody::IsIntWith(CSolidBody *pBody,double *pIntersPt/*=NULL*/,POLYGON *pIgnoreRgn/*=NULL*/)
{
	UINT i,n;
	f3dLine straight,tm_straight;
	f3dArcLine line,otherLine;
	f3dPoint inters,face_pick,line_vec;
	n=PolyFaceNum();
	CRawSolidFace face,otherface;
	for(i=0;i<n;i++)
	{
		GetPolyFaceAt(i,face);
		CFaceLoop loop=face.GetOutterLoop();
		WORD j,edge_n=loop.LoopEdgeLineNum();
		for(j=0;j<edge_n;j++)
		{
			loop.GetLoopEdgeLineAt(this,j,line);
			straight.startPt = line.Start();
			straight.endPt   = line.End();
			line_vec = line.End()-line.Start();
			UINT i2,n2=pBody->PolyFaceNum();
			for(i2=0;i2<n2;i2++)
			{
				pBody->GetPolyFaceAt(i2,otherface);
				CFaceLoop otherloop=otherface.GetOutterLoop();
				WORD j2,edge_n2=otherloop.LoopEdgeLineNum();
				f3dPoint *pArr=new f3dPoint[edge_n2];
				for(j2=0;j2<edge_n2;j2++)
				{
					otherloop.GetLoopEdgeLineAt(pBody,j2,otherLine);
					if((otherLine.ID&0x80000000)==0)
						pArr[j2]=otherLine.Start();
					else	//负向边
						pArr[j2]=otherLine.End();
				}
				POLYGON poly;
				if(!poly.CreatePolygonRgn(pArr,edge_n2))
				{
					delete []pArr;
					continue;
				}
				else
					delete []pArr;
				int ret=Int3dlf(inters,straight.startPt,line_vec,poly.GetVertexAt(0),poly.GetAxisZ());
				if(ret!=1)
					continue;	//求交失败,线方向有误或与平面平行
				f3dPoint vec2start=straight.startPt-inters;
				f3dPoint vec2end  =straight.endPt  -inters;
				if(vec2start*vec2end>EPS)	//直线段位于多边形面的同一侧不必计算　wjh-2015.4.12
					continue;
				if(poly.PtInRgn(inters)==1&&straight.PtInLine(inters)>0)	//交点不在该线段上
				{
					if(pIgnoreRgn&&pIgnoreRgn->GetVertexCount()>2)
					{
						f3dPoint pt=inters;
						project_point(pt,pIgnoreRgn->GetVertexAt(0),pIgnoreRgn->GetAxisZ());
						int nRet=pIgnoreRgn->PtInRgn(pt);
						if(pIgnoreRgn->PtInRgn(pt)==1)
							continue;	//碰撞点在指定区域内时忽略该碰撞点
					}
					if(pIntersPt)
					{
						pIntersPt[0]=inters.x;
						pIntersPt[1]=inters.y;
						pIntersPt[2]=inters.z;
					}
					poly.PtInRgn(inters);
					return TRUE;//点在多边形域面内
				}
				else
					continue;
			}
		}
	}
	n=pBody->PolyFaceNum();
	for(i=0;i<n;i++)
	{
		pBody->GetPolyFaceAt(i,face);
		CFaceLoop loop=face.GetOutterLoop();
		WORD j,edge_n=loop.LoopEdgeLineNum();
		for(j=0;j<edge_n;j++)
		{
			loop.GetLoopEdgeLineAt(pBody,j,line);
			straight.startPt = line.Start();
			straight.endPt   = line.End();
			line_vec = line.End()-line.Start();
			UINT i2,n2=PolyFaceNum();
			for(i2=0;i2<n2;i2++)
			{
				GetPolyFaceAt(i2,otherface);
				CFaceLoop otherloop=otherface.GetOutterLoop();
				WORD j2,edge_n2=otherloop.LoopEdgeLineNum();
				f3dPoint *pArr=new f3dPoint[edge_n2];
				for(j2=0;j2<edge_n2;j2++)
				{
					otherloop.GetLoopEdgeLineAt(this,j2,otherLine);
					if((otherLine.ID&0x80000000)==0)
						pArr[j2]=otherLine.Start();
					else	//负向边
						pArr[j2]=otherLine.End();
				}
				POLYGON poly;
				if(!poly.CreatePolygonRgn(pArr,edge_n2))
				{
					delete []pArr;
					continue;
				}
				else
					delete []pArr;
				int ret=Int3dlf(inters,straight.startPt,line_vec,poly.GetVertexAt(0),poly.GetAxisZ());
				if(ret!=1)
					continue;	//求交失败,线方向有误或与平面平行
				f3dPoint vec2start=straight.startPt-inters;
				f3dPoint vec2end  =straight.endPt  -inters;
				if(vec2start*vec2end>EPS)	//直线段位于多边形面的同一侧不必计算　wjh-2015.4.12
					continue;
				if(poly.PtInRgn(inters)==1&&straight.PtInLine(inters)>0)	//交点不在该线段上
				{
					if(pIgnoreRgn&&pIgnoreRgn->GetVertexCount()>2)
					{
						f3dPoint pt=inters;
						project_point(pt,pIgnoreRgn->GetVertexAt(0),pIgnoreRgn->GetAxisZ());
						if(pIgnoreRgn->PtInRgn(pt)==1)
							continue;	//碰撞点在指定区域内时忽略该碰撞点
					}
					if(pIntersPt)
					{
						pIntersPt[0]=inters.x;
						pIntersPt[1]=inters.y;
						pIntersPt[2]=inters.z;
					}
					return TRUE;//点在多边形域面内
				}
				else
					continue;
			}
		}
	}
	return FALSE;
}
#endif
bool CSolidBody::ReadFrom(BUFFER_IO* io,DWORD size)
{
	if(data&&m_bExternalBuffer)
		return false;	//外挂内存不能再读入　wjh-2016.01.17
	else if(data&&size>m_dwMaxBufSize)
	{
		delete []data;
		m_dwMaxBufSize=0;
	}
	if(size>m_dwMaxBufSize)
	{
		data=new char[size];
		m_dwMaxBufSize=size;
		m_bExternalBuffer=false;
	}
	m_dwBufSize=io->Read(data,size);
	return true;
}
void CSolidBody::WriteTo(BUFFER_IO* io)
{
	io->Write(data,m_dwBufSize);
}
//从buf中复制实体数据缓存
bool CSolidBody::CopyBuffer(char* buf,DWORD size, bool blReallocMemBuff/* = true*/)
{
	if (!blReallocMemBuff&&size <= m_dwMaxBufSize)
	{
		if ((m_dwBufSize = size) > 0)
			memcpy(data, buf, size);
		return true;
	}
	if(data&&!m_bExternalBuffer)
	{
		if(m_dwMaxBufSize<size)
		{
			delete []data;
			m_dwMaxBufSize=size;
			data=new char[size];
		}
	}
	else
	{
		m_bExternalBuffer=false;
		if(size>0)
			data=new char[size];
		m_dwMaxBufSize=size;
	}
	if((m_dwBufSize=size)>0)
		memcpy(data,buf,size);
	return true;
}
//外挂buf为实体数据缓存（外挂数据缓存）
bool CSolidBody::AttachBuffer(char* buf,DWORD size)
{
	if(data&&!IsExternalSolidBuffer())
		delete []data;
	data=buf;
	m_dwBufSize=size;
	m_bExternalBuffer=true;
	return true;
}
//由外挂实体数据缓存转换为内部独立数据缓存
bool CSolidBody::ToInternalBuffer()
{
	if(data&&IsExternalSolidBuffer())
	{
		char* temp=new char[m_dwBufSize];
		memcpy(temp,data,m_dwBufSize);
		data=temp;
		m_bExternalBuffer=false;
		m_dwMaxBufSize=m_dwBufSize;
	}
	return true;
}

int CSolidBody::KeyPointNum()
{
	return CSolidBodyBuffer((char*)data,m_dwBufSize).VertexNumber;
}
int CSolidBody::KeyEdgeLineNum()
{
	return CSolidBodyBuffer((char*)data,m_dwBufSize).EdgeNumber;
}
int CSolidBody::PolyFaceNum()
{
	return CSolidBodyBuffer((char*)data,m_dwBufSize).RawFaceNumber;
}
int CSolidBody::BasicGLFaceNum()
{
	return CSolidBodyBuffer((char*)data,m_dwBufSize).BasicFaceNumber;
}
int CSolidBody::BasicFaceDataStartAddr()
{
	CSolidBodyBuffer solidbuf((char*)data,m_dwBufSize);
	return solidbuf.BasicFaceDataStartAddr;
}
bool CSolidBody::GetBasicGLFaceAt(int i,CBasicGlFace& face)
{
	if(i<0||i>=BasicGLFaceNum())
		return false;
	CSolidBodyBuffer solidbuf((char*)data,m_dwBufSize);
	char* sectindexdata=data+solidbuf.BasicFaceIndexStartAddr+i*4;
	char* sectdata=data+*((DWORD*)sectindexdata);
	WORD wLen=*((WORD*)sectdata);;
	face.InitBuffer(sectdata+2,wLen);
	return true;
}
GEPOINT CSolidBody::GetKeyPointAt(int i)
{
	CSolidBodyBuffer solidbuf((char*)data,m_dwBufSize);
	GEPOINT point;
	if(solidbuf.SeekPosition(solidbuf.VertexDataStartAddr+solidbuf.VertexRecordLength*i))
		solidbuf.ReadPoint(point);
	return point;
}
bool CSolidBody::GetKeyEdgeLineAt(int i,CRawSolidEdge& edge)
{
	CSolidBodyBuffer solidbuf((char*)data,m_dwBufSize);
	if(solidbuf.SeekPosition(solidbuf.EdgeIndexStartAddr+4*i))
	{
		DWORD offset;
		solidbuf.ReadDword(&offset);
		solidbuf.SeekPosition(offset);
		WORD edgebuf_len=0;
		solidbuf.ReadWord(&edgebuf_len);
		edge.InitBuffer(solidbuf.GetCursorBuffer(),edgebuf_len,true);
		return true;
	}
	else
		return false;
}
bool CSolidBody::GetKeyEdgeLineAt(int i,f3dArcLine& line)
{
	CSolidBodyBuffer solidbuf((char*)data,m_dwBufSize);
	if(solidbuf.SeekPosition(solidbuf.EdgeIndexStartAddr+4*i))
	{
		DWORD offset;
		solidbuf.ReadDword(&offset);
		solidbuf.SeekPosition(offset);
		WORD edgebuf_len=0;
		solidbuf.ReadWord(&edgebuf_len);
		BYTE edgeType;
		solidbuf.ReadByte(&edgeType);		//当前边定义类型NURBS:0;STRAIGHT:0x01;ARCLINE:0x02;ELLIPSE:0x03;
		solidbuf.ReadByte(&line.pen.width);	//当前边为特征边时，在实体模式下的显示宽度（=0表示不显示）
		if(edgebuf_len<10)
			return false;	//字节数过少，缺少信息
		DWORD start_i,end_i;
		solidbuf.ReadDword(&start_i);
		solidbuf.ReadDword(&end_i);
		GEPOINT start=GetKeyPointAt(start_i-1);
		GEPOINT end  =GetKeyPointAt(end_i-1);
		char line_data[256];
		CBuffer buf(line_data,48+edgebuf_len-10);
		buf.SeekToBegin();
		buf.WritePoint(start);
		buf.WritePoint(end);
		if(edgebuf_len>10)
			buf.Write(solidbuf.GetCursorBuffer(),edgebuf_len-10);
		line.FromByteArr(line_data,edgebuf_len+38);
		//line.ID=i+1;	//线上附加的ID属性
		return true;
	}
	else
		return false;
}
bool CSolidBody::GetPolyFaceAt(int i,CRawSolidFace& face)
{
	CSolidBodyBuffer solidbuf((char*)data,m_dwBufSize);
	if(solidbuf.SeekPosition(solidbuf.RawFaceIndexStartAddr+4*i))
	{
		DWORD offset;
		solidbuf.ReadDword(&offset);
		solidbuf.SeekPosition(offset);
		WORD facebuf_len=0;
		solidbuf.ReadWord(&facebuf_len);
		face.InitBuffer(solidbuf.GetCursorBuffer(),facebuf_len);
		return true;
	}
	else
		return false;
}
void CSolidBody::TransFromACS(const UCS_STRU& from)
{
	GECS fcs=from;
	CSolidBodyBuffer solidbuf(data,m_dwBufSize);
	//全部顶点坐标转换
	UINT i,vertex_n=solidbuf.VertexNumber;
	BYTE vertex_record_len=solidbuf.VertexRecordLength;
	char* vertex_data_addr=solidbuf.GetBufferPtr()+solidbuf.VertexDataStartAddr;
	GEPOINT vertex;
	for(i=0;i<vertex_n;i++)
	{
		memcpy(&vertex,vertex_data_addr+i*vertex_record_len,24);
		//coord_trans(vertex,from,FALSE);
		vertex=fcs.TransPToCS(vertex);
		memcpy(vertex_data_addr+i*vertex_record_len,&vertex,24);
	}
	//全部实体边中圆弧矢量参数转换
	UINT edge_n=solidbuf.EdgeNumber;
	char* edge_data_addr=solidbuf.GetBufferPtr()+solidbuf.EdgeDataStartAddr;
	CRawSolidEdge rawedge;
	WORD edgebuf_len;
	for(i=0;i<edge_n;i++)
	{
		memcpy(&edgebuf_len,edge_data_addr,2);
		edge_data_addr+=2;
		rawedge.InitBuffer(edge_data_addr,edgebuf_len,true);
		edge_data_addr+=edgebuf_len;
		if(rawedge.EdgeType==CRawSolidEdge::ARCLINE||rawedge.EdgeType==CRawSolidEdge::ELLIPSE)
		{
			vertex=rawedge.Center;
			//coord_trans(vertex,from,FALSE);
			vertex=fcs.TransPToCS(vertex);
			rawedge.Center=vertex;
			vertex=rawedge.WorkNorm;
			//vector_trans(vertex,from,FALSE);
			vertex=fcs.TransVToCS(vertex);
			rawedge.WorkNorm=vertex;
			if(rawedge.EdgeType==CRawSolidEdge::ELLIPSE)
			{
				vertex=rawedge.ColumnNorm;
				//vector_trans(vertex,from,FALSE);
				vertex=fcs.TransVToCS(vertex);
				rawedge.ColumnNorm=vertex;
			}
		}
	}
	//全部实体面中指定面法线参数转换
	UINT face_n=solidbuf.RawFaceNumber;
	WORD facebuf_len;
	CRawSolidFace rawface;
	char* rawface_data_addr=solidbuf.GetBufferPtr()+solidbuf.RawFaceDataStartAddr;
	for(i=0;i<face_n;i++)
	{
		memcpy(&facebuf_len,rawface_data_addr,2);
		rawface_data_addr+=2;
		rawface.InitBuffer(rawface_data_addr,facebuf_len);
		rawface_data_addr+=facebuf_len;
		vertex = rawface.WorkNorm;
		//vector_trans(vertex,from,FALSE);
		vertex=fcs.TransVToCS(vertex);
		rawface.WorkNorm=vertex;
	}
}
void CSolidBody::TransToACS(const UCS_STRU& to)
{
	CSolidBodyBuffer solidbuf(data,m_dwBufSize);
	GECS tcs=to;
	//全部顶点坐标转换
	UINT i,vertex_n=solidbuf.VertexNumber;
	BYTE vertex_record_len=solidbuf.VertexRecordLength;
	char* vertex_data_addr=solidbuf.GetBufferPtr()+solidbuf.VertexDataStartAddr;
	GEPOINT vertex;
	for(i=0;i<vertex_n;i++)
	{
		memcpy(&vertex,vertex_data_addr+i*vertex_record_len,24);
		//coord_trans(vertex,to,TRUE);
		vertex=tcs.TransPFromCS(vertex);
		memcpy(vertex_data_addr+i*vertex_record_len,&vertex,24);
	}
	//全部实体边中圆弧矢量参数转换
	UINT edge_n=solidbuf.EdgeNumber;
	char* edge_data_addr=solidbuf.GetBufferPtr()+solidbuf.EdgeDataStartAddr;
	CRawSolidEdge rawedge;
	WORD edgebuf_len;
	for(i=0;i<edge_n;i++)
	{
		memcpy(&edgebuf_len,edge_data_addr,2);
		edge_data_addr+=2;
		rawedge.InitBuffer(edge_data_addr,edgebuf_len,true);
		edge_data_addr+=edgebuf_len;
		if(rawedge.EdgeType==CRawSolidEdge::ARCLINE||rawedge.EdgeType==CRawSolidEdge::ELLIPSE)
		{
			vertex=rawedge.Center;
			//coord_trans(vertex,to,TRUE);
			vertex=tcs.TransPFromCS(vertex);
			rawedge.Center=vertex;
			vertex=rawedge.WorkNorm;
			//vector_trans(vertex,to,TRUE);
			vertex=tcs.TransVFromCS(vertex);
			rawedge.WorkNorm=vertex;
			if(rawedge.EdgeType==CRawSolidEdge::ELLIPSE)
			{
				vertex=rawedge.ColumnNorm;
				//vector_trans(vertex,to,TRUE);
				vertex=tcs.TransVFromCS(vertex);
				rawedge.ColumnNorm=vertex;
			}
		}
	}
	//全部实体面中指定面法线参数转换
	UINT face_n=solidbuf.RawFaceNumber;
	WORD facebuf_len;
	CRawSolidFace rawface;
	char* rawface_data_addr=solidbuf.GetBufferPtr()+solidbuf.RawFaceDataStartAddr;
	for(i=0;i<face_n;i++)
	{
		memcpy(&facebuf_len,rawface_data_addr,2);
		rawface_data_addr+=2;
		rawface.InitBuffer(rawface_data_addr,facebuf_len);
		rawface_data_addr+=facebuf_len;
		vertex = rawface.WorkNorm;
		//vector_trans(vertex,to,TRUE);
		vertex=tcs.TransVFromCS(vertex);
		rawface.WorkNorm=vertex;
	}
}
void CSolidBody::TransACS(const UCS_STRU& from_cs,const UCS_STRU& to_cs)
{
	CSolidBodyBuffer solidbuf(data,m_dwBufSize);
	//一次性单位化源坐标系及目标坐标系，以便后续坐标转换时启用快速计算模式 wjh-2016.3.06
	GECS from=from_cs;
	GECS to=to_cs;
	if(!is_ucs_valid(from_cs)||!is_ucs_valid(to_cs))
		return;

	//全部顶点坐标转换
	UINT i,vertex_n=solidbuf.VertexNumber;
	BYTE vertex_record_len=solidbuf.VertexRecordLength;
	char* vertex_data_addr=solidbuf.GetBufferPtr()+solidbuf.VertexDataStartAddr;
	GEPOINT vertex;
	for(i=0;i<vertex_n;i++)
	{
		memcpy(&vertex,vertex_data_addr+i*vertex_record_len,24);
		//coord_trans(vertex,from,FALSE,TRUE);
		vertex=from.TransPToCS(vertex);
		//coord_trans(vertex,to,TRUE,TRUE);
		vertex=to.TransPFromCS(vertex);
		memcpy(vertex_data_addr+i*vertex_record_len,&vertex,24);
	}
	//全部实体边中圆弧矢量参数转换
	UINT edge_n=solidbuf.EdgeNumber;
	char* edge_data_addr=solidbuf.GetBufferPtr()+solidbuf.EdgeDataStartAddr;
	CRawSolidEdge rawedge;
	WORD edgebuf_len;
	for(i=0;i<edge_n;i++)
	{
		memcpy(&edgebuf_len,edge_data_addr,2);
		edge_data_addr+=2;
		rawedge.InitBuffer(edge_data_addr,edgebuf_len,true);
		edge_data_addr+=edgebuf_len;
		if(rawedge.EdgeType==CRawSolidEdge::ARCLINE||rawedge.EdgeType==CRawSolidEdge::ELLIPSE)
		{
			vertex=rawedge.Center;
			//coord_trans(vertex,from,FALSE,TRUE);
			vertex=from.TransPToCS(vertex);
			//coord_trans(vertex,to,TRUE,TRUE);
			vertex=to.TransPFromCS(vertex);
			rawedge.Center=vertex;
			vertex=rawedge.WorkNorm;
			//vector_trans(vertex,from,FALSE,TRUE);
			vertex=from.TransVToCS(vertex);
			//vector_trans(vertex,to,TRUE,TRUE);
			vertex=to.TransVFromCS(vertex);
			rawedge.WorkNorm=vertex;
			if(rawedge.EdgeType==CRawSolidEdge::ELLIPSE)
			{
				vertex=rawedge.ColumnNorm;
				//vector_trans(vertex,from,FALSE,TRUE);
				vertex=from.TransVToCS(vertex);
				//vector_trans(vertex,to,TRUE,TRUE);
				vertex=to.TransVFromCS(vertex);
				rawedge.ColumnNorm=vertex;
			}
		}
	}
	//全部实体面中指定面法线参数转换
	UINT face_n=solidbuf.RawFaceNumber;
	WORD facebuf_len;
	CRawSolidFace rawface;
	char* rawface_data_addr=solidbuf.GetBufferPtr()+solidbuf.RawFaceDataStartAddr;
	for(i=0;i<face_n;i++)
	{
		memcpy(&facebuf_len,rawface_data_addr,2);
		rawface_data_addr+=2;
		rawface.InitBuffer(rawface_data_addr,facebuf_len);
		rawface_data_addr+=facebuf_len;
		vertex = rawface.WorkNorm;
		//vector_trans(vertex,from,FALSE,TRUE);
		vertex=from.TransVToCS(vertex);
		//vector_trans(vertex,to,TRUE,TRUE);
		vertex=to.TransVFromCS(vertex);
		rawface.WorkNorm=vertex;
	}
}
