//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdFieldMgr.h
 *
 *  His:      River created @ 2003-12-23
 *
 *  Des:      用于管理地图格子的类，使用当前的相机，得到当前应该调用哪一张地图。
 *   
 * “吾恐季氏之忧，不在颛臾，而在萧墙之内也。” 
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# ifndef __FDFIELDMGR_INCLUDE__
# define __FDFIELDMGR_INCLUDE__

# pragma warning( disable : 173 )

//! 包含地图管理器用到的相关的数据结构。
# include "fdTGQuadtree.h"
# include "../include/fdFieldStruct.h"
# include "fdNewFieldStruct.h"
# include "fdTerrainDecal.h"
# include "../../mfpipe/include/Font.h"
# include "../../effect/include/fdRain.h"
# include "../../effect/include/fdlensflare.h"
# include "../GeoMipTerrain/CeoMipTerrainAll.h"
# include "../../effect/include/fdSpecialEffect.h"
# include "../../effect/include/SceneLight.h"
# include "../../fdrEngine/light.h"

//@{ tzz add for galaxy space hole and other effection

# include "../../fdrengine/sceneGrow.h"
# include "../../fdrengine/sceneHDR.h"


# if GALAXY_VERSION


# include "../../fdrengine/GridCurve.h"

# include "../../fdrengine/coronaEff.h"
# include "../../fdrengine/blood.h"

# endif //GALAXY_VERSION
//@}

# include "QuadTree.h"
# include <list>

class osHDRLight;
extern float gOSAPLAYERSpeed ;
//! 客户端最多可以使用多少种cursor名字。
# define  MAX_SCECURSOR_NUM  4


//! 定义从相机到人物之间最多可以有20个物品，如果超出这个数目，
//!  则超出部分物品不会变得透明。
# define  MAXOBJNUM_TOCHAR   32

//! 渲染天空时，用到的远剪切面.
# define  SKYRENDER_FARDIS 1200.0f


//! 是否使用Lens Flare
# define USE_LENSFLARE


//! 定义自定义碰撞信息的文件头和版本号.
# define  CCOL_HEADER    "ccl"
# define  CCOL_VERSION    0x100

//! 声明skinMesh管理类.
class      osc_skinMeshMgr;
class      osc_boltMgr;

struct     os_sceneTexName
{
	char   m_szTexname[32];
};



/** \brief
 *  整个场景中物品种类，对于静态的物品，我们在第一次进入的场景的时候.
 *  把所有的物品都创建到内存。
 */
struct     os_sceneObjectName
{
	char   m_szObjName[32];
	DWORD  m_dwObjType;
};

//! 整个场景中所有的物品的内存数据
struct     os_sceneOjbMemData
{
	//! 如果物品在场景中的类型是静态物品，则调入此静态物品到内存
	int    m_iSize;
	BYTE*  m_arrOjbMemFile;
};

//! 场景最多支持10个水面高度.
# define MAX_WATERHEIGHT   10

//! 整个场景用到的数据,比如全场景的纹理名字等等.
struct     os_wholeSceneData
{
	int                m_iTexNum;
	os_sceneTexName*    m_arrTexnameList;

	VEC_int            m_iShaderId;

	//! 当前场景对应的背景音乐文件名.
	char               m_szBgMusic[32];

	//! 当前场景中对应的所有Object文件。
	int                m_iObjectNum;
	os_sceneObjectName*  m_arrObjNameList;

	//! 当前场景水面的高度
	float              m_fWaterHeight;

private:
	//! 定义一个数组,由此数组来整理场景内多个高度的水面
	float              m_vecWaterHeight[MAX_WATERHEIGHT];
	int                m_iWHNum;
public:
	os_wholeSceneData();
	~os_wholeSceneData();

	//! 调入整个场景中的物品到内存
	bool              init_wholeSceneData( void );

	//! push water height.
	void              push_waterHeight( float _wh );

	//! 得到当前最科学的水面高度.
	float             get_waterHeight( void );

	//! 
	void              release( void );


};

# include "fdTGLoadReleaseMgr.h"

typedef std::vector<os_terrainTexSnd>  VEC_terrainTexSnd;

extern osMatrix  g_sMatReflectView;
extern osMatrix  g_sMatReflectProj;


//! River @ 2010-6-24:全局的变量，确认是否需要渲染屏幕光波.
extern BOOL      g_bRenderScreenWave;

/** \brief
 *  组织整个地图的类。TileGroundManager.
 *  
 *  根据相机确定哪一个地图需要调入到内存，哪一个地图需要从内存中
 *  释放以节省内存空间。
 *  
 */
class osc_TGManager : public I_fdScene 
{
	friend class osc_newTileGround;
	friend class osc_mapLoadThread;
	friend class osc_tgLoadReleaseMgr;
	friend class osc_mapTerrain;
	friend struct os_aniNode;

public:

	void register_updateLoadingStepCallbackFunc( UPDATE_LOADINGSTEPCALLBACKIMM _pFunc );
	static UPDATE_LOADINGSTEPCALLBACKIMM m_pUpdate_uiLoadingStep;
	
	// HDR Light
	void init_hdrLight();
	void set_hdrRenderTarget();
	void set_hdrParameter( float _fmx, float _fmy, float _diffuse, float _exposure );
	void set_hdrFm( float _fm );
	void set_hdrDiffuse( float _diffuse );
	void set_hdrExposure( float _exposure );
	static void ReleaseStatic(void);


	//! River added @ 2009-7-23:全局得到水面的高度
	float       get_waterSurfaceHeight( void );



private:
	//! HDR light
	osHDRLight*    m_pHDRLight;

	//! 从地图的调入和置换管理器中得到可以操作的地图指针。
	osc_TileGroundPtr*  m_arrPtrTG;
	//! 当前帧需要操作的地图数目。
	int               m_iActMapNum;

	//!精确相交坐档的保存
	osVec3D           m_vec3PickPos;


	/** \brief
	 *  地形管理器
	 */
	osc_tgLoadReleaseMgr*  m_ptrTerrainMgr;



	//! 当前地图的名字，目录加前面的标识，不包括后面的数字和.map扩展。
	s_string         m_strMapName;


	//! 当前相机所在的图块索引，这个数据应该有一个初始值，
	//! 初始值为0,0.
	os_tileIdx        m_vec2CamInIdx;

	//@{ 
	/** \brief
	 *  X,Z方向上地图块的最大索引值,此值为显示格子的牵引值.
	 *  以64为基准,为64的倍数.
	 */
	int              m_iMaxX;
	int              m_iMaxZ;

	//! 在x,z方向上最大的TileGround数目。
	int              m_iMaxXTgNum;
	int              m_iMaxZTgNum;
	//@}

	/** \brief
	 *  场景管理器包含的中间管道。
	 */
	osc_middlePipe    m_middlePipe;

	//! 场景中保存的设备管理器类指针,这个指针由场景调入函数传入.
	static I_deviceManager*  m_deviceMgr;

	//! 场景中保存的整个人物管理器实例。
	osc_skinMeshMgr*     m_skinMeshMgr;


	//@{
	/** 特效相关的管理器,MeshMgr使用的是全局的变量
	osc_meshMgr*     g_ptrMeshMgr;
	*/

	//! 场景中处理billBoard的管理器。
	osc_bbManager*        m_bbMgr;
	//! 场景中的粒子场景管理器，使用这个管理器创建和渲染粒子。
	osc_particleMgr       m_particleMgr;

