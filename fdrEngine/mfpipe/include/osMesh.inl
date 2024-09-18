




///////////////////////////////////////////////////////////////////////////////////////////////////
//
//                开始osc_Mesh和osc_meshMgr的inline 函数
//
///////////////////////////////////////////////////////////////////////////////////////////////////


/** \brief
*  得到当前的mesh是否在使用中。
*/
inline bool osc_Mesh::is_inUse( void )                
{ 
	if( this->m_ptrPMesh )
		return true;
	else
		return false;
}

//! 减少一个mesh的引用　。
inline void osc_meshMgr::release_mesh( int _id )
{
	// ATTENTIO TO OPP: 空函数。
	osassert( (_id>=0)&&(_id<(int)m_vecMesh.size()) );
}


/** \brief
*  每一帧中调用这个函数，以增加没有渲染到的mesh的lru值。
*/
inline void osc_meshMgr::frame_moveLru( void )
{

	// 
	// 重设每一帧渲染使用到的数据。
	memset( m_vecMeshInsNum,0,sizeof( int )*MAX_MESHRSTYLE );
	m_iColorVerMeshNum = 0;
	m_iRAniMeshInsNum = 0;

	// 顺便把我们渲染到的mesh的面数目清零.
	m_iRFaceNum = 0;

}


//! 得到mesh的半径。
inline float osc_meshMgr::get_meshRadius( int _idx )
{
	osassert( _idx < (int)m_vecMesh.size() );
	return m_vecMesh[_idx].get_bsRadius();
}



//@{
//   物品动画相关的数据。
//! 开始动画
inline void osc_meshMgr::start_animationMesh( int _id )
{
	WORD   t_wId = WORD( _id );
	osassert( m_vecAniMeshIns.validate_id( t_wId ) );
	if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
		m_vecAniMeshIns[t_wId].restart_aniMeshIns();
}

//! 隐藏当前id的osa文件
inline void osc_meshMgr::set_osaHide( int _id,bool _hide )
{
	WORD   t_wId = WORD( _id );

	//! 缓和处理.
	if( !m_vecAniMeshIns.validate_id( t_wId ) )
		return;
	if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
		m_vecAniMeshIns[t_wId].set_hide( _hide );
}


//! fade或是fadeIn当前的osa动画.
inline void osc_meshMgr::start_fadeOsa( int _id,
		float _fadeTime,bool _fadeOut/* = true*/,float _minFade/* = 0.0f*/ )
{
	WORD   t_wId = WORD( _id );
	osassert( m_vecAniMeshIns.validate_id( t_wId ) );
	if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
		m_vecAniMeshIns[t_wId].start_fade( _fadeTime,_fadeOut,_minFade );
}

//! 设置动画的位置。
inline void osc_meshMgr::set_aniMeshPos( int _id,osVec3D& _pos )
{
	WORD   t_wId = WORD( _id );
	if( m_vecAniMeshIns.validate_id( t_wId ) )
	{
		if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
			m_vecAniMeshIns[t_wId].set_aniMeshPos( _pos );
	}
}

//! 设置动画的旋转
inline void osc_meshMgr::set_aniMeshRot( int _id,float _angle )
{
	WORD   t_wId = WORD( _id );
	if( m_vecAniMeshIns.validate_id( t_wId ) )
	{
		if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
			m_vecAniMeshIns[t_wId].set_aniMeshRot( _angle );
	}
}

//! 缩放当前的osa动画
inline void osc_meshMgr::scale_aniMesh( int _id,float _rate,float _time )
{
	WORD   t_wId = WORD( _id );
	if( m_vecAniMeshIns.validate_id( t_wId ) )
	{
		if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
			m_vecAniMeshIns[t_wId].scale_aniMesh( _rate,_time );
	}
}



//! 即时更新osa的位置
inline void osc_meshMgr::update_osaPosNRot( int _id )
{
	WORD   t_wId = WORD( _id );
	if( m_vecAniMeshIns.validate_id( t_wId ) )
	{
		// 因为是多次更新，所以此次更新的时间流失为零　
		if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
			m_vecAniMeshIns[t_wId].update_osaAnimation( 0.0f );
	}
}


//! 重置某一个topRenderLayer的meshIns，以用于在某一帧得到渲染。
inline void osc_meshMgr::reset_osaMeshInsRender( int _id )
{
	WORD   t_wId = WORD( _id );

	osassert( (t_wId>=0)&&(t_wId<MAXEFFECT_OSANUM) );
	osassert( m_vecAniMeshIns.validate_id( t_wId ) );

	m_vecAniMeshIns[t_wId].m_bRenderedInCurFrame = false;

}

//! 得到aniMeshIns的播放时间。
inline float osc_meshMgr::get_osaPlayTime( int _id )
{
	WORD   t_wId = WORD( _id );

	osassert( (t_wId>=0)&&(t_wId<MAXEFFECT_OSANUM) );
	osassert( m_vecAniMeshIns.validate_id( t_wId ) );

	return m_vecAniMeshIns[t_wId].get_osaPlayTime();
}

//! 设置osaMeshIns的播放时间。
inline void osc_meshMgr::set_osaPlayTime( int _id,float _timeScale )
{
	WORD   t_wId = WORD( _id );

	osassert( (t_wId>=0)&&(t_wId<MAXEFFECT_OSANUM) );
	if( m_vecAniMeshIns.validate_id( t_wId ) )
		m_vecAniMeshIns[t_wId].set_osaPlayTime( _timeScale );
}


