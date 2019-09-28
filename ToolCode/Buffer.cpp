// Buffer.cpp: implementation of the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include <stdafx.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "Buffer.h"

//因不在Windows MFC环境下运行，无法执行下面的调试跟踪宏定义功能
#if defined(_DEBUG)&&!defined(_DISABLE_DEBUG_NEW_)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
DWORD BUFFER_IO::ReadBuffer(BUFFER_IO *buffer_io,DWORD size,char* pool/*=NULL*/,DWORD pool_size/*=0*/)
{
	char block[1024];
	DWORD nread=0,nwritten=0;
	if(pool==NULL)
	{
		pool=block;
		pool_size=1024;
	}
	nread=min(size,pool_size);
	while(nread>0&&(nread=Read(pool,nread))>0){
		nwritten+=buffer_io->Write(pool,nread);
		nread=min(size-nwritten,pool_size);
	}
	return nwritten;
}

DWORD BUFFER_IO::WriteBuffer(BUFFER_IO *buffer_io,DWORD size,char* pool/*=NULL*/,DWORD pool_size/*=0*/)
{
	char block[1024];
	DWORD nread=0,nwritten=0;
	if(pool==NULL)
	{
		pool=block;
		pool_size=1024;
	}
	nread=min(size,pool_size);
	while(nread>0&&(nread=buffer_io->Read(pool,nread))>0){
		nwritten+=Write(pool,nread);
		nread=min(size-nwritten,pool_size);
	}
	return nwritten;
}
void BUFFER_IO::ReadBoolean(bool *b){
	char bt[8]={0};
	Read(bt,8);
	//if(stricmp(bt,"true")==0)	//由于stricmp报编译过时警告，用以下原始代码代替
	if((bt[0]=='t'||bt[0]=='T') && (bt[1]=='r'||bt[1]=='R') &&
		(bt[2]=='u'||bt[1]=='U') && (bt[3]=='e'||bt[1]=='E') && bt[4]==0)
		*b = true;
	else
		*b = false;
}
void BUFFER_IO::WriteBoolean(bool b){
	char bt[8]={ 0 };
	//由于strcpy报编译过时警告，用以下原始代码代替
	if(b)
	{
		bt[0]='t';bt[1]='r';bt[2]='u';bt[3]='e';bt[4]=0;//strcpy(bt,"true");
	}
	else
	{
		bt[0]='f';bt[1]='a';bt[2]='l';bt[3]='s';bt[4]='e';bt[5]=0;//strcpy(bt,"false");
	}
	Write(bt,8);
}
int BUFFER_IO::PushPositionStack()				//将当前缓存位置压栈
{
	if(BufferStack())
		return BufferStack()->PushPosition();
	else
		return 0;
}
bool BUFFER_IO::PopPositionStack(int push_pos/*=-1*/)	//弹出至相配对的PushStack时缓存位置
{
	if(BufferStack())
		return BufferStack()->PopPosition(push_pos);
	else
		return false;
}
bool BUFFER_IO::RestoreNowPosition()		//恢复PopStack之前的缓存位置
{
	if(BufferStack())
		return BufferStack()->RestoreNowPosition();
	else
		return false;
}
bool BUFFER_IO::ClearLevelCount(int iLevel)
{
	if(BufferStack())
		return BufferStack()->ClearLevelCount(iLevel);
	else
		return false;
}
bool BUFFER_IO::IncrementLevelCount(int iLevel)
{
	if(BufferStack())
		return BufferStack()->IncrementLevelCount(iLevel);
	else
		return false;
}
long BUFFER_IO::LevelCount(int iLevel)
{
	if(BufferStack())
		return BufferStack()->LevelCount(iLevel);
	else
		return 0;
}
void BUFFER_IO::ReadThinPoint(double *pt_arr)		//三维单精度浮点数组
{
	float coords[3]={0.0f};
	Read(coords,12);
	pt_arr[0]=coords[0];
	pt_arr[1]=coords[1];
	pt_arr[2]=coords[2];
}
void BUFFER_IO::WriteThinPoint(double *pt_arr)
{
	float coords[3]={(float)pt_arr[0],(float)pt_arr[1],(float)pt_arr[2]};
	Write(coords,12);
}

