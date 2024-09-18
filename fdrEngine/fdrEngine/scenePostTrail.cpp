//===============================================================================
/*! \file
*	Filename:	scenePostTrail.cpp
*	Desc:		this class will use the VS\PS to generate the grow effect 
*				of scene
*	His:		tzz	2008-6-30
*/
//===============================================================================
# include "stdafx.h"
# include ".\scenePostTrail.h"
# include ".\galaEffect_common.h"
# include "..\CardProfile\gfxCardProfile.h"

//====================================================================================================================
//
//	 CPostTrailEff class 
//
//====================================================================================================================

//--------------------------------------------------------------------------------------------------------------------
//Name: constructor and destructor
//Desc: ...
//--------------------------------------------------------------------------------------------------------------------
CPostTrailEff::CPostTrailEff(void):m_lpSceneDepthStencil(NULL),
									m_refreshTimer(0),
									m_extraMainSurface(NULL)
{
	ZeroMemory(m_lpTrailFrameTex,sizeof(m_lpTrailFrameTex));
	ZeroMemory(m_lpTrailFrameSur,sizeof(m_lpTrailFrameSur));
}
CPostTrailEff::~CPostTrailEff(void)
{
	DestroyEffRes();
}

//--------------------------------------------------------------------------------------------------------------------
//Name: DestroyGrowRes
//Desc: destroy the d3d resource
//--------------------------------------------------------------------------------------------------------------------
void CPostTrailEff::DestroyEffRes(void)
{
	

	SAFE_RELEASE(m_lpSceneDepthStencil);

	for(size_t i = 0;i <scm_maxTrailFrame;i++){
		SAFE_RELEASE(m_lpTrailFrameTex[i]);
		SAFE_RELEASE(m_lpTrailFrameSur[i]);
	}

	// call the base class' function
	// to destroy the resource
	CGalaEff::DestroyEffRes();

}
//--------------------------------------------------------------------------------------------------------------------
//Name: InitGrowScene
//Desc: initialize hte grow scene (set the 
//--------------------------------------------------------------------------------------------------------------------
BOOL CPostTrailEff::InitPostTrailScene(LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidth,int _iHeight,D3DFORMAT _format /* = D3DFMT_A8R8G8B8*/ ,BOOL _bUseFSAA /* = FALSE*/)
{
	guard;

	//class base class to initialize
	CGalaEff::InitEffScene(_lpd3dDev,_iWidth,_iHeight,_format,_bUseFSAA);

	const BOOL t_result = CreateRes(_lpd3dDev);

	if(!t_result){
		m_bInit = FALSE;
	}

	return t_result;
	
	unguard;
}


//! clear the current frame texture
void CPostTrailEff::ClearCurrFrameTex(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	guard;

	if(m_bInit){
		VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0,m_lpTrailFrameSur[0]));
		VALIDATE_D3D(_lpd3dDev->Clear(0,NULL,D3DCLEAR_TARGET,0,0,0));
	}
	
	unguard;
}
BOOL  CPostTrailEff::CreateRes(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	guard;
	
	LPDIRECT3DSURFACE9 t_surface;
	VALIDATE_D3D(_lpd3dDev->GetRenderTarget(0,&t_surface));

	for(size_t i = 0;i<scm_maxTrailFrame;i++){

		//create the texture to render on it
		if(FAILED(_lpd3dDev->CreateTexture( (UINT)sm_iWidth,(UINT)sm_iHeight,1,
											D3DUSAGE_RENDERTARGET,sm_format,D3DPOOL_DEFAULT,
											&m_lpTrailFrameTex[i],NULL))){
			DestroyEffRes();

			return FALSE;
		}

		VALIDATE_D3D(m_lpTrailFrameTex[i]->GetSurfaceLevel(0,&m_lpTrailFrameSur[i]));

		VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0,m_lpTrailFrameSur[i]));
		VALIDATE_D3D(_lpd3dDev->Clear(0,NULL,D3DCLEAR_TARGET,0,0,0));

	}
	VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0,t_surface));
	SAFE_RELEASE(t_surface);

	
	D3DSURFACE_DESC t_desc;
	VALIDATE_D3D(m_lpTrailFrameSur[0]->GetDesc(&t_desc));

	if(sm_bUseFSAA){
		// if using FSAA create a depth sentcil buffer to clear
		//
		VALIDATE_D3D(_lpd3dDev->CreateDepthStencilSurface((UINT)sm_iWidth,
															(UINT)sm_iHeight,
															D3DFMT_D24S8,
															t_desc.MultiSampleType,
															t_desc.MultiSampleQuality,
															FALSE,
															&m_lpSceneDepthStencil,
															NULL));

	}
	

	//create the rectangle 
	CreateRect(_lpd3dDev);

	//initilize the shader vertex declaration
	InitEffect(_lpd3dDev,"data\\effect\\post_trail.fx");

	m_refreshTimer = 0;

	return TRUE;

	unguard;
}

