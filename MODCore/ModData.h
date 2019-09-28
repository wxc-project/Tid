#pragma once
#include "ModCore.h"
#include "f_ent.h"
#include "ldsptr_list.h"
#include "ArrayList.h"
#include "HashTable.h"
#include "XhCharString.h"
#include "SolidBody.h"
#include "LogFile.h"

class CModNode;
class CModRod;
typedef CTmaPtrList<CModNode,CModNode*> NODESET;
typedef CTmaPtrList<CModRod,CModRod*> RODSET;
//
class CModModel;
class CModHeightGroup;
class MOD_CFGWORD{
public:
	union{
		DWORD word[6];
		BYTE bytes[24];
	}flag;
	MOD_CFGWORD(){memset(&flag,0,24);}
	MOD_CFGWORD(int iNo){SetWordByNo(iNo);}
	MOD_CFGWORD(int iBodyNo,int iLegNo,BYTE schema=0);
	void Clear(){memset(&flag,0,24);}
	BOOL And(MOD_CFGWORD wcfg) const;	//相当于用‘&’号判断是否有交集
	BOOL IsHasNo(int iNo);			//配材字中是否含有指定的iNo号位
	BOOL IsHasBodyNoOnly(int iBodyNo,BYTE schema=0);	//配材字中是否仅指定字节是有值，即不为0，iByte以1为基数(即索引值+1)
	BOOL IsHasBodyNo(int iBodyNo,BYTE schema=0);		//配材字中指定字节是否有值，即不为0，iByte以1为基数(即索引值+1)
	BOOL AddBodyLegs(int iBodyNo,DWORD legword=0xffffff,BYTE schema=0);
	BOOL SetBodyLegs(int iBodyNo,DWORD legword=0xffffff,BYTE schema=0);
	MOD_CFGWORD SetWordByNo(int iNo);			//根据指定的iNo号位指定配材字
	MOD_CFGWORD AddSpecWord(MOD_CFGWORD cfgword);	//相当于两个配材字进行或操作,并将结果赋值给当前配材字
public:
	static BYTE MULTILEG_SCHEMA;	//当前默认的呼高接腿占位分配模式
	static const BYTE MULTILEG_DEFAULT	= 0;
	static const BYTE MULTILEG_MAX08	= 1;
	static const BYTE MULTILEG_MAX16	= 2;
	static const BYTE MULTILEG_MAX24	= 3;
	static BYTE SetSchema(BYTE cbMultiLegSchema);
	static BYTE MaxLegs(BYTE schema=0);	//指定模式支持最多呼高接腿数MULTILEG_DEFAULT=0
	static BYTE MaxBodys(BYTE schema=0);//指定模式支持最多呼高本体数MULTILEG_DEFAULT=0
};
class CModNode : public IModNode
{
	IModModel* m_pBelongModel;
public:
	char m_sLayer[4];
	MOD_CFGWORD m_xCfgword;
	long handle;
	f3dPoint xOrg;		//GIM坐标系下的节点坐标
	f3dPoint xLdsOrg;	//建模坐标系下的节点坐标
public:	
	CModNode();
	void SetKey(DWORD key){handle=key;}
	BYTE GetLegQuad();
	//
	virtual long GetId(){return handle;}
	virtual IModModel* BelongModel(){return m_pBelongModel;}
	virtual void SetBelongModel(IModModel* pModel){m_pBelongModel=pModel;}
	virtual void SetCfgword(BYTE* cfgword_bytes24){
		memcpy(m_xCfgword.flag.bytes,cfgword_bytes24,24);
	}
	virtual void SetOrg(MOD_POINT pt){xOrg.Set(pt.x,pt.y,pt.z);}
	virtual void SetLdsOrg(MOD_POINT pt){xLdsOrg.Set(pt.x,pt.y,pt.z);}
	virtual void SetLayer(char cLayer){m_sLayer[0]=cLayer;}
	virtual BOOL IsLegNode(){return m_sLayer[0]=='L';}
	virtual MOD_POINT NodePos(){return MOD_POINT(xOrg);}
	virtual void Clone(IModNode* pSrcNode);
};
class CModRod : public IModRod
{
	BOOL Create3dJgSolidModel(CSolidBody *pSolidBody,BOOL bToLds=TRUE);
	BOOL Create3dTubeSolidModel(CSolidBody *pSolidBody,BOOL bToLds=TRUE);
	BOOL Create3dConeSolidModel(CSolidBody *pSolidBody,BOOL bToLds=TRUE);
	IModModel* m_pBelongModel;
public:
	long handle;
	BYTE m_ciRodType;	//1.角钢|2.钢管|3.锥管
	char m_sLayer[4];
	MOD_CFGWORD m_xCfgword;
	char m_cMaterial;
	double m_fWidth;
	double m_fWidth2;	//椎管的终端直径
	double m_fThick;
	int m_nFace;		//椎管边数
	UINT m_uiNodeS,m_uiNodeE;
	f3dPoint m_vNormX,m_vNormY;
	f3dPoint m_vWingX,m_vWingY;
	f3dLine base_line;
public:
	CModRod();
	~CModRod();
	//
	void SetKey(DWORD key){handle=key;}
	int GetLength(){return (int)(0.5+DISTANCE(base_line.startPt,base_line.endPt));}
	UCS_STRU BuildUcs(BOOL bToLds=TRUE);
	BYTE GetLegQuad();
	//
	virtual void SetLayer(char cLayer){m_sLayer[0]=cLayer;}
	virtual void SetRodType(BYTE type){m_ciRodType=type;}
	virtual void SetMaterial(char cMat){m_cMaterial=cMat;}
	virtual void SetCfgword(BYTE* cfgword_bytes24){
		memcpy(m_xCfgword.flag.bytes,cfgword_bytes24,24);
	}
	virtual void SetWidth(double fWidth){m_fWidth=fWidth;}
	virtual void SetThick(double fThick){m_fThick=fThick;}
	virtual void SetWingXVec(MOD_POINT vWingX){m_vWingX.Set(vWingX.x,vWingX.y,vWingX.z);}
	virtual void SetWingYVec(MOD_POINT vWingY){m_vWingY.Set(vWingY.x,vWingY.y,vWingY.z);}
	virtual void SetNodeS(IModNode* pNodeS){m_uiNodeS=((CModNode*)pNodeS)->handle;}
	virtual void SetNodeE(IModNode* pNodeE){m_uiNodeE=((CModNode*)pNodeE)->handle;}
	virtual void Clone(IModRod* pSrcRod);
	//
	virtual BYTE GetRodType(){return m_ciRodType;}
	virtual BOOL IsAngle(){return m_ciRodType==1;}
	virtual BOOL IsLegRod(){return m_sLayer[0]=='L';}
	virtual IModModel* BelongModel(){return m_pBelongModel;}
	virtual void SetBelongModel(IModModel* pModel){m_pBelongModel=pModel;}
	virtual MOD_LINE GetBaseLineToLdsModel();
	virtual long GetId(){return handle;}
	virtual BOOL Create3dSolidModel(CSolidBody *pSolidBody,BOOL bToLds=TRUE);
	virtual MOD_POINT LinePtS(){return MOD_POINT(base_line.startPt);}
	virtual MOD_POINT LinePtE(){return MOD_POINT(base_line.endPt);}
	virtual double GetWidth(){return m_fWidth;}
	virtual double GetThick(){return m_fThick;}
};
class CModTowerInstance : public IModTowerInstance
{
	long m_id;
	RODSET RodSet;
	NODESET NodeSet;
	CModHeightGroup* m_pBelongHG;
public:
	GEPOINT m_xBaseLocation[4];
public:
	CModTowerInstance(){m_pBelongHG=NULL;m_id=0;}
	//
	void SetKey(DWORD key){m_id=(long)key;}
	void SetBelongHuGao(CModHeightGroup* pHeightGroup){m_pBelongHG=pHeightGroup;}
	IModHeightGroup* BelongHuGao(){return (IModHeightGroup*)m_pBelongHG;}
	void SetBaseLocation(int iLegQuad,GEPOINT pos){
		if(m_xBaseLocation[iLegQuad-1].z<pos.z)
			m_xBaseLocation[iLegQuad-1]=pos;
	}
	virtual MOD_POINT GetBaseLocation(int iLegQuad){return MOD_POINT(m_xBaseLocation[iLegQuad-1]);}
	//节点
	void AppendNode(CModNode* pNode){NodeSet.append(pNode);}
	virtual int GetModNodeNum(){return NodeSet.GetNodeNum();}
	virtual IModNode* EnumModNodeFir(){return NodeSet.GetFirst();}
	virtual IModNode* EnumModNodeNext(){return NodeSet.GetNext();}
	//杆件
	void AppendRod(CModRod* pRod){RodSet.append(pRod);}
	virtual int GetModRodNum(){return RodSet.GetNodeNum();}
	virtual IModRod* EnumModRodFir(){return RodSet.GetFirst();}
	virtual IModRod* EnumModRodNext(){return RodSet.GetNext();}
	//
	virtual bool ExportModFile(const char* sFileName);
};
class CModHeightGroup : public IModHeightGroup
{
	IModModel* m_pBelongModel;
public:
	int m_iNo;					//呼高号
	int m_iBody;				//上接本体
	double lowest_z;			//
	double m_fNamedHeight;		//呼称高度
	CXhChar50 m_sHeightName;	//呼高名称
	MOD_CFGWORD m_dwLegCfgWord;		//接腿配材号
	int m_arrActiveQuadLegNo[4];
	CHashListEx<CModTowerInstance> hashTowerInstance;
	CHashStrList<int> hashSubLegSerial;
public:
	CModHeightGroup();
	//
	virtual void SetBelongModel(IModModel* pModel){m_pBelongModel=pModel;}
	virtual IModModel* BelongModel(){return m_pBelongModel;}
	virtual int GetNo(){return m_iNo;}
	virtual void SetLowestZ(double fLowestZ){lowest_z=fLowestZ;}
	virtual void SetLegCfg(BYTE* cfgword_bytes24){
		memcpy(m_dwLegCfgWord.flag.bytes,cfgword_bytes24,24);
	}
	virtual void GetName(char* sName){strcpy(sName,m_sHeightName);}
	virtual double GetNamedHeight(){return m_fNamedHeight;}
	virtual void SetNameHeight(double fNameHeight){m_fNamedHeight=fNameHeight;}
	virtual double GetLegDiffDist(int iLegSerial);
	virtual int* GetLegSerialId(const char* sLegDiffDist){return hashSubLegSerial.GetValue(sLegDiffDist);}
	virtual int* EnumFirstLegSerial(){return hashSubLegSerial.GetFirst();}
	virtual int* EnumNextLegSerial(){return hashSubLegSerial.GetNext();}
	virtual IModTowerInstance* GetTowerInstance(int legSerialQuad1, int legSerialQuad2, int legSerialQuad3, int legSerialQuad4);
	//
	void SetKey(DWORD key){m_iNo=key;}
	BYTE GetLegBitSerialFromSerialId(int serial);
	BOOL IsSonNode(CModNode *pNode);
	BOOL IsSonRod(CModRod *pRod);
};

