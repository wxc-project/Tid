// TowerTreeDlg.cpp : implementation file
//<LOCALE_TRANSLATE BY wbt />

#include "stdafx.h"
#include "TID.h"
#include "DialogPanel.h"
#include "TIDDoc.h"
#include "TIDView.h"
#include "MainFrm.h"
#include "TowerTreeDlg.h"
#include "TidCplus.h"
#include "image.h"
#include "XhCharString.h"
#include "ArrayList.h"
#include "SetActiveItemDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

static BOOL VisitViewItem(CTowerTreeCtrl *pTreeCtrl,HTREEITEM hItem)
{	//设置所有视图节点为普通格式
	TREEITEM_INFO *pItem=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hItem);
	//if(pItem&&pItem->itemType==VIEW_CASE)
	//	pTreeCtrl->SetItemState(hItem,0,TVIS_BOLD);
	return FALSE;
}

// CTowerTreeDlg dialog
IMPLEMENT_DYNCREATE(CTowerTreeDlg, CDialog)

CTowerTreeDlg::CTowerTreeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTowerTreeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTowerTreeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hDataRootItem=m_hActiveModuleItem=m_hActiveBlockItem=m_hActiveViewItem=NULL;
	m_hServerContentItem=m_hBodyItem=NULL;
	m_hLayerViewItem=NULL;
}


void CTowerTreeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTowerTreeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
		DDX_Control(pDX, IDC_TREE_CTRL, m_treeCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTowerTreeDlg, CDialog)
	//{{AFX_MSG_MAP(CTowerTreeDlg)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_CTRL, OnDblclkTreeCtrl)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_CTRL, OnRclickTreeCtrl)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_CTRL, OnExpandingTreeCtrl)
	ON_COMMAND(ID_SET_ACTIVE_ITEM, OnSetActiveItem)
	ON_COMMAND(ID_REFRESH_TREE, OnRefreshTree)
	ON_COMMAND(ID_SORT_MODULE_BY_HEIGHT,OnSortModuleByHeight)
	ON_NOTIFY(TVN_KEYDOWN,IDC_TREE_CTRL, OnKeydownTreeCtrl)
	ON_COMMAND(ID_SORT_ITEM, OnSortItem)
	//ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTowerTreeDlg message handlers

BOOL CTowerTreeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	RECT rc;
	GetClientRect(&rc);
	CTowerTreeCtrl *pTreeCtrl = GetTreeCtrl();
	if(pTreeCtrl)
		pTreeCtrl->MoveWindow(&rc);
	//pTreeCtrl->SetSelectItemFunc(_FireSelectItem);
	m_ModelImages.Create(IDB_IL_PROJECT, 16, 1, RGB(0,255,0));
	pTreeCtrl->SetImageList(&m_ModelImages,TVSIL_NORMAL);
	pTreeCtrl->ModifyStyle(0,TVS_HASLINES|TVS_HASBUTTONS);
	RefreshTreeCtrl();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

TREEITEM_INFO* CTowerTreeDlg::InsertOrUpdateItemInfo(TREEITEM_INFO& iteminfo)
{
	UINT id=iteminfo.dwRefData>0?iteminfo.dwRefData:iteminfo.itemType;
	//TODO:附加风荷载不能这样处理
	TREEITEM_INFO* pItemInfo=NULL;//hashItemInfos.GetValue(id);
	if(pItemInfo==NULL)
	{
		pItemInfo=itemInfoList.append(iteminfo);
		//pItemInfo=hashItemInfos.SetValue(id,pItemInfo);
	}
	else
		*pItemInfo=iteminfo;
	return pItemInfo;
}
CTowerTreeCtrl *CTowerTreeDlg::GetTreeCtrl()
{
	return &m_treeCtrl;
}

void CTowerTreeDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	CWnd *pWnd = GetTreeCtrl();
	if(pWnd->GetSafeHwnd())
		pWnd->MoveWindow(0,0,cx,cy);
}

void DeleteAllSubItems(CTreeCtrl* pTreeCtrl,HTREEITEM hParentItem)
{
	if (pTreeCtrl->ItemHasChildren(hParentItem))
	{
		HTREEITEM hChildItem = pTreeCtrl->GetChildItem(hParentItem);
		while (hChildItem != NULL)
		{
			HTREEITEM hNextItem = pTreeCtrl->GetNextItem(hChildItem, TVGN_NEXT);
			pTreeCtrl->DeleteItem(hChildItem);
			hChildItem = hNextItem;
		}
	}
}

