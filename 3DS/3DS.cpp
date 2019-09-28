#include "StdAfx.h"
#include "3DS.h"
#include "glDrawTool.h"

//////////////////////////////////////////////////////////////////////////
//局部功能函数
//矢量方向的坐标系转换(与坐标原点无关)
//TRUE：UCS-->WCS; FALSE：WCS-->UCS
static bool Standize(double* vector3d)
{
	double mod_len=vector3d[0]*vector3d[0]+vector3d[1]*vector3d[1]+vector3d[2]*vector3d[2];
	mod_len=sqrt(max(0,mod_len));
	if(mod_len<EPS)
		return false;
	vector3d[0]/=mod_len;
	vector3d[1]/=mod_len;
	vector3d[2]/=mod_len;
	return true;
}
int FloatToInt(double f)
{
	int fi=(int)f;
	return f-fi>0.5?fi+1:fi;
}
int CalArcResolution(double radius,double sector_angle)
{
	double user2scr_scale=1.0;
	int sampling=5;		//圆弧绘制时的抽样长度(象素数)
	UINT uMinSlices=36;	//整圆弧显示的最低平滑度片数
	double length=sector_angle*radius;
	int n = FloatToInt((length/user2scr_scale)/sampling);
	int min_n=FloatToInt(uMinSlices*sector_angle/Pi)/2;
	int max_n=FloatToInt(72*sector_angle/Pi);
	if(max_n==0)
		max_n=1;
	if(min_n==0)
		min_n=1;
	n=max(n,min_n);
	n=min(n,max_n);
	return n;
}
void GetArcSimuPolyVertex(f3dArcLine* pArcLine, GEPOINT vertex_arr[], int slices)
{
	int i,sign=1;
	if(pArcLine->ID&0x80000000)	//负边
		sign=-1;
	double slice_angle = pArcLine->SectorAngle()/slices;
	if(pArcLine->WorkNorm()==pArcLine->ColumnNorm())	//圆弧
	{
		if(sign<0)
			slice_angle*=-1;
		double sina = sin(slice_angle);	//扇片角正弦
		double cosa = cos(slice_angle);	//扇片角余弦
		vertex_arr[0].Set(pArcLine->Radius());
		for(i=1;i<=slices;i++)
		{
			vertex_arr[i].x=vertex_arr[i-1].x*cosa-vertex_arr[i-1].y*sina;
			vertex_arr[i].y=vertex_arr[i-1].y*cosa+vertex_arr[i-1].x*sina;
		}
		GEPOINT origin=pArcLine->Center();
		GEPOINT axis_x=pArcLine->Start()-origin;
		Standize(axis_x);
		GEPOINT axis_y=pArcLine->WorkNorm()^axis_x;
		for(i=0;i<=slices;i++)
		{
			vertex_arr[i].Set(	origin.x+vertex_arr[i].x*axis_x.x+vertex_arr[i].y*axis_y.x,
				origin.y+vertex_arr[i].x*axis_x.y+vertex_arr[i].y*axis_y.y,
				origin.z+vertex_arr[i].x*axis_x.z+vertex_arr[i].y*axis_y.z);
		}
	}
	else	//椭圆弧
	{
		if(sign<0)
		{
			for(i=slices;i>=0;i--)
				vertex_arr[i] = pArcLine->PositionInAngle(i*slice_angle);
		}
		else
		{
			for(i=0;i<=slices;i++)
				vertex_arr[i] = pArcLine->PositionInAngle(i*slice_angle);
		}
	}
}
static f3dPoint GetPolyFaceWorkNorm(CSolidBody* pBody,CRawSolidFace& rawface)
{
	CFaceLoop loop=rawface.GetOutterLoop();
	f3dPoint vec1,vec2,norm;//临时作为中间三维矢量
	long n = loop.LoopEdgeLineNum();
	if(n<3)
		return norm;
	//--------------VVVVVVVVV---2.建立多边形面用户坐标系ucs---------------------
	if(rawface.WorkNorm.IsZero())
	{	//非指定法线情况
		f3dArcLine line1,line2;
		for(long start=0;start<n;start++)
		{
			loop.GetLoopEdgeLineAt(pBody,start,line1);
			if(line1.ID&0x80000000)
				Sub_Pnt( vec1, line1.Start(), line1.End());
			else
				Sub_Pnt( vec1, line1.End(), line1.Start());
			Standize(vec1);
			long i,j;
			for(i=start+1;i<n;i++)
			{
				loop.GetLoopEdgeLineAt(pBody,i,line2);
				if(line2.ID&0x80000000)	//负边
					Sub_Pnt( vec2, line2.Start(), line2.End());
				else
					Sub_Pnt( vec2, line2.End(), line2.Start());
				Standize(vec2);
				norm = vec1^vec2;
				double dd=norm.mod();
				if(dd>EPS2)
				{
					norm /= dd;	//单位化
					break;
				}
			}
			if( i==n)
				return norm;	//多边形面定义有误(所有边界线重合)返回零向量
			//通过建立临时坐标系，判断法线正负方向
			GECS cs;
			cs.origin=line1.Start();
			cs.axis_x=vec1;
			cs.axis_z=norm;
			cs.axis_y=cs.axis_z^cs.axis_x;
			for(j=0;j<n;j++)
			{
				if(j==0||j==i)
					continue;
				loop.GetLoopEdgeLineAt(pBody,start+j,line2);
				f3dPoint vertice=line2.End();
				vertice=cs.TransPToCS(vertice);
				if(vertice.y<-EPS)	//遍历点在直线上或附近时，应容许计算误差
					break;
			}
			if(j==n)	//全部顶点都在当前边的左侧，即表示计算的法线为正法线，否则应继续寻找
				break;
		}
	}
	else
		norm=rawface.WorkNorm;
	return norm;
}
void WriteToSolidBuffer(CSolidBodyBuffer& solidbuf,int indexBasicFace,CXhSimpleList<CGLFace>& listFacets,COLORREF color,const double* poly_norm)
{
	//写入一组基本面片数据（对应一个多边形面拆分成的若干基本面片簇
	solidbuf.SeekPosition(solidbuf.BasicFaceIndexStartAddr+indexBasicFace*4);
	DWORD basicface_lenpos=solidbuf.GetLength();
	solidbuf.WriteDword(basicface_lenpos);	//将数据区位置写入索引区
	solidbuf.SeekToEnd();
	solidbuf.WriteWord((WORD)0);	//CTidBasicFace的数据记录长度
	solidbuf.Write(&color,4);
	WORD facets_n=0;
	LIST_NODE<CGLFace>* pTailNode=listFacets.EnumTail();
	if(pTailNode!=NULL)
		facets_n=(WORD)listFacets.EnumTail()->serial;
	BUFFERPOP stack(&solidbuf,facets_n);
	solidbuf.WriteWord(facets_n);
	for(CGLFace* pGLFace=listFacets.EnumObjectFirst();pGLFace;pGLFace=listFacets.EnumObjectNext())
	{
		WORD cluster_buf_n=1+24+2+pGLFace->header.uVertexNum*24;
		solidbuf.WriteWord(cluster_buf_n);
		solidbuf.WriteByte(pGLFace->header.mode);
		if(pGLFace->header.clr_norm&0x02)
		{	//指定面片簇法线值
			solidbuf.WriteDouble(pGLFace->nx);
			solidbuf.WriteDouble(pGLFace->ny);
			solidbuf.WriteDouble(pGLFace->nz);
		}
		else	//面片簇法线即为原始定义面的法线
			solidbuf.WritePoint(GEPOINT(poly_norm));
		//写入连续三角面片数量
		if(pGLFace->header.mode==GL_TRIANGLES)
			solidbuf.WriteWord((WORD)(pGLFace->header.uVertexNum/3));
		else
			solidbuf.WriteWord((WORD)(pGLFace->header.uVertexNum-2));
		solidbuf.Write(pGLFace->m_pVertexCoordArr,pGLFace->header.uVertexNum*24);
		stack.Increment();
	}
	stack.VerifyAndRestore(true,2);
	WORD wBasicfaceBufLen=(WORD)(solidbuf.GetCursorPosition()-basicface_lenpos-2);
	solidbuf.SeekPosition(basicface_lenpos);
	solidbuf.WriteWord(wBasicfaceBufLen);
	solidbuf.SeekToEnd();
}
bool SplitToBasicFacets(CSolidBody* pSrcBody,CSolidBody* pDstBody)
{
	if(pSrcBody==NULL)
		return false;
	//迁移原实体定义内存同时腾出基于三角面片的基本实体显示面数据区空间
	int face_n=pSrcBody->PolyFaceNum();
	int edge_n=pSrcBody->KeyEdgeLineNum();
	if(face_n==0||edge_n==0)
		return false;
	static char _facets_buff_pool[0x100000];
	CSolidBodyBuffer solidbuf(_facets_buff_pool, 0x100000);
	solidbuf.Write(pSrcBody->BufferPtr(),33);
	solidbuf.BasicFaceNumber=face_n;	//写入基本面片数=原始多边形面数
	DWORD dwIndexBufSize=(edge_n+face_n)*4;
	solidbuf.WriteAt(45,pSrcBody->BufferPtr()+45,dwIndexBufSize);
	solidbuf.BasicFaceIndexStartAddr=45+dwIndexBufSize;
	solidbuf.SeekToEnd();	//BasicFaceIndexStartAddr＝赋值会变当前存储指针位置
	solidbuf.Write(NULL,face_n*4);	//@45+dwIndexBufSize	实体基本面片索引区暂写入相应的空字节占位
	DWORD dwDataBufSize=pSrcBody->BufferLength()-solidbuf.VertexDataStartAddr;
	if(pSrcBody->BasicGLFaceNum()>0)	//只复制实体原始定义数据区域，忽略原有的基本面片数据区
		dwDataBufSize=pSrcBody->BasicFaceDataStartAddr()-solidbuf.VertexDataStartAddr;
	long iNewVertexDataStartAddr=solidbuf.GetCursorPosition();
	long iOldVertexDataStartAddr=solidbuf.VertexDataStartAddr;
	solidbuf.VertexDataStartAddr=iNewVertexDataStartAddr;
	solidbuf.EdgeDataStartAddr=solidbuf.EdgeDataStartAddr+4*face_n;
	solidbuf.RawFaceDataStartAddr=solidbuf.RawFaceDataStartAddr+4*face_n;
	solidbuf.SeekToEnd();
	solidbuf.Write(pSrcBody->BufferPtr()+iOldVertexDataStartAddr,dwDataBufSize);	//写入原实体定义的数据区内存
	//计算因增加基本面片索引记录数带来的后续地址位移值
	int addr_offset=(face_n-pSrcBody->BasicGLFaceNum())*4;
	if(addr_offset!=0)
	{	//重新移位各项基本图元索引所指向的内存地址偏移
		DWORD* RawFaceAddr=(DWORD*)(solidbuf.GetBufferPtr()+solidbuf.RawFaceIndexStartAddr);
		for(int i=0;i<face_n;i++)
			*(RawFaceAddr+i)+=addr_offset;
		DWORD* RawEdgeAddr=(DWORD*)(solidbuf.GetBufferPtr()+solidbuf.EdgeIndexStartAddr);
		for(int i=0;i<face_n;i++)
			*(RawEdgeAddr+i)+=addr_offset;
	}
	if(solidbuf.BasicFaceDataStartAddr==0)	//以前基本面片数为空
		solidbuf.BasicFaceDataStartAddr=solidbuf.GetCursorPosition();
	else	//重写原基本面片数据区
		solidbuf.BasicFaceDataStartAddr=solidbuf.BasicFaceDataStartAddr+4*face_n;
	//写入三角面数据
	double alpha=0.6;	//考虑到显示效果的经验系数
	int i=0,j=0,n=0;
	CRawSolidFace rawface;
	CXhSimpleList<CGLFace> listFacets;
	CXhSimpleList<GEPOINT> listVertices;
	for(int indexFace=0;indexFace<face_n;indexFace++)
	{
		listFacets.DeleteList();
		listVertices.DeleteList();
		if(!pSrcBody->GetPolyFaceAt(indexFace,rawface))
		{
			WriteToSolidBuffer(solidbuf,indexFace,listFacets,0,GEPOINT(0,0,0));
			continue;
		}
		/*对于一个复杂面，肯定有一个外环，此外还可能有无数个内环，它们的法线方向是相同的
		  相对法线来说，外环应该逆时针方向，而内环则应该是顺时针方向，
		*/
		f3dArcLine edgeLine[4];
		GEPOINT poly_norm=rawface.WorkNorm;
		CFaceLoop outerloop=rawface.GetOutterLoop();
		if(outerloop.LoopEdgeLineNum()==3)
		{
			outerloop.GetLoopEdgeLineAt(pSrcBody,0,edgeLine[0]);
			outerloop.GetLoopEdgeLineAt(pSrcBody,1,edgeLine[1]);
			outerloop.GetLoopEdgeLineAt(pSrcBody,2,edgeLine[2]);
			f3dPoint pt_arr[3];
			if(edgeLine[0].ID&0x80000000)	//负边
				pt_arr[0] = edgeLine[0].End();
			else
				pt_arr[0] = edgeLine[0].Start();
			if(edgeLine[1].ID&0x80000000)	//负边
				pt_arr[1] = edgeLine[1].End();
			else
				pt_arr[1] = edgeLine[1].Start();
			if(edgeLine[2].ID&0x80000000)	//负边
				pt_arr[2] = edgeLine[2].End();
			else
				pt_arr[2] = edgeLine[2].Start();
			poly_norm=rawface.WorkNorm;
			if(poly_norm.IsZero())
			{
				f3dPoint vec1=pt_arr[1]-pt_arr[0];
				f3dPoint vec2=pt_arr[2]-pt_arr[1];
				poly_norm=vec1^vec2;
			}
			if(!Standize(poly_norm))
			{
				WriteToSolidBuffer(solidbuf,indexFace,listFacets,rawface.MatColor(),poly_norm);
				continue;//return false;
			}
			//设置三角面法线、光照颜色及顶点坐标等信息
			CGLFace *pGLFace=listFacets.AttachObject();
			pGLFace->nx=poly_norm.x;
			pGLFace->ny=poly_norm.y;
			pGLFace->nz=poly_norm.z;
			pGLFace->red   = GetRValue(rawface.MatColor())/255.0f;
			pGLFace->green = GetGValue(rawface.MatColor())/255.0f;
			pGLFace->blue  = GetBValue(rawface.MatColor())/255.0f;
			pGLFace->alpha = (GLfloat)alpha;
			pGLFace->header.uVertexNum=3;
			pGLFace->header.clr_norm=0x03;	//默认变换颜色及法线
			pGLFace->m_pVertexCoordArr=new GLdouble[9];
			for(j=0;j<3;j++)
			{
				pGLFace->m_pVertexCoordArr[3*j]=pt_arr[j].x;
				pGLFace->m_pVertexCoordArr[3*j+1]=pt_arr[j].y;
				pGLFace->m_pVertexCoordArr[3*j+2]=pt_arr[j].z;
			}
			WriteToSolidBuffer(solidbuf,indexFace,listFacets,rawface.MatColor(),poly_norm);
			continue;
		}
		if(outerloop.LoopEdgeLineNum()==4)
		{
			outerloop.GetLoopEdgeLineAt(pSrcBody,0,edgeLine[0]);
			outerloop.GetLoopEdgeLineAt(pSrcBody,1,edgeLine[1]);
			outerloop.GetLoopEdgeLineAt(pSrcBody,2,edgeLine[2]);
			outerloop.GetLoopEdgeLineAt(pSrcBody,3,edgeLine[3]);
			if(rawface.WorkNorm.IsZero())
			{
				f3dPoint vec1=edgeLine[0].End()-edgeLine[0].Start();
				f3dPoint vec2=edgeLine[1].End()-edgeLine[1].Start();
				poly_norm=vec1^vec2;
				int sign1=1,sign2=1;
				if(edgeLine[0].ID&0x80000000)
					sign1=-1;
				if(edgeLine[1].ID&0x80000000)
					sign2=-1;
				if(sign1+sign2==0)	//异号边线
					poly_norm*=-1;
			}
			else
				poly_norm=rawface.WorkNorm;
			if(!Standize(poly_norm))
			{
				if(edgeLine[0].SectorAngle()>0)
				{
					poly_norm=edgeLine[0].WorkNorm();
					if(edgeLine[0].ID&0x80000000)
						poly_norm*=-1;
				}
				else if(edgeLine[1].SectorAngle()>0)
				{
					poly_norm=edgeLine[1].WorkNorm();
					if(edgeLine[1].ID&0x80000000)
						poly_norm*=-1;
				}
				//TODO: 未理解原意，可能是担心共线边出现
				//edgeLine[0]=NULL;
			}
			if(edgeLine[0].SectorAngle()>0&&edgeLine[1].SectorAngle()==0&&edgeLine[2].SectorAngle()>0&&edgeLine[3].SectorAngle()==0
				&&fabs(edgeLine[0].WorkNorm()*poly_norm)<EPS_COS)
			{
				n=max(edgeLine[0].m_uDisplaySlices,edgeLine[2].m_uDisplaySlices);
				if(n==0)
				{
					int n1=CalArcResolution(edgeLine[0].Radius(),edgeLine[0].SectorAngle());
					int n2=CalArcResolution(edgeLine[2].Radius(),edgeLine[2].SectorAngle());
					n=max(n1,n2);
				}
				n=min(n,200);
				GEPOINT vertex_arr1[200],vertex_arr2[200];
				GetArcSimuPolyVertex(&edgeLine[0],vertex_arr1,n);
				GetArcSimuPolyVertex(&edgeLine[2],vertex_arr2,n);
				// 			double part_angle1=edgeLine[0]->SectorAngle()/n;
				// 			double part_angle2=edgeLine[2]->SectorAngle()/n;
				// 			double posAngle;

				for(i=0;i<n;i++)
				{
					f3dPoint pt_arr[3];
					//１号圆弧中间点
					//posAngle=edgeLine[0]->SectorAngle()-i*part_angle1;
					pt_arr[0] = vertex_arr1[n-i];//edgeLine[0]->PositionInAngle(posAngle);
					//２号圆弧中间点
					//posAngle=i*part_angle2;
					pt_arr[1] = vertex_arr2[i];//edgeLine[2]->PositionInAngle(posAngle);
					//２号圆弧中间点
					//posAngle=(i+1)*part_angle2;
					pt_arr[2] = vertex_arr2[i+1];//edgeLine[2]->PositionInAngle(posAngle);
					f3dPoint axis_x=pt_arr[1]-pt_arr[0];
					f3dPoint axis_y=pt_arr[2]-pt_arr[0];
					poly_norm=axis_x^axis_y;
					Standize(poly_norm);
					//设置三角面法线、光照颜色及顶点坐标等信息
					CGLFace *pGLFace=listFacets.AttachObject();
					pGLFace->nx=poly_norm.x;
					pGLFace->ny=poly_norm.y;
					pGLFace->nz=poly_norm.z;
					pGLFace->red   = GetRValue(rawface.MatColor())/255.0f;
					pGLFace->green = GetGValue(rawface.MatColor())/255.0f;
					pGLFace->blue  = GetBValue(rawface.MatColor())/255.0f;
					pGLFace->alpha = (GLfloat)alpha;
					pGLFace->header.uVertexNum=3;
					pGLFace->m_pVertexCoordArr=new GLdouble[9];
					for(j=0;j<3;j++)
					{
						pGLFace->m_pVertexCoordArr[3*j]=pt_arr[j].x;
						pGLFace->m_pVertexCoordArr[3*j+1]=pt_arr[j].y;
						pGLFace->m_pVertexCoordArr[3*j+2]=pt_arr[j].z;
					}

					//２号圆弧中间点
					//posAngle=(i+1)*part_angle2;
					pt_arr[0] = vertex_arr2[i+1];//edgeLine[2]->PositionInAngle(posAngle);
					//１号圆弧中间点
					//posAngle=edgeLine[0]->SectorAngle()-(i+1)*part_angle1;
					pt_arr[1] = vertex_arr1[n-i-1];//edgeLine[0]->PositionInAngle(posAngle);
					//１号圆弧中间点
					//posAngle=edgeLine[0]->SectorAngle()-i*part_angle1;
					pt_arr[2] = vertex_arr1[n-i];//edgeLine[0]->PositionInAngle(posAngle);
					axis_x = pt_arr[1]-pt_arr[0];
					axis_y = pt_arr[2]-pt_arr[0];
					poly_norm=axis_x^axis_y;
					Standize(poly_norm);
					//设置三角面法线、光照颜色及顶点坐标等信息
					pGLFace=listFacets.AttachObject();
					pGLFace->nx=poly_norm.x;
					pGLFace->ny=poly_norm.y;
					pGLFace->nz=poly_norm.z;
					pGLFace->red   = GetRValue(rawface.MatColor())/255.0f;
					pGLFace->green = GetGValue(rawface.MatColor())/255.0f;
					pGLFace->blue  = GetBValue(rawface.MatColor())/255.0f;
					pGLFace->alpha = (GLfloat)alpha;
					pGLFace->header.uVertexNum=3;
					pGLFace->m_pVertexCoordArr=new GLdouble[9];
					for(j=0;j<3;j++)
					{
						pGLFace->m_pVertexCoordArr[3*j]=pt_arr[j].x;
						pGLFace->m_pVertexCoordArr[3*j+1]=pt_arr[j].y;
						pGLFace->m_pVertexCoordArr[3*j+2]=pt_arr[j].z;
					}
				}
				WriteToSolidBuffer(solidbuf,indexFace,listFacets,rawface.MatColor(),poly_norm);
				continue;
			}
			else if(edgeLine[0].SectorAngle()==0&&edgeLine[1].SectorAngle()>0&&edgeLine[2].SectorAngle()==0&&edgeLine[3].SectorAngle()>0
				&&fabs(edgeLine[1].WorkNorm()*poly_norm)<EPS_COS)
			{
				n=max(edgeLine[1].m_uDisplaySlices,edgeLine[3].m_uDisplaySlices);
				if(n==0)
				{
					int n1=CalArcResolution(edgeLine[1].Radius(),edgeLine[1].SectorAngle());
					int n2=CalArcResolution(edgeLine[3].Radius(),edgeLine[3].SectorAngle());
					n=max(n1,n2);
				}
				n=min(n,200);
				GEPOINT vertex_arr1[200],vertex_arr2[200];
				GetArcSimuPolyVertex(&edgeLine[1],vertex_arr1,n);
				GetArcSimuPolyVertex(&edgeLine[3],vertex_arr2,n);
				// 			double part_angle1=edgeLine[1]->SectorAngle()/n;
				// 			double part_angle2=edgeLine[3]->SectorAngle()/n;
				// 			double posAngle;
				glEnable(GL_NORMALIZE);
				glEnable(GL_AUTO_NORMAL);
				for(i=0;i<n;i++)
				{
					f3dPoint pt_arr[3];
					//１号圆弧中间点
					//posAngle=edgeLine[1]->SectorAngle()-i*part_angle1;
					pt_arr[0] = vertex_arr1[n-i];//edgeLine[1]->PositionInAngle(posAngle);
					//２号圆弧中间点
					//posAngle=i*part_angle2;
					pt_arr[1] = vertex_arr2[i];//edgeLine[3]->PositionInAngle(posAngle);
					//２号圆弧中间点
					//posAngle=(i+1)*part_angle2;
					pt_arr[2] = vertex_arr2[i+1];//edgeLine[3]->PositionInAngle(posAngle);
					f3dPoint axis_x=pt_arr[1]-pt_arr[0];
					f3dPoint axis_y=pt_arr[2]-pt_arr[0];
					poly_norm=axis_x^axis_y;
					Standize(poly_norm);
					//设置三角面法线、光照颜色及顶点坐标等信息
					CGLFace *pGLFace=listFacets.AttachObject();
					pGLFace->nx=poly_norm.x;
					pGLFace->ny=poly_norm.y;
					pGLFace->nz=poly_norm.z;
					pGLFace->red   = GetRValue(rawface.MatColor())/255.0f;
					pGLFace->green = GetGValue(rawface.MatColor())/255.0f;
					pGLFace->blue  = GetBValue(rawface.MatColor())/255.0f;
					pGLFace->alpha = (GLfloat)alpha;
					pGLFace->header.uVertexNum=3;
					pGLFace->m_pVertexCoordArr=new GLdouble[9];
					for(j=0;j<3;j++)
					{
						pGLFace->m_pVertexCoordArr[3*j]=pt_arr[j].x;
						pGLFace->m_pVertexCoordArr[3*j+1]=pt_arr[j].y;
						pGLFace->m_pVertexCoordArr[3*j+2]=pt_arr[j].z;
					}

					//２号圆弧中间点
					//posAngle=(i+1)*part_angle2;
					pt_arr[0] = vertex_arr2[i+1];//edgeLine[3]->PositionInAngle(posAngle);
					//１号圆弧中间点
					//posAngle=edgeLine[1]->SectorAngle()-(i+1)*part_angle1;
					pt_arr[1] = vertex_arr1[n-i-1];//edgeLine[1]->PositionInAngle(posAngle);
					//１号圆弧中间点
					//posAngle=edgeLine[1]->SectorAngle()-i*part_angle1;
					pt_arr[2] = vertex_arr1[n-i];//edgeLine[1]->PositionInAngle(posAngle);
					axis_x=pt_arr[1]-pt_arr[0];
					axis_y=pt_arr[2]-pt_arr[0];
					poly_norm=axis_x^axis_y;
					Standize(poly_norm);
					//设置三角面法线、光照颜色及顶点坐标等信息
					pGLFace=listFacets.AttachObject();
					pGLFace->nx=poly_norm.x;
					pGLFace->ny=poly_norm.y;
					pGLFace->nz=poly_norm.z;
					pGLFace->red   = GetRValue(rawface.MatColor())/255.0f;
					pGLFace->green = GetGValue(rawface.MatColor())/255.0f;
					pGLFace->blue  = GetBValue(rawface.MatColor())/255.0f;
					pGLFace->alpha = (GLfloat)alpha;
					pGLFace->header.uVertexNum=3;
					pGLFace->m_pVertexCoordArr=new GLdouble[9];
					for(j=0;j<3;j++)
					{
						pGLFace->m_pVertexCoordArr[3*j]=pt_arr[j].x;
						pGLFace->m_pVertexCoordArr[3*j+1]=pt_arr[j].y;
						pGLFace->m_pVertexCoordArr[3*j+2]=pt_arr[j].z;
					}
				}
				WriteToSolidBuffer(solidbuf,indexFace,listFacets,rawface.MatColor(),poly_norm);
				continue;
			}
		}
		CGLTesselator t;
		t.SetFilling(TRUE);
		t.SetWindingRule(GLU_TESS_WINDING_ODD);
		if(poly_norm.IsZero())
			poly_norm=GetPolyFaceWorkNorm(pSrcBody,rawface);
		t.StartDef();
		t.TessNormal(poly_norm.x,poly_norm.y,poly_norm.z);
		//第一个为外环（参见B-rep模型）
		int ei=0,edge_n=outerloop.LoopEdgeLineNum();
		//for(pLine=pFace->outer_edge.GetFirst();pLine!=NULL;pLine=pFace->outer_edge.GetNext())
		f3dArcLine line;
		f3dPoint vertice;
		for(ei=0;ei<edge_n;ei++)
		{
			outerloop.GetLoopEdgeLineAt(pSrcBody,ei,line);
			if(line.SectorAngle()==0)
			{
				if(line.Start()==line.End())
					continue;
				if(line.ID&0x80000000)
					vertice = line.End();
				else
					vertice = line.Start();
				listVertices.AttachObject(vertice);
			}
			else
			{
				if(line.m_uDisplaySlices>0)
					n=line.m_uDisplaySlices;
				else
					n=CalArcResolution(line.Radius(),line.SectorAngle());
				double piece_angle=line.SectorAngle()/n;
				for(i=0;i<n;i++)
				{
					if(line.ID&0x80000000)
					{
						if(i==0)
							vertice=line.End();
						else
							vertice = line.PositionInAngle((n-i-1)*piece_angle);
					}
					else
					{
						if(i==0)
							vertice=line.Start();
						else
							vertice = line.PositionInAngle(i*piece_angle);
					}
					listVertices.AttachObject(vertice);
				}
			}
		}
		for(GEPOINT *pp=listVertices.EnumObjectFirst();pp!=NULL;pp=listVertices.EnumObjectNext())
			t.AddVertex(*pp);
		//第二个为内环
		//for(pLoop=pFace->inner_loop.GetFirst();pLoop!=NULL;pLoop=pFace->inner_loop.GetNext())
		for(int loopi=0;loopi<rawface.InnerLoopNum();loopi++)
		{
			CFaceLoop innerloop=rawface.GetInnerLoopAt(loopi);
			t.ContourSeparator();	//环边界区分
			edge_n=innerloop.LoopEdgeLineNum();
			//for(pLine=pLoop->loop->GetFirst();pLine!=NULL;pLine=pLoop->loop->GetNext())
			for(ei=0;ei<edge_n;ei++)
			{
				innerloop.GetLoopEdgeLineAt(pSrcBody,ei,line);
				if(line.SectorAngle()==0)
				{
					vertice = line.Start();
					GEPOINT *pp=listVertices.AttachObject(vertice);
					t.AddVertex(*pp);
				}
				else
				{
					if(line.m_uDisplaySlices>0)
						n=line.m_uDisplaySlices;
					else
						n=CalArcResolution(line.Radius(),line.SectorAngle());
					double piece_angle=line.SectorAngle()/n;
					for(j=0;j<n;j++)
					{
						if(j==0)
							vertice=line.Start();
						else
							vertice = line.PositionInAngle(j*piece_angle);
						GEPOINT *pp=listVertices.AttachObject(vertice);
						t.AddVertex(*pp);
					}
				}
			}
		}
		t.EndDef();
		trianglesBuffer.SeekPosition(0);
		while(trianglesBuffer.GetRemnantSize()>0)
		{
			//设置三角面法线、光照颜色及顶点坐标等信息
			CGLFace *pGLFace=listFacets.AttachObject();
			pGLFace->nx=poly_norm.x;
			pGLFace->ny=poly_norm.y;
			pGLFace->nz=poly_norm.z;
			pGLFace->red   = GetRValue(rawface.MatColor())/255.0f;
			pGLFace->green = GetGValue(rawface.MatColor())/255.0f;
			pGLFace->blue  = GetBValue(rawface.MatColor())/255.0f;
			pGLFace->alpha = (GLfloat)alpha;
			CGLFace *pPrevGLFace=listFacets.EnumObjectTail();
			pGLFace->header.clr_norm=0x03;	//默认变换颜色及法线
			if(pPrevGLFace!=NULL)
			{
				if( pPrevGLFace->red==pGLFace->red&&pPrevGLFace->green==pGLFace->green&&
					pPrevGLFace->blue==pGLFace->blue&&pPrevGLFace->alpha==pGLFace->alpha)
					pGLFace->header.clr_norm &= 0x02;
				if( pPrevGLFace->nx==pGLFace->nx&&pPrevGLFace->ny==pGLFace->ny&&pPrevGLFace->nz==pGLFace->nz)
					pGLFace->header.clr_norm &= 0x01;
			}
			trianglesBuffer.ReadByte(&pGLFace->header.mode);
			trianglesBuffer.ReadWord(&pGLFace->header.uVertexNum);
			pGLFace->m_pVertexCoordArr=new GLdouble[pGLFace->header.uVertexNum*3];
			trianglesBuffer.Read(pGLFace->m_pVertexCoordArr,pGLFace->header.uVertexNum*24);
		}
		WriteToSolidBuffer(solidbuf,indexFace,listFacets,rawface.MatColor(),poly_norm);
	}
	if(pDstBody)
		pDstBody->CopyBuffer(solidbuf.GetBufferPtr(),solidbuf.GetLength(), false);
	return true;
}
static int GetIndexFromArr(Lib3dsPoint* pvecteData,int nSize,f3dPoint point)
{
	Lib3dsPoint* pPoint;
	ARRAY_LIST<Lib3dsPoint> vecteArr;
	vecteArr.SetSize(nSize);
	memcpy(vecteArr.m_pData,pvecteData,sizeof(Lib3dsPoint)*nSize);
	for (int nIndex=0;nIndex<vecteArr.GetSize();nIndex++)
	{
		pPoint=vecteArr.GetByIndex(nIndex);
		if(vecteArr[nIndex].pos[0]==(float)point.x &&
			vecteArr[nIndex].pos[1]==(float)point.y &&
			vecteArr[nIndex].pos[2]==(float)point.z)
			return nIndex;
	}
	return -1;
}
//////////////////////////////////////////////////////////////////////////
//C3DSData
C3DSData::C3DSData(int iNo)
{
	m_iNo=iNo;
}
C3DSData::~C3DSData(void)
{
}
//解析实体buffer,重新生成面
bool C3DSData::AddSolidPart(CSolidBody* pSolidBody,int nId,char* sSolidName,BOOL bTransPtMMtoM/*=FALSE*/,int nParentId/*=-1*/)
{
	//将SolidBody数据解析为三角面片实体数据
	static char solidbufpool[0x200000];	//零件实体缓存2M
	CSolidBody dstSolidBody(solidbufpool, 0x200000,true);
	if(!SplitToBasicFacets(pSolidBody,&dstSolidBody))
		return false;
	Lib3dsMesh* pMesh=lib3ds_mesh_new(sSolidName);
	Lib3dsNode* pNode=lib3ds_node_new_object();
	float fMatrix[4][4]= {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
	float red,green,blue;
	ARRAY_LIST<Lib3dsPoint> vecteArr(0,256); //点
	ARRAY_LIST<Lib3dsTexel> texelArr(0, 16);//纹理
	ARRAY_LIST<Lib3dsFace>  faceArr(0, 128); //面
	BOOL isFirstFace=TRUE;
	/*塔型默认尺寸为毫米，但是3DS MAX工具默认的尺寸单位为英寸
		根据客户要求，在生成3ds文件时，将塔型尺寸统一转换为英寸 wxc-2018.11.19*/
	//点
	int vertex_n=dstSolidBody.KeyPointNum();
	vecteArr.SetSize(vertex_n);
	double fCoef=0.0393701;	//毫米到英寸的转换比例
	for(int i=0;i<vertex_n;i++)
	{
		GEPOINT pt=dstSolidBody.GetKeyPointAt(i);
		vecteArr[i].pos[0]=(float)(pt.x*fCoef);
		vecteArr[i].pos[1]=(float)(pt.y*fCoef);
		vecteArr[i].pos[2]=(float)(pt.z*fCoef);
	}
	//面
	int face_n=dstSolidBody.BasicGLFaceNum();
	GEPOINT xarrVertexPool[16];
	for(int i=0;i<face_n;i++)
	{
		CBasicGlFace basicface;
		dstSolidBody.GetBasicGLFaceAt(i,basicface);
		int cluster_n=basicface.FacetClusterNumber;
		for(int j=0;j<cluster_n;j++)
		{
			CFacetCluster facet;
			basicface.GetFacetClusterAt(j,facet);
			DYN_ARRAY<GEPOINT> vptrs(facet.VertexNumber,false, xarrVertexPool,16);
			for(int jj=0;jj<facet.VertexNumber;jj++)
			{
				f3dPoint pt=facet.VertexAt(jj);
				vptrs[jj].x=pt.x*fCoef;
				vptrs[jj].y=pt.y*fCoef;
				vptrs[jj].z=pt.z*fCoef;
				if(GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[jj])==-1)
				{
					Lib3dsPoint point;
					point.pos[0]=(float)vptrs[jj].x;
					point.pos[1]=(float)vptrs[jj].y;
					point.pos[2]=(float)vptrs[jj].z;
					vecteArr.append(point);
				}
			}
			for(int k=0;k<facet.FacetNumber;k++)
			{
				Lib3dsFace* pFace=faceArr.append();
				if(facet.Mode==GL_TRIANGLES)
				{ 
					for(int ii=0;ii<3;ii++)
						pFace->points[ii]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k*3+ii]);
				}
				else if(facet.Mode==GL_TRIANGLE_STRIP)
				{
					if(k%2==0)
					{
						for(int ii=0;ii<3;ii++)
							pFace->points[ii]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k+ii]);
					}
					else
					{
						pFace->points[0]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k]);
						pFace->points[1]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k+2]);
						pFace->points[2]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k+1]);
					}
				}
				else if(facet.Mode==GL_TRIANGLE_FAN)
				{
					pFace->points[0]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[0]);
					pFace->points[1]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k+1]);
					pFace->points[2]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k+2]);
				}
				else
					continue;
				if(isFirstFace)
				{
					red   = GetRValue(basicface.Color)/255.0f;
					green = GetGValue(basicface.Color)/255.0f;
					blue  = GetBValue(basicface.Color)/255.0f;
					CreatNewMaterial(red,green,blue,sSolidName);
					isFirstFace=FALSE;
				}
				memcpy(pMesh->matrix,fMatrix,sizeof(Lib3dsMatrix));
				//
				pFace->normal[0]=(float)facet.Nx;
				pFace->normal[1]=(float)facet.Ny;
				pFace->normal[2]=(float)facet.Nz;
				strcpy(pFace->material,sSolidName);
			}
		}
	}
	if(vecteArr.GetSize()>0)
	{
		lib3ds_mesh_new_point_list(pMesh, vecteArr.GetSize());
		memcpy(pMesh->pointL,vecteArr.m_pData,vecteArr.Size() * sizeof(Lib3dsPoint));
	}
	if(faceArr.GetSize()>0)
	{
		lib3ds_mesh_new_face_list(pMesh, faceArr.GetSize());
		memcpy(pMesh->faceL,faceArr.m_pData,faceArr.Size() * sizeof(Lib3dsFace));
	}
	if(texelArr.GetSize()>0)
	{
		lib3ds_mesh_new_texel_list(pMesh, texelArr.GetSize());
		memcpy(pMesh->texelL,texelArr.m_pData,vecteArr.Size() * sizeof(Lib3dsPoint));
	}
	pNode->node_id=nId;
	pNode->type=LIB3DS_OBJECT_NODE;
	FillName(pNode->name,sSolidName,64);
	if(nParentId!=-1)
		pNode->parent_id=nParentId;
	else
		pNode->parent_id=LIB3DS_NO_PARENT;
	memcpy(pNode->matrix,fMatrix,sizeof(Lib3dsMatrix));
	m_vNode.push_back(pNode);
	m_vMesh.push_back(pMesh);
	pNode=pNode->next;
	pMesh=pMesh->next;
	return true;
}