//! set curr frame target
void CPostTrailEff::SetCurrFrameTarget(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	guard;

	osassert(_lpd3dDev);

	if(m_bInit){
		VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0,m_lpTrailFrameSur[0]));
	}

	unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name: SetEffective
//Desc: set the new render target
//--------------------------------------------------------------------------------------------------------------------
void CPostTrailEff::SetEffective(LPDIRECT3DDEVICE9 _lpd3dDev,BOOL _bEffective /* = TRUE*/)
{
	guard;

	if(!m_bInit){
		return ;
	}

	m_bEffective = _bEffective;

	unguard;
}
void CPostTrailEff::SetMainRenderSurface(LPDIRECT3DSURFACE9 _mainSurface)
{
	guard;

	if(m_bInit){
		m_extraMainSurface = _mainSurface;
	}

	unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name: RenderEffScene
//Desc: render the grow scene with some complex step
//--------------------------------------------------------------------------------------------------------------------
void CPostTrailEff::RenderEffScene(LPDIRECT3DDEVICE9 _lpd3dDev,bool _alpha)
{
	guard;

	//not effective or initialize
	if(!m_bInit || !IsEffective()){
		return ;
	}

	// call the prepare effect render function
	// set the world view projection matrix and viewport size
	PrepareEffScene(_lpd3dDev);

	if(sm_bUseFSAA){
		VALIDATE_D3D(_lpd3dDev->SetDepthStencilSurface(m_lpSceneDepthStencil));
	}
	
	osassert(gCardProfiler);
	if(gCardProfiler->getBooleanValue("ATI_PostTrail_Tech")){
		VALIDATE_D3D(m_lpEffect->SetTechnique("ATI_PostTrailTech"));
	}else{
		VALIDATE_D3D(m_lpEffect->SetTechnique("NV_PostTrailTech"));
	}


	//begin shader
	UINT	t_uiPass = 0;
	VALIDATE_D3D(m_lpEffect->Begin(&t_uiPass,0));
	osassert(t_uiPass == 3);
	{
		// pass.0 fade pass
		//
		{
			VALIDATE_D3D(m_lpEffect->SetTexture("g_frameFade",m_lpTrailFrameTex[1]));

			VALIDATE_D3D(m_lpEffect->BeginPass(0));

			RenderToTex(_lpd3dDev,m_lpTrailFrameSur[2]);

			VALIDATE_D3D(m_lpEffect->EndPass());
		}

		// pass.1 blend pass
		//
		VALIDATE_D3D(m_lpEffect->SetTexture("g_frameFade",m_lpTrailFrameTex[2]));
		VALIDATE_D3D(m_lpEffect->SetTexture("g_frameMain",m_lpTrailFrameTex[0]));
		{

			VALIDATE_D3D(m_lpEffect->BeginPass(1));

			RenderToTex(_lpd3dDev,m_lpTrailFrameSur[1]);

			VALIDATE_D3D(m_lpEffect->EndPass());

		}

		// pass.2 render to screen
		//
		VALIDATE_D3D(m_lpEffect->SetTexture("g_frameMain",m_lpTrailFrameTex[1]));
		{
			VALIDATE_D3D(m_lpEffect->BeginPass(2));

			//D3DXSaveSurfaceToFile("0.png",D3DXIFF_PNG,m_lpTrailFrameSur[0],NULL,NULL);
		
			// set the main render target back and 		
			// render the rectangle with grow texture to the screen
			// and restore the scene texture to render target
			if(m_extraMainSurface){
				VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0, m_extraMainSurface));
			}else{
				VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0, m_lpMainSurface));
			}
			

			
			//D3DXSaveSurfaceToFile("1.png",D3DXIFF_PNG,m_lpMainSurface,NULL,NULL);

			if(sm_bUseFSAA){
				VALIDATE_D3D(_lpd3dDev->SetDepthStencilSurface(m_lpBackupDepthStencil));

				VALIDATE_D3D(_lpd3dDev->Clear(0,NULL,D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,0,1.0f,0));
			}


			VALIDATE_D3D(_lpd3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2));

			//D3DXSaveSurfaceToFile("2.png",D3DXIFF_PNG,m_lpMainSurface,NULL,NULL);

			if(sm_bUseFSAA){
				VALIDATE_D3D(_lpd3dDev->SetDepthStencilSurface(m_lpSceneDepthStencil));

				VALIDATE_D3D(_lpd3dDev->Clear(0,NULL,D3DCLEAR_ZBUFFER| D3DCLEAR_STENCIL,0,1.0f,0));
			}

			VALIDATE_D3D(m_lpEffect->EndPass());

			
		}
		
	}

	VALIDATE_D3D(m_lpEffect->End());

	m_refreshTimer++;
	
	
	unguard;
}