void CTowerTreeDlg::ShiftActiveItemState(HTREEITEM hActiveItem,int nType)
{
	CTreeCtrl *pTreeCtrl=GetTreeCtrl();
	if(nType==ACTIVE_MODULE||nType==ACTIVE_DISPVIEW)//呼高激活,视图激活
	{
		HTREEITEM hObjSetItem=m_treeCtrl.GetParentItem(hActiveItem);
		HTREEITEM hSonItem=m_treeCtrl.GetChildItem(hObjSetItem);
		while(hSonItem)
		{
			if(hSonItem==hActiveItem)
				m_treeCtrl.SetItemState(hSonItem,TVIS_BOLD,TVIS_BOLD);
			else
				m_treeCtrl.SetItemState(hSonItem,0,TVIS_BOLD);
			hSonItem=m_treeCtrl.GetNextSiblingItem(hSonItem);
		}
		if(nType==ACTIVE_MODULE)
		{
			m_hActiveModuleItem=hActiveItem;
			if(m_hActiveBlockItem!=NULL)
			{
				pTreeCtrl->SetItemState(m_hActiveBlockItem,0,TVIS_BOLD);
				m_hActiveBlockItem=NULL;
			}
		}
		else if(nType==ACTIVE_DISPVIEW)
			m_hActiveViewItem=hActiveItem;
	}
	else if(nType==ACTIVE_BLOCK)//部件激活
	{
		if(m_hActiveBlockItem!=NULL&&m_hActiveBlockItem!=hActiveItem)
			pTreeCtrl->SetItemState(m_hActiveBlockItem,0,TVIS_BOLD);
		if(m_hActiveBlockItem!=hActiveItem)
		{
			pTreeCtrl->SetItemState(hActiveItem,TVIS_BOLD,TVIS_BOLD);
			m_hActiveBlockItem=hActiveItem;
		}
		else
			pTreeCtrl->SetItemState(m_hActiveBlockItem,TVIS_BOLD,TVIS_BOLD);
		if(m_hActiveModuleItem!=NULL)
		{
			pTreeCtrl->SetItemState(m_hActiveModuleItem,0,TVIS_BOLD);
			m_hActiveModuleItem=NULL;
		}
	}
	pTreeCtrl->RedrawWindow();
}
//查找呼当前激活呼高对应的Item
static HTREEITEM SearchModuleItem(CTreeCtrl *pTreeCtrl)
{
	HTREEITEM hRootItem=pTreeCtrl->GetRootItem();
	HTREEITEM hTaItem=pTreeCtrl->GetChildItem(hRootItem);
	HTREEITEM hModuleItem=pTreeCtrl->GetChildItem(hTaItem);
	while(hModuleItem)
	{
		TREEITEM_INFO *pItemInfo=NULL;
		pItemInfo=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hModuleItem);
		if(pItemInfo && pItemInfo->itemType==MODEL_CASE)
		{
			ITidHeightGroup *pModule=(ITidHeightGroup*)pItemInfo->dwRefData;
			if(pModule->GetSerialId()==1)//TODO:未完待改Ta.m_hActiveModule)
				return hModuleItem;
		}
		hModuleItem=pTreeCtrl->GetNextItem(hModuleItem,TVGN_NEXT);
	}
	return NULL;
}
static HTREEITEM FindTreeNodeByAttachData(CTowerTreeCtrl *pTreeCtrl,HTREEITEM hItem,TREEITEM_INFO &itemInfo)
{
	HTREEITEM  hChildItem = pTreeCtrl->GetChildItem(hItem);
	HTREEITEM hFindItem = NULL;
	TREEITEM_INFO *pInfo=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hItem);
	if(pInfo&&pInfo->itemType==itemInfo.itemType&&itemInfo.dwRefData==NULL)
		hFindItem=hItem;
	else if(itemInfo.dwRefData!=NULL&&(pInfo&&pInfo->itemType==itemInfo.itemType&&pInfo->dwRefData==itemInfo.dwRefData))
		hFindItem=hItem;
	else
	{
		while(hChildItem!=NULL)
		{
			hFindItem=FindTreeNodeByAttachData(pTreeCtrl,hChildItem,itemInfo);
			if(hFindItem)
				break;
			hChildItem = pTreeCtrl->GetNextSiblingItem(hChildItem);
		}
	}
	return hFindItem;
}

