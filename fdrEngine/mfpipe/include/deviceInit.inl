



/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// osc_d3dManager���Inline����.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////



inline void osc_d3dManager::d3d_enddraw( void )
{
	// 
	// �ͻ�����beginScene��endScene֮���г����л��Ĵ���
	// �ڳ������л������У�����ҪbeginScene��endScene,
	//  
	// ���ֽ����������򵥵Ľ����������������õķ�����
	if( m_bInDrawScene )
		m_bInDrawScene = false;
	else
		return;

	HRESULT   t_hr;
	if( FAILED( t_hr = m_pd3dDevice->EndScene() ) )
	{
		OutputDebugString( DXGetErrorString9( t_hr ) );
		osassert( false );
	}
}

//! river added @ 2010-3-1:���ڶര����Ⱦ
inline void osc_d3dManager::d3d_addSwapClear( DWORD _Count,
									  CONST D3DRECT* _pRects,
									  DWORD _Flags,
									  D3DCOLOR _Color )
{
	HRESULT  t_hr;
	osassert( this->m_ptrAddSwapChain );

	LPDIRECT3DSURFACE9 t_pBack=NULL;
	
	if( FAILED( t_hr = m_ptrAddSwapChain->GetBackBuffer( 0,
		D3DBACKBUFFER_TYPE_MONO,&t_pBack ) ) )
		osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );

	if( FAILED( t_hr = m_pd3dDevice->SetRenderTarget( 0,t_pBack ) ) )
		osassertex( false,osn_mathFunc::get_errorStr( t_hr ) );

	if( FAILED(  t_hr = m_pd3dDevice->Clear( _Count,_pRects,_Flags,
		            _Color,1,0 ) ) )
	{
		osassertex( false,(char*)osn_mathFunc::get_errorStr( t_hr ) );
	}
	m_bInRender = true;

	t_pBack->Release();
}
inline BOOL osc_d3dManager::d3d_isSwapPrepare(void)const
{
	return m_ptrAddSwapChain != NULL;
}


//! �����м�ܵ���ָ��������������ָ��.���ڴ���device reset�����.
inline void osc_d3dManager::set_deviceResetNeedRes( 
	    osc_middlePipe* _mptr,osc_skinMeshMgr* _cptr )
{
	osassert( _mptr );
	osassert( _cptr );
	m_ptrMiddlePipe = _mptr;
	m_ptrSmMgr = _cptr;
}


//! �����Ƿ����ع�ꡣ
inline void osc_d3dManager::hide_corsor( bool _hide )
{
	if( _hide == m_bHideCursor )
		return;
	m_bHideCursor = _hide;
	m_pd3dDevice->ShowCursor( !_hide );
}




/** \brief
*  ����3d Device��Gamma ֵ.
*/
inline void osc_d3dManager::set_gamma( const os_gamma* _gamma )
{
    D3DGAMMARAMP   gamma;

	memcpy( &gamma,_gamma,sizeof( D3DGAMMARAMP ) );
	m_pd3dDevice->SetGammaRamp( 0, D3DSGR_NO_CALIBRATION ,&gamma );

}


//@{
/**
*  �õ�3d Deivce.
*/
inline LPDIRECT3DDEVICE9 osc_d3dManager::get_d3ddevice( void )
{
	return m_pd3dDevice;
}

inline LPDIRECT3D9 osc_d3dManager::get_d3d( void )
{
	return m_pD3D;
}
//@}


//@{
/**
*  Not open for upper layer.inner part will use these func.
*/
inline int osc_d3dManager::get_scrwidth( void )
{
	return m_iScrWidth;
}

inline int osc_d3dManager::get_scrheight( void )
{
	return m_iScrHeight;
}
inline int osc_d3dManager::get_scrbpp( void )
{
	return m_iScrBpp;
}
inline RECT osc_d3dManager::get_windowBounds( void )
{
	return m_rcWindowBounds;
}
inline bool osc_d3dManager::get_fullScreen( void )
{
	return !m_bWindowed;
}


/** \brief
*  �õ������豸��Stencil bits number.
*/
inline int osc_d3dManager::get_stencilBitNum( void )
{
	return m_iStencilBitsNum;
}

/** \brief
*  �õ������豸��clip plane����Ŀ��
*/
inline int osc_d3dManager::get_maxClipPlane( void )
{
	return m_iMaxClipPlane;
}


//@}


/** \brief
*  �õ���ǰ���豸�Ƿ������Vertex Shader.
*/
inline bool osc_d3dManager::is_softwareVS( void )
{

	if( ( m_eVPT == SOFTWARE_VP) ||
		( m_eVPT == MIXED_VP )   )
		return true;
	else
		return false;

}


/** \brief
*  �õ���ǰ����ͨ���崦���Ƿ�����Ӳ��TNL
*/
inline bool osc_d3dManager::is_hardwareTL( void )
{
	if( m_eVPT == SOFTWARE_VP )
		return false;
	else
		return true;
}


/** \brief
*  �ϲ�õ�fade�İٷֱ�;
*  
*   ���ڿͻ��˴���һЩ�¼������Ҫfadeout����fadeIn��ʱ�򣬲�����Ч��������Ϣ�ȡ�
*/
inline float osc_d3dManager::get_fullscrFadePercent( void )
{
	return m_fElaFadeTime/m_fFadeTime;
}

