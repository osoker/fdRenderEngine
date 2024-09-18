

/** \brief
*  得到当前的相机所在的TG.
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



//! River added @ 2009-7-23:全局得到水面的高度
inline float osc_TGManager::get_waterSurfaceHeight( void )
{
	/*
	//! 如果高度过高，则此值可能没有初始化
	if( m_sSceneTexData.m_fWaterHeight > 300.0f )
		return -1000.0f;
	else
		return m_sSceneTexData.m_fWaterHeight;
	*/
	return m_sSceneTexData.get_waterHeight();
}



/** \brief
*  得到当前场景中主相机的frustum.
*/
inline os_ViewFrustum* osc_TGManager::get_camFrustum( void )
{
	if( g_ptrCamera )
		return g_ptrCamera->get_frustum();
	else 
		return NULL;
}



//@{
// 动态光相关的接口。
/** \brief
*  创建一个动态光。
*
*  \return int  如果返回-1,则创建失败。
*/
inline int osc_TGManager::create_dynamicLight( os_dlightStruct& _dl )
{
	return m_dlightMgr.create_dynamicLight( _dl );
}



/** \brief
*  得到一个动态光的各项参数。
*/
inline void osc_TGManager::get_dynamicLight( int _id,os_dlightStruct& _dl )
{
	m_dlightMgr.get_dynamicLight( _id,_dl );
}

/** \brief
*  FrameMove一个动态光。
*/
inline void osc_TGManager::set_dynamicLight( int _id,os_dlightStruct& _dl )
{
	m_dlightMgr.set_dynamicLight( _id,_dl );
}

//! 设置动态光的位置。
inline void osc_TGManager::set_dLightPos( int _id,osVec3D& _pos )
{
	m_dlightMgr.set_dLightPos( _id,_pos );
}

/** \brief
*  删除一个动态光。
*/
inline void osc_TGManager::delete_dynamicLight( int _id,float _fadeTime/* = 0.0f*/ )
{
	m_dlightMgr.delete_dynamicLight( _id,_fadeTime );
}



/** \brief
*  得到场景中的环境光数据。
*/
inline DWORD osc_TGManager::get_ambientLight( void )        
{ 
	return DWORD( osColor( g_fAmbiR,g_fAmbiG,g_fAmbiB,1.0f ) );
}

//@}




//@{
// 
// 场景中动画相关的函数.此处的动画非骨骼动画.
// 动画文件的格式为*.osa格式的动画.
/** \brief
*  场景中加入动画文件,如果加入的动画文件不在当前相机的视野内,则不需要渲染新加入的
*  动画.
*
*  \param _mc 创建动画数据的结构,填充这个结构,创建我们的动画数据.
*  \param _bool _topLayer 是否最上层渲染，用于回合制的情况
*/
inline int osc_TGManager::create_osaAmimation( os_aniMeshCreate* _mc,bool _topLayer/* = false*/ )
{
	BOOL    t_b;

	return g_ptrMeshMgr->create_aniMeshFromFile( _mc,t_b,false,_topLayer );

}

//! 设置某一个动画的位置。
inline void osc_TGManager::set_aniMeshPos( int _id,osVec3D& _pos )
{
	g_ptrMeshMgr->set_aniMeshPos( _id,_pos );
}
//! 设置一个动作的旋转方向.
inline void osc_TGManager::set_aniMeshRot( int _id,float _angle )
{
	g_ptrMeshMgr->set_aniMeshRot( _id,_angle ); 
}
//! 缩放当前的osa动画
inline void osc_TGManager::scale_aniMesh( int _id,float _rate,float _time )
{
	g_ptrMeshMgr->scale_aniMesh( _id,_rate,_time );
}


/** \brief
*  设置osa动画的播放时间。
*  
*   \return 返回了要设置osa动画的正常播放时间。
*   \param _timeScale 要设置的osa动画的播放时间与正常播放时间比例，如果为负值，
*                接口只返回公告板的正常播放时间，不设置播放时间。
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
*  设置一个动画的世界矩阵。
*
*  这个接口主要用于一些特效的播放，需要在游戏层填充世界矩阵数据。
*/
inline void osc_TGManager::set_aniMeshTrans( int _id,osMatrix& _transMat ) 
{
	g_ptrMeshMgr->set_aniMeshTrans( _id,_transMat );
}






//! 开始动画,对动画中时间和帧相关的变量重置，使从头开始动画。
inline void osc_TGManager::start_animationMesh( int _id )
{
	g_ptrMeshMgr->start_animationMesh( _id );
}
//! fade或是fadeIn当前的osa动画.
inline void osc_TGManager::start_fadeOsa( int _id,
	      float _fadeTime,bool _fadeOut/* = true*/,float _minFade/* = 0.0f*/ )
{
	g_ptrMeshMgr->start_fadeOsa( _id,_fadeTime,_fadeOut,_minFade );
}
//! 隐藏当前的osa动画
inline void osc_TGManager::hide_osa( int _id,bool _hide )
{
	g_ptrMeshMgr->set_osaHide( _id,_hide );
}