HTREEITEM CTowerTreeDlg::SearchTreeItem(TREEITEM_TYPE itemType,void* pObj/*=NULL*/)
{
	HTREEITEM hRootItem=m_treeCtrl.GetRootItem();
	return FindTreeNodeByAttachData(&m_treeCtrl,hRootItem,TREEITEM_INFO(itemType,(DWORD)pObj));
}
//校审时切换呼高，导入TTA文件时缺少基准图元，插入标准面，拷贝接腿
static void RemoveObsoleteSiblingTreeItems(CTowerTreeCtrl *pTreeCtrl,HTREEITEM hStartObsoleteItem)
{	//移除过时的树节点
	HTREEITEM hNextItem=NULL,hItem=hStartObsoleteItem;
	while(hItem)
	{
		hNextItem=pTreeCtrl->GetNextSiblingItem(hItem);
		pTreeCtrl->DeleteItem(hItem);
		hItem=hNextItem;
	}
}
bool CTowerTreeDlg::ShiftActiveItemByObj(void* pObj)
{
	TREEITEM_TYPE itemType=MODEL_GROUP;
	//TODO:未完待改if(pObj->GetClassTypeId()==CLS_HEIGHTMODULE)
		itemType=MODEL_CASE;
	//else if (pObj->GetClassTypeId()==CLS_DISPLAYVIEW)
	//	itemType = VIEW_DEFINEDFOLDER;
	//else if (pObj->GetClassTypeId()==CLS_GEPOINT)
	//	itemType=MODEL_DATUM_POINT_CASE;
	//else if (pObj->GetClassTypeId()==CLS_GELINE)
	//	itemType=MODEL_DATUM_LINE_CASE;
	//else if (pObj->GetClassTypeId()==CLS_GEPLANE)
	//	itemType=MODEL_DATUM_PLANE_CASE;
	//else if (pObj->GetClassTypeId()==CLS_ARCLIFT)
	//	itemType=MODEL_ARCLIFT_CASE;
	//else if (pObj->GetClassTypeId()==CLS_BLOCKREF)
	//	itemType=BLOCKREF_CASE;
	/*else if (pObj->GetClassTypeId()==CLS_WORKCASE)
		itemType=WORKSTATUS_CASE;
	else
		return false;
	HTREEITEM hObjItem=SearchTreeItem(itemType,pObj);
	if(hObjItem==NULL)
		return false;
	if(pObj->GetClassTypeId()==CLS_HEIGHTMODULE)
	{	//需要加粗当前激活呼高
		HTREEITEM hObjSetItem=m_treeCtrl.GetParentItem(hObjItem);
		HTREEITEM hSonItem=m_treeCtrl.GetChildItem(hObjSetItem);
		while(hSonItem)
		{
			if(hSonItem==hObjItem)
				m_treeCtrl.SetItemState(hSonItem,TVIS_BOLD,TVIS_BOLD);
			else
				m_treeCtrl.SetItemState(hSonItem,0,TVIS_BOLD);
			hSonItem=m_treeCtrl.GetNextSiblingItem(hSonItem);
		}
		return true;
	}*/
	return false;
}