//////////////////////////////////////////////////////////////////
typedef IBufferStackPtr (*CREATEBUFFERSTACKFUNC)(BUFFER_IO* pBuffIO);	//只是创建IBufferStack类型的缓存位置栈，需要用户自行delete
CREATEBUFFERSTACKFUNC CBuffer::CreateBufferStackFunc=NULL;
CBuffer::CBuffer(int nBlockSize/* = 1024*/ )
{
	buffer = NULL;
	buffer_len = file_len = 0;
	log_mem_position = mem_cursor = 0;
	m_nBlockSize = nBlockSize;
	m_bExternalBuf=false;
	m_bExternalPosStack=true;
	m_pPosStack=NULL;
	_OverFlowBuffPool=NULL;		//溢出缓存文件读写时的文件缓存
	m_dwOverFlowBuffPoolSize=0;	//_OverFlowBuffPool大小
	m_liOverflowFileCurrPos=0;
	m_fpOverflowBuffFile=NULL;
	m_dwOverflowBuffFileLength=0;
	m_dwMaxAllocMemBuffSize=0x40000000;
}
CBuffer::CBuffer(char* srcBuf, DWORD buf_size)
{
	buffer = srcBuf;
	buffer_len = file_len = buf_size;	//file_len不能清零，因为相关代码认为传入的是整个有效数据块 wjh-2017.7.5
	log_mem_position = mem_cursor = 0;
	m_nBlockSize = buf_size;
	m_bExternalBuf=(srcBuf!=NULL);
	m_bExternalPosStack=true;
	m_pPosStack=NULL;
	_OverFlowBuffPool=NULL;		//溢出缓存文件读写时的文件缓存
	m_dwOverFlowBuffPoolSize=0;	//_OverFlowBuffPool大小
	m_liOverflowFileCurrPos=0;
	m_fpOverflowBuffFile=NULL;
	m_dwOverflowBuffFileLength=0;
	m_dwMaxAllocMemBuffSize=0x40000000;
}
CBuffer::CBuffer(BYTE* srcBuf, DWORD buf_size)
{
	buffer = (char*)srcBuf;
	buffer_len = file_len = buf_size;
	log_mem_position = mem_cursor = 0;
	m_nBlockSize = buf_size;
	m_bExternalBuf=(srcBuf!=NULL);
	m_bExternalPosStack=true;
	m_pPosStack=NULL;
	_OverFlowBuffPool=NULL;		//溢出缓存文件读写时的文件缓存
	m_dwOverFlowBuffPoolSize=0;	//_OverFlowBuffPool大小
	m_liOverflowFileCurrPos=0;
	m_fpOverflowBuffFile=NULL;
	m_dwOverflowBuffFileLength=0;
	m_dwMaxAllocMemBuffSize=0x40000000;
}
bool CBuffer::AttachMemory(char* srcBuf, DWORD buf_size)
{
	if(srcBuf==NULL||buf_size<=0)
		return false;
	if(buffer!=NULL&&!m_bExternalBuf)
		delete[] buffer;
	buffer = srcBuf;
	buffer_len = file_len = buf_size;
	log_mem_position = mem_cursor = 0;
	m_bExternalBuf=true;
	return true;
}
void CBuffer::DetachMemory()
{
	buffer=NULL;
	buffer_len = file_len =0;
	log_mem_position = mem_cursor = 0;
	m_bExternalBuf=false;
}

