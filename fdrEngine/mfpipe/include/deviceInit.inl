



/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// osc_d3dManager类的Inline函数.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////



inline void osc_d3dManager::d3d_enddraw( void )
{
	// 
	// 客户端在beginScene和endScene之间有场景切换的处理，
	// 在场景的切换处理中，又需要beginScene和endScene,
	//  
	// 这种解决方案是最简单的解决方案，但不是最好的方案。
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

//! river added @ 2010-3-1:用于多窗口渲染
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


//! 设置中间管道的指针和人物管理器的指针.用于处理device reset的情况.
inline void osc_d3dManager::set_deviceResetNeedRes( 
	    osc_middlePipe* _mptr,osc_skinMeshMgr* _cptr )
{
	osassert( _mptr );
	osassert( _cptr );
	m_ptrMiddlePipe = _mptr;
	m_ptrSmMgr = _cptr;
}


//! 设置是否隐藏光标。
inline void osc_d3dManager::hide_corsor( bool _hide )
{
	if( _hide == m_bHideCursor )
		return;
	m_bHideCursor = _hide;
	m_pd3dDevice->ShowCursor( !_hide );
}




/** \brief
*  设置3d Device的Gamma 值.
*/
inline void osc_d3dManager::set_gamma( const os_gamma* _gamma )
{
    D3DGAMMARAMP   gamma;

	memcpy( &gamma,_gamma,sizeof( D3DGAMMARAMP ) );
	m_pd3dDevice->SetGammaRamp( 0, D3DSGR_NO_CALIBRATION ,&gamma );

}


//@{
/**
*  得到3d Deivce.
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
*  得到创建设备的Stencil bits number.
*/
inline int osc_d3dManager::get_stencilBitNum( void )
{
	return m_iStencilBitsNum;
}

/** \brief
*  得到创建设备的clip plane的数目。
*/
inline int osc_d3dManager::get_maxClipPlane( void )
{
	return m_iMaxClipPlane;
}


//@}


/** \brief
*  得到当前的设备是否是软件Vertex Shader.
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
*  得到当前的普通物体处理是否是用硬件TNL
*/
inline bool osc_d3dManager::is_hardwareTL( void )
{
	if( m_eVPT == SOFTWARE_VP )
		return false;
	else
		return true;
}


/** \brief
*  上层得到fade的百分比;
*  
*   用于客户端处理一些事件，如快要fadeout或是fadeIn的时候，播放特效，发送消息等。
*/
inline float osc_d3dManager::get_fullscrFadePercent( void )
{
	return m_fElaFadeTime/m_fFadeTime;
}

