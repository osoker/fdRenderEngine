//===============================================================================
/*! \file
*	Filename:	sceneGrow.cpp
*	Desc:		this class will use the VS\PS to generate the grow effect 
*				of scene
*	His:		tzz	2008-6-30
*/
//===============================================================================
# include "stdafx.h"
# include ".\sceneGrow.h"
# include ".\galaEffect_common.h"
# include "../interface/miskFunc.h"
# include "../terrain/include/fdFieldMgr.h"


# define SCREEN_WAVEPS  "data\\effect\\screenWavePS.fx"
//WZ 扰动纹理地址
# define NOISE_TEXTURE  "data\\effect\\noiseTexture.jpg"

//! 
# define SCENE_GROWEFFECT "data\\effect\\grow.fx"
//====================================================================================================================
//
//	 CGrowEff class 
//
//====================================================================================================================

//--------------------------------------------------------------------------------------------------------------------
//Name: constructor and destructor
//Desc: ...
//--------------------------------------------------------------------------------------------------------------------
CGrowEff::CGrowEff(void):m_lpSceneTex(NULL),
						m_lpSceneSurface(NULL),
						m_lpBlurTex(NULL),
						m_lpBlurSurface(NULL),
						m_lpBlurTex1(NULL),
						m_lpBlurSurface1(NULL),
						m_lpSceneDepthStencil(NULL)
					//WZ
						, m_bWaterActive(false)// 水下扰动效果开关
						, m_lpNoiseTex(NULL)//扰动随机纹理指针
{
	// River @ 2010-6-24:屏幕效果
	m_lpScreenTex = NULL;
	m_lpScreenSurface = NULL;
	m_lpScreenWaveEffect = NULL;
	// noting to do now
}
CGrowEff::~CGrowEff(void)
{
	DestroyEffRes();
}

//--------------------------------------------------------------------------------------------------------------------
//Name: DestroyGrowRes
//Desc: destroy the d3d resource
//--------------------------------------------------------------------------------------------------------------------
void CGrowEff::DestroyEffRes(void)
{
	// call the base class' function
	// to destroy the resource
	CGalaEff::DestroyEffRes();
	
	SAFE_RELEASE(m_lpSceneSurface);
	SAFE_RELEASE(m_lpSceneTex);
	SAFE_RELEASE(m_lpBlurSurface);
	SAFE_RELEASE(m_lpBlurTex);
	SAFE_RELEASE(m_lpBlurSurface1);
	SAFE_RELEASE(m_lpBlurTex1);
	SAFE_RELEASE(m_lpSceneDepthStencil);
	//@{WZ
	//release the point of noiseTexture.
	SAFE_RELEASE(m_lpNoiseTex);
	//@}
	// River @ 2010-6-25:屏幕光波相关的效果 
	release_screenWaveRes();


}
//--------------------------------------------------------------------------------------------------------------------
//Name: InitGrowScene
//Desc: initialize hte grow scene (set the 
//--------------------------------------------------------------------------------------------------------------------
BOOL CGrowEff::InitGrowScene(LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidth,int _iHeight,D3DFORMAT _format /* = D3DFMT_A8R8G8B8*/ ,BOOL _bUseFSAA /* = FALSE*/)
{
	guard;

	BOOL t_result = TRUE;

	//class base class to initialize
	CGalaEff::InitEffScene(_lpd3dDev,_iWidth,_iHeight,_format,_bUseFSAA);

	if(m_lpSceneTex == NULL){
		t_result = CreateRes(_lpd3dDev);
	}

	if(!t_result){
		m_bInit = FALSE;
	}

	return t_result;
	
	unguard;
}


