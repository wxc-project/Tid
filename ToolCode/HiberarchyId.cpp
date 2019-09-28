#include "StdAfx.h"
#include "HiberarchyId.h"

static DWORD MASKINT4_ARR[32]={
	0X00000001,0X00000002,0X00000004,0X00000008,0X00000010,0X00000020,0X00000040,0X00000080,
	0X00000100,0X00000200,0X00000400,0X00000800,0X00001000,0X00002000,0X00004000,0X00008000,
	0X00010000,0X00020000,0X00040000,0X00080000,0X00100000,0X00200000,0X00400000,0X00800000,
	0X01000000,0X02000000,0X04000000,0X08000000,0X10000000,0X20000000,0X40000000,0X80000000};
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
union HIBERINT64{
	_int64 id64;
	BYTE idbytes[8];
public:
	HIBERINT64(_int64 i64=0){id64=i64;}
};
HIBERARCHY::HIBERARCHY(_int64 hiberid/*=0*/)
{
	memcpy(this,&hiberid,8);
}
_int64 HIBERARCHY::Hiber64() const
{
	_int64 i8;
	memcpy(&i8,this,8);
	return i8;
}
DWORD HIBERARCHY::HiberUpId(int hiberarchy1or2) const	//hiber表示正向的层级，只能是1或2
{
	DWORD hiberid=0;
	HIBERINT64 hiberint;
	memcpy(hiberint.idbytes,hiberarchy,6);
	BYTE bitpos=((holder[0]>>4)+(holder[0]&0x0f))*2;
	BYTE place=(holder[1]&0x0f)*2;
	if(hiberarchy1or2==2)
	{
		bitpos+=place;
		place=((holder[1]&0xf0)>>4)*2;
		hiberid=(DWORD)(hiberint.id64>>bitpos);
	}
	else
		hiberid=(DWORD)(hiberint.id64>>bitpos);
	DWORD mask=0;
	if(place>0&&place<=32)	//0xffffffff>>32=0xffffffff
		mask=(0xffffffff>>(32-place));
	hiberid&=mask;
	return hiberid;
}

