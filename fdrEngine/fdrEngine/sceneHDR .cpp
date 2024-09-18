//===============================================================================
/*! \file
*	Filename:	sceneGrow.cpp
*	Desc:		this class will use the VS\PS to generate the HDR effect 
*				of scene
*	His:		tzz	2008-6-30
*/
//===============================================================================
# include "stdafx.h"
# include ".\sceneHDR.h"
# include ".\galaEffect_common.h"

//====================================================================================================================
//
//	 CHDREff class 
//
//====================================================================================================================

//--------------------------------------------------------------------------------------------------------------------
//Name: constructor and destructor
//Desc: ...
//--------------------------------------------------------------------------------------------------------------------
CHDREff::CHDREff(void):m_lpSceneTex(NULL),
						m_lpSceneSurface(NULL),
						m_lpBlurTex(NULL),
						m_lpBlurSurface(NULL),
						m_lpBlurTex1(NULL),
						m_lpBlurSurface1(NULL),
						m_fGaussMean(0.0f),
						m_fGaussStdDev(0.8f),
						m_fGaussMultiplier(0.5f),
						m_fExposure(1.6f),
						m_fBrightSelThreshold(0.8f)
					
{
	
	ZeroMemory(m_arrBlurWeight,sizeof(m_arrBlurWeight));
	ZeroMemory(m_downSamplerOffset,sizeof(m_downSamplerOffset));
	ZeroMemory(m_HBlurOffset,sizeof(m_HBlurOffset));
	ZeroMemory(m_VBlurOffset,sizeof(m_VBlurOffset));

	ZeroMemory(m_lpLuminTex,sizeof(m_lpLuminTex));
	ZeroMemory(m_lpLuminSurface,sizeof(m_lpLuminSurface));
}
CHDREff::~CHDREff(void)
{
	DestroyEffRes();
}

//--------------------------------------------------------------------------------------------------------------------
//Name: DestroyGrowRes
//Desc: destroy the d3d resource
//--------------------------------------------------------------------------------------------------------------------
void CHDREff::DestroyEffRes(void)
{
	// call the base class' function
	// to destroy the resource
	CGalaEff::DestroyEffRes();
	
	SAFE_RELEASE(m_lpSceneSurface);
	SAFE_RELEASE(m_lpSceneTex);

	for(int i = 0 ;i < scm_iLuminanceLev;i++){
		SAFE_RELEASE(m_lpLuminTex[i]);
		SAFE_RELEASE(m_lpLuminSurface[i]);
	}

	SAFE_RELEASE(m_lpBlurSurface);
	SAFE_RELEASE(m_lpBlurTex);
	SAFE_RELEASE(m_lpBlurSurface1);
	SAFE_RELEASE(m_lpBlurTex1);
	
	
}

//--------------------------------------------------------------------------------------------------------------------
//Name: InitGrowScene
//Desc: initialize hte grow scene (set the 
//--------------------------------------------------------------------------------------------------------------------
BOOL CHDREff::InitGrowScene(LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidth,int _iHeight,D3DFORMAT _format /* = D3DFMT_A8R8G8B8*/)
{
	guard;

	//class base class to initialize
	CGalaEff::InitEffScene(_lpd3dDev,_iWidth,_iHeight,_format);

	const BOOL t_result = CreateRes(_lpd3dDev);

	if(!t_result){
		m_bInit = FALSE;
	}

	return t_result;
	
	unguard;
}

