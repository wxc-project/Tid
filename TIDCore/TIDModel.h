#pragma once
#include "f_ent.h"
#include "XhCharString.h"
#include "TidCplus.h"
#include "TIDBuffer.h"
#include "SolidBody.h"
#include "HashTable.h"
#include "gl/glu.h"
#include "GimDef.h"

class CTidModel;
class CTidRawSolidEdge : public ITidRawSolidEdge{
	bool reverseStartEnd;
	DWORD m_id;
	CSolidBody* m_pSolid;
public:
	CRawSolidEdge edge;
	CTidRawSolidEdge(DWORD id=0,CSolidBody* pSolid=NULL);
public:
	bool IsReverse(){return reverseStartEnd;}
	bool SetReverse(bool reverse=true){return reverseStartEnd=reverse;}
	//边类型,NURBS;STRAIGHT;ARCLINE;ELLIPSE
	virtual BYTE EdgeType();
public:
	virtual BYTE SolidDrawWidth(){return edge.SolidDrawWidth;}
	virtual DWORD LineStartId();
	virtual DWORD LineEndId();
	virtual TID_COORD3D WorkNorm();
	virtual TID_COORD3D ColumnNorm(){return TID_COORD3D(edge.ColumnNorm);}
	virtual TID_COORD3D Center(){return TID_COORD3D(edge.Center);}
};
class CTidFaceLoop : public IFaceLoop{
	DWORD m_id;
	CSolidBody* m_pSolidBody;
	CSuperHashList<CTidRawSolidEdge> hashEdges;
public:
	friend class CTidRawFace;
	CFaceLoop loop;
	CTidFaceLoop(DWORD id=0,CSolidBody* pSolidBody=NULL);
	void Empty(){hashEdges.Empty();}
	~CTidFaceLoop(){hashEdges.Empty();}
	virtual WORD EdgeLineNum(){return loop.LoopEdgeLineNum();}
	virtual bool EdgeLineAt(int index,TidArcline* pArcline);
	virtual ITidRawSolidEdge* EdgeLineAt(int index);
};
class CTidRawFace : public IRawSolidFace{
	DWORD m_id;
	CSolidBody* m_pSolidBody;
public:
	friend class CTidSolidBody;
	CRawSolidFace face;
	CTidFaceLoop outer;
	CSuperHashList<CTidFaceLoop>hashInnerLoops;
	CTidRawFace(DWORD id=0,CSolidBody* pSolidBody=NULL);
	~CTidRawFace();
	void SetKey(DWORD key){m_id=key;}
public:
	virtual COLORREF MatColor();		// 可用于记录此面的特征信息(如材质等)
	virtual DWORD FaceId();	//用于标识多边形面链中的某一特定面
	virtual WORD InnerLoopNum();
	virtual IFaceLoop* InnerLoopAt(int i);
	virtual IFaceLoop* OutterLoop();
	virtual TID_COORD3D WorkNorm();
};
class CTidFacetCluster : public IFacetCluster
{
	CFacetCluster cluster;
	friend class CTidBasicFace;
public:
	BYTE Mode(){return cluster.Mode;}		//绘制启动函数glBegin()中需要的绘制模式
	TID_COORD3D Normal(){return TID_COORD3D(cluster.Nx,cluster.Ny,cluster.Nz);}
	WORD FacetNumber(){return cluster.FacetNumber;}
	WORD VertexNumber(){return cluster.VertexNumber;}
	TID_COORD3D VertexAt(int i){return TID_COORD3D(cluster.VertexAt(i));}
};
class CTidGLFace  
{
public:
	struct GLFACEINFO{
		//第一位：0.表示面颜色与面片组CGLFaceGroup相同；1.表示特殊指定值
		//第二位：0.表示面片法线与面片组CGLFaceGroup相同；1.表示特殊指定值
		char clr_norm;
		char mode;		//绘制启动函数glBegin()中需要的绘制模式
		WORD uVertexNum;//每一面片的顶点数
	};
public:
	GLfloat red,green,blue,alpha;	//颜色
	GLdouble nx,ny,nz;				//面法线
	GLFACEINFO header;
	GLdouble *m_pVertexCoordArr;
	CTidGLFace(){memset(this,0,sizeof(CTidGLFace));header.mode=GL_TRIANGLES;}
	~CTidGLFace()
	{
		if(m_pVertexCoordArr)	//header.uVertexNum>0&&
			delete []m_pVertexCoordArr;
		m_pVertexCoordArr=NULL;
		header.uVertexNum=0;
	}
};
class CTidBasicFace : public ITidBasicFace
{
	DWORD m_id;
	CSolidBody* m_pSolidBody;
	CBasicGlFace basicface;
	friend class CTidSolidBody;
public:
	CHashList<CTidFacetCluster> listFacets;
	CTidBasicFace(DWORD id=0,CSolidBody* pSolidBody=NULL);
	void SetKey(DWORD key){m_id=key;}
	//virtual TID_COORD3D Normal();
	virtual COLORREF Color();
	virtual WORD FacetClusterNumber();
	virtual IFacetCluster* FacetClusterAt(int index);
};
class CTidSolidBody : public ITidSolidBody{
	CSolidBody solid;
public:
	BOOL m_bModified;
	CSuperHashList<CTidRawSolidEdge> hashEdges;
	CSuperHashList<CTidRawFace> hashRawFaces;
	CSuperHashList<CTidBasicFace> hashBasicFaces;
	//考虑到实体数据经常需要进行坐标转换，默认为内生而非外挂的数据缓存
	CTidSolidBody(char* buf=NULL,DWORD size=0);
	void AttachBuffer(char* buf=NULL,DWORD size=0);
	void CopySolidBuffer(char* buf=NULL,DWORD size=0);
	virtual char* SolidBufferPtr(){return solid.BufferPtr();}
	virtual DWORD SolidBufferLength(){return solid.BufferLength();}
	virtual ~CTidSolidBody(void){;}
	/////////////////////////////////////////////////////////////////////
	//ITidSolidBody接口基类虚拟函数
	virtual void ReleaseTemporaryBuffer();
	virtual int KeyPointNum();
	virtual TID_COORD3D GetKeyPointAt(int i);
	virtual int KeyEdgeLineNum();
	virtual bool GetKeyEdgeLineAt(int i,TidArcline& line);
	virtual ITidRawSolidEdge* GetKeyEdgeLineAt(int i);
	virtual int PolyFaceNum();
	virtual IRawSolidFace* GetPolyFaceAt(int i);
	virtual int BasicFaceNum();
	virtual ITidBasicFace* GetBasicFaceAt(int i);
	virtual bool SplitToBasicFacets();
	//将实体从装配坐标系fromACS移位到装配坐标系toACS
	virtual void TransACS(const TID_CS& fromACS,const TID_CS& toACS);
	virtual void TransFromACS(const TID_CS& fromACS);
	virtual void TransToACS(const TID_CS& toACS);
};
//////////////////////////////////////////////////////////////////////////
// TID_CPlusPlus
//问题如下：
//ISteelMaterialLibrary
//1．个数不对GetCount
//2．获取材质GetSteelMatAt 会死机
//ITidTowerInstance 
//1.	 GetName  字符串乱码
//ITidPart
//1.SteelMaterial 字符乱码
//ITidAssembleBolt
//1.	单线（Start\ End）的坐标显示不了

