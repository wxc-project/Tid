// XhCharString.h: interface for the CXhCharString class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XHCHARSTRING_H__5771CACC_35CE_4B7B_BB64_6E180FDB0231__INCLUDED_)
#define AFX_XHCHARSTRING_H__5771CACC_35CE_4B7B_BB64_6E180FDB0231__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "stdio.h"
#include "string.h"

#pragma warning(disable: 4996)
char* StrCopy(char* strDest,const char* strSource,int lengthOfDestBuf);
//不能使用虚函数，否则在用户使用memset()时容易导致异常出现
struct IXhString
{
protected:
	virtual char* GetBuffer()const=0;
	virtual int InternalLengthMax()=0;			//获取许可最大长度
public:
	virtual int GetLengthMax(){return InternalLengthMax();}			//获取许可最大长度
	virtual int Printf(char *format,...);		//写入内容
	virtual char* Copy(const char *);			//复制
	virtual char* NCopy(const char *src_str, WORD length=0xFFFF);			//复制
	virtual char* InsertBefore(const char *,int insert_pos=0);		//插入拼接
	virtual char* InsertBefore(const char ch,int insert_pos=0);		//插入拼接
	virtual char* AppendFormat(const char *format,...);	//拼接格式化字符串
	virtual char* Append(const char *src_str,char cInsertSplitter);	//拼接
	virtual char* Append(const char ch,char cInsertSplitter=0);		//拼接
	//pcNew为-1时,相当于Remove(char pcOld) wjh-2014.2.24
	virtual int Replace(char pcOld,char pcNew);
	virtual int Replace(const char* psOld,const char* psNew);
	virtual int TrimLeft(char chTarget=' ');
	virtual int TrimRight(char chTarget=' ');
	virtual char& At(long i);			//获取指定索引位置处的字符
	virtual bool Equal(const char* src_str){return strcmp(GetBuffer(),src_str)==0;}
	virtual bool EqualNoCase(const char* src_str){return _stricmp(GetBuffer(),src_str)==0;}
	virtual char* operator =(const char *src_str){return Copy(src_str);}
	virtual char* operator +=(const char *src_str){return AppendFormat(src_str);}
	virtual operator char*() const{return GetBuffer();};
	virtual int GetLength(){return (int)strlen(GetBuffer());}	//获取长度
	virtual int ResizeLength(int length,char fillChar=' ',bool fillBefore=false);
	virtual void Empty(){strcpy(GetBuffer(),"");}
	virtual bool StartWith(char c);
	virtual bool EndWith(char c);
#ifdef __ENABLE_XHSTRING_LENGTH_PROPERTY_
	__declspec( property(get=GetLength))	int Length;	//字符串长度
	__declspec( property(get=GetLengthMax)) int LengthMax;//最大容纳字符串长度
#else
	int Length(){return GetLength();}
	int LengthMax(){return GetLengthMax();}
#endif
	static void ResizeString(char* data,int length,char fillChar,bool fillBefore);
};
class CXhString : public IXhString
{
	char *lpString;
	int buf_len;
	char* current;
	virtual int InternalLengthMax(){return buf_len-1;}			//获取许可最大长度
public:
	virtual char* GetBuffer()const{return lpString;}
public:
	CXhString();
	CXhString(char* str_buf,int buf_len){AttachStringBuffer(str_buf,buf_len);}
	CXhString(char* str_buf,int buf_len,double f);	//直接将浮点数转换为只含有效数字在内的字符串
	CXhString(char* str_buf,int buf_len,char *format,...);
	//操作符函数好像无法继承，如无定义直接调用CXhString::operator =时编译会出错　wjh-2017.7.14
	virtual char* operator =(const char *src_str){return Copy(src_str);}
	virtual char* operator +=(const char *src_str){return AppendFormat(src_str);}
	void AttachStringBuffer(char* str_buf,int buf_len);
	char* StrToKey(char* strToken,const char* strDelimit);
};

