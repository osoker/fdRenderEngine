//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdNewFieldStruct.h
 *
 *  His:      River created @ 2006-1-10
 *
 *  Des:      地图数据类相关的结构。
 *   
 * “吾恐季氏之忧，不在颛臾，而在萧墙之内也。” 
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "fdFieldStruct.h"
# include "../geoMipTerrain/newHeader.h"
# include "fdTGQuadtree.h"
# include "../geoMipTerrain/MapTile.h"

typedef std::vector<os_newTGObject>  VEC_newTgObj;
typedef std::vector<os_aabbox>       VEC_aabbox;


//!　场景内资料版本号
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

//! 占用两个位确认物品的类型
# define OBJ_BRIDGETYPE			0x01
# define OBJ_TREETYPE			(0x01<<1)
# define OBJ_ACCEPTFOGTYPE		(0x01<<2)
# define OBJ_NOALPHATYPE		(0x01<<3)


//! 在一个chunk内部，地表格子的索引
struct osc_tileInChunk
{
	int     m_iX;
	int     m_iY;
};

typedef std::vector<osc_tileInChunk> VEC_tileInChunk;
 
//! River mod @ 2009-9-14:用于新版本的，地表格子边界的硬边无融合纹理
//! 每一个chunk可以有多个对应的hare texture tile的列表数据
struct osc_hardTextureTile
{
	//! 地表格子的数目。
	int     m_iTileNum;

	//! 硬接触地表格子的索引
	VEC_tileInChunk   m_vecTileInChunk;

	//! 对应的纹理名字和UV缩放值
	char             m_szTexName[32];
	int              m_wU;
	int              m_wV;
	int				 m_rotation; // 0 (0 rad),1 (pi*0.5f rad),2 (pi rad),3 (pi * 1.5f rad)
};


//! 每一个chunk对应的hardTexture数据
struct osc_chunkHardTile
{
	int   m_iHardTileNum;

	osc_hardTextureTile*   m_arrTileInChunk;

public:
	osc_chunkHardTile()
	{
		m_arrTileInChunk = NULL;
		m_iHardTileNum = 0;
	}

	void reset( void )
	{
		m_iHardTileNum = 0;
		SAFE_DELETE_ARRAY( m_arrTileInChunk );
	}

	~osc_chunkHardTile()
	{
		reset();
	}
};

//! 定义每一个tileGround用到的数据结构
typedef std::vector<osc_chunkHardTile>  VEC_chunkHardTile;

/** \brief
 *  新的基础地块组织类
 *  
 *  这个地块组织类调入新生成的Geomipmap数据,把地表相关的数据传给Geomipmap
 *  相关的数据结构,把物品等显示信息自己组织处理.
 */
class osc_newTileGround
{
	friend class             osc_TGManager;
	friend class             osc_tgLoadReleaseMgr;
	friend class             osc_tgQuadtree;

public:
	//! TG内部保存的他所在的管理器的指针。
	static osc_TGManager*     m_ptrTgMgr;

	//! 全局的资料版本号
	static MapVersionHead     m_sMapVersionHead;
	static BOOL               m_bMapVersionChange;

	//! 地图对应的硬盘文件的地图名字。
	s_string          m_strMapFName;

	//! 碰撞信息
	os_TileCollisionInfo  m_arrCollInfo[GROUNDMASK_WIDTH*GROUNDMASK_HEIGHT];

	// 每个碰撞格子在最上层纹理idx
	BYTE                m_vecUpmostTexIdx[GROUNDMASK_WIDTH*GROUNDMASK_HEIGHT];
	//! 地图中每一个格子属性，目前只需要知道是否显示。
	VEC_BYTE            m_vecTileAttribute;

	//! 保存一份当前tg对应文件的文件头信息 
	os_newFileHeader     m_sTgHeader;

	//! 场景地图编辑器产生物品的数目
	int                 m_iTGObjectNum;
	VEC_newTgObj         m_vecTGObject;
	VEC_aabbox           m_vecObjAabbox;
	//! 记录每一个物品相应的动态光,如果对应为零，则不受动态光的影响
	VEC_meshDLight       m_vecTGObjDL;



	//! 把osa文件调入到相应的smInit结构内。
	BOOL               load_smInitData( const char* _sz,
		                 os_skinMeshInit& _smInit,os_newTGObject*  _ptrObj );

	/** \brief
	 *  每一个物体在中间管道中渲染时需要的ID.
	 */
	VEC_int              m_vecObjectDevIdx;
	//! River added @ 2008-12-16:加入数组，以确定对应ID是否为brideObj.
	VEC_BYTE             m_vecObjectType;

