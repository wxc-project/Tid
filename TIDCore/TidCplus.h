// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 TIDCORE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// TIDCORE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
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

//三维坐标-单位mm
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
//三维坐标系-单位mm
struct TIDCORE_API TID_CS{
	TID_COORD3D origin;
	//坐标系的X/Y/Z坐标轴, 三坐标轴遵循右手螺栓正交，且为单位化矢量
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
//材质
struct TIDCORE_API TID_STEELMAT{
	char cBriefSymbol;
	char nameCodeStr[8];
};
//材质库
struct TIDCORE_API ISteelMaterialLibrary{
	virtual int GetCount()=0;
	virtual TID_STEELMAT GetSteelMatAt(int i)=0;
	virtual char QuerySteelBriefSymbol(const char *steelmark)=0;
	virtual bool QuerySteelNameCode(char cBriefSymbol,char* nameCode)=0;
};
//实体
struct ITidSolidBody;
struct TIDCORE_API TidArcline{
protected:
	TID_COORD3D posStart,posEnd;
	TID_COORD3D center;
	TID_COORD3D work_norm;
	TID_COORD3D column_norm;
	double radius;
	double sector_angle;//弧度制单位
public:
	TidArcline();
	int ToByteArr(char* buf);	//直线48Bytes,圆弧112Bytes,椭圆弧136Bytes;注意只输出几何信息部分
	void FromByteArr(char* buf,DWORD buf_size);//从缓存中还原弧或直线
	virtual TID_COORD3D StartPosition() const;
	virtual TID_COORD3D EndPosition() const;
	virtual TID_COORD3D ColumnNorm() const;
	virtual TID_COORD3D WorkNorm() const;
	virtual TID_COORD3D Center() const;
	virtual double Radius() const;		//单位mm
	virtual double SectorAngle() const;	//弧度制单位
	virtual double Length();			//圆弧长度mm
	virtual TID_COORD3D PositionInAngle(double posAngle);		//弧度制单位
	virtual TID_COORD3D TangentVecInAngle(double posAngle);	//逆时针旋转为正切向方向,弧度制单位
};
struct TIDCORE_API ITidRawSolidEdge{
public:
	ITidRawSolidEdge(){;}
public:
	static const BYTE NURBS		=0x00;
	static const BYTE STRAIGHT	=0x01;
	static const BYTE ARCLINE	=0x02;
	static const BYTE ELLIPSE	=0x03;
	//边类型,NURBS;STRAIGHT;ARCLINE;ELLIPSE
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
	virtual COLORREF MatColor()=0;		// 可用于记录此面的特征信息(如材质等)
	virtual DWORD FaceId()=0;	//用于标识多边形面链中的某一特定面
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
	virtual BYTE Mode()=0;		//绘制启动函数glBegin()中需要的绘制模式
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
	virtual void ReleaseTemporaryBuffer()=0;	//释放临时缓存后，由该实体接口获取的所有相关接口类均失效　wjh-2016.1.18
	virtual TID_COORD3D GetKeyPointAt(int i)=0;
	virtual int KeyEdgeLineNum()=0;
	virtual bool GetKeyEdgeLineAt(int i,TidArcline& line)=0;
	virtual ITidRawSolidEdge* GetKeyEdgeLineAt(int i)=0;
	virtual int PolyFaceNum()=0;
	virtual IRawSolidFace* GetPolyFaceAt(int i)=0;
	virtual int BasicFaceNum()=0;
	virtual ITidBasicFace* GetBasicFaceAt(int i)=0;
	virtual bool SplitToBasicFacets()=0;
	//将实体从装配坐标系fromACS移位到装配坐标系toACS
	virtual void TransACS(const TID_CS& fromACS,const TID_CS& toACS)=0;
	virtual void TransFromACS(const TID_CS& fromACS)=0;
	virtual void TransToACS(const TID_CS& toACS)=0;
};
//螺栓规格
struct TIDCORE_API ITidBoltNut{
	virtual short GetDiameter()=0;			//螺栓直径
	virtual ITidSolidBody* GetSolidBody()=0;//螺栓实体	
};
struct TIDCORE_API IBoltSizeSpec{
	virtual UINT GetId()=0;
	virtual UINT GetSeriesId()=0;			//螺栓归属规格系列的标识
	virtual short GetDiameter()=0;			//螺栓直径(mm)
	virtual short GetLenValid()=0;			//螺栓有效长(mm)
	virtual short GetLenNoneThread()=0;		//螺栓无扣长(mm)
	virtual double GetTheoryWeight()=0;		//理论重量(kg)
	virtual short GetSpec(char* spec)=0;	//螺栓规格描述字符（含末尾0截止字符）
	virtual ITidSolidBody* GetBoltSolid()=0;//螺栓实体	
	virtual ITidSolidBody* GetNutSolid()=0;//螺栓实体	
};
struct TIDCORE_API IAnchorBoltSpec{
	virtual short GetDiameter()=0;		//地脚螺栓名义直径(mm)
	virtual short GetLenValid()=0;		//地脚螺栓出露长(mm)
	virtual short GetLenNoneThread()=0;	//地脚螺栓无扣长(mm) <底座板厚度
	virtual short GetPadWidth()=0;		//地脚螺栓垫板宽度(mm)
	virtual short GetPadThick()=0;		//地脚螺栓垫板厚度(mm)
	virtual short GetHoleD()=0;			//地脚螺栓板孔径(mm)
	virtual short GetBasePlateHoleD()=0;//地脚螺栓连接底座板建议孔径值(mm)
	virtual short GetBasePlateThick()=0;		//基础底板厚度(mm)
	virtual char* GetSizeSymbol()=0;	//地脚螺栓规格标记字符串
};
//螺栓系列
struct TIDCORE_API IBoltSeries{
	virtual UINT GetSeriesId()=0;
	virtual short GetName(char* name)=0;
	virtual int GetCount()=0;
	virtual short GetBoltNutCount()=0;		//单帽/双帽
	virtual short GetWasherCount()=0;		//附带垫圈数量
	//virtual bool IsFootNail()=0;			//脚钉
	//virtual bool IsBurglarproof()=0;		//是否为防盗螺栓
	virtual IBoltSizeSpec* EnumFirst()=0;
	virtual IBoltSizeSpec* EnumNext()=0;
	virtual IBoltSizeSpec* GetBoltSizeSpecById(int id)=0;
};
//螺栓库
struct TIDCORE_API IBoltSeriesLib{
	virtual int GetCount()=0;
	virtual IBoltSeries* GetBoltSeriesAt(int i)=0;
	virtual IBoltSizeSpec* GetBoltSizeSpec(int seriesId,int sizeSpecId)=0;
};
//构件(角钢、钢板、螺栓、钢管、槽钢等)
struct TIDCORE_API ITidPart{
	const static int TYPE_NODE	= 0;		// 节点
	const static int TYPE_ANGLE	= 1;		// 角钢
	const static int TYPE_BOLT	= 2;		// 螺栓
	const static int TYPE_PLATE	= 3;		// 钢板
	const static int TYPE_TUBE	= 4;		// 钢管
	const static int TYPE_FLAT	= 5;		// 扁铁
	const static int TYPE_SLOT	= 6;		// 槽钢