BOOL  CGrowEff::CreateRes(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	guard;


	//create the texture to render on it
	if(FAILED( _lpd3dDev->CreateTexture( (UINT)sm_iWidth,(UINT)sm_iHeight,1,
			D3DUSAGE_RENDERTARGET,sm_format,D3DPOOL_DEFAULT,&m_lpSceneTex,NULL ) )){
		
		return FALSE;
	}
	
	//osassert(m_lpSceneTex);
	VALIDATE_D3D(m_lpSceneTex->GetSurfaceLevel(0,&m_lpSceneSurface));

	D3DSURFACE_DESC t_desc;
	VALIDATE_D3D(m_lpSceneSurface->GetDesc(&t_desc));

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

	//create the blur texture which blur in fx file
	if(FAILED(_lpd3dDev->CreateTexture((UINT)sm_iWidth,(UINT)sm_iHeight,1,
										D3DUSAGE_RENDERTARGET,sm_format,D3DPOOL_DEFAULT,&m_lpBlurTex,NULL))){

		DestroyEffRes();

		return FALSE;
	}

	osassert(m_lpBlurTex);
	VALIDATE_D3D(m_lpBlurTex->GetSurfaceLevel(0,&m_lpBlurSurface));




	//create the blur texture which blur in fx file
	if(FAILED(_lpd3dDev->CreateTexture((UINT)sm_iWidth,(UINT)sm_iHeight,1,
										D3DUSAGE_RENDERTARGET,sm_format,D3DPOOL_DEFAULT,&m_lpBlurTex1,NULL))){
		DestroyEffRes();

		return FALSE;
	}

	osassert(m_lpBlurTex1);
	VALIDATE_D3D(m_lpBlurTex1->GetSurfaceLevel(0,&m_lpBlurSurface1));

	//create the rectangle 
	CreateRect(_lpd3dDev);

	//initilize the shader vertex declaration
	InitEffect(_lpd3dDev,SCENE_GROWEFFECT );

	//@{WZ 2010-7
	// Read the Texture file into tmp buffer
	//The texture is used for ramdom noise.
	CReadFile t_file;
	if(!t_file.OpenFile(NOISE_TEXTURE)){
		osassert(false);
	}

	if(FAILED(D3DXCreateTextureFromFileInMemory(_lpd3dDev,t_file.GetFullBuffer(),t_file.GetFileSize(),&m_lpNoiseTex))){
		osassertex( false,va( "Create  texture <%s> failed",NOISE_TEXTURE ) );
	}

	t_file.CloseFile();
	//@}

	// re-active water effect
	SetWaterActive(_lpd3dDev,m_bWaterActive);

	return TRUE;

	unguard;
}

//! River: 重设Effect用的renderTarget.
void CGrowEff::reset_renderTarget( LPDIRECT3DDEVICE9 _lpd3dDev )
{
	guard;

	if( !m_bEffective )
		return;

	if(sm_bUseFSAA)
		VALIDATE_D3D(_lpd3dDev->SetDepthStencilSurface(m_lpSceneDepthStencil));

	// set the render target to the scene texture
	VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0,m_lpSceneSurface));

	return;

	unguard;
}


//--------------------------------------------------------------------------------------------------------------------
//Name: SetEffective
//Desc: set the new render target
//--------------------------------------------------------------------------------------------------------------------
void CGrowEff::SetEffective(LPDIRECT3DDEVICE9 _lpd3dDev,BOOL _bEffective /* = TRUE*/)
{
	guard;

	if(!m_bInit){
		return ;
	}

	if(m_bEffective == _bEffective){
		return;
	}

	//pre-condition to confirm has been initialized
	osassert(sm_iWidth && m_lpSceneTex);
	
	if(_bEffective){
		
		if(!m_bEffective){	// not active

			if(sm_bUseFSAA){
				// backup the stencil buffer and set the current's NULL
				//
				if(!m_lpBackupDepthStencil){
					VALIDATE_D3D(_lpd3dDev->GetDepthStencilSurface(&m_lpBackupDepthStencil));
				}
				VALIDATE_D3D(_lpd3dDev->SetDepthStencilSurface(m_lpSceneDepthStencil));
				
			}

			if(!m_lpMainSurface){
				VALIDATE_D3D(_lpd3dDev->GetRenderTarget(0,&m_lpMainSurface));				
			}	
			
			osassert(m_lpSceneSurface);
			// set the render target to the scene texture
			VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0,m_lpSceneSurface));
		}

	}else{
		if(m_bEffective){	// active
			//set the main rendering surface back 
			osassert(m_lpMainSurface);
			if(FAILED(_lpd3dDev->SetRenderTarget(0,m_lpMainSurface))){
				osassert(false);
			}
			
			if(sm_bUseFSAA){
				// restore the backup depth stencil
				//
				VALIDATE_D3D(_lpd3dDev->SetDepthStencilSurface(m_lpBackupDepthStencil));
			}
		}
	}
	
	m_bEffective = _bEffective;

	unguard;
}

//! 屏幕多边形渲染用到的fvf.
# define SCREENWAVE_DISFVF ( D3DFVF_XYZRHW | D3DFVF_TEX1   )

struct s_swVert
{
	osVec4D    m_vec4Pos;
	osVec2D    m_vec2Uv;
};

//! 删除屏幕光波相关的资源.
BOOL CGrowEff::release_screenWaveRes( void )
{
	guard;

	//! River @ 20106-24:屏幕效果
	SAFE_RELEASE( m_lpScreenSurface );
	SAFE_RELEASE( m_lpScreenTex );
	SAFE_RELEASE( m_lpScreenWaveEffect );

	m_iWidth = 0;
	m_iHeight = 0;

	return true;

	unguard;
}


