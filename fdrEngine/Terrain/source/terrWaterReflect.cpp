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
# include "../../mfpipe/include/osShader.h"
# include "../../mfpipe/include/ostexture.h"
# include "../../effect/include/VolumeFog.h"
# include "../../mfpipe/include/osEffect.h"
# include "../../backpipe/include/mainEngine.h"
#include "../../interface/stringTable.h"
#include "../../CardProfile/gfxCardProfile.h"
osMatrix  g_sMatReflectView;
osMatrix  g_sMatReflectProj;
extern osc_camera*         g_ptrCamera;
// 测试Reflection的功能
void osc_TGManager::UpDate_reflectionMap()
{
	guard;

	HRESULT   t_hr;

	// 不画倒影水面
	if( (!g_bWaterReflect)||(!osc_meshMgr::m_ptrWaterEffect) )
		return;


	//////////////////////////////////////////////////////////////////////////
	// 设置新的视矩阵 g_matView 及SetTransForm 及CAMER	
	osMatrix OldView = g_matView;
	osMatrix t_sOldProj = g_matProj;
	osMatrix t_sReflect,t_sTerrView;
	osPlane       t_sClipPlane;

	osVec3D       t_vec3CamPos,t_vec3BackCamPos;
	osVec3D       t_vec3CamFocus,t_vec3BackCamFocus;

	g_ptrCamera->get_curpos( &t_vec3BackCamPos );
	g_ptrCamera->get_camFocus( t_vec3BackCamFocus );

	BOOL  t_bCamTerrColBack = g_bCamTerrCol;
	g_bCamTerrCol = false;

	// 计算reflect面的矩阵
	osPlaneFromPointNormal( &t_sClipPlane,
		&osVec3D( 0.0f,m_sSceneTexData.get_waterHeight(),0.0f ),&osVec3D( 0.0f,1.0f,0.0f ) );


	osMatrixReflect( &t_sReflect,&t_sClipPlane );

	// 得到地形应该使用reflect view.
	t_sTerrView = t_sReflect*OldView;

	// 天空的reflect view 需要在camera设置
	// 测试去掉此段代码
	
	osVec3Transform( &t_vec3CamPos,&t_vec3BackCamPos,&t_sReflect );
	osVec3Transform( &t_vec3CamFocus,&t_vec3BackCamFocus,&t_sReflect );
	g_ptrCamera->back_upCamCaState();
	
	g_matView = t_sTerrView;
	m_middlePipe.set_viewMatrix( t_sTerrView );

	// process projectmatrix.
    osMatrix  matITView;
	osMatrix* t_ptrMat;
	float  t_fFarClip = g_ptrCamera->get_farClipPlaneDis();
	t_ptrMat = osMatrixInverse(&matITView, NULL, &g_matView);
	
	// River @ 2009-4-14:使用更稳定的错误处理方式,如果反矩阵为NULL,不更新水面.
	//osassert( NULL != t_ptrMat );
	if( NULL != t_ptrMat )
	{
		osMatrixTranspose(&matITView, &matITView);
		// transform world space water plane into camera space   
		osPlaneTransform(&t_sClipPlane, &t_sClipPlane, &matITView);
		// do oblique view frustum 
		g_ptrCamera->set_farClipPlane( SKYRENDER_FARDIS );
		g_ptrCamera->get_projmat( g_matProj );
		osn_mathFunc::clipProjectionMatrix(t_sClipPlane, g_matProj);
		m_middlePipe.set_projMatrix( g_matProj );


		// River @ 2007-1-27:这两句去掉debug版本d3d的warnning
		m_middlePipe.get_device()->SetTexture( 0,NULL );
		m_middlePipe.get_device()->SetTexture( 1,NULL );

		// 
		// WORK START:
		// 此处是5200显卡显示水面不正确的原因，在DEbug版本的dx下，会输出对应的警告信息
		m_pRefMapTexture->GetSurfaceLevel(0,&mpRefMapSurface);
		t_hr = m_middlePipe.get_device()->SetRenderTarget(0, mpRefMapSurface );
		osassert( !FAILED( t_hr ) );
		t_hr = m_middlePipe.get_device()->SetDepthStencilSurface( m_pRefMapDepth );
		osassert( !FAILED( t_hr ) );

		// 释放此surface
		SAFE_RELEASE( mpRefMapSurface );


		// 
		// 如果使用全屏抗锯齿，则不使用depth stencil Buffer.
		m_middlePipe.get_device()->Clear( 0, NULL, 
			D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x0, 1.0f, 0 );

		// 先渲染天空盒子.
		// 以后天空盒子预渲染成CubeMap，可以每帧渲染，也可以预渲染
		if (g_bRendSky)
		{
			if( m_bLoadedMap )
				render_skyDome( true );
		}


		// River added @ 2009-8-30:如果没有地形倒影，则不需要如下的渲染
		if( g_bTerrWaterReflect )
		{
			if( m_bLoadedMap )
			{
				// River @ 2009-8-5: 地形倒影有雾的状态出现.
				fog_rsSet();
				MapTile::DrawTerrain( true );
			}

			// 
			// 渲染场景内的桥类物品
			render_opacityObjPart( true ); 
		}

	}
	else
	{
		// ATTETION: 测试后，如果水面有问题，可能问题出在这。
		osDebugOut(" NULL INVERSE MATRIX....\n" );

		// 不能错过middlePipe的terrView.
		g_matView = t_sTerrView;
		m_middlePipe.set_viewMatrix( t_sTerrView );

	}

	//  
	//! 重新设置原来的camera和相关矩阵
	g_ptrCamera->set_curpos( &t_vec3BackCamPos );
	g_ptrCamera->set_camFocus( t_vec3BackCamFocus );
	g_ptrCamera->get_viewmat( g_matView );
	m_middlePipe.set_viewMatrix( g_matView );
	g_matProj = t_sOldProj;
	m_middlePipe.set_projMatrix( g_matProj );
	g_bCamTerrCol = t_bCamTerrColBack;
	g_ptrCamera->set_farClipPlane( t_fFarClip );

	g_ptrCamera->reset_CamCaState();

	
	/*
	static int t_iref = 0;
	if( t_iref < 100 )
		t_iref ++;
	else
	{
		D3DXSaveTextureToFileA("d:\\test.tga",D3DXIFF_TGA,m_pRefMapTexture,NULL );
		t_iref = 0;
	}
	*/

	// 
	//还原render target和depth stencil buffer
	if( !m_growEff.IsEffective() )
		m_middlePipe.get_device()->SetRenderTarget(0, mBackUpSurface);
	else
		m_growEff.reset_renderTarget( m_middlePipe.get_device() );
	t_hr = m_middlePipe.get_device()->SetDepthStencilSurface( m_pDepthStencilBackSur );
	osassert( !FAILED( t_hr ) );


	unguard;
}


