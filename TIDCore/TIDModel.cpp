#include "StdAfx.h"
#include "tidmodel.h"
#include "SegI.h"
#include "list.h"
#include "ArrayList.h"
#include "LogFile.h"
#include "WirePlaceCode.h"

#if defined(_DEBUG)&&!defined(_DISABLE_DEBUG_NEW_)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//////////////////////////////////////////////////////////////////////////
// ȫ�ֹ��ߺ���
#ifndef _DISABLE_MOD_CORE_
#include "ModCore.h"
static GECS TransToUcs(MOD_CS modCs)
{
	GECS cs;
	cs.origin.Set(modCs.origin.x,modCs.origin.y,modCs.origin.z);
	cs.axis_x.Set(modCs.axisX.x,modCs.axisX.y,modCs.axisX.z);
	cs.axis_y.Set(modCs.axisY.x,modCs.axisY.y,modCs.axisY.z);
	cs.axis_z.Set(modCs.axisZ.x,modCs.axisZ.y,modCs.axisZ.z);
	return cs;
}
#endif
static TID_CS ConvertCSFrom(const GECS& ucs)
{
	TID_CS cs;
	cs.origin.Set(ucs.origin.x, ucs.origin.y, ucs.origin.z);
	cs.axisX.Set(ucs.axis_x.x, ucs.axis_x.y, ucs.axis_x.z);
	cs.axisY.Set(ucs.axis_y.x, ucs.axis_y.y, ucs.axis_y.z);
	cs.axisZ.Set(ucs.axis_z.x, ucs.axis_z.y, ucs.axis_z.z);
	return cs;
}
static UCS_STRU ConvertUCSFrom(const TID_CS& cs)
{
	UCS_STRU ucs;
	ucs.origin.Set(cs.origin.x, cs.origin.y, cs.origin.z);
	ucs.axis_x.Set(cs.axisX.x, cs.axisX.y, cs.axisX.z);
	ucs.axis_y.Set(cs.axisY.x, cs.axisY.y, cs.axisY.z);
	ucs.axis_z.Set(cs.axisZ.x, cs.axisZ.y, cs.axisZ.z);
	return ucs;
}
static bool IsIncludeObject(CFGWORD moduleCfgWord, BYTE arrQuadLegNo[4], CFGWORD objCfgWord, BYTE cLegQuad)
{
	bool validpart = false;
	if (cLegQuad == 0 || cLegQuad > 4)
	{	//cLegQuad>4ʱ��ʾ���ݲ��Ϸ����ݰ������� wjh-2016.2.16
		if (objCfgWord.And(moduleCfgWord))
			validpart = true;
	}
	else if (cLegQuad <= 4)
	{
		if (objCfgWord.IsHasNo(arrQuadLegNo[cLegQuad - 1]))
			validpart = true;
	}
	return validpart;
}
static bool IsIncludeObject(CFGWORD moduleCfgWord, CFGWORD objCfgWord, BYTE cLegQuad)
{
	bool validpart = false;
	if (cLegQuad == 0 || cLegQuad > 4)
	{	//cLegQuad>4ʱ��ʾ���ݲ��Ϸ����ݰ�������
		if (objCfgWord.And(moduleCfgWord))
			validpart = true;
	}
	else
	{	//1~4ʱ�������ȴ���
		if (moduleCfgWord.And(objCfgWord))
			validpart = true;
	}
	return validpart;
}
//////////////////////////////////////////////////////////////////////////
// CTidModel
CTidModel::CTidModel(long serial/*=0*/)
{
	m_iSerial=serial;
	m_dwAssmblyIter=1;
	m_dwBoltCount=m_dwPartCount=m_dwBlockCount=0;
}
CTidModel::~CTidModel()
{
}
bool CTidModel::ReadTidFile(const char* file_path)
{
	FILE* fp=fopen(file_path,"rb");
	if(fp==NULL)
		return false;
	fseek(fp,0,SEEK_END);
	DWORD file_len=ftell(fp);
	fseek(fp,0,SEEK_SET);
	DYN_ARRAY<char> pool(file_len);
	fread(pool,1,file_len,fp);
	fclose(fp);
	InitTidBuffer(pool,file_len);
	return true;
}
bool CTidModel::InitTidBuffer(const char* src_buf,long buf_len)
{
	if(!m_xTidBuffer.InitBuffer(src_buf,buf_len))
		return false;
	m_xModuleSection=m_xTidBuffer.ModuleSection();
	m_xFoundationSection = m_xTidBuffer.SubLegFoundationSection();
	m_xWireNodeSection = m_xTidBuffer.WireNodeSection();
	m_xProjectInfoSection = m_xTidBuffer.ProjectInfoSection();
	m_xPartSection=m_xTidBuffer.PartSection();
	m_xBoltLib.sectdata=m_xTidBuffer.BoltSection();
	m_xBlockSection=m_xTidBuffer.BlockSection();
	m_xAssembleSection=m_xTidBuffer.AssembleSection();
	m_xNodeSection=m_xAssembleSection.NodeSection();
	m_xAssemblyParts=m_xAssembleSection.PartSection();
	m_xAssemblyBolts=m_xAssembleSection.BoltSection();
	m_dwPartCount=m_xAssemblyParts.AssemblyCount();
	m_dwBoltCount=m_xAssemblyBolts.AssemblyCount();
	m_dwBlockCount=m_xBlockSection.GetBlockCount();
	//
	m_xBoltLib.hashBoltSeries.Empty();
	hashHeightGroup.Empty();
	hashHangPoint.Empty();
	hashTidNode.Empty();
	hashAssemblePart.Empty();
	hashAssembleBolt.Empty();
	hashAssembleAnchorBolt.Empty();
	InitTidDataInfo();
	return true;
}
void CTidModel::InitTidDataInfo()
{
	UINT uCount = 0, iHuGaoKey = 0, iPartKey = 0, iBoltKey = 0, iNodeKey = 0;
	//��ȡ������Ϣ
	uCount = m_xModuleSection.GetModuleCount();
	for (UINT i = 0; i < uCount; i++)
	{
		CTidHeightGroup* pHeightGroup = hashHeightGroup.Add(++iHuGaoKey);
		pHeightGroup->m_pModel = this;
		pHeightGroup->module = m_xModuleSection.GetModuleAt(i);
	}
	//�ڵ���Ϣ
	CTidNode* pTidNode = NULL;
	uCount = m_xNodeSection.NodeCount();
	for (UINT i = 0; i < uCount; i++)
	{
		NODE_ASSEMBLY node;
		m_xNodeSection.GetNodeByIndexId(i + 1, true, node);
		//
		pTidNode = hashTidNode.Add(++iNodeKey);
		pTidNode->SetBelongModel(this);
		pTidNode->node = node;
	}
	uCount = m_xNodeSection.NodeCount(false);
	for (UINT i = 0; i < uCount; i++)
	{
		NODE_ASSEMBLY node;
		m_xNodeSection.GetNodeByIndexId(i + 1, false, node);
		UINT uBlkRef = m_xAssembleSection.BlockAssemblyCount();
		for (UINT indexId = 1; indexId <= uBlkRef; indexId++)
		{
			BLOCK_ASSEMBLY blkassembly = m_xAssembleSection.GetAssemblyByIndexId(indexId);
			if (blkassembly.wIndexId != node.wBlockIndexId)
				continue;	//�ù��������ڵ�ǰ����װ�����
			TOWER_BLOCK block = m_xBlockSection.GetBlockAt(blkassembly.wIndexId - 1);
			node.xPosition = block.lcs.TransPToCS(node.xPosition);
			node.xPosition = blkassembly.acs.TransPFromCS(node.xPosition);
			//
			pTidNode = hashTidNode.Add(++iNodeKey);
			pTidNode->SetBelongModel(this);
			pTidNode->node = node;
		}
	}
	//��ȡ����װ�乹��
	CTidAssemblePart* pAssmPart = NULL;
	uCount = m_xAssemblyParts.AssemblyCount();
	for (UINT i = 0; i < uCount; i++)
	{
		PART_ASSEMBLY assemble;
		m_xAssemblyParts.GetAssemblyByIndexId(i + 1, true, assemble);
		PART_INFO partinfo = m_xPartSection.GetPartInfoByIndexId(assemble.dwIndexId);
		//
		pAssmPart = hashAssemblePart.Add(++iPartKey);
		pAssmPart->SetBelongModel(this);
		pAssmPart->part = assemble;
		pAssmPart->solid.CopySolidBuffer(partinfo.solid.BufferPtr(), partinfo.solid.BufferLength());
		pAssmPart->solid.TransToACS(ConvertCSFrom(assemble.acs));
	}
	//��ȡ����װ�乹��
	uCount = m_xAssemblyParts.AssemblyCount(false);
	for (UINT i = 0; i < uCount; i++)
	{
		PART_ASSEMBLY assemble;
		m_xAssemblyParts.GetAssemblyByIndexId(i + 1, false, assemble);
		PART_INFO partinfo = m_xPartSection.GetPartInfoByIndexId(assemble.dwIndexId);
		UINT uBlkRef = m_xAssembleSection.BlockAssemblyCount();
		for (UINT indexId = 1; indexId <= uBlkRef; indexId++)
		{
			BLOCK_ASSEMBLY blkassembly = m_xAssembleSection.GetAssemblyByIndexId(indexId);
			if (blkassembly.wIndexId != assemble.wBlockIndexId)
				continue;	//�ù��������ڵ�ǰ����װ�����
			TOWER_BLOCK block = m_xBlockSection.GetBlockAt(blkassembly.wIndexId - 1);
			//
			pAssmPart = hashAssemblePart.Add(++iPartKey);
			pAssmPart->SetBelongModel(this);
			pAssmPart->part = assemble;
			pAssmPart->solid.CopySolidBuffer(partinfo.solid.BufferPtr(), partinfo.solid.BufferLength());
			pAssmPart->solid.TransToACS(ConvertCSFrom(assemble.acs));
			pAssmPart->solid.TransACS(ConvertCSFrom(block.lcs), ConvertCSFrom(blkassembly.acs));
		}
	}
	//��ȡ����װ����˨
	IBoltSeriesLib* pBoltLibrary = GetBoltLib();
	uCount = m_xAssemblyBolts.AssemblyCount();
	for (UINT i = 0; i < uCount; i++)
	{
		BOLT_ASSEMBLY assemble;
		assemble = m_xAssemblyBolts.GetAssemblyByIndexId(i + 1, true);
		IBoltSeries* pBoltSeries = pBoltLibrary->GetBoltSeriesAt(assemble.cSeriesId - 1);
		IBoltSizeSpec* pBoltSize = pBoltSeries->GetBoltSizeSpecById(assemble.wIndexId);
		if (pBoltSize == NULL)
		{
			logerr.Log("BoltSizeSpec@%d not found!\n", assemble.wIndexId);
			continue;
		}
		CTidSolidBody* pBoltSolid = (CTidSolidBody*)pBoltSize->GetBoltSolid();
		CTidSolidBody* pNutSolid = (CTidSolidBody*)pBoltSize->GetNutSolid();
		if (pBoltSolid == NULL || pNutSolid == NULL)
		{
			logerr.Log("BoltSolid@%d not found!\n", assemble.wIndexId);
			continue;
		}
		//
		CTidAssembleBolt* pAssmBolt = hashAssembleBolt.Add(++iBoltKey);
		pAssmBolt->SetBelongModel(this);
		pAssmBolt->bolt = assemble;
		pAssmBolt->solid.bolt.CopySolidBuffer(pBoltSolid->SolidBufferPtr(), pBoltSolid->SolidBufferLength());
		pAssmBolt->solid.nut.CopySolidBuffer(pNutSolid->SolidBufferPtr(), pNutSolid->SolidBufferLength());
		TID_CS bolt_acs, nut_acs;
		nut_acs = bolt_acs = pAssmBolt->GetAcs();
		nut_acs.origin.x += bolt_acs.axisZ.x*assemble.wL0;
		nut_acs.origin.y += bolt_acs.axisZ.y*assemble.wL0;
		nut_acs.origin.z += bolt_acs.axisZ.z*assemble.wL0;
		pAssmBolt->solid.bolt.TransToACS(bolt_acs);
		pAssmBolt->solid.nut.TransToACS(nut_acs);
	}
	//��ȡ����װ����˨
	uCount = m_xAssemblyBolts.AssemblyCount(false);
	for (UINT i = 0; i < uCount; i++)
	{
		BOLT_ASSEMBLY assemble = m_xAssemblyBolts.GetAssemblyByIndexId(i + 1, false);
		IBoltSeries* pBoltSeries = pBoltLibrary->GetBoltSeriesAt(assemble.cSeriesId - 1);
		IBoltSizeSpec* pBoltSize = pBoltSeries->GetBoltSizeSpecById(assemble.wIndexId);
		if (pBoltSize == NULL)
		{
			logerr.Log("BoltSizeSpec@%d not found!\n", assemble.wIndexId);
			continue;
		}
		CTidSolidBody* pBoltSolid = (CTidSolidBody*)pBoltSize->GetBoltSolid();
		CTidSolidBody* pNutSolid = (CTidSolidBody*)pBoltSize->GetNutSolid();
		if (pBoltSolid == NULL || pNutSolid == NULL)
		{
			logerr.Log("BoltSolid@%d not found!\n", assemble.wIndexId);
			continue;
		}
		UINT uBlkRef = m_xAssembleSection.BlockAssemblyCount();
		for (UINT indexId = 1; indexId <= uBlkRef; indexId++)
		{
			BLOCK_ASSEMBLY blkassembly = m_xAssembleSection.GetAssemblyByIndexId(indexId);
			if (blkassembly.wIndexId != assemble.wBlockIndexId)
				continue;	//�ù��������ڵ�ǰ����װ�����
			TOWER_BLOCK block = m_xBlockSection.GetBlockAt(blkassembly.wIndexId - 1);
			//
			CTidAssembleBolt* pAssmBolt = hashAssembleBolt.Add(++iBoltKey);
			pAssmBolt->SetBelongModel(this);
			pAssmBolt->bolt = assemble;
			pAssmBolt->solid.bolt.CopySolidBuffer(pBoltSolid->SolidBufferPtr(), pBoltSolid->SolidBufferLength());
			pAssmBolt->solid.nut.CopySolidBuffer(pNutSolid->SolidBufferPtr(), pNutSolid->SolidBufferLength());
			TID_CS bolt_acs, nut_acs;
			nut_acs = bolt_acs = pAssmBolt->GetAcs();
			nut_acs.origin.x += bolt_acs.axisZ.x*assemble.wL0;
			nut_acs.origin.y += bolt_acs.axisZ.y*assemble.wL0;
			nut_acs.origin.z += bolt_acs.axisZ.z*assemble.wL0;
			pAssmBolt->solid.bolt.TransToACS(bolt_acs);
			pAssmBolt->solid.nut.TransToACS(nut_acs);
			pAssmBolt->solid.bolt.TransACS(ConvertCSFrom(block.lcs), ConvertCSFrom(blkassembly.acs));
			pAssmBolt->solid.nut.TransACS(ConvertCSFrom(block.lcs), ConvertCSFrom(blkassembly.acs));
		}
	}
	//����������Ϣ�ڴ���
	for (int i = 0; i < m_xProjectInfoSection.m_ciSubProjSectionCount; i++)
	{
		CProjInfoSubSection xSubSection= m_xProjectInfoSection.GetSubSectionAt(i);
		CBuffer sectbuf(xSubSection.BufferPtr(), xSubSection.BufferLength());
		if (xSubSection.m_uidSubSection == 23545686)
		{	//GIM����������Ϣ
			sectbuf.Read(m_xGimFileHeadInfo.m_sFileTag, 16);
			sectbuf.Read(m_xGimFileHeadInfo.m_sFileName, 256);
			sectbuf.Read(m_xGimFileHeadInfo.m_sDesigner, 64);
			sectbuf.Read(m_xGimFileHeadInfo.m_sUnit, 256);
			sectbuf.Read(m_xGimFileHeadInfo.m_sSoftName, 128);
			sectbuf.Read(m_xGimFileHeadInfo.m_sTime, 16);
			sectbuf.Read(m_xGimFileHeadInfo.m_sSoftMajorVer, 8);
			sectbuf.Read(m_xGimFileHeadInfo.m_sSoftMinorVer, 8);
			sectbuf.Read(m_xGimFileHeadInfo.m_sMajorVersion, 8);
			sectbuf.Read(m_xGimFileHeadInfo.m_sMinorVersion, 8);
			sectbuf.Read(m_xGimFileHeadInfo.m_sBufSize, 8);
			//
			sectbuf.ReadInteger(&m_xGimPropertyInfo.m_nCircuit);
			sectbuf.ReadDouble(&m_xGimPropertyInfo.m_fWindSpeed);
			sectbuf.ReadDouble(&m_xGimPropertyInfo.m_fNiceThick);
			sectbuf.ReadDouble(&m_xGimPropertyInfo.m_fFrontRulingSpan);
			sectbuf.ReadDouble(&m_xGimPropertyInfo.m_fBackRulingSpan);
			sectbuf.ReadDouble(&m_xGimPropertyInfo.m_fMaxSpan);
			sectbuf.ReadDouble(&m_xGimPropertyInfo.m_fDesignKV);
			sectbuf.ReadDouble(&m_xGimPropertyInfo.m_fFrequencyRockAngle);
			sectbuf.ReadDouble(&m_xGimPropertyInfo.m_fLightningRockAngle);
			sectbuf.ReadDouble(&m_xGimPropertyInfo.m_fSwitchingRockAngle);
			sectbuf.ReadDouble(&m_xGimPropertyInfo.m_fWorkingRockAngle);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sVoltGrade);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sType);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sTexture);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sFixedType);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sTaType);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sCWireSpec);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sEWireSpec);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sWindSpan);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sWeightSpan);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sAngleRange);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sRatedHeight);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sHeightRange);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sTowerWeight);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sManuFacturer);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sMaterialCode);
			sectbuf.ReadString(m_xGimPropertyInfo.m_sProModelCode);
		}
		if (xSubSection.m_uidSubSection == 11111111)
		{	//�ҵ���Ϣ
			WORD nCount = 0;
			sectbuf.ReadWord(&nCount);
			for (int i = 0; i < nCount; i++)
			{
				CTidHangPoint* pHangPoint = hashHangPoint.Add(i + 1);
				sectbuf.SeekPosition(2 + i * 112);
				sectbuf.ReadWord(&pHangPoint->wireNode.wiCode);
				sectbuf.ReadPoint(pHangPoint->wireNode.position);
				sectbuf.ReadPoint(pHangPoint->wireNode.relaHolePt[0]);
				sectbuf.ReadPoint(pHangPoint->wireNode.relaHolePt[1]);
				sectbuf.Read(pHangPoint->wireNode.name, 38);
			}
		}
	}
}
int CTidModel::GetTowerTypeName(char* towerTypeName,UINT maxBufLength/*=0*/)
{
	StrCopy(towerTypeName,m_xTidBuffer.GetTowerType(),maxBufLength);
	return strlen(towerTypeName);
}
TID_CS CTidModel::ModelCoordSystem()
{
	GECS cs=m_xTidBuffer.ModelCoordSystem();
	TID_CS tidCS;
	tidCS.origin=cs.origin;
	tidCS.axisX=cs.axis_x;
	tidCS.axisY=cs.axis_y;
	tidCS.axisZ=cs.axis_z;
	return tidCS;
}
double CTidModel::GetNamedHeightZeroZ()
{
	return m_xModuleSection.m_fNamedHeightZeroZ;
}
//�����ؽ���˨��Ϣ
bool CTidModel::GetAnchorBoltSolid(CTidSolidBody* pBoltSolid,CTidSolidBody* pNutSolid)
{
	if(compareVersion(m_xTidBuffer.Version(),"1.4")>=0)
	{
		pBoltSolid->CopySolidBuffer(m_xBoltLib.sectdata.xAnchorSolid.bolt.BufferPtr(),m_xBoltLib.sectdata.xAnchorSolid.bolt.BufferLength());
		pNutSolid->CopySolidBuffer(m_xBoltLib.sectdata.xAnchorSolid.nut.BufferPtr(),m_xBoltLib.sectdata.xAnchorSolid.nut.BufferLength());
		return true;
	}
	else
		return false;
}
WORD CTidModel::GetBasePlateThick()
{
	return m_xFoundationSection.ciBasePlateThick;
}
WORD CTidModel::GetAnchorCount()
{
	return m_xFoundationSection.cnAnchorBoltCount;
}
bool CTidModel::GetAnchorAt(short index,short* psiPosX,short* psiPosY)
{
	if(index<0||index>=m_xFoundationSection.cnAnchorBoltCount)
		return false;
	ANCHOR_LOCATION location=m_xFoundationSection.GetAnchorLocationAt((short)index);
	if(psiPosX)
		*psiPosX=location.siPosX;
	if(psiPosY)
		*psiPosY=location.siPosY;
	return true;
}
WORD CTidModel::GetSubLegCount(BYTE ciBodySerial)
{
	return m_xFoundationSection.cnSubLegCount;
}
//��ȡָ��������ĳ���ȵĻ�������
bool CTidModel::GetSubLegBaseLocation(BYTE ciBodySerial,BYTE ciLegSerial,double* pos3d)
{
	GEPOINT pos=m_xFoundationSection.GetSubLegFoundationOrgBySerial(ciBodySerial,ciLegSerial);
	if(pos3d!=NULL)
		memcpy(pos3d,(double*)pos,24);
	return true;
}
//��ȡָ��������ĳ���ȵĻ�������
double CTidModel::GetSubLegBaseWidth(BYTE ciBodySerial, BYTE ciLegSerial)
{
	GEPOINT pos = m_xFoundationSection.GetSubLegFoundationOrgBySerial(ciBodySerial, ciLegSerial);
	double fHalfFrontBaseWidth = pos.x;
	return fHalfFrontBaseWidth * 2;
}
ISteelMaterialLibrary* CTidModel::GetSteelMatLib()
{
	m_xSteelMatLib.matsect=m_xPartSection.GetMatLibrarySection();
	return &m_xSteelMatLib;
}
ITidPartsLib* CTidModel::GetTidPartsLib()
{
	m_xPartLib.sectdata=m_xTidBuffer.PartSection();
	return &m_xPartLib;
}
IBoltSeriesLib* CTidModel::GetBoltLib()
{
	m_xBoltLib.sectdata=m_xTidBuffer.BoltSection();
	return &m_xBoltLib;
}
ITidHeightGroup* CTidModel::GetHeightGroupAt(short i)
{
	if(i<0||i>=m_xModuleSection.GetModuleCount())
		return NULL;
	CTidHeightGroup* pHeightGroup=hashHeightGroup.GetValue(i+1);
	if(pHeightGroup==NULL)
	{
		pHeightGroup=hashHeightGroup.Add(i+1);
		pHeightGroup->module=m_xModuleSection.GetModuleAt(i);
	}
	pHeightGroup->m_pModel=this;
	return pHeightGroup;
}
ITidHangPoint* CTidModel::GetHangPointAt(DWORD i)
{
	if(i<0||i>m_xWireNodeSection.m_wnWireNodeCount)
		return NULL;
	CTidHangPoint* pHangPoint=hashHangPoint.GetValue(i+1);
	if(pHangPoint==NULL)
	{
		pHangPoint=hashHangPoint.Add(i+1);
		pHangPoint->wireNode=m_xWireNodeSection.GetWireNodeAt(i);
	}
	pHangPoint->m_pModel=this;
	return pHangPoint;
}
ITidNode* CTidModel::EnumTidNodeFirst(long hHeightSerial /*= 0*/)
{
	ITidNode* pTidNode = hashTidNode.GetFirst();
	if (hHeightSerial == 0)
		return pTidNode;
	else
	{
		CTidHeightGroup* pHuGao = (CTidHeightGroup*)GetHeightGroup(hHeightSerial);
		if (pHuGao == NULL)
		{
			logerr.Log("%d���߲�����!", hHeightSerial);
			return NULL;
		}
		while (pTidNode != NULL)
		{
			CFGWORD node_cfg = ((CTidNode*)pTidNode)->node.cfgword;
			if (IsIncludeObject(pHuGao->module.m_dwLegCfgWord, node_cfg, pTidNode->GetLegQuad()))
				return pTidNode;
			else
				pTidNode = hashTidNode.GetNext();
		}
		return NULL;
	}
}
ITidNode* CTidModel::EnumTidNodeNext(long hHeightSerial /*= 0*/)
{
	ITidNode* pTidNode = hashTidNode.GetNext();
	if (hHeightSerial == 0)
		return pTidNode;
	else
	{
		CTidHeightGroup* pHuGao = (CTidHeightGroup*)GetHeightGroup(hHeightSerial);
		if (pHuGao == NULL)
		{
			logerr.Log("%d���߲�����!", hHeightSerial);
			return NULL;
		}
		while (pTidNode != NULL)
		{
			CFGWORD node_cfg = ((CTidNode*)pTidNode)->node.cfgword;
			if (IsIncludeObject(pHuGao->module.m_dwLegCfgWord, node_cfg, pTidNode->GetLegQuad()))
				return pTidNode;
			else
				pTidNode = hashTidNode.GetNext();
		}
		return NULL;
	}
}
ITidAssemblePart* CTidModel::EnumAssemblePartFirst(long hHeightSerial /*= 0*/)
{
	ITidAssemblePart* pAssemPart = hashAssemblePart.GetFirst();
	if (hHeightSerial == 0)
		return pAssemPart;
	else
	{
		CTidHeightGroup* pHuGao = (CTidHeightGroup*)GetHeightGroup(hHeightSerial);
		if (pHuGao == NULL)
		{
			logerr.Log("%d���߲�����!",hHeightSerial);
			return NULL;
		}
		while (pAssemPart != NULL)
		{
			CFGWORD part_cfg = ((CTidAssemblePart*)pAssemPart)->part.cfgword;
			if (IsIncludeObject(pHuGao->module.m_dwLegCfgWord, part_cfg,pAssemPart->GetLegQuad()))
				return pAssemPart;
			else
				pAssemPart = hashAssemblePart.GetNext();
		}
		return NULL;
	}
}
ITidAssemblePart* CTidModel::EnumAssemblePartNext(long hHeightSerial /*= 0*/)
{
	ITidAssemblePart* pAssemPart = hashAssemblePart.GetNext();
	if (hHeightSerial == 0)
		return pAssemPart;
	else
	{
		CTidHeightGroup* pHuGao = (CTidHeightGroup*)GetHeightGroup(hHeightSerial);
		if (pHuGao == NULL)
		{
			logerr.Log("%d���߲�����!", hHeightSerial);
			return NULL;
		}
		while (pAssemPart != NULL)
		{
			CFGWORD part_cfg = ((CTidAssemblePart*)pAssemPart)->part.cfgword;
			if (IsIncludeObject(pHuGao->module.m_dwLegCfgWord, part_cfg,pAssemPart->GetLegQuad()))
				return pAssemPart;
			else
				pAssemPart = hashAssemblePart.GetNext();
		}
		return NULL;
	}
}
ITidAssembleBolt* CTidModel::EnumAssembleBoltFirst(long hHeightSerial /*= 0*/)
{
	ITidAssembleBolt* pAssemBolt = hashAssembleBolt.GetFirst();
	if (hHeightSerial == 0)
		return pAssemBolt;
	else
	{
		CTidHeightGroup* pHuGao = (CTidHeightGroup*)GetHeightGroup(hHeightSerial);
		if (pHuGao)
		{
			logerr.Log("%d���߲�����!", hHeightSerial);
			return NULL;
		}
		while (pAssemBolt != NULL)
		{
			CFGWORD bolt_cfg = ((CTidAssembleBolt*)pAssemBolt)->bolt.cfgword;
			if (IsIncludeObject(pHuGao->module.m_dwLegCfgWord, bolt_cfg,pAssemBolt->GetLegQuad()))
				return pAssemBolt;
			else
				pAssemBolt = hashAssembleBolt.GetNext();
		}
		return NULL;
	}
}
ITidAssembleBolt* CTidModel::EnumAssembleBoltNext(long hHeightSerial /*= 0*/)
{
	ITidAssembleBolt* pAssemBolt = hashAssembleBolt.GetNext();
	if (hHeightSerial == 0)
		return pAssemBolt;
	else
	{
		CTidHeightGroup* pHuGao = (CTidHeightGroup*)GetHeightGroup(hHeightSerial);
		if (pHuGao)
		{
			logerr.Log("%d���߲�����!", hHeightSerial);
			return NULL;
		}
		while (pAssemBolt != NULL)
		{
			CFGWORD bolt_cfg = ((CTidAssembleBolt*)pAssemBolt)->bolt.cfgword;
			if (IsIncludeObject(pHuGao->module.m_dwLegCfgWord, bolt_cfg,pAssemBolt->GetLegQuad()))
				return pAssemBolt;
			else
				pAssemBolt = hashAssembleBolt.GetNext();
		}
		return NULL;
	}
}
bool CTidModel::ExportModFile(const char* sFileName)
{
#ifdef _DISABLE_MOD_CORE_
	return false;
#else
	IModModel* pModModel=CModModelFactory::CreateModModel();
	//��ȡMOD������Ϣ������MOD����ϵ
	double fTowerHeight = 0;
	for(int i=0;i<HeightGroupCount();i++)
	{
		CTidHeightGroup* pModule=(CTidHeightGroup*)GetHeightGroupAt(i);
		double fModuleMaxZ = pModule->GetLowestZ();
		if (fTowerHeight < fModuleMaxZ)
			fTowerHeight = fModuleMaxZ;
		//
		IModHeightGroup* pHeightGroup=pModModel->AppendHeightGroup(pModule->GetSerialId());
		pHeightGroup->SetBelongModel(pModModel);
		pHeightGroup->SetLowestZ(fModuleMaxZ);
		pHeightGroup->SetLegCfg(pModule->module.m_dwLegCfgWord.flag.bytes);
		pHeightGroup->SetNameHeight(pModule->GetNamedHeight());
	}
	pModModel->SetNamedHeightZeroZ(GetNamedHeightZeroZ());
	pModModel->SetTowerHeight(fTowerHeight);
	GECS ucs = TransToUcs(pModModel->BuildUcsByModCS());
	//��ȡ�ڵ���Ϣ
	UINT uCount = m_xNodeSection.NodeCount();
	for(UINT i=0;i<uCount;i++)
	{
		NODE_ASSEMBLY node;
		m_xNodeSection.GetNodeByIndexId(i+1,true,node);
		GEPOINT org_pt=ucs.TransPFromCS(node.xPosition);
		//
		IModNode* pModNode=pModModel->AppendNode(i+1);
		pModNode->SetBelongModel(pModModel);
		pModNode->SetCfgword(node.cfgword.flag.bytes);
		pModNode->SetLdsOrg(MOD_POINT(node.xPosition));
		pModNode->SetOrg(MOD_POINT(org_pt));
		if(node.cLegQuad>=1&&node.cLegQuad<=4)
			pModNode->SetLayer('L');	//�Ȳ�Leg
		else
			pModNode->SetLayer('B');	//����Body
	}
	//��ȡ�˼���Ϣ
	uCount=m_xAssemblyParts.AssemblyCount();
	for(UINT i=0;i<uCount;i++)
	{
		PART_ASSEMBLY assemble;
		m_xAssemblyParts.GetAssemblyByIndexId(i+1,true,assemble);
		PART_INFO partinfo=m_xPartSection.GetPartInfoByIndexId(assemble.dwIndexId);
		if(!assemble.bIsRod)
			continue;
		IModNode* pModNodeS=pModModel->FindNode(assemble.uiStartPointI);
		IModNode* pModNodeE=pModModel->FindNode(assemble.uiEndPointI);
		if(pModNodeS==NULL || pModNodeE==NULL)
			continue;	//�̽Ǹ�
		IModRod* pModRod=pModModel->AppendRod(i+1);
		pModRod->SetBelongModel(pModModel);
		pModRod->SetNodeS(pModNodeS);
		pModRod->SetNodeE(pModNodeE);
		pModRod->SetCfgword(assemble.cfgword.flag.bytes);
		pModRod->SetMaterial(partinfo.cMaterial);
		pModRod->SetWidth(partinfo.fWidth);
		pModRod->SetThick(partinfo.fThick);
		if(assemble.cLegQuad>=1&&assemble.cLegQuad<=4)
			pModRod->SetLayer('L');	//�Ȳ�Leg
		else
			pModRod->SetLayer('B');	//����Body
		if(partinfo.cPartType==1)
		{
			pModRod->SetRodType(1);
			GEPOINT vec_wing_x=ucs.TransVToCS(assemble.acs.axis_x);
			GEPOINT vec_wing_y=ucs.TransVToCS(assemble.acs.axis_y);
			pModRod->SetWingXVec(MOD_POINT(vec_wing_x));
			pModRod->SetWingYVec(MOD_POINT(vec_wing_y));
		}
		else
			pModRod->SetRodType(2);
	}
	//��ȡ�ҵ���Ϣ
	int nHangPt=HangPointCount();
	for(int i=0;i<nHangPt;i++)
	{
		ITidHangPoint* pHangPt=GetHangPointAt(i);
		if(pHangPt==NULL)
			continue;
		CXhChar50 sDes;
		pHangPt->GetWireDescription(sDes);
		TID_COORD3D pt=pHangPt->GetPos();
		//
		MOD_HANG_NODE* pHangNode=pModModel->AppendHangNode();
		pHangNode->m_xHangPos=ucs.TransPFromCS(f3dPoint(pt));
		strcpy(pHangNode->m_sHangName,sDes);
		pHangNode->m_ciWireType=pHangPt->GetWireType();
	}
	//��ʼ����������͵�MOD�ṹ
	if (pModModel->InitMultiModData())
	{
		FILE *fp = fopen(sFileName, "wt,ccs=UTF-8");
		if (fp == NULL)
			return false;
		pModModel->WriteModFileByUtf8(fp);
		return true;
	}
	return false;
#endif
}
//////////////////////////////////////////////////////////////////////////
// CTidHangPoint
short CTidHangPoint::GetWireDescription(char* sDes)
{
	if(sDes)
	{
		strcpy(sDes,wireNode.name);
		return strlen(wireNode.name);
	}
	else
		return 0;
}
bool CTidHangPoint::IsCircuitDC()
{
	WIREPLACE_CODE code(wireNode.wiCode);
	return code.blCircuitDC;
}
BYTE CTidHangPoint::GetCircuitSerial()
{
	WIREPLACE_CODE code(wireNode.wiCode);
	return code.ciCircuitSerial;
}
BYTE CTidHangPoint::GetTensionType()
{
	WIREPLACE_CODE code(wireNode.wiCode);
	return code.ciTensionType;
}
BYTE CTidHangPoint::GetWireDirection()
{
	WIREPLACE_CODE code(wireNode.wiCode);
	return code.ciWireDirection;
}
BYTE CTidHangPoint::GetPhaseSerial()
{
	WIREPLACE_CODE code(wireNode.wiCode);
	return code.ciPhaseSerial;
}
BYTE CTidHangPoint::GetWireType()
{
	WIREPLACE_CODE code(wireNode.wiCode);
	return code.ciWireType;
}
BYTE CTidHangPoint::GetHangingStyle()
{
	WIREPLACE_CODE code(wireNode.wiCode);
	return code.ciHangingStyle;
}
BYTE CTidHangPoint::GetPostCode()
{
	WIREPLACE_CODE code(wireNode.wiCode);
	return code.ciPostCode;
}
BYTE CTidHangPoint::GetSerial()
{
	WIREPLACE_CODE code(wireNode.wiCode);
	return code.iSerial;
}
BYTE CTidHangPoint::GetPosSymbol()
{
	WIREPLACE_CODE code(wireNode.wiCode);
	if (code.ciWireType == 'J' || code.ciTensionType != 2)
		return 0;	//���߼����������߲�����ǰ��
	if (code.ciWireDirection == 'Y')	//���·���ΪY���ҵ���X�ᵣ��
	{	//����GIM�淶Ҫ��:X��ᵣ��Y������Ϊǰ,����Ϊ��;���ǽ�ģ����ϵ��Y����GIMģ������ϵ��Y���Ƿ����
		if (wireNode.position.y < -0.03)
			return 'Q';
		else
			return 'H';
	}
	else //if(code.ciWireDirection=='X') //���·���ΪX���ҵ���Y�ᵣ��
	{	//����GIM�淶Ҫ��:Y��ᵣ��X�Ḻ��Ϊǰ,����Ϊ��;�ҽ�ģ����ϵ��X����GIMģ������ϵ��X��ͬ��
		if (wireNode.position.x < -0.03)
			return 'Q';
		else
			return 'H';
	}
}
BYTE CTidHangPoint::GetRelaHoleNum()
{
	int nNum = 0;
	if (!wireNode.relaHolePt[0].IsZero())
		nNum++;
	if (!wireNode.relaHolePt[1].IsZero())
		nNum++;
	return nNum;
}
TID_COORD3D CTidHangPoint::GetRelaHolePos(int index)
{
	if (index == 0)
		return TID_COORD3D(wireNode.relaHolePt[0]);
	else if(index==1)
		return TID_COORD3D(wireNode.relaHolePt[1]);
	else
		return TID_COORD3D();
}
//////////////////////////////////////////////////////////////////////////
// CTidHeightGroup
int CTidHeightGroup::GetName(char *moduleName,UINT maxBufLength/*=0*/)
{
	if(!StrCopy(moduleName,module.name,maxBufLength))
		strcpy(moduleName,module.name);
	return strlen(moduleName);
}
BYTE CTidHeightGroup::RetrieveSerialInitBitPos()
{
	int ciInitSerialBitPosition=0;
	for(int i=1;i<=192;i++)
	{
		if(module.m_dwLegCfgWord.IsHasNo(i))
			return i;
	}
	return ciInitSerialBitPosition;
}
int CTidHeightGroup::GetLegSerialArr(int* legSerialArr)
{
	ARRAY_LIST<int> legArr(0,8);
	int legSerial=0;
	for(int i=1;i<=192;i++)
	{
		if(module.m_dwLegCfgWord.IsHasNo(i))
		{
			if(legSerial==0)
				legSerial=1;
			legArr.append(legSerial);
		}
		if(legSerial>0)
			legSerial++;
		if(legSerial>24)
			break;	//һ��������������24�����
	}
	if(legSerialArr!=NULL)
		memcpy(legSerialArr,legArr.m_pData,sizeof(int)*legArr.GetSize());
	return legArr.GetSize();
}
BYTE CTidHeightGroup::GetLegBitSerialFromSerialId(int serial)
{
	int legBitSerial = 0;
	for (int i = 1; i <= 192; i++)
	{
		if (!module.m_dwLegCfgWord.IsHasNo(i))
			continue;
		legBitSerial += 1;
		if (legBitSerial == serial)
			return i;
		if (legBitSerial >= 24)
			break; //һ��������������24�����
	}
	return 0;
}
static int f2i(double fval)
{
	if(fval>=0)
		return (int)(fval+0.5);
	else
		return (int)(fval-0.5);
}
int CTidHeightGroup::GetLegSerial(double heightDifference)
{
	int init_level=module.m_cbLegInfo>>6;	//ǰ��λ��1���ȵķ��򴹸����������
	int level_height =module.m_cbLegInfo&0x3f;	//����
	if(level_height==0)
		level_height=15;
	int nLevel=f2i(heightDifference*10)/level_height;	//�߶�����
	if(init_level==3)
	{
		for (int i=0;i<24;i++)
		{
			char ciLegSerial=abs(module.xarrUdfLegs[i].ciLegSerial);
			int sign=module.xarrUdfLegs[i].ciLegSerial>0?-1:1;
			double reduction=sign*module.xarrUdfLegs[i].wiReduction*0.1;
			if (fabs(reduction-heightDifference)<5)
				return ciLegSerial;
			else if (module.xarrUdfLegs[i].ciLegSerial==0)
				break;
		}
	}
	return (init_level+1-nLevel);
}
double CTidHeightGroup::GetLegHeightDifference(int legSerial)
{
	int init_level=module.m_cbLegInfo>>6;	//ǰ��λ��1���ȵķ��򴹸����������
	int level_height =module.m_cbLegInfo&0x3f;	//����
	if(level_height==0)
		level_height=15;
	if(init_level==3)
	{
		for (int i=0;i<24;i++)
		{
			if (abs(module.xarrUdfLegs[i].ciLegSerial)==legSerial)
				return module.xarrUdfLegs[i].ciLegSerial>0?-module.xarrUdfLegs[i].wiReduction*0.1:module.xarrUdfLegs[i].wiReduction*0.1;
			else if (module.xarrUdfLegs[i].ciLegSerial==0)
				break;
		}
	}
	return (init_level+1-legSerial)*level_height*0.1;
}
double CTidHeightGroup::GetLegHeight(int legSerial)
{
	//������ȳ�
	double fBody2LegTransZ = GetBody2LegTransitZ();
	double fLowestLegZ = GetLowestZ();
	double fMaxLegH = fLowestLegZ - fBody2LegTransZ;
	//����ָ����ŵ�������ȵĸ߶Ȳ�
	double fLegDiffH = GetLegHeightDifference(legSerial);
	fLegDiffH *= 1000;
	//
	double fLegH = (fLegDiffH < 0) ? fMaxLegH + fLegDiffH : fMaxLegH - fLegDiffH;
	return fLegH * 0.001;
}
ITidTowerInstance* CTidHeightGroup::GetTowerInstance(int legSerialQuad1, int legSerialQuad2, int legSerialQuad3, int legSerialQuad4)
{
	union UNIQUEID{
		BYTE bytes[4];
		DWORD id;
	}key;
	key.bytes[0]=GetLegBitSerialFromSerialId(legSerialQuad1);
	key.bytes[1]=GetLegBitSerialFromSerialId(legSerialQuad2);
	key.bytes[2]=GetLegBitSerialFromSerialId(legSerialQuad3);
	key.bytes[3]=GetLegBitSerialFromSerialId(legSerialQuad4);
	CTidTowerInstance* pTidTowerInstance=hashTowerInstance.Add(key.id);
	pTidTowerInstance->SetBelongHeight(this);
	memcpy(pTidTowerInstance->arrLegBitSerial,key.bytes,4);
	pTidTowerInstance->InitAssemblePartAndBolt();
	return pTidTowerInstance;
}
double CTidHeightGroup::GetLowestZ()
{
	double fMaxNodeZ = 0;
	ITidAssemblePart* pPart = NULL;
	for (pPart = m_pModel->EnumAssemblePartFirst(m_id); pPart; pPart = m_pModel->EnumAssemblePartNext(m_id))
	{
		if (!pPart->IsHasBriefRodLine())
			continue;
		if (pPart->GetLegQuad() < 1 || pPart->GetLegQuad() > 4)
			continue;	//���Ȳ��˼�
		fMaxNodeZ = max(fMaxNodeZ, pPart->BriefLineStart().z);
		fMaxNodeZ = max(fMaxNodeZ, pPart->BriefLineEnd().z);
	}
	return fMaxNodeZ;
}
double CTidHeightGroup::GetBody2LegTransitZ()
{
	double fTransitZ = 0;
	ITidAssemblePart* pPart = NULL;
	for (pPart = m_pModel->EnumAssemblePartFirst(m_id); pPart; pPart = m_pModel->EnumAssemblePartNext(m_id))
	{
		if(!pPart->IsHasBriefRodLine() ||
			(pPart->GetLegQuad() >= 1 && pPart->GetLegQuad() <= 4))
			continue;	//������˼�
		fTransitZ = max(fTransitZ, pPart->BriefLineStart().z);
		fTransitZ = max(fTransitZ, pPart->BriefLineEnd().z);
	}
	return fTransitZ;
}
double CTidHeightGroup::GetBodyNamedHeight()
{
	double fBody2LegTransZ = GetBody2LegTransitZ();
	double fNamedHeightZeroZ = m_pModel->GetNamedHeightZeroZ();
	double fBody2LegHeight = fBody2LegTransZ - fNamedHeightZeroZ;
	return fBody2LegHeight;
}
bool CTidHeightGroup::GetConfigBytes(BYTE* cfgword_bytes24)
{
	if (cfgword_bytes24 == NULL)
		return false;
	memcpy(cfgword_bytes24, module.m_dwLegCfgWord.flag.bytes, 24);
	return true;
}
//////////////////////////////////////////////////////////////////////////
// CTidTowerInstance
CTidTowerInstance::CTidTowerInstance()
{
	m_pModel=NULL;
	m_pBelongHeight=NULL;
	m_fInstanceHeight=0;
	arrLegBitSerial[0]=arrLegBitSerial[1]=arrLegBitSerial[2]=arrLegBitSerial[3]=0;
}
ITidHeightGroup* CTidTowerInstance::SetBelongHeight(ITidHeightGroup* pHeightGroup)
{
	m_pBelongHeight=(CTidHeightGroup*)pHeightGroup;
	if(m_pBelongHeight)
		m_pModel=m_pBelongHeight->BelongModel();
	return m_pBelongHeight;
}
short CTidTowerInstance::GetLegSerialIdByQuad(short siQuad)
{
	siQuad--;
	if(siQuad<0||siQuad>=4)
		return 0;
	BYTE initPosition=m_pBelongHeight->RetrieveSerialInitBitPos();	//1Ϊ��ʼ����
	return arrLegBitSerial[siQuad]-initPosition+1;
}
void CTidTowerInstance::InitAssemblePartAndBolt()
{
	if(Nodes.GetNodeNum()==0)
	{
		
		NODE_ASSEMBLY node;
		UINT uCount=m_pModel->m_xNodeSection.NodeCount();
		for(UINT i=0;i<uCount;i++)
		{
			m_pModel->m_xNodeSection.GetNodeByIndexId(i+1,true,node);
			if(!IsIncludeObject(m_pBelongHeight->module.m_dwLegCfgWord,arrLegBitSerial,node.cfgword,node.cLegQuad))
				continue;
			CTidNode* pNode=Nodes.Add(i+1);
			pNode->node=node;
			pNode->SetBelongModel(m_pModel);
		}
		uCount=m_pModel->m_xNodeSection.NodeCount(false);
		for(UINT i=0;i<uCount;i++)
		{
			m_pModel->m_xNodeSection.GetNodeByIndexId(i+1,false,node);
			UINT uBlkRef=m_pModel->m_xAssembleSection.BlockAssemblyCount();
			for(UINT indexId=1;indexId<=uBlkRef;indexId++)
			{
				BLOCK_ASSEMBLY blkassembly=m_pModel->m_xAssembleSection.GetAssemblyByIndexId(indexId);
				if(blkassembly.wIndexId!=node.wBlockIndexId)
					continue;	//�ù��������ڵ�ǰ����װ�����
				if(!IsIncludeObject(m_pBelongHeight->module.m_dwLegCfgWord,arrLegBitSerial,blkassembly.cfgword,blkassembly.cLegQuad))
					continue;
				TOWER_BLOCK block=m_pModel->m_xBlockSection.GetBlockAt(blkassembly.wIndexId-1);
				TID_CS fromTidCS=ConvertCSFrom(block.lcs);
				TID_CS toTidCS=ConvertCSFrom(blkassembly.acs);
				//coord_trans(node.xPosition,block.lcs,FALSE,TRUE);
				//coord_trans(node.xPosition,blkassembly.acs,TRUE,TRUE);
				node.xPosition=block.lcs.TransPToCS(node.xPosition);
				node.xPosition=blkassembly.acs.TransPFromCS(node.xPosition);
				//
				CTidNode* pNode=Nodes.Add(i+1);
				pNode->node=node;
				pNode->SetBelongModel(m_pModel);
			}
		}
	}
	int iKey=0;
	if(partAssembly.GetNodeNum()==0)
	{	//��Ҫ��ȡ����װ�伯��
		double fMaxNodeZ = 0;
		UINT uCount=m_pModel->m_xAssemblyParts.AssemblyCount();
		for(UINT i=0;i<uCount;i++)
		{
			PART_ASSEMBLY assemble;
			m_pModel->m_xAssemblyParts.GetAssemblyByIndexId(i+1,true,assemble);
			if(!IsIncludeObject(m_pBelongHeight->module.m_dwLegCfgWord,arrLegBitSerial,assemble.cfgword,assemble.cLegQuad))
				continue;
			CTidAssemblePart* pAssmPart=partAssembly.Add(++iKey);
			pAssmPart->part=assemble;
			pAssmPart->SetBelongModel(m_pModel);
			PART_INFO partinfo=m_pModel->m_xPartSection.GetPartInfoByIndexId(assemble.dwIndexId);
			pAssmPart->solid.CopySolidBuffer(partinfo.solid.BufferPtr(),partinfo.solid.BufferLength());
			TID_CS acs=pAssmPart->GetAcs();
			pAssmPart->solid.TransToACS(acs);
			if (pAssmPart->IsHasBriefRodLine())
			{
				fMaxNodeZ = max(fMaxNodeZ, pAssmPart->BriefLineStart().z);
				fMaxNodeZ = max(fMaxNodeZ, pAssmPart->BriefLineEnd().z);
			}
		}
		m_fInstanceHeight = fMaxNodeZ;
		uCount=m_pModel->m_xAssemblyParts.AssemblyCount(false);
		for(UINT i=0;i<uCount;i++)
		{
			PART_ASSEMBLY assemble;
			m_pModel->m_xAssemblyParts.GetAssemblyByIndexId(i+1,false,assemble);
			UINT uBlkRef=m_pModel->m_xAssembleSection.BlockAssemblyCount();
			for(UINT indexId=1;indexId<=uBlkRef;indexId++)
			{
				BLOCK_ASSEMBLY blkassembly=m_pModel->m_xAssembleSection.GetAssemblyByIndexId(indexId);
				if(blkassembly.wIndexId!=assemble.wBlockIndexId)
					continue;	//�ù��������ڵ�ǰ����װ�����
				if(!IsIncludeObject(m_pBelongHeight->module.m_dwLegCfgWord,arrLegBitSerial,blkassembly.cfgword,blkassembly.cLegQuad))
					continue;
				CTidAssemblePart* pAssmPart=partAssembly.Add(++iKey);
				pAssmPart->part=assemble;
				pAssmPart->SetBelongModel(m_pModel);
				PART_INFO partinfo=m_pModel->m_xPartSection.GetPartInfoByIndexId(assemble.dwIndexId);
				pAssmPart->solid.CopySolidBuffer(partinfo.solid.BufferPtr(),partinfo.solid.BufferLength());
				pAssmPart->solid.TransToACS(ConvertCSFrom(assemble.acs));
				TOWER_BLOCK block=m_pModel->m_xBlockSection.GetBlockAt(blkassembly.wIndexId-1);
				pAssmPart->solid.TransACS(ConvertCSFrom(block.lcs),ConvertCSFrom(blkassembly.acs));
			}
		}
	}
	if(boltAssembly.GetNodeNum()==0)
	{	//��Ҫ��ȡ��˨װ�伯��
		UINT uCount=m_pModel->m_xAssemblyBolts.AssemblyCount();
		for(UINT i=0;i<uCount;i++)
		{
			BOLT_ASSEMBLY assemble;
			assemble=m_pModel->m_xAssemblyBolts.GetAssemblyByIndexId(i+1,true);
			if(!IsIncludeObject(m_pBelongHeight->module.m_dwLegCfgWord,arrLegBitSerial,assemble.cfgword,assemble.cLegQuad))
				continue;
			IBoltSeriesLib* pBoltLibrary=m_pModel->GetBoltLib();
			IBoltSeries* pBoltSeries=pBoltLibrary->GetBoltSeriesAt(assemble.cSeriesId-1);
			IBoltSizeSpec* pBoltSize=pBoltSeries->GetBoltSizeSpecById(assemble.wIndexId);
			if(pBoltSize==NULL)
			{
				logerr.Log("BoltSizeSpec@%d not found!\n",assemble.wIndexId);
				continue;
			}
			CTidSolidBody* pBoltSolid=(CTidSolidBody*)pBoltSize->GetBoltSolid();
			CTidSolidBody* pNutSolid=(CTidSolidBody*)pBoltSize->GetNutSolid();
			//
			CTidAssembleBolt* pAssmBolt=boltAssembly.Add(++iKey);
			pAssmBolt->bolt=assemble;
			pAssmBolt->SetBelongModel(m_pModel);
			pAssmBolt->solid.bolt.CopySolidBuffer(pBoltSolid->SolidBufferPtr(),pBoltSolid->SolidBufferLength());
			pAssmBolt->solid.nut.CopySolidBuffer(pNutSolid->SolidBufferPtr(),pNutSolid->SolidBufferLength());
			TID_CS acs=pAssmBolt->GetAcs();
			pAssmBolt->solid.bolt.TransToACS(acs);
			acs.origin.x+=acs.axisZ.x*assemble.wL0;
			acs.origin.y+=acs.axisZ.y*assemble.wL0;
			acs.origin.z+=acs.axisZ.z*assemble.wL0;
			pAssmBolt->solid.nut.TransToACS(acs);
		}
		uCount=m_pModel->m_xAssemblyBolts.AssemblyCount(false);
		for(UINT i=0;i<uCount;i++)
		{
			BOLT_ASSEMBLY assemble=m_pModel->m_xAssemblyBolts.GetAssemblyByIndexId(i+1,false);
			UINT uBlkRef=m_pModel->m_xAssembleSection.BlockAssemblyCount();
			for(UINT indexId=1;indexId<=uBlkRef;indexId++)
			{
				BLOCK_ASSEMBLY blkassembly=m_pModel->m_xAssembleSection.GetAssemblyByIndexId(indexId);
				if(blkassembly.wIndexId!=assemble.wBlockIndexId)
					continue;	//�ù��������ڵ�ǰ����װ�����
				if(!IsIncludeObject(m_pBelongHeight->module.m_dwLegCfgWord,arrLegBitSerial,blkassembly.cfgword,blkassembly.cLegQuad))
					continue;
				IBoltSeriesLib* pBoltLibrary=m_pModel->GetBoltLib();
				IBoltSeries* pBoltSeries=pBoltLibrary->GetBoltSeriesAt(assemble.cSeriesId-1);
				IBoltSizeSpec* pBoltSize=pBoltSeries->GetBoltSizeSpecById(assemble.wIndexId);
				if(pBoltSize==NULL)
				{
					logerr.Log("BoltSizeSpec@%d not found!\n",assemble.wIndexId);
					continue;
				}
				CTidSolidBody* pBoltSolid=(CTidSolidBody*)pBoltSize->GetBoltSolid();
				CTidSolidBody* pNutSolid=(CTidSolidBody*)pBoltSize->GetNutSolid();
				//
				CTidAssembleBolt* pAssmBolt=boltAssembly.Add(++iKey);
				pAssmBolt->bolt=assemble;
				pAssmBolt->SetBelongModel(m_pModel);
				pAssmBolt->solid.bolt.CopySolidBuffer(pBoltSolid->SolidBufferPtr(),pBoltSolid->SolidBufferLength());
				pAssmBolt->solid.nut.CopySolidBuffer(pNutSolid->SolidBufferPtr(),pNutSolid->SolidBufferLength());
				TID_CS acs=pAssmBolt->GetAcs();
				pAssmBolt->solid.bolt.TransToACS(acs);
				acs.origin.x+=acs.axisZ.x*assemble.wL0;
				acs.origin.y+=acs.axisZ.y*assemble.wL0;
				acs.origin.z+=acs.axisZ.z*assemble.wL0;
				pAssmBolt->solid.nut.TransToACS(acs);
				TOWER_BLOCK block=m_pModel->m_xBlockSection.GetBlockAt(blkassembly.wIndexId-1);
				pAssmBolt->solid.bolt.TransACS(ConvertCSFrom(block.lcs),ConvertCSFrom(blkassembly.acs));
				pAssmBolt->solid.nut.TransACS(ConvertCSFrom(block.lcs),ConvertCSFrom(blkassembly.acs));
			}
		}
	}
	if(hashAnchorBolts.GetNodeNum()==0)
	{
		WORD wiAnchorCountPerBase=m_pModel->GetAnchorCount();
		for(WORD i=0;i<4;i++)
		{
			GEPOINT xBaseLocation;
			BYTE ciLegSerial=(BYTE)GetLegSerialIdByQuad(i+1);
			if(!m_pModel->GetSubLegBaseLocation(this->m_pBelongHeight->GetSerialId(),ciLegSerial,xBaseLocation))
				continue;
			if(i==1||i==3)	 //���1����Y��Գ�
				xBaseLocation.x*=-1.0;
			if(i==2||i==3)	//���1����X��Գ�
				xBaseLocation.y*=-1.0;
			for(WORD j=0;j<wiAnchorCountPerBase;j++)
			{
				short siPosX=0,siPosY=0;
				m_pModel->GetAnchorAt(j,&siPosX,&siPosY);
				//
				CTidAssembleAnchorBolt* pAnchorBolt=hashAnchorBolts.Add(++iKey);
				pAnchorBolt->SetBelongModel(m_pModel);
				pAnchorBolt->xLocation.Set(xBaseLocation.x+siPosX,xBaseLocation.y+siPosY,xBaseLocation.z);
				pAnchorBolt->m_ciLegQuad=i+1;
				m_pModel->GetAnchorBoltSolid(&pAnchorBolt->solid.bolt,&pAnchorBolt->solid.nut);
				TID_CS acs=pAnchorBolt->GetAcs();
				pAnchorBolt->solid.bolt.TransToACS(acs);
				acs.origin.x+=acs.axisZ.x*m_pModel->GetBasePlateThick();
				acs.origin.y+=acs.axisZ.y*m_pModel->GetBasePlateThick();
				acs.origin.z+=acs.axisZ.z*m_pModel->GetBasePlateThick();
				pAnchorBolt->solid.nut.TransToACS(acs);
			}
		}
	}
}
bool CTidTowerInstance::ExportModFile(const char* sFileName)
{
#ifdef _DISABLE_MOD_CORE_
	return false;
#else
	if(BelongHeightGroup()==NULL||BelongTidModel()==NULL)
		return false;
	IModModel* pModModel=CModModelFactory::CreateModModel();
	if(pModModel==NULL)
		return false;
	CLogErrorLife logErrLife;
	pModModel->SetTowerHeight(GetInstanceHeight());
	GECS ucs=TransToUcs(pModModel->BuildUcsByModCS());
	//��ʼ��MODģ�͵Ľڵ���˼�
	CXhChar16 sLayer;
	for(ITidNode* pNode=EnumNodeFirst();pNode;pNode=EnumNodeNext())
	{
		pNode->GetLayer(sLayer);
		TID_COORD3D pos=pNode->GetPos();
		f3dPoint node_org=ucs.TransPFromCS(f3dPoint(pos.x,pos.y,pos.z));
		//
		IModNode* pModNode=pModModel->AppendNode(pNode->GetId());
		pModNode->SetBelongModel(pModModel);
		pModNode->SetOrg(MOD_POINT(node_org));
		if(sLayer[0]=='L'||sLayer[0]=='l')
			pModNode->SetLayer('L');	//�Ȳ�Leg
		else
			pModNode->SetLayer('B');	//����Body
	}
	for(ITidAssemblePart* pAssmPart=EnumAssemblePartFirst();pAssmPart;pAssmPart=EnumAssemblePartNext())
	{
		if(!pAssmPart->IsHasBriefRodLine())
			continue;
		ITidPart* pTidPart=pAssmPart->GetPart();
		if(pTidPart==NULL)
			continue;
		UINT iNodeIdS=pAssmPart->GetStartNodeId();
		UINT iNodeIdE=pAssmPart->GetEndNodeId();
		IModNode* pModNodeS=pModModel->FindNode(iNodeIdS);
		IModNode* pModNodeE=pModModel->FindNode(iNodeIdE);
		if(pModNodeS==NULL || pModNodeE==NULL)
		{
			//logerr.Log("S:%d-E:%d�ڵ��Ҳ���",iNodeIdS,iNodeIdE);
			continue;	//�̽Ǹ�
		}
		pAssmPart->GetLayer(sLayer);
		TID_CS assm_cs=pAssmPart->GetAcs();
		//
		IModRod* pModRod=pModModel->AppendRod(pAssmPart->GetId());
		pModRod->SetBelongModel(pModModel);
		pModRod->SetMaterial(pTidPart->GetBriefMark());
		pModRod->SetWidth(pTidPart->GetWidth());
		pModRod->SetThick(pTidPart->GetThick());
		if(pTidPart->GetPartType()==ITidPart::TYPE_ANGLE)
		{
			f3dPoint vWingX(assm_cs.axisX),vWingY(assm_cs.axisY);
			pModRod->SetRodType(1);
			pModRod->SetWingXVec(MOD_POINT(ucs.TransVToCS(vWingX)));
			pModRod->SetWingYVec(MOD_POINT(ucs.TransVToCS(vWingY)));
		}
		else
			pModRod->SetRodType(2);
		if(sLayer[0]=='L'||sLayer[0]=='l')
			pModRod->SetLayer('L');	//�Ȳ�Leg
		else
			pModRod->SetLayer('B');	//����Body
		if(pModRod->IsLegRod())
		{	//�����Ȳ��˼�����ӹ��ɽڵ�
			if(pModNodeS && !pModNodeS->IsLegNode())
			{
				IModNode* pNewModNode=pModModel->AppendNode(0);
				pNewModNode->SetBelongModel(pModModel);
				pNewModNode->SetOrg(pModNodeS->NodePos());
				pNewModNode->SetLayer('S');
				pModNodeS=pNewModNode;
			}
			if(pModNodeE && !pModNodeE->IsLegNode())
			{
				IModNode* pNewModNode=pModModel->AppendNode(0);
				pNewModNode->SetBelongModel(pModModel);
				pNewModNode->SetOrg(pModNodeE->NodePos());
				pNewModNode->SetLayer('S');
				pModNodeE=pNewModNode;
			}
		}
		pModRod->SetNodeS(pModNodeS);
		pModRod->SetNodeE(pModNodeE);
	}
	//��ʼ������������MOD�ṹ(�����������ʵ�ʺ���ֵ)
	double fNameHeightZeroZ = BelongTidModel()->GetNamedHeightZeroZ();
	double fActualHeight = m_fInstanceHeight-fNameHeightZeroZ;
	pModModel->InitSingleModData(fActualHeight);
	//��ӹҵ�
	int nHangPt=BelongTidModel()->HangPointCount();
	for(int i=0;i<nHangPt;i++)
	{
		ITidHangPoint* pHangPt=BelongTidModel()->GetHangPointAt(i);
		if(pHangPt==NULL)
			continue;
		CXhChar50 sDes;
		pHangPt->GetWireDescription(sDes);
		TID_COORD3D pt=pHangPt->GetPos();
		//
		MOD_HANG_NODE* pHangNode=pModModel->AppendHangNode();
		pHangNode->m_xHangPos=ucs.TransPFromCS(f3dPoint(pt));
		strcpy(pHangNode->m_sHangName,sDes);
		pHangNode->m_ciWireType=pHangPt->GetWireType();
	}
	//����Mod�ļ�
	FILE *fp=fopen(sFileName,"wt,ccs=UTF-8");
	if(fp==NULL)
	{
		logerr.Log("%s�ļ���ʧ��!",sFileName);
		return false;
	}
	pModModel->WriteModFileByUtf8(fp);
	return true;
#endif
}
//////////////////////////////////////////////////////////////////////////
// CBoltSeriesLib
int CBoltSeriesLib::GetCount()
{
	return sectdata.BoltSeriesCount();
}
IBoltSeries* CBoltSeriesLib::GetBoltSeriesAt(int i)
{
	if(i<0||i>=sectdata.BoltSeriesCount())
		return NULL;
	CTidBoltSeries* pBoltSeries=hashBoltSeries.GetValue(i+1);
	if(pBoltSeries==NULL)
	{
		pBoltSeries=hashBoltSeries.Add(i+1);
		pBoltSeries->sectdata=sectdata.GetBoltSeriesAt(i);
	}
	return pBoltSeries;
}
IBoltSizeSpec* CBoltSeriesLib::GetBoltSizeSpec(int seriesId,int sizeSpecId)
{
	IBoltSeries* pBoltSeries=GetBoltSeriesAt(seriesId-1);
	if(pBoltSeries!=NULL)
		return pBoltSeries->GetBoltSizeSpecById(sizeSpecId);
	return NULL;
}
//////////////////////////////////////////////////////////////////////////
// CTidPartsLib
int CTidPartsLib::GetCount()
{
	return sectdata.GetPartCount();
}
ITidPart* CTidPartsLib::GetTidPartBySerialId(int serialid)
{
	if(serialid<=0||serialid>(int)sectdata.GetPartCount())
		return NULL;
	CTidPart* pTidPart=hashTidParts.GetValue(serialid);
	if(pTidPart==NULL)
	{
		pTidPart=hashTidParts.Add(serialid);
		pTidPart->partinfo=sectdata.GetPartInfoByIndexId(serialid);
	}
	return pTidPart;
}
//////////////////////////////////////////////////////////////////////////
// CTidBoltSeries
short CTidBoltSeries::GetName(char* name)
{
	CXhChar24 seriesname=sectdata.GetSeriesName();
	if(name)
		strcpy(name,seriesname);
	return seriesname.GetLength();
}
/*
bool CTidBoltSeries::IsFootNail()				//�Ŷ�
{
	return false;
}
bool CTidBoltSeries::IsBurglarproof()			//�Ƿ�Ϊ������˨
{
	return false;
}
*/
IBoltSizeSpec* CTidBoltSeries::EnumFirst()
{
	m_iterIndex=1;
	return GetBoltSizeSpecById(m_iterIndex);
}
IBoltSizeSpec* CTidBoltSeries::EnumNext()
{
	m_iterIndex++;
	return GetBoltSizeSpecById(m_iterIndex);
}
IBoltSizeSpec* CTidBoltSeries::GetBoltSizeSpecById(int id)
{
	if(id<=0||id>GetCount())
		return NULL;
	CBoltSizeSection boltsect;
	if(!sectdata.GetBoltSizeAt(id-1,boltsect))
		return NULL;
	CTidBoltSizeSpec* pSizeSpec=hashBoltSizes.Add(id);
	pSizeSpec->sectdata=boltsect;
	pSizeSpec->solid.bolt.AttachBuffer(boltsect.solid.bolt.BufferPtr(),boltsect.solid.bolt.BufferLength());
	pSizeSpec->solid.nut.AttachBuffer(boltsect.solid.nut.BufferPtr(),boltsect.solid.nut.BufferLength());
	pSizeSpec->SetSeriesId(GetSeriesId());
	return pSizeSpec;
}
//////////////////////////////////////////////////////////////////////////
// CTidPart
short CTidPart::GetSegStr(char* segstr)
{
	SEGI segi=partinfo.dwSeg;
	strcpy(segstr,segi.ToString());
	return strlen(segstr);
}
short CTidPart::GetSpec(char* sizeSpec)
{
	if(strlen(partinfo.spec)>0)
		strcpy(sizeSpec,partinfo.spec);
	else
		sprintf(sizeSpec,"%g*%g",partinfo.fWidth,partinfo.fThick);
	return strlen(sizeSpec);
}
short CTidPart::GetLabel(char* label)
{
	strcpy(label,partinfo.sPartNo);
	return strlen(label);
}
ITidSolidBody* CTidPart::GetSolidPart()
{
	solid.AttachBuffer(partinfo.solid.BufferPtr(),partinfo.solid.BufferLength());
	return &solid;
}
UINT CTidPart::GetProcessBuffBytes(char* processbytes,UINT maxBufLength/*=0*/)
{
	if(processbytes==NULL)
		return partinfo.dwProcessBuffSize;
	int copybuffsize=maxBufLength<=0?partinfo.dwProcessBuffSize:min(maxBufLength,partinfo.dwProcessBuffSize);
	memcpy(processbytes,partinfo.processBuffBytes,copybuffsize);
	return 0;
}
//////////////////////////////////////////////////////////////////////////
// CTidBoltSizeSpec
CTidBoltSizeSpec::CTidBoltSizeSpec(DWORD key/*=0*/)
{
	m_id=key;
	m_uiSeriesId=0;
}
ITidSolidBody* CTidBoltSizeSpec::GetBoltSolid()//��˨ʵ��	
{
	return &solid.bolt;
}
ITidSolidBody* CTidBoltSizeSpec::GetNutSolid()//��˨ʵ��	
{
	return &solid.nut;
}
//////////////////////////////////////////////////////////////////////////
//CTidAssembleAnchorBolt
IAnchorBoltSpec* CTidAssembleAnchorBolt::GetAnchorBolt()
{
	xAnchorBolt.anchor_bolt.d = m_pModel->m_xBoltLib.sectdata.anchorInfo.d;				//�ؽ���˨����ֱ��
	xAnchorBolt.anchor_bolt.wiLe = m_pModel->m_xBoltLib.sectdata.anchorInfo.wiLe;		//�ؽ���˨��¶��(mm)
	xAnchorBolt.anchor_bolt.wiLa = m_pModel->m_xBoltLib.sectdata.anchorInfo.wiLa;		//�ؽ���˨�޿۳�(mm) <��������
	xAnchorBolt.anchor_bolt.wiWidth = m_pModel->m_xBoltLib.sectdata.anchorInfo.wiWidth;	//�ؽ���˨�����
	xAnchorBolt.anchor_bolt.wiThick = m_pModel->m_xBoltLib.sectdata.anchorInfo.wiThick;	//�ؽ���˨�����
	xAnchorBolt.anchor_bolt.wiHoleD = m_pModel->m_xBoltLib.sectdata.anchorInfo.wiHoleD;	//�ؽ���˨��׾�
	xAnchorBolt.anchor_bolt.wiBasePlateHoleD = m_pModel->m_xBoltLib.sectdata.anchorInfo.wiBasePlateHoleD;
	memcpy(xAnchorBolt.anchor_bolt.szSizeSymbol, m_pModel->m_xBoltLib.sectdata.anchorInfo.szSizeSymbol, 64);
	xAnchorBolt.m_nBaseThick = m_pModel->m_xFoundationSection.ciBasePlateThick;
	return &xAnchorBolt;
}
TID_CS CTidAssembleAnchorBolt::GetAcs()
{
	TID_CS acs;
	acs.origin = TID_COORD3D(xLocation);
	acs.axisZ = TID_COORD3D(0, 0, -1);
	acs.axisX = TID_COORD3D(1, 0, 0);//GEPOINT(acs.axisZ).Perpendicular(true);
	acs.axisY = GEPOINT(acs.axisZ) ^ GEPOINT(acs.axisX);
	return acs;
}
ITidSolidBody* CTidAssembleAnchorBolt::GetBoltSolid()
{
	return &solid.bolt;
}
ITidSolidBody* CTidAssembleAnchorBolt::GetNutSolid()
{
	return &solid.nut;
}
//////////////////////////////////////////////////////////////////////////
// CTidAssembleBolt
CTidAssembleBolt::CTidAssembleBolt()
{
	m_id = 0;
	m_pModel = NULL;
}
CTidAssembleBolt::~CTidAssembleBolt()
{

}
IBoltSizeSpec *CTidAssembleBolt::GetTidBolt()
{
	CBoltSeries series=m_pModel->m_xBoltLib.sectdata.GetBoltSeriesAt(bolt.cSeriesId-1);
	CBoltSizeSection boltsect;
	if(!series.GetBoltSizeAt(bolt.wIndexId-1,boltsect))
		return NULL;
	sizespec.sectdata=boltsect;
	sizespec.SetSeriesId(bolt.cSeriesId);	//�趨������˨���ϵ�еı�ʶ
	sizespec.solid.bolt.AttachBuffer(boltsect.solid.bolt.BufferPtr(),boltsect.solid.bolt.BufferLength());
	sizespec.solid.nut.AttachBuffer(boltsect.solid.nut.BufferPtr(),boltsect.solid.nut.BufferLength());
	return &sizespec;
}
TID_CS CTidAssembleBolt::GetAcs()
{
	TID_CS acs;
	acs.origin=TID_COORD3D(bolt.origin);
	acs.axisZ =TID_COORD3D(bolt.work_norm);
	acs.axisX =GEPOINT(acs.axisZ).Perpendicular(true);
	acs.axisY = GEPOINT(acs.axisZ)^GEPOINT(acs.axisX);
	return acs;
}
ITidSolidBody* CTidAssembleBolt::GetBoltSolid()
{
	return &solid.bolt;
}
ITidSolidBody* CTidAssembleBolt::GetNutSolid()
{
	return &solid.nut;
}
//�κ�ǰ׺�ַ����ɺ������ַ������κ����ֲ���;�۸����ռ�װ����˨���ԣ�
//���ͳ����������λΪ1ʱ���ˣ����ֽڱ�ʾ����˨�Ķκ�ͳ�Ʒ�Χ�ַ����Ĵ洢��ַ
int CTidAssembleBolt::SegmentBelongStr(char* seg_str)			//�۸����ռ�װ����˨���ԣ�
{
	if(bolt.cStatFlag&0x80)	//ͬʱ��������κ�
	{
		if(seg_str)
			strcpy(seg_str,bolt.statSegStr);
		return bolt.statSegStr.GetLength();
	}
	else
	{
		CXhChar16 segstr=SEGI(bolt.dwSeg).ToString();
		if(seg_str)
			strcpy(seg_str,segstr);
		return segstr.GetLength();
	}
}
bool CTidAssembleBolt::GetConfigBytes(BYTE* cfgword_bytes24)
{
	if (cfgword_bytes24 == NULL)
		return false;
	memcpy(cfgword_bytes24, bolt.cfgword.flag.bytes, 24);
	return true;
}
//////////////////////////////////////////////////////////////////////////
// CTidAssemblePart
short CTidAssemblePart::GetType()
{
	ITidPart* pTidPart=GetPart();
	if(pTidPart)
		return pTidPart->GetPartType();
	else
		return 0;	//�������࣬1:�Ǹ�2:��˨3:�ְ�4:�ֹ�5:����6:�۸�
}
ITidPart *CTidAssemblePart::GetPart()
{
	PART_INFO partinfo=m_pModel->m_xPartSection.GetPartInfoByIndexId(part.dwIndexId);
	CTidPart* pTidPart=hashTidParts.Add(part.dwIndexId);
	pTidPart->partinfo=partinfo;
	return pTidPart;
}
TID_CS CTidAssemblePart::GetAcs()
{
	TID_CS acs;
	acs.origin=TID_COORD3D(part.acs.origin);
	acs.axisX=TID_COORD3D(part.acs.axis_x);
	acs.axisY=TID_COORD3D(part.acs.axis_y);
	acs.axisZ=TID_COORD3D(part.acs.axis_z);
	return acs;
}
ITidSolidBody* CTidAssemblePart::GetSolidPart()
{
	return &solid;
}
short CTidAssemblePart::GetLayer(char* sizeLayer)
{
	strcpy(sizeLayer,part.szLayerSymbol);
	return strlen(sizeLayer);
}
bool CTidAssemblePart::GetConfigBytes(BYTE* cfgword_bytes24)
{
	if (cfgword_bytes24 == NULL)
		return false;
	memcpy(cfgword_bytes24, part.cfgword.flag.bytes, 24);
	return true;
}
//////////////////////////////////////////////////////////////////////////
// CTidRawSolidEdge
DWORD CTidRawSolidEdge::LineStartId()
{
	if(IsReverse())
		return edge.LineEndId;
	else
		return edge.LineStartId;
}
DWORD CTidRawSolidEdge::LineEndId()
{
	if(IsReverse())
		return edge.LineStartId;
	else
		return edge.LineEndId;
}
TID_COORD3D CTidRawSolidEdge::WorkNorm()
{
	TID_COORD3D worknorm(edge.WorkNorm);
	if(!worknorm.IsZero()&&IsReverse())
		worknorm.Set(worknorm.x,worknorm.y,worknorm.z);
	return worknorm;
}
//////////////////////////////////////////////////////////////////////////
// CTidFaceLoop
CTidRawSolidEdge* CreateTidRawSolidEdge(int idClsType,DWORD key,void* pContext)	//����ص�����
{
	return new CTidRawSolidEdge(key,(CSolidBody*)pContext);
}
CTidFaceLoop::CTidFaceLoop(DWORD id/*=0*/,CSolidBody* pSolidBody/*=NULL*/)
{
	m_id=id;
	hashEdges.m_pContext=m_pSolidBody=pSolidBody;
	hashEdges.CreateNewAtom=CreateTidRawSolidEdge;
}
bool CTidFaceLoop::EdgeLineAt(int i,TidArcline* pArcline)
{
	CRawSolidEdge edge;
	if(!loop.GetLoopEdgeLineAt(m_pSolidBody,i,edge))
		return false;
	pArcline->FromByteArr(edge.BufferPtr(),edge.Size());
	return true;
}
ITidRawSolidEdge* CTidFaceLoop::EdgeLineAt(int index)
{
	CTidRawSolidEdge* pEdge=hashEdges.Add(index+1);
	DWORD edgeid=loop.LoopEdgeLineIdAt(index);
	pEdge->SetReverse((edgeid&0x80000000)>0);
	edgeid&=0x7fffffff;
	m_pSolidBody->GetKeyEdgeLineAt(edgeid-1,pEdge->edge);
	return pEdge;
}
//////////////////////////////////////////////////////////////////////////
// CTidRawFace
CTidFaceLoop* CreateTidFaceLoop(int idClsType,DWORD key,void* pContext)	//����ص�����
{
	return new CTidFaceLoop(key,(CSolidBody*)pContext);
}
CTidRawFace::CTidRawFace(DWORD id/*=0*/,CSolidBody* pSolidBody/*=NULL*/)
{
	m_id=id;
	outer.hashEdges.m_pContext=outer.m_pSolidBody=m_pSolidBody=pSolidBody;
	hashInnerLoops.m_pContext=pSolidBody;
	hashInnerLoops.CreateNewAtom=CreateTidFaceLoop;
}
CTidRawFace::~CTidRawFace()
{
	outer.Empty();
	hashInnerLoops.Empty();
}
COLORREF CTidRawFace::MatColor()		// �����ڼ�¼�����������Ϣ(����ʵ�)
{
	return face.MatColor();
}
DWORD CTidRawFace::FaceId()	//���ڱ�ʶ����������е�ĳһ�ض���
{
	return face.FaceId();
}
WORD CTidRawFace::InnerLoopNum()
{
	return face.InnerLoopNum();;
}
IFaceLoop* CTidRawFace::InnerLoopAt(int i)
{
	CTidFaceLoop* pLoop=hashInnerLoops.Add(i+1);
	pLoop->loop=face.GetInnerLoopAt(i);
	return pLoop;
}
IFaceLoop* CTidRawFace::OutterLoop()
{
	outer.loop= face.GetOutterLoop();
	return &outer;
}
TID_COORD3D CTidRawFace::WorkNorm()
{
	return TID_COORD3D(face.WorkNorm);
}
//////////////////////////////////////////////////////////////////////////
// CTidBasicFace
//TID_COORD3D CTidBasicFace::Normal()
//{
//	return basicface.
//}
CTidBasicFace::CTidBasicFace(DWORD id/*=0*/,CSolidBody* pSolidBody/*=NULL*/)
{
	m_id=id;
	m_pSolidBody=pSolidBody;
}
COLORREF CTidBasicFace::Color()
{
	if(basicface.BufferSize==0)
		return 0;
	else
		return basicface.Color;
}
WORD CTidBasicFace::FacetClusterNumber()
{
	if(basicface.BufferSize==0)
		return 0;
	else
		return basicface.FacetClusterNumber;
}
IFacetCluster* CTidBasicFace::FacetClusterAt(int index)
{
	CFacetCluster facet;
	if(!basicface.GetFacetClusterAt(index,facet))
		return NULL;
	CTidFacetCluster* pFacet=listFacets.Add(index+1);
	pFacet->cluster=facet;
	return pFacet;
}
//////////////////////////////////////////////////////////////////////////
// CTidRawSolidEdge
CTidRawSolidEdge::CTidRawSolidEdge(DWORD id/*=0*/,CSolidBody* pSolid/*=NULL*/)
{
	m_pSolid=pSolid;
	m_id=id;
	reverseStartEnd=false;
}
BYTE CTidRawSolidEdge::EdgeType()
{
	return STRAIGHT;
}
//////////////////////////////////////////////////////////////////////////
// CTidSolidBody
#include "SolidBodyBuffer.h"

