

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ����������������������ʼ��ǰ�ļ����õ���Inline����
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


/** \brief
*  ���õ�ǰ�����Ĳ����ٶ�.
*/
inline void os_boneAction::set_actPlaySpeed( float _speed )
{
	m_iPlaySpeed = int(m_iNumFrame * _speed);
	m_fTimeToPlay = 1.0f/_speed; 
}

/** \brief
*  ���õ�ǰ���������֡�
*/
inline void os_boneAction::set_actName( const char *_name )
{
	strcpy( m_szActName,_name );
}


//! ʹ��id�õ�������Ҫ���豸������ݡ�
inline os_smMeshDevdep* osc_smDepdevResMgr::get_smMesh( int _id )
{
	return &m_vecDevdepRes[_id];
}


//! ����bpmInstance������.
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

//! river added @ 2010-3-11:�����������ͼ.
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

//! ��ǰBodyPartMeshʵ���Ƿ�ʹ�õ��⡣
inline bool osc_bpmInstance::is_glossySM( void ) 
{ 
	if( m_ptrSGIns ) 
		return true;
	else 
		return false; 
} 

//! ������������Ч����
inline void osc_bpmInstance::scale_weaponEffect( float _rate )
{
	if( m_ptrSGIns )
		m_ptrSGIns->scale_weaponEffect( _rate );
}



///////////////////////////////////////////////////////////////////////////////////////////////////////
// �����õ���inline ������
//! ��ʼ��Ⱦ���⡣
inline DWORD osc_swordGlossyIns::start_swordGlossy( int _idx )
{
	m_iPRSword = _idx;


	osassert( m_ptrSwordGlossy );

	int   t_iActIdx = -1;

	// ֻ��Ҫ���õĶ���������,��������.
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

//! ���õ���ʵ��ָ��ĵ������ݡ�
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



//! ���赱ǰ�ĵ������ݡ�ÿһ֡��ʼʱ���������������
inline void osc_swordGlossyIns::reset_swordGlossy( void )
{
	m_iGlossyPos = 0;
	m_bPosOverFlow = false;
}
//! �õ���������ĵ㡣
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
*  �Ƿ���Ҫ�ƽ��������λ��.
*  
*  ������⸽�ӵ�����Ч,����ʱ�ֲ���ʾ����Ļ�,����Ҫ�ƽ��������λ��.
*  ���û����Ч,��û�е���,����ȫ����Ҫ���ƽ���������λ��.
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










//!  ��һ��id�еõ�Body part Mesh��ָ�롣
inline osc_bodyPartMesh*  osc_bpmMgr::get_bpmPtrFromId( int _id )
{
	return &m_vecBPM[_id];
}


//! ��ǰ��BPM�Ƿ���ʹ���С�
inline bool osc_bodyPartMesh::is_inUsed(  void )        
{ 
	return m_bInuse;
}
//! ��ǰ��bpm�Ƿ����������֡�
inline bool osc_bodyPartMesh::is_weaponPart( void )
{
	osassert( m_strFname[0] );
	char   t_ch = m_strFname[ strlen(m_strFname)- WEAPON_PARTOFFSET ];

	if( t_ch == WEAPON_PARTCHAR||t_ch == WEAPON_PART8CHAR )
		return true;
	else
		return false;
}




//! �õ������õڶ�����ͼ��id.
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
//! River @ 2010-3-11:װ���ڵ����������ID.
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


//! �����Ƿ���ʾ���⡣
inline DWORD osc_bpmInstance::set_disSGlossy( int _idx )
{
	if( m_ptrSGIns )
		return m_ptrSGIns->start_swordGlossy( _idx );
	else
		return 0;

}

// 
//! ��ǰ���������Ƿ��Ƕ�Ӧ����Ķ�����
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


//! �õ���ǰ��������ĵ㡣
inline void osc_bpmInstance::get_glossyCenter( osVec3D& _center )
{
	m_ptrSGIns->get_glossyCenter( _center );
}


//!  Device reset���.
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





//! ���ڵ�skin���������ڵ�ʱ�򣬲�����������ʾ���һ֡�Ķ���
inline void osc_skinMesh::play_lastFrameStopAction( void )
{

	if( m_actChange.m_changeType != OSE_DIS_NEWACT_LASTFRAME )
		return;

	frame_moveAction( sg_timer::Instance()->get_lastelatime() );

}




//! �õ���ǰskinMesh����ת����
inline float osc_skinMesh::get_smAgl( void )
{
	return m_fRotAgl;
}

//! �ϲ������Чʱ�õ�����ת�����������Ⱦ�õ�����������
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
*  ���ص�ǰskinMesh����ʾ��
*
*/
inline void osc_skinMesh::set_hideSkinMesh( bool _hide )
{
	if( m_bHideSm == _hide )
		return;

	//! River @ 2010-10-19:�����ʱ������������״̬������
	if( _hide )
		hide_weapon( false );

	m_bHideSm = _hide;

	// �����Ҫ����,����Ҫɾ��������Ч,�����Ҫ��ʾ,����Ҫ����������Ч
	if( m_bHideSm )
	{
		m_bPlaySGlossyBack = m_bPlaySGlossy;
		m_bPlayWeaponEffectBack = m_bPlayWeaponEffect;
		// River added @ 2010-8-18:��С���뿪��.
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
		// River added @ 2010-8-18:��С���뿪��.
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
*  ���ö������ŵ�ʱ����ٶ������Ϣ��
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

//! ��������ʱ,�õ���ʼ��������id.
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
			sprintf( buffer, "��ɫ:%s\n�Ҳ���Ĭ�ϵĶ���:%s\n", _smi->m_strSMDir, _smi->m_szDefaultAct );
			MessageBox( NULL,buffer,"����",MB_OK );
			m_iAniId = 0;
		}
	}

	return;

}


