#include "StdAfx.h"
#include ".\solidtowermodel.h"

static DWORD CFG_NO[32]={ 0X00000001,0X00000002,0X00000004,0X00000008,0X00000010,0X00000020,0X00000040,0X00000080,
						  0X00000100,0X00000200,0X00000400,0X00000800,0X00001000,0X00002000,0X00004000,0X00008000,
						  0X00010000,0X00020000,0X00040000,0X00080000,0X00100000,0X00200000,0X00400000,0X00800000,
						  0X01000000,0X02000000,0X04000000,0X08000000,0X10000000,0X20000000,0X40000000,0X80000000};

CFGWORD GetCfgWord(long iNo)	//从1开始计数
{
	CFGWORD cfgword(iNo);
	return cfgword;
}
CFGWORD CFGWORD::SetWordByNo(int iNo)
{
	iNo--;
	Clear();
	if(iNo>=0&&iNo<192)
	{
		int iWord=iNo/32;
		int iBit=iNo%32;
		word[iWord]=CFG_NO[iBit];
	}
	return *this;
}
BOOL CFGWORD::And(CFGWORD wcfg) const
{
	return (word[0]&wcfg.word[0])||(word[1]&wcfg.word[1])||(word[2]&wcfg.word[2])||(word[3]&wcfg.word[3])||(word[4]&wcfg.word[4])||(word[4]&wcfg.word[5]);
}
BOOL CFGWORD::IsEqual(CFGWORD cfgword)
{
	return word[0]==cfgword.word[0]&&word[1]==cfgword.word[1] && 
		word[2]==cfgword.word[2]&&word[3]==cfgword.word[3]&&word[4]==cfgword.word[4]&&word[5]==cfgword.word[5];
}
BOOL CFGWORD::IsHasNo(int iNo)
{
	return And(GetCfgWord(iNo));
}
///////////////////////////////////////////////////////////////////////////////////////////////
//将字符串型的版本号转换为一个长整数，如"1.01.3.21"-->1 01 03 21
long FromStringVersion(const char* version)
{
	char local_version[20]="";
	strncpy(local_version,version,20);
	char *key=strtok(local_version," .,-");
	UINT version_item[4]={0};
	int n,i=0;
	while(key!=NULL&&i<4)
	{
		n=(int)strlen(key);
		if(n==1)
			version_item[i]=key[0]-'0';
		else if(n>1)
			version_item[i]=(key[0]-'0')*10+key[1]-'0';
		key=strtok(NULL,".,-");
		i++;
	}
	long nVersion=version_item[0]*1000000+version_item[1]*10000+version_item[2]*100+version_item[3];
	//long nVersion=version_item[0]*lic.ConstE6()+version_item[1]*lic.ConstE4()+version_item[2]*lic.ConstE2()+version_item[3];
	return nVersion;
}
//大于零表示版本号1高,等于零表示同版本，否则表示版本号1低
int CompareVersion(const char* version1,const char* version2)
{
	if(version1==NULL)
		return 1;
	else if(version2==NULL)
		return -1;
	long nVersion1=FromStringVersion(version1);
	long nVersion2=FromStringVersion(version2);
	if(nVersion1>nVersion2)
		return 1;
	else if(nVersion1<nVersion2)
		return -1;
	else
		return 0;
}
CSolidPartModel::CSolidPartModel()
{
	cLegQuad=0;
}
CSolidPartModel::~CSolidPartModel()
{
	;
}
///////////////////////////////////////////////////////////////////////////////////////////////
TOWER_MODULE CModuleSection::GetModuleAt(int i)
{
	TOWER_MODULE module;
	BYTE* pBodyNo=(BYTE*)BufferPtr()+1+21*i;
	module.m_cBodyNo=*pBodyNo;
	int record_n=25;
	if(CompareVersion(m_sVersion,"1.1")>=0)
	{
		BYTE* pcbLegInfo=pBodyNo+1;
		module.m_cbLegInfo=*pcbLegInfo;
		memcpy(&module.m_dwLegCfgWord,pBodyNo+2,24);
		record_n=26;	//V1.1增加了减腿信息１Ｂ
	}
	else
		memcpy(&module.m_dwLegCfgWord,pBodyNo+1,24);
	for(BYTE j=1;j<=192;j++)
	{
		if(module.m_dwLegCfgWord.IsHasNo(j))
		{
			module.m_arrActiveQuadLegNo[0]=module.m_arrActiveQuadLegNo[1]=
			module.m_arrActiveQuadLegNo[2]=module.m_arrActiveQuadLegNo[3]=j;
			break;
		}
	}
	//读取呼高分组的名称
	char* pNameSection=BufferPtr()+1+record_n*GetModuleCount();
	CBuffer pool(pNameSection,GetModuleCount()*100);	//因不知该数据区具体长度，只能给定一个肯定比较大的缓存空间
	for(int j=0;j<i;j++)
	{
		UINT uLen=pool.ReadStringLength();
		pool.Offset(uLen);
	}
	pool.ReadString(module.name,51);
	return module;
}
CBlockSection::CBlockSection(DWORD zeroAddrOffset,char* buf/*=NULL*/,DWORD size/*=0*/){
	m_dwZeroAddrOffset=zeroAddrOffset;
	m_data=buf;
	m_dwBufSize=size;
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
BOLT_INFO CBoltSeries::GetBoltSizeAt(int i)
{
	CBuffer buf(BufferPtr(),BufferLength());
	BYTE d_count;
	WORD size_count;
	buf.ReadByte(&d_count);
	buf.Offset(2);	//偏移1B垫圈数量，1B螺帽数量
	buf.ReadWord(&size_count);
	if(CompareVersion(m_sVersion,"1.1")>=0)
	{	//增加通厚上限2B，下限2B
		buf.Offset(23);	//偏移23B螺栓库名称
		buf.Offset(i*36);
	}
	else
		buf.Offset(i*32);
	BOLT_INFO bolt;
	buf.ReadWord(&bolt.d);			//螺栓直径
	buf.ReadWord(&bolt.L);			//有效长(mm)
	buf.ReadWord(&bolt.Lnt);		//无扣长
	buf.ReadDouble(&bolt.weight);	//kg
	buf.Read(bolt.spec,18);	//固定长度的螺栓规格描述字符（含末尾0截止字符）
	//附加螺栓柱实体数据内存
	DWORD offset,len;
	if(CompareVersion(m_sVersion,"1.1")>=0)
		buf.SeekPosition(5+23+size_count*36+d_count*6+4*i);
	else
		buf.SeekPosition(5+size_count*32+d_count*6+4*i);
	buf.ReadDword(&offset);
	buf.SeekPosition(AddrGtoL(offset));
	buf.ReadDword(&len);
	bolt.solidOfBolt.AttachBuffer(buf.GetCursorBuffer(),len);
	//附加螺帽实体数据内存
	if(CompareVersion(m_sVersion,"1.1")>=0)
		buf.SeekPosition(5+23+size_count*32);
	else
		buf.SeekPosition(5+size_count*32);
	BYTE di;
	for(di=0;di<d_count;di++)
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
	if(di==d_count)
		int b=3;//TODO: 报错未处理
	return bolt;
}
bool CBoltSeries::GetBoltCapSolidBody(int indexId, CSolidBody& body)
{
	CBuffer buf(BufferPtr(),BufferLength());
	if(CompareVersion(m_sVersion,"1.1")>=0)
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
	if(CompareVersion(m_sVersion,"1.1")>=0)
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

CBoltSeries CBoltSection::GetBoltSeriesAt(int i)
{
	CBuffer buf(BufferPtr(),BufferLength());
	buf.SeekPosition(1+i*4);
	DWORD offset,len;
	buf.ReadDword(&offset);
	buf.SeekPosition(AddrGtoL(offset));
	buf.ReadDword(&len);
	return CBoltSeries(ZeroAddrOffset()+buf.GetCursorPosition(),buf.GetCursorBuffer(),len);
}

CMaterialLibrary CPartSection::GetMatLibrary()
{
	CBuffer buf(BufferPtr(),BufferLength());
	BYTE mat_n;
	buf.ReadByte(&mat_n);
	DWORD part_n;
	buf.ReadDword(&part_n);
	return CMaterialLibrary(mat_n,buf.GetCursorBuffer(),mat_n*8);
}
PART_INFO CPartSection::GetPartInfoAt(int indexId)
{
	BYTE mat_n;
	DWORD part_n;
	CBuffer buf(BufferPtr(),BufferLength());
	buf.ReadByte(&mat_n);
	buf.ReadDword(&part_n);
	buf.Offset(mat_n*8);
	if(CompareVersion(m_sVersion,"1.1")>=0)
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
	if(CompareVersion(m_sVersion,"1.1")>=0)
		buf.ReadByte(&part.cFuncType);
	buf.ReadDword(&part.addrAttachInfo);
	//读取附加信息
	buf.SeekPosition(AddrGtoL(part.addrAttachInfo));
	DWORD size;
	buf.ReadDword(&size);
	part.solid.AttachBuffer(buf.GetCursorBuffer(),size);
	buf.Offset(size);
	buf.ReadString(part.sPartNo,16);
	buf.ReadString(part.spec,16);
	buf.ReadString(part.sNotes,50);
	buf.ReadDword(&size);	//构件生产工艺信息存储长度
	buf.Offset(size);		//不需要输出工艺信息，所以略掉此部分信息
	return part;
}
DWORD CAssembleSection::BlockAssemblyCount()
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD block_count;
	buf.ReadDword(&block_count);
	return block_count;
}
BLOCK_ASSEMBLY CAssembleSection::GetAssemblyAt(int indexId)
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
CBoltAssemblySection CAssembleSection::BoltSection()
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD block_assembly_n,size;
	buf.ReadDword(&block_assembly_n);
	buf.Offset(block_assembly_n*128);
	buf.ReadDword(&size);
	return CBoltAssemblySection(buf.GetCursorBuffer(),size);
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
BOLT_ASSEMBLY CBoltAssemblySection::GetAssemblyAt(int indexId,bool bTowerSpace/*=true*/)
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
CPartAssemblySection CAssembleSection::PartSection()
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD block_assembly_n,size;
	buf.ReadDword(&block_assembly_n);
	buf.Offset(block_assembly_n*128);
	buf.ReadDword(&size);
	buf.Offset(size);	//跳过螺栓装配区
	buf.ReadDword(&size);
	return CPartAssemblySection(buf.GetCursorBuffer(),size);
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
PART_ASSEMBLY CPartAssemblySection::GetAssemblyAt(int indexId,bool bTowerSpace/*=true*/)
{
	CBuffer buf(BufferPtr(),BufferLength());
	DWORD block_part_n,tower_part_n,rod_n;
	buf.ReadDword(&block_part_n);
	buf.ReadDword(&tower_part_n);
	if(CompareVersion(m_sVersion,"1.1")>=0)
		buf.ReadDword(&rod_n);	//杆塔杆件简化线数量

	PART_ASSEMBLY assembly;
	if(bTowerSpace)
	{
		assembly.bInBlockSpace=false;
		assembly.wBlockIndexId=0;
		if(CompareVersion(m_sVersion,"1.1")>=0)
		{	//增加简化线存储位置4B及孔位影射表存储位置4B wjh-2016.1.18
			buf.Offset(block_part_n*110);	//跳过部件模型空间中构件装配信息区域
			buf.Offset((indexId-1)*133);	//跳到当前索引标识指向的记录
		}
		else
		{
			buf.Offset(block_part_n*102);	//跳过部件模型空间中构件装配信息区域
			buf.Offset((indexId-1)*125);	//跳到当前索引标识指向的记录
		}
		buf.ReadDword(&assembly.dwIndexId);
		buf.ReadByte(&assembly.cLegQuad);
		buf.Read(&assembly.cfgword,24);
	}
	else
	{
		if(CompareVersion(m_sVersion,"1.1")>=0)
			buf.Offset((indexId-1)*110);	//跳到当前索引标识指向的记录
		else
			buf.Offset((indexId-1)*102);	//跳到当前索引标识指向的记录
		assembly.bInBlockSpace=true;
		assembly.cLegQuad=0;
		buf.ReadDword(&assembly.dwIndexId);
		buf.ReadWord(&assembly.wBlockIndexId);
	}
	buf.ReadPoint(assembly.acs.origin);
	buf.ReadPoint(assembly.acs.axis_x);
	buf.ReadPoint(assembly.acs.axis_y);
	buf.ReadPoint(assembly.acs.axis_z);
	//读取装配记录螺栓孔信息地址
	if(CompareVersion(m_sVersion,"1.1")>=0)
	{
		DWORD addrHoleInfo=0;
		buf.ReadDword(&addrHoleInfo);
		/*if(addrHoleInfo>0)
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
		}*/
		//读取杆件简化线数据
		if(assembly.dwAddrBriefLine>0)
		{
			buf.SeekPosition(AddrGtoL(assembly.dwAddrBriefLine));
			buf.ReadPoint(assembly.startPt);
			buf.ReadPoint(assembly.endPt);
			assembly.bIsRod=true;
		}
	}
	return assembly;
}

///////////////////////////////////////////////////////////////////////////////////////////////
CSolidTowerModel model;
CSolidTowerModel::CSolidTowerModel(void)
{
	m_iActiveModule=0;
	version=0;
	m_dwZeroAddrOffset=0;
	mcs.axis_x.Set(1,0,0);
	mcs.axis_y.Set(0,0,1);
	mcs.axis_z.Set(0,-1,0);
}

CSolidTowerModel::~CSolidTowerModel(void)
{
}
void CSolidTowerModel::InitBuffer(char* srcBuf, DWORD buf_size)
{
	ClearBuffer();
	Write(srcBuf,buf_size);
	CXhChar50 sDocType;
	SeekToBegin();
	ReadString(sDocType,50);
	if(strstr(sDocType,"Xerofox tower solid data media file")==NULL)
		throw "非预期的文件格式，文件打开失败!";
	char* key=strtok(sDocType,"-");
	char* ver=strtok(NULL,"-");
	if(ver!=NULL)
		version=atof(ver);
	if(version>1.0)
		throw "此文件版本号太高或太低,不能直接读取此文件";
	ReadString(m_sTowerType,50);
	ReadPoint(mcs.axis_x);
	ReadPoint(mcs.axis_y);
	ReadPoint(mcs.axis_z);
	m_dwZeroAddrOffset=GetCursorPosition();
}
CModuleSection CSolidTowerModel::ModuleSection()
{
	DWORD pos,sect_len;
	SeekPosition(AddrLtoG(0));
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	return CModuleSection(GetCursorBuffer(),sect_len);
}
CBlockSection CSolidTowerModel::BlockSection()
{
	DWORD pos,sect_len;
	SeekPosition(AddrLtoG(4));
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	return CBlockSection(GetCursorPosition(),GetCursorBuffer(),sect_len);
}
CBoltSection CSolidTowerModel::BoltSection()
{
	DWORD pos,sect_len;
	SeekPosition(AddrLtoG(8));
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	return CBoltSection(GetCursorPosition(),GetCursorBuffer(),sect_len);
}
CPartSection CSolidTowerModel::PartSection()
{
	DWORD pos,sect_len;
	SeekPosition(AddrLtoG(12));
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	return CPartSection(GetCursorPosition(),GetCursorBuffer(),sect_len);
}
CAssembleSection CSolidTowerModel::AssembleSection()
{
	DWORD pos,sect_len;
	SeekPosition(AddrLtoG(16));
	ReadDword(&pos);
	SeekPosition(pos);
	ReadDword(&sect_len);
	return CAssembleSection(GetCursorBuffer(),sect_len);
}
