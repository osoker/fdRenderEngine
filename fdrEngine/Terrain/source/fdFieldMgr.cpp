//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdFieldMgr.cpp
 *
 *  His:      River created @ 2003-12-23
 *
 *  Des:      ���ڹ����ͼ���ӵ��࣬ʹ�õ�ǰ��������õ���ǰӦ�õ�����һ�ŵ�ͼ��
 *   
 *
 * ����ּ���֮�ǣ����������������ǽ֮��Ҳ����   
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

//! ��������Ҫ��Ⱦ��͸����Ʒ�Ͳ�͸����Ʒ����Ŀ
# define  MAX_ALPHAOBJNUM     512
# define  MAX_OPACITYOBJNUM   512


//! �����������գ��������߳��ȣ��ڴ˳��ȴ������۶�
# define  MAX_RAYDISDANCE  20


//! River @ 2010-6-24:ȫ�ֵı�����ȷ���Ƿ���Ҫ��Ⱦ��Ļ�Ⲩ.
BOOL      g_bRenderScreenWave = FALSE;

OSENGINE_API BOOL		g_bRendSky = TRUE;
//! �Ƿ���µ���
OSENGINE_API BOOL	   g_bUpdateTile = TRUE;
//! �Ƿ���ʾtileΪ�߿�ģʽ.
OSENGINE_API BOOL      g_bTileWireFrame = false;

//! �Ƿ���ʾ��ƷΪ��ģ��ģʽ.
OSENGINE_API BOOL      g_bObjWireFrame = FALSE;


//! �Ƿ�ʹ�ڵ�ס���������Ʒ��ð�͸����
OSENGINE_API BOOL      g_bAlphaShelter = TRUE;


//! �Ƿ���ʾ��̬��obj��Ʒ��
OSENGINE_API BOOL      g_bDisplayStaticMesh = TRUE;


//! �Ƿ�ʹ�������
OSENGINE_API BOOL      g_bUseVolumeFog = TRUE;

//! ������Ƿ��ʼ����
OSENGINE_API BOOL      g_bVolumeFogInited = FALSE;

//! River @ 2010-6-25:�Ƿ�ʹ����Ļ�ⲨЧ��.
OSENGINE_API BOOL      g_bScreenBlastWave = false;                  


# if __QTANG_VERSION__
//! TGMgr������tileGround����Ŀ,Q����Ŀ��ʹ�����2*2�ĵ�ͼ
OSENGINE_API int     MAXTG_INTGMGR = 4;
# else
//! TGMgr������tileGround����Ŀ
OSENGINE_API int     MAXTG_INTGMGR = 16;
# endif 

float gOSAPLAYERSpeed = 1.0f;

//! ����ÿһ�����ӵ�����ֵ��
const int MASK_TILEDISPLAY     =      (1<<5);
const int MASK_WATERTILE       =      (1<<6);



//! ���峡���и�������ʵ�����еĴ�С.
//! ʹ�ñ�������,ʹ���ⲿ�����޸ĸ��ӵĴ�С.
OSENGINE_API float    TILE_WIDTH = 3.0f;
//! �����ͼ���ӵĿ�ȡ�
OSENGINE_API float    HALF_TWIDTH = 1.5f;
//! �����ײ��ͼ���ӵĿ�ȡ�
OSENGINE_API float    HALF_CTWIDTH = 0.75f;

//! ������������Ƿ���Ⱦ�����еĵ��Ρ�
OSENGINE_API BOOL     g_bRenderTerr = true;
OSENGINE_API BOOL     g_bRenderCollision = FALSE;
OSENGINE_API BOOL     g_bTransparentWater = TRUE;



//! ������Ч��������صĽӿ�
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

	//! ��ʼ���ɼ����Ĵ�С.
	m_vecVisualSet.resize( INIT_TILEVSSIZE );
	m_vecVASet.resize( INIT_VATNUM );
	m_iVATNum = 0;


	m_deviceMgr = ::get_deviceManagerPtr();

	// ����̬�ĳ�Ա������ָ�븳ֵ��
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

	// ��ʼ��skinMesh������.
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
	//! River added @ 2009-5-19:Ϊ�˵�ͼ����ײ��Ϣ����������ϲ�һ�¡�
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

	// �˳�������Ⱦ����ʱ���ͷ�
	SAFE_RELEASE( m_pDepthStencilBackSur );


	SAFE_DELETE_ARRAY( m_ptrDstr );

	// ���ͷŵ��ι������еĵ�ͼ�����Դ.
	m_ptrTerrainMgr->release_tgLRMgr();

	// 
	
	

	// �ͷ�skinMesh������.skinMesh����ȳ����ͷ���������ͷ�
	osc_skinMeshMgr::DInstance();


	SAFE_DELETE(m_pLensflare);

	// ɾ���ؿ���Ҫ���ڴ�
	SAFE_DELETE_ARRAY( m_arrPtrTG );

	// ɾ�������ڵ���Ч������
	SAFE_DELETE( m_sEffectMgr );

	// River mod @ 2006-6-21:
	// ɾ�������ڵ���Ч����ɾ�������Ĺ�����
	osc_bbManager::DInstance();

	osc_osaSkyTerr::DInstance();

	

	//syq...delete hdr light
	SAFE_DELETE( m_pHDRLight );

	SAFE_DELETE(m_SceneFlashLightMgr);

	// 2011-2-23: �˳�ʱ�����ͷţ�û������.
	//m_middlePipe.release_middelePipe();

	//��ɾ����ͼ����������ֹ��ͼ�����߳�
	//SAFE_DELETE( m_ptrTerrainMgr );

# if GALAXY_VERSION
	m_vecBlood.clear();
# endif

}

