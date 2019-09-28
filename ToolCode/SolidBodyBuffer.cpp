#include "stdafx.h"
#include ".\solidbodybuffer.h"
#include ".\SolidBody.h"

#ifndef _DISABLE_DEBUG_NEW_	//TIDCORE_EXPORTS
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

const short CSolidBodyBuffer::RAWFACE_INDEXDATA = 1;
CSolidBodyBuffer::CSolidBodyBuffer(int nBlockSize/* = 1024*/)
{
	buffer = NULL;
	buffer_len = file_len = 0;
	log_mem_position = mem_cursor = 0;
	m_nBlockSize = nBlockSize;
	m_bExternalBuf=false;
}
CSolidBodyBuffer::CSolidBodyBuffer(char* srcBuf, DWORD buf_size)
{
	buffer = srcBuf;
	buffer_len = file_len = buf_size;
	log_mem_position = mem_cursor = 0;
	m_nBlockSize = buf_size;
	m_bExternalBuf=true;
}

CSolidBodyBuffer::~CSolidBodyBuffer(void)
{
}
bool CSolidBodyBuffer::AllocateMemory(DWORD buf_size)
{
	char* data=buf_size>0 ? new char[buf_size] : NULL;
	if(data==NULL)	//内存分配失败
		return false;
	file_len=min(buf_size,file_len);
	if(file_len>0&&data!=NULL)
		memcpy(data,buffer,min(buf_size,file_len));
	if(!m_bExternalBuf&&file_len>0&&data!=NULL)
		delete []buffer;
	buffer=data;
	buffer_len=buf_size;
	mem_cursor=0;
	return true;
}
bool CSolidBodyBuffer::SeekToSection(short section)
{
	if(section==RAWFACE_INDEXDATA)
	{
		if(!SeekPosition(45))
		{
			SeekToEnd();
			Write(NULL,45-file_len);
		}
		return true;
	}
	return false;
}

WORD CSolidBodyBuffer::GetPolyFaceBuffLength(int i)
{
	SeekPosition(RawFaceIndexStartAddr+4*i);
	DWORD offset;
	ReadDword(&offset);
	SeekPosition(offset);
	WORD facebuf_len=0;
	ReadWord(&facebuf_len);
	return facebuf_len;
}
char* CSolidBodyBuffer::GetPolyFaceBuffer(int i)
{
	SeekPosition(RawFaceIndexStartAddr+4*i);
	DWORD offset;
	ReadDword(&offset);
	SeekPosition(offset+2);
	return GetCursorBuffer();
}
WORD CSolidBodyBuffer::GetRawEdgeBuffLength(int i)
{
	SeekPosition(EdgeIndexStartAddr+4*i);
	DWORD offset;
	ReadDword(&offset);
	SeekPosition(offset);
	WORD edgebuf_len=0;
	ReadWord(&edgebuf_len);
	return edgebuf_len;
}
char* CSolidBodyBuffer::GetRawEdgeBuffer(int i)
{
	SeekPosition(EdgeIndexStartAddr+4*i);
	DWORD offset;
	ReadDword(&offset);
	SeekPosition(offset+2);
	return GetCursorBuffer();
}
WORD CSolidBodyBuffer::GetBasicFaceBuffLength(int i)
{
	SeekPosition(BasicFaceIndexStartAddr+4*i);
	DWORD offset;
	ReadDword(&offset);
	SeekPosition(offset);
	WORD facebuf_len=0;
	ReadWord(&facebuf_len);
	return facebuf_len;
}
char* CSolidBodyBuffer::GetBasicFaceBuffer(int i)
{
	SeekPosition(BasicFaceIndexStartAddr+4*i);
	DWORD offset;
	ReadDword(&offset);
	SeekPosition(offset+2);
	return GetCursorBuffer();
}