/*
CTidRawSolidEdge* CreateTidRawSolidEdge(int idClsType,DWORD key,void* pContext)	//����ص�����
{
	return new CTidRawSolidEdge(key,(CSolidBody*)pContext);
}
*/
CTidRawFace* CreateTidRawFace(int idClsType,DWORD key,void* pContext)	//����ص�����
{
	return new CTidRawFace(key,(CSolidBody*)pContext);
}
CTidBasicFace* CreateTidBasicFace(int idClsType,DWORD key,void* pContext)	//����ص�����
{
	return new CTidBasicFace(key,(CSolidBody*)pContext);
}
CTidSolidBody::CTidSolidBody(char* buf/*=NULL*/,DWORD size/*=0*/)
{
	hashEdges.m_pContext=&solid;
	hashEdges.CreateNewAtom=CreateTidRawSolidEdge;
	hashRawFaces.m_pContext=&solid;
	hashRawFaces.CreateNewAtom=CreateTidRawFace;
	hashBasicFaces.m_pContext=&solid;
	hashBasicFaces.CreateNewAtom=CreateTidBasicFace;
	solid.AttachBuffer(buf,size);
}
void CTidSolidBody::AttachBuffer(char* buf/*=NULL*/,DWORD size/*=0*/)
{
	solid.AttachBuffer(buf,size);
}
void CTidSolidBody::CopySolidBuffer(char* buf/*=NULL*/,DWORD size/*=0*/)
{
	solid.CopyBuffer(buf,size);
}
void CTidSolidBody::ReleaseTemporaryBuffer()
{
	hashEdges.Empty();
	hashRawFaces.Empty();
	hashBasicFaces.Empty();
}
#include "./Splitter/glDrawTool.h"
bool Standize(double* vector3d)
{
	double mod_len=vector3d[0]*vector3d[0]+vector3d[1]*vector3d[1]+vector3d[2]*vector3d[2];
	mod_len=sqrt(max(0,mod_len));
	if(mod_len<EPS)
		return false;
	vector3d[0]/=mod_len;
	vector3d[1]/=mod_len;
	vector3d[2]/=mod_len;
	return true;
}
int FloatToInt(double f)
{
	int fi=(int)f;
	return f-fi>0.5?fi+1:fi;
}
int CalArcResolution(double radius,double sector_angle)
{
	double user2scr_scale=1.0;
	int sampling=5;		//Բ������ʱ�ĳ�������(������)
	UINT uMinSlices=36;	//��Բ����ʾ�����ƽ����Ƭ��
	double length=sector_angle*radius;
	int n = FloatToInt((length/user2scr_scale)/sampling);
	int min_n=FloatToInt(uMinSlices*sector_angle/Pi)/2;
	int max_n=FloatToInt(72*sector_angle/Pi);
	if(max_n==0)
		max_n=1;
	if(min_n==0)
		min_n=1;
	n=max(n,min_n);
	n=min(n,max_n);
	return n;
}
void GetArcSimuPolyVertex(f3dArcLine* pArcLine, GEPOINT vertex_arr[], int slices)
{
	int i,sign=1;
	if(pArcLine->ID&0x80000000)	//����
		sign=-1;
	double slice_angle = pArcLine->SectorAngle()/slices;
	if(pArcLine->WorkNorm()==pArcLine->ColumnNorm())	//Բ��
	{
		if(sign<0)
			slice_angle*=-1;
		double sina = sin(slice_angle);	//��Ƭ������
		double cosa = cos(slice_angle);	//��Ƭ������
		vertex_arr[0].Set(pArcLine->Radius());
		for(i=1;i<=slices;i++)
		{
			vertex_arr[i].x=vertex_arr[i-1].x*cosa-vertex_arr[i-1].y*sina;
			vertex_arr[i].y=vertex_arr[i-1].y*cosa+vertex_arr[i-1].x*sina;
		}
		GEPOINT origin=pArcLine->Center();
		GEPOINT axis_x=pArcLine->Start()-origin;
		Standize(axis_x);
		GEPOINT axis_y=pArcLine->WorkNorm()^axis_x;
		for(i=0;i<=slices;i++)
		{
			vertex_arr[i].Set(	origin.x+vertex_arr[i].x*axis_x.x+vertex_arr[i].y*axis_y.x,
								origin.y+vertex_arr[i].x*axis_x.y+vertex_arr[i].y*axis_y.y,
								origin.z+vertex_arr[i].x*axis_x.z+vertex_arr[i].y*axis_y.z);
		}
	}
	else	//��Բ��
	{
		if(sign<0)
		{
			for(i=slices;i>=0;i--)
				vertex_arr[i] = pArcLine->PositionInAngle(i*slice_angle);
		}
		else
		{
			for(i=0;i<=slices;i++)
				vertex_arr[i] = pArcLine->PositionInAngle(i*slice_angle);
		}
	}
}
static f3dPoint GetPolyFaceWorkNorm(CSolidBody* pBody,CRawSolidFace& rawface)
{
	CFaceLoop loop=rawface.GetOutterLoop();
    f3dPoint vec1,vec2,norm;//��ʱ��Ϊ�м���άʸ��
	long n = loop.LoopEdgeLineNum();
	if(n<3)
		return norm;
	//--------------VVVVVVVVV---2.������������û�����ϵucs---------------------
	if(rawface.WorkNorm.IsZero())
	{	//��ָ���������
		f3dArcLine line1,line2;
		for(long start=0;start<n;start++)
		{
			loop.GetLoopEdgeLineAt(pBody,start,line1);
			if(line1.ID&0x80000000)
				Sub_Pnt( vec1, line1.Start(), line1.End());
			else
				Sub_Pnt( vec1, line1.End(), line1.Start());
			Standize(vec1);
			long i,j;
			for(i=start+1;i<n;i++)
			{
				loop.GetLoopEdgeLineAt(pBody,i,line2);
				if(line2.ID&0x80000000)	//����
					Sub_Pnt( vec2, line2.Start(), line2.End());
				else
					Sub_Pnt( vec2, line2.End(), line2.Start());
				Standize(vec2);
				norm = vec1^vec2;
				double dd=norm.mod();
				if(dd>EPS2)
				{
					norm /= dd;	//��λ��
					break;
				}
			}
			if( i==n)
				return norm;	//������涨������(���б߽����غ�)����������
			//ͨ��������ʱ����ϵ���жϷ�����������
			GECS cs;
			cs.origin=line1.Start();
			cs.axis_x=vec1;
			cs.axis_z=norm;
			cs.axis_y=cs.axis_z^cs.axis_x;
			for(j=0;j<n;j++)
			{
				if(j==0||j==i)
					continue;
				loop.GetLoopEdgeLineAt(pBody,start+j,line2);
				f3dPoint vertice=line2.End();
				vertice=cs.TransPToCS(vertice);
				if(vertice.y<-EPS)	//��������ֱ���ϻ򸽽�ʱ��Ӧ����������
					break;
			}
			if(j==n)	//ȫ�����㶼�ڵ�ǰ�ߵ���࣬����ʾ����ķ���Ϊ�����ߣ�����Ӧ����Ѱ��
				break;
		}
	}
	else
		norm=rawface.WorkNorm;
	return norm;
}
void WriteToSolidBuffer(CSolidBodyBuffer& solidbuf,int indexBasicFace,CXhSimpleList<CTidGLFace>& listFacets,COLORREF color,const double* poly_norm)
{
	//д��һ�������Ƭ���ݣ���Ӧһ����������ֳɵ����ɻ�����Ƭ��
	solidbuf.SeekPosition(solidbuf.BasicFaceIndexStartAddr+indexBasicFace*4);
	DWORD basicface_lenpos=solidbuf.GetLength();
	solidbuf.WriteDword(basicface_lenpos);	//��������λ��д��������
	solidbuf.SeekToEnd();
	solidbuf.WriteWord((WORD)0);	//CTidBasicFace�����ݼ�¼����
	solidbuf.Write(&color,4);
	WORD facets_n=0;
	LIST_NODE<CTidGLFace>* pTailNode=listFacets.EnumTail();
	if(pTailNode!=NULL)
		facets_n=(WORD)listFacets.EnumTail()->serial;
	BUFFERPOP stack(&solidbuf,facets_n);
	solidbuf.WriteWord(facets_n);
	for(CTidGLFace* pGLFace=listFacets.EnumObjectFirst();pGLFace;pGLFace=listFacets.EnumObjectNext())
	{
		WORD cluster_buf_n=1+24+2+pGLFace->header.uVertexNum*24;
		solidbuf.WriteWord(cluster_buf_n);
		solidbuf.WriteByte(pGLFace->header.mode);
		if(pGLFace->header.clr_norm&0x02)
		{	//ָ����Ƭ�ط���ֵ
			solidbuf.WriteDouble(pGLFace->nx);
			solidbuf.WriteDouble(pGLFace->ny);
			solidbuf.WriteDouble(pGLFace->nz);
		}
		else	//��Ƭ�ط��߼�Ϊԭʼ������ķ���
			solidbuf.WritePoint(GEPOINT(poly_norm));
		//д������������Ƭ����
		if(pGLFace->header.mode==GL_TRIANGLES)
			solidbuf.WriteWord((WORD)(pGLFace->header.uVertexNum/3));
		else
			solidbuf.WriteWord((WORD)(pGLFace->header.uVertexNum-2));
		solidbuf.Write(pGLFace->m_pVertexCoordArr,pGLFace->header.uVertexNum*24);
		stack.Increment();
	}
	//solidbuf.WriteWord((WORD)stack.Count());
	stack.VerifyAndRestore(true,2);
	WORD wBasicfaceBufLen=(WORD)(solidbuf.GetCursorPosition()-basicface_lenpos-2);
	solidbuf.SeekPosition(basicface_lenpos);
	solidbuf.WriteWord(wBasicfaceBufLen);
	solidbuf.SeekToEnd();
}
bool CTidSolidBody::SplitToBasicFacets()
{
	CRawSolidFace rawface;
	f3dPoint vertice;
	double alpha = 0.6;	//���ǵ���ʾЧ���ľ���ϵ��
	int i,j,n=0,face_n=solid.PolyFaceNum();
	CXhSimpleList<CTidGLFace> listFacets;
	CXhSimpleList<GEPOINT> listVertices;
	GEPOINT *pp;
	//Ǩ��ԭʵ�嶨���ڴ�ͬʱ���ڳ�����������Ƭ�Ļ���ʵ����ʾ���������ռ�
	CSolidBodyBuffer solidbuf;
	solidbuf.Write(solid.BufferPtr(),33);//solid.BufferLength());
	solidbuf.BasicFaceNumber=face_n;	//д�������Ƭ��=ԭʼ���������
	DWORD dwIndexBufSize=(solid.KeyEdgeLineNum()+solid.PolyFaceNum())*4;
	solidbuf.WriteAt(45,solid.BufferPtr()+45,dwIndexBufSize);
	solidbuf.BasicFaceIndexStartAddr=45+dwIndexBufSize;
	solidbuf.SeekToEnd();	//BasicFaceIndexStartAddr����ֵ��䵱ǰ�洢ָ��λ��
	solidbuf.Write(NULL,face_n*4);	//@45+dwIndexBufSize	ʵ�������Ƭ��������д����Ӧ�Ŀ��ֽ�ռλ
	DWORD dwDataBufSize=solid.BufferLength()-solidbuf.VertexDataStartAddr;
	if(solid.BasicGLFaceNum()>0)	//ֻ����ʵ��ԭʼ�����������򣬺���ԭ�еĻ�����Ƭ������
		dwDataBufSize=solid.BasicFaceDataStartAddr()-solidbuf.VertexDataStartAddr;
	long iNewVertexDataStartAddr=solidbuf.GetCursorPosition();	//=VertexDataStartAddr+4*face_n
	long iOldVertexDataStartAddr=solidbuf.VertexDataStartAddr;
	solidbuf.VertexDataStartAddr=iNewVertexDataStartAddr;	//=VertexDataStartAddr+4*face_n
	solidbuf.EdgeDataStartAddr=solidbuf.EdgeDataStartAddr+4*face_n;
	solidbuf.RawFaceDataStartAddr=solidbuf.RawFaceDataStartAddr+4*face_n;
	solidbuf.SeekToEnd();
	solidbuf.Write(solid.BufferPtr()+iOldVertexDataStartAddr,dwDataBufSize);	//д��ԭʵ�嶨����������ڴ�
	//���������ӻ�����Ƭ������¼�������ĺ�����ַλ��ֵ
	int addr_offset=(face_n-solid.BasicGLFaceNum())*4;
	if(addr_offset!=0)
	{	//������λ�������ͼԪ������ָ����ڴ��ַƫ��
		DWORD* RawFaceAddr=(DWORD*)(solidbuf.GetBufferPtr()+solidbuf.RawFaceIndexStartAddr);
		for(i=0;i<face_n;i++)
			*(RawFaceAddr+i)+=addr_offset;
		DWORD* RawEdgeAddr=(DWORD*)(solidbuf.GetBufferPtr()+solidbuf.EdgeIndexStartAddr);
		for(i=0;i<face_n;i++)
			*(RawEdgeAddr+i)+=addr_offset;
	}
	if(solidbuf.BasicFaceDataStartAddr==0)	//��ǰ������Ƭ��Ϊ��
		solidbuf.BasicFaceDataStartAddr=solidbuf.GetCursorPosition();
	else	//��дԭ������Ƭ������
		solidbuf.BasicFaceDataStartAddr=solidbuf.BasicFaceDataStartAddr+4*face_n;
	for(int indexFace=0;indexFace<face_n;indexFace++)
	{
		listFacets.DeleteList();
		listVertices.DeleteList();
		if(!solid.GetPolyFaceAt(indexFace,rawface))
		{
			WriteToSolidBuffer(solidbuf,indexFace,listFacets,0,GEPOINT(0,0,0));
			continue;//return false;
		}
		/*����һ�������棬�϶���һ���⻷�����⻹�������������ڻ������ǵķ��߷�������ͬ��
		  ��Է�����˵���⻷Ӧ����ʱ�뷽�򣬶��ڻ���Ӧ����˳ʱ�뷽��
		*/
		f3dArcLine edgeLine[4];
		GEPOINT poly_norm=rawface.WorkNorm;
		CFaceLoop outerloop=rawface.GetOutterLoop();
		if(outerloop.LoopEdgeLineNum()==3)
		{
			outerloop.GetLoopEdgeLineAt(&solid,0,edgeLine[0]);
			outerloop.GetLoopEdgeLineAt(&solid,1,edgeLine[1]);
			outerloop.GetLoopEdgeLineAt(&solid,2,edgeLine[2]);
			f3dPoint pt_arr[3];
			if(edgeLine[0].ID&0x80000000)	//����
				pt_arr[0] = edgeLine[0].End();
			else
				pt_arr[0] = edgeLine[0].Start();
			if(edgeLine[1].ID&0x80000000)	//����
				pt_arr[1] = edgeLine[1].End();
			else
				pt_arr[1] = edgeLine[1].Start();
			if(edgeLine[2].ID&0x80000000)	//����
				pt_arr[2] = edgeLine[2].End();
			else
				pt_arr[2] = edgeLine[2].Start();
			poly_norm=rawface.WorkNorm;
			if(poly_norm.IsZero())
			{
				f3dPoint vec1=pt_arr[1]-pt_arr[0];
				f3dPoint vec2=pt_arr[2]-pt_arr[1];
				poly_norm=vec1^vec2;
			}
			if(!Standize(poly_norm))
			{
				WriteToSolidBuffer(solidbuf,indexFace,listFacets,rawface.MatColor(),poly_norm);
				continue;//return false;
			}
			//���������淨�ߡ�������ɫ�������������Ϣ
			CTidGLFace *pGLFace=listFacets.AttachObject();
			pGLFace->nx=poly_norm.x;
			pGLFace->ny=poly_norm.y;
			pGLFace->nz=poly_norm.z;
			pGLFace->red   = GetRValue(rawface.MatColor())/255.0f;
			pGLFace->green = GetGValue(rawface.MatColor())/255.0f;
			pGLFace->blue  = GetBValue(rawface.MatColor())/255.0f;
			pGLFace->alpha = (GLfloat)alpha;
			pGLFace->header.uVertexNum=3;
			pGLFace->header.clr_norm=0x03;	//Ĭ�ϱ任��ɫ������
			pGLFace->m_pVertexCoordArr=new GLdouble[9];
			for(j=0;j<3;j++)
			{
				pGLFace->m_pVertexCoordArr[3*j]=pt_arr[j].x;
				pGLFace->m_pVertexCoordArr[3*j+1]=pt_arr[j].y;
				pGLFace->m_pVertexCoordArr[3*j+2]=pt_arr[j].z;
			}
			WriteToSolidBuffer(solidbuf,indexFace,listFacets,rawface.MatColor(),poly_norm);
			continue;
		}
		if(outerloop.LoopEdgeLineNum()==4)
		{
			outerloop.GetLoopEdgeLineAt(&solid,0,edgeLine[0]);
			outerloop.GetLoopEdgeLineAt(&solid,1,edgeLine[1]);
			outerloop.GetLoopEdgeLineAt(&solid,2,edgeLine[2]);
			outerloop.GetLoopEdgeLineAt(&solid,3,edgeLine[3]);
			if(rawface.WorkNorm.IsZero())//->poly_norm.IsZero())
			{
				f3dPoint vec1=edgeLine[0].End()-edgeLine[0].Start();
				f3dPoint vec2=edgeLine[1].End()-edgeLine[1].Start();
				poly_norm=vec1^vec2;
				int sign1=1,sign2=1;
				if(edgeLine[0].ID&0x80000000)
					sign1=-1;
				if(edgeLine[1].ID&0x80000000)
					sign2=-1;
				if(sign1+sign2==0)	//��ű���
					poly_norm*=-1;
			}
			else
				poly_norm=rawface.WorkNorm;//pFace->poly_norm;
			if(!Standize(poly_norm))
			{
				if(edgeLine[0].SectorAngle()>0)
				{
					poly_norm=edgeLine[0].WorkNorm();
					if(edgeLine[0].ID&0x80000000)
						poly_norm*=-1;
				}
				else if(edgeLine[1].SectorAngle()>0)
				{
					poly_norm=edgeLine[1].WorkNorm();
					if(edgeLine[1].ID&0x80000000)
						poly_norm*=-1;
				}
				//TODO: δ���ԭ�⣬�����ǵ��Ĺ��߱߳���
				//edgeLine[0]=NULL;
			}
			if(edgeLine[0].SectorAngle()>0&&edgeLine[1].SectorAngle()==0&&edgeLine[2].SectorAngle()>0&&edgeLine[3].SectorAngle()==0
				&&fabs(edgeLine[0].WorkNorm()*poly_norm)<EPS_COS)
			{
				n=max(edgeLine[0].m_uDisplaySlices,edgeLine[2].m_uDisplaySlices);
				if(n==0)
				{
					int n1=CalArcResolution(edgeLine[0].Radius(),edgeLine[0].SectorAngle());
					int n2=CalArcResolution(edgeLine[2].Radius(),edgeLine[2].SectorAngle());
					n=max(n1,n2);
				}
				n=min(n,200);
				GEPOINT vertex_arr1[200],vertex_arr2[200];
				GetArcSimuPolyVertex(&edgeLine[0],vertex_arr1,n);
				GetArcSimuPolyVertex(&edgeLine[2],vertex_arr2,n);
				// 			double part_angle1=edgeLine[0]->SectorAngle()/n;
				// 			double part_angle2=edgeLine[2]->SectorAngle()/n;
				// 			double posAngle;

				for(i=0;i<n;i++)
				{
					f3dPoint pt_arr[3];
					//����Բ���м��
					//posAngle=edgeLine[0]->SectorAngle()-i*part_angle1;
					pt_arr[0] = vertex_arr1[n-i];//edgeLine[0]->PositionInAngle(posAngle);
					//����Բ���м��
					//posAngle=i*part_angle2;
					pt_arr[1] = vertex_arr2[i];//edgeLine[2]->PositionInAngle(posAngle);
					//����Բ���м��
					//posAngle=(i+1)*part_angle2;
					pt_arr[2] = vertex_arr2[i+1];//edgeLine[2]->PositionInAngle(posAngle);
					f3dPoint axis_x=pt_arr[1]-pt_arr[0];
					f3dPoint axis_y=pt_arr[2]-pt_arr[0];
					poly_norm=axis_x^axis_y;
					Standize(poly_norm);
					//���������淨�ߡ�������ɫ�������������Ϣ
					CTidGLFace *pGLFace=listFacets.AttachObject();
					pGLFace->nx=poly_norm.x;
					pGLFace->ny=poly_norm.y;
					pGLFace->nz=poly_norm.z;
					pGLFace->red   = GetRValue(rawface.MatColor())/255.0f;
					pGLFace->green = GetGValue(rawface.MatColor())/255.0f;
					pGLFace->blue  = GetBValue(rawface.MatColor())/255.0f;
					pGLFace->alpha = (GLfloat)alpha;
					pGLFace->header.uVertexNum=3;
					pGLFace->m_pVertexCoordArr=new GLdouble[9];
					for(j=0;j<3;j++)
					{
						pGLFace->m_pVertexCoordArr[3*j]=pt_arr[j].x;
						pGLFace->m_pVertexCoordArr[3*j+1]=pt_arr[j].y;
						pGLFace->m_pVertexCoordArr[3*j+2]=pt_arr[j].z;
					}

					//����Բ���м��
					//posAngle=(i+1)*part_angle2;
					pt_arr[0] = vertex_arr2[i+1];//edgeLine[2]->PositionInAngle(posAngle);
					//����Բ���м��
					//posAngle=edgeLine[0]->SectorAngle()-(i+1)*part_angle1;
					pt_arr[1] = vertex_arr1[n-i-1];//edgeLine[0]->PositionInAngle(posAngle);
					//����Բ���м��
					//posAngle=edgeLine[0]->SectorAngle()-i*part_angle1;
					pt_arr[2] = vertex_arr1[n-i];//edgeLine[0]->PositionInAngle(posAngle);
					axis_x = pt_arr[1]-pt_arr[0];
					axis_y = pt_arr[2]-pt_arr[0];
					poly_norm=axis_x^axis_y;
					Standize(poly_norm);
					//���������淨�ߡ�������ɫ�������������Ϣ
					pGLFace=listFacets.AttachObject();
					pGLFace->nx=poly_norm.x;
					pGLFace->ny=poly_norm.y;
					pGLFace->nz=poly_norm.z;
					pGLFace->red   = GetRValue(rawface.MatColor())/255.0f;
					pGLFace->green = GetGValue(rawface.MatColor())/255.0f;
					pGLFace->blue  = GetBValue(rawface.MatColor())/255.0f;
					pGLFace->alpha = (GLfloat)alpha;
					pGLFace->header.uVertexNum=3;
					pGLFace->m_pVertexCoordArr=new GLdouble[9];
					for(j=0;j<3;j++)
					{
						pGLFace->m_pVertexCoordArr[3*j]=pt_arr[j].x;
						pGLFace->m_pVertexCoordArr[3*j+1]=pt_arr[j].y;
						pGLFace->m_pVertexCoordArr[3*j+2]=pt_arr[j].z;
					}
				}
				WriteToSolidBuffer(solidbuf,indexFace,listFacets,rawface.MatColor(),poly_norm);
				continue;
			}
			else if(edgeLine[0].SectorAngle()==0&&edgeLine[1].SectorAngle()>0&&edgeLine[2].SectorAngle()==0&&edgeLine[3].SectorAngle()>0
				&&fabs(edgeLine[1].WorkNorm()*poly_norm)<EPS_COS)
			{
				n=max(edgeLine[1].m_uDisplaySlices,edgeLine[3].m_uDisplaySlices);
				if(n==0)
				{
					int n1=CalArcResolution(edgeLine[1].Radius(),edgeLine[1].SectorAngle());
					int n2=CalArcResolution(edgeLine[3].Radius(),edgeLine[3].SectorAngle());
					n=max(n1,n2);
				}
				n=min(n,200);
				GEPOINT vertex_arr1[200],vertex_arr2[200];
				GetArcSimuPolyVertex(&edgeLine[1],vertex_arr1,n);
				GetArcSimuPolyVertex(&edgeLine[3],vertex_arr2,n);
				// 			double part_angle1=edgeLine[1]->SectorAngle()/n;
				// 			double part_angle2=edgeLine[3]->SectorAngle()/n;
				// 			double posAngle;
				glEnable(GL_NORMALIZE);
				glEnable(GL_AUTO_NORMAL);
				for(i=0;i<n;i++)
				{
					f3dPoint pt_arr[3];
					//����Բ���м��
					//posAngle=edgeLine[1]->SectorAngle()-i*part_angle1;
					pt_arr[0] = vertex_arr1[n-i];//edgeLine[1]->PositionInAngle(posAngle);
					//����Բ���м��
					//posAngle=i*part_angle2;
					pt_arr[1] = vertex_arr2[i];//edgeLine[3]->PositionInAngle(posAngle);
					//����Բ���м��
					//posAngle=(i+1)*part_angle2;
					pt_arr[2] = vertex_arr2[i+1];//edgeLine[3]->PositionInAngle(posAngle);
					f3dPoint axis_x=pt_arr[1]-pt_arr[0];
					f3dPoint axis_y=pt_arr[2]-pt_arr[0];
					poly_norm=axis_x^axis_y;
					Standize(poly_norm);
					CTidGLFace *pGLFace=listFacets.AttachObject();
					//���������淨�ߡ�������ɫ�������������Ϣ
					pGLFace->nx=poly_norm.x;
					pGLFace->ny=poly_norm.y;
					pGLFace->nz=poly_norm.z;
					pGLFace->red   = GetRValue(rawface.MatColor())/255.0f;
					pGLFace->green = GetGValue(rawface.MatColor())/255.0f;
					pGLFace->blue  = GetBValue(rawface.MatColor())/255.0f;
					pGLFace->alpha = (GLfloat)alpha;
					pGLFace->header.uVertexNum=3;
					pGLFace->m_pVertexCoordArr=new GLdouble[9];
					for(j=0;j<3;j++)
					{
						pGLFace->m_pVertexCoordArr[3*j]=pt_arr[j].x;
						pGLFace->m_pVertexCoordArr[3*j+1]=pt_arr[j].y;
						pGLFace->m_pVertexCoordArr[3*j+2]=pt_arr[j].z;
					}

					//����Բ���м��
					//posAngle=(i+1)*part_angle2;
					pt_arr[0] = vertex_arr2[i+1];//edgeLine[3]->PositionInAngle(posAngle);
					//����Բ���м��
					//posAngle=edgeLine[1]->SectorAngle()-(i+1)*part_angle1;
					pt_arr[1] = vertex_arr1[n-i-1];//edgeLine[1]->PositionInAngle(posAngle);
					//����Բ���м��
					//posAngle=edgeLine[1]->SectorAngle()-i*part_angle1;
					pt_arr[2] = vertex_arr1[n-i];//edgeLine[1]->PositionInAngle(posAngle);
					axis_x=pt_arr[1]-pt_arr[0];
					axis_y=pt_arr[2]-pt_arr[0];
					poly_norm=axis_x^axis_y;
					Standize(poly_norm);
					//���������淨�ߡ�������ɫ�������������Ϣ
					pGLFace=listFacets.AttachObject();
					pGLFace->nx=poly_norm.x;
					pGLFace->ny=poly_norm.y;
					pGLFace->nz=poly_norm.z;
					pGLFace->red   = GetRValue(rawface.MatColor())/255.0f;
					pGLFace->green = GetGValue(rawface.MatColor())/255.0f;
					pGLFace->blue  = GetBValue(rawface.MatColor())/255.0f;
					pGLFace->alpha = (GLfloat)alpha;
					pGLFace->header.uVertexNum=3;
					pGLFace->m_pVertexCoordArr=new GLdouble[9];
					for(j=0;j<3;j++)
					{
						pGLFace->m_pVertexCoordArr[3*j]=pt_arr[j].x;
						pGLFace->m_pVertexCoordArr[3*j+1]=pt_arr[j].y;
						pGLFace->m_pVertexCoordArr[3*j+2]=pt_arr[j].z;
					}
				}
				WriteToSolidBuffer(solidbuf,indexFace,listFacets,rawface.MatColor(),poly_norm);
				continue;
			}
		}
		CGLTesselator t;
		t.SetFilling(TRUE);
		t.SetWindingRule(GLU_TESS_WINDING_ODD);
		if(poly_norm.IsZero())
			poly_norm=GetPolyFaceWorkNorm(&solid,rawface);
		t.StartDef();
		t.TessNormal(poly_norm.x,poly_norm.y,poly_norm.z);
		//��һ��Ϊ�⻷���μ�B-repģ�ͣ�
		int ei=0,edge_n=outerloop.LoopEdgeLineNum();
		//for(pLine=pFace->outer_edge.GetFirst();pLine!=NULL;pLine=pFace->outer_edge.GetNext())
		f3dArcLine line;
		for(ei=0;ei<edge_n;ei++)
		{
			outerloop.GetLoopEdgeLineAt(&solid,ei,line);
			if(line.SectorAngle()==0)
			{
				if(line.Start()==line.End())
					continue;
				if(line.ID&0x80000000)
					vertice = line.End();
				else
					vertice = line.Start();
				listVertices.AttachObject(vertice);
			}
			else
			{
				if(line.m_uDisplaySlices>0)
					n=line.m_uDisplaySlices;
				else
					n=CalArcResolution(line.Radius(),line.SectorAngle());
				double piece_angle=line.SectorAngle()/n;
				for(i=0;i<n;i++)
				{
					if(line.ID&0x80000000)
					{
						if(i==0)
							vertice=line.End();
						else
							vertice = line.PositionInAngle((n-i-1)*piece_angle);
					}
					else
					{
						if(i==0)
							vertice=line.Start();
						else
							vertice = line.PositionInAngle(i*piece_angle);
					}
					listVertices.AttachObject(vertice);
				}
			}
		}
		for(pp=listVertices.EnumObjectFirst();pp!=NULL;pp=listVertices.EnumObjectNext())
			t.AddVertex(*pp);
		//�ڶ���Ϊ�ڻ�
		//for(pLoop=pFace->inner_loop.GetFirst();pLoop!=NULL;pLoop=pFace->inner_loop.GetNext())
		for(int loopi=0;loopi<rawface.InnerLoopNum();loopi++)
		{
			CFaceLoop innerloop=rawface.GetInnerLoopAt(loopi);
			t.ContourSeparator();	//���߽�����
			edge_n=innerloop.LoopEdgeLineNum();
			//for(pLine=pLoop->loop->GetFirst();pLine!=NULL;pLine=pLoop->loop->GetNext())
			for(ei=0;ei<edge_n;ei++)
			{
				innerloop.GetLoopEdgeLineAt(&solid,ei,line);
				if(line.SectorAngle()==0)
				{
					vertice = line.Start();
					pp=listVertices.AttachObject(vertice);
					t.AddVertex(*pp);
				}
				else
				{
					if(line.m_uDisplaySlices>0)
						n=line.m_uDisplaySlices;
					else
						n=CalArcResolution(line.Radius(),line.SectorAngle());
					double piece_angle=line.SectorAngle()/n;
					for(j=0;j<n;j++)
					{
						if(j==0)
							vertice=line.Start();
						else
							vertice = line.PositionInAngle(j*piece_angle);
						pp=listVertices.AttachObject(vertice);
						t.AddVertex(*pp);
					}
				}
			}
		}
		t.EndDef();
		trianglesBuffer.SeekPosition(0);
		while(trianglesBuffer.GetRemnantSize()>0)
		{
			//���������淨�ߡ�������ɫ�������������Ϣ
			CTidGLFace *pGLFace=listFacets.AttachObject();
			pGLFace->nx=poly_norm.x;
			pGLFace->ny=poly_norm.y;
			pGLFace->nz=poly_norm.z;
			pGLFace->red   = GetRValue(rawface.MatColor())/255.0f;
			pGLFace->green = GetGValue(rawface.MatColor())/255.0f;
			pGLFace->blue  = GetBValue(rawface.MatColor())/255.0f;
			pGLFace->alpha = (GLfloat)alpha;
			CTidGLFace *pPrevGLFace=listFacets.EnumObjectTail();
			pGLFace->header.clr_norm=0x03;	//Ĭ�ϱ任��ɫ������
			if(pPrevGLFace!=NULL)
			{
				if( pPrevGLFace->red==pGLFace->red&&pPrevGLFace->green==pGLFace->green&&
					pPrevGLFace->blue==pGLFace->blue&&pPrevGLFace->alpha==pGLFace->alpha)
					pGLFace->header.clr_norm &= 0x02;
				if( pPrevGLFace->nx==pGLFace->nx&&pPrevGLFace->ny==pGLFace->ny&&pPrevGLFace->nz==pGLFace->nz)
					pGLFace->header.clr_norm &= 0x01;
			}
			trianglesBuffer.ReadByte(&pGLFace->header.mode);
			trianglesBuffer.ReadWord(&pGLFace->header.uVertexNum);
			pGLFace->m_pVertexCoordArr=new GLdouble[pGLFace->header.uVertexNum*3];
			trianglesBuffer.Read(pGLFace->m_pVertexCoordArr,pGLFace->header.uVertexNum*24);
		}
		WriteToSolidBuffer(solidbuf,indexFace,listFacets,rawface.MatColor(),poly_norm);
	}
	solid.CopyBuffer(solidbuf.GetBufferPtr(),solidbuf.GetLength());
	return true;
}
int CTidSolidBody::KeyPointNum()
{
	return solid.KeyPointNum();
}
TID_COORD3D CTidSolidBody::GetKeyPointAt(int i)
{
	GEPOINT vertex=solid.GetKeyPointAt(i);
	return TID_COORD3D(vertex);
}
int CTidSolidBody::KeyEdgeLineNum()
{
	return solid.KeyEdgeLineNum();
}
ITidRawSolidEdge* CTidSolidBody::GetKeyEdgeLineAt(int i)
{
	CTidRawSolidEdge* pEdge=hashEdges.Add(i+1);
	solid.GetKeyEdgeLineAt(i,pEdge->edge);
	return pEdge;
}
bool CTidSolidBody::GetKeyEdgeLineAt(int i,TidArcline& line)
{
	f3dArcLine arcline;
	if(!solid.GetKeyEdgeLineAt(i,arcline))
		return false;
	char linebuf[200]={0};
	DWORD size=arcline.ToByteArr(linebuf);
	line.FromByteArr(linebuf,size);
	return true;
}
int CTidSolidBody::PolyFaceNum()
{
	return solid.PolyFaceNum();
}
IRawSolidFace* CTidSolidBody::GetPolyFaceAt(int i)
{
	CRawSolidFace face;
	if(!solid.GetPolyFaceAt(i,face))
		return false;
	CTidRawFace* pFace=hashRawFaces.Add(i+1);
	pFace->m_pSolidBody=&solid;
	pFace->face=face;
	return pFace;
}
int CTidSolidBody::BasicFaceNum()
{
	return solid.BasicGLFaceNum();
}
ITidBasicFace* CTidSolidBody::GetBasicFaceAt(int i)
{
	CBasicGlFace basicface;
	if(!solid.GetBasicGLFaceAt(i,basicface))
		return NULL;
	CTidBasicFace* pFace=hashBasicFaces.Add(i+1);
	pFace->basicface=basicface;
	return pFace;
}
//��ʵ���װ������ϵfromACS��λ��װ������ϵtoACS
void CTidSolidBody::TransACS(const TID_CS& fromACS,const TID_CS& toACS)
{
	if(solid.IsExternalSolidBuffer())
		solid.ToInternalBuffer();
	UCS_STRU fromacs=ConvertUCSFrom(fromACS),toacs=ConvertUCSFrom(toACS);
	solid.TransACS(fromacs,toacs);
}
void CTidSolidBody::TransFromACS(const TID_CS& fromACS)
{
	if(solid.IsExternalSolidBuffer())
		solid.ToInternalBuffer();
	solid.TransFromACS(ConvertUCSFrom(fromACS));
}
void CTidSolidBody::TransToACS(const TID_CS& toACS)
{
	if(solid.IsExternalSolidBuffer())
		solid.ToInternalBuffer();
	solid.TransToACS(ConvertUCSFrom(toACS));
}
TID_STEELMAT CSteelMaterialLibrary::GetSteelMatAt(int i)
{
	TID_STEELMAT mat;
	mat.cBriefSymbol=matsect.BriefSymbolAt(i);
	matsect.NameCodeAt(i,mat.nameCodeStr);	//�����ַ�������,��������NULL
	return mat;
}
char CSteelMaterialLibrary::QuerySteelBriefSymbol(const char *steelmark)
{
	char nameCode[8]={0};
	for(BYTE i=0;i<matsect.TypeCount();i++)
	{
		matsect.NameCodeAt(i,nameCode);
		if(_stricmp(nameCode,steelmark)==0)
			return matsect.BriefSymbolAt(i);
	}
	return 0;
}
bool CSteelMaterialLibrary::QuerySteelNameCode(char briefmark,char* steelmark)
{
	for(BYTE i=0;i<matsect.TypeCount();i++)
	{
		if(matsect.BriefSymbolAt(i)==briefmark)
			return matsect.NameCodeAt(i,steelmark)>0;
	}
	return false;
}

