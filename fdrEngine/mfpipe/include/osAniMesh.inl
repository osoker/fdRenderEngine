

/** \brief
*  �õ���ǰ��Bounding Sphere,��Ʒ���꣬��Ҫת��������ռ䡣
*/
inline void os_aniNode::get_bsphere( os_bsphere& _bs )
{
	_bs = this->m_bsphere;
}

//! ��ǰ��mesh�Ƿ���ʹ�á�
inline bool osc_aniMesh::is_aniMeshInUse( void )
{
	return m_bInuse;
}


//! ������objectVertex���в�ֵ��
inline void os_aniNode::objver_lerp( os_objectVertex* _res,
							  os_objectVertex* _src,os_objectVertex* _dst,float _factor )
{
	osVec3Lerp( &_res->m_vecPos,&_src->m_vecPos,&_dst->m_vecPos,_factor );
	osVec3Lerp( &_res->m_vecNormal,&_src->m_vecNormal,&_dst->m_vecNormal,_factor );

	if( m_bNotInterpolate )
		_res->m_vec2Tex1 = _src->m_vec2Tex1;
	else
		osVec2Lerp( &_res->m_vec2Tex1,&_src->m_vec2Tex1,&_dst->m_vec2Tex1,_factor );
}



//! ��ǰ��mesh�Ƿ�ʹ�ò��ʶ�����
inline bool osc_aniMesh::is_aniMeshAniMat( void )
{
	return m_bAniMat;
}




/** \brief
*  ��ǰ��aniMeshIns�Ƿ���ʹ���У�
*/
inline bool osc_aniMeshIns::is_amInsInuse( void )
{
	return m_bInuse;
}

//! ��ǰ��meshIns��Ⱦ�ǲ���ʹ�úͱ�����ӵ���Ⱦ��
inline int osc_aniMeshIns::get_RenderStyle( void )
{
	osassert( m_ptrMesh );
	return m_ptrMesh->m_btRStyle;
}


//! �õ���ǰ��ʵ��������������mesh��ref��Ŀ��
inline osc_aniMesh* osc_aniMeshIns::get_meshPtr( void )
{
	return m_ptrMesh;
}



//@{
//! ���ú͵õ���ǰmeshIns�Ĺ���ϵ����
inline void osc_aniMeshIns::set_light( DWORD _light )
{
	m_dwAmbientL = _light;
}

inline DWORD osc_aniMeshIns::get_light( void )
{
	return m_dwAmbientL;
}
//@} 

//! ��ǰ��Mesh�Ƿ��ǵ���ɫ��mesh
inline bool osc_aniMeshIns::is_colorVertexMesh( void )
{
	return m_ptrMesh->m_bColorVertex;
}



/** \brief
*  �õ���ǰmesh Ins��Bonding sphere.
*/
inline os_bsphere* osc_aniMeshIns::get_bsphere( void )
{
	return &m_bsphere;
}



/** \brief
*  ���¿�ʼ��ǰ�Ķ�����
*/
inline void osc_aniMeshIns::restart_aniMeshIns( void )
{
	m_fPlayTime = 0.0f;
	m_iPlayFrames = 0;
	m_bStopPlay = false;
}

/** \brief
*  ��ʼ��ǰosaAni��fade����.
*/
inline void osc_aniMeshIns::start_fade( float _time,bool _fadeOut/* = true*/,float _minFade/* = 0.0f*/ )
{
	osassert( _time > 0.0f );

	m_fFadeTime = _time;
	m_fCurEleFadeTime = 0.0f;
	if( _fadeOut )
		m_iFadeState = 2;
	else
		m_iFadeState = 1;
	m_fCurFadeRate = 1.0f;

	//! river added @ 2009-10-10:
	m_fMinFade = _minFade;
		
}
# if __EFFECT_EDITOR__
/** \brief
*  ֹͣ��ǰ�����Ĳ��š�
*/
inline void osc_aniMeshIns::stop_aniMeshIns( void )
{
	this->m_bStopPlay = true;
}

