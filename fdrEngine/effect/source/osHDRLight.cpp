# include "stdafx.h"
#include "../../effect/include/osHDRLight.h"
#include "../../terrain/include/fdFieldStruct.h"

// #define DEBUG_VS   // Uncomment this line to debug vertex shaders 
// #define DEBUG_PS   // Uncomment this line to debug pixel shaders 

#define PI					3.141592653589793238462643383279502884197169399375105820974944592f
#define HDRFX_FILENAME      "data\\effect\\hdr.fx"

bool osHDRLight::m_bSupportHDRLight = true;


struct CUSTOMVERTEX
{
	FLOAT       x;
	FLOAT       y;
};

D3DVERTEXELEMENT9 decl[] =
{
	{0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
	{0, 24, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0},
	{0, 36, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  0},
	{0, 48, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 },
	D3DDECL_END()
};

D3DVERTEXELEMENT9 decl2[]=
{
	// stream, offset, type, method, semantic type (for example normal), ?
	{0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	D3DDECL_END()
};

osHDRLight::osHDRLight(void) : 
m_pd3dDevice(0),
m_pRenderTarget(0),
m_pRenderTarget2(0),
m_pRenderTarget3(0),
m_pBackBuffer(0),
m_pZBuffer(0),
m_pSurface(0),
m_pSurface2(0),
m_pSurface3(0),
m_pSurface4(0),
m_pImageVB(0),
m_pDecl(0),
m_pDecl2(0),
m_pEffect(0),
m_hTechnique(0),
m_hmWorldViewProj(0),
m_hmWorld(0),
m_hvMaterialColor(0),
m_hvLightPos(0),
m_hvCamPos(0),
m_hScreen(0),
m_hMap(0),
m_hFullResMap(0),
m_hScaleBuffer(0),
m_hGaussX(0),
m_hGaussY(0),
m_hIteration(0),
m_hPixelSize(0),
m_hmx(0),
m_hmy(0),
m_hA(0),
m_hkd(0),
m_hExposure(0)
{
	m_LightPos = D3DXVECTOR4( 23.0f, 23.0f, -23.0f, 0.0f );
	m_fmx = 200.0f;
	m_fmy = 200.0f;
	m_fDiffuse =  3.47f;
	m_fExposureLevel = 1.99f;
	ZeroMemory( &m_backBufferSurfaceDesc, sizeof(D3DSURFACE_DESC) );
}

osHDRLight::~osHDRLight(void)
{
	SAFE_RELEASE( m_pEffect );
	SAFE_RELEASE( m_pDecl );
	SAFE_RELEASE( m_pDecl2 );

	SAFE_RELEASE( m_pSurface ); 
	SAFE_RELEASE( m_pSurface2 ); 
	SAFE_RELEASE( m_pSurface3 ); 
	SAFE_RELEASE( m_pBackBuffer );
	SAFE_RELEASE( m_pZBuffer );

	SAFE_RELEASE( m_pImageVB );
	SAFE_RELEASE( m_pRenderTarget );
	SAFE_RELEASE( m_pRenderTarget2 );
	SAFE_RELEASE( m_pRenderTarget3 );
}

HRESULT osHDRLight::init_hdrLight()
{
	I_deviceManager* pDevice = get_deviceManagerPtr();
	osassert( pDevice );
	LPDIRECT3DDEVICE9 pd3dDevice = pDevice->get_d3ddevice();
	osassert( pd3dDevice );
	m_pd3dDevice = pd3dDevice;

	D3DCAPS9 d3dCaps;
	m_pd3dDevice->GetDeviceCaps( &d3dCaps );

	// 如果全局关掉Hdr.
	if( !g_bUseHDRLight )
	{
		m_bSupportHDRLight = false;
		return E_FAIL;
	}
    
	// 检测硬件性能
	if( d3dCaps.PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
	{
		m_bSupportHDRLight = false;
		osDebugOut( "ps shader:MAJOR:%d, MINOR:%d, doesn`t support HDR Light...\n", D3DSHADER_VERSION_MAJOR( d3dCaps.PixelShaderVersion ),
			D3DSHADER_VERSION_MINOR( d3dCaps.PixelShaderVersion ) );
        return E_FAIL;
	}
    if( d3dCaps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
    {
		osDebugOut( "vs shader:MAJOR:%d, MINOR:%d, doesn`t support HDR Light...\n", D3DSHADER_VERSION_MAJOR( d3dCaps.VertexShaderVersion ),
			D3DSHADER_VERSION_MINOR( d3dCaps.VertexShaderVersion ) );
		m_bSupportHDRLight = false;
        return E_FAIL;
    }

	HRESULT hr;
	DWORD dwShaderFlags = 0;

#ifdef DEBUG_VS
	dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
#endif

#ifdef DEBUG_PS
	dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
#endif

	// Declare the vertex structure
	pd3dDevice->CreateVertexDeclaration( decl, &m_pDecl );
	pd3dDevice->CreateVertexDeclaration( decl2, &m_pDecl2 );

	if( g_bShaderDebug )
	{
		// Compile effect file
		hr = D3DXCreateEffectFromFile( pd3dDevice,
										HDRFX_FILENAME,
										NULL,
										NULL,
										dwShaderFlags, 
										NULL,
										&m_pEffect,
										NULL
										);
		if( FAILED(hr) )
		{
			char temp[250];
			sprintf(temp,"Could not D3DXCreateEffectFromFile hdr.fx");
			osassertex(false,temp);
			//MessageBox( NULL, "Could not D3DXCreateEffectFromFile hdr.fx", "hdr.fx", MB_OK );
		}
	}
	else
	{
		int   t_iGBufIdx = -1;
		BYTE*  pStart = START_USEGBUF( t_iGBufIdx );
		int size = read_fileToBuf( HDRFX_FILENAME, pStart, TMP_BUFSIZE );
		if( size <= 0 )
		{
			END_USEGBUF( t_iGBufIdx );
			osassertex( false, va("调入Effect文件出错<%s>..\n", HDRFX_FILENAME ) );
			return E_FAIL;
		}
		//  Create Effect from temp buffer.
		ID3DXBuffer* pErrorBuffer = NULL;
		hr = D3DXCreateEffect(  pd3dDevice, pStart, size,
			                 NULL, NULL, 0, NULL, &m_pEffect, &pErrorBuffer );
		if( FAILED( hr ) )
		{
			END_USEGBUF( t_iGBufIdx );
			osDebugOut( "HDR ERROR:%s\n", (char*)( pErrorBuffer->GetBufferPointer() ) );
			osassertex( false,va( "创建HDR effect出错<%s>..\n",osn_mathFunc::get_errorStr( hr ) ) );
			return false;
		}	
		osassert( m_pEffect );
		
		END_USEGBUF( t_iGBufIdx );

	}

	// get the handles to provide data to the HLSL shader
	m_hTechnique = m_pEffect->GetTechniqueByName("Ashikhmin");
	m_hScaleBuffer = m_pEffect->GetTechniqueByName("ScaleBuffer");
	m_hGaussX = m_pEffect->GetTechniqueByName("GaussX");
	m_hGaussY = m_pEffect->GetTechniqueByName("GaussY");
	m_hScreen = m_pEffect->GetTechniqueByName("Screenblit");
	m_hmWorldViewProj = m_pEffect->GetParameterByName(NULL, "matWorldViewProj");
	m_hvMaterialColor = m_pEffect->GetParameterByName(NULL, "MaterialColor");
	m_hvLightPos = m_pEffect->GetParameterByName(NULL, "vLightPos");
	m_hvCamPos = m_pEffect->GetParameterByName(NULL, "vCamPos");
	m_hmWorld = m_pEffect->GetParameterByName(NULL, "matWorld");
	m_hMap = m_pEffect->GetParameterByName(NULL, "RenderMap");
	m_hFullResMap = m_pEffect->GetParameterByName(NULL, "FullResMap");
	m_hIteration = m_pEffect->GetParameterByName(NULL, "fIteration");
	m_hPixelSize = m_pEffect->GetParameterByName(NULL, "pixelSize");

	m_hmx = m_pEffect->GetParameterByName(NULL, "mx");
	m_hmy = m_pEffect->GetParameterByName(NULL, "my");
	m_hA = m_pEffect->GetParameterByName(NULL, "A");
	m_hkd = m_pEffect->GetParameterByName(NULL, "kd");
	m_hExposure = m_pEffect->GetParameterByName(NULL, "ExposureLevel");

    // Store render target surface desc
    IDirect3DSurface9* pBackBuffer;
    hr = pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    if( SUCCEEDED( hr ) )
    {
        pBackBuffer->GetDesc( &m_backBufferSurfaceDesc );
        SAFE_RELEASE( pBackBuffer );
    }
 
	// Create texture as render target
	hr = pd3dDevice->CreateTexture(    m_backBufferSurfaceDesc.Width, 
		                               m_backBufferSurfaceDesc.Height,
		                               1, 
		                               D3DUSAGE_RENDERTARGET, 
		                               D3DFMT_A8R8G8B8,
		                               D3DPOOL_DEFAULT, 
		                               &m_pRenderTarget,
		                               NULL
		                             ); 
	m_pRenderTarget->GetSurfaceLevel( 0, &m_pSurface );

	// Create second texture as render target
	hr = pd3dDevice->CreateTexture(  m_backBufferSurfaceDesc.Width / 2, 
		                               m_backBufferSurfaceDesc.Height / 2,
		                               1, 
		                               D3DUSAGE_RENDERTARGET, 
		                               D3DFMT_A8R8G8B8,
		                               D3DPOOL_DEFAULT, 
		                               &m_pRenderTarget2,
		                               NULL
		                              );
	m_pRenderTarget2->GetSurfaceLevel( 0, &m_pSurface2 );

	// Create third texture as render target
	hr = pd3dDevice->CreateTexture(
		                               m_backBufferSurfaceDesc.Width / 2, 
		                               m_backBufferSurfaceDesc.Height / 2,
		                               1, 
		                               D3DUSAGE_RENDERTARGET, 
	                               	   D3DFMT_A8R8G8B8,
		                               D3DPOOL_DEFAULT, 
		                               &m_pRenderTarget3,
		                               NULL
		                              );
	m_pRenderTarget3->GetSurfaceLevel( 0, &m_pSurface3 );

	hr = pd3dDevice->CreateVertexBuffer(  4 * sizeof(CUSTOMVERTEX), 
		                                    D3DUSAGE_WRITEONLY,
		                                    0, 
		                                    D3DPOOL_DEFAULT,
		                                    &m_pImageVB,
		                                    NULL
		                                  );

	float left		= -1.0f;
	float right		= 1.0f;
	float top		= 1.0f;
	float bottom	= -1.0f;

	CUSTOMVERTEX* v;
	m_pImageVB->Lock(0, 4 * sizeof(CUSTOMVERTEX), (VOID**)&v, 0);

	// left bottom
	v[0].x = left;
	v[0].y = bottom;

	// left top
	v[1].x = left;
	v[1].y = top;

	// right bottom 
	v[2].x = right;
	v[2].y = bottom;

	// right top
	v[3].x = right;
	v[3].y = top;

	m_pImageVB->Unlock();

	pd3dDevice->GetRenderTarget( 0, &m_pBackBuffer );
	pd3dDevice->GetDepthStencilSurface( &m_pZBuffer );
/*
	// Set render states for Direct3D run-time
	pd3dDevice->SetRenderState(D3DRS_DITHERENABLE,   0 );
	pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, 0 );
	pd3dDevice->SetRenderState(D3DRS_ZENABLE,        TRUE);
	pd3dDevice->SetRenderState(D3DRS_AMBIENT,        0xffff0000 );
	pd3dDevice->SetRenderState(D3DRS_LIGHTING,       1 );*/

    return S_OK;
}

void osHDRLight::set_hdrRenderTarget()
{
	if( m_pd3dDevice && m_pEffect && g_bUseHDRLight && osHDRLight::m_bSupportHDRLight )
	{
		m_pd3dDevice->SetRenderTarget( 0, m_pSurface );
		m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET,  0xffffffff, 1.0f, 0L );
	}
}

void osHDRLight::set_hdrParameter( float _fmx, float _fmy, float _diffuse, float _exposure )
{
	if( m_pEffect && g_bUseHDRLight && osHDRLight::m_bSupportHDRLight )
	{
		m_fmx = _fmx;
		m_fmy = _fmy;
		m_fDiffuse = _diffuse;
		m_fExposureLevel = _exposure;
		osassert( m_hA );
		osassert( m_hkd );
		osassert( m_hExposure );
		m_pEffect->SetFloat( m_hA, 1.0f/( 8.0f*PI ) * sqrt((m_fmx + 1) * (m_fmy + 1)));
		m_pEffect->SetFloat( m_hkd, m_fDiffuse );
		m_pEffect->SetFloat( m_hExposure, m_fExposureLevel );
	}
}

void osHDRLight::set_hdrFm( float _fm )
{
	if( m_pEffect && g_bUseHDRLight && osHDRLight::m_bSupportHDRLight )
	{
		m_fmx = m_fmy = _fm;
		m_pEffect->SetFloat( m_hA, 1.0f/( 8.0f*PI ) * sqrt((m_fmx + 1) * (m_fmy + 1)));
	}
}

void osHDRLight::set_hdrDiffuse( float _diffuse )
{
	if( m_pEffect && g_bUseHDRLight && osHDRLight::m_bSupportHDRLight )
	{
		m_fDiffuse = _diffuse;
		m_pEffect->SetFloat( m_hkd, m_fDiffuse );
	}
}

void osHDRLight::set_hdrExposure( float _exposure )
{
	if( m_pEffect && g_bUseHDRLight && osHDRLight::m_bSupportHDRLight )
	{
		m_fExposureLevel = _exposure;
		m_pEffect->SetFloat( m_hExposure, m_fExposureLevel );
	}
}

void osHDRLight::render_hdrLight()
{
	if( !g_bUseHDRLight || !osHDRLight::m_bSupportHDRLight || !m_pEffect || !m_pd3dDevice || !g_ptrCamera )
		return;

    
	
	m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );

	//防止点击出现白屏	
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

	HRESULT hr;

	D3DXMatrixIdentity( &m_mWorld );
	g_ptrCamera->get_viewmat( m_mView );
	g_ptrCamera->get_projmat( m_mProj );
	
	m_pEffect->SetFloat( m_hmx, m_fmx );
	m_pEffect->SetFloat( m_hmy, m_fmy );

	// part A of the Ashikhmin-Shirley equation
	m_pEffect->SetFloat( m_hA, 1.0f/( 8.0f*PI ) * sqrt((m_fmx + 1) * (m_fmy + 1)));
	m_pEffect->SetFloat( m_hkd, m_fDiffuse );
	m_pEffect->SetFloat( m_hExposure, m_fExposureLevel );

	osVec3D campos;
	g_ptrCamera->get_curpos( &campos );
	D3DXVECTOR4 vTemp = D3DXVECTOR4( campos.x, campos.y, campos.z, 1.0f );
	m_pEffect->SetVector( m_hvCamPos, &vTemp );
	m_pEffect->SetVector( m_hvLightPos, &m_LightPos );

	// pixel size to convert to texel space
	float fPixelSizeX = 1.0f / ( m_backBufferSurfaceDesc.Width / 2.0f );
	float fPixelSizeY = 1.0f / ( m_backBufferSurfaceDesc.Height / 2.0f );
	m_pEffect->SetVector( m_hPixelSize, &D3DXVECTOR4( fPixelSizeX, fPixelSizeY, 1.0f, 1.0f ) );
	

	// 2nd pass
	// downsample
	m_pd3dDevice->SetRenderTarget( 0, m_pSurface2 );
	m_pEffect->SetTechnique( m_hScaleBuffer );
	m_pd3dDevice->SetVertexDeclaration( m_pDecl2 );

	// sample down render target
	m_pEffect->SetTexture(m_hMap, m_pRenderTarget);

	m_pEffect->Begin(NULL, 0);
	m_pEffect->BeginPass(0);
	m_pd3dDevice->SetStreamSource( 0, m_pImageVB, 0, sizeof(CUSTOMVERTEX) );
	m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	(m_pEffect->EndPass());
	m_pEffect->End();

	// 3rd pass
	// GaussX filter			
	D3DXVECTOR2 vSampleOffsets[16];
	FLOAT       fSampleOffsets[16];
	D3DXVECTOR4 vSampleWeights[16];

	m_pd3dDevice->SetVertexDeclaration( m_pDecl2 );
	m_pd3dDevice->SetRenderTarget(0, m_pSurface3 );
	m_pEffect->SetTexture( m_hMap, m_pRenderTarget2 );

	D3DXVECTOR4 v[7];
	hr = CreateTexCoordNTexelWeights( m_backBufferSurfaceDesc.Height / 4, 
										fSampleOffsets, 
										vSampleWeights, 
										7.0f, 
										1.5f
		
										);
	int i;
	for( i=0; i < 16; i++ )
	{
		vSampleOffsets[i] = D3DXVECTOR2( 0.0f, fSampleOffsets[i] );
	}

	m_pEffect->SetValue( "vertTapOffs", vSampleOffsets, sizeof(vSampleOffsets) );
	m_pEffect->SetValue( "TexelWeight", vSampleWeights, sizeof(vSampleWeights) );

	m_pd3dDevice->SetStreamSource(0, m_pImageVB, 0, sizeof(CUSTOMVERTEX));

	m_pEffect->SetTechnique( m_hGaussY );
	
	m_pEffect->Begin(NULL, 0);
	m_pEffect->BeginPass(0);
	m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	m_pEffect->EndPass();
	m_pEffect->End();
	
	// 4th pass
	// GaussY filter			
	m_pd3dDevice->SetRenderTarget( 0, m_pSurface2 );
	m_pd3dDevice->SetVertexDeclaration( m_pDecl2 );
	m_pEffect->SetTexture( m_hMap, m_pRenderTarget3 );
	

	hr = CreateTexCoordNTexelWeights( m_backBufferSurfaceDesc.Width / 4, 
		                              fSampleOffsets, 
		                              vSampleWeights, 
		                              7.0f,
		                              1.5f
									 );
	for( i=0; i < 16; i++ )
	{
		vSampleOffsets[i] = D3DXVECTOR2( fSampleOffsets[i], 0.0f );
	}

	m_pEffect->SetValue( "horzTapOffs", vSampleOffsets, sizeof(vSampleOffsets) );
	m_pEffect->SetValue( "TexelWeight", vSampleWeights, sizeof(vSampleWeights) );

	// Gauss in x direction
	m_pEffect->SetTechnique( m_hGaussX );
	m_pEffect->Begin( NULL, 0 );
	m_pEffect->BeginPass(0);
	m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	m_pEffect->EndPass();
	m_pEffect->End();

	// 5th pass
	// Change the rendertarget back to the main backbuffer
	m_pd3dDevice->SetRenderTarget( 0, m_pBackBuffer );
	m_pEffect->SetTechnique( m_hScreen );
	m_pd3dDevice->SetVertexDeclaration( m_pDecl2 );
	m_pEffect->SetTexture( m_hFullResMap, m_pRenderTarget );
	m_pEffect->SetTexture( m_hMap, m_pRenderTarget2 );

	m_pEffect->Begin( NULL, 0 );
	m_pEffect->BeginPass( 0 );
	m_pd3dDevice->SetStreamSource( 0, m_pImageVB, 0, sizeof(CUSTOMVERTEX) );
	m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	m_pEffect->EndPass();
	m_pEffect->End();

}

HRESULT osHDRLight::CreateTexCoordNTexelWeights(DWORD dwRenderTargetSize,
									float fTexCoordOffset[15],
									D3DXVECTOR4* vTexelWeight,
									float fDeviation,
									float fMultiplier )
{
	int i=0;

	// width or height depending on which Gauss filter is applied
	// the vertical or horizontal filter
	float tu = 1.0f / (float)dwRenderTargetSize; 

	// Fill the center texel
	float weight = fMultiplier * GaussianDistribution( 0, 0, fDeviation );
	vTexelWeight[0] = D3DXVECTOR4( weight, weight, weight, 1.0f );

	fTexCoordOffset[0] = 0.0f;

	// Fill the first half
	for( i=1; i < 8; i++ )
	{
		// Get the Gaussian intensity for this offset
		weight = fMultiplier * GaussianDistribution( (float)i, 0, fDeviation );
		fTexCoordOffset[i] = i * tu;

		vTexelWeight[i] = D3DXVECTOR4( weight, weight, weight, 1.0f );
	}

	// Mirror to the second half
	for( i=8; i < 15; i++ )
	{
		vTexelWeight[i] = vTexelWeight[i-7];
		fTexCoordOffset[i] = -fTexCoordOffset[i-7];
	}

	return S_OK;
}

float osHDRLight::GaussianDistribution( float x, float y, float rho )
{
	float g = 1.0f / sqrtf( 2.0f * D3DX_PI * rho * rho );
	g *= expf( -(x * x + y * y)/(2 * rho * rho));

	return g;
}
