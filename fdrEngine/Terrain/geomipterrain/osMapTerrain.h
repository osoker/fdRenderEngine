//===================================================================
/** \file  
* Filename :   osMapTerrain.h
* Desc     :   对Geomipmap中maptile和Chunk的组织。使整个场景内的Chunk
*              在一起FrameMove和渲染
* 
* His      :   River create @ 2006-1-12
*/
//===================================================================
#pragma once
# include "../../interface/osInterface.h"
# include "CeoMipTerrainAll.h"
# include "../../mfpipe/include/osPolygon.h"
# include "../../effect/include/dlight.h"




typedef std::vector<MapTile*> VEC_MapTilePtr;


/** \brief
 *  对整个场景内的mapTile和chunk进行组织.
 *
 *  
 */
class osc_mapTerrain
{
	friend class    MapTile;
private:
	//! 整个场景内用到的Chunk列表
	MapChunkPtrList    m_vecChunkPtr;

	//! 使用静态的ActiveChunkPtr列表，配合一个整形指针，这样不需要动态内存操作
	MapChunkPtrList	   m_vecActiveChunkPtr;
	int               m_iActChunkNum;

	//! 整个场景内用到的MapTile列表
	VEC_MapTilePtr     m_vecMapTilePtr;

	//! 底层渲染时用到的数据结构
	static os_meshSet  m_sRenderMesh;
	static os_meshSet  m_sLightMapRenderMesh;
	

	static os_meshSet m_sRenderMeshPS_PassList[4];
	//! 底层渲染有动态光的Chunk时，用到的meshSet
	static os_meshSet  m_sRenderDLight[MAX_LIGHTNUM];

	osMatrix		m_ViewProjmat;
private:
	//! 加入一个MapTilePtr
	void             push_mapTilePtr( MapTile* _ptr );

	//! 推入激活的Chunk队列。此函数在MapTile的PushActiveChunk调用
	void             push_activeChunkPtr( MapChunk* _chunk );

	//! 得到当前MapTile的相对偏移的MapTile
	MapTile*          get_offsetMapTile( MapTile* _ptr,int _xoff,int _zoff );

	/** 设置第nPass 层的纹理*/
	void        	   SetupTextureStages(size_t _nPass);
	/** 清除纹理避免影响别的物体 */
	void        	   UnSetupTextureStages();

	//! 画地表地表
	void              render_terrain( BOOL _envMap );
	///@{
	//! 画地表深度信息
	void              render_terrain_Depth();

	static IDirect3DTexture9 *			m_pSceneDepthTex;
	static LPDIRECT3DSURFACE9			m_pSceneDepthSurface;
	static int							m_SceneDepthId;
	static LPDIRECT3DSURFACE9			mpTargetZ;

	static LPDIRECT3DSURFACE9			mpOldRenderTarget;
	static LPDIRECT3DSURFACE9			mpOldStencilZ;

	static os_meshSet m_sRenderMeshDepth_Pass;
	
	///@}


	//! 设置全局的动态光信息
	void              set_terrainDLight( void );
	//! 设置地表的全局光照信息
	void              set_terrainLightInfo( void );

	//! set the mesh set light information
	void				set_meshSetLightInfo(os_meshSet& _meshSet);

public:
	osc_mapTerrain();
	~osc_mapTerrain();


	//! 得到实例的指针。
	static osc_mapTerrain*  get_mapTerrainIns( void );

	static void		oneTimeDeinit( void );
	//! 初始化MapTerrain整体的数据
	static void   oneTimeInit( void );

	//! 设备lost
	void          onLostMapTerrain( void );

	//! 设备restore后,需要重新填充每一个chunk的动态顶点缓冲区
	void          onRestoreMapTerrain( void );

	//! 得到一个Chunk,mapTile内部使用
	MapChunk*     create_chunk( MapTile* _mt,int _x,int _z );
	void			clearVisible();

	//! 每一帧对整个Chunk列表进行FrameMove和画
	BOOL            frame_processTerrain( BOOL _envMap = FALSE ,BOOL _depth=FALSE);

	//! 设置当前mapTile内每一个Chunk之间的关联关系,另加相连接的MapTile.
	void            SetNeighbors( MapTile* _ptr );


	//! 根据地图的索引，得到此mapTile的地表高度数据。
	const float*    get_mapTileHeightInfo( int _x,int _z );

	//! 根据世界坐标，得到此chunk的光照图颜色
	DWORD osc_mapTerrain::get_mapTileLightInfo(const osVec3D& _pos);

//--------------------------------------------------------------------------------
//@{ tzz added for galaxy game @ 2008-6-5
//--------------------------------------------------------------------------------
#if GALAXY_VERSION

	//@{ tzz added for galaxy game @ 2008-6-5
public:
 

	/*! \fn static void gala_iniEffect(gala_explode* _ptrExplode = NULL)
	*	\brief : initialize the effect shader data
	*
	*	\param _ptrExplode : the struct to initialize fx file's data
	*	\return :void
	*/
	static void gala_initExplode(gala_explode* _ptrExplode = NULL);

	/*! \fn 	static void gala_initSpaceHole(gala_spaceHole* _ptrHole = NULL);
	*	\brief : initialize the effect shader data
	*
	*	\param _ptrHole : the struct to initialize fx file's data
	*	\return :void
	*/
	static void gala_initSpaceHole(gala_spaceHole* _ptrHole = NULL);

	/*! \fn static void gala_delSpaceHole(int _index)
	*	\brief : delete the effect shader data
	*
	*	\param _ptrBullet : the struct to initialize fx file's data
	*	\return :void
	*/
	static void gala_delSpaceHole(int _index);


private:
	//! rander gala effect?
	//static bool			sm_bGalaEff;

	//! our counter float time 
	//static float		sm_fTime;

	//@ static constant variable 
	//! caution !!!
	//! these const variable must is same as Max bullet and
	//! Max explode number in .fx file(geomipmap.fx)
	//! can NOT change one but other one

	//! max bullet number 
	static const int	scm_iMaxBullet = 5;

	//! max explode number
	static const int	scm_iMaxExplode = 3;
	//@}

	//! array of explode 
	static gala_explode		sm_arrExplode[scm_iMaxExplode];

	//! array of bullet 
	static gala_spaceHole		sm_spaceHole;

	//! ptr of explode cycle queue4
	static int				sm_iCurrExplode;

	//! ptr of bullet cycle queue
	//static int				sm_iCurrBullet;

public:

	/*! \fn void gala_frameMove(void)
	*	\brief : every frame call
	*
	*	\param   :void
	*	\return	: void
	*/
	static void gala_frameMove(gala_spaceHole* _hole = NULL);

#endif	//GALAXY_VERSION
//--------------------------------------------------------------------------------
//@}
//--------------------------------------------------------------------------------

};