	//! 在当前渲染帧内，物品的每一个索引是否在视野内。
	VEC_BYTE            m_vecObjInView;

	//! 此tileGround内部碰撞格子的高度数据.
	VEC_float           m_vecHeight;

	//! 当前的tileGround是否已经使用.
	BOOL                m_bInuse;
	//! 当前的Tg是否在多线程排队调入状态
	BOOL                m_bInMTQueue;

	//! 管理场景中可见的物品列表和物品淡入淡出相关数据的管理器.
	osc_inViewObjMgr     m_ivoMgr;

	//! 用于管理地表相关数据的渲染的类
	MapTile             m_sMapTerrain;

	//! 调整水面高度时用 snowwin add
	static std::vector<float> m_vecWaterHeight;

	//！ 高版本的场景，可能会用到的一些数据结构
	VEC_chunkHardTile   m_vecChunkHardTile;

public:
	//! 此tg对应的四叉树结构
	osc_tgQuadtree       m_sQuadTree;

	//! 是否整个TG没有一个Chunk需要画,此变量用于上层其它的处理
	BOOL                m_bEntireCullOff;

	//! 当前相机在TG内的哪一大块。
	static osn_inTgPos   m_seCamTgPos;

	// windy mod
	float				mWaterHeight;

	/** \brief
	 *  得到可见的物品列表。
	 *
	 *  慢慢的优化这个算法，就简单的算法是只要是视域内物品，都是可见的物品。
	 */
	void              get_inviewObjList( void );


private:

	//! 设置一个地表的数据Chunk为可画的状态
	void                set_chunkDraw( int _idx,BOOL _draw = TRUE );

	//! 设置一个数据Chunk受动态光的影响
	void                set_chunkAffectByDL( int _idx );

	/** \brief
	 *  第一次调入地图时，根据当前meshMgr中可容纳的最大mesh数目，调入当前
	 *  地图的mesh.
	 *  
	 *  动态的调入地图的obj时，调用这个函数。
	 *  
	 */
	void                load_tgObj( void );

	//! 得到每一个物品是否为bridge物品
	void                process_objIsBridge( void );


	//! 创建此tg内所有的物品的设备相关数据
	BOOL                create_devdepObj( void );


	/** \brief
	 *  对地图上billBoard物品进行创建。
	 *  
	 */
	BOOL              create_billBoardObj( void );

	/** \brief
	 *  创建动画物品相关的数据。
	 */
	BOOL              creaet_keyAniObj( void );

	/** \brief
	 *  创建TileGround中的Water相关物品。
	 */
	BOOL              create_waterObj( void );

	/** \brief
	 *  创建场景中粒子相关的物品。
	 */
	BOOL              create_particleObj( void );

	/** \brief
	 *  创建场景内的整体特效物品，即spe物品
	 */
	BOOL              create_effectObj( void );


	/** \brief
	*  创建TileGround中的体积雾相关物品。
	*/
	BOOL              create_FogObj( void );

	/** \brief
	 *  创建地图上使用skinMesh进行动画的物体。
	 *
	 *  真接使用skinMesh管理进行创建。
	 */
	BOOL              create_smAniObj( void );

	//! 立即的创建一个objMesh.
	BOOL              create_meshImmediate( int _idx );

	//! 释放地图的物品。
	BOOL              relase_tgObj( void );

	/** \brief
	*  仅渲染场景内的可接受体积雾物品
	*/
	BOOL              render_AcceptFogObject( );

	//! 把一个物品推入不透明渲染列表中，主要用于雾类物品
	void PushFogObject( int _idx );

	/** \brief
	 *  仅渲染场景内的桥类物品
	 */
	BOOL              render_bridgeObject( osc_middlePipe* _mpipe );

	/** \brief
	 *  渲染场景内桥类物品的透明部分。
	 */
	BOOL              render_bridgeObjAlphaPart( osc_middlePipe* _mpipe );

	/** \brief
	 *  地图块上物品渲染的函数. 
	 *
	 *  \param _alpha 控制当前渲染的是物品中透明的部分还是不透明的部分。
	 *                透明的部分需要放到最后渲染。
	 */
	BOOL              render_TGsObject( osc_middlePipe* _mpipe,
		                                        BOOL _alpha = false );

	//! 渲染淡入的Object.
	BOOL              render_TGsFadeInObject( osc_middlePipe* _mpipe,
		BOOL _alpha = false );

	//! 渲染淡出的Object.
	BOOL              render_TGsFadeOutObject( osc_middlePipe* _mpipe,
		BOOL _alpha = false );

