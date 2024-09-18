//===============================================================================
/*! \file
*	Filename:	blood.h	
*	Desc:		simple class to display the process of blood
*				
*	His:		tzz	2008-7-11
*/
//===============================================================================

# pragma once

# include <d3dx9.h>

/*! \class CBlood
*	\brief show the process of blood
*/
class CBlood
{

	//@{ state variable
	//! is active?
	BOOL		m_bActive;

	//! curr hp
	float		m_fCurrHP;

	//! max hp
	float		m_fMaxHP;

	//! model scale to calculate the height of blood process
	float		m_fCraftScale;

	//! screen position
	float		m_fScrPos_x;
	float		m_fScrPos_y;
	//@}

	//@{ d3d resource

	typedef struct _MyVertex{
		D3DXVECTOR3		position;
		DWORD			color;
		//float			tu0,tv0;
	}MyVertex;

	enum{ e_myVertexUse = (D3DFVF_XYZ | D3DFVF_DIFFUSE ),e_rectColor = D3DCOLOR_XRGB(255,0,0)};

	//! vertex buffer
	LPDIRECT3DVERTEXBUFFER9			m_lpVertexBuf;

	//@}

public:

	//! texture all object of blood use only one
	static LPDIRECT3DTEXTURE9		sm_lpBloodTex;

	//@}
public: // constructor and destructor

	/*! \fn constructor and destructor
	*	...
	*/
	CBlood(void);
	~CBlood(void);

public:
	//! load the texture
	static BOOL LoadBloodTexture(LPDIRECT3DDEVICE9 _lpd3dDev);

	//! is active??
	BOOL IsActive(void){return m_bActive;}

	//! Activate
	void Activate(BOOL _bAct = TRUE){m_bActive = _bAct;}

	//! init the blood struct
	void InitBlood(LPDIRECT3DDEVICE9 _lpd3dDev);

	//! initialize this class
	void RefreshBlood(const D3DXVECTOR2& _vecScreenPos,float _fCraftScale,float _fMaxHP,float _fCurrHP);

	//! display the blood process
	void DisplayBlood(LPDIRECT3DDEVICE9 _lpd3dDev);

	//! destory the blood resource
	void DestoryRes(void);
	

};