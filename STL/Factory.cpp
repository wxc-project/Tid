#include "StdAfx.h"
#include "STL.h"
#include "HashTable.h"

CHashList<CSTLData*> g_hashStlBySerial;
class CStlDataLife{
public:
	~CStlDataLife(){
		for(CSTLData** ppSTL=g_hashStlBySerial.GetFirst();ppSTL;ppSTL=g_hashStlBySerial.GetNext())
		{
			if(*ppSTL==NULL)
				continue;
			CSTLData* pModel=(CSTLData*)*ppSTL;
			delete pModel;
		}
		g_hashStlBySerial.Empty();
	}
};
CStlDataLife gStlDataLife;
//////////////////////////////////////////////////////////////////////////
IStlData* CStlFactory::CreateStl()
{
	int iNo=1;
	do{
		if(g_hashStlBySerial.GetValue(iNo)!=NULL)
			iNo++;
		else	//ÕÒµ½Ò»¸ö¿ÕºÅ
			break;
	}while(true);
	CSTLData* pSTL = new CSTLData(iNo);
	g_hashStlBySerial.SetValue(iNo,pSTL);
	return pSTL;
}

IStlData* CStlFactory::GetStlFromSerial(long serial)
{
	CSTLData** ppStl=g_hashStlBySerial.GetValue(serial);
	if(ppStl&&*ppStl!=NULL)
		return *ppStl;
	else
		return NULL;
}
BOOL CStlFactory::Destroy(long h)
{
	CSTLData** ppStl=g_hashStlBySerial.GetValue(h);
	if(ppStl==NULL||*ppStl==NULL)
		return FALSE;
	CSTLData* pModel=(CSTLData*)*ppStl;
	delete pModel;
	return g_hashStlBySerial.DeleteNode(h);
}

