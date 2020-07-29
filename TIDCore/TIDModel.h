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
	//������,NURBS;STRAIGHT;ARCLINE;ELLIPSE
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
	virtual COLORREF MatColor();		// �����ڼ�¼�����������Ϣ(����ʵ�)
	virtual DWORD FaceId();	//���ڱ�ʶ����������е�ĳһ�ض���
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
	BYTE Mode(){return cluster.Mode;}		//������������glBegin()����Ҫ�Ļ���ģʽ
	TID_COORD3D Normal(){return TID_COORD3D(cluster.Nx,cluster.Ny,cluster.Nz);}
	WORD FacetNumber(){return cluster.FacetNumber;}
	WORD VertexNumber(){return cluster.VertexNumber;}
	TID_COORD3D VertexAt(int i){return TID_COORD3D(cluster.VertexAt(i));}
};
class CTidGLFace  
{
public:
	struct GLFACEINFO{
		//��һλ��0.��ʾ����ɫ����Ƭ��CGLFaceGroup��ͬ��1.��ʾ����ָ��ֵ
		//�ڶ�λ��0.��ʾ��Ƭ��������Ƭ��CGLFaceGroup��ͬ��1.��ʾ����ָ��ֵ
		char clr_norm;
		char mode;		//������������glBegin()����Ҫ�Ļ���ģʽ
		WORD uVertexNum;//ÿһ��Ƭ�Ķ�����
	};
public:
	GLfloat red,green,blue,alpha;	//��ɫ
	GLdouble nx,ny,nz;				//�淨��
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
	//���ǵ�ʵ�����ݾ�����Ҫ��������ת����Ĭ��Ϊ����������ҵ����ݻ���
	CTidSolidBody(char* buf=NULL,DWORD size=0);
	void AttachBuffer(char* buf=NULL,DWORD size=0);
	void CopySolidBuffer(char* buf=NULL,DWORD size=0);
	virtual char* SolidBufferPtr(){return solid.BufferPtr();}
	virtual DWORD SolidBufferLength(){return solid.BufferLength();}
	virtual ~CTidSolidBody(void){;}
	/////////////////////////////////////////////////////////////////////
	//ITidSolidBody�ӿڻ������⺯��
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
	//��ʵ���װ������ϵfromACS��λ��װ������ϵtoACS
	virtual void TransACS(const TID_CS& fromACS,const TID_CS& toACS);
	virtual void TransFromACS(const TID_CS& fromACS);
	virtual void TransToACS(const TID_CS& toACS);
};
//////////////////////////////////////////////////////////////////////////
// TID_CPlusPlus
//�������£�
//ISteelMaterialLibrary
//1����������GetCount
//2����ȡ����GetSteelMatAt ������
//ITidTowerInstance 
//1.	 GetName  �ַ�������
//ITidPart
//1.SteelMaterial �ַ�����
//ITidAssembleBolt
//1.	���ߣ�Start\ End����������ʾ����

