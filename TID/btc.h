#pragma once
#include <WinDef.h>
#include "f_ent.h"
#include "MemberProperty.h"

//Basic Toolkit Class Definition
class btc{
public:
	struct RAYLINE{GEPOINT pick,vec;};
	struct WORKPLANE{GEPOINT pick,norm;};
	class SKETCH_PLANE{
		UINT _nVertexCount;
		GEPOINT* _pVertexArr;
	public:
		double m_fUnderLength;	//平面法线箭头示意图,自平面基点向平面下延伸的实际长度(非屏幕长度)
		GEPOINT normal;
		SKETCH_PLANE(const double* origin=NULL,const double* normvec=NULL,const double* edgevec=NULL,
					WORD wEdgeLength=300,WORD wEdgeCount=4,double underLength=0);
		~SKETCH_PLANE();
	public:
		READONLY_PROPERTY(UINT,VertexCount);
		GET(VertexCount){return _nVertexCount;}
		READONLY_PROPERTY(GEPOINT*,pVertexArr);
		GET(pVertexArr){return _pVertexArr;}
	public:
		bool CreateStdPlane(const double* origin,const double* normvec,const double* edgevec=NULL,
					WORD wEdgeLength=300,WORD wEdgeCount=4,double underLength=0);
		bool CreatePlaneIndirect(const double* normal,GEPOINT* pVertexArr,UINT nVertexCount,double underLength=0);
	};
	static GEPOINT CalLocalCS_X_AXIS(const double* axis_z_coords);
};
