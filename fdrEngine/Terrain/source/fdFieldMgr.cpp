//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdFieldMgr.cpp
 *
 *  His:      River created @ 2003-12-23
 *
 *  Des:      用于管理地图格子的类，使用当前的相机，得到当前应该调用哪一张地图。
 *   
 *
 * “吾恐季氏之忧，不在颛臾，而在萧墙之内也。”   
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"
# include "../../backPipe/include/osCamera.h"
# include  "../../mfpipe/include/osMesh.h"
# include "../../mfpipe/include/osCharacter.h"
# include "../../effect/include/fdEleBolt.h"
# include "../../effect/include/osHDRLight.h"
# include "../../interface/miskFunc.h"
#include "../../mfpipe/include/osShader.h"
#include "../../mfpipe/include/osEffect.h"
#include "../../mfpipe/include/ostexture.h"
#include "../../effect/include/VolumeFog.h"
#include "../../interface/stringTable.h"
#include "../../CardProfile/gfxCardProfile.h"
# if GALAXY_VERSION
// tzz add for galaxy space hole
# include "../../fdrengine/GridCurve.h"
# include "../../fdrengine/galaEffect_common.h"
# endif //GALAXY_VERSION

using namespace osn_mathFunc;
# define  USE_GEOMIPMAP		1

//! 场景内需要渲染的透明物品和不透明物品的数目
# define  MAX_ALPHAOBJNUM     512
# define  MAX_OPACITYOBJNUM   512


//! 如果相机点击天空，最大的射线长度，在此长度处射线折断
# define  MAX_RAYDISDANCE  20


//! River @ 2010-6-24:全局的变量，确认是否需要渲染屏幕光波.
BOOL      g_bRenderScreenWave = FALSE;

OSENGINE_API BOOL		g_bRendSky = TRUE;
//! 是否更新地形
OSENGINE_API BOOL	   g_bUpdateTile = TRUE;
//! 是否显示tile为线框模式.
OSENGINE_API BOOL      g_bTileWireFrame = false;

//! 是否显示物品为线模框模式.
OSENGINE_API BOOL      g_bObjWireFrame = FALSE;


//! 是否使遮挡住了人物的物品变得半透明。
OSENGINE_API BOOL      g_bAlphaShelter = TRUE;


//! 是否显示静态的obj物品。
OSENGINE_API BOOL      g_bDisplayStaticMesh = TRUE;


//! 是否使用体积雾。
OSENGINE_API BOOL      g_bUseVolumeFog = TRUE;

//! 体积雾是否初始化。
OSENGINE_API BOOL      g_bVolumeFogInited = FALSE;

//! River @ 2010-6-25:是否使用屏幕光波效果.
OSENGINE_API BOOL      g_bScreenBlastWave = false;                  


# if __QTANG_VERSION__
//! TGMgr中最多的tileGround的数目,Q唐项目暂使用最多2*2的地图
OSENGINE_API int     MAXTG_INTGMGR = 4;
# else
//! TGMgr中最多的tileGround的数目
OSENGINE_API int     MAXTG_INTGMGR = 16;
# endif 

float gOSAPLAYERSpeed = 1.0f;

//! 定义每一个格子的属性值。
const int MASK_TILEDISPLAY     =      (1<<5);
const int MASK_WATERTILE       =      (1<<6);



//! 定义场景中格子在现实世界中的大小.
//! 使用变量定义,使在外部容易修改格子的大小.
OSENGINE_API float    TILE_WIDTH = 3.0f;
//! 半个地图格子的宽度。
OSENGINE_API float    HALF_TWIDTH = 1.5f;
//! 半个碰撞地图格子的宽度。
OSENGINE_API float    HALF_CTWIDTH = 0.75f;

//! 这个变量控制是否渲染场景中的地形。
OSENGINE_API BOOL     g_bRenderTerr = true;
OSENGINE_API BOOL     g_bRenderCollision = FALSE;
OSENGINE_API BOOL     g_bTransparentWater = TRUE;



//! 播放特效的声音相关的接口
I_soundManager* osc_TGManager::m_ptrSoundMgr = NULL;

//! reflection map
int osc_TGManager::mrefMapId = -1;
//! river added @ 2009-6-29: refraction target.
int osc_TGManager::m_iRefractionMapId = -1;

//! river added @ 2009-6-29: refraction target.
LPDIRECT3DSURFACE9 osc_TGManager::m_pRefractionSurface = NULL;
LPDIRECT3DTEXTURE9 osc_TGManager::m_pRefractionTexture = NULL;

//int mRefTargetId;
//int	mRefMapSurfaceId;
LPDIRECT3DSURFACE9 osc_TGManager::mpRefMapSurface  = NULL;
LPDIRECT3DTEXTURE9 osc_TGManager::m_pRefMapTexture = NULL;
LPDIRECT3DSURFACE9 osc_TGManager::m_pRefMapDepth = NULL;
LPDIRECT3DSURFACE9 osc_TGManager::mBackUpSurface = NULL;
LPDIRECT3DSURFACE9 osc_TGManager::m_pDepthStencilBackSur = NULL;

# if GALAXY_VERSION

std::vector<CBlood>		osc_TGManager::m_vecBlood;
 CGridRect				osc_TGManager::m_gridRect;


# endif //GALAXY_VERSION

 CGrowEff				osc_TGManager::m_growEff;
 CHDREff				osc_TGManager::m_realHDREff;


osc_TGManager::osc_TGManager()
{
	m_arrPtrTG = NULL;

	//! 初始化可见集的大小.
	m_vecVisualSet.resize( INIT_TILEVSSIZE );
	m_vecVASet.resize( INIT_VATNUM );
	m_iVATNum = 0;


	m_deviceMgr = ::get_deviceManagerPtr();

	// 给静态的成员管理器指针赋值。
	osc_newTileGround::m_ptrTgMgr = this;


	m_vec2CamInIdx.m_iX = 0;
	m_vec2CamInIdx.m_iY = 0;

	m_iAlphaObjNum = 0;
	m_vecAlphaObj.resize( MAX_ALPHAOBJNUM );
	m_iOpacityObjNum = 0;
	m_vecOpacityObj.resize( MAX_OPACITYOBJNUM );


	m_vecScrPicDis.resize( MIN_TMPTEXTURE );
	m_iCurScrDisNum = 0;

	m_bLoadedMap = false;

	m_bWeatherChanging = false;

	// 初始化skinMesh管理类.
	m_skinMeshMgr = osc_skinMeshMgr::Instance();
	m_bbMgr = osc_bbManager::Instance();
	m_boltMgr = osc_boltMgr::Instance();

	m_ptrTerrainMgr = new osc_tgLoadReleaseMgr;
	m_ptrDstr = NULL;
	m_sEffectMgr = new osc_specEffMgr;
	m_SceneFlashLightMgr = new SceneLightMgr;

	m_iMaxX = 0;
	m_iMaxZ = 0;
	m_iMaxXTgNum = 0;
	m_iMaxZTgNum = 0;

	m_vec3HeroPos.x = 0;
	m_vec3HeroPos.z = 0;

	m_iActMapNum = 0;

	m_pUpdate_uiLoadingStep = 0;
	m_pLensflare = NULL;

	m_vec3PickPos = osVec3D( 0,0,0 );


	m_pHDRLight = NULL;

# ifdef _DEBUG
	m_ptrD3dLine = NULL;
# endif 
	osc_TGManager::m_ptrSoundMgr = ::get_soundManagerPtr();

	mBackUpSurface = NULL;


# if __QTANG_VERSION__
	//! River added @ 2009-5-19:为了地图的碰撞信息与服务器和上层一致。
	m_pMapCollInfo = NULL;
# endif 


#if GALAXY_VERSION
	//made the corona around hero false 
	m_bCorona = FALSE;
		
#endif //GALAXY_VERSION

}

osc_TGManager::~osc_TGManager()
{
	release_scene();

	m_growEff.DestroyEffRes();
	m_realHDREff.DestroyEffRes();

	osc_boltMgr::DInstance();

	///@{
	gVolumeFogPtr->ReleaseVolumeFog();
	///@}

	MapTile::OneTimeDeinit();


# ifdef _DEBUG
	SAFE_RELEASE( m_ptrD3dLine );
# endif 

	SAFE_RELEASE(mpRefMapSurface);
	SAFE_RELEASE(mBackUpSurface);
	SAFE_RELEASE(m_pRefMapDepth);

	// 退出整个渲染引擎时，释放
	SAFE_RELEASE( m_pDepthStencilBackSur );


	SAFE_DELETE_ARRAY( m_ptrDstr );

	// 先释放地形管理器中的地图相关资源.
	m_ptrTerrainMgr->release_tgLRMgr();

	// 
	
	

	// 释放skinMesh管理类.skinMesh必须等场景释放完成了再释放
	osc_skinMeshMgr::DInstance();


	SAFE_DELETE(m_pLensflare);

	// 删除地块需要的内存
	SAFE_DELETE_ARRAY( m_arrPtrTG );

	// 删除场景内的特效管理器
	SAFE_DELETE( m_sEffectMgr );

	// River mod @ 2006-6-21:
	// 删除场景内的特效后再删除公告板的管理器
	osc_bbManager::DInstance();

	osc_osaSkyTerr::DInstance();

	

	//syq...delete hdr light
	SAFE_DELETE( m_pHDRLight );

	SAFE_DELETE(m_SceneFlashLightMgr);

	// 2011-2-23: 退出时无需释放，没有意义.
	//m_middlePipe.release_middelePipe();

	//　删除地图管理器，终止地图调入线程
	//SAFE_DELETE( m_ptrTerrainMgr );

# if GALAXY_VERSION
	m_vecBlood.clear();
# endif

}

/** \brief
*  释放一个场景.
*  
*  释放这个场景中所有的数据,比如地图传送等操作需要整个场景都变化需要
*  使用这个函数。
*  
*  ATTENTION TO FIX: 完整的处理这个函数。
*/
bool osc_TGManager::release_scene( bool _rlseScrPic/* = true*/ )
{
    guard;

	//! River @ 2009-5-11: 先禁掉此场景内的声音播放
	for (size_t i = 0; i < mCurrentPlaySoundItemList.size(); i++)
	{
		if( !mCurrentPlaySoundItemList[i].m_bPlayingSound )
			continue;
		m_ptrSoundMgr->StopSound( mCurrentPlaySoundItemList[i].idCloseLoopSound );
		mCurrentPlaySoundItemList[i].reset();
	}
	//! 清空当前的声音空间
	mCurrentPlaySoundItemList.clear();

	//! River added @ 2009-6-18: 先释放地图内的资源,否则动态释放，
	//! 有可能删除掉新创建的资源
	for( int t_i = 0;t_i<m_iActMapNum;t_i ++ )
		m_arrPtrTG[t_i]->release_TG();


	// 释放资源
	for( int t_i=0;t_i<this->m_sSceneTexData.m_iTexNum;t_i ++ )
	{
		g_shaderMgr->release_shader( 
			m_sSceneTexData.m_iShaderId[t_i],true );
		m_sSceneTexData.m_iShaderId[t_i] = -1;
	}

	m_sDecalMgr.release_allDecal();
	m_ptrTerrainMgr->release_tgLRMgr();

	// zmg
	m_sSceneTexData.release();

	//! River @ 2009-8-12：此时地图处于无内容状态
	m_bLoadedMap = false;

	osDebugOut( "\n\n\n\n地图释放工作完成...\n\n\n\n" );

	return true;

	unguard;
}