	//@}


	//! 场景中处理bolt的mgr.
	osc_boltMgr*          m_boltMgr;
	
	//! 当前场景中的动态光管理器。
	osc_dlightMgr        m_dlightMgr;

	//! 当前场景中管理天空的管理器.
	osc_fdSkyMgr        m_skyMgr;

	//@{
	// Windy mod @ 2005-9-14 17:13:16
	//Desc: lens flare
	os_Lensflare *		m_pLensflare;
	//@}
	//! 闪电管理器,它负责修改方向光
	SceneLightMgr*		m_SceneFlashLightMgr;

	//! 场景中的雨效果。
	osc_rain            m_effectRain;
	//! 场景中的雪效果
	osc_snowStorm		m_effectSnow;

	//! 场景中的特效元素
	osc_specEffMgr*      m_sEffectMgr;

	//! 解晰字串用到的临时缓冲区。
	os_stringDisp*       m_ptrDstr;

	//! 地表上Decal的显示处理
	osc_decalMgr         m_sDecalMgr;

	//@{
	//
	//! 场景中需要显示的二维图片的数据结构。
	VEC_screenPicDis     m_vecScrPicDis;
	int                 m_iCurScrDisNum;
	
	//! 对场景中的二维图片进行渲染。
	void                render_ScrPic( bool _LensPic = false,bool _renderPic = true );
	//@}


	//! 用于点选场景内的Chunk格子的帮助类
	osc_fdChunkPick     m_sChunkPick;

	//! 引擎内画线
	static LPD3DXLINE          m_ptrD3dLine;


	//! 整张地图的非透明物品也是放在一起渲染
	VEC_meshRender       m_vecOpacityObj;
	//! 每一次渲染完成后，清这个值为零。
	int                 m_iOpacityObjNum;
	bool                flush_opacityObj( void );
	//! 渲染并清空物品
	bool                flush_opacityObjNoMaterial( ID3DXEffect* _pEffect,osMatrix& _ViewProjMat);


	//@{
	// 必须整张地图的Alpha物品放在一起渲染.
	/** \brief
	 *  当前帧存储的需要渲染的透明物体的信息，
	 *　
	 *  在渲染之前需要根据每一个物体相对于相机的远近进行排序。
	 *  
	 */
	VEC_meshRender       m_vecAlphaObj;
	//! 每一次渲染完成后，清这个值为零。
	int                 m_iAlphaObjNum;
	//@} 

	/** \brief
	 *  把地图上的透明物体排序后渲染。
	 *  
	 *  \param bool _opacity 渲染透明面中的透明部分或是不透明部分。
	 *                       分两次渲染可以让树的叶子渲染的更加正确，第一次先渲染树叶的
	 *                       不透明部分，第二次渲染树叶的透明部分。使用alphaTest来处理
	 *                       不同的渲染。River @ 2008-10-8
	 *  
	 *  render_TGsObject的时候，如果是透明物体，则放到一个数组中，等待这个函数去做处理。
	 *  
	 */
	bool              flush_alphaObj( bool _opacity );


	/** \brief
	 *  当前场景是否调入了地图。
	 */
	bool              m_bLoadedMap;

	/** \brief
	 *  用于记录从相机到人物之间的物品的索引数据。
	 *
	 *  其中结构中的m_iX表示物品所在tileGround在TGMgr中的索引。
	 *  m_iY表示tileGround中物品的索引。
	 */
	os_tileIdx        m_vecCamChrObj[MAXOBJNUM_TOCHAR];
	int               m_iCamChrObjNum;

	//! 场景纹理数据.
	os_wholeSceneData  m_sSceneTexData;

	
	//! 地表声音和特效相关的信息
	VEC_terrainTexSnd  m_vecTerrainTexSnd;
	bool               load_terrainTexSnd( const char* _mapName );

	/** \brief
	 *  从lst文件中调入场景的相关信息.
	 *
	 *  \param _mapName 要调入信息的地图名字,比如"vilg"就代表map\\vilg目录下的map
	 */
	bool             load_sceneInfoFile( const char* _mapName );


	//! 渲染遮挡物品列表。
	void             render_shelterObj( bool _alpha = true );


	/** \brief
	 *  得到从相机到主角人物的物品的列表,对得到的物品的列表进行处理。
	 *
	 *  把每一个物品分配到每一个tileGround.
	 */
	bool             process_camChrObjList( void );


	//! 对下雨效果的处理。
	void             render_rain( void );
	//! 下雪 windy add 7.11
	void			render_snow(void);
	//! 设置全局的雾渲染状态.
	void             fog_rsSet( void );

	//! 当前帧的渲染中有挡住相机的半透明物品.
	bool             m_bHasAlphaShelter;


private:
	//! 场景中保存当前可见的视图集
	VEC_tileIdx         m_vecVisualSet;
	int                 m_iVTNum;

	//! 场景中最远处有透明效果的Tile集合。
	VEC_tileIdx         m_vecVASet;
	int                m_iVATNum;


	//! 场景中当前帧渲染的三角形数目。
	int                m_iRenderTriNum;


	//! 每一帧保存当前屏幕大致中心坐标的三维位置，由客户程序提供的参照变量。
	int                m_iCenterX,m_iCenterY;
	//! 当前主角人物所在的三维位置。
	osVec3D             m_vec3HeroPos;

	//@{ 
	//! 场景中的天气相关数据结构。
	os_weather          m_sCurrentWeather;
	//! 新用到的场景天气数据结构.
	os_weather          m_sNewWeather;
	//! 是否正处于天气变换的状态.
	bool               m_bWeatherChanging;
	//! 换天气需要的时间.
	float              m_fWeatherChangeTime;
	//! 从天气变换到现在过去的时间.
	float              m_fTimeFromChangeW;   

	//! windy 7.17 add
	//OSSoundItem*		m_TGSoundList;

	std::vector<OSSoundItem>	m_TGSoundList;
	std::vector<os_aabbox>		m_TGSoundAABBoxList;

	QuadTree<OSSoundItem>		mSoundQuadTree;
	std::vector<OSSoundPlayingItem>		mCurrentPlaySoundItemList;

	void frame_moveSound(osVec3D* _camPos,float _timeMs);
	//! 播放特效的声音相关的接口
	static I_soundManager*  m_ptrSoundMgr;

	//std::vector<osVec3D>	mPassLineList;
	std::vector<std::vector<osVec3D> >  mPassLineListList;
	//std::vector<osVec3D>	mUnPassLineList;
	// windy add
	static int mrefMapId;
	//! river added @ 2009-6-29: refraction target.
	static int m_iRefractionMapId;
	static LPDIRECT3DSURFACE9  m_pRefractionSurface;
	static LPDIRECT3DTEXTURE9  m_pRefractionTexture;

	//int mRefTargetId;
	//int	mRefMapSurfaceId;
	//static IDirect3DTexture9* mpRefTexPtr;
	static LPDIRECT3DSURFACE9 mpRefMapSurface;
	static LPDIRECT3DTEXTURE9 m_pRefMapTexture;
	static LPDIRECT3DSURFACE9 m_pRefMapDepth;

	static LPDIRECT3DSURFACE9 m_pDepthStencilBackSur;

	/*std::vector<int>	m_VecSoundOffset;
	std::vector<int>	m_VecSoundCnt;*/

