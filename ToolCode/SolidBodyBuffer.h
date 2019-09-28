#pragma once
#include "Buffer.h"
#include "MemberProperty.h"

class CSolidBodyBuffer : public CBuffer
{
public:
	CSolidBodyBuffer(int nBlockSize = 1024);
	CSolidBodyBuffer(char* srcBuf, DWORD buf_size);
	~CSolidBodyBuffer(void);
	bool AllocateMemory(DWORD buf_size);
public:
	//顶点数
	PROPERTY(DWORD, VertexNumber);
	GET(VertexNumber){
		DWORD nn=0;
		if(ReadAt(0,&nn,4)==4)
			return nn;
		else
			return 0;
	}
	SET(VertexNumber){
		WriteAt(0,(void*)&value,4);
	}
	//顶点记录长度
	PROPERTY(BYTE, VertexRecordLength);
	GET(VertexRecordLength){
		BYTE len=0;
		if(ReadAt(4,&len,1)==1)
			return len;
		else
			return 0;
	}
	SET(VertexRecordLength){
		WriteAt(4,(void*)&value,1);
	}
	//顶点数据区起地址偏移量
	PROPERTY(DWORD, VertexDataStartAddr);
	GET(VertexDataStartAddr){
		DWORD addr=0;
		if(ReadAt(5,&addr,4)==4)
			return addr;
		else
			return 0;
	}
	SET(VertexDataStartAddr){
		WriteAt(5,(void*)&value,4);
	}
	//原始定义面数
	PROPERTY(DWORD, RawFaceNumber);
	GET(RawFaceNumber){
		DWORD nn=0;
		if(ReadAt(9,&nn,4)==4)
			return nn;
		else
			return 0;
	}
	SET(RawFaceNumber){
		WriteAt(9,(void*)&value,4);
	}
	//原始定义面索引区起地址偏移量
	PROPERTY(DWORD, RawFaceIndexStartAddr);
	GET(RawFaceIndexStartAddr){
		DWORD addr=0;
		if(ReadAt(13,&addr,4)==4)
			return addr;
		else
			return 0;
	}
	SET(RawFaceIndexStartAddr){
		WriteAt(13,(void*)&value,4);
	}
	//原始定义面数据区起地址偏移量
	PROPERTY(DWORD, RawFaceDataStartAddr);
	GET(RawFaceDataStartAddr){
		DWORD addr=0;
		if(ReadAt(17,&addr,4)==4)
			return addr;
		else
			return 0;
	}
	SET(RawFaceDataStartAddr){
		WriteAt(17,(void*)&value,4);
	}
	//原始定义边数
	PROPERTY(DWORD, EdgeNumber);
	GET(EdgeNumber){
		DWORD nn=0;
		if(ReadAt(21,&nn,4)==4)
			return nn;
		else
			return 0;
	}
	SET(EdgeNumber){
		WriteAt(21,(void*)&value,4);
	}
	//原始定义边索引区起地址偏移量
	PROPERTY(DWORD, EdgeIndexStartAddr);
	GET(EdgeIndexStartAddr){
		DWORD addr=0;
		if(ReadAt(25,&addr,4)==4)
			return addr;
		else
			return 0;
	}
	SET(EdgeIndexStartAddr){
		WriteAt(25,(void*)&value,4);
	}
	//原始定义边数据区起地址偏移量
	PROPERTY(DWORD, EdgeDataStartAddr);
	GET(EdgeDataStartAddr){
		DWORD addr=0;
		if(ReadAt(29,&addr,4)==4)
			return addr;
		else
			return 0;
	}
	SET(EdgeDataStartAddr){
		WriteAt(29,(void*)&value,4);
	}
	//实体显示面数
	PROPERTY(DWORD, BasicFaceNumber);
	GET(BasicFaceNumber){
		DWORD nn=0;
		if(ReadAt(33,&nn,4)==4)
			return nn;
		else
			return 0;
	}
	SET(BasicFaceNumber){
		WriteAt(33,(void*)&value,4);
	}
	//实体显示面索引区起地址偏移量
	PROPERTY(DWORD, BasicFaceIndexStartAddr);
	GET(BasicFaceIndexStartAddr){
		DWORD addr=0;
		if(ReadAt(37,&addr,4)==4)
			return addr;
		else
			return 0;
	}
	SET(BasicFaceIndexStartAddr){
		WriteAt(37,(void*)&value,4);
	}
	//实体显示面数据区起地址偏移量
	PROPERTY(DWORD, BasicFaceDataStartAddr);
	GET(BasicFaceDataStartAddr){
		DWORD addr=0;
		if(ReadAt(41,&addr,4)==4)
			return addr;
		else
			return 0;
	}
	SET(BasicFaceDataStartAddr){
		WriteAt(41,(void*)&value,4);
	}
public:
	static const short RAWFACE_INDEXDATA;
	bool SeekToSection(short section);
	WORD GetPolyFaceBuffLength(int i);
	char* GetPolyFaceBuffer(int i);
	WORD GetRawEdgeBuffLength(int i);
	char* GetRawEdgeBuffer(int i);
	WORD GetBasicFaceBuffLength(int i);
	char* GetBasicFaceBuffer(int i);
	bool MergeBodyBuffer(char *pExterBuffer, DWORD size,CSolidBodyBuffer* pTurboBuf=NULL);
};
