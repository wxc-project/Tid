#ifndef __TOWER_PTR_LIST_H_
#define __TOWER_PTR_LIST_H_
#include "f_ent_list.h"
#include "list.h"

template <class TYPE,class TYPE_PTR> class CTmaPtrList : public ATOM_LIST<TYPE_PTR>,public IXhSet<TYPE_PTR>
{
	BOOL *m_pModified;
public:
	CTmaPtrList(){m_pModified=NULL;}
	~CTmaPtrList(){;}
public:
	TYPE* append(TYPE* pType)//在节点链表的末尾添加节点
	{
		TYPE **ppTypeObj = ATOM_LIST<TYPE_PTR>::append(pType);
		return *ppTypeObj;
	}
    TYPE* GetNext()
	{
		TYPE **ppType=ATOM_LIST<TYPE_PTR>::GetNext();
		if(ppType)
			return *ppType;
		else
			return NULL;
	}
    TYPE* GetPrev()
	{
		TYPE **ppType=ATOM_LIST<TYPE_PTR>::GetPrev();
		if(ppType)
			return *ppType;
		else
			return NULL;
	}
    TYPE* GetFirst()
	{
		TYPE **ppType=ATOM_LIST<TYPE_PTR>::GetFirst();
		if(ppType)
			return *ppType;
		else
			return NULL;
	}
    TYPE* GetTail()
	{
		TYPE **ppType=ATOM_LIST<TYPE_PTR>::GetTail();
		if(ppType)
			return *ppType;
		else
			return NULL;
	}
    TYPE* GetCursor()
	{
		TYPE **ppType=ATOM_LIST<TYPE_PTR>::GetCursor();
		if(ppType)
			return *ppType;
		else
			return NULL;
	}
	TYPE* SetCursorValue(TYPE* pType)//在节点链表的当前位置更新节点值
	{
		TYPE **ppType=ATOM_LIST<TYPE_PTR>::GetCursor();
		if(ppType)
			return *ppType=pType;
		else
			return NULL;
	}
	TYPE* FromHandle(long handle)	//根据句柄获得节点
	{
		TYPE* pType;
		BOOL bPush=push_stack();
		for(pType=GetFirst();pType!=NULL;pType=GetNext())
		{
			if(pType->handle == handle)
			{
				if(bPush)
					pop_stack();
				return pType;
			}
		}
		if(bPush)
			pop_stack();
		return NULL;
	}
	BOOL DeleteAt(long ii)
	{
		return ATOM_LIST<TYPE_PTR>::DeleteAt(ii);
	}
	BOOL DeleteNode(long handle)
	{
		int hits=0;
		BOOL pushed=push_stack();
		for(TYPE* pType=GetFirst();pType;pType=GetNext())
		{
			if(!IsCursorDeleted()&&pType->handle== handle)
			{
				ATOM_LIST<TYPE_PTR>::DeleteCursor();
				hits++;
			}
		}
		pop_stack(pushed);
		if(hits>0)
			return TRUE;
		else
			return FALSE;
	}
	void Empty(){
		ATOM_LIST<TYPE_PTR>::Empty(); 
	}
	IXhEnumerator* NewEnumerator()
	{
		CXhInternalTemplEnumerator<TYPE>* pInterEnum=(CXhInternalTemplEnumerator<TYPE>*)IXhEnumerator::NewEnumerator();
		for(TYPE_PTR pTypeObj=GetFirst();pTypeObj;pTypeObj=GetNext())
			pInterEnum->AppendObjPtr(pTypeObj);
		return pInterEnum;
	}
public:	//IXhSet重载函数
	virtual TYPE_PTR MoveFirst(){return GetFirst();}
	virtual TYPE_PTR MoveNext(){return GetNext();}
	virtual TYPE_PTR getCurrent(){return GetCursor();}
	virtual UINT  getCount(){return GetNodeNum();}
};

#endif