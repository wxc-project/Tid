#include "StdAfx.h"
#include ".\TIDBuffer.h"
#include "LogFile.h"

#ifndef TIDCORE_EXPORTS
#include "ParseAdaptNo.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CSubLegFoundationSection::CSubLegFoundationSection(BYTE _cnHeightCount,DWORD zeroAddrOffset,char* buf/*=NULL*/,DWORD size/*=0*/,const char* version/*=NULL*/)
{
	m_dwZeroAddrOffset=zeroAddrOffset;
	m_data=buf;
	m_dwBufSize=size;
	m_sVersion.Copy(version);
	cnHeightCount=_cnHeightCount;
	if(m_data!=NULL)
	{
		ciBasePlateThick =(BYTE)*m_data;
		cnAnchorBoltCount=(BYTE)*(m_data+1);
		cnSubLegCount=*((BYTE*)(buf+2+cnAnchorBoltCount*4+cnHeightCount));
	}
}
ANCHOR_LOCATION CSubLegFoundationSection::GetAnchorLocationAt(short index)	//0 based index;
{
	//SeekPosition(AddrLtoG(2+index*4));
	CBuffer sectbuf(m_data,m_dwBufSize);
	sectbuf.SeekPosition(2+index*4);
	ANCHOR_LOCATION location;
	sectbuf.ReadWord(&location.siPosX);
	sectbuf.ReadWord(&location.siPosY);
	return location;
}
GEPOINT CSubLegFoundationSection::GetSubLegFoundationOrgBySerial(BYTE ciBodySerial,BYTE ciLegSerial)	//1 based serial
{
	CBuffer sectbuf(m_data,m_dwBufSize);
	sectbuf.SeekPosition(2+cnAnchorBoltCount*4+ciBodySerial-1);//(wiLegSerial-1)*24);
	BYTE ciStartIndexofHeightSubLeg=0;
	sectbuf.ReadByte(&ciStartIndexofHeightSubLeg);
	sectbuf.SeekPosition(2+cnAnchorBoltCount*4+cnHeightCount+1+(ciStartIndexofHeightSubLeg+ciLegSerial-1)*24);
	GEPOINT xBaseOrg;
	sectbuf.ReadPoint(xBaseOrg);
	return xBaseOrg;
}
//呼高
CModuleSection::CModuleSection(DWORD zeroAddrOffset,char* buf/*=NULL*/,DWORD size/*=0*/,const char* version/*=NULL*/)
{
	m_dwZeroAddrOffset=zeroAddrOffset;
	m_data=buf;
	m_dwBufSize=size;
	m_sVersion.Copy(version);
	if(m_data!=NULL&&compareVersion(version,"1.4")>=0)
		memcpy(&m_fNamedHeightZeroZ,m_data,8);
	else
		m_fNamedHeightZeroZ=0;
}
BYTE CModuleSection::GetModuleCount()
{
	if(m_data!=NULL&&m_dwBufSize>0)
	{
		if(compareVersion(m_sVersion,"1.4")>=0)
			return *(m_data+8);	//呼高值8B
		else
			return *m_data;
	}
	else
		return 0;
}
TOWER_MODULE CModuleSection::GetModuleAt(int i)
{
	TOWER_MODULE module;
	bool bAfterV11=compareVersion(m_sVersion,"1.1")>=0;
	bool bAfterV14=compareVersion(m_sVersion,"1.4")>=0;
	int j,record_n=25,init_offset=0;
	if(bAfterV14)
	{
		init_offset=8;	
		record_n=30;	//V1.4增加了呼高信息4Ｂ
	}
	else if(bAfterV11)
		record_n=26;	//V1.1增加了减腿信息１Ｂ
	BYTE* pBodyNo=(BYTE*)BufferPtr()+init_offset+1+record_n*i;
	module.m_cBodyNo=*pBodyNo;
	module.m_uiNamedHeight=0;
	if(bAfterV11)//compareVersion(m_sVersion,"1.1")>=0)
	{
		module.m_cbLegInfo=*(pBodyNo+1);
		if(bAfterV14)
			module.m_uiNamedHeight=*((UINT*)(pBodyNo+2));
		BYTE offset=bAfterV14?6:2;
		memcpy(&module.m_dwLegCfgWord,pBodyNo+offset,24);
	}
	else
	{
		module.m_cbLegInfo=0;
		memcpy(&module.m_dwLegCfgWord,pBodyNo+1,24);
	}
	for(j=1;j<=192;j++)
	{
		if(module.m_dwLegCfgWord.IsHasNo(j))
		{
			module.m_arrActiveQuadLegNo[0]=module.m_arrActiveQuadLegNo[1]=
				module.m_arrActiveQuadLegNo[2]=module.m_arrActiveQuadLegNo[3]=j;
			break;
		}
	}
	//读取呼高分组的名称
	char* pNameSection=BufferPtr()+init_offset+1+record_n*GetModuleCount();
	CBuffer pool(pNameSection,GetModuleCount()*100);	//因不知该数据区具体长度，只能给定一个肯定比较大的缓存空间
	for(j=0;j<i;j++)
		pool.Offset(pool.ReadStringLength());
	pool.ReadString(module.name,51);
	return module;
}
CWireNodeSection::CWireNodeSection(DWORD zeroAddrOffset,char* buf/*=NULL*/,DWORD size/*=0*/,const char* version/*=NULL*/)
{
	m_dwZeroAddrOffset=zeroAddrOffset;
	m_data=buf;
	m_dwBufSize=size;
	m_sVersion.Copy(version);
	m_wnWireNodeCount=buf?*((WORD*)buf):0;
}
WIRE_NODE CWireNodeSection::GetWireNodeAt(int i)
{
	CBuffer sectbuf(m_data,m_dwBufSize);
	sectbuf.SeekPosition(2+i*64);	//开头为2个字节，存储挂点数
	WIRE_NODE wirenode;
	sectbuf.ReadWord(&wirenode.wiCode);
	sectbuf.ReadPoint(wirenode.position);
	sectbuf.Read(wirenode.name,38);
	return wirenode;
}
CProjectInfoSection::CProjectInfoSection(DWORD zeroAddrOffset,char* buf/*=NULL*/,DWORD size/*=0*/,const char* version/*=NULL*/)
{
	m_dwZeroAddrOffset=zeroAddrOffset;
	m_data=buf;
	m_dwBufSize=size;
	m_sVersion.Copy(version);
	m_ciSubProjSectionCount=buf?(BYTE)*buf:0;
}
CProjInfoSubSection::CProjInfoSubSection(DWORD zeroAddrOffset,char* buf/*=NULL*/,DWORD size/*=0*/,const char* version/*=NULL*/)
{
	m_dwZeroAddrOffset=zeroAddrOffset;
	m_data=buf;
	m_dwBufSize=size;
	m_sVersion.Copy(version);
}
CProjInfoSubSection CProjectInfoSection::GetSubSectionAt(int i)
{
	CBuffer sectbuf(m_data,m_dwBufSize);
	sectbuf.SeekPosition(1+i*12);
	DWORD uidSubSection,dwSectAddr=0,dwSectSize=0;
	sectbuf.ReadDword(&uidSubSection);
	sectbuf.ReadDword(&dwSectAddr);
	sectbuf.ReadDword(&dwSectSize);
	//TODO:待测试完善
	char* pBuff=m_data+AddrGtoL(dwSectAddr);
	CProjInfoSubSection subsect(dwSectAddr,pBuff,dwSectSize,m_sVersion);
	subsect.m_uidSubSection=uidSubSection;
	return subsect;
}
//部件
CBlockSection::CBlockSection(DWORD zeroAddrOffset,char* buf/*=NULL*/,DWORD size/*=0*/,const char* version/*=NULL*/){
	m_dwZeroAddrOffset=zeroAddrOffset;
	m_data=buf;
	m_dwBufSize=size;
	m_sVersion.Copy(version);
}
TOWER_BLOCK CBlockSection::GetBlockAt(int i)
{
	TOWER_BLOCK block;
	CBuffer buf(BufferPtr(),BufferLength());
	buf.SeekPosition(2+100*i);
	buf.ReadPoint(block.lcs.origin);
	buf.ReadPoint(block.lcs.axis_x);
	buf.ReadPoint(block.lcs.axis_y);
	buf.ReadPoint(block.lcs.axis_z);
	DWORD name_addr;
	buf.ReadDword(&name_addr);
	buf.SeekPosition(AddrGtoL(name_addr));
	buf.ReadString(block.name,50);
	return block;
}
void CBlockSection::GetBlockBufferAt(int index,CBuffer& blockBuf)
{
	blockBuf.ClearContents();
	CBuffer buf(BufferPtr(),BufferLength());
	buf.SeekPosition(2+100*index);
	blockBuf.Write(buf,96);
	DWORD dwAddr;
	buf.ReadDword(&dwAddr);
	buf.SeekPosition(AddrGtoL(dwAddr));
	char name[50]="";
	buf.ReadString(name,50);
	blockBuf.WriteString(name,50);
}
//螺栓系列
CXhChar24 CBoltSeries::GetSeriesName()
{
	if(compareVersion(m_sVersion,"1.1")>=0)
		return CXhChar24(m_data+5);
	return CXhChar24("");
}
bool CBoltSeries::GetBoltSizeAt(int i,CBoltSizeSection& bolt)
{
	CBuffer buf(BufferPtr(),BufferLength());
	bolt.InitBuffer();
	BYTE d_count;
	WORD size_count;
	buf.ReadByte(&d_count);
	buf.Offset(2);	//偏移1B垫圈数量，1B螺帽数量
	buf.ReadWord(&size_count);
	if(compareVersion(m_sVersion,"1.1")>=0)
	{	//增加通厚上限2B，下限2B
		buf.Offset(23);	//偏移23B螺栓库名称
		buf.Offset(i*36);
		bolt.InitBuffer(0,buf.GetCursorBuffer(),36,m_sVersion);
		buf.SeekPosition(5+23+size_count*36+d_count*6+4*i);
	}
	else
	{
		buf.Offset(i*32);
		bolt.InitBuffer(0,buf.GetCursorBuffer(),32,m_sVersion);
		buf.SeekPosition(5+size_count*32+d_count*6+4*i);
	}
	//附加螺栓柱实体数据内存
	DWORD offset,len;
	buf.ReadDword(&offset);
	buf.SeekPosition(AddrGtoL(offset));
	buf.ReadDword(&len);
	bolt.solid.bolt.AttachBuffer(buf.GetCursorBuffer(),len);
	//附加螺帽实体数据内存
	if(compareVersion(m_sVersion,"1.1")>=0)
		buf.SeekPosition(5+23+size_count*36);
	else
		buf.SeekPosition(5+size_count*32);
	for(BYTE di=0;di<d_count;di++)
	{
		WORD rd;
		buf.ReadWord(&rd);
		if(rd==bolt.GetDiameter())
		{
			buf.ReadDword(&offset);	//螺栓帽实体数据存储地址
			buf.SeekPosition(AddrGtoL(offset));
			buf.ReadDword(&len);
			bolt.solid.nut.AttachBuffer(buf.GetCursorBuffer(),len);
			break;
		}
		else
			buf.Offset(4);
	}
	return true;
}
bool CBoltSeries::GetBoltSizeAt(int i,BOLTINFO& bolt)
{
	CBuffer buf(BufferPtr(),BufferLength());
	BYTE d_count;
	WORD size_count;
	buf.ReadByte(&d_count);
	buf.Offset(2);	//偏移1B垫圈数量，1B螺帽数量
	buf.ReadWord(&size_count);
	if(compareVersion(m_sVersion,"1.1")>=0)
	{	//增加通厚上限2B，下限2B
		buf.Offset(23);	//偏移23B螺栓库名称
		buf.Offset(i*36);
	}
	else//比V1.1少:通厚上限2B及通厚下限2B
		buf.Offset(i*32);
	buf.ReadWord(&bolt.d);			//螺栓直径
	buf.ReadWord(&bolt.L);			//有效长(mm)
	buf.ReadWord(&bolt.Lnt);		//无扣长
	if(compareVersion(m_sVersion,"1.1")>=0)
	{	//增加通厚上限2B，下限2B
		buf.ReadWord(&bolt.up_lim);		//通厚上限
		buf.ReadWord(&bolt.down_lim);	//通厚下限
	}
	else
		bolt.up_lim=bolt.down_lim=0;
	buf.ReadDouble(&bolt.weight);	//kg
	buf.Read(bolt.spec,18);	//固定长度的螺栓规格描述字符（含末尾0截止字符）
	//附加螺栓柱实体数据内存
	DWORD offset,len;
	if(compareVersion(m_sVersion,"1.1")>=0)
		buf.SeekPosition(5+23+size_count*36+d_count*6+4*i);
	else
		buf.SeekPosition(5+size_count*32+d_count*6+4*i);
	buf.ReadDword(&offset);
	buf.SeekPosition(AddrGtoL(offset));
	buf.ReadDword(&len);
	bolt.solidOfBolt.AttachBuffer(buf.GetCursorBuffer(),len);
	//附加螺帽实体数据内存
	if(compareVersion(m_sVersion,"1.1")>=0)
		buf.SeekPosition(5+23+size_count*36);
	else
		buf.SeekPosition(5+size_count*32);
	for(BYTE di=0;di<d_count;di++)
	{
		WORD rd;
		buf.ReadWord(&rd);
		if(rd==bolt.d)
		{
			buf.ReadDword(&offset);	//螺栓帽实体数据存储地址
			buf.SeekPosition(AddrGtoL(offset));
			buf.ReadDword(&len);
			bolt.solidOfCap.AttachBuffer(buf.GetCursorBuffer(),len);
			break;
		}
		else
			buf.Offset(4);
	}
	return true;
}
#ifndef TIDCORE_EXPORTS
bool CBoltSeries::GetBoltCapSolidBody(int indexId, CSolidBody& body)
{
	CBuffer buf(BufferPtr(),BufferLength());
	if(compareVersion(m_sVersion,"1.1")>=0)
		buf.SeekPosition(5+23+BoltSizeCount()*36+indexId*6-4);
	else
		buf.SeekPosition(5+BoltSizeCount()*32+indexId*6-4);
	DWORD offset,len;
	buf.Offset(2);	//螺栓直径
	buf.ReadDword(&offset);
	buf.SeekPosition(AddrGtoL(offset));
	buf.ReadDword(&len);
	body.ReadFrom(&buf,len);
	return true;
}
bool CBoltSeries::GetBoltSolidBody(int indexId, CSolidBody& body)
{
	CBuffer buf(BufferPtr(),BufferLength());
	if(compareVersion(m_sVersion,"1.1")>=0)
		buf.SeekPosition(5+23+BoltSizeCount()*36+BoltDCount()*6+indexId*4-4);
	else
		buf.SeekPosition(5+BoltSizeCount()*32+BoltDCount()*6+indexId*4-4);
	DWORD offset,len;
	buf.ReadDword(&offset);
	buf.SeekPosition(AddrGtoL(offset));
	buf.ReadDword(&len);
	body.ReadFrom(&buf,len);
	return true;
}
#endif
CBoltSizeSection::CBoltSizeSection(DWORD zeroAddrOffset/*=0*/,char* buf/*=NULL*/,DWORD size/*=0*/,const char* version/*=NULL*/)
{
	InitBuffer(zeroAddrOffset,buf,size,version);
}
void CBoltSizeSection::InitBuffer(DWORD zeroAddrOffset/*=0*/,char* buf/*=NULL*/,DWORD size/*=0*/,const char* version/*=NULL*/)
{
	m_dwZeroAddrOffset=zeroAddrOffset;
	m_data=buf;
	m_dwBufSize=size;
	m_sVersion.Copy(version);
}
short CBoltSizeSection::GetDiameter()
{	//螺栓直径
	CBuffer buf(BufferPtr(),BufferLength());
	WORD wDiameter;
	buf.ReadWord(&wDiameter);
	return wDiameter;
}
short CBoltSizeSection::GetLenValid()
{	//螺栓有效长
	CBuffer buf(BufferPtr(),BufferLength());
	buf.SeekPosition(2);
	short lenValid;
	buf.ReadWord(&lenValid);
	return lenValid;
}
short CBoltSizeSection::GetLenNoneThread()
{	//螺栓无扣长
	CBuffer buf(BufferPtr(),BufferLength());
	buf.SeekPosition(4);
	short lenNoneThread;
	buf.ReadWord(&lenNoneThread);
	return lenNoneThread;
}
short CBoltSizeSection::GetMaxL0Limit()	//螺栓通厚上限
{
	if(compareVersion(m_sVersion,"1.1")<0)
		return 0;
	CBuffer buf(BufferPtr(),BufferLength());
	buf.SeekPosition(6);
	short wMaxL0Limit;
	buf.ReadWord(&wMaxL0Limit);
	return wMaxL0Limit;
}
short CBoltSizeSection::GetMinL0Limit()	//螺栓通厚下限
{
	if(compareVersion(m_sVersion,"1.1")<0)
		return 0;
	CBuffer buf(BufferPtr(),BufferLength());
	buf.SeekPosition(8);
	short wMinL0Limit;
	buf.ReadWord(&wMinL0Limit);
	return wMinL0Limit;
}
double CBoltSizeSection::GetTheoryWeight()
{	//理论重量(kg)
	CBuffer buf(BufferPtr(),BufferLength());
	if(compareVersion(m_sVersion,"1.1")>=0)
		buf.SeekPosition(10);
	else
		buf.SeekPosition(6);
	double theoryWeight;
	buf.ReadDouble(&theoryWeight);
	return theoryWeight;
}
short CBoltSizeSection::GetSpec(char* spec)	//螺栓规格描述字符（含末尾0截止字符）
{
	CBuffer buf(BufferPtr(),BufferLength());
	if(compareVersion(m_sVersion,"1.1")>=0)
		buf.SeekPosition(18);
	else
		buf.SeekPosition(14);
	char sizestr[18]={0};
	buf.Read(sizestr,18);
	strcpy(spec,sizestr);
	return strlen(sizestr);
}
CBoltSection::CBoltSection(DWORD zeroAddrOffset/*=0*/,char* buf/*=NULL*/,DWORD size/*=0*/,const char* version/*=NULL*/)
{
	m_dwZeroAddrOffset=zeroAddrOffset;
	m_data=buf;
	m_dwBufSize=size;
	m_sVersion.Copy(version);
	if(m_data!=NULL&&compareVersion(version,"1.4")>=0)
	{	//读取地脚螺栓信
		WORD wBoltSeriesCount=BoltSeriesCount();
		CBuffer sectbuf(BufferPtr(),BufferLength());
		sectbuf.SeekPosition(1+wBoltSeriesCount*4);
		DWORD dwAnchorAddr=0,dwSolidSize=0;
		sectbuf.ReadDword(&dwAnchorAddr);
		sectbuf.SeekPosition(this->AddrGtoL(dwAnchorAddr));
		sectbuf.ReadWord(&anchorInfo.d);
		sectbuf.ReadWord(&anchorInfo.wiLe);	//地脚螺栓出露长(mm)
		sectbuf.ReadWord(&anchorInfo.wiLa);	//地脚螺栓无扣长(mm) <底座板厚度
		sectbuf.ReadWord(&anchorInfo.wiWidth);	//地脚螺栓垫板宽度与厚度
		sectbuf.ReadWord(&anchorInfo.wiThick);
		sectbuf.ReadWord(&anchorInfo.wiHoleD);	//地脚螺栓板孔径
		sectbuf.ReadWord(&anchorInfo.wiBasePlateHoleD);	//地脚螺栓连接底座板建议孔径值
		sectbuf.Read(anchorInfo.szSizeSymbol,64);		//６４个字节：固定长度的地脚螺栓规格标记字符串（含末尾0截止字符）。
		//读取地脚螺栓实体信息
		dwSolidSize=sectbuf.ReadDword();
		xAnchorSolid.nut.AttachBuffer(sectbuf.GetCursorBuffer(),dwSolidSize);
		sectbuf.Offset(dwSolidSize);
		dwSolidSize=sectbuf.ReadDword();
		xAnchorSolid.bolt.AttachBuffer(sectbuf.GetCursorBuffer(),dwSolidSize);
	}
}
CBoltSeries CBoltSection::GetBoltSeriesAt(int i)
{
	CBuffer buf(BufferPtr(),BufferLength());
	buf.SeekPosition(1+i*4);
	DWORD offset,len;
	buf.ReadDword(&offset);
	buf.SeekPosition(AddrGtoL(offset));
	buf.ReadDword(&len);
	return CBoltSeries(ZeroAddrOffset()+buf.GetCursorPosition(),buf.GetCursorBuffer(),len,m_sVersion);
}
#ifndef TIDCORE_EXPORTS
void CBoltSection::InitLsFamily(int i,CLsFamily* pFamily)
{
	CBoltSeries boltSeries=GetBoltSeriesAt(i);
	//之前的垫圈数量已转换为是否含防松垫圈属性问题  wjh-2018.4.21
	pFamily->m_bAntiLoose=boltSeries.BoltWasherCount()>0;
	pFamily->m_bDualCap=boltSeries.BoltCapCount()>=2;
	int nRec=boltSeries.BoltSizeCount();
	StrCopy(pFamily->name,boltSeries.GetSeriesName(),17);
	pFamily->CreateFamily(nRec);
	char sGrade[50]="";
	for(int j=0;j<pFamily->GetCount();j++)
	{
		LS_XING_HAO* pLsXingHao=pFamily->RecordAt(j);
		BOLTINFO boltInfo;
		boltSeries.GetBoltSizeAt(j,boltInfo);
		pLsXingHao->d=boltInfo.d;
		pLsXingHao->valid_len=boltInfo.L;
		pLsXingHao->no_thread_len=boltInfo.Lnt;
		pLsXingHao->up_lim=boltInfo.up_lim;
		pLsXingHao->down_lim=boltInfo.down_lim;
		strcpy(pLsXingHao->guige,boltInfo.spec);
		sscanf(pLsXingHao->guige,"%[^M]",sGrade);
		pLsXingHao->grade=atof(sGrade);
		pLsXingHao->weight=boltInfo.weight;
	}
}
#endif
//材质库
char CMaterialLibrarySection::BriefSymbolAt(int i)
{
	char* cursor=BufferPtr()+i*8;
	return *cursor;
}
int  CMaterialLibrarySection::NameCodeAt(int i,char* nameCode)	//返回字符串长度,允许输入NULL
{
	char* cursor=BufferPtr()+i*8+1;
	char name[8]={0};
	memcpy(name,cursor,7);
	if(nameCode)
		strcpy(nameCode,name);
	return strlen(name);
}
CMaterialLibrarySection CPartSection::GetMatLibrarySection()
{
	CBuffer buf(BufferPtr(),BufferLength());
	BYTE mat_n;
	buf.ReadByte(&mat_n);
	DWORD part_n;
	buf.ReadDword(&part_n);
	CMaterialLibrarySection material(mat_n,buf.GetCursorBuffer(),mat_n*8);
	material.SetBelongTidBuffer(BelongTidBuffer());
	return material;
}

