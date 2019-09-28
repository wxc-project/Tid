#include "stdafx.h"
#include "Hashtable.h"
#include "3ds.h"

CHashList<C3DSData*> g_hash3DSBySerial;
class C3dsDataLife{
public:
	~C3dsDataLife(){
		for(C3DSData** pp3DS=g_hash3DSBySerial.GetFirst();pp3DS;pp3DS=g_hash3DSBySerial.GetNext())
		{
			if(*pp3DS==NULL)
				continue;
			C3DSData* pModel=(C3DSData*)*pp3DS;
			delete pModel;
		}
		g_hash3DSBySerial.Empty();
	}
};
C3dsDataLife g3dsDataLife;

I3DSData* C3DSFactory::Create3DSInstance()
{
	int iNo=1;
	do{
		if(g_hash3DSBySerial.GetValue(iNo)!=NULL)
			iNo++;
		else	//ÕÒµ½Ò»¸ö¿ÕºÅ
			break;
	}while(true);
	C3DSData* p3ds = new C3DSData(iNo);
	g_hash3DSBySerial.SetValue(iNo,p3ds);
	return p3ds;
};
I3DSData* C3DSFactory::Get3DSFromSerial(long serial)
{
	C3DSData** pp3ds=g_hash3DSBySerial.GetValue(serial);
	if(pp3ds&&*pp3ds!=NULL)
		return *pp3ds;
	else
		return NULL;
}
BOOL C3DSFactory::Destroy(long h)
{
	C3DSData** pp3ds=g_hash3DSBySerial.GetValue(h);
	if(pp3ds==NULL||*pp3ds==NULL)
		return FALSE;
	C3DSData* pModel=(C3DSData*)*pp3ds;
	delete pModel;
	return g_hash3DSBySerial.DeleteNode(h);
}