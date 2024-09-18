///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: deployMonster_mod.h
 *
 *  Desc:     用于修改刷怪信息的头文件
 * 
 *  His:      River created @ 2007-3-8
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "gc_globalMData.h"


//! 不规则区域的顶点信息
struct SPoints
{
	int   mCount;
    POINT mPoints[MAX_POINTS];
};

//!区域的附加信息
//!add by blueboy @2007,7,26
struct sAreaInfoEx
{
	//!区域是否激活
	BOOL	m_bActive;
	//保留
	int		m_nReserve[50];
};

enum EAREA_TYPE
{
	TYPE_RECT = 0, //矩形区域
	TYPE_SPECIAL,  //特殊区域
	TYPE_RANDOM,   //随机区域
};

struct sAreaInfo 
{	
	//!基本信息
	DWORD			m_dwId;
	char			m_szName[20];

	//!区域类型
	EAREA_TYPE		m_AreaType;

	//!特殊区域时：为0,站岗(不移动);为1,顺序巡逻;为2,往返巡逻;为3,只走一次
	int		m_MotionType;
	
	//!如果是1特殊区域表示标记点列表，如果是2随机区域表示多边形顶点列表
	SPoints m_PointList;

	//!区域范围，对于随机区域无意思
	LONG    m_lCen_x;
	LONG    m_lCen_y;
	LONG    m_lWidth;
	LONG    m_lHeight;
	
	DWORD	m_dwColor;

	//!怪物个数
	DWORD	m_dwnMonsts;
	//!怪物的偏移
	DWORD	m_dwofsMonst;

	//!扩展信息
	sAreaInfoEx m_AreaEx;

};


struct sMonstInfo {
	DWORD		m_dwMonstId;
	char		m_szMonstName[20];

	POINT       m_ptPostion;

	//!怪物刷新数量
	DWORD		m_dwMonstNum;
	//!0不按照时间,1真实时间
	DWORD		m_dwMonstType;
	DWORD		m_dwBeginTime;
	DWORD		m_dwEndTime;
	char		m_szRemark[20];


};

struct MonstDistribHeader {
	char id[4];
	DWORD version;
	DWORD nAreas;
	DWORD ofsArea;
	DWORD nMonsts;
	DWORD ofsMonst;
};



//! 在leapMagic内容插入怪物的怪物信息结构
struct gc_monsterInsert
{
	//! 怪物位置
	POINT      m_sInsertPt;
	//! 怪物的对应目录
	char       m_szMonsterDir[128];
	//! 场景内创建怪物后，怪物的handle.
	int        m_iMonsterHandle;

	//! 怪物的视野信息,
	float      m_fVisualField;

	//! 怪物的触发半径.
	float      m_fTriggerDis;
	//! 怪物的逃跑半径
	float      m_fEscapeDis;
	//! 怪物的呼叫矩离
	float      m_fCallDis;
	//! 怪物的缩放比例
	float      m_fScale;


};



//! 地图资料版本头结构
struct MapVersionHead
{
	// 版本号
	unsigned int	m_dwVersion;
public:
	MapVersionHead()
	{
		m_dwVersion = 0;
	}
};

//! 一个区域内最多的怪物种类数
# define MAX_MONSTERTYPENUM  20
/** \brief
 *  刷怪信息的类,调入二维编辑器内的刷怪信息,在三维内修改
 *
 *
 */
class gc_deployMonsterMod : public ost_Singleton<gc_deployMonsterMod> 
{
private:

	gc_deployMonsterMod();
	~gc_deployMonsterMod();

	//! 怪物刷新信息是否已经存在
	bool      m_bLoadedDeployData;


	std::vector<sAreaInfo>	      m_sAreaList;
	std::vector<sMonstInfo>	      m_sMonstList;
	MonstDistribHeader            m_sHeader;

	// River mod @ 2007-6-11:文件头相关
	MapVersionHead                m_sMVHead;


	std::vector<gc_monsterInsert> m_vecDmmMonInfo;

	//! 刷怪信息文件的名字
	s_string                      m_szDmmFileName;

	//! 随机刷怪区域的数目.
	int                        m_iRandomAreaNum;

	//! 得到第n个随机区域
	sAreaInfo*                 get_randAreaFromIdx( int _idx );

	//! 得到随机区域的索引
	bool                        get_randAreaFromIdx( int _idx,int& _resIdx );


	// 加入怪物时使用的一些操作信息.
	static  s_string   m_sMonsterTypeDir[MAX_MONSTERTYPENUM];
	static  int        m_sMonsterTypeId[MAX_MONSTERTYPENUM];
	static  int        m_iMonsterTypeNum;

public:

	/** 得到一个sg_timer的Instance指针.
	 */
	static gc_deployMonsterMod*    Instance( void );
	
	/** 删除一个sg_timer的Instance指针.
	 */
	static void              DInstance( void );

	//! 调入和存储
	BOOL   lod_dmmData( const char* _fname );
	BOOL   save_dmmData( void );
	BOOL   is_dmmDataLoaded( void ) { return m_bLoadedDeployData; } 

	//! 更新场景内所有的刷怪的位置信息
	BOOL   update_dmmPosInfo( void );


	//! 重设此信息编辑器
	void   reset_dmmData( void );


	//! 对怪物进行操作的函数
	bool   mod_monsterPos( osVec3D& _newpos );

	//! 得到刷怪资料的函数
	bool   get_monsterData( int _idx );

	//! 得到区域的数目
	int    get_monsterAreaNum( void );
	const char* get_monsterAreaName( int _idx );

	//! 得到区域的几何数据
	const SPoints* get_monAreaGeoData( int _idx );
	
	//! 得到此区域内怪物的数目
	int    get_monsterNum( int _idx );
	//! 得到此区域内每一个怪物信息
	gc_monsterInsert* get_monsterInfoFromIdx( int _areaIdx,int _monIdx );

	//! 在一个区域内删除一个怪物
	bool   delete_monsterInArea( int _areaIdx,int _monIdx,int _handle );

	/** \brief
	 *  在一个区域内添加一个怪物
	 *
	 *  \return  gc_monsterInsert* 返回怪物的插入结构,用于在上层处理怪物的handle信息
	 */
	gc_monsterInsert*   insert_monsterInArea( int _areaIdx,int _monIdx,int _x,int _z );

	//! 得到当前区域内总共有怪物种类
	int          get_monsterType( int _areaIdx );
	const char*  get_monsterTypeDir( int _areaIdx,int _monIdx );
	//! 从区域索引和怪物索引得到怪物的信息
	bool         get_monInfoFromAreaTypeIdx( int _areaIdx,int _monIdx,sMonstInfo& _res );
	//
	bool         get_monInsertFromAreaTypeIdx( int _areaIdx,int _monIdx,gc_monsterInsert& _res );

	//! 处理怪物的目录数据
	const char*                process_monsterDir( char* _dir );


};