CBuffer::~CBuffer()
{
	if(buffer&&!m_bExternalBuf)
		delete []buffer;
	if(!m_bExternalPosStack&&m_pPosStack!=NULL)
		delete m_pPosStack;
}
DWORD CBuffer::SetBlockSize(DWORD nBlockSize/* = 1024 */)
{
	m_nBlockSize = nBlockSize;
	return m_nBlockSize;
}
char* CBuffer::GetBufferPtr()
{
	return buffer;
}
bool  CBuffer::InitOverflowBuffFile(FILE* fp,DWORD dwMaxAllocMemBuffSize/*=0x40000000*/,
		char* buff_pool/*=NULL*/,UINT buff_pool_size/*=0*/)
{
	this->_OverFlowBuffPool=buff_pool;
	this->m_dwOverFlowBuffPoolSize=buff_pool_size;
	if (m_fpOverflowBuffFile!=fp)
		m_dwOverflowBuffFileLength=0;
	if ((m_fpOverflowBuffFile=fp)!=NULL)
	{
		if(_OverFlowBuffPool!=NULL&&buff_pool_size>4096)	//实践证明缓存对写入文件速度影响不大 wjh-2019.8.15
			setvbuf(fp,_OverFlowBuffPool,_IOFBF,buff_pool_size);
		fseek(fp,0,SEEK_SET);
		m_liOverflowFileCurrPos=0;
	}
	m_dwMaxAllocMemBuffSize=dwMaxAllocMemBuffSize;
	return m_fpOverflowBuffFile!=NULL;
}
UINT CBuffer::DetachOverflowBuffFile()
{
	m_fpOverflowBuffFile=NULL;
	UINT dwBuffSize=m_dwOverflowBuffFileLength;
	m_dwOverflowBuffFileLength=0;
	m_dwMaxAllocMemBuffSize=0x40000000;
	return dwBuffSize;
}
DWORD CBuffer::GetOverflowBuffFileLength()
{
	return m_fpOverflowBuffFile!=NULL?m_dwOverflowBuffFileLength:0;
}
//清除缓存，释放内存，再需要时需要重新分配
void CBuffer::ClearBuffer()
{
	if(buffer)
		delete []buffer;
	buffer = NULL;
	buffer_len = file_len = 0;
	log_mem_position = mem_cursor = 0;
}
//只清空内容，不清除缓存，即只将file_len置0
void CBuffer::ClearContents()
{
	file_len = 0;
	log_mem_position = mem_cursor = 0;
	m_dwOverflowBuffFileLength=0;
}
DWORD CBuffer::GetLength()
{
	return file_len;
}
bool CBuffer::SeekToBegin()
{
	mem_cursor = 0;
	return true;
}
bool CBuffer::SeekToEnd()
{
	mem_cursor = file_len;
	return true;
}

//自当前位置移位offset个字节
DWORD CBuffer::Offset(int offset)
{
	mem_cursor+=offset;
	if(mem_cursor<0)
		mem_cursor=0;
	return mem_cursor;
}

//标记指针位置
DWORD CBuffer::LogPosition()
{
	log_mem_position=mem_cursor;
	return mem_cursor;
}
//返回当前指针指向缓存
char* CBuffer::GetCursorBuffer()
{
	return buffer+mem_cursor;
}
//返回当前指针位置
DWORD CBuffer::GetCursorPosition()
{
	return mem_cursor;
}
//循环召回指针位置
DWORD CBuffer::RecallPosition()
{
	DWORD tm_pos=log_mem_position;
	log_mem_position=mem_cursor;
	mem_cursor=tm_pos;
	return mem_cursor;
}
bool CBuffer::AttachStack(IBufferStack* pPosStack)
{
	if(pPosStack==NULL)
		return false;
	if(!m_bExternalPosStack&&m_pPosStack)
		delete m_pPosStack;
	m_pPosStack=pPosStack;
	pPosStack->AttachBuffer(this);
	m_bExternalPosStack=true;
	return true;
}
void CBuffer::DetachStack()
{
	if(m_pPosStack&&!m_bExternalPosStack)
		delete m_pPosStack;
	m_pPosStack=NULL;
}

