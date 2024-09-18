//===============================================================================
/*! \file
*	Filename:	galaEffect_common.h
*	Desc:		some helper function for galaxy effect
*	His:		tzz	2008-7-1
*/
//===============================================================================

# pragma once


# include <d3dx9.h>

# define VALIDATE_D3D(exp)	if(FAILED(exp)){ \
									osassert(false);\
							}

//! delcare for set the ortho camera function
HRESULT SetOrthoCamera(LPDIRECT3DDEVICE9 _lpd3dDev,const D3DXVECTOR3& _vectPos,
					   const D3DXVECTOR3& _vectTarget,float _fWidth,float _fHeight,
					   float _fNearZ, float _fFarZ,D3DXMATRIX* _matView = NULL ,D3DXMATRIX* _matProject = NULL);


//! delcare for set the projection camera function
HRESULT SetCamera(LPDIRECT3DDEVICE9 _lpd3dDev,const D3DXVECTOR3& _vectPos,
				  const D3DXVECTOR3& _vectTarget,float _fFov,float _fAspectRatio,
				  float _fNearClip,float _fFarClip,D3DXMATRIX* _matView = NULL, D3DXMATRIX* _matProject = NULL);
