
//================================================================================================================
//================================================================================================================
//================================================================================================================
//
//   中间管道的Inline函数.
//
//================================================================================================================
//================================================================================================================
//================================================================================================================


//@{


//! 设置sampleState为reapeat状态，在渲染地表的时候，使用clamp状态，调用这个函数
//! 设置为reapeat状态。
inline void osc_middlePipe::set_ssRepeat( void )
{
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP );

	m_pd3dDevice->SetSamplerState( 1,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
	m_pd3dDevice->SetSamplerState( 1,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP );
}

//! 设置为clamp状态，在渲染地图的时候，需要设置为clamp状态。
inline void osc_middlePipe::set_ssClamp( void )
{
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP );

	m_pd3dDevice->SetSamplerState( 1,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP );
	m_pd3dDevice->SetSamplerState( 1,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP );
}






//! 释放当前的静态顶点缓冲区。
inline void osc_middlePipe::release_staticVB( int _vbIdx )
{
	osassert( m_vecVerBuf[_vbIdx].m_bInuse );
//	if( !m_vecVerBuf.validate_id( _vbIdx ) )
//		return;
	if( m_vecVerBuf[_vbIdx].m_bInuse )
	{
		m_vecVerBuf[_vbIdx].reset_buf();
		m_vecVerBuf.release_node( _vbIdx );
	}
}



/** \brief
*  reset MiddlePipe前需要释放的数据.
*/
inline void osc_middlePipe::mdpipe_onLostDevice( void )
{
	m_shaderMgr.shaderMgr_onLostDevice();
	release_dynamicIB();
	release_dynamicVB( false );
	osDebugOut( "处理设备丢失.....\n" );
}




/** \brief
*  为上层提供mesh的渲染函数。
*  
*  \param _mr 渲染mesh所填充的结构。
*/
inline bool  osc_middlePipe::render_mesh( os_meshRender* _mr,bool _alpha/* = false*/  )
{
	return m_meshMgr.render_mesh( _mr,_alpha );
}



//! 设置fvf值。
inline void osc_middlePipe::set_fvf( DWORD _fvf )
{
	HRESULT  t_hr;
	t_hr = m_pd3dDevice->SetFVF( _fvf );
	if( FAILED( t_hr ) )
		osassertex( false,va("设置fvf失败,原因<%s>..\n",osn_mathFunc::get_errorStr( t_hr )) );
}




/** \brief
*  每一帧上层需要调用的keyAniMesh渲染函数。
*/
inline bool  osc_middlePipe::frame_renderKeyAniMesh( 
	         os_ViewFrustum* _vf,bool _alpha,os_FightSceneRType _rtype/* = OS_COMMONRENDER*/ )
{
	return m_meshMgr.render_aniMesh( _vf,_alpha,_rtype );
}

//! 处理水面的渲染
inline bool osc_middlePipe::render_aniWater( os_ViewFrustum* _vf )
{
	return m_meshMgr.render_waterAni();
}
//! 处理雾的渲染
inline bool osc_middlePipe::render_aniFog( int ShaderId,int dwtype /*= 2*/ )
{
	return m_meshMgr.render_FogAni(ShaderId,dwtype);
}

//@{
/** \brief
*  简单的设置3d设备状态函数.
*/
inline void osc_middlePipe::set_worldMatrix( osMatrix& _mat )
{
	HRESULT     t_hr;

	if( FAILED( t_hr = m_pd3dDevice->SetTransform( D3DTS_WORLD,&_mat ) ) )
	{
		osassertex( false,
			va( "设置世界矩阵失败,原因<%s>..\n",osn_mathFunc::get_errorStr( t_hr ) ) );
	}
}
inline void osc_middlePipe::set_viewMatrix( osMatrix& _mat )
{
	HRESULT     t_hr;

	t_hr = m_pd3dDevice->SetTransform( D3DTS_VIEW,&_mat );
	if( FAILED( t_hr ) )
		osassertex( false,(char*)osn_mathFunc::get_errorStr( t_hr ) );
}

inline void osc_middlePipe::set_projMatrix( osMatrix& _mat )
{
	HRESULT    t_hr;

	t_hr = m_pd3dDevice->SetTransform( D3DTS_PROJECTION,&_mat );
	if( FAILED( t_hr ) )
		osassertex( false,(char*)osn_mathFunc::get_errorStr( t_hr ) );
}

