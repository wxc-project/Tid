#pragma once

#include "SegI.h"
#include "HashTable.h"
#include "ArrayList.h"
#ifndef __ATOM_LIST_H_
#include "f_ent_list.h"
#endif

bool IsAdaptNoEnd();
bool IsAdaptNoEndForSegI();
int GetNoArr(const char *limit_str,int no_arr[],int maxNoN);
long FindAdaptNo(const char *limit_str,char *delimiter1=NULL,char *delimiter2=NULL);
SEGI FindAdaptNoForSegI(const char *limit_str,const char *delimiter1=NULL,const char *delimiter2=NULL);
//通过段号字符串得到段号哈希表
DWORD GetSegNoHashTblBySegStr(const char* sSegStr,CHashList<SEGI> &segNoHashTbl);
//根据段号哈希表获取有序的段号链表
int GetSortedSegNoList(CHashList<SEGI> &segNoHashList,ATOM_LIST<SEGI> &segNoList);
//根据段号哈希表获取段号字符串
CXhChar200 SegNoHashListToString(CHashList<SEGI> &segNoHashList);
//根据有序的段号链表获取段号字符串
CXhChar200 SortedSegNoListToString(ATOM_LIST<SEGI> &segNoList);
//根据段号字符串获取有序的段号链表
int GetSortedSegNoList(const char* sSegStr,ATOM_LIST<SEGI> &segNoList);

struct COEF_NO{
	int no;
	double coef;
	COEF_NO(){coef=no=0;}
	COEF_NO(int no_,double coef_){no=no_;coef=coef_;}
	CXhChar100 ToString(bool bSegI,bool bIncCoef=true){
		CXhChar100 sNo("%d",no);
		if(bSegI)
			sNo.Printf("%s",(char*)SEGI((long)no).ToString());
		if(coef>1&&bIncCoef)
			sNo.Printf("%s*%.f",(char*)sNo,coef);
		return sNo;
	}
};
// <summary>
// 将一个代表含系数的编号序列的字符串解析为逐个含系数编号
// limit_str ：编号序列字符串 </param>
// delimiter1：编号间分隔符，一般为','</param>
// delimiter2：编号范围延续符，一般为'-'</param>
// delimiter3：编号的关联系数分隔，一般为'*'</param>
COEF_NO* FindAdaptCoefNo(const char *limit_str,const char* delimiter1=NULL,const char *delimiter2=NULL,const char *delimiter3=NULL);

//从字符串中提取COEF_NO数组
void RetrievedCoefHashListFromStr(const char* coefNoStr,CHashList<COEF_NO> &hashCoefNoBySegI);
//根据COEF_NO哈希表获取有序的COEF_NO链表
int GetSortedCoefNoList(CHashList<COEF_NO> &coefNoHashList,ATOM_LIST<COEF_NO> &coefNoList);
//根据有序的COEF_NO链表获取COEF_NO字符串
CXhChar200 SortedCoefNoListToString(ATOM_LIST<COEF_NO> &coefNoList,bool bSegI);