	CLightManger			  m_lightManager;

# if __QTANG_VERSION__
	//! River added @ 2009-5-19:为了地图的碰撞信息与服务器和上层一致。
	char*      m_pMapCollInfo;
# endif 


	//! 把当前场景的天气信息收集到当前的天气数据结构中.
	void               gather_curWeather( void );

	/** \brief
	 *  用于判断当前的天气变化是否大到可以重设当前的重局变量.
	 *
	 *  因为重设环境光用到的计算量比较大，如果环境光的变化太小，
	 *  则等到变化到一定的程度后再变化
	 *
	 *  \param _etime 当前帧过去的时间。
	 */
	bool               change_sceneAmbient( float _etime );

	//! 天气间的熔合。
	void               frame_moveWeatherChanging( float _etime );

	//@} 

	//! 得到位置射线与传入tg高度相交的高度数据，用于更精确的得到桥类物品的碰撞信息
	float              get_intersectHeight( osc_TileGroundPtr  _tg,
		                     int _objNum,const int* _objList,osVec3D& _pos );

public:
	//! 得到场景内某个位置上跟地表物品相关的高度信息,如果没有物品,则高度信息为很小的负值.
	float              get_objHeightInfo( osVec3D& _pos ); 

	//! River added @ 2007-6-8:加入地表高度的信息处理。此处的高度，为纯地表高度，即渲染地表的数据高度。
	bool               get_earthHeight( osVec3D& _pos );

	// River @ 2010-6-23:测试在别的地方
	static LPDIRECT3DSURFACE9 mBackUpSurface;


	//@{
	// 场景中的动态光相关的数据.
private:


	//! 对场景中的顶点颜色数据使用环境光系数进行处理。
	void               reset_ambientCoeff( void );


	//! 处理animation Mesh的动态光数据。
	bool               process_aniMeshDlight( void );

	//@} 

	//! 从一个格子坐标得到当前格子对应的渲染polygon.
	BOOL             get_rPolygon( osVec3D* _vec,int _tx,int _tz );

	/** \brief
	 *  当前的polygon的边能否和输入的射线相交。
	 *
	 *  \param _vs        传入的polygon的开始顶点。
	 *  \param _idx       要判断和polygon的哪一条边是否相交。
	 *  \param _ray       射线的方向。
	 *  \param _rayStart  射线的开始位置。
	 */
	bool               ray_intersectPolyEdge( osVec3D* _vs,
		                   int _idx,osVec2D& _ray,osVec2D& _rayStart );

# if WATER_ALPHA
	//! 渲染场景中与水邻接的地图tile块.
	void               render_waterTile( void );
# endif 

	//! 渲染场景中物品的不透明部分。
	bool               render_opacityObjPart( bool _bridgeDraw = false );
	//! 渲染场景中物品的透明部分。
	bool               render_transparentObjPart( void );

private:


	/** \brief
	 *  得到新地图的名字。
	 *
	 *  \param _nMname 传入的文件名，也使用这个字符串指针返回值。
	 *  \param _idr 共有6个方向，从0开始: x,xy,y,-x,-xy,-y
	 */
	bool                get_newMapName( char* _nMname,int _dir );


	/** \brief
	 *  得到当前的相机射线与地面交点所在的TG.
	 */
	osc_TileGroundPtr      get_curCamIn_tground( void );


	/** \brief
	 *  设置场景中用到的Material值。
	 *
	 *  \param _alpha 当前material用到的alpha值。
	 */
	void              set_material( float _alpha );

	//! 当前的碰撞格子画Cursor时,使用地表数据还是使用高度数据.
	bool              weather_useTileHeight( int _x,int _y,osVec3D* _pos );

	//! 根据一个碰撞格子在场景中的位置，得到当前碰撞格子对应的四个顶点数据。
	bool              get_colVertexData( osVec3D* _pos,int _x,int _y );

	//
	// 
private:

	/** \brief
	 *  用于渲染天空和远景的函数.
	 */
	void                render_skyDome( bool _reflect = false );



	/** \brief
	*  判断一条射线是否和一个quad四边形相交,并根据传入的参数，处理得到离相机位置
	*  最近的交点。
	*
	*  \param const osVec3D* _quadVer 传入的四边形顶点，为两个三角形，两个三角形由顶点索引
	*                                 (0,2,1),(1,2,3) 构成 
	*  \param osVec3D& _pickVec     如果相交，并且比传入的矩离离相机近，返回。
	*  \param osVec3D& _camPos      相机的位置。
	*  \param float& _dis           此次相交测试前得到的离相机最近交点的矩离。
	*/
	bool               pick_nearestTri( const osVec3D* _quadVer,const os_Ray &_ray,
		                           osVec3D& _pickVec,osVec3D& _camPos,float& _dis );


	//@{
	//! 鼠标点选出类的函数可以放到一个新的cpp文件内
	/** \brief
	 *  从rayStart开始，处理Chunk的内的桥类物品，如果相交，则返回true.
	 *
	 *  \param int _cnum 传入的chunk数目
	 *  \param const os_tileIdx* 传入的chunk列表
	 */
	bool                  get_raypickBridge( osVec3D& _rayS,osVec3D& _rayDir,float& _resLength  );
		                           
	/** \brief
	 *  新的geomipmap中得到当前鼠标点击所在格子的索引。
	 *
	 *  \param osVec3D& _pickVec 得到的返回结果。
	 *  \param os_tileIdx& _sidx 鼠标点击的屏幕坐标。
	 *  \return bool             如果跟地表有交点，返回true。
	 */
	bool				  get_newpickTileIdx(osVec3D & _pickVec,os_tileIdx& _sidx );

	//@}


	//! 画场景内的每一个声音的包围盒子
	void                  draw_soundBox( void );

public:
	osc_TGManager();
	~osc_TGManager();


	// windy mod
	void	UpDate_reflectionMap();

	//  River @ 2009-12-30: 用于渲染二维界面内用到的相关特效
	//! 高层图层的渲染，用于战斗时的特效，人物，decal等
	void                  render_topLayer( I_camera* _cam,os_FightSceneRType _rtype );

	//! River added @ 2009-6-29:加入refraction map.
	void    update_refractionMap( void );

	//! 渲染场景深度信息
	void    UpDate_DepthMapAndRenderFog();
	/** \brief
	 *  初始化场景，只有初始化场景后，才可以调入和使用场景指针做其它的工作。
	 *
	 *  外界第一次得到场景指针的时候，调用这个函数。
	 */
	bool                init_sceneWork( void );



	/**********************************************************************/
	//
	//  场景的初始化.
	//
	/**********************************************************************/	

	/** \brief
	 *  使用我们的场景调入结构调入场景地图。
	 *
	 */
	virtual bool          load_scene( os_sceneLoadStruct* _sl );

# if __QTANG_VERSION__
	//! 由上层设置地图的碰撞信息,其实这些工作完全可以由客户端完成，为兼容以前的接口。
	virtual bool          set_sceneCollInfo( char* _coll );
# endif 


	//! 是否加载了地图?
	virtual bool          isLoadMap(){ return m_bLoadedMap; }

	
	virtual osVec2D       get_mapSize();
	



# if __COLHEI_MODIFY__
	//! 调入地图的高度和碰撞调节信息。
	bool                 load_colheiAdjData( const char* _chName,os_TileCollisionInfo* _arr );
# endif 

