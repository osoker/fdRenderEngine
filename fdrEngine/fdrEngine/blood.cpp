//===============================================================================
/*! \file
*	Filename:	blood.cpp
*	Desc:		simple class to display the process of blood
*				
*	His:		tzz	2008-7-11
*/
//===============================================================================
# include "stdafx.h"
# include "blood.h"
# include "galaEffect_common.h"



//====================================================================================================================
//
//	CAirCraftAI class(base class)
//
//====================================================================================================================

//! static variable initialize
LPDIRECT3DTEXTURE9 CBlood::sm_lpBloodTex = NULL;

//--------------------------------------------------------------------------------------------------------------------
//Name: constructor and destructor
//Desc: ...
//--------------------------------------------------------------------------------------------------------------------
CBlood::CBlood():m_fMaxHP(0.0f),
				m_fCurrHP(0.0f),
				m_fCraftScale(1.0f),
				m_fScrPos_x(0.0f),
				m_fScrPos_y(0.0f),
				m_lpVertexBuf(NULL)
{

}
CBlood::~CBlood(void)
{
	DestoryRes();	
}
void CBlood::DestoryRes(void)
{
	
	SAFE_RELEASE(m_lpVertexBuf);
	
}



//--------------------------------------------------------------------------------------------------------------------
//Name:LoadBloodTexture
//Desc: //! load the texture
//--------------------------------------------------------------------------------------------------------------------
BOOL CBlood::LoadBloodTexture(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	guard;

	osassert(_lpd3dDev && !sm_lpBloodTex);

	//VALIDATE_D3D(D3DXCreateTextureFromFile(_lpd3dDev,"ui\\galaxy\\blood_64_8.tga",&sm_lpBloodTex));

	return TRUE;

	unguard;
}

//--------------------------------------------------------------------------------------------------------------------
//Name:RefreshBlood
//Desc: refresh the blood the position
//--------------------------------------------------------------------------------------------------------------------
void CBlood::RefreshBlood(const D3DXVECTOR2& _vecScreenPos,float _fCraftScale,float _fMaxHP,float _fCurrHP)
{
	m_fScrPos_x		= _vecScreenPos.x;
	m_fScrPos_y		= _vecScreenPos.y;

	m_fCraftScale	= _fCraftScale;
	m_fMaxHP		= _fMaxHP;
	m_fCurrHP		= _fCurrHP;

	Activate();
}

//--------------------------------------------------------------------------------------------------------------------
//Name:InitBlood
//Desc: initialzie the vertexbuffer
//--------------------------------------------------------------------------------------------------------------------
void CBlood::InitBlood(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	guard;

	osassert(_lpd3dDev && !m_lpVertexBuf);

	const int t_iLen = 4 * sizeof(MyVertex);
	
	VALIDATE_D3D(_lpd3dDev->CreateVertexBuffer(4 * sizeof(MyVertex),
											D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
											e_myVertexUse,
											D3DPOOL_DEFAULT,
											&m_lpVertexBuf,NULL));

	osassert(m_lpVertexBuf);

	MyVertex*	t_pBuffer = NULL;
	VALIDATE_D3D(m_lpVertexBuf->Lock(0,t_iLen,(void**)&t_pBuffer,0));
	
	const float t_width			= 32.0f;
	const float t_height		= 2.0f;
	
	//left-bottom corner position
	const float t_pos_x			= 0.0f;
	const float t_pos_y			= 0.0f;
	const float t_pos_u0		= 0.0f;
	const float t_pos_v0		= 0.0f;

	

	//delta of rectangle offset
	const float t_pos_delta_u	= 0.5f;
	const float t_pos_delta_v	= 1.0f;

	//top-left
	t_pBuffer[0].color		= e_rectColor;
	t_pBuffer[0].position.x = t_pos_x ;
	t_pBuffer[0].position.y = t_pos_y;
	t_pBuffer[0].position.z = 0.5f;
	//t_pBuffer[0].tu0		= t_pos_u0;
	//t_pBuffer[0].tv0		= t_pos_v0 ;
	
	//top-right
	t_pBuffer[1].color		= e_rectColor;
	t_pBuffer[1].position.x = t_pos_x; 
	t_pBuffer[1].position.y = t_pos_y - t_height;
	t_pBuffer[1].position.z = 0.5f;
	//t_pBuffer[1].tu0		= t_pos_u0;
	//t_pBuffer[1].tv0		= t_pos_v0 + t_pos_delta_v;
	

	//bottom-left
	t_pBuffer[2].color		= e_rectColor;
	t_pBuffer[2].position.x = t_pos_x + t_width;
	t_pBuffer[2].position.y = t_pos_y ;
	t_pBuffer[2].position.z = 0.5f;
	//t_pBuffer[2].tu0		= t_pos_u0 + t_pos_delta_u;
	//t_pBuffer[2].tv0		= t_pos_v0 ;
	

	//bottom-right
	t_pBuffer[3].color		= e_rectColor;
	t_pBuffer[3].position.x = t_pos_x + t_width;
	t_pBuffer[3].position.y = t_pos_y - t_height;
	t_pBuffer[3].position.z = 0.5f;
	//t_pBuffer[3].tu0		= t_pos_u0 + t_pos_delta_u;
	//t_pBuffer[3].tv0		= t_pos_v0 + t_pos_delta_v;
	
	VALIDATE_D3D(m_lpVertexBuf->Unlock());


	unguard;
}
//--------------------------------------------------------------------------------------------------------------------
//Name:InitBlood
//Desc: initialzie the vertexbuffer
//--------------------------------------------------------------------------------------------------------------------
void CBlood::DisplayBlood(LPDIRECT3DDEVICE9 _lpd3dDev)
{
	guard;

	if(!IsActive()){
		return;
	}

	// needn't set the d3d device state and texture
	// because the it was set outside together

	// offset of blood slider
	const float t_offset_x = -5.0f * m_fCraftScale;
	const float t_offset_y = 20.0f * m_fCraftScale;

	D3DXMATRIX t_mat,t_scaleMat;
	D3DXMatrixIdentity(&t_mat);
	D3DXMatrixIdentity(&t_scaleMat);
	D3DXMatrixTranslation(&t_mat,m_fScrPos_x + t_offset_x,m_fScrPos_y + t_offset_y,0.0f);
	D3DXMatrixScaling(&t_scaleMat,m_fCurrHP / m_fMaxHP,1.0f,1.0f);
	D3DXMatrixMultiply(&t_mat,&t_scaleMat,&t_mat);
	
	VALIDATE_D3D(_lpd3dDev->SetTransform(D3DTS_WORLD,&t_mat));

	//D3DXMatrixIdentity(&t_mat);
	//t_mat._31 = 0.5f * (m_fMaxHP - m_fCurrHP) / m_fMaxHP;

	//VALIDATE_D3D(_lpd3dDev->SetTransform(D3DTS_TEXTURE0,&t_mat));
	
	VALIDATE_D3D(_lpd3dDev->SetStreamSource(0,m_lpVertexBuf,0,sizeof(MyVertex)));

	VALIDATE_D3D(_lpd3dDev->SetFVF(e_myVertexUse));

	VALIDATE_D3D(_lpd3dDev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2));


	unguard;
}



