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
#include "SetActiveHeightLegsDlg.h"

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
	CString ss_leg[4],sTemp;
	CTowerTreeCtrl* pTreeCtrl=GetTreeCtrl();
	TREEITEM_INFO* pItemInfo=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hHeightItem);
	if(pItemInfo==NULL||pItemInfo->itemType!=MODEL_CASE||pItemInfo->dwRefData==0)
		return false;
	ITidHeightGroup* pModule=(ITidHeightGroup*)pItemInfo->dwRefData;
	char strdata[64]={0};
	CXhString xhstr(strdata,64);
	/*
#ifdef AFX_TARG_ENU_ENGLISH
	xhstr.Printf("Anti-theft Bolt Z Scope:%.0f~%.0f",pModule->m_fBurglarLsStartZ,pModule->m_fBurglarLsEndZ);
#else 
	xhstr.Printf("防盗螺栓Z坐标范围:%.0f~%.0f",pModule->m_fBurglarLsStartZ,pModule->m_fBurglarLsEndZ);
#endif
	*/
	HTREEITEM hSonItem=pTreeCtrl->GetChildItem(hHeightItem);
	if(hSonItem==NULL)
		hSonItem=pTreeCtrl->InsertItem(xhstr,PRJ_IMG_MAT_BOLT,PRJ_IMG_MAT_BOLT,hHeightItem);
	else
		pTreeCtrl->SetItemText(hSonItem,xhstr);
	pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_BURGLAR_LS_SCOPE,0));
	pTreeCtrl->SetItemData(hSonItem,(DWORD)pItemInfo);
#ifdef AFX_TARG_ENU_ENGLISH
	xhstr.Printf("Body No:%d",pModule->GetBodyNo());
#else 
	xhstr.Printf("本体号:%d",pModule->GetSerialId());//GetBodyNo());