	/** \brief
	 *  渲染半透明的object.
	 *
	 *  ATTENTION TO OPP:以下三个函数可以合并成一个.
	 *  没有前两个函数alpha为true的情况发生.
	 */
	BOOL              render_halfAlphaObj( 
		osc_middlePipe* _mpipe,BOOL _alpha );

	/** 根据地图的全地图路径名,得到地图名
	*	如:_szfullname map\vilg\vilg0000.geo,
	*	_szmapname vilg
	*/
	void             getmapname( const char* _szfullname,s_string& _szmapname);

	//! 调入当前tileGround内的硬地表数据。
	void             load_hardTileData( const BYTE* _fstart );

public:
	osc_newTileGround();
	~osc_newTileGround();
	

	/** \brief
	 *  从内存中调入一个地图文件。
	 *
	 *  \param _fload  是否是游戏初始化时第一次调入地图。
	 */
	BOOL              load_TGData( const char* _fname,
		                   const osc_middlePipe* _mpipe,bool _fload = false );

	/** \brief
	 *  释放当前地图的资源.
	 */
	BOOL              release_TG( void );


	//! FrameSet,每一帧重置当前tg内的数据
	void              frame_setTG( void );

	/** \brief
	 *  把当前帧可见的object放到渲染列表中.
	 */ 
	void              push_objIdx( int _oidx );
	//! 当前的obj索引是否可见
	bool              is_objInView( int _oidx ){ return m_vecObjInView[_oidx]; }

	//! 设置当前帧受动态光影响的object的动态光颜色
	void              set_objDlight( int _oidx,int _dlidx );


	//! 当前的地图块是否已经投入使用。
	BOOL              is_tgInuse( void ) { return m_bInuse; }
	//! 得到和设置当前图块的多线程排队状态,m_bInuse一个状态，不能保证多线程调入时地图的正确状态
	void              set_mtQueueState( BOOL _s ) { m_bInMTQueue = _s; }
	BOOL              get_mtQueueState( void )    { return m_bInMTQueue;} 


	//@{
	//! 从一个格子坐标得到当前格子对应的渲染polygon顶点。
	void               get_polyVertex( osVec3D* _vec,int _x,int _z ); 
	//@} 

	/** \brief
	 *  得到地图对应的硬盘地图文件名。
	 */
	const char*         get_mapFName( void ){ return m_strMapFName.c_str(); }

	//! 得到物品索引的BoundingBox和aabbox
	void               get_objBBox( int _objIdx,os_bbox& _box );
	//! River @ 2007-4-11:得到是否是桥类的物品,桥类物品需要精确碰撞检测.
	BOOL               is_bridgeObj( int _objIdx )
	{
		return m_vecObjectType[_objIdx] & OBJ_BRIDGETYPE;
	}
	//! RIVER @ 2009-3-3: 得到是否树类物品，然后精确的碰撞检测
	BOOL               is_treeObj( int _objIdx )
	{
		return m_vecObjectType[_objIdx] & OBJ_TREETYPE;
	}
	BOOL               is_AcceptFogObj( int _objIdx )
	{
		return m_vecObjectType[_objIdx] & OBJ_ACCEPTFOGTYPE;
	}
	BOOL               is_NoAlphaObj( int _objIdx )
	{
		return m_vecObjectType[_objIdx] & OBJ_NOALPHATYPE;
	}


	os_aabbox*          get_objAaabbox( int _objIdx )
	{
		return &m_vecObjAabbox[_objIdx];
	}
	DWORD              get_objType( int _objIdx )
	{
		return m_vecTGObject[_objIdx].m_dwType; 
	}

	//! 检测射线是否和物品索引相交.
	BOOL               is_intersectObj( int _objIdx,osVec3D& _rayS,
		                    float _dis,osVec3D& _rayE,float* _resDis = NULL );


	



	//@{
	//  动态光相关的函数。
	/** \brief
	*  使用给出的灯光系数对场景中每一个顶点的灯光进行缩放。
	*/
	void              set_ambientLCoeff( float _r,float _g,float _b );

	/** \brief
	 *  处理动态光。
	 */
	BOOL              process_dLight( osc_dlight* _dl ,int _lightID);

	

	/** \brief
	 *  根据传入的位置，判断相机所在的TG的哪一大块。
	 *
	 *  传入的参数为碰撞坐标在整个大地图上的位置。
	 */
	static void   frame_moveCamInTgPos( int _camx,int _camy );   

	//! 得到相机在tg内的哪一块.
	static osn_inTgPos get_camInTgPos( void );