struct SUB_LEG_INFO{
	double m_fLegH;		//减腿高度
	NODESET legNodeSet;	//腿部节点集合
	RODSET legRodSet;
	//
	SUB_LEG_INFO(){m_fLegH=0;}
};
class CLegItem
{
public:
	long m_hTagNode;
	int m_iLegNo;		//
	double m_fSegmentH;	//共用部分段高
	double m_fMaxLegH;	//最长腿高
	GEPOINT m_maxLegPt;	//最长腿坐标
	NODESET segmentNodeSet;	//共有分段节点
	RODSET segmentRodSet;	//
	ATOM_LIST<SUB_LEG_INFO> subLegInfoList;
public:
	CLegItem();
	~CLegItem(){;}
	//
	void InitLegItemByModule(CModHeightGroup* pModule);
};
class CBodyItem
{
public:
	long m_hTagNode;		//本体终止特征节点(第二象限Z值最大节点)
	double m_fBodyH;		//本体高度
	int m_iLegS;			//本体下开始接腿序号
	NODESET bodyNodeSet;	//
	RODSET bodyRodSet;
public:
	CBodyItem(){m_hTagNode=0;m_fBodyH=0;m_iLegS=0;}
	~CBodyItem(){;}
	//
	void InitBodyItem(CModNode* &pTagNode,NODESET& selNodeSet,RODSET& selRodSet);
};

