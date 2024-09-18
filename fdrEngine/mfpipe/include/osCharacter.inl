

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  　　　　　　　　　　开始当前文件中用到的Inline函数
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


/** \brief
*  设置当前动作的播放速度.
*/
inline void os_boneAction::set_actPlaySpeed( float _speed )
{
	m_iPlaySpeed = int(m_iNumFrame * _speed);
	m_fTimeToPlay = 1.0f/_speed; 
}

/** \brief
*  设置当前动作的名字。
*/
inline void os_boneAction::set_actName( const char *_name )
{
	strcpy( m_szActName,_name );
}


//! 使用id得到我们需要的设备相关数据。
inline os_smMeshDevdep* osc_smDepdevResMgr::get_smMesh( int _id )
{
	return &m_vecDevdepRes[_id];
}


//! 得以bpmInstance的面数.
inline int osc_bpmInstance::get_faceNum( void )      
{ 
	if( m_ptrBodyPartMesh )
		return m_ptrBodyPartMesh->m_dwNumIndices/3; 
	else
		return 0;
} 

inline bool osc_bpmInstance::has_secondEffect( void )
{ 
	if( m_ptrSecEffect ) 
		return true;
	else 
		return false; 
}


inline osMatrix* osc_bpmInstance::get_texTransMat( void )
{
	osassert( m_ptrSecEffect );
	m_ptrSecEffect->frame_moveSecEff();

	return &m_ptrSecEffect->m_matFinal;
}

inline 	bool osc_bpmInstance::is_shineTexEffect( void )
{
	if( m_ptrSecEffect->m_wEffectType == 7 )
		return true;
	else
		return false;
}

//! river added @ 2010-3-11:更多种类的贴图.
inline bool osc_bpmInstance::is_rotEnvEffect( void )
{
	if( m_ptrSecEffect->m_wEffectType == 8 )
		return true;
	else
		return false;
}

inline bool osc_bpmInstance::is_rotShineEffect( void )
{
	if( m_ptrSecEffect->m_wEffectType == 9 )
		return true;
	else
		return false;
}

inline bool osc_bpmInstance::is_envShineEffect( void )
{
	if( m_ptrSecEffect->m_wEffectType == 10 )
		return true;
	else
		return false;
}


inline bool osc_bpmInstance::is_rottexEffect( void )
{
	if( m_ptrSecEffect->m_wEffectType == 6 )
		return true;
	else
		return false;
}

//! 当前BodyPartMesh实例是否使用刀光。
inline bool osc_bpmInstance::is_glossySM( void ) 
{ 
	if( m_ptrSGIns ) 
		return true;
	else 
		return false; 
} 

//! 缩放武器的特效数据
inline void osc_bpmInstance::scale_weaponEffect( float _rate )
{
	if( m_ptrSGIns )
		m_ptrSGIns->scale_weaponEffect( _rate );
}



///////////////////////////////////////////////////////////////////////////////////////////////////////
// 刀光用到的inline 函数。
//! 开始渲染刀光。
inline DWORD osc_swordGlossyIns::start_swordGlossy( int _idx )
{
	m_iPRSword = _idx;


	osassert( m_ptrSwordGlossy );

	int   t_iActIdx = -1;

	// 只有要设置的动作大于零,才起作用.
	if( m_iPRSword >= 0 )
	{
		for( int t_i=0;t_i<m_ptrSwordGlossy->m_iActNum;t_i ++ )
		{
			if( _idx == m_ptrSwordGlossy->m_vecActIdx[t_i] )
			{
				t_iActIdx = t_i;
				m_dwGlossyColor = 
					m_ptrSwordGlossy->m_vecGlossyColor[t_i];

				break;
			}
		}
	}
# if __GLOSSY_EDITOR__

	if( t_iActIdx == -1 )
		m_dwGlossyColor = 0xffff0000;

# else

	if( t_iActIdx == -1 )
		m_dwGlossyColor = 0;

# endif 

	reset_swordGlossy();

	return m_dwGlossyColor;

}

//! 设置刀光实例指向的刀光数据。
inline void osc_swordGlossyIns::set_swordGlossyPtr( osc_swordGlossy* _sg,osc_skinMesh* _mesh )
{
	osassert( _sg );
	m_ptrSwordGlossy = _sg;
	
	osVec3D  t_vec3;
	t_vec3 = m_ptrSwordGlossy->m_vec3End - m_ptrSwordGlossy->m_vec3Start;
	m_ptrSwordGlossy->m_fSgLength = osVec3Length( &t_vec3 );

# if SKINMESH_DEBUG
	m_ptrmesh = _mesh;
# endif 

	m_fScaleWeaponEffect = 1.0f; 

}