inline void osc_middlePipe::set_viewport( D3DVIEWPORT9& _vp )
{
	HRESULT   t_hr;

	if( FAILED( t_hr = m_pd3dDevice->SetViewport( & _vp ) ) )
		osassertex( false,va("设置vp失败,原因<%s>",osn_mathFunc::get_errorStr( t_hr ) ) );

	return;
}
/** \brief
*  3d 填充状态的设置使用这个函数,可以设置是线框模式或是实体模式.
*
*  \param _mode:  1:point. 2:是线框模式.3: 为实体模式.
*/
inline void osc_middlePipe::set_deviceFillMode( int _mode )
{
	HRESULT      t_hr;
	D3DFILLMODE  t_fill;

	if( _mode == 1 )
		t_fill = D3DFILL_POINT;
	else if( _mode == 2 )
		t_fill = D3DFILL_WIREFRAME;
	else if( _mode == 3 )
		t_fill = D3DFILL_SOLID;
	else 
		t_fill = D3DFILL_FORCE_DWORD;


	t_hr = m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,t_fill );
	if( FAILED( t_hr ) )
		osassertex( false,(char*)osn_mathFunc::get_errorStr( t_hr ) );
}

//@}


//@{
/** \brief 
*  从中间管道得到一些渲染信息.
*/
inline int osc_middlePipe::get_renderTriNum( void )
{
	return m_frontRender.get_renderTrinum();
}

inline int osc_middlePipe::get_meshFaceNum( void )
{
	return m_meshMgr.get_rFaceNum();
}

inline int osc_middlePipe::get_renderVerNum( void )
{
	return m_frontRender.get_renderVernum();
}


inline void osc_middlePipe::set_nullStream( void )
{
	m_frontRender.reset_vbufCatch();
}

//! 设置dx的灯光相关数据。
inline void osc_middlePipe::set_dxLight( D3DLIGHT9* _dxL,int _num )
{
	osassert( _dxL );

	int t_i=0;
	for( t_i=0;t_i<_num;t_i++ )
	{
		m_pd3dDevice->SetLight( t_i,&_dxL[t_i] );
		m_pd3dDevice->LightEnable( t_i,true );
	} 
	m_pd3dDevice->LightEnable( t_i,false );

}


//! 处理aniMesh数据的动态光
inline void osc_middlePipe::process_amDlight( os_dLightData& _dlight )
{
	m_meshMgr.process_aniMeshDlight( _dlight );
}

inline void osc_middlePipe::set_material( D3DMATERIAL9& _mat )
{
	HRESULT  t_hr;
	t_hr = m_pd3dDevice->SetMaterial( &_mat );
	if( FAILED( t_hr ) )
		osassertex( false,va("SetMaterial failed<%s>..\n",osn_mathFunc::get_errorStr( t_hr )) );

}

//! 关闭dx的灯光.
inline void osc_middlePipe::set_dxLightDisable( int _num )
{
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,false ); 

	for( int t_i=0;t_i<_num;t_i++ )
		m_pd3dDevice->LightEnable( t_i,false );
}



inline void osc_middlePipe::set_renderState( D3DRENDERSTATETYPE _State,DWORD _Value )
{
	m_pd3dDevice->SetRenderState( _State,_Value );
}

inline void osc_middlePipe::get_renderState( D3DRENDERSTATETYPE _State,DWORD* _Value )
{
	m_pd3dDevice->GetRenderState( _State,_Value );
}

inline void osc_middlePipe::set_textureStageState( DWORD _Stage,D3DTEXTURESTAGESTATETYPE _Type,DWORD _Value )
{
	m_pd3dDevice->SetTextureStageState( _Stage,_Type,_Value );
}
# ifdef _DEBUG
inline void osc_middlePipe::get_textureStageState( DWORD _Stage,D3DTEXTURESTAGESTATETYPE _Type,DWORD* _Value )
{
	m_pd3dDevice->GetTextureStageState( _Stage,_Type,_Value );
}
# endif 

inline void osc_middlePipe::set_sampleState( DWORD _sample,D3DSAMPLERSTATETYPE _state,DWORD _value )
{
	m_pd3dDevice->SetSamplerState( _sample,_state,_value );
}


//@}

