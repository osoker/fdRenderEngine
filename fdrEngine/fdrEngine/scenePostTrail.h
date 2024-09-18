//===============================================================================
/*! \file
*	Filename:	sceneGrow.h	
*	Desc:		this class will use the VS\PS to generate the grow effect 
*				of scene
*	His:		tzz	2008-6-30
*/
//===============================================================================
# pragma once

# include <d3dx9.h>

# include ".\galaEff.h"



/*! \class CPostTrailEff
*	this grow class will contain a rectangle
*	fill with a texture rendered by whole scene
*	and display it finally by PS grow effect
*/
class CPostTrailEff : public CGalaEff
{
	//! the max trail surface....
	const static size_t				scm_maxTrailFrame = 3;

protected: // protected data
	
	//@{ d3d resource

	//! the stencil buffer of m_lpSceneSurface...
	LPDIRECT3DSURFACE9				m_lpSceneDepthStencil;

	//! render target texture
	//! the trail frame
	LPDIRECT3DTEXTURE9				m_lpTrailFrameTex[scm_maxTrailFrame];
	LPDIRECT3DSURFACE9				m_lpTrailFrameSur[scm_maxTrailFrame];

	//! extra render target
	LPDIRECT3DSURFACE9				m_extraMainSurface;
	//@}

	//! the frame refresh timer
	size_t							m_refreshTimer;
	//@}

public: //constructor and destructor
	/*! \fn constructor and destructor
	*	...
	*/
	CPostTrailEff(void);
	~CPostTrailEff(void);


public:	// public function

	/*!	\fn BOOL InitEffScene(LPDIRECT3DDEVICE9 _lpd3dDev)
	*	\brief : initialize the resource and set the render target
	*
	*	\param _lpd3dDev	: d3d deviece
	*	\param _iWidth		: the viewport width(screen width)
	*	\param _iHeight		: the viewport height(screen height)
	*	\param _format		: the color format of main screen surface
	*	\param _bUseFSAA	: use FSAA effect?
	*	\return BOOL		: FALSE if failed
	*/
	BOOL InitPostTrailScene(LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidth,int _iHeight,D3DFORMAT _format =  D3DFMT_A8R8G8B8,BOOL _bUseFSAA  = FALSE);

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
	virtual void SetEffective( LPDIRECT3DDEVICE9 _lpd3dDev,BOOL _bEffective = TRUE);

	/*! \brief create the resource when lost device
	*/
	virtual BOOL CreateRes(LPDIRECT3DDEVICE9 _lpd3dDev);

	//! clear the current frame texture
	void ClearCurrFrameTex(LPDIRECT3DDEVICE9 _lpd3dDev);

	//! set curr frame target
	void SetCurrFrameTarget(LPDIRECT3DDEVICE9 _lpd3dDev);


	void SetMainRenderSurface(LPDIRECT3DSURFACE9 _mainSurface);


};