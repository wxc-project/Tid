// ColorSelectComboBox.cpp : implementation file
//<LOCALE_TRANSLATE BY wbt />

#include "stdafx.h"
#include "ColorSelectComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorSelectComboBox

CColorSelectComboBox::CColorSelectComboBox()
{
}

CColorSelectComboBox::~CColorSelectComboBox()
{
}


BEGIN_MESSAGE_MAP(CColorSelectComboBox, CComboBox)
	//{{AFX_MSG_MAP(CColorSelectComboBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorSelectComboBox message handlers

void CColorSelectComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	// TODO: Add your code to determine the size of specified item
}

void CColorSelectComboBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your code to draw the specified item
	if(lpDrawItemStruct->CtlType!=ODT_COMBOBOX)
		return;
	if(lpDrawItemStruct->itemID <0)
		return;
	COLORREF	itemColor=GetItemData(lpDrawItemStruct->itemID);//项目的颜色
	COLORREF	textColor=::GetSysColor( COLOR_WINDOWTEXT );//文本的颜色
	COLORREF	backColor=::GetSysColor(COLOR_WINDOW);//项目背景色
	CString		itemString;//存储项目字符串
	//得到项目的包围矩形
	CRect	itemRect=lpDrawItemStruct->rcItem;
	CDC	*pDC=CDC::FromHandle (lpDrawItemStruct->hDC );
	if( lpDrawItemStruct->itemState & ODS_FOCUS )
	{
		//如果项目具有输入焦点,则设置背景色为用系统高亮色(通常为蓝色)
		backColor=::GetSysColor(COLOR_HIGHLIGHT);
		textColor= 0x00FFFFFF & ~( textColor );
	}
	if(  lpDrawItemStruct->itemState & ODS_DISABLED )
	{
		//如果控件被禁止,则设置文本色和项目颜色为禁止色
		textColor = ::GetSysColor( COLOR_INACTIVECAPTIONTEXT );
		itemColor=textColor;
	}
	//先绘制背景
	pDC->FillRect (&itemRect,&CBrush(backColor));
	if( lpDrawItemStruct->itemState & ODS_FOCUS )
		pDC->DrawFocusRect( &itemRect);	//如果项目具有输入焦点,则还要画出一个焦点框
	//设置颜色区域
	CRect	colorRect;
	colorRect.left=itemRect.left+COLOR_RECT_BORDER;
	colorRect.top=itemRect.top+COLOR_RECT_BORDER;
	colorRect.right=colorRect.left+COLOR_RECT_WIDTH;
	colorRect.bottom=itemRect.bottom - COLOR_RECT_BORDER;
	//画出颜色区域
	if(itemColor!=0xFFFFFFFF&&itemColor!=0xEFFFFFFF)
	{
		CBrush	brush(itemColor);
		CBrush	*oldbrush=pDC->SelectObject (&brush);
		pDC->Rectangle (&colorRect);
		pDC->SelectObject (oldbrush);
		brush.DeleteObject();
	}
	else
	{
		CBrush *oldbrush = (CBrush*)pDC->SelectStockObject(HOLLOW_BRUSH);
		pDC->Rectangle(&colorRect);
		pDC->MoveTo(colorRect.left,colorRect.top);
		pDC->LineTo(colorRect.right,colorRect.bottom);
		pDC->MoveTo(colorRect.left,colorRect.bottom);
		pDC->LineTo(colorRect.right,colorRect.top);
		pDC->SelectObject(oldbrush);
	}

	//得到并画出文字
	if((int)lpDrawItemStruct->itemID>=0)
	{
		GetLBText(lpDrawItemStruct->itemID, itemString );
		itemRect.OffsetRect (2*COLOR_RECT_BORDER+COLOR_RECT_WIDTH+5,0);
		pDC->SetBkMode(TRANSPARENT);//设置文字输出模式为透明背景
		pDC->SetTextColor (textColor);
		DrawText(lpDrawItemStruct->hDC, itemString, -1, &itemRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	}
}

int CColorSelectComboBox::AddColor(COLORREF rgbColor, LPCTSTR sComment,BOOL bAppend)
{
	if(IsHaveThisColor(rgbColor)!=-1)
		return -1;
	int n = GetCount();
	int nItem;
	if(bAppend)
		nItem = AddString(sComment);
	else
		nItem=InsertString(n-1,sComment);
	if(nItem >=0)
		SetItemData (nItem,rgbColor);
	return	nItem;
}

int CColorSelectComboBox::AddString(LPCTSTR lpszItem)
{
	return ((CComboBox*)this)->AddString(lpszItem);
}

int CColorSelectComboBox::IsHaveThisColor(COLORREF rgbColor)
{
	int ntotal=GetCount( );
	for(int i=0;i<ntotal;i++)
	{
		if(GetItemData(i)==rgbColor)
			return	i;
	}
	return -1;
}

void CColorSelectComboBox::InitBox(COLORREF crColor)
{
#ifdef AFX_TARG_ENU_ENGLISH
	AddColor (RGB(255,0,0),"Red");
	AddColor (RGB(0,255,0),"Green");
	AddColor (RGB(0,0,255),"Blue");
	AddColor (RGB(255,255,0),"Yellow");
	AddColor (RGB(255,0,255)," Purplish Red");
	AddColor (RGB(0,255,255),"Cyan");
	AddColor (RGB(128,0,0),"Dark Red");
	AddColor (RGB(0,128,0),"Dark Green");
	AddColor (RGB(0,0,128),"Dark Blue");
	AddColor (RGB(128,128,0),"Dark Yellow");
	AddColor (RGB(128,0,128),"Dark Purple");
	AddColor (RGB(0,128,128),"Dark Cyan");
	AddColor (RGB(255,255,255),"White");
	AddColor (RGB(0,0,0),"Black");
#else 
	AddColor (RGB(255,0,0),"红");
	AddColor (RGB(0,255,0),"绿");
	AddColor (RGB(0,0,255),"蓝");
	AddColor (RGB(255,255,0),"黄");
	AddColor (RGB(255,0,255),"紫红");
	AddColor (RGB(0,255,255),"青");
	AddColor (RGB(128,0,0),"暗红");
	AddColor (RGB(0,128,0),"暗绿");
	AddColor (RGB(0,0,128),"暗蓝");
	AddColor (RGB(128,128,0),"暗黄");
	AddColor (RGB(128,0,128),"暗紫");
	AddColor (RGB(0,128,128),"暗青");
	AddColor (RGB(255,255,255),"白");
	AddColor (RGB(0,0,0),"黑");
#endif
	AddColor (0XFFFFFFFF,"....");
	AddColor (0XEFFFFFFF,"拾取颜色");
	int iCur = IsHaveThisColor(crColor);
	if(iCur>=0)
		SetCurSel (iCur);
	else
	{
		iCur = AddColor(crColor,"自定义",FALSE);
		SetCurSel(iCur);
	}
}

COLORREF CColorSelectComboBox::GetSelColor()
{
	int nSelectedItem=this->GetCurSel ();
	return GetItemData(nSelectedItem);
}


BOOL CColorSelectComboBox::PreTranslateMessage(MSG* pMsg) 
{
	if( pMsg->message == WM_KEYDOWN )	
	{		
		if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{			
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			GetParent()->SetFocus();
			return 1;
		}	
	}
	return CComboBox::PreTranslateMessage(pMsg);
}
