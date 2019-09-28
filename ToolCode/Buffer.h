// Buffer.h: interface for the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUFFER_H__10945DBC_F91B_433A_97DD_482C7BC01747__INCLUDED_)
#define AFX_BUFFER_H__10945DBC_F91B_433A_97DD_482C7BC01747__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <stdio.h> //定义FILE类型

struct IBufferStack;
struct BUFFER_IO{
public:	//以下两函数纯粹是为了实现property,property中不能直接指向虚函数
	DWORD InternalCursorPosition(){return GetCursorPosition();}
	DWORD InternalLength(){return GetLength();}
public:
	virtual DWORD Read(void *pch, DWORD size)=0;
	virtual DWORD Write(const void *pch,DWORD size)=0;

	virtual DWORD GetCursorPosition()=0;
	__declspec( property(get=InternalCursorPosition)) DWORD CursorPosition;//获取缓存当前存取位置

	virtual DWORD GetLength()=0;
	__declspec( property(get=InternalLength)) DWORD Length;//获取已读(写)缓存的有效长度
	virtual DWORD ReadBuffer(BUFFER_IO *buffer_io,DWORD size,char* pool=NULL,DWORD pool_size=0);
	virtual DWORD WriteBuffer(BUFFER_IO *buffer_io,DWORD size,char* pool=NULL,DWORD pool_size=0);
	virtual bool SeekPosition(DWORD pos)=0;
	virtual bool SeekOffset(DWORD offset){return false;}
	virtual bool SeekToBegin()=0;
	virtual bool SeekToEnd()=0;
	//缓存当前位置压栈出栈操作
	virtual IBufferStack* BufferStack(){return NULL;}
	virtual bool AttachStack(IBufferStack* pPosStack){return false;}
	virtual void DetachStack(){;}
	virtual int PushPositionStack();				//将当前缓存位置压栈
	virtual bool PopPositionStack(int push_pos=-1);	//弹出至相配对的PushStack时缓存位置
	virtual bool RestoreNowPosition();		//恢复PopStack之前的缓存位置
	virtual bool ClearLevelCount(int iLevel);
	virtual bool IncrementLevelCount(int iLevel);
	virtual long LevelCount(int iLevel);
	//典型数据类型的快整读写
	virtual DWORD ReadByte(BYTE *byte){return Read(byte,1);}
	virtual DWORD ReadByte(char *ch){return Read(ch,1);}
	virtual BYTE ReadByte(){
		BYTE cbValue=0;
		Read(&cbValue,1);
		return cbValue;
	}
	virtual void ReadBoolean(bool *b);	//为避免更改代码全局编译，实现代码移至Buffer.cpp wjh-2014.5.15
	virtual void ReadBooleanThin(bool *b){Read(b,1);}
	virtual void ReadInteger(int *ii){Read(ii,4);}
	virtual void ReadInteger(long *ii){Read(ii,4);}
	virtual void ReadInteger(UINT *u){Read(u,4);}
	virtual void ReadInteger(DWORD *u){Read(u,4);}
	virtual long ReadInteger(){
		long liValue=0;
		Read(&liValue,4);
		return liValue;
	}
	virtual void ReadWord(WORD *w){Read(w,2);}
	virtual void ReadWord(short *w){Read(w,2);}
	virtual WORD ReadWord(){
		WORD wiValue=0;
		Read(&wiValue,2);
		return wiValue;
	}
	virtual void ReadDword(DWORD *dw){Read(dw,4);}
	virtual DWORD ReadDword(){
		DWORD dwValue=0;
		Read(&dwValue,4);
		return dwValue;
	}
	virtual void ReadFloat(float *f){Read(f,4);}
	virtual void ReadFloat(double *ff){float f=0; Read(&f,4); *ff=f;}
	virtual void ReadDouble(double *d){Read(d,8);}
	virtual double ReadDouble(){
		double dfValue=0;
		Read(&dfValue,8);
		return dfValue;
	}
	virtual void ReadPoint(double *pt_arr){Read(pt_arr,24);}	//三维双精度浮点数组
	virtual void ReadThinPoint(double *pt_arr);		//三维单精度浮点数组
	virtual void WriteByte(BYTE byte){Write(&byte,1);}
	virtual void WriteBoolean(bool b);	//为避免更改代码全局编译，实现代码移至Buffer.cpp wjh-2014.5.15
	virtual void WriteBooleanThin(bool b){Write(&b,1);}
	virtual void WriteInteger(long ii){Write(&ii,4);}
	virtual void WriteWord(WORD w){Write(&w,2);}
	virtual void WriteWord(short w){Write(&w,2);}
	virtual void WriteDword(DWORD dw){Write(&dw,4);}
	virtual void WriteFloat(float f){Write(&f,4);}
	virtual void WriteFloat(double ff){float f=(float)ff; Write(&f,4);}
	virtual void WriteDouble(double d){Write(&d,8);}
	virtual void WritePoint(double *pt_arr){Write(pt_arr,24);}	//三维双精度浮点数组
	virtual void WriteThinPoint(double *pt_arr);		//三维单精度浮点数组
	// return string length or -1 if UNICODE string is found in the archive
	UINT ReadStringLength();
	UINT ReadString(char *sReadString,UINT maxBufLength=0);
	// String format:
	//      UNICODE strings are always prefixed by 0xff, 0xfffe
	//      if < 0xff chars: len:BYTE, TCHAR chars
	//      if >= 0xff characters: 0xff, len:WORD, TCHAR chars
	//      if >= 0xfffe characters: 0xff, 0xffff, len:DWORD, TCHARs
	//maxWriteLength防止写入错误字符串导致将来因ReadString设定最大读取长度再打不开文件
	//maxWriteLength＝0时表示不限制最大写入长度 wjh-2013.11.08
	void WriteString(const char *ss,UINT maxWriteLength=0);
};
typedef struct IBufferStack
{
	virtual void AttachBuffer(BUFFER_IO* pBuffIO)=0;
	virtual int PushPosition()=0;
	virtual bool PopPosition(int pos=-1)=0;
	virtual bool RestoreNowPosition()=0;
	virtual int GetStackRemnantSize()=0;
	virtual bool ClearLevelCount(int iLevel)=0;
	virtual bool IncrementLevelCount(int iLevel)=0;
	virtual long LevelCount(int iLevel)=0;
}*IBufferStackPtr;
class BUFFERPOP{
	UINT count,m_nPlanningCount;
	DWORD logPosition,currPosition;
	BUFFER_IO* m_pIO;
public:
	BUFFERPOP(BUFFER_IO* pIO,UINT nHits){
		Initialize(pIO,nHits);
	}
	void Initialize(BUFFER_IO* pIO,UINT nHits){
		count=0;
		m_nPlanningCount=nHits;
		m_pIO=pIO;
		currPosition=logPosition=pIO->GetCursorPosition();
	}
	void Increment(){count++;}
	UINT Count(){return count;}
	bool VerifyAndRestore(bool bUpdate=true,BYTE cDataTypeLen=4)
	{
		if(m_nPlanningCount==count)
			return true;
		currPosition=m_pIO->GetCursorPosition();
		if(bUpdate)
		{
			m_pIO->SeekPosition(logPosition);
			cDataTypeLen=min(cDataTypeLen,4);
			m_pIO->Write(&count,cDataTypeLen);
			m_pIO->SeekPosition(currPosition);
		}
		return false;
	}
};
struct BUFF_POSITION_ITEM{
	long lPushPosition;	//压栈时缓存位置
	long lNowPosition;	//出栈前缓存当前位置
	int level;
	long counts;
	BUFF_POSITION_ITEM(long push=0,long popNow=0){
		lPushPosition=push;
		lNowPosition=popNow;
		level=counts=0;
	};
};
class STD_BUFFER : public BUFFER_IO{
	DWORD cursor,buf_size;
	BYTE* data;
	STD_BUFFER(void* buff,DWORD size)
	{
		cursor=0;
		buf_size=size;
		data=(BYTE*)buff;
	}
	virtual bool SeekPosition(DWORD pos)
	{
		if(pos<0||pos>=buf_size)
			return false;
		else
			cursor=pos;
		return true;
	}
	virtual DWORD Read(void* pch,DWORD size)
	{
		DWORD dwRead=min(size,buf_size-cursor);
		memcpy(pch,data+cursor,dwRead);
		cursor+=dwRead;
		return dwRead;
	}
	virtual DWORD Write(const void* pch,DWORD size)
	{
		DWORD dwWrite=min(size,buf_size-cursor);
		memcpy(data+cursor,pch,dwWrite);
		cursor+=dwWrite;
		return dwWrite;
	}
};