int CBuffer::PushPositionStack()				//将当前缓存位置压栈
{
	if(BufferStack()==NULL&&CreateBufferStackFunc!=NULL)
	{
		m_pPosStack=CreateBufferStackFunc(this);
		m_bExternalPosStack=false;
	}
	if(BufferStack())
		return BufferStack()->PushPosition();
	else
	{
		temporary.lNowPosition=temporary.lPushPosition=GetCursorPosition();
		return temporary.level;
	}
}
bool CBuffer::PopPositionStack(int push_pos/*=-1*/)	//弹出至相配对的PushStack时缓存位置
{
	if(BufferStack())
		return BufferStack()->PopPosition(push_pos);
	else
	{
		temporary.lNowPosition=mem_cursor;
		mem_cursor=temporary.lPushPosition;
		return true;
	}
}
bool CBuffer::RestoreNowPosition()		//恢复PopStack之前的缓存位置
{
	if(BufferStack())
		return BufferStack()->RestoreNowPosition();
	else
	{
		mem_cursor=temporary.lNowPosition;
		return true;
	}
}
bool CBuffer::ClearLevelCount(int iLevel)
{
	if(BufferStack())
		return BufferStack()->ClearLevelCount(iLevel);
	else if(temporary.level==iLevel)
	{
		temporary.counts=0;
		return true;
	}
	else
		return false;
}
bool CBuffer::IncrementLevelCount(int iLevel)
{
	if(BufferStack())
		return BufferStack()->IncrementLevelCount(iLevel);
	else if(temporary.level==iLevel)
	{
		temporary.counts++;
		return true;
	}
	else
		return false;
}
long CBuffer::LevelCount(int iLevel)
{
	if(BufferStack())
		return BufferStack()->LevelCount(iLevel);
	else if(temporary.level==iLevel)
		return temporary.counts;
	else
		return 0;
}

