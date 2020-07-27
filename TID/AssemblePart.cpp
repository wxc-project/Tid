#include "StdAfx.h"
#include "TID.h"
#include "AssemblePart.h"
#include "XhCharString.h"

//////////////////////////////////////////////////////////////////////////
//ͨ������Id��ȡ����ֵ
static CXhChar16 QuerySteelMatMark(char cMat)
{
	CXhChar16 sMatMark;
	if('H'==cMat)
		sMatMark.Copy("Q345");
	else if('G'==cMat)
		sMatMark.Copy("Q390");
	else if('P'==cMat)
		sMatMark.Copy("Q420");
	else if('T'==cMat)
		sMatMark.Copy("Q460");
	else //if('S'==cMat)
		sMatMark.Copy("Q235");
	return sMatMark;
}
//////////////////////////////////////////////////////////////////////////
//CAssemblePart
IMPLEMENT_PROP_FUNC(CAssemblePart)
CAssemblePart::CAssemblePart(ITidAssemblePart *pAssemblePart)
{
	m_pAssemblePart=pAssemblePart;
	m_pTidPart=pAssemblePart->GetPart();
}
//��ʼ������id
const int HASHTABLESIZE=500;
const int STATUSHASHTABLESIZE=500;
void CAssemblePart::InitPropHashtable()
{
	int id=1;
	propHashtable.SetHashTableGrowSize(HASHTABLESIZE);
	propStatusHashtable.CreateHashTable(STATUSHASHTABLESIZE);
	AddPropItem("basicInfo",PROPLIST_ITEM(id++,"������Ϣ"));
	AddPropItem("m_sPartNo",PROPLIST_ITEM(id++,"����"));
	AddPropItem("m_id",PROPLIST_ITEM(id++,"���"));
	AddPropItem("m_Seg",PROPLIST_ITEM(id++,"�κ�"));
	AddPropItem("m_cMaterial",PROPLIST_ITEM(id++,"����"));
	AddPropItem("m_sLayer",PROPLIST_ITEM(id++,"ͼ��"));
	AddPropItem("m_sSpec",PROPLIST_ITEM(id++,"���"));
	AddPropItem("m_wThick",PROPLIST_ITEM(id++,"���"));
	AddPropItem("m_wLength",PROPLIST_ITEM(id++,"����"));
	AddPropItem("m_fWeight",PROPLIST_ITEM(id++,"����"));
	
	AddPropItem("acs",PROPLIST_ITEM(id++,"װ�䷽λ","װ�䷽λ"));
	AddPropItem("acs.origin",PROPLIST_ITEM(id++,"λ��"));
	AddPropItem("acs.origin.x",PROPLIST_ITEM(id++,"X"));
	AddPropItem("acs.origin.y",PROPLIST_ITEM(id++,"Y"));
	AddPropItem("acs.origin.z",PROPLIST_ITEM(id++,"Z"));
	AddPropItem("acs.axisX",PROPLIST_ITEM(id++,"X�᷽��"));
	AddPropItem("acs.axisX.x",PROPLIST_ITEM(id++,"X"));
	AddPropItem("acs.axisX.y",PROPLIST_ITEM(id++,"Y"));
	AddPropItem("acs.axisX.z",PROPLIST_ITEM(id++,"Z"));
	AddPropItem("acs.axisY",PROPLIST_ITEM(id++,"Y�᷽��"));
	AddPropItem("acs.axisY.x",PROPLIST_ITEM(id++,"X"));
	AddPropItem("acs.axisY.y",PROPLIST_ITEM(id++,"Y"));
	AddPropItem("acs.axisY.z",PROPLIST_ITEM(id++,"Z"));
	AddPropItem("acs.axisZ",PROPLIST_ITEM(id++,"Z�᷽��"));
	AddPropItem("acs.axisZ.x",PROPLIST_ITEM(id++,"X"));
	AddPropItem("acs.axisZ.y",PROPLIST_ITEM(id++,"Y"));
	AddPropItem("acs.axisZ.z",PROPLIST_ITEM(id++,"Z"));
}
int CAssemblePart::GetPropValueStr(long id, char *valueStr,UINT nMaxStrBufLen/*=100*/)
{
	CXhChar100 sText;
	if(m_pTidPart)
	{
		if(GetPropID("m_sPartNo")==id)
			m_pTidPart->GetLabel(sText);
		else if(GetPropID("m_Seg")==id)
			m_pTidPart->GetSegStr(sText);
		else if(GetPropID("m_cMaterial")==id)	
			sText=QuerySteelMatMark(m_pTidPart->GetBriefMark());
		else if(GetPropID("m_sSpec")==id)
		{
			if(m_pTidPart->GetPartType()==1)
				sprintf(sText,"%gX%g",m_pTidPart->GetWidth(),m_pTidPart->GetThick());
			else
				m_pTidPart->GetSpec(sText);
		}
		else if(GetPropID("m_wThick")==id)
			sText.Printf("%d",m_pTidPart->GetThick());
		else if(GetPropID("m_wLength")==id)
			sText.Printf("%d",m_pTidPart->GetLength());
		else if(GetPropID("m_fWeight")==id)
		{
			sText.Printf("%f",m_pTidPart->GetWeight());
			//SimplifiedNumString(sText);
		}
	}
	if(m_pAssemblePart)
	{
		TID_CS acs=m_pAssemblePart->GetAcs();
		if(GetPropID("acs.origin")==id)
			sText.Copy(CXhChar100(acs.origin));
		else if(GetPropID("acs.origin.x")==id)
		{
			sprintf(sText,"%g",acs.origin.x);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.origin.y")==id)
		{
			sprintf(sText,"%g",acs.origin.y);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.origin.z")==id)
		{
			sprintf(sText,"%g",acs.origin.z);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisX")==id)
			sText.Copy(CXhChar100(acs.axisX));
		else if(GetPropID("acs.axisX.x")==id)
		{
			sprintf(sText,"%g",acs.axisX.x);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisX.y")==id)
		{
			sprintf(sText,"%g",acs.axisX.y);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisX.z")==id)
		{
			sprintf(sText,"%g",acs.axisX.z);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisY")==id)
			sText.Copy(CXhChar100(acs.axisY));
		else if(GetPropID("acs.axisY.x")==id)
		{
			sprintf(sText,"%g",acs.axisY.x);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisY.y")==id)
		{
			sprintf(sText,"%g",acs.axisY.y);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisY.z")==id)
		{
			sprintf(sText,"%g",acs.axisY.z);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisZ")==id)
			sText.Copy(CXhChar100(acs.axisZ));
		else if(GetPropID("acs.axisZ.x")==id)
		{
			sprintf(sText,"%g",acs.axisZ.x);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisZ.y")==id)
		{
			sprintf(sText,"%g",acs.axisZ.y);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisZ.z")==id)
		{
			sprintf(sText,"%g",acs.axisZ.z);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("m_sLayer")==id)
			m_pAssemblePart->GetLayer(sText);
		else if(GetPropID("m_id")==id)
			sprintf(sText,"%d",m_pAssemblePart->GetId());
	}
	if(valueStr)
		StrCopy(valueStr,sText,nMaxStrBufLen);
	return strlen(sText);
}
//////////////////////////////////////////////////////////////////////////
//
IMPLEMENT_PROP_FUNC(CAssembleBolt)
CAssembleBolt::CAssembleBolt(ITidAssembleBolt *pAssembleBolt)
{
	m_pAssembleBolt=pAssembleBolt;
	m_pTidBolt=pAssembleBolt->GetTidBolt();
}
void CAssembleBolt::InitPropHashtable()
{
	int id=1;
	propHashtable.SetHashTableGrowSize(HASHTABLESIZE);
	propStatusHashtable.CreateHashTable(STATUSHASHTABLESIZE);
	AddPropItem("basicInfo",PROPLIST_ITEM(id++,"������Ϣ"));
	AddPropItem("m_sBoltD",PROPLIST_ITEM(id++,"��˨ֱ��"));
	AddPropItem("m_sBoltSpec",PROPLIST_ITEM(id++,"��˨���"));
	AddPropItem("m_sBoltFamily",PROPLIST_ITEM(id++,"��˨ϵ��"));
	//
	AddPropItem("acs",PROPLIST_ITEM(id++,"װ�䷽λ","װ�䷽λ"));
	AddPropItem("acs.origin",PROPLIST_ITEM(id++,"λ��"));
	AddPropItem("acs.origin.x",PROPLIST_ITEM(id++,"X"));
	AddPropItem("acs.origin.y",PROPLIST_ITEM(id++,"Y"));
	AddPropItem("acs.origin.z",PROPLIST_ITEM(id++,"Z"));
	AddPropItem("acs.axisX",PROPLIST_ITEM(id++,"X�᷽��"));
	AddPropItem("acs.axisX.x",PROPLIST_ITEM(id++,"X"));
	AddPropItem("acs.axisX.y",PROPLIST_ITEM(id++,"Y"));
	AddPropItem("acs.axisX.z",PROPLIST_ITEM(id++,"Z"));
	AddPropItem("acs.axisY",PROPLIST_ITEM(id++,"Y�᷽��"));
	AddPropItem("acs.axisY.x",PROPLIST_ITEM(id++,"X"));
	AddPropItem("acs.axisY.y",PROPLIST_ITEM(id++,"Y"));
	AddPropItem("acs.axisY.z",PROPLIST_ITEM(id++,"Z"));
	AddPropItem("acs.axisZ",PROPLIST_ITEM(id++,"Z�᷽��"));
	AddPropItem("acs.axisZ.x",PROPLIST_ITEM(id++,"X"));
	AddPropItem("acs.axisZ.y",PROPLIST_ITEM(id++,"Y"));
	AddPropItem("acs.axisZ.z",PROPLIST_ITEM(id++,"Z"));
}
int CAssembleBolt::GetPropValueStr(long id, char *valueStr,UINT nMaxStrBufLen/*=100*/)
{
	CXhChar100 sText;
	if(m_pTidBolt)
	{
		if(GetPropID("m_sBoltD")==id)
			sText.Printf("%d",m_pTidBolt->GetDiameter());
		else if(GetPropID("m_sBoltSpec")==id)
			m_pTidBolt->GetSpec(sText);
		else if(GetPropID("m_sBoltFamily")==id)
			sText.Printf("%d",m_pTidBolt->GetSeriesId());
	}
	if(m_pAssembleBolt)
	{
		TID_CS acs=m_pAssembleBolt->GetAcs();
		if(GetPropID("acs.origin")==id)
			sText.Copy(CXhChar100(acs.origin));
		else if(GetPropID("acs.origin.x")==id)
		{
			sprintf(sText,"%g",acs.origin.x);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.origin.y")==id)
		{
			sprintf(sText,"%g",acs.origin.y);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.origin.z")==id)
		{
			sprintf(sText,"%g",acs.origin.z);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisX")==id)
			sText.Copy(CXhChar100(acs.axisX));
		else if(GetPropID("acs.axisX.x")==id)
		{
			sprintf(sText,"%g",acs.axisX.x);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisX.y")==id)
		{
			sprintf(sText,"%g",acs.axisX.y);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisX.z")==id)
		{
			sprintf(sText,"%g",acs.axisX.z);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisY")==id)
			sText.Copy(CXhChar100(acs.axisY));
		else if(GetPropID("acs.axisY.x")==id)
		{
			sprintf(sText,"%g",acs.axisY.x);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisY.y")==id)
		{
			sprintf(sText,"%g",acs.axisY.y);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisY.z")==id)
		{
			sprintf(sText,"%g",acs.axisY.z);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisZ")==id)
			sText.Copy(CXhChar100(acs.axisZ));
		else if(GetPropID("acs.axisZ.x")==id)
		{
			sprintf(sText,"%g",acs.axisZ.x);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisZ.y")==id)
		{
			sprintf(sText,"%g",acs.axisZ.y);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisZ.z")==id)
		{
			sprintf(sText,"%g",acs.axisZ.z);
			//SimplifiedNumString(sText);
		}
	}
	if(valueStr)
		StrCopy(valueStr,sText,nMaxStrBufLen);
	return strlen(sText);
}
//////////////////////////////////////////////////////////////////////////
//CAssembleAnchorBolt
IMPLEMENT_PROP_FUNC(CAssembleAnchorBolt)
CAssembleAnchorBolt::CAssembleAnchorBolt(ITidAssembleAnchorBolt *pAssembleAnchor)
{
	m_pAssembleAnchor=pAssembleAnchor;
	m_pAnchorBolt=pAssembleAnchor->GetAnchorBolt();
}
void CAssembleAnchorBolt::InitPropHashtable()
{
	int id=1;
	propHashtable.SetHashTableGrowSize(HASHTABLESIZE);
	propStatusHashtable.CreateHashTable(STATUSHASHTABLESIZE);
	//������Ϣ
	AddPropItem("basicInfo",PROPLIST_ITEM(id++,"�ؽ���˨������Ϣ"));
	AddPropItem("d",PROPLIST_ITEM(id++,"��˨ֱ��"));
	AddPropItem("wiLe",PROPLIST_ITEM(id++,"��¶����"));
	AddPropItem("wiLa",PROPLIST_ITEM(id++,"�޿۳���"));
	AddPropItem("wiWidth",PROPLIST_ITEM(id++,"�����"));
	AddPropItem("wiThick",PROPLIST_ITEM(id++,"�����"));
	AddPropItem("wiHoleD",PROPLIST_ITEM(id++,"���׾�"));
	AddPropItem("wiBPHoleD",PROPLIST_ITEM(id++,"������׾�"));
	AddPropItem("SizeSymbol",PROPLIST_ITEM(id++,"����ʶ"));
	//װ�䷽λ
	AddPropItem("acs",PROPLIST_ITEM(id++,"�ؽ���˨װ�䷽λ","װ�䷽λ"));
	AddPropItem("acs.origin",PROPLIST_ITEM(id++,"λ��"));
	AddPropItem("acs.origin.x",PROPLIST_ITEM(id++,"X"));
	AddPropItem("acs.origin.y",PROPLIST_ITEM(id++,"Y"));
	AddPropItem("acs.origin.z",PROPLIST_ITEM(id++,"Z"));
	AddPropItem("acs.axisX",PROPLIST_ITEM(id++,"X�᷽��"));
	AddPropItem("acs.axisX.x",PROPLIST_ITEM(id++,"X"));
	AddPropItem("acs.axisX.y",PROPLIST_ITEM(id++,"Y"));
	AddPropItem("acs.axisX.z",PROPLIST_ITEM(id++,"Z"));
	AddPropItem("acs.axisY",PROPLIST_ITEM(id++,"Y�᷽��"));
	AddPropItem("acs.axisY.x",PROPLIST_ITEM(id++,"X"));
	AddPropItem("acs.axisY.y",PROPLIST_ITEM(id++,"Y"));
	AddPropItem("acs.axisY.z",PROPLIST_ITEM(id++,"Z"));
	AddPropItem("acs.axisZ",PROPLIST_ITEM(id++,"Z�᷽��"));
	AddPropItem("acs.axisZ.x",PROPLIST_ITEM(id++,"X"));
	AddPropItem("acs.axisZ.y",PROPLIST_ITEM(id++,"Y"));
	AddPropItem("acs.axisZ.z",PROPLIST_ITEM(id++,"Z"));
}
int CAssembleAnchorBolt::GetPropValueStr(long id, char *valueStr,UINT nMaxStrBufLen/*=100*/)
{
	CXhChar100 sText;
	if(m_pAnchorBolt)
	{
		if(GetPropID("m_sBoltD")==id)
			sText.Printf("%d",m_pAnchorBolt->GetDiameter());
		else if(GetPropID("wiLe")==id)
			sText.Printf("%d",m_pAnchorBolt->GetLenValid());
		else if(GetPropID("wiLa")==id)
			sText.Printf("%d",m_pAnchorBolt->GetLenNoneThread());
		else if(GetPropID("wiWidth")==id)
			sText.Printf("%d",m_pAnchorBolt->GetPadWidth());
		else if(GetPropID("wiThick")==id)
			sText.Printf("%d",m_pAnchorBolt->GetPadThick());
		else if(GetPropID("wiHoleD")==id)
			sText.Printf("%d",m_pAnchorBolt->GetHoleD());
		else if(GetPropID("wiBPHoleD")==id)
			sText.Printf("%d",m_pAnchorBolt->GetBasePlateHoleD());
		else if(GetPropID("SizeSymbol")==id)
			sText.Printf("%s",m_pAnchorBolt->GetSizeSymbol());
	}
	if(m_pAssembleAnchor)
	{
		TID_CS acs=m_pAssembleAnchor->GetAcs();
		if(GetPropID("acs.origin")==id)
			sText.Copy(CXhChar100(acs.origin));
		else if(GetPropID("acs.origin.x")==id)
		{
			sprintf(sText,"%g",acs.origin.x);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.origin.y")==id)
		{
			sprintf(sText,"%g",acs.origin.y);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.origin.z")==id)
		{
			sprintf(sText,"%g",acs.origin.z);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisX")==id)
			sText.Copy(CXhChar100(acs.axisX));
		else if(GetPropID("acs.axisX.x")==id)
		{
			sprintf(sText,"%g",acs.axisX.x);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisX.y")==id)
		{
			sprintf(sText,"%g",acs.axisX.y);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisX.z")==id)
		{
			sprintf(sText,"%g",acs.axisX.z);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisY")==id)
			sText.Copy(CXhChar100(acs.axisY));
		else if(GetPropID("acs.axisY.x")==id)
		{
			sprintf(sText,"%g",acs.axisY.x);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisY.y")==id)
		{
			sprintf(sText,"%g",acs.axisY.y);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisY.z")==id)
		{
			sprintf(sText,"%g",acs.axisY.z);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisZ")==id)
			sText.Copy(CXhChar100(acs.axisZ));
		else if(GetPropID("acs.axisZ.x")==id)
		{
			sprintf(sText,"%g",acs.axisZ.x);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisZ.y")==id)
		{
			sprintf(sText,"%g",acs.axisZ.y);
			//SimplifiedNumString(sText);
		}
		else if(GetPropID("acs.axisZ.z")==id)
		{
			sprintf(sText,"%g",acs.axisZ.z);
			//SimplifiedNumString(sText);
		}
	}
	if(valueStr)
		StrCopy(valueStr,sText,nMaxStrBufLen);
	return strlen(sText);
}