//! River @ 2010-6-25:屏幕光波相关资源初始化.
BOOL CGrowEff::reinit_screenWave( LPDIRECT3DDEVICE9 _lpd3dDev )
{
	guard;

	osc_d3dManager* t_ptrMgr = (osc_d3dManager*)::get_deviceManagerPtr();

	m_iWidth = t_ptrMgr->get_scrwidth();
	m_iHeight = t_ptrMgr->get_scrheight();

	if( FAILED( _lpd3dDev->CreateTexture( m_iWidth,m_iHeight,1,
		D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&m_lpScreenTex,NULL ) ) )
		osassertex( false,"Create screen texture failed....\n" );
	if( FAILED( m_lpScreenTex->GetSurfaceLevel( 0,&m_lpScreenSurface ) ) )
		osassertex( false,"Get screen surface failed....\n" );


	// 创建EffectFile.
	LPD3DXBUFFER	t_error = NULL;
	CReadFile t_file;
	if(!t_file.OpenFile( SCREEN_WAVEPS )){
		osassert(false);
	}

	if(FAILED(D3DXCreateEffect(_lpd3dDev,t_file.GetFullBuffer(),
		t_file.GetFileSize(),NULL,NULL,NULL,NULL,&m_lpScreenWaveEffect,&t_error)))
	{
		osassertex( false,
			va("Effect<%s> compile error<%s>..\n",SCREEN_WAVEPS,
			(char*)(t_error->GetBufferPointer())) );
	}

	t_file.CloseFile();


	return true;

	unguard;
}


// River @ 2010-6-24:渲染屏幕效果
void CGrowEff::RenderScreenWave( LPDIRECT3DDEVICE9 _lpd3dDev )
{

	if( (!g_bRenderScreenWave)||(!g_bScreenBlastWave) )
		return ;

	// 第一次创建.
	if( !m_lpScreenTex )
		reinit_screenWave( _lpd3dDev );

	osassert( m_lpScreenWaveEffect );

	_lpd3dDev->StretchRect( osc_TGManager::mBackUpSurface,0,
		m_lpScreenSurface,0,D3DTEXF_NONE );

	_lpd3dDev->SetFVF( SCREENWAVE_DISFVF );

	s_swVert   t_vert[4];

	t_vert[0].m_vec2Uv = osVec2D( 0.0f,0.0f );
	t_vert[1].m_vec2Uv = osVec2D( 1.0f,0.0f );
	t_vert[2].m_vec2Uv = osVec2D( 1.0f,1.0f );
	t_vert[3].m_vec2Uv = osVec2D( 0.0f,1.0f );

	t_vert[0].m_vec4Pos = osVec4D( 0.0f,0.0f,0.5f,0.5f );
	t_vert[1].m_vec4Pos = osVec4D( (float)m_iWidth-0.5f,0.0f,0.5f,0.5f );
	t_vert[2].m_vec4Pos = osVec4D( (float)m_iWidth-0.5f,(float)m_iHeight-0.5f,0.5f,0.5f );
	t_vert[3].m_vec4Pos = osVec4D( 0.0f,(float)m_iHeight-0.5f,0.5f,0.5f );


	//begin shader
	static osVec2D  t_vec2Pos = osVec2D( 0.0f,0.0f );

	t_vec2Pos.x = float_mod( sg_timer::Instance()->get_talelatime(),1.0f );
	t_vec2Pos.y = t_vec2Pos.x;

	//! 设置相关的变量.
	VALIDATE_D3D( m_lpScreenWaveEffect->SetTexture("Texture0",m_lpScreenTex ) );

	UINT	t_uiPass = 0;	
	VALIDATE_D3D(m_lpScreenWaveEffect->Begin(&t_uiPass,0));

	VALIDATE_D3D(m_lpScreenWaveEffect->BeginPass(0));

	VALIDATE_D3D(_lpd3dDev->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,2,t_vert,sizeof(s_swVert) ));

	VALIDATE_D3D(m_lpScreenWaveEffect->EndPass());

	m_lpScreenWaveEffect->End();
	_lpd3dDev->SetPixelShader( NULL );


	// TEST CODE:
	//D3DXSaveSurfaceToFile("d:\\test.TGA",D3DXIFF_TGA,osc_TGManager::mBackUpSurface,NULL,NULL);


}



