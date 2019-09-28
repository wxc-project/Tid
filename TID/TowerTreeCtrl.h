#if !defined(AFX_TOWERTREECTRL_H__60830DE4_02E4_4E38_9016_1A7412A838A3__INCLUDED_)
#define AFX_TOWERTREECTRL_H__60830DE4_02E4_4E38_9016_1A7412A838A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TowerTreeCtrl.h : header file
//
//#include "SuperDropTarget.h"

/////////////////////////////////////////////////////////////////////////////
// CTowerTreeCtrl window
#define		TVGN_EX_ALL				0x000F
#define		DRAG_DELAY				60	//拖放延迟时间
class CTowerTreeCtrl : public CTreeCtrl
{
	UINT        m_nTimerTicks;		//处理滚动的定时器所经过的时间
	UINT        m_nScrollTimerID;	//处理滚动的定时器
	CPoint      m_ptHover;			//鼠标位置
	UINT        m_nHoverTimerID;	//鼠标敏感定时器
	DWORD       m_dwDragStart;		//按下鼠标左键那一刻的时间
	BOOL        m_bDragging;		//标识是否正在拖动过程中
	CImageList* m_pDragImage;		//拖动时显示的图象列表
	HTREEITEM   m_hItemDragS;		//被拖动的节点
	HTREEITEM	m_hItemDragD;		//接受拖动的节点
	BOOL		m_bSelectPending;
	CPoint		m_ptClick;
	HTREEITEM	m_hClickedItem;
	HTREEITEM	m_hFirstSelectedItem;
	BOOL		m_bSelectionComplete;
	
	DROPEFFECT (*DragOverFunc)(CTreeCtrl *pTreeCtrl, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL (*DropFunc)(CTreeCtrl *pTreeCtrl, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	//树节点之间进行拖放时:用来判断拖放源节点是否合法的函数
	BOOL (*FireIsValidDragS)(CTowerTreeCtrl *pTreeCtrl, HTREEITEM hItemDragS);
	//树节点之间进行拖放时:用来判断拖放目标节点是否合法的函数
	BOOL (*FireIsValidDragD)(CTowerTreeCtrl *pTreeCtrl, HTREEITEM hItemDragS, HTREEITEM hItemDragD);
	//树节点之间进行拖放时:用来比较源节点与目标节点数据类型级别的函数
	//返回值：<0 源节点数据级别低于目标节点  =0 级别相等  >0 源节点数据级别大于目标节点
	int (*FireCompareDataLevel)(CTowerTreeCtrl *pTreeCtrl, HTREEITEM hItemDragS, HTREEITEM hItemDragD);
	//遍历指定节点的所有子节点时，对节点进行操作的函数 返回TRUE则中止遍历
	int (*FireVisitItem)(CTowerTreeCtrl *pTreeCtrl, HTREEITEM hItem);
	//选中指定节点时，对节点触发的操作函数
	int (*FireSelectItem)(CTowerTreeCtrl *pTreeCtrl, HTREEITEM hItem);
// Construction
public:
	CTowerTreeCtrl();
	DECLARE_DYNCREATE(CTowerTreeCtrl)
// Attributes

// Operations
public:
	void SetDragOverFunc(DROPEFFECT (*func)(CTreeCtrl *pTreeCtrl, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)){DragOverFunc = func;}
	void SetDropFunc(BOOL (*func)(CTreeCtrl *pTreeCtrl, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)){DropFunc = func;}
	void SetIsValidDragSFunc(BOOL (*func)(CTowerTreeCtrl *pTreeCtrl, HTREEITEM hItemDragS)){FireIsValidDragS = func;}
	void SetIsValidDragDFunc(BOOL (*func)(CTowerTreeCtrl *pTreeCtrl, HTREEITEM hItemDragS, HTREEITEM hItemDragD)){FireIsValidDragD = func;}
	void SetCompareDataLevelFunc(BOOL (*func)(CTowerTreeCtrl *pTreeCtrl, HTREEITEM hItemDragS, HTREEITEM hItemDragD)){FireCompareDataLevel = func;}
	void SetVisitItemFunc(BOOL (*func)(CTowerTreeCtrl *pTreeCtrl, HTREEITEM hItem)){FireVisitItem = func;}
	void SetSelectItemFunc(BOOL (*func)(CTowerTreeCtrl *pTreeCtrl, HTREEITEM hItem)){FireSelectItem = func;}
	void TraverseItem(HTREEITEM hItem);
	HTREEITEM CopyItem(HTREEITEM hBranch, HTREEITEM hNewParent, HTREEITEM hAfter);
	HTREEITEM CopyBranch(HTREEITEM hBranch, HTREEITEM hNewParent, HTREEITEM hAfter);
	//HTREEITEM GetDragSourceItem(){return m_hItemDragS;}
public:
	UINT GetSelectedCount() const;
	HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode);
	HTREEITEM GetFirstSelectedItem();
	HTREEITEM GetNextSelectedItem(HTREEITEM hItem);
	HTREEITEM GetPrevSelectedItem(HTREEITEM hItem);
	HTREEITEM ItemFromData(DWORD dwData, HTREEITEM hStartAtItem=NULL) const;

	BOOL SelectItemEx(HTREEITEM hItem, BOOL bSelect=TRUE);
	BOOL SelectItems(HTREEITEM hFromItem, HTREEITEM hToItem);
	void ClearSelection(BOOL bMultiOnly=FALSE);
protected:
  void SelectMultiple( HTREEITEM hClickedItem, UINT nFlags, CPoint point );
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTowerTreeCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	//CSuperDropTarget m_dropTarget;
	BOOL m_bLMouseDown;
	virtual ~CTowerTreeCtrl();
	virtual	DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropList, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	//virtual void OnDragLeave(CWnd* pWnd);
protected:
	//{{AFX_MSG(CTowerTreeCtrl)
	afx_msg void OnDestroy();
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnSetfocus(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnKillfocus(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);	
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOWERTREECTRL_H__60830DE4_02E4_4E38_9016_1A7412A838A3__INCLUDED_)
