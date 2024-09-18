///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: os_characterMtLoader.cpp
 * 
 *  His:      River created @ 2009-6-6
 *
 *  Desc:     人物的多线程调入器,为了游戏内新出现人物的时候不卡
 * 
 *  "物有本末，事有始终，知所先后，则近道矣" 
 *
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../include/middlePipe.h"
# include "../../Terrain/include/fdFieldMgr.h"
# include "../../backpipe/include/mainEngine.h"

/*
//! 多线程调入的几个重要事件：
1: 初始化cs和event
2: 主线程推入要调入的数据,并唤醒调入线程。
3: 调入线程一直待命，直到新的数据推入，或是循环调入推入的数据
4: 由主线程发出退出指令
5: 
*/

//! 在推进和读出要调入的人物数据时,使用的临界区.
CRITICAL_SECTION osc_charLoadThread::m_sCharNameCS;

//! River @ 2011-2-25:同步要删除的人物指针临界区.
CRITICAL_SECTION osc_charLoadThread::m_sCharReleaseCS;

//! 人物调入线程是否处于空闲状态，在等待线程被唤醒的消息
HANDLE osc_charLoadThread::m_sEventLCThreadFree = NULL;

//! 是否唤醒人物调入线程的event.
HANDLE osc_charLoadThread::m_sEventLCStartMt = NULL;

//! 退出当前线程
HANDLE osc_charLoadThread::m_sEventLCExitThread = NULL;

//! 当前的人物是否调入完成.
HANDLE osc_charLoadThread::m_sEventCLLoadFinish = NULL;


osc_charLoadThread::osc_charLoadThread()
{
	//!  char name相关数据
	::InitializeCriticalSection( &osc_charLoadThread::m_sCharNameCS );

	//! char release.
	::InitializeCriticalSection( &osc_charLoadThread::m_sCharReleaseCS );

	m_sEventLCStartMt = ::CreateEvent( 
		NULL,
		TRUE,
		FALSE,  // 初始化为无信号.
		NULL );
	osassert( m_sEventLCStartMt );


	//! 人物调入线程是否处于空闲状态，在等待线程被唤醒的消息
	m_sEventLCThreadFree = ::CreateEvent(
		NULL,
		TRUE,
		FALSE,// 初始化为无信号
		NULL );
	osassert( m_sEventLCThreadFree );

	// 线程退出
	m_sEventLCExitThread = ::CreateEvent(
		NULL,
		TRUE,
		FALSE,// 初始化为无信号
		NULL );
	osassert( m_sEventLCExitThread );

	m_sEventCLLoadFinish = ::CreateEvent(
		NULL,
		TRUE,
		FALSE, // 初始化为无信号.
		NULL );
	osassert( m_sEventCLLoadFinish );
	::ResetEvent( m_sEventCLLoadFinish );

}

osc_charLoadThread::~osc_charLoadThread()
{
	//! char name相关数据
	::DeleteCriticalSection( &osc_charLoadThread::m_sCharNameCS );
	::DeleteCriticalSection( &osc_charLoadThread::m_sCharReleaseCS );

	if( m_sEventLCStartMt )
		::CloseHandle( m_sEventLCStartMt );
	if( m_sEventLCThreadFree )
		::CloseHandle( m_sEventLCThreadFree );
}

//  
//! 传入相应的指针.
void osc_charLoadThread::init_mtLoadCharMgr( void )
{
	guard;




	unguard;
}


//! 每一帧对多线程地图调入管理器进行处理。
void osc_charLoadThread::frame_moveCLMgr( void )
{

}

