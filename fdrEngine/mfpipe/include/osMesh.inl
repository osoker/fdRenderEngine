




///////////////////////////////////////////////////////////////////////////////////////////////////
//
//                ��ʼosc_Mesh��osc_meshMgr��inline ����
//
///////////////////////////////////////////////////////////////////////////////////////////////////


/** \brief
*  �õ���ǰ��mesh�Ƿ���ʹ���С�
*/
inline bool osc_Mesh::is_inUse( void )                
{ 
	if( this->m_ptrPMesh )
		return true;
	else
		return false;
}

//! ����һ��mesh�����á���
inline void osc_meshMgr::release_mesh( int _id )
{
	// ATTENTIO TO OPP: �պ�����
	osassert( (_id>=0)&&(_id<(int)m_vecMesh.size()) );
}


/** \brief
*  ÿһ֡�е������������������û����Ⱦ����mesh��lruֵ��
*/
inline void osc_meshMgr::frame_moveLru( void )
{

	// 
	// ����ÿһ֡��Ⱦʹ�õ������ݡ�
	memset( m_vecMeshInsNum,0,sizeof( int )*MAX_MESHRSTYLE );
	m_iColorVerMeshNum = 0;
	m_iRAniMeshInsNum = 0;

	// ˳���������Ⱦ����mesh������Ŀ����.
	m_iRFaceNum = 0;

}


//! �õ�mesh�İ뾶��
inline float osc_meshMgr::get_meshRadius( int _idx )
{
	osassert( _idx < (int)m_vecMesh.size() );
	return m_vecMesh[_idx].get_bsRadius();
}



//@{
//   ��Ʒ������ص����ݡ�
//! ��ʼ����
inline void osc_meshMgr::start_animationMesh( int _id )
{
	WORD   t_wId = WORD( _id );
	osassert( m_vecAniMeshIns.validate_id( t_wId ) );
	if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
		m_vecAniMeshIns[t_wId].restart_aniMeshIns();
}

//! ���ص�ǰid��osa�ļ�
inline void osc_meshMgr::set_osaHide( int _id,bool _hide )
{
	WORD   t_wId = WORD( _id );

	//! ���ʹ���.
	if( !m_vecAniMeshIns.validate_id( t_wId ) )
		return;
	if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
		m_vecAniMeshIns[t_wId].set_hide( _hide );
}


//! fade����fadeIn��ǰ��osa����.
inline void osc_meshMgr::start_fadeOsa( int _id,
		float _fadeTime,bool _fadeOut/* = true*/,float _minFade/* = 0.0f*/ )
{
	WORD   t_wId = WORD( _id );
	osassert( m_vecAniMeshIns.validate_id( t_wId ) );
	if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
		m_vecAniMeshIns[t_wId].start_fade( _fadeTime,_fadeOut,_minFade );
}

//! ���ö�����λ�á�
inline void osc_meshMgr::set_aniMeshPos( int _id,osVec3D& _pos )
{
	WORD   t_wId = WORD( _id );
	if( m_vecAniMeshIns.validate_id( t_wId ) )
	{
		if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
			m_vecAniMeshIns[t_wId].set_aniMeshPos( _pos );
	}
}

//! ���ö�������ת
inline void osc_meshMgr::set_aniMeshRot( int _id,float _angle )
{
	WORD   t_wId = WORD( _id );
	if( m_vecAniMeshIns.validate_id( t_wId ) )
	{
		if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
			m_vecAniMeshIns[t_wId].set_aniMeshRot( _angle );
	}
}

//! ���ŵ�ǰ��osa����
inline void osc_meshMgr::scale_aniMesh( int _id,float _rate,float _time )
{
	WORD   t_wId = WORD( _id );
	if( m_vecAniMeshIns.validate_id( t_wId ) )
	{
		if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
			m_vecAniMeshIns[t_wId].scale_aniMesh( _rate,_time );
	}
}



//! ��ʱ����osa��λ��
inline void osc_meshMgr::update_osaPosNRot( int _id )
{
	WORD   t_wId = WORD( _id );
	if( m_vecAniMeshIns.validate_id( t_wId ) )
	{
		// ��Ϊ�Ƕ�θ��£����Դ˴θ��µ�ʱ����ʧΪ�㡡
		if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
			m_vecAniMeshIns[t_wId].update_osaAnimation( 0.0f );
	}
}


//! ����ĳһ��topRenderLayer��meshIns����������ĳһ֡�õ���Ⱦ��
inline void osc_meshMgr::reset_osaMeshInsRender( int _id )
{
	WORD   t_wId = WORD( _id );

	osassert( (t_wId>=0)&&(t_wId<MAXEFFECT_OSANUM) );
	osassert( m_vecAniMeshIns.validate_id( t_wId ) );

	m_vecAniMeshIns[t_wId].m_bRenderedInCurFrame = false;

}