DWORD HIBERARCHY::HiberDownId(int hiberarchy1or2) const	//hiber表示负向的层级，只能是1或2
{
	DWORD hiberid=0;
	HIBERINT64 hiberint;
	memcpy(hiberint.idbytes,hiberarchy,6);
	BYTE bitpos=0,place=(holder[0]&0x0f)*2;
	if(hiberarchy1or2==2)
	{
		bitpos+=place;
		place=((holder[0]&0xf0)>>4)*2;
		hiberid=(DWORD)(hiberint.id64>>bitpos);
	}
	else
		hiberid=(DWORD)(hiberint.id64>>bitpos);
	DWORD mask=0;
	if(place>0&&place<=32)	//0xffffffff>>32=0xffffffff
		mask=(0xffffffff>>(32-place));
	hiberid&=mask;
	return hiberid;
}
DWORD HIBERARCHY::ModifyUpId(int hiberarchy1or2,DWORD hiber)	//hiber表示正向的层级，只能是1或2
{
	BYTE bitpos=(holder[0]>>4)+(holder[0]&0x0f);
	BYTE place=holder[1]&0x0f;
	if(hiberarchy1or2==2)
	{
		bitpos+=place*2;
		place=(holder[1]&0xf0)>>4;
	}
	DWORD mask4=MASKINT4_ARR[place*2]-1;
	if(hiber>mask4)
	{	//需要扩充该层级占位长度
		if(hiberarchy1or2==1)
		{
			SetHiberarchy(hiber,HiberUpId(2),HiberDownId(1),HiberDownId(2));
			return HiberUpId(1);
		}
		else
		{
			SetHiberarchy(HiberUpId(1),hiber,HiberDownId(1),HiberDownId(2));
			return HiberUpId(2);
		}
	}
	else
	{
		//先获取仅相关占位置１掩码
		HIBERINT64 zeromask=mask4;
		zeromask.id64<<=bitpos;
		//通过异或将占位置１掩码(mask)变为占位置0掩码(zeromask)
		for(int i=0;i<8;i++)
			zeromask.idbytes[i]^=0xff;
		HIBERINT64 hiberint,hiber_new(hiber);
		memcpy(hiberint.idbytes,hiberarchy,6);
		hiberint.id64&=zeromask.id64;
		hiber_new.id64<<=bitpos;
		hiberint.id64|=hiber_new.id64;
		return hiber;
	}
}
DWORD HIBERARCHY::ModifyDownId(int hiberarchy1or2,DWORD hiber)	//hiber表示负向的层级，只能是1或2
{
	BYTE bitpos=0;
	BYTE place=holder[0]&0x0f;
	if(hiberarchy1or2==2)
	{
		bitpos+=place*2;
		place=(holder[0]&0xf0)>>4;
	}
	DWORD mask4=MASKINT4_ARR[place*2]-1;
	if(hiber>mask4)
	{	//需要扩充该层级占位长度
		if(hiberarchy1or2==1)
		{
			SetHiberarchy(HiberUpId(1),HiberUpId(2),hiber,HiberDownId(2));
			return HiberUpId(1);
		}
		else
		{
			SetHiberarchy(HiberUpId(1),HiberUpId(2),HiberDownId(1),hiber);
			return HiberUpId(2);
		}
	}
	else
	{
		//先获取仅相关占位置１掩码
		HIBERINT64 zeromask=mask4;
		zeromask.id64<<=bitpos;
		//通过异或将占位置１掩码(mask)变为占位置0掩码(zeromask)
		for(int i=0;i<8;i++)
			zeromask.idbytes[i]^=0xff;
		HIBERINT64 hiberint,hiber_new(hiber);
		memcpy(hiberint.idbytes,hiberarchy,6);
		hiberint.id64&=zeromask.id64;
		hiber_new.id64<<=bitpos;
		hiberint.id64|=hiber_new.id64;
		return hiber;
	}
}
bool HIBERARCHY::SetHiberarchy(DWORD up1/*=0*/,DWORD up2/*=0*/,DWORD down1/*=0*/,DWORD down2/*=0*/)
{
	DWORD hiberarr[4]={down1,down2,up1,up2};
	holder[0]=holder[1]=hiberarchy[0]=hiberarchy[1]=hiberarchy[2]=hiberarchy[3]=hiberarchy[4]=hiberarchy[5]=0;
	BYTE bitpos=0;
	for(int i=0;i<4;i++)
	{
		int updown=i/2;
		BYTE place=0;
		if(hiberarr[i]==0)
			place=0;
		else if(hiberarr[i]<=0x0000000F)
			place=0x02;
		else if(hiberarr[i]<=0x000000FF)
			place=0x04;
		else if(hiberarr[i]<=0x00000FFF)
			place=0x06;
		else if(hiberarr[i]<=0x0000FFFF)
			place=0x08;
		else if(hiberarr[i]<=0x000FFFFF)
			place=0x0A;
		else if(hiberarr[i]<=0x00FFFFFF)
			place=0x0C;
		else if(hiberarr[i]<=0x0FFFFFFF)
			place=0x0E;
		else //if(up1<=0x3FFFFFFF)
			place=0x0F;
		//根据当前及之前的占位信息按位将相应的层级Id值拷贝至层级存储位hiber中
		BYTE bytepos=bitpos/8;
		HIBERINT64 wid;
		wid.id64=hiberarr[i];
		wid.id64<<=bitpos;
		for(int j=bytepos;j<6;j++)
			hiberarchy[j]|=wid.idbytes[j];
		bitpos+=place*2;
		if(bitpos>48)	//超出预定层级深度
			return false;
		if(i%2>0)
			holder[updown]|=place<<4;
		else
			holder[updown] =place;
	}
	return true;
}
bool HIBERID::IsEqual(const HIBERID& hiberid)
{
	if(masterId!=hiberid.masterId)
		return false;
	else if(Hiber64()!=hiberid.Hiber64())
		return false;
	else
		return true;
}
HIBERID::HIBERID(long master_id, HIBERARCHY hiberachy) : HIBERARCHY(hiberachy)
{
	masterId=master_id;
}
