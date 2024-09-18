/////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: osMesh.h
 * 
 *  His:      River created @ 2003-12-27
 *
 *  Desc:     中间管道可接受的独立的渲染数据类型.
 *  
 *  
 *  "罗马贵族总有一种强烈的竞争意识,这种不甘落后的意念构成了巨大的动力和自豪感。
 *   正是这种不愿让自己家族衰落的动力支撑着他们。"
 *   
 */
/////////////////////////////////////////////////////////////////////////////////////////////////

# pragma once

# include "osShader.h"
# include "../../interface/osInterface.h"
# include "../../exLib/sceneEx.h"
# include "osAniMesh.h"


//! 天空最远的矩离
# define  FAR_TERRMAXSIZE   350.0f


//! 一个屏幕上最多可存在的mesh种类数目.
# define  MAXMESHNUM_INSCREEN    230

# define  MAXSUBSET_PERMESH      12 
# define  MAXMESH_FILENAMELEN    64

//! 定义每一个64*64的小地图最多可以使用osa文件的数目。
# define  MAX_OSANUM_PERTG       224

//! 定义每一个tileGround最多可以使用的mehsInstance的数目。
# define  MAXMESHINS_PERSCE      96

//! 定义场景中的特效最多可以使用的osa文件的数目。
# define  MAXEFFECT_OSANUM      1024

//! 整个场景中最多可以使用AniMesh的数目。所有的aniMeshIns都使用这个数组中的引用。
# define  INIT_ANIMESHSIZE      384

struct  os_ViewFrustum;
class   osc_middlePipe;



//! 手工得到staticMesh的顶点，索引缓冲区，然后于得到Mesh的attributeTable,然后调用DIP.
# define CUSTOM_MESHDRAW     0


//! 影响一个物品的动态光数据结构
struct os_meshDLight
{
	//! 此处的byte值索引了对应在动态光管理器内的灯光信息
	char      m_btDLIdx[MAX_LIGHTNUM];

public:
	os_meshDLight()
	{
		m_btDLIdx[0] = -1; 		
	}

	BOOL         is_usedDl( void )
	{
		if( -1 == m_btDLIdx[0] )
			return FALSE;
		else
			return TRUE;
	}
	void         reset_dl( void )
	{ 
		for( int t_i=0;t_i<MAX_LIGHTNUM;t_i ++ ) 
			m_btDLIdx[t_i] = -1; 
	} 

};


/// 渲染一个Mesh需要填充的结构。
struct os_meshRender
{
	int          m_iMeshId;
	float        m_fAngle;
	osVec3D      m_vec3Pos;
	
	//! 一个静态Mesh可以受一个静态的点光源的影响,用于更精确的计算场景内物品的光照效果
	os_meshDLight m_sDLight;

	//! 渲染此mesh时，是否使用自定义的点着色,如果为零，则不使用此颜色。
	DWORD         m_dwTerrColor;

	//! 模型的缩放信息
	osVec3D      m_vec3Scale;


public:
	os_meshRender()
	{
		m_iMeshId = -1;
		m_dwTerrColor = 0;
		m_vec3Scale.x = m_vec3Scale.y = m_vec3Scale.z = 1.0f;
	}
	void reset( void ) 
	{ 
		m_iMeshId = -1;
		m_dwTerrColor = 0;
		m_vec3Scale.x = m_vec3Scale.y = m_vec3Scale.z = 1.0f;
	}
};


typedef std::vector<os_meshDLight>  VEC_meshDLight;


typedef std::vector<os_meshRender>   VEC_meshRender;


typedef std::vector<D3DXATTRIBUTERANGE>  VEC_meshAttribute;

/// 独立的mesh渲染数据类型, 当前的Mesh是对d3dxMesh的包装.
class osc_Mesh 
{
	friend class osc_meshMgr;
public:
	osc_Mesh( void );
	~osc_Mesh( void );

	/** 从文件初始化一个Mesh
	 *
	 *  \param _fname 要创建的文件的名字.
	 *  \param _meshMem 如果不为空,为此mesh文件对应的Ex文件
	 */
	bool              create_meshFromFile( const char* _fname,
		                     LPDIRECT3DDEVICE9 _dev, BYTE* _meshMem = NULL );

