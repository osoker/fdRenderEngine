//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdWater.h
 *
 *  Desc:     场景中水相关的特效的处理。
 *
 *  His:      River created @ 2004-5-13.
 *
 *  "最不依赖幸运的人最容易保持他自己的地位"　( 马基雅维里 )
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "../../interface/osInterface.h"

# include "../../mfpipe/include/osShader.h"               
# include "../../mfpipe/include/middlePipe.h"
//

//! River @ 2005-9-22: 以后引擎内的水由osa文件来实现，
//                     这两个文件暂时弃用。

# if 0  //弃用此文件。

/*
水相关的物品的处理，水相关的物品基本上可以用公告板来实现，所以如果可能的话，
使用公告板相关的数据结构。

使用公告板类似的代码，水必须有一个简单的编辑器,刘国习写编辑器相关。
*/


#define WATERFILETITLE   "wat"

//!文件头结构
struct WATER_FILE_HEAD
{
	//!文件名字
	char			m_szFileName[4];

	//!文件版本号100
	DWORD			m_dwFileVersion;
};


/** \brief
 *  水面顶点用到的顶点格式。
 */
struct os_verWater
{
	osVec3D    m_vecPos;
	DWORD      m_dwColor;
	osVec2D    m_vecUv;
};

typedef std::vector< os_verWater > VEC_verWater;


//! 最多可以使用的水面纹理数目。
# define MAX_WATERFRAME        60

/** \brief
 *  水相关的类。
 *
 */
class osc_dwater
{
	friend class   osc_dwaterMgr;
private:

	//! 当前的水面对应的图片的名字。
	char      m_strWTextureName[128];

	//! 帧间切换需要的时间。
	float     m_fMoveSpeed;


	//! x轴方向格子的数目。
	int       m_iXSize;

	//! z轴方向格子的数目。
	int       m_iZSize;

	/** \brief
	 *  当前水流的方向。1:x 2:-y 3: -x 4:y
	 *
	 *  通过控制每一个水流格子的uv坐标来控制这个方向。
	 */
	int      m_iWaterDir;

	//! 当前水面中心点的位置。
	osVec3D  m_vec3CPos;

	//! 当前水面的旋转方向。
	float    m_fRotAgl;

	//! 当前水面开始点的位置,就是x,z最小值的位置。
	osVec3D  m_vec3WStart;


	//! 当前水面的纹理动画的动画帧数。
	int      m_iFrameNum;

	//! 当前水面用到的纹理序列id,只需要存储第一个就可以了。
	int      m_vecDevdepShaderId[MAX_WATERFRAME];

	//! 当前水面的透明度。
	float    m_fAlpha;

	/** \brief
	 *  渲染当前的水面使用的数据
	 *
	 *  每一个水面格子的长度和地表格子一样是3M.
	 *  当前水面可见的时候，每一帧更新每一个polygon的shaderid.
	 *  
	 */
	VEC_polygon    m_arrPolygon;
	VEC_verWater   m_arrWVertex;
	VEC_int        m_vecShaderId;


	// 运行过程中当前帧过去的时间。
	float         m_fCurFrameEleTime;

	//! 创建水面的时候，应该得到当前水面的bounding Square.


	//! 当前水面是否处于使用中。
	bool          m_bInuse;

private:

	//! 从水面的中心点得到水面的起始点位置。
	void     get_waterStartPos( void );

	//! 初始化水面的shader id.
	int      get_polygonSId( int _x,int _z );

	//! 根据我们的数据初始化我们的polygon.
	void     init_rPolygon( osc_middlePipe* _pipe );

	//! 根据方向的不同得到格子用到的uv坐标。
	void     get_uvFromWaterDir( osVec2D* _uv );

	//! 填充格子水面的顶点。
	void     fill_waterTileVer( int _x,int _y,os_verWater* _ver );

	//! 每一帧处理水的动画贴图变化。
	void     frame_moveWater( float _etime );

	//! 从ini文件中读入水的相关配置。
	bool     read_waterIni( const char* _iname );

	//! 从水面文件中读入数据。
	bool     read_waterFile( const char* _wname );

public:
	osc_dwater();
	~osc_dwater();

	//  ATTENTION TO OPP:最终版本中去掉这个函数。
	//! 从文件中调入单独的水文件。
	bool     load_waterFromfile( const char* _fname,
		        osVec3D& _pos,float _rgl,osc_middlePipe* _pipe  );


	//! 使用传入的结构，创建水面数据。
	bool     create_water( os_waterCreate& _wc,
		               osVec3D& _pos,float _rgl,osc_middlePipe* _pipe );


	//! 渲染当前的水面。
	bool     render_water( I_camera*  _camptr,float _etime,osc_middlePipe* _pipe );


	//! 释放当前的水资源。
	void     release_water( void );

};
//! 
typedef std::vector< osc_dwater > VEC_water;


// 
//! ATTENTION TO OPP: 任一时刻，场景中只能有一个可见的水面。
//! 当前场景中最多可以有的水面数目。
# define  MAX_WATERINSCE   4

