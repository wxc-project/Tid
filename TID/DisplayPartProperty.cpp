#include "StdAfx.h"
#include "TID.h"
#include "TIDView.h"
#include "MainFrm.h"
#include "TowerPropertyDlg.h"
#include "objptr_list.h"
#include "PropertyListOper.h"
#include "AssemblePart.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//static DWORD CFG_NO[32]={ 0X00000001,0X00000002,0X00000004,0X00000008,0X00000010,0X00000020,0X00000040,0X00000080,
//	0X00000100,0X00000200,0X00000400,0X00000800,0X00001000,0X00002000,0X00004000,0X00008000,
//	0X00010000,0X00020000,0X00040000,0X00080000,0X00100000,0X00200000,0X00400000,0X00800000,
//	0X01000000,0X02000000,0X04000000,0X08000000,0X10000000,0X20000000,0X40000000,0X80000000};
//DWORD GetSingleWord(long iNo)
//{
//	if(iNo>0&&iNo<=32)
//		return CFG_NO[iNo-1];
//	else
//		return 0;
//}
CString MakeMaterialMarkSetString()
{
	CString matStr="Q235|Q345|Q390|Q420|";
	return matStr;
}
BOOL CTIDView::DisplayPartProperty(ITidAssemblePart *pAssmblePart)
{
	CTowerPropertyDlg *pPropDlg=((CMainFrame*)AfxGetMainWnd())->GetTowerPropertyPage();
	ITidPart *pTidPart=pAssmblePart?pAssmblePart->GetPart():NULL;
	if(pPropDlg==NULL||pTidPart==NULL)
		return FALSE;
	CAssemblePart assmblePart(pAssmblePart);
	CPropertyList *pPropList = pPropDlg->GetPropertyList();
	CPropertyListOper<CAssemblePart> oper(pPropList,&assmblePart);

	CPropTreeItem* pRootItem=pPropList->GetRootItem();
	const int GROUP_GENERAL=1,GROUP_POS=2;
	pPropDlg->m_arrPropGroupLabel.RemoveAll();
	pPropDlg->m_arrPropGroupLabel.SetSize(2);
	pPropDlg->m_arrPropGroupLabel.SetAt(GROUP_GENERAL-1,"常规");
	pPropDlg->m_arrPropGroupLabel.SetAt(GROUP_POS-1,"装配位置");
	pPropDlg->RefreshTabCtrl(0);
	pPropList->CleanCallBackFunc();
	pPropList->CleanTree();
	pPropList->m_nObjClassTypeId = pTidPart->GetPartType();
	pPropList->m_pObj=pAssmblePart;

	CPropTreeItem *pParentItem=NULL,*pPropItem=NULL;
	//常规属性
	pParentItem=oper.InsertPropItem(pRootItem,"basicInfo");
	pParentItem->m_dwPropGroup=GetSingleWord(GROUP_GENERAL);
	oper.InsertEditPropItem(pParentItem,"m_id");
	oper.InsertEditPropItem(pParentItem,"m_sPartNo");
	oper.InsertEditPropItem(pParentItem,"m_sLayer");
	oper.InsertCmbListPropItem(pParentItem,"m_cMaterial",MakeMaterialMarkSetString());
	oper.InsertEditPropItem(pParentItem,"m_Seg");
	oper.InsertEditPropItem(pParentItem,"m_sSpec");
	if(pAssmblePart->IsHasBriefRodLine())
		oper.InsertEditPropItem(pParentItem,"m_wLength");
	oper.InsertEditPropItem(pParentItem,"m_fWeight");
	pPropList->SetAllSonItemsReadOnly(pParentItem);

	//装配属性
	pParentItem=oper.InsertPropItem(pRootItem,"acs");
	pParentItem->m_dwPropGroup=GetSingleWord(GROUP_POS);
	if(pParentItem)
	{
		pPropItem=oper.InsertButtonPropItem(pParentItem,"acs.origin");
		if(pPropItem)
		{
			oper.InsertEditPropItem(pPropItem,"acs.origin.x");
			oper.InsertEditPropItem(pPropItem,"acs.origin.y");
			oper.InsertEditPropItem(pPropItem,"acs.origin.z");
			pPropItem->m_bHideChildren=TRUE;
		}
		pPropItem=oper.InsertButtonPropItem(pParentItem,"acs.axisX");
		if(pPropItem)
		{
			oper.InsertEditPropItem(pPropItem,"acs.axisX.x");
			oper.InsertEditPropItem(pPropItem,"acs.axisX.y");
			oper.InsertEditPropItem(pPropItem,"acs.axisX.z");
			pPropItem->m_bHideChildren=TRUE;
		}
		pPropItem=oper.InsertButtonPropItem(pParentItem,"acs.axisY");
		if(pPropItem)
		{
			oper.InsertEditPropItem(pPropItem,"acs.axisY.x");
			oper.InsertEditPropItem(pPropItem,"acs.axisY.y");
			oper.InsertEditPropItem(pPropItem,"acs.axisY.z");
			pPropItem->m_bHideChildren=TRUE;
		}
		pPropItem=oper.InsertButtonPropItem(pParentItem,"acs.axisZ");
		if(pPropItem)
		{
			oper.InsertEditPropItem(pPropItem,"acs.axisZ.x");
			oper.InsertEditPropItem(pPropItem,"acs.axisZ.y");
			oper.InsertEditPropItem(pPropItem,"acs.axisZ.z");
			pPropItem->m_bHideChildren=TRUE;
		}
	}
	pPropList->SetAllSonItemsReadOnly(pParentItem);
	pPropList->Redraw();
	return TRUE;
}
BOOL CTIDView::DisplayBoltProperty(ITidAssembleBolt* pAssembleBolt)
{
	CTowerPropertyDlg *pPropDlg=((CMainFrame*)AfxGetMainWnd())->GetTowerPropertyPage();
	IBoltSizeSpec* pBoltSpec=pAssembleBolt?pAssembleBolt->GetTidBolt():NULL;
	if(pPropDlg==NULL||pBoltSpec==NULL)
		return FALSE;
	const int GROUP_GENERAL=1,GROUP_POS=2;
	pPropDlg->m_arrPropGroupLabel.RemoveAll();
	pPropDlg->m_arrPropGroupLabel.SetSize(2);
	pPropDlg->m_arrPropGroupLabel.SetAt(GROUP_GENERAL-1,"常规");
	pPropDlg->m_arrPropGroupLabel.SetAt(GROUP_POS-1,"装配位置");
	pPropDlg->RefreshTabCtrl(0);
	//
	CPropertyList *pPropList=pPropDlg->GetPropertyList();
	pPropList->CleanTree();
	pPropList->m_pObj=pAssembleBolt;
	CAssembleBolt assmbleBolt(pAssembleBolt);
	CPropertyListOper<CAssembleBolt> oper(pPropList,&assmbleBolt);
	CPropTreeItem* pPropItem=NULL,*pParentItem=NULL,*pRootItem=pPropList->GetRootItem();
	//
	pParentItem=oper.InsertPropItem(pRootItem,"basicInfo");
	pParentItem->m_dwPropGroup=GetSingleWord(GROUP_GENERAL);
	pPropItem=oper.InsertEditPropItem(pParentItem,"m_sBoltD");
	pPropItem=oper.InsertEditPropItem(pParentItem,"m_sBoltSpec");
	pPropItem=oper.InsertEditPropItem(pParentItem,"m_sBoltFamily");
	pPropList->SetAllSonItemsReadOnly(pParentItem);
	//装配属性
	pParentItem=oper.InsertPropItem(pRootItem,"acs");
	pParentItem->m_dwPropGroup=GetSingleWord(GROUP_POS);
	//原点
	pPropItem=oper.InsertButtonPropItem(pParentItem,"acs.origin");
	pPropItem->m_bHideChildren=TRUE;
	oper.InsertEditPropItem(pPropItem,"acs.origin.x");
	oper.InsertEditPropItem(pPropItem,"acs.origin.y");
	oper.InsertEditPropItem(pPropItem,"acs.origin.z");
	//X轴
	pPropItem=oper.InsertButtonPropItem(pParentItem,"acs.axisX");
	pPropItem->m_bHideChildren=TRUE;
	oper.InsertEditPropItem(pPropItem,"acs.axisX.x");
	oper.InsertEditPropItem(pPropItem,"acs.axisX.y");
	oper.InsertEditPropItem(pPropItem,"acs.axisX.z");
	//Y轴
	pPropItem=oper.InsertButtonPropItem(pParentItem,"acs.axisY");
	pPropItem->m_bHideChildren=TRUE;
	oper.InsertEditPropItem(pPropItem,"acs.axisY.x");
	oper.InsertEditPropItem(pPropItem,"acs.axisY.y");
	oper.InsertEditPropItem(pPropItem,"acs.axisY.z");
	//Z轴
	pPropItem=oper.InsertButtonPropItem(pParentItem,"acs.axisZ");
	pPropItem->m_bHideChildren=TRUE;
	oper.InsertEditPropItem(pPropItem,"acs.axisZ.x");
	oper.InsertEditPropItem(pPropItem,"acs.axisZ.y");
	oper.InsertEditPropItem(pPropItem,"acs.axisZ.z");	
	pPropList->SetAllSonItemsReadOnly(pParentItem);
	//
	pPropList->Redraw();
	return TRUE;
}
BOOL CTIDView::DisplayAnchorBoltProperty(ITidAssembleAnchorBolt* pAssembleAnchorBolt)
{
	CTowerPropertyDlg *pPropDlg=((CMainFrame*)AfxGetMainWnd())->GetTowerPropertyPage();
	if(pPropDlg==NULL)
		return FALSE;
	const int GROUP_GENERAL=1,GROUP_POS=2;
	pPropDlg->m_arrPropGroupLabel.RemoveAll();
	pPropDlg->m_arrPropGroupLabel.SetSize(2);
	pPropDlg->m_arrPropGroupLabel.SetAt(GROUP_GENERAL-1,"常规");
	pPropDlg->m_arrPropGroupLabel.SetAt(GROUP_POS-1,"装配位置");
	pPropDlg->RefreshTabCtrl(0);
	//
	CPropertyList *pPropList=pPropDlg->GetPropertyList();
	pPropList->CleanTree();
	pPropList->m_pObj=pAssembleAnchorBolt;
	CAssembleAnchorBolt assmbleAnchorBolt(pAssembleAnchorBolt);
	CPropertyListOper<CAssembleAnchorBolt> oper(pPropList,&assmbleAnchorBolt);
	CPropTreeItem* pPropItem=NULL,*pParentItem=NULL,*pRootItem=pPropList->GetRootItem();
	//
	pParentItem=oper.InsertPropItem(pRootItem,"basicInfo");
	pParentItem->m_dwPropGroup=GetSingleWord(GROUP_GENERAL);
	pPropItem=oper.InsertEditPropItem(pParentItem,"d");
	pPropItem=oper.InsertEditPropItem(pParentItem,"wiLe");
	pPropItem=oper.InsertEditPropItem(pParentItem,"wiLa");
	pPropItem=oper.InsertEditPropItem(pParentItem,"wiWidth");
	pPropItem=oper.InsertEditPropItem(pParentItem,"wiThick");
	pPropItem=oper.InsertEditPropItem(pParentItem,"wiHoleD");
	pPropItem=oper.InsertEditPropItem(pParentItem,"wiBPHoleD");
	pPropItem=oper.InsertEditPropItem(pParentItem,"SizeSymbol");
	pPropList->SetAllSonItemsReadOnly(pParentItem);
	//装配属性
	pParentItem=oper.InsertPropItem(pRootItem,"acs");
	pParentItem->m_dwPropGroup=GetSingleWord(GROUP_POS);
	//原点
	pPropItem=oper.InsertButtonPropItem(pParentItem,"acs.origin");
	pPropItem->m_bHideChildren=TRUE;
	oper.InsertEditPropItem(pPropItem,"acs.origin.x");
	oper.InsertEditPropItem(pPropItem,"acs.origin.y");
	oper.InsertEditPropItem(pPropItem,"acs.origin.z");
	//X轴
	pPropItem=oper.InsertButtonPropItem(pParentItem,"acs.axisX");
	pPropItem->m_bHideChildren=TRUE;
	oper.InsertEditPropItem(pPropItem,"acs.axisX.x");
	oper.InsertEditPropItem(pPropItem,"acs.axisX.y");
	oper.InsertEditPropItem(pPropItem,"acs.axisX.z");
	//Y轴
	pPropItem=oper.InsertButtonPropItem(pParentItem,"acs.axisY");
	pPropItem->m_bHideChildren=TRUE;
	oper.InsertEditPropItem(pPropItem,"acs.axisY.x");
	oper.InsertEditPropItem(pPropItem,"acs.axisY.y");
	oper.InsertEditPropItem(pPropItem,"acs.axisY.z");
	//Z轴
	pPropItem=oper.InsertButtonPropItem(pParentItem,"acs.axisZ");
	pPropItem->m_bHideChildren=TRUE;
	oper.InsertEditPropItem(pPropItem,"acs.axisZ.x");
	oper.InsertEditPropItem(pPropItem,"acs.axisZ.y");
	oper.InsertEditPropItem(pPropItem,"acs.axisZ.z");	
	pPropList->SetAllSonItemsReadOnly(pParentItem);
	//
	pPropList->Redraw();
	return TRUE;
}