//===============================================================================
/*! \file
*	Filename:	coronaEff.h
*
*	Desc:		This effect is draw some corona around the object.
*				pass.1 : draw the object onto a texure
*				pass.2 : draw the rectangle with pure white pixel blur(alpha value too)
*				pass.3 : draw the blur rectangle with alphablend
*				pass.4 : draw the original texture rectangle with alpha blend
*
*	His:		tzz	2008-7-3
*/
//===============================================================================

# pragma once

# include <d3dx9.h>

# include "./galaEff.h"

/*! \class CCoronaEff
*	\brief : corona effect manager class
*/
class CCoronaEff : public CGalaEff
{
protected: // own data

	//! original scene texture
	//! a object will render onto it (just this object) without 
	//! background and frontground
	LPDIRECT3DTEXTURE9				m_lpOrigTex;

	//! original texture's surface
	LPDIRECT3DSURFACE9				m_lpOrigSurface;

	//! corona texture with alpha value
	LPDIRECT3DTEXTURE9				m_lpCoronaTex;

	//1 corona texture's surface
	LPDIRECT3DSURFACE9				m_lpCoronaSurface;

	//! the color of corona
	DWORD							m_dwCoronaColor;

public: // constructor and destructor

	/*! \fn constructor adn destructor
	*	...
	*/
	CCoronaEff(void);
	~CCoronaEff(void);

public: //own public function
	
	/*! \fn BOOL InitCoronaEff(LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidth,int _iHeight,D3DFORMAT _format =  D3DFMT_A8R8G8B8)
	*	\brief : initialize the Corona effect
	*	
	*	\param _lpd3dDev	: d3d device
	*	\param _iWidth		: the viewport width(screen width)
	*	\param _iHeight		: the viewport height(screen height)
	*	\param _format		: the color format of main screen surface
	*	\return BOOL		: FALSE if failed
	*/
	BOOL InitCoronaEff(LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidth,int _iHeight,D3DFORMAT _format =  D3DFMT_A8R8G8B8);


public:	//virutal function

	
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




};