/** \brief
 *  管理场景中所有的水相关的特效。
 *
 *  使用水特效管理器创建的水特效，只能用于场景小范围的水面，至于场景中大范围的水面，
 *  比如海面等等，必须使用max加贴图移到来得，不然使用此种方式过于浪费渲染资源。
 * 
 */
class osc_dwaterMgr
{
private:
	//!  中间管道的指针。
	osc_middlePipe*    m_ptrMPipe;

	//!  存储当前场景中水面的数据。
	VEC_water         m_vecWater;

	//!  上上帧渲染到的水面的id.
	int               m_iLastRenderId;


public:
	osc_dwaterMgr();
	~osc_dwaterMgr();

	//! 初始化我们的水特效管理器。
	void        init_dwaterMgr( osc_middlePipe* _pipe );


# if __WATER_EDITOR__
	//! 从参数创建一个水面数据。
	int         create_dwater( os_waterCreate& _wc,osVec3D& _pos,float _rgl = 0.0f );
# endif 


	//! 创建一个水面数据。
	int         create_dwater( const char* _fname,osVec3D& _pos,float _rgl );

	//! 渲染当前的水特效管理器。
	bool        render_dWater( I_camera*  _camptr );

	//! 从水面管理器中删除一个水面。
	void        delete_dWater( int _id );

};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   水相关函数的inline函数。
// 

# endif //弃用此文件。




/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*\
|*                 新的water类         snowwin create @ 2006-1-3                   *|
\*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


/// 用来创建bump水面的结构
struct os_water
{
	char		m_szShaderIni[128];
	char		m_szShaderName[64];

	float		m_fSpeed;
	int			m_iDir;

	int			m_iXSize;
	int			m_iZSize;

	float		m_fAlpha;
};


/// 水相关的类。
class osc_water
{
	friend class   osc_waterMgr;

	/// 水面顶点用到的顶点格式
	struct os_verWater
	{
		osVec3D    m_vecPos;
		DWORD      m_dwColor;
		osVec2D    m_vecUv;
	};

	typedef std::vector< os_verWater > VEC_verWater;

private:
	char      m_szShaderIni[128];
	char      m_szShaderName[64];


	//! x轴方向格子的数目。
	int       m_iXSize;

	//! z轴方向格子的数目。
	int       m_iZSize;

	/** \brief
	 *  当前水流的方向。1:x 2:-y 3: -x 4:y
	 *
	 *  通过控制每一个水流格子的uv坐标来控制这个方向。
	 */
	int      m_iWaterDir;

	//! 当前水面中心点的位置。
	osVec3D  m_vec3CPos;

	//! 当前水面开始点的位置,就是x,z最小值的位置。
	osVec3D  m_vec3WStart;

	//! 当前水面用到的纹理序列id,只需要存储第一个就可以了。
	int      m_iShaderId;

	//! 当前水面的透明度。
	float    m_fAlpha;

	/** \brief
	 *  渲染当前的水面使用的数据
	 *
	 *  每一个水面格子的长度和地表格子一样是3M.
	 */
	VEC_polygon    m_arrPolygon;
	os_polygonSet	m_cPolySet;
	VEC_verWater   m_arrWVertex;

	//! 当前水面是否处于使用中。
	bool          m_bInuse;

private:

	//! 从水面的中心点得到水面的起始点位置。
	void     get_waterStartPos( void );


	//! 根据我们的数据初始化我们的polygon.
	void     init_rPolygon( osc_middlePipe* _pipe );

	//! 根据方向的不同得到格子用到的uv坐标。
	void     get_uvFromWaterDir( osVec2D* _uv );

	//! 填充格子水面的顶点。
	void     fill_waterTileVer( int _x,int _y,os_verWater* _ver );


public:
	osc_water();
	~osc_water();


	//! 使用传入的结构，创建水面数据。
	bool     create_water( os_water& _wc, osVec3D& _pos, osc_middlePipe* _pipe );


	//! 渲染当前的水面。
	bool     render_water( I_camera*  _camptr, osc_middlePipe* _pipe );


	//! 释放当前的水资源。
	void     release_water( void );
};

typedef std::vector< osc_water > VEC_water;

//! 当前场景中最多可以有的水面数目。
# define  MAX_WATERINSCE   4

/** \brief
 *  管理场景中所有的水相关的特效。
 */
class osc_waterMgr
{
private:
	//!  中间管道的指针。
	osc_middlePipe*    m_ptrMPipe;

	//!  存储当前场景中水面的数据。
	VEC_water         m_vecWater;

	//!  上上帧渲染到的水面的id.
	int               m_iLastRenderId;

public:
	osc_waterMgr();
	~osc_waterMgr();

	//! 初始化我们的水特效管理器。
	void        init_waterMgr( osc_middlePipe* _pipe );


	//! 从参数创建一个水面数据。
	int         create_water( os_water& _wc,osVec3D& _pos,float _rgl = 0.0f );

	//! 渲染当前的水特效管理器。
	bool        render_water( I_camera*  _camptr );

	//! 从水面管理器中删除一个水面。
	void        delete_water( int _id );
};
