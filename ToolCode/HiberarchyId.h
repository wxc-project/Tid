#pragma once

#include "WinDef.h"
//多层级结构的层级Id, 各层级的Id值必须小于等于0x3fffffff
class HIBERARCHY{
protected:
	BYTE holder[2];	//holder[0]负向层级占位，后四位0x0f表示-1级占位，前四级0xf0表示-2级占位；holder[1]表示正向层级占位，具体与负向层级占位类同
	BYTE hiberarchy[6];	//各层级的实际id存储位,由低到高依次存储：-1级占位=>-2级占位=>+1级占位=>+2级占位
public:
	HIBERARCHY(DWORD up1,DWORD up2,DWORD down1,DWORD down2){SetHiberarchy(up1,up2,down1,down2);}
	HIBERARCHY(_int64 hiberid=0);
	_int64 Hiber64() const;
	DWORD HiberUpId(int hiberarchy1or2) const;	//hiber表示正向的层级，只能是1或2
	DWORD HiberDownId(int hiberarchy1or2) const;	//hiber表示负向的层级，只能是1或2
	DWORD ModifyUpId(int hiberarchy1or2,DWORD hiber);	//hiber表示正向的层级，只能是1或2
	DWORD ModifyDownId(int hiberarchy1or2,DWORD hiber);	//hiber表示负向的层级，只能是1或2
	bool SetHiberarchy(DWORD up1=0,DWORD up2=0,DWORD down1=0,DWORD down2=0);
	bool IsEqual(const HIBERARCHY& hiberarchy) const{return Hiber64()==hiberarchy.Hiber64();}
};
struct HIBERID : public HIBERARCHY{
	DWORD masterId;	//数据对象的主操作级(０级)标识Id
	HIBERID(long master_id=0){masterId=master_id;}
	HIBERID(long master_id, HIBERARCHY hiberachy);
	bool IsEqual(const HIBERID& hiberid);
};