void CTowerTreeDlg::RefreshActiveItemStateAndActiveModel(ITidModel *pModel,HTREEITEM hItem)
{
	CTreeCtrl *pTreeCtrl=GetTreeCtrl();
	TREEITEM_INFO *pItemInfo=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hItem);
	if(pItemInfo==NULL)
		return;
	//1.查找需要加粗显示的项
	HTREEITEM hViewItem=NULL,hModuleItem=NULL;
	HTREEITEM hCurModelItem=pTreeCtrl->GetChildItem(m_hDataRootItem);	//默认激活模型项为杆塔
	if(pItemInfo->itemType==MODEL_CASE)
		hModuleItem=hItem;
	else if(pItemInfo->itemType==BLOCK_CASE)
		hCurModelItem=hItem;
	else if(pItemInfo->itemType!=BLOCKREF_CASE)
		return;
	//2.加粗显示指定项
	//if(!pModel->IsTowerModel()&&!((CBlockModel*)pModel)->IsEmbeded()&&hCurModelItem)
	//	ShiftActiveItemState(hCurModelItem,ACTIVE_BLOCK);		//加粗部件项
	//else 
	{	//当前激活模型为杆塔时，查找需要激活的呼高项并加粗
		if(hModuleItem==NULL)	
			hModuleItem=SearchModuleItem(pTreeCtrl);			
		if(hModuleItem)
			ShiftActiveItemState(hModuleItem,ACTIVE_MODULE);	//加粗呼高项
	}
}
HTREEITEM CTowerTreeDlg::GetFirstHeightGroupItem()
{
	CTreeCtrl *pTreeCtrl=GetTreeCtrl();
	HTREEITEM hItem=pTreeCtrl->GetNextItem(m_hBodyItem,TVGN_CHILD);
	while(hItem!=NULL)
	{
		TREEITEM_INFO *pItemInfo=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hItem);
		if(pItemInfo==NULL||pItemInfo->itemType!=MODEL_CASE)
			continue;
		else	//返回找到的第一个呼高组节点
			return hItem;
		hItem=pTreeCtrl->GetNextSiblingItem(hItem);
	}
	return NULL;
}
HTREEITEM CTowerTreeDlg::GetNextHeightGroupItem(HTREEITEM hItem)
{
	CTreeCtrl *pTreeCtrl=GetTreeCtrl();
	hItem=pTreeCtrl->GetNextSiblingItem(hItem);
	TREEITEM_INFO *pItemInfo=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hItem);
	if(pItemInfo!=NULL&&pItemInfo->itemType==MODEL_CASE)
		return hItem;	//返回找到的第一个呼高组节点
	else
		return NULL;
}

bool CTowerTreeDlg::RefreshHeightGroupItem(HTREEITEM hHeightItem)
{
	CTowerTreeCtrl* pTreeCtrl=GetTreeCtrl();
	TREEITEM_INFO* pItemInfo=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hHeightItem);
	if(pItemInfo==NULL||pItemInfo->itemType!=MODEL_CASE||pItemInfo->dwRefData==0)
		return false;
	//本体
	ITidHeightGroup* pModule=(ITidHeightGroup*)pItemInfo->dwRefData;
	CXhChar100 sBodyNo("本体号:%d",pModule->GetSerialId());
	HTREEITEM hSonItem=pTreeCtrl->GetChildItem(hHeightItem);
	if(hSonItem==NULL)
		hSonItem=pTreeCtrl->InsertItem(sBodyNo,PRJ_IMG_TOWERBODY,PRJ_IMG_TOWERBODY,hHeightItem);
	else
		pTreeCtrl->SetItemText(hSonItem,sBodyNo);
	pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_BODY_LEGNO,0));
	pTreeCtrl->SetItemData(hSonItem,(DWORD)pItemInfo);
	//配腿
	CString ss_leg[4],sTemp;
	int nLeg=pModule->GetLegSerialArr(NULL);
	ARRAY_LIST<int> legArr;
	legArr.SetSize(nLeg);
	pModule->GetLegSerialArr(legArr.m_pData);
	for(int iQuad=0;iQuad<4;iQuad++)
	{
		for(int i=0;i<nLeg;i++)
		{
			if(legArr[i]==theApp.m_uiActiveLegSerial[iQuad])
				sTemp.Format("%d(*),",legArr[i]);
			else
				sTemp.Format("%d,",legArr[i]);
			ss_leg[iQuad]+=sTemp;
		}
	}
	for(int iQuad=0;iQuad<4;iQuad++)
	{
		ss_leg[iQuad]=ss_leg[iQuad].Left(ss_leg[iQuad].GetLength()-1);	//去右侧的','
		sTemp.Format("接腿(%d)配材号:",iQuad+1);
		ss_leg[iQuad]=sTemp+ss_leg[iQuad];
		hSonItem=pTreeCtrl->GetNextSiblingItem(hSonItem);
		if(hSonItem==NULL)
			hSonItem=pTreeCtrl->InsertItem(ss_leg[iQuad],PRJ_IMG_TOWERLEG,PRJ_IMG_TOWERLEG,hHeightItem);
		else
			pTreeCtrl->SetItemText(hSonItem,ss_leg[iQuad]);
		if(iQuad==0)
			pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_QUAD1LEG_CFGNO,0));
		else if(iQuad==1)
			pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_QUAD2LEG_CFGNO,0));
		else if(iQuad==2)
			pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_QUAD3LEG_CFGNO,0));
		else if(iQuad==3)
			pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_QUAD4LEG_CFGNO,0));
		else
			pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(0,0));
		pTreeCtrl->SetItemData(hSonItem,(DWORD)pItemInfo);
	}
	return true;
}
struct MODULE_HEIGHT{
	ITidHeightGroup* m_pModule;
	//CFGWORD bodyword;
	CMaxDouble lowestZ;
	MODULE_HEIGHT(ITidHeightGroup* pModule=NULL){m_pModule=pModule;}
};
#include "SortFunc.h"
#include "ArrayList.h"

