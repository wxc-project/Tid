// glDrawTool.cpp : implementation file
//

#include "stdafx.h"
#include "gl/glu.h"
#include "glDrawTool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//////////////////////////////////////////////////////////////////////
//
// Implementation of CGLDispList class.
//
/*** DESCRIPTION

  This is actually a helper class which wraps the
  use of display list in OGL.
  It must be used inside an GLEnabledView cause
  a display list must refer to a Rendering Context.
  At present there is no support for Disp. Lists
  Sharing among multiple RCs (that is multiple MDI
  child windows).

****************************************/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CGLDispList::CGLDispList():
	m_glListId(0), m_bIsolated(FALSE)
{
}

CGLDispList::~CGLDispList()
{
// remove display list
	glDeleteLists(m_glListId,1); 
}

//////////////////////////////////////////////////////////////////////
// Member functions

void CGLDispList::Draw()
{
// if the list is not empty...
	if(m_glListId)
	{
		if(m_bIsolated)
		{
// save current transformation matrix
			glPushMatrix();
// save current OGL internal state (lighting, shading, and such)
			glPushAttrib(GL_ALL_ATTRIB_BITS);
		};
// draw the list
		glCallList(m_glListId);
		if(m_bIsolated)
		{
// restore transformation matrix
			glPopMatrix();
// restore OGL internal state
			glPopAttrib();
		};
	};
}

void CGLDispList::StartDef(BOOL bImmediateExec)
{
// check if another list is under construction
	int cur;
	glGetIntegerv(GL_LIST_INDEX,&cur);
	if(cur != 0)
	{
		//TRACE0("CDrawSolid\n\tError: Nested display list definition!\n");
		//ASSERT(FALSE);
	};
// if the list is empty firstly allocate one
	if(!m_glListId) m_glListId=glGenLists(1);

// start or replace a list definition
	if (bImmediateExec) glNewList(m_glListId,GL_COMPILE_AND_EXECUTE);
	else  glNewList(m_glListId,GL_COMPILE);
}

void CGLDispList::EndDef()
{
// check the coupling with a preceding call to StartDef()
	int cur;
	glGetIntegerv(GL_LIST_INDEX,&cur);
	if(cur != m_glListId) 
	{
		//TRACE0("CGLDispList:Missing StartDef() before EndDef()\n");
		return;
	};
// close list definition
	glEndList();
}

void CGLDispList::DeleteList()
{
	if(m_glListId>0)
	{
		glDeleteLists(m_glListId,1);
		m_glListId=0;
	}
}

//////////////////////////////////////////////////////////////////////
//
// Implementation of CGLTesselator class.
//
/*** DESCRIPTION

  This is actually a helper class which wraps the
  use of tessellation objects in OGL (see guide).
  It must be used inside an GLEnabledView cause
  a tesselator object must refer to a Rendering Context.

****************************************/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CBuffer trianglesBuffer(2048);
static WORD vertexHits=0;
void CALLBACK BeginCallback(GLenum type)
{
// issue corresponding GL call
	//glBegin(type);	//现在不在这里绘图而时把显示列表输出给用户管理
	trianglesBuffer.WriteByte((BYTE)type);
	trianglesBuffer.LogPosition();
	vertexHits=0;
	trianglesBuffer.WriteWord(vertexHits);
}

void CALLBACK ErrorCallback(GLenum errorCode)
{
	/*
	const GLubyte *estring;
	CString mexstr;
// get the error descritption from OGL
	estring = gluErrorString(errorCode);
// prepare and show a message box
	CXhChar50("Tessellation/Quadric Error: %s\n", estring);
	AfxMessageBox(mexstr,MB_OK | MB_ICONEXCLAMATION);
// replicate mex to debug trace
	TRACE0(mexstr);
	*/
}

void CALLBACK EndCallback()
{
// issue corresponding GL call
	//glEnd();	//现在不在这里绘图而时把显示列表输出给用户管理
	trianglesBuffer.RecallPosition();
	trianglesBuffer.WriteWord(vertexHits);
	trianglesBuffer.RecallPosition();
}