	/// 在动态调入和释放mesh时，这个函数常会被用到
	void              release_mesh( void );

	/// 渲染当前的Mesh
	bool              render_mesh( os_meshRender* _mr,bool _alpha = false  );

	 /// 得到当前的mesh是否在使用中
	bool              is_inUse( void );

	//! 得以当前Mesh的三角形数目.
	int               get_meshTriNum( void );

	//! 得到当前Mesh对应的文件名。
	const char*       get_meshFName( void )       { return m_strMeshName; }
	DWORD             get_hashVal( void )         { return m_dwHash; } 

	//! 得到当前Mesh是否存在alpha部分，如果存在，则渲染时，需要排序。
	bool              has_alphaSubset( void )     { return m_bAlphaMesh; }
	//! 当前mesh是否存在非透明的部分，如果不在在，则渲染非透明物品时，不需要参与
	bool              has_opacitySubset( void )   { return m_bOpacityMesh; } 

	//! 得到当前Mesh的sphere半径。
	float             get_bsRadius( void )        { return m_bsphere.radius;}

	//! 得到mesh的bounding Box.
	os_aabbox*        get_meshBBox( void )        { return &this->m_bbox; }

	//! 输入一条射线,看是否和*.x的mesh文件相交.
	bool              intersect_mesh( osVec3D& _rayPos,
		                   float _dis,osVec3D& _rayDir,float* _resDis = NULL );

	//! 得到当前的物品是否是关键物品
	bool              is_keyObj( void )           { return m_bVipObj; } 

	//! 当前的物品是否是树类透明部分不需要写入ZBuffer的物品
	bool              is_treeObj( void )          { return m_bTreeObj; }

	//! 是否是桥类物品.
	BOOL              is_bridgeObj( void )        { return m_bBridgeObj;}
	//! 是否受体积雾影响
	BOOL              is_AcceptFogObj( void )        { return m_bAcceptFog;}
	//! 是否是无alpha无碰撞
	BOOL              is_NoAlphaObj( void )        { return m_bVipObj;}

	//! 得到faceNormal.
	bool              get_faceNormal( int _faceIdx,osVec3D& _normal );

	//! 设置当前的meshIn memory.
	void              set_inMemory( void ) { m_bInMemoryForEver = true; } 
	//! 得到当前的mesh in memory状态。
	BOOL              get_inMemory( void ) { return m_bInMemoryForEver; } 

private:
	//! 存放真正的渲染数据.
	LPD3DXMESH        m_ptrPMesh;

	//! 当前Mesh用到的ShaderId,就当前mesh的subset数目。
	int               m_iShaNum;
	int               m_vecShaderId[MAXSUBSET_PERMESH];


# if CUSTOM_MESHDRAW
	//! 自画xMesh需要的资源，包括attributeTable和需要的顶点和索引缓冲区指针
	VEC_meshAttribute  m_vecMeshAttribute;
	LPDIRECT3DVERTEXBUFFER9  m_ptrMeshVBuf;
	LPDIRECT3DINDEXBUFFER9   m_ptrMeshIBuf;
# endif 
	
	//! 当前的mesh的shaderId是否是使用了纹理alpha的mesh.
	bool              m_vecAlphaSubset[MAXSUBSET_PERMESH];

	//! 当前的mesh中是否有透明面片。
	bool              m_bAlphaMesh;
	//! 当前的Mesh中是否有非透明的面片
	bool              m_bOpacityMesh;

	//! 当前Mesh的名字。
	char              m_strMeshName[MAXMESH_FILENAMELEN];
	DWORD             m_dwHash;

	static            LPDIRECT3DDEVICE9  m_pd3dDevice;


	//@{
	//  River Mod @ 2005-6-3: 加入了以下两项,这两项需要在版本为2的ex文件中体现
	/** 当前的mesh是否是关键物品.
	 *
	 *  关键物品的定义为:只要此物品所在的格子可见,此物品就可见.在相机移动的过程中,
	 *  物品不会淡入淡出.
	 */
	BOOL             m_bVipObj;

