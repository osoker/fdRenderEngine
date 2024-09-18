//===================================================================
/**\file  
 *  Filename:   FindPathMap.h
 *  Desc:       在地图中寻路
 *		        给出起止点,返回下一步.
 *		        主函数：CFindPathMap::FindPathMap()
 *      
 *  His:      xlq create @  2007-5-16   16:17
 */
//===================================================================

#ifndef _def_FindPathMap
#define _def_FindPathMap

#define MAX_MAP_POINT_CNT	1280*1280		//地图上最
#define MAPBAR		0						//地图上的障碍物标记
#define MAPBOO		1						//地图上的空点标记
#define MAXCLINECOUNT		1280*5			//填充过程中的最大周长
#define BOXPOINT			1500			//每一大格的精细分辨度(1500mm)
#define BOXBORDER			15       //边缘留空，避免正好踏在边角上，更好看。

#define MAX_STEPLEN			BOXPOINT		//步长，mm
#define FINDPATH_BOXCOUNT	8				//每次最多分析的格子数


#define LIMIT_AREA			24	//! 限制区(格子)


#define MAKEFOURCC32(ch0, ch1, ch2, ch3)                \
	((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
	((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

#define PERMAP_SIZE 128


//! 定义某一个位置点是否可以移动
#define MAP_CAN_MOVE  0x1
#define MAP_NOT_MOVE  0x0

//! 地图每一个碰撞格子细分的size.
#define	_MAP_FINESIZE		1500


struct TPoint {
	int	x;
	int	y;
};

struct tagMAPFILEHEADER;

#define PATH_TOLERANCE		4

//! 一个路经点最长的短离
# define MAX_WAYLENGTH  35.0f



//! River added @ 2008-11-11:把长于最长路经点的坐标分割成短一些的路经点。
void divide_wayPoint( osVec2D& _src,osVec2D* _arrayWayPoint,int& _num,int _maxWayPoint,float _currSpeed );

//! 服务器用到的碰撞信息的文件头
typedef struct tagMAPFILEHEADER
{
	char	szDesc[20];
	short	shVer;
	short	shWidth;
	short	shHeight;
	char	cEventFileIdx;
	char	cFogColor;
}MAPFILEHEADER, *LPMAPFILEHEADER;


class CFindPathMap
{
public:
	CFindPathMap();
	~CFindPathMap();

	/**brief
	*寻路算法:  (只寻找到目的点的路径上第一个可以直线到达的坐标)
	*\param	_pMapbuf	地图碰撞信息,字节数组 0表示空位 1表示障碍
	*\param	_nMapW		地图宽度
	*\param	_nMapH		地图高度
	*\param	_nX1		源坐标(精确到mm)
	*\param	_nY1		
	*\param	_nX2		目的坐标(精确到mm)
	*\param	_nY2		
	*\param	_nX			返回的坐标(精确到mm)
	*\param	_nY			
	*\return	bool		是否可以到达(可以到达,返回的坐标才有效)
	*/
	//bool FindPathMap(char * _pMapBuf, int _nMapW, int _nMapH,
	//	int _nX1, int _nY1, int _nX2, int _nY2, 
	//	int &_nX, int &_nY);
	bool FindPathMap( int _nX1, int _nY1, int _nX2, int _nY2, 
		int &_nX, int &_nY);

	//返回在键盘8方向行走控制之下的下一步精确坐标（可侧滑）    
	//输入、输出与FindPathMap相同。 要求：_nX2,_nY2必须与_nX1,_nY1只差1格	
	//bool GetNextPoint(char * _pMapBuf, int _nMapW, int _nMapH,
	//	int _nX1, int _nY1, int _nX2, int _nY2, 
	//	int &_nX, int &_nY);
	bool GetNextPoint( int _nX1, int _nY1, int _nX2, int _nY2, 
		int &_nX, int &_nY);
	osVec2D get_mapSize();
	
public:

	//地图buf
	char* m_pMapBuf;

	//地图大小
	int m_MapW,m_MapH;
	//每步的跨度 以mm为单位
	int m_dlen;
	//碰撞存储的时间
	DWORD m_timeData;

	//! 保存文件头
	tagMAPFILEHEADER*  m_sHeader;

	//---------------返回值,以下返回公开，便于外部使用-------------------
	int			m_pathcount;//pathlist的点的个数
	TPoint		m_pathlist[MAXCLINECOUNT];
	TPoint		m_NextPoint;//返回下一阶段精确坐标，以mm为单位
	TPoint		m_FirstBox;//下一阶段目标点，以box为单位
protected:
	//----------以下是每次寻路都用的临时变量
	//此次原始坐标(以mm为单位)
	int			m_x1,m_y1,m_x2,m_y2;
	//此次寻路以格子坐标为单位的大坐标
	int			m_bx1,m_by1,m_bx2,m_by2;
	//本算法要用到的地图缓冲
	unsigned char m_maptmp[MAX_MAP_POINT_CNT];
	int			m_Cline1Count;
	TPoint		m_Cline1[MAXCLINECOUNT];
	int			m_Cline2Count;
	TPoint		m_Cline2[MAXCLINECOUNT];
	int			m_LineCount;//直线上的点的总数
	TPoint		m_LinePoint[MAXCLINECOUNT];
protected:
	//----------以下为内部使用的一些工具函数--------------
	//获取长距离路径
	bool GetPathList();
	//从pathlist中获取第一个在直线上的坐标
	void GetFirstPoint();
	//返回x1,y1到x2,y2的精确到毫米的坐标tx,ty，步长为_dlen (mm) ,其中x1,y1,x2,y2均指mm坐标
	bool GetStepPoint(int _x1,int _y1,int _x2,int _y2,int &_tx,int &_ty);
	//获得从x1,y1到x2,y2直线上的各个点
	int GetLine(TPoint _arrLine[], int _nX1, int _nY1, int _nX2, int _nY2);
	//检测某直线上是否有障碍物
	bool ChkBarLine(char* _pMapBuf, int _nMapH, int _nMapW, int _nX1, int _nY1, int _nX2, int _nY2);
	//将直线的方向近似到8个方向
	int GetFwordNear(int x1,int y1,int x2,int y2);
	//在自动走路过程中，可以暂存m_FirstBox,一次计算，多次调用GetStepPointEx函数，获得最新m_NextPoint。
	//返回x1,y1到x2,y2的精确到毫米的坐标tx,ty，步长为_dlen (mm) 
	/**\brief
	* 本函数根据当前的阶段目标m_FirstBox，求出下一步的具体坐标m_NextPoint (精度mm)	
	* \param   x1,y1,指当前点mm坐标
	* \param   m_FirstBox　这个在函数内直接使用。
	* \param   
	* \return  m_NextPoint 返回值保存在该属性中
	*/
	bool GetStepPointEx(int _x1,int _y1);

public:
	//! 载入服务器地图
	bool LoadMapInfo( const char* _pMapFile );	
	
	//! 存储服务器地图
	bool saveMapInfo( const char* _filename );


	//! 判断一点是否能通过
	bool CanMove( const osVec3D& _pPos );
	bool CanMove( const osVec2D& _pPos );
	bool CanMove( int _nX,int _nY );
	bool CanMoveGrid( int _nX, int _nY ); //格子坐标
	bool CHKBAR( int x, int y );

	//! 检查一条直线上是否有障碍物
	bool CheckLine(  int _nBeginX, int _nBeginY , int _nTargetX , int _nTargetY,bool _accu = false );
	bool CheckLine( osVec3D *_pStartPos, osVec3D *_pEndPos );

	//! 
	void CalcNextPoint( int _wSx, int _wSy, int _wDx, int _wDy, int _wSpace, int &_wX, int &_wY );

	/**\brief
	* 根据指定方向寻找直线最远的坐标 by sisun @ 2007-06-25
	* \param    _sStartPos: 起始坐标
	* \param	_sDir:		移动方向
	*/
	void FindLinePath( osVec2D& _sStartPos, osVec2D _sDir );

	/** \brief
	 * 根据起始点和终点的方向，计算离终点最近的可通过的坐标点。 by river@ 2008-11-10:
	 *
	 * 参数使用与CalcNextPoint函数相同，引用值返回细分(mm单位)格子的坐标。
	 */
	bool FindNearestMovePt( int _wSx, int _wSy, int _wDx, int _wDy,int &_wX, int &_wY );


	/** \brief
	 *
	 * \param osVec2D& _src 寻路开始点的位置。
	 * \param osVec2D& _dest 寻路结束点的位置。
	 * \param osVec2D* _arrayWayPoint 寻路返回的关键路点
	 * \param int& _num          寻路返回的关键路点位置.
	 * \param int _maxWayPoint   传入的数组,最多可以返回的路点位置
	 * \param _currSpeed		当前人物的速度
	 */
	bool ComputeWayPoint(osVec2D& _src,osVec2D& _dest,osVec2D* _arrayWayPoint,int& _num,int _maxWayPoint,float _currSpeed);

	/*! \brief 

		优化NoObstacle的算法描述：
		1: 算出起始点所在格子，算出结束点所在的格子。
		2: 从开始点的格子算起，每次相交一个格子，得到此格子是否有碰撞信息。
		3: 优化：根据线段与X轴的角度，每次相交，只会跟格子的两个边相交，这样只需要
		处理当前的线段是否跟另外的哪边线段相交。
		4: 根据起始点与格子的哪边线段相交，得到射线下一个到达的格子....循环当前的算
		法，直接到达最终的格子。

		如果中间没有格子有碰撞信息，则返回true,否则返回false.

	*/
	BOOL NoObstacle(float x0,float y0,float x1, float y1);

	//! 得到此次地图存储碰撞的时间值
	DWORD GetMapCollTime(void)const {return m_timeData;}
	

	//! get the collision info
	char* GetCollisionInfo(void){return m_pMapBuf;}

};

/**\brief
* 根据相似三角形求指定距离的一个合法点
* \param    
* \return   
*/
void CalcPoint(int _nBeginX,int _nBeginY,int _nTargetX,int _nTargetY,int _nSpace,int &_nX,int &_nY);


#endif

