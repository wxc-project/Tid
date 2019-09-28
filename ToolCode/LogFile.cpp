#include "stdafx.h"
#include <time.h>
#include ".\logfile.h"

CLogFile logto("c:\\lds.log");
CLogFile logerr("c:\\warning.log");
CLogFile::CLogFile(char* fname/*=NULL*/,bool overwriteMode/*=true*/)
{
	m_fp=NULL;
	if(fname==NULL)	//临时抑制日志记录
		m_ciLowestWarningLevel=0;
	else
		InitLogFile(fname);
	m_bInsertBlankRow=false;
	m_bHasContens=false;
	m_bInLifeCycle=false;
	m_bGlobalDisabled=false;
	m_bInsertTitleRow=false;
	m_bAppendTimestamp=!overwriteMode;
	memset(_internal_titlestr,0,201);
	_FireReflectTo=NULL;
	_iTempReflectWarningType=0;							//临时存储变量，常用于将日志信息反射到任务栏中
	_hTempReflectRelaObj[0]=_hTempReflectRelaObj[1]=0;	//临时存储变量，常用于将日志信息反射到任务栏中
}

CLogFile::~CLogFile(void)
{
	CloseFile();
}
void CLogFile::AttachReflector(bool (*_ReflectFunc)(int iWarningType,long hRelaObj[2],const char* errmsg))
{
	_FireReflectTo=_ReflectFunc;
}
void CLogFile::UnAttachReflector()
{
	_FireReflectTo=NULL;
}
bool CLogFile::SetCurrTempRelation(int iWarningType,long hRelaObj1/*=0*/,long hRelaObj2/*=0*/)
{
	_hTempReflectRelaObj[0]=hRelaObj1;
	_hTempReflectRelaObj[1]=hRelaObj2;
	return true;
}
bool CLogFile::InitLogFile(char* fname,bool overwriteMode/*=true*/)
{
	FILE* fp2=fopen(fname,overwriteMode?"wt":"at");
	if(fp2!=NULL)
	{
		if(m_fp!=NULL)
			fclose(m_fp);
		m_fp=fp2;
		strcpy(file_name,fname);
	}
	if(m_fp==NULL)
		return false;
	m_bHasContens=false;
	m_bGlobalDisabled=false;
	m_bAppendTimestamp=!overwriteMode;
	m_ciLowestWarningLevel=WARNING_LEVEL2_COMMON;	//默认输出普通等级以上的警示信息
	return true;
}
char* CLogFile::LogFilePath()
{
	return file_name;
}
void CLogFile::InsertTitleRow(const char* title)
{
	if(strlen(title)<201)
		strcpy(_internal_titlestr,title);	//防止lengthOfDestBuf输入错误,实际字符串也并不超过strDest缓存长度时出错 
	else
	{
		strncpy(_internal_titlestr,title,200);
		_internal_titlestr[200]=0;
	}
	m_bInsertTitleRow=true;
}
bool CLogFile::ClearContents()
{
	if(m_fp!=NULL)
	{
		fclose(m_fp);
		m_fp=fopen(file_name,"wt");
		if(m_fp==NULL)
			return false;
	}
	m_bHasContens=false;
	return true;
}
void CLogFile::ShowToScreen()
{
	if(m_fp)
		fflush(m_fp);
	if(m_bHasContens)
	{
		char cmdstr[200]={0};
		_snprintf(cmdstr,200,"notepad.exe %s",file_name);
		WinExec(cmdstr,SW_SHOW);
	}
}

