#pragma once
#include "XhLdsLm.h"
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
class CModNode
{
	CModModel* m_pBelongModel;
public:
	char m_sLayer[4];
	CFGWORD cfgword;
	long point_i;
	f3dPoint xOrg;
public:	
	CModNode();
	//
	CModModel* BelongModel(){return m_pBelongModel;}
	void SetBelongModel(CModModel* pModel){m_pBelongModel=pModel;}
	void SetKey(DWORD key){point_i=key;}
	BOOL IsLegNode(){return m_sLayer[0]=='L';}
	BYTE GetLegQuad();
};
class CModRod
{
	BOOL Create3dJgSolidModel(CSolidBody *pSolidBody,BOOL bToLds=TRUE);
	BOOL Create3dTubeSolidModel(CSolidBody *pSolidBody,BOOL bToLds=TRUE);
	CModModel* m_pBelongModel;
public:
	long handle;
	BYTE m_ciRodType;	//1.角钢|2.钢管
	char m_sLayer[4];
	CFGWORD cfgword;
	char m_cMaterial;
	double m_fWidth;
	double m_fThick;
	UINT m_uiNodeS,m_uiNodeE;
	f3dPoint m_vNormX,m_vNormY;
	f3dPoint m_vWingX,m_vWingY;
	f3dLine base_line;
public:
	CModRod();
	~CModRod();
	//
	CModModel* BelongModel(){return m_pBelongModel;}
	void SetBelongModel(CModModel* pModel){m_pBelongModel=pModel;}
	void SetKey(DWORD key){handle=key;}
	BOOL IsAngle(){return m_ciRodType==1;}
	BOOL IsLegRod(){return m_sLayer[0]=='L';}
	int GetLength(){return ftoi(DISTANCE(base_line.startPt,base_line.endPt));}
	UCS_STRU BuildUcs(BOOL bToLds=TRUE);
	BOOL Create3dSolidModel(CSolidBody *pSolidBody,BOOL bToLds=TRUE);
	f3dLine GetBaseLineToLdsModel();
	BYTE GetLegQuad();
};
class CModTowerInstance
{
	long m_id;
	RODSET RodSet;
	NODESET NodeSet;
	CModHeightGroup* m_pBelongHG;
public:
	GEPOINT m_xBaseLocation[4];
public:
	CModTowerInstance(){m_pBelongHG=NULL;m_id=0;}
	void SetKey(DWORD key){m_id=(long)key;}
	void SetBelongHuGao(CModHeightGroup* pHeightGroup){m_pBelongHG=pHeightGroup;}
	CModHeightGroup* BelongHuGao(){return m_pBelongHG;}
	void SetBaseLocation(int iLegQuad,GEPOINT pos){
		if(m_xBaseLocation[iLegQuad-1].z<pos.z)
			m_xBaseLocation[iLegQuad-1]=pos;
	}
	GEPOINT GetBaseLocation(int iLegQuad){return m_xBaseLocation[iLegQuad-1];}
	//节点
	void AppendNode(CModNode* pNode){NodeSet.append(pNode);}
	int GetModNodeNum(){return NodeSet.GetNodeNum();}
	CModNode* EnumModNodeFir(){return NodeSet.GetFirst();}
	CModNode* EnumModNodeNext(){return NodeSet.GetNext();}
	//杆件
	void AppendRod(CModRod* pRod){RodSet.append(pRod);}
	int GetModRodNum(){return RodSet.GetNodeNum();}
	CModRod* EnumModRodFir(){return RodSet.GetFirst();}
	CModRod* EnumModRodNext(){return RodSet.GetNext();}
};
class CModHeightGroup
{
	CModModel* m_pModel;
public:
	int m_iNo;					//呼高号
	int m_iBody;				//上接本体
	double m_fNamedHeight;		//呼称高度
	CXhChar50 m_sHeightName;	//呼高名称
	CFGWORD m_dwLegCfgWord;		//接腿配材号
	int m_arrActiveQuadLegNo[4];
	CHashListEx<CModTowerInstance> hashTowerInstance;
	CHashStrList<int> hashSubLegSerial;
public:
	CModHeightGroup();
	void SetKey(DWORD key){m_iNo=key;}
	void SetBelongModel(CModModel* pModel){m_pModel=pModel;}
	BYTE GetLegBitSerialFromSerialId(int serial);
	CModTowerInstance* GetTowerInstance(int legSerialQuad1, int legSerialQuad2, int legSerialQuad3, int legSerialQuad4);
};

