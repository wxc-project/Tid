// STLData.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "STL.h"
#include "glDrawTool.h"
#include "f_alg_fun.h"

//
//TODO: 如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//


#include "stdafx.h"
#include "Stl.h"
#include "io.h"
#include <iostream>
#include <sstream>

#pragma region 
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
	CSolidBodyBuffer solidbuf;
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
		pDstBody->CopyBuffer(solidbuf.GetBufferPtr(),solidbuf.GetLength());
	return true;
}

//////////////////////////////////////////////////////////////////////////
//CSTLData
CSTLData::CSTLData(int iNo/*=1*/)
{
	m_iNo=iNo;
	m_FacetList.Empty();
	m_FacetList.SetSize(0, 0x200000);	//0x200000=2M个面片
}
CSTLData::~CSTLData(void)
{
}
bool CSTLData::IsSTLBinary(const char *filename)
{
	bool bBinary = false;//return value
	FILE *fp = nullptr;
	int errorCode = fopen_s(&fp,filename, "r");
	if (errorCode==0)//成功打开文件
	{
		//确定文件实际大小
		fseek(fp, 0, SEEK_END);//将fp移动到文件尾部
		int fileSize = ftell(fp);//返回文档首部到fp位置大小（bytes）
		int facetNum;
		//计算标准stl二进制文件的大小
		fseek(fp, STL_LABEL_SIZE, SEEK_SET);//跳过文档开头的注释
		fread(&facetNum, sizeof(int), 1, fp);//读取facet的数目并保存在facetNum中
		int standardBinaryFileSize = 80 + sizeof(int)+facetNum * 50;
		//判断是否是标准stl文件
		if (fileSize==standardBinaryFileSize)
		{
			bBinary = true;        
		}
		//判断是否是非标准stl文件
		unsigned char tmpbuf[128];//如果文件过短，这里会有Bug
		fread(tmpbuf, sizeof(tmpbuf), 1, fp);//读取128个char大小的数据
		for (unsigned int i = 0;i<sizeof(tmpbuf);i++)
		{
			//char类型取值范围是-128~127,如果是ASCII格式，应该全是char
			if (tmpbuf[i]>127)
			{
				bBinary = true;
				break;
			}
		}

		fclose(fp);
	}
	return bBinary;
}