# if __EFFECT_EDITOR__
//! 强制结束一个动画.
inline void osc_TGManager::stop_animationMesh( int _id )
{
	osassert( g_ptrMeshMgr );
	g_ptrMeshMgr->stop_animationMesh( _id );
}




//! 设置某一个动画相关的参数.
inline bool osc_TGManager::set_animationMesh( int _id,os_aniMeshCreate* _mc )
{
	g_ptrMeshMgr->set_animationMesh( _id,_mc );
	return true;
}
# endif 



//! 需要验证每一个动画物品id的合法性。
inline  bool osc_TGManager::validate_aniMeshId( int _id )
{
	return g_ptrMeshMgr->validate_aniMeshId( _id );
}


//! 删除一个动画.
inline void osc_TGManager::delete_animationMesh( int _id,bool _sceC/* = false*/ )
{
	g_ptrMeshMgr->delete_animationMesh( _id,_sceC );
}

inline const char* osc_TGManager::get_aniNameFromId( int _id )
{
	return g_ptrMeshMgr->get_aniNameFromId( _id );
}

//@{
//  粒子相关的函数接口.创建,设置和删除.
//! 创建一个particle实例.
inline int osc_TGManager::create_particle( 
	os_particleCreate& _pcreate,BOOL _inScene,bool _topLayer/* = false*/ )
{
	return m_particleMgr.create_parSceIns( _pcreate,_inScene,_topLayer );
}

//! 隐藏当前的particle.
inline int osc_TGManager::hide_particle( int _id,bool _hide )
{
	return m_particleMgr.hide_particle( _id,_hide );
}


inline bool osc_TGManager::is_loopPlayParticle( int _id ) 
{ 
	return this->m_particleMgr.is_loopPlayParticle( _id ); 
}


//! 设置particle的位置.
inline void osc_TGManager::set_particlePos( int _idx,osVec3D& _pos )
{
	m_particleMgr.set_particlePos( _pos,_idx );
}
inline void osc_TGManager::set_particlePos( int _idx,osVec3D& _pos,bool _forceUpdate )
{
	m_particleMgr.set_particlePos( _pos,_idx,_forceUpdate );
}

//! 设置粒子发散的开始和结束线段点
inline void osc_TGManager::set_particleLineSeg( int _id,osVec3D& _start,osVec3D& _end )
{
	m_particleMgr.set_parGenLineSeg( _id,_start,_end );
}


/** \brief
*  设置粒子动画的播放时间。
*  
*   \return 返回了要设置粒子动画的正常播放时间。
*   \param _timeScale 要设置的粒子动画的播放时间与正常播放时间的比例，如果为负值，
*                接口只返回公告板的正常播放时间，不设置播放时间。
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


//! 检测particleId的可用性，如果不可用，则上层需要处理当前的particle Id.
inline bool osc_TGManager::validate_parId( int _id )
{
	return m_particleMgr.validate_parId( _id );
}


//! 删除一个particle.
inline void osc_TGManager::delete_particle( int _idx,bool _finalRelease/* = false*/ )
{
	m_particleMgr.delete_particle( _idx,_finalRelease );
}

//! 减少特效删除与创建操作
inline const char* osc_TGManager::get_parNameFromId( int _id )
{
	return m_particleMgr.get_parNameFromId( _id );
}
//@} 

//@} 





//! 验证billBoardId的有效性。
inline bool osc_TGManager::validate_bbId( int _id )
{
	return m_bbMgr->validate_bbId( _id );
}


//@{
// 简单阴影相关的接口，人物,npc和怪物下面有一个黑色的小圈。
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

//! 创建和删除场景地表上的Decal
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
//! 场景中开始和结束下雨效果。
inline void osc_TGManager::start_sceneRain( os_rainStruct* _rs/* = NULL*/ )
{
	m_effectRain.start_rain( _rs );
}
inline void osc_TGManager::stop_sceneRain( void )
{
	m_effectRain.stop_rain();
}
//! 场景内开始下雪
inline void osc_TGManager::scene_snowStorm( BOOL _start/* = TRUE*/,int _grade/* = 0*/ )
{

}
//@} 



/** \brief
*  得到当前帧渲染的三角形数目。
*/
inline int osc_TGManager::get_renderTriNum( void )
{
	return m_iRenderTriNum;
}




//! 得到当前场景中可以看到的格子的数目。
inline int osc_TGManager::get_inViewTileNum( void )
{
	int   t_iNum = 0;
	for( int t_i=0;t_i<this->m_iActMapNum;t_i ++ )
	{
		osassert( m_arrPtrTG[t_i] );
	
	}

	//! 返回的是渲染格子的数目
	return t_iNum;

}




//@{
//  程序中光标相关的接口。
/** \brief
*  使用文件名创建光标。
*/
inline int osc_TGManager::create_cursor( const char* _cursorName )
{
	osc_d3dManager* t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	return t_ptrMgr->create_cursor( _cursorName );
}
//! 设置引擎中显示的光标。
inline void osc_TGManager::set_cursor( int _id, int _centerX/*=0*/, int _centerY/*=0*/ )
{
	osc_d3dManager* t_ptrMgr = (osc_d3dManager*)m_deviceMgr;
	t_ptrMgr->set_cursor( _id, _centerX, _centerY );
}

