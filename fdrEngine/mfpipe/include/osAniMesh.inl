

/** \brief
*  得到当前的Bounding Sphere,物品坐标，需要转化到世界空间。
*/
inline void os_aniNode::get_bsphere( os_bsphere& _bs )
{
	_bs = this->m_bsphere;
}

//! 当前的mesh是否在使用。
inline bool osc_aniMesh::is_aniMeshInUse( void )
{
	return m_bInuse;
}


//! 对两个objectVertex进行插值。
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



//! 当前的mesh是否使用材质动画。
inline bool osc_aniMesh::is_aniMeshAniMat( void )
{
	return m_bAniMat;
}




/** \brief
*  当前的aniMeshIns是否在使用中，
*/
inline bool osc_aniMeshIns::is_amInsInuse( void )
{
	return m_bInuse;
}

//! 当前的meshIns渲染是不是使用和背景相加的渲染。
inline int osc_aniMeshIns::get_RenderStyle( void )
{
	osassert( m_ptrMesh );
	return m_ptrMesh->m_btRStyle;
}


//! 得到当前的实例数据索引到的mesh的ref数目。
inline osc_aniMesh* osc_aniMeshIns::get_meshPtr( void )
{
	return m_ptrMesh;
}



//@{
//! 设置和得到当前meshIns的光照系数。
inline void osc_aniMeshIns::set_light( DWORD _light )
{
	m_dwAmbientL = _light;
}

inline DWORD osc_aniMeshIns::get_light( void )
{
	return m_dwAmbientL;
}
//@} 

//! 当前的Mesh是否是点着色的mesh
inline bool osc_aniMeshIns::is_colorVertexMesh( void )
{
	return m_ptrMesh->m_bColorVertex;
}



/** \brief
*  得到当前mesh Ins的Bonding sphere.
*/
inline os_bsphere* osc_aniMeshIns::get_bsphere( void )
{
	return &m_bsphere;
}



/** \brief
*  重新开始当前的动画。
*/
inline void osc_aniMeshIns::restart_aniMeshIns( void )
{
	m_fPlayTime = 0.0f;
	m_iPlayFrames = 0;
	m_bStopPlay = false;
}

/** \brief
*  开始当前osaAni的fade动画.
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
*  停止当前动画的播放。
*/
inline void osc_aniMeshIns::stop_aniMeshIns( void )
{
	this->m_bStopPlay = true;
}

/** \brief
*  设置某一个动画相关的参数.
*
*  根据传入的参数，设置animationMesh内部的数据。
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


//! 设置aniMesh的位置。
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
//! 缩放当前的osa动画
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
*  设置一个动画的世界矩阵。
*
*  这个接口主要用于一些特效的播放，需要在游戏层填充世界矩阵数据。
*/
inline void osc_aniMeshIns::set_aniMeshTrans( osMatrix& _transMat )
{
	m_matTrans = _transMat;
	return;
}

//! 得到当前meshIns用到的世界旋转和位移矩阵。
inline void osc_aniMeshIns::get_transMatrix( osMatrix& _mat )
{

	// 先缩放再旋转
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
		// 这个矩阵仅用于内部使用，处理跟随武器的osa特效效果
		_mat = _mat*m_matTrans;
	}

	_mat._41 = m_vec3Pos.x;
	_mat._42 = m_vec3Pos.y;
	_mat._43 = m_vec3Pos.z;

	return;
}


/** \brief
*  如果按次数播放的话，是否已经播放完
*
*  \return bool 如果返回true,则播放完成。
*/ 
inline bool osc_aniMeshIns::finish_playWithFrame( void )
{
	if( m_ePlayType == OSE_PLAYWITHFRAME )
	{
		// 
		// 从大变小，说明完成了一轮循环。
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

//! 如果按时间播放，是否已经播放完成。
inline bool osc_aniMeshIns::finish_playWithTime( float _et )
{
	// 按时间播放。
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

