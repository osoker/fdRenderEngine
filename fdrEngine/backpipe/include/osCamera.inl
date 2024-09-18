
//================================================================================================================
//================================================================================================================
//================================================================================================================
//
//      ����osc_camera�е�inline����.
//
//================================================================================================================
//================================================================================================================
//================================================================================================================


/** \brief
*  ����һ��Camera��Projection Matrix
*/
inline void osc_camera::set_projmat( float _nearp,
								 float _farp,float _fovh,float _faspect )
{
	this->m_fNearPlane = _nearp;
	m_fFarPlane = _farp;
	this->m_fFov = _fovh;
	this->m_fAspect = _faspect ;

	this->m_bShouldUpdate = true;
}

//! ����Զ������.
inline void osc_camera::set_farClipPlane( float _farp )
{
	// ��Ч��صĸ���
	float  t_fFogStart = g_fFogStart / m_fFarPlane;
	float  t_fFogEnd = g_fFogEnd /  m_fFarPlane;

	m_fFarPlane = _farp;

	// ������ľ���
	g_fFogStart = m_fFarPlane * t_fFogStart;
	g_fFogEnd = m_fFarPlane * t_fFogEnd;

	m_fTADis = float( m_fFarPlane*TA_SCALE );
	m_fHalfFar = m_fTADis/2.0f;
	m_fObjFullView = float(m_fHalfFar*2.0f-(1.0f-TA_SCALE)*m_fFarPlane);

	m_bShouldUpdate = true;
}



/** \brief
*  ����һ��Camera��ViewPort.
*
*  ��ͬ��Camera������Ҫ��ͬ��ViewPort,���ܳ��ֶ��Camera���
*   ViewPort���龰(���л�).
*
*   _tlx,_tly        Viewport����Ļ���ϽǵĶ���.
*
*   _wid,_hei        Viewport�Ŀ�͸ߡ�
*
*   _min_z,_max_z    Viewport��������СZֵ.
*/
inline void osc_camera::set_viewport( int _tlx,int _tly,
				  int _wid,int _hei,float _min_z,float _max_z )
{
	this->m_iTopleftx = _tlx;
	this->m_iToplefty = _tly;
	this->m_iWidth = _wid;
	this->m_iHeight = _hei;
	this->m_fMinz = _min_z;
	this->m_fMaxz = _max_z;


	this->m_bShouldUpdate = true;

}





/** \brief
*  �õ���ǰ��������ҵķ������.
*/
inline void osc_camera::get_rightvec( osVec3D& _vec )
{
	if( m_bShouldUpdate )
		update_camera();

	_vec = m_curRightVec;

}



/** \brief
*  �õ������ǰ��λ��.
*/
inline void osc_camera::get_curpos( osVec3D* _vec )
{
	if( this->m_bShouldUpdate )
		this->update_camera();

	*_vec = this->m_curpos;

	// �������Ⱦ�ر�����Ҫ���assert.
	if( g_bRenderTerr  )
	{
		// River mod @ 2008-4-28:
		if( g_bClipCamInMap )
		{
			osassert( m_curpos.x >= 0.0f );
			osassert( m_curpos.z >= 0.0f );
		}
	}
}
inline const osVec3D&  osc_camera::get_curpos( void )
{
	if( this->m_bShouldUpdate )
		this->update_camera();

	
	// �������Ⱦ�ر�����Ҫ���assert.
	if( g_bRenderTerr  )
	{
		// River mod @ 2008-4-28:
		if( g_bClipCamInMap )
		{
			osassert( m_curpos.x >= 0.0f );
			osassert( m_curpos.z >= 0.0f );
		}
	}

	return m_curpos;
}

/** \brief
*  ʹ���Pitchһ���Ƕ�.
*/
inline void osc_camera::pitch( float _radian )
{
	if( m_fCurpitch + _radian >= 0 )
		m_fCurpitch =-float(0.01);
	else
	{
		// �������Ͽ��ĽǶȱ�Ϊֱ��
		if( (m_fCurpitch + _radian)<-3.0f )
			m_fCurpitch = -3.0f;
		else
			m_fCurpitch += _radian;
	}

	m_bRotInCurFrame = true;
	m_bShouldUpdate = true;


}


