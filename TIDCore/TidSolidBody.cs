using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace TidModel.Solid
{
	public class TidSolidBody
	{
		/// <summary>
		/// 将该实体定义克隆并转换至装配位置
		/// </summary>
		/// <param name="acs">该实体在装配环境下的装配坐标系</param>
		/// <returns>装配环境下的实体定义</returns>
		public TidSolidBody CloneToACS(CoordSystem acs)
		{
			return this;
		}
		/// <summary>
		/// 将实体由当前局部坐标系转换至装配环境下坐标值
		/// </summary>
		/// <param name="acs"></param>
		/// <returns></returns>
		public bool TransToAcs(CoordSystem acs)
		{
			return false;
		}
		//将实体从装配坐标系acsFrom移位到装配坐标系acsTo
		public bool TransToAcs(CoordSystem acsFrom,CoordSystem acsTo)
		{
			return false;
		}
		//virtual bool ReadFrom(BUFFER_IO* io,DWORD size);
		//void WriteTo(BUFFER_IO* io);
		//从buf中复制实体数据缓存
		//virtual bool CopyBuffer(char* buf,DWORD size);
		//外挂buf为实体数据缓存（外挂数据缓存）
		//virtual bool AttachBuffer(char* buf,DWORD size);
		public Coord3d[] VertexArr;
		//public TidEdgeLine[] edgeArr;
		public TidSolidFace[] rawFaceArr=null;
		//public TidTriFace[] triFaceArr;
	}
	public class TidSolidEdge{
		public const byte NURBS	 = 0x00;
		public const byte STRAIGHT= 0x01;
		public const byte ARCLINE = 0x02;
		public const byte ELLIPSE = 0x03;
		//边类型,NURBS;STRAIGHT;ARCLINE;ELLIPSE
		public byte EdgeType{
			get{return STRAIGHT;}
		}
		public uint LineStartId{
			get{ return 0;}
		}
		public uint LineEndId{
			get{ return 0;}
		}
		public Coord3d Center{
			get{ return null;}
		}
		public Coord3d WorkNorm{
			get{ return null;}
		}
		public Coord3d ColumnNorm{
			get{ return null;}
		}
	}
	public class LoopEdgeLine{
	}
	public class TidFaceLoop{
		public LoopEdgeLine[] LoopEdgeLineArr
		{
			get{return null;}
		}
		//bool GetLoopEdgeLineAt(CSolidBody* pBody,int i,f3dArcLine& line);
	};
	public class TidSolidFace{
		public uint MatColor;		// COLORREF可用于记录此面的特征信息(如材质等)
		public uint FaceId;	//用于标识多边形面链中的某一特定面
		public TidFaceLoop[] InnerLoopArr;
		public TidFaceLoop OutterLoop;
		public const byte MATERIAL_COLOR= 0;	//= 0;
		public const byte WORK_NORM = 5;
		public const byte BASICFACE_ID = 29;
		public const byte INNERLOOP_N = 33;
		public const byte INNERLOOP_INDEX_ADDR = 35;
		public Coord3d WorkNorm;			//多边形面的正法线方向
		public uint BasicFaceId;			//多边形面的正法线方向
	};
}