bool CSolidBodyBuffer::MergeBodyBuffer(char *pExterBuffer, DWORD size,CSolidBodyBuffer* pTurboBuf/*=NULL*/)
{
	CSolidBodyBuffer extersolid(pExterBuffer,size);
	if(m_bExternalBuf)
		return false;	//当前缓存中的内存必须是自生的，外部引用的缓存无法扩充返回false
	if(GetLength()==0)
	{	//原始缓存为空时，直接进行拷贝
		if(buffer&&!m_bExternalBuf)
			delete []buffer;
		if(size>0)
			buffer=new char[size];
		buffer_len = file_len = size;
		log_mem_position = mem_cursor = 0;
		if(size>0)
			memcpy(buffer,pExterBuffer,size);
		return true;
	}
	CSolidBodyBuffer legacy;
	if(pTurboBuf==NULL)
		pTurboBuf=&legacy;
	pTurboBuf->ClearContents();
	pTurboBuf->Write(buffer,buffer_len);	//备份初始实体数据缓存
	VertexNumber+=extersolid.VertexNumber;
	EdgeNumber+=extersolid.EdgeNumber;
	RawFaceNumber+=extersolid.RawFaceNumber;
	BasicFaceNumber+=extersolid.BasicFaceNumber;
	if(VertexRecordLength==0)
		VertexRecordLength=25;	//默认25Byte
	if(VertexRecordLength!=extersolid.VertexRecordLength)
		return false;
	DWORD position;
	//合并面索引数据
	RawFaceIndexStartAddr=45;
	SeekToSection(RAWFACE_INDEXDATA);
	SeekPosition(45);
	pTurboBuf->SeekPosition(45);
	Write(pTurboBuf->GetCursorBuffer(),pTurboBuf->RawFaceNumber*4);
	extersolid.SeekPosition(45);
	Write(extersolid.GetCursorBuffer(),extersolid.RawFaceNumber*4);
	//合并边索引数据
	position=GetCursorPosition();	//原始定义边索引区起地址偏移量
	EdgeIndexStartAddr=position;
	pTurboBuf->SeekPosition(pTurboBuf->EdgeIndexStartAddr);
	WriteAt(position,pTurboBuf->GetCursorBuffer(),pTurboBuf->EdgeNumber*4);
	extersolid.SeekPosition(extersolid.EdgeIndexStartAddr);
	Write(extersolid.GetCursorBuffer(),extersolid.EdgeNumber*4);
	//合并实体显示基本面片组索引数据
	position=GetCursorPosition();	//原始定义边索引区起地址偏移量
	BasicFaceIndexStartAddr=position;
	pTurboBuf->SeekPosition(pTurboBuf->BasicFaceIndexStartAddr);
	WriteAt(position,pTurboBuf->GetCursorBuffer(),pTurboBuf->BasicFaceNumber*4);
	extersolid.SeekPosition(extersolid.BasicFaceIndexStartAddr);
	Write(extersolid.GetCursorBuffer(),extersolid.BasicFaceNumber*4);
	//合并顶点数据区
	position=GetCursorPosition();
	VertexDataStartAddr=position;
	pTurboBuf->SeekPosition(pTurboBuf->VertexDataStartAddr);
	WriteAt(position,pTurboBuf->GetCursorBuffer(),pTurboBuf->VertexNumber*VertexRecordLength);
	extersolid.SeekPosition(extersolid.VertexDataStartAddr);
	Write(extersolid.GetCursorBuffer(),extersolid.VertexNumber*VertexRecordLength);
	//合并原始定义面数据区
	position=GetCursorPosition();
	RawFaceDataStartAddr=position;
	SeekPosition(position);
	DWORD iFace;
	pTurboBuf->SeekPosition(pTurboBuf->RawFaceDataStartAddr);
	for(iFace=0;iFace<pTurboBuf->RawFaceNumber;iFace++)
	{
		WORD facebuf_len=pTurboBuf->GetPolyFaceBuffLength(iFace);
		WriteWord(facebuf_len);
		Write(pTurboBuf->GetPolyFaceBuffer(iFace),facebuf_len);
	}
	for(iFace=0;iFace<extersolid.RawFaceNumber;iFace++)
	{
		WORD facebuf_len=extersolid.GetPolyFaceBuffLength(iFace);
		WriteWord(facebuf_len);
		Write(extersolid.GetPolyFaceBuffer(iFace),facebuf_len);
	}
	//合并原始定义边数据区
	position=GetCursorPosition();
	EdgeDataStartAddr=position;
	SeekPosition(position);
	pTurboBuf->SeekPosition(pTurboBuf->EdgeDataStartAddr);
	DWORD iEdge;
	for(iEdge=0;iEdge<pTurboBuf->EdgeNumber;iEdge++)
	{
		WORD edgebuf_len=pTurboBuf->GetRawEdgeBuffLength(iEdge);
		WriteWord(edgebuf_len);
		Write(pTurboBuf->GetRawEdgeBuffer(iEdge),edgebuf_len);
	}
	for(iEdge=0;iEdge<extersolid.EdgeNumber;iEdge++)
	{
		WORD edgebuf_len=extersolid.GetRawEdgeBuffLength(iEdge);
		WriteWord(edgebuf_len);
		Write(extersolid.GetRawEdgeBuffer(iEdge),edgebuf_len);
	}
	//合并实体显示基本面片组数据区
	position=GetCursorPosition();
	BasicFaceDataStartAddr=position;
	SeekPosition(position);
	pTurboBuf->SeekPosition(pTurboBuf->BasicFaceDataStartAddr);
	for(iFace=0;iFace<pTurboBuf->BasicFaceNumber;iFace++)
	{
		WORD facebuf_len=pTurboBuf->GetBasicFaceBuffLength(iFace);
		WriteWord(facebuf_len);
		Write(pTurboBuf->GetBasicFaceBuffer(iFace),facebuf_len);
	}
	for(iFace=0;iFace<extersolid.BasicFaceNumber;iFace++)
	{
		WORD facebuf_len=extersolid.GetBasicFaceBuffLength(iFace);
		WriteWord(facebuf_len);
		Write(extersolid.GetBasicFaceBuffer(iFace),facebuf_len);
	}
	//更新面索引数据
	//更新边索引数据
	//更新实体显示基本面片组索引数据
	//更新原始定义边数据区
	SeekPosition(EdgeDataStartAddr);
	for(iEdge=0;iEdge<EdgeNumber;iEdge++)
	{
		DWORD position=GetCursorPosition();
			//1.更新边索引地址
		memcpy(buffer+EdgeIndexStartAddr+iEdge*4,&position,4);
			//2.更新边始终端顶点标识索引号
		WORD edge_len=0;
		ReadWord(&edge_len);
		CRawSolidEdge edge;
		edge.InitBuffer(GetCursorBuffer(),edge_len,true);
		if(iEdge>=pTurboBuf->EdgeNumber)
		{	//外部并入的边，始终端顶点号应增加原始实体顶点总数
			edge.LineStartId+=pTurboBuf->VertexNumber;
			edge.LineEndId+=pTurboBuf->VertexNumber;
		}
		Offset(edge_len);
	}
	//更新原始定义面数据区
	SeekPosition(RawFaceDataStartAddr);
	for(iFace=0;iFace<RawFaceNumber;iFace++)
	{
		DWORD position=GetCursorPosition();
			//1.更新边索引地址
		memcpy(buffer+RawFaceIndexStartAddr+iFace*4,&position,4);
			//2.更新边始终端顶点标识索引号
		WORD face_len=0;
		ReadWord(&face_len);
		CRawSolidFace face;
		face.InitBuffer(GetCursorBuffer(),face_len);
		if(iFace>=pTurboBuf->RawFaceNumber)
		{	//外部并入的面，更新各环内的组成边标识索引号（增加原始实体边总数）
			if(face.BasicFaceId>0)
				face.BasicFaceId+=pTurboBuf->BasicFaceNumber;
			CFaceLoop outerloop=face.GetOutterLoop();
			for(iEdge=0;iEdge<outerloop.LoopEdgeLineNum();iEdge++)
				*((DWORD*)(outerloop.GetBuffer()+2+iEdge*4))+=pTurboBuf->EdgeNumber;
			for(WORD iLoop=0;iLoop<face.InnerLoopNum();iLoop++)
			{
				CFaceLoop innerloop=face.GetInnerLoopAt(iLoop);
				for(iEdge=0;iEdge<innerloop.LoopEdgeLineNum();iEdge++)
					*((DWORD*)(innerloop.GetBuffer()+2+iEdge*4))+=pTurboBuf->EdgeNumber;
			}
		}
		Offset(face_len);
	}
	//更新实体显示基本面片组数据区
	return true;
}