void C3DSData::Creat3DSFile(const char* sFilePath)
{
	const static char* SCENE_NAME = "custom";
	m_3ds = lib3ds_file_new();
	FillName(m_3ds->name, SCENE_NAME, 12);
	if (!BuildNode())
		return ;
	for (size_t i = 0; i < m_vMaterial.size(); ++i)
		lib3ds_file_insert_material(m_3ds, m_vMaterial[i]);
	for (size_t i = 0; i < m_vMesh.size(); ++i)
		lib3ds_file_insert_mesh(m_3ds, m_vMesh[i]);

	lib3ds_file_save(m_3ds,sFilePath);
	lib3ds_file_free(m_3ds);
	for (size_t i = 0; i < m_vSource.size(); ++i)
		lib3ds_file_free(m_vSource[i]);
	//
	m_vSource.clear();
	m_vNode.clear();
	m_vMesh.clear();
	m_vMaterial.clear();
}

void C3DSData::CreatNewMaterial(float fRed,float fGreen,float fBlue,char* sName)
{
	double alpha = 0.6;	//考虑到显示效果的经验系数
	Lib3dsMaterial* pMaterial=lib3ds_material_new();
	strcpy(pMaterial->name,sName);
	pMaterial->ambient[0]=fRed;
	pMaterial->ambient[1]=fGreen;
	pMaterial->ambient[2]=fBlue;
	pMaterial->ambient[3]=(float)alpha;
	float fDiffuse[4]={0.4f,0.4f,0.4f,1.0f};
	float fSpecular[4]={0.898f,0.898f,0.898f,1.0f};
	memcpy(pMaterial->diffuse, fDiffuse, 4 * sizeof(float));
	memcpy(pMaterial->specular, fSpecular, 4 * sizeof(float));
	pMaterial->shininess=(float)0.67;
	/*Lib3dsTextureMap textule;
	strcpy(textule.name,"BUTTFACE.JPG");
	textule.offset[0]=0;
	textule.offset[1]=0;
	textule.scale[0]=1;
	textule.scale[1]=1;
	textule.percent=1;
	FillTexturemap(pMaterial->texture1_map,textule);*/
	m_vMaterial.push_back(pMaterial);
	pMaterial=pMaterial->next;
}

