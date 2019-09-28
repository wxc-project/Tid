#pragma once

#include "f_ent.h"
#if !defined(DISABLE_SOLID_BODY)&&!defined(_HANDOVER_TO_CLIENT_)
#include "f_ent_list.h"
#endif
#include "list.h"
#include "MemberProperty.h"
#include "./SolidBodyBuffer.h"

/*struct DB_PAGE_DATA_HEADER{
	BYTE* data;
public:
	DB_PAGE_DATA_HEADER(){data=NULL;}
	virtual ~DB_PAGE_DATA_HEADER();
	virtual DWORD PageSize()=0;
	virtual DWORD RemnatSize()=0;
	virtual DWORD RecordNumWrited()=0;
	virtual void* NextDataPage()=0;
	virtual WORD PageType()=0;
	virtual WORD GetFixedRecordLength()=0;
	virtual DWORD GetRecAddrBytesNum()=0;
};
struct DB_PAGE_DATA_VERTEX : public DB_PAGE_DATA_HEADER{

};
//用于存储各类索引信息的索引页
struct DB_PAGE_INDEX{
	static const BYTE VERTEX_PAGE_INDEX = 1;
	static const BYTE RAWDEF_FACE_INDEX = 2;
	static const BYTE OPENGL_FACE_INDEX = 3;
	static const BYTE FRAME_EDGE_INDEX  = 4;
	WORD m_ePageType;	//页号,0为基数
	WORD m_wIndexNum;	//当前页存储顶点页索引数
	void *m_pNextPage;
	BYTE data[1024];
public:
	DB_PAGE_INDEX* SearchIndexPage(int index);
	void* SearchObjAddr(int objectIndex);
	int GetAvailableObjPage(int size);
};
class CSolidDababase{
	CHashList<DB_REC_INDEX> hashRecById;
	DB_PAGE_BUF page_arr[1024];
	DWORD m_nVertexNum,m_nFaceNum,m_nEdgeNum,m_nGLFaceNum;
	BYTE* m_pVertexIT,*m_pFaceIT,*m_pEdgeIT,*m_pGLFaceIT;
public:
	bool LoadSolidBody(long id,CSolidBody& body);
	bool SaveSolidBody(long id,CSolidBody& body);
	bool DeleteSolidBody(long id);
	bool ReadFromDbFile(char* file);
	bool WriteDbFile(char* file);
};*/
class CFacetCluster
{
	char* memoryAddr;
	DWORD m_dwBufSize;
	bool m_bExternalBuf;
public:
	static const BYTE TRIANGLES;		//= 0x04;
	static const BYTE TRIANGLE_STRIP;	//= 0x05;
	static const BYTE TRIANGLE_FAN;		//= 0x06
	CFacetCluster(){memset(this,0,sizeof(CFacetCluster));}
	inline ~CFacetCluster()
	{
		if(m_dwBufSize>0&&!m_bExternalBuf)
			delete []memoryAddr;
		m_dwBufSize=0;
	}
	void InitBuffer(char* address,DWORD size,bool externalBuf=true);
public:
	PROPERTY(BYTE, Mode);		//绘制启动函数glBegin()中需要的绘制模式
	GET(Mode){
		return (BYTE)(*memoryAddr);
	}
	SET(Mode){
		*memoryAddr=value;
	}
	PROPERTY(double, Nx);
	GET(Nx){
		return *((double*)(memoryAddr+1));
	}
	SET(Nx){
		 *((double*)(memoryAddr+1))=value;
	}
	PROPERTY(double, Ny);
	GET(Ny){
		return *((double*)(memoryAddr+9));
	}
	SET(Ny){
		*((double*)(memoryAddr+9))=value;
	}
	PROPERTY(double, Nz);
	GET(Nz){
		return *((double*)(memoryAddr+17));
	}
	SET(Nz){
		*((double*)(memoryAddr+17))=value;
	}
	READONLY_PROPERTY(GEPOINT, Normal);
	GET(Normal){return GEPOINT(Nx,Ny,Nz);}
	PROPERTY(WORD, FacetNumber);
	GET(FacetNumber){
		return *((WORD*)(memoryAddr+25));
	}
	SET(FacetNumber){
		*((WORD*)(memoryAddr+25))=value;
	}
	READONLY_PROPERTY(WORD, VertexNumber);
	GET(VertexNumber){
		if(Mode==TRIANGLE_STRIP||Mode==TRIANGLE_FAN)
			return FacetNumber+2;
		else //if(Mode==GL_TRIANGLES)
			return FacetNumber*3;
	}
	READONLY_PROPERTY(DWORD, VertexDataAddr);
	GET(VertexDataAddr){
		return 27;
	}
public:
	GEPOINT VertexAt(int i){return (double*)(memoryAddr+27+24*i);}
};
class CBasicGlFace
{
	char* memoryAddr;
	DWORD m_dwBufSize;
	bool m_bExternalBuf;
public:
	CBasicGlFace(){memset(this,0,sizeof(CBasicGlFace));}//GL_TRIANGLES=0x04;}
	inline ~CBasicGlFace()
	{
		if(m_dwBufSize>0&&!m_bExternalBuf)
			delete []memoryAddr;
		m_dwBufSize=0;
	}
	void InitBuffer(char* address,DWORD size,bool externalBuf=true);
	bool GetFacetClusterAt(int i,CFacetCluster& cluster);
public:
	READONLY_PROPERTY(DWORD, BufferSize);
	GET(BufferSize){
		return m_dwBufSize;
	}
	PROPERTY(COLORREF, Color);
	GET(Color){
		return *((COLORREF*)memoryAddr);
	}
	SET(Color){
		memcpy(&memoryAddr,&value,4);
	}
	READONLY_PROPERTY(WORD, FacetClusterNumber);
	GET(FacetClusterNumber){
		return *((WORD*)(memoryAddr+4));
	}
	READONLY_PROPERTY(DWORD, FacetClusterDataAddr);
	GET(FacetClusterDataAddr){return 6;}
};
class CRawSolidEdge{
	char* memoryAddr;
	DWORD m_dwBufSize;
	bool m_bExternalBuf;
public:
	CRawSolidEdge(){memoryAddr=NULL;m_dwBufSize=0;m_bExternalBuf=false;}
	~CRawSolidEdge();
	void InitBuffer(char* address,DWORD size,bool externalBuf=false);
	DWORD Size(){return m_dwBufSize;}
	char* BufferPtr(){return memoryAddr;}
public:
	static const BYTE NURBS;		//=0x00;
	static const BYTE STRAIGHT;		//=0x01;
	static const BYTE ARCLINE;		//=0x02;
	static const BYTE ELLIPSE;		//=0x03;
public:
	//边类型,NURBS;STRAIGHT;ARCLINE;ELLIPSE
	PROPERTY(BYTE, EdgeType);
	GET(EdgeType){
		return *memoryAddr;
	}
	SET(EdgeType){
		*memoryAddr=value;
	}
	PROPERTY(BYTE, SolidDrawWidth);
	GET(SolidDrawWidth){
		return *(memoryAddr+1);
	}
	SET(SolidDrawWidth){
		*(memoryAddr+1)=value;
	}
	PROPERTY(DWORD, LineStartId);
	GET(LineStartId){
		if(EdgeType<STRAIGHT||EdgeType>ELLIPSE)
			return 0;
		else
		{
			DWORD line_id;
			memcpy(&line_id,memoryAddr+2,4);
			return line_id;
		}
	}
	SET(LineStartId){
		if(EdgeType>=STRAIGHT&&EdgeType<=ELLIPSE)
			memcpy(memoryAddr+2,&value,4);
	}
	PROPERTY(DWORD, LineEndId);
	GET(LineEndId){
		if(EdgeType<STRAIGHT||EdgeType>ELLIPSE)
			return 0;
		else
		{
			DWORD line_id;
			memcpy(&line_id,memoryAddr+6,4);
			return line_id;
		}
	}
	SET(LineEndId){
		if(EdgeType>=STRAIGHT&&EdgeType<=ELLIPSE)
			memcpy(memoryAddr+6,&value,4);
	}
	PROPERTY(GEPOINT, Center);
	GET(Center){
		if(EdgeType<ARCLINE||EdgeType>ELLIPSE)
			return GEPOINT(0,0,0);
		else
		{
			GEPOINT pt;
			memcpy((double*)pt,memoryAddr+26,24);
			return pt;
		}
	}
	SET(Center){
		if(EdgeType>=ARCLINE&&EdgeType<=ELLIPSE)
			memcpy(memoryAddr+26,(double*)&value.x,24);
	}
	PROPERTY(GEPOINT, WorkNorm);
	GET(WorkNorm){
		if(EdgeType<ARCLINE||EdgeType>ELLIPSE)
			return GEPOINT(0,0,0);
		else
		{
			GEPOINT pt;
			memcpy((double*)pt,memoryAddr+50,24);
			return pt;
		}
	}
	SET(WorkNorm){
		if(EdgeType>=ARCLINE&&EdgeType<=ELLIPSE)
			memcpy(memoryAddr+50,(double*)&value.x,24);
	}
	PROPERTY(GEPOINT, ColumnNorm);
	GET(ColumnNorm){
		if(EdgeType!=ELLIPSE)
			return GEPOINT(0,0,0);
		else
		{
			GEPOINT pt;
			memcpy((double*)pt,memoryAddr+74,24);
			return pt;
		}
	}
	SET(ColumnNorm){
		if(EdgeType==ELLIPSE)
			memcpy(memoryAddr+74,(double*)&value.x,24);
	}
};
class CSolidBody;
class CFaceLoop{
	char* memoryAddr;
public:
	CFaceLoop(char* address=NULL){memoryAddr=address;}
	char* GetBuffer(){return memoryAddr;}
	WORD LoopEdgeLineNum();
	DWORD LoopEdgeLineIdAt(int i);
	bool GetLoopEdgeLineAt(CSolidBody* pBody,int i,f3dArcLine& line);
	DWORD GetLoopEdgeLineAt(CSolidBody* pBody,int i,CRawSolidEdge& edge);
};
class CRawSolidFace{
	char* memoryAddr;
	DWORD m_dwSize;
public:
	CRawSolidFace(){memoryAddr=NULL;}
	void InitBuffer(char* address,DWORD size){memoryAddr=address;m_dwSize=size;}
	COLORREF MatColor();		// 可用于记录此面的特征信息(如材质等)
	DWORD FaceId();	//用于标识多边形面链中的某一特定面
	WORD InnerLoopNum();
	CFaceLoop GetInnerLoopAt(int i);
	CFaceLoop GetOutterLoop();
public:
	static const BYTE MATERIAL_COLOR;	//= 0;
	static const BYTE WORK_NORM;		//= 5;
	static const BYTE BASICFACE_ID;		//= 29;
	static const BYTE INNERLOOP_N;		//= 33;
	static const BYTE INNERLOOP_INDEX_ADDR; //= 35;
public:	//公用属性
	PROPERTY(GEPOINT, WorkNorm);			//多边形面的正法线方向
	GET(WorkNorm){
		GEPOINT norm;
		memcpy((double*)norm,memoryAddr+WORK_NORM,24);
		return norm;
	}
	SET(WorkNorm){
		GEPOINT norm=value;
		double len=norm.mod();
		if(len>EPS)
			norm/=len;
		memcpy(memoryAddr+WORK_NORM,(double*)&norm.x,24);
	}
	PROPERTY(DWORD, BasicFaceId);			//多边形面的正法线方向
	GET(BasicFaceId){
		return *((DWORD*)(memoryAddr+BASICFACE_ID));
	}
	SET(BasicFaceId){
		*((DWORD*)(memoryAddr+BASICFACE_ID))=value;
	}
};
class CSolidBody
{
	bool m_bExternalBuffer;	//data指向缓存是否为外挂的数据缓存(外挂缓存在析构中不会主动释放内存)
	DWORD m_dwMaxBufSize;	//最初为data实际分配的内存空间
	DWORD m_dwBufSize;		//实体模型的实际占用内存空间m_dwBufSize<=m_dwMaxBufSize
	char* data;
public:
	BOOL m_bModified;
	//考虑到实体数据经常需要进行坐标转换，默认为内生而非外挂的数据缓存
	CSolidBody(char* buff=NULL,DWORD size=0,bool blBuffAsAttachMemory =false);
	virtual ~CSolidBody(void);
	virtual bool IsExternalSolidBuffer(){return m_bExternalBuffer;}
	void Empty();
	void Resize(DWORD size,bool bToInternalTrueOrUnchangeFalse=false);
	char* BufferPtr(){return data;}
	DWORD BufferLength(){return m_dwBufSize;}
	bool MergeBodyBuffer(char *pExterBuffer, DWORD size,CSolidBodyBuffer* pTurboBuf=NULL);
#if !defined(DISABLE_SOLID_BODY)&&!defined(_HANDOVER_TO_CLIENT_)
	bool MergeListBodyBuffer(CXhSimpleList<CSolidBody*>& listBodies);
	//pIntersPt		返回碰撞点 wht 15-09-15
	//pIgnoreRgn	合理碰撞区域如果碰撞点位于该区域是为合理碰撞 wht 15-09-23
	BOOL IsIntWith(CSolidBody *pBody,double *pIntersPt=NULL,POLYGON *pIgnoreRgn=NULL);
	virtual bool ConvertFrom(fBody* pBody);
#endif
	virtual bool ReadFrom(BUFFER_IO* io,DWORD size);
	void WriteTo(BUFFER_IO* io);
	//从buf中复制实体数据缓存
	virtual bool CopyBuffer(char* buf,DWORD size, bool blReallocMemBuff = true);
	//外挂buf为实体数据缓存（外挂数据缓存）
	virtual bool AttachBuffer(char* buf,DWORD size);
	//由外挂实体数据缓存转换为内部独立数据缓存
	bool ToInternalBuffer();
	int KeyPointNum();
	GEPOINT GetKeyPointAt(int i);
	int KeyEdgeLineNum();
	bool GetKeyEdgeLineAt(int i,f3dArcLine& line);
	bool GetKeyEdgeLineAt(int i,CRawSolidEdge& edge);
	int PolyFaceNum();
	bool GetPolyFaceAt(int i,CRawSolidFace& face);
	int BasicGLFaceNum();
	int BasicFaceDataStartAddr();
	bool GetBasicGLFaceAt(int i,CBasicGlFace& face);
	//将实体从装配坐标系fromACS移位到装配坐标系toACS
	void TransACS(const UCS_STRU& fromACS,const UCS_STRU& toACS);
	void TransFromACS(const UCS_STRU& fromACS);	//从fromAcs装配环境恢复到自身局部固接坐标系
	void TransToACS(const UCS_STRU& toACS);		//从自身局部固接坐标系转换装到toACS描述的模型环境中
};
