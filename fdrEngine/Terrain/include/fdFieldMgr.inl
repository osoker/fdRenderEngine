

/** \brief
*  �õ���ǰ��������ڵ�TG.
*/
inline osc_TileGroundPtr osc_TGManager::get_curCamIn_tground( void )
{
	for( int t_i=0;t_i<m_iActMapNum;t_i++ )
	{
		if( (m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosX==m_vec2CamInIdx.m_iX)&&
			(m_arrPtrTG[t_i]->m_sTgHeader.m_wMapPosZ==m_vec2CamInIdx.m_iY)  )
		{
			return m_arrPtrTG[t_i];
		}
	}
	return NULL;
}



//! River added @ 2009-7-23:ȫ�ֵõ�ˮ��ĸ߶�
inline float osc_TGManager::get_waterSurfaceHeight( void )
{
	/*
	//! ����߶ȹ��ߣ����ֵ����û�г�ʼ��
	if( m_sSceneTexData.m_fWaterHeight > 300.0f )
		return -1000.0f;
	else
		return m_sSceneTexData.m_fWaterHeight;
	*/
	return m_sSceneTexData.get_waterHeight();
}



/** \brief
*  �õ���ǰ�������������frustum.
*/
inline os_ViewFrustum* osc_TGManager::get_camFrustum( void )
{
	if( g_ptrCamera )
		return g_ptrCamera->get_frustum();
	else 
		return NULL;
}



//@{
// ��̬����صĽӿڡ�
/** \brief
*  ����һ����̬�⡣
*
*  \return int  �������-1,�򴴽�ʧ�ܡ�
*/
inline int osc_TGManager::create_dynamicLight( os_dlightStruct& _dl )
{
	return m_dlightMgr.create_dynamicLight( _dl );
}



/** \brief
*  �õ�һ����̬��ĸ��������
*/
inline void osc_TGManager::get_dynamicLight( int _id,os_dlightStruct& _dl )
{
	m_dlightMgr.get_dynamicLight( _id,_dl );
}

/** \brief
*  FrameMoveһ����̬�⡣
*/
inline void osc_TGManager::set_dynamicLight( int _id,os_dlightStruct& _dl )
{
	m_dlightMgr.set_dynamicLight( _id,_dl );
}

//! ���ö�̬���λ�á�
inline void osc_TGManager::set_dLightPos( int _id,osVec3D& _pos )
{
	m_dlightMgr.set_dLightPos( _id,_pos );
}

/** \brief
*  ɾ��һ����̬�⡣
*/
inline void osc_TGManager::delete_dynamicLight( int _id,float _fadeTime/* = 0.0f*/ )
{
	m_dlightMgr.delete_dynamicLight( _id,_fadeTime );
}



/** \brief
*  �õ������еĻ��������ݡ�
*/
inline DWORD osc_TGManager::get_ambientLight( void )        
{ 
	return DWORD( osColor( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f ) );
}

//@}




//@{
// 
// �����ж�����صĺ���.�˴��Ķ����ǹ�������.
// �����ļ��ĸ�ʽΪ*.osa��ʽ�Ķ���.
/** \brief
*  �����м��붯���ļ�,�������Ķ����ļ����ڵ�ǰ�������Ұ��,����Ҫ��Ⱦ�¼����
*  ����.
*
*  \param _mc �����������ݵĽṹ,�������ṹ,�������ǵĶ�������.
*  \param _bool _topLayer �Ƿ����ϲ���Ⱦ�����ڻغ��Ƶ����
*/
inline int osc_TGManager::create_osaAmimation( os_aniMeshCreate* _mc,bool _topLayer/* = false*/ )
{
	BOOL    t_b;

	return g_ptrMeshMgr->create_aniMeshFromFile( _mc,t_b,false,_topLayer );

}

