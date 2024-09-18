//===================================================================
/** \file  
* Filename :   osMapTerrain.h
* Desc     :   ��Geomipmap��maptile��Chunk����֯��ʹ���������ڵ�Chunk
*              ��һ��FrameMove����Ⱦ
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
 *  �����������ڵ�mapTile��chunk������֯.
 *
 *  
 */
class osc_mapTerrain
{
	friend class    MapTile;
private:
	//! �����������õ���Chunk�б�
	MapChunkPtrList    m_vecChunkPtr;

	//! ʹ�þ�̬��ActiveChunkPtr�б����һ������ָ�룬��������Ҫ��̬�ڴ����
	MapChunkPtrList	   m_vecActiveChunkPtr;
	int               m_iActChunkNum;

	//! �����������õ���MapTile�б�
	VEC_MapTilePtr     m_vecMapTilePtr;

	//! �ײ���Ⱦʱ�õ������ݽṹ
	static os_meshSet  m_sRenderMesh;
	static os_meshSet  m_sLightMapRenderMesh;
	

	static os_meshSet m_sRenderMeshPS_PassList[4];
	//! �ײ���Ⱦ�ж�̬���Chunkʱ���õ���meshSet
	static os_meshSet  m_sRenderDLight[MAX_LIGHTNUM];

	osMatrix		m_ViewProjmat;
private:
	//! ����һ��MapTilePtr
	void             push_mapTilePtr( MapTile* _ptr );

	//! ���뼤���Chunk���С��˺�����MapTile��PushActiveChunk����
	void             push_activeChunkPtr( MapChunk* _chunk );

	//! �õ���ǰMapTile�����ƫ�Ƶ�MapTile
	MapTile*          get_offsetMapTile( MapTile* _ptr,int _xoff,int _zoff );

	/** ���õ�nPass �������*/
	void        	   SetupTextureStages(size_t _nPass);
	/** ����������Ӱ�������� */
	void        	   UnSetupTextureStages();

	//! ���ر�ر�
	void              render_terrain( BOOL _envMap );
	///@{
	//! ���ر������Ϣ
	void              render_terrain_Depth();

	static IDirect3DTexture9 *			m_pSceneDepthTex;
	static LPDIRECT3DSURFACE9			m_pSceneDepthSurface;
	static int							m_SceneDepthId;
	static LPDIRECT3DSURFACE9			mpTargetZ;

	static LPDIRECT3DSURFACE9			mpOldRenderTarget;
	static LPDIRECT3DSURFACE9			mpOldStencilZ;

	static os_meshSet m_sRenderMeshDepth_Pass;
	
	///@}


	//! ����ȫ�ֵĶ�̬����Ϣ
	void              set_terrainDLight( void );
	//! ���õر��ȫ�ֹ�����Ϣ
	void              set_terrainLightInfo( void );

	//! set the mesh set light information
	void				set_meshSetLightInfo(os_meshSet& _meshSet);

public:
	osc_mapTerrain();
	~osc_mapTerrain();


	//! �õ�ʵ����ָ�롣
	static osc_mapTerrain*  get_mapTerrainIns( void );

	static void		oneTimeDeinit( void );
	//! ��ʼ��MapTerrain���������
	static void   oneTimeInit( void );

	//! �豸lost
	void          onLostMapTerrain( void );

	//! �豸restore��,��Ҫ�������ÿһ��chunk�Ķ�̬���㻺����
	void          onRestoreMapTerrain( void );

	//! �õ�һ��Chunk,mapTile�ڲ�ʹ��
	MapChunk*     create_chunk( MapTile* _mt,int _x,int _z );
	void			clearVisible();

	//! ÿһ֡������Chunk�б����FrameMove�ͻ�
	BOOL            frame_processTerrain( BOOL _envMap = FALSE ,BOOL _depth=FALSE);

	//! ���õ�ǰmapTile��ÿһ��Chunk֮��Ĺ�����ϵ,��������ӵ�MapTile.
	void            SetNeighbors( MapTile* _ptr );


	//! ���ݵ�ͼ���������õ���mapTile�ĵر�߶����ݡ�
	const float*    get_mapTileHeightInfo( int _x,int _z );

	//! �����������꣬�õ���chunk�Ĺ���ͼ��ɫ
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