/** \brief
*  场景的FrameMove.
*
*  ATTENTION TO OPP: 想办法把这个函数放到endscene之后,增加cpu和Gpu之间的并行性.
*  
*/
bool osc_TGManager::frame_move( os_sceneFM* _fm )
{
	guard;

	osMatrix        t_viewMat,t_projMat,t_worldMat;

	// frameMove开始，先清空地表块的可见性
	if( m_bLoadedMap )
		MapTile::ClearVisibleTerr();

	// 
	//! River Added @ 2009-6-27:用于地形渲染
	if( !g_bRenderTerr )
		osVec3Normalize( &g_vec3LPos,&g_vec3LPos );


	// 
	// 设置当前ＴＧＭ内的动画物品数目为零.
	m_iAlphaObjNum = 0;

	// 当前帧渲染的三角形数目为0.
	m_iRenderTriNum = 0;

	// 从相机到主角人物之间的物品数目，初始化为0.
	m_iCamChrObjNum = 0;

	// 
	m_bHasAlphaShelter = false;

	//
	// 如果不传入相机，则不需要FrameMove.
	if( _fm->m_ptrCam == NULL )
		return true;

	m_iCenterX = _fm->m_iCx;
	m_iCenterY = _fm->m_iCy;
	m_vec3HeroPos = _fm->m_vecChrPos;

	// 
	// 中间管道的相关设置和处理.相机focus在没有主角人物的时候,不需要处理.
	g_ptrCamera = (osc_camera*)_fm->m_ptrCam;	

	g_ptrCamera->frame_moveCamera( 
		sg_timer::Instance()->get_lastelatime() );

	g_ptrCamera->get_viewmat( t_viewMat );
	g_ptrCamera->get_projmat( t_projMat );
	osMatrixIdentity( &t_worldMat );
	m_middlePipe.set_projMatrix( t_projMat );
	m_middlePipe.set_viewMatrix( t_viewMat );
	m_middlePipe.set_worldMatrix( t_worldMat );
	
	m_middlePipe.frame_setMiddlePipe( g_ptrCamera );

	// 如果有天气变化。
	if( m_bWeatherChanging ){
		frame_moveWeatherChanging( sg_timer::Instance()->get_lastelatime() );
		m_dlightMgr.dxlight_enable(true);
	}

	g_shaderMgr->frame_move();

	// 相机frameMove完成后，再处理skinMgr的FrameMove
	m_skinMeshMgr->frame_setSmMgr();


	// River mod @ 2007-8-26:
	// 对于激活地图列表相关的处理，使用临界区，界为上层使用了多线程
	::EnterCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );
	
	//! 得到当前激活的地图指针列表.
	int   t_idx = 0;
	memset( m_arrPtrTG,NULL,sizeof( osc_TileGroundPtr )*MAXTG_INTGMGR );


	// 不渲染地表，也需要渲染地表之上的静态mesh信息
	m_iActMapNum = m_ptrTerrainMgr->frame_getInViewTg( 
		g_ptrCamera,m_arrPtrTG,MAXTG_INTGMGR );

	// River mod @ 2007-8-26:
	// 对于激活地图列表相关的处理，使用临界区，界为上层使用了多线程
	::LeaveCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );


	//
	// 对每一个tg进行帧间的初始化。
	if (g_bUpdateTile)
	{
		for( int t_i=0;t_i<m_iActMapNum;t_i++ )
		{
			osassert( m_arrPtrTG[t_i] );
			m_arrPtrTG[t_i]->frame_setTG();		
		}
		
	}

	

	// 
	// 完全由Shader计算的动态光
	os_dlInfo     t_dlInfo;  
	m_dlightMgr.frame_moveDLMgr();
	m_dlightMgr.get_dlINfo( t_dlInfo );
	// River @ 2010-12-31:暂时关掉动态光，去掉地表硬纹理渲染出现的bug.
	/*
	if( t_dlInfo.m_iDLNum > 0 )
	{
		for( int t_i=0;t_i<m_iActMapNum;t_i++ )
		{
			osassert( m_arrPtrTG[t_i] );
			m_arrPtrTG[t_i]->m_sQuadTree.set_dlAffectChunk( &t_dlInfo );			
		}
	}
	*/


	// 特效管理器
	m_sEffectMgr->frame_move();
	float   t_fEtime = sg_timer::Instance()->get_lastelatime();


	if (g_bRendSky&&g_bHasBolt)
	{
		m_SceneFlashLightMgr->frame_move(t_fEtime);
	}
	else if (g_bRendSky==NULL)
	{
		g_fDirectLR = 1.0f;
		g_fDirectLG = 1.0f;
		g_fDirectLB = 1.0f;
	}
	
	

	// 粒子管理器，公告板管理器的处理和渲染分开.
	m_particleMgr.framemove_particle( g_ptrCamera );
	m_bbMgr->frameMove_bbMgr( (I_camera*)g_ptrCamera );
	m_boltMgr->frame_move( (I_camera*)g_ptrCamera,
		    t_fEtime );

	//@{
	// River @ 2004-5-20:
	// 加入动画物品的动态光。
	process_aniMeshDlight();


	// River @ 2007-7-7:如果处于天气变化状态，则不处理这个半透明
	if( g_bAlphaShelter &&(!m_bWeatherChanging) )
	{
		process_camChrObjList();
		g_ptrCamera->get_viewmat( t_viewMat );
		m_middlePipe.set_viewMatrix( t_viewMat );
	}

	m_middlePipe.m_dwAmbientColor = 
		osColor( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f );

	static osVec3D camPos ;
	g_ptrCamera->get_curpos(&camPos);


	// NEW ATTENTION OPP:river @ 2007-3-20: 声音在某些帧内很费时间.
	frame_moveSound(&camPos,sg_timer::Instance()->get_lastelatime());


	//! River @ 2009-6-13:
	//! 处理smMgr的frameMove,此处调用，用于上层能够更加准确的得到sm骨骼的位置
	m_skinMeshMgr->framemove_smMgr();


	return true;

	unguard;
}



//! 设置全局的雾渲染状态.
void osc_TGManager::fog_rsSet( void )
{
	guard;

	m_middlePipe.set_renderState( D3DRS_FOGENABLE,g_bUseFog );

	// 测试基于顶点的rangeBase fog.
	if( g_bUseFog )
	{
		// 
		//
		// 目前场景中只支持线性雾。
		m_middlePipe.set_renderState( D3DRS_FOGVERTEXMODE,  D3DFOG_LINEAR );
		m_middlePipe.set_renderState( D3DRS_FOGTABLEMODE,D3DFOG_NONE );
		m_middlePipe.set_renderState( D3DRS_RANGEFOGENABLE, TRUE );
		m_middlePipe.set_renderState( D3DRS_FOGSTART, *(DWORD *)(&g_fFogStart) );
		m_middlePipe.set_renderState( D3DRS_FOGEND,   *(DWORD *)(&g_fFogEnd) );

		// 设置全局的雾.
		m_middlePipe.set_renderState( D3DRS_FOGCOLOR, g_dwFogColor );
	}

	unguard;
}

//! 画场景内的每一个声音的包围盒子
void osc_TGManager::draw_soundBox( void )
{
	guard;

	//! test 调试场景声音用，显示每个声音的盒子
	if (g_SceneSoundBox)
	{
		std::vector<OSSoundItem*> pSoundPtrItemList = mSoundQuadTree.GetNodePtrList();
		for (size_t iSoundNr = 0; iSoundNr < pSoundPtrItemList.size();iSoundNr++)
		{

			os_bbox   t_box;
			float EdgeSize = pSoundPtrItemList[iSoundNr]->m_flMinDistance;
			

			float t_fMaxDis = pSoundPtrItemList[iSoundNr]->m_flMaxDistance;
			t_box.set_bbMinVec( osVec3D(-EdgeSize,-EdgeSize,-EdgeSize) );
			t_box.set_bbMaxVec( osVec3D(EdgeSize,EdgeSize,EdgeSize) );
			t_box.set_bbPos(pSoundPtrItemList[iSoundNr]->m_pos);
			draw_bbox( &t_box,0xffff0000 );
		}
	}

	unguard;
}


//! 高层图层的渲染，用于战斗时的特效，人物，decal等
void osc_TGManager::render_topLayer( I_camera* _cam,os_FightSceneRType _rtype )
{
	guard;


	m_middlePipe.set_viewMatrix( g_matView );
	m_middlePipe.set_projMatrix( g_matProj );

	// 处理特效相关的顶层渲染
	// 渲染keyAniMesh: ATTENTION:
	// 处理，根据矩离相机的远近，渲染我们的aniMesh
	m_dlightMgr.dxlight_enable( false );
	m_middlePipe.frame_renderKeyAniMesh( g_ptrCamera->get_frustum(),false,OS_RENDER_TOPLAYER );
	m_middlePipe.frame_renderKeyAniMesh( g_ptrCamera->get_frustum(),true,OS_RENDER_TOPLAYER );


	m_dlightMgr.dxlight_enable( false );
	m_middlePipe.set_renderState( D3DRS_ZWRITEENABLE,false );

	//
	// partilceManager的渲染，多种透明物品的重叠会出现问题:
	m_particleMgr.render_particle( NULL,OS_RENDER_TOPLAYER );

	return;


	// 如果是分层渲染，则此处开始渲染top layer.
	if( _rtype != OS_RENDER_LAYER )
		return;

		
	HRESULT t_hr = 
		((osc_d3dManager*)m_deviceMgr)->get_d3ddevice()->Clear( NULL,NULL,
		D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,0x0,1.0f,0L );
	osassert( !FAILED( t_hr ) );

	m_sDecalMgr.render_decalmgr( OS_RENDER_TOPLAYER );

	if( !m_skinMeshMgr->render_smMgr( OS_RENDER_TOPLAYER,false ) )
		osassert( false );

	if( !m_skinMeshMgr->render_smMgr( OS_RENDER_TOPLAYER,true ) )
		osassert( false );

	/*
	// 处理特效相关的顶层渲染
	// 渲染keyAniMesh: ATTENTION:
	// 处理，根据矩离相机的远近，渲染我们的aniMesh
	m_dlightMgr.dxlight_enable( true );
	m_middlePipe.frame_renderKeyAniMesh( g_ptrCamera->get_frustum(),false,OS_RENDER_TOPLAYER );
	m_middlePipe.frame_renderKeyAniMesh( g_ptrCamera->get_frustum(),true,OS_RENDER_TOPLAYER );
	*/

	m_dlightMgr.dxlight_enable( false );
	m_middlePipe.set_renderState( D3DRS_ZWRITEENABLE,false );

	////
	//// 渲染场景中的billBoard面片。
	if( !m_bbMgr->render_bbManager( g_ptrCamera,OS_RENDER_TOPLAYER  ) )
		osassertex( false,"渲染bbManager失败...\n" );

	//
	// partilceManager的渲染，多种透明物品的重叠会出现问题:
	m_particleMgr.render_particle( g_ptrCamera,OS_RENDER_TOPLAYER );

	return;

	unguard;
}