/** \brief
*  给管理器指定一个要调入的地图。
*
*  \param _osc_charLoadData* _ldata 
*  \param _waitForFinish 是否等待另外的线程完成地图的调入,
*                  此机制用于完成阻塞式的地图调入.
*/
void osc_charLoadThread::push_loadChar( os_charLoadData* _ldata,
									  BOOL _waitForFinish/* = FALSE*/ )
{
	guard;

	//  先进入临界区
	CCriticalSec t_charName(&osc_charLoadThread::m_sCharNameCS);

	if( _waitForFinish )
		memcpy( &m_sVipChar,_ldata,sizeof(os_charLoadData ) );
	else
	{
		m_listCharLoadData.push_back( *_ldata );
	}


	//! 唤起人物调入线程
	::SetEvent( m_sEventLCStartMt );
	t_charName.LeaveCriticalSec();

	// 如果等待最终调入.
	if( _waitForFinish )
	{
		::WaitForSingleObject( m_sEventCLLoadFinish,INFINITE );
		::ResetEvent( m_sEventCLLoadFinish );
	}

	unguard;
}

//! 清空管理器要调入的人物,在释放整个地图时，可能有另外的线程正在调入人物，处理。
void osc_charLoadThread::reset_loadCharMgr( void )
{
	guard;

	// 如果当前线程处于等待状态,直接返回.
	if( !is_freeStatus() )
	{
		m_iDataPushPtr = 0;
		m_iProcessPtr = 0;

		// 提高线程的优先级别.
		setPriority( THREAD_PRIORITY_ABOVE_NORMAL );

		DWORD   t_dwRes;
		// 此处不应该等待这个事件，此事件表示调入线程开始工作，而非完成工作
		//                使用一个新的event来确保这个状态。
		t_dwRes = ::WaitForSingleObject( m_sEventLCThreadFree,INFINITE );
		osassert( t_dwRes != WAIT_FAILED );

		::ResetEvent( m_sEventLCStartMt );

		// 确认此处的地图调入线程处于非调入状态
		osassertex( is_freeStatus(),"退出人物调入线程时，非空闲的状态....\n" );

		// 重设地图调入线程的优化级别.
		setPriority( THREAD_PRIORITY_BELOW_NORMAL );
	}

	// 确认此处的地图调入线程处于非调入状态
	osassertex( is_freeStatus(),"free状态出错...\n" );

	return;

	unguard;
}


//! 当前是否处于闲置状态,即等待主线程推进要调入的人物
bool osc_charLoadThread::is_freeStatus( void )
{
	guard;

	DWORD    t_dwWait;
	bool     t_bRes;

	// FreeStatus状态除了不处于调入状态外，必须没有需要调入的地图。
	::EnterCriticalSection( &osc_charLoadThread::m_sCharNameCS );

	if( m_iProcessPtr>=m_iDataPushPtr )
	{
		::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );
		
		t_dwWait = ::WaitForSingleObject( m_sEventLCStartMt,0 );
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
		::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );
		t_bRes = false;
	}

	return t_bRes;

	unguard;
}

