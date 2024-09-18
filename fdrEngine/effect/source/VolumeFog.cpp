//===================================================================
/** \file  
* Filename :   VolumeFog.cpp
* Desc     : Reference http://developer.nvidia.com/view.asp?IO=FogPolygonVolumes  
*/
//===================================================================
#include "stdafx.h"

# include "../../backPipe/include/osCamera.h"
# include  "../../mfpipe/include/osMesh.h"
# include "../../mfpipe/include/osCharacter.h"
# include "../../mfpipe/include/osStateBlock.h"
# include "../../effect/include/fdEleBolt.h"
#include "../../mfpipe/include/osShader.h"
#include "../../mfpipe/include/ostexture.h"
#include "../../backpipe/include/mainEngine.h"
#include "../include/VolumeFog.h"

CVolumeFog gVolumeFog;
CVolumeFog* gVolumeFogPtr = &gVolumeFog;

CVolumeFog::CVolumeFog(void)
{
	mpd3dDevice   = 0;
	mpOldRenderTarget	= 0;
	mpOldStencilZ		= 0;
	mpTargetZ			= 0;
	mpTempSurface		= 0;
	mpBackFogDist		= 0;
	mpFrontFogDist		= 0;
	mpSceneDist			= 0;
	mDepthEffect		= 0;
	mFrontFogEffect		= 0;
	mBackFogEffect		= 0;
	mScreenFogEffect	= 0;
	mpScreenVertBuf		= 0;
	mpFogDeclaration	= 0;

	m_pTexRedGreenRamp = 0;
	m_pTexBlueRamp     = 0;

	mbInited           = false;
//	mWaterDepthShaderId = 0;
//	mpWaterDepthTex = 0;
//	mWaterDepthTexId = 0;
}

CVolumeFog::~CVolumeFog(void)
{
}
void CVolumeFog::ReleaseVolumeFog()
{
	guard;

	SAFE_RELEASE(mDepthEffect);
	SAFE_RELEASE(mScreenFogEffect);

	SAFE_RELEASE(mpBackFogDist);
	SAFE_RELEASE(mpFrontFogDist);
	SAFE_RELEASE(mpSceneDist);
	SAFE_RELEASE(mpTargetZ);
	SAFE_RELEASE(mpScreenVertBuf);
	SAFE_RELEASE(mpFogDeclaration);

	SAFE_RELEASE(mpOldRenderTarget);
	SAFE_RELEASE(mpOldStencilZ);

	SAFE_RELEASE (mpScreenVertBuf);
	SAFE_RELEASE (mpFogDeclaration);
	SAFE_RELEASE (mpTempSurface);

	g_shaderMgr->release_shader(mFrontFogShaderId);
	g_shaderMgr->release_shader(mBackFogShaderId);

	g_shaderMgr->release_texture(m_TexRedGreenId);
	g_shaderMgr->release_texture(m_TexBlueId);
	g_shaderMgr->release_texture(m_TexBlueOneDimId);

//	g_shaderMgr->release_shader(mWaterDepthShaderId);
//	g_shaderMgr->release_texture(mWaterDepthTexId);
	//SAFE_RELEASE(mpd3dDevice);
	unguard;
}
void CVolumeFog::InitVolumeFog(I_deviceManager*    _middlePipe,IDirect3DDevice9* _d3dDevice)
{
	guard;
	if (!mbInited)
	{
		const D3DSURFACE_DESC* t_ptrDesc = _middlePipe->get_backBufDesc();
		mTargetWidth = t_ptrDesc->Width;
		mTargetHeight = t_ptrDesc->Height;
		mpd3dDevice = _d3dDevice;
		ComputeStepTexture();
		LoadShaders();
		mbInited = true;
	}
	unguard;
}
//创建雾查表纹理,1d纹理
HRESULT CVolumeFog::ComputeStepTexture()
{
	guard;

	SetVolumeSize(PVF_16_SURFACES_12_BIT_DEPTH);
	return S_OK;
	unguard;
}
HRESULT CVolumeFog::OnLostDevice()
{
	guard;
	if (!mbInited)
	{
		return S_OK;
	}
	if (mDepthEffect)
	{
		mDepthEffect->OnLostDevice();
	}
	
	if (mScreenFogEffect)
	{
		mScreenFogEffect->OnLostDevice();
	}
	

	SAFE_RELEASE(mpBackFogDist);
	SAFE_RELEASE(mpFrontFogDist);
	SAFE_RELEASE(mpSceneDist);
	SAFE_RELEASE(mpTargetZ);
	

	SAFE_RELEASE(mpOldRenderTarget);
	SAFE_RELEASE(mpOldStencilZ);
	
	/*osc_texture*  t_tex = g_shaderMgr->getTexMgr()->get_textureById( mWaterDepthTexId );
	if( t_tex )
		t_tex->release_texture();*/

	return 0;
	unguard;
}
HRESULT CVolumeFog::OnRestoreDevice(IDirect3DDevice9* _d3dDevice)
{
	guard;
	if (!mbInited)
	{
		return S_OK;
	}
	HRESULT hr;
	mpd3dDevice = _d3dDevice;
	if (mDepthEffect)
	{
		mDepthEffect->OnResetDevice();
	}
	if (mScreenFogEffect)
	{
		mScreenFogEffect->OnResetDevice();
	}
	
	hr = mpd3dDevice->CreateTexture( mTargetWidth, mTargetHeight, 0,
		D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &mpBackFogDist, NULL ) ;
	if(FAILED(hr)||!mpBackFogDist){
		g_bUseVolumeFog = false;
		return 0;
	}

	
	hr = mpd3dDevice->CreateTexture( mTargetWidth, mTargetHeight, 0,
		D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &mpFrontFogDist, NULL ) ;
	if(FAILED(hr)||!mpFrontFogDist){
		g_bUseVolumeFog = false;
		return 0;
	}



	hr = mpd3dDevice->CreateTexture( mTargetWidth, mTargetHeight, 0,
		D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &mpSceneDist, NULL ) ;
	if(FAILED(hr)||!mpSceneDist){
		g_bUseVolumeFog = false;
		return 0;
	}



	hr = mpd3dDevice->CreateDepthStencilSurface( mTargetWidth,mTargetHeight,
		D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &mpTargetZ, NULL );
	if(FAILED(hr)||!mpTargetZ){
		g_bUseVolumeFog = false;
		return 0;
	}



	hr = mpd3dDevice->GetRenderTarget( 0, &mpOldRenderTarget );
	if(FAILED(hr)||!mpOldRenderTarget){
		g_bUseVolumeFog = false;
		return 0;
	}



	hr = mpd3dDevice->GetDepthStencilSurface( &mpOldStencilZ );
	if(FAILED(hr)||!mpOldStencilZ){
		g_bUseVolumeFog = false;
		return 0;
	}
	
	mpTempSurface = NULL;
	return 0;
	unguard;
}