/** \brief
*  渲染当前的地图。
*
*
*  IMPORTANT FUNC: 整个场景的渲染需要靠这个函数来实现。
*  
* 
*  \param _scrPic 如果当前变量为真，则只渲染屏幕内的二维图片，否则渲染场景
*                 加二维图片。
*  
*  渲染顺序：
*  1: 先渲染地表，渲染地表的时候渲染地表上的不透明物体。
*  2: 渲染场景中的人物,先渲染人物的不透明部分，再渲染的人物的透明部分。
*  3: 渲染场景中物体的透明部分。
*  4: 渲染场景中的billBoard物体。
*  4: 渲染场景中的粒子物体。
*
*  River mod @ 2008-11-27:用于渲染回合制状态下的场景,主要工作：
*  1: 设置某部分人物人物渲染在整个场景之后。
*  2：设置非场景内的特效渲染在整个场景之后。
*  3: 场景渲染完成后，在场景内渲染一个面片用于跟场景背景混合。
*  4: 此时渲染参与战斗的人物和特效。
*  
*/
bool osc_TGManager::render_scene(  bool _scrPic/* = false */,
		bool _LensPic,os_FightSceneRType _rtype/* = OS_COMMONRENDER*/  )
{
	guard;


	osc_d3dManager* t_ptrMgr = (osc_d3dManager*)m_deviceMgr;

	//@{	
#if GALAXY_VERSION
	if(m_gridRect.IsReadyToRender())
	{
		osassert(m_deviceMgr);
		m_gridRect.RenderFrame(m_deviceMgr->get_d3ddevice());
		return true;
	}
#endif // GALAXY_VERSION
	//@}

	//
	// 如果场景中仅需要显示二维图片。
	if( _scrPic )
	{
		render_ScrPic(_LensPic);
		// 最后渲染屏幕上的光标。	
		t_ptrMgr->render_cursor();
		return true;
	}

	if( m_pHDRLight )
		m_pHDRLight->set_hdrRenderTarget();
	
	// River @ 2010-3-13:为了全屏grow效果,此处设置
	if( m_growEff.IsEffective() )
		m_growEff.reset_renderTarget( g_frender->get_d3dDevice() );

	// 先渲染天空盒子.
	if (g_bRendSky)
		render_skyDome();

	//@{
	// Windy mod @ 2005-9-14 17:24:17
	//Desc: lens flare
#ifdef USE_LENSFLARE
	m_pLensflare->render(g_ptrCamera,1);
#endif
	//@}
	
	// 设置全局雾相关的数据
	fog_rsSet();


	//!在FRAMEMOVE中渲染
	if( g_bRenderTerr && m_bLoadedMap  )
	{
		// 渲染场景中普通的TileSet.
		MapTile::DrawTerrain();
		if (g_bTransparentWater && g_bWaterReflect)
		{
			///@{
			MapTile::DrawTerrain(FALSE,TRUE);
			///@}
		}
	}

	//@}	

	//
	// 开始渲染场景中人物或是怪物阴影相关的数据
    /*	
	// 原雨滴渲染的位置。的下落到地下的效果。
	*/

	//
	// 渲染场景中的不透明物品。
	//
	if( g_bDisplayStaticMesh )
		if( !render_opacityObjPart() )
			return false;

	// 
	// 场景内Decal相关的渲染,为了能在物品上正确的显示Decal.
	// 放在此处还是会出现问题：对于桥面下有水的情形，因为高度的插值问题，会出现黑的方块。
	// 便如果放到透明的keyAni后面渲染，则透明的keyani会写入Zbuffer,从而影响decal的显示。
	// 解决方案是：把光标的Decal放到后面显示？？？
	m_sDecalMgr.render_decalmgr( _rtype );


	osc_skinMesh::clear_postTrail();
	// 
	// River @ 2005-8-16:把不透明的keyAni放到人物之前渲染。
	// 渲染keyAniMesh.
	m_middlePipe.frame_renderKeyAniMesh( g_ptrCamera->get_frustum(),false,OS_RENDER_LAYER );

	if( g_eSkinMethod == OSE_D3DINDEXED )
		m_dlightMgr.dxlight_enable( true );

	//@{
	//! River @ 2010-1-29: 渲染墙后的人物.
	m_middlePipe.set_nullStream();
	m_skinMeshMgr->renderBehindWallChar();
	m_middlePipe.set_nullStream();
	//@}


	
	//@{ 人物部分的渲染.	
	// 渲染场景中的人物的不透明部分。
	if( !m_skinMeshMgr->render_smMgr( _rtype ) )
		osassert( false );
	m_middlePipe.set_nullStream();
	//
	// River @ 2006-7-1:把人物的透明部分渲染放到keyAni渲染的后面，之前
	// 此模块的渲染紧接着人物的不透明部分的渲染
	// 渲染场景中人物的透明部分
	m_skinMeshMgr->render_smMgr( _rtype,true );
	/**/
	
	m_middlePipe.set_nullStream();
	osc_skinMesh::end_renderPostTrail(true);


	// River @ 2009-5-14:提示渲染水面，用于特效的正确渲染
	m_dlightMgr.dxlight_enable( true );
	m_middlePipe.set_renderState( D3DRS_ZWRITEENABLE,true );

	// River @ 2009-7-30: 物品的透明部分，应该在水面之前渲染，否则无折射。
	// 渲染物品透明相关的部分.
	if( g_bDisplayStaticMesh )
	{
		if( !render_transparentObjPart() )
			return false;
	}		
	//@}

	//windy add
	UpDate_reflectionMap();

	//! River @ 2009-6-29: 更新refractionMap.
	update_refractionMap();
	m_middlePipe.render_aniWater( g_ptrCamera->get_frustum() );

	//! River @ 2009-6-28:重设雾相关的数据
	fog_rsSet();


	// 
	// River @ 2009-7-7:把透明物品部分和人物透明部分放到水面之后渲染


	// 渲染keyAniMesh: ATTENTION:
	// 处理，根据矩离相机的远近，渲染我们的aniMesh
	m_middlePipe.set_renderState( D3DRS_FOGENABLE,true );
	m_dlightMgr.dxlight_enable( true );
	g_bRenderScreenWave = FALSE;
	m_middlePipe.frame_renderKeyAniMesh( g_ptrCamera->get_frustum(),true,OS_RENDER_LAYER );
	
	// River @ 2010-6-23:恢复状态.
	m_middlePipe.set_renderState( D3DRS_FOGENABLE,true );
	m_dlightMgr.dxlight_enable( true );


	// 场景内的SceneCursor,即点击地面的光标
	m_sDecalMgr.render_sceneCursor();


	// River @ 2009-7-25:雨滴提到此处，
	// 为的是跟水面兼容，不让水面的折射影响雨滴
	// 雨滴的下落到地下的效果。
	if( !m_bbMgr->render_raindropBB( g_ptrCamera ) )
		return false;

	////
	//// 渲染场景中的billBoard面片。
	if( !m_bbMgr->render_bbManager( g_ptrCamera,OS_RENDER_LAYER  ) )
		return false;


	//! 渲染场景中的boltMgr.
	m_boltMgr->render();

	//
	// partilceManager的渲染，多种透明物品的重叠会出现问题:
	m_particleMgr.render_particle( g_ptrCamera,OS_RENDER_LAYER );

	//
	// 渲染挡在相机和人之间半透明物品。仅次于下雨和屏幕图块的渲染顺序,关掉雾和灯光
	// River @ 2009-6-28: 必须打开灯光才能半透明。关掉雾的效果也不太好，打开雾更好.
	m_dlightMgr.dxlight_enable( true );
	if( g_bAlphaShelter )
	{
		render_shelterObj( false );
		render_shelterObj( true );
	}
	m_dlightMgr.dxlight_enable( false );


	//
	// 下雨效果的渲染。
	if (g_bRendSky)
	{
		if( m_effectRain.get_rainning() )     
			render_rain();
		if (m_effectSnow.get_weatherSnowing()) 
			render_snow();
	}


	//@{
	// Windy mod @ 2005-9-14 17:24:17
	//Desc: lens flare
#ifdef USE_LENSFLARE
	m_pLensflare->render(g_ptrCamera,2);
#endif
	//@}
	///@{
	//! 渲染场景深度信息
	if (g_bUseVolumeFog&&g_bVolumeFogInited)
	{
		UpDate_DepthMapAndRenderFog();
	}
	///@}


	// 处理场景，使可以得到
	// 渲染场景中的二维屏幕显示图片。
	t_ptrMgr->draw_fullscrFade();	
	render_ScrPic();

	//
	// 统计场景中可见的多边形数目。
	m_iRenderTriNum += m_middlePipe.get_meshFaceNum();
	m_iRenderTriNum += m_bbMgr->get_bbMgrRenderTriNum();

	//
	if( m_pHDRLight )
		m_pHDRLight->render_hdrLight();
	
	draw_soundBox();


# if GALAXY_VERSION
	m_skinMeshMgr->draw_projectShadow();
	//! display the blood
	DisplayBlood();
	// fog of throughing out space hole
	m_gridRect.RenderThroughFog(m_deviceMgr->get_d3ddevice());
# endif 

//	if( g_bRenderScreenWave )
//		D3DXSaveSurfaceToFileA("d:\\test.tga",D3DXIFF_TGA,mBackUpSurface,NULL,NULL );

	// River: 屏幕光波效果
	m_growEff.RenderScreenWave( m_deviceMgr->get_d3ddevice() );

	m_growEff.RenderEffScene(m_deviceMgr->get_d3ddevice());
	//@}




	//@}

	return true;

	unguard;
}


// 
//! 声音部分必须修改，不然此处有内存的分配和释放操作，在某些情况，会让整个场景卡起来!!!!!
//! 场景音效更新函数.
void osc_TGManager::frame_moveSound(osVec3D* _camPos,float _timeMs)
{
	guard;
	
	static os_aabbox camBox;
	static osVec3D camEdge(1.0f,1.0f,1.0f);
	camBox.m_vecMax = *_camPos + camEdge;
	camBox.m_vecMin = *_camPos - camEdge;
	
	mSoundQuadTree.Calculate(camBox);
	std::vector<OSSoundItem*> pSoundPtrItemList = mSoundQuadTree.GetNodePtrList();
	
	//// 维护已经有的声音
	for (size_t i = 0; i < mCurrentPlaySoundItemList.size(); i++)
	{
		if( !mCurrentPlaySoundItemList[i].m_bPlayingSound )
			continue;

		osVec3D length3D = mCurrentPlaySoundItemList[i].pItem->m_pos- *_camPos;
		float length = osVec3Length(&length3D);

		std::vector<OSSoundItem*>::iterator it;

		// River @ 2008-3-21:
		// 如果现有的播放声音列表存在一个声音，他已经不在声音四叉树可播放的声音列表内，
		// 则停止此声音的播放
		it = std::find(pSoundPtrItemList.begin(),pSoundPtrItemList.end(),mCurrentPlaySoundItemList[i].pItem);
		if (it == pSoundPtrItemList.end())
		{
			m_ptrSoundMgr->StopSound(mCurrentPlaySoundItemList[i].idCloseLoopSound);
			mCurrentPlaySoundItemList[i].reset();
		}
		else
		{
			mCurrentPlaySoundItemList[i].mPlayTimeMs += _timeMs;

			if( mCurrentPlaySoundItemList[i].mPlayTimeMs>mCurrentPlaySoundItemList[i].pItem->m_fSoundPlayTime)
			{
				mCurrentPlaySoundItemList[i].mPlayTimeMs = 0.0f;
				float NewRate =  (float)OSRAND/RAND_MAX;
				float SoundRate = (float)mCurrentPlaySoundItemList[i].pItem->m_playType/10.0f;

				//! river @ 2009-5-11: 循环的声音播放
				if( mCurrentPlaySoundItemList[i].pItem->m_playType == 10 )
					continue;

				if (NewRate<SoundRate)
				{
					m_ptrSoundMgr->StopSound(mCurrentPlaySoundItemList[i].idCloseLoopSound);
					os_initSoundSrc soundSrc;
					soundSrc.m_vPosition = mCurrentPlaySoundItemList[i].pItem->m_pos;
					soundSrc.m_flMinDistance = mCurrentPlaySoundItemList[i].pItem->m_flMinDistance;
					soundSrc.m_flMaxDistance = mCurrentPlaySoundItemList[i].pItem->m_flMaxDistance;
					soundSrc.m_fPlayTime = mCurrentPlaySoundItemList[i].pItem->m_fSoundPlayTime;

					mCurrentPlaySoundItemList[i].idCloseLoopSound = m_ptrSoundMgr->PlayFromFile
						(mCurrentPlaySoundItemList[i].pItem->m_SoundName,&soundSrc);
				}
			}

		}
	}
	// 插入新的声音
	for (size_t i = 0 ; i < pSoundPtrItemList.size();i++)
	{
		std::vector<OSSoundPlayingItem>::iterator it;
		it = std::find(mCurrentPlaySoundItemList.begin(),
			mCurrentPlaySoundItemList.end(),pSoundPtrItemList[i] );
		if (it == mCurrentPlaySoundItemList.end())
		{
			OSSoundPlayingItem item;
			item.pItem = pSoundPtrItemList[i];
			item.mPlayTimeMs = 0;
			
			//item.mTimeLength = item.pItem->m_fSoundPlayTime;
			os_initSoundSrc soundSrc;
			soundSrc.m_vPosition = item.pItem->m_pos;
			soundSrc.m_flMinDistance = item.pItem->m_flMinDistance;
			soundSrc.m_flMaxDistance =item.pItem->m_flMaxDistance;

			float  t_fSoundTime;
			
			// River added @ 2009-5-11:循环的声音播放
			bool   t_bLoop = false;
			if( item.pItem->m_playType == 10 )
				t_bLoop = true;

			item.idCloseLoopSound = m_ptrSoundMgr->PlayFromFile(
				item.pItem->m_SoundName,&soundSrc,t_bLoop,false,&t_fSoundTime );

			// river mod @ 2008-3-21:如果编辑指定的播放时间长度为零，则按照普通的播放时间来定
 			if( item.pItem->m_fSoundPlayTime <= 0.0f )
				item.pItem->m_fSoundPlayTime = t_fSoundTime;
			item.m_bPlayingSound = true;

			int t_i;
			for( t_i = 0;t_i<(int)mCurrentPlaySoundItemList.size();t_i ++ )
			{
				if( !mCurrentPlaySoundItemList[t_i].m_bPlayingSound )
				{
					mCurrentPlaySoundItemList[t_i] = item;
					break;
				}
			}
			if( t_i == mCurrentPlaySoundItemList.size() )
				mCurrentPlaySoundItemList.push_back(item);
		}

	}

	unguard;
}

