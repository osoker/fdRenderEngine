

//! 是否是巨形公告板
inline bool osc_billBoard::is_hugeBillBoard( void )
{
	if( m_billBoardType != BILLBOARD_XZ )
		return false;

	// 至少大于100的公告板才算是巨型公告板
	if( (m_vecBBSize[0] + m_vecBBSizeAdj[0]) < 100.0f )
		return false;

	return true;
}


/** \brief
*  根据公告板的类型来设置当前billBoard的位置。
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

//! 设置公告板的播放时间.
inline void osc_billBoardIns::set_bbPlayTime( float _timeScale )
{
	m_fTimeScale = _timeScale;
}

//! 得到公告板的播放时间.
inline float osc_billBoardIns::get_bbPlayTime( void )
{

	// 
	// 如果是循环播放的公告板数据，则返回负的播放时间值
	if( m_ptrBBData->m_bLoopBB )
		return -1.0f;
	else
		return m_ptrBBData->m_fTotalPlayTime;
}



//! 从bbIns中得到要渲染的shaderId,用于优化的BB渲染模式。
inline int osc_billBoardIns::get_bbInsRShader( void )
{
	osassert( m_ptrBBData );
	// River @ 2009-5-28：多帧bb对应一张图片
	//osassert( m_iCurFrame < m_ptrBBData->m_iTotalTexNum );
	return m_ptrBBData->m_iShaderId[m_ptrBBData->m_vecTexId[m_iCurFrame]];
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
//



//! 设置billBoard的lru.
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

//! 设置公告板的播放时间。
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

//! 设置公告板的旋转轴
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


//! 得到公告板的正常播放时间。
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

//! 缩放当前的公告板.river added @ 2010-1-7:
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
		// 处理新的缩放值
		if( m_vecBBIns[t_wId].validate_cval( _id ) )
			m_vecBBIns[t_wId].m_fSizeScale = m_vecBBIns[t_wId].m_fBaseSizeScale*_rate;
	}
}


//! 隐藏或显示当前的bb.
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



//! 验证当前的billBoardId是否有效。
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