	/** \brief
	 *  创建keyAniMesh.
	 *  
	 *  由中间管道负责创建。
	 */
	int                  create_keyAniMesh( os_aniMeshCreate& _mc );
	//! 释放kenAniMesh.
	void                 release_keyAniMesh( int _id );


	/** \brief
	 *  为解决地图中的瞬移问题加入的接口。
	 *
	 *  \param _mapName 瞬移后地图的名字，可以使用和当前地图同样的名字。
	 *  \param _xCord,_yCord 瞬移后人物的坐标。
	 */
	virtual bool          spacemove_process( const char* _mapName,int _xCord,int _yCord );



	/** \brief
	 *  释放一个场景.
	 *
	 *  释放这个场景中所有的数据,比如地图传送等操作需要整个场景都变化需要
	 *  使用这个函数。
	 *  
	 */
	virtual bool          release_scene( bool _rlseScrPic = true );


	/** \brief
	 *  得到可见性? 
	 *
	 *  
	 */

	/** \brief
	 *  场景的FrameMove.
	 *
	 *  
	 */
	virtual bool         frame_move( os_sceneFM* _fm );

	/** \brief
	 *  得到场景中某一个地图的碰撞信息数据块。
	 * 
	 *  \param _tgIdx     场景中地图的索引，从(0,0)开始。
	 *  \return os_TileCollisionInfo** 返回的是os_TileCollisionInfo的二维数组。
	 *                                 就是os_TileCollisionInfo［128][128] 
	 */
	virtual const os_TileCollisionInfo* get_sceneColData( os_tileIdx& _tgIdx );
	/** \brief
	*  得到场景中某一个地图的高度信息数据块。
	*  windy add 4-27
	*  \param _tgIdx     场景中地图的索引，从(0,0)开始。
	*  \return float*	返回的是65X6的高度信息数组
	*/
	virtual const float*				get_sceneColHeightData(os_tileIdx& _tgIdx );

	//! 从一个开始点和一个dir,计算跟地表或是桥类物品相关的长度信息。
	float                 get_pickRayLength( BOOL& _bridgeInter,
		                   osVec3D& _rayS,osVec3D& _rayDir,osVec3D& _pickVec );

	/** \brief
	 *  渲染当前的地图。
	 *  
	 *  \param _scrPic 如果当前变量为真，则只渲染屏幕内的二维图片，否则渲染场景
	 *                 加二维图片。
	 *  \param _LensPic 表示，渲染的二维图片用LensFlare的渲染状态。
	 *  \param _rtype   渲染场景的模式，为回合制游戏而加入
	 *                  此参数为OS_COMMONRENDER，普通的渲染
	 *                  参数为OS_RENDER_LAYER,分层渲染，先渲染背景，再渲染参战的人物和特效
	 *                  
	 */
	virtual bool          render_scene( bool _scrPic = false,
		                    bool _LensPic = false,os_FightSceneRType _rtype = OS_COMMONRENDER ); 

	/** \brief
	 *  得到当前帧渲染的三角形数目。
	 */
	virtual int           get_renderTriNum( void );

	/** \brief
	 *  得到当前场景中的skinMesh用到的三角形的数目。
	 *
	 *  skinMesh主要用于人物的渲染，所以此处得到的值，为人物的面片数目。
	 */
	virtual int           get_skinMeshTriNum( void );

	/** \brief
	 *  改变场景的天气。
	 *
	 *  \param _weather  新的场景天气数据结构。
	 *  \param _itime    改变天气需要的天气熔合时间,以秒为单位。
	 *  
	 */
	virtual void          change_sceneWeather( os_weather& _weather,float _itime );

	/** \brief
	 *  改变场景的远景显示osa模型.
	 *  
	 *  \param _terr 远山的模型文件名.
	 */
	virtual void          change_sceneFarTerr( const char* _terr );


	/** \brief
	 *  得到当前鼠标点击所在格子的索引。
	 *
	 *  这个索引值是在整张大地图上的格子的索引值。必须由服务器确认才有效。
	 *  
	 */
	virtual bool          get_pickTileIdx( os_pickTileIdx& _dix,
		                    os_tileIdx& _sidx,osVec3D* _intersectPos = NULL );	


	/** \brief
	 *  往场景中加入billBoard.
	 *
	 *  场景中的Billboard在初始化的时候固定位置，以后不能改变。
	 *  
	 *  ATTENTION: 如果是加入了循环播放的公告板，则不能删除。
	 *             运行过程中主要是加入运行一次的公告板动画，播放完成后，会自己删除自己。
	 */
	virtual int           create_billBoard( const char *_bfile,osVec3D& _pos,
		                     bool _createIns = true,float _sizeScale = 1.0f,bool _topLayer = false  );

	//! 在场景内创建不受远剪切面影响的公告板,其它的操作跟公告板的操作一致
	virtual int           create_hugeBillBoard( const char* _bfile,osVec3D& _pos,
		                    bool _createIns = true,float _sizeScale = 1.0f );

	//! 释放一个公告板物品。
	virtual void          release_billBoard( int _id );

	//! 验证billBoardId的有效性。
	virtual bool          validate_bbId( int _id );


	/** \brief
	 *  控制billBoard的位置。一次性运行的公告板，不能控制位置。
	 *
	 *  \param _id 　要设置的公告板的id。
	 *  \param _pos  要设置的公告板的位置
	 */
	virtual void          set_billBoardPos( int _id,osVec3D& _pos );

	//! 设置公告板使用的旋转轴
	void                  set_bbRotAxis( int _id,osVec3D& _axis );

	/** \brief
	 *  即时更新当前id公告板实例的位置
	 *
	 *  用于更新在场景整体的FrameMove后，又想准确得到公告板位置的情况，
	 *  比如武器上用到的公告板。
	 */
	virtual void          update_bbPos( int _id,I_camera* _camptr );



	/** \brief
	 *  设置公告板的播放时间。
	 *  
	 *   \return 返回了要设置公告板的正常播放时间。
	 *   \param _timeScale 要设置的公告板的播放时间与正常播放时间的比例，如果为负值，
	 *                接口只返回公告板的正常播放时间，不设置播放时间。
	 */
	virtual float         set_bbPlayTime( int _id,float _timeScale );


	/** \brief
	 *  删除billBoard.
	 *
	 *  \param _id 删除指定id的billBoard.
	 */
	virtual void          delete_billBoard( int _id,bool _finalRelease = false );

	// River @ 2010-12-24: 从id得到billBoard的文件名.
	const char*           get_bbNameFromId( int _id );


	//! 隐藏当前的billBoard.
	virtual void          hide_billBoard( int _id,bool _hide );
	//! 缩放当前的公告板.river added @ 2010-1-7:
	virtual void          scale_billBoard( int _id,float _rate );


	/** \brief
	 *  客户端的碰撞检测函数.
	 *  
	 *  如果客户端碰撞检测不能通过，则不需要再由服务器进行确认，
	 *  如果能通过，则由服务器确认后进行一系列的动作和操作。 
	 *  
	 */
	virtual bool          collision_detection( const os_tileIdx& _tile );


	/** \brief
	 *  根据一个地图坐标，得到当前地图坐标中心点内对应的三维，如果当前坐标不在场景内，
	 *  则对应的三维坐标的y值为0.0f.
	 */
	virtual bool          get_tilePos( int _x,int _y,osVec3D& _pos,BOOL _precision = FALSE  );