/** \brief
*  判断一条射线是否和一个quad四边形相交,并根据传入的参数，处理得到离相机位置
*  最近的交点。
*
*  \param const osVec3D* _quadVer 传入的四边形顶点，为两个三角形，两个三角形由顶点索引
*                                 (0,2,1),(1,2,3) 构成 
*  \param osVec3D& _intersectPt 如果相交，并且比传入的矩离离相机近，返回。
*  \param osVec3D& _camPos      相机的位置。
*  \param float& _dis           此次相交测试前得到的离相机最近交点的矩离。
*/
bool osc_TGManager::pick_nearestTri( const osVec3D* _quadVer,const os_Ray &_ray,
									osVec3D& _pickVec,osVec3D& _camPos,float& _dis )
{
	guard;

	osVec3D     t_interPT;
	bool        t_hr = false;

	// 三角开面片级别的相交检测
	if (osn_mathFunc::IntersectTriangle( _ray,
		_quadVer[0],_quadVer[2],_quadVer[1],t_interPT))
	{
		osVec3D t_LenVec = t_interPT -_ray.getOrigin();
		if (osVec3Dot(&t_LenVec,&_ray.getDirection())>0)
		{
			float dist = osVec3LengthSq( &(t_interPT - _camPos) );
			if ( dist<_dis){
				_dis = dist;
				_pickVec = t_interPT;
				t_hr = true;
			}
		}

	}
	//!1 2 3 三角片
	else if (osn_mathFunc::IntersectTriangle( 
		_ray,_quadVer[1],_quadVer[2],_quadVer[3],t_interPT))
	{
		osVec3D t_LenVec = t_interPT -_ray.getOrigin();
		if (osVec3Dot(&t_LenVec,&_ray.getDirection())>0)
		{
			float dist = osVec3LengthSq( &(t_interPT - _camPos) );
			if (dist<_dis){
				_dis = dist;
				_pickVec = t_interPT;
				t_hr = true;
			}
		}
	} 

	return t_hr;

	unguard;
}


/** \brief
*  从rayStart开始，处理Chunk的内的桥类物品，如果相交，则返回true.
*
*  \param int _cnum 传入的chunk数目
*  \param const os_tileIdx* 传入的chunk列表
*  
*  算法描述：
*  得到每一个TileGround的QuadTree的aabbox,然后跟当前的Ray做碰撞，如果
*  有碰撞发生，则从这个TileGround内的物品列表中，找出桥类物品，然后
*  计算桥类物品是否跟Ray碰撞，如果碰撞，计算离相机最近的射线的位置。
*/
bool osc_TGManager::get_raypickBridge( osVec3D& _rayS,osVec3D& _rayDir,float& _resLength )
{
	guard;

	float t_fMinDis = 10000.0f,t_fTmpDis = 100000.0;

	for( int t_i=0;t_i<this->m_iActMapNum;t_i ++ )
	{
		osc_TileGroundPtr t_TilePtr = NULL;
		t_TilePtr = m_arrPtrTG[t_i];

		// 
		// 对每一个Chunk得到此Chunk的物品列表，如果列表内有桥类的物品，
		// 则射线处理一下，是否跟桥类物品的boundingbox相交，如果相交，
		// 则进一步处理准确的交点位置
		if( !t_TilePtr->m_sQuadTree.ray_intersect( _rayS,_rayDir ) )
			continue;

		// 得到此tileGround内的物品列表，对于桥类的物品，做AABBox碰撞检测
		for( int t_k = 0;t_k<t_TilePtr->m_iTGObjectNum;t_k ++ )
		{
			// 非固定物品
			if( 0 != t_TilePtr->m_vecTGObject[t_k].m_dwType )
				continue;
			// 非桥类物品
			if( !g_ptrMeshMgr->is_bridgeObj( t_TilePtr->m_vecObjectDevIdx[t_k] ) )
				continue;

			// 物品的BBox. // River @ 2010-3-30:使用一个临时变量
			float t_fBoxDis;
			if( !t_TilePtr->m_vecObjAabbox[t_k].
				ray_intersect( _rayS,_rayDir,t_fBoxDis ) )//t_fTmpDis ) )
				continue;

			// 精确的相交测试？
			if( !t_TilePtr->is_intersectObj( 
				t_k,_rayS,10000.0f,_rayDir,&t_fTmpDis ) )
				continue;

			if( t_fTmpDis < t_fMinDis )
				t_fMinDis = t_fTmpDis;
		}	
	}

	//! 跟桥类的物品有碰撞发生
	if( t_fMinDis < 10000.0f )
	{
		_resLength = t_fMinDis;
		return true;
	}
	else 
		return false;

	unguard;
}



//! 从一个开始点和一个dir,计算跟地表或是桥类物品相关的长度信息。
float osc_TGManager::get_pickRayLength( BOOL& _bridgeInter,
					osVec3D& _rayS,osVec3D& _rayDir,osVec3D& _pickVec )
{
	guard;

	float    t_fLength = 1000.0f;
	osVec3D  t_vec3Right,t_vec3RayEnd;
	ose_geopos       t_arrPPClip[4];
	bool            t_hr = false;
	osPlane         t_pickPlane;
	float	        t_fdistCamPosToSelect = 10000000.0f;
	osVec3D         t_vec3RayDir = _rayDir;
	osVec3D         t_vec3BackRayDir = t_vec3RayDir;


	os_Ray pickRay( _rayS,t_vec3RayDir );

	// 使用相机位置，相机的右方顶点，pickray射线方向的一个位置，
	// 构建一个机机射线方向的平面，如果一个地表格子的四个顶点全在平面的一测，则
	// 这个格子肯定不和相机的射线相交。因为地表格子是凸面体。
	t_vec3RayEnd = _rayS + t_vec3RayDir;
	osVec3Cross( &t_vec3Right,&osVec3D( 0.0f,1.0f,0.0f ),&_rayDir );
	t_vec3Right = t_vec3Right + _rayS;
	osPlaneFromPoints( &t_pickPlane,&_rayS,&t_vec3Right,&t_vec3RayEnd );

	float   t_fBridgeColl = 10001.0f;

	for (int i =0 ; i < m_iActMapNum;i++)
	{
		os_aabbox box;
		osc_TileGroundPtr TilePtr = NULL;
		TilePtr = m_arrPtrTG[i];

		// 以四叉树为基准的测试:
		const os_tileIdx*   t_ptrChunkIdx;
		const osVec2D*      t_ptrChunkXZ;
		int                t_iPickChunkNum = 0;

		// 从当前TileGround的QuadTree得到相交的Chunk列表
		t_iPickChunkNum = TilePtr->m_sQuadTree.get_pickChunkList( 
			       _rayS,t_vec3RayDir,t_ptrChunkIdx,t_ptrChunkXZ );
		if( 0 == t_iPickChunkNum )
			continue;


		// 对每一个ray相交的Chunk进行更细致的处理
		for( int t_i=0;t_i<t_iPickChunkNum;t_i ++ )
		{
			osVec3D    t_interPT;
			osVec3D    t_poly[4];

			// 
			// 开始查找可能相交的tile格子	列表。
			int t_tileNum = m_sChunkPick.get_pickTile(
				_rayS,t_vec3RayDir,t_ptrChunkXZ[t_i].x,t_ptrChunkXZ[t_i].y );

			// 对每一个tile格子查看是否相交
			const os_tileIdx*   t_ptrIdx = m_sChunkPick.get_testTileArr();
			int   t_iBeginx = t_ptrChunkIdx[t_i].m_iX*16;
			int   t_iBeginy = t_ptrChunkIdx[t_i].m_iY*16;
			int   t_iTx,t_iTz;

			for( int t_i=0;t_i<t_tileNum;t_i ++ )
			{
				t_iTx = t_iBeginx + t_ptrIdx[t_i].m_iX;
				t_iTz = t_iBeginy + t_ptrIdx[t_i].m_iY;

				if( (t_iTx >= MAX_GROUNDWIDTH) ||
					(t_iTz >= MAX_GROUNDWIDTH) )
					continue;

				TilePtr->get_polyVertex( t_poly,t_iTx,t_iTz );

				// 
				// 先做大体的检测,根据凸多边原理,此处可以过滤掉大半的四边形
				t_arrPPClip[0] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[0] );
				t_arrPPClip[1] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[1] );
				t_arrPPClip[2] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[2] );
				t_arrPPClip[3] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[3] );
				if( (t_arrPPClip[0] == t_arrPPClip[1])&&
					(t_arrPPClip[1] == t_arrPPClip[2])&&
					(t_arrPPClip[2] == t_arrPPClip[3]) )
					continue;

				// 做三角形级别的相交测试
				if( pick_nearestTri( t_poly,pickRay,
					_pickVec,_rayS,t_fdistCamPosToSelect ) )
					t_hr = true;
			}
		}
	}

	_bridgeInter = false;
	// 
	// River mod @ 2008-10-16:加入跟桥类物品的交点,用于点击桥类物品时，精确的走到桥上
	// River added @ 2009-5-4:缩放类的物品，在bridge上点击时的处理
	if( get_raypickBridge( _rayS,t_vec3RayDir,t_fBridgeColl ) )
	{
		if( t_hr )
		{
		
			float   t_f = osVec3Length( &(_rayS - _pickVec) );

			if( t_f < t_fBridgeColl )
				return t_f;
		}
		
		_bridgeInter = true;
		return t_fBridgeColl; 
	}
	
	if( t_hr )
		return osVec3Length( &(_pickVec-_rayS) );
	else
		return 200.0f;

	unguard;
}