/** \brief
*  �ͷ�һ������.
*  
*  �ͷ�������������е�����,�����ͼ���͵Ȳ�����Ҫ�����������仯��Ҫ
*  ʹ�����������
*  
*  ATTENTION TO FIX: �����Ĵ������������
*/
bool osc_TGManager::release_scene( bool _rlseScrPic/* = true*/ )
{
    guard;

	//! River @ 2009-5-11: �Ƚ����˳����ڵ���������
	for (size_t i = 0; i < mCurrentPlaySoundItemList.size(); i++)
	{
		if( !mCurrentPlaySoundItemList[i].m_bPlayingSound )
			continue;
		m_ptrSoundMgr->StopSound( mCurrentPlaySoundItemList[i].idCloseLoopSound );
		mCurrentPlaySoundItemList[i].reset();
	}
	//! ��յ�ǰ�������ռ�
	mCurrentPlaySoundItemList.clear();

	//! River added @ 2009-6-18: ���ͷŵ�ͼ�ڵ���Դ,����̬�ͷţ�
	//! �п���ɾ�����´�������Դ
	for( int t_i = 0;t_i<m_iActMapNum;t_i ++ )
		m_arrPtrTG[t_i]->release_TG();


	// �ͷ���Դ
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

	//! River @ 2009-8-12����ʱ��ͼ����������״̬
	m_bLoadedMap = false;

	osDebugOut( "\n\n\n\n��ͼ�ͷŹ������...\n\n\n\n" );

	return true;

	unguard;
}



/** \brief
*  ������FrameMove.
*
*  ATTENTION TO OPP: ��취����������ŵ�endscene֮��,����cpu��Gpu֮��Ĳ�����.
*  
*/
bool osc_TGManager::frame_move( os_sceneFM* _fm )
{
	guard;

	osMatrix        t_viewMat,t_projMat,t_worldMat;

	// frameMove��ʼ������յر��Ŀɼ���
	if( m_bLoadedMap )
		MapTile::ClearVisibleTerr();

	// 
	//! River Added @ 2009-6-27:���ڵ�����Ⱦ
	if( !g_bRenderTerr )
		osVec3Normalize( &g_vec3LPos,&g_vec3LPos );


	// 
	// ���õ�ǰ�ԣǣ��ڵĶ�����Ʒ��ĿΪ��.
	m_iAlphaObjNum = 0;

	// ��ǰ֡��Ⱦ����������ĿΪ0.
	m_iRenderTriNum = 0;

	// ���������������֮�����Ʒ��Ŀ����ʼ��Ϊ0.
	m_iCamChrObjNum = 0;

	// 
	m_bHasAlphaShelter = false;

	//
	// ������������������ҪFrameMove.
	if( _fm->m_ptrCam == NULL )
		return true;

	m_iCenterX = _fm->m_iCx;
	m_iCenterY = _fm->m_iCy;
	m_vec3HeroPos = _fm->m_vecChrPos;

	// 
	// �м�ܵ���������úʹ���.���focus��û�����������ʱ��,����Ҫ����.
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

	// ����������仯��
	if( m_bWeatherChanging ){
		frame_moveWeatherChanging( sg_timer::Instance()->get_lastelatime() );
		m_dlightMgr.dxlight_enable(true);
	}

	g_shaderMgr->frame_move();

	// ���frameMove��ɺ��ٴ���skinMgr��FrameMove
	m_skinMeshMgr->frame_setSmMgr();


	// River mod @ 2007-8-26:
	// ���ڼ����ͼ�б���صĴ���ʹ���ٽ�������Ϊ�ϲ�ʹ���˶��߳�
	::EnterCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );
	
	//! �õ���ǰ����ĵ�ͼָ���б�.
	int   t_idx = 0;
	memset( m_arrPtrTG,NULL,sizeof( osc_TileGroundPtr )*MAXTG_INTGMGR );


	// ����Ⱦ�ر�Ҳ��Ҫ��Ⱦ�ر�֮�ϵľ�̬mesh��Ϣ
	m_iActMapNum = m_ptrTerrainMgr->frame_getInViewTg( 
		g_ptrCamera,m_arrPtrTG,MAXTG_INTGMGR );

	// River mod @ 2007-8-26:
	// ���ڼ����ͼ�б���صĴ���ʹ���ٽ�������Ϊ�ϲ�ʹ���˶��߳�
	::LeaveCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );


	//
	// ��ÿһ��tg����֡��ĳ�ʼ����
	if (g_bUpdateTile)
	{
		for( int t_i=0;t_i<m_iActMapNum;t_i++ )
		{
			osassert( m_arrPtrTG[t_i] );
			m_arrPtrTG[t_i]->frame_setTG();		
		}
		
	}

	

	// 
	// ��ȫ��Shader����Ķ�̬��
	os_dlInfo     t_dlInfo;  
	m_dlightMgr.frame_moveDLMgr();
	m_dlightMgr.get_dlINfo( t_dlInfo );
	// River @ 2010-12-31:��ʱ�ص���̬�⣬ȥ���ر�Ӳ������Ⱦ���ֵ�bug.
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


	// ��Ч������
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
	
	

	// ���ӹ������������������Ĵ������Ⱦ�ֿ�.
	m_particleMgr.framemove_particle( g_ptrCamera );
	m_bbMgr->frameMove_bbMgr( (I_camera*)g_ptrCamera );
	m_boltMgr->frame_move( (I_camera*)g_ptrCamera,
		    t_fEtime );

	//@{
	// River @ 2004-5-20:
	// ���붯����Ʒ�Ķ�̬�⡣
	process_aniMeshDlight();


	// River @ 2007-7-7:������������仯״̬���򲻴��������͸��
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


	// NEW ATTENTION OPP:river @ 2007-3-20: ������ĳЩ֡�ںܷ�ʱ��.
	frame_moveSound(&camPos,sg_timer::Instance()->get_lastelatime());


	//! River @ 2009-6-13:
	//! ����smMgr��frameMove,�˴����ã������ϲ��ܹ�����׼ȷ�ĵõ�sm������λ��
	m_skinMeshMgr->framemove_smMgr();


	return true;

	unguard;
}



//! ����ȫ�ֵ�����Ⱦ״̬.
void osc_TGManager::fog_rsSet( void )
{
	guard;

	m_middlePipe.set_renderState( D3DRS_FOGENABLE,g_bUseFog );

	// ���Ի��ڶ����rangeBase fog.
	if( g_bUseFog )
	{
		// 
		//
		// Ŀǰ������ֻ֧��������
		m_middlePipe.set_renderState( D3DRS_FOGVERTEXMODE,  D3DFOG_LINEAR );
		m_middlePipe.set_renderState( D3DRS_FOGTABLEMODE,D3DFOG_NONE );
		m_middlePipe.set_renderState( D3DRS_RANGEFOGENABLE, TRUE );
		m_middlePipe.set_renderState( D3DRS_FOGSTART, *(DWORD *)(&g_fFogStart) );
		m_middlePipe.set_renderState( D3DRS_FOGEND,   *(DWORD *)(&g_fFogEnd) );

		// ����ȫ�ֵ���.
		m_middlePipe.set_renderState( D3DRS_FOGCOLOR, g_dwFogColor );
	}

	unguard;
}

