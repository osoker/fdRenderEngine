//====================================================================================================
/** \file  
 *  Filename: com_thread.cpp
 *  Desc:     多线程应用的封装类,使用当前文件提供类可以用简单的接口使用多
 *            线程函数.
 *  His:      River create @ 4/11 2003 0:02
 */
//====================================================================================================

# include "stdafx.h"

#include "../include/com_thread.h"
# include "../include/sgdebug.h"
#include "../include/timecounter.h"


COMMON_API ExceptionCatchHandler_multi_thread      g_pExceptionFuncHandler_multi_thread = NULL;

static DWORD WINAPI cThreadProc( com_thread *pThis )
{
  return pThis->thread_process();
}


//! 对线程运行函数的包装.
DWORD com_thread::thread_process( void )
{

# if ( _DEBUGRUNTIME )
	CSE::MapSEToCE();
	try{	
#endif

	return ThreadProc();

# if ( _DEBUGRUNTIME )
	}catch(CSE& _se){
		if(g_pExceptionFuncHandler_multi_thread){
			(g_pExceptionFuncHandler_multi_thread)(_se);
		}

		// 线程必须执行以下对主程序的 Event 释放。不然程序不退出。
		ThreadExceptionProc();
	}
#endif

	return 1;
}



/**
 *  线程结束运行的函数.
 */
void com_thread::End()
{
#if defined( _WIN32 ) && defined( _MT )

	if( m_threadHandle != NULL )
	{
		m_bIsRunning = false;
		WaitForSingleObject( m_threadHandle, INFINITE );
		CloseHandle( m_threadHandle );
		m_threadHandle = NULL;
	}
# else

# error "必须定义win32和多线程的宏...."

# endif 

}

//SetThreadPrionrity

//! 设置线程的优化级.
void com_thread::setPriority( int _pri )
{
	guard;

	if( m_threadHandle )
	{
		if( !SetThreadPriority( m_threadHandle,_pri ) )
			osassertex( false,"设置线程优先级失败...\n" );
	}

	return ;

	unguard;
}


/**
*  线程开始运行的函数.
*/
void com_thread::Begin()
{
	guard;

	{
#if defined( _WIN32 ) && defined( _MT )
		if( m_threadHandle )
			End();  // just to be safe.

		// Start the thread.
		m_threadHandle = CreateThread( NULL,
			0,
			(LPTHREAD_START_ROUTINE)cThreadProc,
			this,
			0,
			(LPDWORD)&m_threadID );
		if( m_threadHandle == NULL )
		{
			// Arrooga! Dive, dive!  And deal with the error, too!
			osassertex( false,"创建线程失败..\n" );
		}
		m_bIsRunning = true;
		if(sg_timer::Instance()->get_cpuNum() > 1){

			if(!SetThreadAffinityMask(m_threadHandle,2)){
				DWORD t_dwError = ::GetLastError();
				t_dwError = 0;

			}
		}

# else

# error "必须定义win32和多线程的宏...."

# endif 
	}

	unguard;
}