	virtual int GetPartType()=0;
	virtual UINT GetSerialId()=0;
	virtual char GetBriefMark()=0;
	virtual WORD GetLength()=0;		//单位mm
	virtual double GetWidth()=0;	//单位mm,一般用于存储角钢肢宽、槽钢及扁铁的宽度、钢管直径、球直径等。
	virtual double GetThick()=0;	//单位mm,一般用于存储角钢肢厚、槽钢及扁铁的厚度、钢管壁厚、球壁厚等。
	virtual double GetHeight()=0;	//单位mm,一般用于存储不等肢角钢的另一肢厚或槽钢的高度等不常见参数。
	virtual double GetWeight()=0;	//单位kg
	virtual WORD StateFlag()=0;
	virtual short GetSegStr(char* segstr)=0;
	virtual short GetSpec(char* sizeSpec)=0;
	virtual short GetLabel(char* label)=0;
	virtual short GetFuncType()=0;
	virtual ITidSolidBody* GetSolidPart()=0;
	virtual UINT GetProcessBuffBytes(char* processbytes,UINT maxBufLength=0)=0;
};
//构件库
struct TIDCORE_API ITidPartsLib{
	virtual int GetCount()=0;
	virtual ITidPart* GetTidPartBySerialId(int serialid)=0;
};
//构件装配实例
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
	//0表示杆塔本体上构件，1~4表示归属接腿象限
	virtual BYTE GetLegQuad() = 0;
	//该呼高所包括的接腿号位信息（1~192）
	virtual bool GetConfigBytes(BYTE* cfgword_bytes24) = 0;
};
//螺栓装配实例
struct TIDCORE_API ITidAssembleBolt{
	virtual long GetId() = 0;
	virtual IBoltSizeSpec *GetTidBolt()=0;
	virtual TID_CS GetAcs()=0;
	virtual ITidSolidBody* GetBoltSolid()=0;
	virtual ITidSolidBody* GetNutSolid()=0;
	//0表示杆塔本体上构件，1~4表示归属接腿象限
	virtual BYTE GetLegQuad() = 0;
	//该呼高所包括的接腿号位信息（1~192）
	virtual bool GetConfigBytes(BYTE* cfgword_bytes24)=0;
};
//地脚螺栓装配实例
struct TIDCORE_API ITidAssembleAnchorBolt{
	virtual IAnchorBoltSpec *GetAnchorBolt()=0;
	virtual TID_CS GetAcs()=0;
	virtual ITidSolidBody* GetBoltSolid()=0;
	virtual ITidSolidBody* GetNutSolid()=0;
	virtual long GetId()=0;
	virtual BYTE BelongQuad()=0;
};
//节点信息
struct TIDCORE_API ITidNode{
	virtual long GetId()=0;
	virtual UINT GetPointI()=0;
	virtual BYTE BelongQuad()=0;
	virtual TID_COORD3D GetPos()=0;
	virtual short GetLayer(char* sizeLayer)=0;
	//0表示杆塔本体上构件，1~4表示归属接腿象限
	virtual BYTE GetLegQuad() = 0;
	//该呼高所包括的接腿号位信息（1~192）
	virtual bool GetConfigBytes(BYTE* cfgword_bytes24) = 0;
};
//挂点信息
struct TIDCORE_API ITidHangPoint{
	virtual TID_COORD3D GetPos()=0;
	virtual short GetWireDescription(char* sDes)=0;
	virtual bool IsCircuitDC()=0;		//blCircuitDC		回路类型: false（交流）,true直流
	virtual BYTE GetCircuitSerial()=0;	//ciCircuitSerial	回路序号: 0（地线）,1,2,3,4
	virtual BYTE GetTensionType()=0;	//ciTensionType		张力类型: 1.悬垂;2.耐张;3.终端
	virtual BYTE GetWireDirection()=0;	//ciWireDirection	线缆方向: 'X' or 'Y'
	virtual BYTE GetPhaseSerial()=0;	//ciPhaseSerial;	相序号,取值:0（地线）,1,2,3
	virtual BYTE GetWireType()=0;		//ciWireType; 		挂线类型：'E'or 1地线;'C'or 2导线;'J'or 3跳线;0.无挂线
	virtual BYTE GetHangingStyle()=0;	//ciHangingStyle	线缆在塔身上的挂接方式'S' or 'V',0表示普通挂接
	virtual BYTE GetPostCode()=0;		//ciPostCode		挂点附加码: 如"导1-V3"中V后面的'3'表示1号相序导线的V挂点中的第三个
	virtual BYTE GetSerial()=0;			//ciSerial			挂点的序号，根据挂点所在回路及回路中的相序号计算
	virtual BYTE GetPosSymbol()=0;		//位置标识	0.无|Q.前侧|H.后侧
	//挂点关联挂孔信息
	virtual BYTE GetRelaHoleNum() = 0;
	virtual TID_COORD3D GetRelaHolePos(int index) = 0;
};
//呼高
struct ITidTowerInstance;
struct TIDCORE_API ITidHeightGroup {
	//呼高编号(从1开始)
	virtual int GetSerialId() = 0;
	//呼高名称
	virtual int GetName(char *name, UINT maxBufLength = 0) = 0;
	//该呼高的多接腿信息
	virtual int GetLegSerialArr(int* legSerialArr) = 0;
	//减腿与平推间的高度差,一般为负值（表示比平腿更短） wjh-2019.5.10
	virtual int GetLegSerial(double heightDifference) = 0;
	//legSerial，平腿为0，各减腿依次+1,返回值为指定等级减腿与平推间的高度差(一般为负值,单位m)
	virtual double GetLegHeightDifference(int legSerial) = 0;	//
	//获取指定腿的高度（指定接腿Z值-接身高）,单位m
	virtual double GetLegHeight(int legSerial) = 0;	//
	//呼高高度(最长腿Z值-呼高基点Z值)
	virtual UINT GetNamedHeight() = 0;	//单位,mm(V1.4新增特性)
	//获取呼高的最大高度(按最长腿计算)
	virtual double GetLowestZ() = 0;	//单位(mm)
	//获取呼高的塔身与接腿的过渡Z值
	virtual double GetBody2LegTransitZ() = 0;	//单位(mm)
	//接身高度(塔身与接腿的过渡Z值-呼高基点Z值)
	virtual double GetBodyNamedHeight() = 0;	//单位(mm)
	//该呼高所包括的接腿号位信息（1~192）
	virtual bool GetConfigBytes(BYTE* cfgword_bytes24) = 0;
	//根据A|B|C|D配腿信息，激活塔例
	virtual ITidTowerInstance* GetTowerInstance(int legSerialQuad1, int legSerialQuad2, int legSerialQuad3, int legSerialQuad4) = 0;
};
//塔实例
struct TIDCORE_API ITidTowerInstance{
	virtual short GetLegSerialIdByQuad(short siQuad)=0;
	virtual ITidHeightGroup* BelongHeightGroup()=0;
	virtual double GetInstanceHeight()=0;
	//节点
	virtual int GetNodeNum()=0;
	virtual ITidNode* FindNode(int iKey)=0;
	virtual ITidNode* EnumNodeFirst()=0;
	virtual ITidNode* EnumNodeNext()=0;
	//装配螺栓
	virtual int GetAssembleBoltNum()=0;
	virtual ITidAssembleBolt* FindAssembleBolt(int iKey)=0;
	virtual ITidAssembleBolt* EnumAssembleBoltFirst()=0;
	virtual ITidAssembleBolt* EnumAssembleBoltNext()=0;
	//装配构件
	virtual int GetAssemblePartNum()=0;
	virtual ITidAssemblePart* FindAssemblePart(int iKey)=0;
	virtual ITidAssemblePart* EnumAssemblePartFirst()=0;
	virtual ITidAssemblePart* EnumAssemblePartNext()=0;
	//地脚螺栓
	virtual int GetAssembleAnchorBoltNum()=0;
	virtual ITidAssembleAnchorBolt* FindAnchorBolt(int iKey)=0;
	virtual ITidAssembleAnchorBolt* EnumFirstAnchorBolt()=0;
	virtual ITidAssembleAnchorBolt* EnumNextAnchorBolt()=0;
	//
	virtual bool ExportModFile(const char* sFileName)=0;
};
//塔模型
struct TIDCORE_API ITidModel {
	//塔模型编号(从1开始)
	virtual long GetSerialId()=0;
	//模型坐标系
	virtual TID_CS ModelCoordSystem()=0;
	//材质库
	virtual ISteelMaterialLibrary* GetSteelMatLib()=0;
	//螺栓库
	virtual IBoltSeriesLib* GetBoltLib()=0;
	//构件库
	virtual ITidPartsLib* GetTidPartsLib()=0;
	//获取呼高名称
	virtual int GetTowerTypeName(char* towerTypeName, UINT maxBufLength = 0) = 0;
	//遍历呼高信息
	virtual short HeightGroupCount() = 0;
	virtual ITidHeightGroup* GetHeightGroupAt(short i)=0;//0 based index 
	virtual ITidHeightGroup* GetHeightGroup(long hHeightSerial) = 0;
	virtual ITidHeightGroup* EnumHeightGroupFirst() = 0;
	virtual ITidHeightGroup* EnumHeightGroupNext() = 0;
	//遍历挂点信息
	virtual DWORD HangPointCount()=0;
	virtual ITidHangPoint* GetHangPointAt(DWORD i)=0;
	//遍历节点
	virtual int GetTidNodeNum() = 0;
	virtual ITidNode* EnumTidNodeFirst(long hHeightSerial = 0) = 0;
	virtual ITidNode* EnumTidNodeNext(long hHeightSerial = 0) = 0;
	//遍历装配螺栓(可指定呼高)
	virtual int GetAssemblePartNum() = 0;
	virtual ITidAssemblePart* EnumAssemblePartFirst(long hHeightSerial = 0) = 0;
	virtual ITidAssemblePart* EnumAssemblePartNext(long hHeightSerial = 0) = 0;
	//遍历装配构件(可指定呼高)
	virtual int GetAssembleBoltNum() = 0;
	virtual ITidAssembleBolt* EnumAssembleBoltFirst(long hHeightSerial = 0) = 0;
	virtual ITidAssembleBolt* EnumAssembleBoltNext(long hHeightSerial = 0) = 0;
	//读取保存TID文件数据
	virtual bool ReadTidFile(const char* file_path) = 0;
	virtual bool InitTidBuffer(const char* src_buf,long buf_len)=0;
public:	//V1.4新增特性
	//呼高基点Z值(用户计算实际呼高高度)
	virtual double GetNamedHeightZeroZ()=0;	//单位(mm)
	//地脚螺栓信息
	virtual WORD GetBasePlateThick()=0;		//单位(mm)
	virtual WORD GetAnchorCount()=0;
	virtual bool GetAnchorAt(short index,short* psiPosX,short* psiPosY)=0;
	//基础信息
	virtual WORD GetSubLegCount(BYTE ciBodySerial)=0;
	virtual bool GetSubLegBaseLocation(BYTE ciBodySerial,BYTE ciLegSerial,double* pos3d)=0;
	virtual double GetSubLegBaseWidth(BYTE ciBodySerial, BYTE ciLegSerial) = 0;
	//导出MOD文件
	virtual bool ExportModFile(const char* sFileName)=0;
};
class TIDCORE_API CTidModelFactory{
public:
	static ITidModel* CreateTidModel();
	static ITidModel* TidModelFromSerial(long serial);
	static bool Destroy(long serial);
};