void CTowerTreeDlg::RefreshModelItem(HTREEITEM hModelItem,ITidModel* pModel)
{
	if(pModel==NULL)
		pModel=gpTidModel;
	if(pModel==NULL)
		return;
	CTreeCtrl *pTreeCtrl=GetTreeCtrl();
	if(pTreeCtrl==NULL)
		return;
	DeleteAllSubItems(pTreeCtrl,hModelItem);
	for(int i=0;i<pModel->HeightGroupCount();i++)
	{
		ITidHeightGroup *pHeightGroup=pModel->GetHeightGroupAt(i);
		CXhChar100 sHeightName;
		pHeightGroup->GetName(sHeightName,sHeightName.GetLengthMax());
		HTREEITEM hParentItem=pTreeCtrl->InsertItem(sHeightName,PRJ_IMG_MODULECASE,PRJ_IMG_MODULECASE,hModelItem);
		TREEITEM_INFO* pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_CASE,(DWORD)pHeightGroup));
		pTreeCtrl->SetItemData(hParentItem,(DWORD)pItemInfo);
		RefreshHeightGroupItem(hParentItem);
		//
		if(pHeightGroup->GetSerialId()==theApp.m_uiActiveHeightSerial)	//当前活动状态模型
		{
			ShiftActiveItemState(hParentItem,ACTIVE_MODULE);
			m_treeCtrl.Expand(hParentItem,TVE_EXPAND);
		}
		pTreeCtrl->Expand(hModelItem,TVE_EXPAND);
	}
}
void CTowerTreeDlg::RefreshTreeCtrl()
{
	TREEITEM_INFO *pItemInfo;
	itemInfoList.Empty();
	hashItemInfos.Empty();
	CString ss;
	CTreeCtrl *pTreeCtrl=GetTreeCtrl();
	pTreeCtrl->DeleteAllItems();
	//模型－杆塔层级
	m_hBodyItem = pTreeCtrl->InsertItem("杆塔",PRJ_IMG_CALMODULE,PRJ_IMG_CALMODULE,TVI_ROOT);
	pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_GROUP,(DWORD)gpTidModel));
	pTreeCtrl->SetItemData(m_hBodyItem,(DWORD)pItemInfo);
	RefreshModelItem(m_hBodyItem,gpTidModel);
	pTreeCtrl->Expand(m_hDataRootItem,TVE_EXPAND);
}

void CTowerTreeDlg::OnDblclkTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnSetActiveItem();//激活对应选项
	*pResult = 0;
}

void CTowerTreeDlg::OnKeydownTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	*pResult = 0;
}