//! ����ĳһ��������λ�á�
inline void osc_TGManager::set_aniMeshPos( int _id,osVec3D& _pos )
{
	g_ptrMeshMgr->set_aniMeshPos( _id,_pos );
}
//! ����һ����������ת����.
inline void osc_TGManager::set_aniMeshRot( int _id,float _angle )
{
	g_ptrMeshMgr->set_aniMeshRot( _id,_angle ); 
}
//! ���ŵ�ǰ��osa����
inline void osc_TGManager::scale_aniMesh( int _id,float _rate,float _time )
{
	g_ptrMeshMgr->scale_aniMesh( _id,_rate,_time );
}


/** \brief
*  ����osa�����Ĳ���ʱ�䡣
*  
*   \return ������Ҫ����osa��������������ʱ�䡣
*   \param _timeScale Ҫ���õ�osa�����Ĳ���ʱ������������ʱ����������Ϊ��ֵ��
*                �ӿ�ֻ���ع�������������ʱ�䣬�����ò���ʱ�䡣
*/
inline float osc_TGManager::set_osaPlayTime( int _id,float _timeScale )
{
	guard;

	float    t_fRes;

	t_fRes = g_ptrMeshMgr->get_osaPlayTime( _id );

	if( _timeScale > 0.0f )
		g_ptrMeshMgr->set_osaPlayTime( _id,_timeScale );

	return t_fRes;

	unguard;
}


/** \brief
*  ����һ���������������
*
*  ����ӿ���Ҫ����һЩ��Ч�Ĳ��ţ���Ҫ����Ϸ���������������ݡ�
*/
inline void osc_TGManager::set_aniMeshTrans( int _id,osMatrix& _transMat ) 
{
	g_ptrMeshMgr->set_aniMeshTrans( _id,_transMat );
}






//! ��ʼ����,�Զ�����ʱ���֡��صı������ã�ʹ��ͷ��ʼ������
inline void osc_TGManager::start_animationMesh( int _id )
{
	g_ptrMeshMgr->start_animationMesh( _id );
}
//! fade����fadeIn��ǰ��osa����.
inline void osc_TGManager::start_fadeOsa( int _id,
	      float _fadeTime,bool _fadeOut/* = true*/,float _minFade/* = 0.0f*/ )
{
	g_ptrMeshMgr->start_fadeOsa( _id,_fadeTime,_fadeOut,_minFade );
}
//! ���ص�ǰ��osa����
inline void osc_TGManager::hide_osa( int _id,bool _hide )
{
	g_ptrMeshMgr->set_osaHide( _id,_hide );
}



# if __EFFECT_EDITOR__
//! ǿ�ƽ���һ������.
inline void osc_TGManager::stop_animationMesh( int _id )
{
	osassert( g_ptrMeshMgr );
	g_ptrMeshMgr->stop_animationMesh( _id );
}




//! ����ĳһ��������صĲ���.
inline bool osc_TGManager::set_animationMesh( int _id,os_aniMeshCreate* _mc )
{
	g_ptrMeshMgr->set_animationMesh( _id,_mc );
	return true;
}
# endif 



//! ��Ҫ��֤ÿһ��������Ʒid�ĺϷ��ԡ�
inline  bool osc_TGManager::validate_aniMeshId( int _id )
{
	return g_ptrMeshMgr->validate_aniMeshId( _id );
}


//! ɾ��һ������.
inline void osc_TGManager::delete_animationMesh( int _id,bool _sceC/* = false*/ )
{
	g_ptrMeshMgr->delete_animationMesh( _id,_sceC );
}

inline const char* osc_TGManager::get_aniNameFromId( int _id )
{
	return g_ptrMeshMgr->get_aniNameFromId( _id );
}

//@{
//  ������صĺ����ӿ�.����,���ú�ɾ��.
//! ����һ��particleʵ��.
inline int osc_TGManager::create_particle( 
	os_particleCreate& _pcreate,BOOL _inScene,bool _topLayer/* = false*/ )
{
	return m_particleMgr.create_parSceIns( _pcreate,_inScene,_topLayer );
}

//! ���ص�ǰ��particle.
inline int osc_TGManager::hide_particle( int _id,bool _hide )
{
	return m_particleMgr.hide_particle( _id,_hide );
}