/** \brief
*  ʹ���Yawһ���Ƕ�.
*/
inline void osc_camera::yaw( float _radian )
{
	//�޶���0-2pi֮��
	if( (m_fCuryaw + _radian ) > 2*OS_PI )
		m_fCuryaw = m_fCuryaw + _radian - 2*OS_PI;
	else if( (m_fCuryaw + _radian )< 0 )
		m_fCuryaw = 2*OS_PI + m_fCuryaw + _radian;
	else
		m_fCuryaw += _radian;

	m_bRotInCurFrame = true;
	m_bShouldUpdate = true;

}




//@{
//!  View port releated func.
inline osVec2D osc_camera::get_screencenter( void )
{
	return this->m_scenter;
}
inline int osc_camera::get_viewporttlx( void )
{
	return this->m_iTopleftx;
}
inline int osc_camera::get_viewporttly( void )
{
	return this->m_iToplefty;
}
inline int osc_camera::get_viewportwidth( void )
{
	return this->m_iWidth;
}
inline int osc_camera::get_viewportheight( void )
{
	return this->m_iHeight;
}
//@}


//!  Get the view matrix..
inline void osc_camera::get_viewmat( osMatrix& _mat,bool _reflect/* = false*/ )
{
	if(this->m_bShouldUpdate)
		this->update_camera( true,true,_reflect );

	_mat = m_viewmat;
}


//! �õ����������view mat.
inline void osc_camera::get_rhViewMat( osMatrix& _mat )
{
	if( this->m_bShouldUpdate )
		this->update_camera();

	_mat = m_rhViewMat;
}


//! �õ����������proj Matrix.
inline void osc_camera::get_rhProjMat( osMatrix& _mat )
{
	//now only use the hfov....use d3dx helper func.fov_horiz1.5*SG_PI
	osMatrixPerspectiveFovRH( &_mat,this->m_fFov,this->m_fAspect,
		this->m_fNearPlane,this->m_fFarPlane );

}



//!  �õ���Ļ�ռ��׼�Ǳ仯������ռ��vec.
inline osVec3D osc_camera::get_upprojvec( void )
{
	// ��ǰʹ�õ�λ�������.
	osMatrix   t_mat;
	osMatrixIdentity( &t_mat );
	return get_upprojvec( &m_scenter,t_mat );
}

//!  �õ����camera��frustum.
inline os_ViewFrustum* osc_camera::get_frustum( void )
{
	if( m_bShouldUpdate )
		update_camera( true,false );
	return &m_Frustum;
}



//!  �õ�Զ������ľ��롣
inline float osc_camera::get_farPlaneDis( void )
{
	return m_fFarPlane;
}

//!  ����Զ������ľ���.
inline void osc_camera::set_farPlaneDis( float _fdis )
{

	m_fFarPlane = _fdis;
	m_bShouldUpdate = true;

	m_fTADis = float( m_fFarPlane*TA_SCALE );
	m_fHalfFar = m_fTADis/2.0f;
	m_fObjFullView = float(m_fHalfFar*2.0f-(1.0f-TA_SCALE)*m_fFarPlane);

}

//! ���������fovֵ��
inline void osc_camera::set_cameraFov( float _fov )
{
	if( (_fov>=0)&&(_fov<=(OS_PI-0.4)) )
		m_fFov = _fov;
	else
		m_fFov = 0.6f;

	this->m_bShouldUpdate = true;
}





//!  �õ���ǰ�����yaw
inline float osc_camera::get_curYaw( void )
{
	return m_fCuryaw;
}

//!  �õ���ǰ�����pitchֵ��
inline float osc_camera::get_curPitch( void )
{

	return m_fCurpitch;
}

//! �õ�Tile��ʼ͸��������Ӿء�
inline float osc_camera::get_taDis( void ) 
{
	return m_fTADis;
}


inline void osc_camera::get_camFocus( osVec3D& _focus )const
{
	_focus = this->m_vec3CamFocus;
}
inline const osVec3D& osc_camera::get_camFocus(void)const
{
	return m_vec3CamFocus;
}

/** \brief
*  ��������camear��aspect.
*
*  \param _aspectAdd ��ǰ�����aspect����������ݡ��ڲ���_aspectAdd����Ϊ
*                    �ı���aspect���ݣ�������.
*/
inline void osc_camera::set_camAspect( float& _aspectAdd )
{
	m_fAspect += _aspectAdd;
	m_bShouldUpdate = true;
	_aspectAdd = m_fAspect;
}


//! ��������Ķ���״̬
inline void osc_camera::back_upCamCaState( void )
{
	m_bBackCaState = m_bInCAState;
	m_bInCAState = false;
}
inline void osc_camera::reset_CamCaState( void )
{
	m_bInCAState = m_bBackCaState;
}