//! 重设当前的刀光数据。每一帧开始时，调用这个函数。
inline void osc_swordGlossyIns::reset_swordGlossy( void )
{
	m_iGlossyPos = 0;
	m_bPosOverFlow = false;
}
//! 得到刀光的中心点。
inline void osc_swordGlossyIns::get_glossyCenter( osVec3D& _center )
{
	int   t_idx;

	// ATTENTION TO FIX:
	if( m_bPosOverFlow&&(m_iGlossyPos==0) )
		t_idx = get_sgMoveLength()-1;
	else if( m_iGlossyPos == 0 )
		t_idx = 0;
	else
		t_idx = m_iGlossyPos -1;

	_center = m_arrVec3Start[t_idx] + m_arrVec3End[t_idx];
	_center /= 2.0f;

	return;

}

/** \brief
*  是否需要推进多个刀光位置.
*  
*  如果刀光附加的有特效,而此时又不显示刀光的话,则不需要推进多个刀光位置.
*  如果没有特效,又没有刀光,则完全不需要的推进多个刀光的位置.
*/
inline bool osc_swordGlossyIns::need_pushMultiSPos( void )
{


	if( (m_iEffectType[0]>0)&&
		(m_iEffectType[0]<=3) )
	{
		if( (m_dwGlossyColor & 0xff000000) == 0 )
			return false;
	}

	return true;
}










//!  从一个id中得到Body part Mesh的指针。
inline osc_bodyPartMesh*  osc_bpmMgr::get_bpmPtrFromId( int _id )
{
	return &m_vecBPM[_id];
}


//! 当前的BPM是否在使用中。
inline bool osc_bodyPartMesh::is_inUsed(  void )        
{ 
	return m_bInuse;
}
//! 当前的bpm是否是武器部分。
inline bool osc_bodyPartMesh::is_weaponPart( void )
{
	osassert( m_strFname[0] );
	char   t_ch = m_strFname[ strlen(m_strFname)- WEAPON_PARTOFFSET ];

	if( t_ch == WEAPON_PARTCHAR||t_ch == WEAPON_PART8CHAR )
		return true;
	else
		return false;
}




//! 得到和设置第二层贴图的id.
inline int osc_bpmInstance::get_secTexid( void )
{
	osassert( m_ptrSecEffect );
	return m_ptrSecEffect->m_iSecTexId;
}
inline int osc_bpmInstance::get_ThdTexid(void)
{
	osassert( m_ptrSecEffect );
	return m_ptrSecEffect->m_iThdId;
}
//! River @ 2010-3-11:装备内第四张纹理的ID.
inline int osc_bpmInstance::get_fourthTexid( void )
{
	osassert( m_ptrSecEffect );
	return m_ptrSecEffect->m_iFourthId;
}

inline int osc_bpmInstance::get_shaderId( void )
{
	osassert( m_iShaderID>= 0 );
	return m_iShaderID;
}

inline void osc_bpmInstance::set_secTexid( int _id )
{
	osassert( this->m_ptrSecEffect );
	osassert( _id>=0 );
	m_ptrSecEffect->m_iSecTexId = _id;
}
inline void osc_bpmInstance::set_thdTexid( int _id )
{
	osassert( this->m_ptrSecEffect );
	osassert( _id>=0 );
	m_ptrSecEffect->m_iThdId = _id;
}


//! 设置是否显示刀光。
inline DWORD osc_bpmInstance::set_disSGlossy( int _idx )
{
	if( m_ptrSGIns )
		return m_ptrSGIns->start_swordGlossy( _idx );
	else
		return 0;

}

// 
//! 当前动作索引是否是对应刀光的动作。
inline bool osc_bpmInstance::is_glossyAction( int _id )
{
	if( !m_ptrBodyPartMesh->m_ptrSwordGlossy )
		return false;

	for( int t_i=0;t_i<m_ptrBodyPartMesh->m_ptrSwordGlossy->m_iActNum;t_i ++ )
	{
		if( m_ptrBodyPartMesh->m_ptrSwordGlossy->m_vecActIdx[t_i] == _id )
			return true;
	}

	return false;

}