inline bool osc_TGManager::is_loopPlayParticle( int _id ) 
{ 
	return this->m_particleMgr.is_loopPlayParticle( _id ); 
}


//! ����particle��λ��.
inline void osc_TGManager::set_particlePos( int _idx,osVec3D& _pos )
{
	m_particleMgr.set_particlePos( _pos,_idx );
}
inline void osc_TGManager::set_particlePos( int _idx,osVec3D& _pos,bool _forceUpdate )
{
	m_particleMgr.set_particlePos( _pos,_idx,_forceUpdate );
}

//! �������ӷ�ɢ�Ŀ�ʼ�ͽ����߶ε�
inline void osc_TGManager::set_particleLineSeg( int _id,osVec3D& _start,osVec3D& _end )
{
	m_particleMgr.set_parGenLineSeg( _id,_start,_end );
}


/** \brief
*  �������Ӷ����Ĳ���ʱ�䡣
*  
*   \return ������Ҫ�������Ӷ�������������ʱ�䡣
*   \param _timeScale Ҫ���õ����Ӷ����Ĳ���ʱ������������ʱ��ı��������Ϊ��ֵ��
*                �ӿ�ֻ���ع�������������ʱ�䣬�����ò���ʱ�䡣
*/
inline float osc_TGManager::set_parPlayTime( int _id,float _timeScale )
{
	guard;

	float    t_fRes = 0.0f;

	t_fRes = m_particleMgr.get_parPlayTime( _id );
	if( _timeScale > 0.0f )
		m_particleMgr.set_parPlayTime( _id,_timeScale );
	
	return t_fRes;

	unguard;
}


//! ���particleId�Ŀ����ԣ���������ã����ϲ���Ҫ����ǰ��particle Id.
inline bool osc_TGManager::validate_parId( int _id )
{
	return m_particleMgr.validate_parId( _id );
}


//! ɾ��һ��particle.
inline void osc_TGManager::delete_particle( int _idx,bool _finalRelease/* = false*/ )
{
	m_particleMgr.delete_particle( _idx,_finalRelease );
}

//! ������Чɾ���봴������
inline const char* osc_TGManager::get_parNameFromId( int _id )
{
	return m_particleMgr.get_parNameFromId( _id );
}
//@} 

//@} 





//! ��֤billBoardId����Ч�ԡ�
inline bool osc_TGManager::validate_bbId( int _id )
{
	return m_bbMgr->validate_bbId( _id );
}


//@{
// ����Ӱ��صĽӿڣ�����,npc�͹���������һ����ɫ��СȦ��
inline int   osc_TGManager::create_fakeShadow( osVec3D& _pos,float _scale/* = 1.0f*/ )
{
	os_decalInit   t_di;

	t_di.m_dwDecalColor = 0xffffffff;
	t_di.m_fRotAgl = 0.0f;
	t_di.m_fSize = g_fFakeShadowSize*_scale;
	t_di.m_vec3Pos = _pos;

	return m_sDecalMgr.create_fakeShadow( t_di );  
}
inline void osc_TGManager::set_shadowPos( int _id,osVec3D& _pos )
{
	m_sDecalMgr.reset_decalPos( _id,_pos );
}
inline void osc_TGManager::delete_shadowId( int _id )
{
	m_sDecalMgr.delete_decal( _id );
}
inline void osc_TGManager::hide_shadow( int _id,bool _hide/* = true*/ )
{
	m_sDecalMgr.hide_decal( _id,_hide );
}
//@} 

//! ������ɾ�������ر��ϵ�Decal
inline int osc_TGManager::create_decal( os_decalInit& _dinit )
{
	return m_sDecalMgr.create_decal( _dinit );
}
inline void osc_TGManager::delete_decal( int _id )
{
	m_sDecalMgr.delete_decal( _id );
}
inline void osc_TGManager::reset_decalPos( int _id,osVec3D& _pos )
{
	m_sDecalMgr.reset_decalPos( _id,_pos );
}



