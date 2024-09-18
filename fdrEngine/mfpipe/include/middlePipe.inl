
//================================================================================================================
//================================================================================================================
//================================================================================================================
//
//   �м�ܵ���Inline����.
//
//================================================================================================================
//================================================================================================================
//================================================================================================================


//@{


//! ����sampleStateΪreapeat״̬������Ⱦ�ر��ʱ��ʹ��clamp״̬�������������
//! ����Ϊreapeat״̬��
inline void osc_middlePipe::set_ssRepeat( void )
{
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP );

	m_pd3dDevice->SetSamplerState( 1,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
	m_pd3dDevice->SetSamplerState( 1,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP );
}

//! ����Ϊclamp״̬������Ⱦ��ͼ��ʱ����Ҫ����Ϊclamp״̬��
inline void osc_middlePipe::set_ssClamp( void )
{
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP );
	m_pd3dDevice->SetSamplerState( 0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP );

	m_pd3dDevice->SetSamplerState( 1,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP );
	m_pd3dDevice->SetSamplerState( 1,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP );
}






//! �ͷŵ�ǰ�ľ�̬���㻺������
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
*  reset MiddlePipeǰ��Ҫ�ͷŵ�����.
*/
inline void osc_middlePipe::mdpipe_onLostDevice( void )
{
	m_shaderMgr.shaderMgr_onLostDevice();
	release_dynamicIB();
	release_dynamicVB( false );
	osDebugOut( "�����豸��ʧ.....\n" );
}




/** \brief
*  Ϊ�ϲ��ṩmesh����Ⱦ������
*  
*  \param _mr ��Ⱦmesh�����Ľṹ��
*/
inline bool  osc_middlePipe::render_mesh( os_meshRender* _mr,bool _alpha/* = false*/  )
{
	return m_meshMgr.render_mesh( _mr,_alpha );
}



//! ����fvfֵ��
inline void osc_middlePipe::set_fvf( DWORD _fvf )
{
	HRESULT  t_hr;
	t_hr = m_pd3dDevice->SetFVF( _fvf );
	if( FAILED( t_hr ) )
		osassertex( false,va("����fvfʧ��,ԭ��<%s>..\n",osn_mathFunc::get_errorStr( t_hr )) );
}




/** \brief
*  ÿһ֡�ϲ���Ҫ���õ�keyAniMesh��Ⱦ������
*/
inline bool  osc_middlePipe::frame_renderKeyAniMesh( 
	         os_ViewFrustum* _vf,bool _alpha,os_FightSceneRType _rtype/* = OS_COMMONRENDER*/ )
{
	return m_meshMgr.render_aniMesh( _vf,_alpha,_rtype );
}

//! ����ˮ�����Ⱦ
inline bool osc_middlePipe::render_aniWater( os_ViewFrustum* _vf )
{
	return m_meshMgr.render_waterAni();
}
//! ���������Ⱦ
inline bool osc_middlePipe::render_aniFog( int ShaderId,int dwtype /*= 2*/ )
{
	return m_meshMgr.render_FogAni(ShaderId,dwtype);
}

//@{
/** \brief
*  �򵥵�����3d�豸״̬����.
*/
inline void osc_middlePipe::set_worldMatrix( osMatrix& _mat )
{
	HRESULT     t_hr;

	if( FAILED( t_hr = m_pd3dDevice->SetTransform( D3DTS_WORLD,&_mat ) ) )
	{
		osassertex( false,
			va( "�����������ʧ��,ԭ��<%s>..\n",osn_mathFunc::get_errorStr( t_hr ) ) );
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
		osassertex( false,va("����vpʧ��,ԭ��<%s>",osn_mathFunc::get_errorStr( t_hr ) ) );

	return;
}
/** \brief
*  3d ���״̬������ʹ���������,�����������߿�ģʽ����ʵ��ģʽ.
*
*  \param _mode:  1:point. 2:���߿�ģʽ.3: Ϊʵ��ģʽ.
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
*  ���м�ܵ��õ�һЩ��Ⱦ��Ϣ.
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

//! ����dx�ĵƹ�������ݡ�
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


//! ����aniMesh���ݵĶ�̬��
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

//! �ر�dx�ĵƹ�.
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