class CModModel : public IModModel
{
public:
	long m_iSerial;
	double m_fTowerHeight;
	ATOM_LIST<CLegItem> m_listLegItem;
	ATOM_LIST<CBodyItem> m_listBodyItem;
	ARRAY_LIST<MOD_HANG_NODE> m_listGuaNode;
private:
	CHashListEx<CModHeightGroup> ModHeightGroup;
	CHashListEx<CModRod> ModRods;
	CHashListEx<CModNode> ModNodes;
protected:
	void AmendModData();
public:
	CModModel(long serial=0);
	~CModModel();
	void Empty();
	CLegItem* FindLegItem(int module_no);
	//
	static char QueryBriefMatMark(const char* sMatMark);
	static void QuerySteelMatMark(char cMat,char* sMatMark);
	//
	virtual CModNode* AppendNode(DWORD key){return ModNodes.Add(key);}
	virtual CModNode* FindNode(DWORD key){return ModNodes.GetValue(key);}
	virtual CModNode* EnumFirstNode(){return ModNodes.GetFirst();}
	virtual CModNode* EnumNextNode(){return ModNodes.GetNext();}
	//
	virtual CModRod* AppendRod(DWORD key){return ModRods.Add(key);}
	virtual CModRod* FindRod(DWORD key){return ModRods.GetValue(key);}
	virtual CModRod* EnumFirstRod(){return ModRods.GetFirst();}
	virtual CModRod* EnumNextRod(){return ModRods.GetNext();}
	//
	int GetHeightGroupNum(){return ModHeightGroup.GetNodeNum();}
	virtual IModHeightGroup* AppendHeightGroup(DWORD key){return ModHeightGroup.Add(key);}
	virtual IModHeightGroup* EnumFirstHGroup(){return ModHeightGroup.GetFirst();}
	virtual IModHeightGroup* EnumNextHGroup(){return ModHeightGroup.GetNext();}
	virtual IModHeightGroup* GetHeightGroup(int iNo);
	virtual IModHeightGroup* GetHeightGroup(const char* sName);
	//
	virtual MOD_CS BuildUcsByModCS();	//建立LDS模型坐标系（以MOD模型坐标系为基底参照）
	virtual void SetTowerHeight(double fTowerHeight){m_fTowerHeight=fTowerHeight;}
	virtual long GetSerialId(){return m_iSerial;}
	virtual BOOL ImportModFile(const char* sModFile);
	virtual void ReadModFile(FILE* fp,BOOL bUtf8=FALSE);
	virtual void WriteModFile(FILE* fp,BOOL bUtf8=FALSE);
	virtual void WriteModFileByUtf8(FILE* fp);
	virtual void WriteModFileByAnsi(FILE* fp);
	virtual void InitSingleModData(double fNameH);
	virtual void InitMultiModData();
	//
	virtual MOD_HANG_NODE* AppendHangNode(){return m_listGuaNode.append();}
	virtual int GetHangNodeNum(){return m_listGuaNode.GetSize();}
	virtual MOD_HANG_NODE* GetHangNodeById(int index){return m_listGuaNode.GetByIndex(index);}
};