HRESULT CVolumeFog::LoadShaders()
{
	guard;
	
	mFrontFogShaderId = g_shaderMgr->add_shader( 
		"FogFrontFace", osc_engine::get_shaderFile( "object" ).c_str() );
	osassert(mFrontFogShaderId>0);
	mBackFogShaderId = g_shaderMgr->add_shader( 
		"FogBackFace", osc_engine::get_shaderFile( "object" ).c_str() );
	osassert(mBackFogShaderId>0);

	HRESULT hr;
	//DWORD flag = NULL;

	
	BYTE*  t_ptrStart;
	int    t_i;
	int    t_iGBufIdx = -1;
	t_ptrStart = START_USEGBUF( t_iGBufIdx );

	t_i = ::read_fileToBuf( "data\\effect\\depth.fx",t_ptrStart,TMP_BUFSIZE );
	LPD3DXBUFFER     t_errorBuf;
	hr = D3DXCreateEffect( mpd3dDevice, t_ptrStart,t_i,NULL,NULL,
		NULL/*D3DXSHADER_USE_LEGACY_D3DX9_31_DLL*/,NULL,&mDepthEffect,&t_errorBuf );		

	if( FAILED( hr ) )
	{
		//char*  t_str;
		//t_str = (char*)t_errorBuf->GetBufferPointer();
		//osassertex( false,t_str );
		g_bUseVolumeFog = false;
		END_USEGBUF( t_iGBufIdx );
		return 0;
	}
	END_USEGBUF( t_iGBufIdx );

	t_ptrStart = START_USEGBUF( t_iGBufIdx );

	t_i = ::read_fileToBuf( "data\\effect\\screenfog.fx",t_ptrStart,TMP_BUFSIZE );
	//LPD3DXBUFFER     t_errorBuf;
	hr = D3DXCreateEffect( mpd3dDevice, t_ptrStart,t_i,NULL,NULL,
		NULL/*D3DXSHADER_USE_LEGACY_D3DX9_31_DLL*/,NULL,&mScreenFogEffect,&t_errorBuf );		
	if( FAILED( hr ) )
	{
		//char*  t_str;
		//t_str = (char*)t_errorBuf->GetBufferPointer();
		//osassertex( false,t_str );
		g_bUseVolumeFog = false;
		END_USEGBUF( t_iGBufIdx );
		return 0;
	}
	END_USEGBUF( t_iGBufIdx );


	//hr = D3DXCreateEffectFromFile( mpd3dDevice, "effect/depth.fx", NULL, NULL, flag, NULL, &mDepthEffect, NULL );
	//osassert(SUCCEEDED(hr));
	//hr = D3DXCreateEffectFromFile( mpd3dDevice, "effect/screenfog.fx", NULL, NULL, flag, NULL, &mScreenFogEffect, NULL );
	//osassert(SUCCEEDED(hr));

	hr = mpd3dDevice->CreateTexture( mTargetWidth, mTargetHeight, 0,
	D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &mpBackFogDist, NULL ) ;
	if(FAILED(hr)||!mpBackFogDist){
		g_bUseVolumeFog = false;
		return 0;
	}
	

	hr = mpd3dDevice->CreateTexture( mTargetWidth, mTargetHeight, 0,
		D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &mpFrontFogDist, NULL ) ;
	if(FAILED(hr)||!mpFrontFogDist){
		g_bUseVolumeFog = false;
		return 0;
	}

	hr = mpd3dDevice->CreateTexture( mTargetWidth, mTargetHeight, 0,
		D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &mpSceneDist, NULL ) ;
	if(FAILED(hr)||!mpSceneDist){
		g_bUseVolumeFog = false;
		return 0;
	}

	hr = mpd3dDevice->CreateDepthStencilSurface( mTargetWidth,mTargetHeight,
		D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &mpTargetZ, NULL );
	if(FAILED(hr)||!mpTargetZ){
		g_bUseVolumeFog = false;
		return 0;
	}
	
	hr = mpd3dDevice->GetRenderTarget( 0, &mpOldRenderTarget );
	if(FAILED(hr)||!mpOldRenderTarget){
		g_bUseVolumeFog = false;
		return 0;
	}

	hr = mpd3dDevice->GetDepthStencilSurface( &mpOldStencilZ );
	if(FAILED(hr)||!mpOldStencilZ){
		g_bUseVolumeFog = false;
		return 0;
	}

	mpTempSurface = NULL;

	hr = mpd3dDevice->CreateVertexBuffer( 7 * sizeof(FLOAT) * 4, 0,
		D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE, 
		D3DPOOL_MANAGED, &mpScreenVertBuf, NULL );
	if(FAILED(hr)){
		g_bUseVolumeFog = false;
		return 0;
	}

	static struct { FLOAT x,y,z,w; DWORD color; FLOAT u,v; } s_Verts[] =
	{
		{1.0f,   1.0f, 0.5f, 1.0f , D3DCOLOR_XRGB( 0xff, 0xff, 0x8f ), 1,0 },
		{1.0f, -1.0f, 0.5f, 1.0f, D3DCOLOR_XRGB( 0xff, 0xff, 0x8f),  1,1},
		{ -1.0f, 1.0f, 0.5f, 1.0f,   D3DCOLOR_XRGB( 0xff, 0xff, 0x8f),  0,0},
		{ -1.0f,  -1.0f, 0.5f, 1.0f,  D3DCOLOR_XRGB( 0xff, 0xff, 0x8f),  0,1},
	};
	VOID* pVerts;
	hr = mpScreenVertBuf->Lock( 0, sizeof(s_Verts), (void**)(&pVerts), 0 );
	if(FAILED(hr)){
		g_bUseVolumeFog = false;
		return 0;
	}

	memcpy( (void*)pVerts, (void*)s_Verts, sizeof(s_Verts) );

	hr = mpScreenVertBuf->Unlock();
	if(FAILED(hr)){
		g_bUseVolumeFog = false;
		return 0;
	}

	D3DVERTEXELEMENT9 decl2[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 20, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	hr = mpd3dDevice->CreateVertexDeclaration( decl2, &mpFogDeclaration ) ;
	if(FAILED(hr)){
		g_bUseVolumeFog = false;
		return 0;
	}
	return S_OK;
	unguard;
}
//@{
void CVolumeFog::PreRenderSceneDist()
{
	guard;
	mDepthEffect->SetTexture( "Texture0", m_pTexRedGreenRamp);
	mDepthEffect->SetTexture( "Texture1", m_pTexBlueRamp);

	SAFE_RELEASE(mpTempSurface);
	
	
	mpSceneDist->GetSurfaceLevel(0,&mpTempSurface);
	HRESULT hr = mpd3dDevice->SetRenderTarget(0,mpTempSurface);
	hr = mpd3dDevice->SetDepthStencilSurface(mpTargetZ);
	SAFE_RELEASE(mpTempSurface);
	hr = mpd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_ARGB( 0x00, 0x00, 0x00, 0x00), 1.0,0);
	unguard;
}
void CVolumeFog::PreRenderBackFog()
{
	guard;
	HRESULT hr ;
	SAFE_RELEASE(mpTempSurface);
	
	hr = mpBackFogDist->GetSurfaceLevel(0,&mpTempSurface);
	hr = mpd3dDevice->SetRenderTarget(0,mpTempSurface);
	SAFE_RELEASE(mpTempSurface);
	hr = mpd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET,D3DCOLOR_ARGB( 0x00, 0x00, 0x00, 0x00), 1.0,0);
	unguard;
}
void CVolumeFog::PreRenderFrontFog()
{
	guard;
	HRESULT hr ;
	SAFE_RELEASE(mpTempSurface);
	
	hr = mpFrontFogDist->GetSurfaceLevel(0,&mpTempSurface);
	hr = mpd3dDevice->SetRenderTarget(0,mpTempSurface);
	SAFE_RELEASE(mpTempSurface);
	hr = mpd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET,D3DCOLOR_ARGB( 0x00, 0x00, 0x00, 0x00), 1.0,0);
	unguard;
}
//void CVolumeFog::PreRenderWaterDepth()
//{
//
//	guard;
//	HRESULT hr ;
//	
//	hr = mpWaterDepthTex->GetSurfaceLevel(0,&mpTempSurface);
//	hr = mpd3dDevice->SetRenderTarget(0,mpTempSurface);
//	hr = mpd3dDevice->SetDepthStencilSurface(mpTargetZ);
//	
//	hr = mpd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_ARGB( 0x00, 0x00, 0x00, 0x00), 1.0,0);
//	unguard;
//}
//void CVolumeFog::RestoreRenderTarget()
//{
//	guard;
//
//	/*IDirect3DSurface9* pSurface;
//	mpWaterDepthTex->GetSurfaceLevel(0,&pSurface);
//
//	D3DXSaveSurfaceToFileA("WaterDepth_A.TGA",D3DXIFF_TGA,pSurface,NULL,NULL);
//	pSurface->Release();*/
//
//	SAFE_RELEASE(mpTempSurface);
//
//	osc_TGManager*    t_ptrScene = (osc_TGManager*)::get_sceneMgr();
//	if( !t_ptrScene->m_growEff.IsEffective() )
//		mpd3dDevice->SetRenderTarget(0, mpOldRenderTarget);
//	else
//		t_ptrScene->m_growEff.reset_renderTarget( mpd3dDevice );
//
//	mpd3dDevice->SetDepthStencilSurface( mpOldStencilZ );
//	unguard;
//}
void CVolumeFog::DrawSceneFog()
{
	guard;
	HRESULT hr;

	SAFE_RELEASE(mpTempSurface);

	osc_TGManager*    t_ptrScene = (osc_TGManager*)::get_sceneMgr();
	if( !t_ptrScene->m_growEff.IsEffective() )
		g_frender->get_d3dDevice()->SetRenderTarget(0, mpOldRenderTarget);
	else
		t_ptrScene->m_growEff.reset_renderTarget( g_frender->get_d3dDevice() );

	//mpd3dDevice->SetRenderTarget( 0, mpOldRenderTarget );
	mpd3dDevice->SetDepthStencilSurface( mpOldStencilZ );
	extern BOOL gHaveVolumeFog ;

	if (gHaveVolumeFog)
	{
		mScreenFogEffect->SetTexture( "Texture0", mpFrontFogDist);
		mScreenFogEffect->SetTexture( "Texture1", mpBackFogDist);
		mScreenFogEffect->SetTexture( "Texture2", mpSceneDist);

		osVec4D DeRamp = osVec4D(	1.0f, 
			1.0f / m_fGrnRamp,
			1.0f / m_fBluRamp,
			-1.0f / m_fGrnRamp );

		//!体积雾浓度
		DeRamp*=600;
		mScreenFogEffect->SetVector("DeRampScale",&DeRamp);

		mpd3dDevice->SetVertexDeclaration( mpFogDeclaration );
		mpd3dDevice->SetStreamSource( 0, mpScreenVertBuf, 0, 7 * sizeof(FLOAT) );

		UINT iPass, cPasses;
		mScreenFogEffect->Begin(&cPasses, 0) ;
		for (iPass = 0; iPass < cPasses; iPass++)
		{
			mScreenFogEffect->BeginPass(iPass) ;


			hr = mpd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
			mScreenFogEffect->EndPass() ;
		}
		mScreenFogEffect->End() ;
	}
	mpd3dDevice->SetVertexShader(NULL);
	mpd3dDevice->SetPixelShader(NULL);
	mpd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
	mpd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
	

	//IDirect3DSurface9* pSurface;
	//mpOldRenderTarget->GetSurfaceLevel(0,&pSurface);
	//mpd3dDevice->StretchRect(
	//	mpOldRenderTarget, 0, pSurface, 0, D3DTEXF_NONE);


	//D3DXSaveSurfaceToFileA("FogScene.TGA",D3DXIFF_TGA,pSurface,NULL,NULL);
	//pSurface->Release();	
	unguard;
}
//@}

