//===============================================================================
/*! \file
*	Filename:	GridCurve.h	
*	Desc:		this class will change the screen image to many grid and 
*				make them disappeared around 
*	His:		tzz	2008-6-19
*/
//===============================================================================

# pragma once

# include "d3dx9.h"


/*! \class CSpaceHole
*	\brief : this class hold the cylinder space hole's
*			index vertex buffer
*/
class CSpaceHole
{
	/*! \struct _MyVertex
	*	struct to define the vertex
	*	data (texture coordinate)
	*/
	typedef struct _MyVertex{
		D3DXVECTOR3	position;
		DWORD		color;
		float		tu0,tv0;
	}MyVertex;

	/*! \enum 
	*	FVF usage 
	*	\param
	*/
	enum{
		e_myVertexUse = ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 ), /*!< my rectangle vertex use*/
	};

private : //own data

	//@{ variable to construct cylinder
	//! cylinder height segments
	static const int				scm_iLayer			= 10;	

	//! cycle side points
	static const int				scm_iCyclePoint		= 24;

	//! total number of cycle point 
	static const int				scm_iTotalPoint		= scm_iLayer * (scm_iCyclePoint+1);

	//! total index size
	static const int				scm_iTotalIndexNum	= (scm_iLayer-1) * scm_iCyclePoint * 6 ;

	//! triangle number
	static const int				scm_iTriangleNum	= (scm_iLayer-1) * (scm_iCyclePoint * 2);
	//@}

	//@{ D3D resource
	//! d3d vertex buffer pointer
	LPDIRECT3DVERTEXBUFFER9			m_lpVertBuf;

	//! d3d vertex buffer for backup
	LPDIRECT3DVERTEXBUFFER9			m_lpVertBufBackup;

	//! d3d index buffer pointer
	LPDIRECT3DINDEXBUFFER9			m_lpIndexBuf;

	//! the space hole texture
	LPDIRECT3DTEXTURE9				m_lpTex;

	//! the space hole other texture
	LPDIRECT3DTEXTURE9				m_lpTex2;
	//@}

	//@{ variable to control the animation
	//! world coordinate
	D3DXVECTOR3						m_vecWorldPos;

	//! camera position
	D3DXVECTOR3						m_vecCamPos;
	D3DXVECTOR3						m_vecCamPosBackup;

	//! camera target
	D3DXVECTOR3						m_vecCamTarget;
	D3DXVECTOR3						m_vecCamTargetBackup;

	//! random direction of space hole
	float							m_fRotAgl_sin;
	float							m_fRotAgl_cos;

	//! current scale
	float							m_fCurrScale;

	//! ultra scale
	float							m_fScale;

	//! the hole is near end....
	BOOL							m_bHoleEnd;	

	//! difference of scale
	float							m_fDiffScale;

	////! offset of texture coordinate to show the animation
	////! of rotating 
	float							m_fOffset;

	//! the counter
	float							m_fCounter;

	//! reference z 
	//! this point will move from far of negative Z axes to the positive
	//! the cylinder will be waved according this point
	float							m_fRefZ;

	//! end flare light
	DWORD							m_dwLight;
	//@}

private: //own function (sub-function)

	/*!	\fn void FillLayerVertex(int _iLayer,MyVertex* _pVertBuf)
	*	\brief : create the vertexes of very layer
	*	
	*	\param _iLayer		: layer's index
	*	\param _pVertBuff	: vertexes of very layers data
	*	\return void
	*/
	void FillLayerVertex(int _iLayer,MyVertex* _pVertBuf);

	/*! \fn void FillIndexTriangle(int _iCycleRing,DWORD* _wRingIndex)
	*	\brief  : fill index triangle ring of cylinder
	*
	*	\param _iCycleRing		: the cycle ring index 
	*	\param _wRingIndex		: index buffer
	*/
	void FillIndexTriangle(int _iCycleRing,WORD* _wRingIndex);

	/*! \fn BOOL ProcessCylinder(void)
	*	\brief process the cylinder's vertexes to 
	*
	*	\param void
	*	\return void
	*/
	BOOL ProcessCylinder(void);

	/*! \fn void TransformLayerVert(MyVertex* _buffer)
	*	\brief Transfrom a layer 's veretexes
	*
	*	\param _iLayer : index of layer
	*	\param _bufferScr : pointer of every layer's vertexes(source coordinate)
	*	\param _bufferDest: pointer of every layer's vertexes(destiny coordinate)
	*	\return void
	*/
	void TransformLayerVert(int _iLayer,MyVertex* _bufferScr,MyVertex* _bufferDest);