//! River added @ 2009-8-26: ��������ļ�ʱ��Ӱ.
inline bool osc_skinMesh::hide_realShadow( bool _hide/* = true*/ )
{
	m_bHideRealShadow = _hide;
	return true;

}
/** \brief
*  �õ���ǰskinMesh������
*/
inline int osc_skinMesh::get_smFaceNum( void )
{
	int t_num = 0;
	for( int t_i=0;t_i<this->m_iNumParts;t_i++ )
		t_num += this->m_vecBpmIns[t_i].get_faceNum();

	return t_num;
}

//! �õ��Ӷ�����ʼ������ֹͣ֡���õ�ʱ�䡣
inline float osc_skinMesh::get_timeFromStartToSF( void )
{
	// ����Ƿ��ڶ���ֹͣ״̬��
	os_boneAction*  t_ptrBA;
	t_ptrBA = &m_ptrBoneAni->m_vecBATable[m_iAniId];

	return m_fTPF*t_ptrBA->m_iStopFrameIdx;
}


/** \brief
*  �������������в��������ֵ������ľ���
*/
inline void osc_skinMesh::lerp_actionPose( osMatrix& _mat,
								   os_bactPose* _pa,os_bactPose* _pb,float _lf )
{
	osQuat  t_quat;
	osVec3D t_vec3;

	// River @ 2009-6-17:����Ժ�˴�������ֱ�����������ۺϣ�ʹ���µĶ�������
	if( _pa->m_bFlipDeterminant == _pb->m_bFlipDeterminant )
		osQuaternionSlerp( &t_quat,&_pa->m_actQuat,&_pb->m_actQuat,_lf );
	else
		t_quat = _pb->m_actQuat;

	osMatrixRotationQuaternion( &_mat,&t_quat );

	// ATTENTION TO OPP: �˴������Ż�,������quaternionԤ��������ɵ�quaternion.
	if( _pa->m_bFlipDeterminant )
		osMatrixMultiply( &_mat,&_mat,&osc_skinMesh::m_sFDMatrix );	

	osVec3Lerp( (osVec3D*)&_mat._41,&_pa->m_vec3Offset,&_pb->m_vec3Offset,_lf );

	return;
	
}

//  River Added @ 2007-4-20:�����µĶ�����ֵ��ʽ��
//! ������������ֵ������һ���µ�pose,ʹ�����poseȥ���µĶ����ٲ�ֵ��
inline void osc_skinMesh::lerp_actionPose( os_bactPose* _res,
								  os_bactPose* _pa,os_bactPose* _pb,float _lf )
{
	osQuat  t_quat;
	osVec3D t_vec3;

	// River @ 2009-6-17:����Ժ�˴�������ֱ�����������ۺϣ�ʹ���µĶ�������
	if( _pa->m_bFlipDeterminant == _pb->m_bFlipDeterminant )
		osQuaternionSlerp( &_res->m_actQuat,&_pa->m_actQuat,&_pb->m_actQuat,_lf );
	else
		_res->m_actQuat = _pb->m_actQuat;
	

	osVec3Lerp( (osVec3D*)&_res->m_vec3Offset,&_pa->m_vec3Offset,&_pb->m_vec3Offset,_lf );
	_res->m_bFlipDeterminant = _pa->m_bFlipDeterminant;

	return;
}