void C3DSData::BuildMaterial()
{
	set<string> setNames;
	for (size_t i = 0; i < m_vSource.size(); ++i)
	{
		Lib3dsMaterial* listMat = m_vSource[i]->materials;
		while (listMat)
		{
			string strName = listMat->name;
			if (RectifyName(strName, setNames, 63))
				RectifyMaterialReference(strName, listMat->name);

			Lib3dsMaterial* pMat = lib3ds_material_new();
			FillName(pMat->name, strName.c_str());
			pMat->next = NULL;

			FillMaterial(pMat, listMat);

			m_vMaterial.push_back(pMat);
			listMat = listMat->next;
		}
	}
	for (size_t i = 0; i < m_vMaterial.size(); ++i)
		lib3ds_file_insert_material(m_3ds, m_vMaterial[i]);
}
void C3DSData::BuildMesh()
{
	set<string> setNames;
	for (size_t i = 0; i < m_vSource.size(); ++i)
	{
		Lib3dsMesh* listMesh = m_vSource[i]->meshes;
		while (listMesh)
		{
			string strName = listMesh->name;
			if (RectifyName(strName, setNames, 63))
				RectifyMeshReference(strName, listMesh->name);

			Lib3dsMesh* pMesh = lib3ds_mesh_new(strName.c_str());
			pMesh->user = listMesh->user;
			pMesh->next = NULL;

			pMesh->object_flags = listMesh->object_flags;
			pMesh->color = listMesh->color;
			memcpy(pMesh->matrix, listMesh->matrix, sizeof(Lib3dsMatrix));

			if (listMesh->points)
			{
				lib3ds_mesh_new_point_list(pMesh, listMesh->points);
				memcpy(pMesh->pointL, listMesh->pointL, listMesh->points * sizeof(Lib3dsPoint));
			}

			if (listMesh->flags)
			{
				lib3ds_mesh_new_flag_list(pMesh, listMesh->flags);
				memcpy(pMesh->flagL, listMesh->flagL, listMesh->flags * sizeof(Lib3dsWord));
			}

			if (listMesh->texels)
			{
				lib3ds_mesh_new_texel_list(pMesh, listMesh->texels);
				memcpy(pMesh->texelL, listMesh->texelL, listMesh->texels * sizeof(Lib3dsTexel));
			}

			if (listMesh->faces)
			{
				lib3ds_mesh_new_face_list(pMesh, listMesh->faces);
				memcpy(pMesh->faceL, listMesh->faceL, listMesh->faces * sizeof(Lib3dsFace));
			}

			pMesh->box_map = listMesh->box_map;
			pMesh->map_data = listMesh->map_data;

			m_vMesh.push_back(pMesh);
			listMesh = listMesh->next;
		}
	}

	for (size_t i = 0; i < m_vMesh.size(); ++i)
		lib3ds_file_insert_mesh(m_3ds, m_vMesh[i]);
}
bool C3DSData::BuildNode()
{
	if (!RectifyNodeId())
		return false;
	for (size_t i = 0; i < m_vNode.size(); ++i)
		lib3ds_file_insert_node(m_3ds, m_vNode[i]);
	return true;
}
void C3DSData::FillName(char* strDst, const char* strSrc, size_t nMaxLen)
{
	size_t nNameLen = strlen(strSrc);
	if (nMaxLen && nNameLen > nMaxLen)
		nNameLen = nMaxLen;

	memcpy(strDst, strSrc, nNameLen);
	strDst[nNameLen] = 0;
}
bool C3DSData::RectifyName(string& strName, set<string>& setName, size_t nMaxLen)
{
	if (setName.find(strName) == setName.end())
		return false;

	char digit[32];
	strName = strName + _itoa(m_nName++, digit, 10);
	while (strName.size() > nMaxLen || setName.find(strName) != setName.end())
		strName = _itoa(m_nName++, digit, 10);

	setName.insert(strName);
	return true;
}
void C3DSData::FillMaterial(Lib3dsMaterial* pMat, const Lib3dsMaterial* pSrc)
{
	pMat->ambient[0] = pSrc->ambient[0]; // r
	pMat->ambient[1] = pSrc->ambient[1]; // g;
	pMat->ambient[2] = pSrc->ambient[2]; // b;
	pMat->ambient[3] = pSrc->ambient[3]; // a

	memcpy(pMat->diffuse, pSrc->diffuse, 4 * sizeof(float));
	memcpy(pMat->specular, pSrc->specular, 4 * sizeof(float));

	pMat->shininess = pSrc->shininess;
	pMat->shin_strength = pSrc->shin_strength;
	pMat->use_blur = pSrc->use_blur;
	pMat->blur = pSrc->blur;
	pMat->transparency = pSrc->transparency;
	pMat->falloff = pSrc->falloff;
	pMat->additive = pSrc->additive;
	pMat->self_ilpct = pSrc->self_ilpct;
	pMat->use_falloff = pSrc->use_falloff;
	pMat->self_illum = pSrc->self_illum;
	pMat->shading = pSrc->shading;
	pMat->soften = pSrc->soften;
	pMat->face_map = pSrc->face_map;
	pMat->two_sided = pSrc->two_sided;
	pMat->map_decal = pSrc->map_decal;
	pMat->use_wire = pSrc->use_wire;
	pMat->use_wire_abs = pSrc->use_wire_abs;
	pMat->wire_size = pSrc->wire_size;

	FillTexturemap(pMat->texture1_map, pSrc->texture1_map);
	FillTexturemap(pMat->texture1_mask, pSrc->texture1_mask);
	FillTexturemap(pMat->texture2_map, pSrc->texture2_map);
	FillTexturemap(pMat->texture2_mask, pSrc->texture2_mask);
	FillTexturemap(pMat->opacity_map, pSrc->opacity_map);
	FillTexturemap(pMat->opacity_mask, pSrc->opacity_mask);
	FillTexturemap(pMat->bump_map, pSrc->bump_map);
	FillTexturemap(pMat->bump_mask, pSrc->bump_mask);
	FillTexturemap(pMat->specular_map, pSrc->specular_map);
	FillTexturemap(pMat->specular_mask, pSrc->specular_mask);
	FillTexturemap(pMat->shininess_map, pSrc->shininess_map);
	FillTexturemap(pMat->shininess_mask, pSrc->shininess_mask);
	FillTexturemap(pMat->self_illum_map, pSrc->self_illum_map);
	FillTexturemap(pMat->self_illum_mask, pSrc->self_illum_mask);
	FillTexturemap(pMat->reflection_map, pSrc->reflection_map);
	FillTexturemap(pMat->reflection_mask, pSrc->reflection_mask);

	pMat->autorefl_map = pSrc->autorefl_map;
}
void C3DSData::FillTexturemap(Lib3dsTextureMap& texMap, const Lib3dsTextureMap& src)
{
	texMap.user = src.user;
	FillName(texMap.name, src.name);
	texMap.flags = src.flags;
	texMap.percent = src.percent;
	texMap.blur = src.blur;
	memcpy(texMap.scale, src.scale, 2*sizeof(Lib3dsFloat));
	memcpy(texMap.offset, src.offset, 2*sizeof(Lib3dsFloat));
	texMap.rotation = src.rotation;
	memcpy(texMap.tint_1, src.tint_1, 3*sizeof(Lib3dsFloat));
	memcpy(texMap.tint_2, src.tint_2, 3*sizeof(Lib3dsFloat));
	memcpy(texMap.tint_r, src.tint_r, 3*sizeof(Lib3dsFloat));
	memcpy(texMap.tint_g, src.tint_g, 3*sizeof(Lib3dsFloat));
	memcpy(texMap.tint_b, src.tint_b, 3*sizeof(Lib3dsFloat));
}
void C3DSData::RectifyMaterialReference(const string& strNewName, const char* strOldName)
{
	for (size_t i = 0; i < m_vSource.size(); ++i)
	{
		Lib3dsMesh* pMesh = m_vSource[i]->meshes;
		while (pMesh)
		{
			for (Lib3dsWord i = 0; i < pMesh->faces; ++i)
			{
				Lib3dsFace& face = pMesh->faceL[i];
				if (strcmp(strOldName, face.material) == 0)
					FillName(face.material, strNewName.c_str());
			}
		}
	}
}
void C3DSData::RectifyMeshReference(const string& strNewName, const char* strOldName)
{
	for (size_t i = 0; i < m_vSource.size(); ++i)
	{
		Lib3dsNode* pNode = m_vSource[i]->nodes;
		while (pNode)
		{
			if (strcmp(pNode->name, strOldName) == 0)
				FillName(pNode->name, strNewName.c_str());
		}
	}
}
bool C3DSData::RectifyNodeId()
{
	Lib3dsWord nIdStep = 2, nMaxId = 0;
	for (size_t i = 0; i < m_vSource.size(); ++i)
	{
		Lib3dsNode* listNode = m_vSource[i]->nodes;
		while (listNode)
		{
			nMaxId = RectifyNodeId(listNode, nIdStep);

			nIdStep = nMaxId + 1;
			listNode = listNode->next;
		}
	}

	return nMaxId != 0xFFFF;
}
Lib3dsWord C3DSData::RectifyNodeId(Lib3dsNode* pNode, Lib3dsWord nStep)
{
	unsigned int nTestNodeId = pNode->node_id + (unsigned int)nStep;
	if (nTestNodeId > 0xFFFF)
		return 0xFFFF;

	pNode->node_id += nStep;
	if (pNode->parent_id != LIB3DS_NO_PARENT)
		pNode->parent_id += nStep;

	Lib3dsWord nMaxId = pNode->node_id, nNodeId = 0;
	Lib3dsNode* pChild = pNode->childs;
	while (pChild)
	{
		nNodeId = RectifyNodeId(pChild, nStep);
		if (nNodeId > nMaxId)
			nMaxId = nNodeId;

		pChild = pChild->next;
	}

	return nMaxId;
}
void C3DSData::CloneNodeDataAmbient(Lib3dsAmbientData& dstAmbient, const Lib3dsAmbientData& srcAmbient)
{
	memcpy(dstAmbient.col, srcAmbient.col, sizeof(Lib3dsRgb));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstAmbient.col_track, srcAmbient.col_track);
}
void C3DSData::CloneNodeDataObject(Lib3dsObjectData& dstObject, const Lib3dsObjectData& srcObject)
{
	memcpy(dstObject.pivot, srcObject.pivot, sizeof(Lib3dsVector));
	memcpy(dstObject.instance, srcObject.instance, 64 * sizeof(char));
	memcpy(dstObject.bbox_min, srcObject.bbox_min, sizeof(Lib3dsVector));
	memcpy(dstObject.bbox_max, srcObject.bbox_max, sizeof(Lib3dsVector));
	memcpy(dstObject.pos, srcObject.pos, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstObject.pos_track, srcObject.pos_track);
	memcpy(dstObject.rot, srcObject.rot, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsQuatTrack, Lib3dsQuatKey, SQuatKeyCloner>(dstObject.rot_track, srcObject.rot_track);
	memcpy(dstObject.scl, srcObject.scl, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstObject.scl_track, dstObject.scl_track);
	dstObject.morph_smooth = srcObject.morph_smooth;
	memcpy(dstObject.morph, srcObject.morph, 64 * sizeof(char));
	CloneTrack<Lib3dsMorphTrack, Lib3dsMorphKey, SMorphKeyCloner>(dstObject.morph_track, srcObject.morph_track);
	dstObject.hide = srcObject.hide;
	CloneTrack<Lib3dsBoolTrack, Lib3dsBoolKey, SBoolKeyCloner>(dstObject.hide_track, dstObject.hide_track);
}
void C3DSData::CloneNodeDataCamera(Lib3dsCameraData& dstCamera, const Lib3dsCameraData& srcCamera)
{
	memcpy(dstCamera.pos, srcCamera.pos, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstCamera.pos_track, srcCamera.pos_track);
	dstCamera.fov = srcCamera.fov;
	CloneTrack<Lib3dsLin1Track, Lib3dsLin1Key, SLin1KeyCloner>(dstCamera.fov_track, srcCamera.fov_track);
	dstCamera.roll = srcCamera.roll;
	CloneTrack<Lib3dsLin1Track, Lib3dsLin1Key, SLin1KeyCloner>(dstCamera.roll_track, srcCamera.roll_track);
}
void C3DSData::CloneNodeDataTarget(Lib3dsTargetData& dstTarget, const Lib3dsTargetData& srcTarget)
{
	memcpy(dstTarget.pos, srcTarget.pos, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstTarget.pos_track, srcTarget.pos_track);
}
void C3DSData::CloneNodeDataLight(Lib3dsLightData& dstLight, const Lib3dsLightData& srcLight)
{
	memcpy(dstLight.pos, srcLight.pos, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstLight.pos_track, srcLight.pos_track);
	memcpy(dstLight.col, srcLight.col, sizeof(Lib3dsRgb));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstLight.col_track, srcLight.col_track);
	dstLight.hotspot = srcLight.hotspot;
	CloneTrack<Lib3dsLin1Track, Lib3dsLin1Key, SLin1KeyCloner>(dstLight.hotspot_track, srcLight.hotspot_track);
	dstLight.falloff = srcLight.falloff;
	CloneTrack<Lib3dsLin1Track, Lib3dsLin1Key, SLin1KeyCloner>(dstLight.falloff_track, srcLight.falloff_track);
	dstLight.roll = srcLight.roll;
	CloneTrack<Lib3dsLin1Track, Lib3dsLin1Key, SLin1KeyCloner>(dstLight.roll_track, srcLight.roll_track);
}
void C3DSData::CloneNodeDataSpot(Lib3dsSpotData& dstSpot, const Lib3dsSpotData& srcSpot)
{
	memcpy(dstSpot.pos, srcSpot.pos, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstSpot.pos_track, srcSpot.pos_track);
}
void C3DSData::CloneNodeData(Lib3dsNodeData& data, const Lib3dsNodeData& srcData)
{
	CloneNodeDataAmbient(data.ambient, srcData.ambient);
	CloneNodeDataObject(data.object, srcData.object);
	CloneNodeDataCamera(data.camera, srcData.camera);
	CloneNodeDataTarget(data.target, srcData.target);
	CloneNodeDataLight(data.light, srcData.light);
	CloneNodeDataSpot(data.spot, srcData.spot);
}
void C3DSData::CloneNode(const Lib3dsNode* pSrcNode)
{
	Lib3dsNode* pNode = lib3ds_node_new_object();
	pNode->user = pSrcNode->user;
	pNode->node_id = pSrcNode->node_id;
	memcpy(pNode->matrix, pSrcNode->matrix, sizeof(Lib3dsMatrix));
	pNode->type = pSrcNode->type;
	FillName(pNode->name, pSrcNode->name);
	pNode->flags1 = pSrcNode->flags1;
	pNode->flags2 = pSrcNode->flags2;
	pNode->parent_id = pSrcNode->parent_id;
	CloneNodeData(pNode->data, pSrcNode->data);
	m_vNode.push_back(pNode);

	Lib3dsNode* pChild = pSrcNode->childs;
	while (pChild)
	{
		CloneNode(pChild);
		pChild = pChild->next;
	}
}
void SBoolKeyCloner::Clone(Lib3dsBoolKey*& keyDst, const Lib3dsBoolKey* keySrc)
{
	keyDst = lib3ds_bool_key_new();
	keyDst->tcb = keySrc->tcb;
}
void SLin1KeyCloner::Clone(Lib3dsLin1Key*& keyDst, const Lib3dsLin1Key* keySrc)
{
	keyDst = lib3ds_lin1_key_new();
	keyDst->tcb = keySrc->tcb;
	keyDst->value = keySrc->value;
	keyDst->dd = keySrc->dd;
	keyDst->ds = keyDst->ds;
}
void SLin3KeyCloner::Clone(Lib3dsLin3Key*& keyDst, const Lib3dsLin3Key* keySrc)
{
	keyDst = lib3ds_lin3_key_new();
	keyDst->tcb = keySrc->tcb;
	keyDst->next = NULL;
	memcpy(keyDst->value, keySrc->value, sizeof(Lib3dsVector));
	memcpy(keyDst->dd, keySrc->dd, sizeof(Lib3dsVector));
	memcpy(keyDst->ds, keySrc->ds, sizeof(Lib3dsVector));
}
void SQuatKeyCloner::Clone(Lib3dsQuatKey*& keyDst, const Lib3dsQuatKey* keySrc)
{
	keyDst = lib3ds_quat_key_new();
	keyDst->tcb = keySrc->tcb;
	memcpy(keyDst->axis, keySrc->axis, sizeof(Lib3dsVector));
	keyDst->angle = keySrc->angle;
	memcpy(keyDst->q, keySrc->q, sizeof(Lib3dsQuat));
	memcpy(keyDst->dd, keySrc->dd, sizeof(Lib3dsQuat));
	memcpy(keyDst->ds, keySrc->ds, sizeof(Lib3dsQuat));
}
void SMorphKeyCloner::Clone(Lib3dsMorphKey*& keyDst, const Lib3dsMorphKey* keySrc)
{
	keyDst = lib3ds_morph_key_new();
	keyDst->tcb = keySrc->tcb;
	memcpy(keyDst->name, keySrc->name, 64 * sizeof(char));
}