//! �õ�aniMeshIns�Ĳ���ʱ�䡣
inline float osc_meshMgr::get_osaPlayTime( int _id )
{
	WORD   t_wId = WORD( _id );

	osassert( (t_wId>=0)&&(t_wId<MAXEFFECT_OSANUM) );
	osassert( m_vecAniMeshIns.validate_id( t_wId ) );

	return m_vecAniMeshIns[t_wId].get_osaPlayTime();
}

//! ����osaMeshIns�Ĳ���ʱ�䡣
inline void osc_meshMgr::set_osaPlayTime( int _id,float _timeScale )
{
	WORD   t_wId = WORD( _id );

	osassert( (t_wId>=0)&&(t_wId<MAXEFFECT_OSANUM) );
	if( m_vecAniMeshIns.validate_id( t_wId ) )
		m_vecAniMeshIns[t_wId].set_osaPlayTime( _timeScale );
}


/** \brief
*  ����һ���������������
*
*  ����ӿ���Ҫ����һЩ��Ч�Ĳ��ţ���Ҫ����Ϸ���������������ݡ�
*/
inline void osc_meshMgr::set_aniMeshTrans( int _id,osMatrix& _transMat )
{
	WORD   t_wId = WORD( _id );

	// ���͵ĸ��ʻ���ִ��󣬴˴�ֱ�ӷ���
	if( !m_vecAniMeshIns.validate_id( t_wId ) )
		return;

	if( m_vecAniMeshIns[t_wId].validate_cval( _id ) )
		m_vecAniMeshIns[t_wId].set_aniMeshTrans( _transMat );
}


//! ��֤����id����Ч�ԡ�
inline bool osc_meshMgr::validate_aniMeshId( int _id )
{
	guard;
	WORD   t_wId = WORD( _id );

	// River @ 2011-10-14:���ID�Ƿ�������false.
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
//! ǿ�ƽ���һ������.
inline void osc_meshMgr::stop_animationMesh( int _id )
{
	WORD   t_wId = WORD( _id );
	osassert( m_vecAniMeshIns.validate_id( t_wId ) );
	m_vecAniMeshIns[t_wId].stop_aniMeshIns();
}

//! ����ĳһ��������صĲ���.
inline bool osc_meshMgr::set_animationMesh( int _id,os_aniMeshCreate* _mc )
{
	WORD   t_wId = WORD( _id );

	osassert( m_vecAniMeshIns.validate_id( t_wId ) );
	m_vecAniMeshIns[t_wId].set_animationMesh( _mc );
	return true;
}
# endif 




//! �õ���̬mesh��BoundingBox.
inline os_aabbox* osc_meshMgr::get_meshBBox( int _idx )
{
	guard;
	osassert( (_idx >= 0)&&(_idx<(int)m_vecMesh.size()) );
	return m_vecMesh[_idx].get_meshBBox();
	unguard;
}

//! �õ�һ��osaMesh��Boundingsphere.
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
//! �õ�һ��osaMesh��os_bbox
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

//! �����л�ʱ������meshMgr
inline void osc_meshMgr::sceneChange_reset( void )
{
	m_ptrWaterEffect = NULL;
}



//! ����һ������,���Ƿ��*.x��mesh�ļ��ཻ.
inline bool osc_meshMgr::intersect_mesh( 
	int _idx,osVec3D& _rayPos,float _dis,osVec3D& _rayDir,float* _resDis/* = NULL*/ )
{
	return m_vecMesh[_idx].intersect_mesh( _rayPos,_dis,_rayDir,_resDis );
}

//! �Ƿ��ǹؼ���Ʒ
inline bool osc_meshMgr::is_keyObj( int _idx )
{
	return m_vecMesh[_idx].is_keyObj();
}
//! �Ƿ�������Ʒ.
inline BOOL osc_meshMgr::is_bridgeObj( int _idx )
{
	return m_vecMesh[_idx].is_bridgeObj();
}
//! �Ƿ����������Ӱ����Ʒ
inline BOOL osc_meshMgr::is_AcceptFogObj( int _idx )
{
	return m_vecMesh[_idx].is_AcceptFogObj();
}
//! �Ƿ�������Ʒ
inline BOOL osc_meshMgr::is_treeObj( int _idx )
{
	return m_vecMesh[_idx].is_treeObj();
}
//! �Ƿ�����alpha����ײ
inline BOOL osc_meshMgr::is_NoAlphaObj( int _idx )
{
	return m_vecMesh[_idx].is_NoAlphaObj();
}


/** \brief
*  ��һ��meshId�õ����mesh�Ƿ���alphaMesh.
*/
inline bool osc_meshMgr::is_alphaMesh( int _id )            
{ 
	return m_vecMesh[_id].has_alphaSubset(); 
}
//! ��ǰMesh�Ƿ���OpacitySubse
inline bool osc_meshMgr::is_opacityMesh( int _id )
{
	return m_vecMesh[_id].has_opacitySubset();
}


/** \brief
*  �õ���Ⱦ�������Ŀ.
*/
inline int osc_meshMgr::get_rFaceNum( void )               
{ 
	return m_iRFaceNum; 
}



//@} 