/** 新的鼠标点选方式 
@remarks
	大致算法如下:
	先得到大致相交的地表格子，然后对这些格子做更精细的碰撞处理。
*/
bool osc_TGManager::get_newpickTileIdx(osVec3D& _pickVec,os_tileIdx& _sidx )
{
	guard;

	osassert( (_sidx.m_iX>=0)&&(_sidx.m_iY>=0) );

	osVec3D         t_vecRayDir,t_vec3RayEnd,t_camPos,t_camRightVec;
	osPlane         t_pickPlane;
	ose_geopos       t_arrPPClip[4];
	bool            t_hr = false;
	float	        t_fdistCamPosToSelect = 10000000.0f;

	if( !g_ptrCamera )
		return false;

	g_ptrCamera->get_curpos( &t_camPos );
	g_ptrCamera->get_rightvec( t_camRightVec );
	t_camRightVec = t_camPos + t_camRightVec;


	// 得到从屏幕空间到世界空间的方向Vector.
	 g_ptrCamera->get_upprojvec( _sidx.m_iX,_sidx.m_iY,t_vecRayDir );

	osVec3Normalize( &t_vecRayDir,&t_vecRayDir );

	osVec3D   t_vec3BackRayDir = t_vecRayDir;

	os_Ray pickRay(t_camPos,t_vecRayDir );

	// 使用相机位置，相机的右方顶点，pickray射线方向的一个位置，
	// 构建一个机机射线方向的平面，如果一个地表格子的四个顶点全在平面的一测，则
	// 这个格子肯定不和相机的射线相交。因为地表格子是凸面体。
	t_vec3RayEnd = t_camPos + t_vecRayDir;
	osPlaneFromPoints( &t_pickPlane,&t_camPos,&t_camRightVec,&t_vec3RayEnd );


	float   t_fBridgeColl = 10001.0f;

	for (int i =0 ; i < m_iActMapNum;i++)
	{
		os_aabbox box;
		osc_TileGroundPtr TilePtr = NULL;
		TilePtr = m_arrPtrTG[i];

		// 以四叉树为基准的测试:
		const os_tileIdx*   t_ptrChunkIdx;
		const osVec2D*      t_ptrChunkXZ;
		int                t_iPickChunkNum = 0;

		// 从当前TileGround的QuadTree得到相交的Chunk列表
		t_iPickChunkNum = TilePtr->m_sQuadTree.get_pickChunkList( 
			       t_camPos,t_vecRayDir,t_ptrChunkIdx,t_ptrChunkXZ );
		if( 0 == t_iPickChunkNum )
			continue;


		// 对每一个ray相交的Chunk进行更细致的处理
		for( int t_i=0;t_i<t_iPickChunkNum;t_i ++ )
		{
			osVec3D    t_interPT;
			osVec3D    t_poly[4];

			// 
			// 开始查找可能相交的tile格子	列表。
			int t_tileNum = m_sChunkPick.get_pickTile(
				t_camPos,t_vecRayDir,t_ptrChunkXZ[t_i].x,t_ptrChunkXZ[t_i].y );

			// 对每一个tile格子查看是否相交
			const os_tileIdx*   t_ptrIdx = m_sChunkPick.get_testTileArr();
			int   t_iBeginx = t_ptrChunkIdx[t_i].m_iX*16;
			int   t_iBeginy = t_ptrChunkIdx[t_i].m_iY*16;
			int   t_iTx,t_iTz;

			for( int t_i=0;t_i<t_tileNum;t_i ++ )
			{
				t_iTx = t_iBeginx + t_ptrIdx[t_i].m_iX;
				t_iTz = t_iBeginy + t_ptrIdx[t_i].m_iY;

				if( (t_iTx >= MAX_GROUNDWIDTH) ||
					(t_iTz >= MAX_GROUNDWIDTH) )
					continue;

				TilePtr->get_polyVertex( t_poly,t_iTx,t_iTz );

				// 
				// 先做大体的检测,根据凸多边原理,此处可以过滤掉大半的四边形
				t_arrPPClip[0] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[0] );
				t_arrPPClip[1] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[1] );
				t_arrPPClip[2] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[2] );
				t_arrPPClip[3] = 
					osn_mathFunc::classify_PointByPlane( t_pickPlane,t_poly[3] );
				if( (t_arrPPClip[0] == t_arrPPClip[1])&&
					(t_arrPPClip[1] == t_arrPPClip[2])&&
					(t_arrPPClip[2] == t_arrPPClip[3]) )
					continue;

				// 做三角形级别的相交测试
				if( pick_nearestTri( t_poly,pickRay,
					_pickVec,t_camPos,t_fdistCamPosToSelect ) )
					t_hr = true;
			}
		}
	}

	// 
	// River mod @ 2008-10-16:加入跟桥类物品的交点,用于点击桥类物品时，精确的走到桥上
	// River added @ 2009-5-4:缩放类的物品，在bridge上点击时的处理
	if( get_raypickBridge( t_camPos,t_vecRayDir,t_fBridgeColl ) )
	{
		if( t_hr )
		{
			if( osVec3Length( &(t_camPos - _pickVec) ) > t_fBridgeColl )
				_pickVec = t_camPos + t_vec3BackRayDir * t_fBridgeColl;
		}
		else
		{
			t_hr = true;
			_pickVec = t_camPos + t_vec3BackRayDir * t_fBridgeColl;
		}
	}

	return t_hr;

	unguard;
}




/** \brief
*  得到当前鼠标点击所在格子的索引。
*
*  这个索引值是在整张大地图上的格子的索引值。必须由服务器确认才有效。
*
*  \param _dix   引擎内部处理完成后输出到客户程序的值.
*  \param _sidx  输入到引擎内部用于处理的屏幕坐标. 
*  
*  River @ 2004-6-25: 加入格子高度的考虑，使用更加精确的鼠标点取功能。
*  算法描述：
*  得到相机与地平面交点的射线。
*  把相机的位置投影到二维的平面和地平面的交点连成一条射线。
*  从相机所在的格子出发，得到这条射线与当前格子的哪一条边相交，
*  根据相交的边和交点，得到当前射线通过的下一个格子，检查当前相
*  机的射线在三维中能否和这个格子相交。
* 
*  如果不能相交，从交点出发，得到新的射线，重复上一步的操作。
*  如果到达了终点以后，还没有找到相交的格子，则实际相交格子低于地平线，
*  这种情况先不计算，使用相机射线和地平面相交得到的格子*  做为最终的相交格子。
*  
*  River add@ 2004-9-16: 
*  如果射线的方向高于水平面，则返回false.即朝上的方向去选取，此时点击的位置为天空。
*  windy mod@ 2006-2-8
*  新的鼠标点选，原来的算法暂时不用
*  
*/
bool osc_TGManager::get_pickTileIdx( os_pickTileIdx& _dix,
									os_tileIdx& _sidx,osVec3D* _intersectPos/* = NULL*/ )
{
	guard;
	osVec3D InterPT;
	osVec3D t_vec;
	int             t_x,t_z;

	//! River @ 2009-8-12: 如果地图没有调入，直接返回
	if( !m_bLoadedMap )
		return false;

	if (get_newpickTileIdx(InterPT,_sidx)){

		t_x = int(InterPT.x/TILE_WIDTH);
		t_z = int(InterPT.z/TILE_WIDTH);

		// 如果x,z索引大于地图的最大索引，返回错。
		t_x = t_x<0?0:t_x;
		t_x = t_x>=m_iMaxX?m_iMaxX:t_x;
		t_z = t_x<0?0:t_z;
		t_z = t_z>=m_iMaxZ?m_iMaxZ:t_z;
		

		_dix.m_tgIdx.m_iX = t_x/MAX_GROUNDWIDTH;
		_dix.m_tgIdx.m_iY = t_z/MAX_GROUNDHEIGHT;

		t_vec.x = InterPT.x - ( _dix.m_tgIdx.m_iX*MAX_GROUNDWIDTH*TILE_WIDTH );
		t_vec.z = InterPT.z - ( _dix.m_tgIdx.m_iY*MAX_GROUNDHEIGHT*TILE_WIDTH );

		_dix.m_colTileIdx.m_iX = WORD(t_vec.x / (TILE_WIDTH/2));
		_dix.m_colTileIdx.m_iY = WORD(t_vec.z / (TILE_WIDTH/2));


		m_vec3PickPos = InterPT;
		if( _intersectPos )
			*_intersectPos = InterPT;

		return true;

	}
	//!处理点到天上的情形
	float			t_Distance = 30.0f;

	osVec3D         t_vecRayDir,t_vec3RayEnd,t_camPos,t_camRightVec;
	osPlane         t_pickPlane;

	bool            t_hr = false;
	float	        t_fdistCamPosToSelect = 10000000.0f;

	if( !g_ptrCamera )
		return false;

	// 得到从屏幕空间到世界空间的方向Vector.
	g_ptrCamera->get_upprojvec( _sidx.m_iX,_sidx.m_iY,t_vecRayDir );
	
	osVec3Normalize(&t_vecRayDir,&t_vecRayDir);
	osVec3D destPos = m_vec3HeroPos + t_vecRayDir*t_Distance;

	t_x = int(destPos.x/TILE_WIDTH);
	t_z = int(destPos.z/TILE_WIDTH);

	// 如果x,z索引大于地图的最大索引，返回错。
	if( (t_x<0)||(t_x>=m_iMaxX)||
		(t_z<0)||(t_z>=m_iMaxZ)  )
	{
		//osassertex(false,"x,z索引大于地图的最大索引");
		//@{
		// River mod @ 2007-4-17:加入稳定的错误处理机制
		if( t_x < 0 )
			t_x = 0;
		if( t_x >= m_iMaxX )
			t_x = m_iMaxX-1;
		if( t_z < 0 )
			t_z = 0;
		if( t_z >= m_iMaxZ )
			t_z = m_iMaxZ-1;
		//@}
	}

	_dix.m_tgIdx.m_iX = t_x/MAX_GROUNDWIDTH;
	_dix.m_tgIdx.m_iY = t_z/MAX_GROUNDHEIGHT;

	t_vec.x = destPos.x - ( _dix.m_tgIdx.m_iX*MAX_GROUNDWIDTH*TILE_WIDTH );
	t_vec.z = destPos.z - ( _dix.m_tgIdx.m_iY*MAX_GROUNDHEIGHT*TILE_WIDTH );

	_dix.m_colTileIdx.m_iX = WORD(t_vec.x / (TILE_WIDTH/2));
	_dix.m_colTileIdx.m_iY = WORD(t_vec.z / (TILE_WIDTH/2));


	m_vec3PickPos = destPos;
	if( _intersectPos )
		*_intersectPos = destPos;

	
	
	return true;
	
	unguard;
}


/** \brief
*  客户端的碰撞检测函数.
*  
*  如果客户端碰撞检测不能通过，则不需要再由服务器进行确认，
*  如果能通过，则由服务器确认后进行一系列的动作和操作。 
*  
*/
bool osc_TGManager::collision_detection( const os_tileIdx& _tile )
{
    guard;


# if __QTANG_VERSION__

	// River @ 2010-9-13:加入保证机制，确保底层不会出问题。
	if( (_tile.m_iX > 256)||(_tile.m_iY > 256) )
		return false;

	//！ 简单算法，上层的碰撞信息
	int t_idx = _tile.m_iY*m_iMaxX*2 + _tile.m_iX;
	if( m_pMapCollInfo )
		return m_pMapCollInfo[t_idx];

# else
	int    t_iMapIdxX,t_iMapIdxY;
	int    t_iTileX,t_iTileY;


	t_iMapIdxX = _tile.m_iX/GROUNDMASK_WIDTH;
	t_iMapIdxY = _tile.m_iY/GROUNDMASK_WIDTH;
	t_iTileX = _tile.m_iX%GROUNDMASK_WIDTH;
	t_iTileY = _tile.m_iY%GROUNDMASK_WIDTH;

	//
	// 得到我们使用的地图，然后使用这个内存地图的碰撞信息。
	for( int t_i=0;t_i<this->m_iActMapNum;t_i++ )
	{
		osassert( this->m_arrPtrTG[t_i] );

		if( (m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosX == t_iMapIdxX)&&
			(m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosZ == t_iMapIdxY ) )
		{
			const os_TileCollisionInfo*    t_info;
			os_tileIdx    t_tileIdx;

			t_tileIdx.m_iX = t_iMapIdxX;
			t_tileIdx.m_iY = t_iMapIdxY;

			t_info = get_sceneColData( t_tileIdx );
			osassert( t_info );

			return !t_info[t_iTileY*GROUNDMASK_WIDTH+t_iTileX].m_bHinder;
		}
	}
# endif 
	// 
	// 如果到达此处，则格子不在当前可见的场景内部，
	// 返回不能通过的信息。
	return false;

	unguard;
}




/** \brief
*  当前传入的地图是否已经被调入内存。
*/
bool osc_TGManager::is_tgLoaded( int _x,int _y )
{
	guard;

	for( int t_i=0;t_i<this->m_iActMapNum;t_i++ )
	{
		osassert( m_arrPtrTG[t_i] );

		if( ( m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosX == _x)&&
			( m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosZ == _y)  )
		{
			return true;
		}
	}

	return false;

	unguard;
}



