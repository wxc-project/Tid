// TIDCore.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "TIDCore.h"
#include "TIDModel.h"

/*#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif*/

//////////////////////////////////////////////////////////////////////////
// CTidModel
static IAssembly* CreateAssembly(int idClsType,DWORD key,void* pContext)
{
	IAssembly *pAssembly=NULL;
	switch (idClsType){
	case IAssembly::BOLT:
		pAssembly=new BOLT_ASSEMBLY();
		break;
	case IAssembly::PART:
		pAssembly=new PART_ASSEMBLY();
		break;
	case IAssembly::BLOCK:
		pAssembly=new BLOCK_ASSEMBLY();
		break;
	default:
		break;
	}
	return pAssembly;
}
static BOOL DeleteAssembly(IAssembly *pAssembly)
{
	if(pAssembly==NULL)
		return FALSE;
	switch (pAssembly->GetType()){
	case IAssembly::BOLT:
		delete (BOLT_ASSEMBLY*)pAssembly;
		break;
	case IAssembly::PART:
		delete (PART_ASSEMBLY*)pAssembly;
		break;
	case IAssembly::BLOCK:
		delete (BLOCK_ASSEMBLY*)pAssembly;
		break;
	default:
		break;
	}
	return TRUE;
}
CTidModel::CTidModel(long serial/*=0*/)
{
	m_iSerial=serial;
	m_dwAssmblyIter=1;
	m_dwBoltCount=m_dwPartCount=m_dwBlockCount=0;
	m_hashAssemblyByIter.CreateNewAtom=CreateAssembly;
	m_hashAssemblyByIter.DeleteAtom=DeleteAssembly;
}
CTidModel::~CTidModel()
{
	m_hashAssemblyByIter.Empty();
}
GECS CTidModel::ModelCoordSystem()
{
	return m_xTidBuffer.ModelCoordSystem();
}
IAssembly* CTidModel::GetPartAssemblyByIndex(DWORD index)
{
	if(index<1||index>m_dwPartCount)
		return NULL;
	PART_ASSEMBLY assembly;
	m_xAssemblyParts.GetAssemblyAt(index,true,assembly);
	PART_ASSEMBLY *pAssembly=(PART_ASSEMBLY*)m_hashAssemblyByIter.Add(0,IAssembly::PART);
	*pAssembly=assembly;
	return pAssembly;
}
IAssembly* CTidModel::GetBoltAssemblyByIndex(DWORD index)
{
	if(index<1||index>m_dwBoltCount)
		return NULL;
	BOLT_ASSEMBLY assembly=m_xAssemblyBolts.GetAssemblyAt(index,true);
	BOLT_ASSEMBLY *pAssembly=(BOLT_ASSEMBLY*)m_hashAssemblyByIter.Add(0,IAssembly::BOLT);
	*pAssembly=assembly;
	return pAssembly;
}
IAssembly* CTidModel::GetBlockAssemblyByIndex(DWORD index)
{
	if(index<1||index>m_dwBlockCount)
		return NULL;
	BLOCK_ASSEMBLY assembly=m_xAssembleSection.GetAssemblyAt(index);
	BLOCK_ASSEMBLY *pAssembly=(BLOCK_ASSEMBLY*)m_hashAssemblyByIter.Add(0,IAssembly::BLOCK);
	*pAssembly=assembly;
	return pAssembly;
}
IAssembly* CTidModel::EnumAssemblyFirst()
{
	m_dwAssmblyIter=1;
	m_hashAssemblyByIter.Empty();
	return EnumAssemblyNext();
}
IAssembly* CTidModel::EnumAssemblyNext()
{
	IAssembly *pAssembly=NULL;
	if(m_dwAssmblyIter<=m_dwPartCount)
		pAssembly=GetPartAssemblyByIndex(m_dwAssmblyIter);
	else if(m_dwAssmblyIter<=m_dwPartCount+m_dwBoltCount)
		pAssembly=GetBoltAssemblyByIndex(m_dwAssmblyIter-m_dwPartCount);
	else if(m_dwAssmblyIter<=m_dwPartCount+m_dwBoltCount+m_dwBlockCount)
		pAssembly=GetBlockAssemblyByIndex(m_dwAssmblyIter-m_dwPartCount-m_dwBoltCount);
	m_dwAssmblyIter++;
	return pAssembly;
}
IDbPart* CTidModel::GetDbPart(int indexId)
{
	return NULL;
}
void CTidModel::Init(const char* src_buf,long buf_len)
{
	m_xTidBuffer.InitBuffer(src_buf,buf_len);
	m_xPartSection=m_xTidBuffer.PartSection();
	m_xBoltSection=m_xTidBuffer.BoltSection();
	m_xBlockSection=m_xTidBuffer.BlockSection();
	m_xAssembleSection=m_xTidBuffer.AssembleSection();
	m_xAssemblyParts=m_xAssembleSection.PartSection();
	m_xAssemblyBolts=m_xAssembleSection.BoltSection();
	m_dwPartCount=m_xAssemblyParts.AssemblyCount();
	m_dwBoltCount=m_xAssemblyBolts.AssemblyCount();
	m_dwBlockCount=m_xBlockSection.GetBlockCount();
	//
	PART_ASSEMBLY::m_pPartSection=&m_xPartSection;
	BOLT_ASSEMBLY::m_pBoltSection=&m_xBoltSection;
	BLOCK_ASSEMBLY::m_pPartSection=&m_xPartSection;
	BLOCK_ASSEMBLY::m_pBoltSection=&m_xBoltSection;
	BLOCK_ASSEMBLY::m_pAssemblyParts=&m_xAssemblyParts;
	BLOCK_ASSEMBLY::m_pAssemblyBolts=&m_xAssemblyBolts;
}

//////////////////////////////////////////////////////////////////////////
// CTidModelFactory
CHashPtrList<CTidModel> g_hashTidModels;
class CTidModelsLife{
public:
	~CTidModelsLife(){g_hashTidModels.Empty();}
};
CTidModelsLife modelsLife;

ITidModel* CTidModelFactory::CreateTidModel()
{
	int iNo=1;
	do{
		if(g_hashTidModels.GetValue(iNo)!=NULL)
			iNo++;
		else	//找到一个空号
			break;
	}while(true);
	CTidModel* pModel = g_hashTidModels.Add(iNo);
	return pModel;
	return NULL;
};
ITidModel* CTidModelFactory::TidModelFromSerial(long serial)
{
	return g_hashTidModels.GetValue(serial);
}
BOOL CTidModelFactory::Destroy(long serial)
{
	for(CTidModel *pTidModel=g_hashTidModels.GetFirst();pTidModel;pTidModel=g_hashTidModels.GetNext())
	{
		if(pTidModel->GetSerialId()==serial)
			return g_hashTidModels.DeleteCursor(TRUE);
	}
	return FALSE;
}