//! �������ڵ�ÿһ�������İ�Χ����
void osc_TGManager::draw_soundBox( void )
{
	guard;

	//! test ���Գ��������ã���ʾÿ�������ĺ���
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


//! �߲�ͼ�����Ⱦ������ս��ʱ����Ч�����decal��
void osc_TGManager::render_topLayer( I_camera* _cam,os_FightSceneRType _rtype )
{
	guard;


	m_middlePipe.set_viewMatrix( g_matView );
	m_middlePipe.set_projMatrix( g_matProj );

	// ������Ч��صĶ�����Ⱦ
	// ��ȾkeyAniMesh: ATTENTION:
	// �������ݾ��������Զ������Ⱦ���ǵ�aniMesh
	m_dlightMgr.dxlight_enable( false );
	m_middlePipe.frame_renderKeyAniMesh( g_ptrCamera->get_frustum(),false,OS_RENDER_TOPLAYER );
	m_middlePipe.frame_renderKeyAniMesh( g_ptrCamera->get_frustum(),true,OS_RENDER_TOPLAYER );


	m_dlightMgr.dxlight_enable( false );
	m_middlePipe.set_renderState( D3DRS_ZWRITEENABLE,false );

	//
	// partilceManager����Ⱦ������͸����Ʒ���ص����������:
	m_particleMgr.render_particle( NULL,OS_RENDER_TOPLAYER );

	return;


	// ����Ƿֲ���Ⱦ����˴���ʼ��Ⱦtop layer.
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
	// ������Ч��صĶ�����Ⱦ
	// ��ȾkeyAniMesh: ATTENTION:
	// �������ݾ��������Զ������Ⱦ���ǵ�aniMesh
	m_dlightMgr.dxlight_enable( true );
	m_middlePipe.frame_renderKeyAniMesh( g_ptrCamera->get_frustum(),false,OS_RENDER_TOPLAYER );
	m_middlePipe.frame_renderKeyAniMesh( g_ptrCamera->get_frustum(),true,OS_RENDER_TOPLAYER );
	*/

	m_dlightMgr.dxlight_enable( false );
	m_middlePipe.set_renderState( D3DRS_ZWRITEENABLE,false );

	////
	//// ��Ⱦ�����е�billBoard��Ƭ��
	if( !m_bbMgr->render_bbManager( g_ptrCamera,OS_RENDER_TOPLAYER  ) )
		osassertex( false,"��ȾbbManagerʧ��...\n" );

	//
	// partilceManager����Ⱦ������͸����Ʒ���ص����������:
	m_particleMgr.render_particle( g_ptrCamera,OS_RENDER_TOPLAYER );

	return;

	unguard;
}



/** \brief
*  ��Ⱦ��ǰ�ĵ�ͼ��
*
*
*  IMPORTANT FUNC: ������������Ⱦ��Ҫ�����������ʵ�֡�
*  
* 
*  \param _scrPic �����ǰ����Ϊ�棬��ֻ��Ⱦ��Ļ�ڵĶ�άͼƬ��������Ⱦ����
*                 �Ӷ�άͼƬ��
*  
*  ��Ⱦ˳��
*  1: ����Ⱦ�ر���Ⱦ�ر��ʱ����Ⱦ�ر��ϵĲ�͸�����塣
*  2: ��Ⱦ�����е�����,����Ⱦ����Ĳ�͸�����֣�����Ⱦ�������͸�����֡�
*  3: ��Ⱦ�����������͸�����֡�
*  4: ��Ⱦ�����е�billBoard���塣
*  4: ��Ⱦ�����е��������塣
*
*  River mod @ 2008-11-27:������Ⱦ�غ���״̬�µĳ���,��Ҫ������
*  1: ����ĳ��������������Ⱦ����������֮��
*  2�����÷ǳ����ڵ���Ч��Ⱦ����������֮��
*  3: ������Ⱦ��ɺ��ڳ�������Ⱦһ����Ƭ���ڸ�����������ϡ�
*  4: ��ʱ��Ⱦ����ս�����������Ч��
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
	// ��������н���Ҫ��ʾ��άͼƬ��
	if( _scrPic )
	{
		render_ScrPic(_LensPic);
		// �����Ⱦ��Ļ�ϵĹ�ꡣ	
		t_ptrMgr->render_cursor();
		return true;
	}

	if( m_pHDRLight )
		m_pHDRLight->set_hdrRenderTarget();
	
	// River @ 2010-3-13:Ϊ��ȫ��growЧ��,�˴�����
	if( m_growEff.IsEffective() )
		m_growEff.reset_renderTarget( g_frender->get_d3dDevice() );

	// ����Ⱦ��պ���.
	if (g_bRendSky)
		render_skyDome();

	//@{
	// Windy mod @ 2005-9-14 17:24:17
	//Desc: lens flare
#ifdef USE_LENSFLARE
	m_pLensflare->render(g_ptrCamera,1);
#endif
	//@}
	
	// ����ȫ������ص�����
	fog_rsSet();


	//!��FRAMEMOVE����Ⱦ
	if( g_bRenderTerr && m_bLoadedMap  )
	{
		// ��Ⱦ��������ͨ��TileSet.
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
	// ��ʼ��Ⱦ������������ǹ�����Ӱ��ص�����
    /*	
	// ԭ�����Ⱦ��λ�á������䵽���µ�Ч����
	*/

	//
	// ��Ⱦ�����еĲ�͸����Ʒ��
	//
	if( g_bDisplayStaticMesh )
		if( !render_opacityObjPart() )
			return false;

	// 
	// ������Decal��ص���Ⱦ,Ϊ��������Ʒ����ȷ����ʾDecal.
	// ���ڴ˴����ǻ�������⣺������������ˮ�����Σ���Ϊ�߶ȵĲ�ֵ���⣬����ֺڵķ��顣
	// ������ŵ�͸����keyAni������Ⱦ����͸����keyani��д��Zbuffer,�Ӷ�Ӱ��decal����ʾ��
	// ��������ǣ��ѹ���Decal�ŵ�������ʾ������
	m_sDecalMgr.render_decalmgr( _rtype );


	osc_skinMesh::clear_postTrail();
	// 
	// River @ 2005-8-16:�Ѳ�͸����keyAni�ŵ�����֮ǰ��Ⱦ��
	// ��ȾkeyAniMesh.
	m_middlePipe.frame_renderKeyAniMesh( g_ptrCamera->get_frustum(),false,OS_RENDER_LAYER );

	if( g_eSkinMethod == OSE_D3DINDEXED )
		m_dlightMgr.dxlight_enable( true );

	//@{
	//! River @ 2010-1-29: ��Ⱦǽ�������.
	m_middlePipe.set_nullStream();
	m_skinMeshMgr->renderBehindWallChar();
	m_middlePipe.set_nullStream();
	//@}


	
	//@{ ���ﲿ�ֵ���Ⱦ.	
	// ��Ⱦ�����е�����Ĳ�͸�����֡�
	if( !m_skinMeshMgr->render_smMgr( _rtype ) )
		osassert( false );
	m_middlePipe.set_nullStream();
	//
	// River @ 2006-7-1:�������͸��������Ⱦ�ŵ�keyAni��Ⱦ�ĺ��棬֮ǰ
	// ��ģ�����Ⱦ����������Ĳ�͸�����ֵ���Ⱦ
	// ��Ⱦ�����������͸������
	m_skinMeshMgr->render_smMgr( _rtype,true );
	/**/
	
	m_middlePipe.set_nullStream();
	osc_skinMesh::end_renderPostTrail(true);


	// River @ 2009-5-14:��ʾ��Ⱦˮ�棬������Ч����ȷ��Ⱦ
	m_dlightMgr.dxlight_enable( true );
	m_middlePipe.set_renderState( D3DRS_ZWRITEENABLE,true );

	// River @ 2009-7-30: ��Ʒ��͸�����֣�Ӧ����ˮ��֮ǰ��Ⱦ�����������䡣
	// ��Ⱦ��Ʒ͸����صĲ���.
	if( g_bDisplayStaticMesh )
	{
		if( !render_transparentObjPart() )
			return false;
	}		
	//@}

	//windy add
	UpDate_reflectionMap();

	//! River @ 2009-6-29: ����refractionMap.
	update_refractionMap();
	m_middlePipe.render_aniWater( g_ptrCamera->get_frustum() );

	//! River @ 2009-6-28:��������ص�����
	fog_rsSet();


	// 
	// River @ 2009-7-7:��͸����Ʒ���ֺ�����͸�����ַŵ�ˮ��֮����Ⱦ


	// ��ȾkeyAniMesh: ATTENTION:
	// �������ݾ��������Զ������Ⱦ���ǵ�aniMesh
	m_middlePipe.set_renderState( D3DRS_FOGENABLE,true );
	m_dlightMgr.dxlight_enable( true );
	g_bRenderScreenWave = FALSE;
	m_middlePipe.frame_renderKeyAniMesh( g_ptrCamera->get_frustum(),true,OS_RENDER_LAYER );
	
	// River @ 2010-6-23:�ָ�״̬.
	m_middlePipe.set_renderState( D3DRS_FOGENABLE,true );
	m_dlightMgr.dxlight_enable( true );


	// �����ڵ�SceneCursor,���������Ĺ��
	m_sDecalMgr.render_sceneCursor();


	// River @ 2009-7-25:����ᵽ�˴���
	// Ϊ���Ǹ�ˮ����ݣ�����ˮ�������Ӱ�����
	// ��ε����䵽���µ�Ч����
	if( !m_bbMgr->render_raindropBB( g_ptrCamera ) )
		return false;

	////
	//// ��Ⱦ�����е�billBoard��Ƭ��
	if( !m_bbMgr->render_bbManager( g_ptrCamera,OS_RENDER_LAYER  ) )
		return false;


	//! ��Ⱦ�����е�boltMgr.
	m_boltMgr->render();

	//
	// partilceManager����Ⱦ������͸����Ʒ���ص����������:
	m_particleMgr.render_particle( g_ptrCamera,OS_RENDER_LAYER );

	//
	// ��Ⱦ�����������֮���͸����Ʒ���������������Ļͼ�����Ⱦ˳��,�ص���͵ƹ�
	// River @ 2009-6-28: ����򿪵ƹ���ܰ�͸�����ص����Ч��Ҳ��̫�ã��������.
	m_dlightMgr.dxlight_enable( true );
	if( g_bAlphaShelter )
	{
		render_shelterObj( false );
		render_shelterObj( true );
	}
	m_dlightMgr.dxlight_enable( false );


	//
	// ����Ч������Ⱦ��
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
	//! ��Ⱦ���������Ϣ
	if (g_bUseVolumeFog&&g_bVolumeFogInited)
	{
		UpDate_DepthMapAndRenderFog();
	}
	///@}


	// ��������ʹ���Եõ�
	// ��Ⱦ�����еĶ�ά��Ļ��ʾͼƬ��
	t_ptrMgr->draw_fullscrFade();	
	render_ScrPic();

	//
	// ͳ�Ƴ����пɼ��Ķ������Ŀ��
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

	// River: ��Ļ�ⲨЧ��
	m_growEff.RenderScreenWave( m_deviceMgr->get_d3ddevice() );

	m_growEff.RenderEffScene(m_deviceMgr->get_d3ddevice());
	//@}




	//@}

	return true;

	unguard;
}