//! River @ 2011-2-12:修改此函数，使处理更高效。
//! 得到可以调入的character,如果队列内没有任务，返回false.
bool osc_charLoadThread::get_frontChar( os_charLoadData& _cl )
{
	guard;

	m_bVipProcess = false;

	//  
	//! 此处必须使用CS,不同线程进行的操作,由主线程加入。
	::EnterCriticalSection( &osc_charLoadThread::m_sCharNameCS );
	// 
	// River @ 2011-2-12:对于还没有创建即要删除的人物，队列内的两个都要删除。
	//                   有两个以上的人物数组时，才删除。
	LIST_charLoadData::iterator t_iter;
	if( m_listCharLoadData.size() >= 2 )
	{
		for( t_iter = m_listCharLoadData.begin();
			t_iter != m_listCharLoadData.end(); )
		{
			if( t_iter->m_bReleaseChar )
			{
				LIST_charLoadData::iterator  t_find;
				bool                         t_bPreRelease = false;
				for( t_find = m_listCharLoadData.begin();
					t_find != t_iter;  )
				{
					if( DWORD( t_iter->m_ptrSkinPtr) != DWORD( t_find->m_ptrSkinPtr) )
					{
						++ t_find;
						continue;
					}
					
					//! River mod @ 2011-3-6:为了解决场景内刀光特效留下的BUG
					//  如果要释放的人物，前面有人物指针需要播放刀光特效或者要删除，
					//  则删除这些指针，但不需要删除操作的人物指针。
					if( (!t_find->m_bPlaySwordGlossy)&&(!t_find->m_bReleaseChar)
						&&(!t_find->m_bChangeEquipent)  )
						t_bPreRelease = true;

					// 删除创建指针结构的操作
					m_listCharLoadData.erase( t_find++ );

				}

				if( !t_bPreRelease )
				{
					++ t_iter;
					continue;
				}

				// 删除删除指针的操作
				m_listCharLoadData.erase( t_iter ++ );
			}
			else // River @ 2011-3-7：使用正确的删除iterator的方法。
				++ t_iter;


			//! 如果是最后一个，也退出.
			if( t_iter == m_listCharLoadData.end() )
				break;

			//！ 此时有可能整个数组为空的,如果为空，退出。
			if( m_listCharLoadData.size() == 0 )
				break;
		}
	}

	// VIP process的处理
	if( m_sVipChar.m_ptrSkinPtr )
	{
		//! 如果当前的char是切换装备的话，先处理在队列内此ptr一切其它的消息。
		if( m_sVipChar.m_bChangeEquipent )
		{
			for( t_iter = m_listCharLoadData.begin();
				t_iter != m_listCharLoadData.end();t_iter ++ )
			{
				if( (DWORD)t_iter->m_ptrSkinPtr != (DWORD)m_sVipChar.m_ptrSkinPtr )
					continue;

				memcpy( &_cl,(os_charLoadData*)&(*t_iter),sizeof( os_charLoadData ) );
				m_listCharLoadData.erase( t_iter );

				// 先处理vip char指针对应的其它序列消息
				::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );
				return true;
			}
		}

		memcpy( &_cl,&m_sVipChar,
			sizeof( os_charLoadData ) );
		m_bVipProcess = true;
		m_sVipChar.m_ptrSkinPtr = NULL;

		::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );

		return true;
	}

	if( m_listCharLoadData.size() == 0 )
	{
		// 查看是否是退出线程的信号。
		DWORD    t_dwWait;
		t_dwWait = ::WaitForSingleObject( m_sEventLCExitThread,0 );
		if( WAIT_TIMEOUT == t_dwWait )
		{
			// 设置等待主线程推进人物调入到此线程.	
			::ResetEvent( osc_charLoadThread::m_sEventLCStartMt );
			
			::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );
			return false;
		}
		else if( WAIT_OBJECT_0 == t_dwWait )
		{
			_cl.m_ptrSkinPtr = NULL;
			::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );

			return true;
		}
	}

	// River @ 2011-2-25:找到调入优先级别最高的character并调入。
	int  t_sLoadLevel = 0;
	LIST_charLoadData::iterator   t_iLoadIter;
	for( t_iter = m_listCharLoadData.begin();
		t_iter != m_listCharLoadData.end();t_iter ++ )
	{
		if( t_iter->m_ptrSkinPtr->get_mtLoadLevel() > t_sLoadLevel )
		{
			t_sLoadLevel = t_iter->m_ptrSkinPtr->get_mtLoadLevel();
			t_iLoadIter = t_iter;
		}
	}

	// 如果上层人为设置了调入的优先级别,则优先调入最后设置.
	if( t_sLoadLevel > 0 )
	{
		memcpy( &_cl,(os_charLoadData*)&(*t_iLoadIter),sizeof( os_charLoadData ) );
		m_listCharLoadData.erase( t_iLoadIter );
	}
	else
	{
		// 正常游戏情况下，一个个调入.
		t_iter = m_listCharLoadData.begin();
		memcpy( &_cl,(os_charLoadData*)&(*t_iter),sizeof( os_charLoadData ) );
		m_listCharLoadData.pop_front();
	}

	::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );

	return true;

	unguard;
}