//! 得到当前刀光的中心点。
inline void osc_bpmInstance::get_glossyCenter( osVec3D& _center )
{
	m_ptrSGIns->get_glossyCenter( _center );
}


//!  Device reset相关.
inline void osc_bpmInstance::bpmins_onLostDevice( void )
{
	if( m_ptrShadow )
		m_ptrShadow->shadow_onLostDevice();

}
inline void osc_bpmInstance::bpmins_onResetDevice( void )
{
	if( m_ptrShadow )
		m_ptrShadow->shadow_onResetDevice();
}





//! 用于当skin不在视域内的时候，播放死亡类显示最后一帧的动作
inline void osc_skinMesh::play_lastFrameStopAction( void )
{

	if( m_actChange.m_changeType != OSE_DIS_NEWACT_LASTFRAME )
		return;

	frame_moveAction( sg_timer::Instance()->get_lastelatime() );

}




//! 得到当前skinMesh的旋转方向，
inline float osc_skinMesh::get_smAgl( void )
{
	return m_fRotAgl;
}

//! 上层更新特效时用到的旋转方向，这个是渲染用到的真正方向。
inline float osc_skinMesh::get_renderSmAgl( void )
{
	return m_fFinalRotAgl+OS_PI/2.0f;
}


//! is hide state?
inline bool	osc_skinMesh::is_hideState(void)const
{
	return m_bHideSm;
}
/** \brief
*  隐藏当前skinMesh的显示。
*
*/
inline void osc_skinMesh::set_hideSkinMesh( bool _hide )
{
	if( m_bHideSm == _hide )
		return;

	//! River @ 2010-10-19:如果此时处于武器隐藏状态，则处理。
	if( _hide )
		hide_weapon( false );

	m_bHideSm = _hide;

	// 如果是要隐藏,则需要删除武器特效,如果是要显示,则需要创建武器特效
	if( m_bHideSm )
	{
		m_bPlaySGlossyBack = m_bPlaySGlossy;
		m_bPlayWeaponEffectBack = m_bPlayWeaponEffect;
		// River added @ 2010-8-18:减小代码开销.
		if( m_iNumParts>6 )
			playSwordGlossyInner( false,false );

		if (mEffectId!=-1)
		{
			//ZeroMemory(mszspeFileName,MAX_PATH);	
			get_sceneMgr()->delete_effect(mEffectId);
			mEffectId = -1;
		}
	}
	else
	{
		// River added @ 2010-8-18:减小代码开销.
		if( m_iNumParts>6 )
			playSwordGlossy( m_bPlaySGlossyBack,m_bPlayWeaponEffectBack );

		//ZeroMemory(mszspeFileName,MAX_PATH);
		if (mszSpeFileName.size()>0&&mEffectId==-1)
		{
			//strcpy(mszspeFileName,_smi->m_szspeFileName);
			//mEffectOffset = _smi->m_spePostion;
			//mszSpeFileName = mszspeFileName;
			mEffectId = get_sceneMgr()->create_effect(mszSpeFileName.c_str(),m_vec3SMPos+mEffectOffset,m_fRotAgl);
		}
	}
	
}




/** \brief
*  设置动作播放的时间和速度相关信息。
*/
inline void osc_skinMesh::set_actSpeedInfo( float _actTime )
{
	m_fNewActSpeed = _actTime;

	if( _actTime < 0.0f )
	{
		m_dwFPS = m_ptrBoneAni->m_vecBATable[m_iAniId].m_iPlaySpeed;
		m_fTPF =  1.0f/float(m_dwFPS);
	}
	else
	{
		m_fTPF = _actTime / 
			float(m_ptrBoneAni->m_vecBATable[m_iAniId].m_iNumFrame);
	}
}

//! 创建人物时,得到初始化动作的id.
inline void osc_skinMesh::get_defaultAct( os_skinMeshInit* _smi )
{
	osassert( _smi );
	osassert( m_ptrBoneAni );

	if( _smi->m_iActIdx >=0 )
		m_iAniId = _smi->m_iActIdx;
	else
	{
		m_iAniId = m_ptrBoneAni->
			get_actIdxFromName( _smi->m_szDefaultAct,m_iAniNameIdx );
		if( m_iAniId < 0 )
		{
			char buffer[256];
			sprintf( buffer, "角色:%s\n找不到默认的动作:%s\n", _smi->m_strSMDir, _smi->m_szDefaultAct );
			MessageBox( NULL,buffer,"出错",MB_OK );
			m_iAniId = 0;
		}
	}

	return;

}