/** \brief
*  由传入的世界空间点计算当前世界空间点在哪一个地图上，在地图的哪一个图块上.
*  
*  \param  _res     使用os_pickTileIdx结构返回我们得到的值.
*  \param  _pos   　传入世界空间的顶点位置。
*/
bool osc_TGManager::get_sceneTGIdx( 
	          os_pickTileIdx& _res,const osVec3D& _pos )
{
	guard;

	float                 t_x,t_y;

	osVec3D newPos = _pos;
	newPos.x = _pos.x >= 0?_pos.x:0;
	newPos.z = _pos.z >= 0?_pos.z:0;

	

	_res.m_tgIdx.m_iX = WORD( newPos.x / (MAX_GROUNDWIDTH*TILE_WIDTH) );
	_res.m_tgIdx.m_iY = WORD( newPos.z / (MAX_GROUNDHEIGHT*TILE_WIDTH));

	t_x = newPos.x - (_res.m_tgIdx.m_iX*MAX_GROUNDWIDTH*TILE_WIDTH);
	t_y = newPos.z - (_res.m_tgIdx.m_iY*MAX_GROUNDHEIGHT*TILE_WIDTH);
	_res.m_colTileIdx.m_iX = WORD( t_x / (TILE_WIDTH/2) );
	_res.m_colTileIdx.m_iY = WORD( t_y / (TILE_WIDTH/2) );

	//
	// 如果超出当前大地图的高度或是宽度，返回错误,但还是有返回值。
	if( (_res.m_tgIdx.m_iX > (m_iMaxXTgNum)) ||
		(_res.m_tgIdx.m_iY > (m_iMaxZTgNum) ) )
		return false;

	//
	// 如果当前的地图还没有被调入，高度值设为0.
	if( !is_tgLoaded( _res.m_tgIdx.m_iX,_res.m_tgIdx.m_iY ) )
	{
		_res.m_fTileHeight = 0.0f;
		return false;
	}


	//
	// 得到当前格子的高度值。
	osVec3D   t_vec3Pos = newPos;
	if( !this->get_detailPos( t_vec3Pos ) )
	{
		_res.m_fTileHeight = 0.0f;
		return false;
	}


	_res.m_fTileHeight = t_vec3Pos.y;
	return true;

	unguard;
}

/** \brief
*  根据一个地图坐标，得到当前地图坐标中心点内对应的三维，如果当前坐标不在场景内，
*  则对应的三维坐标的y值为0.0f.
*/
bool osc_TGManager::get_tilePos( int _x,int _y,osVec3D& _pos,BOOL _precision/* = FALSE*/  )
{
	guard;


	// TEST CODE:
	//osassertex( !( (_x<0) || (_x>=(m_iMaxX*2) ) || (_y<0) || (_y>=(m_iMaxZ*2) ) ),
	//	va( "The X,Y cord is:<%d,%d>,MaxX,MaxZ is:<%d,%d>..\n",_x,_y,m_iMaxX,m_iMaxZ ) );
	// River @ 2011-4-10:在极限情况下，不让客户端挂掉，先能继续下去再说。
	if( (_x<0) || (_x>=(m_iMaxX*2) ) || (_y<0) || (_y>=(m_iMaxZ*2) ) )
	{
		int t_x = 0;
		int t_y = 0;

		if( _x >= (m_iMaxX*2) )
			t_x = (m_iMaxX*2) - 1;
		if( _y >= (m_iMaxZ*2) )
			t_y = (m_iMaxZ*2) - 1;

		_pos.x = t_x*(HALF_TWIDTH)+HALF_CTWIDTH;
		_pos.z = t_y*(HALF_TWIDTH)+HALF_CTWIDTH;
		_pos.y = 0.0f;

		return false;
	}

	// 传入的值为碰撞格子值。
	osassert( (_x>=0) );
	osassert( (_x<(m_iMaxX*2))  );
	osassert( (_y>=0) );
	osassert( (_y<(m_iMaxZ*2)) );

	os_tileIdx   t_tgIdx;
	int         t_iTx,t_iTy;

	t_tgIdx.m_iX = (_x/2)/MAX_GROUNDWIDTH;
	t_tgIdx.m_iY = (_y/2)/MAX_GROUNDHEIGHT;

	t_iTx = _x%(GROUNDMASK_WIDTH);
	t_iTy = _y%(GROUNDMASK_WIDTH);

	_pos.x = _x*(HALF_TWIDTH)+HALF_CTWIDTH;
	_pos.z = _y*(HALF_TWIDTH)+HALF_CTWIDTH;

	//
	// 如果当前的地图还没有被调入，高度值设为0.
	if( !is_tgLoaded( t_tgIdx.m_iX,t_tgIdx.m_iY ) )
	{
		_pos.y = 0.0f;
		return false;
	}

	return get_detailPos( _pos,_precision ); 

	unguard;
}
void osc_TGManager::get_collision_info(const osVec3D& _pos,float _radius /*= 2*/)
{
	mPassLineListList.clear();
	//mPassLineList.clear();
	//mUnPassLineList.clear();
	os_pickTileIdx TileIdx;
	get_sceneTGIdx(TileIdx,_pos);
	int halfsize = (int)_radius;
	for (int iX = TileIdx.m_colTileIdx.m_iX - halfsize;iX < TileIdx.m_colTileIdx.m_iX + halfsize;iX++)
	{
		std::vector<osVec3D> mPassLineList;
		for (int iY = TileIdx.m_colTileIdx.m_iY - halfsize;iY < TileIdx.m_colTileIdx.m_iY + halfsize;iY++)
		{
			/*if (iX >=0&&iX<=127&&iY >=0&&iY<=127)
			{*/
				os_tileIdx idx;
				idx.m_iX = TileIdx.m_tgIdx.m_iX*GROUNDMASK_WIDTH + iX;
				idx.m_iY = TileIdx.m_tgIdx.m_iY*GROUNDMASK_WIDTH + iY;

				/*
				00 10
				01 11
				*/
				osVec3D v00(idx.m_iX*1.5f,0,idx.m_iY*1.5f),
					v10((idx.m_iX+1)*1.5f,0,idx.m_iY*1.5f),
					v01(idx.m_iX*1.5f,0,(idx.m_iY+1)*1.5f),
					v11((idx.m_iX+1)*1.5f,0,(idx.m_iY+1)*1.5f);
				get_detailPos(v00);
				get_detailPos(v10);
				get_detailPos(v01);
				get_detailPos(v11);

				/*get_tilePos(iX,iY,v00);
				get_tilePos(iX+1,iY,v10);
				get_tilePos(iX,iY+1,v01);
				get_tilePos(iX+1,iY+1,v11);
				*/

				if (collision_detection(idx))
				{
					mPassLineList.push_back(v00);
					mPassLineList.push_back(v10);

					mPassLineList.push_back(v10);
					mPassLineList.push_back(v11);

					mPassLineList.push_back(v11);
					mPassLineList.push_back(v01);

					mPassLineList.push_back(v01);
					mPassLineList.push_back(v00);
				}
				
			//}
		}
		mPassLineListList.push_back(mPassLineList);
	}

}
# define DETAIL_TILENUM2	(DETAIL_TILENUM*2)

//  ATTENTION TO OPP:
//! 得到每一个碰撞格子细分成15份的格子三维空间位置.
bool osc_TGManager::get_detailTilePos( int _x,int _y,osVec3D& _pos )
{
	guard;

	int         t_iXCTile,t_iYCTile;
	os_tileIdx   t_tgIdx;
	int          t_iTx,t_iTy;

	t_iXCTile = _x/DETAIL_TILENUM;
	t_iYCTile = _y/DETAIL_TILENUM;
	//@{
	// Windy mod @ 2005-11-7 15:52:49
	//Desc: 在一个格子内的相对数值，插值用
	int t_i64X,t_i64Y;
	
	float t_u,t_v;
	t_u = (float)(_x%DETAIL_TILENUM2);
	t_v = (float)(_y%DETAIL_TILENUM2);
	t_u /= DETAIL_TILENUM*2.0f; 
	t_v /= DETAIL_TILENUM*2.0f; 

	//@}

	// 先得到三维空间位置的Y高度.
	t_tgIdx.m_iX = (t_iXCTile/2)/MAX_GROUNDWIDTH;
	t_tgIdx.m_iY = (t_iYCTile/2)/MAX_GROUNDHEIGHT;
	t_iTx = t_iXCTile%(GROUNDMASK_WIDTH);
	t_iTy = t_iYCTile%(GROUNDMASK_WIDTH);
	//@{
	// Windy mod @ 2005-11-7 15:52:49
	t_i64X = t_iTx/2;
	t_i64Y = t_iTy/2;
	osassertex(t_i64X>=0&&t_i64X<MAX_GROUNDWIDTH,
		va( "The wrong val is:<%d>..\n",t_i64X )  );
	osassertex(t_i64Y>=0&&t_i64Y<MAX_GROUNDWIDTH,
		va( "The wrong val is:<%d>..\n",t_i64Y ) );
	//@}

	//
	// 得到x,z的三维坐标.细分的坐标相当于以0.1米为单位.
	_pos.x = 0.1f * _x + 0.05f;
	_pos.z = 0.1f * _y + 0.05f;

	if( !is_tgLoaded( t_tgIdx.m_iX,t_tgIdx.m_iY ) )
	{
		_pos.y = 0.0f;
		return false;
	}

	// windy add 4-27 
	const float* t_tileColHeightInfo;
	t_tileColHeightInfo = get_sceneColHeightData(t_tgIdx);
	osassert( t_tileColHeightInfo);
	

	float origX = (_x/DETAIL_TILENUM2)*TILE_WIDTH;
	float origY = (_y/DETAIL_TILENUM2)*TILE_WIDTH;
	osVec3D v00( origX,
		t_tileColHeightInfo[65*t_i64Y+t_i64X],
				 origY);
	osVec3D v10( origX +TILE_WIDTH,
		t_tileColHeightInfo[65*t_i64Y+t_i64X+1],
				origY);
	osVec3D v01( origX,
		t_tileColHeightInfo[65*(t_i64Y+1)+t_i64X],
				origY + TILE_WIDTH);
	osVec3D v11( origX +TILE_WIDTH,
		t_tileColHeightInfo[65*(t_i64Y+1)+t_i64X+1],
				origY+TILE_WIDTH);


	osPlane t_plane;
	if (t_u+t_v<1.0f)
	{
		osPlaneFromPoints(&t_plane,&v01,&v10,&v00);
		_pos.y = (-t_plane.d-_pos.x*t_plane.a-_pos.z*t_plane.c)/t_plane.b;
	}
	else{
		osPlaneFromPoints(&t_plane,&v01,&v10,&v11);
		_pos.y = (-t_plane.d-_pos.x*t_plane.a-_pos.z*t_plane.c)/t_plane.b;
	}
	
	return true;

	unguard;
}