bool CBuffer::SeekPosition(DWORD pos)
{
	//旧代码　wjh-2012.12.14
	//if(pos>=0&&pos<=file_len)
	//	mem_cursor = pos;
	if(pos<0||pos>file_len+this->m_dwOverflowBuffFileLength)
		return false;
	else
		mem_cursor=pos;
	return true;
}
UINT CBuffer::GetRemnantSize()
{
	return file_len-mem_cursor;
}
DWORD CBuffer::ReadFromFileAt(DWORD posBeginFromFile,void *pch,DWORD size)
{
	if (this->m_fpOverflowBuffFile==NULL)
		return 0;
	long liReadBytes=posBeginFromFile+size<this->m_dwOverflowBuffFileLength?size:this->m_dwOverflowBuffFileLength-posBeginFromFile;
	if (liReadBytes<=0)
		return 0;
	else if (m_liOverflowFileCurrPos!=posBeginFromFile)
		fseek(m_fpOverflowBuffFile,posBeginFromFile,SEEK_SET);
	return (DWORD)fread(pch,liReadBytes,1,m_fpOverflowBuffFile);
}
DWORD CBuffer::WriteToFileAt(DWORD posBeginFromFile,const void *pch,DWORD size)
{
	if (this->m_fpOverflowBuffFile==NULL)
		return 0;
	long liFillNullBytes=posBeginFromFile>this->m_dwOverflowBuffFileLength?m_dwOverflowBuffFileLength-posBeginFromFile:0;
	DWORD dwZero=0,dwWriteByts=0;
	//实践证明ftell函数对文件读写速度影响巨大，有必要内存记录当前游标位置 wjh-2019.8.15
	//m_liOverflowFileCurrPos=ftell(m_fpOverflowBuffFile);
	if (liFillNullBytes>0)
	{
		fseek(m_fpOverflowBuffFile,m_dwOverflowBuffFileLength,SEEK_SET);
		for (int i=0;i<liFillNullBytes;i+=4)
		{
			dwWriteByts=i+4<liFillNullBytes?4:liFillNullBytes-i;
			fwrite(&dwZero,dwWriteByts,1,m_fpOverflowBuffFile);
		}
	}
	else if(m_liOverflowFileCurrPos!=posBeginFromFile)
		fseek(m_fpOverflowBuffFile,posBeginFromFile,SEEK_SET);
	dwWriteByts= (DWORD)fwrite(pch,size,1,m_fpOverflowBuffFile);
	if(m_dwOverflowBuffFileLength<posBeginFromFile+size)
		m_dwOverflowBuffFileLength=posBeginFromFile+size;
	m_liOverflowFileCurrPos=posBeginFromFile+size;
	return dwWriteByts;
}
DWORD CBuffer::Read(void *pch,DWORD size)
{
	DWORD dwCount = file_len-mem_cursor;
	if(file_len<=mem_cursor)
		return 0;
	else if(dwCount>(DWORD)size)
		dwCount = size;
	memcpy(pch,buffer+mem_cursor,dwCount);
	mem_cursor+=dwCount;
	return dwCount;
}
//从另一数据源缓存当前位置读取指定长度字节流,同时会对数据源缓存进行移位操作
DWORD CBuffer::Write(CBuffer& buffer,DWORD size)
{
	if(size>buffer.GetRemnantSize())
		size=buffer.GetRemnantSize();
	Write(buffer.GetCursorBuffer(),size);
	buffer.Offset(size);
	return size;
}
DWORD CBuffer::Write(const void *pch,DWORD size)
{
	if(size<=0)
		return 0;
	DWORD dwWriteSize=size;
	DWORD dwWriteToExtraFileBytes=0;
	if (m_fpOverflowBuffFile!=NULL && mem_cursor+size>m_dwMaxAllocMemBuffSize)
	{
		if (mem_cursor<m_dwMaxAllocMemBuffSize)
		{
			dwWriteToExtraFileBytes=size-(m_dwMaxAllocMemBuffSize-mem_cursor);//WriteToFileAt
			size-=dwWriteToExtraFileBytes;
		}
		else
		{
			dwWriteToExtraFileBytes=size;
			size=0;
		}
	}
	if (dwWriteToExtraFileBytes>0)
	{
		DWORD posBeginFromFile=mem_cursor+size-m_dwMaxAllocMemBuffSize;
		this->WriteToFileAt(posBeginFromFile,((char*)pch)+size,dwWriteToExtraFileBytes);
		if (posBeginFromFile+dwWriteToExtraFileBytes>this->m_dwOverflowBuffFileLength)
			m_dwOverflowBuffFileLength=posBeginFromFile+dwWriteToExtraFileBytes;
	}
	if(size>0&&mem_cursor+size>buffer_len)
	{
		int size_incre;
		if(size>m_nBlockSize)
			size_incre = size;
		else
			size_incre = m_nBlockSize;
		char *tm = new char[buffer_len+size_incre];
		if(tm==NULL)
			throw "缓冲区内存分配失败！";
		if(buffer)
		{
			memcpy(tm,buffer,buffer_len);
			if(!m_bExternalBuf)
				delete []buffer;
		}
		buffer = tm;
		buffer_len+=size_incre;
		m_bExternalBuf=false;
	}
	if (size>0&&mem_cursor<buffer_len)
	{
		if (pch==NULL)	//写入零数据
			memset(buffer+mem_cursor,0,size);
		else
			memcpy(buffer+mem_cursor,pch,size);
	}
	if(size>0)
	{
		file_len = max(mem_cursor+size,file_len);
		//file_len = min(file_len,buffer_len);
		//file_len = min(file_len,m_dwMaxAllocMemBuffSize);
	}
	mem_cursor+=size+dwWriteToExtraFileBytes;
	return dwWriteSize;
}
DWORD CBuffer::ReadAt(DWORD pos,void *pch,DWORD size,bool moveCursorPosition/*=false*/)
{
	if(pos+size>file_len)
	{
		if(file_len>pos)
			size=max(0,file_len-pos);
		else
			size=0;
	}
	if(size>0)
	{
		memcpy(pch,buffer+pos,size);
		if(moveCursorPosition)
			mem_cursor=pos+size; //不能全部更新mem_cursor，因为SolidBodyBuffer中调用了ReadAt，且涉及当前游标位置
	}
	return size;
}
void CBuffer::WriteAt(DWORD pos,void *pch,DWORD size,bool moveCursorPosition/*=true*/)
{
	long oldCursor=mem_cursor;
	if(pos>file_len)
	{
		SeekToEnd();
		Write(NULL,pos-file_len);
	}
	else
		mem_cursor=pos;
	Write(pch,size);
	if(!moveCursorPosition)	//为了与过往代码有继承性，moveCursorPosition默认为true
		mem_cursor=oldCursor;
}
DWORD CBuffer::ReadByte(BYTE *byte)
{
	return Read(byte,1);
}
DWORD CBuffer::ReadByte(char *ch)
{
	return Read(ch,1);
}
void CBuffer::ReadBooleanThin(bool *b)
{
	Read(b,1);//sizeof(bool));
}
/* 作为基类虚函数移至BUFFER_IO中
void CBuffer::ReadBoolean(bool *b)
{
	char bt[8]={0};
	Read(bt,8);
	if(_stricmp(bt,"true")==0)
		*b = true;
	else
		*b = false;
}
*/
void CBuffer::ReadInteger(int *ii)
{
	Read(ii,4);
}
void CBuffer::ReadInteger(long *ii)
{
	Read(ii,4);
}
void CBuffer::ReadInteger(UINT *u)
{
	Read(u,4);
}
void CBuffer::ReadInteger(DWORD *u)
{
	Read(u,4);
}
long CBuffer::ReadInteger()
{
	long id=0;
	ReadInteger(&id);
	return id;
}
DWORD CBuffer::ReadDword()
{
	long dw=0;
	ReadInteger(&dw);
	return dw;
}