public:	//constuctor and destructor
	/*! \fn constructor and destructor
	*	...
	*/
	CSpaceHole(void);
	~CSpaceHole(void);


public:	//public function
	
	/*! \fn void InitSpaceHole(LPDIRECT3DDEVICE9 _lpd3dDev)
	*	\brief : initialize the space hole(index buffer)
	*	
	*	\param _lpd3dDev	: d3d device pointer
	*	\param _vecWorldPos	: this space hole's world position
	*	\return void
	*/
	void InitSpaceHole(LPDIRECT3DDEVICE9 _lpd3dDev,D3DXVECTOR3& _vecWorldPos);


	/*! \fn void RenderFrame(LPDIRECT3DDEVICE9 _lpd3dDev)
	*	\brief : render the space hole(index vertex buffer)
	*
	*	\param _lpd3dDev	: d3d device pointer
	*	\return BOOL		: FALSE if render over
	*/
	BOOL RenderFrame(LPDIRECT3DDEVICE9 _lpd3dDev);

	/*! \fn void DestroySpaceHole(void)
	*	\brief : destroy the index buffer (release the resource)
	*
	*	\param _lpd3dDev : d3d device pointer
	*	\return void
	*/
	void DestroySpaceHole(void);
	

};

/*! \class	CGridRect
*	\brief : change the screen to many rectangles
*/
class CGridRect
{
private:
	/*! \enum 
     * rectangle color
     */
	enum{
		e_rectColor = D3DCOLOR_ARGB(255,255,255,255), /*!< rectangle color*/
	};
	/*! \struct _MyRectVertex
	*	struct to define the vertex of rectangle
	*	data (texture coordinate)
	*/
	typedef struct _MyRectVertex{
		D3DVECTOR	position;
		DWORD		color;
		float		tu0,tv0;
	}MyRectVertex;


	/*! \enum 
	*	FVF usage 
	*	\param
	*/
	enum{
		e_myRectVertexUse = ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 ), /*!< my rectangle vertex use*/
	};

	//! the number of rectangles
	//static const int			scm_iRectNum = 100;

private: //own private data

	//@{ resource of D3D
	//! back ground texture buffer(screen)
	LPDIRECT3DTEXTURE9			m_lpBgTexture;

	//! back ground texture2
	LPDIRECT3DTEXTURE9			m_lpBgTexture2;

	//! old back buffer buffer
	LPDIRECT3DSURFACE9			m_lpOldSurface;

	//! rectangle's buffer
	LPDIRECT3DVERTEXBUFFER9*	m_lppVertBuff;

	//! fog rectangle vertexes buffer
	LPDIRECT3DVERTEXBUFFER9			m_lpFogVertexBuf;

	//! pointer of CSpaceHole to show the space hole
	CSpaceHole					m_spaceHole;

	//@}

	//@{ variables of  metrics 
	//! the number of horizon grid
	int		m_iWidthNum;	

	//! the number of vertical grid
	int		m_iHeightNum;		
	
	//! width of texture in pixel (a whole screen)
	int		m_iWidth;

	//! height of texture in pixel (..)			
	int		m_iHeight;			

	//! width of grid 
	int		m_iGridWidth;	
	
	//! height of grid	
	int		m_iGridHeight;		

	//! the number of rectangles
	int		m_iGridNum;

	//! the center to rotate
	//! x
	int		m_iStartX;

	//! the center to rotate
	//! y
	int		m_iStartY;

	//@}


	//@{ state to manage

	//! is ready to render?
	BOOL	m_bReadyToRender;

	//! is first render 
	BOOL	m_bFirstRender;

	//! the counter
	DWORD	m_dwCounter;

	//! is through the hole?
	BOOL	m_bThroughHole;

	//! current fog level(0~255)
	DWORD	m_dwCurrFogLevel;

	//! is rectangle and grid (but space hole) render over?
	BOOL	m_bGridOver;

	//@}
	
private: //own private function

	/*! \fn BOOL CreateRect(LPDIRECT3DDEVICE9 _lpd3dDev,const int _index)
	*	create very rectangle in array and initialize them
	*	
	*	\param _lpd3dDev	d3d device to create the vertex buffer
	*	\param _index		index of array
	*/
	BOOL CreateRect(LPDIRECT3DDEVICE9 _lpd3dDev,const int _index);