UINT CPartSection::GetPartCount()
{
	UINT part_n;
	CBuffer buf(BufferPtr(),BufferLength());
	buf.SeekPosition(1);
	buf.ReadInteger(&part_n);
	return part_n;
}
//构件
PART_INFO CPartSection::GetPartInfoByIndexId(int indexId)
{
	BYTE mat_n;
	DWORD part_n;
	CBuffer buf(BufferPtr(),BufferLength());
	buf.ReadByte(&mat_n);
	buf.ReadDword(&part_n);
	buf.Offset(mat_n*8);
	if(compareVersion(m_sVersion,"1.1")>=0)
		buf.Offset((indexId-1)*33);
	else
		buf.Offset((indexId-1)*32);
	PART_INFO part;
	buf.ReadWord(&part.wModelSpace);
	buf.ReadByte(&part.cPartType);
	buf.ReadWord(&part.wStateFlag);
	buf.ReadByte(&part.cMaterial);
	buf.ReadDword(&part.dwSeg);
	buf.ReadFloat(&part.fWidth);
	buf.ReadFloat(&part.fThick);
	buf.ReadFloat(&part.fHeight);
	buf.ReadWord(&part.wLength);
	buf.ReadFloat(&part.fWeight);
	if(compareVersion(m_sVersion,"1.1")>=0)
		buf.ReadByte(&part.cFuncType);
	else
		part.cFuncType=0;
	buf.ReadDword(&part.addrAttachInfo);
	//读取附加信息
	buf.SeekPosition(AddrGtoL(part.addrAttachInfo));
	DWORD size;
	buf.ReadDword(&size);
	part.solid.AttachBuffer(buf.GetCursorBuffer(),size);
	buf.Offset(size);
	buf.ReadString(part.sPartNo,50);
	buf.ReadString(part.spec,50);
	buf.ReadString(part.sNotes,50);
	buf.ReadDword(&part.dwProcessBuffSize);	//构件生产工艺信息存储长度
	part.processBuffBytes=buf.GetCursorBuffer();
	buf.Offset(size);		//不需要输出工艺信息，所以略掉此部分信息
	return part;
}
void CPartSection::GetPartBufferAt(int indexId,CBuffer& partBuffer)
{
	partBuffer.ClearContents();
	CBuffer sectionBuf(BufferPtr(),BufferLength());
	BYTE mat_n;
	sectionBuf.ReadByte(&mat_n);
	DWORD iValue,iOffset,size;
	sectionBuf.ReadDword(&iValue);
	sectionBuf.Offset(mat_n*8);
	if(compareVersion(m_sVersion,"1.1")>=0)
	{
		sectionBuf.Offset((indexId-1)*33);	//跳到第indexId处的信息处
		partBuffer.Write(sectionBuf,29);	//读取构件基本信息
	}
	else
	{
		sectionBuf.Offset((indexId-1)*32);	//跳到第indexId处的信息处
		partBuffer.Write(sectionBuf,28);	//读取构件基本信息
		partBuffer.WriteByte(0);	//补齐构件功能类型字节	wjh-2016.1.18
	}
	sectionBuf.ReadDword(&iOffset);
	sectionBuf.SeekPosition(AddrGtoL(iOffset));
	sectionBuf.ReadDword(&size);
	partBuffer.WriteDword(size);		//读取构件实体信息内存大小
	partBuffer.Write(sectionBuf,size);	//读取构件实体信息
	CXhChar50 sText;
	sectionBuf.ReadString(sText,50);
	partBuffer.WriteString(sText);		//读取构件编号
	sectionBuf.ReadString(sText,50);
	partBuffer.WriteString(sText);		//读取构件规格
	sectionBuf.ReadString(sText,50);
	partBuffer.WriteString(sText);		//读取备注
	sectionBuf.ReadDword(&size);		
	partBuffer.WriteDword(size);		//读取构件工艺信息内存大小
	partBuffer.Write(sectionBuf,size);	//读取构件工艺信息
}
DWORD CAssembleSection::BlockAssemblyCount()
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD block_count;
	buf.ReadDword(&block_count);
	return block_count;
}
WORD CAssembleSection::GetBlockIndexId(int indexId)	//获取指定部件装配对象的关联部件索引标识序号
{
	CBuffer buf(BufferPtr(),BufferLength());
	buf.Offset(4+(indexId-1)*128);
	WORD wBlockIndexId=0;
	buf.ReadWord(&wBlockIndexId);
	return wBlockIndexId;
}
BLOCK_ASSEMBLY CAssembleSection::GetAssemblyByIndexId(int indexId)
{
	CBuffer buf(BufferPtr(),BufferLength());
	buf.Offset(4+(indexId-1)*128);
	BLOCK_ASSEMBLY assembly;
	buf.ReadWord(&assembly.wIndexId);
	buf.ReadPoint(assembly.acs.origin);
	buf.ReadPoint(assembly.acs.axis_x);
	buf.ReadPoint(assembly.acs.axis_y);
	buf.ReadPoint(assembly.acs.axis_z);
	buf.ReadDword(&assembly.dwSeg);
	buf.ReadByte(&assembly.cLegQuad);
	buf.Read(&assembly.cfgword,24);
	buf.ReadByte(&assembly.reserved);
	return assembly;
}
CNodeSection CAssembleSection::NodeSection()
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD block_assembly_n,size;
	buf.ReadDword(&block_assembly_n);
	buf.Offset(block_assembly_n*128);
	if(compareVersion(this->m_sVersion,"1.4")>=0)
		buf.ReadDword(&size);	//节点装配区
	else
		size=0;
	return CNodeSection(m_dwZeroAddrOffset+buf.GetCursorPosition(),buf.GetCursorBuffer(),size,m_sVersion);
}
CBoltAssemblySection CAssembleSection::BoltSection()
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD block_assembly_n,size;
	buf.ReadDword(&block_assembly_n);
	buf.Offset(block_assembly_n*128);
	if(compareVersion(this->m_sVersion,"1.4")>=0)
	{	//跳过节点装配区
		buf.ReadDword(&size);
		buf.Offset(size);
	}
	buf.ReadDword(&size);
	return CBoltAssemblySection(m_dwZeroAddrOffset+buf.GetCursorPosition(),buf.GetCursorBuffer(),size,m_sVersion);
}
DWORD CBoltAssemblySection::AssemblyCount(bool bTowerSpace/*=true*/)
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD assemble_n;
	if(bTowerSpace)
		buf.Offset(4);
	buf.ReadDword(&assemble_n);
	return assemble_n;
}
BOLT_ASSEMBLY CBoltAssemblySection::GetAssemblyByIndexId(int indexId,bool bTowerSpace/*=true*/)
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD block_bolt_n,tower_bolt_n;
	buf.ReadDword(&block_bolt_n);
	buf.ReadDword(&tower_bolt_n);

	BOLT_ASSEMBLY assembly;
	if(bTowerSpace)
	{
		buf.Offset(block_bolt_n*64);//跳过部件模型空间中螺栓装配信息区域
		buf.Offset((indexId-1)*92);	//跳到当前索引标识指向的记录
		assembly.bInBlockSpace=false;
		assembly.wBlockIndexId=0;
	}
	else
	{
		buf.Offset((indexId-1)*64);
		assembly.bInBlockSpace=true;
	}
	//共用属性
	buf.ReadByte(&assembly.cSeriesId);
	buf.ReadWord(&assembly.wIndexId);
	buf.ReadFloat(&assembly.grade);
	//共用属性
	buf.ReadPoint(assembly.origin);
	buf.ReadPoint(assembly.work_norm);
	buf.ReadWord(&assembly.wL0);
	buf.ReadByte(&assembly.cPropFlag);
	buf.ReadByte(&assembly.cDianQuanN);
	buf.ReadByte(&assembly.cDianQuanThick);
	buf.ReadWord(&assembly.wDianQuanOffset);
	if(!bTowerSpace)	//部件空间中螺栓装配对象对应的部件索引标识
		buf.ReadWord(&assembly.wBlockIndexId);
	else //if(bTowerSpace)
	{					//杆塔模型空间中螺栓装配对象对应的部件索引标识
		buf.ReadByte(&assembly.cStatFlag);
		buf.ReadDword(&assembly.dwSeg);
		buf.ReadByte(&assembly.cLegQuad);
		buf.Read(&assembly.cfgword,24);
		if(assembly.cStatFlag&0x80)	//螺栓同时归属多个段号，段号统计范围以字符串形式存储在后续４个字节所指地址中
		{
			buf.SeekPosition(AddrGtoL(assembly.dwSeg));
			buf.ReadString(assembly.statSegStr,50);
		}
	}
	return assembly;
}
void CBoltAssemblySection::GetBoltInfoBufferAt(int indexId,CBuffer& boltBuffer,bool bTowerSpace/*=true*/)
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD block_bolt_n,tower_bolt_n;
	buf.ReadDword(&block_bolt_n);
	buf.ReadDword(&tower_bolt_n);
	if(bTowerSpace)
	{
		buf.Offset(block_bolt_n*64);//跳过部件模型空间中螺栓装配信息区域
		buf.Offset((indexId-1)*92);	//跳到当前索引标识指向的记录
	}
	else
		buf.Offset((indexId-1)*64);
	boltBuffer.ClearContents();
	boltBuffer.Write(buf,7);
}
void CBoltAssemblySection::GetBoltAssemblyBufferAt(int indexId,CBuffer& assemblyBuf,bool bTowerSpace/*=true*/)
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD block_bolt_n,tower_bolt_n;
	buf.ReadDword(&block_bolt_n);
	buf.ReadDword(&tower_bolt_n);
	if(bTowerSpace)
	{
		buf.Offset(block_bolt_n*64);//跳过部件模型空间中螺栓装配信息区域
		buf.Offset((indexId-1)*92);	//跳到当前索引标识指向的记录
	}
	else
		buf.Offset((indexId-1)*64);
	buf.Offset(7);
	assemblyBuf.ClearContents();
	assemblyBuf.Write(buf,55);
	WORD wBlockIndexId=0;
	if(bTowerSpace)
	{					
		assemblyBuf.WriteWord(wBlockIndexId);
		BYTE cFlag=0,cQuad=0;
		DWORD dwSeg=0;
		CFGWORD cfgword;
		CXhChar50 statSetStr;
		// 最高位为０表示螺栓仅归属后续４个字节所代表段号；
		// 最高位为１表示螺栓同时归属多个段号，段号统计范围以字符串形式存储在后续４个字节所指地址中；
		// 次高位为１表示是否为接腿上螺栓；
		// 其余低６位表示归属接腿的象限号，如为０表示归属杆塔本体。
		buf.ReadByte(&cFlag);
		buf.ReadDword(&dwSeg);
		buf.ReadByte(&cQuad);
		buf.Read(&cfgword,24);
		if(cFlag&0x80)		//螺栓同时归属多个段号
		{	//
			buf.SeekPosition(AddrGtoL(dwSeg));
			buf.ReadString(statSetStr,50);
#ifndef TIDCORE_EXPORTS
			ATOM_LIST<SEGI> segNoList;
			if(GetSortedSegNoList(statSetStr,segNoList)>0)
				dwSeg=segNoList.GetFirst()->iSeg;
#else
			CXhSimpleList<SEGI> listSegI;
			if(GetSortedSegNoList(statSetStr,listSegI)>0)
				dwSeg=listSegI.EnumObjectFirst()->iSeg;
#endif
			else 
				dwSeg=0;
		}
		assemblyBuf.WriteDword(dwSeg);
		assemblyBuf.WriteString(statSetStr);
		assemblyBuf.WriteByte(cQuad);	//腿部构件从属象限
		assemblyBuf.Write(&cfgword,24);	//配材号
	}
	else
	{
		buf.ReadWord(&wBlockIndexId);
		assemblyBuf.WriteWord(wBlockIndexId);
	}
}
void CNodeSection::GetNodeByIndexId(int indexId,bool bTowerSpace,NODE_ASSEMBLY &node)
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD block_node_n,tower_node_n;
	buf.ReadDword(&block_node_n);
	buf.ReadDword(&tower_node_n);

	UINT uiBlkRecLength=33,uiTowerRecLength=56;
	if(bTowerSpace)
	{
		node.bInBlockSpace=false;
		node.wBlockIndexId=0;
		buf.Offset(block_node_n*uiBlkRecLength);	//跳过部件模型空间中构件装配信息区域
		buf.Offset((indexId-1)*uiTowerRecLength);	//跳到当前索引标识指向的记录
		buf.ReadInteger(&node.uiPointI);
		buf.ReadByte(&node.cLegQuad);
		buf.Read(node.szLayerSymbol,3);
		buf.Read(&node.cfgword,24);
	}
	else
	{
		node.bInBlockSpace=true;
		node.cLegQuad=0;
		buf.Offset((indexId-1)*uiBlkRecLength);	//跳到当前索引标识指向的记录
		buf.ReadInteger(&node.uiPointI);
		buf.Read(node.szLayerSymbol,3);
	}
	buf.ReadPoint(node.xPosition);
}
DWORD CNodeSection::NodeCount(bool bTowerSpace/*=true*/)
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD assemble_n;
	if(bTowerSpace)
		buf.Offset(4);
	buf.ReadDword(&assemble_n);
	return assemble_n;
}
PART_ASSEMBLY::PART_ASSEMBLY()
{
	bInBlockSpace=false;
	wBlockIndexId=0;
	cLegQuad=0;
	szLayerSymbol[0]=szLayerSymbol[1]=szLayerSymbol[2]=szLayerSymbol[3]=0;
	uiStartPointI=uiEndPointI=xGroupAngle.uidGroupAngle=0;
	dwIndexId=dwAddrBriefLine=0;
	bIsRod=false;
	cStartCoordType=cEndCoordType=0;
}
PART_ASSEMBLY::PART_ASSEMBLY(PART_ASSEMBLY &srcPart)
{
	CloneFrom(srcPart);
}
PART_ASSEMBLY& PART_ASSEMBLY::operator=(PART_ASSEMBLY &srcPart)
{
	CloneFrom(srcPart);
	return *this;
}
void PART_ASSEMBLY::CloneFrom(PART_ASSEMBLY &srcPart)
{
	bInBlockSpace=srcPart.bInBlockSpace;	//是否属于装配部件空间［共享属性］
	wBlockIndexId=srcPart.wBlockIndexId;//归属部件的标识索引号［部件装配构件属性］
	cLegQuad=srcPart.cLegQuad;		//归属接腿的象限号，如为０表示属塔身本件［杆塔装配构件属性］
	cfgword=srcPart.cfgword;	//杆塔模型空间中用于区分归属呼高的配材号［杆塔装配构件属性］
	dwIndexId=srcPart.dwIndexId;	//在构件信息分区中的标识索引号［共享属性］
	uiStartPointI=srcPart.uiStartPointI;
	uiEndPointI=srcPart.uiEndPointI;
	memcpy(szLayerSymbol,srcPart.szLayerSymbol,4);
	xGroupAngle=srcPart.xGroupAngle;
	dwAddrBriefLine=srcPart.dwAddrBriefLine;
	bIsRod=srcPart.bIsRod;
	cStartCoordType=srcPart.cStartCoordType;
	cEndCoordType=srcPart.cEndCoordType;
	startPt=srcPart.startPt;
	endPt=srcPart.endPt;
	acs=srcPart.acs;			//在归属模型空间内的装配坐标系定义［共享属性］
	hashBoltIndexByHoleId.Empty();
	for(DWORD *pBoltIndexId=srcPart.hashBoltIndexByHoleId.GetFirst();pBoltIndexId;pBoltIndexId=srcPart.hashBoltIndexByHoleId.GetNext())
		hashBoltIndexByHoleId.SetValue(srcPart.hashBoltIndexByHoleId.GetCursorKey(),*pBoltIndexId);
}
CPartAssemblySection CAssembleSection::PartSection()
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD block_assembly_n,size;
	buf.ReadDword(&block_assembly_n);
	buf.Offset(block_assembly_n*128);
	if(compareVersion(this->m_sVersion,"1.4")>=0)
	{	//跳过节点装配区
		buf.ReadDword(&size);
		buf.Offset(size);
	}
	buf.ReadDword(&size);
	buf.Offset(size);	//跳过螺栓装配区
	buf.ReadDword(&size);
	CPartAssemblySection section(m_dwZeroAddrOffset+buf.GetCursorPosition(),buf.GetCursorBuffer(),size,m_sVersion);
	section.SetBelongTidBuffer(BelongTidBuffer());
	return section;
}
DWORD CPartAssemblySection::AssemblyCount(bool bTowerSpace/*=true*/)
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD assemble_n;
	if(bTowerSpace)
		buf.Offset(4);
	buf.ReadDword(&assemble_n);
	return assemble_n;
}
void CPartAssemblySection::GetAssemblyByIndexId(int indexId,bool bTowerSpace,PART_ASSEMBLY &assembly)
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD block_part_n,tower_part_n,rod_n;
	buf.ReadDword(&block_part_n);
	buf.ReadDword(&tower_part_n);
	if(compareVersion(m_pTidBuff->Version(),"1.1")>=0)
		buf.ReadDword(&rod_n);	//杆塔杆件简化线数量

	bool bAfterVer14=compareVersion(m_pTidBuff->Version(),"1.4")>=0;
	UINT uiBlkRecLength=!bAfterVer14?110:117;
	UINT uiRecLength=!bAfterVer14?133:140;
	if(bTowerSpace)
	{
		assembly.bInBlockSpace=false;
		assembly.wBlockIndexId=0;
		if(compareVersion(m_pTidBuff->Version(),"1.1")>=0)
		{	//增加简化线存储位置4B及孔位影射表存储位置4B wjh-2016.1.18
			buf.Offset(block_part_n*uiBlkRecLength);	//跳过部件模型空间中构件装配信息区域
			buf.Offset((indexId-1)*uiRecLength);	//跳到当前索引标识指向的记录
			buf.ReadDword(&assembly.dwIndexId);
			if(bAfterVer14)
			{
				buf.Read(assembly.szLayerSymbol,3);
				buf.ReadInteger(&assembly.xGroupAngle.uidGroupAngle);
			}
			buf.ReadDword(&assembly.dwAddrBriefLine);
		}
		else
		{
			buf.Offset(block_part_n*102);	//跳过部件模型空间中构件装配信息区域
			buf.Offset((indexId-1)*125);	//跳到当前索引标识指向的记录
			buf.ReadDword(&assembly.dwIndexId);
		}
		buf.ReadByte(&assembly.cLegQuad);
		buf.Read(&assembly.cfgword,24);
	}
	else
	{
		assembly.bInBlockSpace=true;
		assembly.cLegQuad=0;
		if(compareVersion(m_pTidBuff->Version(),"1.1")>=0)
		{
			buf.Offset((indexId-1)*uiBlkRecLength);	//跳到当前索引标识指向的记录
			buf.ReadDword(&assembly.dwIndexId);
			if(bAfterVer14)
			{
				buf.Read(assembly.szLayerSymbol,3);
				buf.ReadInteger(&assembly.xGroupAngle.uidGroupAngle);
			}
			buf.ReadDword(&assembly.dwAddrBriefLine);
		}
		else
		{
			buf.Offset((indexId-1)*106);	//跳到当前索引标识指向的记录
			buf.ReadDword(&assembly.dwIndexId);
		}
		buf.ReadWord(&assembly.wBlockIndexId);
	}
	buf.ReadPoint(assembly.acs.origin);
	buf.ReadPoint(assembly.acs.axis_x);
	buf.ReadPoint(assembly.acs.axis_y);
	buf.ReadPoint(assembly.acs.axis_z);
	//读取装配记录螺栓孔信息地址
	if(compareVersion(m_pTidBuff->Version(),"1.1")>=0)
	{
		DWORD addrHoleInfo=0;
		buf.ReadDword(&addrHoleInfo);
		if(addrHoleInfo>0)
		{	//读取附加信息
			buf.SeekPosition(AddrGtoL(addrHoleInfo));
			DWORD size;
			buf.ReadDword(&size);
			assembly.hashBoltIndexByHoleId.Empty();
			for(DWORD i=0;i<size;i++)
			{
				DWORD dwBoltIndex=0,dwHoleId=0;
				buf.ReadDword(&dwHoleId);
				buf.ReadDword(&dwBoltIndex);
				assembly.hashBoltIndexByHoleId.SetValue(dwHoleId,dwBoltIndex);
			}
		}
		//读取杆件简化线数据
		if(assembly.dwAddrBriefLine>0)
		{
			buf.SeekPosition(AddrGtoL(assembly.dwAddrBriefLine));
			buf.ReadPoint(assembly.startPt);
			buf.ReadPoint(assembly.endPt);
			if(bAfterVer14)//compareVersion(m_pTidBuff->Version(),"1.4")>=0)
			{
				buf.ReadInteger(&assembly.uiStartPointI);
				buf.ReadInteger(&assembly.uiEndPointI);
			}
			if(compareVersion(m_pTidBuff->Version(),"1.2")>=0)
			{	//读取杆件始终端节点类型，以便后期模型中可以自动提取模型尺寸标注 wht 16-12-01
				buf.ReadByte(&assembly.cStartCoordType);
				buf.ReadByte(&assembly.cEndCoordType);
			}
			assembly.bIsRod=true;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
// 
CPartOrgProcessInfoSection CAttachDataSection::PartOrgProcessInfoSection()
{
	CBuffer buf(BufferPtr(),BufferLength());
	buf.SeekPosition(0);
	DWORD pos=buf.ReadDword();
	DWORD sect_len=0;
	if(pos>0)
	{
		buf.SeekPosition(AddrGtoL(pos));
		sect_len=buf.ReadDword();
	}
	return CPartOrgProcessInfoSection(m_dwZeroAddrOffset+buf.GetCursorPosition(),buf.GetCursorBuffer(),sect_len);
}
//////////////////////////////////////////////////////////////////////////
//CPartOrgProcessInfoSection
BOOL CPartOrgProcessInfoSection::GetOrgProcessInfoBufAt(int indexId,CBuffer& orgProcessInfoBuf)
{
	if(BufferLength()<=0)	//没有附加信息
		return FALSE;
	CBuffer buf(BufferPtr(),BufferLength());
	buf.Offset((indexId-1)*4);
	DWORD offset,len;
	buf.ReadDword(&offset);
	if(offset==0)
		return FALSE;
	buf.SeekPosition(AddrGtoL(offset));
	buf.ReadDword(&len);
	if(len>0)
		orgProcessInfoBuf.Write(buf.GetCursorBuffer(),len);
	else
		return FALSE;
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//CIIDBuffer
//////////////////////////////////////////////////////////////////////////////////////////////
CTIDBuffer::CTIDBuffer(void)
{
	m_dwZeroAddrOffset=0;
}

CTIDBuffer::~CTIDBuffer(void)
{
}
CModuleSection CTIDBuffer::ModuleSection()
{
	DWORD pos,sect_len;
	SeekPosition(AddrLtoG(0));
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	CModuleSection section(GetCursorPosition(),GetCursorBuffer(),sect_len,m_sVersion);
	section.SetBelongTidBuffer(this);
	return section;
}
CSubLegFoundationSection CTIDBuffer::SubLegFoundationSection()
{
	DWORD pos,sect_len;
	BYTE cnHeightCount=ModuleSection().GetModuleCount();
	SeekPosition(AddrLtoG(4));
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	CSubLegFoundationSection section(cnHeightCount,GetCursorPosition(),GetCursorBuffer(),sect_len,m_sVersion);
	section.SetBelongTidBuffer(this);
	return section;
}
CWireNodeSection CTIDBuffer::WireNodeSection()
{
	DWORD pos,sect_len;
	SeekPosition(AddrLtoG(8));
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	CWireNodeSection section(GetCursorPosition(),GetCursorBuffer(),sect_len,m_sVersion);
	section.SetBelongTidBuffer(this);
	return section;
}
CProjectInfoSection CTIDBuffer::ProjectInfoSection()
{
	DWORD pos,sect_len;
	SeekPosition(AddrLtoG(12));
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	CProjectInfoSection section(GetCursorPosition(),GetCursorBuffer(),sect_len,m_sVersion);
	section.SetBelongTidBuffer(this);
	return section;
}
CBlockSection CTIDBuffer::BlockSection()
{
	DWORD pos,sect_len;
	if(compareVersion(m_sVersion,"1.4")>=0)
		SeekPosition(AddrLtoG(4+12));
	else
		SeekPosition(AddrLtoG(4));
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	CBlockSection section(GetCursorPosition(),GetCursorBuffer(),sect_len,m_sVersion);
	section.SetBelongTidBuffer(this);
	return section;
}
CBoltSection CTIDBuffer::BoltSection()
{
	DWORD pos,sect_len;
	if(compareVersion(m_sVersion,"1.4")>=0)
		SeekPosition(AddrLtoG(8+12));
	else
		SeekPosition(AddrLtoG(8));
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	CBoltSection section(GetCursorPosition(),GetCursorBuffer(),sect_len,m_sVersion);
	section.SetBelongTidBuffer(this);
	return section;
}
CPartSection CTIDBuffer::PartSection()
{
	DWORD pos,sect_len;
	if(compareVersion(m_sVersion,"1.4")>=0)
		SeekPosition(AddrLtoG(12+12));
	else
		SeekPosition(AddrLtoG(12));
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	CPartSection section(GetCursorPosition(),GetCursorBuffer(),sect_len,m_sVersion);
	section.SetBelongTidBuffer(this);
	return section;
}
CAssembleSection CTIDBuffer::AssembleSection()
{
	DWORD pos,sect_len;
	if(compareVersion(m_sVersion,"1.4")>=0)
		SeekPosition(AddrLtoG(16+12));
	else
		SeekPosition(AddrLtoG(16));
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	CAssembleSection section(GetCursorPosition(),GetCursorBuffer(),sect_len,m_sVersion);
	section.SetBelongTidBuffer(this);
	return section;
}

CAttachDataSection CTIDBuffer::AttachDataSection()
{
	DWORD pos,sect_len;
	SeekPosition(AddrLtoG(16));	//附加数据分区在最后一个分区之后，此处根据最后一个分区进行跳转
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	SeekPosition(pos+sect_len+4);	//+4为sect_len所在的4个字节
	DWORD attach_sect_len=GetLength()-GetCursorPosition();
	CAttachDataSection section(GetCursorPosition(),GetCursorBuffer(),attach_sect_len,m_sVersion);
	section.SetBelongTidBuffer(this);
	return section;
}
bool CTIDBuffer::InitBuffer(const char* srcBuf, DWORD buf_size)
{
	ClearBuffer();
	Write(srcBuf,buf_size);
	SeekToBegin();
	CXhChar50 sDocType;
	ReadString(sDocType,50);
	if(strstr(sDocType,"Xerofox tower solid data media file")==NULL)
	{
		//throw "非预期的文件格式，文件打开失败!";
		logerr.Log("非预期的文件格式，文件打开失败!\n");
		return false;
	}
	char* key=strtok(sDocType,"-");
	char* ver=strtok(NULL,"-");
	double version=1000;
	if(ver!=NULL)
		version=atof(ver);
	if(version>1.4)
	{
		logerr.Log("此文件版本号太高,不能直接读取此文件\n");
		return false;
	}
	m_sVersion.Copy(ver);
	ReadString(m_sTowerType,50);
	ReadPoint(mcs.axis_x);
	ReadPoint(mcs.axis_y);
	ReadPoint(mcs.axis_z);
	SetZeroAddrOffset(GetCursorPosition());
	return true;
}
