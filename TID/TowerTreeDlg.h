#if !defined(AFX_TOWERTREEDLG_H__6C9A5FBA_F3DA_4DC1_AFD9_9750BC2D3525__INCLUDED_)
#define AFX_TOWERTREEDLG_H__6C9A5FBA_F3DA_4DC1_AFD9_9750BC2D3525__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TowerTreeDlg.h : header file
//
#include "TowerTreeCtrl.h"
#include "f_ent_list.h"
#include "HashTable.h"
#include "TidCplus.h"
/////////////////////////////////////////////////////////////////////////////
// CTowerTreeDlg dialog
enum TOWER_TREE_CMD{TREE_ITEM_ADD,TREE_ITEM_DELETE,TREE_ITEM_REFRESH};

struct TREEITEM_INFO{
	TREEITEM_INFO(){;}
	TREEITEM_INFO(long type,DWORD dw){itemType=type;dwRefData=dw;}
	long itemType;
	DWORD dwRefData;
};
enum TREEITEM_TYPE{
	NONE_INFO,
	//部件
	BLOCK_REF,
	BLOCK_GROUP,
	BLOCKREF_GROUP,
	BLOCK_CASE,
	BLOCKREF_CASE,
	BLOCK_VIEW_CASE,
	BLOCKREF_VIEW_CASE,
	//计算模型组
	DATA_SPACE,
	MODEL_GROUP,
	MODEL_CASE,
	MODEL_LAND_HIGH,
	MODEL_QUAD1LEG_CFGNO,
	MODEL_QUAD2LEG_CFGNO,
	MODEL_QUAD3LEG_CFGNO,
	MODEL_QUAD4LEG_CFGNO,
	MODEL_BODY_LEGNO,
	MODEL_BURGLAR_LS_SCOPE,
	MODEL_DATUM_POINT_GROUP,
	MODEL_DATUM_POINT_CASE,
	MODEL_DATUM_LINE_GROUP,
	MODEL_DATUM_LINE_CASE,
	MODEL_DATUM_PLANE_GROUP,
	MODEL_DATUM_PLANE_CASE,
	MODEL_ARCLIFT_GROUP,
	MODEL_ARCLIFT_CASE,
};
class CTowerTreeDlg : public CDialog
{
	static const int ACTIVE_MODULE	=0;
	static const int ACTIVE_BLOCK	=1;
	static const int ACTIVE_DISPVIEW=2;
// Construction
	ATOM_LIST<TREEITEM_INFO>itemInfoList;
	CHashSet<TREEITEM_INFO*> hashItemInfos;
	HTREEITEM m_hServerContentItem,m_hBodyItem;
	HTREEITEM m_hDataRootItem,m_hActiveModuleItem,m_hActiveBlockItem,m_hActiveViewItem;
	HTREEITEM m_hLayerViewItem;
	void ShiftActiveItemState(HTREEITEM hActiveModelItem,int nType);
	void RefreshActiveItemStateAndActiveModel(ITidModel *pModel,HTREEITEM hItem);
	TREEITEM_INFO* InsertOrUpdateItemInfo(TREEITEM_INFO& iteminfo);
	HTREEITEM SearchTreeItem(TREEITEM_TYPE itemType,void* pObj=NULL);
public:
	//HTREEITEM InsertBlockItem(CBlockModel *pBlock);
	//void SortItem(HTREEITEM hSelItem);
	CTowerTreeDlg(CWnd* pParent = NULL);   // standard constructor
	DECLARE_DYNCREATE(CTowerTreeDlg)
	CImageList m_ModelImages;
	CTowerTreeCtrl m_treeCtrl;
	int m_iCurModuleNo;
	int m_iCurStatusNo;
// Operations
	bool ShiftActiveItemByObj(void* pObj);
	HTREEITEM GetModuleItemByNo(long iModuleNo);
	void ContextMenu(CWnd *pWnd, CPoint point);
	HTREEITEM GetFirstHeightGroupItem();
	HTREEITEM GetNextHeightGroupItem(HTREEITEM hItem);
	bool RefreshHeightGroupItem(HTREEITEM hHeightGroupItem);
	void RefreshModelItem(HTREEITEM hModelItem,ITidModel* pModel);
	void RefreshTreeCtrl();
	void DeleteItem(HTREEITEM hItem);
	//void OnRButtonDown(UINT nFlags, CPoint point);
	CTowerTreeCtrl *GetTreeCtrl();
	BOOL RenameSelectedItem(CString sName);
	long InsertArcLiftItem(HTREEITEM hArcLiftGropItem,f3dPoint* pRodOrg=NULL,f3dPoint* pDatumLiftPos=NULL);
	BOOL ActivateLayerViewItem(int iViewType);	//激活图层视图 0.透视图 1.前视图 2.后视图 3.左视图 4.右视图 5.俯视图
// Dialog Data
	//{{AFX_DATA(CTowerTreeDlg)
	enum { IDD = IDD_MODEL_TREE_DLG};//IDD_TOWER_STRUCTURE_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTowerTreeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTowerTreeDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblclkTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnExpandingTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetActiveItem();
	afx_msg void OnRefreshTree();
	afx_msg void OnSortModuleByHeight();
	afx_msg void OnKeydownTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortItem();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOWERTREEDLG_H__6C9A5FBA_F3DA_4DC1_AFD9_9750BC2D3525__INCLUDED_)