/** \brief
*  给出场景内的一个xz值，得到此处的地表高度
*
*  \param osVec3D& _pos x,z的值由_pos内的x,z值给出.返回的高度值在_pos.y内
*
*  River @ 2007-4-3:如果某一个格子的碰撞信息不可通过,则这个格子相连的格子的高
*                   度信息需要经过特殊的处理.
*/
bool osc_TGManager::get_detailPos( osVec3D& _pos,BOOL _precision/* = FALSE*/ )
{
	guard;


	os_tileIdx                    t_tgIdx;
	float                         t_fU,t_fV,t_fOrigX,t_fOrigZ;
	int                           t_iColX,t_iColZ;

	_pos.y = 0.0f;

	// River mod @ 2007-4-18:
	if( (_pos.x < 0.0f)||(_pos.z<0.0f)||
		(_pos.x > g_fMapMaxX) || (_pos.z > g_fMapMaxZ) )
		return false;


	// 先算出整张大地图格子的索引
	t_fU = _pos.x / TILE_WIDTH;
	t_fV = _pos.z / TILE_WIDTH;
	t_iColX = int(t_fU);
	t_iColZ = int(t_fV);

	// 得到碰撞信息
	t_tgIdx.m_iX = WORD(t_iColX / MAX_GROUNDWIDTH);
	t_tgIdx.m_iY = WORD(t_iColZ / MAX_GROUNDWIDTH);

	// River mod @ 2007-8-26:
	// 对于激活地图列表相关的处理，使用临界区，界为上层使用了多线程
	::EnterCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );

	// windy add 4-27
	if( !is_tgLoaded( t_tgIdx.m_iX,t_tgIdx.m_iY ) )
	{
		_pos.y = 0.0f;
		
		::LeaveCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );

		return false;
	}
	const float* t_tileColHeightInfo;
	t_tileColHeightInfo = get_sceneColHeightData(t_tgIdx);
	if (!t_tileColHeightInfo)
	{
		_pos.y = 0.0f;
		::LeaveCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );
		return false;
	}
	osassertex( t_tileColHeightInfo,
		va( "The tg idx is:<%d,%d>..pos is:<%f,%f,%f>..\n",
		t_tgIdx.m_iX,t_tgIdx.m_iY,_pos.x,_pos.y,_pos.z ) );

	// 算传入坐标在一个显示格子内的uv.
	t_fU = t_fU - t_iColX;
	t_fV = t_fV - t_iColZ;

	// 单独显示格子的原点坐标x,z值
	t_fOrigX = t_iColX*TILE_WIDTH;
	t_fOrigZ = t_iColZ*TILE_WIDTH;

	t_iColX -= (t_tgIdx.m_iX*MAX_GROUNDWIDTH);
	t_iColZ -= (t_tgIdx.m_iY*MAX_GROUNDWIDTH);

	// 构建碰撞格子的信息
	//t_iColX *= 2;
	//t_iColZ *= 2;

	osVec3D v00( t_fOrigX,
		t_tileColHeightInfo[65*t_iColZ+t_iColX],
		t_fOrigZ );
	osVec3D v10( t_fOrigX +TILE_WIDTH,
		t_tileColHeightInfo[65*t_iColZ+t_iColX+1],
		t_fOrigZ );
	osVec3D v01( t_fOrigX,
		t_tileColHeightInfo[65*(t_iColZ+1)+t_iColX],
		t_fOrigZ + TILE_WIDTH );
	osVec3D v11( t_fOrigX +TILE_WIDTH,
		t_tileColHeightInfo[65*(t_iColZ+1)+t_iColX+1],
		t_fOrigZ+TILE_WIDTH );

	osPlane t_plane;
	if( (t_fU+t_fV)<1.0f )
	{
		osPlaneFromPoints(&t_plane,&v01,&v10,&v00);
		_pos.y = (-t_plane.d-_pos.x*t_plane.a-_pos.z*t_plane.c)/t_plane.b;
		osassertex( !_isnan( _pos.y ),"平面高度浮点数造成的问题" );
	}
	else
	{
		osPlaneFromPoints(&t_plane,&v01,&v10,&v11);
		_pos.y = (-t_plane.d-_pos.x*t_plane.a-_pos.z*t_plane.c)/t_plane.b;
		osassertex( !_isnan( _pos.y ),"平面1高度浮点数造成的问题" );
	}

	// 
	// River mod @ 2007-4-4:加入更精确的高度检测信息:
	if( _precision )
	{

		float t_fHeight = get_objHeightInfo( _pos );

		// River mod @ 2007-6-8:加入地表高度的处理
		// River mod @ 2007-6-9:如果地表上此时没有桥类物品，则使用原来的碰撞高度信息
		if( t_fHeight > -10000.0f )
		{
			get_earthHeight( _pos );
			osassertex( !_isnan( _pos.y ),"地图资料的高度不是浮点数造成的问题" );
		}

		if( t_fHeight > _pos.y )
		{
			_pos.y = t_fHeight;
			osassertex( !_isnan( _pos.y ),"高度资料的高度不是浮点数造成的问题" );
		}
	}
    

	osassertex( !_isnan( _pos.y ),"有可能是地图资料的高度不是浮点数造成的问题" );

	// River added @ 2007-8-16:
	::LeaveCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );	

	//! River @ 2010-3-29:加入些许的偏移，如果特效在地表播放，则可以去除ZFighting现象。
	_pos.y += 0.03f;

	return true;

	unguard;
}
//! 传入世界坐标的位置,得到该位置的材质索引
/*! \note 传入的坐标只用到了X,Z分量*/
int	osc_TGManager::get_materialIdx(osVec2D& _pos,os_terrainTexSnd& _texsnd )
{
	guard;

	osassert( _pos.x >= 0.0f );
	osassert( _pos.y >= 0.0f );

	os_tileIdx  t_tgIdx;
	os_tileIdx  t_Collidx;
	float t_fU,t_fV;

	// 先算出整张大地图格子的索引
	t_fU = _pos.x / TILE_WIDTH;
	t_fV = _pos.y / TILE_WIDTH;

	t_tgIdx.m_iX = WORD(int(t_fU) / MAX_GROUNDWIDTH);
	t_tgIdx.m_iY = WORD(int(t_fV) / MAX_GROUNDWIDTH);

	t_fU = _pos.x / HALF_TWIDTH;
	t_fV = _pos.y / HALF_TWIDTH;

	t_Collidx.m_iX = WORD(int(t_fU) % GROUNDMASK_WIDTH);
	t_Collidx.m_iY = WORD(int(t_fV) % GROUNDMASK_HEIGHT);
	
	int              t_i;
	osc_TileGroundPtr   t_ptr; 


	// 更新当前的tileManager中所有TG的设备相关数据id表。
	for( t_i=0;t_i<this->m_iActMapNum;t_i++ )
	{	
		osassert( m_arrPtrTG[t_i] );
		t_ptr = this->m_arrPtrTG[t_i];

		if( !t_ptr->m_bInuse )
			continue;

		if( (t_ptr->m_sTgHeader.m_wMapPosX != t_tgIdx.m_iX )||
			(t_ptr->m_sTgHeader.m_wMapPosZ != t_tgIdx.m_iY )  )
			continue;
		int    t_idx = t_ptr->get_tgMaterialIdxInfo(t_Collidx.m_iX,t_Collidx.m_iY);
		if (t_idx==0xff)
		{
			_texsnd.m_iMaterialID = -1;
			_texsnd.mWaterHeight = 3.0f;
			_texsnd.mWaterHeight = t_ptr->get_tgWaterHeight();
			return -1;
		}

		//! river add @ 2009-2-19:加入判断，上层安全使用
		if( t_idx >= (int)m_vecTerrainTexSnd.size() )
			return -1;

		memcpy( &_texsnd,&m_vecTerrainTexSnd[t_idx],sizeof( os_terrainTexSnd ) );

		return t_idx;
	}

	return 0;

	unguard;
}


/** \biref
*  得到当前视域内的地图的数目和索引.
* 
*  客户端在寻路算法时会用到这些数据.
*  
*  \return int     返回在视域内的地图的数目.
*  \param  _tgidx  传入的返回地图索引的指针结构,最多返回4个._tgidx应该是os_tileIdx[4].
*  
*/
int osc_TGManager::get_inViewTGIdx( os_tileIdx* _tgidx )
{
	guard;

	int    t_iTGNum,t_i;

	osassert( _tgidx );

	t_iTGNum = 0;
	for( t_i=0;t_i<this->m_iActMapNum;t_i++ )
	{

		osassert( m_arrPtrTG[t_i] );

		if( m_arrPtrTG[t_i]->is_vfEntireCullOff() )
			continue;

		_tgidx[t_iTGNum].m_iX = m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosX;
		_tgidx[t_iTGNum].m_iY = m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosZ;

		t_iTGNum ++;

		// ATTENTION TO FIX: 加入更完整的处理,
		// 因为以后看的矩离更完,此处的处理变的更为复杂.
		if( t_iTGNum >= 4 )
			break;
	}

	return t_iTGNum;

	unguard;
}





/** \brief
*  得到场景中某一个地图的碰撞信息数据块。
* 
*  \param _tgIdx     场景中地图的索引，从(0,0)开始。
*  \return os_TileCollisionInfo* 返回的是os_TileCollisionInfo的二维数组。
*                                 就是os_TileCollisionInfo［128][128] 
*/
const os_TileCollisionInfo*  osc_TGManager::get_sceneColData( os_tileIdx& _tgIdx )
{
	guard;

	int              t_i;
	osc_TileGroundPtr   t_ptr; 

	
	osassertex(_tgIdx.m_iX>=0,va("地图的索引X:%d",_tgIdx.m_iX));
	osassertex(_tgIdx.m_iX<m_iMaxXTgNum,va("地图的索引X:%d,MaxTGNum:%d",_tgIdx.m_iX,m_iMaxXTgNum));

	osassertex(_tgIdx.m_iY>=0 ,va("地图的索引Y:%d", _tgIdx.m_iY));
	osassertex(_tgIdx.m_iY<m_iMaxZTgNum ,va("地图的索引Y:%d,MaxTGNum:%d", _tgIdx.m_iY,m_iMaxXTgNum));


	


	//
	// 更新当前的tileManager中所有TG的设备相关数据id表。
	for( t_i=0;t_i<this->m_iActMapNum;t_i++ )
	{	
		osassert( m_arrPtrTG[t_i] );
		t_ptr = this->m_arrPtrTG[t_i];

		if( !t_ptr->m_bInuse )
			continue;

		if( (t_ptr->m_sTgHeader.m_wMapPosX != _tgIdx.m_iX )||
			(t_ptr->m_sTgHeader.m_wMapPosZ != _tgIdx.m_iY )  )
			continue;


		return t_ptr->get_tgColInfo();

	}

	osassertex( false,"找不出要检测的碰撞信息数据" );

	return NULL;


	unguard;
}
const float* osc_TGManager::get_sceneColHeightData(os_tileIdx& _tgIdx )
{
	guard;

	int              t_i;
	osc_TileGroundPtr   t_ptr; 

	// River mod @ 2007-3-7:更加柔和的错误处理方式
	if( (_tgIdx.m_iX<0)||(_tgIdx.m_iX>=m_iMaxXTgNum)||
		(_tgIdx.m_iY<0)||(_tgIdx.m_iY>=m_iMaxZTgNum) )
	{
		return NULL;
	}

	//
	// 更新当前的tileManager中所有TG的设备相关数据id表。
	for( t_i=0;t_i<this->m_iActMapNum;t_i++ )
	{	
		osassert( m_arrPtrTG[t_i] );
		t_ptr = this->m_arrPtrTG[t_i];

		if( !t_ptr->m_bInuse )
			continue;

		if( (t_ptr->m_sTgHeader.m_wMapPosX != _tgIdx.m_iX )||
			(t_ptr->m_sTgHeader.m_wMapPosZ != _tgIdx.m_iY )  )
			continue;


		return t_ptr->get_tgColHeightInfo();

	}


	return NULL;

	unguard;

}


//@{
///关于Lens Flare的接口
void osc_TGManager::start_LensFlare()
{
	osassert(m_pLensflare);
	m_pLensflare->m_IsRender = true;
}

void osc_TGManager::stop_LensFlare()
{
	osassert(m_pLensflare);
	m_pLensflare->m_IsRender = false;
}

///从文件中加载lens flare
void osc_TGManager::Load_LensFlare(const char * _fname)
{
	guard;
	osassert(m_pLensflare);
	if (!file_exist((char *)_fname)){
		Outstr_tolog("Lens Flare: %s 文件不存在" ,_fname);
		osassertex( false, "Lens Flare 文件不存在" );
		return;
	}
	m_pLensflare->loadStdFlare(_fname);
	unguard;
}

void osc_TGManager::clear_LensFlare()
{
	osassert(m_pLensflare);
	m_pLensflare->clear();
}
void osc_TGManager::set_RealPos(osVec3D & _vec3Pos)
{
	osassert(m_pLensflare);
	m_pLensflare->setPosition(_vec3Pos);
}

///以下是编辑器的接口
# if __LENSFLARE_EDITOR__
void osc_TGManager::set_LensFlarePart(int nPartID,LFMask _Mask,FlarePart & _flarePart)
{
	osassert(m_pLensflare);
	if (_Mask&&LF_INDEX)
	{
		m_pLensflare->set_texid(nPartID,_flarePart.m_nIndex);
	}
	if (_Mask&&LF_POS)
	{
		m_pLensflare->set_pos(nPartID,_flarePart.m_fPos);
	}
	if (_Mask&&LF_SIZE)
	{
		m_pLensflare->set_size(nPartID,_flarePart.m_fSize);
	}
	if (_Mask&&LF_COLOR)
	{
		m_pLensflare->set_color(nPartID,_flarePart.m_color);
	}
	if (_Mask&&LF_ALPHA)
	{
		m_pLensflare->set_alpha(nPartID,_flarePart.m_fAlpha);
	}


}
void osc_TGManager::get_LensFlarePart(int nPartID,LFMask _Mask,FlarePart & _flarePart)
{
	osassert(m_pLensflare);
	if (_Mask&&LF_INDEX)
	{
		m_pLensflare->get_texid(nPartID,_flarePart.m_nIndex);
	}
	if (_Mask&&LF_POS)
	{
		m_pLensflare->get_pos(nPartID,_flarePart.m_fPos);
	}
	if (_Mask&&LF_SIZE)
	{
		m_pLensflare->get_size(nPartID,_flarePart.m_fSize);
	}
	if (_Mask&&LF_COLOR)
	{
		m_pLensflare->get_color(nPartID,_flarePart.m_color);
	}
	if (_Mask&&LF_ALPHA)
	{
		m_pLensflare->get_alpha(nPartID,_flarePart.m_fAlpha);
	}

}