void CBuffer::ReadWord(WORD *w)
{
	Read(w,2);
}
void CBuffer::ReadWord(short *w)
{
	Read(w,2);
}
void CBuffer::ReadDword(DWORD *dw)
{
	Read(dw,4);
}
//void CBuffer::ReadFloat(float *f)
//{
//	Read(f,4);
//}
double CBuffer::ReadDouble()
{
	double d=0;
	Read(&d, 8);
	return d;
}
void CBuffer::ReadDouble(double *d)
{
	Read(d,8);
}
void CBuffer::WriteByte(BYTE byte)
{
	Write(&byte,1);
}
/*
void CBuffer::ReadPoint(double *pt_arr)	//三维双精度浮点数组
{
	Read(pt_arr,24);
}
*/
void CBuffer::WriteBooleanThin(bool b)
{
	Write(&b,1);//sizeof(bool));
}
/* 作为基类虚函数移至BUFFER_IO中
void CBuffer::WriteBoolean(bool b)
{
	char bt[8];
	if(b)
		strcpy(bt,"true");
	else
		strcpy(bt,"false");
	Write(bt,8);
}
*/
void CBuffer::WriteInteger(long ii)
{
	Write(&ii,4);
}
void CBuffer::WriteWord(WORD w)
{
	Write(&w,2);
}
void CBuffer::WriteWord(short w)
{
	Write(&w,2);
}
void CBuffer::WriteDword(DWORD dw)
{
	Write(&dw,4);
}
//void CBuffer::WriteFloat(float f)
//{
//	Write(&f,4);//sizeof(double));
//}
void CBuffer::WriteDouble(double d)
{
	Write(&d,8);//sizeof(double));
}
/*
void CBuffer::WritePoint(double *pt_arr)	//三维双精度浮点数组
{
	Write(pt_arr,24);
}
*/
// return string length or -1 if UNICODE string is found in the archive
UINT BUFFER_IO::ReadStringLength()
{
	DWORD nNewLen;

	// attempt BYTE length first
	BYTE bLen=0;
	ReadByte(&bLen);

	if (bLen < 0xff)
		return bLen;

	// attempt WORD length
	WORD wLen;
	ReadWord(&wLen);
	if (wLen == 0xfffe)
	{
		// UNICODE string prefix (length will follow)
		return (UINT)-1;
	}
	else if (wLen == 0xffff)
	{
		// read DWORD of length
		ReadDword(&nNewLen);
		return (UINT)nNewLen;
	}
	else
		return wLen;
}
typedef char* char_ptr;
//UTF-8到GB2312的转换,gb2312需要在调用处释放
int U2G(const char* utf8,char_ptr &gb2312)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	if(len<=0)
		return 0;
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	gb2312 = new char[len+1];
	memset(gb2312, 0, len+1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, gb2312, len, NULL, NULL);
	if(wstr) 
		delete[] wstr;
	return len;
}