//材质库
//TODO:未完成函数更名
class CSteelMaterialLibrary : public ISteelMaterialLibrary
{
public:
	CMaterialLibrarySection matsect;
	virtual int GetCount(){return matsect.TypeCount();}
	virtual TID_STEELMAT GetSteelMatAt(int i);
	virtual char QuerySteelBriefSymbol(const char *steelmark);
	virtual bool QuerySteelNameCode(char briefmark,char* steelmark);

};
//螺栓规格
class CTidBoltSizeSpec : public IBoltSizeSpec
{
	UINT m_id;
	UINT m_uiSeriesId;	//螺栓归属螺栓系列Id
public:
	CBoltSizeSection sectdata;
	struct SOLID{CTidSolidBody bolt,nut;}solid;
	CTidBoltSizeSpec(DWORD key=0);
	void SetKey(DWORD key){m_id=key;}
	void SetSeriesId(DWORD seriesId){m_uiSeriesId=seriesId;}
public:
	virtual UINT GetId(){return m_id;}
	virtual UINT GetSeriesId(){return m_uiSeriesId;}
	virtual short GetDiameter(){return sectdata.GetDiameter();}			//螺栓直径
	virtual short GetLenValid(){return sectdata.GetLenValid();}			//螺栓有效长
	virtual short GetLenNoneThread(){return sectdata.GetLenNoneThread();}		//螺栓无扣长
	virtual short GetMaxL0Limit(){return sectdata.GetMaxL0Limit();}		//螺栓通厚上限
	virtual short GetMinL0Limit(){return sectdata.GetMinL0Limit();}		//螺栓通厚下限
	virtual double GetTheoryWeight(){return sectdata.GetTheoryWeight();}		//理论重量(kg)
	virtual short GetSpec(char* spec){return sectdata.GetSpec(spec);}	//螺栓规格描述字符（含末尾0截止字符）
	virtual ITidSolidBody* GetBoltSolid();	//螺栓实体	
	virtual ITidSolidBody* GetNutSolid();		//螺帽实体	
};
//
class CTidAnchorBoltSpec : public IAnchorBoltSpec{
public:
	short m_nBaseThick;
	ANCHORBOLT_INFO anchor_bolt;  
public:
	virtual short GetDiameter(){return anchor_bolt.d;}
	virtual short GetLenValid(){return anchor_bolt.wiLe;}
	virtual short GetLenNoneThread(){return anchor_bolt.wiLa;}
	virtual short GetPadWidth(){return anchor_bolt.wiWidth;}
	virtual short GetPadThick(){return anchor_bolt.wiThick;}
	virtual short GetHoleD(){return anchor_bolt.wiHoleD;}
	virtual short GetBasePlateHoleD(){return anchor_bolt.wiBasePlateHoleD;}
	virtual short GetBasePlateThick(){return m_nBaseThick;}
	virtual char* GetSizeSymbol(){return anchor_bolt.szSizeSymbol;}
};
//螺栓系列
class CTidBoltSeries : public IBoltSeries
{
	UINT m_id;
	long m_iterIndex;
public:
	CHashListEx<CTidBoltSizeSpec> hashBoltSizes;
	CBoltSeries sectdata;
	CTidBoltSeries(){m_id=m_iterIndex=0;}
	void SetKey(DWORD key){m_id=(long)key;}
	virtual UINT GetSeriesId(){return m_id;}
	virtual short GetName(char* name);
	virtual int GetCount(){return sectdata.BoltSizeCount();}
	virtual short GetBoltNutCount(){return sectdata.BoltCapCount();}//单帽/双帽
	virtual short GetWasherCount(){return sectdata.BoltWasherCount();}			//附带垫圈数量
	//virtual bool IsFootNail();				//脚钉
	//virtual bool IsBurglarproof();			//是否为防盗螺栓
	virtual IBoltSizeSpec* EnumFirst();
	virtual IBoltSizeSpec* EnumNext();
	virtual IBoltSizeSpec* GetBoltSizeSpecById(int id);
};
//螺栓库
class CBoltSeriesLib : public IBoltSeriesLib
{
public:
	CBoltSection sectdata;
	CHashListEx<CTidBoltSeries> hashBoltSeries;
	virtual int GetCount();
	virtual IBoltSeries* GetBoltSeriesAt(int i);
	virtual IBoltSizeSpec* GetBoltSizeSpec(int seriesId,int sizeSpecId);
};
//构件(角钢、钢板、螺栓、钢管、槽钢等)
class CTidPart : public ITidPart
{
	PART_INFO partinfo;
	DWORD m_id;
	CTidSolidBody solid;
	friend class CTidAssemblePart;
	friend class CTidPartsLib;
public:
	void SetKey(DWORD key){m_id=key;}
public:
	virtual int GetPartType(){return partinfo.cPartType;}
	virtual UINT GetSerialId(){return m_id;}
	virtual char GetBriefMark(){return partinfo.cMaterial;}
	virtual WORD GetLength(){return partinfo.wLength;}
	virtual double GetWidth(){return partinfo.fWidth;}	//一般用于存储角钢肢宽、槽钢及扁铁的宽度、钢管直径、球直径等。
	virtual double GetThick(){return partinfo.fThick;}	//一般用于存储角钢肢厚、槽钢及扁铁的厚度、钢管壁厚、球壁厚等。
	virtual double GetHeight(){return partinfo.fHeight;}	//一般用于存储不等肢角钢的另一肢厚或槽钢的高度等不常见参数。
	virtual double GetWeight(){return partinfo.fWeight;}
	virtual WORD StateFlag(){return partinfo.wStateFlag;}
	virtual short GetSegStr(char* segstr);
	virtual short GetSpec(char* sizeSpec);
	virtual short GetLabel(char* label);
	virtual short GetFuncType(){return partinfo.cFuncType;}
	virtual ITidSolidBody* GetSolidPart();
	virtual UINT GetProcessBuffBytes(char* processbytes,UINT maxBufLength=0);
};
//构件库
class CTidPartsLib : public ITidPartsLib
{
public:
	CPartSection sectdata;
	CHashListEx<CTidPart> hashTidParts;
	virtual int GetCount();
	virtual ITidPart* GetTidPartBySerialId(int serialid);
};
class CTidAssemblePart : public ITidAssemblePart
{
	long m_id;
	CTidModel* m_pModel;
	CHashListEx<CTidPart> hashTidParts;
public:
	PART_ASSEMBLY part;
	CTidSolidBody solid;
	CTidModel* BelongTidModel(){return m_pModel;}
	void SetBelongModel(CTidModel* pTidModel){m_pModel=pTidModel;}
	void SetKey(DWORD key){m_id=(long)key;}
public:
	virtual long GetId() { return m_id; }
	virtual short GetType();
	virtual ITidPart *GetPart();
	virtual bool IsHasBriefRodLine(){return part.bIsRod;}
	virtual TID_COORD3D BriefLineStart(){return TID_COORD3D(part.startPt);}
	virtual TID_COORD3D BriefLineEnd(){return TID_COORD3D(part.endPt);}
	virtual long GetStartNodeId(){return part.uiStartPointI;}
	virtual long GetEndNodeId(){return part.uiEndPointI;}
	virtual TID_CS GetAcs();
	virtual ITidSolidBody* GetSolidPart();
	virtual short GetLayer(char* sizeLayer);
	virtual BYTE GetLegQuad() { return part.cLegQuad; }
	virtual bool GetConfigBytes(BYTE* cfgword_bytes24);
};
class CTidAssembleBolt : public ITidAssembleBolt
{
	long m_id;
	CTidModel* m_pModel;
public:
	BOLT_ASSEMBLY bolt;
	CTidBoltSizeSpec sizespec;
	struct SOLID{CTidSolidBody bolt,nut;}solid;
public:
	CTidAssembleBolt();
	~CTidAssembleBolt();
	CTidModel* BelongTidModel(){return m_pModel;}
	void SetBelongModel(CTidModel* pTidModel){m_pModel=pTidModel;}
	void SetKey(DWORD key){m_id=(long)key;}
	//已开放接口函数
	virtual long GetId() { return m_id; }
	virtual TID_CS GetAcs();
	virtual IBoltSizeSpec *GetTidBolt();
	virtual ITidSolidBody* GetBoltSolid();
	virtual ITidSolidBody* GetNutSolid();
	virtual DWORD GetSerialId(){return bolt.wIndexId;}	//螺栓规格在系列内的索引标识号［共享属性］
	virtual BYTE GetLegQuad() { return bolt.cLegQuad; }
	virtual bool GetConfigBytes(BYTE* cfgword_bytes24);
	//待开放接口函数
	virtual float Grade(){return bolt.grade;}		//螺栓等级
	virtual TID_COORD3D Location(){return TID_COORD3D(bolt.origin);}		//螺栓装配原点［共享属性］
	virtual TID_COORD3D Normal(){return TID_COORD3D(bolt.work_norm);}	//螺栓工作法线［共享属性］
	virtual WORD PassThickness(){return bolt.wL0;}			//螺栓通厚(mm)［共享属性］
	virtual BYTE PropFlag(){return bolt.cPropFlag;}		//特殊要求标识字节（目前末位为１表示防盗螺栓，其余位保留）［共享属性］
	virtual BYTE DianQuanN(){return bolt.cDianQuanN;};	//垫圈数量,垫圈厚度［共享属性］
	virtual BYTE DianQuanThick(){return bolt.cDianQuanThick;}
	virtual WORD DianQuanOffset(){return bolt.wDianQuanOffset;}	//垫圈位置偏移量（自螺栓工作基点沿工作法线的偏移量）［共享属性］
	//统材类型，最高位为０表示螺栓仅归属后续４个字节所代表段号；［杆塔空间装配螺栓属性］
	//最高位为１表示螺栓同时归属多个段号，段号统计范围以字符串形式存储在后续４个字节所指地址中；
	//次高位为1表示是否为接腿上螺栓；其余低６位表示归属接腿的象限号，如为０表示归属杆塔本体
	virtual BYTE StatFlag(){return bolt.cStatFlag;}
	//段号前缀字符（可含２个字符），段号数字部分;［杆塔空间装配螺栓属性］
	//如果统材类型中首位为1时，此４个字节表示该螺栓的段号统计范围字符串的存储地址
	virtual int SegmentBelongStr(char* seg_str);			//［杆塔空间装配螺栓属性］		
};
//地脚螺栓装配实例
class CTidAssembleAnchorBolt : public ITidAssembleAnchorBolt
{
	long m_id;
	CTidModel* m_pModel;
public:
	BYTE m_ciLegQuad;
	GEPOINT xLocation;
	CTidAnchorBoltSpec xAnchorBolt;
	struct SOLID{CTidSolidBody bolt,nut;}solid;
	CTidAssembleAnchorBolt(){m_pModel=NULL;}
	CTidModel* BelongTidModel(){return m_pModel;}
	void SetBelongModel(CTidModel* pTidModel){m_pModel=pTidModel;}
	void SetKey(DWORD key){m_id=(long)key;}
	virtual long GetId(){return m_id;}
public:
	virtual IAnchorBoltSpec *GetAnchorBolt();
	virtual TID_CS GetAcs();
	virtual ITidSolidBody* GetBoltSolid();
	virtual ITidSolidBody* GetNutSolid();
	virtual BYTE BelongQuad(){return m_ciLegQuad;}
};
//
class CTidNode : public ITidNode
{
	long m_id;
	CTidModel* m_pModel;
public:
	NODE_ASSEMBLY node;
	CTidNode(){m_pModel=NULL;}
	CTidModel* BelongTidModel(){return m_pModel;}
	void SetBelongModel(CTidModel* pTidModel){m_pModel=pTidModel;}
	void SetKey(DWORD key){m_id=(long)key;}
public:
	virtual long GetId(){return m_id;}
	virtual UINT GetPointI(){return node.uiPointI;}
	virtual BYTE BelongQuad(){return node.cLegQuad;}
	virtual TID_COORD3D GetPos(){return TID_COORD3D(node.xPosition);}
	virtual short GetLayer(char* sizeLayer){
		if(sizeLayer!=NULL){
			strcpy(sizeLayer,node.szLayerSymbol);
			return strlen(sizeLayer);
		}
		else
			return 0;
	}
	virtual BYTE GetLegQuad() { return node.cLegQuad; }
	virtual bool GetConfigBytes(BYTE* cfgword_bytes24)
	{
		if (cfgword_bytes24 == NULL)
			return false;
		memcpy(cfgword_bytes24, node.cfgword.flag.bytes, 24);
		return true;
	}
};
//塔实例
class CTidHeightGroup;
class CTidTowerInstance : public ITidTowerInstance
{
	long m_id;
	CTidModel* m_pModel;
	CTidHeightGroup* m_pBelongHeight;
public:
	double m_fInstanceHeight;	//塔位实际高度
	BYTE arrLegBitSerial[4];	//1,2,3,4象限的接腿位号 取值范围1~192
	CHashListEx<CTidNode> Nodes;
	CHashListEx<CTidAssembleBolt> boltAssembly;
	CHashListEx<CTidAssemblePart> partAssembly;
	CHashListEx<CTidAssembleAnchorBolt> hashAnchorBolts;
public:
	CTidTowerInstance();
	void SetKey(DWORD key){m_id=(long)key;}
	CTidModel* BelongTidModel(){return m_pModel;}
	void InitAssemblePartAndBolt();
	//
	virtual ITidHeightGroup* BelongHeightGroup(){return (ITidHeightGroup*)m_pBelongHeight;}
	virtual ITidHeightGroup* SetBelongHeight(ITidHeightGroup* pHeightGroup);
	virtual short GetLegSerialIdByQuad(short siQuad);
	virtual double GetInstanceHeight(){return m_fInstanceHeight;}
	//节点
	virtual int GetNodeNum(){return Nodes.GetNodeNum();}
	virtual ITidNode* FindNode(int iKey){return Nodes.GetValue(iKey);}
	virtual ITidNode* EnumNodeFirst(){return Nodes.GetFirst();}
	virtual ITidNode* EnumNodeNext(){return Nodes.GetNext();}
	//普通螺栓
	virtual int GetAssembleBoltNum(){return boltAssembly.GetNodeNum();}
	virtual ITidAssembleBolt* FindAssembleBolt(int iKey){return boltAssembly.GetValue(iKey);}
	virtual ITidAssembleBolt* EnumAssembleBoltFirst(){return boltAssembly.GetFirst();}
	virtual ITidAssembleBolt* EnumAssembleBoltNext(){return boltAssembly.GetNext();}
	//转配件
	virtual int GetAssemblePartNum(){return partAssembly.GetNodeNum();}
	virtual ITidAssemblePart* FindAssemblePart(int iKey){return partAssembly.GetValue(iKey);}
	virtual ITidAssemblePart* EnumAssemblePartFirst(){return partAssembly.GetFirst();}
	virtual ITidAssemblePart* EnumAssemblePartNext(){return partAssembly.GetNext();}
	//地脚螺栓
	virtual int GetAssembleAnchorBoltNum(){return hashAnchorBolts.GetNodeNum();}
	virtual ITidAssembleAnchorBolt* FindAnchorBolt(int iKey){return hashAnchorBolts.GetValue(iKey);}
	virtual ITidAssembleAnchorBolt* EnumFirstAnchorBolt(){return hashAnchorBolts.GetFirst();}
	virtual ITidAssembleAnchorBolt* EnumNextAnchorBolt(){return hashAnchorBolts.GetNext();}
	//
	virtual bool ExportModFile(const char* sFileName);
};
//呼高
class CTidHeightGroup : public ITidHeightGroup
{
	int m_id;
public:
	CTidModel* m_pModel;
	TOWER_MODULE module;	//呼高分组名称
	CHashListEx<CTidTowerInstance> hashTowerInstance;
	void SetKey(DWORD key){m_id=(int)key;}
	CTidModel* BelongModel(){return m_pModel;}
	//
	BYTE RetrieveSerialInitBitPos();
	//根据接腿序号返回在192位配材号中的占位号(1~192)
	BYTE GetLegBitSerialFromSerialId(int serial);	
public:
	virtual int GetSerialId(){return m_id;}
	virtual int GetName(char *name,UINT maxBufLength=0);
	virtual int GetLegSerialArr(int* legSerialArr);
	virtual int GetLegSerial(double heightDifference);
	virtual double GetLegHeightDifference(int legSerial);
	virtual double GetLegHeight(int legSerial);
	virtual UINT GetNamedHeight(){return module.m_uiNamedHeight;}
	virtual ITidTowerInstance* GetTowerInstance(int legSerialQuad1, int legSerialQuad2, int legSerialQuad3, int legSerialQuad4);
	virtual double GetLowestZ();
	virtual double GetBody2LegTransitZ();
	virtual double GetBodyNamedHeight();
	virtual bool GetConfigBytes(BYTE* cfgword_bytes24);
};
//挂点
class CTidHangPoint : public ITidHangPoint
{
	int m_id;
public:
	CTidModel* m_pModel;
	WIRE_NODE wireNode;
	void SetKey(DWORD key){m_id=(int)key;}
	CTidModel* BelongModel(){return m_pModel;}
public:
	virtual TID_COORD3D GetPos(){return TID_COORD3D(wireNode.position);}
	virtual short GetWireDescription(char* sDes);
	virtual bool IsCircuitDC();			//blCircuitDC		回路类型: false（交流）,true直流
	virtual BYTE GetCircuitSerial();	//ciCircuitSerial	回路序号: 0（地线）,1,2,3,4
	virtual BYTE GetTensionType();		//ciTensionType		张力类型: 1.悬垂;2.耐张;3.终端
	virtual BYTE GetWireDirection();	//ciWireDirection	线缆方向: 'X' or 'Y'
	virtual BYTE GetPhaseSerial();		//ciPhaseSerial;	相序号,取值:0（地线）,1,2,3
	virtual BYTE GetWireType();			//ciWireType; 		挂线类型：'E'or 1地线;'C'or 2导线;'J'or 3跳线;0.无挂线
	virtual BYTE GetHangingStyle();		//ciHangingStyle	线缆在塔身上的挂接方式'S' or 'V',0表示普通挂接
	virtual BYTE GetPostCode();			//ciPostCode		挂点附加码: 如"导1-V3"中V后面的'3'表示1号相序导线的V挂点中的第三个
	virtual BYTE GetSerial();			//ciSerial			挂点的序号，根据挂点所在回路及回路中的相序号计算
	virtual BYTE GetPosSymbol();		//位置标识
	virtual BYTE GetRelaHoleNum();
	virtual TID_COORD3D GetRelaHolePos(int index);
};
//塔模型
class CTidModel : public ITidModel
{
	CTIDBuffer m_xTidBuffer;
	friend class CTidHeightGroup;
	friend class CTidTowerInstance;
	friend class CTidAssembleBolt;
	friend class CTidAssemblePart;
	friend class CTidAssembleAnchorBolt;
private:
	long m_iSerial;
	DWORD m_dwAssmblyIter;	//装配记录迭代索引
	CAssembleSection m_xAssembleSection;	//部件装配对象分区
	CPartAssemblySection m_xAssemblyParts;
	CBoltAssemblySection m_xAssemblyBolts;
	CModuleSection m_xModuleSection;
	CSubLegFoundationSection m_xFoundationSection;
	CWireNodeSection m_xWireNodeSection;
	CProjectInfoSection m_xProjectInfoSection;
	CBlockSection m_xBlockSection;
	//
	CSteelMaterialLibrary m_xSteelMatLib;
	CBoltSeriesLib m_xBoltLib;
	CTidPartsLib m_xPartLib;
	CNodeSection m_xNodeSection;
	CPartSection m_xPartSection;
	//
	DWORD m_dwBoltCount;
	DWORD m_dwPartCount;
	DWORD m_dwBlockCount;
	GIM_HEAD_PROP_ITEM m_xGimFileHeadInfo;		//GIM文件头信息
	TOWER_PRPERTY_ITEM m_xGimPropertyInfo;		//GIM工程属性信息
	CHashListEx<CTidHangPoint> hashHangPoint;
	CHashListEx<CTidHeightGroup> hashHeightGroup;
	CHashListEx<CTidNode> hashTidNode;
	CHashListEx<CTidAssemblePart> hashAssemblePart;
	CHashListEx<CTidAssembleBolt> hashAssembleBolt;
	CHashListEx<CTidAssembleAnchorBolt> hashAssembleAnchorBolt;
private:
	void InitTidDataInfo();
public:
	CTidModel(long serial=0);
	~CTidModel();
	CXhChar16 MemBufVersion(){return m_xTidBuffer.MemBufVersion();}
	//
	virtual long GetSerialId(){return m_iSerial;}
	virtual int GetTowerTypeName(char* towerTypeName,UINT maxBufLength=0);
	virtual TID_CS ModelCoordSystem();
	virtual ISteelMaterialLibrary* GetSteelMatLib();
	virtual IBoltSeriesLib* GetBoltLib();
	virtual ITidPartsLib* GetTidPartsLib();
	//遍历挂点信息
	virtual DWORD HangPointCount(){return m_xWireNodeSection.m_wnWireNodeCount;}
	virtual ITidHangPoint* GetHangPointAt(DWORD i);
	//遍历呼高信息
	virtual short HeightGroupCount() { return m_xModuleSection.GetModuleCount(); }
	virtual ITidHeightGroup* GetHeightGroupAt(short i);
	virtual ITidHeightGroup* GetHeightGroup(long hHeightSerial) { return hashHeightGroup.GetValue(hHeightSerial); }
	virtual ITidHeightGroup* EnumHeightGroupFirst() { return hashHeightGroup.GetFirst(); }
	virtual ITidHeightGroup* EnumHeightGroupNext() { return hashHeightGroup.GetNext(); }
	//遍历节点(可指定呼高)
	virtual int GetTidNodeNum() { return hashTidNode.GetNodeNum(); }
	virtual ITidNode* EnumTidNodeFirst(long hHeightSerial = 0);
	virtual ITidNode* EnumTidNodeNext(long hHeightSerial = 0);
	//遍历装配构件(可指定呼高)
	virtual int GetAssemblePartNum() { return hashAssemblePart.GetNodeNum(); }
	virtual ITidAssemblePart* EnumAssemblePartFirst(long hHeightSerial = 0);
	virtual ITidAssemblePart* EnumAssemblePartNext(long hHeightSerial = 0);
	//遍历装配螺栓(可指定呼高)
	virtual int GetAssembleBoltNum() { return hashAssembleBolt.GetNodeNum(); }
	virtual ITidAssembleBolt* EnumAssembleBoltFirst(long hHeightSerial = 0);
	virtual ITidAssembleBolt* EnumAssembleBoltNext(long hHeightSerial = 0);
	//文件导入导出
	virtual bool InitTidBuffer(const char* src_buf,long buf_len);
	virtual bool ReadTidFile(const char* file_path);
	virtual bool ExportModFile(const char* sFileName);
public:
	//V1.4 新增属性
	virtual double GetNamedHeightZeroZ();
	//地脚螺栓信息
	virtual bool GetAnchorBoltSolid(CTidSolidBody* pBoltSolid,CTidSolidBody* pNutSolid);
	virtual WORD GetBasePlateThick();
	virtual WORD GetAnchorCount();
	virtual bool GetAnchorAt(short index,short* psiPosX,short* psiPosY);
	//基础信息
	virtual WORD GetSubLegCount(BYTE ciBodySerial);
	virtual bool GetSubLegBaseLocation(BYTE ciBodySerial, BYTE ciLegSerial, double* pos3d);
	virtual double GetSubLegBaseWidth(BYTE ciBodySerial, BYTE ciLegSerial);	//基础根开
};