void CTowerTreeDlg::ContextMenu(CWnd *pWnd, CPoint point)
{
	CPoint scr_point = point;
	CTreeCtrl *pTreeCtrl=GetTreeCtrl();
	pTreeCtrl->ClientToScreen(&scr_point);
	HTREEITEM hItem=pTreeCtrl->GetSelectedItem();
	HTREEITEM hParentItem=pTreeCtrl->GetParentItem(hItem);
	CMenu popMenu;
	popMenu.LoadMenu(IDR_ITEM_CMD_POPUP);
	CMenu *pMenu=popMenu.GetSubMenu(0);
	while(pMenu->GetMenuItemCount()>0)
		pMenu->DeleteMenu(0,MF_BYPOSITION);
	TREEITEM_INFO *pItemInfo=NULL;
	if(hItem)
		pItemInfo=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hItem);
	if(pItemInfo==NULL||hItem==NULL)
		return;
	//计算模型
	if(pItemInfo->itemType==MODEL_GROUP)
		pMenu->AppendMenu(MF_STRING,ID_REFRESH_TREE,"刷新");
	else if(pItemInfo->itemType==MODEL_CASE)
		pMenu->AppendMenu(MF_STRING,ID_SET_ACTIVE_ITEM,"激活呼高");
	else if(pItemInfo->itemType==MODEL_QUAD1LEG_CFGNO)
		pMenu->AppendMenu(MF_STRING,ID_SET_ACTIVE_ITEM,"激活接腿");
	else if(pItemInfo->itemType==MODEL_QUAD2LEG_CFGNO)
		pMenu->AppendMenu(MF_STRING,ID_SET_ACTIVE_ITEM,"激活接腿");
	else if(pItemInfo->itemType==MODEL_QUAD3LEG_CFGNO)
		pMenu->AppendMenu(MF_STRING,ID_SET_ACTIVE_ITEM,"激活接腿");
	else if(pItemInfo->itemType==MODEL_QUAD4LEG_CFGNO)
		pMenu->AppendMenu(MF_STRING,ID_SET_ACTIVE_ITEM,"激活接腿");
	else
		return;

	popMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,scr_point.x,scr_point.y,this);
}

void CTowerTreeDlg::OnRclickTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TVHITTESTINFO HitTestInfo;
	GetCursorPos(&HitTestInfo.pt);
	CTowerTreeCtrl *pTreeCtrl = GetTreeCtrl();
	pTreeCtrl->ScreenToClient(&HitTestInfo.pt);
	pTreeCtrl->HitTest(&HitTestInfo);
	BOOL bSelect=FALSE;
	TREEITEM_INFO* pSelInfo=(HitTestInfo.hItem==NULL)?NULL:(TREEITEM_INFO*)pTreeCtrl->GetItemData(HitTestInfo.hItem);
	for(HTREEITEM hSelectItem=pTreeCtrl->GetFirstSelectedItem();hSelectItem;hSelectItem=pTreeCtrl->GetNextSelectedItem(hSelectItem))
	{
		TREEITEM_INFO* pItemInfo=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hSelectItem);
		if(pItemInfo==NULL||pSelInfo==NULL)
			continue;
		if(pSelInfo->itemType!=pItemInfo->itemType)
		{
			bSelect=TRUE;
			break;
		}
	}
	if(pTreeCtrl->GetSelectedCount()==1||bSelect)
		pTreeCtrl->Select(HitTestInfo.hItem,TVGN_CARET);
	//else
	//{
	//	int count=pTreeCtrl->GetSelectedCount();
	//	HTREEITEM hCurrSelItem=pTreeCtrl->GetSelectedItem();
	//	pTreeCtrl->SetItemState(hCurrSelItem,TVIS_SELECTED,TVIS_SELECTED);
	//}
	ContextMenu(this,HitTestInfo.pt);
	*pResult = 0;
}
void CTowerTreeDlg::OnExpandingTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult)
{
	CTreeCtrl *pTreeCtrl = GetTreeCtrl();
	HTREEITEM hItem=pTreeCtrl->GetSelectedItem();
	HTREEITEM hParentItem=hItem!=NULL?pTreeCtrl->GetParentItem(hItem):NULL;
	TREEITEM_INFO *pItemInfo=hItem!=NULL?(TREEITEM_INFO*)pTreeCtrl->GetItemData(hItem):NULL;
	TREEITEM_INFO *pParentItemInfo=hParentItem!=NULL?(TREEITEM_INFO*)pTreeCtrl->GetItemData(hParentItem):NULL;
}
//<DEVELOP_PROCESS_MARK nameId="CTowerTreeDlg.OnEditItemPropItem">

//</DEVELOP_PROCESS_MARK>

