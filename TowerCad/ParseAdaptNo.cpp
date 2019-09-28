#include "stdafx.h"
#ifndef __ATOM_LIST_H_
#include "f_ent_list.h"
#endif
#include "ParseAdaptNo.h"
#include "LogFile.h"

//从各种类型适配号字符串中分解出单个适配号
static ATOM_LIST<long>NoArr;
static ATOM_LIST<SEGI>SegIArr;
bool IsAdaptNoEnd()
{
	return NoArr.GetCursor()==NULL;
}
bool IsAdaptNoEndForSegI()
{
	return SegIArr.GetCursor()==NULL;
}
int GetNoArr(const char *limit_str,int no_arr[],int maxNoN)
{
	int noArr[1000]={0};
	char noStrTemp[1000];
	if(maxNoN>1000)	//再多无实际工程意义
		maxNoN=1000;
	int noN=0;
	strcpy(noStrTemp,limit_str);
	if(strlen(limit_str)>0)
	{
		char *key=strtok(noStrTemp,",");
		while(key!=NULL)
		{
			int i,j,key_len=strlen(key);
			for(i=0;i<key_len;i++)
			{
				if(key[i]=='-')
				{
					key[i]='\0';
					break;
				}
			}
			int min_no=atoi(key);
			int max_no=min_no;
			if(i<key_len)
				max_no=atoi(&key[i+1]);
			for(i=min_no;i<=max_no;i++)
			{
				for(j=0;j<noN;j++)
				{
					if(noArr[j]==i)
						break;
					else if(i<noArr[j])	//&&segN<100
					{
						memmove(&noArr[j+1],&noArr[j],sizeof(int)*(noN-j));
						noArr[j]=i;
						noN++;	//新增一个段号
						break;
					}
				}
				if(j==noN)
				{
					noArr[j]=i;
					noN++;
				}
				if(noN==1000)
					break;
			}
			key=strtok(NULL,",");
		}
		int copy_n=min(noN,maxNoN);
		memcpy(no_arr,noArr,sizeof(int)*copy_n);
	}
	else
		return 0;
	return noN;
}
SEGI FindAdaptNoForSegI(const char *limit_str,const char *delimiter1/*=NULL*/,const char *delimiter2/*=NULL*/)
{
	char *limstr=NULL;
	if(limit_str)
	{
		limstr=new char[strlen(limit_str)+1];
		strcpy(limstr,limit_str);
	}
	SEGI iNo;
	if(limit_str)
	{
		SegIArr.Empty();
		char *sKey;
		if(delimiter1)
			sKey=strtok(limstr,delimiter1);
		else
			sKey=strtok(limstr,".,，\n");
		while(sKey)
		{
			char *delimiter;
			if(delimiter2)
				delimiter=strchr(sKey,*delimiter2);
			else
				delimiter=strchr(sKey,'-');
			if(delimiter)
			{		
				*delimiter=' ';
				SEGI start_i,end_i;
				char start_str[16]="",end_str[16]="";
				sscanf(sKey,"%s%s",&start_str,&end_str);
				start_i=SEGI(start_str);
				end_i=SEGI(end_str);
				if(end_i.iSeg<start_i.iSeg)
					end_i=start_i;
				for(iNo.iSeg=start_i.iSeg;iNo.iSeg<=end_i.iSeg;iNo.iSeg++)
					SegIArr.append(iNo);
			}
			else
			{
				iNo=SEGI(sKey);
				SegIArr.append(iNo);
			}
			sKey=strtok(NULL,".,，\n");
		}
		SEGI *pNo=SegIArr.GetFirst();
		if(pNo)
			iNo=*pNo;
		else
			iNo=SEGI();
	}
	else
	{
		SEGI *pNo=SegIArr.GetNext();
		if(pNo)
			iNo=*pNo;
		else
			iNo=SEGI();
	}
	if(limstr)
		delete []limstr;
	return iNo;
}
long FindAdaptNo(const char *limit_str,char *delimiter1/*=NULL*/,char *delimiter2/*=NULL*/)
{
	char *limstr=NULL;
	if(limit_str)
	{
		limstr=new char[strlen(limit_str)+1];
		strcpy(limstr,limit_str);
	}
	long iNo;
	if(limit_str)
	{
		NoArr.Empty();
		char *sKey;
		if(delimiter1)
			sKey=strtok(limstr,delimiter1);
		else
			sKey=strtok(limstr,",\n");
		while(sKey)
		{
			char *delimiter;
			if(delimiter2)
				delimiter=strchr(sKey,*delimiter2);
			else
				delimiter=strchr(sKey,'-');
			if(delimiter)
			{		
				*delimiter=' ';
				int start_i=0,end_i=0;	//不加默认值在遇到"-9"这样的异常字符串时会出错 wjh-2013.12-20
				sscanf(sKey,"%d%d",&start_i,&end_i);
				if(end_i<start_i)
					end_i=start_i;
				for(iNo=start_i;iNo<=end_i;iNo++)
					NoArr.append(iNo);
			}
			else
			{
				iNo=atoi(sKey);
				NoArr.append(iNo);
			}
			sKey=strtok(NULL,",\n");
		}
		long *pNo=NoArr.GetFirst();
		if(pNo)
			iNo=*pNo;
		else
			iNo=0;
	}
	else
	{
		long *pNo=NoArr.GetNext();
		if(pNo)
			iNo=*pNo;
		else
			iNo=0;
	}
	if(limstr)
		delete []limstr;
	return iNo;
}
//通过段号字符串得到段号哈希表
DWORD GetSegNoHashTblBySegStr(const char* sSegStr,CHashList<SEGI> &segNoHashTbl)
{
	char seg_str[200]="";
	_snprintf(seg_str,199,"%s",sSegStr);
	if(seg_str[0]=='*')
		segNoHashTbl.Empty();
	else
	{
		for(SEGI seg_i=FindAdaptNoForSegI(seg_str,".,，","-");!IsAdaptNoEndForSegI();seg_i=FindAdaptNoForSegI(NULL,".,，","-"))
		{
			DWORD dwSegKey=(seg_i.iSeg==0)?-1:seg_i;
			segNoHashTbl.SetValue(dwSegKey,seg_i);
		}
	}
	return segNoHashTbl.GetNodeNum();
}
//根据段号哈希表获取有序的段号链表
int GetSortedSegNoList(CHashList<SEGI> &segNoHashList,ATOM_LIST<SEGI> &segNoList)
{
	//选择排序分段号
	int n=segNoHashList.GetNodeNum();
	SEGI *seg_i_arr = new SEGI[n];
	int i=0;
	for(SEGI *pSegI=segNoHashList.GetFirst();pSegI;pSegI=segNoHashList.GetNext())
	{
		if(i>0)
		{
			int j;
			for(j=0;j<i;j++)
			{
				if(*pSegI<seg_i_arr[j])
				{
					memmove(&seg_i_arr[j+1],&seg_i_arr[j],(n-j-1)*sizeof(int));
					seg_i_arr[j]=*pSegI;
					break;
				}
			}
			if(j==i)
				seg_i_arr[i]=*pSegI;
		}
		else
			seg_i_arr[i]=*pSegI;
		i++;
	}
	segNoList.Empty();
	for(int i=0;i<n;i++)
		segNoList.append(seg_i_arr[i]);
	delete []seg_i_arr;
	return n;
}
//根据段号哈希表获取段号字符串
CXhChar200 SegNoHashListToString(CHashList<SEGI> &segNoHashList)
{
	ATOM_LIST<SEGI> segNoList;
	GetSortedSegNoList(segNoHashList,segNoList);
	return SortedSegNoListToString(segNoList);
}
//根据有序的段号链表获取段号字符串
CXhChar200 SortedSegNoListToString(ATOM_LIST<SEGI> &segNoList)
{
	CXhChar200 sSegStr;
	SEGI startSegI=-1,endSegI=-1;
	for(SEGI *pSegI=segNoList.GetFirst();pSegI;pSegI=segNoList.GetNext())
	{
		if(startSegI==-1)
			startSegI = *pSegI;
		else if(endSegI==-1&&*pSegI==startSegI+1)
			endSegI = *pSegI;
		else if(endSegI>-1&&*pSegI==endSegI+1)
			endSegI = *pSegI;
		else 
		{
			if(sSegStr.GetLength()>0)
				sSegStr.Append(",");
			if(startSegI<endSegI)	//存在区间段
				sSegStr.Printf("%s%s-%s",(char*)sSegStr,(char*)startSegI.ToString(),(char*)endSegI.ToString());
			else if(startSegI>-1)
				sSegStr.Printf("%s%s",(char*)sSegStr,(char*)startSegI.ToString());
			startSegI = *pSegI;
			endSegI = -1;
		}
	}
	//将最后一段添加到字符串中
	if(sSegStr.GetLength()>0)
		sSegStr.Append(",");
	if(startSegI<endSegI)	//存在区间段
		sSegStr.Printf("%s%s-%s",(char*)sSegStr,(char*)startSegI.ToString(),(char*)endSegI.ToString());
	else if(startSegI>-1)
		sSegStr.Printf("%s%s",(char*)sSegStr,(char*)startSegI.ToString());
	return sSegStr;
}
//根据段号字符串获取有序的段号链表
int GetSortedSegNoList(const char* sSegStr,ATOM_LIST<SEGI> &segNoList)
{
	CHashList<SEGI> segNoHashList;
	GetSegNoHashTblBySegStr(sSegStr,segNoHashList);
	segNoList.Empty();
	GetSortedSegNoList(segNoHashList,segNoList);
	return segNoList.GetNodeNum();
}