// 
//! �������ֱ����޸ģ���Ȼ�˴����ڴ�ķ�����ͷŲ�������ĳЩ�����������������������!!!!!
//! ������Ч���º���.
void osc_TGManager::frame_moveSound(osVec3D* _camPos,float _timeMs)
{
	guard;
	
	static os_aabbox camBox;
	static osVec3D camEdge(1.0f,1.0f,1.0f);
	camBox.m_vecMax = *_camPos + camEdge;
	camBox.m_vecMin = *_camPos - camEdge;
	
	mSoundQuadTree.Calculate(camBox);
	std::vector<OSSoundItem*> pSoundPtrItemList = mSoundQuadTree.GetNodePtrList();
	
	//// ά���Ѿ��е�����
	for (size_t i = 0; i < mCurrentPlaySoundItemList.size(); i++)
	{
		if( !mCurrentPlaySoundItemList[i].m_bPlayingSound )
			continue;

		osVec3D length3D = mCurrentPlaySoundItemList[i].pItem->m_pos- *_camPos;
		float length = osVec3Length(&length3D);

		std::vector<OSSoundItem*>::iterator it;

		// River @ 2008-3-21:
		// ������еĲ��������б����һ�����������Ѿ����������Ĳ����ɲ��ŵ������б��ڣ�
		// ��ֹͣ�������Ĳ���
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

				//! river @ 2009-5-11: ѭ������������
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
	// �����µ�����
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
			
			// River added @ 2009-5-11:ѭ������������
			bool   t_bLoop = false;
			if( item.pItem->m_playType == 10 )
				t_bLoop = true;

			item.idCloseLoopSound = m_ptrSoundMgr->PlayFromFile(
				item.pItem->m_SoundName,&soundSrc,t_bLoop,false,&t_fSoundTime );

			// river mod @ 2008-3-21:����༭ָ���Ĳ���ʱ�䳤��Ϊ�㣬������ͨ�Ĳ���ʱ������
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
*  �ж�һ�������Ƿ��һ��quad�ı����ཻ,�����ݴ���Ĳ���������õ������λ��
*  ����Ľ��㡣
*
*  \param const osVec3D* _quadVer ������ı��ζ��㣬Ϊ���������Σ������������ɶ�������
*                                 (0,2,1),(1,2,3) ���� 
*  \param osVec3D& _intersectPt ����ཻ�����ұȴ���ľ���������������ء�
*  \param osVec3D& _camPos      �����λ�á�
*  \param float& _dis           �˴��ཻ����ǰ�õ���������������ľ��롣
*/
bool osc_TGManager::pick_nearestTri( const osVec3D* _quadVer,const os_Ray &_ray,
									osVec3D& _pickVec,osVec3D& _camPos,float& _dis )
{
	guard;

	osVec3D     t_interPT;
	bool        t_hr = false;

	// ���ǿ���Ƭ������ཻ���
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
	//!1 2 3 ����Ƭ
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
*  ��rayStart��ʼ������Chunk���ڵ�������Ʒ������ཻ���򷵻�true.
*
*  \param int _cnum �����chunk��Ŀ
*  \param const os_tileIdx* �����chunk�б�
*  
*  �㷨������
*  �õ�ÿһ��TileGround��QuadTree��aabbox,Ȼ�����ǰ��Ray����ײ�����
*  ����ײ������������TileGround�ڵ���Ʒ�б��У��ҳ�������Ʒ��Ȼ��
*  ����������Ʒ�Ƿ��Ray��ײ�������ײ�������������������ߵ�λ�á�
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
		// ��ÿһ��Chunk�õ���Chunk����Ʒ�б�����б������������Ʒ��
		// �����ߴ���һ�£��Ƿ��������Ʒ��boundingbox�ཻ������ཻ��
		// ���һ������׼ȷ�Ľ���λ��
		if( !t_TilePtr->m_sQuadTree.ray_intersect( _rayS,_rayDir ) )
			continue;

		// �õ���tileGround�ڵ���Ʒ�б������������Ʒ����AABBox��ײ���
		for( int t_k = 0;t_k<t_TilePtr->m_iTGObjectNum;t_k ++ )
		{
			// �ǹ̶���Ʒ
			if( 0 != t_TilePtr->m_vecTGObject[t_k].m_dwType )
				continue;
			// ��������Ʒ
			if( !g_ptrMeshMgr->is_bridgeObj( t_TilePtr->m_vecObjectDevIdx[t_k] ) )
				continue;

			// ��Ʒ��BBox. // River @ 2010-3-30:ʹ��һ����ʱ����
			float t_fBoxDis;
			if( !t_TilePtr->m_vecObjAabbox[t_k].
				ray_intersect( _rayS,_rayDir,t_fBoxDis ) )//t_fTmpDis ) )
				continue;

			// ��ȷ���ཻ���ԣ�
			if( !t_TilePtr->is_intersectObj( 
				t_k,_rayS,10000.0f,_rayDir,&t_fTmpDis ) )
				continue;

			if( t_fTmpDis < t_fMinDis )
				t_fMinDis = t_fTmpDis;
		}	
	}

	//! ���������Ʒ����ײ����
	if( t_fMinDis < 10000.0f )
	{
		_resLength = t_fMinDis;
		return true;
	}
	else 
		return false;

	unguard;
}



//! ��һ����ʼ���һ��dir,������ر����������Ʒ��صĳ�����Ϣ��
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

	// ʹ�����λ�ã�������ҷ����㣬pickray���߷����һ��λ�ã�
	// ����һ���������߷����ƽ�棬���һ���ر���ӵ��ĸ�����ȫ��ƽ���һ�⣬��
	// ������ӿ϶���������������ཻ����Ϊ�ر������͹���塣
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

		// ���Ĳ���Ϊ��׼�Ĳ���:
		const os_tileIdx*   t_ptrChunkIdx;
		const osVec2D*      t_ptrChunkXZ;
		int                t_iPickChunkNum = 0;

		// �ӵ�ǰTileGround��QuadTree�õ��ཻ��Chunk�б�
		t_iPickChunkNum = TilePtr->m_sQuadTree.get_pickChunkList( 
			       _rayS,t_vec3RayDir,t_ptrChunkIdx,t_ptrChunkXZ );
		if( 0 == t_iPickChunkNum )
			continue;


		// ��ÿһ��ray�ཻ��Chunk���и�ϸ�µĴ���
		for( int t_i=0;t_i<t_iPickChunkNum;t_i ++ )
		{
			osVec3D    t_interPT;
			osVec3D    t_poly[4];

			// 
			// ��ʼ���ҿ����ཻ��tile����	�б�
			int t_tileNum = m_sChunkPick.get_pickTile(
				_rayS,t_vec3RayDir,t_ptrChunkXZ[t_i].x,t_ptrChunkXZ[t_i].y );

			// ��ÿһ��tile���Ӳ鿴�Ƿ��ཻ
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
				// ��������ļ��,����͹���ԭ��,�˴����Թ��˵������ı���
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

				// �������μ�����ཻ����
				if( pick_nearestTri( t_poly,pickRay,
					_pickVec,_rayS,t_fdistCamPosToSelect ) )
					t_hr = true;
			}
		}
	}

	_bridgeInter = false;
	// 
	// River mod @ 2008-10-16:�����������Ʒ�Ľ���,���ڵ��������Ʒʱ����ȷ���ߵ�����
	// River added @ 2009-5-4:���������Ʒ����bridge�ϵ��ʱ�Ĵ���
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