//! River added @ 2009-8-26: 隐藏人物的即时阴影.
inline bool osc_skinMesh::hide_realShadow( bool _hide/* = true*/ )
{
	m_bHideRealShadow = _hide;
	return true;

}
/** \brief
*  得到当前skinMesh的面数
*/
inline int osc_skinMesh::get_smFaceNum( void )
{
	int t_num = 0;
	for( int t_i=0;t_i<this->m_iNumParts;t_i++ )
		t_num += this->m_vecBpmIns[t_i].get_faceNum();

	return t_num;
}

//! 得到从动作开始到动作停止帧所用的时间。
inline float osc_skinMesh::get_timeFromStartToSF( void )
{
	// 检测是否处于动作停止状态。
	os_boneAction*  t_ptrBA;
	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];

	return m_fTPF*t_ptrBA->m_iStopFrameIdx;
}


/** \brief
*  对两个动作进行插件，填充插值后产生的矩阵。
*/
inline void osc_skinMesh::lerp_actionPose( osMatrix& _mat,
								   os_bactPose* _pa,os_bactPose* _pb,float _lf )
{
	osQuat  t_quat;
	osVec3D t_vec3;

	// River @ 2009-6-17:如果以后此处出错，可直接跳过动作熔合，使用新的动作数据
	if( _pa->m_bFlipDeterminant == _pb->m_bFlipDeterminant )
		osQuaternionSlerp( &t_quat,&_pa->m_actQuat,&_pb->m_actQuat,_lf );
	else
		t_quat = _pb->m_actQuat;

	osMatrixRotationQuaternion( &_mat,&t_quat );

	// ATTENTION TO OPP: 此处可以优化,尽量把quaternion预处理成正成的quaternion.
	if( _pa->m_bFlipDeterminant )
		osMatrixMultiply( &_mat,&_mat,&osc_skinMesh::m_sFDMatrix );	

	osVec3Lerp( (osVec3D*)&_mat._41,&_pa->m_vec3Offset,&_pb->m_vec3Offset,_lf );

	return;
	
}

//  River Added @ 2007-4-20:加入新的动作插值方式。
//! 对两个动作插值，产生一个新的pose,使用这个pose去跟新的动作再插值。
inline void osc_skinMesh::lerp_actionPose( os_bactPose* _res,
								  os_bactPose* _pa,os_bactPose* _pb,float _lf )
{
	osQuat  t_quat;
	osVec3D t_vec3;

	// River @ 2009-6-17:如果以后此处出错，可直接跳过动作熔合，使用新的动作数据
	if( _pa->m_bFlipDeterminant == _pb->m_bFlipDeterminant )
		osQuaternionSlerp( &_res->m_actQuat,&_pa->m_actQuat,&_pb->m_actQuat,_lf );
	else
		_res->m_actQuat = _pb->m_actQuat;
	

	osVec3Lerp( (osVec3D*)&_res->m_vec3Offset,&_pa->m_vec3Offset,&_pb->m_vec3Offset,_lf );
	_res->m_bFlipDeterminant = _pa->m_bFlipDeterminant;

	return;
}


//! 从当前的动作得到矩阵。
inline void os_bactPose::get_posMatrix( osMatrix& _mat )
{
	osMatrixRotationQuaternion( &_mat,&m_actQuat );
	if( m_bFlipDeterminant )
		osMatrixMultiply( &_mat,&_mat,&osc_skinMesh::m_sFDMatrix );
	_mat._41 = m_vec3Offset.x;
	_mat._42 = m_vec3Offset.y;
	_mat._43 = m_vec3Offset.z;

}


/** \brief
*  人物动作是否到达当前动作的关键帧
*
*  到达关键帧之前返回值为假，到达之后返回值为真。
*  
*/
inline bool osc_skinMesh::arrive_stopFrameOrNot( void )
{
	return m_bArriveStopFrame;
}

//! 使停止状态的动作继续播放。
inline void osc_skinMesh::start_moveAction( void )
{
	m_bActStopStatus = false;
}

/** \brief
*  从当前的character得到一个boundingBox.
*/
inline os_bbox* osc_skinMesh::get_smBBox( void )
{
	return &m_sBBox;
}

/** \brief
*  从当前的character得到一个boundingBox.
*/
inline const os_bbox* osc_skinMesh::get_smBBox( void )const
{
	return &m_sBBox;
}

