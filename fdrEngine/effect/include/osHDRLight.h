#pragma once
# include "../../interface/osInterface.h"




class osHDRLight
{
public:
	osHDRLight(void);
	~osHDRLight(void);

	HRESULT init_hdrLight();
	void set_hdrRenderTarget();
	void render_hdrLight();
	void set_hdrParameter( float _fmx, float _fmy, float _diffuse, float _exposure );
	void set_hdrFm( float _fm );
	void set_hdrDiffuse( float _diffuse );
	void set_hdrExposure( float _exposure );
	HRESULT CreateTexCoordNTexelWeights(DWORD dwRenderTargetSize,
										float fTexCoordOffset[15],
										D3DXVECTOR4* vTexelWeight,
										float fDeviation,
										float fMultiplier
									    ); 
	float GaussianDistribution( float x, float y, float rho );
private:
	LPDIRECT3DDEVICE9		m_pd3dDevice;
	D3DXVECTOR4				m_LightPos;

	D3DXMATRIX				m_mWorld;
	D3DXMATRIX				m_mView;
	D3DXMATRIX				m_mProj;

	float					m_fmx;
	float					m_fmy;
	float					m_fDiffuse;
	float					m_fExposureLevel;

	D3DSURFACE_DESC         m_backBufferSurfaceDesc;
	LPDIRECT3DTEXTURE9      m_pRenderTarget;
	LPDIRECT3DTEXTURE9      m_pRenderTarget2;
	LPDIRECT3DTEXTURE9      m_pRenderTarget3;
	LPDIRECT3DSURFACE9			m_pBackBuffer, m_pZBuffer;
	LPDIRECT3DSURFACE9			m_pSurface, m_pSurface2, m_pSurface3, m_pSurface4;
	LPDIRECT3DVERTEXBUFFER9		m_pImageVB;
	LPDIRECT3DVERTEXDECLARATION9	m_pDecl;		// vertex declaration
	LPDIRECT3DVERTEXDECLARATION9	m_pDecl2;		// 
	ID3DXEffect*            m_pEffect;					// D3DX effect interface
	D3DXHANDLE				m_hTechnique;										// handle effect technique
	D3DXHANDLE				m_hmWorldViewProj;									// handle world-view-projection matrix
	D3DXHANDLE				m_hmWorld;											// handle world matrix
	D3DXHANDLE				m_hvMaterialColor;									// handle color of *.x file material
	D3DXHANDLE				m_hvLightPos;										// handle light position
	D3DXHANDLE				m_hvCamPos;											// handle Eye position
	D3DXHANDLE				m_hScreen;				// handle for screen blit
	D3DXHANDLE				m_hMap;					// handle render targets
	D3DXHANDLE				m_hFullResMap;
	D3DXHANDLE				m_hScaleBuffer;
	D3DXHANDLE				m_hGaussX;
	D3DXHANDLE				m_hGaussY;
	D3DXHANDLE				m_hIteration;
	D3DXHANDLE				m_hPixelSize;

	// Texture map handles
	D3DXHANDLE				m_hmx;				   // handle mx
	D3DXHANDLE				m_hmy;				   // handle my
	D3DXHANDLE				m_hA;				   // handle part A of Ashikhmin-Shirley
	D3DXHANDLE				m_hkd;				   // handle diffuse component
	D3DXHANDLE				m_hExposure;	       // handle exposure level
public:
	static bool m_bSupportHDRLight;
};









