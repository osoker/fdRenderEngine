//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: tgObjoadThread.cpp
 *
 *  His:      River created @ 2004-9-25
 *
 *  Des:      调入地图上的物品时，使用多线程。
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"


# include "../../backPipe/include/osCamera.h"

# include  "../../mfpipe/include/osMesh.h"

# include "../../backpipe/include/mainEngine.h"



//! TEST CODE:当前是否处于多线程地图调入时刻
OSENGINE_API BOOL     g_bMapLoadding = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  多线程的地图调入相关类.
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! 全局使用的critical_section.
CRITICAL_SECTION osc_mapLoadThread::m_sMapNameCS;


//! 临界创建粒子时用到的资源.
CRITICAL_SECTION osc_mapLoadThread::m_sParticleCS;

//! 临界创建mesh时用到的资源.
CRITICAL_SECTION osc_mapLoadThread::m_sMeshCS;

//! 临界创建osa时用到的资源.
CRITICAL_SECTION osc_mapLoadThread::m_sOsaCS;

//! 临界创建特效用到的资源
CRITICAL_SECTION osc_mapLoadThread::m_sEffectCS;

//! 场景内活动地图列表更新时，用到的临界区资源，上层可能用到相关的信息
CRITICAL_SECTION osc_mapLoadThread::m_sActiveMapCS;


//! 临界创建billboard用到的资源.
CRITICAL_SECTION osc_mapLoadThread::m_sBbCS;


//! 临界shader创建用到的资源.
CRITICAL_SECTION osc_mapLoadThread::m_sShaderCS;

//! 创建Effect需要的CS.
CRITICAL_SECTION osc_mapLoadThread::m_sDxEffectCS;


//! 临界shader创建用到的资源.
CRITICAL_SECTION osc_mapLoadThread::m_sShaderReleaseCS;


//! 是否唤醒地图调入线程的event.
HANDLE osc_mapLoadThread::m_sEventStartMt = NULL;

//! 地图调入线程是否处于空闲状态，在等待线程被唤醒的消息
HANDLE osc_mapLoadThread::m_sEventThreadFree = NULL;


osc_mapLoadThread::osc_mapLoadThread()
{
	::InitializeCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	//! 临界创建粒子时用到的资源.
	::InitializeCriticalSection( &osc_mapLoadThread::m_sParticleCS );

	//! 临界创建mesh时用到的资源.
	::InitializeCriticalSection( &osc_mapLoadThread::m_sMeshCS );

	//! 临界创建osa时用到的资源.
	::InitializeCriticalSection( &osc_mapLoadThread::m_sOsaCS );

	//! 临界创建billboard用到的资源.
	::InitializeCriticalSection( &osc_mapLoadThread::m_sBbCS );

	//! 临界创建billboard用到的资源.
	::InitializeCriticalSection( &osc_mapLoadThread::m_sShaderCS );

	::InitializeCriticalSection( &osc_mapLoadThread::m_sDxEffectCS );

	::InitializeCriticalSection( &osc_mapLoadThread::m_sShaderReleaseCS );

	::InitializeCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	//! 场景内活动地图列表更新时，用到的临界区资源，上层可能用到相关的信息
	::InitializeCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );


	m_ptrLRMgr = NULL;
	m_ptrTGMgr = NULL;

	m_sEventStartMt = ::CreateEvent( 
		NULL,
		TRUE,
		FALSE,  // 初始化为无信号.
		NULL );

	osassert( m_sEventStartMt );

	m_sEventMapLoadFinish = ::CreateEvent(
		NULL,
		TRUE,
		FALSE, // 初始化为无信号.
		NULL );
	osassert( m_sEventMapLoadFinish );
	::ResetEvent( m_sEventMapLoadFinish );


	// 线程退出
	m_sEventExitThread = ::CreateEvent(
		NULL,
		TRUE,
		FALSE,// 初始化为无信号
		NULL );
	osassert( m_sEventExitThread );

	//! 地图调入线程是否处于空闲状态，在等待线程被唤醒的消息
	m_sEventThreadFree = ::CreateEvent(
		NULL,
		TRUE,
		FALSE,// 初始化为无信号
		NULL );
	osassert( m_sEventThreadFree );

}