/** �µ�����ѡ��ʽ 
@remarks
	�����㷨����:
	�ȵõ������ཻ�ĵر���ӣ�Ȼ�����Щ����������ϸ����ײ����
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


	// �õ�����Ļ�ռ䵽����ռ�ķ���Vector.
	 g_ptrCamera->get_upprojvec( _sidx.m_iX,_sidx.m_iY,t_vecRayDir );

	osVec3Normalize( &t_vecRayDir,&t_vecRayDir );

	osVec3D   t_vec3BackRayDir = t_vecRayDir;

	os_Ray pickRay(t_camPos,t_vecRayDir );

	// ʹ�����λ�ã�������ҷ����㣬pickray���߷����һ��λ�ã�
	// ����һ���������߷����ƽ�棬���һ���ر���ӵ��ĸ�����ȫ��ƽ���һ�⣬��
	// ������ӿ϶���������������ཻ����Ϊ�ر������͹���塣
	t_vec3RayEnd = t_camPos + t_vecRayDir;
	osPlaneFromPoints( &t_pickPlane,&t_camPos,&t_camRightVec,&t_vec3RayEnd );


	float   t_fBridgeColl = 10001.0f;

	for (int i =0 ; i < m_iActMapNum;i++)
	{
		os_aabbox box;
		osc_TileGroundPtr TilePtr = NULL;
		TilePtr = m_arrPtrTG[i];

		// ���Ĳ���Ϊ��׼�Ĳ���:
		const os_tileIdx*   t_ptrChunkIdx;
		const osVec2D*      t_ptrChunkXZ;
		int                t_iPickChunkNum = 0;

		// �ӵ�ǰTileGround��QuadTree�õ��ཻ��Chunk�б�
		t_iPickChunkNum = TilePtr->m_sQuadTree.get_pickChunkList( 
			       t_camPos,t_vecRayDir,t_ptrChunkIdx,t_ptrChunkXZ );
		if( 0 == t_iPickChunkNum )
			continue;


		// ��ÿһ��ray�ཻ��Chunk���и�ϸ�µĴ���
		for( int t_i=0;t_i<t_iPickChunkNum;t_i ++ )
		{
			osVec3D    t_interPT;
			osVec3D    t_poly[4];

			// 
			// ��ʼ���ҿ����ཻ��tile����	�б�
			int t_tileNum = m_sChunkPick.get_pickTile(
				t_camPos,t_vecRayDir,t_ptrChunkXZ[t_i].x,t_ptrChunkXZ[t_i].y );

			// ��ÿһ��tile���Ӳ鿴�Ƿ��ཻ
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
				// ��������ļ��,����͹���ԭ��,�˴����Թ��˵������ı���
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

				// �������μ�����ཻ����
				if( pick_nearestTri( t_poly,pickRay,
					_pickVec,t_camPos,t_fdistCamPosToSelect ) )
					t_hr = true;
			}
		}
	}

	// 
	// River mod @ 2008-10-16:�����������Ʒ�Ľ���,���ڵ��������Ʒʱ����ȷ���ߵ�����
	// River added @ 2009-5-4:���������Ʒ����bridge�ϵ��ʱ�Ĵ���
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
*  �õ���ǰ��������ڸ��ӵ�������
*
*  �������ֵ�������Ŵ��ͼ�ϵĸ��ӵ�����ֵ�������ɷ�����ȷ�ϲ���Ч��
*
*  \param _dix   �����ڲ�������ɺ�������ͻ������ֵ.
*  \param _sidx  ���뵽�����ڲ����ڴ������Ļ����. 
*  
*  River @ 2004-6-25: ������Ӹ߶ȵĿ��ǣ�ʹ�ø��Ӿ�ȷ������ȡ���ܡ�
*  �㷨������
*  �õ�������ƽ�潻������ߡ�
*  �������λ��ͶӰ����ά��ƽ��͵�ƽ��Ľ�������һ�����ߡ�
*  ��������ڵĸ��ӳ������õ����������뵱ǰ���ӵ���һ�����ཻ��
*  �����ཻ�ıߺͽ��㣬�õ���ǰ����ͨ������һ�����ӣ���鵱ǰ��
*  ������������ά���ܷ����������ཻ��
* 
*  ��������ཻ���ӽ���������õ��µ����ߣ��ظ���һ���Ĳ�����
*  ����������յ��Ժ󣬻�û���ҵ��ཻ�ĸ��ӣ���ʵ���ཻ���ӵ��ڵ�ƽ�ߣ�
*  ��������Ȳ����㣬ʹ��������ߺ͵�ƽ���ཻ�õ��ĸ���*  ��Ϊ���յ��ཻ���ӡ�
*  
*  River add@ 2004-9-16: 
*  ������ߵķ������ˮƽ�棬�򷵻�false.�����ϵķ���ȥѡȡ����ʱ�����λ��Ϊ��ա�
*  windy mod@ 2006-2-8
*  �µ�����ѡ��ԭ�����㷨��ʱ����
*  
*/
bool osc_TGManager::get_pickTileIdx( os_pickTileIdx& _dix,
									os_tileIdx& _sidx,osVec3D* _intersectPos/* = NULL*/ )
{
	guard;
	osVec3D InterPT;
	osVec3D t_vec;
	int             t_x,t_z;

	//! River @ 2009-8-12: �����ͼû�е��룬ֱ�ӷ���
	if( !m_bLoadedMap )
		return false;

	if (get_newpickTileIdx(InterPT,_sidx)){

		t_x = int(InterPT.x/TILE_WIDTH);
		t_z = int(InterPT.z/TILE_WIDTH);

		// ���x,z�������ڵ�ͼ��������������ش�
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
	//!����㵽���ϵ�����
	float			t_Distance = 30.0f;

	osVec3D         t_vecRayDir,t_vec3RayEnd,t_camPos,t_camRightVec;
	osPlane         t_pickPlane;

	bool            t_hr = false;
	float	        t_fdistCamPosToSelect = 10000000.0f;

	if( !g_ptrCamera )
		return false;

	// �õ�����Ļ�ռ䵽����ռ�ķ���Vector.
	g_ptrCamera->get_upprojvec( _sidx.m_iX,_sidx.m_iY,t_vecRayDir );
	
	osVec3Normalize(&t_vecRayDir,&t_vecRayDir);
	osVec3D destPos = m_vec3HeroPos + t_vecRayDir*t_Distance;

	t_x = int(destPos.x/TILE_WIDTH);
	t_z = int(destPos.z/TILE_WIDTH);

	// ���x,z�������ڵ�ͼ��������������ش�
	if( (t_x<0)||(t_x>=m_iMaxX)||
		(t_z<0)||(t_z>=m_iMaxZ)  )
	{
		//osassertex(false,"x,z�������ڵ�ͼ���������");
		//@{
		// River mod @ 2007-4-17:�����ȶ��Ĵ��������
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
*  �ͻ��˵���ײ��⺯��.
*  
*  ����ͻ�����ײ��ⲻ��ͨ��������Ҫ���ɷ���������ȷ�ϣ�
*  �����ͨ�������ɷ�����ȷ�Ϻ����һϵ�еĶ����Ͳ����� 
*  
*/
bool osc_TGManager::collision_detection( const os_tileIdx& _tile )
{
    guard;


# if __QTANG_VERSION__

	// River @ 2010-9-13:���뱣֤���ƣ�ȷ���ײ㲻������⡣
	if( (_tile.m_iX > 256)||(_tile.m_iY > 256) )
		return false;

	//�� ���㷨���ϲ����ײ��Ϣ
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
	// �õ�����ʹ�õĵ�ͼ��Ȼ��ʹ������ڴ��ͼ����ײ��Ϣ��
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
	// �������˴�������Ӳ��ڵ�ǰ�ɼ��ĳ����ڲ���
	// ���ز���ͨ������Ϣ��
	return false;

	unguard;
}




/** \brief
*  ��ǰ����ĵ�ͼ�Ƿ��Ѿ��������ڴ档
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
*  �ɴ��������ռ����㵱ǰ����ռ������һ����ͼ�ϣ��ڵ�ͼ����һ��ͼ����.
*  
*  \param  _res     ʹ��os_pickTileIdx�ṹ�������ǵõ���ֵ.
*  \param  _pos   ����������ռ�Ķ���λ�á�
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
	// ���������ǰ���ͼ�ĸ߶Ȼ��ǿ�ȣ����ش���,�������з���ֵ��
	if( (_res.m_tgIdx.m_iX > (m_iMaxXTgNum)) ||
		(_res.m_tgIdx.m_iY > (m_iMaxZTgNum) ) )
		return false;

	//
	// �����ǰ�ĵ�ͼ��û�б����룬�߶�ֵ��Ϊ0.
	if( !is_tgLoaded( _res.m_tgIdx.m_iX,_res.m_tgIdx.m_iY ) )
	{
		_res.m_fTileHeight = 0.0f;
		return false;
	}


	//
	// �õ���ǰ���ӵĸ߶�ֵ��
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
*  ����һ����ͼ���꣬�õ���ǰ��ͼ�������ĵ��ڶ�Ӧ����ά�������ǰ���겻�ڳ����ڣ�
*  ���Ӧ����ά�����yֵΪ0.0f.
*/
bool osc_TGManager::get_tilePos( int _x,int _y,osVec3D& _pos,BOOL _precision/* = FALSE*/  )
{
	guard;


	// TEST CODE:
	//osassertex( !( (_x<0) || (_x>=(m_iMaxX*2) ) || (_y<0) || (_y>=(m_iMaxZ*2) ) ),
	//	va( "The X,Y cord is:<%d,%d>,MaxX,MaxZ is:<%d,%d>..\n",_x,_y,m_iMaxX,m_iMaxZ ) );
	// River @ 2011-4-10:�ڼ�������£����ÿͻ��˹ҵ������ܼ�����ȥ��˵��
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

	// �����ֵΪ��ײ����ֵ��
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
	// �����ǰ�ĵ�ͼ��û�б����룬�߶�ֵ��Ϊ0.
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
//! �õ�ÿһ����ײ����ϸ�ֳ�15�ݵĸ�����ά�ռ�λ��.
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
	//Desc: ��һ�������ڵ������ֵ����ֵ��
	int t_i64X,t_i64Y;
	
	float t_u,t_v;
	t_u = (float)(_x%DETAIL_TILENUM2);
	t_v = (float)(_y%DETAIL_TILENUM2);
	t_u /= DETAIL_TILENUM*2.0f; 
	t_v /= DETAIL_TILENUM*2.0f; 

	//@}

	// �ȵõ���ά�ռ�λ�õ�Y�߶�.
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
	// �õ�x,z����ά����.ϸ�ֵ������൱����0.1��Ϊ��λ.
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
*  ���������ڵ�һ��xzֵ���õ��˴��ĵر�߶�
*
*  \param osVec3D& _pos x,z��ֵ��_pos�ڵ�x,zֵ����.���صĸ߶�ֵ��_pos.y��
*
*  River @ 2007-4-3:���ĳһ�����ӵ���ײ��Ϣ����ͨ��,��������������ĸ��ӵĸ�
*                   ����Ϣ��Ҫ��������Ĵ���.
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


	// ��������Ŵ��ͼ���ӵ�����
	t_fU = _pos.x / TILE_WIDTH;
	t_fV = _pos.z / TILE_WIDTH;
	t_iColX = int(t_fU);
	t_iColZ = int(t_fV);

	// �õ���ײ��Ϣ
	t_tgIdx.m_iX = WORD(t_iColX / MAX_GROUNDWIDTH);
	t_tgIdx.m_iY = WORD(t_iColZ / MAX_GROUNDWIDTH);

	// River mod @ 2007-8-26:
	// ���ڼ����ͼ�б���صĴ���ʹ���ٽ�������Ϊ�ϲ�ʹ���˶��߳�
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

	// �㴫��������һ����ʾ�����ڵ�uv.
	t_fU = t_fU - t_iColX;
	t_fV = t_fV - t_iColZ;

	// ������ʾ���ӵ�ԭ������x,zֵ
	t_fOrigX = t_iColX*TILE_WIDTH;
	t_fOrigZ = t_iColZ*TILE_WIDTH;

	t_iColX -= (t_tgIdx.m_iX*MAX_GROUNDWIDTH);
	t_iColZ -= (t_tgIdx.m_iY*MAX_GROUNDWIDTH);

	// ������ײ���ӵ���Ϣ
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
		osassertex( !_isnan( _pos.y ),"ƽ��߶ȸ�������ɵ�����" );
	}
	else
	{
		osPlaneFromPoints(&t_plane,&v01,&v10,&v11);
		_pos.y = (-t_plane.d-_pos.x*t_plane.a-_pos.z*t_plane.c)/t_plane.b;
		osassertex( !_isnan( _pos.y ),"ƽ��1�߶ȸ�������ɵ�����" );
	}

	// 
	// River mod @ 2007-4-4:�������ȷ�ĸ߶ȼ����Ϣ:
	if( _precision )
	{

		float t_fHeight = get_objHeightInfo( _pos );

		// River mod @ 2007-6-8:����ر�߶ȵĴ���
		// River mod @ 2007-6-9:����ر��ϴ�ʱû��������Ʒ����ʹ��ԭ������ײ�߶���Ϣ
		if( t_fHeight > -10000.0f )
		{
			get_earthHeight( _pos );
			osassertex( !_isnan( _pos.y ),"��ͼ���ϵĸ߶Ȳ��Ǹ�������ɵ�����" );
		}

		if( t_fHeight > _pos.y )
		{
			_pos.y = t_fHeight;
			osassertex( !_isnan( _pos.y ),"�߶����ϵĸ߶Ȳ��Ǹ�������ɵ�����" );
		}
	}
    

	osassertex( !_isnan( _pos.y ),"�п����ǵ�ͼ���ϵĸ߶Ȳ��Ǹ�������ɵ�����" );

	// River added @ 2007-8-16:
	::LeaveCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );	

	//! River @ 2010-3-29:����Щ���ƫ�ƣ������Ч�ڵر��ţ������ȥ��ZFighting����
	_pos.y += 0.03f;

	return true;

	unguard;
}
//! �������������λ��,�õ���λ�õĲ�������
/*! \note ���������ֻ�õ���X,Z����*/
int	osc_TGManager::get_materialIdx(osVec2D& _pos,os_terrainTexSnd& _texsnd )
{
	guard;

	osassert( _pos.x >= 0.0f );
	osassert( _pos.y >= 0.0f );

	os_tileIdx  t_tgIdx;
	os_tileIdx  t_Collidx;
	float t_fU,t_fV;

	// ��������Ŵ��ͼ���ӵ�����
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


	// ���µ�ǰ��tileManager������TG���豸�������id��
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

		//! river add @ 2009-2-19:�����жϣ��ϲ㰲ȫʹ��
		if( t_idx >= (int)m_vecTerrainTexSnd.size() )
			return -1;

		memcpy( &_texsnd,&m_vecTerrainTexSnd[t_idx],sizeof( os_terrainTexSnd ) );

		return t_idx;
	}

	return 0;

	unguard;
}


