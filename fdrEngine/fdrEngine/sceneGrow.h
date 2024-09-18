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



/*! \class CGrowEff
*	this grow class will contain a rectangle
*	fill with a texture rendered by whole scene
*	and display it finally by PS grow effect
*/
class CGrowEff : public CGalaEff
{
protected: // protected data
	
	//@{ d3d resource
	//! render target texture
	//! this texture is original texture of the scene
	LPDIRECT3DTEXTURE9				m_lpSceneTex;

	//! scene surface
	LPDIRECT3DSURFACE9				m_lpSceneSurface;

	//! the stencil buffer of m_lpSceneSurface...
	LPDIRECT3DSURFACE9				m_lpSceneDepthStencil;

	//! render target texture
	//! this texture is blured from the scene texture 
	LPDIRECT3DTEXTURE9				m_lpBlurTex;

	//! blur texture surface
	LPDIRECT3DSURFACE9				m_lpBlurSurface;

	//! render target texture
	//! this texture is blured from the scene texture 
	LPDIRECT3DTEXTURE9				m_lpBlurTex1;

	//! blur texture surface
	LPDIRECT3DSURFACE9				m_lpBlurSurface1;
	//@}

	

	//@{ controlling state variable

	//@}

	//! 屏幕纹理.
	LPDIRECT3DTEXTURE9				m_lpScreenTex;
	//! 屏幕纹理的surface.
	LPDIRECT3DSURFACE9				m_lpScreenSurface;
	//! 屏幕光波用到的effect
	ID3DXEffect*                    m_lpScreenWaveEffect;

	//! 屏幕的宽度与高度.
	int                             m_iWidth,m_iHeight;

private:
	//! 删除屏幕光波相关的资源.
	BOOL    release_screenWaveRes( void );


public: //constructor and destructor
	/*! \fn constructor and destructor
	*	...
	*/
	CGrowEff(void);
	~CGrowEff(void);


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
	BOOL InitGrowScene(LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidth,int _iHeight,D3DFORMAT _format =  D3DFMT_A8R8G8B8,BOOL _bUseFSAA  = FALSE);

	//! River @ 2010-6-25:屏幕光波相关资源初始化.
	BOOL    reinit_screenWave( LPDIRECT3DDEVICE9 _lpd3dDev );

public: //virutal public function

	/*! \fn void RenderEffScene(LPDIRECT3DDEVICE9 _lpd3dDev)
	*	\brief : render the grow scene with some complex step
	*	
	*	\param _lpd3dDev	: d3d device
	*	\param bool			: some param
	*	\return void
	*/
	virtual void RenderEffScene(LPDIRECT3DDEVICE9 _lpd3dDev,bool=false);


	// River @ 2010-6-24:渲染屏幕效果
	virtual void RenderScreenWave( LPDIRECT3DDEVICE9 _lpd3dDev );


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

	//! River: 重设Effect用的renderTarget.
	virtual void reset_renderTarget( LPDIRECT3DDEVICE9 _lpd3dDev );


	/*! \brief create the resource when lost device
	*/
	virtual BOOL CreateRes(LPDIRECT3DDEVICE9 _lpd3dDev);

	//! River: 用于水面折射.
	LPDIRECT3DSURFACE9 get_targetSurface( void ) { return m_lpSceneSurface; } 

protected: //protected function

	/*! \fn void RenderBlurTex(LPDIRECT3DDEVICE9 _lpd3dDev)
	*	\brief : render blur texture in pass 0 
	*
	*	\param _lpd3dDev	: d3d deviece
	*	\return void
	*/
	inline void RenderBlurTex(LPDIRECT3DDEVICE9 _lpd3dDev);

//WZ 2010-7-26
protected:
	BOOL m_bWaterActive;
	LPDIRECT3DTEXTURE9				m_lpNoiseTex;
public:
	void SetWaterActive(LPDIRECT3DDEVICE9 _lpd3dDev,BOOL _bAct);
};