osc_mapLoadThread::~osc_mapLoadThread()
{

	::DeleteCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	//! 临界创建粒子时用到的资源.
	::DeleteCriticalSection( &osc_mapLoadThread::m_sParticleCS );

	//! 临界创建mesh时用到的资源.
	::DeleteCriticalSection( &osc_mapLoadThread::m_sMeshCS );

	//! 临界创建osa时用到的资源.
	::DeleteCriticalSection( &osc_mapLoadThread::m_sOsaCS );

	//! 临界创建billboard用到的资源.
	::DeleteCriticalSection( &osc_mapLoadThread::m_sBbCS );

	//! 临界创建billboard用到的资源.
	::DeleteCriticalSection( &osc_mapLoadThread::m_sShaderCS );
	::DeleteCriticalSection( &osc_mapLoadThread::m_sDxEffectCS );

	::DeleteCriticalSection( &osc_mapLoadThread::m_sShaderReleaseCS );

	::DeleteCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	::DeleteCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );


	//! 
	if( m_sEventStartMt )
		CloseHandle( m_sEventStartMt );
	if( m_sEventMapLoadFinish )
		::CloseHandle( m_sEventMapLoadFinish );

	if( m_sEventThreadFree )
		::CloseHandle( m_sEventThreadFree );

	//syq
	if( m_sEventExitThread )
		::CloseHandle( m_sEventExitThread );

}

//! 传入内部使用的指针
void osc_mapLoadThread::init_mtLoadMapMgr( 
	osc_TGManager* _mgr,osc_tgLoadReleaseMgr* _lrMgr )
{
	guard;

	osassert( _mgr );
	osassert( _lrMgr );

	m_ptrTGMgr = _mgr;
	this->m_ptrLRMgr = _lrMgr;



	unguard;
}


/** \brief
*  是否是可操作的TG,即可使用的tg.
*
*  在地图调入线程的置换地图队列中没有当前tg.这个tg就是activeTg,
*  为上层渲染需要使用到的tg.
*/
bool osc_mapLoadThread::is_activeTg( int _idx )
{
	guard;

	bool   t_bRes = true;
	std::list< os_tgMTLoader >::iterator     t_iter;

	osassert( _idx < m_ptrLRMgr->m_iCurTgNum );

	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	// 
	// 如果当前tg在使用状态和非“排队调入”状态，则查看置换队列中如果没有，则返回为active.
	// 如果tg不在使用状态，则直接返回非active.
	if( m_ptrLRMgr->m_ptrRealTg[_idx].is_tgInuse() &&
		(!m_ptrLRMgr->m_ptrRealTg[_idx].get_mtQueueState()) )
	{
		// 在线程队列中存在此tg,为不可操作tg.
		for( t_iter = m_sMapNeedLoad.begin();
			t_iter != m_sMapNeedLoad.end();t_iter ++ )
		{
			if( t_iter->m_iMapIdx == _idx )
			{
				t_bRes = false;
				break;
			}
		}

		// 要调入的地图不在正在调入的地图列表中.
		if( t_iter == m_sMapNeedLoad.end() )
			t_bRes = true;
	}
	else
		t_bRes = false;

	::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	return t_bRes;

	unguard;
}


//! 结束线程的执行
void osc_mapLoadThread::end_thread( void )
{
	guard;

	// 使用地图调入处于等待状态.
	reset_loadMapMgr();

	// 
	// 先激活地图调入线程，再激活退出线程事件，如果地图调入
	// 队列为空，而此时退出线程事件有信号，则退出线程。
	::SetEvent( osc_mapLoadThread::m_sEventStartMt );
	::SetEvent( m_sEventExitThread );

	unguard;
}



//! 当前是否处于闲置状态,即等待主线程推进要调入的地图.
bool osc_mapLoadThread::is_freeStatus( void )
{
	guard;

	DWORD    t_dwWait;
	bool     t_bRes;

	// FreeStatus状态除了不处于调入状态外，必须没有需要调入的地图。
	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	if( 0 == m_sMapNeedLoad.size() )
	{
		::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );
		
		t_dwWait = ::WaitForSingleObject( m_sEventStartMt,0 );
		if( t_dwWait == WAIT_OBJECT_0 )
		{
			t_bRes = false;
		}
		else if( t_dwWait == WAIT_TIMEOUT )
			t_bRes = true;
		else
			osassertex( false,"不可预知的返回结果...\n" );
	}
	else
	{
		::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );
		t_bRes = false;
	}


	return t_bRes;

	unguard;
}




