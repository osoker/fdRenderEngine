//====================================================================================================
/** \file  
 *  Filename: com_thread.h
 *  Desc:     多线程应用的封装类,使用当前文件提供类可以用简单的接口使用多
 *            线程函数.
 *  His:      River create @ 4/11 2003 0:02
 */
//====================================================================================================
#pragma once


# include <windows.h>

# include "../common.h"

/** \brief 对多线程的封装类.
 *  
 *  使用这个类可以用简单的接口使用多线程,但需要从当前的类派生类,因为每一个
 *  线程都有自己特殊的处理函数.
 *
 */
class COMMON_API com_thread
{
protected:
	//! 线程的handle.
	HANDLE      m_threadHandle;
	//! 线程的ID.
	DWORD       m_threadID;
	//! 线程是不是正在运行.
	bool        m_bIsRunning;

public:
	/**
	 *  线程结束运行的函数.
	 */
	void End();

	/**
	 *  线程开始运行的函数.
	 */
	void Begin();


	//! 设置线程的优化级.
	void setPriority( int _pri );

	/**
	 *  线程是不是运行的函数.
	 */
	bool IsRunning()
	{
		return m_bIsRunning;
	}


	/**
	 *  每一个派生类都必须重载的函数.
	 */
	virtual DWORD ThreadProc() = 0;
	virtual void ThreadExceptionProc() = 0;

	com_thread(void)
	{
		m_threadID      = 0;
		m_threadHandle  = NULL;
		m_bIsRunning    = false;
	}

	virtual ~com_thread(void)
	{
		End();
	}

	//! 对线程运行函数的包装.
	DWORD        thread_process( void );

};


/** \ brief 对Mutex类的封装.
 *  
 *  这个类封装Mutex后,使用多线程可以安全的运行在系统中.
 *
 */
class COMMON_API com_mutex
{
	//! Mutex的句柄
	HANDLE m_mutex;


public:
	com_mutex()
	{
#if defined( _WIN32 ) && defined( _MT )
		// This mutex will help the two threads share their toys.
		m_mutex = CreateMutex( NULL, false, NULL );
		if( m_mutex == NULL )
			throw ( "cMonitor::cMonitor() - Mutex creation failed." );
#endif
	}


	virtual ~com_mutex()
	{
#if defined( _WIN32 ) && defined( _MT )
		if( m_mutex != NULL )
		{
			CloseHandle( m_mutex );
			m_mutex = NULL;
		}
#endif
	}

	/**
	 *  开始使用Mutex,进入关键数据操作区。
	 */
	void MutexOn() const
	{
#if defined( _WIN32 ) && defined( _MT )
		WaitForSingleObject( m_mutex, INFINITE );  // To be safe...
#endif
	}


	/**
	 *  离开关键数据操作区，关闭Mutex.
	 */
	void MutexOff() const
	{
#if defined( _WIN32 ) && defined( _MT )
		ReleaseMutex( m_mutex );  // To be safe...
#endif
	}


};