public:	//constructor and destructor

	CGridRect(void);
	~CGridRect(void);

public:	//own function

	/*! \fn BOOL IsReadyToRender(void)const
	*	\brief : is ready to render (after initialize the resource)
	*
	*	\param void		
	*	\return BOOL	:  TRUE if after initializing
	*/
	BOOL IsReadyToRender(void)const{return m_bReadyToRender;}

	/*! \fn BOOL IsFirstRender(void)const
	*	brief : is first render to the texture target??
	*	
	*	\param void
	*	\return BOOL : TRUE is first render
	*/
	BOOL IsFirstRender(void)const{return m_bFirstRender;}

	/*!	\fn void SetReadyToRender(void)
	*	\brief : set the flag to begin render
	*
	*	\param void
	*	\return void
	*/
	void SetReadyToRender(void){m_bReadyToRender = TRUE;}

	/*! \fn void InitScreenWH(LPDIRECT3DDEVICE9 _lpd3dDev);
	*	\brief : this function wil  call in scene init
	*			it will record the width and height of screen
	*			and calculate the grid width and height and so on
	*			most important is set the bg texture's surface to the 
	*			render target,and begin to render all scene to the texture
	*			
	*	\param _iScreenWidth	: screen width
	*	\param _iScreenHeight	: screen height
	*	\return void
	*/
	void InitScreenWH(int _iScreenWidth,int _iScreenHeight);

	/*! \fn void InitGridRes(LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidthNum,int _iHeightNum)
	*	\brief : initialize the rectangle grid and 
	*			record the vertical and horizontal rectangles number
	*
	*	\param _lpd3dDev	: d3d device
	*	\param _iWidthNum	: horizontal number
	*	\param _iHeightNum	: vertical number
	*	\param _iStartX		: rotating center x
	*	\param _iStartY		: rotating center y
	*	\return void
	*/
	void InitGridRes(LPDIRECT3D9 _lp3d,LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidthNum,int _iHeightNum,int _iStartX,int _iStartY);

	/*! \fn void DestroyGridRes(LPDIRECT3DDEVICE9 _lpd3dDev);
	*	\brief : destroy the resource of grid(vertex buffer )
	*			and restore the surface;
	*	
	*	\param _lpd3dDev	: d3d device
	*	\return void
	*/
	void DestroyGridRes(void);

	/*! \fn void RenderFrame(LPDIRECT3DDEVICE9 _lpd3dDev);
	*	\brief : render every frame and don't through the middle pipe of OSOK engine
	*
	*	\param _lpd3dDev	: d3d device
	*	\return void
	*/
	void RenderFrame(LPDIRECT3DDEVICE9 _lpd3dDev);

	/*! \fn void ProcessRect(void)
	*	\brief : process the rectangles' vertex buffer in every frame(in space hole render)
	*
	*	\param void
	*	\return BOOL		: return the FALSE when rendering the space hole is over
	*/
	BOOL ProcessRect(void);

	/*! \fn void RestoreRenderTarget(void)
	*	\brief : restore the render target
	*
	*	\param  _lpd3dDev : d3d device to restore
	*	\return void
	*/
	void RestoreRenderTarget(LPDIRECT3DDEVICE9 _lpd3dDev);
	
	/*! \fn void ProcessARect(MyRectVertex* _grid)
	*	\brief : process a grid's position
	*
	*	\param _index	: index of grids
	*	\param _grid	: grid's buffer pointer
	*	\return void 
	*/
	void ProcessARect(const int _index,MyRectVertex* _grid);

	/*! \fn BOOL CreateFogRect(LPDIRECT3DDEVICE9 _lpd3dDev)
	*	\brief : create the fog rectangle to blend with the next scene
	*
	*	\param _lpd3dDev : d3d device
	*	\return BOOL		: FALSE if failed
	*/
	BOOL CreateFogRect(LPDIRECT3DDEVICE9 _lpd3dDev);

	/*! \fn void RenderThroughFog(LPDIRECT3DDEVICE9 _lpd3dDev)
	*	\brief : render the fog after throughing the hole
	*
	*	\param _lpd3dDev		: d3d device
	*	\return void
	*/
	void RenderThroughFog(LPDIRECT3DDEVICE9 _lpd3dDev);
};
