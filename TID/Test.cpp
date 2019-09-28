CBuffer trianglesBuffer;
bool SplitToBasicFacets(CSolidBody* pSrcBody, CSolidBody* pDstBody)
{
	if (pSrcBody == NULL)
		return false;
	//迁移原实体定义内存同时腾出基于三角面片的基本实体显示面数据区空间
	int face_n = pSrcBody->PolyFaceNum();
	int edge_n = pSrcBody->KeyEdgeLineNum();
	if (face_n == 0 || edge_n == 0)
		return false;
	static char _facets_buff_pool[0x100000];
	CSolidBodyBuffer solidbuf(_facets_buff_pool, 0x100000);
	solidbuf.Write(pSrcBody->BufferPtr(), 33);
	solidbuf.BasicFaceNumber = face_n;	//写入基本面片数=原始多边形面数
	DWORD dwIndexBufSize = (edge_n + face_n) * 4;
	solidbuf.WriteAt(45, pSrcBody->BufferPtr() + 45, dwIndexBufSize);
	solidbuf.BasicFaceIndexStartAddr = 45 + dwIndexBufSize;
	solidbuf.SeekToEnd();	//BasicFaceIndexStartAddr＝赋值会变当前存储指针位置
	solidbuf.Write(NULL, face_n * 4);	//@45+dwIndexBufSize	实体基本面片索引区暂写入相应的空字节占位
	DWORD dwDataBufSize = pSrcBody->BufferLength() - solidbuf.VertexDataStartAddr;
	if (pSrcBody->BasicGLFaceNum() > 0)	//只复制实体原始定义数据区域，忽略原有的基本面片数据区
		dwDataBufSize = pSrcBody->BasicFaceDataStartAddr() - solidbuf.VertexDataStartAddr;
	long iNewVertexDataStartAddr = solidbuf.GetCursorPosition();
	long iOldVertexDataStartAddr = solidbuf.VertexDataStartAddr;
	solidbuf.VertexDataStartAddr = iNewVertexDataStartAddr;
	solidbuf.EdgeDataStartAddr = solidbuf.EdgeDataStartAddr + 4 * face_n;
	solidbuf.RawFaceDataStartAddr = solidbuf.RawFaceDataStartAddr + 4 * face_n;
	solidbuf.SeekToEnd();
	solidbuf.Write(pSrcBody->BufferPtr() + iOldVertexDataStartAddr, dwDataBufSize);	//写入原实体定义的数据区内存
	//计算因增加基本面片索引记录数带来的后续地址位移值
	int addr_offset = (face_n - pSrcBody->BasicGLFaceNum()) * 4;
	if (addr_offset != 0)
	{	//重新移位各项基本图元索引所指向的内存地址偏移
		DWORD* RawFaceAddr = (DWORD*)(solidbuf.GetBufferPtr() + solidbuf.RawFaceIndexStartAddr);
		for (int i = 0; i < face_n; i++)
			*(RawFaceAddr + i) += addr_offset;
		DWORD* RawEdgeAddr = (DWORD*)(solidbuf.GetBufferPtr() + solidbuf.EdgeIndexStartAddr);
		for (int i = 0; i < face_n; i++)
			*(RawEdgeAddr + i) += addr_offset;
	}
	if (solidbuf.BasicFaceDataStartAddr == 0)	//以前基本面片数为空
		solidbuf.BasicFaceDataStartAddr = solidbuf.GetCursorPosition();
	else	//重写原基本面片数据区
		solidbuf.BasicFaceDataStartAddr = solidbuf.BasicFaceDataStartAddr + 4 * face_n;
	//写入三角面数据
	double alpha = 0.6;	//考虑到显示效果的经验系数
	int i = 0, j = 0, n = 0;
	CRawSolidFace rawface;
	CXhSimpleList<CGLFace> listFacets;
	CXhSimpleList<GEPOINT> listVertices;
	for (int indexFace = 0; indexFace < face_n; indexFace++)
	{
		listFacets.DeleteList();
		listVertices.DeleteList();
		if (!pSrcBody->GetPolyFaceAt(indexFace, rawface))
		{
			WriteToSolidBuffer(solidbuf, indexFace, listFacets, 0, GEPOINT(0, 0, 0));
			continue;
		}
		/*对于一个复杂面，肯定有一个外环，此外还可能有无数个内环，它们的法线方向是相同的
		  相对法线来说，外环应该逆时针方向，而内环则应该是顺时针方向，
		*/
		f3dArcLine edgeLine[4];
		GEPOINT poly_norm = rawface.WorkNorm;
		CFaceLoop outerloop = rawface.GetOutterLoop();
		if (outerloop.LoopEdgeLineNum() == 3)
		{
			outerloop.GetLoopEdgeLineAt(pSrcBody, 0, edgeLine[0]);
			outerloop.GetLoopEdgeLineAt(pSrcBody, 1, edgeLine[1]);
			outerloop.GetLoopEdgeLineAt(pSrcBody, 2, edgeLine[2]);
			f3dPoint pt_arr[3];
			if (edgeLine[0].ID & 0x80000000)	//负边
				pt_arr[0] = edgeLine[0].End();
			else
				pt_arr[0] = edgeLine[0].Start();
			if (edgeLine[1].ID & 0x80000000)	//负边
				pt_arr[1] = edgeLine[1].End();
			else
				pt_arr[1] = edgeLine[1].Start();
			if (edgeLine[2].ID & 0x80000000)	//负边
				pt_arr[2] = edgeLine[2].End();
			else
				pt_arr[2] = edgeLine[2].Start();
			poly_norm = rawface.WorkNorm;
			if (poly_norm.IsZero())
			{
				f3dPoint vec1 = pt_arr[1] - pt_arr[0];
				f3dPoint vec2 = pt_arr[2] - pt_arr[1];
				poly_norm = vec1 ^ vec2;
			}
			if (!Standize(poly_norm))
			{
				WriteToSolidBuffer(solidbuf, indexFace, listFacets, rawface.MatColor(), poly_norm);
				continue;//return false;
			}
			//设置三角面法线、光照颜色及顶点坐标等信息
			CGLFace *pGLFace = listFacets.AttachObject();
			pGLFace->nx = poly_norm.x;
			pGLFace->ny = poly_norm.y;
			pGLFace->nz = poly_norm.z;
			pGLFace->red = GetRValue(rawface.MatColor()) / 255.0f;
			pGLFace->green = GetGValue(rawface.MatColor()) / 255.0f;
			pGLFace->blue = GetBValue(rawface.MatColor()) / 255.0f;
			pGLFace->alpha = (GLfloat)alpha;
			pGLFace->header.uVertexNum = 3;
			pGLFace->header.clr_norm = 0x03;	//默认变换颜色及法线
			pGLFace->m_pVertexCoordArr = new GLdouble[9];
			for (j = 0; j < 3; j++)
			{
				pGLFace->m_pVertexCoordArr[3 * j] = pt_arr[j].x;
				pGLFace->m_pVertexCoordArr[3 * j + 1] = pt_arr[j].y;
				pGLFace->m_pVertexCoordArr[3 * j + 2] = pt_arr[j].z;
			}
			WriteToSolidBuffer(solidbuf, indexFace, listFacets, rawface.MatColor(), poly_norm);
			continue;
		}
		if (outerloop.LoopEdgeLineNum() == 4)
		{
			outerloop.GetLoopEdgeLineAt(pSrcBody, 0, edgeLine[0]);
			outerloop.GetLoopEdgeLineAt(pSrcBody, 1, edgeLine[1]);
			outerloop.GetLoopEdgeLineAt(pSrcBody, 2, edgeLine[2]);
			outerloop.GetLoopEdgeLineAt(pSrcBody, 3, edgeLine[3]);
			if (rawface.WorkNorm.IsZero())
			{
				f3dPoint vec1 = edgeLine[0].End() - edgeLine[0].Start();
				f3dPoint vec2 = edgeLine[1].End() - edgeLine[1].Start();
				poly_norm = vec1 ^ vec2;
				int sign1 = 1, sign2 = 1;
				if (edgeLine[0].ID & 0x80000000)
					sign1 = -1;
				if (edgeLine[1].ID & 0x80000000)
					sign2 = -1;
				if (sign1 + sign2 == 0)	//异号边线
					poly_norm *= -1;
			}
			else
				poly_norm = rawface.WorkNorm;
			if (!Standize(poly_norm))
			{
				if (edgeLine[0].SectorAngle() > 0)
				{
					poly_norm = edgeLine[0].WorkNorm();
					if (edgeLine[0].ID & 0x80000000)
						poly_norm *= -1;
				}
				else if (edgeLine[1].SectorAngle() > 0)
				{
					poly_norm = edgeLine[1].WorkNorm();
					if (edgeLine[1].ID & 0x80000000)
						poly_norm *= -1;
				}
				//TODO: 未理解原意，可能是担心共线边出现
				//edgeLine[0]=NULL;
			}
			if (edgeLine[0].SectorAngle() > 0 && edgeLine[1].SectorAngle() == 0 && edgeLine[2].SectorAngle() > 0 && edgeLine[3].SectorAngle() == 0
				&& fabs(edgeLine[0].WorkNorm()*poly_norm) < EPS_COS)
			{
				n = max(edgeLine[0].m_uDisplaySlices, edgeLine[2].m_uDisplaySlices);
				if (n == 0)
				{
					int n1 = CalArcResolution(edgeLine[0].Radius(), edgeLine[0].SectorAngle());
					int n2 = CalArcResolution(edgeLine[2].Radius(), edgeLine[2].SectorAngle());
					n = max(n1, n2);
				}
				n = min(n, 200);
				GEPOINT vertex_arr1[200], vertex_arr2[200];
				GetArcSimuPolyVertex(&edgeLine[0], vertex_arr1, n);
				GetArcSimuPolyVertex(&edgeLine[2], vertex_arr2, n);
				// 			double part_angle1=edgeLine[0]->SectorAngle()/n;
				// 			double part_angle2=edgeLine[2]->SectorAngle()/n;
				// 			double posAngle;

				for (i = 0; i < n; i++)
				{
					f3dPoint pt_arr[3];
					//１号圆弧中间点
					//posAngle=edgeLine[0]->SectorAngle()-i*part_angle1;
					pt_arr[0] = vertex_arr1[n - i];//edgeLine[0]->PositionInAngle(posAngle);
					//２号圆弧中间点
					//posAngle=i*part_angle2;
					pt_arr[1] = vertex_arr2[i];//edgeLine[2]->PositionInAngle(posAngle);
					//２号圆弧中间点
					//posAngle=(i+1)*part_angle2;
					pt_arr[2] = vertex_arr2[i + 1];//edgeLine[2]->PositionInAngle(posAngle);
					f3dPoint axis_x = pt_arr[1] - pt_arr[0];
					f3dPoint axis_y = pt_arr[2] - pt_arr[0];
					poly_norm = axis_x ^ axis_y;
					Standize(poly_norm);
					//设置三角面法线、光照颜色及顶点坐标等信息
					CGLFace *pGLFace = listFacets.AttachObject();
					pGLFace->nx = poly_norm.x;
					pGLFace->ny = poly_norm.y;
					pGLFace->nz = poly_norm.z;
					pGLFace->red = GetRValue(rawface.MatColor()) / 255.0f;
					pGLFace->green = GetGValue(rawface.MatColor()) / 255.0f;
					pGLFace->blue = GetBValue(rawface.MatColor()) / 255.0f;
					pGLFace->alpha = (GLfloat)alpha;
					pGLFace->header.uVertexNum = 3;
					pGLFace->m_pVertexCoordArr = new GLdouble[9];
					for (j = 0; j < 3; j++)
					{
						pGLFace->m_pVertexCoordArr[3 * j] = pt_arr[j].x;
						pGLFace->m_pVertexCoordArr[3 * j + 1] = pt_arr[j].y;
						pGLFace->m_pVertexCoordArr[3 * j + 2] = pt_arr[j].z;
					}

					//２号圆弧中间点
					//posAngle=(i+1)*part_angle2;
					pt_arr[0] = vertex_arr2[i + 1];//edgeLine[2]->PositionInAngle(posAngle);
					//１号圆弧中间点
					//posAngle=edgeLine[0]->SectorAngle()-(i+1)*part_angle1;
					pt_arr[1] = vertex_arr1[n - i - 1];//edgeLine[0]->PositionInAngle(posAngle);
					//１号圆弧中间点
					//posAngle=edgeLine[0]->SectorAngle()-i*part_angle1;
					pt_arr[2] = vertex_arr1[n - i];//edgeLine[0]->PositionInAngle(posAngle);
					axis_x = pt_arr[1] - pt_arr[0];
					axis_y = pt_arr[2] - pt_arr[0];
					poly_norm = axis_x ^ axis_y;
					Standize(poly_norm);
					//设置三角面法线、光照颜色及顶点坐标等信息
					pGLFace = listFacets.AttachObject();
					pGLFace->nx = poly_norm.x;
					pGLFace->ny = poly_norm.y;
					pGLFace->nz = poly_norm.z;
					pGLFace->red = GetRValue(rawface.MatColor()) / 255.0f;
					pGLFace->green = GetGValue(rawface.MatColor()) / 255.0f;
					pGLFace->blue = GetBValue(rawface.MatColor()) / 255.0f;
					pGLFace->alpha = (GLfloat)alpha;
					pGLFace->header.uVertexNum = 3;
					pGLFace->m_pVertexCoordArr = new GLdouble[9];
					for (j = 0; j < 3; j++)
					{
						pGLFace->m_pVertexCoordArr[3 * j] = pt_arr[j].x;
						pGLFace->m_pVertexCoordArr[3 * j + 1] = pt_arr[j].y;
						pGLFace->m_pVertexCoordArr[3 * j + 2] = pt_arr[j].z;
					}
				}
				WriteToSolidBuffer(solidbuf, indexFace, listFacets, rawface.MatColor(), poly_norm);
				continue;
			}
			else if (edgeLine[0].SectorAngle() == 0 && edgeLine[1].SectorAngle() > 0 && edgeLine[2].SectorAngle() == 0 && edgeLine[3].SectorAngle() > 0
				&& fabs(edgeLine[1].WorkNorm()*poly_norm) < EPS_COS)
			{
				n = max(edgeLine[1].m_uDisplaySlices, edgeLine[3].m_uDisplaySlices);
				if (n == 0)
				{
					int n1 = CalArcResolution(edgeLine[1].Radius(), edgeLine[1].SectorAngle());
					int n2 = CalArcResolution(edgeLine[3].Radius(), edgeLine[3].SectorAngle());
					n = max(n1, n2);
				}
				n = min(n, 200);
				GEPOINT vertex_arr1[200], vertex_arr2[200];
				GetArcSimuPolyVertex(&edgeLine[1], vertex_arr1, n);
				GetArcSimuPolyVertex(&edgeLine[3], vertex_arr2, n);
				// 			double part_angle1=edgeLine[1]->SectorAngle()/n;
				// 			double part_angle2=edgeLine[3]->SectorAngle()/n;
				// 			double posAngle;
				glEnable(GL_NORMALIZE);
				glEnable(GL_AUTO_NORMAL);
				for (i = 0; i < n; i++)
				{
					f3dPoint pt_arr[3];
					//１号圆弧中间点
					//posAngle=edgeLine[1]->SectorAngle()-i*part_angle1;
					pt_arr[0] = vertex_arr1[n - i];//edgeLine[1]->PositionInAngle(posAngle);
					//２号圆弧中间点
					//posAngle=i*part_angle2;
					pt_arr[1] = vertex_arr2[i];//edgeLine[3]->PositionInAngle(posAngle);
					//２号圆弧中间点
					//posAngle=(i+1)*part_angle2;
					pt_arr[2] = vertex_arr2[i + 1];//edgeLine[3]->PositionInAngle(posAngle);
					f3dPoint axis_x = pt_arr[1] - pt_arr[0];
					f3dPoint axis_y = pt_arr[2] - pt_arr[0];
					poly_norm = axis_x ^ axis_y;
					Standize(poly_norm);
					//设置三角面法线、光照颜色及顶点坐标等信息
					CGLFace *pGLFace = listFacets.AttachObject();
					pGLFace->nx = poly_norm.x;
					pGLFace->ny = poly_norm.y;
					pGLFace->nz = poly_norm.z;
					pGLFace->red = GetRValue(rawface.MatColor()) / 255.0f;
					pGLFace->green = GetGValue(rawface.MatColor()) / 255.0f;
					pGLFace->blue = GetBValue(rawface.MatColor()) / 255.0f;
					pGLFace->alpha = (GLfloat)alpha;
					pGLFace->header.uVertexNum = 3;
					pGLFace->m_pVertexCoordArr = new GLdouble[9];
					for (j = 0; j < 3; j++)
					{
						pGLFace->m_pVertexCoordArr[3 * j] = pt_arr[j].x;
						pGLFace->m_pVertexCoordArr[3 * j + 1] = pt_arr[j].y;
						pGLFace->m_pVertexCoordArr[3 * j + 2] = pt_arr[j].z;
					}

					//２号圆弧中间点
					//posAngle=(i+1)*part_angle2;
					pt_arr[0] = vertex_arr2[i + 1];//edgeLine[3]->PositionInAngle(posAngle);
					//１号圆弧中间点
					//posAngle=edgeLine[1]->SectorAngle()-(i+1)*part_angle1;
					pt_arr[1] = vertex_arr1[n - i - 1];//edgeLine[1]->PositionInAngle(posAngle);
					//１号圆弧中间点
					//posAngle=edgeLine[1]->SectorAngle()-i*part_angle1;
					pt_arr[2] = vertex_arr1[n - i];//edgeLine[1]->PositionInAngle(posAngle);
					axis_x = pt_arr[1] - pt_arr[0];
					axis_y = pt_arr[2] - pt_arr[0];
					poly_norm = axis_x ^ axis_y;
					Standize(poly_norm);
					//设置三角面法线、光照颜色及顶点坐标等信息
					pGLFace = listFacets.AttachObject();
					pGLFace->nx = poly_norm.x;
					pGLFace->ny = poly_norm.y;
					pGLFace->nz = poly_norm.z;
					pGLFace->red = GetRValue(rawface.MatColor()) / 255.0f;
					pGLFace->green = GetGValue(rawface.MatColor()) / 255.0f;
					pGLFace->blue = GetBValue(rawface.MatColor()) / 255.0f;
					pGLFace->alpha = (GLfloat)alpha;
					pGLFace->header.uVertexNum = 3;
					pGLFace->m_pVertexCoordArr = new GLdouble[9];
					for (j = 0; j < 3; j++)
					{
						pGLFace->m_pVertexCoordArr[3 * j] = pt_arr[j].x;
						pGLFace->m_pVertexCoordArr[3 * j + 1] = pt_arr[j].y;
						pGLFace->m_pVertexCoordArr[3 * j + 2] = pt_arr[j].z;
					}
				}
				WriteToSolidBuffer(solidbuf, indexFace, listFacets, rawface.MatColor(), poly_norm);
				continue;
			}
		}
		CGLTesselator t;
		t.SetFilling(TRUE);
		t.SetWindingRule(GLU_TESS_WINDING_ODD);
		if (poly_norm.IsZero())
			poly_norm = GetPolyFaceWorkNorm(pSrcBody, rawface);
		t.StartDef();
		t.TessNormal(poly_norm.x, poly_norm.y, poly_norm.z);
		//第一个为外环（参见B-rep模型）
		int ei = 0, edge_n = outerloop.LoopEdgeLineNum();
		//for(pLine=pFace->outer_edge.GetFirst();pLine!=NULL;pLine=pFace->outer_edge.GetNext())
		f3dArcLine line;
		f3dPoint vertice;
		for (ei = 0; ei < edge_n; ei++)
		{
			outerloop.GetLoopEdgeLineAt(pSrcBody, ei, line);
			if (line.SectorAngle() == 0)
			{
				if (line.Start() == line.End())
					continue;
				if (line.ID & 0x80000000)
					vertice = line.End();
				else
					vertice = line.Start();
				listVertices.AttachObject(vertice);
			}
			else
			{
				if (line.m_uDisplaySlices > 0)
					n = line.m_uDisplaySlices;
				else
					n = CalArcResolution(line.Radius(), line.SectorAngle());
				double piece_angle = line.SectorAngle() / n;
				for (i = 0; i < n; i++)
				{
					if (line.ID & 0x80000000)
					{
						if (i == 0)
							vertice = line.End();
						else
							vertice = line.PositionInAngle((n - i - 1)*piece_angle);
					}
					else
					{
						if (i == 0)
							vertice = line.Start();
						else
							vertice = line.PositionInAngle(i*piece_angle);
					}
					listVertices.AttachObject(vertice);
				}
			}
		}
		for (GEPOINT *pp = listVertices.EnumObjectFirst(); pp != NULL; pp = listVertices.EnumObjectNext())
			t.AddVertex(*pp);
		//第二个为内环
		//for(pLoop=pFace->inner_loop.GetFirst();pLoop!=NULL;pLoop=pFace->inner_loop.GetNext())
		for (int loopi = 0; loopi < rawface.InnerLoopNum(); loopi++)
		{
			CFaceLoop innerloop = rawface.GetInnerLoopAt(loopi);
			t.ContourSeparator();	//环边界区分
			edge_n = innerloop.LoopEdgeLineNum();
			//for(pLine=pLoop->loop->GetFirst();pLine!=NULL;pLine=pLoop->loop->GetNext())
			for (ei = 0; ei < edge_n; ei++)
			{
				innerloop.GetLoopEdgeLineAt(pSrcBody, ei, line);
				if (line.SectorAngle() == 0)
				{
					vertice = line.Start();
					GEPOINT *pp = listVertices.AttachObject(vertice);
					t.AddVertex(*pp);
				}
				else
				{
					if (line.m_uDisplaySlices > 0)
						n = line.m_uDisplaySlices;
					else
						n = CalArcResolution(line.Radius(), line.SectorAngle());
					double piece_angle = line.SectorAngle() / n;
					for (j = 0; j < n; j++)
					{
						if (j == 0)
							vertice = line.Start();
						else
							vertice = line.PositionInAngle(j*piece_angle);
						GEPOINT *pp = listVertices.AttachObject(vertice);
						t.AddVertex(*pp);
					}
				}
			}
		}
		t.EndDef();
		trianglesBuffer.SeekPosition(0);
		while (trianglesBuffer.GetRemnantSize() > 0)
		{
			//设置三角面法线、光照颜色及顶点坐标等信息
			CGLFace *pGLFace = listFacets.AttachObject();
			pGLFace->nx = poly_norm.x;
			pGLFace->ny = poly_norm.y;
			pGLFace->nz = poly_norm.z;
			pGLFace->red = GetRValue(rawface.MatColor()) / 255.0f;
			pGLFace->green = GetGValue(rawface.MatColor()) / 255.0f;
			pGLFace->blue = GetBValue(rawface.MatColor()) / 255.0f;
			pGLFace->alpha = (GLfloat)alpha;
			CGLFace *pPrevGLFace = listFacets.EnumObjectTail();
			pGLFace->header.clr_norm = 0x03;	//默认变换颜色及法线
			if (pPrevGLFace != NULL)
			{
				if (pPrevGLFace->red == pGLFace->red&&pPrevGLFace->green == pGLFace->green&&
					pPrevGLFace->blue == pGLFace->blue&&pPrevGLFace->alpha == pGLFace->alpha)
					pGLFace->header.clr_norm &= 0x02;
				if (pPrevGLFace->nx == pGLFace->nx&&pPrevGLFace->ny == pGLFace->ny&&pPrevGLFace->nz == pGLFace->nz)
					pGLFace->header.clr_norm &= 0x01;
			}
			trianglesBuffer.ReadByte(&pGLFace->header.mode);
			trianglesBuffer.ReadWord(&pGLFace->header.uVertexNum);
			pGLFace->m_pVertexCoordArr = new GLdouble[pGLFace->header.uVertexNum * 3];
			trianglesBuffer.Read(pGLFace->m_pVertexCoordArr, pGLFace->header.uVertexNum * 24);
		}
		WriteToSolidBuffer(solidbuf, indexFace, listFacets, rawface.MatColor(), poly_norm);
	}
	if (pDstBody)
		pDstBody->CopyBuffer(solidbuf.GetBufferPtr(), solidbuf.GetLength(), false);
	return true;
}