//--------------------------------------------------------------------------------------------------------------------
//Name: RenderEffScene
//Desc: render the grow scene with some complex step
//--------------------------------------------------------------------------------------------------------------------
void CGrowEff::RenderEffScene(LPDIRECT3DDEVICE9 _lpd3dDev,bool _alpha)
{
	guard;


	//not effective
	if(!IsEffective()){
		return ;
	}

	// call the prepare effect render function
	// set the world view projection matrix and viewport size
	PrepareEffScene(_lpd3dDev);

	// set the texture 
	VALIDATE_D3D(m_lpEffect->SetTexture("g_sceneTex",m_lpSceneTex));

	//const BOOL t_writeSurface = ::GetAsyncKeyState(VK_CONTROL) & 0x8000;

// 	if(t_writeSurface){
// 		D3DXSaveSurfaceToFile("0.png",D3DXIFF_PNG,m_lpSceneSurface,NULL,NULL);
// 	}


	//@{ WZ
	// 水下扰动效果相关设置
	//.
	if(m_bWaterActive){
		VALIDATE_D3D(m_lpEffect->SetTexture("g_noiseTex",m_lpNoiseTex));//设置纹理
		VALIDATE_D3D(m_lpEffect->SetFloat("g_fTime",float_mod(0.0075f*sg_timer::Instance()->get_talelatime(),2.0f*OS_PI)));//设置时间
		float t_texSize[2]={(float)sm_iWidth,(float)sm_iHeight};//设置屏幕大小，
		VALIDATE_D3D(m_lpEffect->SetFloatArray("g_texSize",t_texSize,2));//使得放缩屏幕扰动大小不变
	}	
	//@}
	
	if(sm_bUseFSAA){
		VALIDATE_D3D(_lpd3dDev->SetDepthStencilSurface(m_lpSceneDepthStencil));
	}
	
	//begin shader
	UINT	t_uiPass = 0;	
	VALIDATE_D3D(m_lpEffect->Begin(&t_uiPass,0));
	osassert(t_uiPass == 3);
	{
	
		// pass.0 horizontal blur
		//
		{
			
			VALIDATE_D3D(m_lpEffect->BeginPass(0));

			//render the blur texture;
			RenderToTex(_lpd3dDev,m_lpBlurSurface);
			VALIDATE_D3D(m_lpEffect->EndPass());

		}
// 		if(t_writeSurface){
// 			D3DXSaveSurfaceToFile("1.png",D3DXIFF_PNG,m_lpBlurSurface,NULL,NULL);
// 		}


		// pass.1 vertical blur
		//
		VALIDATE_D3D(m_lpEffect->SetTexture("g_blurTex",m_lpBlurTex));
		{
			
			VALIDATE_D3D(m_lpEffect->BeginPass(1));

			//render the blur texture;
			RenderToTex(_lpd3dDev,m_lpBlurSurface1);
			VALIDATE_D3D(m_lpEffect->EndPass());

		}

// 		if(t_writeSurface){
// 			D3DXSaveSurfaceToFile("2.png",D3DXIFF_PNG,m_lpBlurSurface1,NULL,NULL);
// 		}

		// In pass 1 the we must use the sampler of blurtexture 
		// set the variable in fx file 
		// but in pass the blur texture is render texture
		VALIDATE_D3D(m_lpEffect->SetTexture("g_blurTex",m_lpBlurTex1));

		{
			VALIDATE_D3D(m_lpEffect->BeginPass(2));

			// set the main render target back and 		
			// render the rectangle with grow texture to the screen
			// and restore the scene texture to render target
			VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0, m_lpMainSurface));

		
			if(sm_bUseFSAA){
				VALIDATE_D3D(_lpd3dDev->SetDepthStencilSurface(m_lpBackupDepthStencil));

				VALIDATE_D3D(_lpd3dDev->Clear(0,NULL,D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,0,1.0f,0));
			}


			VALIDATE_D3D(_lpd3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2));

// 			if(t_writeSurface){
// 				D3DXSaveSurfaceToFile("3.png",D3DXIFF_PNG,m_lpMainSurface,NULL,NULL);
// 			}


			VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0, m_lpSceneSurface));
			
			if(sm_bUseFSAA){
				VALIDATE_D3D(_lpd3dDev->SetDepthStencilSurface(m_lpSceneDepthStencil));

				VALIDATE_D3D(_lpd3dDev->Clear(0,NULL,D3DCLEAR_ZBUFFER| D3DCLEAR_STENCIL,0,1.0f,0));
			}

			VALIDATE_D3D(m_lpEffect->EndPass());

			
		}
		
	}

	VALIDATE_D3D(m_lpEffect->End());

	
	unguard;
}
//! WZ 设置是否使用水下扰动效果
void CGrowEff::SetWaterActive(LPDIRECT3DDEVICE9 _lpd3dDev,BOOL _bAct)
{
	guard;

	if(m_bInit){

		if (_bAct){
			osassert(m_lpNoiseTex);
			VALIDATE_D3D(m_lpEffect->SetTechnique("BlurWithWaterTech"));
		}else{
			VALIDATE_D3D(m_lpEffect->SetTechnique("BlurTech"));
		}
	}

	m_bWaterActive= _bAct;
	
	unguard;
}
