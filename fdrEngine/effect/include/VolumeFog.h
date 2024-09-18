//===================================================================
/** \file  
* Filename :   VolumeFog.h
* Desc     : Reference http://developer.nvidia.com/view.asp?IO=FogPolygonVolumes  
*/
//===================================================================
#pragma once

struct I_deviceManager;
class CVolumeFog
{
public:
	CVolumeFog(void);
	~CVolumeFog(void);

	///@{ 创建和释放资源
	void InitVolumeFog(I_deviceManager*    _middlePipe,IDirect3DDevice9*	_dedDevice);
	void ReleaseVolumeFog();
	///@}

	//@{ Must Be Scope in BegineScene/EndScene.
	void PreRenderSceneDist();
	void PreRenderFrontFog();
	void PreRenderBackFog();
	void DrawSceneFog();
	//@}

	///@{
	//void PreRenderWaterDepth();
	//void RestoreRenderTarget();
	///@}

	//@{ For Debug
	void DumpSceneDist();	
	void DumpFrontFogDist();	
	void DumpBackFogDist();	
	//@}

	//@{
	HRESULT OnLostDevice();
	HRESULT OnRestoreDevice(IDirect3DDevice9* _d3dDevice);
	//@}

	///@{
	HRESULT CreateGradient2D( IDirect3DTexture9 ** ppTex2D,
		IDirect3DDevice9 * pD3DDev,
		int width, int height, byte lower, byte upper,
		D3DXVECTOR3 col_mask_u, D3DXVECTOR3 col_mask_v );

	void CreateGradientDithered( IDirect3DTexture9 ** ppTex,
		IDirect3DDevice9 * pD3DDev,
		int width, int height,
		byte lower, byte upper,
		D3DXVECTOR3 color_mask,
		const D3DCOLOR * pDither_upper );

	float m_fBitReduce;

	IDirect3DTexture9 *		m_pTexRedGreenRamp;
	int                     m_TexRedGreenId;
	IDirect3DTexture9 *		m_pTexBlueRamp;
	int                     m_TexBlueId;
	IDirect3DTexture9 *		m_pTexBlueOneDim;
	int                     m_TexBlueOneDimId;

	HRESULT		CreateRampTextures( IDirect3DDevice9 * pD3DDev );
	HRESULT		FreeRampTextures();
	HRESULT     CreateBlueOneDimTex();

	enum DepthValuesPerChannel
	{
		//  | # of surfaces that can be rendered before artifacts occur
		//  |
		//  |				| depth values from front to back clip planes
		PVF_256,	//  1 surfaces      24-bit
		PVF_128,	//  2 surfaces		21-bit
		PVF_64,		//  4 surfaces		18-bit
		PVF_32,		//  8 surfaces		15-bit = 32768
		PVF_16,		//  16 surfaces		12-bit = 4096
		PVF_8,		//  32 surfaces		9-bit  = 512
		PVF_4,		//  64 surfaces		6-bit  = 64
		PVF_2,		//  128 surfaces	3-bit  = 8
		PVF_UNSET
	};

	// Maximum number of surfaces that can overlap before artifacts
	//  occur.  Front faces are handled separately from back faces, 
	//  so if the max depth complexity is two, that is two front 
	//  surfaces and two back surfaces.
	// The resulting depth precision between near and far clip planes
	//  is listed as the _ BIT_DEPTH
	enum MaxFogDepthComplexity
	{
		PVF_1_SURFACES_24_BIT_DEPTH,
		PVF_2_SURFACES_21_BIT_DEPTH,
		PVF_4_SURFACES_18_BIT_DEPTH,
		PVF_8_SURFACES_15_BIT_DEPTH,
		PVF_16_SURFACES_12_BIT_DEPTH,
		PVF_32_SURFACES_9_BIT_DEPTH,
		PVF_64_SURFACES_6_BIT_DEPTH,
		PVF_128_SURFACES_3_BIT_DEPTH,
		PVF_MAXDEPTHUNSET
	};

	MaxFogDepthComplexity	m_MaxFogDepthComplexity;
	DepthValuesPerChannel	m_DepthValuesPerColorChannel;
	int                     m_DepthLowBitSize;
	// m_fGrnRamp and m_fBluRamp are used to determine the encoding
	// of depth as an RGB color.  They determine the texture coordinate
	// scale for the green and blue color ramps, and are used to control
	// how many green ramps occur per red color increment and how many 
	// blue color ramps occur per green color increment.
	// The values of these depend on the choice of m_fBitReduce

	float		m_fGrnRamp;
	float		m_fBluRamp;

	void SetVolumeSize(MaxFogDepthComplexity _depth_relation);
	///@}
public:
	HRESULT LoadShaders();
	HRESULT ComputeStepTexture();
	int      mTargetWidth;
	int      mTargetHeight; 
	IDirect3DDevice9*	mpd3dDevice;
	LPDIRECT3DSURFACE9	mpOldRenderTarget;
	LPDIRECT3DSURFACE9	mpOldStencilZ;
	LPDIRECT3DSURFACE9	mpTargetZ;

	LPDIRECT3DSURFACE9	mpTempSurface;

	LPDIRECT3DTEXTURE9	mpBackFogDist;
	LPDIRECT3DTEXTURE9	mpFrontFogDist;
	LPDIRECT3DTEXTURE9	mpSceneDist;

	//LPDIRECT3DTEXTURE9	mpStepTexture;
	//int                 mStipTexutreId;

	ID3DXEffect*	    mDepthEffect;
	ID3DXEffect*	    mFrontFogEffect;
	ID3DXEffect*	    mBackFogEffect;
	ID3DXEffect*	    mScreenFogEffect;

	int                 mFrontFogShaderId;
	int                 mBackFogShaderId;

	LPDIRECT3DVERTEXBUFFER9 mpScreenVertBuf;
	LPDIRECT3DVERTEXDECLARATION9   mpFogDeclaration;

	
	bool                mbInited;
};

extern CVolumeFog* gVolumeFogPtr;
