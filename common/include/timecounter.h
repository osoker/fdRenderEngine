//------------------------------------------------------------------------------------------
/**
 * Filename: timecounter.h
 * Desc:     global time counter func
 * His:      River created @ 11/23 2001.
 *
 * River @ 6/27 2003 把这个项目加入到osokWeb3d工程中.
 * 
 */
//------------------------------------------------------------------------------------------

#ifndef __TIMECOUNTER_INCLUDE__
#define __TIMECOUNTER_INCLUDE__


# include "windows.h"
# include "mmsystem.h"
# include "stdio.h"
# include "../common.h"
# include <assert.h>



//! 模板中有没有定义的m_ptrSingleton
#   pragma warning( disable: 4661)


/** \brief
 *   Singleton模板类.
 *
 *   ATTENTION!!! 继承此模板的类必须把建构函数和晰构函数处理为私有函数。
 *
 *   并且必须重写static 函数 Instance和 Dinstance.
 *   不然会assert出错。
 */
template <typename T> class ost_Singleton
{
protected:
	static T*   m_ptrSingleton;

public:
	// 实例化和删除实例，派生类必须重写，如果不重写，调入基类实现,assert.
	static T*   Instance( void )
	{
		assert( false );
		return NULL;

		// 以下代码为实现派生函数的代码，因为派生类的建构函数是私有函数，
		// 所以以下功能必须在派生函数中实现。
/*
		if( !m_ptrSingleton )
			m_ptrSingleton = new T;
		return m_ptrSingleton;
*/
	}
	static void DInstance( void )
	{
		assert( false );
/*
		if( m_ptrSingleton )
		{
			delete m_ptrSingleton;
			m_ptrSingleton = NULL;
		}
*/
	}

};


//! 全局的性能测试工具:

/********************************************************************************************/
/** \brief 全局的时间和帧计算类
 *  
 *  这个类用于整体的环境中,可应用于全局的时间控制和帧速度计算.
 *
 */
/********************************************************************************************/
class COMMON_API sg_timer : public ost_Singleton<sg_timer>
{
private:
	//! 程序启动到现在，总共过去的时间。
	float        total_elatime;                          

	//! 上一帧到现在，总共流逝的时间。
	float        last_elatime;                           


	float        fps;
	float		 maxFps;

	float		 averageFpsCounter;
	double		 averageFpsSum;

	int          fpscal;
	float        fpsstart;


	//! 上一帧对应的时间。
	DWORD        m_dwLastTime;


	//@{ 计算CPU 占用率使用变量
	//! cpu 占用率
	float		m_fCPUUsage;

	//! 计算 计数器
	float		m_fCPUUsageCounter;

	//! 阶段时间计数器
	double		m_dCPUIntervalTime;

	//! 之前的 FILETIME 来记录线程时间
	ULONGLONG	m_formerThreadTime;

	//! CPU 个数
	int			m_iCPUNumber;

	//@}

	sg_timer();
	~sg_timer();

public:

	/** 得到一个sg_timer的Instance指针.
	 */
	static sg_timer*    Instance( void );
	
	/** 删除一个sg_timer的Instance指针.
	 */
	static void         DInstance( void );


	/** 每一帧都必须调用的函数。
	 */ 
	void         framemove( void );                      
	
	/** 得到从程序开始到现在过去的时间.
	 */
	float        get_talelatime( void )const        { return total_elatime; }

	/** 得到上一帧过去的时间.
	 */
	float        get_lastelatime( void )const       { return last_elatime ; }

	/** 得到sg_timer计算出来的fps数目.
	 */
	float        get_fps( void )const               { return fps; }
	float		 get_maxFps(void)const	 {return maxFps;}
	float		 get_averageFps(void)const {return (float)(averageFpsSum / averageFpsCounter);}

	//! 得到CPU 使用率
	float		get_cpuUsage(void)const				{return m_fCPUUsage;}

	//! 得到CPU 数量
	int			get_cpuNum(void)const				{return m_iCPUNumber;}

	/** 得到时间的Identifier.
	 */
	WORD        get_timeId( void )const             { return (WORD)m_dwLastTime; } 

	//! 在某种特殊的情况，中断时间的执行,比如程序的暂停的情况，使上一帧过去的时间变为零
	void        suspend_lastFrameTime( void )  { last_elatime = 0.0f; }

	//! 得到某一帧的帧速
	float       get_framerate( int _idx );

};




#endif