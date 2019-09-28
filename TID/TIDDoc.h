// TIDDoc.h :  CTIDDoc 类的接口
//


#pragma once

class CTIDDoc : public CDocument
{
protected: // 仅从序列化创建
	CTIDDoc();
	DECLARE_DYNCREATE(CTIDDoc)

// 属性
public:

// 操作
public:

// 重写
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
// 实现
public:
	virtual ~CTIDDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	CView* GetView(const CRuntimeClass *pClass);
protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileOpen();
	afx_msg void OnExportSwapInfoFile();
public:
	virtual void DeleteContents();
};