//! �ӵ�ǰ�Ķ����õ�����
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
*  ���ﶯ���Ƿ񵽴ﵱǰ�����Ĺؼ�֡
*
*  ����ؼ�֮֡ǰ����ֵΪ�٣�����֮�󷵻�ֵΪ�档
*  
*/
inline bool osc_skinMesh::arrive_stopFrameOrNot( void )
{
	return m_bArriveStopFrame;
}

//! ʹֹͣ״̬�Ķ����������š�
inline void osc_skinMesh::start_moveAction( void )
{
	m_bActStopStatus = false;
}

/** \brief
*  �ӵ�ǰ��character�õ�һ��boundingBox.
*/
inline os_bbox* osc_skinMesh::get_smBBox( void )
{
	return &m_sBBox;
}

/** \brief
*  �ӵ�ǰ��character�õ�һ��boundingBox.
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
*  �л������������Ч
*
*  /param int _idx           �л�����_idx����Ч�����������Ļ�����ʹ�������Ч��
*  /param WEAPON_ID _weapid  ʹ�õ��߻��ǵڰ˸�������λ��
*/
inline void osc_skinMesh::change_weaponEffect( int _idx,WEAPON_ID _weapid/* = WEAPON7*/ )
{
	if( m_vecBpmIns[_weapid].m_ptrSGIns )
		m_vecBpmIns[_weapid].m_ptrSGIns->change_weaponEffect( _idx );
}


//! �õ���ǰ֡��������ĵ㡣
inline void osc_skinMesh::get_swordGlossyCenter( osVec3D& _pos )
{
	m_ptrSGBpmIns->get_glossyCenter( _pos );
}


//! ����һ��������ֵ��ϵͳ�͵�ǰ�ĳ���������ֵ���Ƚϣ�ʹ�ø����Ĺ�������ǰ��ɫ��
inline void osc_skinMesh::set_maxBright( osColor& _bcolor )
{
	m_sCurSkinMeshAmbi = _bcolor;
	m_sCurSkinMeshAmbi.a = 1.0f;
	m_bSetHighLight = true;
}

//! Device reset���.
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
*  ����Mgr��ĳһ��skin Mesh�Ŀɼ��ԡ�
*/
inline void osc_skinMeshMgr::set_inViewChr( int _id )
{
	osassert( m_vecDadded.validate_id( _id ) );
	m_vecDadded[_id].m_bInView = true;
}



/** \brief
*  �õ�ĳһ��skin Mesh����������λ�á�
*/
inline void osc_skinMeshMgr::get_smPos( int _id ,osVec3D& _pos )
{
	osassert( m_vecDadded.validate_id( _id ) );
	m_vecDadded[_id].get_smPos( _pos );
}

//! Restore ��ǰ��skinMeshManager.
inline void osc_skinMeshMgr::smMgr_onLostDevice( void )
{
	//! river mod @ 2009-4-30:��ȷ�Ĵ�����Ӱ��
	CSpaceMgr<osc_skinMesh>::NODE t_ptrNode;

	for( t_ptrNode = m_vecDadded.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDadded.next_validNode( &t_ptrNode ) )
	{
		if( t_ptrNode.p->m_bLoadLock )
			continue;
		if( !t_ptrNode.p->m_bInUse )
			continue;

		// ���豸lost.
		t_ptrNode.p->sm_onLostDevice();
	}

	//! sm dev depMgr lost
	m_devdepResMgr.onlost_devdepMesh();

	osc_skinMesh::release_postTrail();

}
inline void osc_skinMeshMgr::smMgr_onResetDevice( void )
{
	//! river mod @ 2009-4-30:��ȷ�Ĵ�����Ӱ��
	CSpaceMgr<osc_skinMesh>::NODE t_ptrNode;

	for( t_ptrNode = m_vecDadded.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecDadded.next_validNode( &t_ptrNode ) )
	{
		if( t_ptrNode.p->m_bLoadLock )
			continue;
		if( !t_ptrNode.p->m_bInUse )
			continue;

		// �豸restore.
		t_ptrNode.p->sm_onResetDevice();
	}

	m_devdepResMgr.restore_devdepMesh();

	osc_skinMesh::restore_postTrail();

}


/** \brief
*  ��ͣ��ǰ����Ķ�����
*
*  \param bool _suspend ���Ϊtrue,����ͣ���������Ϊfalse,�����������
*/
inline void osc_skinMesh::suspend_curAction( bool _suspend )
{
	m_bSuspendState = _suspend;
}


# if __GLOSSY_EDITOR__
//@{
// ����Ķ�����װ���༭��ء�



//! �õ���ǰ�������ڲ��Ŷ�����֡������
inline int osc_skinMesh::get_curActFrameIdx( void )
{
	return m_wAniIndex;
}


//@}
# endif 