	// 是否是树类的物品，渲染透明部分时，不需要写入Zbuffer
	BOOL             m_bTreeObj;

	// River mod @ 2007-4-11:桥类的物品,计算精确的碰撞检测.
	BOOL             m_bBridgeObj;

	//! 标志物品是否受体积雾影响.
	BOOL             m_bAcceptFog;
	


	/** 当前的物品是否受只受方向光的影响.
	 *
	 *  只受方向光影响的物品,会的场景中的方向做为物品的环境光来使用,物品没有明暗面.
	 *  用于测试显示场景中的草,花等物品.
	 */
	BOOL             m_bUseDirLAsAmbient;
	//@} 

	//! River @ 2010-12-30:此物品常驻内存不释放，为了调入大场景更快速
	BOOL             m_bInMemoryForEver;

private:
	//! 创建当前Mesh用到的Shader.

	//! 当前mesh的AABB Bounding Box.需要预处理的单元。
	os_aabbox       m_bbox;

	//! 当前mesh的Bounding Sphere.
	os_bsphere      m_bsphere;

	//! River @ 2010-3-30:如果是桥类物品，则处理此物品所有面的normal.
	osVec3D*        m_vecFaceNormal;

};
typedef std::vector<osc_Mesh> VEC_mesh;


class   osc_meshMgr;
class   osc_effect;

/// 对osaMesh类进行包装，使可以处理天空和远景。
class osc_osaSkyTerr : public ost_Singleton<osc_osaSkyTerr>
{
private:
	//! 使用两个数组，使可以使用两种天空和远景进行切换
	osc_aniMesh       m_sSkyMesh[2];

	//{@ windy add 6-20
	osc_aniMesh       m_sSkyMesh1;
	osc_aniMesh       m_sSkyMesh2;
	osc_aniMeshIns	  m_SkymeshIns1;
	osc_aniMeshIns	  m_SkymeshIns2;
	char              m_szSky1[128];
	char              m_szSky2[128];

	osc_aniMesh       m_sBoltMesh;
	osc_aniMeshIns	  m_BoltmeshIns;
	char              m_szBolt[128];
	float			  m_szBoltRate;
	float			  m_fBoltAngle;
	osColor			  m_BoltColor;
	float			  m_BoltHeight;

	//@}
	osc_aniMesh       m_sTerrMesh;
	osc_aniMeshIns	  m_sTerrMeshIns;
	char              m_szSky0Name[128];
	char              m_szSky1Name[128];

	char              m_szTerrName[128];
	//char              m_szTerr1Name[128];
	//@{
	// Windy mod @ 2005-9-11 9:58:13
	//Desc: 
	//osc_aniMesh     m_sSkyMesh3;
	//char            m_szSkyName3[128];
	//!远景第三层天空用到的是不同的Projmat
	//D3DXMATRIXA16   m_sky3matProj,
	//				  m_oldmatProj;
	//@}

	osc_meshMgr*      m_ptrMeshMgr;

	//! 当前激活的天空和远山
	int               m_iCurActiveSkyIdx;
	int               m_iCurActiveTerrIdx;

	//! 天空和远山熔合时，第二层的透明度。
	float             m_fSkyAlphaVal;
	float             m_fTerrAlphaVal;

	//! 天空旋转的角度
	float             m_fSkyRotAgl;
	float			  m_fSkyRotAgl1;
	float			  m_fSkyRotAgl2;

private:
	//! 根据不同的状态来渲染天空，可能需要两种天空或是远山的熔合。
	void              set_skyRenderState( void );

	//! 对天空和远景的熔合数据进行处理。
	void              frame_moveSkyTerr( bool _reflect );

	//! 渲染天空.
	void              render_bkSky( osVec3D& _cen,bool _reflect );

	//! 渲染远景.
	void              render_bkTerr( osVec3D& _cen,float _terrScale );

