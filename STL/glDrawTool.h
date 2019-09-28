#ifndef _GL_DRAW_TOOL_H__
#define _GL_DRAW_TOOL_H__
/** CGLDispList
DESC:-this is an helper class which let you create "display list objects",
   use these objects to define the key elements in your scene (a disp.
   list is faster than the corresponding GL commands).
  -Through the class members functions you have total control on a
   single display list.
  -An isolated display list save OGL parameters before execution
   (so it's not affected by preceding transformations or settings).
*******/
class CGLDispList
{
friend class CGLEnabledView;
private:
	BOOL m_bIsolated;
	int m_glListId;
public:
	void DeleteList();
	CGLDispList();  // constructor
	~CGLDispList(); // destructor
	void StartDef(BOOL bImmediateExec=FALSE);// enclose a disp.list def.
	void EndDef();
	void Draw();// execute disp list GL commands 
	void SetIsolation(BOOL bValue) {m_bIsolated=bValue;}; // set isolation property
};

/** CGLTesselator
DESC:-this is a wrapper class which let you create "tesselator objects",
  use these objects to create concave or self intersecting polygons.
 -OGL tessellation objects converts a vertices list describing a convex
  or self-intersecting polygon in one or more GL primitives.Read the
  docs to understand the callbacks mechanism.
 -The callbacks have been defined as a simple and direct mapping to
  corresponding GL primitive (no per vertex color or texture information).
 -The callbacks have to be global functions.
 -A very simple garbage collection list has been implemented to manage new
  vertices create by combine callback.
 -Elaboration and drawing occur after EndDef().
*******/
#include "Buffer.h"
extern CBuffer trianglesBuffer;
class CGLTesselator
{
public:
	CGLTesselator();  // constructor
	~CGLTesselator(); // destructor
// properties functions
	void SetFilling(BOOL bFill=TRUE);
	BOOL GetFilling();
	void SetWindingRule(GLdouble which);
	GLdouble GetWindingRule();
// definition functions
	void StartDef();
	void TessNormal(GLdouble x,GLdouble y,GLdouble z);
	void AddVertexArray(GLdouble arr[][3],int size);
	void AddVertex(GLdouble vertData[3]);
	void ContourSeparator();
	void EndDef();// here occur drawing

private:
	GLUtesselator* m_tessObj;
};

#endif