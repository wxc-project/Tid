// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� TIDCORE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// TIDCORE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
//#if (defined(APP_EMBEDDED_MODULE)||defined(APP_EMBEDDED_MODULE_ENCRYPT))&&!defined(DISABLE_TIDCORE_EMBEDDED)
#ifdef _TIDCORE_EMBEDDED_
#define TIDCORE_API
#else
#ifdef TIDCORE_EXPORTS
#define TIDCORE_API __declspec(dllexport)
#else
#define TIDCORE_API __declspec(dllimport)
#endif
#endif

#pragma once
#include "math.h"

//��ά����-��λmm
struct TIDCORE_API TID_COORD3D{
	double x, y, z;
	TID_COORD3D() { x = y = z = 0; }
	TID_COORD3D(double X,double Y,double Z) {x=X;y=Y;z=Z;}
	TID_COORD3D(const double* coord3d) {
		if(coord3d==NULL)
			x = y = z = 0; 
		else
		{
			x=coord3d[0];
			y=coord3d[1];
			z=coord3d[2];
		}
	}
	TID_COORD3D Set(double X=0, double Y=0, double Z=0){
		x = X;y = Y;z = Z;
		return *this;
	}
	bool IsEqual(double ex,double ey,double ez,double epsilon=1e-9) const;
	bool IsZero() const{return fabs(x)+fabs(y)+fabs(z)<1e-9;}
	operator double*(){return &x;}
	const double* operator =(double* coord){
		x=coord[0];y=coord[1];z=coord[2];
		return &x;
	}
};
//��ά����ϵ-��λmm
struct TIDCORE_API TID_CS{
	TID_COORD3D origin;
	//����ϵ��X/Y/Z������, ����������ѭ������˨��������Ϊ��λ��ʸ��
	TID_COORD3D axisX, axisY, axisZ;
	TID_CS(){axisX.x = axisY.y = axisZ.z = 1.0;}
	TID_CS Clone()
	{
		TID_CS cs;
		cs.FromCoordsSet(&origin.x);
		cs.origin.Set(origin.x, origin.y, origin.z);
		cs.axisX.Set(axisX.x, axisX.y, axisX.z);
		cs.axisY.Set(axisY.x, axisY.y, axisY.z);
		cs.axisZ.Set(axisZ.x, axisZ.y, axisZ.z);
		return cs;
	}
	void FromCoordsSet(const double* coords_12)
	{
		origin.Set(coords_12[0],coords_12[1],coords_12[2]);
		axisX.Set(coords_12[3],coords_12[ 4],coords_12[ 5]);
		axisY.Set(coords_12[6],coords_12[ 7],coords_12[ 8]);
		axisZ.Set(coords_12[9],coords_12[10],coords_12[11]);
	}
};
//����
struct TIDCORE_API TID_STEELMAT{
	char cBriefSymbol;
	char nameCodeStr[8];
};
//���ʿ�
struct TIDCORE_API ISteelMaterialLibrary{
	virtual int GetCount()=0;
	virtual TID_STEELMAT GetSteelMatAt(int i)=0;
	virtual char QuerySteelBriefSymbol(const char *steelmark)=0;
	virtual bool QuerySteelNameCode(char cBriefSymbol,char* nameCode)=0;
};
//ʵ��
struct ITidSolidBody;
struct TIDCORE_API TidArcline{
protected:
	TID_COORD3D posStart,posEnd;
	TID_COORD3D center;
	TID_COORD3D work_norm;
	TID_COORD3D column_norm;
	double radius;
	double sector_angle;//�����Ƶ�λ
public:
	TidArcline();
	int ToByteArr(char* buf);	//ֱ��48Bytes,Բ��112Bytes,��Բ��136Bytes;ע��ֻ���������Ϣ����
	void FromByteArr(char* buf,DWORD buf_size);//�ӻ����л�ԭ����ֱ��
	virtual TID_COORD3D StartPosition() const;
	virtual TID_COORD3D EndPosition() const;
	virtual TID_COORD3D ColumnNorm() const;
	virtual TID_COORD3D WorkNorm() const;
	virtual TID_COORD3D Center() const;
	virtual double Radius() const;		//��λmm
	virtual double SectorAngle() const;	//�����Ƶ�λ
	virtual double Length();			//Բ������mm
	virtual TID_COORD3D PositionInAngle(double posAngle);		//�����Ƶ�λ
	virtual TID_COORD3D TangentVecInAngle(double posAngle);	//��ʱ����תΪ��������,�����Ƶ�λ
};
struct TIDCORE_API ITidRawSolidEdge{
public:
	ITidRawSolidEdge(){;}
public:
	static const BYTE NURBS		=0x00;
	static const BYTE STRAIGHT	=0x01;
	static const BYTE ARCLINE	=0x02;
	static const BYTE ELLIPSE	=0x03;
	//������,NURBS;STRAIGHT;ARCLINE;ELLIPSE
	virtual BYTE EdgeType()=0;
public:
	virtual BYTE SolidDrawWidth()=0;
	virtual DWORD LineStartId()=0;
	virtual DWORD LineEndId()=0;
	virtual TID_COORD3D Center()=0;
	virtual TID_COORD3D WorkNorm()=0;
	virtual TID_COORD3D ColumnNorm()=0;
};
struct TIDCORE_API IFaceLoop{
	virtual WORD EdgeLineNum()=0;
	virtual bool EdgeLineAt(int index,TidArcline* pArcline)=0;
	virtual ITidRawSolidEdge* EdgeLineAt(int index)=0;
};
struct TIDCORE_API IRawSolidFace{
	virtual COLORREF MatColor()=0;		// �����ڼ�¼�����������Ϣ(����ʵ�)
	virtual DWORD FaceId()=0;	//���ڱ�ʶ����������е�ĳһ�ض���
	virtual WORD InnerLoopNum()=0;
	virtual TID_COORD3D WorkNorm()=0;
	virtual IFaceLoop* InnerLoopAt(int i)=0;
	virtual IFaceLoop* OutterLoop()=0;
};
struct TIDCORE_API IFacetCluster
{
	static const BYTE TRIANGLES		= 0x04;
	static const BYTE TRIANGLE_STRIP= 0x05;
	static const BYTE TRIANGLE_FAN	= 0x06;
public:
	virtual BYTE Mode()=0;		//������������glBegin()����Ҫ�Ļ���ģʽ
	virtual TID_COORD3D Normal()=0;
	virtual WORD FacetNumber()=0;
	virtual WORD VertexNumber()=0;
	virtual TID_COORD3D VertexAt(int index)=0;
};
struct TIDCORE_API ITidBasicFace
{
	//virtual TID_COORD3D Normal()=0;
	virtual COLORREF Color()=0;
	virtual WORD FacetClusterNumber()=0;
	virtual IFacetCluster* FacetClusterAt(int index)=0;
};
struct TIDCORE_API ITidSolidBody{
	virtual int KeyPointNum()=0;
	virtual char* SolidBufferPtr()=0;
	virtual DWORD SolidBufferLength()=0;
	virtual void ReleaseTemporaryBuffer()=0;	//�ͷ���ʱ������ɸ�ʵ��ӿڻ�ȡ��������ؽӿ����ʧЧ��wjh-2016.1.18
	virtual TID_COORD3D GetKeyPointAt(int i)=0;
	virtual int KeyEdgeLineNum()=0;
	virtual bool GetKeyEdgeLineAt(int i,TidArcline& line)=0;
	virtual ITidRawSolidEdge* GetKeyEdgeLineAt(int i)=0;
	virtual int PolyFaceNum()=0;
	virtual IRawSolidFace* GetPolyFaceAt(int i)=0;
	virtual int BasicFaceNum()=0;
	virtual ITidBasicFace* GetBasicFaceAt(int i)=0;
	virtual bool SplitToBasicFacets()=0;
	//��ʵ���װ������ϵfromACS��λ��װ������ϵtoACS
	virtual void TransACS(const TID_CS& fromACS,const TID_CS& toACS)=0;
	virtual void TransFromACS(const TID_CS& fromACS)=0;
	virtual void TransToACS(const TID_CS& toACS)=0;
};
//��˨���
struct TIDCORE_API ITidBoltNut{
	virtual short GetDiameter()=0;			//��˨ֱ��
	virtual ITidSolidBody* GetSolidBody()=0;//��˨ʵ��	
};
struct TIDCORE_API IBoltSizeSpec{
	virtual UINT GetId()=0;
	virtual UINT GetSeriesId()=0;			//��˨�������ϵ�еı�ʶ
	virtual short GetDiameter()=0;			//��˨ֱ��(mm)
	virtual short GetLenValid()=0;			//��˨��Ч��(mm)
	virtual short GetLenNoneThread()=0;		//��˨�޿۳�(mm)
	virtual double GetTheoryWeight()=0;		//��������(kg)
	virtual short GetSpec(char* spec)=0;	//��˨��������ַ�����ĩβ0��ֹ�ַ���
	virtual ITidSolidBody* GetBoltSolid()=0;//��˨ʵ��	
	virtual ITidSolidBody* GetNutSolid()=0;//��˨ʵ��	
};
struct TIDCORE_API IAnchorBoltSpec{
	virtual short GetDiameter()=0;		//�ؽ���˨����ֱ��(mm)
	virtual short GetLenValid()=0;		//�ؽ���˨��¶��(mm)
	virtual short GetLenNoneThread()=0;	//�ؽ���˨�޿۳�(mm) <��������
	virtual short GetPadWidth()=0;		//�ؽ���˨�����(mm)
	virtual short GetPadThick()=0;		//�ؽ���˨�����(mm)
	virtual short GetHoleD()=0;			//�ؽ���˨��׾�(mm)
	virtual short GetBasePlateHoleD()=0;//�ؽ���˨���ӵ����彨��׾�ֵ(mm)
	virtual short GetBasePlateThick()=0;		//�����װ���(mm)
	virtual char* GetSizeSymbol()=0;	//�ؽ���˨������ַ���
};
//��˨ϵ��
struct TIDCORE_API IBoltSeries{
	virtual UINT GetSeriesId()=0;
	virtual short GetName(char* name)=0;
	virtual int GetCount()=0;
	virtual short GetBoltNutCount()=0;		//��ñ/˫ñ
	virtual short GetWasherCount()=0;		//������Ȧ����
	//virtual bool IsFootNail()=0;			//�Ŷ�
	//virtual bool IsBurglarproof()=0;		//�Ƿ�Ϊ������˨
	virtual IBoltSizeSpec* EnumFirst()=0;
	virtual IBoltSizeSpec* EnumNext()=0;
	virtual IBoltSizeSpec* GetBoltSizeSpecById(int id)=0;
};
//��˨��
struct TIDCORE_API IBoltSeriesLib{
	virtual int GetCount()=0;
	virtual IBoltSeries* GetBoltSeriesAt(int i)=0;
	virtual IBoltSizeSpec* GetBoltSizeSpec(int seriesId,int sizeSpecId)=0;
};
//����(�Ǹ֡��ְ塢��˨���ֹܡ��۸ֵ�)
struct TIDCORE_API ITidPart{
	const static int TYPE_NODE	= 0;		// �ڵ�
	const static int TYPE_ANGLE	= 1;		// �Ǹ�
	const static int TYPE_BOLT	= 2;		// ��˨
	const static int TYPE_PLATE	= 3;		// �ְ�
	const static int TYPE_TUBE	= 4;		// �ֹ�
	const static int TYPE_FLAT	= 5;		// ����
	const static int TYPE_SLOT	= 6;		// �۸�

	virtual int GetPartType()=0;
	virtual UINT GetSerialId()=0;
	virtual char GetBriefMark()=0;
	virtual WORD GetLength()=0;		//��λmm
	virtual double GetWidth()=0;	//��λmm,һ�����ڴ洢�Ǹ�֫���۸ּ������Ŀ�ȡ��ֹ�ֱ������ֱ���ȡ�
	virtual double GetThick()=0;	//��λmm,һ�����ڴ洢�Ǹ�֫�񡢲۸ּ������ĺ�ȡ��ֹܱں���ں�ȡ�
	virtual double GetHeight()=0;	//��λmm,һ�����ڴ洢����֫�Ǹֵ���һ֫���۸ֵĸ߶ȵȲ�����������
	virtual double GetWeight()=0;	//��λkg
	virtual WORD StateFlag()=0;
	virtual short GetSegStr(char* segstr)=0;
	virtual short GetSpec(char* sizeSpec)=0;
	virtual short GetLabel(char* label)=0;
	virtual short GetFuncType()=0;
	virtual ITidSolidBody* GetSolidPart()=0;
	virtual UINT GetProcessBuffBytes(char* processbytes,UINT maxBufLength=0)=0;
};
//������
struct TIDCORE_API ITidPartsLib{
	virtual int GetCount()=0;
	virtual ITidPart* GetTidPartBySerialId(int serialid)=0;
};
//����װ��ʵ��
struct TIDCORE_API ITidAssemblePart{
	virtual long GetId() = 0;
	virtual ITidPart *GetPart()=0;
	virtual short GetLayer(char* sizeLayer) = 0;
	virtual bool IsHasBriefRodLine()=0;
	virtual TID_COORD3D BriefLineStart()=0;
	virtual TID_COORD3D BriefLineEnd()=0;
	virtual long GetStartNodeId()=0;
	virtual long GetEndNodeId()=0;
	virtual TID_CS GetAcs()=0;
	virtual ITidSolidBody* GetSolidPart()=0;
	//0��ʾ���������Ϲ�����1~4��ʾ������������
	virtual BYTE GetLegQuad() = 0;
	//�ú����������Ľ��Ⱥ�λ��Ϣ��1~192��
	virtual bool GetConfigBytes(BYTE* cfgword_bytes24) = 0;
};
//��˨װ��ʵ��
struct TIDCORE_API ITidAssembleBolt{
	virtual long GetId() = 0;
	virtual IBoltSizeSpec *GetTidBolt()=0;
	virtual TID_CS GetAcs()=0;
	virtual ITidSolidBody* GetBoltSolid()=0;
	virtual ITidSolidBody* GetNutSolid()=0;
	//0��ʾ���������Ϲ�����1~4��ʾ������������
	virtual BYTE GetLegQuad() = 0;
	//�ú����������Ľ��Ⱥ�λ��Ϣ��1~192��
	virtual bool GetConfigBytes(BYTE* cfgword_bytes24)=0;
};
//�ؽ���˨װ��ʵ��
struct TIDCORE_API ITidAssembleAnchorBolt{
	virtual IAnchorBoltSpec *GetAnchorBolt()=0;
	virtual TID_CS GetAcs()=0;
	virtual ITidSolidBody* GetBoltSolid()=0;
	virtual ITidSolidBody* GetNutSolid()=0;
	virtual long GetId()=0;
	virtual BYTE BelongQuad()=0;
};
//�ڵ���Ϣ
struct TIDCORE_API ITidNode{
	virtual long GetId()=0;
	virtual UINT GetPointI()=0;
	virtual BYTE BelongQuad()=0;
	virtual TID_COORD3D GetPos()=0;
	virtual short GetLayer(char* sizeLayer)=0;
	//0��ʾ���������Ϲ�����1~4��ʾ������������
	virtual BYTE GetLegQuad() = 0;
	//�ú����������Ľ��Ⱥ�λ��Ϣ��1~192��
	virtual bool GetConfigBytes(BYTE* cfgword_bytes24) = 0;
};
//�ҵ���Ϣ
struct TIDCORE_API ITidHangPoint{
	virtual TID_COORD3D GetPos()=0;
	virtual short GetWireDescription(char* sDes)=0;
	virtual bool IsCircuitDC()=0;		//blCircuitDC		��·����: false��������,trueֱ��
	virtual BYTE GetCircuitSerial()=0;	//ciCircuitSerial	��·���: 0�����ߣ�,1,2,3,4
	virtual BYTE GetTensionType()=0;	//ciTensionType		��������: 1.����;2.����;3.�ն�
	virtual BYTE GetWireDirection()=0;	//ciWireDirection	���·���: 'X' or 'Y'
	virtual BYTE GetPhaseSerial()=0;	//ciPhaseSerial;	�����,ȡֵ:0�����ߣ�,1,2,3
	virtual BYTE GetWireType()=0;		//ciWireType; 		�������ͣ�'E'or 1����;'C'or 2����;'J'or 3����;0.�޹���
	virtual BYTE GetHangingStyle()=0;	//ciHangingStyle	�����������ϵĹҽӷ�ʽ'S' or 'V',0��ʾ��ͨ�ҽ�
	virtual BYTE GetPostCode()=0;		//ciPostCode		�ҵ㸽����: ��"��1-V3"��V�����'3'��ʾ1�������ߵ�V�ҵ��еĵ�����
	virtual BYTE GetSerial()=0;			//ciSerial			�ҵ����ţ����ݹҵ����ڻ�·����·�е�����ż���
	virtual BYTE GetPosSymbol()=0;		//λ�ñ�ʶ	0.��|Q.ǰ��|H.���
	//�ҵ�����ҿ���Ϣ
	virtual BYTE GetRelaHoleNum() = 0;
	virtual TID_COORD3D GetRelaHolePos(int index) = 0;
};
//����
struct ITidTowerInstance;
struct TIDCORE_API ITidHeightGroup {
	//���߱��(��1��ʼ)
	virtual int GetSerialId() = 0;
	//��������
	virtual int GetName(char *name, UINT maxBufLength = 0) = 0;
	//�ú��ߵĶ������Ϣ
	virtual int GetLegSerialArr(int* legSerialArr) = 0;
	//������ƽ�Ƽ�ĸ߶Ȳ�,һ��Ϊ��ֵ����ʾ��ƽ�ȸ��̣� wjh-2019.5.10
	virtual int GetLegSerial(double heightDifference) = 0;
	//legSerial��ƽ��Ϊ0������������+1,����ֵΪָ���ȼ�������ƽ�Ƽ�ĸ߶Ȳ�(һ��Ϊ��ֵ,��λm)
	virtual double GetLegHeightDifference(int legSerial) = 0;	//
	//��ȡָ���ȵĸ߶ȣ�ָ������Zֵ-����ߣ�,��λm
	virtual double GetLegHeight(int legSerial) = 0;	//
	//���߸߶�(���Zֵ-���߻���Zֵ)
	virtual UINT GetNamedHeight() = 0;	//��λ,mm(V1.4��������)
	//��ȡ���ߵ����߶�(����ȼ���)
	virtual double GetLowestZ() = 0;	//��λ(mm)
	//��ȡ���ߵ���������ȵĹ���Zֵ
	virtual double GetBody2LegTransitZ() = 0;	//��λ(mm)
	//����߶�(��������ȵĹ���Zֵ-���߻���Zֵ)
	virtual double GetBodyNamedHeight() = 0;	//��λ(mm)
	//�ú����������Ľ��Ⱥ�λ��Ϣ��1~192��
	virtual bool GetConfigBytes(BYTE* cfgword_bytes24) = 0;
	//����A|B|C|D������Ϣ����������
	virtual ITidTowerInstance* GetTowerInstance(int legSerialQuad1, int legSerialQuad2, int legSerialQuad3, int legSerialQuad4) = 0;
};
//��ʵ��
struct TIDCORE_API ITidTowerInstance{
	virtual short GetLegSerialIdByQuad(short siQuad)=0;
	virtual ITidHeightGroup* BelongHeightGroup()=0;
	virtual double GetInstanceHeight()=0;
	//�ڵ�
	virtual int GetNodeNum()=0;
	virtual ITidNode* FindNode(int iKey)=0;
	virtual ITidNode* EnumNodeFirst()=0;
	virtual ITidNode* EnumNodeNext()=0;
	//װ����˨
	virtual int GetAssembleBoltNum()=0;
	virtual ITidAssembleBolt* FindAssembleBolt(int iKey)=0;
	virtual ITidAssembleBolt* EnumAssembleBoltFirst()=0;
	virtual ITidAssembleBolt* EnumAssembleBoltNext()=0;
	//װ�乹��
	virtual int GetAssemblePartNum()=0;
	virtual ITidAssemblePart* FindAssemblePart(int iKey)=0;
	virtual ITidAssemblePart* EnumAssemblePartFirst()=0;
	virtual ITidAssemblePart* EnumAssemblePartNext()=0;
	//�ؽ���˨
	virtual int GetAssembleAnchorBoltNum()=0;
	virtual ITidAssembleAnchorBolt* FindAnchorBolt(int iKey)=0;
	virtual ITidAssembleAnchorBolt* EnumFirstAnchorBolt()=0;
	virtual ITidAssembleAnchorBolt* EnumNextAnchorBolt()=0;
	//
	virtual bool ExportModFile(const char* sFileName)=0;
};
//��ģ��
struct TIDCORE_API ITidModel {
	//��ģ�ͱ��(��1��ʼ)
	virtual long GetSerialId()=0;
	//ģ������ϵ
	virtual TID_CS ModelCoordSystem()=0;
	//���ʿ�
	virtual ISteelMaterialLibrary* GetSteelMatLib()=0;
	//��˨��
	virtual IBoltSeriesLib* GetBoltLib()=0;
	//������
	virtual ITidPartsLib* GetTidPartsLib()=0;
	//��ȡ��������
	virtual int GetTowerTypeName(char* towerTypeName, UINT maxBufLength = 0) = 0;
	//����������Ϣ
	virtual short HeightGroupCount() = 0;
	virtual ITidHeightGroup* GetHeightGroupAt(short i)=0;//0 based index 
	virtual ITidHeightGroup* GetHeightGroup(long hHeightSerial) = 0;
	virtual ITidHeightGroup* EnumHeightGroupFirst() = 0;
	virtual ITidHeightGroup* EnumHeightGroupNext() = 0;
	//�����ҵ���Ϣ
	virtual DWORD HangPointCount()=0;
	virtual ITidHangPoint* GetHangPointAt(DWORD i)=0;
	//�����ڵ�
	virtual int GetTidNodeNum() = 0;
	virtual ITidNode* EnumTidNodeFirst(long hHeightSerial = 0) = 0;
	virtual ITidNode* EnumTidNodeNext(long hHeightSerial = 0) = 0;
	//����װ����˨(��ָ������)
	virtual int GetAssemblePartNum() = 0;
	virtual ITidAssemblePart* EnumAssemblePartFirst(long hHeightSerial = 0) = 0;
	virtual ITidAssemblePart* EnumAssemblePartNext(long hHeightSerial = 0) = 0;
	//����װ�乹��(��ָ������)
	virtual int GetAssembleBoltNum() = 0;
	virtual ITidAssembleBolt* EnumAssembleBoltFirst(long hHeightSerial = 0) = 0;
	virtual ITidAssembleBolt* EnumAssembleBoltNext(long hHeightSerial = 0) = 0;
	//��ȡ����TID�ļ�����
	virtual bool ReadTidFile(const char* file_path) = 0;
	virtual bool InitTidBuffer(const char* src_buf,long buf_len)=0;
public:	//V1.4��������
	//���߻���Zֵ(�û�����ʵ�ʺ��߸߶�)
	virtual double GetNamedHeightZeroZ()=0;	//��λ(mm)
	//�ؽ���˨��Ϣ
	virtual WORD GetBasePlateThick()=0;		//��λ(mm)
	virtual WORD GetAnchorCount()=0;
	virtual bool GetAnchorAt(short index,short* psiPosX,short* psiPosY)=0;
	//������Ϣ
	virtual WORD GetSubLegCount(BYTE ciBodySerial)=0;
	virtual bool GetSubLegBaseLocation(BYTE ciBodySerial,BYTE ciLegSerial,double* pos3d)=0;
	virtual double GetSubLegBaseWidth(BYTE ciBodySerial, BYTE ciLegSerial) = 0;
	//����MOD�ļ�
	virtual bool ExportModFile(const char* sFileName)=0;
};
class TIDCORE_API CTidModelFactory{
public:
	static ITidModel* CreateTidModel();
	static ITidModel* TidModelFromSerial(long serial);
	static bool Destroy(long serial);
};