template<class TYPE> class CXhCharTempl
{
protected:
	char data[TYPE::BUF_LEN];//[101];
public:
	CXhCharTempl(){memset(data,0,TYPE::BUF_LEN);}
	CXhCharTempl(const CXhCharTempl<TYPE>& str){
		memset(data,0,TYPE::BUF_LEN);
		Copy(str);
	}
	CXhCharTempl(double f)	//直接将浮点数转换为只含有效数字在内的字符串
	{
		memset(data,0,TYPE::BUF_LEN);
		if(f==0)
		{
			Copy("0");
			return;
		}
		else if(f>-1e-6&&f<1e-6)
		{
			Printf("%E",f);
			size_t str_len=strlen(data);
			char* splitDot=strchr(data,'.');
			char* splitE=strchr(data,'E');
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
				data[str_len-sub_len]=0;
			}
			return;
		}
		Printf("%lf",f);
		char* split=strchr(data,'.');
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
	CXhCharTempl(const double* coord,int dimension=3)
	{
		ConvertFromPoint(coord,dimension);
	}
	//(char *format,...)会将某些出错信息中的'%'类字符转义抹掉,如有此类字符串时请用operator=()赋值 wjh-2014.5.23
	CXhCharTempl(const char *format,...)
	{
		memset(data,0,TYPE::BUF_LEN);
		if(format==NULL||strlen(format)==0)
			strcpy(data,"");
		else
		{
			va_list marker;
			va_start(marker, format);
			int valid_len=GetLengthMax();
			_vsnprintf((char*)*this,valid_len,format,marker);
		}
	}
	int Printf(const char *format,...)		//写入内容
	{
		va_list marker;
		va_start(marker, format);
		int valid_len=GetLengthMax();
		return _vsnprintf((char*)*this,valid_len,format,marker);
	}
	char* Copy(const char *src_str)			//复制
	{
		if(src_str==NULL)
		{
			memset(data,0,GetLengthMax());
			return NULL;
		}
		int n_s=(int)strlen(src_str);
		int n=GetLengthMax();
		if(n>n_s)
			return strcpy((char*)*this,src_str);
		else
			return strncpy((char*)*this,src_str,n);
	}
	char* NCopy(const char *src_str, WORD length=0xFFFF)			//复制
	{
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
	char* NCopy(char** p_src_str, WORD length,bool move_srcstr)			//复制
	{
		if(p_src_str==NULL||*p_src_str==NULL)
		{
			memset(data,0,GetLengthMax()+1);
			return NULL;
		}
		if(length>GetLengthMax())
			length=GetLengthMax();
		memcpy(data,*p_src_str,length);
		data[length]=0;
		*p_src_str+=length;
		return data;
	}
	char* InsertBefore(const char *prefixStr,int insert_pos=0)		//插入拼接
	{
		if(prefixStr==NULL)
			return (char*)*this;	//之所以使用(char*)*this而不用data是因为将来可能会统一由CXhCharString派生,那时就不一定有data变量存在 wjh-2013.3.16
		int str_len=(int)strlen(prefixStr);
		if(str_len+GetLength()<=GetLengthMax())
			memmove(data+str_len,data,GetLength()+1);
		else			
			memmove(data+str_len,data,GetLengthMax()-str_len);
		memcpy(data,prefixStr,str_len);
		return (char*)*this;	//之所以使用(char*)*this而不用data是因为将来可能会统一由CXhCharString派生,那时就不一定有data变量存在 wjh-2013.3.16
	}
	char* InsertBefore(const char ch,int insert_pos=0)		//插入拼接
	{
		if(GetLength()<GetLengthMax())
			memmove(data+1,data,GetLength()+1);
		else			
			memmove(data+1,data,GetLengthMax()-1);
		data[0]=ch;
		return (char*)*this;	//之所以使用(char*)*this而不用data是因为将来可能会统一由CXhCharString派生,那时就不一定有data变量存在 wjh-2013.3.16
	}
	//cInsertSplitter!=0时如果字符串已有值就插入相应的分隔符 wjh-2016.12.12
	char* Append(const char *src_str,char cInsertSplitter=0)			//拼接
	{
		int n_s=(int)strlen(src_str);
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
	char* Append(const char ch,char cInsertSplitter=0)		//拼接
	{
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
	void ToUpper()
	{
		int len=GetLength();
		char diff='A'-'a';
		for(int i=0;i<len;i++)
		{
			if(data[i]>='a'&&data[i]<='z')
				data[i]+=diff;
		}
	}
	void ToLower()
	{
		int len=GetLength();
		char diff='a'-'A';
		for(int i=0;i<len;i++)
		{
			if(data[i]>='A'&&data[i]<='Z')
				data[i]+=diff;
		}
	}
	int Remove(char pcRemove)
	{
		int len=GetLength();
		int hits=0;
		for(int i=0;i<len;i++)
		{
			if(data[i]==pcRemove)
			{
				memmove(data+i,data+i+1,len-i);
				i--;	//循环结束会加1,不提前减1检测时会跳过个别字符 wjh-2019.4.23
				len--;
				hits++;
			}
		}
		return hits;
	}
	int Replace(char pcOld,char pcNew)
	{
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
	int Replace(const char* psOld,const char* psNew)
	{
		if(psOld==NULL||psNew==NULL)
			return 0;
		int oldstr_len=(int)strlen(psOld);
		if(oldstr_len==0||GetLength()==0)
			return 0;
		if(strcmp(psNew,psOld)==0)
			return 0;	//新旧字符串完全相同
		int newstr_len=(int)strlen(psNew);
		int hits=0;
		while(true){
			char* psFind=strstr(data,psOld);
			if(psFind==NULL)	//未找到待替换字符串，结束替换
				return hits;
			hits++;
			int remnant_len=GetLength()-(int)(psFind-data)-oldstr_len;	//当前发现的待替换字符串右侧剩余长度
			if(oldstr_len!=newstr_len)
			{	//新旧字符串长度不同需要对原字符串进行移位
				int right_offset_len=newstr_len-oldstr_len;	//字符串右移量即字符串增长量
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
	bool Equal(const char* src_str){return strcmp(data,src_str)==0;}
	bool EqualNoCase(const char* src_str){return _stricmp(data,src_str)==0;}
    char& At(long i)			//获取指定索引位置处的字符
	{
		if(i<0||i>=GetLengthMax())
			throw "字符串索引地址超出界限！";
		return ((char*)*this)[i];
	}
	//CXhCharTempl<TYPE>& operator = (const CXhCharTempl<TYPE>& rightStr);
	char* operator =(const char *src_str){return Copy(src_str);}
	char* operator +=(const char *src_str){return Append(src_str);}
	operator char*() const{return (char*)data;};
	int GetLength() const{return (data[0]==0)?0:(int)strlen(data);}	//获取长度
	int GetLengthMax(){return TYPE::BUF_LEN-1;}//获取许可最大长度
#ifdef __ENABLE_XHSTRING_LENGTH_PROPERTY_
	__declspec( property(get=GetLength))	int Length;	//字符串长度
	__declspec( property(get=GetLengthMax)) int LengthMax;//最大容纳字符串长度
#else
	int Length(){return GetLength();}
	int LengthMax(){return GetLengthMax();}
#endif
	int ResizeLength(int length,char fillChar=' ',bool fillBefore=false)
	{
		if(length>100)
			length=100;
		else if(length<0)
			length=0;
		IXhString::ResizeString(data,length,fillChar,fillBefore);
		return length;
	}
	void Empty(){strcpy(data,"");}
	bool StartWith(char c)
	{
		if(GetLength()>0&&data[0]==c)
			return true;
		return false;
	}
	bool EndWith(char c)
	{
		int len=GetLength();
		if(len>0&&data[len-1]==c)
			return true;
		return false;
	}
	int ConvertFromPoint(const double* coord,int dimension=3)
	{
		char sCoord[100]={""};
		for(int i=0;i<dimension;i++)
		{
			if(i==0)
				Copy("(");
			else
				Append(',');
			sprintf(sCoord,"%.1f",coord[i]);
			char* pchComma=strchr(sCoord,'.');
			if(pchComma&&(*(pchComma+1)==0))
				*pchComma='\0';	//SimplifiedNumString(sCoord);
			Append(sCoord);
		}
		Append(')');
		return GetLength();
	}
};
//将来应通过CXhCharTempl模板完全取代CXhChar16,CXhChar50,CXhChar100,CXhChar200等　wjh-2014.8.2
struct XHSTR_LEN16 {static const int BUF_LEN =  17;};
struct XHSTR_LEN24 {static const int BUF_LEN =  25;};
struct XHSTR_LEN50 {static const int BUF_LEN =  51;};
struct XHSTR_LEN100{static const int BUF_LEN = 101;};
struct XHSTR_LEN200{static const int BUF_LEN = 201;};
struct XHSTR_LEN500{static const int BUF_LEN = 501;};
typedef CXhCharTempl<XHSTR_LEN16> CXhChar16;
typedef CXhCharTempl<XHSTR_LEN24> CXhChar24;
typedef CXhCharTempl<XHSTR_LEN50> CXhChar50;
typedef CXhCharTempl<XHSTR_LEN100> CXhChar100;
typedef CXhCharTempl<XHSTR_LEN200> CXhChar200;
typedef CXhCharTempl<XHSTR_LEN500> CXhChar500;

class DELIMITER_STR : CXhChar16{
protected:
	BYTE cnLength;
public:
	DELIMITER_STR(const char* delimiter){
		Copy(delimiter);
		cnLength=GetLength();
	}
	bool IsDelimiter(char ch)const;
	static UINT ParseWordsFromStr(const char* szCmdStr,char* delimiter=", \t",CXhChar50* xarrWords=NULL,UINT niMaxWords=-1);
};
#endif // !defined(AFX_XHCHARSTRING_H__5771CACC_35CE_4B7B_BB64_6E180FDB0231__INCLUDED_)