BOOL gHaveVolumeFog = FALSE;
void osc_TGManager::UpDate_DepthMapAndRenderFog()
{
	guard;
	gVolumeFogPtr->InitVolumeFog(m_deviceMgr,m_middlePipe.get_device());
	if (!g_bUseVolumeFog)
	{
		return;
	}
	gVolumeFogPtr->PreRenderSceneDist();
	osMatrix ViewProjMat;
	osMatrixMultiply( &ViewProjMat,&g_matView,&g_matProj );

	float farClip = g_ptrCamera->get_farClipPlaneDis();
	float nearClip = g_ptrCamera->get_nearClipPlaneDis();
	static const char* PixelOffsetName = StringTable->insert("DepthPixelShaderOffset");
	double pixeloffset = gCardProfiler->getDoubleValue(PixelOffsetName);


	osVec4D wClip(nearClip,(1.0f/((farClip-nearClip))),(float)pixeloffset,0);
	osVec4D rampScale(1.0f,pow(2.0f,gVolumeFogPtr->m_DepthLowBitSize),pow(2.0f,2*gVolumeFogPtr->m_DepthLowBitSize),16.0f/255.0f);

	if (gHaveVolumeFog)
	{
		for( int t_i=0;t_i<m_iActMapNum;t_i++ )
		{
			osassert( this->m_arrPtrTG[t_i] );

			m_arrPtrTG[t_i]->render_AcceptFogObject();//

		}

		//g_ptrCamera->get_viewmat( g_matView );
		//g_ptrCamera->get_projmat( g_matProj );

		
		gVolumeFogPtr->mDepthEffect->SetVector("wClip",&wClip);
		gVolumeFogPtr->mDepthEffect->SetVector("RampScale",&rampScale);

		flush_opacityObjNoMaterial(gVolumeFogPtr->mDepthEffect,ViewProjMat);
	}
	
	//m_skinMeshMgr->render_smDepthMgr();

	//gVolumeFogPtr->DumpSceneDist();

	gHaveVolumeFog = FALSE;
	gVolumeFogPtr->PreRenderBackFog();

	g_shaderMgr->get_effect(gVolumeFogPtr->mBackFogShaderId)->m_peffect->SetVector("wClip",&wClip);
	g_shaderMgr->get_effect(gVolumeFogPtr->mBackFogShaderId)->m_peffect->SetVector("RampScale",&rampScale);
	m_middlePipe.render_aniFog(gVolumeFogPtr->mBackFogShaderId,8);

	//gVolumeFogPtr->DumpBackFogDist();




	gVolumeFogPtr->PreRenderFrontFog();
	g_shaderMgr->get_effect(gVolumeFogPtr->mFrontFogShaderId)->m_peffect->SetVector("wClip",&wClip);
	g_shaderMgr->get_effect(gVolumeFogPtr->mFrontFogShaderId)->m_peffect->SetVector("RampScale",&rampScale);
	m_middlePipe.render_aniFog(gVolumeFogPtr->mFrontFogShaderId,8);
	//gVolumeFogPtr->DumpFrontFogDist();

	
	gVolumeFogPtr->DrawSceneFog();
	

	unguard;
}