void CTowerTreeDlg::OnSetActiveItem() 
{
	CTowerTreeCtrl* pTreeCtrl = GetTreeCtrl();
	HTREEITEM hCurItem=pTreeCtrl->GetSelectedItem();
	TREEITEM_INFO *pItemInfo=NULL;
	pItemInfo=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hCurItem);
	if(pItemInfo==NULL)
		return;
	HTREEITEM hHeightGroup;
	if(pItemInfo->itemType==MODEL_CASE)
		hHeightGroup=hCurItem;
	else
		hHeightGroup=pTreeCtrl->GetParentItem(hCurItem);
	TREEITEM_INFO *pModuleItemInfo=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hHeightGroup);
	ITidHeightGroup *pHeightGroup=NULL;
	for(int i=0;i<gpTidModel->HeightGroupCount();i++)
	{
		pHeightGroup=gpTidModel->GetHeightGroupAt(i);
		if(pModuleItemInfo&&pHeightGroup==(void*)pModuleItemInfo->dwRefData)
			break;
	}
	if(pHeightGroup==NULL)
		return;
	if(pItemInfo->itemType==MODEL_QUAD1LEG_CFGNO||pItemInfo->itemType==MODEL_QUAD2LEG_CFGNO||
		pItemInfo->itemType==MODEL_QUAD3LEG_CFGNO||pItemInfo->itemType==MODEL_QUAD4LEG_CFGNO)
	{
		int nLeg=pHeightGroup->GetLegSerialArr(NULL);
		ARRAY_LIST<int> legArr;
		legArr.SetSize(nLeg);
		pHeightGroup->GetLegSerialArr(legArr.m_pData);
		CSetActiveItemDlg dlg;
		for(int i=0;i<nLeg;i++)
			dlg.m_arrStrList.Add(CXhChar16("%d",legArr[i]));
		if(pItemInfo->itemType==MODEL_QUAD1LEG_CFGNO)
			dlg.m_sActiveItem.Format("%d",theApp.m_uiActiveLegSerial[0]);
		else if(pItemInfo->itemType==MODEL_QUAD2LEG_CFGNO)
			dlg.m_sActiveItem.Format("%d",theApp.m_uiActiveLegSerial[1]);
		else if(pItemInfo->itemType==MODEL_QUAD3LEG_CFGNO)
			dlg.m_sActiveItem.Format("%d",theApp.m_uiActiveLegSerial[2]);
		else if(pItemInfo->itemType==MODEL_QUAD4LEG_CFGNO)
			dlg.m_sActiveItem.Format("%d",theApp.m_uiActiveLegSerial[3]);
		if(dlg.DoModal()!=IDOK)
			return;
		CString ss,sTemp;
		int iQuad=0;
		if(pItemInfo->itemType==MODEL_QUAD1LEG_CFGNO)
		{
			theApp.m_uiActiveLegSerial[0]=atoi(dlg.m_sActiveItem);
			ss="接腿(1)配材号:";
			iQuad=0;
		}
		else if(pItemInfo->itemType==MODEL_QUAD2LEG_CFGNO)
		{
			theApp.m_uiActiveLegSerial[1]=atoi(dlg.m_sActiveItem);
			ss="接腿(2)配材号:";
			iQuad=1;
		}
		else if(pItemInfo->itemType==MODEL_QUAD3LEG_CFGNO)
		{	
			theApp.m_uiActiveLegSerial[2]=atoi(dlg.m_sActiveItem);
			ss="接腿(3)配材号:";
			iQuad=2;
		}
		else if(pItemInfo->itemType==MODEL_QUAD4LEG_CFGNO)
		{	
			theApp.m_uiActiveLegSerial[3]=atoi(dlg.m_sActiveItem);
			ss="接腿(4)配材号:";
			iQuad=3;
		}
		for(int i=0;i<nLeg;i++)
		{
			if(legArr[i]==theApp.m_uiActiveLegSerial[iQuad])
				sTemp.Format("%d(*),",legArr[i]);
			else
				sTemp.Format("%d,",legArr[i]);
			ss+=sTemp;
		}
		ss=ss.Left(ss.GetLength()-1);
		pTreeCtrl->SetItemText(hCurItem,ss);
	}
	else if(pItemInfo->itemType==MODEL_CASE)
		theApp.m_uiActiveHeightSerial=pHeightGroup->GetSerialId();
	if(hHeightGroup)
		ShiftActiveItemState(hHeightGroup,ACTIVE_MODULE);	//加粗呼高项
	//
	CTIDView *pTidView=theApp.GetTIDView();
	ISolidDraw* pSolidDraw=pTidView->SolidDraw();
	if(pSolidDraw)
	{
		pSolidDraw->BuildDisplayList(pTidView);
		pSolidDraw->Draw();
	}
}