	virtual void	get_collision_info(const osVec3D& _pos,float _radius = 2);
	//! 得到每一个碰撞格子细分成15份的格子三维空间位置.
	virtual bool          get_detailTilePos( int _x,int _y,osVec3D& _pos );

	virtual int			  get_materialIdx(osVec2D& _pos,os_terrainTexSnd& _texsnd );
	/** \brief
	 *  给出场景内的一个xz值，得到此处的地表高度
	 *
	 *  \param osVec3D& _pos x,z的值由_pos内的x,z值给出.返回的高度值在_pos.y内
	 *  \param BOOL _precision 是否精确碰撞,即加入物品高度信息,只有主角人物移动使此变量为true
	 */
	virtual bool         get_detailPos( osVec3D& _pos,BOOL _precision = FALSE );

	/** \brief 
	*	根据一个世界坐标，返回一个DWORD的值，表明这个世界坐标的光照图的颜色。
	*
	*	\param osVec3D& _pos		x,z 有作用,为世界坐标
	*/
	virtual DWORD		get_lightMapColor(const osVec3D& _pos);

	/** \brief
	 *  由传入的世界空间点计算当前世界空间点在哪一个地图上，在地图的哪一个图块上.
	 *  
	 *  \param  _res     使用os_pickTileIdx结构返回我们得到的值.
	 *  \param  _pos   　传入世界空间的顶点位置。
	 */
	virtual bool                   get_sceneTGIdx( 
		                             os_pickTileIdx& _res,const osVec3D& _pos );

	/** \biref
	 *  得到当前视域内的地图的数目和索引.
	 * 
	 *  客户端在寻路算法时会用到这些数据.
	 *  
	 *  \return int     返回在视域内的地图的数目.
	 *  \param  _tgidx  传入的返回地图索引的指针结构,最多返回4个._tgidx应该是os_tileIdx[4].
	 *  
	 */
	virtual int                   get_inViewTGIdx( os_tileIdx* _tgidx );

	//! 得到当前场景中可以看到的格子的数目。
	virtual int                   get_inViewTileNum( void );

	//! 把一个character指针推进到墙后渲染队列中
	virtual void            push_behindWallRenderChar( I_skinMeshObj* _ptr );

	//! 把一个character指针从墙后渲染队列中删除
	virtual void            delete_behindWallRenderChar( I_skinMeshObj* _ptr );


	/** \brief
	 *  用于创建和操作上层需要的SkinMeshObject.
	 *
	 *  \param _smi 用于初始化新创建的skinMeshObject.
	 *  \return     如果创建失败，返回NULL.
	 */
	virtual I_skinMeshObj*  create_character( os_skinMeshInit* _smi,
		                      bool _sceneSm = false,bool _waitForFinish = false );

	//@{
	//  程序中光标相关的接口。
	/** \brief
	 *  使用文件名创建光标。
	 */
	virtual int            create_cursor( const char* _cursorName );
	//! 设置引擎中显示的光标。
	virtual void           set_cursor( int _id, int _centerX = 0, int _centerY = 0 );
	virtual int            get_cursor();
	virtual void           hide_cursor( bool _hide = false );
	//@} 
	

	//! 往管道中推进一个透明渲染物品.
	void                  push_alphaObj( os_meshRender& _mr );

	//! 往管道中推进一个非透明的渲染物品
	void                  push_opacityObj( os_meshRender& _mr );


	/** \brief
	 *  得到地图中的一个顶点是否在可视范围内。
	 *
	 *  由tgMgr使用自己的方法得到。
	 */
	bool                  is_ptInView( osVec3D& _vec );

	/** \brief
	 *  检测sphere,查看是否在场景中.
	 *
	 *  \param _lod  是否使用lod选项，来检测sphere在相机可视范围内,如果使用，
	 *               则远处小的物品不可见。
	 */
	bool                  is_sphereInView( os_bsphere& _sphere,bool _lod = false );

	/** \brief
	 *  得到当前场景中主相机的frustum.
	 */
	os_ViewFrustum*        get_camFrustum( void );

	/** \brief
	 *  得到中间管道指针的函数。
	 *  
	 */
	osc_middlePipe*         get_middlePipe( void )         { return &m_middlePipe; }

	/** \brief
	 *  得到场景中的环境光数据。
	 */
	DWORD                  get_ambientLight( void );


	//@{
	// 动态光相关的接口。
	/** \brief
	 *  创建一个动态光。
	 *
	 *  \return int  如果返回-1,则创建失败。
	 */
	virtual int           create_dynamicLight( os_dlightStruct& _dl );

	/** \brief
	 *  得到一个动态光的各项参数。
	 */
	virtual void          get_dynamicLight( int _id,os_dlightStruct& _dl );


	/** \brief
	 *  FrameMove一个动态光。
	 */
	virtual void          set_dynamicLight( int _id,os_dlightStruct& _dl );

	//! 设置动态光的位置。
	virtual void          set_dLightPos( int _id,osVec3D& _pos );

	/** \brief
	 *  删除一个动态光。
	 */
	virtual void          delete_dynamicLight( int _id,float _fadeTime = 0.0f );

	/** \brief
	 *  改变场景的环境光系数。
	 *
	 *  \param _r,_g,_b  场景中环境光的系数，使用这三个值缩放场景中的环境光。
	 */
	virtual void          change_amibentCoeff( float _r,float _g,float _b );

	//@}


	//@{
	// 简单阴影相关的接口，人物,npc和怪物下面有一个黑色的小圈。
	virtual int           create_fakeShadow( osVec3D& _pos,float _scale = 1.0f );
	virtual void          set_shadowPos( int _id,osVec3D& _pos );
	virtual void          delete_shadowId( int _id );
	virtual void          hide_shadow( int _id,bool _hide = true );
	//@} 

	//! 创建和删除场景地表上的Decal
	virtual int           create_decal( os_decalInit& _dinit );
	virtual void          delete_decal( int _id );
	virtual void          reset_decalPos( int _id,osVec3D& _pos );


	//@{
	//! 场景中开始和结束下雨效果。
	virtual void          start_sceneRain( os_rainStruct* _rs = NULL );
	virtual void          stop_sceneRain( void );
	/** \brief
	 *  场景内开始和结束下雪的效果
	 *
	 *  \param BOOL _start 如果真则开始下雪，如果为false,结束下雪的效果
	 *  \param int _grade  分为0,1,2三个级别。级别越高，则雪花越多。
	 */
	virtual void          scene_snowStorm( BOOL _start = TRUE,int _grade = 0 );

	//@} 


	//{@ windy 6-21 add
	//! 开始播放闪电,
	/*!\param _times ,闪电时间长度
	\param frequency 一秒钟闪几次.
	\param flashTime 每次闪电,所占时间 [0-1]之间数
	\param _color 闪电颜色*/
	virtual void	start_sceneLight(float _times,float frequency,int flashtype,
		                float flashTime,osColor _color = D3DXCOLOR(1.0f,1.0f,1.0f,1.0f));
	virtual void stop_sceneLight();
	//@}


	//@{
	//  加入场景中的二维屏幕图片显示需要的接口。
	//  二维图片的显示需要由客户程序自己进行排序，把需要放到下边的图片优先推进行到场景
	//  中进行显示。
	