/** \brief
*  给管理器指定一个要调入的地图。
*
*  \param _mapName 要调地图的名字.
*  \param _mapIdx 调入地图时,使用的地图索引.ATTENTION TO FIX:
*  \param _waitForFinish 地图调入完成后,是否通知另外的线程地图调入完成,
*                  此机制用于完成阻塞式的地图调入,比如人物第一次Insert进场景
*                  的时候。
*/
void osc_mapLoadThread::push_loadMap( 
	char* _mapName,int _mapIdx,BOOL _waitForFinish/* = FALSE*/ )
{
	guard;

	osassert( _mapName&&(_mapName[0] ) );

	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	os_tgMTLoader    t_ptrLoad;

	//! 如果当前的list中没有这个地图,则推进,否则返回.
	std::list< os_tgMTLoader >::iterator     t_iter;

	for( t_iter = m_sMapNeedLoad.begin();t_iter != m_sMapNeedLoad.end();t_iter ++ )
	{
		if( t_iter->m_szMapName == _mapName )
		{
			::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );
			return ;
		}

		// 
		// 如果名字不同,确认要调入的地图数据区不正在使用
		osassertex( (t_iter->m_iMapIdx != _mapIdx),
			va( "The loadMap Idx<%s>in use,Need Load map<%s>",
			t_iter->m_szMapName.c_str(),_mapName ) );
	}

	
	t_ptrLoad.m_ptrTG = &m_ptrLRMgr->m_ptrRealTg[_mapIdx];
	t_ptrLoad.m_ptrTG->set_mtQueueState( TRUE );

	t_ptrLoad.m_szMapName = _mapName;
	t_ptrLoad.m_iMapIdx = _mapIdx;
	t_ptrLoad.m_bFinishNotify = _waitForFinish;

	m_sMapNeedLoad.push_back( t_ptrLoad );

	osDebugOut( "释放<%d>地图<%s>,调入地图<%s>..,队列中地图数目:<%d>...\n",_mapIdx,
		t_ptrLoad.m_ptrTG->get_mapFName(),_mapName,m_sMapNeedLoad.size() );

	::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	// 
	// 设置事件,如果另外的线程处于等待状态,则唤醒执行.
	::SetEvent( osc_mapLoadThread::m_sEventStartMt );


	//
	// 如果是阻塞式的调入,等待地图调入的事件发生,并重置事件.
	if( _waitForFinish )
	{
		::WaitForSingleObject( m_sEventMapLoadFinish,INFINITE );
		::ResetEvent( m_sEventMapLoadFinish );
	}

	return ;

	unguard;
}

//! 从队列中弹出地图列表.
void osc_mapLoadThread::pop_loadMap()
{
	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	m_sMapNeedLoad.pop_front();

	::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

}

//! 清空要调入地图的队列.
void osc_mapLoadThread::clear_loadMap( void )
{
	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );


	while( !m_sMapNeedLoad.empty() )
	{
		// 确认清空之前，把排队状态清空
		m_sMapNeedLoad.front().m_ptrTG->set_mtQueueState( false );
		m_sMapNeedLoad.pop_front();

	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );
}


//! 得到最前端的地图.
BOOL osc_mapLoadThread::get_frontMap( os_tgMTLoader& _tgLoad )
{
	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	if( 0 == m_sMapNeedLoad.size() )
	{
		::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );
	
		// 查看是否是退出线程的信号
		DWORD    t_dwWait;
		t_dwWait = ::WaitForSingleObject( this->m_sEventExitThread,0 );
		if( WAIT_TIMEOUT == t_dwWait )
		{
			return FALSE;
		}
		else if( WAIT_OBJECT_0 == t_dwWait )
		{
			_tgLoad.m_ptrTG = NULL;
			return TRUE;
		}
	}

	osassert( m_sMapNeedLoad.size() > 0 );
	_tgLoad.m_ptrTG = m_sMapNeedLoad.front().m_ptrTG;
	_tgLoad.m_szMapName = m_sMapNeedLoad.front().m_szMapName;
	_tgLoad.m_bFinishNotify = m_sMapNeedLoad.front().m_bFinishNotify;
	_tgLoad.m_iMapIdx = m_sMapNeedLoad.front().m_iMapIdx;

	// 此函数为调入线程所用，得到数据后在主线程中删除此map name.
	m_sMapNeedLoad.pop_front();

	::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	return TRUE;
}



