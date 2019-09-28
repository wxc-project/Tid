#pragma once
#include "Buffer.h"
#include "f_ent_list.h"
class CBufferStack : public IBufferStack
{
	BUFFER_IO* m_pBuff;
	BUFF_POSITION_ITEM itemNow;
	CStack<BUFF_POSITION_ITEM>POS_STACK;
protected:
	virtual void AttachBuffer(BUFFER_IO* pBuffIO);
public:
	CBufferStack();
	virtual ~CBufferStack(void);
	virtual int PushPosition();
	virtual bool PopPosition(int pos=-1);
	virtual bool RestoreNowPosition();
	virtual int GetStackRemnantSize(){return POS_STACK.GetRemnantSize();}
	virtual bool ClearLevelCount(int iLevel);
	virtual bool IncrementLevelCount(int iLevel);
	virtual long LevelCount(int iLevel);
};

