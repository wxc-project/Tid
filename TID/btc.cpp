#include "Stdafx.h"
#include "btc.h"
#include "f_alg_fun.h"

GEPOINT btc::CalLocalCS_X_AXIS(const double* axis_z_coords)
{
	GEPOINT axis_x,axis_z(axis_z_coords);
	GEPOINT vec = axis_z;
	int iStdVecType = 0;
	if (fabs(vec.y) < EPS&&fabs(vec.z) < EPS)
	{
		if (vec.x > 0)
			iStdVecType = 1;	//X+方向
		else if (vec.x < 0)
			iStdVecType = 2;	//X-方向
	}
	else if (fabs(vec.x) < EPS&&fabs(vec.z) < EPS)
	{
		if (vec.y > 0)
			iStdVecType = 3;	//Y+方向
		else if (vec.y < 0)
			iStdVecType = 4;	//Y-方向
	}
	else if (fabs(vec.x) < EPS&&fabs(vec.y) < EPS)
	{
		if (vec.z > 0)
			iStdVecType = 5;	//Z+方向
		else if (vec.z < 0)
			iStdVecType = 6;	//Z-方向
	}
	//
	if(axis_z.IsZero())
		return GEPOINT(1,0,0);
	else if(iStdVecType==1)	//X+
		axis_x.Set(0,-1,0);
	else if(iStdVecType==2)	//X-
		axis_x.Set(0, 1,0);
	else if(iStdVecType==3)	//Y+
		axis_x.Set( 1,0,0);
	else if(iStdVecType==4)	//Y-
		axis_x.Set(-1,0,0);
	else if(iStdVecType==5)	//Z+
		axis_x.Set( 1,0,0);
	else if(iStdVecType==6)	//Z-
		axis_x.Set( 1,0,0);
	if(!axis_x.IsZero())
		return axis_x;
	if(fabs(axis_z.x)+fabs(axis_z.y)<EPS&&fabs(axis_z.y)>EPS)
		return GEPOINT(1,0,0);	//局部坐标系Z轴与整体坐标系Z轴平行
	else
	{
		GEPOINT axis_x(axis_z.y,-axis_z.x,0);
		normalize(axis_x);
		return axis_x;
	}
}
//////////////////////////////////////////////////////////////////////
btc::SKETCH_PLANE::SKETCH_PLANE(const double* origin/*=NULL*/,const double* normvec/*=NULL*/,const double* edgevec/*=NULL*/,
					WORD wEdgeLength/*=300*/,WORD wEdgeCount/*=4*/,double underLength/*=0*/)
{
	_pVertexArr=NULL;
	m_fUnderLength=_nVertexCount=0;
	if(origin!=NULL&&normvec!=NULL)
		CreateStdPlane(origin,normvec,edgevec,wEdgeLength,wEdgeCount,underLength);
}
btc::SKETCH_PLANE::~SKETCH_PLANE()
{
	if(_pVertexArr!=NULL&&_nVertexCount>0)
		delete []_pVertexArr;
}
bool btc::SKETCH_PLANE::CreateStdPlane(const double* origin,const double* normvec,const double* edgevec,
			WORD wEdgeLength/*=10*/,WORD wEdgeCount/*=4*/,double underLength/*=0*/)
{
	m_fUnderLength=underLength;
	normal=normvec;
	GEPOINT start(origin);
	if(_pVertexArr!=NULL&&_nVertexCount>0)
		delete []_pVertexArr;
	if((_nVertexCount=wEdgeCount)==0)
		return false;
	_pVertexArr = new GEPOINT[_nVertexCount];
	GECS cs;
	cs.axis_z=GEPOINT(normal);
	if(!Standarized(cs.axis_z))
		return false;
	if(edgevec)
	{
		cs.axis_x=edgevec;
		cs.axis_x-=(cs.axis_x*cs.axis_z)*cs.axis_z;
		normalize(cs.axis_x);
	}
	if(cs.axis_x.IsZero())
		cs.axis_x=CalLocalCS_X_AXIS(cs.axis_z);
	cs.axis_y=cs.axis_z^cs.axis_x;
	cs.origin=start;
	if(_nVertexCount==1)
	{
		_pVertexArr[0]=start;
		return true;
	}
	else if(_nVertexCount==2)
	{
		_pVertexArr[0]=start-cs.axis_x*(wEdgeLength*0.5);
		_pVertexArr[1]=start+cs.axis_x*(wEdgeLength*0.5);
		return true;
	}
	else if(_nVertexCount==4)
	{
		double halflen=wEdgeLength*0.5;
		_pVertexArr[0].Set(-halflen,-halflen);
		_pVertexArr[1].Set( halflen,-halflen);
		_pVertexArr[2].Set( halflen, halflen);
		_pVertexArr[3].Set(-halflen, halflen);
		for(int i=0;i<4;i++)
			_pVertexArr[i]=cs.TransPFromCS(_pVertexArr[i]);
		return true;
	}
	else
		return false;
}
bool btc::SKETCH_PLANE::CreatePlaneIndirect(const double* normal,GEPOINT* pPosArr,UINT nVertexCount,
			double underLength/*=0*/)
{
	if (nVertexCount==0||pPosArr==NULL)
		return false;
	if(_nVertexCount>0&&_pVertexArr)
		delete[] _pVertexArr;
	_nVertexCount = nVertexCount;
	_pVertexArr = new GEPOINT[_nVertexCount];
	for (int i=0;i<(int)nVertexCount;i++)
		_pVertexArr[i]=pPosArr[i];
	m_fUnderLength = underLength;
	this->normal = normal;
	return true;
}
#ifdef __ALFA_TEST_VERSION_
void TestSketchPlane()
{
	//btc::SKETCH_PLANE sketch;
	//sketch.CreateStdPlane(normal,origin);
}
#endif