void osc_TGManager::save_LensFlare(const char * _fname)
{
	osassert(m_pLensflare);
	m_pLensflare->save_file(_fname);
}
int	osc_TGManager::get_LensFlareNum()
{
	osassert(m_pLensflare);
	return m_pLensflare->get_partNum();
}
void osc_TGManager::add_LensFlarePart()
{
	osassert(m_pLensflare);
	m_pLensflare->addOnePart();
}
void osc_TGManager::del_LensFlarePart(int iPartID)
{
	osassert(m_pLensflare);
	m_pLensflare->delOnePart(iPartID);
}
# endif
// HDR Light
void osc_TGManager::init_hdrLight()
{
	if( g_bUseHDRLight )
	{
		m_pHDRLight = new osHDRLight;
		m_pHDRLight->init_hdrLight();
	}
}

void osc_TGManager::set_hdrParameter( float _fmx, float _fmy, float _diffuse, float _exposure )
{
	if( m_pHDRLight )
		m_pHDRLight->set_hdrParameter( _fmx, _fmy, _diffuse, _exposure );
}

void osc_TGManager::set_hdrFm( float _fm )
{
	if( m_pHDRLight )
		m_pHDRLight->set_hdrFm( _fm );
}

void osc_TGManager::set_hdrDiffuse( float _diffuse )
{
	if( m_pHDRLight )
		m_pHDRLight->set_hdrDiffuse( _diffuse );
}

void osc_TGManager::set_hdrExposure( float _exposure )
{
	if( m_pHDRLight )
		m_pHDRLight->set_hdrExposure( _exposure );
}

//
void osc_TGManager::set_hdrRenderTarget()
{
	if( m_pHDRLight )
	{
		m_pHDRLight->set_hdrRenderTarget();
	}
}
	
//--------------------------------------------------------------------------------
//@{ tzz added for galaxy shader effect
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------
//Name: gala_actGrowScene
//Desc: activate the grow scene
//--------------------------------------------------------------------------------------------------------------------
void osc_TGManager::gala_actGrowScene(BOOL _bActivate /* = TRUE*/)
{
	guard;

	osassert(m_deviceMgr);

	m_growEff.SetEffective(m_deviceMgr->get_d3ddevice(),_bActivate);

	unguard;
}



//--------------------------------------------------------------------------------------------------------------------
//Name: gala_growScene
//Desc: initialize the grow scene
//--------------------------------------------------------------------------------------------------------------------
void osc_TGManager::gala_initGrowScene(int _iScreenWidth,int _iScreenHeight) 
{
	guard;

	osassert(m_deviceMgr);

	// initialize the grow class
	m_growEff.InitGrowScene(m_deviceMgr->get_d3ddevice(),_iScreenWidth,_iScreenHeight,D3DFMT_A8R8G8B8,g_bUseFSAA);
	//
	//// initialize the corona class
	//m_coronaEff.InitCoronaEff(m_deviceMgr->get_d3ddevice(),_iScreenWidth,_iScreenHeight);

	
	unguard;
	
}
//! initialize the hdr data
void osc_TGManager::init_realHDRScene(int _iScreenWidth,int _iScreenHeight)
{
	guard;

	osassert(m_deviceMgr);

	// initialize the data of real HDR 
	m_realHDREff.InitGrowScene(m_deviceMgr->get_d3ddevice(),_iScreenWidth,_iScreenHeight);


	unguard;
}
//! activate the HDR scene effect
void osc_TGManager::act_realHDRScene(BOOL _bActivate /*= TRUE*/)
{
	guard;

	osassert(m_deviceMgr);

	m_growEff.SetEffective(m_deviceMgr->get_d3ddevice(),_bActivate);

	unguard;
}

void osc_TGManager::act_waterEff(BOOL _bAct /*= TRUE*/) 
{
	guard;

	osassert(m_deviceMgr);

	m_growEff.SetWaterActive(m_deviceMgr->get_d3ddevice(),_bAct);

	unguard;
}
//! enable light 
void osc_TGManager::enable_light(BOOL _bEnable)
{
	guard;

	m_lightManager.EnalbeLight(_bEnable);

	if(!_bEnable){
		// reset the light value
		g_fDirectLB = g_fDirectLG = g_fDirectLR = 1.0f;
		g_fAmbiB = g_fAmbiG = g_fAmbiR = 1.0f;

		g_vec3LPos.x = 0.5f;
		g_vec3LPos.y = 1.0f;
		g_vec3LPos.z = 0.5f;

		osVec3Normalize(&g_vec3LPos,&g_vec3LPos);
	}	

	unguard;
}
void osc_TGManager::set_dayTimeForLight(size_t _dayTime)
{
	guard;

	m_lightManager.SetCurrentDayTime(_dayTime);

	unguard;
}

//! set the day time for light
void osc_TGManager::set_constLight(const osColor& _color,const osVec3D& _pos,float _range)
{
	guard;

	m_lightManager.SetConstLightAttr(_color,_pos,_range);

	unguard;
}

# if GALAXY_VERSION

#include "../geomipterrain/osMapterrain.h"
//--------------------------------------------------------------------------------------------------------------------
//Name: gala_genExplode
//Desc: generate a explode effect in fx
//--------------------------------------------------------------------------------------------------------------------
void osc_TGManager::gala_genExplode(gala_explode* _explode)
{
	guard;
	osassert(MapTile::m_ptrMapTerrain);
	(MapTile::m_ptrMapTerrain)->gala_initExplode(_explode);
	unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name: gala_genBullet
//Desc: generate the Y position of very vertexes
//--------------------------------------------------------------------------------------------------------------------
//void osc_TGManager::gala_genBullet(gala_spaceHole* _bullet)
//{
//	guard;
//	osassert(MapTile::m_ptrMapTerrain);
//	(MapTile::m_ptrMapTerrain)->gala_initBullet(_bullet);
//	unguard;
//}
//--------------------------------------------------------------------------------------------------------------------
//Name: gala_delBullet
//Desc: delete the bullet shader effect
//--------------------------------------------------------------------------------------------------------------------
void osc_TGManager::gala_delSpaceHole(int _index) 
{
	guard;
	osassert(MapTile::m_ptrMapTerrain);
	(MapTile::m_ptrMapTerrain)->gala_delSpaceHole(_index);
	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: gala_refreshSpaceHole
//Desc: refresh the bullet position and direction
//--------------------------------------------------------------------------------------------------------------------
void osc_TGManager::gala_refreshSpaceHole(gala_spaceHole* _hole)
{
	guard;
	osassert(_hole);
	(MapTile::m_ptrMapTerrain)->gala_frameMove(_hole);
	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: gala_renderHole
//Desc: render the space hole effect for galaxy
//--------------------------------------------------------------------------------------------------------------------
void osc_TGManager::gala_renderHole(int _iScreenWidth,int _iScreenHeight,int _iWidthNum,int _iHeightNum,int _iStartX,int _iStartY) 
{
	guard;

	m_gridRect.InitScreenWH(_iScreenWidth,_iScreenHeight);


	osassert(m_deviceMgr);
	m_gridRect.InitGridRes(m_deviceMgr->get_d3d(),m_deviceMgr->get_d3ddevice(),_iWidthNum,_iHeightNum,_iStartX,_iStartY);


	m_deviceMgr->d3d_begindraw();
	m_deviceMgr->d3d_clear( 0,NULL,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 
		0x0, 1.0f, 0L );

	os_sceneFM  t_sFm;
	t_sFm.m_iCx = 128;
	t_sFm.m_iCy = 128;
	t_sFm.m_ptrCam = g_ptrCamera;
	t_sFm.m_vecChrPos = osVec3D( 128*1.5f,0.0f,128*1.5f );
	frame_move( &t_sFm );

	render_scene();

	m_deviceMgr->d3d_enddraw();
	m_deviceMgr->d3d_present();

	m_gridRect.RestoreRenderTarget(m_deviceMgr->get_d3ddevice());
	

	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: gala_isRenderHole
//Desc: is rendering hole
//--------------------------------------------------------------------------------------------------------------------
BOOL osc_TGManager::gala_isRenderHole(void)
{
	return m_gridRect.IsReadyToRender();
}
//--------------------------------------------------------------------------------------------------------------------
//Name: gala_growScene
//Desc: initialize the grow scene
//--------------------------------------------------------------------------------------------------------------------
void osc_TGManager::gala_initGrowScene(int _iScreenWidth,int _iScreenHeight) 
{
	guard;

	osassert(m_deviceMgr);

	// initialize the grow class
	//m_growEff.InitGrowScene(m_deviceMgr->get_d3ddevice(),_iScreenWidth,_iScreenHeight);
	//
	//// initialize the corona class
	//m_coronaEff.InitCoronaEff(m_deviceMgr->get_d3ddevice(),_iScreenWidth,_iScreenHeight);

	
	unguard;
	
}


//--------------------------------------------------------------------------------------------------------------------
//Name: gala_actHeroCorona
//Desc: activate the corona around the hero
//--------------------------------------------------------------------------------------------------------------------
void osc_TGManager::gala_actHeroCorona(BOOL _bActivate /* = TRUE*/)
{
	m_bCorona = _bActivate;
}

//! refresh the blood process
void osc_TGManager::gala_showBlood(const osVec2D& _craftPos,float _fCraftScale,float _fMaxHP,float _fCurrHP)
{
	guard;

	osVec3D t_scrPos;

	//osassert(g_ptrCamera);

	//g_ptrCamera->get_scrCoodFromVec3(_craftPos,t_scrPos);

	m_vecBlood[m_iCurrBlood].RefreshBlood(_craftPos,_fCraftScale,_fMaxHP,_fCurrHP);

	m_iCurrBlood++;

	unguard;
}

//! display the blood
void osc_TGManager::DisplayBlood()

{
	//clear the pointer of blood index
	m_iCurrBlood = 0;

	LPDIRECT3DDEVICE9		t_lpd3d = m_deviceMgr->get_d3ddevice();

	D3DVIEWPORT9   			t_viewport;  	
	g_ptrCamera->get_viewport(t_viewport);

	// confirm the t_viewport is full of windows

	//osassert(false);

	////osassert(false); // set the ortho camera
	VALIDATE_D3D(SetOrthoCamera(t_lpd3d,
		D3DXVECTOR3(t_viewport.Width / 2.0f,t_viewport.Height / 2.0f,-1.0f),
		D3DXVECTOR3(t_viewport.Width / 2.0f,t_viewport.Height / 2.0f,1.0f),
		(float)t_viewport.Width,
		(float)t_viewport.Height,
		0.0f,
		10.0f));

	// set the state to render the blood process
	
	//osassert(CBlood::sm_lpBloodTex);

	//VALIDATE_D3D(t_lpd3d->SetTexture(0,NULL));

	//t_lpd3d->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    //t_lpd3d->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	//t_lpd3d->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

   // t_lpd3d->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
	//t_lpd3d->SetTextureStageState( 0, D3DTSS_ALPHAARG1,   D3DTA_TEXTURE );
	//t_lpd3d->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
	
	t_lpd3d->SetRenderState(D3DRS_LIGHTING,FALSE);
	t_lpd3d->SetRenderState(D3DRS_ZENABLE,FALSE);
	t_lpd3d->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);

	//t_lpd3d->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
	//t_lpd3d->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	//t_lpd3d->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

	t_lpd3d->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE );


	const int& t_size = m_vecBlood.size();

	for(int i = 0;i< t_size ; i++){
		m_vecBlood[i].DisplayBlood(m_deviceMgr->get_d3ddevice());
		m_vecBlood[i].Activate(FALSE);
	}		

	//t_lpd3d->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
	//t_lpd3d->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE );
}
# endif //GALAXY_VERSION

//--------------------------------------------------------------------------------
//@}
//--------------------------------------------------------------------------------