//@{
//! �����п�ʼ�ͽ�������Ч����
inline void osc_TGManager::start_sceneRain( os_rainStruct* _rs/* = NULL*/ )
{
	m_effectRain.start_rain( _rs );
}
inline void osc_TGManager::stop_sceneRain( void )
{
	m_effectRain.stop_rain();
}
//! �����ڿ�ʼ��ѩ
inline void osc_TGManager::scene_snowStorm( BOOL _start/* = TRUE*/,int _grade/* = 0*/ )
{

}
//@} 



/** \brief
*  �õ���ǰ֡��Ⱦ����������Ŀ��
*/
inline int osc_TGManager::get_renderTriNum( void )
{
	return m_iRenderTriNum;
}




//! �õ���ǰ�����п��Կ����ĸ��ӵ���Ŀ��
inline int osc_TGManager::get_inViewTileNum( void )
{
	int   t_iNum = 0;
	for( int t_i=0;t_i<this->m_iActMapNum;t_i ++ )
	{
		osassert( m_arrPtrTG[t_i] );
	
	}

	//! ���ص�����Ⱦ���ӵ���Ŀ
	return t_iNum;

}




//@{
//  �����й����صĽӿڡ�
/** \brief
*  ʹ���ļ���������ꡣ
*/
inline int osc_TGManager::create_cursor( const char* _cursorName )
{
	osc_d3dManager* t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	return t_ptrMgr->create_cursor( _cursorName );
}
//! ������������ʾ�Ĺ�ꡣ
inline void osc_TGManager::set_cursor( int _id, int _centerX/*=0*/, int _centerY/*=0*/ )
{
	osc_d3dManager* t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	t_ptrMgr->set_cursor( _id, _centerX, _centerY );
}

//! �����������ʾ�Ĺ��
inline int osc_TGManager::get_cursor()
{
	osc_d3dManager* t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	osassert( t_ptrMgr );
	return t_ptrMgr->get_cursor();
}

inline void osc_TGManager::hide_cursor( bool _hide/* = false*/ )
{
	osc_d3dManager* t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	t_ptrMgr->hide_corsor( _hide );
}
//@} 


//! ��ʱ���µ�ǰid�����ʵ����λ��
inline void osc_TGManager::update_bbPos( int _id,I_camera* _camptr )
{
	m_bbMgr->update_bbPos( _id,_camptr );
}
//! ��ʱ����osa��λ��
inline void osc_TGManager::update_osaPosNRot( int _id )
{
	g_ptrMeshMgr->update_osaPosNRot( _id );
}

//! ����osa����Ⱦ״̬��ʹ��һ����Ⱦ��osa��ʱ���ܹ�����Ⱦ
inline void osc_TGManager::update_osaRenderState( int _id )
{
	g_ptrMeshMgr->reset_osaMeshInsRender( _id );
}

//! ����particle����Ⱦ״̬��ʹ��һ����Ⱦ��par��ʱ���ܱ���Ⱦ
inline void osc_TGManager::update_parRenderState( int _id )
{
	m_particleMgr.reset_particleInsRender( _id );
}


/** \brief
*  �������м���billBoard.
*
*  �����е�Billboard�ڳ�ʼ����ʱ��̶�λ�ã��Ժ��ܸı䡣
*  
*  ATTENTION: ����Ǽ�����ѭ�����ŵĹ���壬����ɾ����
*             ���й�������Ҫ�Ǽ�������һ�εĹ���嶯����������ɺ󣬻��Լ�ɾ���Լ���
*/
inline int osc_TGManager::create_billBoard( const char *_bfile,osVec3D& _pos,
	bool _createIns/* = true*/,float _sizeScale/* = 1.0f*/,bool _topLayer/* = false*/  )
{
	return m_bbMgr->create_billBoard( _bfile,_pos,_createIns,_sizeScale,_topLayer );
}

//! �ڳ����ڴ�������Զ������Ӱ��Ĺ����,�����Ĳ����������Ĳ���һ��
inline int osc_TGManager::create_hugeBillBoard( const char* _bfile,osVec3D& _pos,
							bool _createIns/* = true*/,float _sizeScale/* = 1.0f*/ )
{
	return m_bbMgr->create_hugeBillBoard( _bfile,_pos,_createIns,_sizeScale );
}