inline float osc_skinMesh::get_smBSRadius( void )
{
	return this->m_fBSRadius;
}




/** \brief
*  切换人物的武器特效
*
*  /param int _idx           切换到第_idx个特效，超出索引的话，都使第零个特效。
*  /param WEAPON_ID _weapid  使用第七还是第八个武器部位。
*/
inline void osc_skinMesh::change_weaponEffect( int _idx,WEAPON_ID _weapid/* = WEAPON7*/ )
{
	if( m_vecBpmIns[_weapid].m_ptrSGIns )
		m_vecBpmIns[_weapid].m_ptrSGIns->change_weaponEffect( _idx );
}


//! 得到当前帧刀光的中心点。
inline void osc_skinMesh::get_swordGlossyCenter( osVec3D& _pos )
{
	m_ptrSGBpmIns->get_glossyCenter( _pos );
}


//! 传入一个环境光值，系统和当前的场景环境光值做比较，使用更亮的光照亮当前角色。
inline void osc_skinMesh::set_maxBright( osColor& _bcolor )
{
	m_sCurSkinMeshAmbi = _bcolor;
	m_sCurSkinMeshAmbi.a = 1.0f;
	m_bSetHighLight = true;
}

//! Device reset相关.
inline void osc_skinMesh::sm_onLostDevice( void )
{
	if(m_bShadowSupport ){
		for( int t_i=0;t_i<this->m_iNumParts;t_i++ ){
			m_vecBpmIns[t_i].bpmins_onLostDevice();
		}
	}
}
inline void osc_skinMesh::sm_onResetDevice( void )
{
	if(m_bShadowSupport){
		for( int t_i=0;t_i<this->m_iNumParts;t_i++ ){
			m_vecBpmIns[t_i].bpmins_onResetDevice();
		}
	}
}






/** \brief
*  设置Mgr内某一个skin Mesh的可见性。
*/
inline void osc_skinMeshMgr::set_inViewChr( int _id )
{
	osassert( m_vecDadded.validate_id( _id ) );
	m_vecDadded[_id].m_bInView = true;
}



/** \brief
*  得到某一个skin Mesh的世界坐标位置。
*/
inline void osc_skinMeshMgr::get_smPos( int _id ,osVec3D& _pos )
{
	osassert( m_vecDadded.validate_id( _id ) );
	m_vecDadded[_id].get_smPos( _pos );
}

//! Restore 当前的skinMeshManager.
inline void osc_skinMeshMgr::smMgr_onLostDevice( void )
{
	//! river mod @ 2009-4-30:正确的处理阴影。
	CSpaceMgr<osc_skinMesh>::NODE t_ptrNode;

	for( t_ptrNode = m_vecDadded.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDadded.next_validNode( &t_ptrNode ) )
	{
		if( t_ptrNode.p->m_bLoadLock )
			continue;
		if( !t_ptrNode.p->m_bInUse )
			continue;

		// 先设备lost.
		t_ptrNode.p->sm_onLostDevice();
	}

	//! sm dev depMgr lost
	m_devdepResMgr.onlost_devdepMesh();

	osc_skinMesh::release_postTrail();

}
inline void osc_skinMeshMgr::smMgr_onResetDevice( void )
{
	//! river mod @ 2009-4-30:正确的处理阴影。
	CSpaceMgr<osc_skinMesh>::NODE t_ptrNode;

	for( t_ptrNode = m_vecDadded.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDadded.next_validNode( &t_ptrNode ) )
	{
		if( t_ptrNode.p->m_bLoadLock )
			continue;
		if( !t_ptrNode.p->m_bInUse )
			continue;

		// 设备restore.
		t_ptrNode.p->sm_onResetDevice();
	}

	m_devdepResMgr.restore_devdepMesh();

	osc_skinMesh::restore_postTrail();

}


/** \brief
*  暂停当前人物的动作。
*
*  \param bool _suspend 如果为true,则暂停动作，如果为false,则继续动作。
*/
inline void osc_skinMesh::suspend_curAction( bool _suspend )
{
	m_bSuspendState = _suspend;
}


# if __GLOSSY_EDITOR__
//@{
// 人物的动作和装备编辑相关。



//! 得到当前人物正在播放动作的帧索引。
inline int osc_skinMesh::get_curActFrameIdx( void )
{
	return m_wAniIndex;
}


//@}
# endif 