//GB2312到UTF-8的转换,utf8需要在调用处释放
int G2U(const char* gb2312,char_ptr &utf8)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	if(len<=0)
		return 0;
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	utf8 = new char[len+1];
	memset(utf8, 0, len+1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8, len, NULL, NULL);
	if(wstr) 
		delete[] wstr;
	return len;
}
//读取UTF8格式字符串并转换为GB2312格式
UINT CBuffer::ReadStringFromUTF8(char *sReadString,UINT maxLength/*=0*/)
{
	UINT slen=ReadInteger();
	if(slen==0xffffffff)
	{
		if(sReadString)
			*sReadString=0;
		return 0;	//-1表示空
	}
	else if(maxLength>0&&slen>=maxLength)	//maxLength为sReadString最大缓存长度时，应考虑字符串收尾字符问题
	{
		DWORD pos=GetCursorPosition();
		SeekPosition(pos+slen);
		throw "字符串超过预定缓存长度，内存溢出，请立即存盘关闭程序。";
	}
	else
	{
		Read(sReadString,slen);
		char *gb2312=NULL;
		int len=U2G(sReadString,gb2312);
		if(gb2312!=NULL)
		{
			memcpy(sReadString,gb2312,len);
			sReadString[len]='\0';
			delete gb2312;
			return len;
		}
		else 
			return 0;
	}

}
//将字符串转换为UTF8格式存储
void CBuffer::WriteStringToUTF8(const char *ss,UINT maxWriteLength/*=0*/)
{
	char *utf8=NULL;
	UINT len=G2U(ss,utf8)-1;	//末尾字符为截止符不用写入
	if(ss==NULL)
		len=0;
	if(maxWriteLength>0&&len>maxWriteLength)
		len=maxWriteLength;
	WriteInteger(len);
	if(len>0)
		Write(utf8,len);
	if(utf8!=NULL)	
		delete[] utf8;
}
//必须保证字符串空间足够大
UINT BUFFER_IO::ReadString(char *sReadString,UINT maxBufLength/*=0*/)
{
	UINT len=ReadStringLength();
	if(maxBufLength>0&&len>=maxBufLength)	//maxLength为sReadString最大缓存长度时，应考虑字符串收尾字符问题
	{
		DWORD pos=GetCursorPosition();
		Read(sReadString, maxBufLength);
		sReadString[maxBufLength] = '\0';
		SeekPosition(pos+len);
		if (len > 500)
			throw "字符串超过预定缓存长度，内存溢出，请立即存盘关闭程序。";
		else
			return maxBufLength;
	}
	if(sReadString==NULL)
		return len;
	else
		Read(sReadString,len);
	sReadString[len]='\0';
	return len;
}
// String format:
//      UNICODE strings are always prefixed by 0xff, 0xfffe
//      if < 0xff chars: len:BYTE, TCHAR chars
//      if >= 0xff characters: 0xff, len:WORD, TCHAR chars
//      if >= 0xfffe characters: 0xff, 0xffff, len:DWORD, TCHARs
void BUFFER_IO::WriteString(const char *ss,UINT maxWriteLength/*=0*/)
{
	BYTE b;
	WORD w;
	DWORD dw,str_len;
	// special signature to recognize unicode strings
#ifdef _UNICODE
	b=0xff;
	w=0xfffe;
	WriteByte(b);
	WriteWord(w);
#endif
	if(ss)
		str_len=(DWORD)strlen(ss);
	else
		str_len=0;
	if(maxWriteLength>0&&str_len>maxWriteLength)
		str_len=maxWriteLength;

	if (str_len< 255)
	{
		b=(BYTE)str_len;
		WriteByte(b);
	}
	else if (str_len < 0xfffe)
	{
		b=0xff;
		w=(WORD)str_len;
		WriteByte(b);
		WriteWord(w);
	}
	else
	{
		b=0xff;
		w=0xffff;
		dw=(DWORD)str_len;
		WriteByte(b);
		WriteWord(w);
		WriteDword(dw);
	}
	if(str_len>0)
		Write(ss,str_len);
}