//! �ͷ�һ���������Ʒ��
inline void osc_TGManager::release_billBoard( int _id )
{
	m_bbMgr->delete_billBoard( _id );
}


/** \brief
*  ����billBoard��λ�á�һ�������еĹ���壬���ܿ���λ�á�
*
*  \param _id ��Ҫ���õĹ�����id��
*  \param _pos  Ҫ���õĹ�����λ�á�
*/
inline void osc_TGManager::set_billBoardPos( int _id,osVec3D& _pos )
{
	m_bbMgr->set_billBoardPos( _id,_pos,true );
}

//! ���ù����ʹ�õ���ת��
inline void osc_TGManager::set_bbRotAxis( int _id,osVec3D& _axis )
{
	m_bbMgr->set_bbRotAxis( _id,_axis );
}


/** \brief
*  ���ù����Ĳ���ʱ�䡣
*  
*   \return ������Ҫ���ù�������������ʱ�䡣
*   \param _timeScale Ҫ���õĹ����Ĳ���ʱ������������ʱ��ı��������Ϊ��ֵ���ӿ�ֻ���ع����
*                ����������ʱ�䣬�����ò���ʱ�䡣
*/
inline float osc_TGManager::set_bbPlayTime( int _id,float _timeScale )
{
	float    t_fRes;

	t_fRes = m_bbMgr->get_bbPlayTime( _id );

	if( _timeScale > 0.0f )
		m_bbMgr->set_bbPlayTime( _id,_timeScale );

	return t_fRes;
}


/** \brief
*  ɾ��billBoard.
*
*  \param _id ɾ��ָ��id��billBoard.
*  
*/
inline void osc_TGManager::delete_billBoard( int _id,bool _finalRelease/* = false*/ )
{
	m_bbMgr->delete_billBoard( _id,_finalRelease );
}

// River @ 2010-12-24: ��id�õ�billBoard���ļ���.
inline const char* osc_TGManager::get_bbNameFromId( int _id )
{
	return m_bbMgr->get_bbNameFromId( _id );
}


//! ���ص�ǰ��billBoard.
inline void osc_TGManager::hide_billBoard( int _id,bool _hide )
{
	m_bbMgr->hide_bb( _id,_hide );
}

//! ���ŵ�ǰ�Ĺ����.river added @ 2010-1-7:
inline void osc_TGManager::scale_billBoard( int _id,float _rate )
{
	//this->m_bbMgr->sca
	;
}



/** \brief
*  �õ���ͼ�е�һ�������Ƿ��ڿ��ӷ�Χ�ڡ�
*
*  ��tgMgrʹ���Լ��ķ����õ���
*/
inline bool osc_TGManager::is_ptInView( osVec3D& _vec )
{
	os_ViewFrustum*    m_frus;
    
	//
	// Ŀǰ��⵱ǰ�Ķ����Ƿ��������ڡ�
	m_frus = g_ptrCamera->get_frustum();
	if( m_frus->objInVF( _vec ) )
		return true;
	else
		return false;


	return true;
}



/** \brief
*  ���sphere,�鿴�Ƿ��ڳ�����.
*/
inline bool osc_TGManager::is_sphereInView( os_bsphere& _sphere,bool _lod/* = false*/ )
{
	os_ViewFrustum*    m_frus;
    
	//
	// Ŀǰ��⵱ǰ�Ķ����Ƿ��������ڡ�
	m_frus = g_ptrCamera->get_frustum();
	if( m_frus->objInVF( _sphere ) )
	{
		if( _lod )
		{
			// 
			// �����Ʒ�İ뾶��С���򳬹�һ���Ӿص�ʱ�򣬿��ܻῴ������
			if( MASK_OUTHALF_NOTVIEW == 
				g_ptrCamera->is_viewByCamera( _sphere.veccen,_sphere.radius ) )
				return false;
		}
		return true;

	}
	else
		return false;
}