	/** \brief
	 *  创建一个二维显示需要的纹理
	 *　
	 *  场景切换的时候，所有的纹理会由场景程序自动的释放。
	 */
	virtual int            create_scrTexture( const char* _texname );

	//! 得到一个纹理
	virtual LPDIRECT3DTEXTURE9   get_sceneScrTex( int _id );

	//! test注册一个纹理,返回id
	int			register_texid( LPDIRECT3DTEXTURE9 _pTexture );
	void				unregister_texid( int _texid );
	int		register_targetAsTex( int _targetIdx );

	//! 创建一个纹理
	int					create_texture( int _width, int _height, LPDIRECT3DTEXTURE9* _pTexture );

	//syq_mask 创建渲染目标
	int                        create_renderTarget( int _width, int _height, D3DFORMAT _format = D3DFMT_A8R8G8B8 );

	//syq_mask 获得渲染目标: 
	LPDIRECT3DSURFACE9         get_renderTarget( int _id );
	LPDIRECT3DTEXTURE9      get_renderTexture( int _id );

	/** \brief
	*  释放一张贴图。
	 */
	virtual void           release_scrTexture( int _texid );


	/** \brief
	 *  把需要显示的数据结构推进到场景中进行显示。
	 *
	 *  这个数据被推进行到场景中的数据结构中，在显示完场景中的三维元素后，
	 *  开始显示这些二维的元素。
	 */
	virtual void           push_scrDisEle( const os_screenPicDisp* _dis );
	//@}



	//@{
	// 
	// 场景中动画相关的函数.此处的动画非骨骼动画.
	// 动画文件的格式为*.osa格式的动画.
	/** \brief
	 *  场景中加入动画文件,如果加入的动画文件不在当前相机的视野内,则不需要渲染新加入的
	 *  动画.
	 *
	 *  \param _mc 创建动画数据的结构,填充这个结构,创建我们的动画数据.
	 */
	virtual int           create_osaAmimation( os_aniMeshCreate* _mc,bool _topLayer = false );
	

	//! 开始动画
	virtual void          start_animationMesh( int _id );

	//! fade或是fadeIn当前的osa动画.
	virtual void          start_fadeOsa( int _id,
		                    float _fadeTime,bool _fadeOut = true,float _minFade = 0.0f );

	//! 隐藏当前的osa动画
	virtual void          hide_osa( int _id,bool _hide );


	//! 需要验证每一个动画物品id的合法性。
	virtual bool          validate_aniMeshId( int _id );


	//! 设置某一个动画的位置。
	virtual void          set_aniMeshPos( int _id,osVec3D& _pos );

	//! 设置一个动作的旋转方向.
	virtual void          set_aniMeshRot( int _id,float _angle );
	//! 缩放当前的osa动画
	virtual void          scale_aniMesh( int _id,float _rate,float _time );


	//! 即时更新osa的位置,主要用于武器特效的
	void                  update_osaPosNRot( int _id );

	//! 更新osa的渲染状态，使下一次渲染此osa的时候能够被渲染
	void                  update_osaRenderState( int _id );
	//! 更新particle的渲染状态，使下一次渲染此par的时候能被渲染
	void                  update_parRenderState( int _id );



	/** \brief
	 *  设置osa动画的播放时间。
	 *  
	 *   \return 返回了要设置osa动画的正常播放时间。
	 *   \param _timeScale 要设置的osa动画的播放时间与正常播放时间的比例，如果为负值，
	 *                接口只返回公告板的正常播放时间，不设置播放时间。
	 */
	virtual float         set_osaPlayTime( int _id,float _timeScale );


	/** \brief
	 *  设置一个动画的世界矩阵。
	 *
	 *  这个接口主要用于一些特效的播放，需要在游戏层填充世界矩阵数据。
	 */
	virtual void          set_aniMeshTrans( int _id,osMatrix& _transMat );

	//! 删除一个动画.
	virtual void          delete_animationMesh( int _id,bool _sceC = false );

	const char*           get_aniNameFromId( int _id );
	//@} 

	//! 上层播放整体个特效的接口
	virtual DWORD         create_effect( const char* _ename,
		                    osVec3D& _pos,float _angle = 0.0f,
							BOOL _instance = TRUE,float _scale = 1.0f,
							float* _effectPlaytime = NULL );

	//! 得到effect对应的boundingBox,其中boxsize是经过scale后的size.此box无旋转
	virtual bool          get_effectBox( DWORD _id,osVec3D& _pos,osVec3D& _boxSize );

	//! 得到effect的包围盒，有旋转方向的包围盒.
	virtual const os_bbox*   get_effectBox( DWORD _id );


	//! 内部使用的接口,用于创建场景内的特效,场景内的特效,不创建动态光.
	DWORD                 create_effect( const char* _ename,
		                    osVec3D& _pos,float _angle,float _scale,bool _inScene );

	//! River @ 2010-1-14:上层可以到一个effect的keyTime.
	virtual float          get_effectKeyTime( DWORD _id );

	//! 在上层删除一个特效
	virtual void          delete_effect( DWORD _id,bool _finalRelease = false );
	//! 对特效进行缩放,用于对场景内的道具进行缩放动画，但此缩放仅限于osa动画和公告板，粒子场景不能动态缩放
	virtual void          scale_effect( int _id,float _rate,float _time );
	//! 对特效进行fadeOut
	virtual void          fade_effect( int _id,float _fadetime,bool _fadeOut = true );

	//! River @ 2010-8-24:对特效的播放速度进行缩放。
	virtual void          scale_effectPlaySpeed( int _id,float _speedRate );


	/** \brief
	 *  设置特效的alpha值,只有osa特效才能正确的设置alpha值。alpha值介于0.0和1.0f之间。
	 *
	 *  \param int _id   特效对应的id.
	 *  \param float _alpha 特效要使用alpha值
	 */
	virtual void          set_effectAlpha( int _id,float _alpha );


	//! 隐藏或显示当前id的effect
	virtual void		  hide_effect(int _id,bool _bHide = TRUE );


	//! 上层设置特效的位置和旋转方向
	virtual void          set_effectPos( DWORD _id,osVec3D& _pos,
		                    float _angle,float _speed = 1.0f,bool _forceUpdate = false );
	//! 验证一个effect是否已经在内部被删除
    virtual BOOL          validate_effectId( DWORD _id );

	//@{
	//  客户端用到的特定数据，引擎只负责调入并由上层调用，引擎内不做处理。
	//! 得到特效播放的位置类型 0:包围盒比例播放。 1:攻击位置播放。2:被攻击位置播放。
	DWORD    get_effectPlayPosType( int _id );
	//! 得到特效播放的包围盒比例，返回的值在0.0到1.0之间。
	float    get_effectPlayBoxScale( int _id );
	//@}


	/** \brief
	 *  加入一个可以显示鼠标点击位置的标志。
	 */
	virtual void	      draw_cursorInScene( os_sceneCursorInit& _scInit );




	//@{
	//  粒子相关的函数接口.创建,设置和删除.
	//! 创建一个particle实例.
	virtual int           create_particle( os_particleCreate& _pcreate,BOOL _inScene,bool _topLayer = false );
	//! 隐藏当前的particle.
	virtual int           hide_particle( int _id,bool _hide );

	bool                   is_loopPlayParticle( int _id );

