#pragma once

#if defined(_EMBEDDED_XHSTL_API_)
#define XHSTL_API	//内嵌时MODCORE_API什么都不代表
#else
#ifdef STL_EXPORTS
#define XHSTL_API __declspec(dllexport)
#else
#define XHSTL_API __declspec(dllimport)
#endif
#endif

//
struct XHSTL_API IStlData
{
	static const int TYPE_SAVE_ASCII =0;
	static const int TYPE_SAVE_BINARY =1;
	//
	virtual int GetSerial()=0;
	virtual void AddSolidBody(char* solidbuf=NULL,DWORD size=0)=0;
	virtual void SaveFile(const char* path,int nType=TYPE_SAVE_ASCII)=0;
};
class XHSTL_API CStlFactory
{
public:
	static IStlData* CreateStl();
	static IStlData* GetStlFromSerial(long serial);
	static BOOL Destroy(long h);
};