#endif
	hSonItem=pTreeCtrl->GetNextSiblingItem(hSonItem);
	if(hSonItem==NULL)
		hSonItem=pTreeCtrl->InsertItem(xhstr,PRJ_IMG_TOWERBODY,PRJ_IMG_TOWERBODY,hHeightItem);
	else
		pTreeCtrl->SetItemText(hSonItem,xhstr);
	pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_BODY_LEGNO,0));
	pTreeCtrl->SetItemData(hSonItem,(DWORD)pItemInfo);
	//TODO:未完待改
	/*int nMaxLegs=CFGLEG::MaxLegs();
	for(int i=1;i<=192;i++)
	{
		for(int j=0;j<4;j++)
		{
			if(pModule->m_dwLegCfgWord.IsHasNo(i))
			{
				if(i==pModule->m_arrActiveQuadLegNo[j])
					sTemp.Format("%C(*),",(i-1)%nMaxLegs+'A');
				else
					sTemp.Format("%C,",(i-1)%nMaxLegs+'A');
				ss_leg[j]+=sTemp;
			}
		}
	}
	for(int j=0;j<4;j++)
	{
		ss_leg[j]=ss_leg[j].Left(ss_leg[j].GetLength()-1);	//去右侧的','
#ifdef AFX_TARG_ENU_ENGLISH
		sTemp.Format("Joint Leg(%d)Model Flag:",j+1);
#else 
		sTemp.Format("接腿(%d)配材号:",j+1);
#endif
		ss_leg[j]=sTemp+ss_leg[j];
		hSonItem=pTreeCtrl->GetNextSiblingItem(hSonItem);
		if(hSonItem==NULL)
			hSonItem=pTreeCtrl->InsertItem(ss_leg[j],PRJ_IMG_TOWERLEG,PRJ_IMG_TOWERLEG,hHeightItem);
		else
			pTreeCtrl->SetItemText(hSonItem,xhstr);
		if(j==0)
			pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_QUAD1LEG_CFGNO,0));
		else if(j==1)
			pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_QUAD2LEG_CFGNO,0));
		else if(j==2)
			pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_QUAD3LEG_CFGNO,0));
		else if(j==3)
			pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_QUAD4LEG_CFGNO,0));
		else
			pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(0,0));
		pTreeCtrl->SetItemData(hSonItem,(DWORD)pItemInfo);
	}*/
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
static int CompareModuleSerial(const MODULE_HEIGHT& height1,const MODULE_HEIGHT& height2)
{
	//if(height1.m_pModule->iNo>height2.m_pModule->iNo)
	//	return  1;
	//else if(height1.m_pModule->iNo<height2.m_pModule->iNo)
	//	return -1;
	//else
		return 0;
}
void CTowerTreeDlg::RefreshModelItem(HTREEITEM hModelItem,ITidModel* pModel)
{
	CString ss;
	CTreeCtrl *pTreeCtrl=GetTreeCtrl();
	DeleteAllSubItems(pTreeCtrl,hModelItem);
	HTREEITEM hParentItem,hItem;
	HTREEITEM hSubItem=NULL,hArcLiftItem=NULL;
	if(pModel==NULL)
		pModel=gpTidModel;
	if(pModel==NULL)
		return;
	//TODO:未完待改
	UINT hActModule=theApp.m_uiActiveHeightSerial;
	ARRAY_LIST<MODULE_HEIGHT>heightsArr(0,pModel->HeightGroupCount());
	ITidHeightGroup *pHeight;
	int hI,i,j;
	for(i=0;i<pModel->HeightGroupCount();i++)
		heightsArr.append(MODULE_HEIGHT(pModel->GetHeightGroupAt(i)));
	CBubbleSort<MODULE_HEIGHT>::BubSort(heightsArr.m_pData,heightsArr.GetSize(),CompareModuleSerial);
	for(hI=0;hI<heightsArr.GetSize();hI++)
	{
		HEIGHT_GROUP* pHeighGroup=theApp.hashModelHeights.Add(heightsArr[hI].m_pModule->GetSerialId());
		pHeighGroup->m_pModule=pHeight=heightsArr[hI].m_pModule;
		CXhChar100 heightname;
		pHeight->GetName(heightname,heightname.GetLengthMax());
		ss.Format("%s",(char*)heightname);
		hParentItem =pTreeCtrl->InsertItem(ss,PRJ_IMG_MODULECASE,PRJ_IMG_MODULECASE,hModelItem);
		//RefreshHeightGroupItem(hParentItem,&Ta);
		TREEITEM_INFO* pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_CASE,(DWORD)pHeight));
		pTreeCtrl->SetItemData(hParentItem,(DWORD)pItemInfo);

		CString ss_leg[4],sTemp;
		ss.Format("本体号:%d",pHeight->GetSerialId());
		hItem=pTreeCtrl->InsertItem(ss,PRJ_IMG_TOWERBODY,PRJ_IMG_TOWERBODY,hParentItem);
		pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_BODY_LEGNO,0));
		pTreeCtrl->SetItemData(hItem,(DWORD)pItemInfo);
		BYTE xarrLegCfgBytes[24]={ 0 };
		BYTE xarrConstBytes[8]={ 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80 };
		pHeight->GetConfigBytes(xarrLegCfgBytes);
		int niLegSerial=0;
		memcpy(theApp.m_uiActiveLegSerial,pHeighGroup->m_uiActiveLegSerial,16);
		for(i=1;i<=192;i++)
		{
			int niByteIndex=(i-1)/8;
			int niBitIndex=(i-1)%8;
			if(xarrLegCfgBytes[niByteIndex]&xarrConstBytes[niBitIndex])
			{
				for(j=0;j<4;j++)
				{
					if (niLegSerial==0&&pHeighGroup->m_uiActiveLegSerial[j]<(UINT)i)
						pHeighGroup->m_uiActiveLegSerial[j]=(UINT)i;
					if(i==theApp.m_uiActiveLegSerial[j])
						sTemp.Format("%C(*),",niLegSerial+'A');
					else
						sTemp.Format("%C,",niLegSerial+'A');
					ss_leg[j]+=sTemp;
				}
				niLegSerial++;
			}
		}
		for(j=0;j<4;j++)
		{
			ss_leg[j]=ss_leg[j].Left(ss_leg[j].GetLength()-1);	//去右侧的','
#ifdef AFX_TARG_ENU_ENGLISH
			sTemp.Format("Joint Leg(%d)Model Flag:",j+1);
#else 
			sTemp.Format("接腿(%d)配材号:",j+1);
#endif
			ss_leg[j]=sTemp+ss_leg[j];
			hItem=pTreeCtrl->InsertItem(ss_leg[j],PRJ_IMG_TOWERLEG,PRJ_IMG_TOWERLEG,hParentItem);
			if(j==0)
				pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_QUAD1LEG_CFGNO,0));
			else if(j==1)
				pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_QUAD2LEG_CFGNO,0));
			else if(j==2)
				pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_QUAD3LEG_CFGNO,0));
			else if(j==3)
				pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(MODEL_QUAD4LEG_CFGNO,0));
			else
				pItemInfo=InsertOrUpdateItemInfo(TREEITEM_INFO(0,0));
			pTreeCtrl->SetItemData(hItem,(DWORD)pItemInfo);
		}
		if(pHeight->GetSerialId()==hActModule)	//当前活动状态模型
		{
			ShiftActiveItemState(hParentItem,ACTIVE_MODULE);
			m_treeCtrl.Expand(hParentItem,TVE_EXPAND);
		}

		theApp.m_uiActiveHeightSerial=hActModule;
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
	if(hCurItem)
		pItemInfo=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hCurItem);
	if(pItemInfo==NULL)
		return;
	//CTIDView *pLdsView=(CTIDView*)theApp.GetTIDDoc()->GetView(RUNTIME_CLASS(CTIDView));
	//int nMaxLegs=CFGLEG::MaxLegs();
	if(pItemInfo->itemType==MODEL_CASE)
	{
		ITidHeightGroup *pModule=NULL;
		for(int i=0;i<gpTidModel->HeightGroupCount();i++)
		{
			pModule=gpTidModel->GetHeightGroupAt(i);
			if(pModule==(void*)pItemInfo->dwRefData)
				break;
		}
		if(pModule==NULL)
			return;
		theApp.m_uiActiveHeightSerial=pModule->GetSerialId();
		HEIGHT_GROUP *pHeightGroup=theApp.hashModelHeights.GetValue(theApp.m_uiActiveHeightSerial);
		if (pHeightGroup)
			memcpy(theApp.m_uiActiveLegSerial,pHeightGroup->m_uiActiveLegSerial,16);
		ShiftActiveItemState(hCurItem,ACTIVE_MODULE);
		CTIDView *pTidView=theApp.GetTIDView();
		ISolidDraw* pSolidDraw=pTidView->SolidDraw();
		if(pSolidDraw)
		{
			pSolidDraw->BuildDisplayList(pTidView);
			pSolidDraw->Draw();
		}
	}
	else if( pItemInfo->itemType==MODEL_QUAD1LEG_CFGNO||pItemInfo->itemType==MODEL_QUAD2LEG_CFGNO||
		pItemInfo->itemType==MODEL_QUAD3LEG_CFGNO||pItemInfo->itemType==MODEL_QUAD4LEG_CFGNO)
	{
		CSetActiveHeightLegsDlg dlg;
		HTREEITEM hParentItem=pTreeCtrl->GetParentItem(hCurItem);
		HTREEITEM xarrQuadLegItems[4]={ NULL };
		HTREEITEM hLegItem=pTreeCtrl->GetChildItem(hParentItem);
		while (hLegItem!=NULL)
		{
			TREEITEM_INFO *pItem=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hLegItem);
			CString ss=pTreeCtrl->GetItemText(hLegItem);
			if (pItem->itemType==MODEL_QUAD1LEG_CFGNO)
				xarrQuadLegItems[0]=hLegItem;
			else if(pItem->itemType==MODEL_QUAD2LEG_CFGNO)
				xarrQuadLegItems[1]=hLegItem;
			else if(pItem->itemType==MODEL_QUAD3LEG_CFGNO)
				xarrQuadLegItems[2]=hLegItem;
			else if(pItem->itemType==MODEL_QUAD4LEG_CFGNO)
				xarrQuadLegItems[3]=hLegItem;
			hLegItem=pTreeCtrl->GetNextSiblingItem(hLegItem);
		}

		TREEITEM_INFO *pItem=(TREEITEM_INFO*)pTreeCtrl->GetItemData(hParentItem);
		HEIGHT_GROUP* pHeighGroup;
		for(pHeighGroup=theApp.hashModelHeights.GetFirst();pHeighGroup;pHeighGroup=theApp.hashModelHeights.GetNext())
		{
			if(pItem&&pHeighGroup->m_pModule==(ITidHeightGroup*)pItem->dwRefData)
				break;
		}
		if(pHeighGroup==NULL)
			return;
		dlg.m_idBodyHeight=pHeighGroup->m_pModule->GetSerialId();
		for (int i=0;i<4;i++)
			dlg.xarrActiveLegSerials[i]=(BYTE)pHeighGroup->m_uiActiveLegSerial[i];
		if (dlg.DoModal()==IDOK)
		{
			theApp.m_uiActiveHeightSerial=dlg.m_idBodyHeight;
			//刷新树控件
			BYTE xarrLegCfgBytes[24]={ 0 };
			BYTE xarrConstBytes[8]={ 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80 };
			pHeighGroup->m_pModule->GetConfigBytes(xarrLegCfgBytes);
			int i,j,niLegSerial=0;
			CString ss_leg[4],sTemp;
			for (i=0;i<4;i++)
				theApp.m_uiActiveLegSerial[i]=pHeighGroup->m_uiActiveLegSerial[i]=dlg.xarrActiveLegSerials[i];
			for(i=1;i<=192;i++)
			{
				int niByteIndex=(i-1)/8;
				int niBitIndex=(i-1)%8;
				if(xarrLegCfgBytes[niByteIndex]&xarrConstBytes[niBitIndex])
				{
					for(j=0;j<4;j++)
					{
						if(i==theApp.m_uiActiveLegSerial[j])
							sTemp.Format("%C(*),",niLegSerial+'A');
						else
							sTemp.Format("%C,",niLegSerial+'A');
						ss_leg[j]+=sTemp;
					}
					niLegSerial++;
				}
			}
			for(j=0;j<4;j++)
			{
				ss_leg[j]=ss_leg[j].Left(ss_leg[j].GetLength()-1);	//去右侧的','
	#ifdef AFX_TARG_ENU_ENGLISH
				sTemp.Format("Joint Leg(%d)Model Flag:",j+1);
	#else 
				sTemp.Format("接腿(%d)配材号:",j+1);
	#endif
				ss_leg[j]=sTemp+ss_leg[j];
				if(xarrQuadLegItems[j]!=NULL)
					pTreeCtrl->SetItemText(xarrQuadLegItems[j],ss_leg[j]);
			}
			//if(pHeight->GetSerialId()==hActModule)	//当前活动状态模型
			//{
			//	ShiftActiveItemState(hParentItem,ACTIVE_MODULE);
			//	m_treeCtrl.Expand(hParentItem,TVE_EXPAND);
			//}

			//
			CTIDView *pTidView=theApp.GetTIDView();
			ISolidDraw* pSolidDraw=pTidView->SolidDraw();
			if(pSolidDraw)
			{
				pSolidDraw->BuildDisplayList(pTidView);
				pSolidDraw->Draw();
			}
		}
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