/** \brief
*  ����ĳһ��������صĲ���.
*
*  ���ݴ���Ĳ���������animationMesh�ڲ������ݡ�
*/
inline bool osc_aniMeshIns::set_animationMesh( os_aniMeshCreate* _mc )
{
	m_vec3Pos = _mc->m_vec3Pos;
	m_fRotAgl = _mc->m_fAngle;
	m_ePlayType = _mc->m_ePlayType;
	m_fPlayTime = _mc->m_fPlayTime;
	m_iPlayFrames = _mc->m_iPlayFrames;
	m_dwAmbientL = _mc->m_dwAmbientL;
	m_bAutoDelete = _mc->m_bAutoDelete;
	return true;
}

# endif 


//! ����aniMesh��λ�á�
inline void osc_aniMeshIns::set_aniMeshPos( osVec3D& _pos )
{
	m_vec3Pos = _pos;

	m_bsphere.veccen = _pos;

	return ;
}
inline void osc_aniMeshIns::set_aniMeshRot(float _RotAgl)
{
	m_fRotAgl = _RotAgl;
}
//! ���ŵ�ǰ��osa����
inline void osc_aniMeshIns::scale_aniMesh( float _rate,float _time )
{
	m_bInScaleState = true;
	m_fNewScale = _rate;
	m_fScaleTime = _time;
	m_fScaleEleTime = 0.0f;
	osassert( _time >= 0.0f );
	osassert( _rate > 0.0f );
}


/** \brief
*  ����һ���������������
*
*  ����ӿ���Ҫ����һЩ��Ч�Ĳ��ţ���Ҫ����Ϸ���������������ݡ�
*/
inline void osc_aniMeshIns::set_aniMeshTrans( osMatrix& _transMat )
{
	m_matTrans = _transMat;
	return;
}

//! �õ���ǰmeshIns�õ���������ת��λ�ƾ���
inline void osc_aniMeshIns::get_transMatrix( osMatrix& _mat )
{

	// ����������ת
	osMatrixIdentity( &_mat );

	if( float_equal( m_matTrans._41,-1.0f ) )
	{
		if( !float_equal( m_fSizeScale,1.0f ) )		
			osMatrixScaling( &_mat,m_fSizeScale,m_fSizeScale,m_fSizeScale );

		if( !float_equal( m_fRotAgl,0.0f ) )
		{
			osMatrix   t_sMatRot;
			osMatrixRotationY( &t_sMatRot,m_fRotAgl );
			osMatrixMultiply( &_mat,&_mat,&t_sMatRot );
		}
	}
	else
	{
		// �������������ڲ�ʹ�ã��������������osa��ЧЧ��
		_mat = _mat*m_matTrans;
	}

	_mat._41 = m_vec3Pos.x;
	_mat._42 = m_vec3Pos.y;
	_mat._43 = m_vec3Pos.z;

	return;
}


/** \brief
*  ������������ŵĻ����Ƿ��Ѿ�������
*
*  \return bool �������true,�򲥷���ɡ�
*/ 
inline bool osc_aniMeshIns::finish_playWithFrame( void )
{
	if( m_ePlayType == OSE_PLAYWITHFRAME )
	{
		// 
		// �Ӵ��С��˵�������һ��ѭ����
		if( m_bOverLoop )
			m_iLoopTimes ++;

		if( m_iLoopTimes >= m_iPlayFrames )
		{
			delete_curIns();
			return true;
		}
	}
	
	return false;

}

//! �����ʱ�䲥�ţ��Ƿ��Ѿ�������ɡ�
inline bool osc_aniMeshIns::finish_playWithTime( float _et )
{
	// ��ʱ�䲥�š�
	if( m_ePlayType == OSE_PLAYWITHTIME )
	{
		m_fEleTime += _et;
		if( m_fEleTime > m_fPlayTime )
		{
			delete_curIns();
			return true;
		}
	}

	return false;
}