//! 清空管理器要调入的地图,在释放整个地图时，可能有另外的线程正在调入地图，处理。
void osc_mapLoadThread::reset_loadMapMgr( void )
{
	guard;

	// 如果当前线程处于等待状态,直接返回.
	if( !is_freeStatus() )
	{
		clear_loadMap();

		// 提高线程的优先级别.
		setPriority( THREAD_PRIORITY_ABOVE_NORMAL );

		DWORD   t_dwRes;

		// 此处不应该等待这个事件，此事件表示调入线程开始工作，而非完成工作
		//                使用一个新的event来确保这个状态。
		t_dwRes = ::WaitForSingleObject( m_sEventThreadFree,INFINITE );
		osassert( t_dwRes != WAIT_FAILED );

		// 可能会出现下面的情形：
		// 主线程设置了事件m_sEventStartMt,而地图调入线程已经执行到了
		// ::ResetEvent( osc_mapLoadThread::m_sEventStartMt );语句之外，
		// 在这种情况下，就会出现m_sEventStartMt一直处于信号状态下的情形，
		// 此处需要手工修改m_sEventStartMt为无信号状态
		::ResetEvent( m_sEventStartMt );

		// 确认此处的地图调入线程处于非调入状态
		osassertex( is_freeStatus(),
			va( "非free状态出错,地图数<%d>....\n",this->m_sMapNeedLoad.size() ) );

		// 重设地图调入线程的优化级别.
		setPriority( THREAD_PRIORITY_BELOW_NORMAL );

	}

	// 确认此处的地图调入线程处于非调入状态
	osassertex( is_freeStatus(),"free状态出错...\n" );

	return;


	unguard;
}


//! 得到要调入的地图的名字.
void osc_mapLoadThread::get_loadMapName( int _idx,char* _name,int _maxname )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	osassert( (int)m_sMapNeedLoad.size() > _idx );
	int t_i = 0;
	std::list< os_tgMTLoader >::iterator     t_iter;

	for( t_iter = m_sMapNeedLoad.begin();t_iter != m_sMapNeedLoad.end();t_iter ++ )
	{
		if( t_i == _idx )
		{
			osassert( (int)strlen( t_iter->m_szMapName.c_str() ) < _maxname );
			strcpy( _name,t_iter->m_szMapName.c_str() );
			break;
		}

		t_i ++;
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );
	
	return ;

	unguard;
}




/**
*  每一个派生类都必须重载的函数.
*
*  函数处理流程: 应该有一个事件
*  
*/
DWORD osc_mapLoadThread::ThreadProc()
{
	guard;

	while( 1 )
	{

		// 如果没有地图,则等待地图调入的事件.如果事件发生,重复调入地图,直到完成.
		if( !get_frontMap( m_sCurLoadMap ) )
		{
			// 设置等待主线程推进地图到此线程.	
			::ResetEvent( osc_mapLoadThread::m_sEventStartMt );

			// 
			// 如果另外的线程在等待地图调入完成的消息,通知事件.
			// 只有在当前的线程设置完等待信息后，才能发出地图调入完成的消息。
			if( m_sCurLoadMap.m_bFinishNotify )
				::SetEvent( m_sEventMapLoadFinish );


			osDebugOut( "等待地图推进后唤醒线程,如果此时程序停顿，则线程死锁!!!!\n" );

			// 如果在线程的地图调入队列内，得不到需要调入的地图，则继续等待。
			do
			{
				
				// 等待线程唤醒之前设为free事件
				::SetEvent( m_sEventThreadFree );

				::WaitForSingleObject( osc_mapLoadThread::m_sEventStartMt,INFINITE );
				
				// 唤配后设free事件为无信号
				::ResetEvent( m_sEventThreadFree );
			}
			while( (!get_frontMap( m_sCurLoadMap )) );
			
			osDebugOut( "等待地图推进后唤醒线程...done\n" );
		}

		// 
		// 退出线程的开关
		if( NULL == m_sCurLoadMap.m_ptrTG ){
			osDebugOut( "退出线程的开关...break\n" );
			break;
		}

		osDebugOut( "先释放地图\n" );

		//! 先释放地图
		m_sCurLoadMap.m_ptrTG->release_TG();

		osDebugOut("start threadSafe_loadNMap...\n");
		//
		//! 调入地图的函数
		if( !m_ptrLRMgr->threadSafe_loadNMap( 
			(char*)m_sCurLoadMap.m_szMapName.c_str(),m_sCurLoadMap.m_ptrTG ) )
			return FALSE;

	}


	osDebugOut( "Exit mapLoad Thread...\n" );

	return TRUE;

	unguard;
}












	