	//! 设置particle的位置.
	virtual void          set_particlePos( int _idx,osVec3D& _pos );
	virtual void          set_particlePos( int _idx,osVec3D& _pos,bool _forceUpdate );

	//! 设置粒子发散的开始和结束线段点
	virtual void          set_particleLineSeg( int _id,osVec3D& _start,osVec3D& _end );



	/** \brief
	 *  设置粒子动画的播放时间。
	 *  
	 *   \return 返回了要设置粒子动画的正常播放时间。
	 *   \param _timeScale 要设置的粒子动画的播放时间与正常播放时间的比例，如果为负值，
	 *                接口只返回公告板的正常播放时间，不设置播放时间。
	 */
	virtual float         set_parPlayTime( int _id,float _timeScale );


	//! 检测particleId的可用性，如果不可用，则上层需要处理当前的particle Id.
	virtual bool          validate_parId( int _id );

	//! 删除一个particle.
	virtual void          delete_particle( int _idx,bool _finalRelease = false );
	//@} 

	//! 减少特效删除与创建操作
	const char*           get_parNameFromId( int _id );


	//@{
	//  电弧相关的特效。
	/** \brief
	 *  创建一个电弧。
	 *
	 *  可以同时使用最多两个电弧对应一个id
	 */
	virtual int           create_eleBolt( os_boltInit* _binit,int _bnum );
	//! 重新设置弧。
	virtual void          reset_eleBolt( os_boltInit* _bset,int _idx );
	//! 删除一个电弧资源。
	virtual void          delete_eleBolt( int _id );
	//@}



	//@{
	/** \brief
	 *  Draw text on the screen.  
	 *
	 *  这个函数是引擎显示文字信息的主要的函数,必须支持Unicode.这个函数必须
	 *  用在beginScene和endScene函数之间.
	 */
	virtual int          disp_string( os_stringDisp* _dstr,int _dnum,int _fontType = 0,bool _useZ = false ,bool _bShadow = false, RECT* _clipRect = NULL );

	//! 立即在屏幕上显示字串，参数跟disp_string参数一致。
	virtual int          disp_stringImm( os_stringDisp* _dstr,int _fontType = 0,os_screenPicDisp* _cursor = NULL );


	//! 渲染有颜色解晰功能的文字字串。
	virtual int          disp_string( os_colorInterpretStr* _str,int _fontType = 0,bool _useZ = false, bool _bShadow = false, RECT* _clipRect = NULL );
	//@}
	//@{
	// Windy mod @ 2005-9-20 15:05:38
	//Desc: 关于Lens Flare的接口
	 void	start_LensFlare() ;
	 void	stop_LensFlare() ;

	///从文件中加载lens flare
	
	void	clear_LensFlare() ;

	void	set_RealPos(osVec3D & _vec3Pos);
	# if __LENSFLARE_EDITOR__
	///以下是编辑器的接口
	virtual void	Load_LensFlare(const char * _fname) ;
	virtual void	set_LensFlarePart(int nPartID,LFMask _Mask,FlarePart & _flarePart) ;
	virtual void	get_LensFlarePart(int nPartID,LFMask _Mask,FlarePart & _flarePart) ;

	virtual int		get_LensFlareNum();

	virtual void	add_LensFlarePart();
	virtual void	del_LensFlarePart(int iPartID);

	virtual void	save_LensFlare(const char * _fname) ;
#endif
	//@}





# if __EFFECT_EDITOR__
	//! 得到当前场景的天气。
	virtual void          get_sceneWeather( os_weather& _weather );

	//! 强制结束一个动画.
	virtual void          stop_animationMesh( int _id );

	//! 设置某一个动画相关的参数.
	virtual bool          set_animationMesh( int _id,os_aniMeshCreate* _mc );
# endif 

	//@{
	//  修改地图的高度信息和碰撞信息的接口。
# if __COLHEI_MODIFY__
	/** \brief
	 *  修改主角人物所在位置的高度信息，os_sceneFM内的m_iCx,m_iCy变量表示的位置。
	 *
	 *  \param float _mod  要修改的高度，可正可负。
	 */
	virtual bool          modify_heightInHeroPos( float _mod );

	//! 把鼠标点击的位置设置为跟原来相反的可通过性。
	virtual bool          modify_colAtMousePick( os_pickTileIdx& _ps ,BOOL bUseParm=FALSE,bool bPass=false);
	
	//! 存储修改后的碰撞信息文件,以后批处理融合到地图文件中。
	virtual bool          save_colheiModInfo( FILE*& _file,int _x,int _z );

# endif
	//@} 



	//! 调试用函数，画一个bounding Box.
	virtual void          draw_bbox( os_bbox* _box,DWORD _color,bool _solid = false );
# ifdef _DEBUG
	//! 用于对下层的变量进行输出.
	virtual void          debug_test( void );

# endif 

	//! 调试用函数，画一条线
	virtual void          draw_line( const osVec3D* _verList,int _vnum,
		                   DWORD _color = 0xffffffff,float _width =1.0f );

	//! 用于设备lost和回复的功能
	static void           on_deviceLost( LPDIRECT3DDEVICE9 pDev );
	static void           on_deviceRestor( LPDIRECT3DDEVICE9 pDev );


# if __ALLCHR_SIZEADJ__
	//! 使用这个接口，可以调整整个场景中所有skinMesh物品的大小，
	//! 此处的调整为统一调整。包括树和草相关的物品。
	virtual void          set_allCharSizeAdj( osVec3D& _sizeAdj );
# endif 

	/** \brief
	 *  从一个坐标得到当前TGMgr中当前坐标的osc_TileGroundPtr.
	 *
	 *  \param _x,_y 两个坐标为TileGround的索引。
	 *  
	 *  River @ 2006-3-8: 变为公有函数，主要用于decal的计算。
	 */
	osc_TileGroundPtr      get_tground( int _x,int _y );


	/** \brief
	 *  根据一条射线来得到是否跟地表相交，以及相交点的位置。
	 *  
	 *  使用跟鼠标点选地表一样的核心算法。
	 *  \param osVec3D& _rayStart  射线的开始位置。
	 *  \param osVec3D& _rayDir    射线的方向
	 *  \param osVec3D& _resPos    如果相交，此值为返回的交点位置。
	 *  
	 */
	bool                  get_rayInterTerrPos( osVec3D& _rayStart,osVec3D& _rayDir,osVec3D& _resPos );  

	/** \brief
	 *  输入一条线段,来确认此线段是否跟地表相交,以及交点的位置.
	 *
	 *  这个函数主要用于优化相机跟地表的碰撞测试,核心算法来自函数get_rayInterTerrPos
	 *
	 *  \param osVec3D& _ls 线段的开始位置.
	 *  \param osVec3D& _le 线段的结束位置.
	 *  \param osVec3D& _resPos 如果相交,返回相交的结束位置.
	 *  \return bool            返回true,则跟地表相交.
	 */
	bool                  get_lineSegInterTerrPos( osVec3D& _ls,osVec3D& _le,osVec3D& _resPos );

	/** \brief
	 *  计算一个包围球跟地表的碰撞检测.
	 *
	 *  算法结果描述:
	 *  每一次计算bsphere的碰撞时,需要传入上一次没有碰撞的bsphere所在的位置.如果当前的bsphere跟
	 *  地表之间有碰撞,则找到此bsphere最可能到达当前的位置,而又没有跟地表碰撞的位置.
	 *  
	 *  \return bool 如果碰撞,返回true,否则返回false.
	 *  \param  os_bsphere* _bs  要跟地形的碰撞信息做碰撞检测的bounding sphere
	 */
	bool	              get_sphereInterTerrPos( os_bsphere* _bs );