void CALLBACK VertexCallback(GLvoid *vertex)
{
// issue corresponding GL call (double is used to get max precision)
	//glVertex3dv( (const double *)vertex );	//现在不在这里绘图而时把显示列表输出给用户管理
	trianglesBuffer.Write(vertex,24);
	vertexHits++;
}
GLdouble g_vertex[3];
void CALLBACK CombineCallback(GLdouble coords[3], GLdouble *data[4], GLfloat weight[4], GLdouble **dataOut )
{
// allocate memory for a new vertex
	GLdouble *vertex;
	vertex = g_vertex;//new GLdouble[3];
// store reported vertex
	vertex[0] = coords[0];
	vertex[1] = coords[1];
	vertex[2] = coords[2];
// return vertex to OGL
	*dataOut = vertex;
}

CGLTesselator::CGLTesselator()
{
// create tessellation object
	m_tessObj=gluNewTess();
// set callback functions
	gluTessCallback(m_tessObj,GLU_TESS_BEGIN,(void (CALLBACK*)())&BeginCallback); 
	gluTessCallback(m_tessObj,GLU_TESS_VERTEX,(void (CALLBACK*)())&VertexCallback); 
	gluTessCallback(m_tessObj,GLU_TESS_END,(void (CALLBACK*)())&EndCallback);
	gluTessCallback(m_tessObj,GLU_TESS_COMBINE,(void (CALLBACK*)())&CombineCallback);
	gluTessCallback(m_tessObj,GLU_TESS_ERROR,(void (CALLBACK*)())&ErrorCallback);
}

CGLTesselator::~CGLTesselator()
{
// remove tessellation object
	gluDeleteTess(m_tessObj);	
}

//////////////////////////////////////////////////////////////////////
// Member functions

void CGLTesselator::SetWindingRule(GLdouble which)
{
// issue the equivalent GL call
	gluTessProperty(m_tessObj,GLU_TESS_WINDING_RULE,which); 
}

GLdouble CGLTesselator::GetWindingRule()
{
//retrieve attribute
	GLdouble temp=-1.0;
	gluTessProperty(m_tessObj,GLU_TESS_WINDING_RULE,temp);
// return value
	return temp;
}

void CGLTesselator::SetFilling(BOOL bFill)
{
// issue the equivalent GL calls
	if(bFill) gluTessProperty(m_tessObj,GLU_TESS_BOUNDARY_ONLY,GL_FALSE);
	else gluTessProperty(m_tessObj,GLU_TESS_BOUNDARY_ONLY,GL_TRUE);
}

BOOL CGLTesselator::GetFilling()
{
//retrieve attribute
	GLdouble temp=-1.0;
	gluTessProperty(m_tessObj,GLU_TESS_BOUNDARY_ONLY,temp);
// convert to a boolean
	return (temp==GL_TRUE);
}

void CGLTesselator::StartDef()
{
	trianglesBuffer.ClearContents();
// start a polygon definition
	gluTessBeginPolygon(m_tessObj,NULL);
// start a contour definition
	gluTessBeginContour(m_tessObj);
}

void CGLTesselator::EndDef()
{
// end contour and polygon definition
	gluTessEndContour(m_tessObj);
	gluTessEndPolygon(m_tessObj);
}

void CGLTesselator::ContourSeparator()
{
// insert a contour separation
	gluTessEndContour(m_tessObj);
	gluTessBeginContour(m_tessObj);
}
void CGLTesselator::TessNormal(GLdouble x,GLdouble y,GLdouble z)
{
	gluTessNormal(m_tessObj,x,y,z);
}

void CGLTesselator::AddVertex(GLdouble vertData[3])
{
// IMPORTANT: the 3rd parameter must be given otherwise an access
// violation will occur, moreover every vertex must have it's own memory
// location till the closing of the polygon (that is you can't pass all
// the vertices trough the same variable in a for loop).
	gluTessVertex(m_tessObj,vertData,vertData); 
}

void CGLTesselator::AddVertexArray(GLdouble arr[][3], int size)
{
	//ASSERT(arr!=NULL);
// pass the vertices to the tessellation object
	for(int ct=0;ct<size;ct++) gluTessVertex(m_tessObj,arr[ct],arr[ct]);
}

