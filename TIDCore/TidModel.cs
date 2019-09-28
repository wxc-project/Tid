using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using TidModel.Solid;

namespace TidModel
{
	public class TidTowerModel
	{
		public bool ReadTidFile(string tid_filename)
		{
			return false;
		}
		public bool InitTidBuffer(byte[] buffer)
		{
			return false;
		}
		public int[] GetAllHeightGroupIds()
		{
			return null;
		}
		public TidHeightGroup GetHeightGroup(int idHeightGroup)
		{
			return null;
		}
		public SteelMaterial[] GetSteelMaterials()
		{
			return null;
		}
		public TidBoltSeries[] GetBoltSeries()
		{
			return null;
		}
	}
	/// <summary>
	/// 三维空间中的矢量或位置点描述
	/// </summary>
	public class Coord3d
	{
        public double x, y, z;
        public Coord3d() { x = y = z = 0; }
        public Coord3d Set(double X, double Y, double Z)
        {
            x = X;
            y = Y;
            z = Z;
            return this;
        }
	}
	/// <summary>
	/// 坐标系定义
	/// </summary>
	public class CoordSystem
	{
		//坐标系的原点位置
		public Coord3d origin;
		//坐标系的X/Y/Z坐标轴, 三坐标轴遵循右手螺栓正交，且为单位化矢量
		public Coord3d axisX, axisY, axisZ;
        public CoordSystem()
        {
            axisX.x = axisY.y = axisZ.z = 1.0;
        }
        public double[] GetTransMatrixFromLocalCS()
        {
            return new double[16];
        }
        public double[] GetTransMatrixToLocalCS()
        {
            return new double[16];
        }
        public CoordSystem Clone()
        {
            CoordSystem cs = new CoordSystem();
            cs.origin.Set(origin.x, origin.y, origin.z);
            cs.axisX.Set(axisX.x, axisX.y, axisX.z);
            cs.axisY.Set(axisY.x, axisY.y, axisY.z);
            cs.axisZ.Set(axisZ.x, axisZ.y, axisZ.z);
            return cs;
        }
	}
	/// <summary>
	/// 钢材的材质
	/// </summary>
	public class SteelMaterial
	{
		private char briefsymbol;
		private string namecode;
		/// <summary>
		/// 材料简化字符标识
		/// </summary>
		public char BriefSymbol
		{
			set { briefsymbol=value; }
			get { return briefsymbol; }
		}
		/// <summary>
		/// 材料名称代号
		/// </summary>
		public string NameCode
		{
			set { namecode = value; }
			get { return namecode; }
		}
	}
	/// <summary>
	/// 该塔型涉及的螺栓规格系列
	/// </summary>
	public class TidBoltSeries
	{
		public string seriesName;
		public byte washerCount;
		public byte nutCount;
		public TidBoltSizeSpec[] boltSizeArr = null;
		public TidBoltNut[] boltNutArr = null;
	}
    public class TidBoltNut
    {
        private short diameter;
        private TidSolidBody solid;
        public TidSolidBody Solid
        {
            get { return solid; }
        }
    }
	public class TidBoltSizeSpec
	{
		public short diameter;	//螺栓直径
		public short lenValid;	//螺栓有效长
		public short lenNoneThread;//螺栓无扣长
		public double theoryWeight;//理论重量(kg)
		public string sizeDescript;//螺栓规格描述字符（含末尾0截止字符）。
		private TidSolidBody solid;
		public TidSolidBody Solid
		{
			get { return solid; }
		}
		private TidBoltNut nut;
		public TidSolidBody NutSolid
		{
			get
			{
				return nut.Solid;
			}
		}
	}
	public class TidHeightGroup
	{
		public int[] GetLegSerialArr()
		{
			return null;
		}
		public double GetLegHeightDifference(int legSerial)
		{
			return 0;
		}
		public int GetLegSerial(double heightDifference)
		{
			return 1;
		}
		/// <summary>
		/// 组装生成一个铁塔呼高实例
		/// </summary>
		/// <param name="legSerialQuad1">1号象限，常对应D腿</param>
		/// <param name="legSerialQuad2">2号象限，常对应A腿</param>
		/// <param name="legSerialQuad3">3号象限，常对应C腿</param>
		/// <param name="legSerialQuad4">4号象限，常对应B腿</param>
		/// <returns></returns>
		public TidTowerInstance AssembleTowerInstance(int legSerialQuad1, int legSerialQuad2, int legSerialQuad3, int legSerialQuad4)
		{
			return null;
		}
		public string Name
		{
			get { return "呼高１"; }
		}
		/// <summary>
		/// 呼高组标识序号
		/// </summary>
		public int SerialId
		{
			get { return 1; }
		}
	}
	public class TidPart
	{
		private byte ciType = 1;	//构件分类,1:角钢;2:螺栓;3:钢板;4:钢管;5:扁铁;6:槽钢;7:球壳
		public byte PartType { get { return ciType; } }
		//private ushort cbState;	//２Ｂ：构件状态属性
		/// <summary>
		/// 第16位：为１表示构件同时属于嵌入式部件和杆塔模型空间，即0x8000；
		/// 其余位暂保留备用。
		/// </summary>
		public bool IsEmbededBlockPart
		{
			get { return false; }
		}
		/// <summary>
		/// 表示是否为构件属制弯杆件中的子直线段
		/// 第15位：为１表示构件属制弯杆件中的子直线段，即0x4000；
		/// 注：制弯杆件中的子直线段实体不显示，亦不进行统计（已在制弯杆件中进行了显示统材），此数据仅用于辅助生成制弯杆件的加工工艺信息用。
		/// </summary>
		public bool IsPolySonRodPart
		{
			get { return false; }
		}
		/// <summary>
		/// 第14位：为１表示构件为多个直线段组成的制弯杆件，即0x2000；
		/// </summary>
		public bool IsPolyRodPart
		{
			get { return false; }
		}
		private SteelMaterial steelmat=null;
		/// <summary>
		/// 钢材的材质
		/// </summary>
		public SteelMaterial SteelMaterial
		{
			get { return steelmat; }
		}
		//４Ｂ：段号
		public string segStr;
		//４Ｂ：宽度(float)，一般用于存储角钢肢宽、槽钢及扁铁的宽度、钢管直径、球直径等。
		public int width;
		//４Ｂ：厚度(float)，一般用于存储角钢肢厚、槽钢及扁铁的厚度、钢管壁厚、球壁厚等。
		public float thick;
		//４Ｂ：高度(float)，一般用于存储不等肢角钢的另一肢厚或槽钢的高度等不常见参数。
		public float height;
		//２Ｂ：长度(unsigned short)
		public float length;
		//对于不常见的型钢类型，一般不能通过简单的宽度和厚度两个参数来描述其规格，此时可通过＜构件附加信息区＞中的＜规格＞字符串项来具体描述其规格类型，如对于槽钢类型“［16a”，其规格参数为：
		//宽度：63；
		//厚度：6.5；
		//高度：160；
		//附加信息区规格字符串：“［16a” 
		public string sizeSpec;
		//４Ｂ：单重(float)
		public float weight;
		//１Ｂ：功用类型
		public byte ciFuncType;
		/// <summary>
		/// 用B-rep模型描述的构件实体定义
		/// </summary>
		public TidSolidBody solid;
	}
	public class TidAssemblePart
	{
		public TidPart tidpart;
		public TidPart Part
		{
			get { return tidpart; }
		}
		/// <summary>
		/// 构件在模型中的装配坐标系
		/// </summary>
		public CoordSystem acs;
		public TidSolidBody TransToAssembleSolid()
		{
			return tidpart.solid;
		}
	}
	public class TidAssembleBolt
	{
		private TidBoltSizeSpec bolt = null;
		public TidBoltSizeSpec StdBolt
		{
			get { return bolt; }
		}
		/// <summary>
		/// 通厚长度
		/// </summary>
		public ushort thicknessThrough;
		private string grade;
		public string BoltGrade
		{
			get { return grade; }
		}
		/// <summary>
		/// 构件在模型环境中的装配坐标系
		/// </summary>
		public CoordSystem acs;
		/// <summary>
		/// 生成模型环境中的装配实体
		/// </summary>
		/// <returns></returns>
		public TidSolidBody CreateAssembleBoltSolid(CoordSystem cs)
		{
			if (cs == null || cs.Equals(acs))
				cs = acs.Clone();
			return bolt.Solid.CloneToACS(cs);
		}
		/// <summary>
		/// 转换至装配坐标系下的实体
		/// </summary>
		/// <param name="cs">螺栓实体装配坐标系</param>
		/// <returns></returns>
		public TidSolidBody TransToAssembleBoltNut(CoordSystem cs)
		{
			if (cs == null || cs.Equals(acs))
				cs = acs.Clone();
			cs.origin.x += thicknessThrough * cs.axisZ.x;
			cs.origin.y += thicknessThrough * cs.axisZ.y;
			cs.origin.z += thicknessThrough * cs.axisZ.z;
			return bolt.NutSolid.CloneToACS(cs);
		}
	}
	public class TidTowerInstance
	{
		private TidHeightGroup _heightGroup = null;
		public TidHeightGroup HeightGroup
		{
			get { return _heightGroup; }
		}
		public TidAssemblePart[] RetrieveAssembleParts()
		{
			return null;
		}
		public TidAssembleBolt[] RetrieveAssembleBolt()
		{
			return null;
		}
	}
}
