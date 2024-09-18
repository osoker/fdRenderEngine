//===============================================================================
/*! \file
*	Filename:	sceneGrow.h	
*	Desc:		this class will use the VS\PS to generate the HDR effect 
*				of scene
*				this Effect of HDR is real HDR light effect and "steal" from the 
*				DX demo : HDRPipeline, pass 13 to generate the dynamic light
*				and compute the pixel of color of HDR image....
*	His:		tzz	2008-6-30
*/
//===============================================================================
# pragma once

# include <d3dx9.h>

# include ".\galaEff.h"



/*! \class CHDREff
*	this grow class will contain a rectangle
*	fill with a texture rendered by whole scene
*	and display it finally by PS grow effect
*/
class CHDREff : public CGalaEff
{

protected: // protected data

	//! the level to compute the luminance..
	static const int				scm_iLuminanceLev = 8;

	//@{ d3d resource
	//! render target texture
	//! this texture is original texture of the scene
	LPDIRECT3DTEXTURE9				m_lpSceneTex;

	//! scene surface
	LPDIRECT3DSURFACE9				m_lpSceneSurface;

	//! luminance compute texture and surface
	LPDIRECT3DTEXTURE9				m_lpLuminTex[scm_iLuminanceLev];
	LPDIRECT3DSURFACE9				m_lpLuminSurface[scm_iLuminanceLev];

	//! render target texture
	//! this texture is blured from the scene texture 
	LPDIRECT3DTEXTURE9				m_lpBlurTex;

	//! the second texture for rendering target 
	//! this texture is blured from above texture 
	LPDIRECT3DTEXTURE9				m_lpBlurTex1;

	//! blur texture surface
	LPDIRECT3DSURFACE9				m_lpBlurSurface;

	//! blur texture surface one
	LPDIRECT3DSURFACE9				m_lpBlurSurface1;

	//@}


	//@{ controlling state variable

	//! down sampler offset data
	D3DXVECTOR4						m_downSamplerOffset[16];


	//! weight of Blur 
	float							m_arrBlurWeight[9];

	//! offset of HBlur
	float							m_HBlurOffset[9];

	//! offset of VBlur
	float							m_VBlurOffset[9];

	//! HDR value (*&%&*^&^$#
	float							m_fGaussMean;

	//! HDR value &^$$(*(#
	float							m_fGaussStdDev;

	//! HDR value &*(^*^*()%
	float							m_fGaussMultiplier;

	//! exposure of HDR
	float							m_fExposure;

	//! passing bright color threshold
	float							m_fBrightSelThreshold;

	//@}

public: //constructor and destructor
	/*! \fn constructor and destructor
	*	...
	*/
	CHDREff(void);
	~CHDREff(void);


public:	// public function

	/*!	\fn BOOL InitEffScene(LPDIRECT3DDEVICE9 _lpd3dDev)
	*	\brief : initialize the resource and set the render target
	*
	*	\param _lpd3dDev	: d3d deviece
	*	\param _iWidth		: the viewport width(screen width)
	*	\param _iHeight		: the viewport height(screen height)
	*	\param _format		: the color format of main screen surface
	*	\return BOOL		: FALSE if failed
	*/
	BOOL InitGrowScene(LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidth,int _iHeight,D3DFORMAT _format =  D3DFMT_A8R8G8B8);

public: //virutal public function

	/*! \fn void RenderEffScene(LPDIRECT3DDEVICE9 _lpd3dDev)
	*	\brief : render the grow scene with some complex step
	*	
	*	\param _lpd3dDev	: d3d device
	*	\param bool			: some param
	*	\return void
	*/
	virtual void RenderEffScene(LPDIRECT3DDEVICE9 _lpd3dDev,bool=false);

	/*! void DestroyEffRes(void)
	*	\brief : destroy the resource of d3d
	*
	*	\param void
	*	\return void
	*/
	virtual void DestroyEffRes(void);

	/*! \fn void SetEffective(BOOL _bEffective = TRUE)
	*	\brief : set the effective flag and render target
	*
	*	\param _lpd3dDev	: d3d device
	*	\param _bEffective	: TRUE if want to activate this effective
	*	\return void
	*/
	virtual void SetEffective(LPDIRECT3DDEVICE9 _lpd3dDev,BOOL _bEffective = TRUE);

	/*! \brief create the resource when lost device
	*/
	virtual BOOL CreateRes(LPDIRECT3DDEVICE9 _lpd3dDev);

public:// public functionn to operate
 

protected: //protected function

	//! prepare the selecting bright pixel data
	void PrepareBrightPassData(void);

	//! prepare the down sample data...
	void PrepareDownSampleData(void);

	//! prepare the horizontal blur pass data ,see the code to detail
	void PrepareHBlurData(void);

	//! prepare the vertical blur pass data ,see the code to detail
	void PrepareVBlurData(void);


	//! prepara the final pass data,see the code to detail
	void PrepareFinalPassData(void);

	/*! \brief The gaussian equation is defined as such:
    *                                                 -(x - mean)^2
    *                                                 -------------
    *                                1.0               2*std_dev^2
    *    f(x,mean,std_dev) = -------------------- * e^
    *                        sqrt(2*pi*std_dev^2)
	*
	*	\param void 
	*/
	float ComputeGaussianValue(float x,float mean,float std_deviation)
	{ 
		return ( 1.0f / sqrt( 2.0f * D3DX_PI * std_deviation * std_deviation ) ) 
                * expf( (-((x-mean)*(x-mean)))/(2.0f * std_deviation * std_deviation) );
	}
	


};