//@{ For Debug
void CVolumeFog::DumpSceneDist()
{
	IDirect3DSurface9* pSurface;
	mpSceneDist->GetSurfaceLevel(0,&pSurface);

	D3DXSaveSurfaceToFileA("SceneDist.TGA",D3DXIFF_TGA,pSurface,NULL,NULL);
	pSurface->Release();	
}
void CVolumeFog::DumpFrontFogDist()
{
	IDirect3DSurface9* pSurface;
	mpFrontFogDist->GetSurfaceLevel(0,&pSurface);

	D3DXSaveSurfaceToFileA("FrontFog.TGA",D3DXIFF_TGA,pSurface,NULL,NULL);
	pSurface->Release();
}
void CVolumeFog::DumpBackFogDist()
{
	IDirect3DSurface9* pSurface;
	mpBackFogDist->GetSurfaceLevel(0,&pSurface);

	D3DXSaveSurfaceToFileA("BackFog.TGA",D3DXIFF_TGA,pSurface,NULL,NULL);
	pSurface->Release();
}
//@}
int g_TexRedGreenId;
int g_TexBlueId;
int g_TexBlueOneDimId;
HRESULT CVolumeFog::CreateGradient2D( IDirect3DTexture9 ** ppTex2D,
				 IDirect3DDevice9 * pD3DDev,
				 int width, int height, byte lower, byte upper,
				 D3DXVECTOR3 col_mask_u, D3DXVECTOR3 col_mask_v )
{
	guard;

	osassert( pD3DDev );
	osassert( ppTex2D  );
	HRESULT hr;
	SAFE_RELEASE( *ppTex2D );

	m_TexRedGreenId = g_shaderMgr->getTexMgr()->addnew_texture("g_FogRedGreen.texture",
		width,height,D3DFMT_A8R8G8B8,0);
	g_TexRedGreenId = m_TexRedGreenId;
	osassert(m_TexRedGreenId!=-1);

	m_pTexRedGreenRamp = (LPDIRECT3DTEXTURE9)g_shaderMgr->getTexMgr()->get_texfromid(m_TexRedGreenId);

	*ppTex2D = m_pTexRedGreenRamp;
	osassert( *ppTex2D );

	// Now lock and fill the texture with the gradient
	IDirect3DTexture9 * pTex = *ppTex2D;
	D3DLOCKED_RECT lr;
	hr = pTex->LockRect( 0, &lr,
		NULL,		// region to lock - NULL locks whole thing
		0 );		// No special lock flags
	// Can't use LOCK_DISCARD unless texture is DYNAMIC
	osassert( SUCCEEDED(hr) );

	byte * pixel;
	byte * pBase = (BYTE*) lr.pBits;
	int bytes_per_pixel = 4;
	osassert( bytes_per_pixel * width == lr.Pitch );

	int colu;
	int colv;
	int i,j;
	D3DXVECTOR3 sum;

	for( j=0; j < height; j++ )
	{
		for( i=0; i < width; i++ )
		{
			pixel = pBase + j * lr.Pitch + i * bytes_per_pixel;


			colu = (int)( lower + (upper+1-lower) * ((float)i / ((float)width)));
			colv = (int)( lower + (upper+1-lower) * ((float)j / ((float)height)));

			// calculate gradient values
			/*colu = (byte)( lower + (upper-lower) * (float)i / ((float)width - 1.0f));
			colv = (byte)( lower + (upper-lower) * (float)j / ((float)height - 1.0f));*/
			sum = (float)colu * col_mask_u + (float)colv * col_mask_v;

			// alpha should be 255 because it is used later in alpha test
			*((D3DCOLOR*)pixel) = D3DCOLOR_ARGB( 0, (BYTE)( sum.x ), (BYTE)( sum.y ), (BYTE)( sum.z ) );
		}	
	}

	hr = pTex->UnlockRect( 0 );
	osassert( SUCCEEDED(hr) );
	//FDebug("Created 2D gradient texture!\n");

	// Option to save the texture to a file
#if 0
	std::string filename;
	filename = "RG_0_15.tga";
	//FMsg("Texture filename [%s]\n", filename.c_str() );

	// D3DXIFF_BMP can't be parsed by Photoshop
	//  but if you re-save it in another app it will be ok for Photoshop
	hr = D3DXSaveTextureToFile( filename.c_str(),
		D3DXIFF_TGA,
		*ppTex2D,
		NULL	);		// palette 
	osassert( SUCCEEDED(hr) );
#endif
	return S_OK;
	unguard;

}
HRESULT CVolumeFog::CreateBlueOneDimTex()
{
	guard;
	bool bSaveTexture = false;

	HRESULT hr;
	int width = 256;
	m_TexBlueOneDimId = g_shaderMgr->getTexMgr()->addnew_texture("g_BlueOneDim.texture",
		width,1,D3DFMT_A8R8G8B8,0);
	osassert(m_TexBlueOneDimId!=-1);
	g_TexBlueOneDimId = m_TexBlueOneDimId;

	m_pTexBlueOneDim = (LPDIRECT3DTEXTURE9)g_shaderMgr->getTexMgr()->get_texfromid(m_TexBlueOneDimId);

	//*ppTex = m_pTexBlueOneDim;
	//osassert( *ppTex );
	

	// Now lock and fill the texture with the gradient
	IDirect3DTexture9 * pTex = m_pTexBlueOneDim;
	D3DLOCKED_RECT lr;
	hr = pTex->LockRect( 0, &lr,
		NULL,		// region to lock - NULL locks whole thing
		0 );		// No special lock flags
	// Can't use LOCK_DISCARD unless texture is DYNAMIC
	osassert( SUCCEEDED(hr) );

	byte * pixel;
	byte * pBase = (BYTE*) lr.pBits;

	int bytes_per_pixel = 4;

	osassert( bytes_per_pixel * width == lr.Pitch );

	int val;
	//	int dithered_val;
	DWORD color;
	int i;

	
	byte lower =0;
	byte upper = 255;
	D3DXVECTOR3 color_mask(0,0,1);
	for( i=0; i < width; i++ )
	{
		pixel = pBase + i * bytes_per_pixel;

		// calculate gradient values
		val = (int)( lower + (upper+1-lower) * ((float)i / ((float)width)));
	
		color = D3DCOLOR_ARGB( 0,
			(BYTE)( val * color_mask.x),
			(BYTE)( val * color_mask.y),
			(BYTE)( val * color_mask.z) );

		*((D3DCOLOR*)pixel) = color;
	}	
	
	hr = pTex->UnlockRect( 0 );
	osassert( SUCCEEDED(hr) );

	if( bSaveTexture == true )
	{
		TCHAR filename[200];
		_stprintf( filename, TEXT("BlueOneDim.TGA"));
		//FMsg("Texture filename [%s]\n", filename );

		// D3DXIFF_TGA is not yet supported
		// D3DXIFF_BMP can't be parsed by Photoshop
		//   but if you open in another app and re-save then it is ok.
		// D3DXIFF_PPM is not supported
		// PNG is not supported
		hr = D3DXSaveTextureToFile( filename,
			D3DXIFF_TGA,
			m_pTexBlueOneDim,
			NULL	);		// palette 
		osassert( SUCCEEDED(hr) );
	}
	return true;
	unguard;

}
void CVolumeFog::CreateGradientDithered( IDirect3DTexture9 ** ppTex,
														   IDirect3DDevice9 * pD3DDev,
														   int width, int height,
														   byte lower, byte upper,
														   D3DXVECTOR3 color_mask,
														   const D3DCOLOR * pDither_upper )
{	
	guard;
	osassert( pD3DDev );

	// width, height are resolution of texture
	// lower, upper are values for the limits of the gradient
	// color_mask is applied to byte values to generate the color
	// pDither_upper can point to a color value.  If not NULL, then the 
	//    upper texels (rightmost) are dithered to the color pointed to.

	// Set true to save the resulting texture to a file for inspection
	bool bSaveTexture = false;


	// Texture dither should continue at the top and bottom to the 
	//  next highest and lowest values, respectively, but this would
	//  require storing and blending a negative green value at the
	//  lowest dither points.  That's impossible, but we can store a
	//  positive value in the alpha channel and treat it as a negative
	//  green value in pixel shaders 2.0.
	// We can't treat the alpha channel as negative green value with
	//  pixel shaders 1.3 in the depth comparison shader because we
	//  run out of instructions.
	// Dither the lower bits only if using ps_2_0.

	bool bDitherLowerToBlueAndAlpha = true;

	HRESULT hr = S_OK;

	// Creates a gradient in X axis of the texture
	// Color mask is from [0,1] and gets multiplied into
	//  the color components

	osassert( ppTex );
	osassert( pD3DDev );	
	SAFE_RELEASE( *ppTex );

	// Generating mip maps for this low bits texture does not
	// improve performance, but increasing or decreasing the
	// texture coordinate scale does affect performance.  As the
	// texture coordinate dither scale increases, performance 
	// decreases.  This is due to the degree of coherence of
	// samples taken from the texture and the texture cache
	// utilization.

	m_TexBlueId = g_shaderMgr->getTexMgr()->addnew_texture("g_FogBlue.texture",
		width,height,D3DFMT_A8R8G8B8,0);
	osassert(m_TexBlueId!=-1);
	g_TexBlueId = m_TexBlueId;

	m_pTexBlueRamp = (LPDIRECT3DTEXTURE9)g_shaderMgr->getTexMgr()->get_texfromid(m_TexBlueId);

	*ppTex = m_pTexBlueRamp;
	osassert( *ppTex );

	// Now lock and fill the texture with the gradient
	IDirect3DTexture9 * pTex = *ppTex;
	D3DLOCKED_RECT lr;
	hr = pTex->LockRect( 0, &lr,
		NULL,		// region to lock - NULL locks whole thing
		0 );		// No special lock flags
	// Can't use LOCK_DISCARD unless texture is DYNAMIC
	osassert( SUCCEEDED(hr) );

	byte * pixel;
	byte * pBase = (BYTE*) lr.pBits;

	int bytes_per_pixel = 4;

	osassert( bytes_per_pixel * width == lr.Pitch );

	int val;
//	int dithered_val;
	DWORD color;
//	int noiseinc;
//	float fcol;
//	float frand;
	int i,j;

    for( j=0; j < height; j++ )
	{
		for( i=0; i < width; i++ )
		{
			pixel = pBase + j * lr.Pitch + i * bytes_per_pixel;

			// calculate gradient values
			val = (int)( lower + (upper+1-lower) * ((float)i / ((float)width)));
			//fcol = lower + (upper+1-lower) * ((float)i / ((float)width));

			//fcol = (float) fmod( fcol, 1.0f );
			//frand = ((float)rand()) / ((float)RAND_MAX);
			//if( fcol < 0.5f )
			//{
			//	fcol = 0.5f - fcol;
			//	if( frand < fcol )
			//		noiseinc = -1;
			//	else
			//		noiseinc = 0;
			//}
			//else
			//{
			//	fcol = fcol - 0.5f;			// 0 at middle of color band
			//	// 0.5 at right edge of color band
			//	if( frand > fcol )
			//		noiseinc = 0;
			//	else
			//		noiseinc = 1;
			//}

			//dithered_val = val + noiseinc;
			//if( dithered_val < 0 )
			//{
			//	if( bDitherLowerToBlueAndAlpha )
			//	{
			//		// Dither bottom into blue and alpha
			//		// Alpha will become negative green increment

			//		color = D3DCOLOR_ARGB( 1, 
			//			(BYTE)( upper * color_mask.x ),
			//			(BYTE)( upper * color_mask.y ),
			//			(BYTE)( upper * color_mask.z ) );
			//	}
			//	else
			//	{
			//		color = 0x00;
			//	}
			//}
			//else if( dithered_val > upper )
			//{
			//	if( pDither_upper != NULL )
			//	{
			//		color = *pDither_upper;
			//	}
			//	else
			//	{
			//		// color = maximum value
			//		dithered_val = dithered_val - 1;
			//		color = D3DCOLOR_ARGB( 0, 
			//			(BYTE)( dithered_val * color_mask.x),
			//			(BYTE)( dithered_val * color_mask.y),
			//			(BYTE)( dithered_val * color_mask.z) );
			//	}
			//}
			//else
			//{
				// standard dithered value
				/*color = D3DCOLOR_ARGB( 0, 
					(BYTE)( dithered_val * color_mask.x),
					(BYTE)( dithered_val * color_mask.y),
					(BYTE)( dithered_val * color_mask.z) );*/
			//}

			// If at the top line of the texture, do not dither, because
			//  dither control of (0,0,0,0) accesses the top line
			// This is only so the demo can illustrate the difference between
			//  dither and no dither.  If you always dither, you can remove this
			//  j==0 case.
		/*	if( j == 0 )
			{*/
			color = D3DCOLOR_ARGB( 0,
				(BYTE)( val * color_mask.x),
				(BYTE)( val * color_mask.y),
				(BYTE)( val * color_mask.z) );
		//	}


			*((D3DCOLOR*)pixel) = color;
		}	
	}

	hr = pTex->UnlockRect( 0 );
	osassert( SUCCEEDED(hr) );

	// mip mapping the blue bits texture doesn't improve perf
	//	pTex->SetAutoGenFilterType( D3DTEXF_POINT );
	//	pTex->GenerateMipSubLevels();

	//FDebug("Created dithered gradient texture.  Resolution: %d, %d\n", width, height );

	if( bSaveTexture == true )
	{
		TCHAR filename[200];
		_stprintf( filename, TEXT("PVFDither_%d_%d%s.TGA"), width, height, pDither_upper ? TEXT("_WrapDither") : TEXT("") );
		//FMsg("Texture filename [%s]\n", filename );

		// D3DXIFF_TGA is not yet supported
		// D3DXIFF_BMP can't be parsed by Photoshop
		//   but if you open in another app and re-save then it is ok.
		// D3DXIFF_PPM is not supported
		// PNG is not supported
		hr = D3DXSaveTextureToFile( filename,
			D3DXIFF_TGA,
			*ppTex,
			NULL	);		// palette 
		osassert( SUCCEEDED(hr) );
	}
	unguard;
}