//从各种类型适配号字符串中分解出单个适配号
static ATOM_LIST<COEF_NO>CoefNoArr;
COEF_NO* FindAdaptCoefNo(const char *limit_str,const char *delimiter1/*=NULL*/,const char *delimiter2/*=NULL*/,const char *delimiter3/*=NULL*/)
{
	char *limstr=NULL;
	if(limit_str)
	{
		limstr=new char[strlen(limit_str)+1];
		strcpy(limstr,limit_str);
	}
	COEF_NO *pNo=NULL;
	if(limit_str&&strlen(limit_str)>0)
	{
		CoefNoArr.Empty();
		char *sKey;
		char *sCoefKey=NULL;
		if(delimiter1)
			sKey=strtok(limstr,delimiter1);
		else
			sKey=strtok(limstr,",\n");

		if(delimiter3==NULL)
			delimiter3="*:";
		while(sKey)
		{
			double coef=1.0;
			for(int i=0;i<(int)strlen(delimiter3);i++)
			{
				sCoefKey=strchr(sKey,delimiter3[i]);
				if(sCoefKey!=NULL)
				{
					*sCoefKey=0;
					sCoefKey=sCoefKey+1;
					coef=atof(sCoefKey);
					break;
				}
			}
			char *delimiter;
			if(delimiter2)
				delimiter=strchr(sKey,*delimiter2);
			else
				delimiter=strchr(sKey,'-');
			if(delimiter)
			{		
				*delimiter=' ';
				SEGI start_i,end_i;	//不加默认值在遇到"-9"这样的异常字符串时会出错 wjh-2013.12-20
				char start_str[16]="",end_str[16]="";
				sscanf(sKey,"%s%s",&start_str,&end_str);
				start_i=SEGI(start_str);
				end_i=SEGI(end_str);
				if(end_i.iSeg<start_i.iSeg)
					end_i=start_i;
				for(int iNo=start_i.Digital();iNo<=end_i.Digital();iNo++)
				{
					SEGI tempSegI(start_i.key.prefix,iNo);
					CoefNoArr.append(COEF_NO(tempSegI.iSeg,coef));
				}
			}
			else
			{
				int iNo=SEGI(sKey).iSeg;
				CoefNoArr.append(COEF_NO(iNo,coef));
			}
			sKey=strtok(NULL,",\n");
		}
		pNo=CoefNoArr.GetFirst();
	}
	else
		pNo=CoefNoArr.GetNext();
	if(limstr)
		delete []limstr;
	return pNo;
}