//! River added @ 2009-6-29:加入refraction map.
void osc_TGManager::update_refractionMap( void )
{
	guard;

	// 不画倒影水面
	if( (!g_bWaterReflect)||(!osc_meshMgr::m_ptrWaterEffect) )
		return;

	static int t_iRefractionMaskId = -1;
	static int t_iRefractionReflection = -1;
	
	if( t_iRefractionMaskId<0 )
	{
		t_iRefractionMaskId = g_shaderMgr->add_shader( 
			"refractionMask", osc_engine::get_shaderFile( "refractionMask" ).c_str() );
	}

	//! 最好先渲染一次屏幕多边形，把整个屏障的alpha颜色清除成白色，然后再渲染水面
	//  有问题的时候再渲染，节约渲染量
	//osc_d3dManager*  t_ptrMgr = (osc_d3dManager*)::get_deviceManagerPtr();
	//t_ptrMgr->draw_fullscrAlpha();


	g_bForceUseCustomShader = true;
	g_iForceCustomShaderIdx = t_iRefractionMaskId;
	m_middlePipe.render_aniWater( g_ptrCamera->get_frustum() );
	g_bForceUseCustomShader = false;
	

	// Get backbuffer into background texture  
	HRESULT   t_hr;
	if( !m_growEff.IsEffective() )
	{
		m_pRefractionTexture->GetSurfaceLevel( 0,&m_pRefractionSurface );
		m_middlePipe.get_device()->StretchRect(
			mBackUpSurface, 0, m_pRefractionSurface, 0, D3DTEXF_NONE);

		/*
		// River TEST CODE:
		static int t_iref = 0;
		if( t_iref < 100 )
			t_iref ++;
		else
		{
			D3DXSaveSurfaceToFileA("d:\\test.tga",D3DXIFF_TGA,m_pRefractionSurface,NULL,NULL );
			t_iref = 0;
		}
		*/
		
	}
	else
	{
		m_pRefractionTexture->GetSurfaceLevel( 0,&m_pRefractionSurface );
		t_hr = m_middlePipe.get_device()->StretchRect(
			m_growEff.get_targetSurface(),0,m_pRefractionSurface, 0, D3DTEXF_NONE);
		osassertex( !FAILED( t_hr ),osn_mathFunc::get_errorStr( t_hr ) );
	}

	SAFE_RELEASE( m_pRefractionSurface );

	m_middlePipe.set_renderState( D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALPHA | 
		D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED );

	return;

	unguard;
}