HRESULT	CVolumeFog::CreateRampTextures( IDirect3DDevice9 * pD3DDev )
{
	guard;
	HRESULT hr = S_OK;

	int size_divide = (int)( 1.0f / m_fBitReduce );

	// Create texture gradients used to encode depth as an RGB color.
	// Blue is a single gradient in a single texture.
	// Red and green gradients are held in one 2D texture with 
	//   each gradient going in one axis (green in X, red in Y )
	//
	// This class will generate it's own gradient textures, and
	//  the blue texture, which encodes the least significant bits
	//  of depth, are dithered to eliminate aliasing artifacts
	// There is a vertex shader 'dither control' variable to control
	//  the texture coordinates fetching from the dithered texture.

	// extra width for dithered values
	int width = 8 * 256 / size_divide;

	if( width > 1024 )
		width = 1024;


	// 2D gradient in red and green
	// No dithering
	CreateGradient2D( & m_pTexRedGreenRamp,
		pD3DDev,
		width, width,
		0, (byte)( 255.0f * m_fBitReduce ),
		D3DXVECTOR3( 1.0f, 0.0f, 0.0f ),	// color mask u axis
		D3DXVECTOR3( 0.0f, 1.0f, 0.0f )  );	// color mask v axis RGB


	D3DCOLOR upper_color;
	upper_color = D3DCOLOR_ARGB( 0, 0, 1, 0 );	 //  1/255 of green

	

	// 64 in y for dithered values.

	CreateGradientDithered( & m_pTexBlueRamp,
		pD3DDev,
		width, 1,
		0, (byte)( 255.0f * m_fBitReduce ),
		D3DXVECTOR3( 0.0f, 0.0f, 1.0f ),	// blue
		& upper_color );

//	m_ppTexRedGreenRamp = &m_pTexRedGreenRamp;
//	m_ppTexBlueRamp		= &m_pTexBlueRamp;

	CreateBlueOneDimTex();


	return( hr );
	unguard;
}
HRESULT CVolumeFog::FreeRampTextures()
{
	guard;
	HRESULT hr = S_OK;
	SAFE_RELEASE( m_pTexRedGreenRamp );
	SAFE_RELEASE( m_pTexBlueRamp );
	return( hr );
	unguard;
}