//从字符串中提取COEF_NO数组
void RetrievedCoefHashListFromStr(const char* coefNoStr,CHashList<COEF_NO> &hashCoefNoBySegI)
{
	hashCoefNoBySegI.Empty();
	for(COEF_NO* pNo=FindAdaptCoefNo(coefNoStr);pNo;pNo=FindAdaptCoefNo(NULL))
	{
		if(hashCoefNoBySegI.GetValue(pNo->no)==NULL)
		{
			COEF_NO *pCoefNo=hashCoefNoBySegI.Add(pNo->no);
			*pCoefNo=*pNo;
		}
	}
}
//根据COEF_NO哈希表获取有序的COEF_NO链表
int GetSortedCoefNoList(CHashList<COEF_NO> &coefNoHashList,ATOM_LIST<COEF_NO> &coefNoList)
{	//选择排序分段号
	int n=coefNoHashList.GetNodeNum();
	DYN_ARRAY<COEF_NO> coefNoArr(n);
	int i=0;
	for(COEF_NO *pCoefNo=coefNoHashList.GetFirst();pCoefNo;pCoefNo=coefNoHashList.GetNext())
	{
		if(i>0)
		{
			int j;
			for(j=0;j<i;j++)
			{
				SEGI curNo(pCoefNo->no),nextNo(coefNoArr[j].no);
				if(curNo.Digital()<nextNo.Digital())
				{
					memmove(&coefNoArr[j+1],&coefNoArr[j],(n-j-1)*sizeof(COEF_NO));
					coefNoArr[j]=*pCoefNo;
					break;
				}
			}
			if(j==i)
				coefNoArr[i]=*pCoefNo;
		}
		else
			coefNoArr[i]=*pCoefNo;
		i++;
	}
	coefNoList.Empty();
	for(int i=0;i<n;i++)
		coefNoList.append(coefNoArr[i]);
	return n;
}

