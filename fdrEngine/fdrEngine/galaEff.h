//===============================================================================
/*! \file
*	Filename:	galaEff.h	
*
*	Desc:		this file contain the base class CGalaEff 
*				CGalaEff manage all effect of Galaxy game using VS and PS 
*				it's main mechanism is rendering to texture and redraw it
*				by a rectangle fill up the screen
*
*	His:		tzz	2008-7-3
*/
//===============================================================================

# pragma once

# include <d3dx9.h>

/*! \class CGalaEff
*	\brief base class of galaxy game's effect Manager class
*
*/
class CGalaEff
{
public:

	//! screen width 
	static int						sm_iWidth;

	//! screen Height
	static int						sm_iHeight;

	//! format of render target
	static D3DFORMAT				sm_format;

	//! using the full screen anit-alias?
	static BOOL						sm_bUseFSAA;

protected: //pretected data (enum) to define the derive class

	/*! \enum 
	*	\brief initializing diffuse color of vertex
	*/
	enum {
		e_rectColor = D3DCOLOR_XRGB(255,255,255),
	};
	/*! \struct MyVertex
	*	\brief describe the vertex format
	*/
	typedef struct _MyVertex
	{
		D3DXVECTOR3		position;
		DWORD			color;
		float			tu0,tv0;
	}MyVertex;

	// there is not FVF because we render the rectangle 
	// by SM

protected: // d3d resource to use
	
	//!	effect pointer
	ID3DXEffect*					m_lpEffect;

	//! main d3d scene render surface(main render target)
	LPDIRECT3DSURFACE9				m_lpMainSurface;

	//! if the engine activate the full screen anit-alias
	//! we must backup the depth stencil buffer and set it NULL before render the screen 
	//! effect (grow)
	LPDIRECT3DSURFACE9				m_lpBackupDepthStencil;


	//! shader declaration
	LPDIRECT3DVERTEXDECLARATION9	m_lpDecl;

	//! vertex buffer is necessary
	//! contain a rectangle filling up the screen
	LPDIRECT3DVERTEXBUFFER9			m_lpVertBuf;

	//! is effective??
	BOOL							m_bEffective;

	//! initialized?
	//! recreate the device of d3d
	BOOL							m_bInit;

	

public: //constructor and destructor

	/*! \fn constructor and destructor
	*	...
	*/
	CGalaEff(void);
	~CGalaEff(void);

public: //virtual function


	/*! \fn void RenderEffScene(LPDIRECT3DDEVICE9 _lpd3dDev)
	*	\brief : render the grow scene with some complex step
	*	
	*	\param _lpd3dDev	: d3d device
	*	\param bool			: some param
	*	\return void
	*/
	virtual void RenderEffScene(LPDIRECT3DDEVICE9 _lpd3dDev,bool=false) = 0;

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
	virtual void SetEffective(LPDIRECT3DDEVICE9 _lpd3dDev,BOOL _bEffective = TRUE) = 0;

	/*! \brief create the resource when lost device
	*/
	virtual BOOL CreateRes(LPDIRECT3DDEVICE9 _lpd3dDev) = 0;

protected : // derive class using function

	/*! \fn void InitEffScene(LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidth,int _iHeight,D3DFORMAT _format =  D3DFMT_A8R8G8B8);
	*	\brief initialize the effect data in base class 
	*			this function will be called in derive class' initiailizing function
	*	
	*	\param _lpd3dDev	: d3d device
	*	\param _iWidth		: the viewport width(screen width)
	*	\param _iHeight		: the viewport height(screen height)
	*	\param _format		: the color format of main screen surface
	*	\param _bUseFSAA	: did use the FSAA?
	*	\return void
	*/
	void InitEffScene(LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidth,int _iHeight,D3DFORMAT _format =  D3DFMT_A8R8G8B8,BOOL _bUseFSAA = FALSE);


	/*! \fn void CreateRect(LPDIRECT3DDEVICE9 _lpd3dDev)
	*	\brief : create the main rectangle to render finally
	*
	*	\param _lpd3dDev	: d3d device
	*	\return void
	*/
	void CreateRect(LPDIRECT3DDEVICE9 _lpd3dDev);

	/*! \fn void InitEffect(LPDIRECT3DDEVICE9 _lpd3dDev)
	*	\brief : initialize some shader interface
	*	
	*	\param _lpd3dDev	: d3d device
	*	\param _filename	: fx filename to compile
	*	\return void
	*/
	void InitEffect(LPDIRECT3DDEVICE9 _lpd3dDev,LPCTSTR _filename);

	/*! \fn void PrepareEffScene(LPDIRECT3DDEVICE9 _lpd3dDev)
	*	\brief : prepare the effect scene ( view project matrix and viewport variable)
	*
	*	\param _lpd3dDev : d3d device
	*	\return void
	*/
	virtual void PrepareEffScene(LPDIRECT3DDEVICE9 _lpd3dDev);

	/*!	\brief : render blur texture in pass 0 
	*
	*	\param _lpd3dDev		: d3d deviece
	*	\param _lpRenderTarget	: surface of rendering target
	*	\return void
	*/
	void RenderToTex(LPDIRECT3DDEVICE9 _lpd3dDev,LPDIRECT3DSURFACE9 _lpRenderTarget);


public: // public function

	/*! \fn BOOL IsEffective(void)
	*	\brief : is effective for show the grow scene
	*
	*	\param void
	*	\return BOOL	: TRUE if effective
	*/
	BOOL IsEffective(void)const {return m_bEffective;}

	//! has Initialized the effect ? want to destroy the device of d3d?
	BOOL IsInitialze(void)const {return m_bInit;}

};