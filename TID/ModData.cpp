#include "stdafx.h"
#include "ModData.h"
#include "CreateFace.h"

void ANSIToUnicode(const char* src_str,wchar_t* des_str)
{
	int nLen=MultiByteToWideChar(CP_ACP,0,src_str,-1,NULL,0 );
	wchar_t* sResult=(wchar_t*)malloc((nLen+1)*sizeof(wchar_t));
	memset(sResult,0,sizeof(wchar_t)*(nLen+1));
	MultiByteToWideChar(CP_ACP,0,src_str,-1,(LPWSTR)sResult,nLen);
	//
	memset(des_str,0,MAX_PATH);
	wcscpy(des_str,sResult);
	free(sResult);
}

void UnicodeToANSI(const wchar_t* src_str,char* des_str)
{
	int nLen=WideCharToMultiByte(CP_ACP,0,src_str,-1, NULL,0,NULL,NULL);
	char* sResult=(char*)malloc((nLen+1)*sizeof(char));
	memset(sResult,0,sizeof(char)*(nLen+1));
	WideCharToMultiByte(CP_ACP,0,src_str,-1,sResult,nLen,NULL,NULL);
	//
	memset(des_str,0,MAX_PATH);
	strcpy(des_str,sResult);
	free(sResult);
}

void UTF8ToUnicode(const char* src_str,wchar_t* des_str)
{
	int nLen=MultiByteToWideChar(CP_UTF8,0,src_str,-1, NULL,0);
	wchar_t* sResult=(wchar_t*)malloc((nLen+1)*sizeof(wchar_t));
	memset(sResult,0,(nLen+1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8,0,src_str,-1,(LPWSTR)sResult,nLen);
	//
	memset(des_str,0,MAX_PATH);
	wcscpy(des_str,sResult);
	free(sResult);
}

void UnicodeToUTF8(const wchar_t* src_str,char* des_str)
{
	int nLen=WideCharToMultiByte(CP_UTF8,0,src_str,-1,NULL,0,NULL,NULL);
	char* sResult=(char*)malloc((nLen+1)*sizeof(char));
	memset(sResult,0,sizeof(char)*(nLen+1));
	WideCharToMultiByte(CP_UTF8,0,src_str,-1,sResult,nLen,NULL,NULL);
	//
	memset(des_str,0,MAX_PATH);
	strcpy(des_str,sResult);
	free(sResult);
}
void ANSIToUTF8(const char* src_str,char* des_str)
{
	wchar_t sWText[MAX_PATH];
	ANSIToUnicode(src_str,sWText);	//ansi多字符转宽字符
	UnicodeToUTF8(sWText,des_str);	//宽字符转utf8多字符
}
void UTF8ToANSI(const char* src_str,char* des_str)
{
	wchar_t sWText[MAX_PATH];
	UTF8ToUnicode(src_str,sWText);	//utf8多字符转宽字符
	UnicodeToANSI(sWText,des_str);	//宽字符转ansi多字符
}
//////////////////////////////////////////////////////////////////////////
//CModNode
CModNode::CModNode()
{
	m_pBelongModel=NULL;
	point_i=0;
	strcpy(m_sLayer,"");
}
BYTE CModNode::GetLegQuad()
{
	if(m_sLayer[0]=='L')
	{
		if(xOrg.x>=0&&xOrg.y>=0)
			return 1;
		else if(xOrg.x<=0&&xOrg.y>=0)
			return 2;
		else if(xOrg.x>=0&&xOrg.y<=0)
			return 3;
		else if(xOrg.x<=0&&xOrg.y<=0)
			return 4;
		else
			return 0;
	}
	else
		return 0;
}
//////////////////////////////////////////////////////////////////////////
//CModRod
CModRod::CModRod()
{
	handle=0;
	m_pBelongModel=NULL;
	m_ciRodType=1;	//1.角钢|2.钢管
	m_cMaterial='S';
	m_fWidth=100;
	m_fThick=10;
	m_uiNodeS=m_uiNodeE=0;
	strcpy(m_sLayer,"");
}
CModRod::~CModRod()
{

}
f3dLine CModRod::GetBaseLineToLdsModel()
{
	UCS_STRU ucs=m_pBelongModel->BuildUcsByModCS();
	f3dLine line=base_line;
	coord_trans(line.startPt,ucs,FALSE);
	coord_trans(line.endPt,ucs,FALSE);
	return line;
}
UCS_STRU CModRod::BuildUcs(BOOL bToLds/*=TRUE*/)
{
	UCS_STRU ucs,lds_ucs=m_pBelongModel->BuildUcsByModCS();
	f3dPoint ptS=base_line.startPt;
	f3dPoint ptE=base_line.endPt;
	if(bToLds)
	{
		coord_trans(ptS,lds_ucs,FALSE);
		coord_trans(ptE,lds_ucs,FALSE);
	}
	f3dPoint exten_vec=(ptE-ptS).normalized();
	if(m_ciRodType==1)
	{
		f3dPoint vNormX=m_vNormX,vNormY=m_vNormY;
		if(bToLds)
		{
			vector_trans(vNormX,lds_ucs,FALSE);
			vector_trans(vNormY,lds_ucs,FALSE);
		}
		ucs.origin=ptS;
		ucs.axis_z=exten_vec;
		ucs.axis_x=ucs.axis_z^vNormX;
		normalize(ucs.axis_x);
		ucs.axis_y=ucs.axis_z^ucs.axis_x;
		normalize(ucs.axis_y);
	}
	else if(m_ciRodType==2)
	{
		ucs.origin=ptS;
		ucs.axis_z=exten_vec;
		ucs.axis_y=inters_vec(ucs.axis_z);
		normalize(ucs.axis_z);
		ucs.axis_x=ucs.axis_y^ucs.axis_z;
		normalize(ucs.axis_x);
		ucs.axis_y=ucs.axis_z^ucs.axis_x;
		normalize(ucs.axis_y);
	}
	return ucs;
}
//生成角钢实体
BOOL CModRod::Create3dJgSolidModel(CSolidBody *pSolidBody,BOOL bToLds/*=TRUE*/)
{
	UCS_STRU ucs=BuildUcs(bToLds);
	fBody body;
	fBody* pBody=&body;
	//初始化轮廓点
	f3dPoint norm_X(0,-1,0),norm_Y(-1,0,0);
	f3dPoint wing_x_vec(1,0,0),wing_y_vec(0,1,0);
	double fLen=DISTANCE(base_line.startPt,base_line.endPt);
	f3dPoint vertex_arr[12];
	vertex_arr[0].Set(0,0,0);
	vertex_arr[1].Set(m_fWidth*wing_y_vec.x,m_fWidth*wing_y_vec.y);
	vertex_arr[2].Set(vertex_arr[1].x+m_fThick*norm_Y.x,vertex_arr[1].y-m_fThick*norm_Y.y);
	vertex_arr[3].Set(m_fThick,m_fThick);
	vertex_arr[4].Set(m_fWidth,m_fThick);
	vertex_arr[5].Set(m_fWidth,0,0);
	for(int i=0;i<6;i++)
		vertex_arr[i+6].Set(vertex_arr[i].x,vertex_arr[i].y,fLen);
	//生成面
	for(int i=0;i<12;i++)
		pBody->vertex.append(vertex_arr[i]);
	CCreateFace createFace;
	createFace.InitVertexList(pBody);
	COLORREF color(RGB(220,220,220));
	f3dPolyFace *pFace=NULL;
	//始端面
	pFace = pBody->faceList.append();
	pFace->material = color;
	createFace.NewOutterEdgeLine(pFace,1,0);
	createFace.NewOutterEdgeLine(pFace,2);
	createFace.NewOutterEdgeLine(pFace,3);
	createFace.NewOutterEdgeLine(pFace,4);
	createFace.NewOutterEdgeLine(pFace,5);
	createFace.NewOutterEdgeLine(pFace,0);
	//终端面
	pFace = pBody->faceList.append();
	pFace->material = color;
	createFace.NewOutterEdgeLine(pFace,11,6);
	createFace.NewOutterEdgeLine(pFace,10);
	createFace.NewOutterEdgeLine(pFace,9);
	createFace.NewOutterEdgeLine(pFace,8);
	createFace.NewOutterEdgeLine(pFace,7);
	createFace.NewOutterEdgeLine(pFace,6);
	//Y肢上底面
	pFace = pBody->faceList.append();
	pFace->material = color;
	createFace.NewOutterEdgeLine(pFace,2,3);
	createFace.NewOutterEdgeLine(pFace,8);
	createFace.NewOutterEdgeLine(pFace,9);
	createFace.NewOutterEdgeLine(pFace,3);
	//Y肢下底面
	pFace = pBody->faceList.append();
	pFace->material = color;
	createFace.NewOutterEdgeLine(pFace,0,1);
	createFace.NewOutterEdgeLine(pFace,6);
	createFace.NewOutterEdgeLine(pFace,7);
	createFace.NewOutterEdgeLine(pFace,1);
	//X肢左底面
	pFace = pBody->faceList.append();
	pFace->material = color;
	createFace.NewOutterEdgeLine(pFace,5,0);
	createFace.NewOutterEdgeLine(pFace,11);
	createFace.NewOutterEdgeLine(pFace,6);
	createFace.NewOutterEdgeLine(pFace,0);
	//X肢右底面
	pFace = pBody->faceList.append();
	pFace->material = color;
	createFace.NewOutterEdgeLine(pFace,3,4);
	createFace.NewOutterEdgeLine(pFace,9);
	createFace.NewOutterEdgeLine(pFace,10);
	createFace.NewOutterEdgeLine(pFace,4);
	//X肢上端面
	pFace = pBody->faceList.append();
	pFace->material = color;
	createFace.NewOutterEdgeLine(pFace,4,5);
	createFace.NewOutterEdgeLine(pFace,10);
	createFace.NewOutterEdgeLine(pFace,11);
	createFace.NewOutterEdgeLine(pFace,5);
	//8.Y肢右端面
	pFace = pBody->faceList.append();
	pFace->material = color;
	createFace.NewOutterEdgeLine(pFace,1,2);
	createFace.NewOutterEdgeLine(pFace,7);
	createFace.NewOutterEdgeLine(pFace,8);
	createFace.NewOutterEdgeLine(pFace,2);
	//坐标系由相对坐标转换到结构体的绝对坐标下
	for(f3dPoint *pVertice=pBody->vertex.GetFirst();pVertice;pVertice=pBody->vertex.GetNext())
	{
		if(coord_trans(*pVertice,ucs,TRUE)!=TRUE)
			return FALSE;
	}
	pSolidBody->ConvertFrom(pBody);
	return TRUE;
}
//生成钢管实体
BOOL CModRod::Create3dTubeSolidModel(CSolidBody *pSolidBody,BOOL bToLds/*=TRUE*/)
{
	return TRUE;
}
BOOL CModRod::Create3dSolidModel(CSolidBody *pSolidBody,BOOL bToLds/*=TRUE*/)
{
	if(m_ciRodType==1)
		return Create3dJgSolidModel(pSolidBody,bToLds);
	else if(m_ciRodType==2)
		return Create3dTubeSolidModel(pSolidBody,bToLds);
	else
		return FALSE;
}
BYTE CModRod::GetLegQuad()
{
	if(m_sLayer[0]=='L')
	{
		GEPOINT ptS=base_line.startPt;
		GEPOINT ptE=base_line.endPt;
		if(ptS.x>=0&&ptS.y>=0&&ptE.x>=0&&ptE.y>=0)
			return 1;
		else if(ptS.x<=0&&ptS.y>=0&&ptE.x<=0&&ptE.y>=0)
			return 2;
		else if(ptS.x>=0&&ptS.y<=0&&ptE.x>=0&&ptE.y<=0)
			return 3;
		else if(ptS.x<=0&&ptS.y<=0&&ptE.x<=0&&ptE.y<=0)
			return 4;
		else
			return 0;
	}
	else
		return 0;
}
//////////////////////////////////////////////////////////////////////////
//CModHeightGroup
CModHeightGroup::CModHeightGroup()
{
	m_iBody=0;
	m_iNo=0;
	m_fNamedHeight=0;
	m_dwLegCfgWord.SetWordByNo(1);	//接腿配材号
	for(int i=0;i<4;i++)
		m_arrActiveQuadLegNo[i]=1;
}
BYTE CModHeightGroup::GetLegBitSerialFromSerialId(int serial)
{
	int legBitSerial=0;
	for(int i=1;i<=192;i++)
	{
		if(m_dwLegCfgWord.IsHasNo(i))
		{
			if(legBitSerial==0)
				legBitSerial=1;
		}
		if(legBitSerial==serial)
			return i;
		if(legBitSerial>0)
			legBitSerial++;
		if(legBitSerial>24)
			break;	//一组呼高最多允许有24组接腿
	}
	return 0;
}
CModTowerInstance* CModHeightGroup::GetTowerInstance(int legSerialQuad1,int legSerialQuad2,int legSerialQuad3,int legSerialQuad4)
{
	union UNIQUEID{
		BYTE bytes[4];
		DWORD id;
	}key;
	key.bytes[0]=GetLegBitSerialFromSerialId(legSerialQuad1);
	key.bytes[1]=GetLegBitSerialFromSerialId(legSerialQuad2);
	key.bytes[2]=GetLegBitSerialFromSerialId(legSerialQuad3);
	key.bytes[3]=GetLegBitSerialFromSerialId(legSerialQuad4);
	//
	int nMaxLegs=CFGLEG::MaxLegs();
	m_arrActiveQuadLegNo[0]=(m_iNo-1)*nMaxLegs+legSerialQuad1;
	m_arrActiveQuadLegNo[1]=(m_iNo-1)*nMaxLegs+legSerialQuad2;
	m_arrActiveQuadLegNo[2]=(m_iNo-1)*nMaxLegs+legSerialQuad3;
	m_arrActiveQuadLegNo[3]=(m_iNo-1)*nMaxLegs+legSerialQuad4;
	CModTowerInstance* pModTowerInstance=hashTowerInstance.Add(key.id);
	pModTowerInstance->SetBelongHuGao(this);
	if(pModTowerInstance->GetModRodNum()==0)
	{	
		//提取节点集合
		for(CModNode* pNode=m_pModel->EnumFirstNode();pNode;pNode=m_pModel->EnumNextNode())
		{
			if(pNode->IsLegNode())
			{
				BYTE ciQuad=pNode->GetLegQuad();
				if(ciQuad>=1&&ciQuad<=4&&pNode->cfgword.IsHasNo(m_arrActiveQuadLegNo[ciQuad-1]))
				{
					pModTowerInstance->AppendNode(pNode);
					pModTowerInstance->SetBaseLocation(ciQuad,pNode->xOrg);
				}
			}
			else if(pNode->cfgword.IsHasBodyNo(m_iNo))
				pModTowerInstance->AppendNode(pNode);
		}
		//提取构件装配集合
		for(CModRod* pRod=m_pModel->EnumFirstRod();pRod;pRod=m_pModel->EnumNextRod())
		{
			if(pRod->IsLegRod())
			{
				BYTE ciQuad=pRod->GetLegQuad();
				if(ciQuad>=1&&ciQuad<=4&&pRod->cfgword.IsHasNo(m_arrActiveQuadLegNo[ciQuad-1]))
					pModTowerInstance->AppendRod(pRod);
			}
			else if(pRod->cfgword.IsHasBodyNo(m_iNo))
				pModTowerInstance->AppendRod(pRod);
		}
	}
	return pModTowerInstance;
}
//////////////////////////////////////////////////////////////////////////
//CModModel
CModModel::CModModel(long serial/*=0*/)
{
	m_iSerial=serial;
	m_fTowerHeight=0;
	Empty();
}
CModModel::~CModModel()
{

}
char CModModel::QueryBriefMatMark(const char* sMatMark)
{
	char cMat='S';
	if(strstr(sMatMark,"Q345"))
		cMat='H';
	else if(strstr(sMatMark,"Q390"))
		cMat='G';
	else if(strstr(sMatMark,"Q420"))
		cMat='P';
	else if(strstr(sMatMark,"Q460"))
		cMat='T';
	return cMat;
}
void CModModel::QuerySteelMatMark(char cMat,char* sMatMark)
{
	if(cMat=='H')
		strcpy(sMatMark,"Q345");
	else if(cMat=='G')
		strcpy(sMatMark,"Q390");
	else if(cMat=='P')
		strcpy(sMatMark,"Q420");
	else if(cMat=='T')
		strcpy(sMatMark,"Q460");
	else
		strcpy(sMatMark,"Q235");
}
void CModModel::Empty()
{
	m_listLegItem.Empty();
	m_listBodyItem.Empty();
	m_listGuaNode.Empty();
	ModHeightGroup.Empty();
	ModRods.Empty();
	ModNodes.Empty();
}
CModHeightGroup* CModModel::GetHeightGroup(int iNo)
{
	CModHeightGroup* pHeightGroup=ModHeightGroup.GetValue(iNo);
	if(pHeightGroup)
		pHeightGroup->SetBelongModel(this);
	return pHeightGroup;
}
CModHeightGroup* CModModel::GetHeightGroup(const char* sName)
{
	CModHeightGroup* pHeightGroup=NULL;
	for(pHeightGroup=ModHeightGroup.GetFirst();pHeightGroup;pHeightGroup=ModHeightGroup.GetNext())
	{
		if(pHeightGroup->m_sHeightName.EqualNoCase(sName))
			break;
	}
	return pHeightGroup;
}
GECS CModModel::BuildUcsByModCS()
{
	GECS ucs;
	ucs.origin.Set(0,0,m_fTowerHeight);
	ucs.axis_x.Set(1,0,0);
	ucs.axis_y.Set(0,-1,0);
	ucs.axis_z.Set(0,0,-1);
	return ucs;
}
void CModModel::AmendModData()
{
	//初始化呼高
	for(CModHeightGroup* pHeightGroup=ModHeightGroup.GetFirst();pHeightGroup;pHeightGroup=ModHeightGroup.GetNext())
	{
		if(pHeightGroup->m_fNamedHeight>0)
		{
			double height=fto_halfi(pHeightGroup->m_fNamedHeight*0.001);
			CXhChar50 name(height);
			name.Append('m');
			strcpy(pHeightGroup->m_sHeightName,name);
		}
		else
			sprintf(pHeightGroup->m_sHeightName,"呼高%d",pHeightGroup->m_iNo);
	}
	//初始化构件配材号
	int nMaxLegsPerBody=0;
	for(CLegItem* pLegItem=m_listLegItem.GetFirst();pLegItem;pLegItem=m_listLegItem.GetNext())
	{
		int nLeg=pLegItem->subLegInfoList.GetNodeNum();
		if(nMaxLegsPerBody<nLeg)
			nMaxLegsPerBody=nLeg;
	}
	if(nMaxLegsPerBody<=8&&CFGLEG::MaxLegs()!=8)
		CFGLEG::SetSchema(CFGLEG::MULTILEG_MAX08);
	else if(nMaxLegsPerBody> 8&&nMaxLegsPerBody<=16&&CFGLEG::MaxLegs()!=16)
		CFGLEG::SetSchema(CFGLEG::MULTILEG_MAX16);
	else if(nMaxLegsPerBody>16&&nMaxLegsPerBody<=24&&CFGLEG::MaxLegs()!=24)
		CFGLEG::SetSchema(CFGLEG::MULTILEG_MAX24);
	const DWORD flagConstArr[24]={
		0x000001,0x000002,0x000004,0x000008,0x000010,0x000020,0x000040,0x000080,
		0x000100,0x000200,0x000400,0x000800,0x001000,0x002000,0x004000,0x008000,
		0x010000,0x020000,0x040000,0x080000,0x100000,0x200000,0x400000,0x800000	};
	for(CLegItem* pLegItem=m_listLegItem.GetFirst();pLegItem;pLegItem=m_listLegItem.GetNext())
	{
		CModHeightGroup* pHeightGroup=ModHeightGroup.GetValue(pLegItem->m_iLegNo);
		if(pHeightGroup==NULL)
			continue;
		pHeightGroup->m_dwLegCfgWord.Clear();
		pHeightGroup->m_dwLegCfgWord.AddBodyLegs(pHeightGroup->m_iNo,1);
		//呼高接腿
		int legword=0;
		for(SUB_LEG_INFO* pSubLeg=pLegItem->subLegInfoList.GetFirst();pSubLeg;pSubLeg=pLegItem->subLegInfoList.GetNext())
		{
			CFGWORD cfgword;
			cfgword.AddBodyLegs(pHeightGroup->m_iNo,flagConstArr[legword]);
			pHeightGroup->m_dwLegCfgWord.AddSpecWord(cfgword);
			for(CModNode* pNode=pSubLeg->legNodeSet.GetFirst();pNode;pNode=pSubLeg->legNodeSet.GetNext())
			{
				pNode->cfgword.AddBodyLegs(pHeightGroup->m_iNo,flagConstArr[legword]);
				pNode->m_sLayer[0]='L';
			}
			for(CModRod* pRod=pSubLeg->legRodSet.GetFirst();pRod;pRod=pSubLeg->legRodSet.GetNext())
			{
				pRod->cfgword.AddBodyLegs(pHeightGroup->m_iNo,flagConstArr[legword]);
				pRod->m_sLayer[0]='L';
			}
			legword++;
			//根据减腿高度记录减腿序号
			double height=fto_halfi(pSubLeg->m_fLegH*0.001);
			pHeightGroup->hashSubLegSerial.SetValue(CXhChar50(height),legword);
		}
		//呼高独属段
		for(CModNode* pNode=pLegItem->segmentNodeSet.GetFirst();pNode;pNode=pLegItem->segmentNodeSet.GetNext())
		{
			pNode->cfgword.SetBodyLegs(pHeightGroup->m_iNo);
			pNode->m_sLayer[0]='S';
		}
		for(CModRod* pRod=pLegItem->segmentRodSet.GetFirst();pRod;pRod=pLegItem->segmentRodSet.GetNext())
		{
			pRod->cfgword.SetBodyLegs(pHeightGroup->m_iNo);
			pRod->m_sLayer[0]='S';
		}
	}
	//本体(呼高共享段)
	for(CBodyItem* pBodyItem=m_listBodyItem.GetFirst();pBodyItem;pBodyItem=m_listBodyItem.GetNext())
	{
		CFGWORD cfgword;
		for(int iNo=pBodyItem->m_iLegS;iNo<=m_listLegItem.GetNodeNum();iNo++)
			cfgword.AddBodyLegs(iNo);
		for(CModNode* pNode=pBodyItem->bodyNodeSet.GetFirst();pNode;pNode=pBodyItem->bodyNodeSet.GetNext())
		{
			pNode->cfgword=cfgword;
			pNode->m_sLayer[0]='B';
		}
		for(CModRod* pRod=pBodyItem->bodyRodSet.GetFirst();pRod;pRod=pBodyItem->bodyRodSet.GetNext())
		{
			pRod->cfgword=cfgword;
			pRod->m_sLayer[0]='B';
		}
	}
	//解析挂点信息
	for(MOD_HANG_NODE* pGuaInfo=m_listGuaNode.GetFirst();pGuaInfo;pGuaInfo=m_listGuaNode.GetNext())
	{
		char* pszWireType=strstr(pGuaInfo->m_sHangName,"导");
		if(pszWireType==NULL)
			pszWireType=strstr(pGuaInfo->m_sHangName,"地");
		if(pszWireType==NULL)
			pszWireType=strstr(pGuaInfo->m_sHangName,"跳");
		if(pszWireType!=NULL)
		{
			pszWireType+=2;
			char* pchSpliter=strchr(pszWireType,'-');
			if(pchSpliter)
				*pchSpliter=0;
			int iPhaseSerial=atoi(pszWireType);
			if(pGuaInfo->m_ciWireType!='E'&&iPhaseSerial>0)
			{
				iPhaseSerial-=1;
				pGuaInfo->m_ciLoopSerial=1+(iPhaseSerial/3); 
				pGuaInfo->m_ciPhaseSerial=1+iPhaseSerial%3;
			}
			else
				pGuaInfo->m_ciPhaseSerial=iPhaseSerial;
			pGuaInfo->m_ciHangingStyle=0;
			pGuaInfo->m_ciHangOrder=0;
			if(pchSpliter && pGuaInfo->m_ciWireType!='T')
			{	//提取挂串类型
				pchSpliter++;
				if(*pchSpliter=='S'||*pchSpliter=='s')
					pGuaInfo->m_ciHangingStyle='S';
				else if(*pchSpliter=='V'||*pchSpliter=='v')
					pGuaInfo->m_ciHangingStyle='V';
			}
			else if(pchSpliter && pGuaInfo->m_ciWireType=='T')
			{	//提取跳线附加码
				pchSpliter++;
				pGuaInfo->m_ciHangOrder=atoi(pchSpliter);
			}
			if(pGuaInfo->m_ciHangingStyle!=0)
			{	//提取导|地线附加码
				pchSpliter++;
				pGuaInfo->m_ciHangOrder=atoi(pchSpliter);
			}
		}
	}
}
void CModModel::ReadModFile(FILE* fp,BOOL bUtf8/*=FALSE*/)
{
	m_fTowerHeight=0;
	int iLegNo=0;
	BYTE ciReadType=0;	//0:本体|1.接腿|2.子腿
	CModHeightGroup* pModuleItem=NULL;
	CBodyItem* pBodyItem=NULL;
	CLegItem* pLegItem=NULL;
	SUB_LEG_INFO* pSubLeg=NULL;
	CXhChar100 line_txt,sText,key_word;
	char sLine1[MAX_PATH]="",sLine2[MAX_PATH]="";
	while(!feof(fp))
	{
		if(fgets(sLine1,MAX_PATH,fp)==NULL)
			break;
		if(bUtf8)
			UTF8ToANSI(sLine1,sLine2);
		else
			strcpy(sLine2,sLine1);
		line_txt.Copy(sLine2);
		line_txt.Replace('\t',' ');
		line_txt.Remove(' ');
		strcpy(sText,line_txt);
		char *skey=strtok(sText,",");
		strncpy(key_word,skey,100);
		if(strstr(key_word,"HBody")||strstr(key_word,"HLeg")||strstr(key_word,"HNum"))
			continue;
		if(strstr(key_word,"HSubLeg")&&pSubLeg)
		{
			skey=strtok(NULL,",");
			pSubLeg->m_fLegH=(skey!=NULL)?atof(skey):0;
			continue;
		}
		if(stricmp(key_word,"H")==0)
		{
			skey=strtok(NULL,",");
			double height=skey!=NULL?atof(skey):0;
			if(skey!=NULL)	//解析对接本体
				skey=strtok(NULL,",");
			if(skey!=NULL)	//解析对接接腿（呼高组）
			{
				UINT idBody=0,iLeg=0;
				CXhChar50 itemstr;
				itemstr.NCopy(&skey,4,true);
				if(itemstr.EqualNoCase("Body"))
					idBody=atoi(skey);
				skey=strtok(NULL,",");
				if(skey!=NULL)
				{
					itemstr.NCopy(&skey,3,true);
					if(itemstr.EqualNoCase("Leg"))
						iLeg=atoi(skey);
					pModuleItem=ModHeightGroup.Add(iLeg);
					pModuleItem->m_fNamedHeight=height;
					pModuleItem->m_iBody=idBody;
				}
			}
			continue;
		}
		else if(strstr(key_word,"Body"))
		{	//本体
			ciReadType=0;
			pBodyItem=m_listBodyItem.append();
		}
		else if(strstr(key_word,"Leg")&&key_word[0]=='L')
		{	//接腿
			ciReadType=1;
			iLegNo+=1;
			pLegItem=m_listLegItem.append();
			pLegItem->m_iLegNo=iLegNo;
			if(pBodyItem->m_iLegS==0)
				pBodyItem->m_iLegS=iLegNo;
		}
		else if(strstr(key_word,"SubLeg"))
		{	//子腿
			ciReadType=2;
			pSubLeg=pLegItem->subLegInfoList.append();
		}
		else
		{
			if(stricmp(key_word,"G")!=0)
				line_txt.Replace(',',' ');
			key_word.Remove(' ');
			if(stricmp(key_word,"P")==0)
			{	//节点
				f3dPoint pos;
				sscanf(line_txt,"%s%d%lf%lf%lf",(char*)key_word,&pos.feature,&pos.x,&pos.y,&pos.z);
				if(pos.z>m_fTowerHeight)
					m_fTowerHeight=pos.z;
				CModNode* pNode=ModNodes.Add(pos.feature);
				pNode->SetBelongModel(this);
				pNode->cfgword.Clear();
				pNode->point_i=pos.feature;
				pNode->xOrg=pos;
				if(ciReadType==0 && pBodyItem)
					pBodyItem->bodyNodeSet.append(pNode);
				else if(ciReadType==1 && pLegItem)
					pLegItem->segmentNodeSet.append(pNode);
				else if(ciReadType==2 && pSubLeg)
					pSubLeg->legNodeSet.append(pNode);
			}
			else if(stricmp(key_word,"R")==0)
			{	//杆件
				int indexS=0,indexE=0;
				double fWidth=0,fThick=0;
				char sMat[16]="",sSpec[16]="";
				if(strstr(line_txt,"L"))
				{	//角钢
					f3dPoint ptX,ptY;
					sscanf(line_txt,"%s%d%d%s%s%lf%lf%lf%lf%lf%lf",(char*)key_word,&indexS,&indexE,sSpec,sMat,&ptX.x,&ptX.y,&ptX.z,&ptY.x,&ptY.y,&ptY.z);
					CModNode* pNodeS=ModNodes.GetValue(indexS);
					CModNode* pNodeE=ModNodes.GetValue(indexE);
					if(pNodeS==NULL || pNodeE==NULL)
					{
						logerr.Log("Mod杆件数据有误(始端点号%d,终端点号%d)!",indexS,indexE);
						continue;
					}
					GEPOINT axis_z=(pNodeE->xOrg-pNodeS->xOrg).normalized();
					char mark,symbol;
					sscanf(sSpec,"%c%lf%c%lf",&symbol,&fWidth,&mark,&fThick);
					//
					CModRod* pRod=ModRods.Add(0);
					pRod->SetBelongModel(this);
					pRod->cfgword.Clear();
					pRod->m_ciRodType=1;
					pRod->m_uiNodeS=indexS;
					pRod->m_uiNodeE=indexE;
					pRod->m_cMaterial=CModModel::QueryBriefMatMark(sMat);
					pRod->m_fWidth=fWidth;
					pRod->m_fThick=fThick;
					pRod->base_line.startPt=pNodeS->xOrg;
					pRod->base_line.endPt=pNodeE->xOrg;
					pRod->m_vWingX=ptX;	//X肢方向
					pRod->m_vWingY=ptY;	//Y肢方向
					RotateVectorAroundVector(ptX,-1,0,axis_z);
					RotateVectorAroundVector(ptY, 1,0,axis_z);
					pRod->m_vNormX=ptX;	//X肢法向
					pRod->m_vNormY=ptY;	//Y肢法向
					//
					if(ciReadType==0 && pBodyItem)
						pBodyItem->bodyRodSet.append(pRod);
					else if(ciReadType==1 && pLegItem)
						pLegItem->segmentRodSet.append(pRod);
					else if(ciReadType==2 && pSubLeg)
						pSubLeg->legRodSet.append(pRod);
				}
				else if(strstr(line_txt,"Φ")||strstr(line_txt,"φ"))
				{	//钢管
					sscanf(line_txt,"%s%d%d%s",(char*)key_word,&indexS,&indexE,sSpec);
					CModNode* pNodeS=ModNodes.GetValue(indexS);
					CModNode* pNodeE=ModNodes.GetValue(indexE);
					if(pNodeS==NULL || pNodeE==NULL)
					{
						logerr.Log("Mod杆件数据有误(始端点号%d,终端点号%d)!",indexS,indexE);
						continue;
					}
					char mark,symbol;
					CXhChar16 ss(sSpec);
					ss.Replace("φ","G");
					sscanf(ss,"%c%lf%c%lf",&symbol,&fWidth,&mark,&fThick);
					//
					CModRod* pRod=ModRods.Add(0);
					pRod->SetBelongModel(this);
					pRod->cfgword.Clear();
					pRod->m_ciRodType=2;
					pRod->m_uiNodeS=indexS;
					pRod->m_uiNodeE=indexE;
					pRod->m_fWidth=(float)fWidth;
					pRod->m_fThick=(float)fThick;
					pRod->base_line.startPt=pNodeS->xOrg;
					pRod->base_line.endPt=pNodeE->xOrg;
					//
					if(ciReadType==0 && pBodyItem)
						pBodyItem->bodyRodSet.append(pRod);
					else if(ciReadType==1 && pLegItem)
						pLegItem->segmentRodSet.append(pRod);
					else if(ciReadType==2 && pSubLeg)
						pSubLeg->legRodSet.append(pRod);
				}
				else
				{
					logerr.Log("Mod杆件数据有误(始端点号%d,终端点号%d)!",indexS,indexE);
					continue;
				}
			}
			else if(stricmp(key_word,"G")==0)
			{	//挂点
				CXhChar16 sType;
				MOD_HANG_NODE* pGuaInfo=m_listGuaNode.append();
				CXhChar200 szBackupText=line_txt;
				char* pszKey=strtok(line_txt,", ");
				int indexOfPrevKeyAddr=0;
				for(int i=0;pszKey!=NULL&&i<6;i++)
				{
					int indexOfCurrKey=pszKey-(char*)line_txt;
					for(int j=indexOfPrevKeyAddr;j<indexOfCurrKey-1;j++)
					{
						if(szBackupText[j]==',')
							i++;
					}
					indexOfPrevKeyAddr=indexOfCurrKey;
					if(i==1)	//线缆类型
						pGuaInfo->m_ciWireType=*pszKey;
					else if(i==2)	//挂点名称
						StrCopy(pGuaInfo->m_sHangName,pszKey,50);
					else if(i==3)
						pGuaInfo->m_xHangPos.x=atof(pszKey);
					else if(i==4)
						pGuaInfo->m_xHangPos.y=atof(pszKey);
					else if(i==5)
						pGuaInfo->m_xHangPos.z=atof(pszKey);
					pszKey=strtok(NULL,", ");
				}
			}
		}
	}
	fclose(fp);
	AmendModData();
}
void CModModel::WriteModFileByUtf8(FILE* fp)
{
	if(fp==NULL)
		return;
	CModNode* pNode=NULL;
	CModRod* pRod=NULL;
	//输出弧高信息
	fwprintf(fp,L"HNum,%d\n",ModHeightGroup.GetNodeNum());
	for(CModHeightGroup* pModuleItem=ModHeightGroup.GetFirst();pModuleItem;pModuleItem=ModHeightGroup.GetNext())
		fwprintf(fp,L"H,%.0f,Body%d,Leg%d\n",pModuleItem->m_fNamedHeight,pModuleItem->m_iBody,pModuleItem->m_iNo);
	//输出多本体多接腿信息
	int body_index=1,leg_index=1;
	for(CBodyItem* pBodyItem=m_listBodyItem.GetFirst();pBodyItem;pBodyItem=m_listBodyItem.GetNext())
	{
		//本体信息
		fwprintf(fp,L"Body%d\n",body_index);
		fwprintf(fp,L"HBody%d,%.0f\n",body_index,pBodyItem->m_fBodyH);
		for(pNode=pBodyItem->bodyNodeSet.GetFirst();pNode;pNode=pBodyItem->bodyNodeSet.GetNext())
			fwprintf(fp,L"P,%d,%f,%f,%f\n",pNode->point_i,pNode->xOrg.x,pNode->xOrg.y,pNode->xOrg.z);
		for(pRod=pBodyItem->bodyRodSet.GetFirst();pRod;pRod=pBodyItem->bodyRodSet.GetNext())
		{
			wchar_t sWMat[MAX_PATH],sWSpec[MAX_PATH];
			CXhChar16 sMat,sSpec;
			CModModel::QuerySteelMatMark(pRod->m_cMaterial,sMat);
			ANSIToUnicode(sMat,sWMat);
			if(pRod->m_ciRodType==1)
			{
				sprintf(sSpec,"L%.0fX%.0f",pRod->m_fWidth,pRod->m_fThick);
				ANSIToUnicode(sSpec,sWSpec);
				fwprintf(fp,L"R,%d,%d,%s,%s,%f,%f,%f,%f,%f,%f\n",pRod->m_uiNodeS,pRod->m_uiNodeE,sWSpec,sWMat,
					pRod->m_vWingX.x,pRod->m_vWingX.y,pRod->m_vWingX.z,
					pRod->m_vWingY.x,pRod->m_vWingY.y,pRod->m_vWingY.z);
			}
			else if(pRod->m_ciRodType==2)
			{
				sprintf(sSpec,"φ%.0fX%.0f",pRod->m_fWidth,pRod->m_fThick);
				ANSIToUnicode(sSpec,sWSpec);
				fwprintf(fp,L"R,%d,%d,%s,%s\n",pRod->m_uiNodeS,pRod->m_uiNodeE,sWSpec,sWMat);
			}
			else
				fwprintf(fp,L"R,%d,%d\n",pRod->m_uiNodeS,pRod->m_uiNodeE);
		}
		//该本体下的接腿信息
		for(CLegItem* pLegItem=m_listLegItem.GetFirst();pLegItem;pLegItem=m_listLegItem.GetNext())
		{
			if(pLegItem->m_hTagNode!=pBodyItem->m_hTagNode)
				continue;
			fwprintf(fp,L"Leg%d\n",leg_index);
			fwprintf(fp,L"HLeg%d,%.0f,%.0f\n",leg_index,pLegItem->m_fSegmentH,pLegItem->m_fMaxLegH);
			for(pNode=pLegItem->segmentNodeSet.GetFirst();pNode;pNode=pLegItem->segmentNodeSet.GetNext())
				fwprintf(fp,L"P,%d,%f,%f,%f\n",pNode->point_i,pNode->xOrg.x,pNode->xOrg.y,pNode->xOrg.z);
			for(pRod=pLegItem->segmentRodSet.GetFirst();pRod;pRod=pLegItem->segmentRodSet.GetNext())
			{
				wchar_t sWMat[MAX_PATH],sWSpec[MAX_PATH];
				CXhChar16 sMat,sSpec;
				CModModel::QuerySteelMatMark(pRod->m_cMaterial,sMat);
				ANSIToUnicode(sMat,sWMat);
				if(pRod->m_ciRodType==1)
				{
					sprintf(sSpec,"L%.0fX%.0f",pRod->m_fWidth,pRod->m_fThick);	
					ANSIToUnicode(sSpec,sWSpec);
					fwprintf(fp,L"R,%d,%d,%s,%s,%f,%f,%f,%f,%f,%f\n",pRod->m_uiNodeS,pRod->m_uiNodeE,sWSpec,sWMat,
						pRod->m_vWingX.x,pRod->m_vWingX.y,pRod->m_vWingX.z,
						pRod->m_vWingY.x,pRod->m_vWingY.y,pRod->m_vWingY.z);
				}
				else if(pRod->m_ciRodType==2)
				{
					sprintf(sSpec,"φ%.0fX%.0f",pRod->m_fWidth,pRod->m_fThick);
					ANSIToUnicode(sSpec,sWSpec);
					fwprintf(fp,L"R,%d,%d,%s,%s\n",pRod->m_uiNodeS,pRod->m_uiNodeE,sWSpec,sWMat);
				}
				else
					fwprintf(fp,L"R,%d,%d\n",pRod->m_uiNodeS,pRod->m_uiNodeE);
			}
			//各减腿信息
			int sub_leg_index=1;
			for(SUB_LEG_INFO* pSubLeg=pLegItem->subLegInfoList.GetFirst();pSubLeg;pSubLeg=pLegItem->subLegInfoList.GetNext())
			{
				fwprintf(fp,L"SubLeg%d\n",sub_leg_index);
				fwprintf(fp,L"HSubLeg%d,%.0f\n",sub_leg_index,pSubLeg->m_fLegH);
				for(pNode=pSubLeg->legNodeSet.GetFirst();pNode;pNode=pSubLeg->legNodeSet.GetNext())
					fwprintf(fp,L"P,%d,%f,%f,%f\n",pNode->point_i,pNode->xOrg.x,pNode->xOrg.y,pNode->xOrg.z);
				for(pRod=pSubLeg->legRodSet.GetFirst();pRod;pRod=pSubLeg->legRodSet.GetNext())
				{
					wchar_t sWMat[MAX_PATH],sWSpec[MAX_PATH];
					CXhChar16 sMat,sSpec;
					CModModel::QuerySteelMatMark(pRod->m_cMaterial,sMat);
					ANSIToUnicode(sMat,sWMat);
					if(pRod->m_ciRodType==1)
					{
						sprintf(sSpec,"L%.0fX%.0f",pRod->m_fWidth,pRod->m_fThick);
						ANSIToUnicode(sSpec,sWSpec);
						fwprintf(fp,L"R,%d,%d,%s,%s,%f,%f,%f,%f,%f,%f\n",pRod->m_uiNodeS,pRod->m_uiNodeE,sWSpec,sWMat,
							pRod->m_vWingX.x,pRod->m_vWingX.y,pRod->m_vWingX.z,
							pRod->m_vWingY.x,pRod->m_vWingY.y,pRod->m_vWingY.z);
					}
					else if(pRod->m_ciRodType==2)
					{
						sprintf(sSpec,"φ%.0fX%.0f",pRod->m_fWidth,pRod->m_fThick);
						ANSIToUnicode(sSpec,sWSpec);
						fwprintf(fp,L"R,%d,%d,%s,%s\n",pRod->m_uiNodeS,pRod->m_uiNodeE,sWSpec,sWMat);
					}
					else
						fwprintf(fp,L"R,%d,%d\n",pRod->m_uiNodeS,pRod->m_uiNodeE);
				}
				sub_leg_index++;
			}
			leg_index++;
		}
		body_index++;
	}
	//输入挂点信息
	for(MOD_HANG_NODE* pGuaInfo=m_listGuaNode.GetFirst();pGuaInfo;pGuaInfo=m_listGuaNode.GetNext())
	{
		wchar_t sWText[MAX_PATH];
		ANSIToUnicode(pGuaInfo->m_sHangName,sWText);
		fwprintf(fp,L"G,%c,%s,%f,%f,%f\n",pGuaInfo->m_ciWireType,sWText,pGuaInfo->m_xHangPos.x,pGuaInfo->m_xHangPos.y,pGuaInfo->m_xHangPos.z);
	}
	fclose(fp);
}
void CModModel::WriteModFileByAnsi(FILE* fp)
{
	if(fp==NULL)
		return;
	CModNode* pNode=NULL;
	CModRod* pRod=NULL;
	//输出呼高信息
	fprintf(fp,"HNum,%d\n",ModHeightGroup.GetNodeNum());
	for(CModHeightGroup* pModuleItem=ModHeightGroup.GetFirst();pModuleItem;pModuleItem=ModHeightGroup.GetNext())
		fprintf(fp,"H,%.2f,Body%d,Leg%d\n",pModuleItem->m_fNamedHeight,pModuleItem->m_iBody,pModuleItem->m_iNo);
	//输出多本体多接腿信息
	int body_index=1,leg_index=1;
	for(CBodyItem* pBodyItem=m_listBodyItem.GetFirst();pBodyItem;pBodyItem=m_listBodyItem.GetNext())
	{
		//本体信息
		fprintf(fp,"Body%d\n",body_index);
		fprintf(fp,"HBody%d,%.0f\n",body_index,pBodyItem->m_fBodyH);
		for(pNode=pBodyItem->bodyNodeSet.GetFirst();pNode;pNode=pBodyItem->bodyNodeSet.GetNext())
			fprintf(fp,"P,%d,%f,%f,%f\n",pNode->point_i,pNode->xOrg.x,pNode->xOrg.y,pNode->xOrg.z);
		for(pRod=pBodyItem->bodyRodSet.GetFirst();pRod;pRod=pBodyItem->bodyRodSet.GetNext())
		{
			CXhChar16 sMat,sSpec;
			CModModel::QuerySteelMatMark(pRod->m_cMaterial,sMat);
			if(pRod->m_ciRodType==1)
			{
				sprintf(sSpec,"L%.0fX%.0f",pRod->m_fWidth,pRod->m_fThick);
				fprintf(fp,"R,%d,%d,%s,%s,%f,%f,%f,%f,%f,%f\n",pRod->m_uiNodeS,pRod->m_uiNodeE,(char*)sSpec,(char*)sMat,
					pRod->m_vWingX.x,pRod->m_vWingX.y,pRod->m_vWingX.z,
					pRod->m_vWingY.x,pRod->m_vWingY.y,pRod->m_vWingY.z);
			}
			else if(pRod->m_ciRodType==2)
			{
				sprintf(sSpec,"φ%.0fX%.0f",pRod->m_fWidth,pRod->m_fThick);
				fprintf(fp,"R,%d,%d,%s,%s\n",pRod->m_uiNodeS,pRod->m_uiNodeE,(char*)sSpec,(char*)sMat);
			}
			else
				fprintf(fp,"R,%d,%d\n",pRod->m_uiNodeS,pRod->m_uiNodeE);
		}
		//该本体下的接腿信息
		for(CLegItem* pLegItem=m_listLegItem.GetFirst();pLegItem;pLegItem=m_listLegItem.GetNext())
		{
			if(pLegItem->m_hTagNode!=pBodyItem->m_hTagNode)
				continue;
			fprintf(fp,"Leg%d\n",leg_index);
			fprintf(fp,"HLeg%d,%s,%s\n",leg_index,(char*)CXhChar16(pLegItem->m_fSegmentH),(char*)CXhChar16(pLegItem->m_fMaxLegH));
			for(pNode=pLegItem->segmentNodeSet.GetFirst();pNode;pNode=pLegItem->segmentNodeSet.GetNext())
				fprintf(fp,"P,%d,%f,%f,%f\n",pNode->point_i,pNode->xOrg.x,pNode->xOrg.y,pNode->xOrg.z);
			for(pRod=pLegItem->segmentRodSet.GetFirst();pRod;pRod=pLegItem->segmentRodSet.GetNext())
			{
				CXhChar16 sMat,sSpec;
				CModModel::QuerySteelMatMark(pRod->m_cMaterial,sMat);
				if(pRod->m_ciRodType==1)
				{
					sprintf(sSpec,"L%.0fX%.0f",pRod->m_fWidth,pRod->m_fWidth);
					fprintf(fp,"R,%d,%d,%s,%s,%3f,%f,%f,%f,%f,%f\n",pRod->m_uiNodeS,pRod->m_uiNodeE,(char*)sSpec,(char*)sMat,
						pRod->m_vWingX.x,pRod->m_vWingX.y,pRod->m_vWingX.z,
						pRod->m_vWingY.x,pRod->m_vWingY.y,pRod->m_vWingY.z);
				}
				else if(pRod->m_ciRodType==2)
				{
					sprintf(sSpec,"φ%.0fX%.0f",pRod->m_fWidth,pRod->m_fWidth);
					fprintf(fp,"R,%d,%d,%s,%s\n",pRod->m_uiNodeS,pRod->m_uiNodeE,(char*)sSpec,(char*)sMat);
				}
				else
					fprintf(fp,"R,%d,%d\n",pRod->m_uiNodeS,pRod->m_uiNodeE);
			}
			//各减腿信息
			int sub_leg_index=1;
			for(SUB_LEG_INFO* pSubLeg=pLegItem->subLegInfoList.GetFirst();pSubLeg;pSubLeg=pLegItem->subLegInfoList.GetNext())
			{
				fprintf(fp,"SubLeg%d\n",sub_leg_index);
				fprintf(fp,"HSubLeg%d,%s\n",sub_leg_index,(char*)CXhChar16(pSubLeg->m_fLegH));
				for(pNode=pSubLeg->legNodeSet.GetFirst();pNode;pNode=pSubLeg->legNodeSet.GetNext())
					fprintf(fp,"P,%d,%f,%f,%f\n",pNode->point_i,pNode->xOrg.x,pNode->xOrg.y,pNode->xOrg.z);
				for(pRod=pSubLeg->legRodSet.GetFirst();pRod;pRod=pSubLeg->legRodSet.GetNext())
				{
					CXhChar16 sMat,sSpec;
					CModModel::QuerySteelMatMark(pRod->m_cMaterial,sMat);
					if(pRod->m_ciRodType==1)
					{
						sprintf(sSpec,"L%.0fX%.0f",pRod->m_fWidth,pRod->m_fThick);
						fprintf(fp,"R,%d,%d,%s,%s,%3f,%f,%f,%f,%f,%f\n",pRod->m_uiNodeS,pRod->m_uiNodeE,(char*)sSpec,(char*)sMat,
							pRod->m_vWingX.x,pRod->m_vWingX.y,pRod->m_vWingX.z,
							pRod->m_vWingY.x,pRod->m_vWingY.y,pRod->m_vWingY.z);
					}
					else if(pRod->m_ciRodType==2)
					{
						sprintf(sSpec,"φ%.0fX%.0f",pRod->m_fWidth,pRod->m_fThick);
						fprintf(fp,"R,%d,%d,%s,%s\n",pRod->m_uiNodeS,pRod->m_uiNodeE,(char*)sSpec,(char*)sMat);
					}
					else
						fprintf(fp,"R,%d,%d\n",pRod->m_uiNodeS,pRod->m_uiNodeE);
				}
				sub_leg_index++;
			}
			leg_index++;
		}
		body_index++;
	}
	//输入挂点信息
	for(MOD_HANG_NODE* pGuaInfo=m_listGuaNode.GetFirst();pGuaInfo;pGuaInfo=m_listGuaNode.GetNext())
		fprintf(fp,"G,%c,%s,%f,%f,%f\n",pGuaInfo->m_ciWireType,pGuaInfo->m_sHangName,pGuaInfo->m_xHangPos.x,pGuaInfo->m_xHangPos.y,pGuaInfo->m_xHangPos.z);
	fclose(fp);
}
//
BOOL CModModel::IsUTF8File(const char* mod_file)
{
	FILE *fp = fopen(mod_file,"rt");
	if(fp==NULL)
		return FALSE;
	fseek(fp, 0, SEEK_END);
	long lSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	CBuffer buffer;
	buffer.Write(NULL,lSize);
	fread(buffer.GetBufferPtr(),buffer.GetLength(),1,fp);
	fclose(fp);
	//
	unsigned char* start = (unsigned char*)buffer.GetBufferPtr();
	unsigned char* end = (unsigned char*)start + lSize;
	BOOL bIsUTF8 = TRUE;
	while(start<end)
	{
		if(*start<0x80)
		{	// (10000000): 值小于0x80的为ASCII字符  	
			start++; 
		}
		else if (*start < (0xC0)) 
		{	// (11000000): 值介于0x80与0xC0之间的为无效UTF-8字符 
			start++; 
		}
		else if (*start < (0xE0)) 
		{	// (11100000): 此范围内为2字节UTF-8字符  
			if (start >= end - 1)
				break;
			if ((start[1] & (0xC0)) != 0x80)
			{
				bIsUTF8 = FALSE;
				break;
			}
			start += 2;
		}
		else if (*start < (0xF0)) 
		{	// (11110000): 此范围内为3字节UTF-8字符  
			if (start >= end - 2)
				break;
			if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80)
			{
				bIsUTF8 = FALSE;
				break;
			}
			start += 3;
		}
		else
		{
			bIsUTF8 = FALSE;
			break;
		}
	}
	return bIsUTF8;
}
//////////////////////////////////////////////////////////////////////////
//
static CHashPtrList<CModModel>localHashModModels;
class CModModelsLife{
public:
	~CModModelsLife(){localHashModModels.Empty();}
};
CModModelsLife modelsLife;

CModModel* CModModelFactory::CreateModModel()
{
	int iNo=1;
	do{
		if(localHashModModels.GetValue(iNo)!=NULL)
			iNo++;
		else	//找到一个空号
			break;
	}while(true);
	CModModel* pModel = localHashModModels.Add(iNo);
	return pModel;
};
CModModel* CModModelFactory::ModModelFromSerial(long serial)
{
	return localHashModModels.GetValue(serial);
}
bool CModModelFactory::Destroy(long serial)
{
	for(CModModel *pModModel=localHashModModels.GetFirst();pModModel;pModModel=localHashModModels.GetNext())
	{
		if(pModModel->GetSerialId()==serial)
			return localHashModModels.DeleteCursor(TRUE)==TRUE;
	}
	return false;
}