//���ʿ�
//TODO:δ��ɺ�������
class CSteelMaterialLibrary : public ISteelMaterialLibrary
{
public:
	CMaterialLibrarySection matsect;
	virtual int GetCount(){return matsect.TypeCount();}
	virtual TID_STEELMAT GetSteelMatAt(int i);
	virtual char QuerySteelBriefSymbol(const char *steelmark);
	virtual bool QuerySteelNameCode(char briefmark,char* steelmark);

};
//��˨���
class CTidBoltSizeSpec : public IBoltSizeSpec
{
	UINT m_id;
	UINT m_uiSeriesId;	//��˨������˨ϵ��Id
public:
	CBoltSizeSection sectdata;
	struct SOLID{CTidSolidBody bolt,nut;}solid;
	CTidBoltSizeSpec(DWORD key=0);
	void SetKey(DWORD key){m_id=key;}
	void SetSeriesId(DWORD seriesId){m_uiSeriesId=seriesId;}
public:
	virtual UINT GetId(){return m_id;}
	virtual UINT GetSeriesId(){return m_uiSeriesId;}
	virtual short GetDiameter(){return sectdata.GetDiameter();}			//��˨ֱ��
	virtual short GetLenValid(){return sectdata.GetLenValid();}			//��˨��Ч��
	virtual short GetLenNoneThread(){return sectdata.GetLenNoneThread();}		//��˨�޿۳�
	virtual short GetMaxL0Limit(){return sectdata.GetMaxL0Limit();}		//��˨ͨ������
	virtual short GetMinL0Limit(){return sectdata.GetMinL0Limit();}		//��˨ͨ������
	virtual double GetTheoryWeight(){return sectdata.GetTheoryWeight();}		//��������(kg)
	virtual short GetSpec(char* spec){return sectdata.GetSpec(spec);}	//��˨��������ַ�����ĩβ0��ֹ�ַ���
	virtual ITidSolidBody* GetBoltSolid();	//��˨ʵ��	
	virtual ITidSolidBody* GetNutSolid();		//��ñʵ��	
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
//��˨ϵ��
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
	virtual short GetBoltNutCount(){return sectdata.BoltCapCount();}//��ñ/˫ñ
	virtual short GetWasherCount(){return sectdata.BoltWasherCount();}			//������Ȧ����
	//virtual bool IsFootNail();				//�Ŷ�
	//virtual bool IsBurglarproof();			//�Ƿ�Ϊ������˨
	virtual IBoltSizeSpec* EnumFirst();
	virtual IBoltSizeSpec* EnumNext();
	virtual IBoltSizeSpec* GetBoltSizeSpecById(int id);
};
//��˨��
class CBoltSeriesLib : public IBoltSeriesLib
{
public:
	CBoltSection sectdata;
	CHashListEx<CTidBoltSeries> hashBoltSeries;
	virtual int GetCount();
	virtual IBoltSeries* GetBoltSeriesAt(int i);
	virtual IBoltSizeSpec* GetBoltSizeSpec(int seriesId,int sizeSpecId);
};
//����(�Ǹ֡��ְ塢��˨���ֹܡ��۸ֵ�)
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
	virtual double GetWidth(){return partinfo.fWidth;}	//һ�����ڴ洢�Ǹ�֫���۸ּ������Ŀ�ȡ��ֹ�ֱ������ֱ���ȡ�
	virtual double GetThick(){return partinfo.fThick;}	//һ�����ڴ洢�Ǹ�֫�񡢲۸ּ������ĺ�ȡ��ֹܱں���ں�ȡ�
	virtual double GetHeight(){return partinfo.fHeight;}	//һ�����ڴ洢����֫�Ǹֵ���һ֫���۸ֵĸ߶ȵȲ�����������
	virtual double GetWeight(){return partinfo.fWeight;}
	virtual WORD StateFlag(){return partinfo.wStateFlag;}
	virtual short GetSegStr(char* segstr);
	virtual short GetSpec(char* sizeSpec);
	virtual short GetLabel(char* label);
	virtual short GetFuncType(){return partinfo.cFuncType;}
	virtual ITidSolidBody* GetSolidPart();
	virtual UINT GetProcessBuffBytes(char* processbytes,UINT maxBufLength=0);
};
//������
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
	//�ѿ��Žӿں���
	virtual long GetId() { return m_id; }
	virtual TID_CS GetAcs();
	virtual IBoltSizeSpec *GetTidBolt();
	virtual ITidSolidBody* GetBoltSolid();
	virtual ITidSolidBody* GetNutSolid();
	virtual DWORD GetSerialId(){return bolt.wIndexId;}	//��˨�����ϵ���ڵ�������ʶ�ţ۹������ԣ�
	virtual BYTE GetLegQuad() { return bolt.cLegQuad; }
	virtual bool GetConfigBytes(BYTE* cfgword_bytes24);
	//�����Žӿں���
	virtual float Grade(){return bolt.grade;}		//��˨�ȼ�
	virtual TID_COORD3D Location(){return TID_COORD3D(bolt.origin);}		//��˨װ��ԭ��۹������ԣ�
	virtual TID_COORD3D Normal(){return TID_COORD3D(bolt.work_norm);}	//��˨�������ߣ۹������ԣ�
	virtual WORD PassThickness(){return bolt.wL0;}			//��˨ͨ��(mm)�۹������ԣ�
	virtual BYTE PropFlag(){return bolt.cPropFlag;}		//����Ҫ���ʶ�ֽڣ�ĿǰĩλΪ����ʾ������˨������λ�������۹������ԣ�
	virtual BYTE DianQuanN(){return bolt.cDianQuanN;};	//��Ȧ����,��Ȧ��ȣ۹������ԣ�
	virtual BYTE DianQuanThick(){return bolt.cDianQuanThick;}
	virtual WORD DianQuanOffset(){return bolt.wDianQuanOffset;}	//��Ȧλ��ƫ����������˨���������ع������ߵ�ƫ�������۹������ԣ�
	//ͳ�����ͣ����λΪ����ʾ��˨���������������ֽ�������κţ��۸����ռ�װ����˨���ԣ�
	//���λΪ����ʾ��˨ͬʱ��������κţ��κ�ͳ�Ʒ�Χ���ַ�����ʽ�洢�ں��������ֽ���ָ��ַ�У�
	//�θ�λΪ1��ʾ�Ƿ�Ϊ��������˨������ͣ�λ��ʾ�������ȵ����޺ţ���Ϊ����ʾ������������
	virtual BYTE StatFlag(){return bolt.cStatFlag;}
	//�κ�ǰ׺�ַ����ɺ������ַ������κ����ֲ���;�۸����ռ�װ����˨���ԣ�
	//���ͳ����������λΪ1ʱ���ˣ����ֽڱ�ʾ����˨�Ķκ�ͳ�Ʒ�Χ�ַ����Ĵ洢��ַ
	virtual int SegmentBelongStr(char* seg_str);			//�۸����ռ�װ����˨���ԣ�		
};
//�ؽ���˨װ��ʵ��
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
//��ʵ��
class CTidHeightGroup;
class CTidTowerInstance : public ITidTowerInstance
{
	long m_id;
	CTidModel* m_pModel;
	CTidHeightGroup* m_pBelongHeight;
public:
	double m_fInstanceHeight;	//��λʵ�ʸ߶�
	BYTE arrLegBitSerial[4];	//1,2,3,4���޵Ľ���λ�� ȡֵ��Χ1~192
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
	//�ڵ�
	virtual int GetNodeNum(){return Nodes.GetNodeNum();}
	virtual ITidNode* FindNode(int iKey){return Nodes.GetValue(iKey);}
	virtual ITidNode* EnumNodeFirst(){return Nodes.GetFirst();}
	virtual ITidNode* EnumNodeNext(){return Nodes.GetNext();}
	//��ͨ��˨
	virtual int GetAssembleBoltNum(){return boltAssembly.GetNodeNum();}
	virtual ITidAssembleBolt* FindAssembleBolt(int iKey){return boltAssembly.GetValue(iKey);}
	virtual ITidAssembleBolt* EnumAssembleBoltFirst(){return boltAssembly.GetFirst();}
	virtual ITidAssembleBolt* EnumAssembleBoltNext(){return boltAssembly.GetNext();}
	//ת���
	virtual int GetAssemblePartNum(){return partAssembly.GetNodeNum();}
	virtual ITidAssemblePart* FindAssemblePart(int iKey){return partAssembly.GetValue(iKey);}
	virtual ITidAssemblePart* EnumAssemblePartFirst(){return partAssembly.GetFirst();}
	virtual ITidAssemblePart* EnumAssemblePartNext(){return partAssembly.GetNext();}
	//�ؽ���˨
	virtual int GetAssembleAnchorBoltNum(){return hashAnchorBolts.GetNodeNum();}
	virtual ITidAssembleAnchorBolt* FindAnchorBolt(int iKey){return hashAnchorBolts.GetValue(iKey);}
	virtual ITidAssembleAnchorBolt* EnumFirstAnchorBolt(){return hashAnchorBolts.GetFirst();}
	virtual ITidAssembleAnchorBolt* EnumNextAnchorBolt(){return hashAnchorBolts.GetNext();}
	//
	virtual bool ExportModFile(const char* sFileName);
};
//����
class CTidHeightGroup : public ITidHeightGroup
{
	int m_id;
public:
	CTidModel* m_pModel;
	TOWER_MODULE module;	//���߷�������
	CHashListEx<CTidTowerInstance> hashTowerInstance;
	void SetKey(DWORD key){m_id=(int)key;}
	CTidModel* BelongModel(){return m_pModel;}
	//
	BYTE RetrieveSerialInitBitPos();
	//���ݽ�����ŷ�����192λ��ĺ��е�ռλ��(1~192)
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
//�ҵ�
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
	virtual bool IsCircuitDC();			//blCircuitDC		��·����: false��������,trueֱ��
	virtual BYTE GetCircuitSerial();	//ciCircuitSerial	��·���: 0�����ߣ�,1,2,3,4
	virtual BYTE GetTensionType();		//ciTensionType		��������: 1.����;2.����;3.�ն�
	virtual BYTE GetWireDirection();	//ciWireDirection	���·���: 'X' or 'Y'
	virtual BYTE GetPhaseSerial();		//ciPhaseSerial;	�����,ȡֵ:0�����ߣ�,1,2,3
	virtual BYTE GetWireType();			//ciWireType; 		�������ͣ�'E'or 1����;'C'or 2����;'J'or 3����;0.�޹���
	virtual BYTE GetHangingStyle();		//ciHangingStyle	�����������ϵĹҽӷ�ʽ'S' or 'V',0��ʾ��ͨ�ҽ�
	virtual BYTE GetPostCode();			//ciPostCode		�ҵ㸽����: ��"��1-V3"��V�����'3'��ʾ1�������ߵ�V�ҵ��еĵ�����
	virtual BYTE GetSerial();			//ciSerial			�ҵ����ţ����ݹҵ����ڻ�·����·�е�����ż���
	virtual BYTE GetPosSymbol();		//λ�ñ�ʶ
	virtual BYTE GetRelaHoleNum();
	virtual TID_COORD3D GetRelaHolePos(int index);
};
//��ģ��
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
	DWORD m_dwAssmblyIter;	//װ���¼��������
	CAssembleSection m_xAssembleSection;	//����װ��������
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
	GIM_HEAD_PROP_ITEM m_xGimFileHeadInfo;		//GIM�ļ�ͷ��Ϣ
	TOWER_PRPERTY_ITEM m_xGimPropertyInfo;		//GIM����������Ϣ
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
	//�����ҵ���Ϣ
	virtual DWORD HangPointCount(){return m_xWireNodeSection.m_wnWireNodeCount;}
	virtual ITidHangPoint* GetHangPointAt(DWORD i);
	//����������Ϣ
	virtual short HeightGroupCount() { return m_xModuleSection.GetModuleCount(); }
	virtual ITidHeightGroup* GetHeightGroupAt(short i);
	virtual ITidHeightGroup* GetHeightGroup(long hHeightSerial) { return hashHeightGroup.GetValue(hHeightSerial); }
	virtual ITidHeightGroup* EnumHeightGroupFirst() { return hashHeightGroup.GetFirst(); }
	virtual ITidHeightGroup* EnumHeightGroupNext() { return hashHeightGroup.GetNext(); }
	//�����ڵ�(��ָ������)
	virtual int GetTidNodeNum() { return hashTidNode.GetNodeNum(); }
	virtual ITidNode* EnumTidNodeFirst(long hHeightSerial = 0);
	virtual ITidNode* EnumTidNodeNext(long hHeightSerial = 0);
	//����װ�乹��(��ָ������)
	virtual int GetAssemblePartNum() { return hashAssemblePart.GetNodeNum(); }
	virtual ITidAssemblePart* EnumAssemblePartFirst(long hHeightSerial = 0);
	virtual ITidAssemblePart* EnumAssemblePartNext(long hHeightSerial = 0);
	//����װ����˨(��ָ������)
	virtual int GetAssembleBoltNum() { return hashAssembleBolt.GetNodeNum(); }
	virtual ITidAssembleBolt* EnumAssembleBoltFirst(long hHeightSerial = 0);
	virtual ITidAssembleBolt* EnumAssembleBoltNext(long hHeightSerial = 0);
	//�ļ����뵼��
	virtual bool InitTidBuffer(const char* src_buf,long buf_len);
	virtual bool ReadTidFile(const char* file_path);
	virtual bool ExportModFile(const char* sFileName);
public:
	//V1.4 ��������
	virtual double GetNamedHeightZeroZ();
	//�ؽ���˨��Ϣ
	virtual bool GetAnchorBoltSolid(CTidSolidBody* pBoltSolid,CTidSolidBody* pNutSolid);
	virtual WORD GetBasePlateThick();
	virtual WORD GetAnchorCount();
	virtual bool GetAnchorAt(short index,short* psiPosX,short* psiPosY);
	//������Ϣ
	virtual WORD GetSubLegCount(BYTE ciBodySerial);
	virtual bool GetSubLegBaseLocation(BYTE ciBodySerial, BYTE ciLegSerial, double* pos3d);
	virtual double GetSubLegBaseWidth(BYTE ciBodySerial, BYTE ciLegSerial);	//��������
};
