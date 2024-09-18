//===============================================================================
/*! \file
*	Filename:	coronaEff.cpp
*
*	Desc:		This effect is draw some corona around the object.
*				pass.1 : draw the object onto a texure with blur alpha
*				pass.2 : draw the rectangle with pure white pixel blur(alpha value too)
*			
*	His:		tzz	2008-7-3
*/
//===============================================================================


# include "stdafx.h"
# include ".\coronaEff.h"

# include ".\galaEffect_common.h"



//====================================================================================================================
//
//	 CCoronaEff class 
//
//====================================================================================================================

//--------------------------------------------------------------------------------------------------------------------
//Name: constructor and destructor
//Desc: ...
//--------------------------------------------------------------------------------------------------------------------
CCoronaEff::CCoronaEff(void):m_lpOrigTex(NULL),
									m_lpOrigSurface(NULL),
									m_lpCoronaTex(NULL),
									m_lpCoronaSurface(NULL),
									m_dwCoronaColor(0xffffffff)
{
	//nothing to do now
}
CCoronaEff::~CCoronaEff(void)
{
	DestroyEffRes();

}
void CCoronaEff::DestroyEffRes(void)
{
	// call the base class' function
	// to destroy the resource
	CGalaEff::DestroyEffRes();

	SAFE_RELEASE(m_lpOrigTex);
	SAFE_RELEASE(m_lpOrigSurface);
	SAFE_RELEASE(m_lpCoronaTex);
	SAFE_RELEASE(m_lpCoronaSurface);
}
//--------------------------------------------------------------------------------------------------------------------
//Name: InitCoronaEff
//Desc: initialize the corona effect data
//--------------------------------------------------------------------------------------------------------------------
BOOL CCoronaEff::InitCoronaEff(LPDIRECT3DDEVICE9 _lpd3dDev,int _iWidth,int _iHeight,D3DFORMAT _format /* =  D3DFMT_A8R8G8B8*/)
{
	guard;

	//initiailze the base class' data
	InitEffScene(_lpd3dDev,_iWidth,_iHeight,_format);

	//create the texture to render on it
	VALIDATE_D3D(_lpd3dDev->CreateTexture((UINT)_iWidth,(UINT)_iHeight,1,
										D3DUSAGE_RENDERTARGET,_format,D3DPOOL_DEFAULT,&m_lpOrigTex,NULL));
	osassert(m_lpOrigTex);

	VALIDATE_D3D(m_lpOrigTex->GetSurfaceLevel(0,&m_lpOrigSurface));

	//create the blur texture which blur in fx file
	VALIDATE_D3D(_lpd3dDev->CreateTexture((UINT)_iWidth,(UINT)_iHeight,1,
										D3DUSAGE_RENDERTARGET,_format,D3DPOOL_DEFAULT,&m_lpCoronaTex,NULL));
	osassert(m_lpCoronaTex);

	VALIDATE_D3D(m_lpCoronaTex->GetSurfaceLevel(0,&m_lpCoronaSurface));

	//create the rectangle 
	CreateRect(_lpd3dDev);

	//initilize the shader vertex declaration
	InitEffect(_lpd3dDev,"data\\effect\\corona.fx");


	return TRUE;

	unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name: SetEffective
//Desc: set the effective
//--------------------------------------------------------------------------------------------------------------------
void CCoronaEff::SetEffective(LPDIRECT3DDEVICE9 _lpd3dDev,BOOL _bEffective /* = TRUE*/)
{
	guard;

	//pre-condition to confirm has been initialized
	osassert(sm_iWidth && m_lpOrigTex);

	
	if(_bEffective){
		
		if(!m_bEffective){	// not active

			SAFE_RELEASE(m_lpMainSurface);

			VALIDATE_D3D(_lpd3dDev->GetRenderTarget(0,&m_lpMainSurface));

			osassert(m_lpOrigSurface);
			// set the render target to the scene texture
			VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0,m_lpOrigSurface));

			// clear the original texture surface
			// in order to prepare the draw corona
			VALIDATE_D3D(_lpd3dDev->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_ARGB(0,0,0,0),0.0f,0));

		}

	}else{
		if(m_bEffective){	// active
			//set the main rendering surface back 
			osassert(m_lpMainSurface);
			if(FAILED(_lpd3dDev->SetRenderTarget(0,m_lpMainSurface))){
				osassert(false);
			}

		}
	}
	
	m_bEffective = _bEffective;

	unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name: RenderEffScene
//Desc: overload the base class function to render
//--------------------------------------------------------------------------------------------------------------------
void CCoronaEff::RenderEffScene(LPDIRECT3DDEVICE9 _lpd3dDev,bool _alpha)
{
	guard;

	/*if(!IsEffective()){
		return;
	}*/

	PrepareEffScene(_lpd3dDev);

	VALIDATE_D3D(m_lpEffect->SetTexture("g_origTex",m_lpOrigTex));


	UINT t_uiPass = 0;
	VALIDATE_D3D(m_lpEffect->Begin(&t_uiPass,0));
	osassert(t_uiPass == 2);
	{
		{
			// pass 1 : draw a blur alpha pure rectangle
			//			check detail in fx file

			VALIDATE_D3D(m_lpEffect->BeginPass(0));

			VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0,m_lpCoronaSurface));

			VALIDATE_D3D(_lpd3dDev->Clear(0,NULL,D3DCLEAR_TARGET,0,0.0f,0));

			VALIDATE_D3D(_lpd3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2));

			VALIDATE_D3D(m_lpEffect->EndPass());

		}


		VALIDATE_D3D(m_lpEffect->SetTexture("g_blurTex",m_lpCoronaTex));

		{
			// pass 2 : draw 

			VALIDATE_D3D(m_lpEffect->BeginPass(1));
			
			//set back main surface
			VALIDATE_D3D(_lpd3dDev->SetRenderTarget(0,m_lpMainSurface));
			
			VALIDATE_D3D(_lpd3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2));
			VALIDATE_D3D(m_lpEffect->EndPass());

		}
	}
	VALIDATE_D3D(m_lpEffect->End());

	//D3DXSaveSurfaceToFile("Main_Surface.tga",D3DXIFF_TGA,m_lpMainSurface,NULL,NULL);

	unguard;
}