class CBuffer : public BUFFER_IO
{
protected:
	bool m_bExternalBuf;
	DWORD file_len,buffer_len; 
	DWORD mem_cursor;		//当前指针位置
	DWORD log_mem_position;	//标记指针位置
	BUFF_POSITION_ITEM temporary;	//标记指针位置
	char *buffer;
	DWORD m_nBlockSize;
	bool m_bExternalPosStack;
	IBufferStack* m_pPosStack;
	//溢出缓存文件读写属性及方法定义，专用于解决写入大文件时内存分配失败情况 wjh-2019.8.15
	FILE* m_fpOverflowBuffFile;		//FILE*类型，只是为了减少头文件包含，所以此处声明为void*
	char* _OverFlowBuffPool;		//溢出缓存文件读写时的文件缓存
	DWORD m_dwOverFlowBuffPoolSize;	//_OverFlowBuffPool大小
	DWORD m_dwMaxAllocMemBuffSize;	//直接从程序堆上分配内存的大小，超出此值如指定外挂溢出文件时读取(或写入)溢出文件
	DWORD m_dwOverflowBuffFileLength;
	//实践证明ftell函数对文件读写速度影响巨大，有必要内存记录当前游标位置 wjh-2019.8.15
	long  m_liOverflowFileCurrPos;	//溢出文件当前的读写位置
	DWORD ReadFromFileAt(DWORD posBeginFromFile,void *pch,DWORD size);
	DWORD WriteToFileAt(DWORD posBeginFromFile,const void *pch,DWORD size);
public:
	static IBufferStackPtr (*CreateBufferStackFunc)(BUFFER_IO* pBuffIO);	//只是创建IBufferStack类型的缓存位置栈，需要用户自行delete
public:
	virtual DWORD GetLength();
	DWORD SetBlockSize(DWORD nBlockSize = 1024 );
	bool  InitOverflowBuffFile(FILE* fp,DWORD dwMaxAllocMemBuffSize=0x40000000,char* buff_pool=NULL,UINT buff_pool_size=0);	//默认从程序堆一次性分配内存最大为1G
	UINT  DetachOverflowBuffFile();	//返回当前溢出缓存文件大小
	DWORD GetOverflowBuffFileLength();
	virtual void ClearBuffer();		//清除缓存，释放内存，再需要时需要重新分配
	void ClearContents();	//只清空内容，不清除缓存，即只将file_len置0
	char* GetBufferPtr();
	virtual bool SeekToBegin();
	virtual bool SeekToEnd();
	DWORD Offset(int offset);//自当前位置移位offset个字节
	DWORD LogPosition();		//标记指针位置
	char* GetCursorBuffer();////返回当前指针指向缓存
	virtual DWORD GetCursorPosition();//返回当前指针位置
	DWORD RecallPosition();	//循环召回指针位置
	UINT GetRemnantSize();
	virtual DWORD Read(void *pch,DWORD size);
	virtual DWORD Write(const void *pch,DWORD size);
	DWORD ReadAt(DWORD pos,void *pch,DWORD size,bool moveCursorPosition=false);
	void WriteAt(DWORD pos,void *pch,DWORD size,bool moveCursorPosition=true);
	virtual bool SeekPosition(DWORD pos);
	virtual bool SeekOffset(DWORD offset){Offset(offset);return true;}
	virtual IBufferStack* BufferStack(){return m_pPosStack;}
	virtual bool AttachStack(IBufferStack* pPosStack);
	virtual void DetachStack();
	virtual int PushPositionStack();				//将当前缓存位置压栈
	virtual bool PopPositionStack(int push_pos=-1);	//弹出至相配对的PushStack时缓存位置
	virtual bool RestoreNowPosition();		//恢复PopStack之前的缓存位置
	virtual bool ClearLevelCount(int iLevel);
	virtual bool IncrementLevelCount(int iLevel);
	virtual long LevelCount(int iLevel);
	//从另一数据源缓存当前位置读取指定长度字节流,同时会对数据源缓存进行移位操作
	DWORD Write(CBuffer& buffer,DWORD size);
	DWORD ReadByte(BYTE *byte);
	DWORD ReadByte(char *ch);
	//virtual void ReadBoolean(bool *b);
	void ReadBooleanThin(bool *b);
	void ReadInteger(int *ii);
	void ReadInteger(long *ii);
	void ReadInteger(UINT *u);
	void ReadInteger(DWORD *u);
	long ReadInteger();
	DWORD ReadDword();
	void ReadWord(WORD *w);
	void ReadWord(short *w);
	void ReadDword(DWORD *dw);
	//virtual void ReadFloat(float *f);
	double ReadDouble();
	void ReadDouble(double *d);
	//void ReadPoint(double *pt_arr);	//三维双精度浮点数组
	void WriteByte(BYTE byte);
	//virtual void WriteBoolean(bool b);
	void WriteBooleanThin(bool b);
	void WriteInteger(long ii);
	void WriteWord(WORD w);
	void WriteWord(short w);
	void WriteDword(DWORD dw);
	//void WriteFloat(float f);
	void WriteDouble(double d);
	//void WritePoint(double *pt_arr);	//三维双精度浮点数组
	BOOL ReadFromPipe(HANDLE hPipeRead,DWORD pack_size=1024);
	BOOL WriteToPipe(HANDLE hPipeWrite,DWORD pack_size=1024);
	UINT ReadStringFromUTF8(char *sReadString,UINT maxLength=0);
	void WriteStringToUTF8(const char *ss,UINT maxWriteLength=0);
	void operator<<(bool &var);
	void operator>>(bool &var);
	void operator<<(WORD &var);
	void operator>>(WORD &var);
	void operator<<(DWORD &var);
	void operator>>(DWORD &var);
	void operator<<(short &var);
	void operator>>(short &var);
	void operator<<(int &var);
	void operator>>(int &var);
	void operator<<(long &var);
	void operator>>(long &var);
	void operator<<(double &var);
	void operator>>(double &var);
	bool AttachMemory(char* srcBuf, DWORD buf_size);
	void DetachMemory();
	CBuffer(int nBlockSize = 1024 );
	CBuffer(char* srcBuf, DWORD buf_size);
	CBuffer(BYTE* srcBuf, DWORD buf_size);
	~CBuffer();
};
class CAttachBuffer : public CBuffer
{
protected:
	long m_idKey;
public:
	char name[17];
	CAttachBuffer(long key){m_idKey=key;}
	virtual ~CAttachBuffer(void){;}
	long Key(){return m_idKey;}
};

#endif // !defined(AFX_BUFFER_H__10945DBC_F91B_433A_97DD_482C7BC01747__INCLUDED_)