void CVolumeFog::SetVolumeSize(MaxFogDepthComplexity _depth_relation)
{
	guard;
	m_MaxFogDepthComplexity = _depth_relation;
	float base_scale = 10.0f;
	float diff = 2.0f;

	switch( m_MaxFogDepthComplexity )
	{
	case PVF_1_SURFACES_24_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_256;
		m_DepthLowBitSize = 8;
//		m_fTexCrdPrecisionFactor = base_scale / (diff*diff*diff*diff);
		break;
	case PVF_2_SURFACES_21_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_128;
		m_DepthLowBitSize = 7;
//		m_fTexCrdPrecisionFactor = base_scale / (diff*diff*diff);
		break;
	case PVF_4_SURFACES_18_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_64;
		m_DepthLowBitSize = 6;
//		m_fTexCrdPrecisionFactor = base_scale / (diff*diff);
		break;
	case PVF_8_SURFACES_15_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_32;
		m_DepthLowBitSize = 5;
//		m_fTexCrdPrecisionFactor = base_scale / diff;
		break;
	case PVF_16_SURFACES_12_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_16;
		m_DepthLowBitSize = 4;
//		m_fTexCrdPrecisionFactor = base_scale;
		break;
	case PVF_32_SURFACES_9_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_8;
		m_DepthLowBitSize = 3;
//		m_fTexCrdPrecisionFactor = base_scale * diff;
		break;
	case PVF_64_SURFACES_6_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_4;
		m_DepthLowBitSize = 2;
//		m_fTexCrdPrecisionFactor = base_scale * diff * diff;
		break;
	case PVF_128_SURFACES_3_BIT_DEPTH :
		m_DepthValuesPerColorChannel = PVF_2;
		m_DepthLowBitSize = 1;
	//	m_fTexCrdPrecisionFactor = base_scale * diff * diff * diff;
		break;

	default :
		//FMsg("Unrecognized MaxFogDepthComplexity\n");
		osassert( false );
		return;
		break;
	}

	float little_bands;
	switch( m_DepthValuesPerColorChannel )
	{
	case PVF_256 :
		m_fBitReduce = 1.0f;
		little_bands = 256.0f;
		break;
	case PVF_128 :
		m_fBitReduce = 0.5f;
		little_bands = 128.0f;
		break;
	case PVF_64 :
		m_fBitReduce = 0.25f;
		little_bands = 64.0f;
		break;
	case PVF_32 :
		m_fBitReduce = 0.125f;
		little_bands = 32.0f;
		break;
	case PVF_16 :
		m_fBitReduce = 0.0625f;
		little_bands = 16.0f;
		break;
	case PVF_8 :
		m_fBitReduce = 0.03125f;
		little_bands = 8.0f;
		break;
	case PVF_4 :
		m_fBitReduce = 0.015625;
		little_bands = 4.0f;
		break;
	case PVF_2 :
		m_fBitReduce = 0.0078125;
		little_bands = 2.0f;
		break;

	default:
		//FDebug("unknown # depth values per channel -- using 16 as default\n");
		m_fBitReduce = 0.0625f;
		little_bands = 16.0f;
		assert( false );
	}

	m_fGrnRamp = m_fBitReduce * 256.0f;		// how many times green cycles for 1 red cycle
	m_fBluRamp = m_fGrnRamp * m_fGrnRamp;
	m_fGrnRamp = little_bands;
	m_fBluRamp = little_bands * little_bands;

	//SetThicknessToColorTexCoordScale( fThicknessToColorScale );

	CreateRampTextures( mpd3dDevice );
	unguard;
}