	/** \brief
	*  查看一个一条射线是否和一个Tile的os_polygon相交。
	*
	*  \param _tileIdx  如果和当前格子的三角形相交，返回和当前格子内的哪
	*                   一个碰撞格子相交。碰撞格子的索引：z值最大的，x值最小的方位为0,
	*                   x,z最大的方位为1,z最小，x最大的方位为2。
	*  \param _poly     传入的格子对应的渲染三角形。
	*  
	*/
	static bool intersect_polygon( 
		const osVec3D* _arrVerData,osVec3D& _s,osVec3D& _e );

	//! 从一个索引得到一个格子的碰撞和高度信息
	const os_TileCollisionInfo*  get_tileColInfo( int _x,int _z )
	{
		return &m_arrCollInfo[_z*GROUNDMASK_WIDTH+_x];
	}
	const os_TileCollisionInfo*  get_tgColInfo( void )
	{
		return m_arrCollInfo;
	}
	const float* get_tgColHeightInfo(void)
	{
		if (!m_vecHeight.size())
		{
			return NULL;
		}
		return &m_vecHeight[0];
	}
	const int get_tgMaterialIdxInfo(WORD dx,WORD dz)
	{
		osassert(dx>=0);
		osassert(dz>=0);
		osassert(dx<128);
		osassert(dz<128);
		return (int)m_vecUpmostTexIdx[dz*128+dx];
	}
	const float get_tgWaterHeight()
	{
		return mWaterHeight;
	}

	//! 是否整个TG都被ViewFrustum Cull掉了
	BOOL         is_vfEntireCullOff( void ) { return m_bEntireCullOff; }



};





// 
//! 场景内点击选中地表多边形的帮助类
class osc_fdChunkPick
{
private:
	//! 存储当前Chunk内要测试的Tile格子
	os_tileIdx       m_arrTestTile[MapTile::GRIDS*MapTile::GRIDS];
	int             m_iTestTileNum;

	//! 用于存储要判断chunk的四个边界剪切面
	osPlane          m_arrPlane[4];

	//! 
	osVec3D          m_vec3RayStart;
	osVec3D          m_vec3RayEnd;

private:
	//! 构建Chunk四周的四个面
	void          construct_clipPlanes( float _minX,float _minZ );

	void          push_testTile( int _x,int _z );

	//! 根据传入的射线和开始格子坐标，依次查找可能相关的格子
	int           find_testTileArr( float _minZ,int _startX );


public:

	/** \brief
	 *  处理chunk内被射线分割的tileIdx
	 *
	 *  \param _rayStart,_rayDir 鼠标pick射线的方向
	 *  \param _minX,_minZ       要处理Chunk的x,z坐标最小值
	 *  大致的算法描述：       
	 *     
	 *         Z方向
	 *         |      1/
	 *   pt[1] -------/----------- pt[2]
	 *         |  |  /  |        |
	 *         -----/-------------
	 *         |  |/ |  |        |
	 *         ---/---------------
	 *      0  | /|  |           |  2
	 *         -/-----------------   
	 *         /  |  |           |
	 *        /-------------------
	 *Ray start|  |  |           | pt[3]
	 *   pt[0] ------------------------X方向
	 *               3
	 *
	 *  一：先把整个Chunk最外面的四条边编号为0,1,2,3.
	 *  二：把四边边构建四个垂直于xz平面的面,面的法向量朝内四边形的内部。
	 *  三：如果Ray跟第零个面的交点在第1,3面之前，则找出此交点对应的格子索引，然后
	 *      设置此格子可能跟射线相交。
	 *      接下来为第零边边右边紧接着的边构建一个直于xz平面的面，然后找出面跟射线
	 *      的交点出来,设置相应可能跟射线相交的格子。
	 *      直到以下发生：交点在面1或3之外，或者计算了编号为2的面。
	 *  四：如果步骤三不成立，则对第3个面进行处理。找到正确的交线,因为如果一条射线跟
	 *      一个Chunk相关，那么必须跟至少两条边相交，所以只需要处理第零和第1,3条边就ok了。
	 */
	int      get_pickTile( osVec3D&  _rayStart,
		         osVec3D& _rayDir,float _minX,float _minZ );


	//! 得到要处理的TileIdx列表
	const os_tileIdx* get_testTileArr( void ) { return m_arrTestTile ; } 

};


//! 设置当前帧受动态光影响的object的动态光颜色
inline void osc_newTileGround::set_objDlight( int _oidx,int _dlidx )
{ 
	osassert( (_oidx>=0)&&(_oidx<m_iTGObjectNum) ); 

	for( int t_i=0;t_i<MAX_LIGHTNUM;t_i ++ )
	{
		if( -1 == m_vecTGObjDL[_oidx].m_btDLIdx[t_i] )
		{
			m_vecTGObjDL[_oidx].m_btDLIdx[t_i] = _dlidx;
			return;
		}
	}

	return;

}