bool CLogFile::LevelLog(BYTE ciWarningLevel,const char *format,...)
{
	if(ciWarningLevel>m_ciLowestWarningLevel||m_bGlobalDisabled)
		return false;
	if(m_fp==NULL)
	{
		m_fp=fopen(file_name,"wt");
		m_bHasContens=false;
	}
	if(m_fp==NULL)
		return false;
	if(m_bHasContens&&m_bInsertBlankRow)
		fprintf(m_fp,"\n");
	if(m_bInsertTitleRow)
		fprintf(m_fp,_internal_titlestr);
	m_bInsertBlankRow=m_bInsertTitleRow=false;
	char timestr[51]={0};
	if(m_bAppendTimestamp)
	{
		time_t nowT=time(NULL);
		tm* _Tm=localtime(&nowT);
		strftime(timestr,50,"%Y-%m-%d %H:%M:%S",_Tm);
		//_snprintf(timestr,50,"%4d-%2d-%2d %2d:%2d:%2d",nowT.GetYear(),nowT.GetMonth(),nowT.GetDay(),
		//	nowT.GetHour(),nowT.GetMinute(),nowT.GetSecond());
	}
	va_list marker;
	va_start(marker, format);
	memset(_internal_tmp_str,0,201);
	_vsnprintf(_internal_tmp_str,200,format,marker);
	size_t str_len=strlen(_internal_tmp_str);
	if(_FireReflectTo)
		_FireReflectTo(_iTempReflectWarningType,_hTempReflectRelaObj,_internal_tmp_str);
	else 
	if(_internal_tmp_str[str_len-1]=='\n')
		fprintf(m_fp,"%s %s",_internal_tmp_str,timestr);
	else
		fprintf(m_fp,"%s %s\n",_internal_tmp_str,timestr);
	fflush(m_fp);
	m_bHasContens=true;
	return true;
}
bool CLogFile::Log(const char *format,...)
{
	if(WARNING_LEVEL2_COMMON>m_ciLowestWarningLevel||m_bGlobalDisabled)
		return false;
	if(m_fp==NULL)
	{
		m_fp=fopen(file_name,"wt");
		m_bHasContens=false;
	}
	if(m_fp==NULL)
		return false;
	if(m_bHasContens&&m_bInsertBlankRow)
		fprintf(m_fp,"\n");
	if(m_bInsertTitleRow)
		fprintf(m_fp,_internal_titlestr);
	m_bInsertBlankRow=m_bInsertTitleRow=false;
	char timestr[51]={0};
	if(m_bAppendTimestamp)
	{
		time_t nowT=time(NULL);
		tm* _Tm=localtime(&nowT);
		strftime(timestr,50,"%Y-%m-%d %H:%M:%S",_Tm);
		//_snprintf(timestr,50,"%4d-%2d-%2d %2d:%2d:%2d",nowT.GetYear(),nowT.GetMonth(),nowT.GetDay(),
		//	nowT.GetHour(),nowT.GetMinute(),nowT.GetSecond());
	}
	va_list marker;
	va_start(marker, format);
	memset(_internal_tmp_str,0,201);
	_vsnprintf(_internal_tmp_str,200,format,marker);
	size_t str_len=strlen(_internal_tmp_str);
	if(_FireReflectTo)
		_FireReflectTo(_iTempReflectWarningType,_hTempReflectRelaObj,_internal_tmp_str);
	else 
	if(_internal_tmp_str[str_len-1]=='\n')
		fprintf(m_fp,"%s %s",_internal_tmp_str,timestr);
	else
		fprintf(m_fp,"%s %s\n",_internal_tmp_str,timestr);
	fflush(m_fp);
	m_bHasContens=true;
	return true;
}
bool CLogFile::LogString(char *error, bool jumpToNextLine/*=true*/)
{
	if(m_fp==NULL)
	{
		m_fp=fopen(file_name,"wt");
		m_bHasContens=false;
	}
	if(m_fp==NULL)
		return false;
	if(m_bHasContens&&m_bInsertBlankRow)
		fprintf(m_fp,"\n");
	if(m_bInsertTitleRow)
		fprintf(m_fp,_internal_titlestr);
	m_bInsertBlankRow=m_bInsertTitleRow=false;
	memset(_internal_tmp_str,0,201);
	strncpy(_internal_tmp_str,error,200);
	size_t str_len=strlen(_internal_tmp_str);
	if(_FireReflectTo)
		_FireReflectTo(_iTempReflectWarningType,_hTempReflectRelaObj,_internal_tmp_str);
	else 
	if(_internal_tmp_str[str_len-1]=='\n'||!jumpToNextLine)
		fprintf(m_fp,"%s",_internal_tmp_str);
	else
		fprintf(m_fp,"%s\n",_internal_tmp_str);
	fflush(m_fp);
	m_bHasContens=true;
	return true;
}
void CLogFile::CloseFile()
{
	if(m_fp!=NULL)
		fclose(m_fp);
	m_fp=NULL;
}
CLogErrorLife::CLogErrorLife(CLogFile* pLogFile/*=NULL*/,bool (*_ReflectFunc)(int iWarningType,long hRelaObj[2],const char* errmsg)/*=NULL*/,void (*StartTaskPanel)()/*=NULL*/)
{
	m_bShowScreen=true;
	if(pLogFile==NULL)
		m_pLogFile=&logerr;
	else
		m_pLogFile=pLogFile;
	if(!m_pLogFile->IsInLifeCycle())
	{
		m_pLogFile->RegisterLifeCycle();
		m_pLogFile->ClearContents();
		m_bEnabled=true;
		m_bAttachReflector=AttachReflector(_ReflectFunc);
		long relaObjs[2]={0,0};
		if(_ReflectFunc)
			_ReflectFunc(0,relaObjs,NULL);
		_FireStartTaskPanelFunc=StartTaskPanel;
	}
	else
		m_bAttachReflector=m_bEnabled=false;
	m_bRestoreWarningLevel=m_bRestoreEnableState=m_bLegacyEnableState=false;
}
CLogErrorLife::~CLogErrorLife()
{
	if(m_bEnabled)
	{
		if(m_bAttachReflector)
		{
			DetachReflector();
			if(m_pLogFile->IsHasContents()&&_FireStartTaskPanelFunc)
				_FireStartTaskPanelFunc();
		}
		m_pLogFile->UnRegisterLifeCycle();
		if(!m_bAttachReflector&&m_bShowScreen&&m_pLogFile->IsHasContents())
			m_pLogFile->ShowToScreen();
		if(m_bRestoreWarningLevel)
			m_pLogFile->SetWarningLevel(m_ciWarningLevel);
		if(m_bRestoreEnableState)
			m_pLogFile->GlobalEnabled(m_bLegacyEnableState);
	}
}
bool CLogErrorLife::AttachReflector(bool (*_ReflectFunc)(int iWarningType,long hRelaObj[2],const char* errmsg))
{
	if(m_pLogFile==NULL||_ReflectFunc==NULL)
		return false;
	m_pLogFile->AttachReflector(_ReflectFunc);
	return true;
}
void CLogErrorLife::DetachReflector()
{
	m_pLogFile->UnAttachReflector();
}
bool CLogErrorLife::SetTemporyWarningLevel(BYTE ciLowestWarningLevel/*=CLogFile::WARNING_LEVEL_ALL*/)
{
	if(m_pLogFile==NULL)
		return false;
	m_ciWarningLevel=m_pLogFile->WarningLevel();
	return m_bRestoreWarningLevel=true;
}
bool CLogErrorLife::EnableExportLogTempory()
{
	if(m_pLogFile==NULL)
		return false;
	m_bLegacyEnableState=m_pLogFile->IsEnabled();
	return m_bRestoreEnableState=true;
}
