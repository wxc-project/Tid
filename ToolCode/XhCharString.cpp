// XhCharString.cpp: implementation of the CXhCharString class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdlib.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include "XhCharString.h"

#if defined(_DEBUG)&&!defined(_DISABLE_DEBUG_NEW_)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool DELIMITER_STR::IsDelimiter(char ch) const
{
	for(BYTE ci=0;ci<cnLength;ci++)
	{
		if(data[ci]==ch)
			return true;
	}
	return false;
}
UINT DELIMITER_STR::ParseWordsFromStr(const char* szCmdStr,char* delimiter/*=", \t"*/,CXhChar50* xarrWords/*=NULL*/,UINT niMaxWords/*=-1*/)
{
	size_t length=strlen(szCmdStr);
	char strpool[512]={0};
	char* tempstr=strpool;
	if(length>511)
		tempstr=new char[length+1];
	memcpy(tempstr,szCmdStr,length);
	tempstr[length]=0;
	DELIMITER_STR delimit(delimiter);
	CXhChar50 itemstr;
	char* pszIter,*pszWordStart=tempstr;
	UINT hits=0;
	for(pszIter=tempstr;true;pszIter++)
	{
		if(!delimit.IsDelimiter(*pszIter)&&*pszIter!=0)
			continue;
		itemstr.Empty();
		itemstr.NCopy(&pszWordStart,(WORD)(pszIter-pszWordStart),false);
		pszWordStart=pszIter+1;
		if(itemstr.GetLength()==0&&(*pszIter!=' '||*pszIter!='\t'))
			continue;	//	单词间允许出现多个连续空格或tab键
		if(xarrWords!=NULL&&hits<niMaxWords)
			xarrWords[hits]=itemstr;
		hits++;
		if(*pszIter==0)
			break;
	}
	if(tempstr!=strpool)
		delete []tempstr;
	return hits;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
char* StrNCopy(char* strDest, const char* strSource, int niStrLen)
{	//自动strDest[niStrLen]=0
	memcpy(strDest, strSource, niStrLen);
	strDest[niStrLen] = 0;
	return strDest;
}
char* StrCopy(char* strDest,const char* strSource,int lengthOfDestBuf)
{
	if(lengthOfDestBuf<=1)
		return NULL;
	if(strSource==NULL)
	{
		*strDest=0;
		return strDest;
	}
	if(strlen(strSource)<(size_t)lengthOfDestBuf)
		strcpy(strDest,strSource);	//防止lengthOfDestBuf输入错误,实际字符串也并不超过strDest缓存长度时出错 
	else
	{
		strncpy(strDest,strSource,lengthOfDestBuf-1);
		strDest[lengthOfDestBuf-1]='\0';
	}
	return strDest;
}
char* d2str(double fval,bool bPrecisionToExp=false)
{
	static char fstr[17]={0};
	CXhString szText(fstr,17);
	if(fval==0||(fval>-1e-6&&fval<1e-6&&!bPrecisionToExp))
	{
		fstr[0]='0';
		fstr[1]=0;
		return fstr;
	}
	else if(fval>-1e-6&&fval<1e-6)
	{
		szText.Printf("%E",fval);
		size_t str_len=strlen(fstr);
		char* splitDot=strchr(fstr,'.');
		char* splitE=strchr(fstr,'E');
		char *valid;
		for(valid=splitE-1;valid>=splitDot;valid--)
		{
			if(*valid!='0'&&*valid!='.')
				break;
			else
				*valid=0;
		}
		valid++;
		INT_PTR sub_len=splitE-valid;
		if(sub_len>0)
		{
			size_t copy_len=strlen(splitE);
			memmove(valid,splitE,copy_len);
			fstr[str_len-sub_len]=0;
		}
		return fstr;
	}
	szText.Printf("%lf",fval);
	char* split=strchr(fstr,'.');
	if(split==NULL)
		return fstr;
	int len=(int)strlen(split);
	for(int i=len-1;i>0;i--)
	{
		if(split[i]=='0')
		{
			split[i]=0;
			if(i==1)	//无小数部分
				*split=0;
		}
		else
			break;
	}
	return fstr;
}
//<DEVELOP_PROCESS_MARK nameId="CXhCharString::ResizeString">
void IXhString::ResizeString(char* data,int length,char fillChar,bool fillBefore)
{
	int str_len=(int)strlen(data);
	int gap=length-str_len;
	if(gap>0)
	{
		if(fillBefore)
		{
			memmove(&data[gap],data,str_len);
			for(int i=0;i<gap;i++)
				data[i]=fillChar;//' ';
		}
		else
		{
			for(int i=str_len;i<length;i++)
				data[i]=fillChar;//' ';
		}
		data[length]='\0';
	}
}
//</DEVELOP_PROCESS_MARK>
static void ResizeString(char* data,int length,char fillChar,bool fillBefore)
{
	IXhString::ResizeString(data,length,fillChar,fillBefore);
}
int IXhString::Printf(char *format,...)		//写入内容
{
	char* data=GetBuffer();
	va_list marker;
	va_start(marker, format);
	int valid_len=GetLengthMax();
	return _vsnprintf(data,valid_len,format,marker);
}
char* IXhString::Copy(const char *src_str)			//复制
{
	char* data=GetBuffer();
	if(src_str==NULL)
	{
		memset(data,0,GetLengthMax()+1);
		return NULL;
	}
	data[GetLengthMax()]=0;
	int n_s=(int)strlen(src_str);
	int n=GetLengthMax();
	if(n>n_s)
		return strcpy(data,src_str);
	else
		return strncpy(data,src_str,n);
}
char* IXhString::NCopy(const char *src_str, WORD length/*=0xFFFF*/)			//复制
{
	char* data=GetBuffer();
	if(src_str==NULL)
	{
		memset(data,0,GetLengthMax()+1);
		return NULL;
	}
	if(length>GetLengthMax())
		length=GetLengthMax();
	memcpy(data,src_str,length);
	data[length]=0;
	return data;
}
char* IXhString::InsertBefore(const char *prefixStr,int insert_pos)		//插入拼接
{
	char* data=GetBuffer();
	if(prefixStr==NULL)
		return data;	//之所以使用(char*)*this而不用data是因为将来可能会统一由CXhCharString派生,那时就不一定有data变量存在 wjh-2013.3.16
	int str_len=(int)strlen(prefixStr);
	if(str_len+GetLength()<=GetLengthMax())
		memmove(data+str_len+insert_pos,data+insert_pos,GetLength()+1-insert_pos);
	else			
		memmove(data+str_len+insert_pos,data+insert_pos,GetLengthMax()-str_len-insert_pos);
	memcpy(data+insert_pos,prefixStr,str_len);
	return data;	//之所以使用(char*)*this而不用data是因为将来可能会统一由CXhCharString派生,那时就不一定有data变量存在 wjh-2013.3.16
}
char* IXhString::InsertBefore(const char ch,int insert_pos)		//插入拼接
{
	char* data=GetBuffer();
	if(GetLength()<GetLengthMax())
		memmove(data+1+insert_pos,data+insert_pos,GetLength()+1-insert_pos);
	else			
		memmove(data+1+insert_pos,data+insert_pos,GetLengthMax()-1-insert_pos);
	data[insert_pos]=ch;
	return data;	//之所以使用(char*)*this而不用data是因为将来可能会统一由CXhCharString派生,那时就不一定有data变量存在 wjh-2013.3.16
}
char* IXhString::AppendFormat(const char *format,...)		//拼接格式化字符串
{
	char appendstr[256]={0};
	va_list marker;
	va_start(marker, format);
	_vsnprintf(appendstr,256,format,marker);
	char* data=GetBuffer();
	int i,buflen=GetLengthMax()+1;
	for(i=0;i<buflen;i++)
	{
		if(data[i]==0)
			break;
	}
	StrCopy(data+i,appendstr,buflen-i);
	return data;
}
char* IXhString::Append(const char *src_str,char cInsertSplitter)
{
	int n_s=(int)strlen(src_str);
	char* data=GetBuffer();
	int n_d=GetLength();
	int n=GetLengthMax();
	if(cInsertSplitter!=0&&n_d>0&&n>n_d+1)
	{
		data[n_d]=cInsertSplitter;
		n_d++;
		data[n_d]=0;
	}
	if(n>n_s+n_d)
		return strcat((char*)*this,src_str);
	else
		return strncat((char*)*this,src_str,n-1-n_d);
}
char* IXhString::Append(const char ch,char cInsertSplitter/*=0*/)
{
	char* data=GetBuffer();
	int n_d=GetLength();
	int n=GetLengthMax();
	if(cInsertSplitter!=0&&n_d>0&&n>n_d+1)
	{
		data[n_d]=cInsertSplitter;
		n_d++;
		data[n_d]=0;
	}
	if(n>n_d+1)
	{
		data[n_d]=ch;
		data[n_d+1]=0;
	}
	return data;
}
//pcNew为-1时,相当于Remove(char pcOld) wjh-2014.2.24
int IXhString::Replace(char pcOld,char pcNew)
{
	char* data=GetBuffer();
	int len=GetLength();
	int hits=0;
	for(int i=0;i<len;i++)
	{
		if(data[i]==pcOld)
		{
			if(pcNew>=0)
				data[i]=pcNew;
			else
			{
				memmove(data+i,data+i+1,len-i);
				i--;	//循环结束会加1,不提前减1检测时会跳过个别字符 wjh-2019.4.23
				len--;
			}
			hits++;
		}
	}
	return hits;
}
int IXhString::Replace(const char* psOld,const char* psNew)
{
	char* data=GetBuffer();
	if(psOld==NULL||psNew==NULL)
		return 0;
	size_t oldstr_len=strlen(psOld);
	if(oldstr_len==0||GetLength()==0)
		return 0;
	if(strcmp(psNew,psOld)==0)
		return 0;	//新旧字符串完全相同
	size_t newstr_len=strlen(psNew);
	int hits=0;
	while(true){
		char* psFind=strstr(data,psOld);
		if(psFind==NULL)	//未找到待替换字符串，结束替换
			return 0;
		hits++;
		INT_PTR remnant_len=GetLength()-(psFind-data)-oldstr_len;	//当前发现的待替换字符串右侧剩余长度
		if(oldstr_len!=newstr_len)
		{	//新旧字符串长度不同需要对原字符串进行移位
			INT_PTR right_offset_len=newstr_len-oldstr_len;	//字符串右移量即字符串增长量
			if(right_offset_len+GetLength()<=GetLengthMax())
			{
				int old_len=GetLength();
				memmove(psFind+newstr_len,psFind+oldstr_len,remnant_len);
				data[old_len+right_offset_len]=0;
			}
			else
				memmove(psFind+newstr_len,psFind+oldstr_len,remnant_len+(GetLengthMax()-GetLength()-right_offset_len));
		}
		if(newstr_len>0)
			memcpy(psFind,psNew,newstr_len);
	};
	return hits;
}
//TODO:TrimLeft及TriRight还未经测试 wjh-2014.2.24
int IXhString::TrimLeft(char chTarget/*=' '*/)
{
	char* data=GetBuffer();
	int firstValidChar=0;
	while(firstValidChar<GetLengthMax()&&data[firstValidChar]==chTarget)
		firstValidChar++;
	int length=GetLength()-firstValidChar;
	memmove(data,data+firstValidChar,length);
	data[length]=0;
	return length;
}
int IXhString::TrimRight(char chTarget/*=' '*/)
{
	char* data=GetBuffer();
	int firstValidChar=GetLengthMax()-1;
	while(firstValidChar>=0&&data[firstValidChar]==chTarget)
		firstValidChar--;
	data[firstValidChar+1]=0;
	return firstValidChar+1;
}
char& IXhString::At(long i)			//获取指定索引位置处的字符
{
	if(i<0||i>=GetLengthMax())
		throw "字符串索引地址超出界限！";
	char* data=GetBuffer();
	return data[i];
}
int IXhString::ResizeLength(int length,char fillChar/*=' '*/,bool fillBefore/*=false*/)
{
	if(length>GetLengthMax())
		length=GetLengthMax();
	else if(length<0)
		length=0;
	ResizeString(GetBuffer(),length,fillChar,fillBefore);
	return length;
}
bool IXhString::StartWith(char c)
{
	char* data=GetBuffer();
	if(GetLength()>0&&data[0]==c)
		return true;
	return false;
}
bool IXhString::EndWith(char c)
{
	char* data=GetBuffer();
	int len=GetLength();
	if(len>0&&data[len-1]==c)
		return true;
	return false;
}

CXhString::CXhString()
{
	current=lpString=NULL;
	buf_len=0;
}
CXhString::CXhString(char* str_buf,int length,char *format,...)
{
	AttachStringBuffer(str_buf,length);
	va_list marker;
	va_start(marker, format);
	int valid_len=GetLengthMax();
	str_buf[valid_len]=0;
	_vsnprintf((char*)*this,valid_len,format,marker);
}
//直接将浮点数转换为只含有效数字在内的字符串
CXhString::CXhString(char* str_buf,int length,double f)
{
	AttachStringBuffer(str_buf,length);
	if(f==0)
	{
		Copy("0");
		return;
	}
	else if(fabs(f)<1e-4)	//由于电线的线膨胀系数在1e-6次级，故增大转换范围
	{
		Printf("%e",f);
		int str_len=(int)strlen(lpString);
		char* splitDot=strchr(lpString,'.');
		char* splitE=strchr(lpString,'e');
		char *valid;
		for(valid=splitE-1;valid>=splitDot;valid--)
		{
			if(*valid!='0'&&*valid!='.')
				break;
			else
				*valid=0;
		}
		valid++;
		INT_PTR sub_len=splitE-valid;
		if(sub_len>0)
		{
			size_t copy_len=strlen(splitE);
			memmove(valid,splitE,copy_len);
			lpString[str_len-sub_len]=0;
		}
		return;
	}
	Printf("%lf",f);
	char* split=strchr(lpString,'.');
	if(split==NULL)
		return;
	int len=(int)strlen(split);
	for(int i=len-1;i>0;i--)
	{
		if(split[i]=='0')
		{
			split[i]=0;
			if(i==1)	//无小数部分
				*split=0;
		}
		else
			break;
	}
}
void CXhString::AttachStringBuffer(char* str_buf,int buflen)
{
	current=lpString=str_buf;
	buf_len=buflen;
}
char* CXhString::StrToKey(char* strToken,const char* strDelimit)
{
	if(strToken!=NULL)
		AttachStringBuffer(strToken,(int)strlen(strToken)+1);
	else
		AttachStringBuffer(current,(int)strlen(current)+1);
	int token_len=(int)strlen(lpString);
	char* key=NULL;
	int delim_len=(int)strlen(strDelimit);
	while(current<lpString+token_len)
	{
		bool matched=false;
		for(int j=0;j<delim_len;j++)
		{
			if(*current==strDelimit[j])
			{
				matched=true;
				break;
			}
		}
		if(matched)
		{
			*current=0;
			current++;
			if(key!=NULL)
				break;
			else
				continue;
		}
		else if(key==NULL)
			key=current;
		current++;
	}
	return key;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*
//测试代码
CXhChar16 TestString(CXhChar16 ss,CXhChar16& st) 
{
	ss.Copy("我是“)");
	st.Copy("式工");
	return ss;
}

int TestMain(LPCREATESTRUCT lpCreateStruct)
{
	CXhChar16 s1="asdflk";
	CXhChar16 s2;
	s2=TestString(s1,s1);
	char str[200];
	sprintf(str,"%s", s2);
	s2=s1;
	sprintf(str,"%s", s2);
}
*/