//根据有序的COEF_NO链表获取COEF_NO字符串
CXhChar200 SortedCoefNoListToString(ATOM_LIST<COEF_NO> &coefNoList,bool bSegI)
{
	CXhChar200 sCoefNoStr;
	COEF_NO startCoefNo(-1,0),endCoefNo(-1,0);
	for(COEF_NO *pCoefNo=coefNoList.GetFirst();pCoefNo;pCoefNo=coefNoList.GetNext())
	{
		if(startCoefNo.no==-1)
			startCoefNo = *pCoefNo;
		else if(endCoefNo.no==-1&&pCoefNo->no==startCoefNo.no+1&&pCoefNo->coef==startCoefNo.coef)
			endCoefNo = *pCoefNo;
		else if(endCoefNo.no>-1&&pCoefNo->no==endCoefNo.no+1&&pCoefNo->coef==startCoefNo.coef)
			endCoefNo = *pCoefNo;
		else 
		{
			if(sCoefNoStr.GetLength()>0)
				sCoefNoStr.Append(",");
			if(startCoefNo.no<endCoefNo.no)	//存在区间段
				sCoefNoStr.Printf("%s%s-%s",(char*)sCoefNoStr,(char*)startCoefNo.ToString(bSegI,false),(char*)endCoefNo.ToString(bSegI));
			else if(startCoefNo.no>-1)
				sCoefNoStr.Printf("%s%s",(char*)sCoefNoStr,(char*)startCoefNo.ToString(bSegI));
			startCoefNo = *pCoefNo;
			endCoefNo.no = -1;
			endCoefNo.coef = 1;
		}
	}
	//将最后一段添加到字符串中
	if(sCoefNoStr.GetLength()>0)
		sCoefNoStr.Append(",");
	if(startCoefNo.no<endCoefNo.no)	//存在区间段
		sCoefNoStr.Printf("%s%s-%s",(char*)sCoefNoStr,(char*)startCoefNo.ToString(bSegI),(char*)endCoefNo.ToString(bSegI));
	else if(startCoefNo.no>-1)
		sCoefNoStr.Printf("%s%s",(char*)sCoefNoStr,(char*)startCoefNo.ToString(bSegI));
	return sCoefNoStr;
}

BOOL TestFindAdaptCoefNo(CXhChar200 sSrc)
{
	CHashList<COEF_NO> hashCoefNoBySegI;
	RetrievedCoefHashListFromStr(sSrc,hashCoefNoBySegI);
	ATOM_LIST<COEF_NO> coefNoList;
	GetSortedCoefNoList(hashCoefNoBySegI,coefNoList);
	CXhChar200 sDest=SortedCoefNoListToString(coefNoList,true);
	if(sSrc.Equal(sDest))
		return TRUE;
	else
		return FALSE;
}

void TestFindAdaptCoefNo()
{
	logerr.InitLogFile("D:\\TestNo.txt");
	if(!TestFindAdaptCoefNo("1-4,6,8-10"))
		logerr.Log("1-4,6,8-10,解析失败！");
	if(!TestFindAdaptCoefNo("1-4*2,5*3,7-10"))
		logerr.Log("1-4*2,5*3,7-10,解析失败！");
	if(!TestFindAdaptCoefNo("A1-A4,D5,C7-C10,11-14"))
		logerr.Log("A1-A4,D5,C7-C10,11-14,解析失败！");
	logerr.ShowToScreen();
}