	//! 建构和晰构.
	osc_osaSkyTerr();
	~osc_osaSkyTerr();

public:
	//! 得到一个osc_osaSkyTerr的Instance指针
	static osc_osaSkyTerr*  Instance( void );
	
	//! 删除一个osc_osaSkyTerr的Instance指针.
	static void             DInstance( void );

	bool                    init_skyTerr( osc_meshMgr* _meshMgr, const char* _skyName,const char* _terrName);

	//! 创建新的天空,天空则同目前的做alpha过渡
	bool                    create_newSky( const char* _skyName,const char* _skyName1,const char* _skyName2 ,const char* _bolt = NULL,float rate = 1.0f,float _angle = 0.0f,osColor _color  = osColor(1.0f,1.0f,1.0f,1.0f),float _boltHeight = 20.0f);

	//! 创建新的远景, 远景同目前的做alpha过渡。
	bool                    create_newTerr( const char* _terrName );


	/** 渲染天空与远景。
	 *
	 *  \param _terrScale 远景要随着视矩的远近进行缩放。
	 */
	void                    render_skyTerr( osVec3D& _cen,float _fardis,bool _reflect );

	//!  得到当前激活的天空名.
	const char*             get_skyOsaName( void );

	//!  得到当前激活的远山的名字.
	const char*             get_terrOsaName( void );

	const char*	get_skyOsaName1();
	const char*	get_skyOsaName2();
	const char*	get_Bolt();
	float	get_BoltRate();
};


//! 当前的动画mesh最多有6种渲染方式。
# define   MAX_MESHRSTYLE    6

/** \brief
 *  用于管道中间管道所有可渲染的mesh资源的类.
 *
 *  渲染的时候,上层使用物品名字从中间管道得到一个渲染id,
 *  以后在渲染的时候,上层传给中间管道要渲染的物品的id,中间
 *  管道交由他的MeshMgr来管道这些 Mesh,得到可以渲染的结构,
 *  并进行渲染.
 *
 *  关于PMesh: 
 *  可以根据mesh占用的格子数据和面数&格子离相机的远近来得到
 *  每一次渲染时Mesh需要渲染的面数.
 *  
 *  ATTENTION: 
 *  在meshMgr中：id< 5000 表示是静态的物品，使用d3dxmesh,
 *                id> 5000 表示是动态的物品，使用我们自己的文件格式。
 *  初期使用两种物品，以保持兼容性，以后修改，不使用d3dxMesh的数据。 
 *  
 *  MeshMgr在渲染物品时，需要根据物品的类型，d3xMesh or aniMesh来调用
 *  不同的例程和数据。 
 */
class osc_meshMgr
{
	//! 远山和天空的相关类。
	friend class osc_osaSkyTerr;
private:
	//! Manager中的Mesh.
	CSpaceMgr<osc_Mesh>     m_vecMesh;
	
	//! Manager中保存的
	LPDIRECT3DDEVICE9       m_pd3dDevice;

	//! 保存中间管道的指针，自定义格式的aniMesh使用中间管道。
	osc_middlePipe*         m_ptrMPipe;

	//! 当前渲染的面的数目.
	int                     m_iRFaceNum;

	//@{
	// 动画mesh相关的数据。
	//! 动画mesh的数据，对应了每一个aniMesh文件。
	CSpaceMgr<osc_aniMesh>  m_vecAniMesh;

	//! 在当前渲染帧中，是否是第一次处理动态光。
	bool                    m_bFirstProcessDLight;
	//@} 


	//@{
	//  场景中的aniMesh相关数据。
	//! 动画mesh的实例数据，可以在场景中多个地方放同一个aniMesh,这个队列用于放置特效。
	CSpaceMgr<osc_aniMeshIns>   m_vecAniMeshIns;

	/** 这个队列用于放置场景中的aniMesh物品。
	 *  分成两个队列的目的，是把动画资源实例的使用数目控制在可控的范围内。
	 */
	CSpaceMgr<osc_aniMeshIns>   m_vecSceneAMIns;

	//! 对不同的渲染类型进行排序得到的整型指针。
	WORD*            m_vecMeshInsPtr[MAX_MESHRSTYLE];	
	int              m_vecMeshInsNum[MAX_MESHRSTYLE];
	int              m_iRAniMeshInsNum;