//! create the effect resource
BOOL CHDREff::CreateRes(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	guard;

	osassert(sm_iWidth && sm_iHeight);

	//create the texture to render on it
	//
	if(FAILED(_lpd3dDev->CreateTexture((UINT)sm_iWidth,(UINT)sm_iHeight,1,
										D3DUSAGE_RENDERTARGET ,sm_format,D3DPOOL_DEFAULT,&m_lpSceneTex,NULL))){
		

		return FALSE;
	}
	osassert(m_lpSceneTex);
	VALIDATE_D3D(m_lpSceneTex->GetSurfaceLevel(0,&m_lpSceneSurface));

	
	
	//create the computing luminance texture 
		//
	for(int i = 0 ; i< scm_iLuminanceLev ; i++){
		
		const int t_luminWidth = sm_iWidth >> i ;
		const int t_luminHeight = sm_iHeight >> i ;
		if(FAILED(_lpd3dDev->CreateTexture((UINT)t_luminWidth,(UINT)t_luminHeight,1,
											D3DUSAGE_RENDERTARGET,sm_format,D3DPOOL_DEFAULT,&m_lpLuminTex[i],NULL))){
			
			DestroyEffRes();

			return FALSE;
		}

		osassert(m_lpLuminTex[i]);
		VALIDATE_D3D(m_lpLuminTex[i]->GetSurfaceLevel(0,&m_lpLuminSurface[i]));
	}



	const int t_blurWidth = sm_iWidth >> 3 ;
	const int t_blurHeight = sm_iHeight >> 3 ;
	//create the blur texture which blur in fx file
	if(FAILED(_lpd3dDev->CreateTexture((UINT)t_blurWidth,(UINT)t_blurHeight,1,
										D3DUSAGE_RENDERTARGET,sm_format,D3DPOOL_DEFAULT,&m_lpBlurTex,NULL))){
		DestroyEffRes();

		return FALSE;
	}
	osassert(m_lpBlurTex);
	VALIDATE_D3D(m_lpBlurTex->GetSurfaceLevel(0,&m_lpBlurSurface));

	//create the blur texture(one) which blur in fx file
	if(FAILED(_lpd3dDev->CreateTexture((UINT)t_blurWidth,(UINT)t_blurHeight,1,
										D3DUSAGE_RENDERTARGET,sm_format,D3DPOOL_DEFAULT,&m_lpBlurTex1,NULL))){
		DestroyEffRes();

		return FALSE;
	}
	osassert(m_lpBlurTex1);
	VALIDATE_D3D(m_lpBlurTex1->GetSurfaceLevel(0,&m_lpBlurSurface1));

	//create the rectangle 
	CreateRect(_lpd3dDev);

	//initilize the shader vertex declaration
	InitEffect(_lpd3dDev,"data\\effect\\hdrEff.fx");

	return TRUE;

	unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name: SetEffective
//Desc: set the new render target
//--------------------------------------------------------------------------------------------------------------------
void CHDREff::SetEffective(LPDIRECT3DDEVICE9 _lpd3dDev,BOOL _bEffective /* = TRUE*/)
{
	guard;

	if(!m_bInit){
		return ;
	}

	//pre-condition to confirm has been initialized
	osassert(sm_iWidth && m_lpSceneTex);

	
	if(_bEffective){
		
		if(!m_bEffective){	// not active
			
			SAFE_RELEASE(m_lpMainSurface);

			VALIDATE_D3D(_lpd3dDev->GetRenderTarget(0,&m_lpMainSurface));

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
		}
	}
	
	m_bEffective = _bEffective;

	unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name: RenderEffScene
//Desc: render the grow scene with some complex step
//--------------------------------------------------------------------------------------------------------------------
void CHDREff::RenderEffScene(LPDIRECT3DDEVICE9 _lpd3dDev,bool _alpha)
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



 	//begin shader
	UINT	t_uiPass = 0;	

	//PrepareComputeLuminance();


	VALIDATE_D3D(m_lpEffect->Begin(&t_uiPass,0));
	osassert(t_uiPass == 13);
	{

		

		int t_iEndLuminPass = 0;
		// pass.0~5 
		// down sample the original scene to get the luminance color
		// to realize the "Dynamic" 
		{

			for(int i = 0 ;i< scm_iLuminanceLev;i++){
				VALIDATE_D3D(m_lpEffect->BeginPass(i));
				RenderToTex(_lpd3dDev,m_lpLuminSurface[i]);
				
				//if(i < scm_iLuminanceLev - 1){
				VALIDATE_D3D(m_lpEffect->SetTexture("g_luminTex",m_lpLuminTex[i]));	
				//}
				VALIDATE_D3D(m_lpEffect->EndPass());
			}
			t_iEndLuminPass = scm_iLuminanceLev ;
			
		}

		// pass.8
		// bright pass (select the bright pixel)
		{
			PrepareBrightPassData();

			VALIDATE_D3D(m_lpEffect->BeginPass(t_iEndLuminPass ));

			// render the bright pixel to pass
			RenderToTex(_lpd3dDev,m_lpBlurSurface1);

			VALIDATE_D3D(m_lpEffect->EndPass());
		}

		//D3DXSaveTextureToFile("./0.png",D3DXIFF_PNG,m_lpBlurTex1,NULL);

		// pass.9 
		// down sample the image to prepare blur
		//
		VALIDATE_D3D(m_lpEffect->SetTexture("g_blurTex",m_lpBlurTex1));
		{
						
			PrepareDownSampleData();

			VALIDATE_D3D(m_lpEffect->BeginPass(t_iEndLuminPass + 1));

			// render the blur texture (don't blur just use the blur texture to down 
			// sample the original scene image;
			RenderToTex(_lpd3dDev,m_lpBlurSurface);

			VALIDATE_D3D(m_lpEffect->EndPass());

		}

		//D3DXSaveTextureToFile("./1.png",D3DXIFF_PNG,m_lpBlurTex,NULL);

	
		VALIDATE_D3D(m_lpEffect->SetTexture("g_blurTex",m_lpBlurTex));
		// pass.10
		// horizontal blur
		// 
		{
			PrepareHBlurData();

			VALIDATE_D3D(m_lpEffect->BeginPass(t_iEndLuminPass + 2));

			RenderToTex(_lpd3dDev,m_lpBlurSurface1);
			VALIDATE_D3D(m_lpEffect->EndPass());
		}

		//D3DXSaveTextureToFile("./2.png",D3DXIFF_PNG,m_lpBlurTex1,NULL);

		VALIDATE_D3D(m_lpEffect->SetTexture("g_blurTex",m_lpBlurTex1));
		// pass.11
		// horizontal blur
		// 
		{
			PrepareVBlurData();

			VALIDATE_D3D(m_lpEffect->BeginPass(t_iEndLuminPass + 3));

			RenderToTex(_lpd3dDev,m_lpBlurSurface);
			VALIDATE_D3D(m_lpEffect->EndPass());
		}

		//D3DXSaveTextureToFile("./3.png",D3DXIFF_PNG,m_lpBlurTex,NULL);


		// pass.12
		// we must use the sampler of blurtexture set the variable in fx file 
		// but in pass the blur texture is render texture
		VALIDATE_D3D(m_lpEffect->SetTexture("g_blurTex",m_lpBlurTex));
		{
			
			PrepareFinalPassData();

			VALIDATE_D3D(m_lpEffect->BeginPass(t_iEndLuminPass + 4));

			// set the main render target back and 		
			// render the rectangle with grow texture to the screen
			// and restore the scene texture to render target
			VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0, m_lpMainSurface));

		
			VALIDATE_D3D(_lpd3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2));


			VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0,m_lpSceneSurface));
			VALIDATE_D3D(m_lpEffect->EndPass());

		}
		
	}
	VALIDATE_D3D(m_lpEffect->End());

	
	unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name: PrepareBrightPassData
