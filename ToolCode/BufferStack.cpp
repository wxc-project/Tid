#include "stdafx.h"
#include "BufferStack.h"

CBufferStack::CBufferStack()
{
	m_pBuff=NULL;
	itemNow.lPushPosition=itemNow.lNowPosition=0;
}
CBufferStack::~CBufferStack(void)
{
	if(m_pBuff&&m_pBuff->BufferStack()==this)
		m_pBuff->DetachStack();
}

void CBufferStack::AttachBuffer(BUFFER_IO* pBuffIO)
{
	m_pBuff=pBuffIO;
	itemNow.lPushPosition=itemNow.lNowPosition=pBuffIO->GetCursorPosition();
}
int CBufferStack::PushPosition()
{
	if(m_pBuff)
	{
		itemNow.lPushPosition=itemNow.lNowPosition=m_pBuff->GetCursorPosition();
		itemNow.counts=0;
		itemNow.level=POS_STACK.push(itemNow);
		POS_STACK.TopAtom()->level=itemNow.level;
		return itemNow.level;
	}
	else
		return 0;//false
}
bool CBufferStack::PopPosition(int pos/*=-1*/)
{
	if(pos==0)
		return false;
	else if(pos>0&&pos!=POS_STACK.GetPushSize())
		return false;
	if(POS_STACK.pop(itemNow))
	{
		itemNow.lNowPosition=m_pBuff->GetCursorPosition();
		m_pBuff->SeekPosition(itemNow.lPushPosition);
		return true;
	}
	else
		return false;
}
bool CBufferStack::RestoreNowPosition()
{
	return m_pBuff->SeekPosition(itemNow.lNowPosition);
}

//只是创建IBufferStack类型的缓存位置栈，需要用户自行delete
IBufferStackPtr CreateBufferStack(BUFFER_IO* pBuffIO){
	IBufferStack* pPosStack = new CBufferStack();
	pPosStack->AttachBuffer(pBuffIO);
	return pPosStack;
}
struct CBufferStackRegisterSelf{
	CBufferStackRegisterSelf(){	CBuffer::CreateBufferStackFunc=CreateBufferStack; }
} RegBufferStack;
bool CBufferStack::ClearLevelCount(int iLevel)
{
	BUFF_POSITION_ITEM* pPosItem=POS_STACK.TopAtom();
	bool inited=false;
	if(pPosItem!=NULL&&pPosItem->level==iLevel)
	{
		pPosItem->counts=0;
		inited=true;
	}
	if(itemNow.level==iLevel)
	{
		itemNow.counts=0;
		inited=true;
	}
	return inited;
}
bool CBufferStack::IncrementLevelCount(int iLevel)
{
	BUFF_POSITION_ITEM* pPosItem=POS_STACK.TopAtom();
	bool inited=false;
	if(pPosItem!=NULL&&pPosItem->level==iLevel)
	{
		pPosItem->counts++;
		inited=true;
	}
	if(itemNow.level==iLevel)
	{
		itemNow.counts++;
		inited=true;
	}
	return inited;
}
long CBufferStack::LevelCount(int iLevel)
{
	BUFF_POSITION_ITEM* pPosItem=POS_STACK.TopAtom();
	if(itemNow.level==iLevel)
		return itemNow.counts;
	else if(pPosItem!=NULL&&pPosItem->level==iLevel)
		return pPosItem->counts;
	else
		return 0;
}
