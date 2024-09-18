

//! �Ƿ��Ǿ��ι����
inline bool osc_billBoard::is_hugeBillBoard( void )
{
	if( m_billBoardType != BILLBOARD_XZ )
		return false;

	// ���ٴ���100�Ĺ��������Ǿ��͹����
	if( (m_vecBBSize[0] + m_vecBBSizeAdj[0]) < 100.0f )
		return false;

	return true;
}


/** \brief
*  ���ݹ��������������õ�ǰbillBoard��λ�á�
*  
*/
inline void osc_billBoardIns::set_billBoardPos( osVec3D& _pos )
{

	m_vec3Pos = _pos;
	if( m_ptrBBData->m_billBoardType == BILLBOARD_Y )
	{
		m_vec3Pos.y += (m_ptrBBData->m_vecBBSizeAdj[1]/2);
	}

	m_bsphere.veccen = m_vec3Pos;

	osassert( m_ptrBBData );
	if( m_ptrBBData->m_vecBBSizeAdj[0]>m_ptrBBData->m_vecBBSizeAdj[1] )
		m_bsphere.radius = m_ptrBBData->m_vecBBSizeAdj[0];
	else
		m_bsphere.radius = m_ptrBBData->m_vecBBSizeAdj[1];

}

//! ���ù����Ĳ���ʱ��.
inline void osc_billBoardIns::set_bbPlayTime( float _timeScale )
{
	m_fTimeScale = _timeScale;
}

//! �õ������Ĳ���ʱ��.
inline float osc_billBoardIns::get_bbPlayTime( void )
{

	// 
	// �����ѭ�����ŵĹ�������ݣ��򷵻ظ��Ĳ���ʱ��ֵ
	if( m_ptrBBData->m_bLoopBB )
		return -1.0f;
	else
		return m_ptrBBData->m_fTotalPlayTime;
}



//! ��bbIns�еõ�Ҫ��Ⱦ��shaderId,�����Ż���BB��Ⱦģʽ��
inline int osc_billBoardIns::get_bbInsRShader( void )
{
	osassert( m_ptrBBData );
	// River @ 2009-5-28����֡bb��Ӧһ��ͼƬ
	//osassert( m_iCurFrame < m_ptrBBData->m_iTotalTexNum );
	return m_ptrBBData->m_iShaderId[m_ptrBBData->m_vecTexId[m_iCurFrame]];
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
//



//! ����billBoard��lru.
inline void osc_bbManager::set_bbMgrLru( void )
{
	CSpaceMgr<osc_billBoard>::NODE t_ptrNode;
	for( t_ptrNode = m_vecBillboard.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_vecBillboard.next_validNode( &t_ptrNode ) )
	{
		t_ptrNode.p->add_nu();
	}
}

//! ���ù����Ĳ���ʱ�䡣
inline void osc_bbManager::set_bbPlayTime( int _id,float _timeScale )
{
	WORD   t_wId = (WORD)_id;
	osassert( t_wId >= 0 );
	
	// HUGE bb
	if( t_wId >= HUGEBB_IDSTART )
	{
		t_wId -= HUGEBB_IDSTART;
		osassert( m_vecHugeBBIns.validate_id( t_wId ) );

		m_vecHugeBBIns[t_wId].set_bbPlayTime( _timeScale );
		return;
	}
	
	osassert( m_vecBBIns.validate_id( t_wId ) );
	m_vecBBIns[t_wId].set_bbPlayTime( _timeScale );
	return;
}

//! ���ù�������ת��
inline void osc_bbManager::set_bbRotAxis( int _id,osVec3D& _axis )
{
	WORD   t_wId = (WORD)_id;
	osassert( t_wId >= 0 );
	
	// HUGE bb
	if( t_wId >= HUGEBB_IDSTART )
	{
		t_wId -= HUGEBB_IDSTART;
		osassert( m_vecHugeBBIns.validate_id( t_wId ) );

		m_vecHugeBBIns[t_wId].set_bbRotAxis( _axis );
		return;
	}
	
	osassert( m_vecBBIns.validate_id( t_wId ) );
	m_vecBBIns[t_wId].set_bbRotAxis( _axis );

	return;
}


//! �õ���������������ʱ�䡣
inline float osc_bbManager::get_bbPlayTime( int _id )
{
	WORD   t_wId = (WORD)_id;
	osassert( t_wId >= 0 );

	// HUGE bb
	if( t_wId >= HUGEBB_IDSTART )
	{
		t_wId -= HUGEBB_IDSTART;
		osassert( m_vecHugeBBIns.validate_id( t_wId ) );

		return m_vecHugeBBIns[t_wId].get_bbPlayTime();
	}

	osassert( m_vecBBIns.validate_id( t_wId ) );
	return m_vecBBIns[t_wId].get_bbPlayTime();
}

//! ���ŵ�ǰ�Ĺ����.river added @ 2010-1-7:
inline void osc_bbManager::scale_bb( int _id,float _rate )
{
	WORD   t_wId = (WORD)_id;
	osassert( (t_wId>=0) );
	
	// HUGE bb
	if( t_wId >= HUGEBB_IDSTART )
		return;

	// common bb.
	if( !m_vecBBIns.validate_id( t_wId ) )
		return;
	else
	{
		// �����µ�����ֵ
		if( m_vecBBIns[t_wId].validate_cval( _id ) )
			m_vecBBIns[t_wId].m_fSizeScale = m_vecBBIns[t_wId].m_fBaseSizeScale*_rate;
	}
}


//! ���ػ���ʾ��ǰ��bb.
inline void osc_bbManager::hide_bb( int _id,bool _hide )
{
	WORD   t_wId = (WORD)_id;
	osassert( (t_wId>=0) );
	
	// HUGE bb
	if( t_wId >= HUGEBB_IDSTART )
		return;

	// common bb.
	if( !m_vecBBIns.validate_id( t_wId ) )
		return;
	else
	{
		if( m_vecBBIns[t_wId].validate_cval( _id ) )
			m_vecBBIns[t_wId].m_bHide = _hide;
	}
}



//! ��֤��ǰ��billBoardId�Ƿ���Ч��
inline bool osc_bbManager::validate_bbId( int _id )
{
	WORD   t_wId = (WORD)_id;
	osassert( (t_wId>=0) );
	
	// HUGE bb
	if( t_wId >= HUGEBB_IDSTART )
	{
		t_wId -= HUGEBB_IDSTART;
		if( !m_vecHugeBBIns.validate_id( t_wId ) )
			return false;
		else
		{
			if( m_vecHugeBBIns[t_wId].validate_cval( _id ) )
				return m_vecHugeBBIns[t_wId].is_billBoardInUsed();
			else
				return false;
		}

		return false;
	}


	// common bb.
	if( !m_vecBBIns.validate_id( t_wId ) )
		return false;
	else
	{
		if( m_vecBBIns[t_wId].validate_cval( _id ) )
			return m_vecBBIns[t_wId].is_billBoardInUsed();
		else
			return false;
	}
}