struct MOD_HANG_NODE
{
	char m_ciWireType;		//线类型:'C'导线|'E'地线|'J'跳线
	char m_ciHangingStyle;	//挂串类型：0.单挂|'S'双挂|'V'V挂
	BYTE m_ciLoopSerial;	//回路序号
	BYTE m_ciPhaseSerial;	//相序号
	BYTE m_ciHangOrder;		//编码号
	BYTE m_ciHangDirect;	//挂点朝向:'Q'前侧|'H'后侧
	char m_sHangName[50];	//挂点名称
	GEPOINT m_xHangPos;		//位置
	//
	MOD_HANG_NODE(){
		m_ciLoopSerial=0;
		m_ciPhaseSerial=0;
		m_ciHangOrder=0;
		m_ciHangDirect=0;
		m_ciWireType=0;
		m_ciHangingStyle=0;
		strcpy(m_sHangName,"");
	}
	void Clone(MOD_HANG_NODE* pSrcHangNode)
	{
		m_ciWireType=pSrcHangNode->m_ciWireType;
		m_ciHangingStyle=pSrcHangNode->m_ciHangingStyle;
		m_ciLoopSerial=pSrcHangNode->m_ciLoopSerial;
		m_ciPhaseSerial=pSrcHangNode->m_ciPhaseSerial;
		m_ciHangOrder=pSrcHangNode->m_ciHangOrder;
		m_ciHangDirect=pSrcHangNode->m_ciHangDirect;
		m_xHangPos=pSrcHangNode->m_xHangPos;
		strcpy(m_sHangName,pSrcHangNode->m_sHangName);
	}
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
	CLegItem(){m_fSegmentH=0;m_fMaxLegH=0;};
	~CLegItem(){;}
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
};

class CModModel
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
	//
	static char QueryBriefMatMark(const char* sMatMark);
	static void QuerySteelMatMark(char cMat,char* sMatMark);
	static BOOL IsUTF8File(const char* mod_file);
	//
	void Empty();
	GECS BuildUcsByModCS();	//建立LDS模型坐标系（以MOD模型坐标系为基底参照）
	long GetSerialId(){return m_iSerial;}
	void ReadModFile(FILE* fp,BOOL bUtf8=FALSE);
	void WriteModFileByUtf8(FILE* fp);
	void WriteModFileByAnsi(FILE* fp);
	//
	int GetHeightGroupNum(){return ModHeightGroup.GetNodeNum();}
	CModHeightGroup* AppendHeightGroup(DWORD key){return ModHeightGroup.Add(key);}
	CModHeightGroup* EnumFirstHGroup(){return ModHeightGroup.GetFirst();}
	CModHeightGroup* EnumNextHGroup(){return ModHeightGroup.GetNext();}
	CModHeightGroup* GetHeightGroup(int iNo);
	CModHeightGroup* GetHeightGroup(const char* sName);
	//
	CModNode* AppendNode(DWORD key){return ModNodes.Add(key);}
	CModNode* FindNode(DWORD key){return ModNodes.GetValue(key);}
	CModNode* EnumFirstNode(){return ModNodes.GetFirst();}
	CModNode* EnumNextNode(){return ModNodes.GetNext();}
	//
	CModRod* AppendRod(DWORD key){return ModRods.Add(key);}
	CModRod* FindRod(DWORD key){return ModRods.GetValue(key);}
	CModRod* EnumFirstRod(){return ModRods.GetFirst();}
	CModRod* EnumNextRod(){return ModRods.GetNext();}
};

class CModModelFactory{
public:
	static CModModel* CreateModModel();
	static CModModel* ModModelFromSerial(long serial);
	static bool Destroy(long serial);
};