void CBuffer::operator<<(bool &var)
{
	WriteBoolean(var);
}
void CBuffer::operator>>(bool &var)
{
	ReadBoolean(&var);
}
void CBuffer::operator<<(short &var)
{
	Write(&var,2);//sizeof(short));
}
void CBuffer::operator>>(short &var)
{
	Read(&var,2);//sizeof(short));
}
void CBuffer::operator<<(int &var)
{
	WriteInteger(var);
}
void CBuffer::operator>>(int &var)
{
	ReadInteger(&var);
}
void CBuffer::operator<<(long &var)
{
	Write(&var,4);//sizeof(long));
}
void CBuffer::operator>>(long &var)
{
	Read(&var,4);//
}
void CBuffer::operator<<(double &var)
{
	WriteDouble(var);
}
void CBuffer::operator>>(double &var)
{
	ReadDouble(&var);
}
void CBuffer::operator<<(WORD &var)
{
	Write(&var,2);//sizeof(WORD));
}
void CBuffer::operator>>(WORD &var)
{
	Read(&var,2);//sizeof(WORD));
}
void CBuffer::operator<<(DWORD &var)
{
	Write(&var,4);//sizeof(DWORD));
}
void CBuffer::operator>>(DWORD &var)
{
	Read(&var,4);//sizeof(DWORD));
}
BOOL CBuffer::ReadFromPipe(HANDLE hPipeRead,DWORD pack_size/*=1024*/)
{
	ClearBuffer();
	DWORD dwRead;
	BYTE* memory_pool = new BYTE[pack_size];
	BOOL result=FALSE;
	while(ReadFile(hPipeRead,memory_pool,pack_size,&dwRead,NULL))
	{
		Write(memory_pool,dwRead-1);
		if(memory_pool[dwRead-1]==0)
		{
			result=TRUE;
			break;	//数据接收结束
		}
	}
	delete []memory_pool;
	return result;
}
BOOL CBuffer::WriteToPipe(HANDLE hPipeWrite,DWORD pack_size/*=1024*/)
{
	SeekToBegin();
	DWORD dwBytes;
	BYTE* memory_pool = new BYTE[pack_size];
	while(GetRemnantSize()>pack_size-1)
	{
		Read(memory_pool,pack_size-1);
		memory_pool[pack_size-1]=1;	//数据发送未结束
		WriteFile(hPipeWrite,memory_pool,pack_size,&dwBytes,NULL);
	}
	UINT dwRemnantSize=GetRemnantSize();
	Read(memory_pool,dwRemnantSize);
	memory_pool[dwRemnantSize]=0;		//数据发送已结束
	BOOL result=WriteFile(hPipeWrite,memory_pool,dwRemnantSize+1,&dwBytes,NULL);
	delete []memory_pool;
	return result;
}