bool CSTLData::SaveStlBinary(const char *file_path)
{
	FILE *fp = fopen(file_path,"wb");
	if(fp==NULL)
		return false;
	int nFaceName=m_FacetList.GetSize();
	char fname[MAX_PATH],fileInf[MAX_PATH];
	_splitpath(file_path,NULL,NULL,fname,NULL);
	sprintf(fileInf,"%s.stl", fname);
	//
	fwrite(fileInf,sizeof(char),80,fp);
	fwrite(&nFaceName,sizeof(int),1,fp);
	for(int i=0;i<nFaceName;i++)
	{
		CSTLFacet face=	m_FacetList[i];
		float v1x = (float)face.m_PointList[0].x;
		float v1y = (float)face.m_PointList[0].y;
		float v1z = (float)face.m_PointList[0].z;
		float v2x = (float)face.m_PointList[1].x;
		float v2y = (float)face.m_PointList[1].y;
		float v2z = (float)face.m_PointList[1].z;
		float v3x = (float)face.m_PointList[2].x;
		float v3y = (float)face.m_PointList[2].y;
		float v3z = (float)face.m_PointList[2].z;
		float nx = (v1y-v3y)*(v2z-v3z)-(v1z-v3z)*(v2y-v3y);
		float ny = (v1z-v3z)*(v2x-v3x)-(v2z-v3z)*(v1x-v3x);
		float nz = (v1x-v3x)*(v2y-v3y)-(v2x-v3x)*(v1y-v3y);
		float nxyz = sqrt(nx*nx+ny*ny+nz*nz);
		float normX = nx/nxyz;
		float normY = ny/nxyz;
		float normZ = nz/nxyz;
		//
		fwrite(&normX, sizeof(float), 1, fp);
		fwrite(&normY, sizeof(float), 1, fp);
		fwrite(&normZ, sizeof(float), 1, fp);
		fwrite(&v1x, sizeof(float), 1, fp);
		fwrite(&v1y, sizeof(float), 1, fp);
		fwrite(&v1z, sizeof(float), 1, fp);
		fwrite(&v2x, sizeof(float), 1, fp);
		fwrite(&v2y, sizeof(float), 1, fp);
		fwrite(&v2z, sizeof(float), 1, fp);
		fwrite(&v3x, sizeof(float), 1, fp);
		fwrite(&v3y, sizeof(float), 1, fp);
		fwrite(&v3z, sizeof(float), 1, fp);
		fwrite("wl",sizeof(char),2,fp);
	}
	fclose(fp);
	return true;
}
bool CSTLData::SaveStlASCII(const char *file_path)
{
	FILE *fp = fopen(file_path,"wt");
	if(fp==NULL)
		return false;
	char fname[MAX_PATH],fileInf[MAX_PATH];
	_splitpath(file_path,NULL,NULL,fname,NULL);
	sprintf(fileInf,"solid %s",fname);
	//写入文件信息
	fprintf(fp,"%s\n",fileInf);
	//写入三角面信息
	for(int i=0;i<m_FacetList.GetSize();i++)
	{
		CSTLFacet face=	m_FacetList[i];
		float v1x = (float)face.m_PointList[0].x;
		float v1y = (float)face.m_PointList[0].y;
		float v1z = (float)face.m_PointList[0].z;
		float v2x = (float)face.m_PointList[1].x;
		float v2y = (float)face.m_PointList[1].y;
		float v2z = (float)face.m_PointList[1].z;
		float v3x = (float)face.m_PointList[2].x;
		float v3y = (float)face.m_PointList[2].y;
		float v3z = (float)face.m_PointList[2].z;
		float nx = (v1y-v3y)*(v2z-v3z)-(v1z-v3z)*(v2y-v3y);
		float ny = (v1z-v3z)*(v2x-v3x)-(v2z-v3z)*(v1x-v3x);
		float nz = (v1x-v3x)*(v2y-v3y)-(v2x-v3x)*(v1y-v3y);
		float nxyz = sqrt(nx*nx+ny*ny+nz*nz);
		//
		fprintf(fp,"facet normal %f %f %f\n",nx/nxyz,ny/nxyz,nz/nxyz);
		fprintf(fp,"outer loop\n");
		fprintf(fp,"vertex %f %f %f\n",v1x,v1y,v1z);
		fprintf(fp,"vertex %f %f %f\n",v2x,v2y,v2z);
		fprintf(fp,"vertex %f %f %f\n",v3x,v3y,v3z);
		fprintf(fp,"endloop\n");
		fprintf(fp,"endfacet\n");
	}
	sprintf(fileInf,"endsolid %s.stl  by master",fname);
	fprintf(fp,"%s\n",fileInf);
	fclose(fp);
	return true;
}
void CSTLData::AddSolidBody(char* solidbuf/*=NULL*/,DWORD size/*=0*/)
{
	//将Solid解析为三角面数据
	CSolidBody xSolid,xSolidExter(solidbuf,size);
	if(!SplitToBasicFacets(&xSolidExter,&xSolid))
		return;
	//记录三角面信息
	int face_n=xSolid.BasicGLFaceNum();
	GEPOINT xarrVertexPool[16];
	for(int i=0;i<face_n;i++)
	{
		CBasicGlFace xBasicFace;
		if(!xSolid.GetBasicGLFaceAt(i,xBasicFace))
			continue;
		int cluster_n=xBasicFace.FacetClusterNumber;
		for(int j=0;j<cluster_n;j++)
		{
			CFacetCluster xFacet;
			if(!xBasicFace.GetFacetClusterAt(j,xFacet))
				continue;

			DYN_ARRAY<GEPOINT> vptrs(xFacet.VertexNumber,false, xarrVertexPool,16);
			for(int jj=0;jj<xFacet.VertexNumber;jj++)
				vptrs[jj]=xFacet.VertexAt(jj);
			for(int k=0;k<xFacet.FacetNumber;k++)
			{
				CSTLFacet* pStlFace=m_FacetList.append();
				pStlFace->m_Normal.x=xFacet.Nx;
				pStlFace->m_Normal.y=xFacet.Ny;
				pStlFace->m_Normal.z=xFacet.Nz;
				if(xFacet.Mode==GL_TRIANGLES)
				{ 
					pStlFace->m_PointList[0]=vptrs[k*3];
					pStlFace->m_PointList[1]=vptrs[k*3+1];
					pStlFace->m_PointList[2]=vptrs[k*3+2];
				}
				else if(xFacet.Mode==GL_TRIANGLE_STRIP)
				{
					if(k%2==0)
					{
						pStlFace->m_PointList[0]=vptrs[k];
						pStlFace->m_PointList[1]=vptrs[k+1];
						pStlFace->m_PointList[2]=vptrs[k+2];
					}
					else
					{
						pStlFace->m_PointList[0]=vptrs[k];
						pStlFace->m_PointList[1]=vptrs[k+2];
						pStlFace->m_PointList[2]=vptrs[k+1];
					}
				}
				else if(xFacet.Mode==GL_TRIANGLE_FAN)
				{
					pStlFace->m_PointList[0]=vptrs[k];
					pStlFace->m_PointList[1]=vptrs[k+1];
					pStlFace->m_PointList[2]=vptrs[k+2];
				}
				else
					continue;
			}
		}
	}
}
void CSTLData::SaveFile(const char* path,int nType)
{
	if(nType==TYPE_SAVE_ASCII)
		SaveStlASCII(path);
	else if(nType==TYPE_SAVE_BINARY)
		SaveStlBinary(path);
}