	//! 相关的顶点着色Mesh指针和场景中顶点着色mesh指针的数目。
	WORD*            m_vecColorMeshPtr;
	int              m_iColorVerMeshNum;
	//@} 


	//@{
	//! 渲染AniMesh时,用到的RenderStateBlock Id
	int              m_iRSBAniMeshId;
	int              m_iRSBAniMeshAlphaId;
	void             set_animeshRState( bool _alpha );
	void             create_amRSB( bool _alpha );
	//@} 

	//@{
	//! 渲染aniMesh中的顶点着色物品时,使用的RenderStateBlock Id.
	int              m_iRsbCVMeshId;
	int              m_iRsbCVMeshAlphaId;
	void             set_colorVertexRState( bool _alpha );
	void             create_colorVertexRSB( bool _alpha );
	//@} 


private:
	//! 在渲染一个Mesh之前对传入的数据进行设置
	void             mesh_renderSet( os_meshRender* _mr,osMatrix* pOut = NULL );

	//! 从当前的aniMeshArray中得到或是创建一个osc_aniMesh.
	osc_aniMesh*     get_aniMeshFromArr( os_aniMeshCreate* _mc,BOOL& _replace );

	/** 处理动画Ani的渲染队列，
	 *
	 *  先得到meshIns是否在可见的范围内，然后
	 *  相当于给不同的动画ani渲染方式进行排序,以减少渲染状态的改变次数。
	 *  这个队列只处理场景中特效相关的aniMeshIns.
	 */
	void             sort_aniMeshPtr( os_ViewFrustum* _vf );

	//! 渲染我们得到的aniMeshInstance的队列。
	bool             render_aniMeshInsArr( bool _alpha,bool _alphaTestOpatity,
		               os_FightSceneRType _rtype = OS_COMMONRENDER );


	//! 渲染点着色相关的Mesh.
	void             render_colorVertexMesh( bool _alpha );

public: // River @ 2008-12-15:水面相关
	//! 水面相关
	static osc_effect*    m_ptrWaterEffect;

public:
	osc_meshMgr();
	~osc_meshMgr();

	//! 初始化当前MeshManager.
	bool             init_meshMgr( LPDIRECT3DDEVICE9 _dev,osc_middlePipe* _pipe );

	//! 释放当前的meshMgr.
	void             release_meshMgr( bool _finalRelease = true );

	/** 从文件创建一个mesh.
	 *
	 *  \param _meshMem 如果当前的内存中有mesh对应的数据,则从内存中创建.
	 */
	int              creat_meshFromFile( const char* _filename,BOOL& _replace );
	//! 减少一个mesh的引用
	void             release_mesh( int _id );

	//! 全部释放静态的mesh.
	void             reset_staticMesh( void );

	//! 全部释放动态的mesh.
	void             reset_osaMesh( void );

	/** 从创建结构中创建我们的aniMesh.
	 *
	 *  \param _sceC 是否是从场景内创建的aniMesh.
	 */
	int              create_aniMeshFromFile( os_aniMeshCreate* _mc, 
		                      BOOL& _replace,bool _sceC = false,bool _topLayer = false );

	//! 往meshMgr的aniMeshIns渲染队列中加入场景中要渲染的aniMeshIns.
	void             add_sceAniMeshIns( int _idx );

	//! 在指定的位置上渲染当前的Mesh
	bool             render_mesh( os_meshRender* _mr,bool _alpha = false );
	//! 渲染一堆Mesh
	bool             render_meshVector( VEC_meshRender* _vec, int _mshCount, bool _alpha = false );

	//! 渲染一堆Mesh,不设置材质信息
	bool             render_meshVectorNoMaterial( VEC_meshRender* _vec, int _mshCount,ID3DXEffect* _pEffect,osMatrix& _ViewProjMat);

	//! 得到mesh的半径。
	float            get_meshRadius( int _idx );