//</DEVELOP_PROCESS_MARK>

void CTowerTreeDlg::OnRefreshTree() 
{
	RefreshTreeCtrl();
}

void CTowerTreeDlg::OnOK() 
{
	//确认命令行输入
}

void CTowerTreeDlg::OnCancel() 
{
}

void CTowerTreeDlg::OnSortItem() 
{		
	HTREEITEM hSelItem=m_treeCtrl.GetSelectedItem();
	//SortItem(hSelItem);
}

static int CompareModuleHeight(const MODULE_HEIGHT& height1,const MODULE_HEIGHT& height2)
{
	if(!height1.lowestZ.IsInited()&&height2.lowestZ.IsInited())
		return  1;
	else if(height1.lowestZ.IsInited()&&!height2.lowestZ.IsInited())
		return -1;
	else if(!height1.lowestZ.IsInited()&&!height2.lowestZ.IsInited())
		return 0;
	if(height1.lowestZ.number>height2.lowestZ.number)
		return  1;
	else if(height1.lowestZ.number<height2.lowestZ.number)
		return -1;
	else
		return 0;
}
void CTowerTreeDlg::OnSortModuleByHeight() 
{
	/*ITidHeightGroup* pModule;
	ARRAY_LIST<MODULE_HEIGHT> heightsArr(0,Ta.Module.GetNodeNum());
	for(pModule=Ta.Module.GetFirst();pModule;pModule=Ta.Module.GetNext())
	{
		MODULE_HEIGHT height(pModule);
		height.bodyword.AddBodyLegs(pModule->GetBodyNo());
		heightsArr.append(height);
	}

	for(CLDSNode* pNode=Ta.Node.GetFirst();pNode;pNode=Ta.Node.GetNext())
	{
		for(int i=0;i<heightsArr.GetSize();i++)
		{
			if(heightsArr[i].bodyword.And(pNode->cfgword))
				heightsArr[i].lowestZ.Update(pNode->Position('Z',false));
		}
	}
	CObjNoGroup *pGroup=Ta.NoManager.FromGroupID(MODEL_GROUP);
	pGroup->EmptyNo();
	CBubbleSort<MODULE_HEIGHT>::BubSort(heightsArr.m_pData,heightsArr.GetSize(),CompareModuleHeight);
	DYN_ARRAY<int> fromHeightArr(heightsArr.GetSize()), toHeightArr(heightsArr.GetSize());
	for(int j=0;j<heightsArr.GetSize();j++)
	{
		fromHeightArr[j]=heightsArr[j].m_pModule->GetBodyNo();
		toHeightArr[j]=j+1;
		heightsArr[j].m_pModule->iNo=j+1;
		pGroup->SetNoState(j+1);
	}
	Ta.ReplaceObjBodyNo(fromHeightArr,toHeightArr,heightsArr.GetSize());
	RefreshModelItem(m_hBodyItem,gpTidModel);
	*/
}



BOOL CTowerTreeDlg::RenameSelectedItem(CString sName)
{
	CTreeCtrl *pTreeCtrl = GetTreeCtrl();
	HTREEITEM hItem=pTreeCtrl->GetSelectedItem();
	if(hItem==NULL)
		return FALSE;
	pTreeCtrl->SetItemText(hItem,sName);
	return TRUE;
}
BOOL CTowerTreeDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	CXhChar200 helpStr;
	//PROPLIST_ITEM* pHelpItem=NULL;
	//if(pHelpInfo->iContextType==HELPINFO_MENUITEM)
	//	pHelpItem=GetSelectHelpItem(pHelpInfo->iCtrlId);
	//else
	//	pHelpItem=GetSelectHelpItem();
	//if(pHelpItem==NULL)
	//	pHelpItem=CLDSApp::hashDialogHelpItemById.GetValue(m_nIDHelp);
	//if(pHelpItem!=NULL)
	//{
	//	sprintf(helpStr,"%s::/%s",theApp.m_pszHelpFilePath,(char*)pHelpItem->Url);
	//	HtmlHelp((DWORD_PTR)(char*)helpStr,HH_DISPLAY_TOPIC);
	//}
	return true;
}