//! 获得引擎中显示的光标
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


//! 即时更新当前id公告板实例的位置
inline void osc_TGManager::update_bbPos( int _id,I_camera* _camptr )
{
	m_bbMgr->update_bbPos( _id,_camptr );
}
//! 即时更新osa的位置
inline void osc_TGManager::update_osaPosNRot( int _id )
{
	g_ptrMeshMgr->update_osaPosNRot( _id );
}

//! 更新osa的渲染状态，使下一次渲染此osa的时候能够被渲染
inline void osc_TGManager::update_osaRenderState( int _id )
{
	g_ptrMeshMgr->reset_osaMeshInsRender( _id );
}

//! 更新particle的渲染状态，使下一次渲染此par的时候能被渲染
inline void osc_TGManager::update_parRenderState( int _id )
{
	m_particleMgr.reset_particleInsRender( _id );
}


/** \brief
*  往场景中加入billBoard.
*
*  场景中的Billboard在初始化的时候固定位置，以后不能改变。
*  
*  ATTENTION: 如果是加入了循环播放的公告板，则不能删除。
*             运行过程中主要是加入运行一次的公告板动画，播放完成后，会自己删除自己。
*/
inline int osc_TGManager::create_billBoard( const char *_bfile,osVec3D& _pos,
	bool _createIns/* = true*/,float _sizeScale/* = 1.0f*/,bool _topLayer/* = false*/  )
{
	return m_bbMgr->create_billBoard( _bfile,_pos,_createIns,_sizeScale,_topLayer );
}

//! 在场景内创建不受远剪切面影响的公告板,其它的操作跟公告板的操作一致
inline int osc_TGManager::create_hugeBillBoard( const char* _bfile,osVec3D& _pos,
							bool _createIns/* = true*/,float _sizeScale/* = 1.0f*/ )
{
	return m_bbMgr->create_hugeBillBoard( _bfile,_pos,_createIns,_sizeScale );
}

//! 释放一个公告板物品。
inline void osc_TGManager::release_billBoard( int _id )
{
	m_bbMgr->delete_billBoard( _id );
}


/** \brief
*  控制billBoard的位置。一次性运行的公告板，不能控制位置。
*
*  \param _id 　要设置的公告板的id。
*  \param _pos  要设置的公告板的位置。
*/
inline void osc_TGManager::set_billBoardPos( int _id,osVec3D& _pos )
{
	m_bbMgr->set_billBoardPos( _id,_pos,true );
}

//! 设置公告板使用的旋转轴
inline void osc_TGManager::set_bbRotAxis( int _id,osVec3D& _axis )
{
	m_bbMgr->set_bbRotAxis( _id,_axis );
}


/** \brief
*  设置公告板的播放时间。
*  
*   \return 返回了要设置公告板的正常播放时间。
*   \param _timeScale 要设置的公告板的播放时间与正常播放时间的比例，如果为负值，接口只返回公告板
*                的正常播放时间，不设置播放时间。
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
*  删除billBoard.
*
*  \param _id 删除指定id的billBoard.
*  
*/
inline void osc_TGManager::delete_billBoard( int _id,bool _finalRelease/* = false*/ )
{
	m_bbMgr->delete_billBoard( _id,_finalRelease );
}

// River @ 2010-12-24: 从id得到billBoard的文件名.
inline const char* osc_TGManager::get_bbNameFromId( int _id )
{
	return m_bbMgr->get_bbNameFromId( _id );
}


//! 隐藏当前的billBoard.
inline void osc_TGManager::hide_billBoard( int _id,bool _hide )
{
	m_bbMgr->hide_bb( _id,_hide );
}

//! 缩放当前的公告板.river added @ 2010-1-7:
inline void osc_TGManager::scale_billBoard( int _id,float _rate )
{
	//this->m_bbMgr->sca
	;
}



/** \brief
*  得到地图中的一个顶点是否在可视范围内。
*
*  由tgMgr使用自己的方法得到。
*/
inline bool osc_TGManager::is_ptInView( osVec3D& _vec )
{
	os_ViewFrustum*    m_frus;
    
	//
	// 目前检测当前的顶点是否在视域内。
	m_frus = g_ptrCamera->get_frustum();
	if( m_frus->objInVF( _vec ) )
		return true;
	else
		return false;


	return true;
}



/** \brief
*  检测sphere,查看是否在场景中.
*/
inline bool osc_TGManager::is_sphereInView( os_bsphere& _sphere,bool _lod/* = false*/ )
{
	os_ViewFrustum*    m_frus;
    
	//
	// 目前检测当前的顶点是否在视域内。
	m_frus = g_ptrCamera->get_frustum();
	if( m_frus->objInVF( _sphere ) )
	{
		if( _lod )
		{
			// 
			// 如果物品的半径过小，则超过一半视矩的时候，可能会看不见。
			if( MASK_OUTHALF_NOTVIEW == 
				g_ptrCamera->is_viewByCamera( _sphere.veccen,_sphere.radius ) )
				return false;
		}
		return true;

	}
	else
		return false;
}