//Desc:  prepare the selecting bright pixel data
//--------------------------------------------------------------------------------------------------------------------
void CHDREff::PrepareBrightPassData(void)
{
	guard;

	// set the bright pass color threshold and exposure of HDR light
	VALIDATE_D3D(m_lpEffect->SetFloat("g_fExposure",m_fExposure));
	VALIDATE_D3D(m_lpEffect->SetFloat("g_fBrightSelThreshold",m_fBrightSelThreshold));
	

	const float sU = 1.0f / sm_iWidth;
	const float sV = 1.0f / sm_iHeight;
	
	m_downSamplerOffset[0] = D3DXVECTOR4( -0.5f * sU,  0.5f * sV, 0.0f, 0.0f );
	m_downSamplerOffset[1] = D3DXVECTOR4(  0.5f * sU,  0.5f * sV, 0.0f, 0.0f );
	m_downSamplerOffset[2] = D3DXVECTOR4( -0.5f * sU, -0.5f * sV, 0.0f, 0.0f );
	m_downSamplerOffset[3] = D3DXVECTOR4(  0.5f * sU, -0.5f * sV, 0.0f, 0.0f );
	

	VALIDATE_D3D(m_lpEffect->SetFloatArray("g_downSamplerOffset",(float*)m_downSamplerOffset,4 * 4));


	unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name: PrepareDownSampleData
//Desc:  prepare the down sample data...
//--------------------------------------------------------------------------------------------------------------------
void CHDREff::PrepareDownSampleData(void)
{
	guard;
	const float sU =  1.0f / static_cast< float >( sm_iWidth );
	const float sV =  1.0f / static_cast< float >(sm_iHeight);

	int idx = 0;
	for( int i = -2; i < 2; i++ ){
        for( int j = -2; j < 2; j++ ){
            m_downSamplerOffset[idx++] = D3DXVECTOR4( 
                                        (static_cast< float >( i ) + 0.5f) * sU, 
                                        (static_cast< float >( j ) + 0.5f) * sV,
										0.0f,
										0.0f
                                        );
        }
    }
	

	VALIDATE_D3D(m_lpEffect->SetFloatArray("g_downSamplerOffset",(float*)m_downSamplerOffset,16 * 4));


	//// Create the 3x3 grid of offsets
 //   D3DXVECTOR2 t_DSoffsets[9];
	//idx = 0;
 //   for( int x = -1; x < 2; x++ )
 //   {
 //       for( int y = -1; y < 2; y++ )
 //       {
 //           t_DSoffsets[idx++] = D3DXVECTOR2(
 //                                   static_cast< float >( x ) / static_cast< float >( sm_iWidth ),
 //                                   static_cast< float >( y ) / static_cast< float >( sm_iHeight )
	//								);
 //       }
 //   }

	//VALIDATE_D3D(m_lpEffect->SetFloatArray("g_LumindownSamplerOffset",(float*)(t_DSoffsets),9 * 2));



	unguard;

}

//--------------------------------------------------------------------------------------------------------------------
//Name: PrepareHBlurData
//Desc:  prepare the horizontal blur pass data ,see the code to detail
//--------------------------------------------------------------------------------------------------------------------
void CHDREff::PrepareHBlurData(void)
{
	guard;

	for(int i = 0 ; i < 9 ; i++){
        // 'x' is just a simple alias to map the [0,8] range down to a [-1,+1]
        const float x = (static_cast< float >( i ) - 4.0f) / 4.0f;

		m_HBlurOffset[i] = (static_cast< float >( i ) - 4.0f) * ( 1.0f / static_cast< float >( sm_iWidth ) );

        // Use a gaussian distribution. Changing the standard-deviation
        // (second parameter) as well as the amplitude (multiplier) gives
        // distinctly different results.
        m_arrBlurWeight[i] = m_fGaussMultiplier * ComputeGaussianValue( x, m_fGaussMean, m_fGaussStdDev );
	}


	VALIDATE_D3D(m_lpEffect->SetFloatArray("g_HBlurOffset",m_HBlurOffset,sizeof(m_HBlurOffset) /  sizeof(m_HBlurOffset[0])));
	VALIDATE_D3D(m_lpEffect->SetFloatArray("g_BlurWeight",m_arrBlurWeight,sizeof(m_arrBlurWeight) / sizeof(m_arrBlurWeight[0])));

	unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name: PrepareHBlurData
//Desc:  prepare the vertical blur pass data ,see the code to detail
//--------------------------------------------------------------------------------------------------------------------
void CHDREff::PrepareVBlurData(void)
{
	guard;

	for(int i = 0 ; i < 9 ; i++){
     	
		m_VBlurOffset[i] = (static_cast< float >( i ) - 4.0f) * ( 1.0f / static_cast< float >( sm_iHeight ) );
        
	}

	VALIDATE_D3D(m_lpEffect->SetFloatArray("g_VBlurOffset",m_VBlurOffset,sizeof(m_VBlurOffset) /  sizeof(m_VBlurOffset[0])));

	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: PrepareFinalPassData
//Desc:  prepara the final pass data,see the code to detail
//--------------------------------------------------------------------------------------------------------------------
void CHDREff::PrepareFinalPassData(void)
{
	guard;

	VALIDATE_D3D(m_lpEffect->SetFloat("g_fGaussMultiplier",m_fGaussMultiplier));


	unguard;
}