/**
*  每一个派生类都必须重载的函数.
*
*  函数处理流程:
*  
*/
DWORD osc_charLoadThread::ThreadProc()
{
	guard;

	while( 1 )
	{

		// 如果没有地图,则等待地图调入的事件.如果事件发生,重复调入地图,直到完成.
		if( !get_frontChar( m_sCurLoadChar ) )
		{
			//osDebugOut( "等待人物推进后唤醒线程,如果此时程序停顿，则线程死锁!!!!\n" );

			// 如果在线程的地图调入队列内，得不到需要调入的地图，则继续等待。
			do
			{
				// 等待线程唤醒之前设为free事件
				::SetEvent( osc_charLoadThread::m_sEventLCThreadFree );

				::WaitForSingleObject( osc_charLoadThread::m_sEventLCStartMt,INFINITE );

				// 唤配后设free事件为无信号
				::ResetEvent( osc_charLoadThread::m_sEventLCThreadFree );
			}
			while( (!get_frontChar( m_sCurLoadChar )) );
			
			//osDebugOut( "等待人物推进后唤醒线程...done\n" );
		}

		// 
		// 退出线程的开关
		if( NULL == m_sCurLoadChar.m_ptrSkinPtr )
		{
			osDebugOut( "退出线程charLoad线程...break\n" );
			break;
		}

		//! 进入Reset设备的临界区
		CCriticalSec t_shadowSec(&g_sShadowSec);

		osassert( m_sCurLoadChar.m_ptrSkinPtr );
		//! 如果是要释放人物.
		if( m_sCurLoadChar.m_bReleaseChar )
		{
			m_sCurLoadChar.m_ptrSkinPtr->
				release_characterThread( m_sCurLoadChar.m_bSceneSm );
		}
		else if( m_sCurLoadChar.m_bChangeEquipent )
		{	
			m_sCurLoadChar.m_ptrSkinPtr->
				change_equipmentThread( m_sCurLoadChar.m_sEquipent );
		}
		// River @ 2011-2-25:多线程处理人物的刀光数据调入
		else if( m_sCurLoadChar.m_bPlaySwordGlossy )
		{
			m_sCurLoadChar.m_ptrSkinPtr->playSwordGlossyThread();
		}
		else
		{
			//! 此处开始真正的人物数据调入
			if( !m_sCurLoadChar.m_ptrSkinPtr->
				load_skinMeshSystem( &m_sCurLoadChar.m_sMeshInit ) )
			{
				osDebugOut( "调入人物<%s>失败...\n",m_sCurLoadChar.m_sMeshInit.m_strSMDir );
			}
			else
			{
				// 此处如果不使用CS,可能刚判断无需删除后，还没有设置之前，主线程设置为删除.
				::EnterCriticalSection( &osc_charLoadThread::m_sCharReleaseCS );
				if( !m_sCurLoadChar.m_ptrSkinPtr->get_toBeRelease() )
					m_sCurLoadChar.m_ptrSkinPtr->set_loadLock( false );
				::LeaveCriticalSection( &osc_charLoadThread::m_sCharReleaseCS );

			}
		}

		// 如果此时调用的vip process,通知.
		if( m_bVipProcess )
		{   
			m_bVipProcess = false;
			::SetEvent( m_sEventCLLoadFinish );
		}

		//! 退出reset储备的临界区
		t_shadowSec.LeaveCriticalSec();

	}

	osDebugOut( "Exit charLoad Thread...\n" );

	return TRUE;

	unguard;
}

void osc_charLoadThread::ThreadExceptionProc()
{
	// 如果此时调用的vip process,通知.
	if( m_bVipProcess )
	{
		m_bVipProcess = false;
		::SetEvent( m_sEventCLLoadFinish );
	}
}

//! 结束线程的执行
void osc_charLoadThread::end_thread( void )
{
	guard;

	// 使用地图调入处于等待状态.
	reset_loadCharMgr();

	// 
	// 先激活地图调入线程，再激活退出线程事件，如果地图调入
	// 队列为空，而此时退出线程事件有信号，则退出线程。
	::SetEvent( osc_charLoadThread::m_sEventLCExitThread );
	::SetEvent( osc_charLoadThread::m_sEventLCStartMt );

	unguard;
}



