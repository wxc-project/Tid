#pragma once

// CSTLDataApp
// 有关此类实现的信息，请参阅 STLData.cpp
//
#include <vector>
#include <string>
#include "ArrayList.h"
#include "f_ent.h"
#include "SolidBody.h"
#include "gl/GL.h"
#include "gl/glu.h"
#include "ISTL.h"
using namespace std;

class CGLFace  
{
public:
	struct GLFACEINFO{
		//第一位：0.表示面颜色与面片组CGLFaceGroup相同；1.表示特殊指定值
		//第二位：0.表示面片法线与面片组CGLFaceGroup相同；1.表示特殊指定值
		char clr_norm;
		char mode;		//绘制启动函数glBegin()中需要的绘制模式
		WORD uVertexNum;//每一面片的顶点数
	};
public:
	GLfloat red,green,blue,alpha;	//颜色
	GLdouble nx,ny,nz;				//面法线
	GLFACEINFO header;
	GLdouble *m_pVertexCoordArr;
	CGLFace(){memset(this,0,sizeof(CGLFace));header.mode=GL_TRIANGLES;}
	~CGLFace()
	{
		if(m_pVertexCoordArr)	//header.uVertexNum>0&&
			delete []m_pVertexCoordArr;
		m_pVertexCoordArr=NULL;
		header.uVertexNum=0;
	}
};
class CSTLPoint
{
public:
	double x,y,z;
	CSTLPoint(){;}
	CSTLPoint(GEPOINT pos){SetParam(pos.x,pos.y,pos.z);}
	CSTLPoint(double posx,double posy,double posz){SetParam(posx,posy,posz);}
	void SetParam(double posx,double posy,double posz){x=posx;y=posy;z=posz;}
};
class CSTLFacet
{
public:
	CSTLPoint m_PointList[3];//三个顶点
	CSTLPoint m_Normal;//法向量
	CSTLFacet(){;}
};
class CSTLData : public IStlData
{
private:
	int m_iNo;
	ARRAY_LIST<CSTLFacet> m_FacetList;
public:
	static const int STL_LABEL_SIZE = 80;
private:
	bool IsSTLBinary(const char *filename);
	bool SaveStlBinary(const char *file_path);
	bool SaveStlASCII(const char *file_path);
public:
	CSTLData(int iNo=1);
	~CSTLData(void);
	//
	virtual int GetSerial(){return m_iNo;}
	virtual void AddSolidBody(char* solidbuf=NULL,DWORD size=0);
	virtual void SaveFile(const char* file_path,int nType=TYPE_SAVE_BINARY);
};