	bool				load_SoundInfo(const char* fileName);

	/** \brief
	 *  当前传入的地图是否已经被调入内存。
	 */
	bool                is_tgLoaded( int _x,int _y );

		
//--------------------------------------------------------------------------------
//@{ tzz added for galaxy shader effect
//--------------------------------------------------------------------------------

	//! whole scene growing manager class
	static CGrowEff		m_growEff;

	/*!	\fn virtual void gala_actGrowScene(BOOL _bActivate = TRUE);
	*	\brief : activate the grow scene after been initialized
	*
	*	\param _bActivate : switch of active
	*	\return void
	*/
	virtual void gala_actGrowScene(BOOL _bActivate = TRUE) ;

	/*! \fn virtual void gala_initGrowScene(ini _iScreenWidth,int _iScreenHeight) ;
	*	\brief : initialize the grow scene 
	*
	*	\param _iScreenWidth	: current screen Width
	*	\param _iScreenHeight	: curren screen Height
	*	\return void
	*/
	virtual void gala_initGrowScene(int _iScreenWidth,int _iScreenHeight) ;

	//! whole scene real HDR manager class
	static CHDREff		m_realHDREff;

	//! initialize the hdr data
	virtual void init_realHDRScene(int _iScreenWidth,int _iScreenHeight) ;

	//! activate the HDR scene effect
	virtual void act_realHDRScene(BOOL _bActivate = TRUE);

	virtual void act_waterEff(BOOL _bAct = TRUE);

	//! enable light 
	virtual void enable_light(BOOL _bEnable);

	//! set the day time for light
	virtual void set_dayTimeForLight(size_t _dayTime);

	//! set the day time for light
	virtual void set_constLight(const osColor& _color,const osVec3D&,float _range);




# if  GALAXY_VERSION

	/*! \fn virtual void gala_genExplode(osc_mapTerrian::gala_exlode* _explode) = 0;
	*	\brief : generate a explode effect
	*
	*	\param _explode		:  struct of exploding effect
	*	\return void
	*/
	virtual void gala_genExplode(gala_explode* _explode);

	/*! \fn virtual void gala_genBullet(gala_spaceHole* _bullet) = 0;
	*	\brief : generate a bullet effect
	*
	*	\param _bullet		:  struct of bullet effect
	*	\return void
	*/
	///virtual void gala_genBullet(gala_spaceHole* _bullet) ;
	
	/*! \fn virutal void gala_delSpaceHole(int	_index);
	*	\brief : delete the shader effect of bullet
	*	
	*	\param _index		: index of bullet effect
	*	\return void		
	*/
	virtual void gala_delSpaceHole(int _index);

	/*! \fn virtual void gala_refreshSpaceHole(gala_spaceHole* _hole)
	*	\brief : refresh the space hole position and direction
	*
	*	\param _bullet : bullet struct
	*	\return void
	*/
	virtual void gala_refreshSpaceHole(gala_spaceHole* _hole);

	/*! \fn virtual void gala_renderHole(int _iScreenWidth,int _iScreenHeight,int _iWidthNum,int _iHeightNum);
	*	\biref : begin the space hole effect
	*	
	*	\param _iScreenWidth	: current screen Width
	*	\param _iScreenHeight	: curren screen Height
	*	\param _iWidthNum		: horizontal grid Number
	*	\param _iHeightNum		: vertical grid Number
	*	\param _iStartX		: rotating center x
	*	\param _iStartY		: rotating center y
	*/
	virtual void gala_renderHole(int _iScreenWidth,int _iScreenHeight,int _iWidthNum,int _iHeightNum,int _iStartX,int _iStartY) ;

	/*! \fn virtual BOOL gala_isRenderHole(void);
	*	\biref : is render hole if yes return TRUE;
	*
	*	\param void
	*	\return BOOL	: TRUE if rendering hole;
	*/
	virtual BOOL gala_isRenderHole(void);

	/*! \fn virtual void gala_initGrowScene(ini _iScreenWidth,int _iScreenHeight) ;
	*	\brief : initialize the grow scene 
	*
	*	\param _iScreenWidth	: current screen Width
	*	\param _iScreenHeight	: curren screen Height
	*	\return void
	*/
	virtual void gala_initGrowScene(int _iScreenWidth,int _iScreenHeight) ;

	/*!	\fn virtual void gala_actGrowScene(BOOL _bActivate = TRUE);
	*	\brief : activate the grow scene after been initialized
	*
	*	\param _bActivate : switch of active
	*	\return void
	*/
	virtual void gala_actGrowScene(BOOL _bActivate = TRUE) ;

	/*!	\fn virtual void gala_actHeroCorona(BOOL _bActivate = TRUE);
	*	\brief : draw the hero around corona
	*
	*	\param _bActivate : switch of active
	*	\return void
	*/
	virtual void gala_actHeroCorona(BOOL _bActivate = TRUE) ;

	/*!	\fn void gala_showBlood(const osVec2D& _craftPos,float _fCraftScale,float _fMaxHP,float _fCurrHP)
	*	\brief show the blood process of craft in game
	*
	*	\param _craftPos	: the screen position of craft
	*	\param _fCraftScale	: the mesh of craft scale
	*	\param _fMaxHP		: the max HP of npc air craft
	*	\param _fCurrHP		: current HP of NPC air craft
	*/
	virtual void gala_showBlood(const osVec2D& _craftPos,float _fCraftScale,float _fMaxHP,float _fCurrHP);

private: //some variable to render scene

	//! render manager to render the space hole
	static CGridRect				m_gridRect;


	//! the container of blood
	static std::vector<CBlood>		m_vecBlood;

	//! the index of current active blood process
	int						m_iCurrBlood;

	//! display the blood process
	void DisplayBlood(void);

	//! draw corona around hero?
	BOOL					m_bCorona;

	/*! \class CProxyGrid
	*	\brief : this class will hole the CGridRect(member m_gridRect)
	*			in osc_TGManager::render_scene function, and will call 
	*			the CGridRect::SetReadyToRender in destructor
	*/
	class CProxyGrid{

		//! reference class
		CGridRect&	m_refGrid;

		//! private defualt constrcutor to prevent
		//! default construct
		CProxyGrid(void);
	public:
		CProxyGrid(CGridRect& _grid):m_refGrid(_grid){}

		//! destructor to call some function
		~CProxyGrid(void)
		{
			if(m_refGrid.IsFirstRender()){
				m_refGrid.SetReadyToRender();
			}
		}
	};

# endif //GALAXY_VERSION
# if __EFFECT_EDITOR__
	//!设计osa模型动画播放速度
	virtual void 			set_OSAPlaySpeed(float _speed) {gOSAPLAYERSpeed = _speed;};

# endif 
//--------------------------------------------------------------------------------
//@}
//--------------------------------------------------------------------------------

};





//! 包含inline函数实现文件。
# include "fdFieldMgr.inl"

//! 背景音乐所在的目录
# define  BKSOUND_DIR    "sound\\"



# endif // # define __FDFIELDMGR_INCLUDE__