/** \brief
*  设置一个动画的世界矩阵。
*
*  这个接口主要用于一些特效的播放，需要在游戏层填充世界矩阵数据。
*/
inline void osc_meshMgr::set_aniMeshTrans( int _id,osMatrix& _transMat )
{
	WORD   t_wId = WORD( _id );

	// 极低的概率会出现错误，此处直接返回
	if( !m_vecAniMeshIns.validate_id( t_wId ) )
		return;

	if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
		m_vecAniMeshIns[t_wId].set_aniMeshTrans( _transMat );
}


//! 验证动画id的有效性。
inline bool osc_meshMgr::validate_aniMeshId( int _id )
{
	guard;
	WORD   t_wId = WORD( _id );

	// River @ 2011-10-14:如果ID非法，返回false.
	if( t_wId >= MAXEFFECT_OSANUM )
		return false;

	osassertex( t_wId < MAXEFFECT_OSANUM,va( "The error id is:<%d>..\n",t_wId ) );

	if( !m_vecAniMeshIns.validate_id( t_wId ) )
		return false;

	if( !m_vecAniMeshIns[t_wId].validate_cval( _id ) )
		return false;

	return m_vecAniMeshIns[t_wId].is_amInsInuse();
	unguard;
}


# if __EFFECT_EDITOR__
//! 强制结束一个动画.
inline void osc_meshMgr::stop_animationMesh( int _id )
{
	WORD   t_wId = WORD( _id );
	osassert( m_vecAniMeshIns.validate_id( t_wId ) );
	m_vecAniMeshIns[t_wId].stop_aniMeshIns();
}

//! 设置某一个动画相关的参数.
inline bool osc_meshMgr::set_animationMesh( int _id,os_aniMeshCreate* _mc )
{
	WORD   t_wId = WORD( _id );

	osassert( m_vecAniMeshIns.validate_id( t_wId ) );
	m_vecAniMeshIns[t_wId].set_animationMesh( _mc );
	return true;
}
# endif 




//! 得到静态mesh的BoundingBox.
inline os_aabbox* osc_meshMgr::get_meshBBox( int _idx )
{
	guard;
	osassert( (_idx >= 0)&&(_idx<(int)m_vecMesh.size()) );
	return m_vecMesh[_idx].get_meshBBox();
	unguard;
}

//! 得到一个osaMesh的Boundingsphere.
inline os_bsphere* osc_meshMgr::get_osaMeshBsphere( int _idx,bool _sce/* = false*/ )
{
	guard;
	WORD   t_wId = WORD( _idx );

	if( !_sce )
	{
		osassert( (t_wId>=0) && (t_wId<MAXEFFECT_OSANUM) );
		return m_vecAniMeshIns[t_wId].get_bsphere();
	}
	else
	{
		int t_idx = t_wId-MAXEFFECT_OSANUM;
	    osassert( (t_idx>=0)&&(t_idx<MAXMESHINS_PERSCE*MAXTG_INTGMGR) );

		return m_vecSceneAMIns[t_wId-MAXEFFECT_OSANUM].get_bsphere();
	}
	unguard;
}
//! 得到一个osaMesh的os_bbox
inline void osc_meshMgr::get_osaMeshBbox( int _idx,os_bbox* _bbox,bool _sce/* = false*/ )
{
	guard;
	WORD   t_wId = WORD( _idx );

	if( !_sce )
	{
		osassert( (t_wId>=0) && (t_wId<MAXEFFECT_OSANUM) );
		return m_vecAniMeshIns[t_wId].get_bbox( _bbox );
	}
	else
	{
		int t_idx = t_wId-MAXEFFECT_OSANUM;
	    osassert( (t_idx>=0)&&(t_idx<MAXMESHINS_PERSCE*MAXTG_INTGMGR) );

		return m_vecSceneAMIns[t_wId-MAXEFFECT_OSANUM].get_bbox( _bbox );
	}
	unguard;
}

//! 场景切换时，重设meshMgr
inline void osc_meshMgr::sceneChange_reset( void )
{
	m_ptrWaterEffect = NULL;
}



//! 输入一条射线,看是否和*.x的mesh文件相交.
inline bool osc_meshMgr::intersect_mesh( 
	int _idx,osVec3D& _rayPos,float _dis,osVec3D& _rayDir,float* _resDis/* = NULL*/ )
{
	return m_vecMesh[_idx].intersect_mesh( _rayPos,_dis,_rayDir,_resDis );
}

//! 是否是关键物品
inline bool osc_meshMgr::is_keyObj( int _idx )
{
	return m_vecMesh[_idx].is_keyObj();
}
//! 是否桥类物品.
inline BOOL osc_meshMgr::is_bridgeObj( int _idx )
{
	return m_vecMesh[_idx].is_bridgeObj();
}
//! 是否是受体积雾影响物品
inline BOOL osc_meshMgr::is_AcceptFogObj( int _idx )
{
	return m_vecMesh[_idx].is_AcceptFogObj();
}
//! 是否树类物品
inline BOOL osc_meshMgr::is_treeObj( int _idx )
{
	return m_vecMesh[_idx].is_treeObj();
}
//! 是否是无alpha无碰撞
inline BOOL osc_meshMgr::is_NoAlphaObj( int _idx )
{
	return m_vecMesh[_idx].is_NoAlphaObj();
}


/** \brief
*  从一个meshId得到这个mesh是否是alphaMesh.
*/
inline bool osc_meshMgr::is_alphaMesh( int _id )            
{ 
	return m_vecMesh[_id].has_alphaSubset(); 
}
//! 当前Mesh是否有OpacitySubse
inline bool osc_meshMgr::is_opacityMesh( int _id )
{
	return m_vecMesh[_id].has_opacitySubset();
}


/** \brief
*  得到渲染的面的数目.
*/
inline int osc_meshMgr::get_rFaceNum( void )               
{ 
	return m_iRFaceNum; 
}



//@} 