	//! 渲染管理器中动态的mesh
	bool             render_aniMesh( os_ViewFrustum* _vf,
		               bool _alpha,os_FightSceneRType _rtype = OS_COMMONRENDER );

	//! 渲染水面相关的数据
	bool             render_waterAni( void );

	//! 渲染体积雾相关的数据
	bool             render_FogAni( int ShaderId ,int dwType);
	


	/** 处理aniMesh的动态光问题。需要对场景中的每一个动态光调用这个函数
	 *  动态光全部由全局光照来模拟。
	 */
	bool             process_aniMeshDlight( os_dLightData& _dlight );


	//@{
	//   物品动画相关的数据。
	//! 开始动画
	void             start_animationMesh( int _id );
	//! fade或是fadeIn当前的osa动画.
	void             start_fadeOsa( int _id,float _fadeTime,bool _fadeOut = true,float _minFade = 0.0f );

	//! 隐藏当前id的osa文件
	void             set_osaHide( int _id,bool _hide );


	//! 设置动画的位置。
	void             set_aniMeshPos( int _id,osVec3D& _pos );
	//! 设置动画的旋转
	void             set_aniMeshRot( int _id,float _angle );
	//! 缩放当前的osa动画
	void             scale_aniMesh( int _id,float _rate,float _time );


	//! 即时更新osa的位置
	void             update_osaPosNRot( int _id );


	//! 得到aniMeshIns的播放时间。
	float            get_osaPlayTime( int _id );
	//! 设置osaMeshIns的播放时间。
	void             set_osaPlayTime( int _id,float _timeScale );


	/** 设置一个动画的世界矩阵。
	 *  这个接口主要用于一些特效的播放，需要在游戏层填充世界矩阵数据。
	 */
	void             set_aniMeshTrans( int _id,osMatrix& _transMat );

	//! 验证动画id的有效性。
	bool             validate_aniMeshId( int _id );

# if __EFFECT_EDITOR__
	//! 强制结束一个动画.
	void             stop_animationMesh( int _id );

	//! 设置某一个动画相关的参数.
	bool             set_animationMesh( int _id,os_aniMeshCreate* _mc );
# endif 

	//! 删除一个动画.
	void             delete_animationMesh( int _id,bool _sceC = false );
	//@} 

	const char*           get_aniNameFromId( int _id );


	/** 每一帧中调用这个函数，以增加没有渲染到的mesh的lru值
	 *  每过三秒种释放一次场景中很长时间不使用的
	 */
	void             frame_moveLru( void );

	//! 从一个meshId得到这个mesh是否是alphaMesh
	bool             is_alphaMesh( int _id );

	//! 当前Mesh是否有OpacitySubse
	bool             is_opacityMesh( int _id );

	//! 得到渲染的面的数目
	int              get_rFaceNum( void );

	//! 得到静态mesh的BoundingBox.
	os_aabbox*       get_meshBBox( int _idx );

	//! 得到一个osaMesh的BoundingSphere.
	os_bsphere*      get_osaMeshBsphere( int _idx,bool _sce = false );
	//! 得到一个osaMesh的os_bbox
	void             get_osaMeshBbox( int _idx,os_bbox* _bbox,bool _sce = false );

	//! 是否是关键物品
	bool             is_keyObj( int _idx );
	//! 是否桥类物品.
	BOOL             is_bridgeObj( int _idx );
	//! 是否是受体积雾影响物品
	BOOL             is_AcceptFogObj( int _idx );
	

	//! 是否树类物品
	BOOL             is_treeObj( int _idx );

	BOOL			is_NoAlphaObj( int _idx );



	//! 输入一条射线,看是否和*.x的mesh文件相交.
	bool             intersect_mesh( int _idx,osVec3D& _rayPos,
		               float _dis,osVec3D& _rayDir,float* _resDis = NULL );

	//! 场景切换时，重设meshMgr
	void             sceneChange_reset( void );

	//! 重置某一个topRenderLayer的meshIns，以用于在某一帧得到渲染。
	void             reset_osaMeshInsRender( int _id );



};


//! 包含当前文件用到的inline函数。
# include "osMesh.inl"

