#pragma once
#include "I3DS.h"
#include <stdio.h>
#include <vector>
#include <set>
#include <map>
#include <tchar.h>
#include <string>
#include <lib3ds/material.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/camera.h>
#include <lib3ds/light.h>
#include <lib3ds/file.h>
#include "SolidBody.h"
#include "ArrayList.h"
#include "gl/glu.h"

using namespace std;

struct SBoolKeyCloner { void Clone(Lib3dsBoolKey*& keyDst, const Lib3dsBoolKey* keySrc); };
struct SLin1KeyCloner { void Clone(Lib3dsLin1Key*& keyDst, const Lib3dsLin1Key* keySrc); };
struct SLin3KeyCloner { void Clone(Lib3dsLin3Key*& keyDst, const Lib3dsLin3Key* keySrc); };
struct SQuatKeyCloner { void Clone(Lib3dsQuatKey*& keyDst, const Lib3dsQuatKey* keySrc); };
struct SMorphKeyCloner { void Clone(Lib3dsMorphKey*& keyDst, const Lib3dsMorphKey* keySrc); };

//
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
//////////////////////////////////////////////////////////////////////////
//
class C3DSData : public I3DSData
{
private:
	vector<Lib3dsFile*> m_vSource;
	vector<Lib3dsMesh*> m_vMesh;
	vector<Lib3dsNode*> m_vNode;
	vector<Lib3dsMaterial*> m_vMaterial;
	//
	Lib3dsFile* m_3ds;
	int m_nName;
	int m_iNo;
public:
	C3DSData(int iNo);
	~C3DSData(void);
	//
	virtual int GetSerial(){return m_iNo;}
	virtual bool AddSolidPart(CSolidBody* pSolidBody,int nId,char* sSolidName,BOOL bTransPtMMtoM=FALSE,int nParentId=-1);
	virtual void Creat3DSFile(const char* sFilePath);
private:
	void CreatNewMaterial(float fRed,float fGreen,float fBlue,char* sName);
	bool BuildNode();
	void BuildMesh();
	void BuildMaterial();
	void FillName(char* strDst, const char* strSrc, size_t nMaxLen = 0);
	bool RectifyName(string& strName, set<string>& setName, size_t nMaxLen);
	void RectifyMaterialReference(const string& strNewName, const char* strOldName);
	void FillMaterial(Lib3dsMaterial* pMat, const Lib3dsMaterial* pSrc);
	void FillTexturemap(Lib3dsTextureMap& texMap, const Lib3dsTextureMap& src);
	void RectifyMeshReference(const string& strNewName, const char* strOldName);
	bool RectifyNodeId();
	Lib3dsWord RectifyNodeId(Lib3dsNode* pNode, Lib3dsWord nStep);
	void CloneNodeDataAmbient(Lib3dsAmbientData& dstAmbient, const Lib3dsAmbientData& srcAmbient);
	void CloneNodeDataObject(Lib3dsObjectData& dstObject, const Lib3dsObjectData& srcObject);
	void CloneNodeDataCamera(Lib3dsCameraData& dstCamera, const Lib3dsCameraData& srcCamera);
	void CloneNodeDataTarget(Lib3dsTargetData& dstTarget, const Lib3dsTargetData& srcTarget);
	void CloneNodeDataLight(Lib3dsLightData& dstLight, const Lib3dsLightData& srcLight);
	void CloneNodeDataSpot(Lib3dsSpotData& dstSpot, const Lib3dsSpotData& srcSpot);
	void CloneNodeData(Lib3dsNodeData& data, const Lib3dsNodeData& srcData);
	void CloneNode(const Lib3dsNode* pSrcNode);
	template <typename TTrack, typename TKey, typename TCloner>
	void CloneTrack(TTrack& dstTrack, const TTrack& srcTrack)
	{
		dstTrack.flags = srcTrack.flags;
		dstTrack.keyL = NULL;
		const TKey *keySrc = srcTrack.keyL;
		TKey *keyDst = NULL, *keyPrev = NULL;

		TCloner cloner;
		while (keySrc)
		{
			cloner.Clone(keyDst, keySrc);

			if (!dstTrack.keyL)
				dstTrack.keyL = keyDst;

			if (keyPrev)
				keyPrev->next = keyDst;

			keyPrev = keyDst;
			keySrc = keySrc->next;
		}
	}
};
