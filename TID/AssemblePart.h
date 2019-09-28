#pragma once
#include "TidCplus.h"
#include "PropListItem.h"

class CAssemblePart
{
private:
	ITidAssemblePart *m_pAssemblePart;
	ITidPart *m_pTidPart;
public:
	CAssemblePart(ITidAssemblePart *pAssemblePart);
	int GetPropValueStr(long id, char *valueStr,UINT nMaxStrBufLen=100);	//通过属性Id获取属性值
	DECLARE_PROP_FUNC(CAssemblePart)
};
class CAssembleBolt
{
private:
	ITidAssembleBolt *m_pAssembleBolt;
	IBoltSizeSpec *m_pTidBolt;
public:
	CAssembleBolt(ITidAssembleBolt *pAssembleBolt);
	int GetPropValueStr(long id, char *valueStr,UINT nMaxStrBufLen=100);	//通过属性Id获取属性值
	DECLARE_PROP_FUNC(CAssembleBolt)
};
class CAssembleAnchorBolt
{
private:
	ITidAssembleAnchorBolt *m_pAssembleAnchor;
	IAnchorBoltSpec* m_pAnchorBolt;
public:
	CAssembleAnchorBolt(ITidAssembleAnchorBolt *pAssembleAnchor);
	int GetPropValueStr(long id, char *valueStr,UINT nMaxStrBufLen=100);	//通过属性Id获取属性值
	DECLARE_PROP_FUNC(CAssembleAnchorBolt)
};

