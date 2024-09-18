//===============================================================================
/*! \file
*	Filename:	galaEffect_common.cpp
*	Desc:		some helper function for galaxy effect
*	His:		tzz	2008-7-1
*/
//===============================================================================
#include "stdafx.h"
# include ".\galaEffect_common.h"




//--------------------------------------------------------------------------------------------------------------------
//Name: SetOrthoCamera
//Desc: set the OrthoCamera to watch the world
//--------------------------------------------------------------------------------------------------------------------
HRESULT SetOrthoCamera(LPDIRECT3DDEVICE9 _lpd3dDev,const D3DXVECTOR3& _vectPos,
					   const D3DXVECTOR3& _vectTarget,float _fWidth,float _fHeight,float _fNearZ, 
					   float _fFarZ,D3DXMATRIX* _matView /* = NULL*/,D3DXMATRIX* _matProject /*= NULL*/)
{
	guard;

	HRESULT		t_re;
	D3DXMATRIX	t_mat;

	//set the view matrix
	D3DXMatrixIdentity(&t_mat);
	const D3DXVECTOR3 t_vectUp(0.0f,1.0f,0.0f);
	D3DXMatrixLookAtLH(&t_mat,&_vectPos,&_vectTarget,&t_vectUp);
	
	//copy the view matrix
	if(_matView){
		memcpy(_matView,t_mat,sizeof(t_mat));
	}
	t_re = _lpd3dDev->SetTransform(D3DTS_VIEW,&t_mat);
	

	D3DXMatrixIdentity(&t_mat);
	//set the project matrix
	D3DXMatrixOrthoLH(&t_mat,_fWidth,_fHeight,_fNearZ,_fFarZ);
	osassert(_lpd3dDev);
	
	//copy the projection matrix
	if(_matProject){
		memcpy(_matProject,&t_mat,sizeof(t_mat));
	}
	t_re &= _lpd3dDev->SetTransform(D3DTS_PROJECTION,&t_mat);

	
	return t_re;

	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name: SetCamera
//Desc: set the Normal projection Camera to watch the world
//--------------------------------------------------------------------------------------------------------------------
HRESULT SetCamera(LPDIRECT3DDEVICE9 _lpd3dDev,const D3DXVECTOR3& _vectPos,
				  const D3DXVECTOR3& _vectTarget,float _fFov,float _fAspectRatio,float _fNearClip,
				  float _fFarClip,D3DXMATRIX* _matView /* = NULL*/,D3DXMATRIX* _matProject /*= NULL*/)
{
	assert(_lpd3dDev);
	const D3DXVECTOR3 t_vectUp(0.0f,1.0f,0.0f);
	D3DXMATRIX t_mat;
	D3DXMatrixIdentity(&t_mat);
	D3DXMatrixLookAtLH(&t_mat,&_vectPos,&_vectTarget,&t_vectUp);
	HRESULT t_re=_lpd3dDev->SetTransform(D3DTS_VIEW,&t_mat);

	//copy the view matrix
	if(_matView){
		memcpy(_matView,t_mat,sizeof(t_mat));
	}

	D3DXMatrixIdentity(&t_mat);
	D3DXMatrixPerspectiveFovLH(&t_mat,_fFov,_fAspectRatio,_fNearClip,_fFarClip);
	t_re &= (_lpd3dDev->SetTransform(D3DTS_PROJECTION,&t_mat));

	//copy the projection matrix
	if(_matProject){
		memcpy(_matProject,t_mat,sizeof(t_mat));
	}

	return t_re;

}