/** \biref
*  �õ���ǰ�����ڵĵ�ͼ����Ŀ������.
* 
*  �ͻ�����Ѱ·�㷨ʱ���õ���Щ����.
*  
*  \return int     �����������ڵĵ�ͼ����Ŀ.
*  \param  _tgidx  ����ķ��ص�ͼ������ָ��ṹ,��෵��4��._tgidxӦ����os_tileIdx[4].
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

		// ATTENTION TO FIX: ����������Ĵ���,
		// ��Ϊ�Ժ󿴵ľ������,�˴��Ĵ����ĸ�Ϊ����.
		if( t_iTGNum >= 4 )
			break;
	}

	return t_iTGNum;

	unguard;
}





/** \brief
*  �õ�������ĳһ����ͼ����ײ��Ϣ���ݿ顣
* 
*  \param _tgIdx     �����е�ͼ����������(0,0)��ʼ��
*  \return os_TileCollisionInfo* ���ص���os_TileCollisionInfo�Ķ�ά���顣
*                                 ����os_TileCollisionInfo��128][128] 
*/
const os_TileCollisionInfo*  osc_TGManager::get_sceneColData( os_tileIdx& _tgIdx )
{
	guard;

	int              t_i;
	osc_TileGroundPtr   t_ptr; 

	
	osassertex(_tgIdx.m_iX>=0,va("��ͼ������X:%d",_tgIdx.m_iX));
	osassertex(_tgIdx.m_iX<m_iMaxXTgNum,va("��ͼ������X:%d,MaxTGNum:%d",_tgIdx.m_iX,m_iMaxXTgNum));

	osassertex(_tgIdx.m_iY>=0 ,va("��ͼ������Y:%d", _tgIdx.m_iY));
	osassertex(_tgIdx.m_iY<m_iMaxZTgNum ,va("��ͼ������Y:%d,MaxTGNum:%d", _tgIdx.m_iY,m_iMaxXTgNum));


	


	//
	// ���µ�ǰ��tileManager������TG���豸�������id��
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

	osassertex( false,"�Ҳ���Ҫ������ײ��Ϣ����" );

	return NULL;


	unguard;
}
const float* osc_TGManager::get_sceneColHeightData(os_tileIdx& _tgIdx )
{
	guard;

	int              t_i;
	osc_TileGroundPtr   t_ptr; 

	// River mod @ 2007-3-7:������͵Ĵ�����ʽ
	if( (_tgIdx.m_iX<0)||(_tgIdx.m_iX>=m_iMaxXTgNum)||
		(_tgIdx.m_iY<0)||(_tgIdx.m_iY>=m_iMaxZTgNum) )
	{
		return NULL;
	}

	//
	// ���µ�ǰ��tileManager������TG���豸�������id��
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
///����Lens Flare�Ľӿ�
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

///���ļ��м���lens flare
void osc_TGManager::Load_LensFlare(const char * _fname)
{
	guard;
	osassert(m_pLensflare);
	if (!file_exist((char *)_fname)){
		Outstr_tolog("Lens Flare: %s �ļ�������" ,_fname);
		osassertex( false, "Lens Flare �ļ�������" );
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

///�����Ǳ༭���Ľӿ�
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