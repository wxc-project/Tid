// TIDView.h : CTIDView 类的接口
//
#pragma once
#include "I_DrawSolid.h"
#include "TIDDoc.h"
#include "CommandDefOper.h"
#include "TidCplus.h"

class CTIDView : public CView
{
	BYTE m_cCurTask;
	TASK_TYPE m_curTask;		//当前任务类型
	IDrawSolid* m_pDrawSolid;
	ISolidDraw *m_pSolidDraw;
	ISolidSet *m_pSolidSet;
	ISolidSnap *m_pSolidSnap;
	ISolidOper *m_pSolidOper;
	CWinThread* m_pPipeTaskThread;
protected: // 仅从序列化创建
	CTIDView();
	DECLARE_DYNCREATE(CTIDView)

// 属性
public:
	CTIDDoc* GetDocument() const;
	ISolidDraw* SolidDraw(){return m_pSolidDraw;}
	ISolidSet * SolidSet(){return m_pSolidSet;}
	ISolidSnap* SolidSnap(){return m_pSolidSnap;}
	ISolidOper* SolidOper(){return m_pSolidOper;}

// 操作
public:

// 重写
	public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CTIDView();
	void UpdatePropertyPage();
	BOOL DisplayPartProperty(ITidAssemblePart* pAssemblePart);
	BOOL DisplayBoltProperty(ITidAssembleBolt* pAssembleBolt);
	BOOL DisplayAnchorBoltProperty(ITidAssembleAnchorBolt* pAssembleAnchorBolt);
	void OperOther();
	static UINT StartPipeTaskListen(LPVOID lpPara);
	void UpdatePipeData(CBuffer& tid_buf);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnInitialUpdate();
protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGlAllZoom();
	afx_msg void OnGlOpenWindow();
	afx_msg void OnUpdateGlOpenWindow(CCmdUI *pCmdUI);
	afx_msg void OnGlPan();
	afx_msg void OnUpdateGlPan(CCmdUI *pCmdUI);
	afx_msg void OnGlZoom();
	afx_msg void OnUpdateGlZoom(CCmdUI *pCmdUI);
	afx_msg void OnGlRotated();
	afx_msg void OnUpdateGlRotated(CCmdUI *pCmdUI);
	afx_msg void OnResetView();
	afx_msg void OnSolidModeDisplay();
	afx_msg void OnUpdateSolidModeDisplay(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg LRESULT ObjectSnappedProcess(WPARAM ID, LPARAM ent_type);
	afx_msg LRESULT ObjectSelectProcess(WPARAM nSelect, LPARAM other);
};

#ifndef _DEBUG  // TIDView.cpp 的调试版本
inline CTIDDoc* CTIDView::GetDocument() const
   { return reinterpret_cast<CTIDDoc*>(m_pDocument); }
#endif

